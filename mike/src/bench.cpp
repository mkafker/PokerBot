#include "bench.h"
#include "showdown.h"
#include "table.h"
#include "player.h"
#include "game.h"
#include "strategy.h"
#include "pybindings.h"
#include <chrono>
#include <any>
#include <fstream>
#include <sstream>

namespace Poker {
  void benchmarkHandRankCalculator(const uint64_t& N) {
      auto start = std::chrono::steady_clock::now();
      FullHandRank lastHand;
      for(int iN = 0; iN < N; iN++) {
          Deck newdeck;
          newdeck.shuffle();          
          std::vector<Card> cards(7);
          for(short j=0; j<7; j++)
              cards[j]=newdeck.pop_card();
          FullHandRank FHR = calcFullHandRank(cards);
          if( iN == 0 ) lastHand = FHR;
          else {
              if( showdownFHR(lastHand, FHR) == FHR  )    { 
                  lastHand = FHR;
              }
          }   
      }
      std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
      std::cout << "Best hand: " << std::endl;
      std::cout << lastHand.handrank << " " << lastHand.maincards << "| " << lastHand.kickers << std::endl;
      std::cout << N << " hands calculated in " << duration.count() << " seconds, or " << double(N)/duration.count() << " hands/second" << std::endl;
  }

  void benchmarkRounds(uint64_t N) {
    auto params = std::multimap<std::string, std::vector<float>>();
    params.emplace("call", std::vector<float>{});
    params.emplace("call", std::vector<float>{});
    params.emplace("call", std::vector<float>{});
    params.emplace("random", std::vector<float>{});
    params.emplace("random", std::vector<float>{});
    params.emplace("random", std::vector<float>{});
    auto start = std::chrono::steady_clock::now();
    monteCarloRounds(N, params);
    std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
    //std::cout << "Best hand: " << std::endl;
    //std::cout << "Player " << bestPlayer->playerID  << " " << PlayerPosition_to_String[bestPlayer->getPosition()]
    //        << " " << bestPlayer->FHR.handrank << " " << bestPlayer->FHR.maincards << "| " << bestPlayer->FHR.kickers << std::endl;
    std::cout << N << " rounds calculated in " << duration.count() << " seconds, or " << double(N)/duration.count() << " rounds/second" << std::endl;
  }

void monteCarloGameStateCompare() {
    int iT = 0;
    auto myTable = Table();
    // populate player list
    vector<string> aiList = {"call", "random", "random", "random" };
    myTable.setPlayerList(aiList);
    myTable.setPlayerBankrolls(100);
    myTable.bigBlind = 10;
    myTable.smallBlind = 5;
    auto game = make_shared<Game>(myTable);
    // instantiate game in a specific state
    game->setup();
    // do a round to make it interesting
    game->doRound();
    // convert player 0 to a SequenceMoveAI that always goes all-in
    auto playerList = myTable.playerList;
    auto playerOneIt = find_if(playerList.begin(), playerList.end(), [] (const shared_ptr<Player>& p) {return p->playerID == 0; });
    if ( playerOneIt != playerList.end()) {
        shared_ptr<Player> playerOne = *playerOneIt;

        auto newStrat = SequenceMoveAI();
        newStrat.moveList.emplace_back(PlayerMove { Move::MOVE_ALLIN, 0});
        playerOne->strategy = make_unique<SequenceMoveAI>(newStrat);
        // player 0 should do nothing but all-in now
    }
    game->doRound();
}

std::tuple<double, double> monteCarloRounds(const uint64_t& N, const std::multimap<std::string, std::vector<float>>& aiInfo) {
    // input: N games, vector of tuples ( Ai type string, ai parameters )
    // Returns (avg win rate, variance of win rate) of player 0
    // Create Table and fill AI List
    auto myTable = Table();
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

    // now begin games part
    std::random_device rd;
    // set blind amounts
    myTable.bigBlind = 10;
    myTable.smallBlind = 5;

    auto game = std::make_shared<Game>(myTable);
    vector<int> pZeroWinnings (N);
    int startingCash = 100;

    int iN = 0;
    while( iN < N ) {
      // Resets game, does a single round, tallies winnings
      // Does not change player positions
      game->table.setPlayerBankrolls(startingCash);
      game->setup();
      game->table.resetCards(rd);
      game->doRound();
      iN++;
      pZeroWinnings[iN] = (game->table.getPlayerByID(0)->bankroll - startingCash)/myTable.bigBlind; // dimensionless winnings
    }


    double avgReturn = 0.0;
    for( const auto& w : pZeroWinnings )
        avgReturn += w;
    avgReturn /= N;
    double sigmaReturn = 0.0;
    for( const auto& w : pZeroWinnings ) {
        const auto tmp = double(w) - avgReturn;
        sigmaReturn += tmp*tmp;
    }
    sigmaReturn /= N;
    sigmaReturn = sqrt(sigmaReturn);
    return std::make_tuple(avgReturn, sigmaReturn);
}


std::tuple<double, double> monteCarloGames(const uint64_t& N, const std::multimap<std::string, std::vector<float>>& aiInfo) {
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

    // setup some statistics
    vector<int> pZeroWinnings (N);

    auto start = std::chrono::steady_clock::now();
    int iT = 0;
    int totalRounds = 0;
    const int startingCash = 100;
    for(int iN = 0; iN < N; iN++) {
        game->table.setPlayerBankrolls(startingCash);
        game->setup();
        game->doGame();
        //pZeroWinnings[iN] = (game->table.getPlayerByID(0)->bankroll - startingCash)/myTable.bigBlind; // dimensionless winnings
        pZeroWinnings[iN] = game->lastRoundWinner == game->table.getPlayerByID(0) ? 1 : 0;
        totalRounds+=game->nRounds;
    }

    std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
    //std::cout << N << " games (" << totalRounds << " rounds) calculated in " << duration.count() << " seconds, or " 
    //<< double(N)/duration.count() << " games/s (" << double(totalRounds)/duration.count() << " rounds/s)" << std::endl;

    double avgReturn = 0.0;
    for( const auto& w : pZeroWinnings )
        avgReturn += w;
    avgReturn /= N;
    double sigmaReturn = 0.0;
    for( const auto& w : pZeroWinnings ) {
        const auto tmp = double(w) - avgReturn;
        sigmaReturn += tmp*tmp;
    }
    sigmaReturn /= N;
    sigmaReturn = sqrt(sigmaReturn);
    return std::make_tuple(avgReturn, sigmaReturn);
}


std::tuple<double,double> monteCarloSingleHand(const std::vector<Card>& cardsA, const int numCommCards, const int numOtherPlayers, const uint64_t N) {
    std::random_device rd;
    std::vector<short> outcomes(N, 0);

    uint64_t winCountA = 0;
    std::vector<std::vector<Card>> otherPlayerCards(numOtherPlayers);
    
    auto start = std::chrono::steady_clock::now();
    winCountA = 0;
    for(int iN = 0; iN < N; iN++) {
        std::mt19937_64 g(rd());
        std::vector<Card> communityCards(numCommCards);
        std::vector<Card> cardsAMutable(cardsA);
        //make new deck without players' cards
        Deck newdeck(cardsA);
        newdeck.shuffle();
        // deal community cards
        for(short j=0; j<numCommCards; j++)
            communityCards[j]=newdeck.pop_card();
        // deal other player's cards
        for( auto& v : otherPlayerCards ) {
            v.clear();
            v.emplace_back( newdeck.pop_card() );
            v.emplace_back( newdeck.pop_card() );
            v.insert(v.end(), communityCards.begin(), communityCards.end());
        }
        cardsAMutable.insert(cardsAMutable.end(), communityCards.begin(), communityCards.end());

        const FullHandRank fhrA = calcFullHandRank(cardsAMutable);
        bool aWins = true;
        for(auto& v : otherPlayerCards) {
            auto otherGuyFHR = calcFullHandRank(v);
            auto showdownResult = showdownFHR(fhrA, otherGuyFHR);
            if( showdownResult == otherGuyFHR ) aWins = false;
        }
        
        if( aWins ) {
            winCountA++;
            outcomes[iN] = 1;
        }
    }

    std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
    const double winRateA = double(winCountA)/N;
    //std::cout << "A winrate: " << winRateA*100.0 << "%" << std::endl;
    //std::cout << N << " hands calculated in " << duration.count() << " seconds, or " << double(N)/duration.count() << " hands/second" << std::endl;
    double sigma = 0.0;
    for(auto& e : outcomes)
        sigma += (e - winRateA)*(e - winRateA);
    sigma /= N;
    sigma = sqrt(sigma);
    return std::make_tuple(winRateA, sigma);
}



std::tuple<double,double> monteCarloSingleHand(const std::vector<Card>& cardsA, const std::vector<Card>& commCards, const int numOtherPlayers, const uint64_t N) {
    // returns Avg win rate and standard deviation
    std::vector<short> outcomes(N, 0);
    uint64_t winCountA = 0;
    std::vector<std::vector<Card>> otherPlayerCards(numOtherPlayers);

    std::vector<Card> cardsUnion (cardsA);
    cardsUnion.insert(cardsUnion.end(), commCards.begin(), commCards.end());
    
    const FullHandRank fhrA = calcFullHandRank(cardsUnion);

    std::random_device rd;
    for(int iN = 0; iN < N; iN++) {
        //make new deck without players' cards
        Deck newdeck(cardsUnion);
        newdeck.setSeed(rd());
        newdeck.shuffle();
        // deal other player's cards
        for( auto& v : otherPlayerCards ) {
            v.clear();
            v.emplace_back( newdeck.pop_card() );
            v.emplace_back( newdeck.pop_card() );
            v.insert(v.end(), commCards.begin(), commCards.end());
        }

        bool aWins = true;
        for(auto& v : otherPlayerCards) {
            auto otherGuyFHR = calcFullHandRank(v);
            auto showdownResult = showdownFHR(fhrA, otherGuyFHR);
            if( showdownResult == otherGuyFHR ) aWins = false;
        }
        
        if( aWins ) {
            winCountA++;
            outcomes[iN] = 1;
        }
    }

    const double winRateA = double(winCountA)/N;
    //std::cout << "A winrate: " << winRateA*100.0 << "%" << std::endl;
    //std::cout << N << " hands calculated in " << duration.count() << " seconds, or " << double(N)/duration.count() << " hands/second" << std::endl;
    double sigma = 0.0;
    for(auto& e : outcomes)
    {
        sigma += (e - winRateA)*(e - winRateA);
    }
    sigma /= N;
    sigma = sqrt(sigma);
    return std::make_tuple(winRateA, sigma);
}

