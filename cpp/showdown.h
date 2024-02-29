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
        std::vector<Card> maincards;        // {A, K}
        std::vector<Card> kickers;          // {3, 2}
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
            }
            void sortCards() {
                // Sort hand by card rank
                std::sort(cards.begin(), cards.end(), [](Card a, Card b) { return a<b; });
            }
            void sortCardsDescending() {
                std::sort(cards.begin(), cards.end(), [](Card a, Card b) { return a>b; });
            }
            // this should really be templated
            void sortCards(Hand* h) {
                // Sorts cards in place
                std::sort(h->cards.begin(), h->cards.end(), [](const Card& a, const Card& b) { return a<b; });
            }
            void sortCards(std::vector<Card> h) {
                // Sorts cards in place
                std::sort(h.begin(), h.end(), [](const Card& a, const Card& b) { return a<b; });
            }
            void sortCardsDescending(Hand* h) {
                std::sort(h->cards.begin(), h->cards.end(), [](const Card& a, const Card& b) { return a>b; });
            }
            
            FullHandRank getFullHandRank() const { return rank; }

            static FullHandRank calcFullHandRank(Hand* hand) {
                FullHandRank ret;
                hand->sortCardsDescending();

                const size_t num_cards = hand->numCards;
                auto   cards     = hand->cards;

                std::vector<Card>     winningCards;
                std::vector<Card>     kickerCards;

                // Maps galore!
                std::unordered_map<Rank, std::pair<size_t, std::vector<Card>>> ranks;
                std::unordered_map<Suit, std::pair<size_t, std::vector<Card>>> suits;

                // yuck!

                bool is_straight_flush  = false;
                bool is_four_kind       = false;
                std::vector<Card>      four_kind_cards;
                bool is_full_house      = false;
                std::vector<Card>      full_house_cards;
                bool is_flush           = false;
                std::vector<Card>      flush_cards;
                bool is_straight        = false;
                std::vector<Card>      straight_cards;
                bool is_three_kind      = false;
                std::vector<Card>      multi_three_kind_cards;         // Can be the case that there are two three of a kinds
                bool is_two_pair        = false;
                bool is_pair            = false;
                std::vector<Card>      multi_pair_cards;               // ditto with two pairs
                std::vector<Card> high_card;
                
                for(int i=0; i<num_cards; i++) {
                    const auto cardranktmp = cards[i].get_rank();
                    const auto cardsuittmp = cards[i].get_suit();
                    ranks[cardranktmp].first++;
                    ranks[cardranktmp].second.emplace_back(cards.at(i));
                    suits[cardsuittmp].first++;
                    suits[cardsuittmp].second.emplace_back(cards.at(i));

                    if( cards[i].get_rank_as_int() == cards[i+1].get_rank_as_int()+1 && !is_straight) {
                        straight_cards.emplace_back(cards[i+1]); 
                        if(straight_cards.size() == 1) {
                            straight_cards.emplace_back(cards[i]);
                        }
                        else if(straight_cards.size() == 5) {
                            is_straight = true;
                        }
                    }
                }
                
                for(const auto& suit_count_pair : suits) {
                    if (suit_count_pair.second.first >= 5) { 
                        is_flush = true;
                        flush_cards = suit_count_pair.second.second;
                        flush_cards.resize(5);
                        // SECOND SECOND FIRST FIRST SECOND
                    }
                }

                if (is_straight && is_flush && (flush_cards == straight_cards)) {
                    is_straight_flush = true;
                }
                
                int three_count = 0; //durrrr
                int two_count   = 0;
                for(const auto& ranks_pair : ranks) {
                    // Look for four of a kind
                    if (ranks_pair.second.first == 4) {
                        is_four_kind = true;
                        four_kind_cards = ranks_pair.second.second;
                        break;
                    }
                    // Look for full house or two pair
                    else if (ranks_pair.second.first == 3) {
                        is_three_kind = true;
                        const auto tmp = ranks_pair.second.second;
                        multi_three_kind_cards.insert(multi_three_kind_cards.end(), tmp.begin(), tmp.end()); 
                        three_count++;
                    }
                    else if (ranks_pair.second.first == 2) {
                        const auto tmp = ranks_pair.second.second;
                        multi_pair_cards.insert(multi_pair_cards.end(), tmp.begin(), tmp.end());
                        two_count++;
                    }
                }

                if ( two_count >= 2 ) {
                    is_two_pair = true;
                    std::sort(multi_pair_cards.begin(), multi_pair_cards.end());
                    multi_pair_cards.resize(4);     // trims low valued pairs
                }
                if ( two_count == 1 ) is_pair = true;


                if ( (is_three_kind && is_pair) || (is_three_kind && is_two_pair) ) {
                    is_full_house = true;

                    full_house_cards.insert(full_house_cards.end(), multi_three_kind_cards.begin(), multi_three_kind_cards.end());
                    full_house_cards.insert(full_house_cards.end(), multi_pair_cards.begin(), multi_pair_cards.end());
                    full_house_cards.resize(5);
                }

                // if none of these, high card
                
                high_card.emplace_back(cards.front());
                
                if ( is_straight_flush )         { ret.handrank =  HandRank::STRAIGHT_FLUSH;        winningCards = straight_cards; }
                else if ( is_four_kind )         { ret.handrank =  HandRank::FOUR_KIND;             winningCards = four_kind_cards; }
                else if ( is_full_house )        { ret.handrank =  HandRank::FULL_HOUSE;            winningCards = full_house_cards; }
                else if ( is_flush )             { ret.handrank =  HandRank::FLUSH;                 winningCards = flush_cards; }
                else if ( is_straight )          { ret.handrank =  HandRank::STRAIGHT;              winningCards = straight_cards; }
                else if ( is_three_kind )        { ret.handrank =  HandRank::THREE_KIND;            winningCards = multi_three_kind_cards; }
                else if ( is_two_pair )          { ret.handrank =  HandRank::TWO_PAIR;              winningCards = multi_pair_cards; }
                else if ( is_pair )              { ret.handrank =  HandRank::ONE_PAIR;              winningCards = multi_pair_cards; }
                else                             { ret.handrank = HandRank::HIGH_CARD;              winningCards = high_card; }

                // get kicker cards

                auto ascendingComparator = [](const Card& a, const Card& b) { return a<b; };
                auto descendingComparator = [](const Card& a, const Card& b) { return a>b; };
                //std::set_difference( cards.begin(), cards.end(), winningCards.begin(), winningCards.end(), std::back_inserter(kickerCards) , ascendingComparator);

                auto itCards = cards.begin();
                while(itCards != cards.end() ) {
                    bool addCard = true;
                    auto itWinners = winningCards.begin();
                    while(itWinners != winningCards.end()) {
                        if( *itWinners == *itCards)
                            addCard = false;
                        itWinners++;
                    }
                    if( addCard ) kickerCards.emplace_back(*itCards);
                    itCards++;
                }
                std::sort(kickerCards.begin(), kickerCards.end(), descendingComparator);
                std::sort(winningCards.begin(), winningCards.end(), descendingComparator);

                ret.kickers = kickerCards;
                ret.maincards = winningCards;


                return ret;
                
            }
        static bool showdown(const FullHandRank& A, const FullHandRank& B) {
            if( A.handrank < B.handrank ) return true;
            if( A.handrank > B.handrank ) return false;

            // handranks equal, check value of main cards
            for(size_t i=0; i<A.maincards.size(); i++) {
                const Card a = A.maincards[i];
                const Card b = B.maincards[i];
                if( a.get_rank() < b.get_rank()) return true;
            }
            for(size_t i=0; i<A.kickers.size(); i++) {
                const Card a = A.kickers[i];
                const Card b = B.kickers[i];
                if( a.get_rank() < b.get_rank()) return true;
            }
            return false;
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

                if( fhrA.maincards.size() != fhrB.maincards.size() ) throw;
                for(int i = 0; i < fhrA.maincards.size(); i++ ) {
                    return fhrA.maincards[i] < fhrB.maincards[i];
                }
                for(int i = 0; i < fhrA.kickers.size(); i++ ) {
                    return fhrA.kickers[i] < fhrB.kickers[i];
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

