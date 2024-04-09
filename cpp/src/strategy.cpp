#include "strategy.h"
#include "player.h"
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
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

      map<BinnedPlayerMove, float> probMap;
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
    
    void CFRAI1::dumpCFRTableToFile(std::string outfile) {
      // Transform a reduced game state to a string
      std::ostringstream oss;
      auto RGSintoStream = [&oss] (ReducedGameState rgs) {
          oss << rgs.position << "," << rgs.street << ",";
          oss << rgs.RFHR.handrank << "," << rgs.RFHR.maincard.get_rank() << ",";
          for(const auto& pair : rgs.playerHistory) {
            auto pos = pair.first;
            std::vector<BinnedPlayerMove> moves = pair.second;
            oss << pos << "," << moves.size() << ",";
            for(int i=0; i<moves.size(); i++) {
              oss << static_cast<int>(moves[i]) << ",";
            }
          }
      };
      
      // Print RGS and probabilities on one line
      for(const auto& pair : CFRTable) {
        RGSintoStream(pair.first);
        auto BPMprobmap = pair.second;
        for(auto it = BPMprobmap.begin(); it != BPMprobmap.end(); it++) {
          // print probabilities
          oss << it->second; 
          if(std::distance(it, BPMprobmap.end()) == 1) oss << std::endl;
          else oss << ",";
        }
      }

      // Output to file
      std::ofstream outFile(outfile);
      if( !outFile ) throw;
      outFile << oss.str();
      outFile.close();
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


    PlayerMove MattAI::makeMove(std::shared_ptr<Table> info, const shared_ptr<Player> p) {
      int numOtherPlayers = info->playerList.size() - 1;
      const double foldCallThres = thresholds[0];
      const double callRaiseThres = thresholds[1];
      const double raiseAllinThres = thresholds[2];
      auto [avg, sigma] = monteCarloSingleHand(p->hand, info->communityCards, numOtherPlayers, 100);
      //std::cout << "MATT SAYS: " << p->hand << " = " << matt << std::endl;
      PlayerMove myMove;
      if( avg < foldCallThres )
        myMove.move = Move::MOVE_FOLD;
      else if( avg < callRaiseThres )
        myMove.move = Move::MOVE_CALL;
      else if( avg < raiseAllinThres )
        myMove.move = Move::MOVE_RAISE;
      else
        myMove.move = Move::MOVE_ALLIN;


      if( myMove.move == Move::MOVE_FOLD) myMove.bet_amount = 0;
      else if( myMove.move == Move::MOVE_CALL) myMove.bet_amount = info->minimumBet;
      else if( myMove.move == Move::MOVE_RAISE) myMove.bet_amount = info->minimumBet * 2;
      else if( myMove.move == Move::MOVE_ALLIN) myMove.bet_amount = p->bankroll;
      myMove.bet_amount = clamp(myMove.bet_amount, 0, p->bankroll);
      if( myMove.bet_amount == p->bankroll) myMove.move = Move::MOVE_ALLIN;

      return myMove;

    }

    void MattAI::updateParametersImpl(std::vector<double> thresholds_in) {
        thresholds = thresholds_in;
    }


}

