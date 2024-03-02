import random
import numpy as np
import re

## Kuhn-sterroids
# Number of betting rounds: 2
# Number of cards: 4 (J, Q, K, A)
# Allowed bets: r, R (small raise, big raise)

_N_CARDS = 4
_MAX_BET = 3 # Includes the antee

class Card:
    SUIT_TO_STRING = {
        1: "s",
        2: "h",
        3: "d",
        4: "c"
    }
    
    RANK_TO_STRING = {
        2: "2",
        3: "3",
        4: "4",
        5: "5",
        6: "6",
        7: "7",
        8: "8",
        9: "9",
        10: "T",
        11: "J",
        12: "Q",
        13: "K",
        14: "A"
    }

    RANK_JACK = 11
    RANK_QUEEN = 12
    RANK_KING = 13
    RANK_ACE = 14
    
    STRING_TO_SUIT = dict([(v, k) for k, v in SUIT_TO_STRING.items()])
    STRING_TO_RANK = dict([(v, k) for k, v in RANK_TO_STRING.items()])

    def __init__(self, rank, suit):
        """
        Create a card. Rank is 2-14, representing 2 through Ace,
        while suit is 1-4 representing spades, hearts, diamonds, clubs
        """
        self.rank = rank
        self.suit = suit

    def __repr__(self):
        return "%s%s" % (self.RANK_TO_STRING[self.rank], self.SUIT_TO_STRING[self.suit])
    
    def __eq__(self, other):
        return (isinstance(other, self.__class__) and self.rank == other.rank and self.suit == other.suit)
    
    def __hash__(self):
        return hash((self.rank, self.suit))

def main():
    """
    Run iterations of counterfactual regret minimization algorithm.
    """
    i_map = {}  # map of information sets
    n_iterations = 100
    expected_game_value = 0

    for i in range(n_iterations):
        
        expected_game_value += cfr(i_map)
        print(i+1, expected_game_value / (i+1))
        for _, v in i_map.items():
            v.next_strategy()

    expected_game_value /= n_iterations

    #quit()
    print()
    info_set = get_info_set(i_map, "P2;3;;aa/crR")
    print(info_set)
    print("strategy", info_set.strategy)
    print("strategy_sum", info_set.strategy_sum)
    print("regret_sum", info_set.regret_sum)
    print("reach_pr_sum", info_set.reach_pr_sum)
    #quit()

    print()
    print(expected_game_value)
    for key, val in i_map.items():
        print(val)

    display_results(expected_game_value, i_map)

