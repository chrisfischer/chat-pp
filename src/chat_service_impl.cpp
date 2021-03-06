#include "src/chat_service_impl.hpp"

#include <algorithm>
#include <iostream>

#include "src/common.hpp"

extern bool IS_VERBOSE;

using namespace std;

ChatServiceImpl::ChatServiceImpl(shared_ptr<ServerState> state, const set<string> &addrs) : state{state} {
    forwarding_clients.resize(addrs.size());
    transform(
        addrs.begin(), addrs.end(), forwarding_clients.begin(),
        [&state](string addr) -> unique_ptr<ForwardingServiceClient> {
            return make_unique<ForwardingServiceClient>(
                grpc::CreateChannel(addr, grpc::InsecureChannelCredentials()), state);
        });
}

grpc::Status ChatServiceImpl::ReceiveMessages(
        grpc::ServerContext *context,
        grpc::ServerReaderWriter<client_server::Message, client_server::Message> *stream) {
    auto sender_addr{context->peer()};

    log("New connection from " + sender_addr);

    state->register_user(context->peer());

    writers[sender_addr] = stream;

    client_server::Message read_message;
    while (stream->Read(&read_message)) {
        handle_message(read_message, sender_addr);
    };

    log(sender_addr + " disconnecting");

    writers.erase(sender_addr);

    if (state->get_room(sender_addr)) {
        // Send goodbye message.
        client_server::LeftMessage *left_message = new client_server::LeftMessage{};
        left_message->set_nickname(state->nickname_for_addr(sender_addr));
        client_server::Message message;
        message.set_allocated_left_message(left_message);
        handle_message(message, sender_addr);
    }

    state->remove_user(context->peer());

    if (IS_VERBOSE) cout << *state;

    return grpc::Status::OK;
}

void ChatServiceImpl::handle_message(client_server::Message message,
                                     const string &sender_addr) {
    log("Handling client message from " + sender_addr);

    string room;
    if (auto opt_room{state->get_room(sender_addr)}; opt_room) {
        // Normal message
        room = opt_room.value();
    } else {
        room = message.room();
    }

    if (message.has_start_vote_message()) {
        auto type{message.start_vote_message().type()};
        if (type == client_server::VoteType::JOIN) {
            // Must specify room
            if (message.room().empty()) {
                log_err("Room not included in request " + sender_addr);
                return;
            }
            if (state->get_room(sender_addr)) {
                log_err("User already in room " + sender_addr);
                return;
            }
        }
        // Must specify nickname on kick
        if (type == client_server::VoteType::KICK) {
            if (message.start_vote_message().nickname().empty()) {
                log_err("Nickname not included in request " + sender_addr);
                return;
            }
            if (message.start_vote_message().nickname() == state->nickname_for_addr(sender_addr)) {
                log_err("Cannot vote to kick yourself " + sender_addr);
                return;
            }
        }

        auto &nickname{type == client_server::VoteType::JOIN ? state->nickname_for_addr(sender_addr) : message.start_vote_message().nickname()};
        auto vote_id{state->start_vote(room, message.start_vote_message().type(), nickname)};

        auto start_vote_message_copy = new client_server::StartVoteMessage{message.start_vote_message()};
        start_vote_message_copy->set_vote_id(vote_id);
        start_vote_message_copy->set_nickname(nickname);
        message.set_allocated_start_vote_message(start_vote_message_copy);

    } else if (message.has_vote_message()) {
        // Prevent counting vote message if sender is target
        if (auto opt_vote_state{state->get_vote(message.vote_message().vote_id())};
            opt_vote_state &&
            opt_vote_state.value().target_nickname == state->nickname_for_addr(sender_addr)) {
            return;
        }
    } else if (message.has_nickname_message()) {
        auto nickname_message = new client_server::NicknameMessage{message.nickname_message()};
        nickname_message->set_old_nickname(state->nickname_for_addr(sender_addr));
        message.set_allocated_nickname_message(nickname_message);

        if (!state->get_room(sender_addr)) {
            state->set_nickname(sender_addr, message.nickname_message().new_nickname());
            writers.at(sender_addr)->Write(message);
            return;
        }
    } else if (message.has_text_message()) {
        auto text_message = new client_server::TextMessage{message.text_message()};
        text_message->set_nickname(state->nickname_for_addr(sender_addr));
        message.set_allocated_text_message(text_message);
    } else if (message.has_left_message()) {
        if (message.left_message().nickname().empty()) {
            auto left_message = new client_server::LeftMessage{message.left_message()};
            left_message->set_nickname(state->nickname_for_addr(sender_addr));
            message.set_allocated_left_message(left_message);
        }
    }

    forward(message, sender_addr, room);
    handle_forwarded_message(message, sender_addr, room, false);

    log("Finished handling client message " + sender_addr);
    if (IS_VERBOSE) cout << *state;
}

