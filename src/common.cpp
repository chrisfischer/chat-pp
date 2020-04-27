#include <iostream>
#include "common.hpp"

using namespace std;


/**
 * Logs the given string to stdout
 *
 * @param msg string to log
 */
void log(string msg) {
    if (IS_VERBOSE) {
        cout << "[" << SERVER_NUMBER << "] " << msg << endl;
    }
}

/**
 * Logs the given string to stderr
 *
 * @param msg string to log
 */
void log_err(string msg) {
    if (IS_VERBOSE) {
        cout << color::red << "[" << SERVER_NUMBER << "] " << msg << endl << color::def;
    }
}