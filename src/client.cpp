#include <iostream>
#include "client.h"
#include "ui.h"
#include "message_helper.h"

#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>

// Constructor
Client::Client()
{
    Start();
}

// Private functions
void Client::Start()
{
    UI ui;

    // Prompt for server IP address
    std::string ip;
    do {
        ip = ui.promptLine("Server IP:");
    } while (ip.empty());
    if (ip.compare("l") == 0 || ip.compare("localhost") == 0 || ip.compare("LOCALHOST") == 0 || ip.compare("") == 0)
    {
        ip = "127.0.0.1"; // Default IP address
    }
    // Attempt to join the ser        ver
    
    Join(ip);
}

void Client::Join(std::string ip)
{
    UI ui;
    const int PORT = 6769; // Server port
    int sock = 0;          // Client socket descriptor
    sockaddr_in serv_addr; // Server address struct

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        std::cerr << "Socket creation failed: " << strerror(errno) << "\n";
        return;
    }

    serv_addr.sin_family = AF_INET;      // IPv4
    serv_addr.sin_port = htons(PORT);   // Set port

    // Convert IP address from string to binary
    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) 
    {
        std::cerr << "Invalid address or address not supported\n";
        return;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
        std::cerr << "Connection to server failed: " << strerror(errno) << "\n";
        return;
    }

    std::cout << "Connected to server!\n";

    // Main Game Loop
    while (true) 
    {
        // Read length prefix (4 bytes)
        char lenBuffer[4] = {0};
        int lenRead = read(sock, lenBuffer, 4);
        
        if (lenRead <= 0) 
        {
            if (lenRead == 0)
            {
                std::cout << "Server disconnected.\n";
                break;
            }
            else
            {
                std::cout << "Read failed.\n";
                continue;
            }
        }
        
        uint32_t messageLen = ntohl(*reinterpret_cast<uint32_t*>(lenBuffer));
        
        // Validate message length
        if (messageLen > MESSAGE_SIZE)
        {
            std::cerr << "Message too large: " << messageLen << " bytes\n";
            continue;
        }
        
        // Read the message (type + data)
        char buffer[MESSAGE_SIZE] = {0};
        int valread = read(sock, buffer, messageLen);
        
        if (valread <= 0) 
        {
            if (valread == 0)
            {
                std::cout << "Server disconnected.\n";
                break;
            }
            else
            {
                std::cout << "Read failed.\n";
                continue;
            }
        }
        
        std::string serverMessage(buffer, valread);
        
        if (serverMessage.empty())
        {
            std::cerr << "Received empty message from server.\n";
            continue;
        }
        
        // Parse message type first
        ServerMessage msgType = static_cast<ServerMessage>(serverMessage[0]);
        std::string parsedMsg = serverMessage.size() > 1 ? serverMessage.substr(1) : "";
        
        // Parse message
        switch (msgType)
        {
            case ServerMessage::NORMAL:
                std::cout << parsedMsg << std::endl; // Display the message
                break;
            case ServerMessage::CLEAR:
                ui.clearScreen();
                break;
            case ServerMessage::PROMPT:
            {
                std::string input = ui.promptLine(parsedMsg);
                if (input == "q" || input == "Q")
                {
                    sendMessageToHost(sock, ClientMessage::QUIT, "Client is quitting.");
                    std::cout << "Quitting...\n";
                    break;
                }
                sendMessageToHost(sock, ClientMessage::RESPONSE, input);
                break;
            }
            default:
                std::cerr << "Unknown message type received.\n";
                continue;
        }
        
    }

    // Close the socket when done
    close(sock);

    
}
void Client::sendMessageToHost(int socket, ClientMessage messageType, const std::string& dataStr)
{
    char msgType = static_cast<char>(messageType);
    std::string packet = std::string(1, msgType) + dataStr;
    //std::cout << "Client --> Server: " << packet << "\n";
    send(socket, packet.c_str(), packet.size(), 0);
}
