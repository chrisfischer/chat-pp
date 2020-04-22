#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cxxopts>

#include "proto/client_server.grpc.pb.h"

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

void startClientServer() {
    std::shared_ptr<ClientReader<Message>> stream(
        stub_->ReceiveMessages(&context, Empty));
    while (reader->Read(&message)) {
        
    }
    Status status = reader->Finish();
}


void programStart(ostream &os, istream &is)
{
    os << "Welcome to Ĉ++!" << endl;
    os << "In order to get started, please specify the IP address and port for the server you would like to connect to." << endl;
    os << "IP Address:" << endl;

    string serverIP;
    string port;

    is >> serverIP;
    os << "Port:" << endl;
    is >> port;
    
    // error message if port or ip address are incorrect
    // TODO: decide if we want to exit or use while loop
    
    // start up client
}





int main()
{
    programStart(cout, cin);
}
