#include "src/chat_service_impl.hpp"

#include <algorithm>
#include <iostream>

ChatServiceImpl::ChatServiceImpl(std::shared_ptr<ServerState> state, const std::set<std::string> &addrs) : state{state} {
    forwarding_clients.resize(addrs.size());
    std::transform(
        addrs.begin(), addrs.end(), forwarding_clients.begin(),
        [&state](std::string addr) -> std::unique_ptr<ForwardingServiceClient> {
            return std::make_unique<ForwardingServiceClient>(
                grpc::CreateChannel(addr, grpc::InsecureChannelCredentials()), state);
        });
}

grpc::Status ChatServiceImpl::ReceiveMessages(
    grpc::ServerContext* context,
    grpc::ServerReaderWriter<client_server::Message, client_server::Message>* stream) {

    std::cout << "impl ReceiveMessages\n";

    auto sender_addr{context->peer()};
    state->set_nickname(context->peer(), context->peer());
    // TODO add lock
    writers[sender_addr] = stream;

    client_server::Message read_message;
    while (stream->Read(&read_message)) {
        // TODO async
        handle_message(read_message, sender_addr);
    };    

    // TODO send leaving message if in a chat room
    std::cout << "Leaving ReceiveMessages\n";
    writers.erase(sender_addr);
    state->remove_user(context->peer());

    return grpc::Status::OK;
}

void ChatServiceImpl::handle_message(client_server::Message message,
                                     const std::string &sender_addr) {
    
    std::cout << "Handling client message " << sender_addr << std::endl;

    std::string room;
    if (auto opt_room {state->room_for_addr(sender_addr)}; opt_room) {
        // Normal message
        room = opt_room.value();
    } else {
        room = message.room();
    }

    if (message.has_start_vote_message()) {
        // Must specify room
        if (message.room().empty()) {
            std::cerr << "room not included in request " << sender_addr << std::endl;
            return;
        }
        auto type{message.start_vote_message().type()};
        if (type == client_server::VoteType::JOIN && state->get_room(sender_addr)) {
            std::cerr << "already in room " << sender_addr << std::endl;
            return;
        }
        // Must specify nickname on kick
        if (type == client_server::VoteType::KICK) {
            if (message.start_vote_message().nickname().empty()) {
                std::cerr << "nickname not included in request " << sender_addr << std::endl;
                return;
            }
            if (message.start_vote_message().nickname() == state->nickname_for_addr(sender_addr)) {
                std::cerr << "cannot vote to kick yourself " << sender_addr << std::endl;
                return;
            }
        }

        // TODO only copy message if necessary
        auto nickname = type == client_server::VoteType::JOIN ? 
            state->nickname_for_addr(sender_addr) : message.start_vote_message().nickname();
        auto vote_id = state->start_vote(room, message.start_vote_message().type(), nickname);

        auto start_vote_message_copy = new client_server::StartVoteMessage{message.start_vote_message()};
        start_vote_message_copy->set_vote_id(vote_id);
        start_vote_message_copy->set_nickname(nickname);
        message.set_allocated_start_vote_message(start_vote_message_copy);

    } else if (message.has_vote_message()) {
        // Prevent counting vote message if sender is target
        if (auto opt_vote_state = state->get_vote(message.vote_message().vote_id());
            opt_vote_state && opt_vote_state.value().target_nickname == state->nickname_for_addr(sender_addr)) {
            return;
        }
    } else if (message.has_nickname_message()) {
        // TODO get old nickname
        auto nickname_message = new client_server::NicknameMessage{message.nickname_message()};
        nickname_message->set_old_nickname(state->nickname_for_addr(sender_addr));
    } else if (message.has_text_message()) {
        auto text_message = new client_server::TextMessage{message.text_message()};
        text_message->set_nickname(state->nickname_for_addr(sender_addr));
    }

    forward(message, sender_addr, room);
    handle_forwarded_message(message, sender_addr, room, false);
    
    std::cout << "Finished handling client message\n";
    std::cout << *state;
}


