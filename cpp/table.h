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
            std::shared_ptr<Deck> deck;
        public:
            std::list<Player> player_list;
            int              n_players;
            
            std::vector<Card> community_cards;
            int                 street              = 0;                // phase of the game. 0 = preflop, 1 = flop, 2 = turn, 3 = river
            int                 bigBlind            = 10;
            int                 smallBlind          = 15;
            int                 pot                 = 0;
            int                 currentBet          = 0;
            Table();
            std::shared_ptr<Deck>   get_deck();
            const int               get_BB();
            const int               get_SB();

            void init(int n = 6);
            void dealCommunityCards();
            void dealAllPlayersCards();
            Player& get_player_by_position(PlayerPosition p);
            
            std::list<Player*> getPlayersInOrder(std::list<Player*> plist = std::list<Player*>(), bool startOfRound = true);


            void set_num_players(int n);
            void kick_player(int i);
            void rotatePlayers();
            void resetPlayerHands();
    };
}
