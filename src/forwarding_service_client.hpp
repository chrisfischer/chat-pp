#ifndef FORWARDING_SERIVCE_CLIENT_HPP_
#define FORWARDING_SERIVCE_CLIENT_HPP_

#include <grpcpp/grpcpp.h>

#include <string>

#include "proto/client_server.grpc.pb.h"
#include "proto/server_server.grpc.pb.h"

class ForwardingServiceClient {
   private:
    std::unique_ptr<SS::ForwardingService::Stub> stub_;
    grpc::CompletionQueue cq;

   public:
    ForwardingServiceClient(std::shared_ptr<grpc::Channel> channel);
    void Forward(const std::string &sender_addr,
                 const client_server::Message &message);
};

#endif /* FORWARDING_SERIVCE_CLIENT_HPP_ */