void ChatServiceImpl::forward(const client_server::Message &message,
                              const std::string &sender_addr,
                              const std::string &room) {
    // TODO change to foreach + functional
    std::cout << "Forwarding to " << forwarding_clients.size() << " servers\n";
    for (unsigned int i = 0; i < forwarding_clients.size(); i++) {
        forwarding_clients.at(i)->Forward(sender_addr, message, room);
    }
}

void ChatServiceImpl::handle_forwarded_message(client_server::Message message,
                                               const std::string &sender_addr,
                                               const std::string &room,
                                               bool forwarded) {

    message.set_room(room);

    // Whether or not a vote is completed
    bool send_completed_vote{false};
    bool vote_result;
    std::string vote_id;

    // If it is a start vote message, the server who has the target user connected is responsible
    // for maintaining the vote state.
    if (message.has_start_vote_message()) {
        // TODO any time we are changing the message here, it is only being fowarded to that server's clients

        if (!forwarded) {
            if (message.start_vote_message().type() == client_server::VoteType::JOIN &&
                    state->get_room_size(room) == 0) {
                send_completed_vote = true;
                vote_result = true;
                vote_id = message.start_vote_message().vote_id();
            }
        } else {
            // if (auto addr = state->addr_for_nickname(message.start_vote_message().nickname()); addr) {
            //     // TODO combine with the above
            //     std::string vote_id = state->start_vote(room, message.start_vote_message().type(), addr.value());

            //     auto start_vote_message_copy = new client_server::StartVoteMessage{message.start_vote_message()};
            //     start_vote_message_copy->set_vote_id(vote_id);
            //     message.set_allocated_start_vote_message(start_vote_message_copy);
            // }
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
        std::cout << *state;
        state->set_room_size(room, message.vote_result_message().total_number_users());

        if (auto addr = state->addr_for_nickname(message.vote_result_message().nickname()); addr) {
            if (message.vote_result_message().type() == client_server::VoteType::JOIN) {
                state->join_room(addr.value(), message.room());
            } else {
                state->leave_room_if(addr.value(), message.room());
            }
        }
    } else if (message.has_nickname_message()) {
        if (!forwarded) {
            state->set_nickname(sender_addr, message.nickname_message().new_nickname());
        }
    } else if (message.has_left_message()) {
        state->leave_room(sender_addr);
    }

    std::cout << "Sending to clients\n";

    // Send to relevant clients

    // TODO send rejection if join unsuccessful
    for (auto addr : state->addrs_in_room(room)) {
        std::cout << "\t" + addr << std::endl;

        // TODO does this actually work
        if (!forwarded && sender_addr == addr && 
                (message.has_left_message() || message.has_vote_result_message())) {
            message.set_for_current_user(true);
        }

        if (writers.find(addr) != writers.end()) {
            writers.at(addr)->Write(message);
        } else {
            std::cerr << "writer not found " << addr << std::endl;
        }

        if (!forwarded && sender_addr == addr &&
                (message.has_left_message() || message.has_vote_result_message())) {
            message.set_for_current_user(false);
        }
    }

    if (send_completed_vote) {
        std::cout << "Sending completed vote\n";
        if (auto opt_vote_state{state->get_vote(vote_id)}; opt_vote_state) {
            auto vote_state{opt_vote_state.value()};

            auto total_number_users{state->get_room_size(room)};
            auto new_size = total_number_users +
                (vote_state.vote_type == client_server::VoteType::JOIN ? 1 : -1);

            auto vote_result_message = new client_server::VoteResultMessage();
            vote_result_message->set_vote(vote_result);
            vote_result_message->set_total_number_users(new_size);
            vote_result_message->set_nickname(vote_state.target_nickname);

            client_server::Message completed_message;
            completed_message.set_room(room);
            completed_message.set_allocated_vote_result_message(vote_result_message);

            // TODO async?
            // TODO bring into separate function
            forward(completed_message, "", room);
            handle_forwarded_message(completed_message, "", room, false);

            state->remove_vote(vote_id);

        } else {
            // std::cerr < "vote not found " << vote_id << std:endl;
        }
    }
}
