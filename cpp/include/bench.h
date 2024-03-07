#pragma once
#include "card.h"

namespace Poker {
  void benchmarkHandRankCalculator();
  void benchmarkRounds();
  void benchmarkGames();

  void monteCarloHandRankCompare(const std::vector<Card>& cardsA, const std::vector<Card>& cardsB, const double& tgt);
  void monteCarloHandRankCompare(const std::vector<Card>& cardsA, const std::vector<Card>& cardsB, const uint64_t& N);

}
