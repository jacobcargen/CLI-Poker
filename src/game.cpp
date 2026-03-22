// game.cpp
#include <iostream>
#include "game.h"
#include "message_helper.h"
#include "host.h"
#include "ui.h"
#include <vector>
#include <map>
#include <algorithm>
#include "time.h"

// Store players here (TYPE 'gamePlayer' (game player) not to be confused with TYPE 'Player'(CLIENT))
std::vector<gamePlayer> players;
std::vector<card> deck;
gamePlayer * currentPlayerTurn = nullptr;
game currentGame = POKER;
int pTurn = 0;
bool isStarted = false;
Host * host;
bool waitingForPlayer = false;


pokerData pokerGameData;

// Constructor
Game::Game() { }

void Game::init(Host * h)
{
    host = h;
}
bool Game::hasStarted()
{
    return isStarted;
}
void Game::setGameAsReady()
{
    isStarted = true;
    waitingForPlayer = false;
    // Decide game here maybe
    startPoker();

}
void Game::AddPlayer(const std::string& name, Player * client)
{
    UI ui;
    host->sendMessageToClient(client, ServerMessage::NORMAL, "You've been added to the game.\n");

    gamePlayer p;
    p.client = client;
    p.name = name;
    p.cards = {}; // Initialize with an empty card list or other logic

    if (!isStarted)
    {
        players.push_back(p);
        std::cout << "Player added\n";
    }
    else
    {
        std::cout << "Game has already started. Cannot add new players.\n";
    }
}
void Game::RemovePlayer(Player * client)
{
    std::cout << "ill remove it later lol\n";
    
}
void Game::gameTick()
{
    std::cout << "--GAME TICK--\n";
    if (!isStarted) 
    {
        std::cout << "Game not started yet. Waiting...\n";
        return;
    }
    

    // Handle waiting for player response
    if (waitingForPlayer)
    {
        std::string lastResponse = host->getResponseFromClientPrompt(currentPlayerTurn->client);;
        
        if (!lastResponse.empty()) 
        {
            std::cout << "Received response: " << lastResponse << std::endl;

            // Process the response
            if (processResponse(lastResponse))
            {
                std::cout << "Valid response processed, moving to next player.\n";
                
                waitingForPlayer = false;
            }
            else
            {
                /*


                std::cout << "Trying reprompt" << std::endl;
                // FIX ME
                waitingForPlayer = true;
                host->reprompt(currentPlayerTurn->client);
                */
            }
        }
        else 
        {
            std::cout << "Waiting for response...\n";
        }
    }
    // Response success, proceed VVV
    // Not waiting for player anymore VVV
    processGame();
}
void Game::nextPlayer()
{
    if (players.empty())
    {
        waitingForPlayer = false;
        currentPlayerTurn = nullptr;
        return;
    }

    // Move to the next player in the list
    int attempts = players.size(); // To prevent infinite loops
    do
    {
        pTurn = (pTurn + 1) % players.size(); // Wrap around to the first player if needed
        currentPlayerTurn = &players.at(pTurn);

        // Ensure the current player is valid and not folded
        if (currentPlayerTurn != nullptr && currentPlayerTurn->client != nullptr && !currentPlayerTurn->isFolded)
        {
            waitingForPlayer = true; // Indicate we are now waiting for this player
            std::cout << "Next player [" << pTurn << "] -> " << currentPlayerTurn->name << std::endl;
            return;
        }

        attempts--;
        if (attempts <= 0)
        {
            std::cerr << "No valid players left to take a turn!" << std::endl;
            waitingForPlayer = false;
            return;
        }
    } while (true);
}

