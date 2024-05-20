#include "player.h"
#include "strategy.h"
#include <random>

namespace Poker{
    
    PlayerMove Player::makeMove(const shared_ptr<Table>& info) {
        if ( ! strategy ) 
            strategy = make_unique<Strategy>();
        PlayerMove move = this->strategy->makeMove(info, this);
        //this->move = move;
        return move;
    }

    void printPlayerMove(const Player& player, const PlayerMove& pmove) {
        auto move = pmove.move;
        std::unordered_map<Move, std::string> strMap {
            {Move::MOVE_CALL, "call "},
            {Move::MOVE_FOLD, "fold "},
            {Move::MOVE_RAISE, "raise "},
            {Move::MOVE_ALLIN, "all in "},
            {Move::MOVE_UNDEF, "undef! "}
        };
        std::cout << "Player " << player.playerID << " (" << PlayerPosition_to_String[player.position] << ") "  << ": ";
        std::vector<Card> pHand = player.hand;
        std::cout << pHand;
        std::cout << strMap[move] << pmove.bet_amount << " (bank: " << player.bankroll << ")" << std::endl;
    }

    inline bool Player::isBankrupt() { return this->bankroll <= 0; }
    void Player::resetHand() { this->hand.clear(); }
    
}
