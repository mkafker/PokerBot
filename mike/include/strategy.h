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
            virtual void callback( const std::shared_ptr<Table>, const shared_ptr<Player>) {};
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
                return this->handrank == other.handrank && this->maincard == other.maincard;
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
    template <typename T>
    static void hashCombine(size_t& seed, const T& v) {
        hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
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

    };

    template <typename T>
    static void hashCombine(size_t& seed, const T& v) {
        hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

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
            void callback() {
                std::cout << "addStrs: " << addStrengths[0] << " " << addStrengths[1] << " " << addStrengths[2] << std::endl;
                std::cout << "thres: " << thresholds[0] << " " << thresholds[1] << " " << thresholds[2] << std::endl;
            }
    };


    struct KillBot : public Strategy {
        public:
            using Strategy::Strategy;
            PlayerMove makeMove(std::shared_ptr<Table> info, const shared_ptr<Player>) override;
            struct InfoSet {
                //std::vector<Move> enemyMoveHistory;
                int handStrength;
                Move enyMove;
                int street;
                bool operator==(const InfoSet& other) const {
                    return handStrength == other.handStrength && enyMove == other.enyMove && street == other.street;
                }
            };
            struct ISHash {
                size_t operator()(const InfoSet& is) const {
                    size_t hash = 0;
                    hashCombine(hash, is.handStrength);
                    hashCombine(hash, static_cast<int>(is.enyMove));
                    hashCombine(hash, is.street);
                    return hash;
                }
            };
            struct ModelParameters {
                float decayRate = -0.001;
                float utilityScale = 0.01;
            };
            // for call bot: decay -0.001, utilityScale 0.02

            //void updateParameters(std::vector<float>) override;
            InfoSet packTableIntoInfoSet(const std::shared_ptr<Table> info, const shared_ptr<Player> me);
            unordered_map<InfoSet, map<Move, float>, ISHash> policy;        // value is a map between moves and utility
            unordered_map<InfoSet, size_t, ISHash> hitCount;
            size_t numRounds = 0;
            size_t numWins   = 0;
            float endRoundUtility = 0.0;
            int startingCash = 0;
            float handEstimateUncertainty = 0.0f; // used in backprop
            unordered_map<InfoSet, Move, ISHash> InfoSetsToUpdate;
            unordered_map<InfoSet, float, ISHash> utility;
            ModelParameters params;
            void callback(const std::shared_ptr<Table> info, const shared_ptr<Player> me) override;
            void updateParameters(std::vector<float>) override;
            void normalizeMap(map<Move, float>& in) {
                size_t numZeros = 0;
                float t = 0.0;
                for(auto& pair : in)  {
                    t += pair.second;
                    if( pair.second == 0.0f) numZeros++;
                }
                if( numZeros == in.size() ) {
                    for(auto& pair : in)
                        pair.second += 1.0f;
                    t = float(in.size());
                }
                for(auto& pair : in) 
                pair.second /= t;
            };
            void reset() {
                numRounds = 0;
                numWins = 0;
                policy.clear();
                hitCount.clear();
                endRoundUtility = 0.0;
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