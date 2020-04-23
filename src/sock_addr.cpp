#include <regex>

#include "SockAddr.hpp";

using namespace std;

SockAddr::SockAddr(string s) : addr{s} {
    regex addr_regex = regex{"^[\\s\\S]+:\\d+$"};
    if (!regex_match(s, addr_regex)) {
        throw std::invalid_argument("Invalid address");
    }
};

bool SockAddr::operator<(const SockAddr &rhs) const { return addr < rhs.addr; }