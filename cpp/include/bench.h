#pragma once
#include "card.h"
#include <vector>

namespace Poker {
  void benchmarkHandRankCalculator();
  void benchmarkRounds(uint64_t);
  void benchmarkGames(uint64_t);

  void monteCarloHandRankCompare(const std::vector<Card>& cardsA, const std::vector<Card>& cardsB, const double& tgt);
  void monteCarloHandRankCompare(const std::vector<Card>& cardsA, const std::vector<Card>& cardsB, const uint64_t& N);
  void monteCarloGames(const uint64_t& N, std::vector<std::string> aiList );

  double monteCarloSingleHand(const std::vector<Card>& cardsA, const int numCommCards, const int numOtherPlayers, const uint64_t N);
  double monteCarloSingleHand(const std::vector<Card>& cardsA, const std::vector<Card>& commCards, const int numOtherPlayers, const uint64_t N);
  std::vector<double> monteCarloSingleHandStdDev(const std::vector<Card>& cardsA, const std::vector<Card>& commCards, const int numOtherPlayers, const uint64_t N);
  std::vector<double> monteCarloSingleHandStdDev(const std::vector<Card>& cardsA, const int numCommCards, const int numOtherPlayers, const uint64_t N);
  void monteCarloGameStateCompare();

}
