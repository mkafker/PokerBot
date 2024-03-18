#include "strategy.h"
#include "player.h"
namespace Poker {
    
    PlayerMove Strategy::makeMove(std::shared_ptr<Table> info, const shared_ptr<Player> p) {
        // Default behavior: Always call
        // p = contains last move info
        auto clamp = [](int a, int b, int c) -> int { if(a<b) a=b; if(a>c) a=c; return a;};
        PlayerMove myMove;
        myMove.move = Move::MOVE_CALL;
        myMove.bet_amount = info->minimumBet;
        myMove.bet_amount = clamp(myMove.bet_amount, 0, p->bankroll);
        if( myMove.bet_amount == p->bankroll) myMove.move = Move::MOVE_ALLIN;
        return myMove;
        
    }   

    PlayerMove SingleMoveCallAI::makeMove(std::shared_ptr<Table> info, const shared_ptr<Player> p) {
        auto clamp = [](int a, int b, int c) -> int { if(a<b) a=b; if(a>c) a=c; return a;};
        
        PlayerMove myMove;
        myMove.move = Move::MOVE_CALL;
        myMove.bet_amount = info->minimumBet;
        myMove.bet_amount = clamp(myMove.bet_amount, 0, p->bankroll);
        if( myMove.bet_amount == p->bankroll) myMove.move = Move::MOVE_ALLIN;
        return myMove;
    }   

    PlayerMove RandomAI::makeMove(std::shared_ptr<Table> info, const shared_ptr<Player> p) {
        // Performs a random valid move
        auto clamp = [](int a, int b, int c) -> int { if(a<b) a=b; if(a>c) a=c; return a;};
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist4(1,4);
        PlayerMove myMove;
        switch( dist4(rng) ) {
            case 1:
                myMove.move = Move::MOVE_FOLD;
                break;
            case 2:
                myMove.move = Move::MOVE_CALL;
                break;
            case 3:
                myMove.move = Move::MOVE_RAISE;
                break;
            case 4:
                myMove.move = Move::MOVE_ALLIN;
        }
        if( myMove.move == Move::MOVE_FOLD) myMove.bet_amount = 0;
        else if( myMove.move == Move::MOVE_CALL) myMove.bet_amount = info->minimumBet;
        else if( myMove.move == Move::MOVE_RAISE) myMove.bet_amount = info->minimumBet * 2;
        else if( myMove.move == Move::MOVE_ALLIN) myMove.bet_amount = p->bankroll;
        myMove.bet_amount = clamp(myMove.bet_amount, 0, p->bankroll);
        if( myMove.bet_amount == p->bankroll) myMove.move = Move::MOVE_ALLIN;
        return myMove;
    }
    PlayerMove SequenceMoveAI::makeMove(std::shared_ptr<Table> info, const shared_ptr<Player> p) {
        // Performs a list of moves
        // If it reaches the end of its move sequence, repeats the last one
        auto clamp = [](int a, int b, int c) -> int { if(a<b) a=b; if(a>c) a=c; return a;};
        
        PlayerMove myMove = moveList.at(index);
        if( index != moveList.size())
            index++;
        // sanitize the move... unnecessary?
        if( myMove.move == Move::MOVE_FOLD) myMove.bet_amount = 0;
        if( myMove.move == Move::MOVE_CALL) myMove.bet_amount = info->minimumBet;
        if( myMove.move == Move::MOVE_ALLIN) myMove.bet_amount = p->bankroll;
        myMove.bet_amount = clamp(myMove.bet_amount, 0, p->bankroll);
        if( myMove.bet_amount == p->bankroll) myMove.move = Move::MOVE_ALLIN;
        return myMove;
    }
    
    PlayerMove FHRProportionalAI::makeMove(shared_ptr<Table> info, const shared_ptr<Player> p) {
      auto clamp = [](int a, int b, int c) -> int { if(a<b) a=b; if(a>c) a=c; return a;};
      vector<Card> pHand = p->hand;
      vector<Card> allCards = info->communityCards;
      allCards.insert(allCards.begin(), pHand.begin(), pHand.end());
      FullHandRank myFHR = calcFullHandRank(allCards);

      PlayerMove myMove;
      if ( info->street == 0) {
        // if preflop, call no matter what
        myMove.bet_amount = info->minimumBet;
        myMove.move = Move::MOVE_CALL;
      }
      else {
        const auto cutoffBadDecentHand = HandRank::HIGH_CARD;
        const auto cutoffDecentGoodHand = HandRank::TWO_PAIR;
        const auto cutoffGoodGreatHand =  HandRank::FLUSH;
        if( myFHR.handrank < cutoffBadDecentHand ) {
            myMove.bet_amount = 0;
            myMove.move = Move::MOVE_FOLD;
        } else {
          if ( myFHR.handrank < cutoffDecentGoodHand ) {
            myMove.bet_amount = info->minimumBet;
            myMove.move = Move::MOVE_CALL;
          }
          if ( myFHR.handrank < cutoffGoodGreatHand ) {
            myMove.bet_amount = info->minimumBet * 2;
            myMove.move = Move::MOVE_RAISE;
          }
          else {
            myMove.bet_amount = p->bankroll;
            myMove.move = Move::MOVE_ALLIN;
          }

        }

      }
      myMove.bet_amount = clamp(myMove.bet_amount, 0, p->bankroll);
      if( myMove.bet_amount == p->bankroll ) myMove.move = Move::MOVE_ALLIN;
      return myMove;
    }
    
    
    PlayerMove HRBetRelAI::makeMove(shared_ptr<Table> info, const shared_ptr<Player> p) {
      auto clamp = [](int& a, const int& b, const int& c)  { if(a<b) a=b; if(a>c) a=c;};
      vector<Card> pHand = p->hand;
      vector<Card> allCards = info->communityCards;
      allCards.insert(allCards.begin(), pHand.begin(), pHand.end());
      FullHandRank myFHR = calcFullHandRank(allCards);
      
      PlayerMove myMove;
      myMove.bet_amount = rankBetRelationship[myFHR.handrank];
      clamp(myMove.bet_amount, 0, p->bankroll);
      if( myMove.bet_amount == 0 ) myMove.move = Move::MOVE_FOLD;
      else if( myMove.bet_amount == p->bankroll ) myMove.move = Move::MOVE_ALLIN;
      else {
        clamp(myMove.bet_amount, info->minimumBet, p->bankroll);
        if( myMove.bet_amount > info->minimumBet )
          myMove.move = Move::MOVE_RAISE;
        else
          myMove.move = Move::MOVE_CALL;
      }
      return myMove;
    }


}

