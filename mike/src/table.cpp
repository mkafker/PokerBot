#include "table.h"
#include "player.h"
#include "strategy.h"
namespace Poker {

    std::unique_ptr<Deck>   Table::getDeck() { return std::make_unique<Deck>(deck); }
    void Table::dealCommunityCards(const Street& street) {
        if( street == Street::UNDEF ) throw;
        if( street == Street::FLOP ) {
            for(int i = 0; i < 3; i++ ) 
                communityCards.push_back(deck.pop_card());
        }
        else if (street != Street::PREFLOP) { communityCards.push_back(deck.pop_card()); }
    }
    void Table::dealPlayersCards(const std::vector<Player*> pList) {
        // Deals every active player two cards
        for(auto p : pList) {
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
            Player guy; 
            if (s == "random") 
                    guy.strategy = make_shared<RandomAI>();
            else if (s == "call")
                    guy.strategy = make_shared<SingleMoveCallAI>();
                    /*
            else if (s == "sequence")
                    guy.strategy = make_shared<SequenceMoveAI>();
            else if (s == "CFRAI1")
                    guy.strategy = make_shared<CFRAI1>();
            else if (s == "Matt")
                    guy.strategy = make_shared<MattAI>();
            else if (s == "Mike")
                    guy->strategy = make_shared<Mike>();
            else if (s == "KillBot")
                    guy->strategy = make_shared<KillBot>();
                    */
            else    throw;
            guy.setPosition(playerPositionList[n]);
            guy.setPlayerID(n);
            this->playerList.emplace_back(guy);
        }
    }

    std::vector<shared_ptr<Player>> Table::getPlayersInBettingOrder(const std::vector<shared_ptr<Player>> pvector) {
        // returns a vector of pointers to players in the correct (game) order
        auto comp = [](const shared_ptr<Player> a, const shared_ptr<Player> b) -> bool { return a->getPosition() < b->getPosition(); };
        if( !pvector.empty() ) {
            vector<shared_ptr<Player>> ret = pvector;
            std::sort(ret.begin(), ret.end(), comp);
            return ret;
        }
        else {
            vector<shared_ptr<Player>> ret;
            std::transform(playerList.begin(), playerList.end(), back_inserter(ret), [] (auto& p) { return make_shared<Player>(p); });
            std::sort(ret.begin(), ret.end(), comp);
            return ret;
        }
        return std::vector<shared_ptr<Player>>();
    }
    // Raw pointer version
    std::vector<Player*> Table::getPlayersInBettingOrder(const std::vector<Player*> pvector) {
        // returns a vector of pointers to players in the correct (game) order
        auto comp = [](const Player* a, const Player* b) -> bool { return a->getPosition() < b->getPosition(); };
        if( !pvector.empty() ) {
            vector<Player*> ret = pvector;
            std::sort(ret.begin(), ret.end(), comp);
            return ret;
        }
        else {
            vector<Player*> ret;
            std::transform(playerList.begin(), playerList.end(), back_inserter(ret), [](auto&& p){ return &p; });
            std::sort(ret.begin(), ret.end(), comp);
            return ret;
        }
        return vector<Player*>();
    }
    void Table::clearPlayerHands() {
        // clears all player hands
        for(auto P : this->playerList) {
            P.resetHand();
        }
    }
    void Table::setPlayerBankrolls(int n) {
        // Sets everybodies wallets to n
        for(auto& P : this->playerList) {
            P.bankroll = n;
        }
    }


    void Table::resetCards(random_device& rd) {
        // Resets all cards everywhere and gets a fresh RNG for the deck. 
        std::mt19937_64 g(rd());
        
        deck = Deck();
        // shuffle cards
        std::shuffle(deck.cards.begin(), deck.cards.end(), g);
        //this->shuffleDeck();
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

    unique_ptr<Player> Table::getPlayerByID(const int& id) {
        return make_unique<Player>(*std::find_if(this->playerList.begin(), this->playerList.end(), [&id] (const auto& p) {return p.playerID == id;}));
    };

}
