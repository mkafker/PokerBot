#pragma once
#include <unordered_map>
#include <algorithm>
#include <list>
#include "card.h"

namespace Poker {
    enum class WinState : int {
        WINSTATE_UNDEF = -9999,
        WINSTATE_A = -1,
        WINSTATE_B = 1,
        WINSTATE_DRAW
    };
    enum class HandRank : int {
        UNDEF_HANDRANK = 0,
        HIGH_CARD = 1,
        ONE_PAIR = 2,
        TWO_PAIR = 3,
        THREE_KIND = 4,
        STRAIGHT = 5,
        FLUSH = 6,
        FULL_HOUSE = 7,
        FOUR_KIND = 8, 
        STRAIGHT_FLUSH = 9
    };
    struct FullHandRank {                   // example: A A A K K 3 2
        HandRank handrank;                  // full house
        std::vector<Rank> maincards;        // {A, K}
        std::vector<Rank> kickers;          // {3, 2}
    };
    static std::unordered_map<HandRank, std::string> HandRank_to_String {
        {HandRank::UNDEF_HANDRANK, "UNDEF_Hand"},
        {HandRank::HIGH_CARD, "High card"},
        {HandRank::ONE_PAIR,  "One pair"},
        {HandRank::TWO_PAIR,  "Two pair"},
        {HandRank::THREE_KIND,    "Three of a kind"},
        {HandRank::STRAIGHT,       "Straight"},
        {HandRank::FLUSH,          "Flush"},
        {HandRank::FULL_HOUSE,     "Full house"},
        {HandRank::FOUR_KIND,      "Four of a kind"},
        {HandRank::STRAIGHT_FLUSH, "Straight flush"}
    };
    class Hand {
        public:
            std::vector<Card>       cards;          // array of cards, has between 2 and 7 cards
            size_t                  numCards;      // number of cards
            FullHandRank            rank;           // full hand rank information            
            Hand() {
                numCards = 0;
            }
            Hand(const Hand&) {};
            Hand(std::vector<Card> cards_in) {
                cards = cards_in;
                numCards = cards_in.size();
                sortCards();
            }
            Card* get_high_card() { 
                auto ret = std::max(cards.begin(), cards.end());
                return &*ret;
            }
            void sortCards() {
                // Sort hand by card rank
                std::sort(cards.begin(), cards.end(), [](Card a, Card b) { return a>b; });
            }
            FullHandRank getFullHandRank() const { return rank; }

            static FullHandRank calcFullHandRank(Hand* hand) {
                FullHandRank ret;
                hand->sortCards();
                const size_t num_cards = hand->numCards;
                const auto   cards     = hand->cards;
                std::unordered_map<Rank, size_t> rank_count;
                std::unordered_map<Suit, size_t> suit_count;
                // todo: redo this!!!!!
                bool is_straight_flush = false;
                bool is_four_kind = false;
                bool is_full_house = false;
                bool is_flush = false;
                bool is_straight = false;
                bool is_three_kind = false;
                bool is_two_pair   = false;
                bool is_pair       = false;
                
                int straight_cnt = 1;
                for(int i=0; i<num_cards; i++) {
                    //std::cout << cards[i] << std::endl;
                    rank_count[cards[i].get_rank()]++;
                    suit_count[cards[i].get_suit()]++;
                    if( cards[i].get_rank_as_int() == cards[i+1].get_rank_as_int()+1) {
                        straight_cnt++;
                        if (straight_cnt == 5) {
                            is_straight = true;
                        }
                    }
                    else {
                        straight_cnt = 0;
                    }
                }
                
                size_t num_unique_suits = suit_count.size();
                size_t num_unique_ranks = rank_count.size();
                for(const auto& suit_count_pair : suit_count) {
                    if (suit_count_pair.second >= 5) {
                        is_flush = true;
                    }
                }

                if (is_straight && is_flush) is_straight_flush = true;
                
                int three_count = 0; //durrrr
                int two_count   = 0;
                for(const auto& rank_count_pair : rank_count) {
                    // Look for four of a kind
                    if (rank_count_pair.second == 4) {
                        is_four_kind = true;
                    }
                    // Look for full house or two pair
                    if (rank_count_pair.second == 3) {
                        is_three_kind = true;
                    }
                    if (rank_count_pair.second == 2) {
                        two_count++;
                    }
                }
                if ( two_count >= 2 ) is_two_pair = true;
                if ( two_count == 1 ) is_pair = true;
                if ( (is_three_kind && is_pair) || (is_three_kind && is_two_pair) ) is_full_house = true;
                
                if ( is_straight_flush )         ret.handrank =  HandRank::STRAIGHT_FLUSH;
                else if ( is_four_kind )         ret.handrank =  HandRank::FOUR_KIND;
                else if ( is_full_house )        ret.handrank =  HandRank::FULL_HOUSE;
                else if ( is_flush )             ret.handrank =  HandRank::FLUSH;
                else if ( is_straight )          ret.handrank =  HandRank::STRAIGHT;
                else if ( is_three_kind )        ret.handrank =  HandRank::THREE_KIND;
                else if ( is_two_pair )          ret.handrank =  HandRank::TWO_PAIR;
                else if ( is_pair )              ret.handrank =  HandRank::ONE_PAIR;
                else                             ret.handrank = HandRank::HIGH_CARD;

                // get list of unique cards
                std::transform(cards.begin(), cards.end(), std::back_inserter(ret.maincards), [](const Card& a){ return a.get_rank(); });

                ret.maincards.erase(std::unique(ret.maincards.begin(), ret.maincards.end()));

                return ret;
                
            }

