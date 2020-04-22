#include <grpcpp/grpcpp.h>

#include <iostream>

#include "src/forwarding_service_impl.hpp"
#include "proto/server_server.grpc.pb.h"

using namespace std;

ForwardingServiceImpl::ForwardingServiceImpl(
    shared_ptr<ServerState> state, std::shared_ptr<ChatServiceImpl> chat_service) : state{state}, chat_service{chat_service} {}

grpc::Status ForwardingServiceImpl::Forward(grpc::ServerContext *context,
                                            const SS::MessageRequest *request,
                                            client_server::Empty *response)
{
    cout << request << endl;

    for (auto addr : state->addrs_in_room(request->room())) {
        if (chat_service->writers.find(addr) != chat_service->writers.end()) {
            // TODO better way to convert between messages?
            // chat_service->writers.at(addr)->Write()
        }
    }

    return grpc::Status::OK;
}