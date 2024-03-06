#include "table.h"
#include "player.h"
namespace Poker {

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
                    p.hand.emplace_back(deck.pop_card());
                    p.hand.emplace_back(deck.pop_card());
                }
            }
            std::vector<Player*> Table::getActivePlayers(std::vector<Player*> pvector) { 
                // Returns a vector of pointers to players that are active
                std::vector<Player*> ret;
                if( !pvector.empty() ) {
                    std::copy_if(pvector.begin(), pvector.end(), std::back_inserter(ret), [](const Player* p) { return p->isActive; } );
                }
                return std::vector<Player*>();
            }
            std::vector<Player*> Table::getPlayersInOrder(std::vector<Player*> pvector) {
                // returns a vector of pointers to players in the correct (game) order
                auto comp = [](const Player* a, const Player* b) -> bool { return a->getPosition() < b->getPosition(); };
                if( !pvector.empty() ) {
                    std::sort(pvector.begin(), pvector.end(), comp);
                    return pvector;
                }
                    //std::transform(player_vector.begin(), player_vector.end(), std::back_inserter(ret), [](Player& a) { return &a; } );
                return std::vector<Player*>();
            }

            std::vector<Player*> Table::getNonBankruptPlayers(std::vector<Player*> pvector) {
                std::vector<Player*> nbp;
                auto isBR = [] (Player* a) -> bool { return a->bankroll <= 0; };
                if( pvector.empty() ) {
                    std::transform(player_list.begin(), player_list.end(), std::back_inserter(pvector), [] (Player& a) {return &a; });
                }
                std::remove_copy_if(pvector.begin(), pvector.end(), std::back_inserter(nbp), isBR);
                return nbp;
            }


            void Table::resetPlayerHands() {
                // clears all player hands
                for(auto P : player_list) {
                    P.resetHand();
                }
            }

            void Table::shuffleDeck() {
                this->deck.shuffle(g);
            }


}
