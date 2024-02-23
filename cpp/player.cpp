#include "player.h"

namespace Poker{

    Player::Player() { hand.resize(2); position = PlayerPosition::POS_UTG; };
    Player::Player(PlayerPosition p) : position { p } {};
    Player::Player(int p)  { position = static_cast<PlayerPosition>(p); };
    Player::Player(std::vector<Card> h) : hand { h } {};

    PlayerPosition Player::getPosition() const {
        return position;
    }
    PlayerMove Player::makeAIMove(std::shared_ptr<Table> info) {
        int minbet = 0;
        if( this->position == PlayerPosition::POS_BB) minbet = info->get_BB();
        if( this->position == PlayerPosition::POS_SB) minbet = info->get_SB();
        int bet = minbet;

        Hand myhand = Hand(this->hand);
        PlayerMove mymove;
        if ( myhand.rank == HandRank::ONE_PAIR) {
            mymove.move = Move::MOVE_RAISE;
        }
        else {
            mymove.move = Move::MOVE_CALL;
        }
        if( mymove.move == Move::MOVE_RAISE ) bet = bet*2;
        mymove.bet_amount = std::clamp(bet, minbet, bankroll);

        return mymove;
    }

    void Player::incPosition() {
        this->position = static_cast<PlayerPosition>(static_cast<int>(this->position) + 1);
    }
    
}