def cfr(i_map, infoset_key=";;;", opponent_card=-1, pr_1=1, pr_2=1, pr_c=1):
    """
    Counterfactual regret minimization algorithm.

    Parameters
    ----------

    i_map: dict
        Dictionary of all information sets.
    infoset_key : A key to represent the game tree path we have taken
        Ex: "P1;1;3;aa/cc/rRc", "PlayerID; Player Card; Community Card; History"

        'f': fold
        'c': check/call
        'r': small raise ($1)
        'R': large raise ($2)
    opponent_card : (0, 3), int
        opennent's card
    pr_1 : (0, 1.0), float
        The probability that player 1 reaches `history`.
    pr_2 : (0, 1.0), float
        The probability that player 2 reaches `history`.
    pr_c: (0, 1.0), float
        The probability contribution of chance events to reach `history`.
    """

    # if infoset_key == "P2;3;;aa/c" and opponent_card == 2:
    #     info_set = get_info_set(i_map, infoset_key)
    #     print()
    #     print(infoset_key, pr_1, pr_2, pr_c)
    #     print(info_set.regret_sum)
    #     print(info_set.strategy)
    #     #quit()
    # if infoset_key == "P1;2;;aa/cr" and opponent_card == 3:
    #     info_set = get_info_set(i_map, infoset_key)
    #     print()
    #     print(infoset_key, pr_1, pr_2, pr_c)
    #     print(info_set.regret_sum)
    #     print(info_set.strategy)
    #     #quit()
    
    # if infoset_key == "P2;3;;aa/crR" and opponent_card == 2:
    #     info_set = get_info_set(i_map, infoset_key)
    #     print()
    #     print(infoset_key, pr_1, pr_2, pr_c)
    #     print(info_set.regret_sum)
    #     print(info_set.strategy)
    #     #quit()

    if is_terminal_node(infoset_key, betting_rounds = 2, max_bet = _MAX_BET):

        player_id = infoset_key.split(";")[0]
        if player_id == "P1":
            p1_card = int(infoset_key.split(";")[1])
            p2_card = opponent_card
        else:
            p1_card = opponent_card
            p2_card = int(infoset_key.split(";")[1])

        return terminal_util(infoset_key, p1_card, p2_card, player_id)
    
    if is_chance_node(infoset_key, betting_rounds = 2):
        return chance_util(i_map, infoset_key, opponent_card, pr_1, pr_2, pr_c)

    player_id = infoset_key.split(";")[0]
    p_card = infoset_key.split(";")[1] # player card
    o_card = opponent_card
    co_card = infoset_key.split(";")[2] # community card
    action_history = infoset_key.split(";")[-1].split('/')
    final_history = action_history[-1]
    
    if len(final_history) % 2 == 0 and player_id == "P2":
        print("ERROR")
        print(infoset_key)
        quit()
    if len(final_history) % 2 == 1 and player_id == "P1":
        print("ERROR")
        print(infoset_key)
        quit()

    info_set = get_info_set(i_map, infoset_key)

    strategy = info_set.strategy
    if player_id == "P1":
        info_set.reach_pr += pr_1
    else:
        info_set.reach_pr += pr_2

    # Counterfactual utility per action.
    action_utils = np.zeros(info_set.n_actions)

    temp_infoset_key, new_opponent_card = swap_players(infoset_key, opponent_card)
    for i, action in enumerate(valid_actions(infoset_key, _MAX_BET)):
        next_infoset_key = temp_infoset_key + action
        # if infoset_key == 'P2;3;;aa/c' and opponent_card == 2:
        #     print("next_infoset_key", next_infoset_key, pr_1, pr_2)
        
        if player_id == "P1":
            action_utils[i] = -1 * cfr(i_map, next_infoset_key, new_opponent_card,
                                    pr_1 * strategy[i], pr_2, pr_c)
        else:
            action_utils[i] = -1 * cfr(i_map, next_infoset_key, new_opponent_card,
                                    pr_1, pr_2 * strategy[i], pr_c)        



    # Utility of information set.
    util = sum(action_utils * strategy)
    regrets = action_utils - util

    # if infoset_key == 'P2;3;;aa/c' and opponent_card == 2:
    #     print(infoset_key, action_utils, opponent_card)
    #     print(regrets)
        

    if player_id == "P1":
        info_set.regret_sum += pr_2 * pr_c * regrets
    else:
        info_set.regret_sum += pr_1 * pr_c * regrets

    return util

def swap_players(infoset_key, opponent_card):
    infoset_key = list(infoset_key)
    if infoset_key[1] == "1":
        infoset_key[1] = "2"
    else:
        infoset_key[1] = "1"
    infoset_key[3], opponent_card = str(opponent_card), infoset_key[3]
    return "".join(infoset_key), int(opponent_card)

class InformationSet():
    def __init__(self, key):
        """
        InformationSet: A certain gamestate (ex: Cards in hand + Community cards + bet history)

        regret_sum: How much regret is associated with taking an action different from
        the one that was chosen. (Ex: Choose action 2, regret_sum => [-1,2,0,3], your
        outcome was better than the outcome of action 0, but worse than the outcomes of
        actions 1 and 3.)
        
        strategy_sum: The sum of the strategies used so far, each weighted by the probability of reaching this gamestate
        through a certain history. This is added to for every history.

        strategy: An array of probabilities for all of the valid actions (Ex: [0.3, 0.3, 0.3] for ["FOLD", "CALL", "RAISE"])

        reach_pr: The probability that this information set is reached for the current strategy

        reach_pr_sum: the sum of the reach_prs (should be the same size array as strategy_sum)
        """
        self.key = key
        self.n_actions = len(valid_actions(key, _MAX_BET))
        self.regret_sum = np.zeros(self.n_actions)
        self.strategy_sum = np.zeros(self.n_actions)
        self.strategy = np.repeat(1/self.n_actions, self.n_actions)
        self.reach_pr = 0
        self.reach_pr_sum = 0
        self.num = 0

    def next_strategy(self):
        # if self.reach_pr == 0:
        #     self.reach_pr = np.random.uniform(0, 0.05)
        # self.strategy_sum += self.reach_pr * self.strategy
        self.num += 1
        self.strategy_sum += self.strategy
        self.strategy = self.calc_strategy()
        self.reach_pr_sum += self.reach_pr
        self.reach_pr = 0

    def calc_strategy(self):
        """
        Calculate current strategy from the sum of regret.
        """
        if all([i < 0 for i in self.regret_sum]): # If all the outcomes are negative regret
            strategy = self.regret_sum + min(self.regret_sum)
        else:
            strategy = self.make_positive(self.regret_sum)
        total = sum(strategy)
        if total > 0:
            strategy = strategy / total
        else:
            n = self.n_actions
            strategy = np.repeat(1/n, n)

        return strategy
        

    def get_average_strategy(self):
        """
        Calculate average strategy over all iterations. This is the
        Nash equilibrium strategy.
        """

        # strategy = self.strategy_sum / self.reach_pr_sum
        strategy = self.strategy_sum / self.num

        # Purify to remove actions that are likely a mistake
        strategy = np.where(strategy < 0.001, 0, strategy)

        # Re-normalize
        total = sum(strategy)
        strategy /= total

        return strategy

    def make_positive(self, x):
        return np.where(x > 0, x, 0)

    def __str__(self):
        strategies = ['{:03.2f}'.format(x)
                      for x in self.get_average_strategy()]
        return '{} {}'.format(self.key.ljust(6), strategies)

