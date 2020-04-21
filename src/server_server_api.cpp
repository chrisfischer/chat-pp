#include <grpcpp/grpcpp.h>

#include "server_server_api.hpp"
#include "proto/server_server.grpc.pb.h"

using namespace std;

MessageServiceClient::MessageServiceClient(std::shared_ptr<grpc::Channel> channel) :
        stub_(SS::MessageService::NewStub(channel)) {}


MessageServiceImpl::MessageServiceImpl(
        shared_ptr<ServerState> state) :
        SS::MessageService::Service(), state{state} {}
