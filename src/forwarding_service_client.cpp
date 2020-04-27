#include "forwarding_service_client.hpp"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <thread>

#include "proto/server_server.grpc.pb.h"

using namespace std;

ForwardingServiceClient::ForwardingServiceClient(
    std::shared_ptr<grpc::Channel> channel,
    std::shared_ptr<ServerState> state)
    : stub_{SS::ForwardingService::NewStub(channel)}, state{state} {}


void ForwardingServiceClient::Forward(const std::string &sender_addr,
                                      const client_server::Message &message,
                                      const std::string &room) {
    

    SS::ForwardedMessage request;
    request.set_sender_id(sender_addr);

    auto copy = new client_server::Message {message};
    copy->set_room(room);
    request.set_allocated_message(copy);

    grpc::ClientContext context;
    client_server::Empty response;
    stub_->Forward(&context, request, &response);

    return;
}
