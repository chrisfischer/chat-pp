#ifndef CLIENT_STATE_HPP_
#define CLIENT_STATE_HPP_

#include <optional>
#include <string>

class ClientState {
   public:
    std::string room;
    std::string nickname;
    bool vote_flag;
    std::string vote_input;
    bool vote_pending;

    bool in_room();
};

#endif /* CLIENT_STATE_HPP_ */