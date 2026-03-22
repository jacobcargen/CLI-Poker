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

Player clients[MAX_CLIENTS] = {};
bool oneTimeOverride = false;

// Constructor: Starts the host server when an object of Host is created
Host::Host() 
{
    start(); 
}

// Main server function
void Host::start() 
{
    
    Game game;
    game.init(this);

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
    address.sin_port = htons(PORT);      // Set the port to listen on
    int opt = 1;
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the configured address and port, and start listening
    if (bind(server_fd, (sockaddr*)&address, addrlen) < 0 || listen(server_fd, MAX_CLIENTS) < 0) 
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
    std::cout << "Port: " << PORT << std::endl;

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

        if (!oneTimeOverride)
        {
            std::cout << "Awaiting activity\n";
            int activity = select(max_sd + 1, &readfds, nullptr, nullptr, nullptr);
            if (activity <= 0)
            {
                std::cout << "returning, no activity\n";
                return;
            }

            // Handle new connections
            if (!game.hasStarted() && FD_ISSET(server_fd, &readfds)) 
            {
                int new_socket = accept(server_fd, (sockaddr*)&address, (socklen_t*)&addrlen);
                if (new_socket < 0) 
                {
                    perror("Accept failed");
                    continue;
                }

                for (Player& client : clients) 
                {
                    if (client.socket == 0) 
                    {
                        client.socket = new_socket;
                        std::cout << "Client[" << client.socket << "] has joined the server.\n";
                        sendMessageToClient(&client, ServerMessage::NORMAL, WELCOME_MSG);
                        sendMessageToClient(&client, ServerMessage::PROMPT, "Enter your nickname: ");
                        break;
                    }
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
                        sendMessageToClient(&client, ServerMessage::NORMAL, "Nickname set to " + client.name);
                        promptComplete(&client);
                        sendMessageToClient(&client, ServerMessage::PROMPT, "Type 'x' to join the game lobby. Type 'q' to quit.");
                    }
                    continue;
                }
                
                // Read up  logic
                if (!client.isReady)
                {
                    if (parsedMsg == "x" || parsedMsg == "X")
                    {
                        promptComplete(&client);
                        client.isReady = true;
                        std::cout << "Client[" << client.socket << "] is now ready.\n";
                        sendMessageToClient(&client, ServerMessage::NORMAL, "Joining game lobby...");
                    }
                    else
                    {
                        reprompt(&client);
                        continue;
                    }
                }

                // Parse message type
                switch (msgType)
                {
                case ClientMessage::QUIT:
                    // Disconnect Client
                    
                    break;
                case ClientMessage::RESPONSE:
                {
                    // Ready up logic - only if game hasn't started yet
                    if (!game.hasStarted()) 
                    {
                        // Add player to the game
                        game.AddPlayer(client.name + " - " + std::to_string(client.socket), &client);
                        client.isReady = true;
                        std::cout << "Client[" << client.socket << "] is now ready.\n";

                        // Count how many clients are connected
                        int connectedCount = 0;
                        int readyCount = 0;
                        for (const Player& c : clients) 
                        {
                            if (c.socket > 0) 
                            {
                                connectedCount++;
                                if (c.isReady)
                                    readyCount++;
                            }
                        }

                        

                        std::cout << "Ready: " << readyCount << "/" << connectedCount << "\n";
                        bool allReady = (connectedCount >= 2) && (readyCount == connectedCount);

                        if (allReady)
                        {
                            std::cout << "All players are ready! Starting game...\n";
                            game.setGameAsReady();
                        }
                    }
                    else if (client.isPrompted)
                    {
                        client.clientPromptResponse = parsedMsg;
                    }
                    break;
                }    
                default:
                    std::cout << "Unknown message type received from Client[" << client.socket << "].\n";
                    break;
                }
            }
        }
        

        if (oneTimeOverride) oneTimeOverride = false;
        
        
        if (game.hasStarted())
        {
            game.gameTick();
        }
    }
    std::cout << "Closing\n";
    close(server_fd);
}

void Host::enableOneTimeOverride()
{
    oneTimeOverride = true;
}

void Host::reprompt(Player * client)
{
    if (client->isPrompted)
    { 
        sendMessageToClient(client, ServerMessage::PROMPT, client->promptMessage);
    }
    else { std::cout << "cannot reprompt" << std::endl; }
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