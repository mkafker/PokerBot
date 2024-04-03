#include <iostream>
#include <iomanip>

#include "card.h"
#include "showdown.h"
#include "table.h"
#include "player.h"
#include "game.h"
#include "bench.h"
#include "pybindings.h"

using namespace Poker;
int main() {
    std::vector<Card> straightflush = {  Card(Rank::C_A, Suit::DIAMOND     ), 
                                    Card(Rank::C_K, Suit::HEART  ),
                                    Card(Rank::C_Q, Suit::HEART    ),
                                    Card(Rank::C_J, Suit::HEART    ),
                                    Card(Rank::C_T, Suit::HEART    ),
                                    Card(Rank::C_3, Suit::DIAMOND    ),
                                    Card(Rank::C_6, Suit::CLUB    ) };
                                    
    std::vector<Card> fourkind = {  Card(Rank::C_2, Suit::HEART     ), 
                                    Card(Rank::C_2, Suit::HEART  ),
                                    Card(Rank::C_2, Suit::DIAMOND    ),
                                    Card(Rank::C_2, Suit::CLUB    ),
                                    Card(Rank::C_T, Suit::HEART    ),
                                    Card(Rank::C_9, Suit::HEART    ),
                                    Card(Rank::C_6, Suit::CLUB    ) };
                                    


    std::vector<Card> theirCards  = { Card(Rank::C_5, Suit::DIAMOND), Card(Rank::C_J, Suit::SPADE) };

    //Poker::pyMonteCarloGames(10);
    std::vector<Card> myCards  = { Card(Rank::C_4, Suit::CLUB), Card(Rank::C_J, Suit::HEART) };
    std::vector<Card> comCards  = { Card(Rank::C_2, Suit::HEART), 
                                    Card(Rank::C_3, Suit::CLUB), 
                                    Card(Rank::C_7, Suit::DIAMOND),
                                    Card(Rank::C_T, Suit::DIAMOND) };
    std::vector<Card> AceTwo = {Card(Rank::C_2, Suit::CLUB), Card(Rank::C_A, Suit::HEART)};
    std::vector<Card> TwoAce = {Card(Rank::C_A, Suit::HEART), Card(Rank::C_2, Suit::CLUB)};
    std::cout << monteCarloSingleHand(AceTwo, 3, 1, 5000) << " " << monteCarloSingleHand(TwoAce, 3, 1, 5000) << std::endl;
    vector<Suit> suitlist = { Suit::HEART, Suit::CLUB, Suit::DIAMOND, Suit::SPADE};
    for(int j=0; j<13; j++) 
        std::cout << std::setw(10) << static_cast<Rank>(j) << " ";
    std::cout << std::endl;
    for(int i=0; i<13; i++) {
        std::cout << static_cast<Rank>(i) << " ";
        for(int j=0; j<13; j++) {
            Card c1(static_cast<Rank>(i), Suit::HEART);
            Card c2(static_cast<Rank>(j), Suit::HEART);
            std::vector<Card> myCards = { c1, c2 };
            double WR = monteCarloSingleHand(myCards, 3, 1, 5000);
            std::cout << std::fixed;
            std::cout << std::setprecision(2) << std::setw(10);
            std::cout << WR << " ";
        }
        std::cout << std::endl;
    }



    return 0;
}