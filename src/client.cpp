#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <string>
#include <string_view>

#include "proto/client_server.grpc.pb.h"
#include "src/client_server_api.hpp"

using namespace std;

mutex m;

void print_help_message()
{
  cout << "Ĉ++ has five special commands:" << endl;
  cout << "1. help : Type this command at any time to see this help menu again."
    << endl;
  cout << "2. enter <chatroom> : This command is available when you are not a "
    "member of a chatroom. Type this command and the users of <chatroom> will "
    "vote on whether to let you in. If they vote yes, you will be entered into "
    "<chatroom>." << endl;
  cout << "3. leave : This command is available when you are a member of a "
    "chatroom. Type this command to leave the chatroom." << endl;
  cout << "4. set nickname <new_nickname> : This command is available when you "
    "are inside a chatroom and is used to change your nickname inside that "
    "chatroom." << endl;
  cout << "5. kick <nickname> : This command is available when you are inside "
    "a chatroom and is used to kick someone out of the chatroom. If no one "
    "exists with the nickname <nickname> nothing will happen." << endl << endl;
}

string prompt_cli()
{
  cout << "Welcome to Ĉ++!" << endl << endl;
  print_help_message();
  cout << "In order to get started, please specify the IP address you would "
    "like to connect to." << endl << "IP Address:" << endl;

  // localhost:50051
  string serverIP;
  getline(cin, serverIP);

  cout << "You're all set! You can enter a chatroom now by typing enter "
    "<chatroom>." << endl;
  return serverIP;
}

void listen_to_server(ClientServerAPI& csAPI, bool& vote_flag, string& vote_result) {
  client_server::Message msg;
  shared_ptr<grpc::ClientReaderWriter<client_server::Message,
    client_server::Message>> stream {csAPI.get_stream()};

  while (stream->Read(&msg)) {
    cout << csAPI.process_msg(msg) << endl;
    if (msg.has_start_vote_message()) {
      cout << csAPI.process_start_vote_msg(msg) << endl;
      vote_result = 1;
      cout << vote_result << "vote_result" << endl;
      csAPI.submit_vote(msg.start_vote_message().vote_id(), vote_result == "YES");
    } else {
      cout << csAPI.process_msg(msg) << endl;
    }
  }
  grpc::Status status = stream->Finish();
}

void parse_input(string &input, ClientServerAPI& csAPI) {
  if (!input.compare("help")) {
    print_help_message();
  } else if (input.rfind("enter", 0) == 0 && !csAPI.in_room()) {
    string room = input.erase(0, 6);
    csAPI.join_room(room);
    cout << "Users from " << room << " will now vote on whether to let you in "
      "the room." << endl;
  } else if (!csAPI.in_room()) {
    cout << "You are not currently in a chatroom. Type enter <chatroom> to "
      "enter a room." << endl;
  } else if (input.rfind("leave", 0) == 0) {
    cout << "leaving." << endl;
    csAPI.leave_room();
  } else if (input.rfind("set nickname", 0) == 0) {
    csAPI.change_nickname(input.erase(0, 13));
  } else if (input.rfind("kick", 0) == 0) {
    csAPI.kick(input.erase(0, 5));
  }  else {
    csAPI.send_text(input);
  }
}

void listen_to_user(ClientServerAPI& csAPI, bool& vote_flag, string& vote_result) {
  string user_input;
  while(getline(cin, user_input)) {
    if (vote_flag) {
      parse_input(vote_result, csAPI);
      vote_flag = 0;
    }
    parse_input(user_input, csAPI);
  }
}

void run_client() {
  string addr {prompt_cli()};

  ClientServerAPI csAPI {
    grpc::CreateChannel("localhost:8000", grpc::InsecureChannelCredentials())};

  bool vote_flag;
  string vote_result;
  thread serverThread {listen_to_server, ref(csAPI), ref(vote_flag), ref(vote_result)};
  thread userThread {listen_to_user, ref(csAPI), ref(vote_flag), ref(vote_result)};

  serverThread.join();
  userThread.join();
}

int main() { run_client(); }