  void monteCarloHandRankCompare(const std::vector<Card>& cardsA, const std::vector<Card>& cardsB, const uint64_t& N) {
      std::random_device rd;

      uint64_t winCountA = 0;
      uint64_t winCountB = 0;

      constexpr int numComCards = 5;
      
      std::vector<Card> cardsUnion (cardsA.begin(), cardsA.end());
      cardsUnion.insert(cardsUnion.end(), cardsB.begin(), cardsB.end());

      auto start = std::chrono::steady_clock::now();
      winCountA = 0;
      winCountB = 0;
      for(int iN = 0; iN < N; iN++) {
        std::vector<Card> communityCards(numComCards);
        std::vector<Card> cardsAMutable(cardsA);
        std::vector<Card> cardsBMutable(cardsB);
        //make new deck without players' cards
        Deck newdeck(cardsUnion);
        newdeck.shuffle();
        for(short j=0; j<numComCards; j++)
            communityCards[j]=newdeck.pop_card();
        cardsAMutable.insert(cardsAMutable.end(), communityCards.begin(), communityCards.end());
        cardsBMutable.insert(cardsBMutable.end(), communityCards.begin(), communityCards.end());

        const FullHandRank fhrA = calcFullHandRank(cardsAMutable);
        const FullHandRank fhrB = calcFullHandRank(cardsBMutable);
        auto showdownResult = showdownFHR(fhrA, fhrB);
        
        if(showdownResult == fhrA) winCountA++;
        else if(showdownResult == fhrB) winCountB++;
      }


      std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
      const double winRateA = double(winCountA)/N;
      const double winRateB = double(winCountB)/N;
      const int64_t numDraws = N - winCountA - winCountB;
      std::cout << "A winrate: " << winRateA*100.0 << "%" << std::endl;
      std::cout << "B winrate: " << winRateB*100.0 << "%" << std::endl;
      std::cout << "number of draws: " << numDraws << std::endl;
      std::cout << N << " hands calculated in " << duration.count() << " seconds, or " << double(N)/duration.count() << " hands/second" << std::endl;
  }




void monteCarloRandomHand(const int numCommCards, const int numOtherPlayers, const uint64_t numHands, const uint64_t numHandMC, std::string outFileName, int numThreads) {
    // Generates a random hand and community cards
    // Records them to a file as well as the likelihood of winning with that hand

    std::ofstream outFile(outFileName, std::ios_base::app);
    if( !outFile ) throw;


    auto doWork = [&]() {
        std::random_device rd;
        int prntCnt = 0;
        std::string ret;
        for(int iN=0; iN<numHands/numThreads; iN++) {
            std::ostringstream oss;
            Deck newdeck;
            newdeck.setSeed(rd());
            newdeck.shuffle();
            std::vector<Card> commCards(numCommCards);
            std::vector<Card> myCards(2);

            // deal community cards
            for(short j=0; j<numCommCards; j++)
                commCards[j]=newdeck.pop_card();
            myCards[0]=newdeck.pop_card();
            myCards[1]=newdeck.pop_card();

            auto [avg, sigma] = monteCarloSingleHand( myCards, commCards, numOtherPlayers, numHandMC );
            std::vector<Card> allCards = myCards;
            allCards.insert(allCards.end(), commCards.begin(), commCards.end());
            std::vector<std::tuple<int, int>> charlesFormatCards = convertMikeToCharles(allCards);
            oss << numOtherPlayers << ",";
            for(auto& pair : charlesFormatCards) {
                oss << std::get<0>(pair) << "," << std::get<1>(pair) << ",";
            }
            oss << avg;
            oss << std::endl;
            ret.append(oss.str());
            //if( ++prntCnt == 1000 ) {
            //    prntCnt = 0;
            //    std::cout << iN << "/" << numHands << std::endl;
            //}
        }
        return ret;
    };
    /*
    std::vector<std::future<std::string>> futures;
    std::vector<std::string> results;
    for(int j = 0; j < numThreads; j++)
        futures.emplace_back(std::async(std::launch::async, doWork));
    for( auto& fut : futures) 
        results.push_back(fut.get());
    for( auto& str : results)
        outFile << str;
    outFile.close();
    */

}
    






}
