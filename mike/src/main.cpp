#include <iostream>
#include <iomanip>
#include <any>
#include <chrono>
#include <string>

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
    constexpr int N = 2000;
    constexpr int writeevery = 1000;
    constexpr int nThreads = 10;
    
    for(int nCards=3; nCards<=5; nCards++) {
      for(int i=0; i<N/writeevery; i++) {
        auto start = std::chrono::steady_clock::now();
        for(int nOpp=1;nOpp<=5;nOpp++){
          std::string out = "MCdata" + std::to_string(nCards) + "cards.csv";
          monteCarloRandomHand(nCards, nOpp, writeevery, 1000, out, nThreads);
        }
        std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
        std::cout << 5*writeevery << " hands calculated in " << duration.count() << " seconds, or " 
        << double(5*writeevery)/duration.count() << " configurations/s" << std::endl;
      }
    }
    */

    auto aiInfo = std::multimap<std::string, std::vector<float>>();
    //aiInfo.emplace("CFRAI1", std::vector<float>{});
    //aiInfo.emplace("Matt", std::vector<float> {0.44, 0.46, 0.54});
    //aiInfo.emplace("Mike", std::vector<float> 
//{-1.0964357164653398, 0.6858257039331285, -0.6811199899767748, -0.9463686817487581, -0.17352864790969913, -1.0813763189426324});
    aiInfo.emplace("KillBot", std::vector<float>{});
    aiInfo.emplace("call", std::vector<float>{});
    //aiInfo.emplace("Matt", std::vector<float> {0.44, 0.46, 0.54});
    // Create Table and fill AI List
    auto myTable = Table();
    // unpack AI list
    std::vector<string> aiStrings;
    std::vector<std::vector<float>> aiParams;
    for(auto& pair : aiInfo) {
      aiStrings.emplace_back(std::get<0>(pair));
      aiParams.emplace_back(std::get<1>(pair));
    }
    
    myTable.setPlayerList(aiStrings);
    for( int i=0; i<aiStrings.size(); i++ ) {
      // Table is filled according to same order as aiStrings
      auto p = myTable.getPlayerByID(i);
      p->strategy->updateParameters(aiParams[i]);
    }
    std::random_device rd;
    // set blind amounts
    myTable.bigBlind = 10;
    myTable.smallBlind = 5;

    auto game = std::make_shared<Game>(myTable);

    const int N = 10000;
    const int superN = 1;
    size_t printInterval = 200;
    // setup some statistics
    vector<int> pZeroWinnings (N*superN);

    auto start = std::chrono::steady_clock::now();
    int totalRounds = 0;
    const int startingCash = 100;
    vector<float> batchWins;
    auto pZero = game->table.getPlayerByID(0);
    float avgLR = 0.0f;
    for(int jN = 0; jN < superN; jN++) {
      auto pZeroStrat = dynamic_pointer_cast<KillBot>(pZero->strategy);
      pZeroStrat->reset();
      float firstSet, lastSet = 0.0f;
      int iT = 0;
      for(int iN = 0; iN < N; iN++) {
          int bigInd = N*jN + iN;
          game->table.setPlayerBankrolls(startingCash);
          game->setup();
          game->doRound();
          pZero->strategy->callback(make_shared<Table>(game->table), pZero);
          pZeroWinnings[bigInd] = (game->table.getPlayerByID(0)->bankroll - startingCash)/myTable.bigBlind; // dimensionless winnings
          float avg = 0.0;
          if( iT == printInterval) {
            avg = std::accumulate(pZeroWinnings.begin()+bigInd-printInterval, pZeroWinnings.begin()+bigInd, 0.0f);
            avg /= printInterval;
            if( iT == printInterval ) {
              std::cout << bigInd << ": " << avg << std::endl;
              iT = 0;
            }
            if(iN == iN)
              firstSet += avg;
            else 
              lastSet = avg;
          }
          iT++;
      }
      float avg = std::accumulate(pZeroWinnings.begin()+jN*N, pZeroWinnings.begin()+(jN+1)*N, 0.0f);
      avg /= N;
      float tmp = (lastSet - firstSet)/N;
      avgLR += tmp;
      //std::cout << "avg learning rate: " << tmp << std::endl;
      std::cout << "batch win rate: " << avg << std::endl;
      batchWins.emplace_back(avg);
    }
    // More statistics
    std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
    std::cout << double(N*superN)/duration.count() << " rounds/s" << std::endl;
    float avg = std::accumulate(pZeroWinnings.begin(), pZeroWinnings.end(), 0.0f);
    avg /= (superN*N);
    std::cout << avg << std::endl;

    std::cout << "Average win rate       = " << avg << std::endl;
    std::cout << "Win rates: ";
    for(auto&  p : batchWins)
      std::cout << p << " ";
    std::cout << std::endl;
    //std::cout << "Average learning rate: = " << avgLR << std::endl;

    


    //Print policy

    auto pZeroStrat = dynamic_pointer_cast<KillBot>(pZero->strategy);
    for(const auto& p : pZeroStrat->policy) {
      Move tmp = p.first.enyMove;
      std::cout << "Bin: " << " hand " << p.first.handStrength << " oppMove " << tmp << " (" << pZeroStrat->hitCount[p.first] << ")" << std::endl;
      for(auto& p1 : p.second) {
        Move m = p1.first;
        std::cout << m << " " << p1.second << " ";
      }
      std::cout << std::endl;
    }


    /*
    // Calculates pre-flop ranges
    vector<float> avgs;
    vector<Suit> suitlist = { Suit::HEART, Suit::CLUB, Suit::DIAMOND, Suit::SPADE};
    for(int j=0; j<13; j++) 
        std::cout << std::setw(10) << static_cast<Rank>(j) << " ";
    std::cout << std::endl;
    for(int i=0; i<13; i++) {
        std::cout << static_cast<Rank>(i) << " ";
        for(int j=0; j<13; j++) {
            float miniavg = 0.0f;
            float minisigma = 0.0f;
            for(auto& suit1 : suitlist) {
              for(auto &suit2 : suitlist) {
                Card c1(static_cast<Rank>(i), suit1);
                Card c2(static_cast<Rank>(j), suit2);
                std::vector<Card> myCards = { c1, c2 };
                auto [avg, sigma] = monteCarloSingleHand(myCards, 3, 1, 500);
                miniavg += avg;
                minisigma += sigma*sigma;
                avgs.push_back(avg);
              }
            }
            miniavg /= 16.0f;
            minisigma = sqrt(minisigma);
            minisigma /= 16.0f;
            std::cout << std::fixed;
            std::cout << std::setprecision(2) << std::setw(10);
            std::cout << miniavg << " (" << minisigma << ")";
        }
        std::cout << std::endl;
    }
    float median;
    std::sort(avgs.begin(), avgs.end());
    median = avgs[avgs.size()/2];
    float bigavg = std::accumulate(avgs.begin(), avgs.end(), 0.0f);
    bigavg /= avgs.size();
    std::cout << "AVERAGE WINRATE: " << bigavg << std::endl;
    std::cout << "MEDIAN WINRATE: " << median << std::endl;

    */


    return 0;
}
