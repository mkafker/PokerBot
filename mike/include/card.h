#pragma once

#include <iostream>
#include <unordered_map>
#include <random>
#include <vector>
#include <algorithm>
#include <chrono>

namespace Poker {
    enum class Suit : char {
        CLUB = 'C',
        HEART = 'H',
        SPADE = 'S',
        DIAMOND = 'D',
        UNDEF_SUIT = 'X'
    };
    enum class Rank : int {
        C_2 = 0,
        C_3 = 1,
        C_4 = 2,
        C_5 = 3, 
        C_6 = 4,
        C_7 = 5,
        C_8 = 6,
        C_9 = 7,
        C_T = 8,
        C_J = 9,
        C_Q = 10,
        C_K = 11,
        C_A = 12,
        UNDEF_RANK = -1
    };
    static std::unordered_map<Rank, char> rank_to_char_map = 
    {
        {Rank::C_2, '2'},
        {Rank::C_3, '3'},
        {Rank::C_4, '4'},
        {Rank::C_5, '5'},
        {Rank::C_6, '6'},
        {Rank::C_7, '7'},
        {Rank::C_8, '8'},
        {Rank::C_9, '9'},
        {Rank::C_T, 'T'},
        {Rank::C_J, 'J'},
        {Rank::C_Q, 'Q'},
        {Rank::C_K, 'K'},
        {Rank::C_A, 'A'},
        {Rank::UNDEF_RANK, 'X'}
  };
    class Card {
        public:
            Rank rank;
            Suit suit;
            Card() : rank(Rank::UNDEF_RANK), suit(Suit::UNDEF_SUIT) {};
            Card(Rank r, Suit s) : rank(r), suit(s) {};
            const Rank get_rank() const { return rank; }
            const Suit get_suit() const { return suit; }
            const int get_rank_as_int() const { return static_cast<int>(rank);}
            const char get_rank_as_char() const {return rank_to_char_map[rank]; }
        
    };
    inline bool operator<(Card a, Card b) { return a.get_rank_as_int() < b.get_rank_as_int(); }
    inline bool operator>(Card a, Card b) { return b<a; }
    inline bool operator==(const Card a, const Card b) { return (a.get_rank_as_int() == b.get_rank_as_int()) && (a.get_suit() == b.get_suit()); }

    inline std::ostream& operator<<(std::ostream& stream, Card &a) { 
        stream << char(a.get_suit()) << a.get_rank_as_char();
        return stream;
    }


    class Deck {
        public:
            std::vector<Card> cards;         // array to hold cards
            Deck() {
                cards = std::vector<Card>(52);
                constexpr int nRanks = 13;
                for(int i=0; i<nRanks; i++) {
                    cards[4*i]  =Card(static_cast<Rank>(i), Suit::CLUB);
                    cards[4*i+1]=Card(static_cast<Rank>(i), Suit::DIAMOND);
                    cards[4*i+2]=Card(static_cast<Rank>(i), Suit::HEART);
                    cards[4*i+3]=Card(static_cast<Rank>(i), Suit::SPADE);
                }
            }
            size_t size() const { return cards.size(); }
            const Card pop_card() {
                if( cards.size() == 0 ) return Card(); // error
                Card ret = cards.back();
                cards.pop_back();
                return ret;
            }
            void push_card(const Card& a) {
                if( cards.size() == 51 ) std::cerr << "Tried to add a card to a full deck." << std::endl; // error
                cards.emplace_back(a);
            }
            Deck(std::vector<Card> cardComplement) {
            // Deck constructor for all cards EXCEPT the argument
            // TODO: make this fast
            constexpr int nRanks = 13;
            cards = std::vector<Card>(52);
            for(int i=0; i<nRanks; i++) {
                cards[4*i]  =Card(static_cast<Rank>(i), Suit::CLUB);
                cards[4*i+1]=Card(static_cast<Rank>(i), Suit::DIAMOND);
                cards[4*i+2]=Card(static_cast<Rank>(i), Suit::HEART);
                cards[4*i+3]=Card(static_cast<Rank>(i), Suit::SPADE);
            }
            for(Card c : cardComplement) {
            auto it = std::find(cards.begin(), cards.end(), c);
            if( *it == c)
                cards.erase(it);
            }
            }
    };




    inline std::ostream& operator<<(std::ostream& stream, Deck &a) { 
        for(int i=0; i<a.size(); i++ )
            stream << a.cards[i] << " ";
        return stream;
    }
    inline std::ostream& operator<<(std::ostream& stream, std::vector<Card>& a) { 
        for(int i=0; i<a.size(); i++ )
            stream << a.at(i) << " ";
        return stream;
    }

    inline std::ostream& operator<<(std::ostream& stream, const Rank& r) { 
        stream << rank_to_char_map[r];
        return stream;
    }
};
