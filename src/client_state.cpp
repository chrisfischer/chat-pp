#include "src/client_state.hpp"

bool ClientState::in_room() {
    return room.compare("");
}