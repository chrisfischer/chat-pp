#ifndef CHAT_SERVICE_CLIENT_HPP_
#define CHAT_SERVICE_CLIENT_HPP_

#include <grpcpp/grpcpp.h>

#include <string>

#include "proto/client_server.grpc.pb.h"
#include "proto/client_server.pb.h"
#include "src/client_state.hpp"

class ChatServiceClient {
   private:
    void send_message(client_server::Message &msg);

    std::shared_ptr<grpc::ClientReaderWriter<client_server::Message,
                                             client_server::Message>>
        stream;
    std::shared_ptr<ClientState> state;

   public:
    std::shared_ptr<client_server::ChatService::Stub> stub_;
    grpc::ClientContext context;

    ChatServiceClient(std::shared_ptr<grpc::Channel> channel, std::shared_ptr<ClientState> state);

    std::shared_ptr<grpc::ClientReaderWriter<client_server::Message,
                                             client_server::Message>>
    get_stream();

    void send_text(const std::string &text);
    void change_nickname(const std::string &new_nickname);
    void leave_room();
    void join_room(const std::string &new_room);
    void kick(const std::string &nickname);
    void submit_vote(const std::string &vote_id, bool vote);
    void update_room(const std::string &new_room);

    void process_msg(client_server::Message &msg);
    void process_text_msg(client_server::Message &msg);

    std::string process_nickname_msg(client_server::Message &msg);
    std::string process_start_vote_msg(client_server::Message &msg);
    std::string process_left_msg(client_server::Message &msg);
    std::string process_vote_result_msg(client_server::Message &msg);
    std::string process_vote_msg(client_server::Message &msg);
    std::string process_getting_kicked_msg(client_server::Message &msg);
};

#endif /* CHAT_SERVICE_CLIENT_HPP_ */
