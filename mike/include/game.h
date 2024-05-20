#pragma once
#include<memory>
#include <list>
#include <iostream>
#include <fstream>
#include "table.h"
#include "player.h"
#define PRINT false
using namespace std;

namespace Poker {


    class Game {
        public:
            random_device gameRD = random_device();   // consider removal... this is only used in passing to the Deck
            Table      table;                              // The public members of Table are what are visible to all and will serve as input to the AI
            vector<Player*> activePlayers;                   // The ones playing the game. This is so you can deactivate players if you wanted to. It should not change during a round
            vector<Player*> foldedPlayers;
            vector<Player*> allInPlayers;
            vector<Player*> bettingPlayers;
            shared_ptr<Player> lastRoundWinner;
            int roundCount = 0;

            void setup() {
                // The table must have its blinds and bankrolls set up before calling setup
                table.resetCards(gameRD);
                activePlayers = table.getPlayersInBettingOrder(vector<Player*>());
                allInPlayers.clear();
                foldedPlayers.clear();
                lastRoundWinner = nullptr;
                bettingPlayers.clear();
                copy_if(activePlayers.begin(), activePlayers.end(), back_inserter(bettingPlayers), [] (auto* P) { return P->bankroll > 0; });
            }
            void resetToDefaults(int n = 6) {
                // creates a fresh game of standard n player poker
                const vector<string> aiList (n, "random");
                table.setPlayerList(aiList);
                table.setPlayerBankrolls(100);
                setup();
            }
            Game(Table& t) {
                table = t;
                setup();
            }

            void setNonBRPlayersPositions(int shift = 0) {
                // This function gets the non-bankrupt players from activePlayers and sets bettingPlayers to them
                // Assigns positions to the players in betting order
                // If player positions are invalid, overwrites them with proper ones
                // Also rotates seats if desired by shift
                // Shift = +1 is typical for rotating seats for hold 'em
                bettingPlayers.clear();
                copy_if(activePlayers.begin(), activePlayers.end(), back_inserter(bettingPlayers), [] (auto* P) { return P->bankroll > 0; });
                auto nPlayers = bettingPlayers.size();
                if( nPlayers > 1 ) {
                    std::vector<PlayerPosition> playerPositionList = numPlayersToPositionList(nPlayers);
                    // If players have ill-formed positions, aka repeats, getPlayersInBettingOrder 
                    // MAY return equivalent players in arbitrary order (this is behavior from std::sort)
                    // After calling this function, they will have valid positions, however
                    bettingPlayers = table.getPlayersInBettingOrder(bettingPlayers);
                    for(int i = 0; i < nPlayers; i++ ) {
                        Player* P = bettingPlayers[i];
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
                    doRound();  
                    // rotate player positions
                    setNonBRPlayersPositions(1);
                    nPlayers = bettingPlayers.size();
                    roundCount++;
                }

                #if PRINT
                if ( lastRoundWinner ) {
                    std::cout << "===========================================================================" << std::endl;
                    std::cout << "Game finished: " << *lastRoundWinner << " wins " ;
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
                    lastRoundWinner = make_shared<Player>(*bettingPlayers[0]);
                    return;
                };

                bettingPlayers = table.getPlayersInBettingOrder(bettingPlayers);
                const auto bPlayersBackup = bettingPlayers;


                // transferAmount is used to keep track of what people owe
                map<Player*, int> transferAmount;
                for_each(bettingPlayers.begin(), bettingPlayers.end(), [&transferAmount] (Player* p) { transferAmount[p]=0;});

                table.clearPlayerHands();

                // clear community cards
                table.communityCards.clear();

                // deal two cards to all active players
                table.dealPlayersCards(bettingPlayers);
                table.playerMoveMap.clear();

                table.street = Street::PREFLOP;                      // set to preflop
                table.pot = 0;
                table.minimumBet   = table.bigBlind; 

                // Process blind bets
                auto SB = *find_if(bettingPlayers.begin(), bettingPlayers.end(), [](const Player* a) { return a->getPosition() == PlayerPosition::POS_SB;});
                auto BB = *find_if(bettingPlayers.begin(), bettingPlayers.end(), [](const Player* a) { return a->getPosition() == PlayerPosition::POS_BB;});
                auto blinds = vector<Player*>({SB, BB});

                for(auto& b : blinds) {
                    if( b->bankroll > table.smallBlind ) {
                        table.pot += table.smallBlind;
                        table.playerMoveMap[b].emplace_back(PlayerMove(Move::MOVE_CALL, table.smallBlind));
                        transferAmount[b] = table.smallBlind;
                    } else {
                        // if paying blind bet bankrupts players, force their move
                        bettingPlayers.erase(find(bettingPlayers.begin(), bettingPlayers.end(), b));
                        allInPlayers.emplace_back(b);
                        const auto bankroll = b->bankroll;
                        table.playerMoveMap[b].emplace_back(PlayerMove(Move::MOVE_ALLIN, bankroll));
                        table.pot += bankroll;
                        transferAmount[b] = bankroll;
                    }
                }

                // Begin the actual round
                while( table.street <= Street::RIVER ) {
                    table.dealCommunityCards( table.street );
                    
                    bool keepgoing = true;
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
                            auto P = *i;
                            PlayerMove Pmove =  P->makeMove(make_shared<Table>(table));
                            if ( !table.skipTurn ) {
                                table.playerMoveMap[P].emplace_back(Pmove);
                                const bool allIn = Pmove.move == Move::MOVE_ALLIN;
                                const bool folded = Pmove.move == Move::MOVE_FOLD;
                                
                                // Set the new minimum bet if player raised
                                table.minimumBet = max(Pmove.bet_amount, table.minimumBet);

                                if( folded ) {
                                    //take their money now
                                    P->bankroll -= transferAmount[P];
                                    transferAmount[P] = 0;
                                    foldedPlayers.emplace_back(P);
                                    i = bettingPlayers.erase(i);
                                }
                                else {
                                    // bet_amount should be equal or greater than transferAmount[P]
                                    table.pot += Pmove.bet_amount - transferAmount[P];
                                    transferAmount[P] = Pmove.bet_amount;
                                    if( allIn ) {
                                        allInPlayers.emplace_back(P);
                                        i = bettingPlayers.erase(i);
                                    }
                                    else {
                                        i++;
                                    }
                                }
                                // Do another round the table only if somebody raised
                                if( minimumBetBeforeRound != table.minimumBet ) { keepgoing = true; }
                                #if PRINT
                                printPlayerMove(*P, Pmove);
                                std::cout << "Minimum bet: " << table.minimumBet << std::endl;
                                std::cout << "Current pot: " << table.pot << std::endl;
                                #endif
                            } else {
                                table.skipTurn = false;
                            }
                        }
                    }
                    // advance game
                    table.street++;
                }

