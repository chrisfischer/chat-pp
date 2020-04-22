#include <string>
#include <grpcpp/grpcpp.h>

#include "proto/client_server.grpc.pb.h"
#include "src/client_server_api.hpp"

using namespace std;

private MessageResult ClientServerAPI::send_message(const Message &msg, MessageResult* msgResult) {
    ClientContext context;
    Status status = stub_->SendMessage(&context, msg, msgResult);
}

ClientServerAPI::ClientServerAPI(std::shared_ptr<grpc::Channel> channel) : channel {channel} {}

bool ClientServerAPI::send_text(string &text) {
    TextMessage msg;
}

bool ClientServerAPI::change_nickname(string &new_nickname) {
    NicknameMessage msg;
}

bool ClientServerAPI::leave_room() {
    LeftMessage msg;
}

bool ClientServerAPI::join_room(string &room) {
    StartVoteMessage msg;
}

bool ClientServerAPI::submit_vote(string &room, string &vote_id, bool vote) {
    StartVoteMessage msg;
}