void ChatServiceImpl::forward(const client_server::Message &message,
                              const string &sender_addr,
                              const string &room) {
    log("Forwarding to " + to_string(forwarding_clients.size()) + " servers");
    for (unsigned int i = 0; i < forwarding_clients.size(); i++) {
        forwarding_clients.at(i)->Forward(sender_addr, message, room);
    }
    log("Done Forwarding");
}

void ChatServiceImpl::handle_forwarded_message(client_server::Message message,
                                               string sender_addr,
                                               const string &room,
                                               bool forwarded) {
    message.set_room(room);

    // Whether or not a vote is completed
    bool send_completed_vote{false};
    bool vote_result;
    string vote_id;

    bool send_to_kicked{false};
    bool send_to_rejected{false};

    // If it is a start vote message, the server who has the target user connected is responsible
    // for maintaining the vote state.
    if (message.has_start_vote_message()) {
        if (!forwarded) {
            if (message.start_vote_message().type() == client_server::VoteType::JOIN &&
                state->get_room_size(room) == 0) {
                send_completed_vote = true;
                vote_result = true;
                vote_id = message.start_vote_message().vote_id();
            }
        }
    } else if (message.has_vote_message()) {
        vote_id = message.vote_message().vote_id();
        if (state->has_vote(vote_id)) {
            if (!state->set_vote(vote_id, message.vote_message().vote(), sender_addr)) {
                // Already voted
                return;
            }

            if (auto complete{state->is_vote_complete(vote_id)}; complete) {
                send_completed_vote = true;
                vote_result = complete.value();
            }
        }
    } else if (message.has_vote_result_message()) {
        if (IS_VERBOSE) cout << *state;
        state->set_room_size(room, message.vote_result_message().total_number_users());

        if (auto addr = state->addr_for_nickname(message.vote_result_message().nickname()); addr) {
            if (message.vote_result_message().vote()) {
                if (message.vote_result_message().type() == client_server::VoteType::JOIN) {
                    state->join_room(addr.value(), message.room());
                } else {
                    // If they are still in this room, kick them.
                    send_to_kicked = state->leave_room_if(addr.value(), message.room());
                }
            } else {
                if (message.vote_result_message().type() == client_server::VoteType::JOIN) {
                    send_to_rejected = true;
                }
            }
            sender_addr = addr.value();
        }
    } else if (message.has_nickname_message()) {
        if (!forwarded) {
            state->set_nickname(sender_addr, message.nickname_message().new_nickname());
        }
    }

    log("Sending to " + to_string(state->addrs_in_room(room).size()) + " clients");

    // Send to relevant clients

    for (auto &addr : state->addrs_in_room(room)) {
        bool for_current_user = !forwarded && sender_addr == addr &&
                                (message.has_left_message() || message.has_vote_result_message() ||
                                 message.has_nickname_message() || message.has_vote_message());
        if (message.has_vote_result_message()) {
            if (auto target_addr = state->addr_for_nickname(message.vote_result_message().nickname()); target_addr) {
                for_current_user = for_current_user || target_addr.value() == addr;
            }
        }
        if (message.has_start_vote_message() && !message.start_vote_message().nickname().empty()) {
            if (auto target_addr = state->addr_for_nickname(message.start_vote_message().nickname()); target_addr) {
                for_current_user = for_current_user || target_addr.value() == addr;
            }
        }
        forward_to_client(message, addr, for_current_user);
    }

    if (send_to_kicked || send_to_rejected) {
        forward_to_client(message, sender_addr, true);
    }

    // Put this after so target receives confirmation
    if (message.has_left_message()) {
        state->leave_room(sender_addr);
        state->decr_room_size(room);
    }

    if (send_completed_vote) {
        if (auto opt_vote_state{state->get_vote(vote_id)}; opt_vote_state) {
            log("Sending completed vote message");
            auto vote_state{opt_vote_state.value()};

            auto total_number_users{state->get_room_size(room)};
            auto new_size = total_number_users +
                            (vote_state.vote_type == client_server::VoteType::JOIN ? 1 : -1);

            auto vote_result_message = new client_server::VoteResultMessage();
            vote_result_message->set_vote(vote_result);
            vote_result_message->set_total_number_users(new_size);
            vote_result_message->set_nickname(vote_state.target_nickname);
            vote_result_message->set_type(vote_state.vote_type);

            client_server::Message completed_message;
            completed_message.set_room(room);
            completed_message.set_allocated_vote_result_message(vote_result_message);

            forward(completed_message, "", room);
            handle_forwarded_message(completed_message, "", room, false);

            state->remove_vote(vote_id);
        }
    }
}

void ChatServiceImpl::forward_to_client(client_server::Message &message, const string &addr, bool for_current_user) {
    message.set_for_current_user(for_current_user);

    if (writers.find(addr) != writers.end()) {
        writers.at(addr)->Write(message);
    } else {
        log_err("Writer not found " + addr);
    }
}
