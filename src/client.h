// client.h
#ifndef CLIENT_H
#define CLIENT_H

#include "message_helper.h"

class Client {
public:
    // Constructor
    Client();

private:
    // Private
    void Start();
    void Join(std::string ip);
    void sendMessageToHost(int socket, ClientMessage messageType, const std::string& dataStr);

};

#endif // CLIENT_H
