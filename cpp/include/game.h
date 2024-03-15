#pragma once
#include<memory>
#include <list>
#include <iostream>
#include "table.h"
#include "player.h"
#define PRINT false
using namespace std;

namespace Poker {
    class Game {
        public:
            random_device gameRD = random_device();
            Table      table;                              // The public members of Table are what are visible to all and will serve as input to the AI
            vector<shared_ptr<Player>> activePlayers;                   // The ones playing the game. This is so you can deactivate players if you wanted to. It should not change during a round
            vector<shared_ptr<Player>> foldedPlayers;
            vector<shared_ptr<Player>> allInPlayers;
            vector<shared_ptr<Player>> bettingPlayers;
            shared_ptr<Player> lastRoundWinner;

            // TODO: Make a new data structure to hold AI strategies
            // Have it be an input to a copy constructor for the Game
            // This copy must be able to be made mid-round or at any stage of the game


            // Betting players = activePlayers - foldedPlayers - allInPlayers
            // This could be done in an inline function or something?
            Game () {
                reset();
            }
            void reset(int n = 6) {
                // generate new RNG and fresh deck (unshuffled)
                table.resetDeck(gameRD);
                // shuffle cards
                table.shuffleDeck();
                // clear community cards
                table.communityCards.clear();
                // create players
                std::vector<PlayerPosition> playerPositionList = numPlayersToPositionList(n);
                table.playerList.clear();
                for(int i = 0; i < n; i++ ) {
                    RandomMoveAI p;
                    p.setPosition(playerPositionList[i]);
                    p.bankroll = 100;
                    p.playerID = i;
                    // FUN FACT: emplacing back derived classes ACTUALLy emplaces back the base class! Very cool!
                    table.playerList.emplace_back(make_shared<RandomMoveAI>(p));
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
                    table.resetDeck(gameRD);
                    table.shuffleDeck();
                    std::vector<PlayerPosition> playerPositionList = numPlayersToPositionList(bettingPlayers.size());
                    for(int i = 0; i < bettingPlayers.size(); i++ ) {
                        bettingPlayers[i]->setPosition(playerPositionList[i]);
                    }

                    doRound();

                    // rotate players to the next seat
                }

                #if PRINT
                if ( lastRoundWinner ) {
                    std::cout << "===========================================================================" << std::endl;
                    std::cout << "Game finished: " << *lastRoundWinner << " wins " << lastRoundWinner->FHR <<  std::endl;
                    std::cout << "===========================================================================" << std::endl;
                } else {
                    std::cout << "===========================================================================" << std::endl;
                    std::cout << "Game finished: Draw!" << std::endl;
                    std::cout << "===========================================================================" << std::endl;
                }
                #endif
            }

