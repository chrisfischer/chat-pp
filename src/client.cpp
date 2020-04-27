#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <string>
#include <string_view>
#include <bits/stdc++.h>

#include "proto/client_server.grpc.pb.h"
#include "src/client_server_api.hpp"

using namespace std;

void print_help_message()
{
  cout << "Ĉ++ has five special commands:" << endl;
  cout << "1. help : Open the help menu"
    << endl;
  cout << "2. enter <chatroom> : Request to join <chatroom>. Users of <chatroom> will "
    "vote on whether to let you in." << endl;
  cout << "3. leave : Leave your current chatroom." << endl;
  cout << "4. nickname <new_nickname> : Change your nickname." << endl;
  cout << "5. kick <nickname> : Start a vote to kick someone out of the chatroom." << endl;
}

string prompt_cli()
{
  cout << "Welcome to Ĉ++!" << endl << endl;
  print_help_message();
  cout << "Please specify the IP address you would "
    "like to connect to." << endl << "IP Address:" << endl;

  // localhost:50051
  string serverIP;
  getline(cin, serverIP);

  cout << "You're all set! You can enter a chatroom now by typing enter "
    "<chatroom>." << endl;
  return serverIP;
}

void listen_to_server(
    ClientServerAPI& csAPI,
    bool& vote_flag,
    string& vote_result,
    bool& verdict_pending) {

  client_server::Message msg;
  shared_ptr<grpc::ClientReaderWriter<client_server::Message,
    client_server::Message>> stream {csAPI.get_stream()};

  while (stream->Read(&msg)) {
    if (msg.has_vote_result_message()) {
      verdict_pending = 0;
    }
    if (msg.has_start_vote_message() && !msg.for_current_user()) {
      cout << csAPI.process_start_vote_msg(msg) << endl;
      vote_flag = 1;
      while (vote_flag) {};
      transform(vote_result.begin(), vote_result.end(), vote_result.begin(), ::tolower); 
      csAPI.submit_vote(msg.start_vote_message().vote_id(), vote_result == "y" || vote_result == "yes");
    } else if (!msg.has_vote_message() || msg.for_current_user()) {
      cout << csAPI.process_msg(msg) << endl;
    }
  }
  grpc::Status status = stream->Finish();
}

void parse_input(string &input, ClientServerAPI& csAPI, bool& verdict_pending) {
  if (verdict_pending) {
    cout << "Vote pending... please wait." << endl;
  } else if (!input.compare("help")) {
    print_help_message();
  } else if (input.rfind("enter", 0) == 0 && !csAPI.in_room()) {
    string room = input.erase(0, 6);
    csAPI.join_room(room);
    cout << "Users from " << room << " will now vote on whether to let you in "
      "the room." << endl;
    verdict_pending = 1;
  } else if (!csAPI.in_room()) {
    cout << "You are not currently in a chatroom. Type enter <chatroom> to "
      "enter a room." << endl;
  } else if (input.rfind("leave", 0) == 0) {
    csAPI.leave_room();
  } else if (input.rfind("nickname", 0) == 0) {
    csAPI.change_nickname(input.erase(0, 13));
  } else if (input.rfind("kick", 0) == 0) {
    csAPI.kick(input.erase(0, 5));
  }  else {
    csAPI.send_text(input);
  }
}

void listen_to_user(
    ClientServerAPI& csAPI,
    bool& vote_flag,
    string& vote_result,
    bool& vote_pending) {

  string user_input;
  while(getline(cin, user_input)) {
    if (vote_flag) {
      vote_result = user_input;
      vote_flag = 0;
    } else {
      parse_input(user_input, csAPI, vote_pending);
    }
  }
}

void run_client() {
  string addr {prompt_cli()};

  ClientServerAPI csAPI {
    grpc::CreateChannel(addr, grpc::InsecureChannelCredentials())};
  // localhost:8000

  bool vote_flag;
  string vote_result;
  bool vote_pending;
  thread serverThread {
    listen_to_server,
    ref(csAPI),
    ref(vote_flag),
    ref(vote_result),
    ref(vote_pending)
  };
  thread userThread {
    listen_to_user,
    ref(csAPI),
    ref(vote_flag),
    ref(vote_result),
    ref(vote_pending)
  };

  serverThread.join();
  userThread.join();
}

int main() { run_client(); }
