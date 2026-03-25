#ifndef GAME_INSTANCE_H
#define GAME_INSTANCE_H

#include <string>
#include <string>
// Forward declarations to avoid circular includes
class Host;
struct Player;

class GameInstance {
public:
    virtual ~GameInstance() = default;

    // Initialize the game instance with a Host pointer
    virtual void init(Host* h) { host = h; }

    // Lifecycle
    virtual bool hasStarted() const { return started; }
    virtual void setGameAsReady() { started = true; }

    // Player management
    virtual void AddPlayer(const std::string& name, Player* client) = 0;
    virtual void RemovePlayer(Player* client) = 0;

    // Main tick for game logic
    virtual void gameTick() = 0;

protected:
    Host* host = nullptr;
    bool started = false;
};

#endif // GAME_INSTANCE_H
