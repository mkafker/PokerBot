#pragma once
#include "player.h"
#include "table.h"
#include "showdown.h"
#include <map>
#include <memory>
#include <random>
#include <numeric>
using namespace std;

namespace Poker {
    struct PlayerMove;
    enum class Move;
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
            // which relationship to use during different phases of the game
            std::map<int, std::map<HandRank, int>> streetRBR;
    };

    struct FHRAwareAI : public Strategy {
        public:
            using Strategy::Strategy;
            PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
            struct ReducedFullHandRank  {
                HandRank handrank = HandRank::UNDEF_HANDRANK;                  
                Card maincard = Card();        
                bool operator==(const ReducedFullHandRank& other) const {
                    return this->handrank == other.handrank and this->maincard == other.maincard;
                }
                bool operator<(const ReducedFullHandRank& other) const {
                    if( this->handrank > other.handrank) return false;
                    if( this->handrank < other.handrank) return true;
                    if( this->maincard > other.maincard) return false;
                    if( this->maincard < other.maincard) return true;
                    return false;
                }
            };
            static ReducedFullHandRank nullRFHR;

            std::map<ReducedFullHandRank, int> RFHRBetRelationship;
            void updateRFHRBetRelationship(const std::vector<int>& vec_in); // unpacks an input vector of parameters to the RFHRBetRel
    };

    struct MoveAwareAI : public Strategy {
        // follows a strategy based off of the moves of other players
        // only heads up games are supported
        public:
            using Strategy::Strategy;
            PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
            map<vector<Move>, int> moveSequenceToBet;
            vector<Move> enemyMoves;
            void updateMoveSequenceToBet(const vector<int>& vec_in);
    };

    enum class PlayerPosition;

    struct CFRAI1 : public Strategy {
        // Makes a probabilistic move based on a map
        // Define a few structs
        struct ReducedFullHandRank  {
            HandRank handrank = HandRank::UNDEF_HANDRANK;                  
            Card maincard = Card();        
            bool operator==(const ReducedFullHandRank& other) const {
                return this->handrank == other.handrank and this->maincard == other.maincard;
            }
            bool operator<(const ReducedFullHandRank& other) const {
                if( this->handrank > other.handrank) return false;
                if( this->handrank < other.handrank) return true;
                if( this->maincard > other.maincard) return false;
                if( this->maincard < other.maincard) return true;
                return false;
            }
            operator int() const {
                return static_cast<int>(handrank) * 13 + maincard.get_rank_as_int();
            }
        };
        enum BinnedPlayerMove {
            Undef = -1,
            Fold = 0,
            Call = 1,
            Raise = 2,
            AllIn = 3
        };
        struct ReducedGameState {
            // (all players)
            // Position, ReducedHandRank, street, and move history of all players
            PlayerPosition position;
            ReducedFullHandRank RFHR;
            int street;
            map<PlayerPosition, vector<BinnedPlayerMove>> playerHistory;

            bool operator==(const ReducedGameState& other) const {
                return position == other.position &&
                        int(RFHR) == int(other.RFHR) &&
                        street == other.street &&
                        playerHistory == other.playerHistory;
            }
        };
        struct RGSHash {
            size_t operator()(const ReducedGameState& rgs) const {
                size_t PPHash = hash<PlayerPosition>{}(rgs.position);
                size_t RFHRHash = hash<int>{}(int(rgs.RFHR));
                size_t streetHash = hash<int>{}(rgs.street);
                size_t PHHash = hash<map<PlayerPosition, vector<BinnedPlayerMove>>>{}(rgs.playerHistory);
                return PPHash ^ (RFHRHash << 1) ^ (streetHash << 2);
            }
        };

        ReducedGameState packTableIntoReducedGameState(Table table);
        BinnedPlayerMove packBinnedPlayerMove(PlayerMove m);
        PlayerMove unpackBinnedPlayerMove(BinnedPlayerMove m, int minimumBet, int bankroll);
        using Strategy::Strategy;
        PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
        unordered_map<ReducedGameState, map<BinnedPlayerMove, float>, RGSHash> CFRTable;       // Maps between the game state and probability to make each move
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
    inline std::string getAIName(shared_ptr<FHRAwareAI> &a) {
        return "fhraware";
    }


    static PlayerMove betAmountToMove(int betAmount, shared_ptr<Table>& info, const shared_ptr<Player>& p);

}