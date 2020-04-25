#ifndef CLIENT_SERVER_IMPL_HPP_
#define CLIENT_SERVER_IMPL_HPP_

#include <grpcpp/grpcpp.h>

#include <set>
#include <vector>

#include "forwarding_service_client.hpp"
#include "proto/client_server.grpc.pb.h"
#include "server_state.hpp"

class ChatServiceImpl final : public client_server::ChatService::Service {
   private:
    std::shared_ptr<ServerState> state;
    std::vector<std::unique_ptr<ForwardingServiceClient>> forwarding_clients;

    // Used to handle non-forwarded
    void handle_message(const grpc::ServerContext &context, 
                        const client_server::Message &request,
                        const std::string &room);
    void forward(const grpc::ServerContext &context, 
                 const client_server::Message &request,
                 const std::string &room);


   public:
    // map from client addr to writer
    // std::map<std::string,
    // std::shared_ptr<grpc::ServerWriter<client_server::Message>>> writers;
    std::map<std::string, grpc::ServerWriter<client_server::Message> *> writers;

    ChatServiceImpl(std::shared_ptr<ServerState> state, const std::set<std::string> &fwd_addrs);

    grpc::Status ReceiveMessages(
        grpc::ServerContext *context, const client_server::Empty *request,
        grpc::ServerWriter<client_server::Message> *writer) override;

    grpc::Status SendMessage(grpc::ServerContext *context,
                             const client_server::Message *request,
                             client_server::MessageResult *response) override;

    // Used to handle forawrd messages from other servers
    void handle_forwarded_message(client_server::Message message,
                                  const std::string &sender_addr,
                                  const std::string &room);

};

#endif /* CLIENT_SERVER_IMPL_HPP_ */