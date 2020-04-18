#include <unistd.h>

#include <iostream>
#include <regex>
#include <string>
#include <map>
#include <set>
#include <fstream>

using namespace std;

bool IS_VERBOSE;
int SERVER_NUMBER;
int NUMBER_OF_SERVERS;

/**
 * Parses the config file into relevant addresses
 *
 * @param fileName name of file to parse
 * @param serverNumber this server's index number (1 indexed)
 * @param fwdAddrs empty set of forwarding addresses to be populated
 * @param bindAddr empty string to be populated with ip and port to bind to
 * @return -1 on error, 0 otherwise
 */
int parseConfigFile(const string& fileName, int serverNumber,
        set<string>& fwdAddrs, string& bindAddr) {

    ifstream infile;
    infile.open(fileName);

    if (infile.fail()) {
        cerr << "Error: opening " << fileName << endl;
        return -1;
    }

    int count = 1;
    bool found = false;
    string line;
    while (infile >> line) {
        string ipport = line;

        string ip = ipport.substr(0, ipport.find(":"));
        int port = atoi(ipport.substr(ipport.find(":") + 1, ipport.size()).c_str());

        if (serverNumber == count) {
            bindAddr = line.substr(0, line.size());
            found = true;
        } else {
            fwdAddrs.insert(ipport); // collection of addresses that we can sendto
        }

        count++;
    }
    infile.close();

    if (!found) {
        cerr << "Error: Server number not found" << endl;
        return -1;
    }

    NUMBER_OF_SERVERS = fwdAddrs.size();

    return 0;
}

/**
 * Parses command line arguments
 *
 * @param v_ptr pointer to be populated if -v option is being used
 * @param fileName empty string to be populated with name of configuration file
 * @param serverNumber int ref to be populated with server number
 * @param argc total number of arguments
 * @param argv array of argument strings
 * @return int -1 on error, 0 otherwise
 */
int parseArgs(bool* v_ptr, int *fileName, int *serverNumber,
        int argc, char* argv[]) {

    char c;

    while ((c = getopt(argc, argv, "vo:")) != -1) {
        switch (c) {
        case 'v':
            *v_ptr = true;
            break;
        default:
            //cerr << "Invalid flag given" << endl;
            return -1;
        }
    }

    // is the filename present
    if (argc - optind == 0) {
        cerr << "Error: Missing configuration file" << endl;
        return -1;
    }
    // is the server number present
    if (argc - optind <= 1) {
        cerr << "Error: Missing server number" << endl;
        return -1;
    }
    *fileName = optind;
    *serverNumber = optind + 1;

    return 0;
}

int main(int argc, char *argv[]) {

    // PARSING ARGUMENTS
    IS_VERBOSE = false;
    int fileNameIndex;
    int serverNumberIndex;
    if (parseArgs(&IS_VERBOSE, &fileNameIndex, &serverNumberIndex, argc, argv) < 0) {
        return 1;
    }

    // HANDLING INVALID SERVER NUMBER
    char *endptr;
    SERVER_NUMBER = strtol(argv[serverNumberIndex], &endptr, 10);
    if (endptr == argv[serverNumberIndex] || *endptr != '\0'
            || SERVER_NUMBER == 0) {
        cerr << "Error: Server number not valid" << endl;
        return 1;
    }

    // PARSING CONFIG FILE
    string bindAddr;
    set<string> serverFwdAddrs;
    if (parseConfigFile(argv[fileNameIndex], SERVER_NUMBER, serverFwdAddrs, bindAddr) < 0) {
        return 1;
    }

}