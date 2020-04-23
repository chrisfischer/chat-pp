#ifndef CLIENT_SERVER_API_HPP_
#define CLIENT_SERVER_API_HPP_

#include <grpcpp/grpcpp.h>

#include <string>

#include "proto/client_server.grpc.pb.h"
#include "proto/client_server.pb.h"

class ClientServerAPI {

private:
    std::string room;
    std::string nickname;
    bool send_message(client_server::Message &msg);

   public:
    std::shared_ptr<client_server::ChatService::Stub> stub_;

    ClientServerAPI(std::shared_ptr<grpc::Channel> channel);

    std::shared_ptr<grpc::ClientReader<client_server::Message>> get_reader();
    bool in_room();
    bool send_text(const std::string &text);
    bool change_nickname(const std::string &new_nickname);
    bool leave_room();
    bool join_room(const std::string &new_room);
    bool kick(const std::string &nickname);
    bool submit_vote(const std::string &vote_id, bool vote);
    void update_room(const std::string &new_room);
    std::string process_msg(client_server::Message &msg);

    std::string process_text_msg(client_server::Message &msg);
    std::string process_nickname_msg(client_server::Message &msg);
    std::string process_start_vote_msg(client_server::Message &msg);
    std::string process_left_msg(client_server::Message &msg);
    std::string process_vote_result_msg(client_server::Message &msg);
};

#endif /* CLIENT_SERVER_API_HPP_ */
