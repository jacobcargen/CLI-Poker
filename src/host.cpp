#include <iostream>
#include "host.h"
#include "message_helper.h"
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "game.h"
#include <vector>
#include <cstdint>
#include <fstream>

std::vector<Player> clients = {};
Game* gameInstance = nullptr;

void parseConfig(ServerConfig * config, char* configFile);


// Constructor: Starts the host server when an object of Host is created
Host::Host() 
{
    // default config
    ServerConfig config;
    start(&config);
}
Host::Host(char* configFile)
{
    // Parse config
    ServerConfig config;

    parseConfig(&config, configFile);
    
    start(&config);
}

void parseConfig(ServerConfig * config, char* configFile)
{
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Could not open config file: " << configFile << "\n";
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        /* EXAMPLE CONFIG
# Server Configuration
port=6769
maxClients=8 
         
        */
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        
        size_t delimPos = line.find('=');
        if (delimPos == std::string::npos) continue;
        
        std::string key = line.substr(0, delimPos);
        std::string value = line.substr(delimPos + 1);
        
        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        // Use config mapping to set values
        auto configMap = config->getConfigMap();
        if (configMap.find(key) != configMap.end()) {
            *configMap[key] = std::stoi(value);
        }
    }
    file.close();
}

// Main server function
void Host::start(ServerConfig * config) 
{
    gameInstance = new Game();
    gameInstance->init(this);

    int server_fd;                     // File descriptor for the server socket
    sockaddr_in address{};             // Struct for storing address info
    int addrlen = sizeof(address);     // Length of the address struct

    // Socket setup
    server_fd = socket(AF_INET, SOCK_STREAM, 0); // Create a TCP socket
    if (server_fd < 0) 
    {
        perror("Socket creation failed");
        return;
    }

    // Configure the address for the socket
    address.sin_family = AF_INET;         // Use IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP
    address.sin_port = htons(config->port);      // Set the port to listen on
    int opt = 1;
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the configured address and port, and start listening
    if (bind(server_fd, (sockaddr*)&address, addrlen) < 0 || listen(server_fd, config->maxClients) < 0) 
    {
        perror("Bind/Listen failed");
        close(server_fd);
        return;
    }
    std::cout << "Listening on ";
    std::cout << std::flush;
    if (system("hostname -I") != 0) {
        std::cerr << "Failed to get IP address\n";
    }
    std::cout << "Port: " << config->port << std::endl;
    std::cout << "Max Clients: " << config->maxClients << std::endl;


    // Main loop to handle incoming connections and messages
    fd_set readfds;
    while (true) 
    {
        std::cout << "--Main loop--\n";

        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;
        for (const Player& client : clients) 
        {
            if (client.socket > 0) 
                FD_SET(client.socket, &readfds);
            max_sd = std::max(max_sd, client.socket);
        }

        std::cout << "Awaiting activity\n";
        int activity = select(max_sd + 1, &readfds, nullptr, nullptr, nullptr);
        if (activity <= 0)
        {
            std::cout << "returning, no activity\n";
            return;
        }

            // Handle new connections
            if (FD_ISSET(server_fd, &readfds)) 
            {
                int new_socket = accept(server_fd, (sockaddr*)&address, (socklen_t*)&addrlen);
                if (new_socket < 0) 
                {
                    perror("Accept failed");
                    continue;
                }

                if (clients.size() < config->maxClients)
                {
                    Player newClient = {};
                    newClient.socket = new_socket;
                    clients.push_back(newClient);
                    std::cout << "Client[" << new_socket << "] has joined the server.\n";
                    sendMessageToClient(&clients.back(), ServerMessage::NORMAL, WELCOME_MSG);
                    sendMessageToClient(&clients.back(), ServerMessage::PROMPT, "Enter your nickname: ");
                }
                else
                {
                    std::cerr << "Server is full. Rejecting connection.\n";
                    close(new_socket);
                }
            }

            

            // Handle msgs
            for (Player& client : clients) 
            {
                if (client.socket <= 0) { continue; }
                if (!FD_ISSET(client.socket, &readfds)) { continue; }
                
                char buffer[MESSAGE_SIZE] = {0};
                int valread = read(client.socket, buffer, MESSAGE_SIZE);

                if (valread == 0) // Client disconnected
                {
                    std::cout << "Client[" << client.socket << "] disconnected.\n";
                    close(client.socket);
                    client = {}; // Reset the client object
                    continue;
                }

                buffer[valread] = '\0'; // Null termd data
                std::string clientMsg(buffer, valread);
                
                if (clientMsg.empty()) continue;

                // Parse data
                ClientMessage msgType = static_cast<ClientMessage>(clientMsg[0]);
                std::string parsedMsg = clientMsg.size() > 1 ? clientMsg.substr(1) : "";

                // Nickname logic   
                if (!client.hasMadeNickname)
                {
                    client.hasMadeNickname = true;
                    for (auto cl : clients)
                    {
                        if (parsedMsg == client.name)
                        {
                            client.hasMadeNickname = false;
                            sendMessageToClient(&client, ServerMessage::NORMAL, "Nickname already taken. " + parsedMsg);
                        }
                    }
                    if (parsedMsg.empty() || parsedMsg.size() > 16)
                    {
                        client.hasMadeNickname = false;
                        sendMessageToClient(&client, ServerMessage::NORMAL, "Invalid nickname. Must be 1-16 characters.");
                    }

                    if (!client.hasMadeNickname)
                    {
                        reprompt(&client);
                    }
                    else
                    {
                        client.name = parsedMsg;
                        client.hasMadeNickname = true;
                        setState(client, playerState::MAIN_LOBBY);
                        promptComplete(&client);

                        // Updatelobby
                        sendMessageToAll(ServerMessage::CANCEL_PROMPT, "", {playerState::MAIN_LOBBY, playerState::GAME_LOBBY}); // cancel all prompts first to prevent dupes
                        sendMessageToAll(ServerMessage::CLEAR, "", {playerState::MAIN_LOBBY, playerState::GAME_LOBBY}); // clear all clients in lobby states
                        std::string lobbyList = "Current Lobby:\n";
                        for (auto cl : clients)
                        {
                            if (cl.hasMadeNickname)
                            {
                                std::string status = hasState(cl, playerState::GAME_LOBBY) ? " (Ready)" : "(Not Ready)";
                                lobbyList += "- " + cl.name + status + "\n";
                            }
                        }
                        sendMessageToAll(ServerMessage::NORMAL, lobbyList, {playerState::MAIN_LOBBY, playerState::GAME_LOBBY}); // refresh others  
                        sendMessageToAll(ServerMessage::PROMPT, "Type 'x' when ready to start the game.", {playerState::MAIN_LOBBY}); // prompt all
                    }
                    continue;
                }

                // Parse message type
                switch (msgType)
                {
                case ClientMessage::QUIT:
                {
                    // Disconnect Client
                    std::cout << "Client[" << client.socket << "] requested to quit.\n";
                    int disconnectSocket = client.socket;
                    close(client.socket);
                    clients.erase(std::remove_if(clients.begin(), clients.end(), 
                        [disconnectSocket](const Player& p) { return p.socket == disconnectSocket; }), 
                        clients.end());
                    break;
                }
                case ClientMessage::RESPONSE:
                    if (!client.isPrompted) break;
                    client.clientPromptResponse = parsedMsg;
                    break;
                default:
                    std::cout << "Unknown message type received from Client[" << client.socket << "].\n";
                    break;
                }
        }
        
        lobbyLoop();
    }
    std::cout << "Closing\n";
    close(server_fd);
}

