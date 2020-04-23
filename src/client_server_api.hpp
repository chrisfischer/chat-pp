#ifndef CLIENT_SERVER_API_HPP_
#define CLIENT_SERVER_API_HPP_

#include <string>
#include <grpcpp/grpcpp.h>

#include "proto/client_server.grpc.pb.h"
#include "proto/client_server.pb.h"

using namespace std;

class ClientServerAPI {

private:
    string room;
    string nickname;
    bool send_message(client_server::Message &msg);

public:
    shared_ptr<client_server::ChatService::Stub> stub_;

    ClientServerAPI(std::shared_ptr<grpc::Channel> channel);
    
    shared_ptr<grpc::ClientReader<client_server::Message>> get_reader();
    bool send_text(string &text);
    bool change_nickname(string &new_nickname);
    bool leave_room();
    bool join_room(string &new_room, string &new_nickname);
    bool submit_vote(string &vote_id, bool vote);
    void update_room(string &new_room);
    string process_msg(client_server::Message &msg);
    
    string process_text_msg(client_server::Message &msg);
    string process_nickname_msg(client_server::Message &msg);
    string process_start_vote_msg(client_server::Message &msg);
    string process_left_msg(client_server::Message &msg);
    string process_vote_result_msg(client_server::Message &msg);
};


#endif /* CLIENT_SERVER_API_HPP_ */
