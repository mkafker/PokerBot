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
#include "pybindings.h"

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

    std::vector<Card> myCards  = { Card(Rank::C_7, Suit::CLUB), Card(Rank::C_8, Suit::CLUB) };
    std::vector<Card> theirCards  = { Card(Rank::C_5, Suit::DIAMOND), Card(Rank::C_J, Suit::SPADE) };

    //monteCarloHandRankCompare( myCards, theirCards, 0.01 );
    //monteCarloHandRankCompare( myCards, theirCards, (uint64_t)1000000);
    //benchmarkRounds(100000);
    //benchmarkGames(10000);
    //monteCarloGameStateCompare();
    //std::vector<std::string> aiList = {"fhrprop", "rand", "rand", "call", "call"};
    //monteCarloGames(20000, aiList);


    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 9.0f);
    for(int j = 0; j < 100; j++ ) {
        std::cout << "==========================================" << std::endl;
        std::vector<float> inputRBRraw = {1., 1., 1., 1., 1., 1., 1., 1.};
        std::cout << "input = [";
        for(auto& i : inputRBRraw) {
            i = dis(gen);
            std::cout << i << ", ";
        }
        std::cout << "]" << std::endl;

        int N = 1000;
        double result = Poker::pyMonteCarloGames( N,  inputRBRraw);
        std::cout << "result = " << result << std::endl;
    }




    return 0;
}