def get_info_set(i_map, infoset_key):
    """
    Retrieve information set from dictionary.
    """

    if infoset_key not in i_map:
        info_set = InformationSet(infoset_key)
        i_map[infoset_key] = info_set
        return info_set

    return i_map[infoset_key]


def valid_actions(infoset_key, max_bet):
    """
    Returns a list of valid actions based off the tree history.
    """
    player_id = infoset_key.split(";")[0]
    action_history = infoset_key.split(";")[-1].split('/')

    p1_commited, p2_commited = player_money_bet(action_history)
    if p1_commited > max_bet or p2_commited > max_bet:
        error_msg = f"P1 or P2 have bet too much money! p1_commited: {p1_commited} and p2_commited: {p2_commited}"
        error_msg += f" with a max bet of {max_bet}"
        raise ValueError(error_msg)

    actions = ['c']
    if player_id == "P1":
        if p2_commited > p1_commited: # Being raised against
            actions = ['f'] + actions
            if p2_commited == p1_commited + 1 and p1_commited == max_bet - 2:
                actions.append('R')
        elif p1_commited == max_bet - 1:
            actions.append('r')
        elif p1_commited == max_bet - 2:
            actions.append('r')
            actions.append('R')
    if player_id == "P2":
        if p1_commited > p2_commited: # Being raised against
            actions = ['f'] + actions
            if p1_commited == p2_commited + 1 and p2_commited == max_bet - 2:
                actions.append('R')
        elif p2_commited == max_bet - 1:
            actions.append('r')
        elif p2_commited == max_bet - 2:
            actions.append('r')
            actions.append('R')

    return actions


def is_chance_node(infoset_key, betting_rounds):
    """
    Determine if we are at a chance node based on tree history.
    """
    action_history = infoset_key.split(";")[-1].split('/')
    if len(action_history) == 1 and len(action_history[0]) == 0:
        return True
    if len(action_history) < betting_rounds + 1: # Not in final betting round
        final_history = action_history[-1]
        if len(final_history) > 1: 
            final_moves = final_history[-2:] # Both players have taken at least one action
            if final_moves in ['aa', 'cc', 'rc']:
                return True
    return False

def chance_util(i_map, infoset_key, opponent_card, pr_1, pr_2, pr_c):
    
    player_id = infoset_key.split(";")[0]
    action_history = infoset_key.split(";")[-1].split('/')
    
    if len(action_history) == 1 and len(action_history[0]) == 0: # Initializing
        expected_value = 0
        n_possibilities = 6 * 2 # (4 choose 2) * 2 players
        for i in range(_N_CARDS):
            for j in range(_N_CARDS):
                if i != j:
                    infoset_key = f'P1;{i};;aa/'
                    opponent_card = j
                    expected_value += cfr(i_map, infoset_key, opponent_card,
                                    1, 1, 1 / n_possibilities)
        return expected_value / n_possibilities
    
    if len(action_history) == 2: # Finished first betting round
        expected_value = 0
        n_possibilities = 2 # Only 2 cards left in the deck
        player_card = int(infoset_key.split(";")[1])
        for i in range(_N_CARDS):
            if i != player_card and i != opponent_card:
                new_infoset_key = infoset_key.split(";")
                new_infoset_key = new_infoset_key[0:2] + [str(i)] + [new_infoset_key[-1]]
                new_infoset_key = ';'.join(new_infoset_key) + "/"
                
                if player_id == "P2":
                    new_infoset_key, opponent_card = swap_players(new_infoset_key, opponent_card)

                expected_value += cfr(i_map, new_infoset_key, opponent_card,
                                    pr_1, pr_2, pr_c * 1 / n_possibilities)
        return expected_value / n_possibilities

