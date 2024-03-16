#pragma once
#include <list>
#include <memory>
#include <algorithm>

#include "card.h"
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
            void reset(random_device& );
            void resetDeck(random_device &rd);
            void dealCommunityCards(int );
            void dealPlayersCards(const std::vector<shared_ptr<Player>>);
            void resetPlayerHands();
            void resetPlayerBankrolls(int n = 100);
            vector<shared_ptr<Player>> getPlayersInOrder(vector<shared_ptr<Player>> in = vector<shared_ptr<Player>>());

            void shuffleDeck();
    };
}
