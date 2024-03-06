#include "table.h"
#include "player.h"
namespace Poker {

            Table::Table() {
              reset();
            }
            std::shared_ptr<Deck>   Table::getDeck() { return std::make_shared<Deck>(deck); }

            void Table::reset(int n) {
                std::random_device rd;
                std::mt19937_64 g(rd());
                
                street = 0;
                minimumBet = 0;
                pot = 0;
                deck = Deck();
                deck.shuffle(g);
            }
            void Table::dealCommunityCards() {
                if( street == 1 ) {
                    for(int i = 0; i < 3; i++ ) 
                        community_cards.push_back(deck.pop_card());
                }
                else if (street > 1) { community_cards.push_back(deck.pop_card()); }
            }
            void Table::dealAllPlayersCards() {
                // Deals everyone two cards
                for(Player& p : player_list) {
                    p.hand.clear();
                    p.hand.add(deck.pop_card());
                    p.hand.add(deck.pop_card());
                }
            }
            Player& Table::get_player_by_position(PlayerPosition p) {
                auto it = std::find_if(player_list.begin(), player_list.end(), [p](Player& a){ return a.getPosition()==p; });
                if( it != player_list.end()) {
                    return *it;
                }
                throw std::runtime_error("No player is UTG! Something went terribly wrong.");
            }
            
            std::list<Player*> Table::getPlayersInOrder(std::list<Player*> plist) {
                // returns a list of pointers to players in the correct (game) order
                auto comp = [](const Player& a, const Player& b) -> bool { return a->getPosition() < b->getPosition(); };
                if( !plist.empty() ) {
                    plist.sort( comp );
                    return plist;
                }
                    //std::transform(player_list.begin(), player_list.end(), std::back_inserter(ret), [](Player& a) { return &a; } );
                return std::list<Player*>();
            }

            std::list<Player*> Table::getNonBankruptPlayers(std::list<Player*> plist) {
                std::list<Player*> nbp;
                auto isBR = [] (Player* a) -> bool { return a->bankroll <= 0; };
                if( plist.empty() ) {
                    std::transform(player_list.begin(), player_list.end(), std::back_inserter(plist), [] (Player& a) {return &a; });
                }
                std::remove_copy_if(plist.begin(), plist.end(), std::back_inserter(nbp), isBR);
                return nbp;
            }


            void Table::resetPlayerHands() {
                // clears all player hands
                for(auto P : player_list) {
                    P.resetHand();
                }
            }

    


}
