#ifndef SERVER_STATE_HPP_
#define SERVER_STATE_HPP_

#include <string>
#include <map>
#include <set>
#include <optional>

#include "proto/client_server.pb.h"

// TODO consistency issues with knowing how many people are in each room
// Turn into sync, rpc?

// TODO add syncronized map that overrides set with getting a lock for row

// TODO make it so we don't need to know how many people are in each room

class VoteState
{
public:
    int room;
    client_server::VoteType vote_type;
    std::string target_addr;
    int votes_for;
    int votes_against;
    VoteState(int room, client_server::VoteType vote_type, const std::string &target_addr);
};

class ServerState
{
private:
    // map from addr -> nickname
    std::map<std::string, std::string> user2nickname;
    // map from addr -> current room
    std::map<std::string, int> user2room;
    // map from room number -> set of addrs connected to this server
    std::map<int, std::set<std::string>> room2users;
    // map from room number -> size
    std::map<int, int> room2Size;
    // map from vote id -> state for vote
    std::map<std::string, VoteState> votes;

public:
    std::optional<std::string> update_nickname(const std::string &addr, const std::string &nickname);
    void leave_room(const std::string &addr);
    void join_room(const std::string &addr, int room);
    int update_room_size(int room, int change);
    std::string start_vote(int room, client_server::VoteType vote_type, const std::string &target_addr);
};

#endif /* SERVER_STATE_HPP_ */