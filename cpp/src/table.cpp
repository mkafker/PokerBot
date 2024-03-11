#include "table.h"
#include "player.h"
namespace Poker {

            std::shared_ptr<Deck>   Table::getDeck() { return std::make_shared<Deck>(deck); }

            void Table::reset(int n) {
                std::random_device rd;
                std::mt19937_64 g(rd());
                
                deck = Deck();
            }
            void Table::dealCommunityCards(int street) {
                if( street == 1 ) {
                    for(int i = 0; i < 3; i++ ) 
                        communityCards.push_back(deck.pop_card());
                }
                else if (street > 1) { communityCards.push_back(deck.pop_card()); }
            }
            void Table::dealPlayersCards(const std::vector<shared_ptr<Player>> playerList) {
                // Deals every active player two cards
                for(shared_ptr<Player> p : playerList) {
                    p->hand.clear();
                    p->hand.emplace_back(deck.pop_card());
                    p->hand.emplace_back(deck.pop_card());
                }
            }


            std::vector<shared_ptr<Player>> Table::getPlayersInOrder(std::vector<shared_ptr<Player>> pvector) {
                // returns a vector of pointers to players in the correct (game) order
                auto comp = [](const shared_ptr<Player> a, const shared_ptr<Player> b) -> bool { return a->getPosition() < b->getPosition(); };
                if( !pvector.empty() ) {
                    std::sort(pvector.begin(), pvector.end(), comp);
                    return pvector;
                }
                else {
                    vector<shared_ptr<Player>> ret;
                    std::transform(playerList.begin(), playerList.end(), std::back_inserter(ret), [](Player& a) { return make_shared<Player>(a); } );
                    return ret;
                }
                return std::vector<shared_ptr<Player>>();
            }

            /*
            std::vector<shared_ptr<Player>> Table::getActivePlayers(std::vector<shared_ptr<Player>> pvector) { 
                // Returns a vector of pointers to players that are active
                std::vector<shared_ptr<Player>> ret;
                if( !pvector.empty() ) {
                    std::copy_if(pvector.begin(), pvector.end(), std::back_inserter(ret), [](const shared_ptr<Player> p) { return p->isActive; } );
                }
                return std::vector<shared_ptr<Player>>();
            }
            std::vector<shared_ptr<Player>> Table::getActivePlayers(std::vector<Player> pvector) { 
                // Returns a vector of pointers to players that are active
                std::vector<shared_ptr<Player>> ret;
                if( !pvector.empty() ) {
                    std::copy_if(pvector.begin(), pvector.end(), std::back_inserter(ret), [](const shared_ptr<Player> p) { return p->isActive; } );
                }
                return std::vector<shared_ptr<Player>>();
            }

            std::vector<shared_ptr<Player>> Table::getNonBankruptPlayers(std::vector<shared_ptr<Player>> pvector) {
                std::vector<shared_ptr<Player>> nbp;
                auto isBR = [] (shared_ptr<Player> a) -> bool { return a->bankroll <= 0; };
                if( pvector.empty() ) {
                    std::transform(playerList.begin(), playerList.end(), std::back_inserter(pvector), [] (Player& a) {return &a; });
                }
                std::remove_copy_if(pvector.begin(), pvector.end(), std::back_inserter(nbp), isBR);
                return nbp;
            }

            std::vector<shared_ptr<Player>> Table::getBettingPlayers(std::vector<shared_ptr<Player>> pvector) {
                std::vector<shared_ptr<Player>> nbp;
                auto isBR = [] (shared_ptr<Player> a) -> bool { return a->isBetting; };
                if( pvector.empty() ) {
                    std::transform(playerList.begin(), playerList.end(), std::back_inserter(pvector), [] (Player& a) {return &a; });
                }
                std::remove_copy_if(pvector.begin(), pvector.end(), std::back_inserter(nbp), isBR);
                return nbp;
            }
            */


            void Table::resetPlayerHands() {
                // clears all player hands
                for(auto P : playerList) {
                    P.resetHand();
                }
            }

            void Table::shuffleDeck() {
                this->deck.shuffle(g);
            }


}
