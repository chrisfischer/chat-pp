#ifndef SOCK_ADDR_HH_
#define SOCK_ADDR_HH_

#include <string>

class SockAddr {
public:
  std::string addr;

  SockAddr(std::string s);
  bool operator<(const SockAddr &rhs) const;
};

#endif /* SOCK_ADDR_HH_ */