void Game::resetGame()
{
    host->promptComplete(currentPlayerTurn->client);

    eliminateBrokePlayers();
    if (players.size() < 2)
    {
        for (gamePlayer& p : players)
        {
            host->sendMessageToClient(p.client, ServerMessage::NORMAL, "Not enough players with money left. Waiting for a new game.\n");
        }
        isStarted = false;
        waitingForPlayer = false;
        currentPlayerTurn = nullptr;
        return;
    }

    pokerGameData.pot = 0;
    pokerGameData.round = 1;
    pokerGameData.gameRaise = 0;
    pokerGameData.expChocie = NONE;
    pokerGameData.sharedCards.clear();

    for (gamePlayer& player : players)
    {
        player.hasGone = false;
        player.isFolded = false;
        player.raise = 0;
        player.cards.clear();
        player.isBetting = false;
    }
    getNewDeck(deck);
    waitingForPlayer = false;

    // Set up the first playedr
    std::cout << "First player setting\n";
    pTurn = -1;
    std::cout << "Game has been reset. Starting a new game!" << std::endl;
    isStarted = true;
    host->promptComplete(currentPlayerTurn->client);
    host->enableOneTimeOverride();
}

void Game::getNewDeck(std::vector<card>& deck)
{
    deck.clear(); // Ensure the deck is empty before adding new cards.
    for (const std::string& face : FACES)
    {
        for (const std::string& suit : SUITS)
        {
            card c;
            c.face = face;
            c.suit = suit;
            //std::cout << "Deck Add >>> FACE: " << c.face << ", SUIT: " << c.suit << std::endl;
            deck.push_back(c);
        }
    }
}
// fix
void Game::drawCardToPlayer(gamePlayer * p, bool shouldBePrivate) 
{
    card c = getRandomCard();
    c.shouldBePrivate = shouldBePrivate;
    p->cards.push_back(c);
}
card Game::getRandomCard()
{

    if (deck.empty())
    {
        std::cout << "DECK EMPTY!" << std::endl;
        // Error condition
        card c;
        c.face = FACE_UNSET;
        c.suit = SUIT_UNSET;
        return c;
    }

    // Seed std::rand with the current time to ensure randomness
    std::srand(static_cast<unsigned int>(time(nullptr))); 
    int i = std::rand() % deck.size(); // Get a random index

    // Get the card at the random index
    card c;
    c.face = deck[i].face;
    c.suit = deck[i].suit;

    std::cout << "!!!!!card is " << c.face << c.suit << std::endl;

    // Remove the card from the deck
    deck.erase(deck.begin() + i);

    return c;
}
void Game::clearHands()
{
    // Clear hands
    for (gamePlayer& p : players)
    {
        p.cards.clear();
    }
}
bool Game::processResponse(const std::string &lastResponse)
{
    std::cout << "|||processResponse|||" << std::endl;
    switch (currentGame)
    {
    case POKER:
        return pokerResponse(lastResponse);
    
    default:
        std::cout << "NO GAME SET" << std::endl;
        return false;
        // error
    }
}
void Game::processGame()
{
    std::cout << "|||processGame|||" << std::endl;
    switch (currentGame)
    {
    case POKER:
        pokerGame();
        break;
    
    default:
        break;
    }
}

std::string Game::getDisplayedPlayer(gamePlayer * player, gamePlayer * viewingPlayer, bool isShow)
{
    UI ui;
    std::string str = "";

    str += player->name;
    str += "\t\t$";
    str += std::to_string(player->money);
    str += "\n";

    std::vector<cardByLine> cardsStr;
    for (card c : player->cards)
    {
        cardByLine cblCard;
        if (c.shouldBePrivate && viewingPlayer != player && !isShow)
        {
            // dont show
            cblCard = ui.cardToString(FACE_UNSET, SUIT_UNSET);
        }
        else
        {
            cblCard = ui.cardToString(c.face, c.suit);
        }
        cardsStr.push_back(cblCard);
    }
    std::string hand = ui.handToPrintable(cardsStr);
    str += hand;
    return str;
}


#pragma region POKER


