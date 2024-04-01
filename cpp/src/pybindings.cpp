
#include <memory>
#include "bench.h"
#include "game.h"
#include "table.h"
#include <chrono>
#include <fstream>


#define PYTHON false
#if PYTHON
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#endif


namespace Poker {


void pyMonteCarloGames(const uint64_t& N) {
    // TODO: make this take in a sequence of moves
    // Need to write a valid poker move generator with fixed move sequence length
    vector<string> aiList = {"CFRAI1", "rand"};
    std::random_device rd;
    auto myTable = Table();
    // populate player list
    myTable.setPlayerList(aiList);
    auto playerZero = myTable.getPlayerByID(0);
    //std::dynamic_pointer_cast<FHRAwareAI>(playerZero->strategy)->updateRFHRBetRelationship(params);
    // set blind amounts
    myTable.bigBlind = 10;
    myTable.smallBlind = 5;

    auto game = std::make_shared<Game>(myTable);

    unordered_map<PlayerPosition, int> posWinnings;
    unordered_map<int, int> playerIDWinnings;
    int startingCash = 100;

    ofstream myFile("testoutput.txt", std::ios_base::app);
    auto start = std::chrono::steady_clock::now();
    int iN = 0;
    int nRounds = 0;
    while( iN < N ) {
      iN++;
      myTable.resetCards(rd);
      game->table.setPlayerBankrolls(startingCash);
      game->setup();
      game->moveRecord.clear();
      game->tableRecord.clear();
      game->printMovesToRecord = false;
      auto nPlayers = game->bettingPlayers.size();
      while( nPlayers > 1) {
          game->table.resetCards(rd);
          game->doRound();  
          game->setNonBRPlayersPositions(1);
          nPlayers = game->bettingPlayers.size();
          nRounds++;
      }
      for_each(game->activePlayers.begin(), game->activePlayers.end(), [&posWinnings, &playerIDWinnings] (const shared_ptr<Player>& p) {
        posWinnings[p->getPosition()] += p->bankroll;
        playerIDWinnings[p->getPlayerID()] += p->bankroll;
      } );


      // CSV output section
      // columns: FOR EACH TABLE IN TABLE HISTORY:
      //            pot, FOR EACH PLAYER IN TABLE:
      //                  move, betAmount
      //          FOR EACH PLAYER IN ACTIVEPLAYERS
      //            bankroll
      auto BB = (double)myTable.bigBlind;
      for(size_t i=0; i<game->tableRecord.size(); i++ ) {
        auto curTable = game->tableRecord[i];
        myFile << curTable.pot / BB << ",";
        for(auto& P : game->activePlayers) {
          auto Pmoves = game->moveRecord[P];
          // If player left bettingPlayers during the round, keep printing their last move.
          size_t ind = min(i, Pmoves.size());
          auto Pmove = Pmoves[ind];
          myFile << Pmove.move << "," << Pmove.bet_amount/BB;
        }
      }
      // print player bankrolls
      for(size_t i=0; i<game->activePlayers.size(); i++) {
        auto P = game->activePlayers[i];
        myFile << (double(P->bankroll)-startingCash)/BB;
        if( i != game->activePlayers.size() - 1 ) myFile << ",";
      }
      myFile << std::endl;
    }
    myFile.close();

    std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;


    //std::cout << N << " games (" << totalRounds << " rounds) calculated in " << duration.count() << " seconds, or " 
    //<< double(N)/duration.count() << " games/s (" << double(totalRounds)/duration.count() << " rounds/s)" << std::endl;
    std::cout << N << " games calculated in " << duration.count() << " seconds, or " << double(N)/duration.count() << " games/s" << std::endl;


}


double pyMonteCarloRounds(const uint64_t& N, const std::vector<int> params) {
    // input: N games
    // params vectorized rank-bet-street relationship map, indexed by street first. size() must be divisible by numStreets=4
    // 

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