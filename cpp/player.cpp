#include "player.h"
#include <random>

namespace Poker{

    Player::Player() { position = PlayerPosition::POS_UTG; };
    Player::Player(PlayerPosition p) : position { p } {};
    Player::Player(int p)  { playerID = p; position = static_cast<PlayerPosition>(p); };
    Player::Player(Hand h) : hand { h } {};

    PlayerPosition Player::getPosition() const {
        return position;
    }
    PlayerMove Player::makeAIMove(std::shared_ptr<Table> info) {
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
        if( myMove.move == Move::MOVE_CALL) myMove.bet_amount = info->currentBet;
        if( myMove.move == Move::MOVE_RAISE) myMove.bet_amount = info->currentBet * 2;
        if( myMove.move == Move::MOVE_ALLIN) myMove.bet_amount = this->bankroll;
        myMove.bet_amount = clamp(myMove.bet_amount, 0, this->bankroll);
        if( myMove.bet_amount == this->bankroll) myMove.move = Move::MOVE_ALLIN;
        return myMove;
    }

    void Player::incPosition() {
        this->position = static_cast<PlayerPosition>(static_cast<int>(this->position) + 1);
    }

    void Player::printPlayerMove(PlayerMove pmove) {
        auto move = pmove.move;
        std::unordered_map<Move, std::string> pastTenseMap {
            {Move::MOVE_CALL, "called "},
            {Move::MOVE_FOLD, "folded "},
            {Move::MOVE_RAISE, "raised by "},
            {Move::MOVE_ALLIN, "is all in "},
            {Move::MOVE_UNDEF, "undef!!!!!!"}
        };
        std::cout << "Player " << playerID << " " << PlayerPosition_to_String[this->position] << ": ";
        this->hand.print();
        std::cout << pastTenseMap[move] << pmove.bet_amount << " (prev. bank: " << this->bankroll << ")" << std::endl;
    }

    void Player::resetHand() {
        this->hand.clear();
    }
    
}