void Host::lobbyLoop()
{
    // List games // for players in lobby
    for (Player& client : clients)
    {

        if (hasState(client, playerState::MAIN_LOBBY))
        {
            // List players in no lobby
            // List games (in game, in lobby)
                // List players in this lobby/game
            // | ID:# | <Game Name> | Players (X/MAX):(<list players here>) 
            // Show lobby options (join/quit/cmds)
            sendMessageToClient(&client, ServerMessage::PROMPT, "Type '#' to join game lobby\n- Type 'q' to quit");
        }

    // Prompt all in main lobby option to join game lobby gamelobbystates:(IN_LOBBY, IN_GAME, )

    // If joined game lobby, add player to game instance
    // handle ready logic from game







    // Check if game should start
    for (Player& client : clients)
    {
        if (hasState(client, playerState::MAIN_LOBBY))
        {
            // Only send prompt if not already prompted
            if (!client.isPrompted)
                reprompt(&client);

            // Read up logic
            if (client.isPrompted)
            {
                auto response = getResponseFromClientPrompt(&client);
                if (response.empty()) continue;

                if (response == "x" || response == "X")
                {
                    promptComplete(&client);
                    setState(client, playerState::GAME_LOBBY);
                    clearState(client, playerState::MAIN_LOBBY);
                    std::cout << "Client[" << client.socket << "] is now ready.\n";
                    sendMessageToClient(&client, ServerMessage::NORMAL, "Joining game lobby...");
                    
                    // Add player to the game when they're ready
                    if (!gameInstance->hasStarted())
                    {
                        gameInstance->AddPlayer(client.name + " - " + std::to_string(client.socket), &client);
                    }
                }
                else
                {
                    reprompt(&client);
                    continue;
                }
            }
        }
    }

    // Check if all ready players should start game
    if (!gameInstance->hasStarted())
    {
        for (Player& client : clients)
        {
            if (hasState(client, playerState::MAIN_LOBBY))
            {
                break;
            }

            gameInstance->setGameAsReady();
        }
    }

    if (gameInstance->hasStarted())
    {
        gameInstance->gameTick();
    }
}

void Host::reprompt(Player * client)
{
    // Always resend the prompt to the client using its stored promptMessage
    if (client->promptMessage.empty()) return; // No prompt to send
    
    sendMessageToClient(client, ServerMessage::PROMPT, client->promptMessage);
}

void Host::sendMessageToClient(Player* client, ServerMessage messageType, const std::string& dataStr) 
{
    // Make sure 

    if (!client || client->socket <= 0) 
    {
        std::cerr << "Invalid client or socket.\n";
        return;
    }
    
    char msgType = static_cast<char>(messageType);
    std::string packet = std::string(1, msgType) + dataStr;
    
    // Send length prefix (4 bytes) + type (1 byte) + data
    uint32_t length = htonl(packet.size());
    std::string lengthPrefix(reinterpret_cast<const char*>(&length), 4);
    std::string fullPacket = lengthPrefix + packet;
    std::cout << "Server --> Client[" << client->socket << "]: " << packet << "\n";

    // Server side type logic
    switch (messageType)
    {
        case ServerMessage::NORMAL:
            break;
        case ServerMessage::CLEAR:
            break;
        case ServerMessage::CANCEL_PROMPT:
            // Server instructs client to cancel any current prompt
            client->isPrompted = false;
            client->promptMessage = "";
            break;
        case ServerMessage::PROMPT:
            client->promptMessage = dataStr;
            client->isPrompted = true;
            break;
        default:
            std::cerr << "Unknown message type.\n";
            return;
    }


    auto result = send(client->socket, fullPacket.c_str(), fullPacket.size(), 0);

    if (result < 0)
    {
        std::cerr << "Error sending data message: " << strerror(errno) << "\n";
    }

}
void Host::sendMessageToAll(ServerMessage messageType, const std::string& dataStr, std::vector<playerState> states)
{
    
    for (Player& client : clients) 
    {
        if (client.socket > 0) 
        {
            // Send once if the client matches any of the requested states
            bool sent = false;
            for (playerState state : states)
            {
                if (!sent && hasState(client, state))
                {
                    sendMessageToClient(&client, messageType, dataStr);
                    sent = true;
                    break;
                }
            }
        }
    }
}
void Host::promptComplete(Player * client)
{
    client->isPrompted = false;
    client->clientPromptResponse = "";
    client->promptMessage = "";
}

std::string Host::getResponseFromClientPrompt(Player * client)
{
    return client->clientPromptResponse;
}

Host::~Host() 
{
    // Close all client sockets
    for (Player& client : clients) 
    {
        if (client.socket > 0) 
        {
            close(client.socket);
            client = {}; // Reset the client object
        }
    }
}