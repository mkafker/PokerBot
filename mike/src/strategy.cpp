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
        if( index != moveList.size() - 1)
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
      std::ofstream outFile(outfile, std::ios_base::app);
      if( !outFile ) throw;

      // Transform a reduced game state to a string
      std::ostringstream oss;
      auto RGSintoStream = [&oss] (ReducedGameState rgs) {
          oss << static_cast<int>(rgs.position) << "," << rgs.street << ",";
          oss << static_cast<int>(rgs.RFHR.handrank) << "," << static_cast<int>(rgs.RFHR.maincard.get_rank()) << ",";
          for(const auto& pair : rgs.playerHistory) {
            auto pos = pair.first;
            std::vector<BinnedPlayerMove> moves = pair.second;
            oss << static_cast<int>(pos) << "," << moves.size() << ",";
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
        outFile << oss.str();
      }
      outFile.close();
    }
    void CFRAI1::loadCFRTableFromFile(std::string infile) {
      std::ifstream inFile(infile);
      if( !inFile ) throw;
      unordered_map<ReducedGameState, map<BinnedPlayerMove, float>, RGSHash> tableIn;
      std::vector<std::vector<std::string>> data;
      std::string line;
      while(std::getline(inFile, line)) {
        std::vector<std::string> row;
        std::stringstream ss(line);
        std::string field;
        while (std::getline(ss, field, ',')) {
          row.push_back(field);
        }
        data.push_back(row);
      }

      for(auto& row : data) {
        // key
        ReducedGameState rgs;
        rgs.position = static_cast<PlayerPosition>(std::stoi(row[0]));
        rgs.street =          std::stoi(row[1]);
        rgs.RFHR.handrank = static_cast<HandRank>(std::stoi(row[2]));
        Card main;
        main.rank = static_cast<Rank>(std::stoi(row[3]));
        rgs.RFHR.maincard = main;
        
        unordered_map<PlayerPosition, vector<BinnedPlayerMove>, PlayerPositionHash> pHistory;
        size_t ind = 4;
        size_t runningInd = 0;
        while(typeid(row[ind]) != typeid(float)) {
          PlayerPosition pp = static_cast<PlayerPosition>(std::stoi(row[ind]));
          size_t bpmv_ld = std::stoi(row[ind+1]);
          vector<BinnedPlayerMove> bpmv;
          for(int j=0; j < bpmv_ld; j++){
            bpmv.emplace_back(static_cast<BinnedPlayerMove>(std::stoi(row[ind+2+j])));
          }
          ind += 2 + bpmv_ld;
          pHistory[pp] = bpmv;
        }
        rgs.playerHistory = pHistory;

        vector<float> probs;
        for(size_t j=0; j < 4; j++) {   // CHANGE THIS is the number of BinnedPlayerMoves increases!
          probs[j] = std::stof(row[ind+j]);
        }

        map<BinnedPlayerMove, float> probMap;
        probMap[BinnedPlayerMove::Fold] = probs[0];
        probMap[BinnedPlayerMove::Call] = probs[1];
        probMap[BinnedPlayerMove::Raise] = probs[2];
        probMap[BinnedPlayerMove::AllIn] = probs[3];

        tableIn[rgs] = probMap;
      }

      inFile.close();
      CFRTable = tableIn;

    }





    static PlayerMove betAmountToMove(int betAmount, shared_ptr<Table>& info, const shared_ptr<Player>& p) {
      PlayerMove myMove;
      myMove.bet_amount = betAmount;
      myMove.bet_amount = clamp(myMove.bet_amount, 0, p->bankroll);
      if( myMove.bet_amount == 0 ) myMove.move = Move::MOVE_FOLD;
      else if( myMove.bet_amount == p->bankroll ) myMove.move = Move::MOVE_ALLIN;
      else {
        myMove.bet_amount = clamp(myMove.bet_amount, info->minimumBet, p->bankroll);
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

    void MattAI::updateParameters(std::vector<float> thresholds_in) {
        thresholds = thresholds_in;
    }

    Mike::InfoSet Mike::packTableIntoInfoSet(const std::shared_ptr<Table> info, std::shared_ptr<Player> me) {
        InfoSet IS;
        // I am player 0
        shared_ptr<Player> enemy = info->getPlayerByID(1);
        vector<PlayerMove> enyMoves = info->playerMoveMap[enemy];
        vector<Move> binnedEnyMoves;
        for(const auto& v : enyMoves)
          binnedEnyMoves.emplace_back(v.move);
        IS.enemyMoveHistory = binnedEnyMoves;
        IS.handStrength = monteCarloSingleHand(me->hand, info->communityCards, 2, 100);
        return IS;
    }

    PlayerMove Mike::makeMove(std::shared_ptr<Table> info, const shared_ptr<Player> p) {
      auto thisIS = packTableIntoInfoSet(info, p);
      PlayerMove myMove;
      constexpr float enyHandStrengthDefault = 0.52;

      if( enyHandStrengthEstimate < 0.0f) enyHandStrengthEstimate = enyHandStrengthDefault;

      //float addStrengthCall = -0.05;
      //float addStrengthRaise = 0.3;
      //float addStrengthAllIn = 0.5;
      float addStrengthCall = addStrengths[0];
      float addStrengthRaise = addStrengths[1];
      float addStrengthAllIn = addStrengths[2];
      auto updateHS = [&] (float addStrength ) -> void {
        enyHandStrengthEstimate = clamp(enyHandStrengthEstimate+addStrength, 0.0f, 1.0f);
      };
      Move enyLastMove;
      if( !thisIS.enemyMoveHistory.empty() ) {
        enyLastMove = thisIS.enemyMoveHistory.back();
        if( enyLastMove == Move::MOVE_CALL)
          updateHS(addStrengthCall);
        else if( enyLastMove == Move::MOVE_FOLD)
          myMove.move = Move::MOVE_CALL;
        else if( enyLastMove == Move::MOVE_RAISE)
          updateHS(addStrengthRaise);
        else if( enyLastMove == Move::MOVE_ALLIN)
          updateHS(addStrengthAllIn);

      }

      float advantageEstimate = std::get<0>(thisIS.handStrength) - enyHandStrengthEstimate;

      //float foldCallThres = -0.15;
      //float callRaiseThres = 0.15;
      //float raiseAllInThres = 0.25;
      float foldCallThres = thresholds[0];
      float callRaiseThres = thresholds[1];
      float raiseAllInThres = thresholds[2];
      if( advantageEstimate < foldCallThres )
        myMove.move = Move::MOVE_FOLD;
      else if( advantageEstimate < callRaiseThres )
        myMove.move = Move::MOVE_CALL;
      else if( advantageEstimate < raiseAllInThres )
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
    void Mike::updateParameters(std::vector<float> in) {
        // Strengths
        auto strengthsIn = std::vector<float>(in.begin(), in.begin()+3);
        for(auto& s : strengthsIn)
          s = tanh(s); // put in range (-1,1)
        std::sort(strengthsIn.begin(), strengthsIn.end());
        this->addStrengths = strengthsIn;

        // Thresholds
        auto thresholdsIn = std::vector<float>(in.begin()+3, in.begin()+6);
        for(auto& s : thresholdsIn)
          s = 0.5*(1.0+tanh(s)); // put in range (0,1)
        std::sort(thresholdsIn.begin(), thresholdsIn.end());
        // normalize it
        float tot = std::accumulate(thresholdsIn.begin(), thresholdsIn.end(), 0.0f);
        for(auto& s : thresholdsIn)
          s /= tot;
        this->thresholds   = thresholdsIn;
    }

    KillBot::InfoSet KillBot::packTableIntoInfoSet(std::shared_ptr<Table> info, const shared_ptr<Player> me) {
      InfoSet is;
      auto [avg, sigma] = monteCarloSingleHand(me->hand, info->communityCards, 1, 100);
      float probWin = avg;
      handEstimateUncertainty = sigma;
      is.handStrength = int( probWin * 3.0);
      auto pOne = info->getPlayerByID(1);
      is.enyMove = pOne->move.move;
      is.street = info->street;
      return is;
    }

    PlayerMove KillBot::makeMove(std::shared_ptr<Table> info, const shared_ptr<Player> me) {
      if( info->street == 0) {
        InfoSetsToUpdate.clear();
        endRoundUtility = 0.0f;
        startingCash = me->bankroll;
      }
      PlayerMove myPMove;
      //  If enemy folded, JUST CALL!
      if( info->getPlayerByID(1)->move.move == Move::MOVE_FOLD) {
        myPMove.move = Move::MOVE_CALL;
        myPMove.bet_amount = info->minimumBet;
        return myPMove;
      }
      auto newInfoSet = packTableIntoInfoSet(info, me);
      map<Move, float> utilityDist;
      auto [updatedIter, ISWasNew] = policy.try_emplace(newInfoSet);
      hitCount[newInfoSet]++;
      // ISWasNew is TRUE is the IS key didn't exist in the CFR table
      // updatedIter is an iterator to the key corresponding to the game state whether or not the insertion took place
      if( ISWasNew ) {
        // make a random move if we don't have a policy
        utilityDist[Move::MOVE_FOLD]  = 0.0;
        utilityDist[Move::MOVE_CALL]  = 0.0;
        utilityDist[Move::MOVE_RAISE] = 0.0;
        policy[newInfoSet] = utilityDist;
      }
      else {
        // policy is already in there
        utilityDist = policy[newInfoSet];
      }

      // Form probability distribution from utility distribution
      
      
      float minel = (*std::min_element(utilityDist.begin(), utilityDist.end(), [](const auto& l, const auto& r) { return l.second < r.second; })).second;
      float maxel = (*std::min_element(utilityDist.begin(), utilityDist.end(), [](const auto& l, const auto& r) { return l.second > r.second; })).second;
      bool allNegative = maxel < 0.0f;
      float possum = 0;

      // trim negatives
      for( auto& pair: utilityDist )
        pair.second = max(pair.second, 0.0f);
      

      // Finally, add a bit to each to explore
      //for( auto& pair : utilityDist )
      //  pair.second += maxel/float(2000);

      //normalize
      normalizeMap(utilityDist);

      //for( auto& pair : utilityDist ) 
      //  pair.second = clamp(pair.second, 0.001f, 0.999f);
      // UtilityDist is now a probability distribution


      // Sample the choices according to the prob dist
      std::uniform_real_distribution<float> dist(0.0f, 1.0f);
      std::random_device rd;
      std::mt19937 rng(rd());
      float choice = dist(rng);
      float partialSum = 0.0f;
      Move myMove;
      for( auto& pair : utilityDist ) {
        partialSum += pair.second;
        if(partialSum >= choice) {
          myMove = pair.first;
          break;
        } 
      }
      // Add the move and infoset to the update list
      InfoSetsToUpdate[newInfoSet] = myMove;
      hitCount[newInfoSet]++;
      numIterations++;

      // calculate bet amount and GO GO GO
      if( myMove == Move::MOVE_FOLD) myPMove.bet_amount = 0;
      else if( myMove == Move::MOVE_CALL) myPMove.bet_amount = info->minimumBet;
      else if( myMove == Move::MOVE_RAISE) myPMove.bet_amount = info->minimumBet * 2;
      else if( myMove == Move::MOVE_ALLIN) myPMove.bet_amount = me->bankroll;
      myPMove.bet_amount = clamp(myPMove.bet_amount, 0, me->bankroll);
      if( myPMove.bet_amount == me->bankroll) myMove = Move::MOVE_ALLIN;
      myPMove.move = myMove;

      return myPMove;
    }

    void KillBot::callback(const shared_ptr<Table> info, const shared_ptr<Player> me) {
        // Weights backpropagation
        endRoundUtility = float(me->bankroll - startingCash) / float(startingCash);
        params.utilityScale = 1.0;
        params.decayRate = 0.0;
        //if( endRoundUtility < 0.0f) endRoundUtility *= 2.0;
        float utilityScale = params.utilityScale * float(endRoundUtility);
        float decayRate = params.decayRate;
        // downplay corrections if the uncertainty in hand estimation was high
        //if( handEstimateUncertainty > 0.10 ) utilityScale *= 0.1;
        float t = numIterations;
        for(auto p = InfoSetsToUpdate.begin(); p != InfoSetsToUpdate.end(); p++) {
          auto& pair = *p;
          auto& MoveUtilities = policy[pair.first];
          //float updateFac = 1.0;
          MoveUtilities[pair.second] += numRounds *  float(endRoundUtility);
          policy[pair.first] = MoveUtilities;
          if( !utility[pair.first])
            utility[pair.first] = 0.0f;
          utility[pair.first] += endRoundUtility;
        }

        numRounds++;
        if (endRoundUtility > 0.0f) numWins++;
    }

    void KillBot::updateParameters(std::vector<float> in) {
      if( !in.empty() ) {
        params.decayRate = in[0];
        params.utilityScale = in[1];
      }
    }

    void KillBot::savePolicy(std::string outfile) {
        std::ofstream outFile(outfile, std::ios_base::out);
        if( !outFile ) throw;

        // Transform a reduced game state to a string
        std::ostringstream oss;
        auto ISintoStream = [&oss] (InfoSet is) {
          oss << static_cast<int>(is.street) << "," << static_cast<int>(is.handStrength) << "," << static_cast<int>(is.enyMove) << ",";
        };
        auto MapIntoStream = [&oss] (map<Move, float> A) {
          for( const auto& a : A) 
            oss << static_cast<int>(a.first) << "," << a.second << ",";
        };

        for(auto& pair : policy) {
          ISintoStream(pair.first);
          MapIntoStream(pair.second);
          oss << std::endl;
          outFile << oss.str();
        }
      outFile.close();
    }
    void KillBot::loadPolicy(std::string infile) {
        std::ifstream inFile(infile);
        if( !inFile ) throw;
        std::vector<std::vector<std::string>> data;
        std::string line;
        while(std::getline(inFile, line)) {
            std::vector<std::string> row;
            std::stringstream ss(line);
            std::string field;
            while (std::getline(ss, field, ',')) {
            row.push_back(field);
            }
            data.push_back(row);
        }
        policy.clear();
        for(auto& row : data) {
          InfoSet key;
          key.street = std::stoi(row[0]);
          key.handStrength = std::stoi(row[1]);
          key.enyMove = static_cast<Move>(std::stoi(row[2]));
          map<Move, float> value;
          value[static_cast<Move>(std::stoi(row[3]))] = std::stof(row[4]);
          value[static_cast<Move>(std::stoi(row[5]))] = std::stof(row[6]);
          value[static_cast<Move>(std::stoi(row[7]))] = std::stof(row[8]);
          policy[key] = value;
        }
    }


}