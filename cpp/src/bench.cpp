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
          newdeck.shuffle(g);
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
  void benchmarkRounds() {
      uint64_t N = 1000;
      auto start = std::chrono::steady_clock::now();
      auto game1 = std::make_shared<Game>();
      game1->doRound();
      std::shared_ptr<Player> bestPlayer = game1->lastRoundWinner;
      int iT = 0;
      for(int iN = 0; iN < N; iN++) {
          auto game = std::make_shared<Game>();
          game->doRound();
          std::shared_ptr<Player> winningPlayer = game->lastRoundWinner;
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
  void benchmarkGames() {


      uint64_t N = 1000;
      auto start = std::chrono::steady_clock::now();
      FullHandRank lastHand;
      int iT = 0;
      for(int iN = 0; iN < N; iN++) {
          auto game = std::make_shared<Game>();
           game->doGame();
      }

      std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
      std::cout << N << " games calculated in " << duration.count() << " seconds, or " << double(N)/duration.count() << " games/second" << std::endl;
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
          newdeck.shuffle(g);
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
      std::mt19937_64 g(rd());

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
        newdeck.shuffle(g);
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
