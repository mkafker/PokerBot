#include "cfr.h"

namespace Poker {
  using namespace CFR;
  std::vector<BinnedPlayerMove> BinnedGameState::getValidMoves() {
    // Returns a list of valid moves from the big blind's perspective
    const auto myPos = this->position;
    const auto theirPos = PlayerPosition::POS_SB == myPos ? PlayerPosition::POS_BB : PlayerPosition::POS_SB;
    const auto myHistory = this->playerHistory[myPos];
    const auto theirHistory = this->playerHistory[theirPos];
    auto figureMoves = [&](const BinnedPlayerMove& theirLastMove) {
      std::vector<BinnedPlayerMove> CallFoldRaiseAllIn ({ BinnedPlayerMove::Call,
                                                              BinnedPlayerMove::Fold,
                                                              BinnedPlayerMove::Raise,
                                                              BinnedPlayerMove::AllIn});
      if(theirLastMove == BinnedPlayerMove::Fold) {
        // If the other guy folded, we can't do anything since we've won
        this->isTerminal() = true;
        return std::vector<BinnedPlayerMove>();
      }
      else if(theirLastMove != BinnedPlayerMove::Undef) {
        // If the other guy didn't fold, we can do whatever we want
        this->isTerminal() = false;
        return CallFoldRaiseAllIn;
      }
    };
    if(myHistory.size() == theirHistory.size()) {
      if(myPos == PlayerPosition::POS_SB) {
        // my turn
        auto theirLastMove = theirHistory[theirHistory.size()];
        return figureMoves(theirLastMove);
      } else {
        // their turn
        return std::vector<BinnedPlayerMove>();
      }
    }
    else if (myHistory.size() == theirHistory.size() + 1) {
      // their turn
      return std::vector<BinnedPlayerMove>();
    }
    else if (myHistory.size() == theirHistory.size() - 1) {
      // my turn
      auto theirLastMove = theirHistory[theirHistory.size()];
      return figureMoves(theirLastMove);
    }
    else throw;
  }
  BinnedGameStateClassification BinnedGameState::classify() {
    // Assumes the game is in a state that is either a TERMINAL or CHOICE node
    BinnedGameStateClassification bgsc = BinnedGameStateClassification::UNDEF;
    std::vector<BinnedPlayerMove> bpms ();
    const auto myPos = this->position;
    const auto theirPos = PlayerPosition::POS_SB == myPos ? PlayerPosition::POS_BB : PlayerPosition::POS_SB;
    const auto myHistory = this->playerHistory[myPos];
    const auto theirHistory = this->playerHistory[theirPos];
    const auto SBHistory = this->playerHistory[PlayerPosition::POS_SB];
    const auto BBHistory = this->playerHistory[PlayerPosition::POS_BB];
    const BinnedPlayerMove myLastMove = myHistory[myHistory.size()];
    const BinnedPlayerMove theirLastMove = theirHistory[theirHistory.size()];
    // If either histories are empty, we're at the beginning of the game i.e. choice node
    if( myHistory.size() == 0 or theirHistory.size() == 0) {
      return BinnedGameStateClassification::CHOICE;
    }
    // If anybody folded or went all-in as their last move, it's terminal
    if( myLastMove == BinnedPlayerMove::Fold or myLastMove == BinnedPlayerMove::AllIn or
        theirLastMove == BinnedPlayerMove::Fold or theirLastMove == BinnedPlayerMove::AllIn ) {
          return BinnedGameStateClassification::TERMINAL;
    }
    // Terminal node: all cards down and (anything) call
    if( (myLastMove == BinnedPlayerMove::Call or theirLastMove == BinnedPlayerMove::Call) and street == 3) {
        return BinnedGameStateClassification::TERMINAL;
    }
    return BinnedGameStateClassification::UNDEF;
  }

  std::tuple<BinnedGameState, BinnedFullHandRank> swapPlayers(const BinnedGameState& key, const BinnedFullHandRank& oppCards) {
    BinnedGameState newBGS = key;
    newBGS.BFHR = oppCards;
    BinnedFullHandRank newOppCards = key.BFHR;
    return std::make_tuple<BinnedGameState, BinnedFullHandRank>(key, oppCards);
  }

}