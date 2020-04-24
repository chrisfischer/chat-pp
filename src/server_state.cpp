#include "server_state.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

VoteState::VoteState(const std::string &room, client_server::VoteType vote_type,
                     const std::string &target_addr)
    : room{room},
      vote_type{vote_type},
      target_addr{target_addr},
      votes_for{0},
      votes_against{0} {}

std::optional<std::string> ServerState::update_nickname(
    const std::string &addr, const std::string &nickname) {
    if (user2nickname.find(addr) == user2nickname.end()) {
        return std::nullopt;
    }
    std::string temp = user2nickname.at(addr);
    user2nickname[addr] = nickname;
    return std::optional<std::string>{temp};
}

void ServerState::leave_room(const std::string &addr) {
    if (user2nickname.find(addr) == user2nickname.end()) {
        return;
    }
    std::string room = user2room.at(addr);
    room2users.at(room).erase(addr);
}

void ServerState::join_room(const std::string &addr, const std::string &room) {
    user2room[addr] = room;
    room2users.at(room).insert(addr);
}

int ServerState::update_room_size(const std::string &room, int change) {
    room2Size[room] += change;
    return room2Size[room];
}

std::string gen_uuid() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return boost::uuids::to_string(uuid);
}

std::string ServerState::start_vote(const std::string &room,
                                    client_server::VoteType vote_type,
                                    const std::string &target_addr) {
    std::string vote_id = gen_uuid();
    votes.emplace(vote_id, VoteState(room, vote_type, target_addr));
    return vote_id;
}

bool ServerState::set_vote(const std::string &vote_id, bool vote_for, const std::string &addr) {
    if (votes.find(vote_id) == votes.end()) {
        std::cerr << "Vote not found " << vote_id << std::endl;
        return false;
    }
    auto vote_state {votes.at(vote_id)};
    if (vote_state.voted_addrs.find(addr) != vote_state.voted_addrs.end()) {
        std::cerr << "Addr already voted " << addr << std::endl;
        return false;
    }
    if (vote_for) {
        vote_state.votes_for += 1;
    } else {
        vote_state.votes_against += 1;
    }
    return true;
}

std::optional<const std::string> ServerState::addr_for_nickname(const std::string &nickname) {
    for (auto elt : user2nickname) {
        if (elt.second == nickname) {
            return std::optional<std::string> {elt.first};
        }
    }
    return std::nullopt;
}

std::optional<const std::string> ServerState::room_for_addr(const std::string &addr) {
    if (user2room.find(addr) == user2room.end()) {
        return std::nullopt;
    }
    return std::optional<std::string> {user2room.at(addr)};
}

const std::set<std::string> &ServerState::addrs_in_room(
    const std::string &room) {
    return room2users[room];
}

std::optional<const std::string> ServerState::target_addr_for_vote(const std::string &vote_id) {
    if (votes.find(vote_id) == votes.end()) {
        return std::nullopt;
    }
    return std::optional<std::string> {votes.at(vote_id).target_addr};
}