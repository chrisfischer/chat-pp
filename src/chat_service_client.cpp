#include "src/chat_service_client.hpp"

#include <grpcpp/grpcpp.h>

#include <string>

#include "proto/client_server.grpc.pb.h"
#include "proto/client_server.pb.h"
#include "src/client_state.hpp"
#include "src/common.hpp"

using namespace std;

ChatServiceClient::ChatServiceClient(std::shared_ptr<grpc::Channel> channel, std::shared_ptr<ClientState> state)
    : state{state}, stub_{client_server::ChatService::NewStub(channel)} {
    stream = stub_->ReceiveMessages(&context);
}

shared_ptr<grpc::ClientReaderWriter<client_server::Message,
                                    client_server::Message>>
ChatServiceClient::get_stream() {
    return stream;
}

void ChatServiceClient::send_message(client_server::Message &msg) {
    stream->Write(msg);
}

void ChatServiceClient::send_text(const string &text) {
    client_server::TextMessage *text_msg = new client_server::TextMessage{};
    text_msg->set_text(text);

    client_server::Message msg;
    msg.set_allocated_text_message(text_msg);
    ChatServiceClient::send_message(msg);
}

void ChatServiceClient::change_nickname(const string &new_nickname) {
    client_server::NicknameMessage *nn_msg = new client_server::NicknameMessage{};
    nn_msg->set_new_nickname(new_nickname);

    client_server::Message msg;
    msg.set_allocated_nickname_message(nn_msg);
    ChatServiceClient::send_message(msg);
}

void ChatServiceClient::leave_room() {
    client_server::LeftMessage *left_msg = new client_server::LeftMessage{};

    client_server::Message msg;
    msg.set_allocated_left_message(left_msg);
    ChatServiceClient::send_message(msg);
}

void ChatServiceClient::join_room(const string &new_room) {
    client_server::StartVoteMessage *sv_msg = new client_server::StartVoteMessage{};
    sv_msg->set_type(client_server::VoteType::JOIN);

    client_server::Message msg;
    msg.set_allocated_start_vote_message(sv_msg);
    msg.set_room(new_room);
    ChatServiceClient::send_message(msg);
}

void ChatServiceClient::kick(const string &nickname) {
    client_server::StartVoteMessage *sv_msg = new client_server::StartVoteMessage{};
    sv_msg->set_type(client_server::KICK);
    sv_msg->set_nickname(nickname);

    client_server::Message msg;
    msg.set_allocated_start_vote_message(sv_msg);
    ChatServiceClient::send_message(msg);
}

void ChatServiceClient::submit_vote(const string &vote_id, bool vote) {
    client_server::VoteMessage *vote_msg = new client_server::VoteMessage{};
    vote_msg->set_vote_id(vote_id);
    vote_msg->set_vote(vote);

    client_server::Message msg;
    msg.set_allocated_vote_message(vote_msg);
    ChatServiceClient::send_message(msg);
}

void ChatServiceClient::process_text_msg(client_server::Message &msg) {
    cout << color::blue << msg.room() + " (" + msg.text_message().nickname() + ") > " 
        << color::def << msg.text_message().text() << endl;
}

string ChatServiceClient::process_nickname_msg(client_server::Message &msg) {
    if (msg.for_current_user()) {
        state->nickname = msg.nickname_message().new_nickname();
    }
    return msg.nickname_message().old_nickname() + " changed their nickname to " +
           msg.nickname_message().new_nickname();
}

string ChatServiceClient::process_start_vote_msg(client_server::Message &msg) {
    if (msg.start_vote_message().type() == client_server::JOIN) {
        return "Allow " + msg.start_vote_message().nickname() + " to join? (y/n)";
    } else {
        return "Kick " + msg.start_vote_message().nickname() + " from the chatroom? (y/n)";
    }
}

string ChatServiceClient::process_left_msg(client_server::Message &msg) {
    if (msg.for_current_user()) {
        state->room = "";
    }
    return (msg.for_current_user() ? "You have " : msg.left_message().nickname() + " has ") +
           "left the room";
}

string ChatServiceClient::process_vote_result_msg(client_server::Message &msg) {
    if (msg.for_current_user()) {
        bool in_room{(msg.vote_result_message().type() == client_server::JOIN) ==
                     msg.vote_result_message().vote()};
        state->room = in_room ? msg.room() : "";
        state->nickname = in_room ? state->nickname : "";
    }
    string user{msg.for_current_user()
                    ? "You have "
                    : msg.vote_result_message().nickname() + " has "};
    string type{(msg.vote_result_message().type() == client_server::JOIN
                    ? "been added to "
                    : "been kicked from ") +
                  msg.room()};
    string result{msg.vote_result_message().vote() ? "" : "not "};
    return "The verdict is in! " + user + result + type;
}

string ChatServiceClient::process_vote_msg(client_server::Message &msg) {
    string vote{msg.vote_message().vote() ? "yes" : "no"};
    return "You voted (" + vote + ")";
}

string ChatServiceClient::process_getting_kicked_msg(client_server::Message &msg) {
    return "Your chatroom is voting on whether to kick you :(";
}

void ChatServiceClient::process_msg(client_server::Message &msg) {
    if (msg.has_text_message()) {
        ChatServiceClient::process_text_msg(msg);
        return;
    }
    string to_show;
    if (msg.has_nickname_message()) {
        to_show = ChatServiceClient::process_nickname_msg(msg);
    } else if (msg.has_left_message()) {
        to_show = ChatServiceClient::process_left_msg(msg);
    } else if (msg.has_vote_result_message()) {
        to_show = ChatServiceClient::process_vote_result_msg(msg);
    } else if (msg.has_vote_message()) {
        to_show = ChatServiceClient::process_vote_msg(msg);
    } else if (msg.has_start_vote_message()) {
        to_show = ChatServiceClient::process_getting_kicked_msg(msg);
    } else {
        cout << color::red << "ERROR: invalid message" << color::def << endl;
    }
    cout << color::blue << msg.room() + " > " << color::def << to_show << endl;
}
