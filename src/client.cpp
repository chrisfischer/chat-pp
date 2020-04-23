#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>

#include "proto/client_server.grpc.pb.h"
#include "src/client_server_api.hpp"

using namespace std;

/*
 1. user starts up chat app
 2. user enters IP and port
 3. user is given list of commands:
    OUTSIDE OF CHATROOM
    1. enter <chatroom>
 
    INSIDE CHATROOM
    1. leave <chatroom>
    2. set nickname <new_nickname>
    3. kick out <username>
    4. <message>
    5. submit vote <yes/no> <username>
    6.
 
    client needs to listen for incoming messages from the server and respond accordingly
    should have separate messages for
 */

string prompt_cli(ostream &os, istream &is)
{
    os << "Welcome to Ĉ++!" << endl;
    os << "In order to get started, please specify the IP address you would like to connect to." << endl << "IP Address:" << endl;

    string serverIP;
    is >> serverIP;
    return serverIP;
}

void run_client(ostream &os, istream &is) {
    string addr {prompt_cli(os, is)};
    
    ClientServerAPI csAPI {grpc::CreateChannel(addr, grpc::InsecureChannelCredentials())};

    client_server::Message msg;
    shared_ptr<grpc::ClientReader<client_server::Message>> reader {csAPI.get_reader()};
    while (reader->Read(&msg)) {
        if (msg.has_start_vote_message()) {
            csAPI.process_start_vote_msg(msg);
            string vote;
            is >> vote;
            csAPI.submit_vote(msg.start_vote_message().vote_id(), vote == "YES");
        } else {
            csAPI.process_msg(msg);
        }
    }
    grpc::Status status = reader->Finish();
}



int main()
{
    run_client(cout, cin);
}
