#ifndef SERVER_SERVER_HPP_
#define SERVER_SERVER_HPP_

#include <grpcpp/grpcpp.h>
#include <string>

#include "src/server_state.hpp"

#include "proto/client_server.grpc.pb.h"
#include "proto/server_server.grpc.pb.h"

class MessageServiceClient {
private:
    std::unique_ptr<SS::MessageService::Stub> stub_;
    grpc::CompletionQueue cq;

public:
    MessageServiceClient(std::shared_ptr<grpc::Channel> channel);
    void Forward(const std::string & sender_addr, const client_server::Message & message);
};

class MessageServiceImpl final : public SS::MessageService::Service {
private:
    std::shared_ptr<ServerState> state;

public:
    MessageServiceImpl(std::shared_ptr<ServerState> state);
    grpc::Status Forward(grpc::ServerContext* context,
            const SS::MessageRequest* request,
            client_server::Empty* response) override;
};

#endif /* SERVER_SERVER_HPP_ */