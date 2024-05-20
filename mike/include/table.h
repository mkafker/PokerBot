#pragma once
#include <list>
#include <memory>
#include <algorithm>
#include <map>

#include "card.h"
#include "strategy.h"
#include "player.h"
using namespace std;
namespace Poker {
    enum class Street {
        UNDEF   = 999,
        PREFLOP = 0,
        FLOP    = 1,
        TURN    = 2,
        RIVER   = 3
    };
    static std::unordered_map<Street, string> StreetToString 
    {
        {Street::UNDEF, "UNDEF!"},
        {Street::PREFLOP, "PREFLOP"},
        {Street::FLOP,    "FLOP"},
        {Street::TURN,    "TURN"},
        {Street::RIVER,   "RIVER"}
    };

    // If a Street is incremented past the RIVER, it goes to UNDEF
    inline Street& operator++(Street& s) {
        auto intform = static_cast<int>(s) + 1;
        if( intform == 4 ) s = Street::UNDEF;
        else               s = static_cast<Street>(intform);
        return s;
    }
    inline Street operator++(Street& s, int) {
        Street stemp = s;
        ++s;
        return stemp;
    }
    inline std::ostream& operator<<(std::ostream& os, Street& s) {
        os << StreetToString[s];
        return os;
    }
    inline bool operator<=(Street a, Street b) {
        //if( a == Street::UNDEF || b == Street::UNDEF ) throw;
        return (static_cast<int>(a) <= static_cast<int>(b));
    }


    class Player;
    struct PlayerMove;
    enum class PlayerPosition;
    class Table {
        private:
            Deck deck;
        public:
            vector<Player> playerList;
            vector<Card> communityCards;
            Street              street              = Street::PREFLOP;
            int                 bigBlind            = 10;
            int                 smallBlind          = 5;
            int                 pot                 = 0;
            int                 minimumBet          = 0;
            bool skipTurn = false;
            map<Player*, vector<PlayerMove>> playerMoveMap;
            unique_ptr<Deck>   getDeck();
            void setPlayerList(const vector<string> sVec);
            void resetCards(random_device& );
            void dealCommunityCards(const Street&);
            void dealPlayersCards(const std::vector<Player*>);
            void clearPlayerHands();
            void setPlayerBankrolls(int n = 100);
            vector<shared_ptr<Player>> getPlayersInBettingOrder(const vector<shared_ptr<Player>> in = vector<shared_ptr<Player>>());
            vector<Player*> getPlayersInBettingOrder(const vector<Player*> in = vector<Player*>());

            bool arePlayerPositionsValid(const vector<shared_ptr<Player>>& pList);

            void shuffleDeck();
            unique_ptr<Player> getPlayerByID(const int& id);
    };
}
