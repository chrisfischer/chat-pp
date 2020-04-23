#ifndef SOCK_ADDR_HPP_
#define SOCK_ADDR_HPP_

#include <string>

class SockAddr {
   public:
    std::string addr;

    SockAddr(std::string s);
    bool operator<(const SockAddr &rhs) const;
};

#endif /* SOCK_ADDR_HPP_ */