#pragma once
#include "player.h"
#include "table.h"
#include "showdown.h"
#include "bench.h"
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
            virtual void callback() {};
            virtual void updateParameters(std::vector<float>)  {};
            /*
            template<typename... Args>
                void updateParameters(Args&&... args) {
                    updateParametersImpl(std::forward<Args>(args)...);
                }
            
            template<typename... Args>
                void callback(Args&&... args) {
                    callbackImpl(std::forward<Args>(args)...);
                }
            template<typename... Args>
                void updateParametersImpl(Args...) {};
            template<typename... Args>
                void callbackImpl(Args...) {};
                */
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

    struct MoveAwareAI : public Strategy {
        // follows a strategy based off of the moves of other players
        // only heads up games are supported
        public:
            using Strategy::Strategy;
            PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
            map<vector<Move>, int> moveSequenceToBet;
            vector<Move> enemyMoves;
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
        enum class BinnedPlayerMove : int {
            Undef = -1,
            Fold = 0,
            Call = 1,
            Raise = 2,
            AllIn = 3
        };
        struct BinnedPlayerMoveHash {
            size_t operator()(const BinnedPlayerMove& pp) const {
                return static_cast<size_t>(pp);
            }
        };
        struct PlayerPositionHash {
            size_t operator()(const PlayerPosition& pp) const {
                return static_cast<size_t>(pp);
            }
        };

        struct ReducedGameState {
            // (all players)
            // Position, ReducedHandRank, street, and move history of all players
            PlayerPosition position;
            ReducedFullHandRank RFHR;
            int street;
            unordered_map<PlayerPosition, vector<BinnedPlayerMove>, PlayerPositionHash> playerHistory;

            bool operator==(const ReducedGameState& other) const {
                return position == other.position &&
                        int(RFHR) == int(other.RFHR) &&
                        street == other.street &&
                        playerHistory == other.playerHistory;
            }
        };
        // Need the hash for the unordered_map
        struct RGSHash {
            size_t operator()(const ReducedGameState& rgs) const {
                size_t hash = 0;
                hashCombine(hash, rgs.position);
                hashCombine(hash, int(rgs.RFHR));
                hashCombine(hash, rgs.street);
                for(const auto& pair : rgs.playerHistory) {
                    hashCombine(hash, pair.first);
                    for(const auto& v : pair.second) {
                        hashCombine(hash, v);
                    }
                }
                return hash;
            }
        };

        ReducedGameState packTableIntoReducedGameState(Table table);
        BinnedPlayerMove packBinnedPlayerMove(PlayerMove m);
        PlayerMove unpackBinnedPlayerMove(BinnedPlayerMove m, int minimumBet, int bankroll);
        using Strategy::Strategy;
        PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
        unordered_map<ReducedGameState, map<BinnedPlayerMove, float>, RGSHash> CFRTable;       // Maps between the game state and probability to make each move

        void dumpCFRTableToFile(std::string outfile);
        void loadCFRTableFromFile(std::string infile);

        template <typename T>
        static void hashCombine(size_t& seed, const T& v) {
            hash<T> hasher;
            seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
    };


    struct MattAI : public Strategy {
        public:
            using Strategy::Strategy;
            PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
            std::vector<float> thresholds = {0.2, 0.7, 0.9};
            void updateParameters(std::vector<float>) override;
    };

    struct Mike : public Strategy {
        public:
            using Strategy::Strategy;
            PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
            struct InfoSet {
                std::vector<Move> enemyMoveHistory;
                std::tuple<float, float> handStrength;
            };
            float enyHandStrengthEstimate = -1.0;
            void updateParameters(std::vector<float>) override;
            InfoSet packTableIntoInfoSet(const std::shared_ptr<Table> info, const shared_ptr<Player> me);
            std::vector<float> addStrengths = {-0.05, 0.2, 0.5}; // call, raise, allin
            std::vector<float> thresholds = {-0.15, 0.15, 0.25}; // F/C, C/R, R/A
            void callback() override {
                std::cout << "addStrs: " << addStrengths[0] << " " << addStrengths[1] << " " << addStrengths[2] << std::endl;
                std::cout << "thres: " << thresholds[0] << " " << thresholds[1] << " " << thresholds[2] << std::endl;
            }
    };



    
    inline std::string getAIName(shared_ptr<Strategy> &a) { 
        return "default";
    }
    inline std::string getAIName(shared_ptr<SingleMoveCallAI> &a) { 
        return "call";
    }
    inline std::string getAIName(SequenceMoveAI*& a) { 
        return "sequence";
    }


    static PlayerMove betAmountToMove(int betAmount, shared_ptr<Table>& info, const shared_ptr<Player>& p);

}