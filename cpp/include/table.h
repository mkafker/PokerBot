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
            mt19937_64 g;
        public:
            vector<Player> playerList;
            
            vector<Card> communityCards;
            int                 street              = 0;                // phase of the game. 0 = preflop, 1 = flop, 2 = turn, 3 = river
            int                 bigBlind            = 10;
            int                 smallBlind          = 5;
            int                 pot                 = 0;
            int                 minimumBet          = 0;
            Table() = default;
            shared_ptr<Deck>   getDeck();

            void reset(int n = 6);
            void dealCommunityCards(int );
            void dealPlayersCards(const std::vector<shared_ptr<Player>>);
            void resetPlayerHands();
            vector<shared_ptr<Player>> getPlayersInOrder(vector<shared_ptr<Player>> in = vector<shared_ptr<Player>>());

            void shuffleDeck();
    };
}
