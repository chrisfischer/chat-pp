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

    return grpc::Status::OK;
}