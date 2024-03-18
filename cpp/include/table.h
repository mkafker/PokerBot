#pragma once
#include <list>
#include <memory>
#include <algorithm>

#include "card.h"
#include "strategy.h"
#include "player.h"
using namespace std;
namespace Poker {

    class Player;
    enum class PlayerPosition;
    class Table {
        private:
            Deck deck;
        public:
            shared_ptr<random_device> rd;
            // playerList contains Player classes which by default call every time
            // Overload the virtual function makeMove to define new behavior
            vector<shared_ptr<Player>> playerList;
            
            vector<Card> communityCards;
            int                 street              = 0;                // phase of the game. 0 = preflop, 1 = flop, 2 = turn, 3 = river
            int                 bigBlind            = 10;
            int                 smallBlind          = 5;
            int                 pot                 = 0;
            int                 minimumBet          = 0;
            shared_ptr<Deck>   getDeck();
            void setPlayerList(const vector<string> sVec);
            void resetCards(random_device& );
            void dealCommunityCards(int );
            void dealPlayersCards(const std::vector<shared_ptr<Player>>);
            void clearPlayerHands();
            void setPlayerBankrolls(int n = 100);
            vector<shared_ptr<Player>> getPlayersInBettingOrder(vector<shared_ptr<Player>> in = vector<shared_ptr<Player>>());

            bool arePlayerPositionsValid(const vector<shared_ptr<Player>>& pList);

            void shuffleDeck();
            shared_ptr<Player> getPlayerByID(const int& id);
    };
}
