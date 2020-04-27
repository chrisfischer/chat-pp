#include <iostream>
#include <grpcpp/grpcpp.h>

#include "src/forwarding_service_impl.hpp"
#include "proto/server_server.grpc.pb.h"
#include "common.hpp"

using namespace std;

ForwardingServiceImpl::ForwardingServiceImpl(
    shared_ptr<ServerState> state,
    std::shared_ptr<ChatServiceImpl> chat_service)
    : state{state}, chat_service{chat_service} {}

grpc::Status ForwardingServiceImpl::Forward(grpc::ServerContext *context,
                                            const SS::ForwardedMessage *request,
                                            client_server::Empty *response) {
    log("Received forward from " + context->peer());
    chat_service->handle_forwarded_message(request->message(), 
                                           request->sender_id(), 
                                           request->message().room(),
                                           true);

    log("Finished handling forwarded message from " + context->peer());
    std::cout << *state;

    return grpc::Status::OK;
}