#include <array>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <vector>
#include <memory>

#include "card.h"
#include "showdown.h"
#include "table.h"
#include "player.h"
#include "game.h"
using namespace Poker;


int main() {
    Hand hand1(std::vector<Card>({  Card(Rank::C_2, Suit::CLUB), 
                                    Card(Rank::C_2, Suit::DIAMOND),
                                    Card(Rank::C_K, Suit::HEART) }));
    Hand hand2(std::vector<Card>({  Card(Rank::C_7, Suit::CLUB), 
                                    Card(Rank::C_7, Suit::DIAMOND),
                                    Card(Rank::C_Q, Suit::HEART) }));
    std::list<Hand*> s({&hand1, &hand2});
    // TODO: I think..... make HandRank a struct that contains info about both Rank (e.g. full house) and unique card value information 
    //  e.g. full house A A A K K 3 2
    // should be represented as {full house, {ace, king, 3, 2}}
    // implement comparator overloads to make life easy
    auto* res = Hand::showdown(s);
    if( res == &hand1) std::cout << "hand1 wins" << std::endl;
    if( res == &hand2) std::cout << "hand2 wins" << std::endl;
    /*
    std::cout << "Begin game" << std::endl;
    auto game = std::make_shared<Game>();
    std::cout << *game->table->get_deck() << std::endl;

    //for(Player& p : game->table->player_list) { std::cout << p.position << " " << p.bankroll << std::endl;}
    for(int it = 0; it < 1; it++) {
        game->doRound();
        game->print();
    }*/
    return 0;
}
