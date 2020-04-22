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
    text_msg.set_text(text);
    
    client_server::Message msg;
    msg.set_allocated_text_message(&text_msg);
    msg.set_room(room);
    return ClientServerAPI::send_message(msg);
}

bool ClientServerAPI::change_nickname(string &new_nickname) {
    client_server::NicknameMessage nn_msg;
    nn_msg.set_old_nickname(nickname);
    nn_msg.set_old_nickname(new_nickname);
    
    client_server::Message msg;
    msg.set_allocated_nickname_message(&nn_msg);
    msg.set_room(room);
    bool received = ClientServerAPI::send_message(msg);
    if (received) {
        nickname = new_nickname;
    }
    return received;
}

bool ClientServerAPI::leave_room() {
    client_server::LeftMessage left_msg;
    left_msg.set_nickname(nickname);
    
    client_server::Message msg;
    msg.set_allocated_left_message(&left_msg);
    msg.set_room(room);
    bool received = ClientServerAPI::send_message(msg);
    if (received) {
        room = "";
    }
    return received;
}

bool ClientServerAPI::join_room(string &new_room, string &new_nickname) {
    client_server::StartVoteMessage sv_msg;
    sv_msg.set_nickname(new_nickname);
    
    client_server::Message msg;
    msg.set_allocated_start_vote_message(&sv_msg);
    msg.set_room(new_room);
    return ClientServerAPI::send_message(msg);
}

bool ClientServerAPI::submit_vote(string &room, string &vote_id, bool vote) {
    client_server::VoteMessage vote_msg;
    vote_msg.set_vote_id(vote_id);
    vote_msg.set_vote(vote);
    
    client_server::Message msg;
    msg.set_allocated_vote_message(&vote_msg);
    msg.set_room(room);
    return ClientServerAPI::send_message(msg);
}

void ClientServerAPI::update_room(string &new_room) {
    room = new_room;
}