void Game::startPoker()
{
    currentGame = POKER;

    for (gamePlayer& p : players)
    {
        p.money = 500;
        card c;
        p.isFolded = false;
        p.isBetting = false;
    }
    pokerGameData.round = 1;

    getNewDeck(deck);

    // Set up the first playedr
    std::cout << "First player setting\n";
    pTurn = -1;
}
bool Game::pokerResponse(const std::string &lastResponse)
{
    std::cout << "\n\n\t\t\t!!!FOUND: " << lastResponse << "\n\n";

    if (currentPlayerTurn == nullptr)
    {
        std::cout << "PLAYER NULL" << std::endl;
        return false;
    }

    currentPlayerTurn->hasGone = true;
    // Fold
    if (lastResponse == "f")
    {
        
        std::cout << "Folding." << std::endl;
        currentPlayerTurn->isFolded = true;
        return true;
        
    }

    if (pokerGameData.expChocie == BET_AMOUNT)
    {
        pokerGameData.expChocie = NONE;
        int betAmount = 0;
        bool isValidNumber = true;
        try
        {
            betAmount = std::stoi(lastResponse);
        }
        catch(const std::exception& e)
        {
            std::cout << "ERROR" << std::endl;
            std::cerr << e.what() << '\n';
            isValidNumber = false;
        }
        
        if (isValidNumber && betAmount > 0 && betAmount <= currentPlayerTurn->money)
        {
            std::cout << "Valid bet of $" + std::to_string(betAmount) + "\n";
            currentPlayerTurn->isBetting = false;
            currentPlayerTurn->money -= betAmount;
            pokerGameData.pot += betAmount;
            pokerGameData.gameRaise += betAmount;
            currentPlayerTurn->raise = pokerGameData.gameRaise;
            return true;
        }

        host->sendMessageToClient(currentPlayerTurn->client, ServerMessage::NORMAL, "Invalid bet amount. Enter a positive amount up to your balance.\n");
        host->sendMessageToClient(currentPlayerTurn->client, ServerMessage::PROMPT, "How much? $");
        pokerGameData.expChocie = BET_AMOUNT;
        return true;
    }
    else if (pokerGameData.expChocie == RAISE_AMOUNT)
    {
        pokerGameData.expChocie = NONE;
        int raiseAmount = 0;
        bool isValidNumber = true;
        try
        {
            raiseAmount = std::stoi(lastResponse);
        }
        catch(const std::exception& e)
        {
            std::cout << "ERROR" << std::endl;
            std::cerr << e.what() << '\n';
            isValidNumber = false;
        }
        
        int requiredAmt = std::max(0, pokerGameData.gameRaise - currentPlayerTurn->raise);
        if (isValidNumber && raiseAmount > 0 && currentPlayerTurn->money >= raiseAmount + requiredAmt)
        {
            std::cout << "Valid bet of $" + std::to_string(raiseAmount) + "\n";
            currentPlayerTurn->isBetting = false;
            currentPlayerTurn->money -= raiseAmount + requiredAmt;
            pokerGameData.pot += raiseAmount + requiredAmt;
            pokerGameData.gameRaise += raiseAmount;
            currentPlayerTurn->raise = pokerGameData.gameRaise;
            return true;
        }

        host->sendMessageToClient(currentPlayerTurn->client, ServerMessage::NORMAL, "Invalid raise amount. Enter a positive raise you can afford.\n");
        host->sendMessageToClient(currentPlayerTurn->client, ServerMessage::PROMPT, "How much? $");
        pokerGameData.expChocie = RAISE_AMOUNT;
        return true;
    }
    else if (pokerGameData.expChocie == BET_CHECK)
    {
        pokerGameData.expChocie = NONE;
        std::cout << currentPlayerTurn->name;
        if (lastResponse == "b") // Bet
        {
            currentPlayerTurn->isBetting = true;
            std::cout << " is betting ... ";
            host->promptComplete(currentPlayerTurn->client);
            host->sendMessageToClient(currentPlayerTurn->client, ServerMessage::PROMPT, "How much? $");
            pokerGameData.expChocie = BET_AMOUNT;
            return true;
        }
        else if (lastResponse == "c") // Check
        {
            std::cout << " has checked." << std::endl;
            return true;
        }
    }
    else if (pokerGameData.expChocie == CALL_RAISE)
    {
        pokerGameData.expChocie = NONE;
        std::cout << currentPlayerTurn->name;
        if (lastResponse == "r") // Raise
        {
            currentPlayerTurn->isBetting = true;
            host->promptComplete(currentPlayerTurn->client);
            host->sendMessageToClient(currentPlayerTurn->client, ServerMessage::PROMPT, "How much? $");
            pokerGameData.expChocie = RAISE_AMOUNT;
            std::cout << " is rasing ... " << std::endl;
            return true;
        }
        else if (lastResponse == "c") // Call
        {
            
            int requiredAmt = std::max(0, pokerGameData.gameRaise - currentPlayerTurn->raise);
            if (currentPlayerTurn->money < requiredAmt) // ALL IN
            {
                pokerGameData.pot += currentPlayerTurn->money;
                currentPlayerTurn->money = 0;
                // MAKE SURE THEY ONLY GET WHAT THEY PUT IN
                currentPlayerTurn->raise = pokerGameData.gameRaise;
                
                std::cout << " is going all in." << std::endl;
                return true;
            }
            else // Call
            {
                currentPlayerTurn->money -= requiredAmt;
                pokerGameData.pot += requiredAmt;
                currentPlayerTurn->raise = pokerGameData.gameRaise;

                std::cout << " has called." << std::endl;
                return true;
            }
        }
    }
    currentPlayerTurn->hasGone = false;
    pokerGameData.expChocie = NONE;
    return false;
}
void Game::determineWhoWins()
{

    gamePlayer * winningPlayer = determineWinningPlayer();
    // Tell all players who won
    for (gamePlayer &p : players)
    {
        std::string winMsg = "";
        if (&p == winningPlayer)
            winMsg = "YOU WON $" + std::to_string(pokerGameData.pot) + "!!!\n";
        else
            winMsg = "YOU LOST :(\n" + winningPlayer->name + " has won $" + std::to_string(pokerGameData.pot);
        
        host->sendMessageToClient(p.client, ServerMessage::NORMAL, winMsg + "\n\n" + "Loading next round in 10 seconds...");
    }
    sleep(10);
    winningPlayer->money += pokerGameData.pot;
    resetGame();
}

