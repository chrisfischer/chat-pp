#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <ostream>
#include <string>

extern bool IS_VERBOSE;
extern int SERVER_NUMBER;

namespace color {
enum Code {
    DEFAULT = 39,
    RED = 31,
    GREEN = 32,
    BLUE = 96,
};
class Modifier {
    Code code;

   public:
    Modifier(Code code) : code{code} {}
    friend std::ostream &operator<<(std::ostream &os, const Modifier &mod) {
        return os << "\033[" << mod.code << "m";
    }
};

const Modifier red = Modifier{RED};
const Modifier green = Modifier{GREEN};
const Modifier blue = Modifier{BLUE};
const Modifier def = Modifier{DEFAULT};
}  // namespace color

void log(std::string msg);
void log_err(std::string msg);

#endif /* COMMON_HPP_ */