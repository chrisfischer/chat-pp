#include <grpcpp/grpcpp.h>

#include "proto/server_server.grpc.pb.h"
#include "server_server_api.hpp"

ServerServerApi::ServerServerApi(std::shared_ptr<grpc::Channel> channel) :
        stub_(SS::ServerServerApi::NewStub(channel)) { }