gamePlayer* Game::determineWinningPlayer()
{
    gamePlayer* winningPlayer = nullptr;
    int bestHandRank = -1; // Higher rank indicates a better hand
    std::vector<int> bestHandValues; // For tie-breaking

    for (gamePlayer& player : players)
    {
        // Skip folded players
        if (player.isFolded)
            continue;

        // Combine player's cards with shared cards
        std::vector<card> combinedCards = player.cards;
        combinedCards.insert(combinedCards.end(), pokerGameData.sharedCards.begin(), pokerGameData.sharedCards.end());

        // Determine hand rank and tie-breaking values
        int handRank;
        std::vector<int> handValues;

        evaluatePokerHand(combinedCards, handRank, handValues);

        // Compare current player's hand with the best hand so far
        if (handRank > bestHandRank || (handRank == bestHandRank && handValues > bestHandValues))
        {
            bestHandRank = handRank;
            bestHandValues = handValues;
            winningPlayer = &player;
        }
    }

    // Announce the winner or handle edge case
    if (winningPlayer == nullptr)
    {
        std::cout << "No winner found!" << std::endl;
    }
    else
    {
        std::cout << "Winning player is: " << winningPlayer->name << " with hand rank " << bestHandRank << std::endl;
    }

    return winningPlayer;
}

