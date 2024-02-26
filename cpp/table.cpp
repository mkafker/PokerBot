#include "table.h"
#include "player.h"
namespace Poker {

            Table::Table() {
                init();
            }
            std::shared_ptr<Deck>   Table::get_deck() { return deck; }
            const int               Table::get_BB() { return bigBlind; }
            const int               Table::get_SB() { return smallBlind; }

            void Table::init(int n) {
                n_players = n;
                for(int i = 0; i < n_players; i++ ) {
                    Player p(i); // generate new players
                    player_list.push_back(p);
                }
                
                street = 0;
                currentBet = 0;
                pot = 0;
                deck = std::make_shared<Deck>();
                deck->shuffle();
            }
            void Table::dealCommunityCards() {
                if( street == 0 ) {
                    for(int i = 0; i < 3; i++ ) 
                        community_cards.push_back(deck->pop_card());
                }
                else { community_cards.push_back(deck->pop_card()); }
            }
            void Table::dealAllPlayersCards() {
                // Deals everyone two cards
                for(Player& p : player_list) {
                    p.hand.clear();
                    p.hand.add(deck->pop_card());
                    p.hand.add(deck->pop_card());
                }
            }
            Player& Table::get_player_by_position(PlayerPosition p) {
                auto it = std::find_if(player_list.begin(), player_list.end(), [p](Player& a){ return a.getPosition()==p; });
                if( it != player_list.end()) {
                    return *it;
                }
                throw std::runtime_error("No player is UTG! Something went terribly wrong.");
            }
            
            std::list<Player*> Table::getPlayersInOrder(std::list<Player*> plist, bool startOfRound) {
                // returns a list of pointers to players in the correct (game) order
                // returns a sorted list of the table players if startOfRound is true
                // 
                auto comp = [](Player* a, Player* b) -> bool { return a->getPosition() < b->getPosition(); };
                if( !plist.empty() && !startOfRound ) {
                    plist.sort( comp );
                    return plist;
                }
                else if (startOfRound) {      // Problem: plist is empty if everybody if folded or all in. The mechanism below is supposed to be used only at the start of the game!
                    std::list<Player*> ret;
                    //ret.resize(player_list.size());
                    std::transform(player_list.begin(), player_list.end(), std::back_inserter(ret), [](Player& a) { return &a; } );
                    ret.sort( comp );
                    return ret;
                }
                return std::list<Player*>();
            }


            void Table::set_num_players(int n) {
                n_players = n;                  // effectively trims the last n_players - n players from the game
            }

            void Table::resetPlayerHands() {
                // clears all player hands
                for(auto P : player_list) {
                    P.resetHand();
                }
            }

    


}