                std::vector<Player*> showdownPlayers;
                showdownPlayers.insert(showdownPlayers.end(), bettingPlayers.begin(), bettingPlayers.end());
                showdownPlayers.insert(showdownPlayers.end(), allInPlayers.begin(), allInPlayers.end());

                auto [winningPlayerPtr, winningFHR] = determineWinner(showdownPlayers);

                if( winningPlayerPtr ) {
                    // process bankrolls
                    for(auto p : showdownPlayers) {
                        p->bankroll -= transferAmount[p];
                    };
                    // reward winner
                    winningPlayerPtr->bankroll += table.pot;
                    lastRoundWinner = make_shared<Player>(*winningPlayerPtr);
                    #if PRINT
                    std::cout << *lastRoundWinner << " won with a " << winningFHR << std::endl;
                    #endif
                }
                else {
                    #if PRINT
                    std::cout << "Draw, returning bets!" << std::endl;
                    #endif
                    lastRoundWinner = nullptr;
                }

                allInPlayers.clear();
                foldedPlayers.clear();

            }

            
            std::tuple<Player*, FullHandRank> determineWinner(vector<Player*> playersIn ) {
                // Determines the winner from a vector of Players, as well as the winning hand
                auto playersInBegin = playersIn.begin();
                auto playersInEnd   = playersIn.end();
                size_t numPlayers = std::distance(playersInBegin, playersInEnd);

                if ( numPlayers == 1) {
                    // just compute the guy's FHR and return him
                    auto cards = table.communityCards;
                    cards.insert(cards.end(), playersIn[0]->hand.begin(), playersIn[0]->hand.end());
                    return std::make_tuple(playersIn[0], calcFullHandRank(cards));
                }
                else if ( numPlayers >= 2) {
                    // showdown
                    // Form hands from community cards and player cards
                    auto it = playersInBegin;
                    std::vector<std::vector<Card>> handList;
                    std::for_each(playersInBegin, playersInEnd, [&](auto* p) {
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
                    
                    
                    //for(size_t i = 0; i < FHRs.size(); i++ )
                    //    (playersInBegin + i)->get()->FHR = FHRs[i];

                    Player* winningPlayer = playersIn[std::distance(handList.begin(), winningHandIterator)];
                    return make_tuple(winningPlayer, *handWinnerFHR);
                }
                return std::make_tuple(nullptr, nullFHR);
            }
            
            void print() {
                for( const Player& p : table.playerList ) {
                    std::cout << std::endl;
                    PlayerPosition pp = p.getPosition();
                    int bankroll = p.getBankroll();
                    auto hand = p.getHand();
                    std::cout << "Player " << p.getPlayerID() << " at seat " << p.getPosition()  << "\t bank: " << bankroll << "\t hand: " << hand << std::endl;
                }
                std::cout << "Community cards: " << table.communityCards << std::endl;
                std::cout << "Pot: " << table.pot << std::endl;
                std::cout << "Current Bet: " << table.minimumBet << std::endl;
            }

    };
}
    
