#include <iostream>
#include <cstring>
#include "host.h"

#define SERVER_VERSION "0.1"

int main(int argc, char* argv[])
{
    std::cout << "---- CLI Poker Server v" << SERVER_VERSION << " ----" << std::endl;

    Host* server = nullptr;

    if (argc > 1) 
    {
        // Config file provided
        std::cout << "Loading config from: " << argv[1] << std::endl;
        server = new Host(argv[1]);
    } 
    else 
    {
        // Use default config
        std::cout << "Using default configuration" << std::endl;
        server = new Host();
    }

    // Cleanup
    delete server;

    return 0;
}