void Game::evaluatePokerHand(const std::vector<card>& cards, int& handRank, std::vector<int>& handValues)
{
    std::map<std::string, int> valueCount; // Map card face to frequency
    std::map<std::string, int> suitCount; // Map card suit to frequency

    // Map string faces to numerical values for easier comparison
    std::map<std::string, int> faceToValue = {
        {"2", 2}, {"3", 3}, {"4", 4}, {"5", 5}, {"6", 6}, {"7", 7},
        {"8", 8}, {"9", 9}, {"10", 10}, {"J", 11}, {"Q", 12}, {"K", 13}, {"A", 14}};

    // Count card values and suits
    for (const card& c : cards)
    {
        valueCount[c.face]++;
        suitCount[c.suit]++;
    }

    // Extract sorted unique face values as integers
    std::vector<int> uniqueFaces;
    for (const auto& pair : valueCount)
    {
        uniqueFaces.push_back(faceToValue[pair.first]);
    }
    std::sort(uniqueFaces.begin(), uniqueFaces.end());

    // Check for Flush
    bool isFlush = false;
    std::string flushSuit = "";
    for (const auto& suit : suitCount)
    {
        if (suit.second >= 5)
        {
            isFlush = true;
            flushSuit = suit.first;
            break;
        }
    }

    // Check for Straight
    bool isStraight = false;
    std::vector<int> straightValues;

    int consecutiveCount = 1;
    for (size_t i = 1; i < uniqueFaces.size(); ++i)
    {
        if (uniqueFaces[i] == uniqueFaces[i - 1] + 1)
        {
            consecutiveCount++;
            if (consecutiveCount >= 5)
            {
                isStraight = true;
                straightValues = std::vector<int>(uniqueFaces.begin() + i - 4, uniqueFaces.begin() + i + 1);
            }
        }
        else
        {
            consecutiveCount = 1;
        }
    }

    // Handle Ace-low straight (e.g., A-2-3-4-5)
    if (!isStraight && uniqueFaces.size() >= 5 &&
        uniqueFaces.back() == 14 && uniqueFaces[0] == 2 &&
        uniqueFaces[1] == 3 && uniqueFaces[2] == 4 && uniqueFaces[3] == 5)
    {
        isStraight = true;
        straightValues = {5, 4, 3, 2, 1};
    }

    // Check for Straight Flush
    bool isStraightFlush = false;
    if (isFlush && isStraight)
    {
        std::vector<int> flushCards;
        for (const card& c : cards)
        {
            if (c.suit == flushSuit)
                flushCards.push_back(faceToValue[c.face]);
        }
        std::sort(flushCards.begin(), flushCards.end());
        int consecutiveFlushCount = 1;
        for (size_t i = 1; i < flushCards.size(); ++i)
        {
            if (flushCards[i] == flushCards[i - 1] + 1)
            {
                consecutiveFlushCount++;
                if (consecutiveFlushCount >= 5)
                {
                    isStraightFlush = true;
                    straightValues = std::vector<int>(flushCards.begin() + i - 4, flushCards.begin() + i + 1);
                }
            }
            else
            {
                consecutiveFlushCount = 1;
            }
        }
    }

    // Check for Four of a Kind, Full House, Three of a Kind, Two Pair, and One Pair
    std::vector<std::pair<int, int>> counts; // {frequency, value}
    for (const auto& pair : valueCount)
    {
        counts.push_back({pair.second, faceToValue[pair.first]});
    }
    std::sort(counts.begin(), counts.end(), std::greater<>());

    // Rank hands by strength
    if (isStraightFlush)
    {
        handRank = 8; // Straight Flush
        handValues = straightValues;
    }
    else if (counts[0].first == 4)
    {
        handRank = 7; // Four of a Kind
        handValues = {counts[0].second, counts[1].second};
    }
    else if (counts[0].first == 3 && counts[1].first >= 2)
    {
        handRank = 6; // Full House
        handValues = {counts[0].second, counts[1].second};
    }
    else if (isFlush)
    {
        handRank = 5; // Flush
        handValues = {};
        for (const card& c : cards)
        {
            if (c.suit == flushSuit)
                handValues.push_back(faceToValue[c.face]);
        }
        std::sort(handValues.begin(), handValues.end(), std::greater<>());
    }
    else if (isStraight)
    {
        handRank = 4; // Straight
        handValues = straightValues;
    }
    else if (counts[0].first == 3)
    {
        handRank = 3; // Three of a Kind
        handValues = {counts[0].second};
        for (const auto& pair : counts)
        {
            if (pair.second != counts[0].second)
                handValues.push_back(pair.second);
        }
    }
    else if (counts[0].first == 2 && counts[1].first == 2)
    {
        handRank = 2; // Two Pair
        handValues = {counts[0].second, counts[1].second, counts[2].second};
    }
    else if (counts[0].first == 2)
    {
        handRank = 1; // One Pair
        handValues = {counts[0].second};
        for (const auto& pair : counts)
        {
            if (pair.second != counts[0].second)
                handValues.push_back(pair.second);
        }
    }
    else
    {
        handRank = 0; // High Card
        for (const card& c : cards)
        {
            handValues.push_back(faceToValue[c.face]);
        }
        std::sort(handValues.begin(), handValues.end(), std::greater<>());
    }
}

