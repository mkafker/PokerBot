#pragma once
#include "player.h"
#include "table.h"
#include "showdown.h"
#include <map>
#include <memory>
using namespace std;

namespace Poker {
    struct PlayerMove;
    class Table;
    class Player;
    struct Strategy {
        public:
            virtual PlayerMove makeMove(const std::shared_ptr<Table>, const shared_ptr<Player>);
    };
    struct RandomAI : public Strategy {
        public:
            // inherit the constructors from the Strategy struct
            using Strategy::Strategy;
            PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
    };
    struct SingleMoveCallAI : public Strategy {
        public:
            using Strategy::Strategy;
            PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
    };
    struct SequenceMoveAI : public Strategy {
        // Strategy instance that follows a sequence of moves
        public:
            std::vector<PlayerMove> moveList;
            size_t index;
            SequenceMoveAI() : Strategy()  {
                index = 0;
                moveList = vector<PlayerMove>();
            }
            PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
    };

    struct FHRProportionalAI : public Strategy {
      public:
        using Strategy::Strategy;
        PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
    };

    struct HandStreetAwareAI : public Strategy {
        public:
            using Strategy::Strategy;
            PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
            // how many big blinds to bet in each situation
            /*std::map<HandRank, int> rankBetRelationship = {
                {HandRank::HIGH_CARD , 1},
                {HandRank::ONE_PAIR , 2},
                {HandRank::TWO_PAIR , 3},
                {HandRank::THREE_KIND , 4},
                {HandRank::STRAIGHT , 5},
                {HandRank::FLUSH , 6},
                {HandRank::FULL_HOUSE , 7},
                {HandRank::FOUR_KIND , 8}, 
                {HandRank::STRAIGHT_FLUSH , 9}
            };*/
            // which relationship to use during different phases of the game
            std::map<int, std::map<HandRank, int>> streetRBR;
    };

    
    inline std::string getAIName(shared_ptr<Strategy> &a) { 
        return "default";
    }
    inline std::string getAIName(shared_ptr<SingleMoveCallAI> &a) { 
        return "call";
    }
    inline std::string getAIName(shared_ptr<SequenceMoveAI> &a) { 
        return "sequence";
    }
    inline std::string getAIName(shared_ptr<FHRProportionalAI> &a) { 
        return "fhrprop";
    }
    inline std::string getAIName(shared_ptr<HandStreetAwareAI> &a) {
        return "handstreetaware";
    }

}