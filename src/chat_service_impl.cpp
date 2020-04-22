#include <algorithm>

#include <iostream>

#include "src/chat_service_impl.hpp"

ChatServiceImpl::ChatServiceImpl(const std::set<std::string> &addrs)
{
    forwarding_clients.resize(addrs.size());
    std::transform(addrs.begin(), addrs.end(), forwarding_clients.begin(),
                   [](std::string addr) -> std::unique_ptr<ForwardingServiceClient> {
                       return std::make_unique<ForwardingServiceClient>(
                           grpc::CreateChannel(addr, grpc::InsecureChannelCredentials()));
                   });
}

grpc::Status ChatServiceImpl::ReceiveMessages(grpc::ServerContext *context,
                                              const client_server::Empty *request,
                                              grpc::ServerWriter<client_server::Message> *writer)
{
    writers["TODO get IP"] = std::shared_ptr<grpc::ServerWriter<client_server::Message>>{writer};

    // TODO
    // while(true) {};
    // condition variables?

    return grpc::Status::OK;
}

grpc::Status ChatServiceImpl::SendMessage(grpc::ServerContext *context,
                                          const client_server::Message *request,
                                          client_server::MessageResult *response)
{

    // TODO check for writer first

    std::cout << context->peer() << std::endl;
    for (int i = 0; i < forwarding_clients.size(); i++)
    {
        // TODO check if peer is the addr
        forwarding_clients.at(i)->Forward(context->peer(), *request);
    }

    return grpc::Status::OK;
}