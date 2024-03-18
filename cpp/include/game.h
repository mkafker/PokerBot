#pragma once
#include<memory>
#include <list>
#include <iostream>
#include "table.h"
#include "player.h"
#define PRINT true
using namespace std;

namespace Poker {
    class Game {
        public:
            random_device gameRD = random_device();   // consider removal... this is only used in passing to the Deck
            // The problem with the above is that when Game is copied, a new random_device is created. Maybe not desirable.
            Table      table;                              // The public members of Table are what are visible to all and will serve as input to the AI
            vector<shared_ptr<Player>> activePlayers;                   // The ones playing the game. This is so you can deactivate players if you wanted to. It should not change during a round
            vector<shared_ptr<Player>> foldedPlayers;
            vector<shared_ptr<Player>> allInPlayers;
            vector<shared_ptr<Player>> bettingPlayers;
            shared_ptr<Player> lastRoundWinner;



            Game() { }
            Game(const Table& t) { table = t; }
            void setup() {
                // The table must have its blinds and bankrolls set up before calling setup
                table.resetCards(gameRD);
                activePlayers = table.getPlayersInBettingOrder();
                allInPlayers.clear();
                foldedPlayers.clear();
                lastRoundWinner = nullptr;
                bettingPlayers.clear();
                copy_if(activePlayers.begin(), activePlayers.end(), back_inserter(bettingPlayers), [] (shared_ptr<Player> P) { return P->bankroll > 0; });
            }
            void resetToDefaults(int n = 6) {
                // creates a fresh game of standard n player poker
                const vector<string> aiList (n, "random");
                table.setPlayerList(aiList);
                table.resetCards(gameRD);
                activePlayers = table.getPlayersInBettingOrder();
                allInPlayers.clear();
                foldedPlayers.clear();
                lastRoundWinner = nullptr;
                table.setPlayerBankrolls(100);
                bettingPlayers = activePlayers;
            }

            void setNonBRPlayersPositions(int shift = 0) {
                // This function gets the non-bankrupt players from activePlayers and sets bettingPlayers to them
                // Assigns positions to the players in betting order
                // If player positions are invalid, overwrites them with proper ones
                // Also rotates seats if desired by shift
                // Shift = +1 is typical for rotating seats for hold 'em
                bettingPlayers.clear();
                copy_if(activePlayers.begin(), activePlayers.end(), back_inserter(bettingPlayers), [] (shared_ptr<Player> P) { return P->bankroll > 0; });
                auto nPlayers = bettingPlayers.size();
                if( nPlayers > 1 ) {
                    std::vector<PlayerPosition> playerPositionList = numPlayersToPositionList(nPlayers);
                    // If players have ill-formed positions, aka repeats, getPlayersInBettingOrder 
                    // MAY return equivalent players in arbitrary order (this is behavior from std::sort)
                    // After calling this function, they will have valid positions, however
                    bettingPlayers = table.getPlayersInBettingOrder(bettingPlayers);
                    for(int i = 0; i < nPlayers; i++ ) {
                        shared_ptr<Player> P = bettingPlayers[i];
                        P->setPosition(playerPositionList[i]);
                        if( shift != 0 ) {
                            int newPos = static_cast<int>(P->getPosition());
                            newPos = (newPos - shift) % nPlayers;
                            P->setPosition(static_cast<PlayerPosition>(newPos));
                        }
                    }
                }
            }

