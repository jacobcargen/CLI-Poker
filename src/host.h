

// host.h

#include <iostream>
#include <cstring>
#include <cstdint>
#include <map>
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

// Bitmask flags for player state (allow combinations if needed)
enum playerState
{
    INIT = 1, 
    MAIN_LOBBY = 2,
    GAME_LOBBY = 4,
    PLAYING = 8,
    READY = 16
};

struct Player {
    int socket;
    uint32_t state = playerState::INIT; // bitmask of `playerState` flags
    std::string name;
    bool hasMadeNickname;
    bool isPrompted = false;
    std::string promptMessage = "";
    std::string clientPromptResponse = "";
} typedef Player;

// Helper functions for manipulating/checking `Player::state` bitmask
inline bool hasState(const Player& p, playerState s) {
    return (p.state & static_cast<uint32_t>(s)) != 0;
}

inline void setState(Player& p, playerState s) {
    p.state |= static_cast<uint32_t>(s);
}

inline void clearState(Player& p, playerState s) {
    p.state &= ~static_cast<uint32_t>(s);
}

struct ServerConfig
{
    int port = PORT;
    int maxClients = MAX_CLIENTS;
    
    // Map config keys to their values
    std::map<std::string, int*> getConfigMap() {
        return {
            {"port", &port},
            {"maxClients", &maxClients}
        };
    }
};



class Host {

public:
    // Constructor
    Host();
    Host(char* configFile);
    // Public methods
    void start(ServerConfig * config);
    void sendMessageToClient(Player* client, ServerMessage messageType, const std::string& dataStr);
    void sendMessageToAll(ServerMessage messageType, const std::string& dataStr, std::vector<playerState> states);
    void promptComplete(Player * client);
    std::string getResponseFromClientPrompt(Player * client);
    void reprompt(Player * client);
    

    // Destructor
    ~Host();

private:
    void lobbyLoop();
};


#endif // HOST_H
