
#include <memory>
#include "bench.h"
#include "game.h"
#include "table.h"
#include <chrono>
#include <fstream>
#include <any>


#define PYTHON false
#if PYTHON
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#endif


namespace Poker {

/*
void pyMonteCarloGames(const uint64_t& N) {
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

std::vector<Card> convertCharlesToMike(std::vector<std::tuple<int,int>> in) {
    //Charles's rank: 2-14 inclusive (A=14, 2=2)
    //          suit: 1='S', 2='H', 3='D', 4='C'
    std::vector<Card> out;
    for(auto& pair : in) {
      Card n;
      n.rank = static_cast<Rank>( get<0>(pair) - 2 );
      const int charlesSuit = get<1>(pair);
      if( charlesSuit == 1) n.suit = Suit::SPADE;
      else if( charlesSuit == 2) n.suit = Suit::HEART;
      else if( charlesSuit == 3) n.suit = Suit::DIAMOND;
      else if( charlesSuit == 4) n.suit = Suit::CLUB;
      out.emplace_back(n);
    }
    return out;
}

std::vector<std::tuple<int, int>> convertMikeToCharles(std::vector<Card> in) {
  std::vector<std::tuple<int, int>> out;
  for(auto& a : in) {
    std::tuple<int, int> card;
    std::get<0>(card) = static_cast<int>( a.get_rank()) + 2;
    const Suit mySuit = a.get_suit();
    if( mySuit == Suit::SPADE ) std::get<1>(card) = 1;
    else if( mySuit == Suit::HEART ) std::get<1>(card) = 2;
    else if( mySuit == Suit::DIAMOND ) std::get<1>(card) = 3;
    else if( mySuit == Suit::CLUB ) std::get<1>(card) = 4;
    out.emplace_back(card);
  }
  return out;
}

int pyShowdownHands(std::vector<std::tuple<int,int>> tupleIntsA, std::vector<std::tuple<int,int>> tupleIntsB, 
                    std::vector<std::tuple<int,int>> communityTupleInts) {
  // returns 0 if A won, 1 if B won, 2 if draw
  vector<Card> cardsA;
  vector<Card> cardsB;
  vector<Card> commCardsA;
  vector<Card> commCardsB;

  cardsA = convertCharlesToMike(tupleIntsA);
  cardsB = convertCharlesToMike(tupleIntsB);
  commCardsA = convertCharlesToMike(communityTupleInts);
  commCardsB = commCardsA;

  // append community cards 
  commCardsA.insert(commCardsA.end(), cardsA.begin(), cardsA.end());
  commCardsB.insert(commCardsB.end(), cardsB.begin(), cardsB.end());

  auto fhrA = calcFullHandRank(commCardsA);
  auto fhrB = calcFullHandRank(commCardsB);
  auto showdownResult = showdownFHR(fhrA, fhrB);
  if(showdownResult == fhrA) return 0;
  else if(showdownResult == fhrB) return 1;
  else return 2;
}

double pyMCSingleHand(const std::vector<std::tuple<int,int>>& cardsA, const std::vector<std::tuple<int,int>>& commCards, const int numOtherPlayers, const uint64_t N) {
  auto [avg, sigma] = monteCarloSingleHand(convertCharlesToMike(cardsA), convertCharlesToMike(commCards), numOtherPlayers, N);
  return avg;
}

double pyMonteCarloRounds(const uint64_t& N, std::vector<float> mikeParams) {
    auto AIList = std::multimap<std::string, std::vector<float>>();
    std::vector<float> mikeParamsAny;
    for( const auto& v : mikeParams)
      mikeParamsAny.emplace_back(v);
    AIList.emplace("Mike", mikeParamsAny);
    AIList.emplace("call", std::vector<float>{});
    //params.emplace("call", std::vector<float>{});
    //params.emplace("random", std::vector<float>{});
    auto [avg, stddev] = monteCarloRounds(N, AIList);
    return avg;
}

float pyTrainKillBot(const std::vector<float> mikeParams) {
    auto aiInfo = std::multimap<std::string, std::vector<float>>();
    aiInfo.emplace("KillBot", mikeParams);
    aiInfo.emplace("call", std::vector<float>{});
    //aiInfo.emplace("call", std::vector<float>{});
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

    const int N = 500;
    const int superN = 15;
    size_t printInterval = 1000;
    // setup some statistics
    vector<int> pZeroWinnings (N*superN);

    auto start = std::chrono::steady_clock::now();
    int iT = 0;
    int totalRounds = 0;
    const int startingCash = 100;

    auto pZero = game->table.getPlayerByID(0);
    float avgLR = 0.0f;
    for(int jN = 0; jN < superN; jN++) {
      //auto pZeroStrat = dynamic_pointer_cast<KillBot>(pZero->strategy);
      //pZeroStrat->reset();
      float firstSet, lastSet = 0.0f;
      for(int iN = 0; iN < N; iN++) {
          int bigInd = N*jN + iN;
          game->table.setPlayerBankrolls(startingCash);
          game->setup();
          game->doRound();
          //pZero->strategy->callback(make_shared<Table>(game->table), pZero);
          pZeroWinnings[bigInd] = (game->table.getPlayerByID(0)->bankroll - startingCash)/myTable.bigBlind; // dimensionless winnings
          float avg = 0.0;
          if( iT == printInterval) {
            avg = std::accumulate(pZeroWinnings.begin()+bigInd-printInterval, pZeroWinnings.begin()+bigInd, 0.0f);
            avg /= printInterval;
            if( iT == printInterval ) {
              //std::cout << "" << avg << std::endl;
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
    }
    // More statistics
    std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
    //std::cout << double(N*superN)/duration.count() << " rounds/s" << std::endl;
    float avg = std::accumulate(pZeroWinnings.begin(), pZeroWinnings.end(), 0.0f);
    avg /= (superN*N);
    std::cout << "=================" << std::endl;
    std::cout << "Parameters: ";
    for(auto&p : mikeParams)
      std::cout << p << " ";
    std::cout << std::endl;
    std::cout << "Average win rate       = " << avg << std::endl;

    return avg;
}


*/
#if PYTHON
  PYBIND11_MODULE(poker, m) {
      m.def("showdownHands", &pyShowdownHands, "showdown hands",
          pybind11::arg("cardsA"), pybind11::arg("cardsB"), pybind11::arg("commCards"));
      m.def("MCSingleHand", &pyMCSingleHand, "monte carlo single hand",
          pybind11::arg("cards"), pybind11::arg("commCards"), pybind11::arg("numOtherPlayers"), pybind11::arg("N"));
      m.def("MCMikeRounds", &pyMonteCarloRounds, "monte carlo rounds (mike)",
          pybind11::arg("N"), pybind11::arg("mikeParams"));
      m.def("MCKillBotRounds", &pyTrainKillBot, "monte carlo rounds (killbot)",
          pybind11::arg("mikeParams"));
  }

#endif

}