#include "src/forwarding_service_impl.hpp"

#include <grpcpp/grpcpp.h>

#include <iostream>

#include "proto/server_server.grpc.pb.h"

using namespace std;

ForwardingServiceImpl::ForwardingServiceImpl(
    shared_ptr<ServerState> state,
    std::shared_ptr<ChatServiceImpl> chat_service)
    : state{state}, chat_service{chat_service} {}

grpc::Status ForwardingServiceImpl::Forward(grpc::ServerContext *context,
                                            const SS::ForwardedMessage *request,
                                            client_server::Empty *response) {
    cout << request << endl;



    return grpc::Status::OK;
}