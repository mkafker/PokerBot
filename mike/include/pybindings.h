#pragma once

namespace Poker {


void pyMonteCarloGames(const uint64_t& N);


double pyMonteCarloRounds(const uint64_t& N, std::vector<int> params);
double pyMonteCarloRounds(const uint64_t& N, std::vector<float> params);
double pyMCSingleHand(const std::vector<std::tuple<int,int>>& cardsA, const std::vector<std::tuple<int,int>>& commCards, const int numOtherPlayers, const uint64_t N);

int pyShowdownHands(std::vector<std::tuple<int,int>> tupleIntsA, std::vector<std::tuple<int,int>> tupleIntsB, 
                    std::vector<std::tuple<int,int>> communityTupleInts);

std::vector<Card> convertCharlesToMike(std::vector<std::tuple<int,int>> in); //converts cards from charles's convention to mine

std::vector<std::tuple<int, int>> convertMikeToCharles(std::vector<Card> in); //ditto but in the other direction

}