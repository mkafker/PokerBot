#pragma once
#include<memory>
#include <list>
#include <iostream>
#include "table.h"
#include "player.h"

using namespace std;

namespace Poker {
    class Game {
        public:

            
            Table      table;                              // The public members of Table are what are visible to all and will serve as input to the AI
            vector<shared_ptr<Player>> activePlayers;                   // The ones playing the game. This is so you can deactivate players if you wanted to. It should not change during a round
            vector<shared_ptr<Player>> foldedPlayers;
            vector<shared_ptr<Player>> allInPlayers;
            vector<shared_ptr<Player>> bettingPlayers;
            shared_ptr<Player> lastRoundWinner;


            // Betting players = activePlayers - foldedPlayers - allInPlayers
            // This could be done in an inline function or something?
            Game () {
                reset();
            }
            void reset(int n = 6) {
                // generate new RNG and fresh deck (unshuffled)
                table.reset(n);
                // shuffle cards
                table.shuffleDeck();
                // create players
                std::vector<PlayerPosition> playerPositionList = numPlayersToPositionList(n);
                for(int i = 0; i < n; i++ ) {
                    Player p;
                    p.setPosition(playerPositionList[i]);
                    p.bankroll = 100;
                    p.playerID = i;
                    table.playerList.push_back(p);
                }
                // set blinds and bet amounts
                table.bigBlind     = 10;
                table.smallBlind   = 5;
                table.street       = 0;
                table.pot          = 0;

                // set all players to active and betting
                activePlayers = table.getPlayersInOrder();
                bettingPlayers = activePlayers;

                // clear lists
                allInPlayers.clear();
                foldedPlayers.clear();
                lastRoundWinner = nullptr;
            }



            void doGame() {
                // Call reset() before this function
                std::vector<shared_ptr<Player>> activePlayers = table.getPlayersInOrder();
                while( bettingPlayers.size() > 1 ) {
                    doRound();
                }
                shared_ptr<Player> winner = bettingPlayers[0];
                std::cout << "Game finished: " << winner << " wins." <<  std::endl;
            }

