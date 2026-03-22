

// host.h

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "game.h"
#include "message_helper.h"

#ifndef HOST_H
#define HOST_H


constexpr int PORT = 6769;
constexpr int MAX_CLIENTS = 8;
const std::string WELCOME_MSG = "Welcome!\n";


struct Player {
    int socket;
    bool isReady;
    std::string name;
    bool hasMadeNickname;
    bool isPrompted = false;
    std::string promptMessage = "";
    std::string clientPromptResponse = "";
} typedef Player;


class Host {

public:
    // Constructor
    Host();
    // Public methods
    void start();
    void sendMessageToClient(Player* client, ServerMessage messageType, const std::string& dataStr);
    void promptComplete(Player * client);
    std::string getResponseFromClientPrompt(Player * client);
    void reprompt(Player * client);
    void enableOneTimeOverride();

};

#endif // HOST_H
