#include "src/chat_service_impl.hpp"

#include <algorithm>
#include <iostream>

ChatServiceImpl::ChatServiceImpl(std::shared_ptr<ServerState> state, const std::set<std::string> &addrs) {
    forwarding_clients.resize(addrs.size());
    std::transform(
        addrs.begin(), addrs.end(), forwarding_clients.begin(),
        [&state](std::string addr) -> std::unique_ptr<ForwardingServiceClient> {
            return std::make_unique<ForwardingServiceClient>(
                grpc::CreateChannel(addr, grpc::InsecureChannelCredentials()), state);
        });
}

grpc::Status ChatServiceImpl::ReceiveMessages(
    grpc::ServerContext *context, const client_server::Empty *request,
    grpc::ServerWriter<client_server::Message> *writer) {
    std::cout << "impl ReceiveMessages" << context->client_metadata().size();
    for (auto elt : context->client_metadata()) {
        std::cout << "Impl ReceiveMessages " << elt.first << " " << elt.second
                  << std::endl;
    }

    writers[context->peer()] =
        writer;  // std::shared_ptr<grpc::ServerWriter<client_server::Message>>{writer};

    // TODO
    while (true) {
    };
    // condition variables?

    return grpc::Status::OK;
}

grpc::Status ChatServiceImpl::SendMessage(
    grpc::ServerContext *context, const client_server::Message *request,
    client_server::MessageResult *response) {
    // TODO check for writer first

    std::cout << "Impl SendMessage " << context->peer() << std::endl;
    for (unsigned int i = 0; i < forwarding_clients.size(); i++) {
        // TODO check if peer is the addr
        forwarding_clients.at(i)->Forward(context->peer(), *request);
    }

    std::string addr{context->peer()};
    if (auto room = state->room_for_addr(addr); room) {
        // Normal message
        handle_message(*request, addr, room.value());
    } else if (request->has_start_vote_message() && !request->room().empty()) {
        // Join message
        handle_message(*request, addr, request->room());
    } else {
        std::cerr << "user not in room " << addr << std::endl;
    }

    return grpc::Status::OK;
}

// Copy message on call
void ChatServiceImpl::handle_message(
    client_server::Message message, const std::string &sender_addr, const std::string &room) {

    // If it is a start vote message, the server who has the target user connected is responsible
    // for maintaining the vote state.
    if (message.has_start_vote_message()) {
        if (auto addr = state->addr_for_nickname(message.start_vote_message().nickname()); addr) {
            std::string vote_id = state->start_vote(room, message.start_vote_message().type(), addr.value());

            auto start_vote_message_copy = new client_server::StartVoteMessage {message.start_vote_message()};
            start_vote_message_copy->set_vote_id(vote_id);
            message.set_allocated_start_vote_message(start_vote_message_copy);
        }
    }

    // Send to relevant clients
    for (auto addr : state->addrs_in_room(room)) {
        if (writers.find(addr) != writers.end()) {
            writers.at(addr)->Write(message);
        } else {
            std::cerr << "writer not found " << addr << std::endl;
        }
    }
}