            void doGame() {
                auto nPlayers = bettingPlayers.size();
                while( nPlayers > 1) {
                    table.resetCards(gameRD);
                    // fill bettingPlayers with non-bankrupt players
                    //bettingPlayers.clear();
                    //copy_if(activePlayers.begin(), activePlayers.end(), back_inserter(bettingPlayers), [] (const shared_ptr<Player>& P) { return P->bankroll > 0;});
                    doRound();  
                    nPlayers = bettingPlayers.size();
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

                // Set everybodies positions and
                // Fills bettingPlayers with non-bankrupt activePlayers
                setNonBRPlayersPositions();
                // If only one player, that's the winner!
                if( bettingPlayers.size() == 1 ) {
                    lastRoundWinner = bettingPlayers[0];
                    return;
                };

                bettingPlayers = table.getPlayersInBettingOrder(bettingPlayers);
                const auto bPlayersBackup = bettingPlayers;

                std::shared_ptr<Player> winningPlayer;
                // bankrolls are updated after the round, transferAmount is used to keep track of what people owe
                unordered_map<shared_ptr<Player>, int> transferAmount;
                for_each(bettingPlayers.begin(), bettingPlayers.end(), [&transferAmount] (const shared_ptr<Player>& p) { transferAmount[p]=0;});

                table.clearPlayerHands();

                // clear community cards
                table.communityCards.clear();

                // deal two cards to all active players
                table.dealPlayersCards(bettingPlayers);

                table.street = 0;                      // set to preflop
                table.pot = 0;
                table.minimumBet   = table.bigBlind; 

                // process blind bets
                auto SB = *find_if(bettingPlayers.begin(), bettingPlayers.end(), [](const shared_ptr<Player>& a) { return a->getPosition() == PlayerPosition::POS_SB;});
                auto BB = *find_if(bettingPlayers.begin(), bettingPlayers.end(), [](const shared_ptr<Player>& a) { return a->getPosition() == PlayerPosition::POS_BB;});

                if( SB->bankroll > table.smallBlind ) {
                    table.pot += table.smallBlind;
                    SB->move.bet_amount = table.smallBlind;
                    SB->move.move = Move::MOVE_CALL;
                    transferAmount[SB] = table.smallBlind;
                } else {
                    // if paying blind bet bankrupts players, force their move
                    bettingPlayers.erase(find(bettingPlayers.begin(), bettingPlayers.end(), SB));
                    allInPlayers.emplace_back(SB);
                    SB->move.bet_amount = SB->bankroll;
                    SB->move.move = Move::MOVE_ALLIN;
                    table.pot += SB->move.bet_amount;
                    transferAmount[SB] = SB->bankroll;
                }
                if( BB->bankroll > table.bigBlind ) {
                    table.pot += table.bigBlind;
                    BB->move.bet_amount = table.bigBlind;
                    BB->move.move = Move::MOVE_CALL;
                    transferAmount[BB] = table.bigBlind;
                } else {
                    // if paying blind bet bankrupts players, force their move
                    bettingPlayers.erase(find(bettingPlayers.begin(), bettingPlayers.end(), BB));
                    allInPlayers.emplace_back(BB);
                    BB->move.bet_amount = BB->bankroll;
                    BB->move.move = Move::MOVE_ALLIN;
                    table.pot += BB->move.bet_amount;
                    transferAmount[BB] = BB->bankroll;
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
                            PlayerMove Pmove =  P->makeMove(make_shared<Table>(table));

                            table.pot += Pmove.bet_amount - transferAmount[P];
                            transferAmount[P] = Pmove.bet_amount;
                            
                            
                            // TODO: Change the entire Move object to be derivable from the bet_amount ?

                            const bool allIn = Pmove.move == Move::MOVE_ALLIN;
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
                                //take their money now
                                P->bankroll -= transferAmount[P];
                                transferAmount[P] = 0;
                                foldedPlayers.emplace_back(P);
                                i = bettingPlayers.erase(i);
                            }
                            else if( allIn ) {
                                allInPlayers.emplace_back(P);
                                i = bettingPlayers.erase(i);
                            }
                            else {
                                i++;
                            }
                            // Do another round the table only if somebody raised
                            if( minimumBetBeforeRound != table.minimumBet ) { keepgoing = true; }
                        }
                    }
                    // advance game
                    table.street++;
                }

                std::vector<shared_ptr<Player>> showdownPlayers;
                showdownPlayers.insert(showdownPlayers.end(), bettingPlayers.cbegin(), bettingPlayers.cend());
                showdownPlayers.insert(showdownPlayers.end(), allInPlayers.cbegin(), allInPlayers.cend());

                winningPlayer = determineWinner(showdownPlayers);
                // process bankrolls
                for_each(showdownPlayers.begin(), showdownPlayers.end(), [&transferAmount] (const shared_ptr<Player>& p) {
                    p->bankroll -= transferAmount[p];
                    transferAmount[p]=0;
                });

                if( winningPlayer ) {
                    // finally, reward player
                    winningPlayer->bankroll += table.pot;
                    lastRoundWinner = winningPlayer;
                    #if PRINT
                    // TODO: fix UNDEF_HAND bug
                    std::cout << *lastRoundWinner << " won with a " << lastRoundWinner->FHR << std::endl;
                    #endif

                }
                else {
                    #if PRINT
                    std::cout << "Draw, returning bets!" << std::endl;
                    #endif
                    // else, draw
                    lastRoundWinner = nullptr;
                    for( shared_ptr<Player> P : showdownPlayers ) {
                        P->bankroll += P->move.bet_amount;
                    }
                }

                allInPlayers.clear();
                foldedPlayers.clear();
                // Fill bettingPlayers with non-bankrupt players
                // also rotate player positions
                setNonBRPlayersPositions(1);

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
    
