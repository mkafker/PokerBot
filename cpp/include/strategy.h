#pragma once
#include "player.h"
#include "table.h"
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
    class RandomAI : public Strategy {
        public:
            // inherit the constructors from the Strategy class
            using Strategy::Strategy;
            PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
    };
    class SingleMoveCallAI : public Strategy {
        public:
            using Strategy::Strategy;
            PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
    };
    class SequenceMoveAI : public Strategy {
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

    class FHRProportionalAI : public Strategy {
      public:
        using Strategy::Strategy;
        PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
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
}