        static Hand* showdown(const std::list<Hand*>& hands) {
            if (hands.empty()) 
                return nullptr;
            // calculate full hand rank for each hand
            std::for_each(hands.begin(), hands.end(), [](Hand* h) { h->rank = calcFullHandRank(h); });

            auto handComparator = [](const Hand* a, const Hand* b) -> bool {
                // returns true if A is a weaker hand than B
                FullHandRank fhrA = a->getFullHandRank();
                FullHandRank fhrB = b->getFullHandRank();
                if( fhrA.handrank < fhrB.handrank ) return true;
                if( fhrA.handrank > fhrB.handrank ) return false;
                // fix fix fi xfix 
                if( fhrA.maincards.size() != fhrB.maincards.size() ) throw;
                for(int i = 0; i < fhrA.maincards.size(); i++ ) {
                    return fhrAmaincards[i] < fhrB.maincards[i];
                }
            };
            // determine best hand by HandRank (not card values)
            auto handWinner = std::max_element(hands.begin(), hands.end(), handComparator);

            // get all equivalent Hands
            std::list<Hand*> topHands;
            for(const auto& h : hands) {
                const auto t = h;
                const auto w = *handWinner;
                if( handComparator(t,w) == handComparator(w,t) ) {
                    topHands.emplace_back(t);
                }
            }
            // if theres a clear winner, return
            if( std::distance(topHands.begin(), topHands.end()) == 1) return topHands.front();
            
            // draw
            return nullptr;
        }

        Card& operator[](int index) {
            return cards[index];
        }
        const Card& operator[](int index) const {
            return cards[index];
        }
        void clear() {
            cards.clear();
        }
        void add(Card c) {
            cards.push_back(c);
        }
        void append(std::vector<Poker::Card> h) {
            cards.insert(cards.end(), h.begin(), h.end());
        }
        void print() {
            for( Card& c : cards ) std::cout << c << " ";
        }
    };
    
    
    


    inline std::ostream& operator<<(std::ostream& stream, Hand &a) { 
        for( auto& c : a.cards) {
            stream << c << " ";
        }
        return stream;
    }
    inline std::ostream& operator<<(std::ostream& stream, HandRank &a) {
        stream << HandRank_to_String[a];
        return stream;
    }
    inline std::ostream& operator<<(std::ostream& stream, const HandRank &a) {
        stream << HandRank_to_String[a];
        return stream;
    }

    /*
    bool operator==(HandRank lhs, HandRank rhs) {
        return static_cast<int>(lhs) == static_cast<int>(rhs);
    }
    bool operator<(HandRank lhs, HandRank rhs) {
        return static_cast<int>(lhs) < static_cast<int>(rhs);
    }
    bool operator>(HandRank lhs, HandRank rhs) {
        return static_cast<int>(lhs) > static_cast<int>(rhs);
    }
    bool operator!=(HandRank lhs, HandRank rhs) {
        return static_cast<int>(lhs) != static_cast<int>(rhs);
    }
    */


}

