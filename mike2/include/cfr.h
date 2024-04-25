#pragma once
#include "player.h"
#include "showdown.h"
#include <tuple>


namespace Poker {
  template <typename T>
  static void hashCombine(size_t& seed, const T& v) {
      hash<T> hasher;
      seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }

  namespace CFR {
    struct BinnedFullHandRank  {
        HandRank handrank = HandRank::UNDEF_HANDRANK;                  
        Card maincard = Card();        
        bool operator==(const BinnedFullHandRank& other) const {
            return this->handrank == other.handrank and this->maincard == other.maincard;
        }
        bool operator<(const BinnedFullHandRank& other) const {
            if( this->handrank > other.handrank) return false;
            if( this->handrank < other.handrank) return true;
            if( this->maincard > other.maincard) return false;
            if( this->maincard < other.maincard) return true;
            return false;
        }
        operator int() const {
            return static_cast<int>(handrank) * 13 + maincard.get_rank_as_int();
        }
    };
    enum class BinnedPlayerMove : int {
        Undef = -1,
        Fold = 0,
        Call = 1,
        Raise = 2,
        AllIn = 3
    };
    struct BinnedPlayerMoveHash {
        size_t operator()(const BinnedPlayerMove& pp) const {
            return static_cast<size_t>(pp);
        }
    };
    struct PlayerPositionHash {
        size_t operator()(const PlayerPosition& pp) const {
            return static_cast<size_t>(pp);
        }
    };
    enum BinnedGameStateClassification {
      UNDEF,      // debug
      TERMINAL,   // no further moves from players
      CHOICE      // further move from players
    };
    struct BinnedGameState {
      // vv Player Position dependent
      PlayerPosition position;
      BinnedFullHandRank BFHR;
      // vv Player Position independent
      int street;
      map<PlayerPosition, vector<BinnedPlayerMove>> playerHistory;

      bool operator==(const BinnedGameState& other) const {
          return position == other.position &&
                  int(BFHR) == int(other.BFHR) &&
                  street == other.street &&
                  playerHistory == other.playerHistory;
      }
      BinnedGameStateClassification classify();
      PlayerPosition getCurrentPlayer(); 
    };
    std::tuple<BinnedGameState, BinnedFullHandRank> swapPlayers(const BinnedGameState& key, const BinnedFullHandRank& oppCards);
    struct BGSHash {
      size_t operator()(const BinnedGameState& rgs) const {
          size_t hash = 0;
          hashCombine(hash, rgs.position);
          hashCombine(hash, int(rgs.BFHR));
          hashCombine(hash, rgs.street);
          for(const auto& pair : rgs.playerHistory) {
              hashCombine(hash, pair.first);
              for(const auto& v : pair.second) {
                  hashCombine(hash, v);
              }
          }
          return hash;
      }
    };
    struct MoveStrategyInfo {
      float regretSum = 0.0;
      float strategyProbSum = 0.0;
      float strategyProb = 0.0;
    };
    struct InfoSet {
      float reachProb = 0;
      float reachProbSum = 0;
      map<BinnedPlayerMove, MoveStrategyInfo> strategyMap;
    };

    unordered_map<BinnedGameState, InfoSet, BGSHash> InfoMap;


    float cfr(unordered_map<BinnedGameState, InfoSet> infomap, BinnedGameState key, BinnedFullHandRank oppCards, float probSB, float probBB, float probC) {
      auto keyClassification = key.classify();
      if( keyClassification == BinnedGameStateClassification::TERMINAL ) {
        // return terminal utility
      }

      // Update reach probabilities
      const PlayerPosition myPos = key.position;
      if( myPos == PlayerPosition::POS_BB )
        infomap[key].reachProb += probBB;
      else if( myPos == PlayerPosition::POS_SB)
        infomap[key].reachProb += probSB;
      
      // Counterfactual utility per action
      std::vector<BinnedPlayerMove> validMoves = key.getValidMoves();
      std::vector<float> actionUtils(validMoves.size(), 0.0);


      auto [swappedBGS, swappedOppCards] = swapPlayers(key, oppCards);
      for(auto moveIt = validMoves.begin(); moveIt != validMoves.end(); moveIt++) {
        size_t d = std::distance(moveIt, validMoves.begin());
        const float thisProb = infomap[key].strategyMap[*moveIt].strategyProb;
        if( myPos == PlayerPosition::POS_BB )
          actionUtils[d] = -1.0 * cfr(infomap, swappedBGS, swappedOppCards, 
                  probSB, probBB * thisProb, probC);
        else if( myPos == PlayerPosition::POS_SB )
          actionUtils[d] = -1.0 * cfr(infomap, swappedBGS, swappedOppCards, 
                  probSB * thisProb, probBB, probC);
      }

      float utility = std::accumulate(actionUtils.begin(), actionUtils.end(), 0.0);
      std::vector<float> regrets(actionUtils);
      std::for_each(regrets.begin(), regrets.end(), [&utility](float& a) { a -= utility;});

      for(auto moveIt = validMoves.begin(); moveIt != validMoves.end(); moveIt++) {
        size_t d = std::distance(moveIt, validMoves.begin());
        float thisMovesRegretSum = infomap[key].strategyMap[*moveIt].regretSum;
        if( myPos == PlayerPosition::POS_BB )
          thisMovesRegretSum += probSB * probC * regrets[d];
        else if( myPos == PlayerPosition::POS_SB )
          thisMovesRegretSum += probBB * probC * regrets[d];
      }
      
      return utility;
    }

  }
}