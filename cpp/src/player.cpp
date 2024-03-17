#include "player.h"
#include <random>

namespace Poker{
    Player::Player(PlayerPosition p) : position { p } {};
    Player::Player(int p)  { playerID = p; };
    Player::Player(int pos, int pID) { position = static_cast<PlayerPosition>(pos); playerID = pID; }
    
    PlayerMove Strategy::makeMove(std::shared_ptr<Table> info, const shared_ptr<Player> p) {
        // Default behavior: Always call
        // p = contains last move info
        auto clamp = [](int a, int b, int c) -> int { if(a<b) a=b; if(a>c) a=c; return a;};
        
        PlayerMove myMove;
        myMove.move = Move::MOVE_CALL;
        int callAmount = info->minimumBet - p->move.bet_amount; // Amount required to meet the minimumBet
        myMove.bet_amount = clamp(callAmount, 0, p->bankroll);
        if( myMove.bet_amount == p->bankroll) myMove.move = Move::MOVE_ALLIN;
        return myMove;
    }   

    PlayerMove SingleMoveCallAI::makeMove(std::shared_ptr<Table> info, const shared_ptr<Player> p) {
        auto clamp = [](int a, int b, int c) -> int { if(a<b) a=b; if(a>c) a=c; return a;};
        
        PlayerMove myMove;
        myMove.move = Move::MOVE_CALL;
        int callAmount = info->minimumBet - p->move.bet_amount; // Amount required to meet the minimumBet
        myMove.bet_amount = clamp(callAmount, 0, p->bankroll);
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
        int callAmount = info->minimumBet - p->move.bet_amount; // Amount required to meet the minimumBet
        if( myMove.move == Move::MOVE_FOLD) myMove.bet_amount = 0;
        else if( myMove.move == Move::MOVE_CALL) myMove.bet_amount = callAmount;
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
        int callAmount = info->minimumBet - p->move.bet_amount; // Amount required to meet the minimumBet
        if( myMove.move == Move::MOVE_FOLD) myMove.bet_amount = 0;
        if( myMove.move == Move::MOVE_CALL) myMove.bet_amount = callAmount;
        if( myMove.move == Move::MOVE_ALLIN) myMove.bet_amount = p->bankroll;
        myMove.bet_amount = clamp(callAmount, 0, p->bankroll);
        if( myMove.bet_amount == p->bankroll) myMove.move = Move::MOVE_ALLIN;
        return myMove;
    }
    
    PlayerMove Player::makeMove(shared_ptr<Table> info) {
        if ( not strategy ) 
            strategy = make_unique<Strategy>();
        PlayerMove move = strategy->makeMove(info, make_shared<Player>(*this));
        this->move = move;
        return move;
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
        std::cout << pastTenseMap[move] << pmove.bet_amount << " (bank: " << player.bankroll << ")" << std::endl;
    }

    inline bool Player::isBankrupt() { return this->bankroll <= 0; }
    void Player::resetHand() { this->hand.clear(); }
    
}
