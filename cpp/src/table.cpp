#include "table.h"
#include "player.h"
#include "strategy.h"
namespace Poker {

    std::shared_ptr<Deck>   Table::getDeck() { return std::make_shared<Deck>(deck); }
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
    void Table::setPlayerList(const vector<string> sVec) {
        this->playerList.clear();
        std::vector<PlayerPosition> playerPositionList = numPlayersToPositionList(sVec.size());
        for(auto it = sVec.begin(); it != sVec.end(); it++) {
            auto s = *it;
            auto n = distance(sVec.begin(), it);
            auto guy = make_shared<Player>();
            if (s == "random") 
                    guy->strategy = make_shared<RandomAI>();
            else if (s == "call")
                    guy->strategy = make_shared<SingleMoveCallAI>();
            else if (s == "sequence")
                    guy->strategy = make_shared<SequenceMoveAI>();
            else if (s == "fhrprop")
                    guy->strategy = make_shared<FHRProportionalAI>();
            else if (s == "hrbetrel")
                    guy->strategy = make_shared<HRBetRelAI>();
            else    guy->strategy = make_shared<RandomAI>();        // default = random moves
            guy->setPosition(playerPositionList[n]);
            guy->playerID = n;
            this->playerList.emplace_back(guy);
        }
    }

    std::vector<shared_ptr<Player>> Table::getPlayersInBettingOrder(std::vector<shared_ptr<Player>> pvector) {
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
    void Table::clearPlayerHands() {
        // clears all player hands
        for(auto P : this->playerList) {
            P->resetHand();
        }
    }
    void Table::setPlayerBankrolls(int n) {
        // Sets everybodies wallets to n
        for(auto P : this->playerList) {
            P->bankroll = n;
        }
    }

    void Table::shuffleDeck() {
        // Make sure deck has it's mersenne twister primed before calling this or you got a one-way ticket to segfault city
        this->deck.shuffle();
    }

    void Table::resetCards(random_device& rd) {
        // Resets all cards everywhere and gets a fresh RNG for the deck. 
        std::mt19937_64 g(rd());
        
        deck = Deck();
        deck.mersenne = g;
        // shuffle cards
        this->shuffleDeck();
        // clear community cards
        this->communityCards.clear();
        this->clearPlayerHands();       
    }

    bool Table::arePlayerPositionsValid(const vector<shared_ptr<Player>>& pList) {
        // Returns true if each player in pList has a unique Position
        // Basically tells you if the Positions have been initialized correctly
        std::vector<shared_ptr<Player>> uniquePositionPlayers;

        unique_copy(pList.begin(), pList.end(), back_inserter(uniquePositionPlayers), [] (const shared_ptr<Player>& a, const shared_ptr<Player>& b) {
            return a->getPosition() == b->getPosition();
        } );
        return uniquePositionPlayers.size() == pList.size();
    }

    shared_ptr<Player> Table::getPlayerByID(const int& id) {
        return *std::find_if(this->playerList.begin(), this->playerList.end(), [&id] (const shared_ptr<Player>& p) {return p->playerID == id;});
    };

}
