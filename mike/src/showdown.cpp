#include <unordered_map>
#include <map>
#include <algorithm>
#include <list>
#include "card.h"
#include "showdown.h"

namespace Poker {
  std::ostream& operator<<(std::ostream& stream, const HandRank& a) {
      stream << HandRank_to_String[a];
      return stream;
  }
  
  bool operator==(const FullHandRank& a, const FullHandRank& b) {
      bool tmp = true;
      if(a.handrank != b.handrank) tmp = false;
      if(a.kickers != b.kickers) tmp = false;
      if(a.maincards != b.maincards) tmp = false;
      return tmp;
  }


   void sortCards(std::vector<Card>& h) {
          std::sort(h.begin(), h.end(), [](const Card& a, const Card& b) { return a<b; });
  }
   void sortCardsDescending(std::vector<Card>& h) {
          std::sort(h.begin(), h.end(), [](const Card& a, const Card& b) { return a>b; });
  }


    #define DEREF_COMPLEMENT_RETURN \
        for( const auto& ptr : winningCards ) { \
            ret.maincards.push_back(*ptr); \
        }  \
          auto itCards = cards.begin();  \
          while(itCards != cards.end() ) { \
              bool addCard = true; \
              auto itWinners = ret.maincards.begin(); \
              while(itWinners != ret.maincards.end()) { \
                  if( *itWinners == *itCards) \
                      addCard = false; \
                  itWinners++; \
              } \
              if( addCard ) ret.kickers.emplace_back(*itCards); \
              itCards++; \
          } \
          ret.kickers.resize(std::min(5-ret.maincards.size(),ret.kickers.size())); \
          return ret;

    struct RankDescendingOrder {
        bool operator()(const Rank& a, const Rank& b) const {
            return a>b;
        }
    };

