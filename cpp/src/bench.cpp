#include "bench.h"
#include "showdown.h"
#include "table.h"
#include "player.h"
#include "game.h"
#include <chrono>

namespace Poker {
  void benchmarkHandRankCalculator() {
      std::random_device rd;
      std::mt19937_64 g(rd());
      uint64_t N = 1000000;
      auto start = std::chrono::steady_clock::now();
      FullHandRank lastHand;
      int iT = 0;
      for(int iN = 0; iN < N; iN++) {
          iT++;
          Deck newdeck;
          newdeck.mersenne = g;
          newdeck.shuffle();
          std::vector<Card> cards(7);
          for(short j=0; j<7; j++)
              cards[j]=newdeck.pop_card();
          FullHandRank FHR = calcFullHandRank(cards);
          /*
          if( iT == 1000) {
              iT = 0;
                  std::cout << FHR.handrank << " " << FHR.maincards << "| " << FHR.kickers << std::endl;

          }
          */
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
      auto start = std::chrono::steady_clock::now();
      std::shared_ptr<Player> bestPlayer;
      int iT = 0;
      auto game = std::make_shared<Game>();
      for(int iN = 0; iN < N; iN++) {
          game->resetToDefaults();
          game->doRound();
          std::shared_ptr<Player> winningPlayer = game->lastRoundWinner;
          if( iN == 0 ) bestPlayer = winningPlayer;
          // Skip if there wasn't a winner (everyone folded for some reason)
          if( winningPlayer ) {
            auto winFHR = winningPlayer->FHR;
            auto bestFHR = bestPlayer->FHR;
            if( showdownFHR(bestFHR, winFHR) == winFHR )  {
                bestPlayer = std::move(winningPlayer);
            }
          }
      }

      std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
      std::cout << "Best hand: " << std::endl;
      std::cout << "Player " << bestPlayer->playerID  << " " << PlayerPosition_to_String[bestPlayer->getPosition()]
                << " " << bestPlayer->FHR.handrank << " " << bestPlayer->FHR.maincards << "| " << bestPlayer->FHR.kickers << std::endl;
      std::cout << N << " rounds calculated in " << duration.count() << " seconds, or " << double(N)/duration.count() << " rounds/second" << std::endl;
  }
  void benchmarkGames(uint64_t N) {
      std::random_device rd;
      vector<string> aiList = {"call", "random", "random", "random", "call", "random", "call"};
      auto start = std::chrono::steady_clock::now();
      int iT = 0;
      auto game = std::make_shared<Game>();
      for(int iN = 0; iN < N; iN++) {
            auto table = game->table;
            table.setPlayerList(aiList);
            // set blinds 
            table.bigBlind     = 10;
            table.smallBlind   = 5;
            table.resetCards(rd);
            // set all players to active and betting
            game->activePlayers = table.getPlayersInBettingOrder();
            game->bettingPlayers = game->activePlayers;
            game->doGame();
      }

      std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
      std::cout << N << " games calculated in " << duration.count() << " seconds, or " << double(N)/duration.count() << " games/second" << std::endl;
  }

void monteCarloGameStateCompare() {
    std::random_device rd;
    auto start = chrono::steady_clock::now();
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


void monteCarloGames(const uint64_t& N, vector<string> aiList) {
    std::random_device rd;
    auto myTable = Table();
    // populate player list
    myTable.setPlayerList(aiList);
    // give everybody a benjamin
    myTable.setPlayerBankrolls(100);
    // set blind amounts
    myTable.bigBlind = 10;
    myTable.smallBlind = 5;

    auto game = std::make_shared<Game>(myTable);
    game->activePlayers = game->table.getPlayersInBettingOrder(); // activate all players

    // setup some statistics
    unordered_map<PlayerPosition, int> posWinCount;
    unordered_map<int, int> playerIDWinCount;

    auto start = std::chrono::steady_clock::now();
    int iT = 0;
    for(int iN = 0; iN < N; iN++) {
        game->setup();
        game->table.setPlayerBankrolls(100);
        game->doGame();
        if( game->lastRoundWinner ) {
            posWinCount[game->lastRoundWinner->position]++;
            playerIDWinCount[game->lastRoundWinner->playerID]++;
        }
    }

    std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
    std::cout << N << " games calculated in " << duration.count() << " seconds, or " << double(N)/duration.count() << " games/second" << std::endl;
    for( const auto& pair : posWinCount) {
        std::cout << PlayerPosition_to_String[pair.first] << ": " << pair.second << " wins" << std::endl;
    }
    for( const auto& pair : playerIDWinCount) {
        std::cout << "Player " << pair.first << ": " << pair.second << " wins" << std::endl;
    }
}


  void monteCarloHandRankCompare(const std::vector<Card>& cardsA, const std::vector<Card>& cardsB, const double& varTarget) {
      std::random_device rd;
      std::mt19937_64 g(rd());
      uint64_t N = 0;

      uint64_t winCountA = 0;
      uint64_t winCountB = 0;

      constexpr int numComCards = 5;
      
      double avgEstimate    = 0.5;
      double varEstimate = 1.0;
      uint64_t updateInterval = 1000;    // calculate variance every # rounds
      bool first = true;
      
      std::vector<Card> cardsUnion (cardsA.begin(), cardsA.end());
      cardsUnion.insert(cardsUnion.end(), cardsB.begin(), cardsB.end());

      auto start = std::chrono::steady_clock::now();
      while ( varEstimate > varTarget or first ) {
        first = false;
        winCountA = 0;
        winCountB = 0;
        for(int iN = 0; iN < updateInterval; iN++) {
          std::vector<Card> communityCards(numComCards);
          std::vector<Card> cardsAMutable(cardsA);
          std::vector<Card> cardsBMutable(cardsB);
          //make new deck without players' cards
          Deck newdeck(cardsUnion);
          newdeck.mersenne = g;
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

        const double roundWinRate = double(winCountA) / updateInterval;
        N+=updateInterval;
        const double oldAvgEstimate = avgEstimate;
        avgEstimate = avgEstimate + (roundWinRate - avgEstimate)/updateInterval; // update avgEstimate
        varEstimate = varEstimate + ((roundWinRate - oldAvgEstimate)*(roundWinRate - avgEstimate) - varEstimate)/updateInterval;
        std::cout << "roundWinRate = " << roundWinRate << std::endl;
        std::cout << "avgEstimate = " << avgEstimate << std::endl;
        std::cout << "varEstimate = " << varEstimate << std::endl;
        std::cout << "N = " << N << std::endl;

      }


      std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
      const double winRateA = double(winCountA)/N;
      const double winRateB = double(winCountB)/N;
      //const int64_t numDraws = N - winCountA - winCountB;
      std::cout << "A winrate: " << avgEstimate*100.0 << "%" << std::endl;
      //std::cout << "number of draws: " << numDraws << std::endl;
      std::cout << N << " hands calculated in " << duration.count() << " seconds, or " << double(N)/duration.count() << " hands/second" << std::endl;
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
        std::mt19937_64 g(rd());
        std::vector<Card> communityCards(numComCards);
        std::vector<Card> cardsAMutable(cardsA);
        std::vector<Card> cardsBMutable(cardsB);
        //make new deck without players' cards
        Deck newdeck(cardsUnion);
        newdeck.mersenne = g;
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






}