            void doRound() {
                // Goes around the table taking bets until every player is all-in, folded, or called, then does a showdown
                // sets lastRoundWinner to a shared_ptr to the winning player
                // Modifies bettingPlayers and lastRoundWinner

                // If only one player, that's the winner!
                if( bettingPlayers.size() == 1 ) {
                    lastRoundWinner = bettingPlayers[0];
                    return;
                };
                allInPlayers.clear();
                foldedPlayers.clear();
                bettingPlayers = table.getPlayersInOrder(bettingPlayers);

                std::shared_ptr<Player> winningPlayer;
                table.resetPlayerHands();

                // clear community cards
                table.communityCards.clear();

                // deal two cards to all active players
                table.dealPlayersCards(bettingPlayers);

                table.street = 0;                      // set to preflop
                table.minimumBet   = table.bigBlind; 
                table.pot = 0;
                // process blind bets
                auto SB = *find_if(bettingPlayers.begin(), bettingPlayers.end(), [](const shared_ptr<Player>& a) { return a->getPosition() == PlayerPosition::POS_SB;});
                auto BB = *find_if(bettingPlayers.begin(), bettingPlayers.end(), [](const shared_ptr<Player>& a) { return a->getPosition() == PlayerPosition::POS_BB;});

                if( SB->bankroll > table.smallBlind ) {
                    SB->bankroll -= table.smallBlind;
                    table.pot += table.smallBlind;
                } else {
                    // if paying blind bet bankrupts players, force their move
                    bettingPlayers.erase(find(bettingPlayers.begin(), bettingPlayers.end(), SB));
                    allInPlayers.emplace_back(SB);
                    SB->move.bet_amount = SB->bankroll;
                    SB->move.move = Move::MOVE_ALLIN;
                }
                if( BB->bankroll > table.bigBlind ) {
                    BB->bankroll -= table.bigBlind;
                    table.pot += table.bigBlind;
                } else {
                    // if paying blind bet bankrupts players, force their move
                    bettingPlayers.erase(find(bettingPlayers.begin(), bettingPlayers.end(), BB));
                    allInPlayers.emplace_back(BB);
                    BB->move.bet_amount = BB->bankroll;
                    BB->move.move = Move::MOVE_ALLIN;
                }
                // TODO: clean this shit up!!! ^^^^^^
                while( table.street < 4) {
                     
                    table.dealCommunityCards( table.street );
                    
                    bool keepgoing = true;                                                            // indicates that we need another round of betting
                    #if PRINT
                    std::cout << "================================================" << std::endl;
                    std::cout << "Phase " << table.street << " " << table.communityCards <<  std::endl;
                    #endif


                    while(keepgoing) {
                        #if PRINT
                        std::cout << "---------------------------------------" << std::endl;
                        #endif

                        keepgoing = false;
                        const int minimumBetBeforeRound = table.minimumBet;
                        
                        auto i = bettingPlayers.begin();

                        while (i != bettingPlayers.end()) {
                            shared_ptr<Player> P = *i;
                            PlayerMove Pmove;
                            Pmove =  P->makeMove(make_shared<Table>(table)) ;
                            table.pot  += Pmove.bet_amount;
                            P->bankroll -= Pmove.bet_amount;
                            P->move = Pmove;
                            // TODO: Change the entire Move object to be derivable from the bet_amount

                            const bool allIn = Pmove.move == Move::MOVE_ALLIN;
                            const bool allInOrRaise = Pmove.move == Move::MOVE_RAISE || allIn;
                            const bool folded = Pmove.move == Move::MOVE_FOLD;
                            // Set the new minimum bet if player raised
                            table.minimumBet = max(Pmove.bet_amount, table.minimumBet);


                            #if PRINT
                            printPlayerMove(*P, Pmove);
                            std::cout << "Minimum bet: " << table.minimumBet << std::endl;
                            std::cout << "Current pot: " << table.pot << std::endl;
                            #endif


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
                    lastRoundWinner = winningPlayer;
                    #if PRINT
                    std::cout << *lastRoundWinner << " won with a " << lastRoundWinner->FHR << std::endl;
                    #endif

                }
                else {
                    #if PRINT
                    std::cout << "Draw, returning bets!" << std::endl;
                    #endif
                    // else, draw
                    for( shared_ptr<Player> P : showdownPlayers ) {
                        P->bankroll += P->move.bet_amount;
                    }
                }
            }

            
            shared_ptr<Player> determineWinner(vector<shared_ptr<Player>>& playersIn ) {
                // From a vector of pointers to player, returns a pointer to the winner
                // Modifies the player by populating their FullHandRank
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
                    

                    
                    // calculate full hand rank for each hand
                    std::vector<FullHandRank> FHRs;
                    std::transform(handList.cbegin(), handList.cend(), std::back_inserter(FHRs), [](std::vector<Card> h) {  return calcFullHandRank(h); });
                    
                    auto FHRComparator = [](const FullHandRank& a, const FullHandRank& b) -> bool {
                        return showdownFHR(a, b) == b;
                    };
                    auto winningHandIterator = handList.begin();
                    // caution... max_element returns only the first max value
                    std::vector<FullHandRank>::iterator handWinnerFHR = std::max_element(FHRs.begin(), FHRs.end(), FHRComparator);        
                    advance(winningHandIterator, std::distance(FHRs.begin(), handWinnerFHR));
                    
                    
                    for(size_t i = 0; i < FHRs.size(); i++ )
                        (playersInBegin + i)->get()->FHR = FHRs[i];

                    shared_ptr<Player> winningPlayer = playersIn[std::distance(handList.begin(), winningHandIterator)];
                    return winningPlayer;
                }
                else {
                    return nullptr;
                }
            }
            
            

            void rotatePlayers() {
                for(auto &P : table.playerList) {
                    P->incPosition();
                }
            }
            void print() {
                for( shared_ptr<Player> p : table.playerList ) {
                    std::cout << std::endl;
                    std::cout << "Player " << p->playerID << " at seat " << p->position  << "\t bank: " << p->bankroll << "\t hand: " << p->hand << std::endl;
                }
                std::cout << "Community cards: " << table.communityCards << std::endl;
                std::cout << "Pot: " << table.pot << std::endl;
                std::cout << "Current Bet: " << table.minimumBet << std::endl;
            }

    };
}
    
