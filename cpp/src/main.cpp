#include <iostream>
#include <iomanip>
#include <any>

#include "card.h"
#include "showdown.h"
#include "table.h"
#include "player.h"
#include "game.h"
#include "bench.h"
#include "pybindings.h"

using namespace Poker;
int main() {
    /*
    auto params = std::multimap<std::string, std::vector<std::any>>();
    params.emplace("Matt", std::vector<std::any> {0.19, 0.32, 0.66});
    params.emplace("call", std::vector<std::any>{});
    params.emplace("call", std::vector<std::any>{});
    params.emplace("random", std::vector<std::any>{});

    auto [avgr, stddevr] = monteCarloRounds(1000, params);
    std::cout << avgr << " (" << stddevr << ")" << std::endl;

    auto [avgg, stddevg] = monteCarloGames(1000, params);
    std::cout << avgg << " (" << stddevg << ")" << std::endl;
    */

    benchmarkHandRankCalculator(1000000);


    // Calculates pre-flop ranges
    /*
    vector<Suit> suitlist = { Suit::HEART, Suit::CLUB, Suit::DIAMOND, Suit::SPADE};
    for(int j=0; j<13; j++) 
        std::cout << std::setw(10) << static_cast<Rank>(j) << " ";
    std::cout << std::endl;
    for(int i=0; i<13; i++) {
        std::cout << static_cast<Rank>(i) << " ";
        for(int j=0; j<13; j++) {
            Card c1(static_cast<Rank>(i), Suit::HEART);
            Card c2(static_cast<Rank>(j), Suit::HEART);
            std::vector<Card> myCards = { c1, c2 };
            auto a = monteCarloSingleHandStdDev(myCards, 5, 5, 5000);
            auto WR = a[0];
            auto var = a[1];
            std::cout << std::fixed;
            std::cout << std::setprecision(2) << std::setw(10);
            std::cout << WR << " (" << var << ")";
        }
        std::cout << std::endl;
    }
    */

    //monteCarloGames(10000, {"Matt", "call"});

    return 0;
}