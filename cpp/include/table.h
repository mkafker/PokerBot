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
            std::vector<Player> player_list;
            
            std::vector<Card> community_cards;
            int                 street              = 0;                // phase of the game. 0 = preflop, 1 = flop, 2 = turn, 3 = river
            int                 bigBlind            = 10;
            int                 smallBlind          = 15;
            int                 pot                 = 0;
            int                 minimumBet          = 0;
            Table() = default;
            std::shared_ptr<Deck>   getDeck();

            void reset(int n = 6);
            void dealCommunityCards();
            void dealAllPlayersCards();
            
            std::vector<Player*> getActivePlayers(std::vector<Player*> pvector = std::vector<Player*>());
            std::vector<Player*> getPlayersInOrder(std::vector<Player*> pvector = std::vector<Player*>());
            std::vector<Player*> getNonBankruptPlayers(std::vector<Player*> pvector = std::vector<Player*>());

            
            void set_num_players(int n);
            void kick_player(int i);
            void rotatePlayers();
            void resetPlayerHands();

            void shuffleDeck();
    };
}
