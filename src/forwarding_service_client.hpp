#ifndef FORWARDING_SERIVCE_CLIENT_HPP_
#define FORWARDING_SERIVCE_CLIENT_HPP_

#include <string>

#include <grpcpp/grpcpp.h>
#include "proto/client_server.grpc.pb.h"
#include "proto/server_server.grpc.pb.h"

#include "server_state.hpp"

class ForwardingServiceClient {
   private:
    std::unique_ptr<SS::ForwardingService::Stub> stub_;
    std::shared_ptr<ServerState> state;
    grpc::CompletionQueue cq;

   public:
    ForwardingServiceClient(std::shared_ptr<grpc::Channel> channel, std::shared_ptr<ServerState> state);
    void Forward(const std::string &sender_addr,
                 const client_server::Message &message);
};

#endif /* FORWARDING_SERIVCE_CLIENT_HPP_ */
