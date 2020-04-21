#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "src/server_state.hpp"

VoteState::VoteState(int room, client_server::VoteType vote_type, const std::string & target_addr)
    : room{room}, vote_type{vote_type}, target_addr{target_addr}, votes_for{0}, votes_against{0} {}


std::optional<std::string> ServerState::updateNickname(const std::string & addr, const std::string & nickname) {
    if (user2nickname.find(addr) == user2nickname.end()) {
        return std::nullopt;
    }
    std::string temp = user2nickname.at(addr);
    user2nickname[addr] = nickname;
    return std::optional<std::string>{temp};
}

void ServerState::leaveRoom(const std::string & addr) {
    if (user2nickname.find(addr) == user2nickname.end()) {
        return;
    }
    int room = user2room.at(addr);
    room2users.at(room).erase(addr);
}

void ServerState::joinRoom(const std::string & addr, int room) {
    user2room[addr] = room;
    room2users.at(room).insert(addr);
}

int ServerState::updateRoomSize(int room, int change) {
    room2Size[room] += change;
    return room2Size[room];
}

std::string gen_uuid() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return boost::uuids::to_string(uuid);
}

std::string ServerState::startVote(int room, client_server::VoteType vote_type, const std::string & target_addr) {
    std::string vote_id = gen_uuid();
    votes.emplace(vote_id, VoteState(room, vote_type, target_addr));
    return vote_id;
}
