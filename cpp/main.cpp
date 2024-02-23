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
    Deck deck;

    std::vector<Card> hand;
    hand.resize(5);
    hand[0] = Card(Rank::C_8, Suit::DIAMOND);
    hand[1] = Card(Rank::C_8, Suit::SPADE);
    hand[2] = Card(Rank::C_8, Suit::HEART);
    hand[3] = Card(Rank::C_8, Suit::DIAMOND);
    hand[4] = Card(Rank::C_7, Suit::HEART);

    Hand myhand(hand);
    std::vector<Card> hand2;
    hand2.resize(2);
    hand2[0] = Card(Rank::C_8, Suit::DIAMOND);
    hand2[1] = Card(Rank::C_8, Suit::SPADE);

    Hand mybhand(hand2);
    std::cout << myhand << std::endl;
    std::cout << myhand.rank << std::endl;
    std::cout << mybhand << std::endl;
    std::cout << mybhand.rank << std::endl;


    std::cout << "Begin game" << std::endl;
    auto game = std::make_shared<Game>();
    std::cout << *game->table->get_deck();
    //for(Player& p : game->table->player_list) { std::cout << p.position << " " << p.bankroll << std::endl;}
    game->print();
    game->doRound();
    game->print();
    return 0;
}
