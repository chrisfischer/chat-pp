#include <string>
#include <grpcpp/grpcpp.h>

#include "proto/client_server.grpc.pb.h"
#include "proto/client_server.pb.h"
#include "src/client_server_api.hpp"

using namespace std;

ClientServerAPI::ClientServerAPI(std::shared_ptr<grpc::Channel> channel) : stub_(client_server::ChatService::NewStub(channel)) {}

shared_ptr<grpc::ClientReader<client_server::Message>> ClientServerAPI::get_reader() {
    grpc::ClientContext context;
    
    shared_ptr<grpc::ClientReader<client_server::Message>> reader(stub_->ReceiveMessages(&context, client_server::Empty()));
    return reader;
}

bool ClientServerAPI::send_message(client_server::Message &msg) {
    client_server::MessageResult msgResult;
    grpc::ClientContext context;
    grpc::Status status = stub_->SendMessage(&context, msg, &msgResult);
    return msgResult.received();
}

bool ClientServerAPI::send_text(string &text) {
    client_server::TextMessage text_msg;
}

bool ClientServerAPI::change_nickname(string &new_nickname) {
    client_server::NicknameMessage msg;
}

bool ClientServerAPI::leave_room() {
    client_server::LeftMessage msg;
}

bool ClientServerAPI::join_room(string &room) {
    client_server::StartVoteMessage msg;
}

bool ClientServerAPI::submit_vote(string &room, string &vote_id, bool vote) {
    client_server::StartVoteMessage msg;
}

