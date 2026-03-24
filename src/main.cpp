#include <iostream>
#include <string>
#include "stdio.h"
#include "host.h"
#include "client.h"

#define GAME_VERSION "0.1"

// Prototpyes
void host();
void client();


int main()
{
    // Here client or server will be determined
    std::cout << "---- Menu ----" << std::endl;
    do
    {
        std::string input;
        std::cout << "Host(h)\t\tJoin(j)\t\tQuit(q): ";
        std::cin >> input;
        if (input.compare("h") == 0)
            host();
        else if (input.compare("j") == 0)
            client();
        else if (input.compare("q") == 0)
            return 0;
        else
            continue;
        return 0;
    } while (1);
    return 0;
}

void host()
{
    // Host will be the only one to use game.h / game.cpp and all other files
    Host *host = new Host();
    delete host;
}
void client()
{
    // Will have access to ui, client classes
    Client client;
}

