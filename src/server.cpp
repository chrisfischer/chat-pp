#include <unistd.h>

#include <csignal>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <thread>

#include "forwarding_service_client.hpp"
#include "forwarding_service_impl.hpp"
#include "server_state.hpp"

// TODO remove
#include "client_server_api.hpp"

using namespace std;

bool IS_VERBOSE;
int SERVER_NUMBER;
int NUMBER_OF_SERVERS;

unique_ptr<grpc::Server> server;

/**
 * Parses the config file into relevant addresses
 *
 * @param file_name name of file to parse
 * @param server_number this server's index number (1 indexed)
 * @param fwd_addrs empty set of forwarding addresses to be populated
 * @param bind_addr empty string to be populated with ip and port to bind to
 * @return -1 on error, 0 otherwise
 */
int parse_config_file(const string &file_name, int server_number,
                      set<string> &fwd_addrs, string &bind_addr) {
    ifstream infile;
    infile.open(file_name);

    if (infile.fail()) {
        cerr << "Error: opening " << file_name << endl;
        return -1;
    }

    int count = 1;
    bool found = false;
    string line;
    while (infile >> line) {
        string ipport = line;

        string ip = ipport.substr(0, ipport.find(":"));
        int port = atoi(ipport.substr(ipport.find(":") + 1, ipport.size()).c_str());

        if (server_number == count) {
            bind_addr = line.substr(0, line.size());
            found = true;
        } else {
            fwd_addrs.insert(ipport);  // collection of addresses that we can sendto
        }

        count++;
    }
    infile.close();

    if (!found) {
        cerr << "Error: Server number not found" << endl;
        return -1;
    }

    NUMBER_OF_SERVERS = fwd_addrs.size();

    return 0;
}

/**
 * Parses command line arguments
 *
 * @param v_ptr pointer to be populated if -v option is being used
 * @param file_name empty string to be populated with name of configuration file
 * @param server_number int ref to be populated with server number
 * @param argc total number of arguments
 * @param argv array of argument strings
 * @return int -1 on error, 0 otherwise
 */
int parse_args(bool *v_ptr, int *file_name, int *server_number, int argc,
               char *argv[]) {
    char c;

    while ((c = getopt(argc, argv, "vo:")) != -1) {
        switch (c) {
            case 'v':
                *v_ptr = true;
                break;
            default:
                // cerr << "Invalid flag given" << endl;
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
    *file_name = optind;
    *server_number = optind + 1;

    return 0;
}

void sig_handler(int s) {
    if (s == SIGINT) {
        server->Shutdown();
    }
}

void run_server(const string &bind_addr, const set<string> &server_fwd_addrs, shared_ptr<ServerState>server_state) {
    auto chat_service{make_shared<ChatServiceImpl>(server_state, server_fwd_addrs)};
    ForwardingServiceImpl fowarding_service{server_state, chat_service};
    grpc::ServerBuilder builder;
    cout << "run_server " << bind_addr << endl;
    builder.AddListeningPort(bind_addr, grpc::InsecureServerCredentials());
    builder.RegisterService(chat_service.get());
    builder.RegisterService(&fowarding_service);
    server = unique_ptr<grpc::Server>(builder.BuildAndStart());
    server->Wait();
}

int main(int argc, char *argv[]) {
    // PARSING ARGUMENTS
    IS_VERBOSE = false;
    int file_name_index;
    int server_number_index;
    if (parse_args(&IS_VERBOSE, &file_name_index, &server_number_index, argc,
                   argv) < 0) {
        return 1;
    }

    // HANDLING INVALID SERVER NUMBER
    char *endptr;
    SERVER_NUMBER = strtol(argv[server_number_index], &endptr, 10);
    if (endptr == argv[server_number_index] || *endptr != '\0' ||
        SERVER_NUMBER == 0) {
        cerr << "Error: Server number not valid" << endl;
        return 1;
    }

    // PARSING CONFIG FILE
    string bind_addr;
    set<string> server_fwd_addrs;
    if (parse_config_file(argv[file_name_index], SERVER_NUMBER, server_fwd_addrs,
                          bind_addr) < 0) {
        return 1;
    }

    // set up sigint handler
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = sig_handler;
    sigaction(SIGINT, &sa, NULL);

    auto server_state = make_shared<ServerState>();
    // {make_shared<ServerState>()};
    thread server_thread{run_server, bind_addr, server_fwd_addrs, server_state};

    if (SERVER_NUMBER == 1) {
        ClientServerAPI chat_service{
        grpc::CreateChannel(*server_fwd_addrs.begin(), grpc::InsecureChannelCredentials())};
        // chat_service.start_stream();
        for (int i = 0; i < 10; i++) {
            chat_service.change_nickname("chris" + to_string(i));
            cout << *server_state;
        }
    }

    server_thread.join();
}