#ifndef CLIENT_SERVER_IMPL_HPP_
#define CLIENT_SERVER_IMPL_HPP_

#include <vector>
#include <grpcpp/grpcpp.h>

#include "proto/client_server.grpc.pb.h"
#include "src/server_server_api.hpp"

class ChatServiceImpl final : public client_server::ChatService::Service {
private:
    std::vector<std::unique_ptr<MessageServiceClient>> messaging_clients;
public:
    // TODO use map in constructor
    ChatServiceImpl(const std::vector<std::string> & addrs);
    grpc::Status SendMessage(grpc::ServerContext* context,
            const client_server::Message* request,
            client_server::MessageResult* response) override;
};

#endif /* CLIENT_SERVER_IMPL_HPP_ */