#include "player.h"
#include <random>

namespace Poker{
    Player::Player(PlayerPosition p) : position { p } {};
    Player::Player(int p)  { playerID = p; };
    Player::Player(int pos, int pID) { position = static_cast<PlayerPosition>(pos); playerID = pID; }

    PlayerMove CallAI::makeMove(std::shared_ptr<Table> info) {
        return Player::makeMove(info);
    }   
    PlayerMove Player::makeMove(std::shared_ptr<Table> info) {
        // Default behavior: Always call
        auto clamp = [](int a, int b, int c) -> int { if(a<b) a=b; if(a>c) a=c; return a;};
        
        PlayerMove myMove;
        myMove.move = Move::MOVE_CALL;
        myMove.bet_amount = info->minimumBet;
        myMove.bet_amount = clamp(myMove.bet_amount, 0, this->bankroll);
        if( myMove.bet_amount == this->bankroll) myMove.move = Move::MOVE_ALLIN;
        return myMove;
    }

    PlayerMove RandomMoveAI::makeMove(std::shared_ptr<Table> info) {
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
        if( myMove.move == Move::MOVE_CALL) myMove.bet_amount = info->minimumBet;
        if( myMove.move == Move::MOVE_RAISE) myMove.bet_amount = info->minimumBet * 2;
        if( myMove.move == Move::MOVE_ALLIN) myMove.bet_amount = this->bankroll;
        myMove.bet_amount = clamp(myMove.bet_amount, 0, this->bankroll);
        if( myMove.bet_amount == this->bankroll) myMove.move = Move::MOVE_ALLIN;
        return myMove;
    }

    void Player::incPosition() {
        // TODO: rework
        this->position = static_cast<PlayerPosition>(static_cast<int>(this->position) + 1 % 6 ) ;
    }

    void printPlayerMove(const Player& player, const PlayerMove& pmove) {
        auto move = pmove.move;
        std::unordered_map<Move, std::string> pastTenseMap {
            {Move::MOVE_CALL, "called "},
            {Move::MOVE_FOLD, "folded "},
            {Move::MOVE_RAISE, "raised by "},
            {Move::MOVE_ALLIN, "is all in "},
            {Move::MOVE_UNDEF, "undef!!!!!!"}
        };
        std::cout << "Player " << player.playerID << " (" << PlayerPosition_to_String[player.position] << ") "  << ": ";
        std::cout << pastTenseMap[move] << pmove.bet_amount << " (prev. bank: " << player.bankroll << ")" << std::endl;
    }

    inline bool Player::isBankrupt() { return this->bankroll <= 0; }
    void Player::resetHand() { this->hand.clear(); }
    
}
