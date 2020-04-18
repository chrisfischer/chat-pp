#include <grpcpp/grpcpp.h>

#include "proto/server_server.grpc.pb.h"

class ServerServerApi {
private:
    std::unique_ptr<server_server::MessageService::Stub> stub_;

public:
    ServerServerApi(std::shared_ptr<grpc::Channel> channel);

};