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

    std::cout << "Begin game" << std::endl;
    auto game = std::make_shared<Game>();
    std::cout << *game->table->get_deck();

    //for(Player& p : game->table->player_list) { std::cout << p.position << " " << p.bankroll << std::endl;}
    for(int it = 0; it < 3; it++) {
        game->doRound();
        game->print();
    }
    return 0;
}
