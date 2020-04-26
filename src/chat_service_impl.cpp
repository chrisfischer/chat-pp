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

    writers[context->peer()] = stream;

    client_server::Message read_message;
    while (stream->Read(&read_message)) {
        // TODO async
        handle_message(context, &read_message);
    };

    std::cout << "Leaving ReceiveMessages\n";
    return grpc::Status::OK;
}

void ChatServiceImpl::handle_message(grpc::ServerContext *context,
                                     const client_server::Message *request) {

    std::cout << "Handling client message " << context->peer() << std::endl;

    std::string addr{context->peer()};
    std::string room;
    if (auto opt_room {state->room_for_addr(addr)}; opt_room) {
        // Normal message
        room = opt_room.value();
    } else {
        room = request->room();
    }

    if (request->has_start_vote_message()) {
        // Must specify room
        if (request->room().empty()) {
            std::cerr << "room not included in request " << addr << std::endl;
            return;
        }
        if (request->start_vote_message().type() == client_server::VoteType::JOIN && state->get_room(addr)) {
            std::cerr << "already in room " << addr << std::endl;
            return;
        }
        // Must specify nickname on kick
        if (request->start_vote_message().type() == client_server::VoteType::KICK) {
            if (request->start_vote_message().nickname().empty()) {
                std::cerr << "nickname not included in request " << addr << std::endl;
                return;
            }
            if (request->start_vote_message().nickname() == state->nickname_for_addr(context->peer())) {
                std::cerr << "cannot vote to kick yourself " << addr << std::endl;
                return;
            }
        }

    } else if (request->has_vote_message()) {
        // Prevent counting vote message if sender is target
        if (auto opt_vote_state = state->get_vote(request->vote_message().vote_id());
            opt_vote_state && opt_vote_state.value().target_addr == context->peer()) {
            return;
        }
    }

    forward(*request, addr, room);
    handle_forwarded_message(*request, addr, room, false);

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
        if (!forwarded) {
            vote_id = state->start_vote(room, message.start_vote_message().type(), sender_addr);

            auto start_vote_message_copy = new client_server::StartVoteMessage{message.start_vote_message()};
            start_vote_message_copy->set_vote_id(vote_id);
            start_vote_message_copy->set_nickname(state->nickname_for_addr(sender_addr));
            message.set_allocated_start_vote_message(start_vote_message_copy);

            if (message.start_vote_message().type() == client_server::VoteType::JOIN &&
                    state->get_room_size(room) == 0) {
                send_completed_vote = true;
                vote_result = true;
            }

        } else if (auto addr = state->addr_for_nickname(message.start_vote_message().nickname()); addr) {
            // TODO combine with the above
            std::string vote_id = state->start_vote(room, message.start_vote_message().type(), addr.value());

            auto start_vote_message_copy = new client_server::StartVoteMessage{message.start_vote_message()};
            start_vote_message_copy->set_vote_id(vote_id);
            message.set_allocated_start_vote_message(start_vote_message_copy);
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
        state->set_room_size(room, message.vote_result_message().total_number_users());

        if (!forwarded) {
            // Target user must be connected here, we pass that address in as the sender
            if (message.vote_result_message().type() == client_server::VoteType::JOIN) {
                state->join_room(sender_addr, message.room());
            } else {
                state->leave_room_if(sender_addr, message.room());
            }
        }
    } else if (message.has_nickname_message()) {
        if (!forwarded) {
            state->update_nickname(sender_addr, message.nickname_message().new_nickname());
        }
    } else if (message.has_left_message()) {
        state->leave_room(sender_addr);
    }

    std::cout << "Sending to clients\n";

    // Send to relevant clients

    client_server::LeftMessage *left_message = nullptr;
    client_server::VoteResultMessage *vote_result_message = nullptr;

    for (auto addr : state->addrs_in_room(room)) {
        std::cout << "\t" + addr << std::endl;

        if (!forwarded && sender_addr == addr) {
            if (message.has_left_message()) {
                left_message = new client_server::LeftMessage{message.left_message()};
                auto for_current = new client_server::LeftMessage{message.left_message()};
                for_current->set_for_current_user(true);
                message.set_allocated_left_message(for_current);
            } else if (message.has_vote_result_message()) {
                vote_result_message = new client_server::VoteResultMessage{message.vote_result_message()};
                auto for_current = new client_server::VoteResultMessage{message.vote_result_message()};
                for_current->set_for_current_user(true);
                message.set_allocated_vote_result_message(for_current);
            }
        }

        // TODO for vote result, set is for current user

        if (writers.find(addr) != writers.end()) {
            writers.at(addr)->Write(message);
        } else {
            std::cerr << "writer not found " << addr << std::endl;
        }

        if (!forwarded && sender_addr == addr) {
            if (message.has_left_message()) {
                message.set_allocated_left_message(left_message);
            } else if (message.has_vote_result_message()) {
                message.set_allocated_vote_result_message(vote_result_message);
            }
        }
    }

    // TODO send vote completed message to everyone
    // TODO if room size is 0 also send
    if (send_completed_vote) {
        std::cout << "Sending completed vote\n";
        // TODO the target addr only matters for this server, but currently we'd have to
        // find the current user by nickname which is not ideal

        if (auto opt_vote_state{state->get_vote(vote_id)}; opt_vote_state) {
            auto vote_state{opt_vote_state.value()};

            auto total_number_users{state->get_room_size(room)};
            auto new_size = total_number_users +
                (vote_state.vote_type == client_server::VoteType::JOIN ? 1 : -1);

            client_server::Message completed_message;
            auto vote_result_message = new client_server::VoteResultMessage();
            vote_result_message->set_vote(vote_result);
            vote_result_message->set_total_number_users(new_size);
            vote_result_message->set_nickname(state->nickname_for_addr(vote_state.target_addr));

            completed_message.set_room(room);
            completed_message.set_allocated_vote_result_message(vote_result_message);

            // TODO async?
            // TODO bring into separate function
            forward(completed_message, vote_state.target_addr, room);
            handle_forwarded_message(completed_message, vote_state.target_addr, room, false);

            state->remove_vote(vote_id);

        } else {
            // std::cerr < "vote not found " << vote_id << std:endl;
        }
    }
}
