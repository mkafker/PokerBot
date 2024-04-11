#pragma once
#include "card.h"
#include <vector>
#include <map>
#include <any>

namespace Poker {
  void benchmarkHandRankCalculator(const uint64_t& N);
  void benchmarkRounds(uint64_t);

  void monteCarloHandRankCompare(const std::vector<Card>& cardsA, const std::vector<Card>& cardsB, const double& tgt);
  void monteCarloHandRankCompare(const std::vector<Card>& cardsA, const std::vector<Card>& cardsB, const uint64_t& N);

  std::tuple<double,double> monteCarloSingleHand(const std::vector<Card>& cardsA, const int numCommCards, const int numOtherPlayers, const uint64_t N);
  std::tuple<double,double> monteCarloSingleHand(const std::vector<Card>& cardsA, const std::vector<Card>& commCards, const int numOtherPlayers, const uint64_t N);
  void monteCarloGameStateCompare();

  std::tuple<double, double> monteCarloRounds(const uint64_t& N, const std::multimap<std::string, std::vector<std::any>>& aiInfo);
  std::tuple<double, double> monteCarloGames(const uint64_t& N, const std::multimap<std::string, std::vector<std::any>>& aiInfo);
  void monteCarloRandomHand(const int numCommCards, const int numOtherPlayers, const uint64_t numHands, const uint64_t numHandMC, std::string outFileName);
}