            void doRound() {
                // Goes around the table taking bets until every player is all-in, folded, or called, then does a showdown
                // sets lastRoundWinner to a shared_ptr to the winning player
                // Modifies bettingPlayers

                // If only one player, that's the winner!
                if( bettingPlayers.size() == 1 ) return;
                allInPlayers.clear();
                foldedPlayers.clear();
                bettingPlayers = table.getPlayersInOrder(bettingPlayers);

                std::shared_ptr<Player> winningPlayer;
                table.resetPlayerHands();

                // deal two cards to all active players
                table.dealPlayersCards(bettingPlayers);

                table.street = 0;                      // set to preflop
                table.minimumBet   = table.bigBlind; 
                table.pot          = table.minimumBet;

                while( table.street < 4) {
                    std::cout << "================================================" << std::endl;
                    std::cout << "Phase " << table.street << " " << table.communityCards <<  std::endl;
                    
                    table.dealCommunityCards( table.street );
                    
                    bool keepgoing = true;                                                            // indicates that we need another round of betting
                    while(keepgoing) {
                        std::cout << "---------------------------------------" << std::endl;
                        keepgoing = false;
                        auto i = bettingPlayers.begin();
                        const int minimumBetBeforeRound = table.minimumBet;
                        while (i != bettingPlayers.end()) {
                            shared_ptr<Player> P = *i;
                            if( P->position == PlayerPosition::POS_BB) {
                                table.pot += table.bigBlind;
                                P->bankroll -= table.bigBlind;
                            }
                            else if( P->position == PlayerPosition::POS_SB) {
                                table.pot += table.smallBlind;
                                P->bankroll -= table.smallBlind;
                            }
                            PlayerMove Pmove = P->makeAIMove(make_shared<Table>(table));
                            P->move = Pmove;


                            const bool allIn = Pmove.move == Move::MOVE_ALLIN;
                            const bool allInOrRaise = Pmove.move == Move::MOVE_RAISE || allIn;
                            const bool folded = Pmove.move == Move::MOVE_FOLD;
                            if( allInOrRaise ) table.minimumBet = Pmove.bet_amount;

                            table.pot  += Pmove.bet_amount;
                            P->bankroll -= Pmove.bet_amount;

                            std::cout << "Current bet: " << table.minimumBet << std::endl;
                            std::cout << "Current pot: " << table.pot << std::endl;


                            // remove player from game if folded or all in
                            if( folded ) {
                                foldedPlayers.emplace_back(P);
                                i = bettingPlayers.erase(i);
                            }
                            else if( allIn ) {
                                allInPlayers.emplace_back(P);
                                i = bettingPlayers.erase(i);
                            }
                            else {
                                // Do another round the table only if somebody raised
                                if( minimumBetBeforeRound != table.minimumBet ) { keepgoing = true; }
                                i++;
                            }
                        }
                    }
                    // advance game
                    table.street++;
                }

                std::vector<shared_ptr<Player>> showdownPlayers;
                showdownPlayers.insert(showdownPlayers.end(), bettingPlayers.cbegin(), bettingPlayers.cend());
                showdownPlayers.insert(showdownPlayers.end(), allInPlayers.cbegin(), allInPlayers.cend());

                winningPlayer = determineWinner(showdownPlayers);

                if( winningPlayer ) {
                    // finally, reward player
                    winningPlayer->bankroll += table.pot;
                    const int pID = winningPlayer->playerID;

                    // FHRs cant be calculated now since the hand is unmodified 
                    // Need a function to do this in Table
                    std::vector<Card> handtmp = winningPlayer->hand;
                    handtmp.insert(handtmp.end(), table.communityCards.begin(), table.communityCards.end());
                    FullHandRank fhr = calcFullHandRank(handtmp);
                    winningPlayer->FHR = fhr;
                    lastRoundWinner = winningPlayer;

                    std::cout << "Guess who won! Player " << winningPlayer->playerID ;
                    //<< " with a " << 
                    //fhr.handrank << " " << fhr.maincards << "| " << fhr.kickers << std::endl;

                }
                else {
                    //std::cout << "Draw, returning bets!" << std::endl;
                    // else, draw
                    for( shared_ptr<Player> P : showdownPlayers ) {
                        P->bankroll += P->move.bet_amount;
                    }
                }
            }

            
            shared_ptr<Player> determineWinner(const vector<shared_ptr<Player>> playersIn ) {
                // From a vector of pointers to player, returns a pointer to the winner
                auto playersInBegin = playersIn.begin();
                auto playersInEnd   = playersIn.end();
                size_t numPlayers = std::distance(playersInBegin, playersInEnd);

                if ( numPlayers == 0) { return nullptr; }   // error

                else if ( numPlayers == 1) {
                        return *playersInBegin;
                }
                else if ( numPlayers >= 2) {
                    // showdown
                    // Form hands from community cards and player cards
                    auto it = playersInBegin;
                    std::vector<std::vector<Card>> handList;
                    std::for_each(playersInBegin, playersInEnd, [&](shared_ptr<Player> p) {
                        std::vector<Card> h (p->hand);  // make a copy of the players hand
                        handList.emplace_back(h);
                    });

                    std::for_each(handList.begin(), handList.end(), [&](std::vector<Card>& h) {
                        auto commCards = table.communityCards;
                        h.insert(h.end(), commCards.begin(), commCards.end() );       // append community cards to the end of the (copied) players hand
                    });
                    
                    std::vector<std::vector<Card>>::iterator winningHand = showdown(handList);
                    shared_ptr<Player> winningPlayer = playersIn[std::distance(handList.begin(), winningHand)];
                    return winningPlayer;
                }
                else {
                    return nullptr;
                }
            }
            
            

            void rotatePlayers() {
                for(auto &P : table.playerList) {
                    P.incPosition();
                }
            }
            void print() {
                for( Player& p : table.playerList ) {
                    std::cout << std::endl;
                    std::cout << "Player " << p.playerID << " at seat " << p.position  << "\t bank: " << p.bankroll << "\t hand: " << p.hand << std::endl;
                }
                std::cout << "Community cards: " << table.communityCards << std::endl;
                std::cout << "Pot: " << table.pot << std::endl;
                std::cout << "Current Bet: " << table.minimumBet << std::endl;
            }

    };
}
    
