// ui.cpp
#include "ui.h"


// Constructor
UI::UI() 
{
    Start();
}

// Public method
void UI::Start() 
{
    
}

void UI::clearScreen() 
{
    //system("clear");
    //std::cout << "\033[2J\033[1;1H"; // ANSI escape code to clear screen and move cursor to top-left
    //std::cout << "\x1B[2J\x1B[H" << std::flush; // Alternative ANSI escape code
    std::cout << std::string(100, '\n'); // Fallback: print many newlines to simulate clearing
}

std::string UI::promptLine(const std::string& text)
{
    std::cout << text;
    std::string input = "";
    std::cin >> input;
    return input;
}
std::string UI::getTestHand()
{
    cardByLine card0, card1, card2, card3, card4;
    card0 = cardToString(ACE, SPADES);
    card1 = cardToString(FOUR, DIAMONDS);
    card2 = cardToString(THREE, CLUBS);
    card3 = cardToString(FACE_UNSET, SUIT_UNSET);
    card4 = cardToString(FACE_UNSET, SUIT_UNSET);
    std::vector<cardByLine> hand;
    hand.push_back(card0);
    hand.push_back(card1);
    hand.push_back(card2);
    hand.push_back(card3);
    hand.push_back(card4);
    return handToPrintable(hand);
}

std::string UI::handToPrintable(std::vector<cardByLine> cards)
{
    std::string const SPACING = "  ";
    int cardCount = cards.size();
    
    std::string firstLine = "";
    std::string mid1Line = "";
    std::string mid2Line = "";
    std::string mid3Line = "";
    std::string lastLine = "";

    // First Line for each card
    for (int i = 0; i < cardCount; i++){
        firstLine += cards.at(i).firstLine;
        firstLine += SPACING;
    }

    for (int i = 0; i < cardCount; i++){
        mid1Line += cards.at(i).mid1Line;
        mid1Line += SPACING;
    }
    for (int i = 0; i < cardCount; i++){
        mid2Line += cards.at(i).mid2Line;
        mid2Line += SPACING;
    }
    for (int i = 0; i < cardCount; i++){
        mid3Line += cards.at(i).mid3Line;
        mid3Line += SPACING;
    }

    for (int i = 0; i < cardCount; i++){
        lastLine += cards.at(i).lastLine;
        lastLine += SPACING;
    }
    
    std::string lines = "";
    lines += firstLine + "\n"; 
    lines += mid1Line + "\n"; 
    lines += mid2Line + "\n"; 
    lines += mid3Line + "\n"; 
    lines += lastLine + "\n";
    return lines;
}

cardByLine UI::cardToString(std::string face, std::string suit)
{
    cardByLine cardLines;
    std::string str = "";
    //First line
    str += CARD_CORNER_TOP_LEFT;
    str += CARD_TOP_BOT;
    str += CARD_TOP_BOT;
    str += CARD_TOP_BOT;
    str += CARD_TOP_BOT;
    str += CARD_TOP_BOT;
    str += CARD_CORNER_TOP_RIGHT;
    cardLines.firstLine = str;
    //mid1,2,3
    if (face == FACE_UNSET || suit == SUIT_UNSET)
    {
        std::string line = "";
        line += CARD_LEFT_RIGHT;
        for (int i = 0; i < 5; i++)  
            line += FACE_UNSET;
        line += CARD_LEFT_RIGHT;

        cardLines.mid1Line = line;
        cardLines.mid2Line = line;
        cardLines.mid3Line = line;

    }
    else
    {
        // mid1
        str = "";
        str += CARD_LEFT_RIGHT;
        str += face;
        str += (face.compare(TEN)==0?"   ":"    ");
        str += CARD_LEFT_RIGHT;
        cardLines.mid1Line = str;
        // mid2
        str = "";
        str += CARD_LEFT_RIGHT;
        str += "  ";
        str += suit;
        str += "  ";
        str += CARD_LEFT_RIGHT;
        cardLines.mid2Line = str;
        // mid3
        str = "";
        str += CARD_LEFT_RIGHT;
        str += (face.compare(TEN)==0?"   ":"    ");
        str += face;
        str += CARD_LEFT_RIGHT;
        cardLines.mid3Line = str;
    }
    //last line
    str = "";
    str += CARD_CORNER_BOT_LEFT;
    str += CARD_TOP_BOT;
    str += CARD_TOP_BOT;
    str += CARD_TOP_BOT;
    str += CARD_TOP_BOT;
    str += CARD_TOP_BOT;
    str += CARD_CORNER_BOT_RIGHT;
    cardLines.lastLine = str;

    return cardLines;
}