void Game::pokerGame()
{
    std::cout << "|||pokerGame|||" << std::endl;
    UI ui;

    eliminateBrokePlayers();
    if (players.size() < 2)
    {
        resetGame();
        return;
    }

    
    if (currentPlayerTurn != nullptr && currentPlayerTurn->isBetting)
    {
        std::cout << "STILL BETTING\n";
        waitingForPlayer = true;
        return;

    }
    

    //if (!currentPlayerTurn->hasGone)
    //{
        //return;
    //}
    // Next player turn
    nextPlayer();
    int breakLmt = players.size();

    // Skip folded players
    while (currentPlayerTurn->isFolded)
    {
        std::cout << "looping\n";
        nextPlayer();
        breakLmt--;
        if (breakLmt < 0)
        {
            std::cout << "End game\n";
            UpdateDisplayForAll(true);
            determineWhoWins();
            // Start new game
            return;
        }
    }

    // Check how many players are left in the game
    int pLeft = 0;
    for (gamePlayer& p : players)
    {
        if (!p.isFolded)
            pLeft++;
    }

    if (pLeft <= 1)
    {
        UpdateDisplayForAll(true);
        determineWhoWins();
        resetGame();
        return;
    }


    // Check if the round is finished
    bool isRoundFinished = true;

    for (gamePlayer& p : players)
    {
        if (!p.isFolded)
        {
            
            if (!(p.hasGone && p.raise == pokerGameData.gameRaise))
            {
                isRoundFinished = false;
                break;
            }
        }
    }
    // Move to the next round if all players are finished
    if (isRoundFinished || pokerGameData.round == 1)
    {
        std::cout << "All players have acted. Advancing to the next round." << std::endl;

        // Reset `hasGone` for all players
        for (gamePlayer& p : players)
            p.hasGone = false;

        // Handle the next round
        switch (pokerGameData.round++)
        {
        case 1: // Give each player 2 cards
            std::cout << "Drawing 2 cards to each player\n";
            for (gamePlayer& p : players)
            {
                if (!p.isFolded)
                {
                    drawCardToPlayer(&p, true);
                    drawCardToPlayer(&p, true);
                }
            }
            break;
        case 2: // 3 cards in the middle
            pokerGameData.sharedCards.push_back(getRandomCard());
            pokerGameData.sharedCards.push_back(getRandomCard());
            pokerGameData.sharedCards.push_back(getRandomCard());
            break;
        case 3: // Fourth card
            pokerGameData.sharedCards.push_back(getRandomCard());
            break;
        case 4: // Last round
            pokerGameData.sharedCards.push_back(getRandomCard());
            break;
        case 5:
            std::cout << "Game round finished. Determine the winner." << std::endl;
            // Handle end-of-game logic here

            UpdateDisplayForAll(true);

            determineWhoWins();
            return;
        default:
            std::cout << "why?\n";
            return;
        }
    }


    
    UpdateDisplayForAll(false);


    // Prompt currentPlayerTurn
    bool isAnyBets = false;
    for (gamePlayer &p : players)
    {
        if (p.raise != pokerGameData.gameRaise && !p.isFolded)
            isAnyBets = true;
    }
    if (!isAnyBets)
    {
        pokerGameData.expChocie = BET_CHECK;
        host->sendMessageToClient(currentPlayerTurn->client, ServerMessage::PROMPT, "Check ('c')   Bet ('b')   Fold ('f'): ");
    }
    else if (currentPlayerTurn->raise != pokerGameData.gameRaise)
    {
        pokerGameData.expChocie = CALL_RAISE;
        host->sendMessageToClient(currentPlayerTurn->client, ServerMessage::PROMPT, "Call ('c')   Raise ('r')   Fold ('f'): ");
    }
}

