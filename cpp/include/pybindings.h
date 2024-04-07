#pragma once

namespace Poker {


void pyMonteCarloGames(const uint64_t& N);


double pyMonteCarloRounds(const uint64_t& N, std::vector<std::tuple<std::string, std::vector<int>>> aiInfo);

double pyMCSingleHand(const std::vector<std::tuple<int,int>>& cardsA, const std::vector<std::tuple<int,int>>& commCards, const int numOtherPlayers, const uint64_t N);

int pyShowdownHands(std::vector<std::tuple<int,int>> tupleIntsA, std::vector<std::tuple<int,int>> tupleIntsB, 
                    std::vector<std::tuple<int,int>> communityTupleInts);

std::vector<Card> convertCharlesToMike(std::vector<std::tuple<int,int>> in); //converts cards from charles's convention to mine

}