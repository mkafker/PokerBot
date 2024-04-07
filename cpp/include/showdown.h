#pragma once
#include <unordered_map>
#include <algorithm>
#include <list>
#include "card.h"

namespace Poker {
    enum class WinState : int {
        WINSTATE_UNDEF = -9999,
        WINSTATE_A = -1,
        WINSTATE_B = 1,
        WINSTATE_DRAW
    };
    enum class HandRank : int {
        UNDEF_HANDRANK = 0,
        HIGH_CARD = 1,
        ONE_PAIR = 2,
        TWO_PAIR = 3,
        THREE_KIND = 4,
        STRAIGHT = 5,
        FLUSH = 6,
        FULL_HOUSE = 7,
        FOUR_KIND = 8, 
        STRAIGHT_FLUSH = 9
    };
    
    struct FullHandRank {
        // Holds full ranking information about a hand
        HandRank handrank;                  
        std::vector<Card> maincards;        
        std::vector<Card> kickers;          
    };
    static FullHandRank nullFHR;            // for erroneous FHRs

    bool operator==(const FullHandRank& a, const FullHandRank& b);
    std::ostream& operator<<(std::ostream& stream, const HandRank& a);

    static std::unordered_map<HandRank, std::string> HandRank_to_String {
        {HandRank::UNDEF_HANDRANK, "UNDEF_Hand"},
        {HandRank::HIGH_CARD, "High card"},
        {HandRank::ONE_PAIR,  "One pair"},
        {HandRank::TWO_PAIR,  "Two pair"},
        {HandRank::THREE_KIND,    "Three of a kind"},
        {HandRank::STRAIGHT,       "Straight"},
        {HandRank::FLUSH,          "Flush"},
        {HandRank::FULL_HOUSE,     "Full house"},
        {HandRank::FOUR_KIND,      "Four of a kind"},
        {HandRank::STRAIGHT_FLUSH, "Straight flush"}
    };

    inline std::ostream& operator<<(std::ostream& stream, FullHandRank& a) { 
      std::cout << HandRank_to_String[a.handrank] << " " << a.maincards << "| " << a.kickers;
        return stream;
    }





    inline void sortCards(std::vector<Card>& h);
    inline void sortCardsDescending(std::vector<Card>& h);

    const FullHandRank calcFullHandRank(const std::vector<Card>& hand_in);

    const FullHandRank& showdownFHR(const FullHandRank& A, const FullHandRank& B);

};






    



