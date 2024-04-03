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
    
    
    PlayerMove HandStreetAwareAI::makeMove(shared_ptr<Table> info, const shared_ptr<Player> p) {
      auto clamp = [](int& a, const int& b, const int& c)  { if(a<b) a=b; if(a>c) a=c;};
      vector<Card> pHand = p->hand;
      vector<Card> allCards = info->communityCards;
      allCards.insert(allCards.begin(), pHand.begin(), pHand.end());
      FullHandRank myFHR = calcFullHandRank(allCards);
      
      PlayerMove myMove = betAmountToMove(        streetRBR[info->street][myFHR.handrank] * info->bigBlind,     info, p);
      return myMove;
    }

    PlayerMove FHRAwareAI::makeMove(shared_ptr<Table> info, const shared_ptr<Player> p) {
      vector<Card> pHand = p->hand;
      vector<Card> allCards = info->communityCards;
      allCards.insert(allCards.begin(), pHand.begin(), pHand.end());
      FullHandRank myFHR = calcFullHandRank(allCards);
      ReducedFullHandRank RFHR;
      RFHR.handrank = myFHR.handrank;
      // max is probably unnecessary
      RFHR.maincard = *min(myFHR.maincards.begin(), myFHR.maincards.end()); // min actually returns the max rank very cool
      PlayerMove myMove = betAmountToMove(        RFHRBetRelationship[RFHR] * info->bigBlind,     info, p);
      return myMove;
    }
    void FHRAwareAI::updateRFHRBetRelationship(const vector<int>& vec_in) {
      // expected # of parameters: 13 card ranks * 9 hand ranks = 117
      // hand rank major order
      if( vec_in.size() != 117 ) throw;
      for(int hr_id=0; hr_id<9; hr_id++){
        for(int c_id=0; c_id<13; c_id++) {
          const int v_id= hr_id*13 + c_id;
          Card c = {static_cast<Rank>(c_id), static_cast<Suit>('X')};
          ReducedFullHandRank r;
          r.handrank =  static_cast<HandRank>(hr_id+1);
          r.maincard = c;
          this->RFHRBetRelationship[r] = vec_in[v_id];
        }
      }
    }


    PlayerMove MoveAwareAI::makeMove(shared_ptr<Table> info, const shared_ptr<Player> p) {
      // Set me to playerID = 0
      auto otherGuy = info->getPlayerByID(1);
      if( info->street == 0) enemyMoves.clear();
      if( otherGuy->getPosition() < p->getPosition()) {
        // other guy already made a move
        enemyMoves.emplace_back(otherGuy->move.move);
      }
      while( enemyMoves.size() > 2) { enemyMoves.erase(enemyMoves.begin()); } //trim to last two moves
      auto myMove = betAmountToMove(  moveSequenceToBet[enemyMoves], info, p);
      return myMove;
    }

    PlayerMove CFRAI1::makeMove(shared_ptr<Table> info, const shared_ptr<Player> p) {
      // Add this game state to the table if it doesn't exist
      ReducedGameState myRGS = packTableIntoReducedGameState(*info);
      PlayerMove myPMove;

      unordered_map<BinnedPlayerMove, float> probMap;
      std::random_device rd;
      std::mt19937_64 gen(rd());

      float tot = 0.0f;

      auto [updatedIter, rgsWasNew] = CFRTable.try_emplace(myRGS);
      // rgsWasNew is true is the RGS key didn't exist in the CFR table
      // updatedIter is an iterator to the key corresponding to the game state whether or not the insertion took place
      if( rgsWasNew ) {
        // make a random move if we don't have a policy
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        probMap[BinnedPlayerMove::AllIn] = dist(gen);
        probMap[BinnedPlayerMove::Raise] = dist(gen);
        probMap[BinnedPlayerMove::Call] = dist(gen);
        probMap[BinnedPlayerMove::Fold] = dist(gen);
        for_each(probMap.begin(), probMap.end(), [&tot](const pair<BinnedPlayerMove, float> pair) { 
          tot += pair.second;
        });
        // insert the normalized random policy
        CFRTable[myRGS] = probMap;
      }
      else {
        // policy is already in there
        probMap = CFRTable[myRGS];
      }

      // Sample the choices according to the prob dist
        std::uniform_real_distribution<float> dist(0.0f, tot);
      float choice = dist(gen);
      float partialSum = 0.0f;
      BinnedPlayerMove bpm;
      vector<float> cumProbs(probMap.size());
      for( auto& pair : probMap ) {
        partialSum += pair.second;
        if(partialSum >= choice) {
          bpm = pair.first;
          break;
        } 
      }

      return unpackBinnedPlayerMove(bpm, info->minimumBet, p->bankroll);
    }

    CFRAI1::BinnedPlayerMove CFRAI1::packBinnedPlayerMove(PlayerMove m) {
      if( m.move == Move::MOVE_ALLIN ) return CFRAI1::BinnedPlayerMove::AllIn;
      else if( m.move == Move::MOVE_FOLD ) return CFRAI1::BinnedPlayerMove::Fold;
      else if( m.move == Move::MOVE_CALL ) return CFRAI1::BinnedPlayerMove::Call;
      else if( m.move == Move::MOVE_RAISE ) return CFRAI1::BinnedPlayerMove::Raise;
      else return CFRAI1::BinnedPlayerMove::Undef;
    }

    CFRAI1::ReducedGameState CFRAI1::packTableIntoReducedGameState(Table table) {
        ReducedGameState rgs;
        rgs.street = table.street;
        // Player 0 is the special player
        shared_ptr<Player> playerZero = table.getPlayerByID(0);
        rgs.position = playerZero->getPosition();

        // This could probably be moved to dealCommunityCards
        vector<Card> pHand = playerZero->hand;
        vector<Card> allCards = table.communityCards;
        allCards.insert(allCards.begin(), pHand.begin(), pHand.end());
        FullHandRank myFHR = calcFullHandRank(allCards);
        rgs.RFHR.handrank = myFHR.handrank;
        rgs.RFHR.maincard = *min(myFHR.maincards.begin(), myFHR.maincards.end()); // min actually returns the max rank very cool
        for( auto& p : table.playerList ) {
          rgs.playerHistory[p->getPosition()].push_back(packBinnedPlayerMove(p->move));
        }
        return rgs;
    }
    PlayerMove CFRAI1::unpackBinnedPlayerMove(BinnedPlayerMove m, int minimumBet, int bankroll) {
      PlayerMove ret;
      if(m == CFRAI1::BinnedPlayerMove::AllIn ) {
        ret.move = Move::MOVE_ALLIN;
        ret.bet_amount = bankroll;
      }
      else if(m == CFRAI1::BinnedPlayerMove::Raise ) {
        ret.move = Move::MOVE_RAISE;
        ret.bet_amount = minimumBet * 2;
      }
      else if(m == CFRAI1::BinnedPlayerMove::Call ) {
        ret.move = Move::MOVE_CALL;
        ret.bet_amount = minimumBet;
      }
      else if(m == CFRAI1::BinnedPlayerMove::Fold ) {
        ret.move = Move::MOVE_FOLD;
        ret.bet_amount = 0;
      }
      return ret;
    }




    static PlayerMove betAmountToMove(int betAmount, shared_ptr<Table>& info, const shared_ptr<Player>& p) {
      PlayerMove myMove;
      myMove.bet_amount = betAmount;
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

