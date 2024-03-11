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
#include "bench.h"
using namespace Poker;
int main() {
    std::vector<Card> straightflush = {  Card(Rank::C_A, Suit::DIAMOND     ), 
                                    Card(Rank::C_K, Suit::HEART  ),
                                    Card(Rank::C_Q, Suit::HEART    ),
                                    Card(Rank::C_J, Suit::HEART    ),
                                    Card(Rank::C_T, Suit::HEART    ),
                                    Card(Rank::C_3, Suit::DIAMOND    ),
                                    Card(Rank::C_6, Suit::CLUB    ) };
                                    
    std::vector<Card> fourkind = {  Card(Rank::C_2, Suit::HEART     ), 
                                    Card(Rank::C_2, Suit::HEART  ),
                                    Card(Rank::C_2, Suit::DIAMOND    ),
                                    Card(Rank::C_2, Suit::CLUB    ),
                                    Card(Rank::C_T, Suit::HEART    ),
                                    Card(Rank::C_9, Suit::HEART    ),
                                    Card(Rank::C_6, Suit::CLUB    ) };
                                    

    //FullHandRank fun = Hand::calcFullHandRank(&straightflush);
    //std::cout << fun.handrank << " " << fun.maincards << "| " << fun.kickers << std::endl;
    //Game mygame;
    //mygame.doGame();
    //benchmarkRounds();
    //
    std::vector<Card> myCards  = { Card(Rank::C_A, Suit::CLUB), Card(Rank::C_K, Suit::CLUB) };
    std::vector<Card> theirCards  = { Card(Rank::C_A, Suit::DIAMOND), Card(Rank::C_K, Suit::SPADE) };
    //monteCarloHandRankCompare( myCards, theirCards, 0.01 );
    monteCarloHandRankCompare( myCards, theirCards, (uint64_t)1000000);
    benchmarkRounds();

    return 0;
}
