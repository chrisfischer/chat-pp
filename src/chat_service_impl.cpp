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

    writers[context->peer()] =
        stream;  // std::shared_ptr<grpc::ServerWriter<client_server::Message>>{writer};

    // TODO
    // std::cout << "Infinite loop!" << std::endl;
    client_server::Message read_message;
    while (stream->Read(&read_message)) {
        // TODO async
        handle_message(context, &read_message);
    };
    

    return grpc::Status::OK;
}

void ChatServiceImpl::handle_message(grpc::ServerContext *context, 
                                     const client_server::Message *request) {
    
    std::cout << "Impl SendMessage " << context->peer() << std::endl;

    std::string addr{context->peer()};
    std::string room;
    if (auto room {state->room_for_addr(addr)}; room) {
        // Normal message
        room = room.value();
    } else if (request->has_start_vote_message()) {
        // Join message
        if (!request->room().empty()) {
            std::cerr << "room not included in request " << addr << std::endl;
            return;
        }

        room = request->room();
    }

    if (request->has_vote_message()) {
        // Prevent counting vote message if sender is target
        if (auto opt_vote_state = state->get_vote(request->vote_message().vote_id());
            opt_vote_state && opt_vote_state.value().target_addr == context->peer()) {
            return;
        }

    }

    forward(*context, *request, room);
    handle_forwarded_message(*request, context->peer(), room, false);
    
    std::cout << "Finished handling client message\n";
    std::cout << *state;
}


void ChatServiceImpl::forward(const grpc::ServerContext &context,
                              const client_server::Message &request,
                              const std::string &room) {
    // TODO change to foreach + functional
    for (unsigned int i = 0; i < forwarding_clients.size(); i++) {
        forwarding_clients.at(i)->Forward(context.peer(), request, room);
    }
}

void ChatServiceImpl::handle_forwarded_message(client_server::Message message, 
                                               const std::string &sender_addr,
                                               const std::string &room,
                                               bool forwarded) {

    message.set_room(room);

    bool send_completed_vote{false};
    bool vote_result;
    std::string vote_id;

    // If it is a start vote message, the server who has the target user connected is responsible
    // for maintaining the vote state.
    if (message.has_start_vote_message()) {
        if (auto addr = state->addr_for_nickname(message.start_vote_message().nickname()); addr) {
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
    } else if (message.has_nickname_message()) {
        if (!forwarded) {
            state->update_nickname(sender_addr, message.nickname_message().new_nickname());
        }
    }

    // Send to relevant clients
    for (auto addr : state->addrs_in_room(room)) {
        // TODO for vote result, set is for current user

        if (writers.find(addr) != writers.end()) {
            writers.at(addr)->Write(message);
        } else {
            std::cerr << "writer not found " << addr << std::endl;
        }
    }

    // TODO send vote completed message to everyone
    // TODO if room size is 0 also send
    if (send_completed_vote) {
        // TODO the target addr only matters for this server, but currently we'd have to 
        // find the current user by nickname which is not ideal
        
        if (auto opt_vote_state{state->get_vote(vote_id)}; opt_vote_state) {
            auto vote_state{opt_vote_state.value()};
            if (auto nickname{state->nickname_for_addr(vote_state.target_addr)}; nickname) {
                if (auto total_number_users{state->get_room_size(room)}; total_number_users) {

                    auto new_size = total_number_users.value() + 
                        (vote_state.vote_type == client_server::VoteType::JOIN ? 1 : -1);
                    
                    client_server::Message completed_message;
                    auto vote_result_message = new client_server::VoteResultMessage();
                    vote_result_message->set_vote(vote_result);
                    vote_result_message->set_nickname(nickname.value());
                    vote_result_message->set_total_number_users(new_size);
                    
                    completed_message.set_room(room);
                    completed_message.set_allocated_vote_result_message(vote_result_message);

                    grpc::ServerContext context;
                    // TODO async?
                    // TODO bring into separate function
                    forward(context, completed_message, room);
                    handle_forwarded_message(completed_message, "", room, false);

                    state->remove_vote(vote_id);
                } else {
                    // std::cerr < "room not found " << room << std:endl;    
                }
            } else {
                // std::cerr < "target addr not found " << target_addr.value() << std:endl;    
            }
        } else {
            // std::cerr < "vote not found " << vote_id << std:endl;
        }
    }
}