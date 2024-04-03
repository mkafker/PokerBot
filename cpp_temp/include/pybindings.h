#pragma once

namespace Poker {


void pyMonteCarloGames(const uint64_t& N);

double pyMonteCarloRounds(const uint64_t& N, const std::vector<int> inputRBRraw);



int pyShowdownHands(std::vector<std::tuple<int,int>> tupleIntsA, std::vector<std::tuple<int,int>> tupleIntsB, 
                    std::vector<std::tuple<int,int>> communityTupleInts);


}