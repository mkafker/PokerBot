#include <array>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <random>

#include "card.h"
#include "showdown.h"
#include "table.h"
#include "player.h"
#include "game.h"
using namespace Poker;


void benchmarkHandRankCalculator() {
    std::random_device rd;
    std::mt19937_64 g(rd());
    uint64_t N = 1000000;
    auto start = std::chrono::steady_clock::now();
    FullHandRank lastHand;
    int iT = 0;
    for(int iN = 0; iN < N; iN++) {
        iT++;
        Deck newdeck;
        newdeck.shuffle(g);
        std::vector<Card> cards(7);
        for(short j=0; j<7; j++)
            cards[j]=newdeck.pop_card();
        Hand hand (cards);
        FullHandRank FHR = Hand::calcFullHandRank(&hand);
        /*
        if( iT == 1000) {
            iT = 0;
                std::cout << FHR.handrank << " " << FHR.maincards << "| " << FHR.kickers << std::endl;

        }
        */
        if( iN == 0 ) lastHand = FHR;
        else {
            if( Hand::showdownFHR(lastHand, FHR) == FHR  )    { 
                lastHand = FHR;
            }
        }   
    }

    std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
    std::cout << "Best hand: " << std::endl;
    std::cout << lastHand.handrank << " " << lastHand.maincards << "| " << lastHand.kickers << std::endl;
    std::cout << N << " hands calculated in " << duration.count() << " seconds, or " << double(N)/duration.count() << " hands/second" << std::endl;
}

void benchmarkRounds() {
    uint64_t N = 1000;
    auto start = std::chrono::steady_clock::now();
    auto game1 = std::make_shared<Game>();
    std::unique_ptr<Player> bestPlayer = game1->doRound();
    int iT = 0;
    for(int iN = 0; iN < N; iN++) {
        auto game = std::make_shared<Game>();
        std::unique_ptr<Player> winningPlayer = game->doRound();

        auto winFHR = winningPlayer->FHR;
        auto bestFHR = bestPlayer->FHR;
        if( Hand::showdownFHR(bestFHR, winFHR) == winFHR )  { 
            bestPlayer = std::move(winningPlayer);
        }
            
    }

    std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
    std::cout << "Best hand: " << std::endl;
    std::cout << "Player " << bestPlayer->playerID  << " " << PlayerPosition_to_String[bestPlayer->getPosition()] 
              << " " << bestPlayer->FHR.handrank << " " << bestPlayer->FHR.maincards << "| " << bestPlayer->FHR.kickers << std::endl;
    std::cout << N << " rounds calculated in " << duration.count() << " seconds, or " << double(N)/duration.count() << " rounds/second" << std::endl;
}
void benchmarkGames() {

    
    uint64_t N = 1000;
    auto start = std::chrono::steady_clock::now();
    FullHandRank lastHand;
    int iT = 0;
    for(int iN = 0; iN < N; iN++) {
        auto game = std::make_shared<Game>();
         game->doGame();
    }

    std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
    std::cout << N << " games calculated in " << duration.count() << " seconds, or " << double(N)/duration.count() << " games/second" << std::endl;
}

int main() {
    Hand straightflush(std::vector<Card>({  Card(Rank::C_A, Suit::DIAMOND     ), 
                                    Card(Rank::C_K, Suit::HEART  ),
                                    Card(Rank::C_Q, Suit::HEART    ),
                                    Card(Rank::C_J, Suit::HEART    ),
                                    Card(Rank::C_T, Suit::HEART    ),
                                    Card(Rank::C_3, Suit::DIAMOND    ),
                                    Card(Rank::C_6, Suit::CLUB    )
                                    
                                    }));
    Hand fourkind(std::vector<Card>({  Card(Rank::C_2, Suit::HEART     ), 
                                    Card(Rank::C_2, Suit::HEART  ),
                                    Card(Rank::C_2, Suit::DIAMOND    ),
                                    Card(Rank::C_2, Suit::CLUB    ),
                                    Card(Rank::C_T, Suit::HEART    ),
                                    Card(Rank::C_9, Suit::HEART    ),
                                    Card(Rank::C_6, Suit::CLUB    )
                                    
                                    }));

    //FullHandRank fun = Hand::calcFullHandRank(&straightflush);
    //std::cout << fun.handrank << " " << fun.maincards << "| " << fun.kickers << std::endl;
    

    //fun = Hand::calcFullHandRank(&fourkind);
    //std::cout << fun.handrank << " " << fun.maincards << "| " << fun.kickers << std::endl;


    //Game mygame;
    //mygame.doGame();
    benchmarkRounds();

    return 0;
}
