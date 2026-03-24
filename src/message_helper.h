#ifndef MESSAGE_HELPER_H
#define MESSAGE_HELPER_H


const int MESSAGE_SIZE = 4096;

enum class ServerMessage : char {
    NORMAL = 40,
    CLEAR = 41,
    PROMPT = 42,
    PROMPT_KEY = 43,
    CANCEL_PROMPT = 44,
};

enum class ClientMessage : char {
    RESPONSE = 40,
    QUIT = 41,

};

#endif // MESSAGE_HELPER_H