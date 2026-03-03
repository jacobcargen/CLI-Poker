// game.h
#ifndef GAME_H
#define GAME_H

#include "ui.h"
#include <string>
#include <vector>
#include <algorithm>
#include "host.h"

enum game {
    POKER = 2,
} typedef game;

struct card
{
    bool shouldBePrivate = false;
    std::string face = FACE_UNSET;
    std::string suit = SUIT_UNSET;
} typedef card;

struct gamePlayer
{
    Player * client;
    int id; // not used i think
    std::string name = "no name";
    std::vector<card> cards;
    int money = 0;
    int raise = 0;
    bool isFolded = false;
    bool hasGone = false;
    bool isBetting = false;

} typedef gamePlayer;

#pragma region Poker
enum expectedChoice
{
    BET_CHECK,
    CALL_RAISE,

    RAISE_AMOUNT,
    BET_AMOUNT,

    NONE,
};
struct pokerData
{
    int round = 0;
    int pot = 0;
    int minBet = 0;
    int gameRaise = 0;
    expectedChoice expChocie = BET_CHECK;
    std::vector<card> sharedCards;
} typedef pokerData;
// Poker variables

#pragma endregion

class Game {
public:
    // Constructor
    Game();
    // Public methods
    void init(Host * h);
    void test();
    bool hasStarted();
    void setGameAsReady();
    void AddPlayer(const std::string& name, Player * client);
    void RemovePlayer(Player * client);
    void gameTick();

private:
    // Private
    std::vector<gamePlayer> players;
    std::vector<card> deck;
    gamePlayer * currentPlayerTurn;
    game currentGame;
    int pTurn;
    bool isStarted;
    Host * host;
    bool waitingForPlayer;
    void resetGame();
    void clearHands();
    void nextPlayer();
    void getNewDeck(std::vector<card> &deck);
    void processGame();
    bool processResponse(const std::string &lastResponse);

    void evaluatePokerHand(const std::vector<card>& cards, int& handRank, std::vector<int>& handValues);
    gamePlayer* determineWinningPlayer();
    void drawCardToPlayer(gamePlayer * p, bool shouldBePrivate);
    card getRandomCard();
    std::string getDisplayedPlayer(gamePlayer * player, gamePlayer * viewingPlayer, bool isShow);
    void startPoker();
    void determineWhoWins();
    void pokerGame();
    bool pokerResponse(const std::string &lastResponse);
    void UpdateDisplayForAll(bool isLast);
    void eliminateBrokePlayers();
};

#endif // GAME_H
