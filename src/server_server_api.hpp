#ifndef SERVER_SERVER_HPP_
#define SERVER_SERVER_HPP_

#include <grpcpp/grpcpp.h>

#include "proto/server_server.grpc.pb.h"

class ServerServerApi {
private:
    std::unique_ptr<server_server::MessageService::Stub> stub_;

public:
    ServerServerApi(std::shared_ptr<grpc::Channel> channel);

};

#endif /* SERVER_SERVER_HPP_ */