void Game::eliminateBrokePlayers()
{
    std::vector<gamePlayer*> removed;
    bool removedCurrentPlayer = false;
    for (gamePlayer& p : players)
    {
        if (p.money <= 0)
        {
            removed.push_back(&p);
            if (&p == currentPlayerTurn)
                removedCurrentPlayer = true;
        }
    }

    for (gamePlayer* p : removed)
    {
        if (p != nullptr && p->client != nullptr)
            host->sendMessageToClient(p->client, ServerMessage::NORMAL, "You are out of money and have been removed from the game.\n");
    }

    players.erase(
        std::remove_if(players.begin(), players.end(), [](const gamePlayer& p)
        {
            return p.money <= 0;
        }),
        players.end()
    );

    if (removedCurrentPlayer)
    {
        currentPlayerTurn = nullptr;
        pTurn = -1;
    }

    if (players.empty())
    {
        currentPlayerTurn = nullptr;
        pTurn = 0;
        return;
    }

    if (pTurn >= static_cast<int>(players.size()))
        pTurn = static_cast<int>(players.size()) - 1;

    if (pTurn < -1)
        pTurn = -1;
}

void Game::UpdateDisplayForAll(bool isLast)
{
    UI ui;
    // Update displays for all players
    std::cout << "Updating displays\n";
    for (gamePlayer& player : players)
    {
        // Clear screen
        host->sendMessageToClient(player.client, ServerMessage::NORMAL, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

        std::cout << "Displaying for -> " << player.name << std::endl;

        std::string str = "|DEALER| Pot: $" + std::to_string(pokerGameData.pot) + 
                " Current Raise: $"+ std::to_string(pokerGameData.gameRaise) + "\n";
        std::vector<cardByLine> cardsStr;

        // Display shared cards
        for (card c : pokerGameData.sharedCards)
        {
            cardByLine cblCard = ui.cardToString(c.face, c.suit);
            cardsStr.push_back(cblCard);
        }
        std::string hand = ui.handToPrintable(cardsStr);
        str += hand;

        // Display player statuses
        for (gamePlayer& otherPlayer : players)
        {
            if (&otherPlayer == currentPlayerTurn && !isLast)
                str += "[WAITING]";
            if (&player == &otherPlayer)
                str += "[YOU]";
            if (otherPlayer.isFolded)
                str += "[FOLDED]";
            str += " \tRaise: $" + std::to_string(otherPlayer.raise) + "   ";
            std::string s;
            if (isLast)
            {
                s = getDisplayedPlayer(&otherPlayer, &player, true);
            }
            else
            {
                s = getDisplayedPlayer(&otherPlayer, &player, false);
            }
            str += s;
        }

        host->sendMessageToClient(player.client, ServerMessage::NORMAL, str);
    }
}

#pragma endregion
