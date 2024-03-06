#pragma once
#include <list>
#include <memory>
#include <algorithm>

#include "card.h"
namespace Poker {

    class Player;
    enum class PlayerPosition;
    class Table {
        private:
            Deck deck;
            std::mt19937_64 g;
        public:
            std::list<Player> player_list;
            
            std::vector<Card> community_cards;
            int                 street              = 0;                // phase of the game. 0 = preflop, 1 = flop, 2 = turn, 3 = river
            int                 bigBlind            = 10;
            int                 smallBlind          = 15;
            int                 pot                 = 0;
            int                 minimumBet          = 0;
            Table() = default;
            std::shared_ptr<Deck>   getDeck();

            void init(int n = 6);
            void dealCommunityCards();
            void dealAllPlayersCards();
            
            std::list<Player*> getPlayersInOrder(std::list<Player*> plist = std::list<Player*>());
            std::list<Player*> getNonBankruptPlayers(std::list<Player*> plist = std::list<Player*>());

            
            void set_num_players(int n);
            void kick_player(int i);
            void rotatePlayers();
            void resetPlayerHands();
    };
}
