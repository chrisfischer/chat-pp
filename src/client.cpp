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

void startClientServer() {
    ClientServerAPI csAPI {grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials())};

    client_server::Message msg;
    shared_ptr<grpc::ClientReader<client_server::Message>> reader {csAPI.get_reader()};
    while (reader->Read(&msg)) {
        
    }
    grpc::Status status = reader->Finish();
}


void programStart(ostream &os, istream &is)
{
    os << "Welcome to Ĉ++!" << endl;
    os << "In order to get started, please specify the IP address you would like to connect to." << endl;
    os << "IP Address:" << endl;

    string serverIP;
    is >> serverIP;
    
    // error message if ip address is incorrect
    // TODO: decide if we want to exit or use while loop
    
    // start up client
}

int main()
{
    programStart(cout, cin);
}
