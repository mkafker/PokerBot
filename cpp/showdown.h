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
            HandRank                rank;           // the hand rank, as in the enum above
            std::vector<Card>       cards;          // array of cards, has between 2 and 7 cards
            size_t                  num_cards;      // number of cards
            Hand() {
                rank = HandRank::UNDEF_HANDRANK;
                num_cards = 0;
            }
            Hand(std::vector<Card> cards_in) {
                cards = cards_in;
                num_cards = cards_in.size();
                //num_cards = std::count_if(cards.begin(), cards.end(), [](Card x){ x.get_rank_as_int() != -1;});
                sort_cards();
                rank = get_hand_rank();
            }

            Card* get_high_card() { 
                auto ret = std::max(cards.begin(), cards.end());
                return &*ret;
            }

            void sort_cards() {
                // Sort hand by card rank
                std::sort(cards.begin(), cards.end(), [](Card a, Card b) { return a>b; });
            }
            HandRank get_hand_rank() {
                sort_cards();
                std::unordered_map<Rank, size_t> rank_count;
                std::unordered_map<Suit, size_t> suit_count;

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
                
                
                if ( is_straight_flush )    return HandRank::STRAIGHT_FLUSH;
                if ( is_four_kind )         return HandRank::FOUR_KIND;
                if ( is_full_house )        return HandRank::FULL_HOUSE;
                if ( is_flush )             return HandRank::FLUSH;
                if ( is_straight )          return HandRank::STRAIGHT;
                if ( is_three_kind )        return HandRank::THREE_KIND;
                if ( is_two_pair )          return HandRank::TWO_PAIR;
                if ( is_pair )              return HandRank::ONE_PAIR;
                return HandRank::HIGH_CARD;
            }

        Hand* showdown(std::list<Hand*> hands) {
            std::list<Hand*>::iterator it = hands.begin();
            std::list<Hand*> topHands;
            auto handWinner = it;
            auto rankA = (*it)->rank;
            auto rankB = (*handWinner)->rank;
            while(it != hands.end()) {
                rankA = (*it)->rank;
                if( rankA > rankB ) {
                    handWinner = it;
                    topHands.erase(topHands.begin(), topHands.end());
                    topHands.push_front(*handWinner);
                }
                else if( rankA == rankB ) topHands.push_front(*handWinner);
            }

            if( topHands.size() == 1) {return topHands.front(); } // return if no kicker required


            // sort cards
            auto topit = topHands.begin();
            while( topit != topHands.end() ) {
                (*topit)->sort_cards();
            }
            topit = topHands.begin();
            auto winner = topit;            
            auto winnerCardVal = (*winner)->get_high_card();

            while( topit != topHands.end()) {
                auto cardVal = (*topit)->get_high_card();
                if( cardVal->get_rank() > winnerCardVal->get_rank() ) return *topit;
                if( cardVal->get_rank() < winnerCardVal->get_rank() ) return *winner;
            }        
            return nullptr;
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




}

