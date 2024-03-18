
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





double pyMonteCarloGames(const uint64_t& N, const std::vector<float> inputRBRraw) {
    // input: N games
    // inputRBRraw vectorized rank-bet relationship map
    //if( inputRBRraw.size() != 9 ) throw;
    std::vector<int> inputRBRlessraw ( inputRBRraw.begin(), inputRBRraw.end());
    std::vector<int> nines (9-inputRBRraw.size(), 9);
    inputRBRlessraw.insert(inputRBRlessraw.end(), nines.begin(), nines.end() );
    for(auto& i : inputRBRlessraw) {
      if(i<0) i=0;
      if(i>9) i=9;
    }
    std::map<HandRank, int> inputRBR;
    for(int i=0; i<9; i++) {
        inputRBR[static_cast<HandRank>(i+1)] = inputRBRlessraw[i];
    }
    vector<string> aiList = {"hrbetrel", "call"};
    std::random_device rd;
    auto myTable = Table();
    // populate player list
    myTable.setPlayerList(aiList);
    auto playerZero = myTable.getPlayerByID(0);
    std::dynamic_pointer_cast<HRBetRelAI>(playerZero->strategy)->rankBetRelationship = inputRBR;
    // set blind amounts
    myTable.bigBlind = 10;
    myTable.smallBlind = 5;

    auto game = std::make_shared<Game>(myTable);

    unordered_map<PlayerPosition, int> posWinCount;
    unordered_map<int, int> playerIDWinCount;

    auto start = std::chrono::steady_clock::now();
    int totalRounds = 0;
    int iN = 0;
    int maxRounds = 100000;
    while( iN < N or totalRounds < maxRounds) {
      iN++;
      game->table.setPlayerBankrolls(100);
      game->setup();
      auto nPlayers = game->bettingPlayers.size();
      while( nPlayers > 1) {
          game->table.resetCards(rd);
          game->doRound();  
          totalRounds++;
          nPlayers = game->bettingPlayers.size();
          if( game->lastRoundWinner ) {
            playerIDWinCount[game->lastRoundWinner->getPlayerID()]++;
            posWinCount[game->lastRoundWinner->getPosition()]++;
            //ignore ties
          }
      }
    }

    std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
    std::cout << N << " games (" << totalRounds << " rounds) calculated in " << duration.count() << " seconds, or " 
    << double(N)/duration.count() << " games/s (" << double(totalRounds)/duration.count() << " rounds/s)" << std::endl;
    //std::cout << "Player 0 wins: " << playerIDWinCount[0] << std::endl;
    const double winRate = (double)(playerIDWinCount[0]) / totalRounds * 100.0;
    std::cout << "Win rate: " << winRate << std::endl;
    std::cout << "f = [";
    for(auto& i : inputRBRraw) {
      std::cout << i << ", ";
    }
    std::cout << "]" << std::endl;
    return winRate;

}




#if PYTHON

  PYBIND11_MODULE(poker, m) {
      m.def("MCGames", &pyMonteCarloGames, "Monte Carlo Games",
          pybind11::arg("N"), pybind11::arg("inputRBRraw"));
  }

#endif
}