#ifndef CLIENT_SERVER_API_HPP_
#define CLIENT_SERVER_API_HPP_

#include <string>
#include <grpcpp/grpcpp.h>

#include "proto/client_server.grpc.pb.h"

class ClientServerAPI {

private:
    std::shared_ptr<grpc::Channel> channel;
    string room;
    string nickname;
    MessageResult send_message(Message &msg);

public:
    ClientServerAPI(std::shared_ptr<grpc::Channel> channel);
    
    bool send_text(string &text);
    bool change_nickname(string &new_nickname);
    bool leave_room();
    bool join_room(string &room);
    bool submit_vote(string &room, string &vote_id, bool vote);
};


#endif /* CLIENT_SERVER_API_HPP_ */
