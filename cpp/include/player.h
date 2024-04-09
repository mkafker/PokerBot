#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <iterator>

#include "table.h"
#include "showdown.h"
#include "strategy.h"
namespace Poker{
    struct Strategy;
    class Table;

    enum class Move : int {
        MOVE_UNDEF = -1,
        MOVE_FOLD = 0,
        MOVE_CALL = 1,
        MOVE_RAISE = 2,
        MOVE_ALLIN = 3
    };
    struct PlayerMove {
        Move move;
        int bet_amount;
        PlayerMove() { move = Move::MOVE_UNDEF; bet_amount = 0;}
        PlayerMove(Move m, int b) { bet_amount=b; move=m; }
    };

    enum class PlayerPosition : int {
        POS_UTG  = 0,
        POS_UTG1 = 1,
        POS_UTG2 = 2,
        POS_UTG3 = 3,
        POS_HJ   = 4,
        POS_CO   = 5,
        POS_BTN  = 6,
        POS_SB   = 7,
        POS_BB   = 8,
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

    static std::vector<PlayerPosition> numPlayersToPositionList (const int n) {
        // Returns a lits of player positions in BETTING ORDER
        // For two player games, SB is also BTN,
        // but we will just call him the SB
        std::vector<PlayerPosition> ret;
        typedef PlayerPosition PP;
        switch (n) {
            case 2:
                ret.emplace_back(PP::POS_SB);
                ret.emplace_back(PP::POS_BB);
                break;
            case 3:
                ret.emplace_back(PP::POS_BTN);
                ret.emplace_back(PP::POS_SB);
                ret.emplace_back(PP::POS_BB);
                break;
            case 4:
                ret.emplace_back(PP::POS_UTG);
                ret.emplace_back(PP::POS_BTN);
                ret.emplace_back(PP::POS_SB);
                ret.emplace_back(PP::POS_BB);
                break;
            case 5:
                ret.emplace_back(PP::POS_UTG);
                ret.emplace_back(PP::POS_CO);
                ret.emplace_back(PP::POS_BTN);
                ret.emplace_back(PP::POS_SB);
                ret.emplace_back(PP::POS_BB);
                break;
            case 6:
                ret.emplace_back(PP::POS_UTG);
                ret.emplace_back(PP::POS_UTG1);
                ret.emplace_back(PP::POS_CO);
                ret.emplace_back(PP::POS_BTN);
                ret.emplace_back(PP::POS_SB);
                ret.emplace_back(PP::POS_BB);
                break;
            case 7:
                ret.emplace_back(PP::POS_UTG);
                ret.emplace_back(PP::POS_UTG1);
                ret.emplace_back(PP::POS_HJ);
                ret.emplace_back(PP::POS_CO);
                ret.emplace_back(PP::POS_BTN);
                ret.emplace_back(PP::POS_SB);
                ret.emplace_back(PP::POS_BB);
                break;
            case 8:
                ret.emplace_back(PP::POS_UTG);
                ret.emplace_back(PP::POS_UTG1);
                ret.emplace_back(PP::POS_UTG2);
                ret.emplace_back(PP::POS_HJ);
                ret.emplace_back(PP::POS_CO);
                ret.emplace_back(PP::POS_BTN);
                ret.emplace_back(PP::POS_SB);
                ret.emplace_back(PP::POS_BB);
                break;
        }
        return ret;
    };


    class Player {
        public:
            int  bankroll;
            int  playerID;                    // 
            PlayerPosition  position;                      // seat the player is on. 0 = UTG, 1 = MP, 2 = CO, 3 = BTN, 4 = SB, 5 = BB 
            PlayerMove  move;
            FullHandRank FHR;
            std::vector<Card> hand;
            std::shared_ptr<Strategy> strategy;
            
            Player() = default;
            Player(const Player& p) = default;
            Player(Player&& p) = default;
            ~Player() = default;
            Player(int p);
            Player(PlayerPosition p);
            Player(int p, int playerID);

            const PlayerPosition getPosition() const { return this->position; };
            void setPosition(const int& pos) { this->position = static_cast<PlayerPosition>(pos); }
            void setPosition(const PlayerPosition &p) { this->position = p; }
            const int getPlayerID() const { return this->playerID; }
            void setPlayerID(const int& id) { this->playerID = id; }
            void resetHand();
            inline bool isBankrupt();
            PlayerMove makeMove(std::shared_ptr<Table> tableInfo);
    };

    
    void printPlayerMove(const Player& player, const PlayerMove& move);


    inline std::ostream& operator<<(std::ostream& stream, PlayerPosition &a) { 
        stream << PlayerPosition_to_String[a];
        return stream;
    }

    inline std::ostream& operator<<(std::ostream& stream, Player &a) { 
        stream << "Player " << a.playerID << " (" <<  PlayerPosition_to_String[a.position] << ")";
        return stream;
    }

    inline std::ostream& operator<<(std::ostream& stream, Move &a) { 
                if( a == Move::MOVE_ALLIN ) stream << "A";
        else    if( a == Move::MOVE_CALL  ) stream << "C";
        else    if( a == Move::MOVE_FOLD  ) stream << "F";
        else    if( a == Move::MOVE_RAISE ) stream << "R";
        else    if( a == Move::MOVE_UNDEF ) stream << "U";
        return stream;
    }
}

