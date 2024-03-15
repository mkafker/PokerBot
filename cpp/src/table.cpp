#include "table.h"
#include "player.h"
namespace Poker {

            std::shared_ptr<Deck>   Table::getDeck() { return std::make_shared<Deck>(deck); }
            void Table::resetDeck(random_device &rd) {
                std::mt19937_64 g(rd());
                
                deck = Deck();
                deck.mersenne = g;
            }
            void Table::dealCommunityCards(int street) {
                if( street == 1 ) {
                    for(int i = 0; i < 3; i++ ) 
                        communityCards.push_back(deck.pop_card());
                }
                else if (street > 1) { communityCards.push_back(deck.pop_card()); }
            }
            void Table::dealPlayersCards(const std::vector<shared_ptr<Player>> pList) {
                // Deals every active player two cards
                for(shared_ptr<Player> p : pList) {
                    p->hand.clear();
                    p->hand.emplace_back(deck.pop_card());
                    p->hand.emplace_back(deck.pop_card());
                }
            }
            void Table::populatePlayerListDefaults(const string& s, const int& n) {
                // populates this Table's player list with n players of AI type s
                std::vector<PlayerPosition> playerPositionList = numPlayersToPositionList(n);
                this->playerList.clear();
                for(int i = 0; i < n; i++ ) {
                    shared_ptr<Player> guyToMake;
                    if (s == "random") 
                            guyToMake = make_shared<RandomAI>();
                    else if (s == "call")
                            guyToMake = make_shared<SingleMoveCallAI>();
                    else    guyToMake = make_shared<Player>();
                    guyToMake->setPosition(playerPositionList[i]);
                    guyToMake->bankroll = 100;
                    guyToMake->playerID = i;
                    // FUN FACT: emplacing back derived classes ACTUALLy emplaces back the base class! Very cool!
                    this->playerList.emplace_back(guyToMake);
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
                    vector<shared_ptr<Player>> ret (playerList);    // copy constructor
                    std::sort(ret.begin(), ret.end(), comp);
                    return ret;
                }
                return std::vector<shared_ptr<Player>>();
            }
            void Table::resetPlayerHands() {
                // clears all player hands
                for(auto P : this->playerList) {
                    P->resetHand();
                }
            }

            void Table::shuffleDeck() {
                // Make sure deck has it's mersenne twister primed before calling this or you got a one-way ticket to segfault city
                this->deck.shuffle();
            }


}
