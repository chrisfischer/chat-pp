#ifndef CLIENT_SERVER_IMPL_HPP_
#define CLIENT_SERVER_IMPL_HPP_

#include <grpcpp/grpcpp.h>

#include <set>
#include <vector>

#include "forwarding_service_client.hpp"
#include "proto/client_server.grpc.pb.h"

class ChatServiceImpl final : public client_server::ChatService::Service {
   private:
    std::vector<std::unique_ptr<ForwardingServiceClient>> forwarding_clients;

   public:
    // map from client addr to writer
    // std::map<std::string,
    // std::shared_ptr<grpc::ServerWriter<client_server::Message>>> writers;
    std::map<std::string, grpc::ServerWriter<client_server::Message> *> writers;

    // TODO use map in constructor
    ChatServiceImpl(const std::set<std::string> &addrs);

    grpc::Status ReceiveMessages(
        grpc::ServerContext *context, const client_server::Empty *request,
        grpc::ServerWriter<client_server::Message> *writer) override;

    grpc::Status SendMessage(grpc::ServerContext *context,
                             const client_server::Message *request,
                             client_server::MessageResult *response) override;
};

#endif /* CLIENT_SERVER_IMPL_HPP_ */