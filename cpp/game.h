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
                std::list<Player*> activePlayers         // holds players participating in betting (not folded or all-in)
                        = table->getPlayersInOrder();
                std::unordered_map<Player*, PlayerMove> playerStatuses;                   // holds playing, all-in, or fold status
                Player* winningPlayer;
                table->resetPlayerHands();

                // deal two cards to all
                table->dealAllPlayersCards();

                table->street = 0;      // preflop
                table->currentBet   = table->bigBlind;
                table->pot          = table->currentBet;

                while( table->street < 4) {
                    std::cout << "================================================" << std::endl;
                    activePlayers = table->getPlayersInOrder(activePlayers, false);
                    std::cout << "Phase " << table->street << " " << table->community_cards <<  std::endl;
                    
                    table->dealCommunityCards();
                    
                    // only do betting round if there are at least two active players
                    if( activePlayers.size() >= 2) {
                        bool keepgoing = true;                                                            // indicates we're ready for the next phase of the game
                        while(keepgoing) {
                            std::cout << "---------------------------------------" << std::endl;
                            keepgoing = false;
                            std::list<Player*>::iterator i = activePlayers.begin();
                            const int currentBetBeforeRound = table->currentBet;
                            while (i != activePlayers.end()) {
                                Player* P = *i;
                                if( P->position == PlayerPosition::POS_BB && table->street != 0) {
                                    table->pot += table->bigBlind;
                                    P->bankroll -= table->bigBlind;
                                }
                                else if( P->position == PlayerPosition::POS_SB && table->street != 0 ) {
                                    table->pot += table->smallBlind;
                                    P->bankroll -= table->smallBlind;
                                }
                                PlayerMove Pmove = P->makeAIMove(table);
                                P->printPlayerMove(Pmove);

                                const bool allIn = (Pmove.bet_amount == P->bankroll);
                                const bool allInOrRaise = Pmove.move == Move::MOVE_RAISE || allIn;
                                if( Pmove.move == Move::MOVE_RAISE || Pmove.move == Move::MOVE_ALLIN) table->currentBet = Pmove.bet_amount;

                                table->pot  += Pmove.bet_amount;
                                P->bankroll -= Pmove.bet_amount;

                                // remove all-in or folded players from play
                                if( allIn ) playerStatuses[P] = Pmove;
                                std::cout << "Current bet: " << table->currentBet << std::endl;
                                std::cout << "Current pot: " << table->pot << std::endl;

                                

                                // remove player from game if folded or all in
                                if( Pmove.move == Move::MOVE_FOLD or allIn) {
                                    i = activePlayers.erase(i);
                                }
                                else {
                                    // Do another round the table only if somebody raised
                                    if( currentBetBeforeRound != table->currentBet ) { keepgoing = true; }
                                    i++;
                                }
                            }
                        }
                    }
                    // advance game
                    table->street++;
                }

                // add all-in players back to activePlayers
                auto it = playerStatuses.begin();
                while(it != playerStatuses.end()) {
                    std::pair<Player*, PlayerMove> PPM(*it);
                    if( PPM.second.move == Move::MOVE_ALLIN)
                        std::transform(playerStatuses.begin(), playerStatuses.end(), std::back_inserter(activePlayers),
                        [](const std::pair<Player*, PlayerMove>& pair) {
                            return pair.first;
                        });
                    it++;
                }

                winningPlayer = determineWinner(activePlayers);
                if( winningPlayer ) {
                    // finally, reward player
                    winningPlayer->bankroll += table->pot;
                    std::cout << "Guess who won! Player " << winningPlayer->playerID << std::endl;

                }
                else {
                    std::cout << "Draw, returning bets!" << std::endl;
                    // else, draw
                    for( Player* P : activePlayers ) 
                        P->bankroll += playerStatuses[P].bet_amount;
                }
                
                rotatePlayers();
            }

            
            Player* determineWinner(std::list<Player*> players) {
                Player* winningPlayer;
                if ( players.size() == 1) {
                        return players.front();
                    }
                else if ( players.size() >= 2) {
                    // showdown
                    // Form hands from community cards and player cards
                    auto it = players.begin();
                    std::unordered_map<Hand*, Player*> playerHandMap;
                    std::list<Hand*> handList;
                    while( it != players.end()) {
                        Player* P = *it;
                        Hand* Phand = &(P->hand);
                        handList.push_back(Phand);
                        Phand->append(table->community_cards);
                        playerHandMap[Phand] = P;
                        it++;
                    }
                    Hand* winningHand = Hand::showdown(handList);
                    return playerHandMap[winningHand];
                }
                else {
                    return nullptr;
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
                const bool allIn    = move.move == Move::MOVE_ALLIN;
                const bool nobodyRaisedYet = table->currentBet == table->get_BB();

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
                if( table->currentBet >= P.bankroll ) {
                    move.bet_amount = P.bankroll;
                    move.move = Move::MOVE_ALLIN;
                    return false;
                }
                if( move.bet_amount < 0 ) {
                    move.bet_amount = 0;
                    move.move = Move::MOVE_FOLD;
                    return false;
                }
                return true;
            }
            void rotatePlayers() {
                for(auto &P : table->player_list) {
                    P.incPosition();
                }
            }
            void print() {
                for( Player& p : table->player_list ) {
                    std::cout << std::endl;
                    std::cout << "Player " << p.playerID << " at seat " << p.position  << "\t bank: " << p.bankroll << "\t hand: " << p.hand << std::endl;
                }
                std::cout << "Community cards: " << table->community_cards << std::endl;
                std::cout << "Pot: " << table->pot << std::endl;
                std::cout << "Current Bet: " << table->currentBet << std::endl;
            }

    };
}
    
