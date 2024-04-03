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
                                    


    std::vector<Card> theirCards  = { Card(Rank::C_5, Suit::DIAMOND), Card(Rank::C_J, Suit::SPADE) };


    /*
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dis(0,10);
    double highestWR = 0.0;
    std::vector<int> inputRBRraw (117,1);
    auto bestRBR = inputRBRraw;

    for(int j = 0; j < 10; j++ ) {
        std::cout << "==========================================" << std::endl;
        std::cout << "input = [";
        for(auto& i : inputRBRraw) {
            i = dis(gen);
            std::cout << i << ", ";
        }
        std::cout << "]" << std::endl;

        int N = 10000;
        double result = Poker::pyMonteCarloGames( N,  inputRBRraw);
        std::cout << "result = " << result << std::endl;
        if( result > highestWR) {
            highestWR = result;
            bestRBR = inputRBRraw;
        }
    }
    std::cout << "highest WR: " << highestWR << std::endl;
    std::cout << "strategy: ";
    for(auto& f : inputRBRraw) {
        std::cout << f << " ";
    }
    std::cout << std::endl;

*/
    //Poker::pyMonteCarloGames(10);
    std::vector<Card> myCards  = { Card(Rank::C_4, Suit::CLUB), Card(Rank::C_2, Suit::HEART) };
    std::vector<Card> comCards  = { Card(Rank::C_5, Suit::CLUB), Card(Rank::C_6, Suit::CLUB), Card(Rank::C_T, Suit::CLUB) };
    monteCarloSingleHand(myCards, comCards, 1, 50000);

    return 0;
}