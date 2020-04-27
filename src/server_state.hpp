#ifndef SERVER_STATE_HPP_
#define SERVER_STATE_HPP_

#include <map>
#include <optional>
#include <set>
#include <string>

#include "proto/client_server.pb.h"

class VoteState {
   public:
    std::string room;
    client_server::VoteType vote_type;
    std::string target_nickname;
    unsigned int votes_for;
    unsigned int votes_against;
    std::set<std::string> voted_addrs;

    VoteState(const std::string &room, client_server::VoteType vote_type,
              const std::string &target_nickname);
};

class ServerState {
   private:
    std::unique_ptr<std::mutex> mutex;

    // Map from addr -> nickname, if an entry exists then the user is currently connected
    std::map<std::string, std::string> user2nickname;
    // Map from addr -> current room
    std::map<std::string, std::string> user2room;
    // Map from room number -> set of addrs connected to this server
    std::map<std::string, std::set<std::string>> room2users;
    // Map from room number -> size
    std::map<std::string, unsigned int> room2size;
    // Map from vote id -> state for vote
    std::map<std::string, VoteState> votes;

   public:
    ServerState();

    // Register a new user
    void register_user(const std::string &addr);
    // Remove all state associated with a user
    void remove_user(const std::string &addr);
    // Set nickname for a given user
    const std::string &set_nickname(const std::string &addr,
                                    const std::string &nickname);
    void leave_room(const std::string &addr);
    // Leave room if equals given room
    bool leave_room_if(const std::string &addr, const std::string &room);
    void join_room(const std::string &addr, const std::string &room);
    // Returns old size
    unsigned int set_room_size(const std::string &room, unsigned int new_size);
    unsigned int decr_room_size(const std::string &room);

    std::string start_vote(const std::string &room,
                           client_server::VoteType vote_type,
                           const std::string &target_addr);
    bool set_vote(const std::string &vote_id, bool vote_for, const std::string &addr);
    void remove_vote(const std::string &vote_id);

    unsigned int get_room_size(const std::string &room) const;

    std::optional<std::string> addr_for_nickname(const std::string &nickname) const;
    // Returns default nickname (port) if none found
    std::string nickname_for_addr(const std::string &addr) const;

    // Return current room for the given user
    std::optional<std::string> get_room(const std::string &addr) const;
    const std::set<std::string> &addrs_in_room(const std::string &room);

    // Whether this server contain state on the given vote_id
    bool has_vote(const std::string &vote_id) const;
    // Get information on the given vote id
    std::optional<VoteState> get_vote(const std::string &vote_id) const;
    // Whether this vote has enough votes to be complete given the room size
    std::optional<bool> is_vote_complete(const std::string &vote_id) const;

    friend std::ostream &operator<<(std::ostream &os, const ServerState &state);
};

#endif /* SERVER_STATE_HPP_ */