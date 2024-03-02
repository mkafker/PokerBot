#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <iterator>

#include "table.h"
#include "showdown.h"
namespace Poker{
    enum class Move : int {
        MOVE_UNDEF = -1,
        MOVE_FOLD = 0,
        MOVE_CALL = 1,
        MOVE_RAISE = 2,
        MOVE_ALLIN = 3
    };
    struct PlayerMove {
        Move move   = Move::MOVE_UNDEF;
        int bet_amount    = -1;
    };

    enum class PlayerPosition : int {
        POS_UTG = 0,
        POS_UTG1 = 1,
        POS_UTG2 = 2,
        POS_UTG3 = 3,
        POS_HJ   = 4,
        POS_CO   = 5,
        POS_BTN  = 6,
        POS_SB   = 7,
        POS_BB   = 8
    };

    static std::unordered_map<PlayerPosition, std::string> PlayerPosition_to_String {
        {PlayerPosition::POS_UTG     , "UTG"},
        {PlayerPosition::POS_UTG1     , "UTG1"},
        {PlayerPosition::POS_UTG2     , "UTG2"},
        {PlayerPosition::POS_UTG3     , "UTG3"},
        {PlayerPosition::POS_BB     , "BB"},
        {PlayerPosition::POS_SB     , "SB"},
        {PlayerPosition::POS_CO     , "CO"},
        {PlayerPosition::POS_HJ     , "HJ"},
        {PlayerPosition::POS_BTN    , "BTN"}
    };
    static std::vector<PlayerPosition> numPlayersToPositionList (int n) {
        std::vector<PlayerPosition> ret;
        ret.emplace_back(PlayerPosition::POS_BTN);
        ret.emplace_back(PlayerPosition::POS_BB);
        for(int j = n; j > 0; j--) {
            if( j == 9 )            ret.emplace_back(PlayerPosition::POS_UTG3);
            else if( n == 8 )       ret.emplace_back(PlayerPosition::POS_UTG2);
            else if( n == 7 )       ret.emplace_back(PlayerPosition::POS_HJ);
            else if( n == 6 )       ret.emplace_back(PlayerPosition::POS_UTG1);
            else if( n == 5 )       ret.emplace_back(PlayerPosition::POS_CO);
            else if( n == 4 )       ret.emplace_back(PlayerPosition::POS_UTG);
            else if( n == 3 )       ret.emplace_back(PlayerPosition::POS_SB);            
        }

        return ret;
    };


    class Player {
        public:
            int  bankroll;
            int  playerID;                    // 
            Hand hand;           // the two-card hand
            PlayerPosition  position;                      // seat the player is on. 0 = UTG, 1 = MP, 2 = CO, 3 = BTN, 4 = SB, 5 = BB 
            PlayerMove  move;
            FullHandRank FHR;
            
            Player(int p);
            Player(PlayerPosition p);
            Player(Hand h);
            Player();

            PlayerPosition getPosition() const { return this->position; };
            void setPosition(int& pos) { this->position = static_cast<PlayerPosition>(pos); }
            void setPosition(PlayerPosition &p) { this->position = p; }
            void incPosition();
            PlayerMove makeAIMove(std::shared_ptr<Table> info);
            void printPlayerMove(PlayerMove move);
            void resetHand();
            inline bool isBankrupt();
    };

    inline std::ostream& operator<<(std::ostream& stream, PlayerPosition &a) { 
        stream << PlayerPosition_to_String[a];
        return stream;
    }

    
}