    const FullHandRank calcFullHandRank(const std::vector<Card>& hand_in) {
        std::vector<Card> cards = hand_in;
        FullHandRank ret;
        // the most important sort in the universe
        sortCardsDescending(cards);

        const size_t num_cards = cards.size();

        std::vector<Card*>     winningCards;

        // Maps galore!
        // consider performance implications of moving to unordered_map
        std::map<Rank, std::vector<Card*>, RankDescendingOrder> ranks;
        std::unordered_map<Suit, std::vector<Card*>> suits;
    
        bool is_flush           = false;
        std::vector<Card*>      flush_cards;
        bool is_straight        = false;
        std::vector<Card*>      straight_cards;

        // Fill maps... these serve as histograms
        for(int i=0; i<num_cards; i++) {
            const auto cardranktmp = cards[i].get_rank();
            const auto cardsuittmp = cards[i].get_suit();
            ranks[cardranktmp].emplace_back(&cards[i]);
            suits[cardsuittmp].emplace_back(&cards[i]);
        }
        
        // Check for a flush
        for(const auto& suit_count_pair : suits) {
            if (suit_count_pair.second.size() >= 5) { 
                is_flush = true;
                flush_cards = suit_count_pair.second;
                flush_cards.resize(5);
            }
        }

        // If it's a flush, check if it's also a straight. Then our work is done
        bool is_straight_flush  = false;
        if( is_flush ) {
            int straightCount = 1;
            for(int i=1; i < flush_cards.size(); i++) {
                    if(flush_cards[i]->get_rank_as_int() == flush_cards[i - 1]->get_rank_as_int() + 1) {
                        straightCount++;
                        // there will only ever be 5 cards in a flush so they must
                        // all have consecutive ranks to be a straight flush
                        if(straightCount == 5) {
                            is_straight = true;
                            is_straight_flush = true;
                            break;
                        }
                    } else {
                        straightCount = 1;
                    }
            }
            if( is_straight_flush ) {
                ret.handrank = HandRank::STRAIGHT_FLUSH;
                winningCards = flush_cards;
                DEREF_COMPLEMENT_RETURN;
            }
        }
        
        bool is_four_kind       = false;
        std::vector<Card*>      four_kind_cards;
        bool is_full_house      = false;
        std::vector<Card*>      full_house_cards;
        std::vector<Card*>      multi_three_kind_cards;         // Can be the case that there are two three of a kinds
        std::vector<Card*>      multi_pair_cards;               // ditto with two pairs
        std::vector<Card*>      high_card;
        int three_count = 0;
        int two_count = 0;
        // Check for two/three/four of a kinds
        for(const auto& r_pair : ranks) {
            if(r_pair.second.size() == 4) {
                is_four_kind = true;
                four_kind_cards = r_pair.second;

                ret.handrank = HandRank::FOUR_KIND;
                winningCards = four_kind_cards;
                DEREF_COMPLEMENT_RETURN;
            }
            else if (r_pair.second.size() == 3) {
                multi_three_kind_cards.insert(multi_three_kind_cards.end(), r_pair.second.begin(), r_pair.second.end()); 
                three_count++;
            }
            else if (r_pair.second.size() == 2) {
                multi_pair_cards.insert(multi_pair_cards.end(), r_pair.second.begin(), r_pair.second.end());
                two_count++;
            }
        }

        if ( three_count == 1 && two_count >= 1 ) {   // FULL  HOUSE
            full_house_cards.insert(full_house_cards.end(), multi_three_kind_cards.begin(), multi_three_kind_cards.end());
            full_house_cards.insert(full_house_cards.end(), multi_pair_cards.begin(), multi_pair_cards.begin()+2); // multi_pair_cards should be sorted already. This should pick the best pair
            ret.handrank = HandRank::FULL_HOUSE;
            winningCards = full_house_cards;
            DEREF_COMPLEMENT_RETURN;
        }
        else if ( is_flush ) {
            ret.handrank = HandRank::FLUSH;
            winningCards = flush_cards;
            DEREF_COMPLEMENT_RETURN;
        }
        // check for a straight
        int straightCount = 1;
        std::vector<int> straightRanks;
        for(const auto& pair : ranks)
            straightRanks.emplace_back(static_cast<int>(pair.first));
        for(int i=0; i < straightRanks.size(); i++) {
            if( straight_cards.empty() ) straight_cards.emplace_back(ranks[static_cast<Rank>(straightRanks[i])][0]);
            else {
                const bool descending = straightRanks[i] == straightRanks[i-1] - 1;
                const bool flat = straightRanks[i] == straightRanks[i-1];
                if ( descending && !flat) {
                    straightCount++;
                    // Put back the first card from the vector of cards with descending Rank
                    // In the eyes of a straight, all of these cards are equivalent
                    straight_cards.emplace_back(ranks[static_cast<Rank>(straightRanks[i])][0]);
                    if( straightCount == 5) {
                        is_straight = true;
                        ret.handrank = HandRank::STRAIGHT;
                        winningCards = straight_cards;
                        DEREF_COMPLEMENT_RETURN;
                    }
                }
                else if (!(flat || descending)) {
                    straightCount = 1;
                    straight_cards.clear();
                }
            }
        }

        // check for three of a kind
        if( three_count >= 1) {
            multi_three_kind_cards.resize(3);
            ret.handrank = HandRank::THREE_KIND;
            winningCards = multi_three_kind_cards;
            DEREF_COMPLEMENT_RETURN;
        }
        // check for two pair
        else if( two_count >= 2) {
            multi_pair_cards.resize(4);
            ret.handrank = HandRank::TWO_PAIR;
            winningCards = multi_pair_cards;
            DEREF_COMPLEMENT_RETURN;
        }
        else if( two_count == 1) {
            ret.handrank = HandRank::ONE_PAIR;
            winningCards = multi_pair_cards;
            DEREF_COMPLEMENT_RETURN;
        }
        else {
            ret.handrank = HandRank::HIGH_CARD;
            high_card.emplace_back(&cards.front());
            winningCards = high_card;
            DEREF_COMPLEMENT_RETURN;
        }
        return nullFHR;
      }

  const FullHandRank& showdownFHR(const FullHandRank& A, const FullHandRank& B) {
      // Returns true if B beats A
      if( A.handrank < B.handrank ) return B;
      if( A.handrank > B.handrank ) return A;

      // handranks equal, check value of main cards
      for(size_t i=0; i<A.maincards.size(); i++) {
          const Card a = A.maincards[i];
          const Card b = B.maincards[i];
          if( a.get_rank() < b.get_rank()) return B;
          if( a.get_rank() > b.get_rank()) return A;
      }

      // whoa! down to the kickers
      for(size_t i=0; i<A.kickers.size(); i++) {
          const Card a = A.kickers[i];
          const Card b = B.kickers[i];
          if( a.get_rank() < b.get_rank()) return B;
          if( a.get_rank() > b.get_rank()) return A;
      }
      // draw
      return nullFHR;
  }



  FullHandRank generateRandomFHR() {
    Deck tmp;
    tmp.shuffle();
    std::vector<Card> tmphand(7);
    for(auto& a : tmphand) 
        a = tmp.pop_card();
    
    return calcFullHandRank(tmphand);
  }

}
