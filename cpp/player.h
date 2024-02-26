#pragma once
#include <vector>
#include <memory>
#include <algorithm>

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
        POS_MP  = 1,
        POS_CO  = 2,
        POS_BTN = 3,
        POS_SB  = 4,
        POS_BB  = 5
    };

    static std::unordered_map<PlayerPosition, std::string> PlayerPosition_to_String {
        {PlayerPosition::POS_UTG     , "UTG"},
        {PlayerPosition::POS_BB     , "BB"},
        {PlayerPosition::POS_SB     , "SB"},
        {PlayerPosition::POS_CO     , "CO"},
        {PlayerPosition::POS_MP     , "MP"},
        {PlayerPosition::POS_BTN    , "BTN"}
    };


    class Player {
        public:
            int  bankroll;
            int  playerID;                    // 
            Hand hand;           // the two-card hand
            PlayerPosition  position;                      // seat the player is on. 0 = UTG, 1 = MP, 2 = CO, 3 = BTN, 4 = SB, 5 = BB 
            PlayerMove  move;
            
            Player();
            Player(int p);
            Player(PlayerPosition p);
            Player(Hand h);

            PlayerPosition getPosition() const;
            void incPosition();
            PlayerMove makeAIMove(std::shared_ptr<Table> info);
            void printPlayerMove(PlayerMove move);
            void resetHand();
    };

    inline std::ostream& operator<<(std::ostream& stream, PlayerPosition &a) { 
        stream << PlayerPosition_to_String[a];
        return stream;
    }

    
}

