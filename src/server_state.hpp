#ifndef SERVER_STATE_HPP_
#define SERVER_STATE_HPP_

#include <map>
#include <optional>
#include <set>
#include <string>

#include "proto/client_server.pb.h"

// TODO consistency issues with knowing how many people are in each room
// Turn into sync, rpc?

// TODO add syncronized map that overrides set with getting a lock for row

// TODO make it so we don't need to know how many people are in each room

class VoteState {
   public:
    std::string room;
    client_server::VoteType vote_type;
    std::string target_addr;
    unsigned int votes_for;
    unsigned int votes_against;
    std::set<std::string> voted_addrs;

    VoteState(const std::string &room, client_server::VoteType vote_type,
              const std::string &target_addr);
};

class ServerState {
   private:
    std::unique_ptr<std::mutex> mutex;

    // map from addr -> nickname
    std::map<std::string, std::string> user2nickname;
    // map from addr -> current room
    std::map<std::string, std::string> user2room;
    // map from room number -> set of addrs connected to this server
    std::map<std::string, std::set<std::string>> room2users;
    // map from room number -> size
    std::map<std::string, unsigned int> room2size;
    // map from vote id -> state for vote
    std::map<std::string, VoteState> votes;

   public:
    ServerState();

    std::optional<std::string> update_nickname(const std::string &addr,
                                               const std::string &nickname);
    void leave_room(const std::string &addr);
    void join_room(const std::string &addr, const std::string &room);
    // Returns old size
    unsigned int set_room_size(const std::string &room, unsigned int new_size);
    // // Returns new size
    // unsigned int update_room_size(const std::string &room, int change);

    std::string start_vote(const std::string &room,
                           client_server::VoteType vote_type,
                           const std::string &target_addr);
    bool set_vote(const std::string &vote_id, bool vote_for, const std::string &addr);
    void remove_vote(const std::string &vote_id);

    std::optional<unsigned int> get_room_size(const std::string &room) const;
    // TODO make more efficient
    std::optional<std::string> addr_for_nickname(const std::string &nickname) const;
    std::optional<std::string> nickname_for_addr(const std::string &addr) const;

    std::optional<std::string> room_for_addr(const std::string &addr) const;
    const std::set<std::string> &addrs_in_room(const std::string &room); // TODO should this be const?

    // Does this sever contain state on the given vote_id
    bool has_vote(const std::string &vote_id) const;
    std::optional<VoteState> get_vote(const std::string &vote_id) const;
    std::optional<bool> is_vote_complete(const std::string &vote_id) const;

    friend std::ostream& operator<<(std::ostream& os, const ServerState& state);
};

#endif /* SERVER_STATE_HPP_ */