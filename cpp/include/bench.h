#pragma once
#include "card.h"

namespace Poker {
  void benchmarkHandRankCalculator();
  void benchmarkRounds(uint64_t);
  void benchmarkGames(uint64_t);

  void monteCarloHandRankCompare(const std::vector<Card>& cardsA, const std::vector<Card>& cardsB, const double& tgt);
  void monteCarloHandRankCompare(const std::vector<Card>& cardsA, const std::vector<Card>& cardsB, const uint64_t& N);

  void monteCarloGameStateCompare();

}
