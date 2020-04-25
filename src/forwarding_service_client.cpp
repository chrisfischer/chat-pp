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

void send_async() {}

void ForwardingServiceClient::Forward(const std::string &sender_addr,
                                      const client_server::Message &message,
                                      const std::string &room) {
    // std::thread([&stub_](){
    //     stub_->A
    // }).detach();

    // auto room = state->room_for_addr(sender_addr);
    // if (!room.has_value()) {
    //     return;
    // }

    SS::ForwardedMessage request;
    request.set_sender_id(sender_addr);

    auto copy = new client_server::Message {message};
    copy->set_room(room);
    request.set_allocated_message(copy);

    cout << "Forwarding" << endl;

    // TODO this is def broken
    grpc::ClientContext context;
    // stub_->AsyncForward(&context, request, &cq);

    client_server::Empty response;
    stub_->Forward(&context, request, &response);

    return;
}
