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
                reset();
            }
            void reset(int n = 6) {
                table = std::make_shared<Table>();
                table->reset(n);
                table->shuffleDeck();
                std::vector<PlayerPosition> playerPositionList = numPlayersToPositionList(n);
                for(int i = 0; i < n; i++ ) {
                    Player p;
                    p.setPosition(playerPositionList[i]);
                    p.bankroll = 100;
                    table->player_list.push_back(p);
                }
                table->bigBlind     = 10;
                table->smallBlind   = 5;
                table->street       = 0;
                table->pot          = 0;
            }

            void doGame() {
                // Call reset() before this function
                std::vector<Player*> players = table->getPlayersInOrder();
                std::vector<Player*> nonBankruptPlayers = table->getNonBankruptPlayers();
                while( nonBankruptPlayers.size() > 1 ) {
                    nonBankruptPlayers = table->getNonBankruptPlayers(nonBankruptPlayers);
                    doRound();
                }
                Player* winner = nonBankruptPlayers.front();
            }

            std::shared_ptr<Player> doRound() {
                // Goes around the table taking bets until every player is all-in, folded, or called, then does a showdown and returns a shared_ptr to the winner
                //
                // first, get a list of pointers to active players
                // Active players are those that are not bankrupt
                std::vector<Player*> activePlayers; 
                std::vector<Player> playerlist = table->player_list; // get list of Players to be converted in a list of pointers
                //std::transform(playerlist.begin(), playerlist.end(), std::back_inserter(activePlayers), [] (const Player& a) {return &a; } );
                std::for_each(playerlist.begin(), playerlist.end(), [&]( Player& p) { activePlayers.emplace_back(&p);});

                activePlayers = table->getActivePlayers(activePlayers);
                activePlayers = table->getPlayersInOrder(activePlayers);
                if( activePlayers.size() == 1 ) return std::make_shared<Player>(*activePlayers.front());

                std::unordered_map<Player*, PlayerMove> playerStatuses;                   // holds all-in or fold status
                std::shared_ptr<Player> winningPlayer;
                table->resetPlayerHands();

                // deal two cards to all
                table->dealAllPlayersCards();

                table->street = 0;                      // set to preflop
                table->minimumBet   = table->bigBlind; 
                table->pot          = table->minimumBet;

                while( table->street < 4) {
                    //std::cout << "================================================" << std::endl;
                    activePlayers = table->getPlayersInOrder(activePlayers);
                    //std::cout << "Phase " << table->street << " " << table->community_cards <<  std::endl;
                    
                    table->dealCommunityCards();
                    
                    // only do betting round if there are at least two active players
                    if( activePlayers.size() >= 2) {
                        bool keepgoing = true;                                                            // indicates we're ready for the next phase of the game
                        while(keepgoing) {
                            //std::cout << "---------------------------------------" << std::endl;
                            keepgoing = false;
                            auto i = activePlayers.begin();
                            const int minimumBetBeforeRound = table->minimumBet;
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
                                //P->printPlayerMove(Pmove);

                                const bool allIn = (Pmove.bet_amount == P->bankroll);
                                const bool allInOrRaise = Pmove.move == Move::MOVE_RAISE || allIn;
                                if( Pmove.move == Move::MOVE_RAISE || Pmove.move == Move::MOVE_ALLIN) table->minimumBet = Pmove.bet_amount;

                                table->pot  += Pmove.bet_amount;
                                P->bankroll -= Pmove.bet_amount;

                                // remove all-in or folded players from play
                                if( allIn ) playerStatuses[P] = Pmove;
                                //std::cout << "Current bet: " << table->minimumBet << std::endl;
                                //std::cout << "Current pot: " << table->pot << std::endl;

                                

                                // remove player from game if folded or all in
                                if( Pmove.move == Move::MOVE_FOLD or allIn) {
                                    i = activePlayers.erase(i);
                                }
                                else {
                                    // Do another round the table only if somebody raised
                                    if( minimumBetBeforeRound != table->minimumBet ) { keepgoing = true; }
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
                    const Move playerMove = (*it).second.move;
                    Player* player = (*it).first;
                    if( playerMove == Move::MOVE_ALLIN) {
                        activePlayers.emplace_back(player);
                    }
                    it++;
                }

                winningPlayer = std::make_shared<Player>(*determineWinner(activePlayers.begin(), activePlayers.end()));
                if( winningPlayer ) {
                    // finally, reward player
                    winningPlayer->bankroll += table->pot;
                    const int pID = winningPlayer->playerID;
                    FullHandRank fhr = calcFullHandRank(winningPlayer->hand);
                    winningPlayer->FHR = fhr;
                    return winningPlayer;
                    //std::cout << "Guess who won! Player " << winningPlayer->playerID 
                    //<< " with a " << 
                    //fhr.handrank << " " << fhr.maincards << "| " << fhr.kickers << std::endl;

                }
                else {
                    //std::cout << "Draw, returning bets!" << std::endl;
                    // else, draw
                    for( Player* P : activePlayers ) 
                        P->bankroll += playerStatuses[P].bet_amount;
                    return std::make_shared<Player>();
                }
            }

            
            Player* determineWinner(std::vector<Player*>::iterator players_begin, std::vector<Player*>::iterator players_end) {
                // From a vector of pointers to player, returns a pointer to the winner
                size_t numPlayers = std::distance(players_begin, players_end);
                if ( numPlayers == 0) { return nullptr; }
                else if ( numPlayers == 1) {
                        return *players_begin;
                }
                else if ( numPlayers >= 2) {
                    // showdown
                    // Form hands from community cards and player cards
                    auto it = players_begin;
                    std::unordered_map<std::vector<Card>*, Player*> playerHandMap;
                    std::vector<std::vector<Card>*> handList(numPlayers);
                    std::for_each(players_begin, players_end, [&playerHandMap, &handList](Player* p) {
                        std::vector<Card> h = p->hand;  // make a copy of the players hand
                        playerHandMap.emplace(&h, p);
                        handList.emplace_back(&h);
                    });

                    std::for_each(handList.begin(), handList.end(), [this](std::vector<Card>* h) {
                        auto commCards = table->community_cards;
                        h->insert(h->end(), commCards.begin(), commCards.end() );       // append community cards to the end of the (copied) players hand
                    });
                    
                    std::vector<Card>* winningHand = showdown(handList.begin(), handList.end());

                    return playerHandMap[winningHand];
                }
                else {
                    return nullptr;
                }
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
                std::cout << "Current Bet: " << table->minimumBet << std::endl;
            }

    };
}
    
