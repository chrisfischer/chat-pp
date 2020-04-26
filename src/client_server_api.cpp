#include "src/client_server_api.hpp"

#include <grpcpp/grpcpp.h>

#include <string>

#include "proto/client_server.grpc.pb.h"
#include "proto/client_server.pb.h"

using namespace std;

ClientServerAPI::ClientServerAPI(
  std::shared_ptr<grpc::Channel> channel) : stub_(
    client_server::ChatService::NewStub(channel)) {
    stream = start_stream();
  }

shared_ptr<grpc::ClientReaderWriter<client_server::Message,
  client_server::Message>> ClientServerAPI::start_stream() {

  shared_ptr<grpc::ClientReaderWriter<client_server::Message,
      client_server::Message>> stream(stub_->ReceiveMessages(&context));
  return stream;
}

shared_ptr<grpc::ClientReaderWriter<client_server::Message,
  client_server::Message>> ClientServerAPI::get_stream() {

  return stream;
}

bool ClientServerAPI::in_room() {
  return room.compare("");
}

void ClientServerAPI::send_message(client_server::Message &msg) {
    stream->Write(msg);
}

void ClientServerAPI::send_text(const string &text) {
    client_server::TextMessage *text_msg = new client_server::TextMessage();
    text_msg->set_text(text);

    client_server::Message msg;
    msg.set_allocated_text_message(text_msg);
    ClientServerAPI::send_message(msg);
}

void ClientServerAPI::change_nickname(const string &new_nickname) {
    client_server::NicknameMessage *nn_msg = new client_server::NicknameMessage();
    nn_msg->set_new_nickname(new_nickname);

    client_server::Message msg;
    msg.set_allocated_nickname_message(nn_msg);
    ClientServerAPI::send_message(msg);
}

void ClientServerAPI::leave_room() {
    client_server::LeftMessage *left_msg = new client_server::LeftMessage();

    client_server::Message msg;
    msg.set_allocated_left_message(left_msg);
    ClientServerAPI::send_message(msg);
}

void ClientServerAPI::join_room(const string &new_room) {
    client_server::StartVoteMessage *sv_msg = new client_server::StartVoteMessage();
    sv_msg->set_type(client_server::VoteType::JOIN);

    client_server::Message msg;
    msg.set_allocated_start_vote_message(sv_msg);
    msg.set_room(new_room);
    ClientServerAPI::send_message(msg);
}

void ClientServerAPI::kick(const string &nickname) {
    client_server::StartVoteMessage * sv_msg =
      new client_server::StartVoteMessage();
    sv_msg->set_type(client_server::KICK);
    sv_msg->set_nickname(nickname);

    client_server::Message msg;
    msg.set_allocated_start_vote_message(sv_msg);
    ClientServerAPI::send_message(msg);
}

void ClientServerAPI::submit_vote(const string &vote_id, bool vote) {
    client_server::VoteMessage *vote_msg = new client_server::VoteMessage();
    vote_msg->set_vote_id(vote_id);
    vote_msg->set_vote(vote);

    client_server::Message msg;
    msg.set_allocated_vote_message(vote_msg);
    ClientServerAPI::send_message(msg);
}

string ClientServerAPI::process_text_msg(client_server::Message &msg) {
    return msg.room() + " (" + msg.text_message().nickname() + ") > " +
      msg.text_message().text();
}

string ClientServerAPI::process_nickname_msg(client_server::Message &msg) {
    if (msg.for_current_user()) {
        nickname = msg.nickname_message().new_nickname();
    }
    return msg.room() + " > " + msg.nickname_message().old_nickname() +
        " has changed their nickname to " +
        msg.nickname_message().new_nickname() + ".";
}

string ClientServerAPI::process_start_vote_msg(client_server::Message &msg) {
    if (msg.start_vote_message().type() == client_server::JOIN) {
        return msg.room() + " > Please vote on whether to allow " +
               msg.start_vote_message().nickname() +
               " to join the chatroom. If you would like to let this person in, "
               "respond YES. Otherwise, respond NO. If you do not answer in the "
               "designated format your vote will default to a no.";
    }
    return msg.room() + " > Please vote on whether to kick " +
           msg.start_vote_message().nickname() +
           " from the chatroom. If you would like to kick this person out, "
           "respond YES. Otherwise, respond NO. If you do not answer in the "
           "designated format your vote will default to a no.";
}

string ClientServerAPI::process_left_msg(client_server::Message &msg) {
  if(msg.for_current_user()) {
    room = "";
  }
  return msg.room() + " > " + msg.left_message().nickname() +
         " has left the room.";
}

string ClientServerAPI::process_vote_result_msg(client_server::Message &msg) {
  if (msg.for_current_user()) {
    bool in_room = (msg.vote_result_message().type() == client_server::JOIN) ==
      msg.vote_result_message().vote();
    room = (in_room) ? msg.room() : "";
  }
  string user = (msg.for_current_user())
                    ? "You have "
                    : msg.vote_result_message().nickname() + " has ";
  string type = ((msg.vote_result_message().type() == client_server::JOIN)
                    ? "been invited to join "
                    : "been kicked from ")
                    + msg.room() + ".";
  string result = (msg.vote_result_message().vote()) ? "" : "not ";
  return msg.room() + " > The verdict is in! " + user + result + type;
}

string ClientServerAPI::process_vote_msg(client_server::Message &msg) {
  return msg.room() + " > Your vote is in. Thanks!";
}

string ClientServerAPI::process_getting_kicked_msg(client_server::Message &msg) {
  if (msg.for_current_user() == 0) {
    return "";
  }
  return msg.room() + " > Your chatroom is voting on whether to kick you :(";
}

string ClientServerAPI::process_msg(client_server::Message &msg) {
    if (msg.has_text_message()) {
        return ClientServerAPI::process_text_msg(msg);
    } else if (msg.has_nickname_message()) {
        return ClientServerAPI::process_nickname_msg(msg);
    } else if (msg.has_left_message()) {
        return ClientServerAPI::process_left_msg(msg);
    } else if (msg.has_vote_result_message()) {
        return ClientServerAPI::process_vote_result_msg(msg);
    } else if (msg.has_vote_message()) {
        return ClientServerAPI::process_vote_msg(msg);
    } else if (msg.has_start_vote_message()) {
        return ClientServerAPI::process_getting_kicked_msg(msg);
    }
    return "ERROR: invalid message";
}
