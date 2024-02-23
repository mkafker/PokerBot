#pragma once
#include<memory>
#include <list>
#include <iostream>
#include "table.h"
#include "player.h"


namespace Poker {



    class Game {
        public:
            std::shared_ptr<Table>      table;                              // The public members of Table are what are visible to all and will serve as input to the AI

            Game () {
                init();
            }
            void init() {
                table = std::make_shared<Table>();                          // make table (and deck)
                for(Player& p : table->player_list) p.bankroll = 100;       // give everybody a benjamin
                table->bigBlind     = 10;
                table->smallBlind   = 5;
                table->street       = 0;
                table->pot          = 0;
            }
            void doRound() {
                std::list<Player*> activePlayers       = table->getPlayersInOrder();
                Player* winningPlayer;
                // deal two cards to all
                table->dealAllPlayersCards();

                table->street = 0;      // preflop

                while( table->street < 4) {
                    // do community card dealing
                    table->dealCommunityCards();
                    table->currentBet   = 0;
                    table->pot          = 0;
                    bool keepgoing = true;                                                                              // indicates we're ready for the next phase of the game
                    while(keepgoing) {   
                        print();
                        keepgoing = false;
                        std::list<Player*>::iterator i = activePlayers.begin();
                        while (i != activePlayers.end()) {
                            Player* P = *i;
                            PlayerMove Pmove = P->makeAIMove(table);
                            const bool moveOK = sanityCheckMove(*P, Pmove);                                             // also clamps to correct bet values
                            if( Pmove.move == Move::MOVE_FOLD) {
                                i = activePlayers.erase(i);
                            }
                            else {
                                if( Pmove.move == Move::MOVE_RAISE) { keepgoing = true; }
                                table->currentBet += Pmove.bet_amount;
                                i++;
                            }
                        }
                    }
                    table->pot += table->currentBet;
                    
                    if ( activePlayers.size() == 1) {
                        winningPlayer = activePlayers.front();
                    }
                    if ( activePlayers.size() == 2) {
                        // showdown
                        // Form hands from community cards and player cards
                        auto it = activePlayers.begin();
                        while( it != activePlayers.end()) {

                        }
                    }

                    table->street++;    // advance phase of the game
                }
                
            }

            auto get_table() { return table; }
            bool sanityCheckMove(Player& P, PlayerMove& move) {
                // sanity checks player moves, and corrects them if it can
                // Player move takes precedence over bet amount
                const bool isBB = P.getPosition() == PlayerPosition::POS_BB;
                const bool isSB = P.getPosition() == PlayerPosition::POS_BB;
                const bool didCall = move.move == Move::MOVE_CALL;
                const bool didRaise = move.move == Move::MOVE_RAISE;
                const bool didFold  = move.move == Move::MOVE_FOLD;
                const bool nobodyRaisedYet = table->currentBet != table->get_BB();

                if(isBB && nobodyRaisedYet && (move.bet_amount != table->get_BB())) {
                    move.bet_amount = table->get_BB();
                    return false;
                }
                if(isSB && nobodyRaisedYet && (move.bet_amount != table->get_SB())) {
                    move.bet_amount = table->get_SB();
                    return false;
                }
                if(didCall && (move.bet_amount != table->currentBet)) {
                    move.bet_amount = table->currentBet;
                    return false;
                }
                if(didRaise && (move.bet_amount < 2*table->currentBet)) {
                    move.bet_amount = table->currentBet;
                    return false;
                }
            }
            
            void print() {
                for( Player& p : table->player_list ) {
                    std::cout << std::endl;
                    std::cout << "Player at seat " << p.position  << "\t bank: " << p.bankroll << "\t hand: " << p.hand << std::endl;
                }
                std::cout << "Community cards: " << table->community_cards << std::endl;
                std::cout << "Pot: " << table->pot << std::endl;
            }

    };
}
    
