#include <unordered_map>
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


   void sortCards(std::vector<Card> h) {
          std::sort(h.begin(), h.end(), [](const Card& a, const Card& b) { return a<b; });
  }
   void sortCardsDescending(std::vector<Card> h) {
          std::sort(h.begin(), h.end(), [](const Card& a, const Card& b) { return a>b; });
  }

  const FullHandRank calcFullHandRank(const std::vector<Card>& hand_in) {
          std::vector<Card> cards = hand_in;
          
          FullHandRank ret;
          sortCardsDescending(cards);

          const size_t num_cards = cards.size();

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

          // TODO: check if this works now. and benchmark it against the impl below
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

  std::vector<Card>* showdown(std::vector<std::vector<Card>*>::iterator hands_begin, std::vector<std::vector<Card>*>::iterator hands_end) {
      if (std::distance(hands_begin, hands_end)==0) 
          return nullptr;
      
      // calculate full hand rank for each hand
      std::vector<FullHandRank> FHRs;
      std::transform(hands_begin, hands_end, std::back_inserter(FHRs), [](std::vector<Card>* h) {  return calcFullHandRank(*h); });
      

      auto FHRComparator = [](const FullHandRank& a, const FullHandRank& b) -> bool {
          return showdownFHR(a, b) == b;
      };
      // determine best hand by HandRank (not card values)
      auto handWinner = std::max_element(FHRs.begin(), FHRs.end(), FHRComparator);

      if( handWinner != FHRs.end()) {
          // get back original hand
          size_t i = std::distance(FHRs.begin(), handWinner);
          auto it = hands_begin;
          // need random acces!!!!
          // TODO: figure out what I meant by the above comment and fix it
          for( int j=0;j<i;j++ )
              it++;
          return *it; 
      }
      
      // draw
      return nullptr;
  }
}
