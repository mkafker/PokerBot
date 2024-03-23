
#include <memory>
#include "bench.h"
#include "game.h"
#include "table.h"
#include <chrono>


#define PYTHON false
#if PYTHON
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#endif


namespace Poker {





double pyMonteCarloRounds(const uint64_t& N, const std::vector<int> params) {
    // input: N games
    // params vectorized rank-bet-street relationship map, indexed by street first. size() must be divisible by numStreets=4
    // 
    /*
    const int numStreets = 4;
    const int ldHRmap = params.size() / numStreets;
    std::map<int, std::map<HandRank, int>> strategyRel;
    for(int i=0; i<numStreets; i++) {
      const size_t id = ldHRmap*i;
      auto inputIt = params.begin()+id;
      std::vector<int> inputRBRlessraw ( inputIt, inputIt+ldHRmap);
      std::vector<int> nines (9-inputRBRlessraw.size(), 9);
      inputRBRlessraw.insert(inputRBRlessraw.end(), nines.begin(), nines.end() );
      for(auto& i : inputRBRlessraw) {
        if(i<0) i=0;
        if(i>9) i=9;
      }
      std::map<HandRank, int> inputRBR;
      for(int i=0; i<9; i++) {
          inputRBR[static_cast<HandRank>(i+1)] = inputRBRlessraw[i];
      }
      strategyRel[i]=inputRBR;
    }*/

    // now begin games part
    vector<string> aiList = {"fhraware", "call"};
    std::random_device rd;
    auto myTable = Table();
    // populate player list
    myTable.setPlayerList(aiList);
    auto playerZero = myTable.getPlayerByID(0);
    //std::dynamic_pointer_cast<HandStreetAwareAI>(playerZero->strategy)->streetRBR = strategyRel;
    std::dynamic_pointer_cast<FHRAwareAI>(playerZero->strategy)->updateRFHRBetRelationship(params);
    // set blind amounts
    myTable.bigBlind = 10;
    myTable.smallBlind = 5;

    auto game = std::make_shared<Game>(myTable);

    unordered_map<PlayerPosition, int> posWinnings;
    unordered_map<int, int> playerIDWinnings;
    int startingCash = 100;

    auto start = std::chrono::steady_clock::now();
    int iN = 0;
    while( iN < N ) {
      game->table.setPlayerBankrolls(startingCash);
      game->setup();
      game->table.resetCards(rd);
      game->doRound();  
      iN++;
      for_each(game->activePlayers.begin(), game->activePlayers.end(), [&posWinnings, &playerIDWinnings] (const shared_ptr<Player>& p) {
        posWinnings[p->getPosition()] += p->bankroll;
        playerIDWinnings[p->getPlayerID()] += p->bankroll;
      } );
    }

    std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
    //std::cout << N << " games (" << totalRounds << " rounds) calculated in " << duration.count() << " seconds, or " 
    //<< double(N)/duration.count() << " games/s (" << double(totalRounds)/duration.count() << " rounds/s)" << std::endl;
    std::cout << N << " rounds calculated in " << duration.count() << " seconds, or " << double(N)/duration.count() << " rounds/s" << std::endl;
    //std::cout << "Player 0 wins: " << playerIDWinCount[0] << std::endl;
    // win count
    const double NTimesStartingCash = double(N)*double(startingCash);
    const double avgReturn = (double(playerIDWinnings[0]) - NTimesStartingCash)/NTimesStartingCash * 100.0;
    std::cout << "Return: " << avgReturn << "%" <<  std::endl;
    std::cout << "f = [";
    for(auto& i : params) {
      std::cout << i << ", ";
    }
    std::cout << "]" << std::endl;
    return avgReturn;

}




#if PYTHON

  PYBIND11_MODULE(poker, m) {
      m.def("MCGames", &pyMonteCarloRounds, "Monte Carlo Rounds",
          pybind11::arg("N"), pybind11::arg("inputRBRraw"));
  }

#endif
}