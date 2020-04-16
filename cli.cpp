#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>

using namespace std;

void programStart(ostream& os, istream& is) {
    os << "Welcome to Ĉ++!" << endl;
    os << "In order to get started, please specify the IP address and port for the server you would like to connect to." << endl;
    os << "IP Address:" << endl;
    
    string serverIP;
    string port;
    
    is >> serverIP;
    os << "Port:" << endl;
    is >> port;
    
    // exit out with message if port or ip address are incorrect
}

int main()
{
    programStart(cout, cin);
}
