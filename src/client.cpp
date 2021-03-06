#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
// #include <mutex>
#include <termios.h>
#include <unistd.h>

#include <cctype>
#include <string>
#include <string_view>

#include "proto/client_server.grpc.pb.h"
#include "src/chat_service_client.hpp"
#include "src/client_state.hpp"
#include "src/common.hpp"

using namespace std;

bool IS_VERBOSE = false;

void print_help_message() {
    cout << color::green;
    cout << "Ĉ++ has five special commands:" << endl;
    cout << "1. help : Open the help menu" << endl;
    cout << "2. enter <chatroom> : Request to join <chatroom>. Users of <chatroom> will "
            "vote on whether to let you in."
         << endl;
    cout << "3. leave : Leave your current chatroom." << endl;
    cout << "4. nickname <new_nickname> : Change your nickname." << endl;
    cout << "5. kick <nickname> : Start a vote to kick someone out of the chatroom." << endl;
    cout << color::def;
}

string prompt_cli() {
    cout << color::green;
    cout << "Welcome to Ĉ++!" << endl
         << endl;
    cout << color::def;
    print_help_message();
    cout << endl;

    cout << "Please specify the IP address you would like to connect to." << endl
         << "IP Address: ";

    string serverIP;
    getline(cin, serverIP);

    return serverIP;
}

void listen_to_server(ChatServiceClient& cs_api, shared_ptr<ClientState> state) {
    client_server::Message msg;
    auto stream{cs_api.get_stream()};

    while (stream->Read(&msg)) {
        if (msg.has_vote_result_message()) {
            state->vote_pending = false;
        }
        if (msg.has_start_vote_message() && !msg.for_current_user()) {
            // Vote regarding someone else
            cout << cs_api.process_start_vote_msg(msg) << endl;
            state->vote_flag = true;
            while (state->vote_flag) {
            };
            cs_api.submit_vote(msg.start_vote_message().vote_id(), state->vote_input == "y" || state->vote_input == "yes");
        } else if (!msg.has_vote_message() || msg.for_current_user()) {
            cs_api.process_msg(msg);
        }
    }
    grpc::Status status = stream->Finish();
}

void parse_input(string& input, ChatServiceClient& cs_api, shared_ptr<ClientState> state) {
    if (state->vote_pending) {
        cout << "Vote pending... please wait." << endl;
    } else if (!input.compare("help")) {
        print_help_message();
    } else if (input.rfind("enter ", 0) == 0 && !state->in_room()) {
        string room{input.erase(0, 6)};
        if (room.empty()) {
            cout << color::red << "Invalid room" << endl
                 << color::def;
            return;
        }
        cs_api.join_room(room);
        cout << color::blue << "Users from " << room << " will now vote..." << endl
             << color::def;
        state->vote_pending = true;
    } else if (input.rfind("nickname ", 0) == 0) {
        string nickname{input.erase(0, 9)};
        if (nickname.empty()) {
            cout << color::red << "Invalid nickname" << endl
                 << color::def;
            return;
        }
        cs_api.change_nickname(nickname);
    } else if (!state->in_room()) {
        cout << color::red << "You are not currently in a chatroom. Type enter <chatroom> to "
                              "enter a room."
             << endl
             << color::def;
    } else if (input.rfind("leave", 0) == 0) {
        cs_api.leave_room();
    } else if (input.rfind("kick ", 0) == 0) {
        cs_api.kick(input.erase(0, 5));
    } else {
        cs_api.send_text(input);
    }
}

void listen_to_user(ChatServiceClient& cs_api, shared_ptr<ClientState> state) {
    string user_input;
    while (getline(cin, user_input)) {
        if (state->vote_flag) {
            transform(user_input.begin(), user_input.end(), user_input.begin(), ::tolower);
            state->vote_input = user_input;
            state->vote_flag = false;
        } else {
            parse_input(user_input, cs_api, state);
        }
    }
}

void run_client() {
    string addr{prompt_cli()};

    auto state{make_shared<ClientState>()};

    ChatServiceClient cs_api{grpc::CreateChannel(addr, grpc::InsecureChannelCredentials()), state};

    cout << "You're all set! You can enter a chatroom now by typing enter <chatroom>." << endl;

    thread serverThread{listen_to_server, ref(cs_api), state};
    thread userThread{listen_to_user, ref(cs_api), state};

    serverThread.join();
    userThread.join();
}

int main() { run_client(); }