def is_terminal_node(infoset_key, betting_rounds, max_bet):
    action_history = infoset_key.split(";")[-1].split('/')
    if len(action_history) > 1:
        
        if len(action_history) < betting_rounds + 1: # +1 is for antee round
            p1_commited, p2_commited = player_money_bet(action_history)
            if p1_commited == max_bet and p2_commited == max_bet:
                return True # Players have all in before the final round
        
        final_history = action_history[-1] # Only look at the latest round
        if len(final_history) > 0 and 'f' == final_history[-1]:
            return True # A player folded
        if len(final_history) > 1:
            final_moves = final_history[-2:] # Both players have taken at least one action
            if final_moves in ['Rc']:
                return True
            if len(action_history) == betting_rounds + 1:
                if final_moves in ['cc', 'rc', 'Rc']:
                    return True
    return False

def player_money_bet(action_history):
    """
    Returns the amount of money p1 and p2 have bet
    (not including the antee) with the format of p1, p2.
    Assumes that player 1 always moves first for each betting round.
    """
    p1_commited = 0
    p2_commited = 0
    for history in action_history:
        p1_temp = 0
        p2_temp = 0
        for key, action in enumerate(history):
            if key % 2 == 0:
                if action == 'c':
                    p1_temp = p2_temp
                elif action == 'r':
                    p1_temp += 1
                elif action == 'R':
                    p1_temp += 2
                elif action == 'a':
                    p1_temp += 1
            else:
                if action == 'c':
                    p2_temp = p1_temp
                elif action == 'r':
                    p2_temp += 1
                elif action == 'R':
                    p2_temp += 2
                elif action == 'a':
                    p2_temp += 1
        p1_commited += p1_temp
        p2_commited += p2_temp
    
    return p1_commited, p2_commited

def terminal_util(infoset_key, p1_card, p2_card, player_id):
    """
    Returns the utility of a terminal history.
    All action has finished, payouts are determined.
    """

    co_card = infoset_key.split(";")[2]
    action_history = infoset_key.split(";")[-1].split('/')

    p1_commited, p2_commited = player_money_bet(action_history)
    winner = evaluate_winner([p1_card], [p2_card], [co_card], action_history)

    if player_id == "P1":
        if winner == 1:
            return p2_commited
        else:
            return -1 * p1_commited
    if player_id == "P2":
        if winner == 2:
            return p1_commited
        else:
            return -1 * p2_commited

def evaluate_winner(cards_1, cards_2, community_cards, history):
    """
    This function returns 1, 2, or -1 for player winning,
    opponent winning, or a tie. 
    
    For this modified version of Kuhn
    poker (J, Q, K, A), it just requires comparing the cards of each
    player and the winner is the one with the higher card. There are
    no ties.
    """
    
    # Check for folding
    final_history = history[-1]
    if len(final_history) > 0 and final_history[-1] == 'f':
        if len(final_history) % 2 == 0:
            return 1
        else:
            return 2

    # Assuming no one folded, evaluate based on high card
    card_1 = int(cards_1[0])
    card_2 = int(cards_2[0])

    if card_1 > card_2:
        return 1
    else:
        return 2


def card_convert(card):
    if card == 0:
        return 'J'
    if card == 1:
        return 'Q'
    if card == 2:
        return 'K'
    if card == 3:
        return 'A'

def display_results(ev, i_map):
    print('player 1 expected value: {}'.format(ev))
    print('player 2 expected value: {}'.format(-1 * ev))

    print()
    print('\nplayer 1 strategies:')
    sorter_items = []
    for infoset_key, val in i_map.items():
        player_id = infoset_key.split(";")[0]
        player_card = infoset_key.split(";")[1]
        community_card = infoset_key.split(";")[2]
        action_history = infoset_key.split(";")[-1].split('/')

        if player_id == "P1":
            print("P1", card_convert(int(player_card)), "/".join(action_history).ljust(12), ['{:03.2f}'.format(x) for x in val.get_average_strategy()])
    
    print('\nplayer 2 strategies:')
    for infoset_key, val in i_map.items():
        player_id = infoset_key.split(";")[0]
        player_card = infoset_key.split(";")[1]
        community_card = infoset_key.split(";")[2]
        action_history = infoset_key.split(";")[-1].split('/')

        if player_id == "P2": 
            print("P2", card_convert(int(player_card)), "/".join(action_history).ljust(12), ['{:03.2f}'.format(x) for x in val.get_average_strategy()])

if __name__ == "__main__":
    main()