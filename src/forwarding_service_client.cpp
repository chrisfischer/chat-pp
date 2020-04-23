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
                                      const client_server::Message &message) {
    // std::thread([&stub_](){
    //     stub_->A
    // }).detach();

    auto room = state->room_for_addr(sender_addr);
    if (room.has_value()) {
        return;
    }

    SS::ForwardedMessage request;

    request.set_sender_id(sender_addr);

    auto copy = new client_server::Message {message};
    copy->set_room(room.value());

    request.set_allocated_message(copy);

    // switch (message.oneof_message_case()) {
    //     case client_server::Message::OneofMessageCase::kTextMessage: {
    //         auto copy = new client_server::TextMessage{message.text_message()};
    //         request.set_allocated_text_message(copy);
    //         break;
    //     }
    //     case client_server::Message::OneofMessageCase::kNicknameMessage: {
    //         auto copy =
    //             new client_server::NicknameMessage{message.nickname_message()};
    //         request.set_allocated_nickname_message(copy);
    //         break;
    //     }
    //     case client_server::Message::OneofMessageCase::kLeftMessage: {
    //         auto copy = new client_server::LeftMessage{message.left_message()};
    //         request.set_allocated_left_message(copy);
    //         break;
    //     }
    //     case client_server::Message::OneofMessageCase::kStartVoteMessage: {
    //         auto copy =
    //             new client_server::StartVoteMessage{message.start_vote_message()};
    //         request.set_allocated_start_vote_message(copy);
    //         break;
    //     }
    //     case client_server::Message::OneofMessageCase::kVoteMessage: {
    //         auto copy = new client_server::VoteMessage{message.vote_message()};
    //         request.set_allocated_vote_message(copy);
    //         break;
    //     }
    //     default:
    //         return;
    // }

    // TODO this is def broken
    grpc::ClientContext context;
    stub_->AsyncForward(&context, request, &cq);

    return;
}
