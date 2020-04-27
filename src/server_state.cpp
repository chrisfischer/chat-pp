#include "server_state.hpp"

#include <iostream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

VoteState::VoteState(const std::string &room, client_server::VoteType vote_type,
                     const std::string &target_nickname)
    : room{room},
      vote_type{vote_type},
      target_nickname{target_nickname},
      votes_for{0},
      votes_against{0} {}

ServerState::ServerState() : mutex{std::make_unique<std::mutex>()} {}

void ServerState::register_user(const std::string &addr) {
    set_nickname(addr, addr.substr(addr.find_last_of(":") + 1, addr.size()));
}

const std::string &ServerState::set_nickname(const std::string &addr, 
                                             const std::string &nickname) {
    std::scoped_lock lock{*mutex};
    bool exists = user2nickname.find(addr) == user2nickname.end();
    std::string temp = user2nickname[addr];
    user2nickname[addr] = nickname;
    return exists ? temp : addr;
}

void ServerState::leave_room(const std::string &addr) {
    std::scoped_lock lock{*mutex};
    if (user2room.find(addr) == user2room.end()) {
        return;
    }
    std::string room = user2room.at(addr);
    user2room.erase(addr);
    room2users.at(room).erase(addr);
}

bool ServerState::leave_room_if(const std::string &addr, const std::string &room) {
    std::scoped_lock lock{*mutex};
    if (user2room.find(addr) == user2room.end()) {
        return false;
    }
    std::string curr_room = user2room.at(addr);
    if (room != curr_room) {
        return false;
    }
    user2room.erase(addr);
    room2users.at(room).erase(addr);
    return true;
}

void ServerState::join_room(const std::string &addr, const std::string &room) {
    std::scoped_lock lock{*mutex};
    user2room[addr] = room;
    room2users.at(room).insert(addr);
}

void ServerState::remove_user(const std::string &addr) {
    leave_room(addr);
    std::scoped_lock lock{*mutex};
    user2nickname.erase(addr);
}

unsigned int ServerState::set_room_size(const std::string &room, unsigned int new_size) {
    std::scoped_lock lock{*mutex};
    auto prev_size = room2size[room];
    room2size[room] = new_size;
    return prev_size;
}

unsigned int ServerState::decr_room_size(const std::string &room) {
    std::scoped_lock lock{*mutex};
    room2size[room] -= 1;
    return room2size[room];
}

std::string gen_uuid() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return boost::uuids::to_string(uuid);
}

std::string ServerState::start_vote(const std::string &room,
                                    client_server::VoteType vote_type,
                                    const std::string &target_addr) {
    std::scoped_lock lock{*mutex};
    std::string vote_id = gen_uuid();
    votes.emplace(vote_id, VoteState(room, vote_type, target_addr));
    return vote_id;
}

bool ServerState::set_vote(const std::string &vote_id, bool vote_for, const std::string &addr) {
    std::scoped_lock lock{*mutex};
    if (votes.find(vote_id) == votes.end()) {
        std::cerr << "Vote not found " << vote_id << std::endl;
        return false;
    }
    VoteState &vote_state{votes.at(vote_id)};
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

void ServerState::remove_vote(const std::string &vote_id) {
    std::scoped_lock lock{*mutex};
    votes.erase(vote_id);
}

// const functions

unsigned int ServerState::get_room_size(const std::string &room) const {
    std::scoped_lock lock{*mutex};
    if (room2size.find(room) == room2size.end()) {
        return 0;
    }
    return room2size.at(room);
}


std::optional<std::string> ServerState::addr_for_nickname(const std::string &nickname) const {
    std::scoped_lock lock{*mutex};
    for (auto elt : user2nickname) {
        if (elt.second == nickname) {
            return std::optional<std::string>{elt.first};
        }
    }
    return std::nullopt;
}

std::string ServerState::nickname_for_addr(const std::string &addr) const {
    std::scoped_lock lock{*mutex};
    if (user2nickname.find(addr) == user2nickname.end()) {
        return addr.substr(addr.find_last_of(":") + 1, addr.size());
    }
    return user2nickname.at(addr);
}

std::optional<std::string> ServerState::get_room(const std::string &addr) const {
    std::scoped_lock lock{*mutex};
    if (user2room.find(addr) == user2room.end()) {
        return std::nullopt;
    }
    return user2room.at(addr);
}

std::optional<std::string> ServerState::room_for_addr(const std::string &addr) const {
    std::scoped_lock lock{*mutex};
    if (user2room.find(addr) == user2room.end()) {
        return std::nullopt;
    }
    return std::optional<std::string>{user2room.at(addr)};
}

const std::set<std::string> &ServerState::addrs_in_room(const std::string &room) {
    std::scoped_lock lock{*mutex};
    return room2users[room];
}

bool ServerState::has_vote(const std::string &vote_id) const {
    std::scoped_lock lock{*mutex};
    return votes.find(vote_id) != votes.end();
}

std::optional<VoteState> ServerState::get_vote(const std::string &vote_id) const {
    if (!has_vote(vote_id)) {
        return std::nullopt;
    }
    std::scoped_lock lock{*mutex};
    return std::optional<VoteState>{votes.at(vote_id)};
}

std::optional<bool> ServerState::is_vote_complete(const std::string &vote_id) const {
    if (!has_vote(vote_id)) {
        return std::nullopt;
    }
    std::scoped_lock lock{*mutex};
    auto vote_state{votes.at(vote_id)};
    auto room_size{room2size.at(vote_state.room)};
    auto target_number_votes{(room_size + 1) / 2};
    // Take the ceiling
    if (vote_state.votes_for >= target_number_votes) {
        return true;
    } else if (vote_state.votes_against >= target_number_votes) {
        return false;
    } else {
        return std::nullopt;
    }
}

// TODO auto& by ref?
template<typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::map<K, V>& m) {
    for (auto &elem : m) {
        os << elem.first << " -> " << elem.second << std::endl;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const ServerState& state) {
    os << "**********\n";
    os << "Nicknames:\n";
    os << state.user2nickname;
    os << "Rooms:\n";
    os << state.user2room;
    os << "Rooms sizes:\n";
    os << state.room2size;
    os << "**********\n";
    return os;
}