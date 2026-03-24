// client.h
#ifndef CLIENT_H
#define CLIENT_H

#include "message_helper.h"

class Client {
public:
    // Constructor
    Client();

private:
    void Start();
    void Join(std::string ip, std::string port);
    void sendMessageToHost(int socket, ClientMessage messageType, const std::string& dataStr);
    std::string promptLineWithSelect(int sock, const std::string& prompt);

};

#endif // CLIENT_H
