#include <iostream>
#include "client.h"
#include "ui.h"
#include "message_helper.h"

#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>
#include <termios.h>
#include <deque>


// Constructor
Client::Client()
{
    Start();
}

// Private functions
void Client::Start()
{
    std::string addr;
    std::getline(std::cin, addr);
    while (addr.empty()) {
        std::cout << "Server IP: ";
        std::getline(std::cin, addr);
    }
    int colonIndex = addr.find_first_of(":") ? addr.find_first_of(":") : -1;
    std::string ip = addr.substr(0, colonIndex);
    std::string port = (colonIndex != -1) ? addr.substr(colonIndex + 1) : "6769";
    if (ip == "l" || ip == "localhost" || ip == "LOCALHOST" || ip == "")
        ip = "127.0.0.1";
    Join(ip, port);
}

// Trim whitespace from both ends
static inline std::string trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

void Client::Join(std::string ip, std::string portStr)
{
    int port = std::stoi(portStr);
    int sock = 0;
    UI ui;

    sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation failed: " << strerror(errno) << "\n";
        return;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address or address not supported\n";
        return;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection to server failed: " << strerror(errno) << "\n";
        return;
    }
    std::cout << "Connected to server!\n";
    std::deque<std::string> history;
    const size_t HISTORY_MAX = 100;
    while (true) {
        char lenBuffer[4] = {0};
        int lenRead = read(sock, lenBuffer, 4);
        if (lenRead <= 0) {
            if (lenRead == 0) {
                std::cout << "Server disconnected.\n";
                break;
            } else {
                std::cout << "Read failed.\n";
                continue;
            }
        }
        uint32_t messageLen = ntohl(*reinterpret_cast<uint32_t*>(lenBuffer));
        if (messageLen > MESSAGE_SIZE) {
            std::cerr << "Message too large: " << messageLen << " bytes\n";
            continue;
        }
        char buffer[MESSAGE_SIZE] = {0};
        int valread = read(sock, buffer, messageLen);
        if (valread <= 0) {
            if (valread == 0) {
                std::cout << "Server disconnected.\n";
                break;
            } else {
                std::cout << "Read failed.\n";
                continue;
            }
        }
        std::string serverMessage(buffer, valread);
        if (serverMessage.empty()) {
            std::cerr << "Received empty message from server.\n";
            continue;
        }
        ServerMessage msgType = static_cast<ServerMessage>(serverMessage[0]);
        std::string parsedMsg = serverMessage.size() > 1 ? serverMessage.substr(1) : "";
        switch (msgType) {
            case ServerMessage::NORMAL:
                std::cout << parsedMsg << std::endl;
                // store displayed lines in history
                history.push_back(parsedMsg);
                if (history.size() > HISTORY_MAX) history.pop_front();
                break;
            case ServerMessage::CLEAR:
                    // Clear the terminal using UI and drop history so old lines aren't reprinted
                    ui.clearScreen();
                    history.clear();
                break;
            case ServerMessage::PROMPT: {
                std::string input;
                bool isFirst = true;
                while (input.empty())
                {
                    if (isFirst) {
                        isFirst = false;
                    }
                    else
                    {
                        // Reprint last HISTORY_MAX lines except parsedMsg
                        ui.clearScreen();
                        for (const auto &line : history) {
                            if (!parsedMsg.empty() && line == parsedMsg) continue;
                            std::cout << line << std::endl;
                        }
                    }

                    input = promptLineWithSelect(sock, parsedMsg);
                    input = trim(input);
                }

                if (input == "q" || input == "Q") {
                    sendMessageToHost(sock, ClientMessage::QUIT, "Client is quitting.");
                    std::cout << "Quitting...\n";
                    // Flush any pending stdin so it doesn't fall through to the shell
                    tcflush(STDIN_FILENO, TCIFLUSH);
                    close(sock);
                    return;
                }
                if (!input.empty())
                    sendMessageToHost(sock, ClientMessage::RESPONSE, input);
                break;
            }
            case ServerMessage::CANCEL_PROMPT:
                // Ignore, not important in this scenario
                break;
            default:
                std::cerr << "Unknown message type received.\n";
                continue;
        }
    }

    // Flush stdin to prevent any leftover input from falling through to the shell
    tcflush(STDIN_FILENO, TCIFLUSH);
    // Close the socket when done
    close(sock);

    
}
void Client::sendMessageToHost(int socket, ClientMessage messageType, const std::string& dataStr)
{
    char msgType = static_cast<char>(messageType);
    std::string packet = std::string(1, msgType) + dataStr;
    //std::cout << "Client --> Server MSGTYPE:{"<<(int)msgType<<"}: " << packet << "\n";
    send(socket, packet.c_str(), packet.size(), 0);
}

// Prompt for input, but also listen for RESET from server
std::string Client::promptLineWithSelect(int sock, const std::string& prompt)
{
    std::string userInput;
    std::cout << prompt;
    std::cout.flush();
    userInput.clear();
    while (true) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
        int maxfd = (sock > STDIN_FILENO) ? sock : STDIN_FILENO;
        int activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            std::cerr << "select() error\n";
            return "";
        }
        if (FD_ISSET(sock, &readfds)) {
            // Incoming message from server
            char lenBuffer[4] = {0};
            int lenRead = read(sock, lenBuffer, 4);
            if (lenRead <= 0) return "";
            uint32_t messageLen = ntohl(*reinterpret_cast<uint32_t*>(lenBuffer));
            if (messageLen > MESSAGE_SIZE) return "";
            char buffer[MESSAGE_SIZE] = {0};
            int valread = read(sock, buffer, messageLen);
            if (valread <= 0) return "";
            std::string serverMessage(buffer, valread);
            if (serverMessage.empty()) return "";
            ServerMessage msgType = static_cast<ServerMessage>(serverMessage[0]);
            std::string parsedMsg = serverMessage.size() > 1 ? serverMessage.substr(1) : "";
            if (msgType == ServerMessage::CANCEL_PROMPT) {
                std::cout << "\n[Input cancelled by server message]\n";
                return "";
            }
            return "";
        }
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            std::getline(std::cin, userInput);
            userInput = trim(userInput);
            break;
        }
    }
    return userInput;
}
