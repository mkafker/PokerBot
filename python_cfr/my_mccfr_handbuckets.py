import random
import itertools
import copy
import time
import math
import json
import numpy as np
import pickle
import re

from datetime import datetime

import sys
sys.path.append('../')
from cpp_temp import poker as mike_poker

from helper_objects import Card, Deck
from better_showdown import showdown_poker

'''
This is supposed to be a version of MC-CFR+ which uses some version of
bucketting for hands. It needs to keep track of the hand strength bucket
as the current state for the player at every sub-round within a hand (ex:
hand strength of 80% after the flop, but then it goes down to 20% at the turn)

Currently this does not work. I need to write a new terminal_util function
and also come up with some sort of MC choosing of the starting player's cards.
Currently this is no set up to optimally save time while training.
'''

_MAX_BET = 5 # Includes the antee of $1
_BET_ROUNDS = 3
_HAND_BINS = 5
_NB_SIMULATION = 200
_N_ITERATIONS = 100
_load_previous_imap = True
#random.seed(3)

def custom_serializer(obj):
    if isinstance(obj, list):
        return json.dumps(obj, separators=(',', ':'))  # Serialize lists without indentation
    else:
        return obj

def main():
    """
    Run iterations of counterfactual regret minimization algorithm.
    """

    if _load_previous_imap:
        try:
            with open('trained_model.pickle', 'rb') as f:
                i_map = pickle.load(f)
        except Exception as e:
            print(e)
            print("Starting from scratch")
            i_map = {}
    else:
        i_map = {}  # map of information sets
    
    expected_game_value = 0

    start_from = "P1"
    start_time = time.time()
    for i in range(_N_ITERATIONS):
        
        expected_game_value += mccfr(i_map, start_from)
        print(i+1, expected_game_value / (i+1), len(i_map))
        for _, v in i_map.items():
            v.next_strategy()

        if start_from == "P1":
            start_from = "P2"
        else:
            start_from = "P1"

        if (i+1) % 10 == 0:
            
            print(f"Avg Iteration Time: {(time.time() - start_time) / (i+1)} s, Current time: {datetime.now()}")

            still_basic = 0
            for key, val in i_map.items():
                actions = valid_actions(key, _MAX_BET)
                basic_strat = [round(1 / len(actions), 3) for _ in actions]
                actual_strat = [round(x, 3) for x in val.get_average_strategy()]
                #print(key, basic_strat, actual_strat)
                if basic_strat == actual_strat or set([1.0, 0.0]) == set(actual_strat):
                    #print("Qualified as basic")
                    still_basic += 1


            print(f'Ratio of basic strategies: {round(still_basic / len(i_map), 5)}')

            try:
                temp_i_map = copy.copy(i_map)
                for key, val in i_map.items():
                    temp_i_map[key] = [round(float(i), 3) for i in val.strategy]
                with open("trained_model.json", 'w') as f:
                    json.dump(temp_i_map, f, default=custom_serializer, indent=4)
                with open("trained_model.pickle", "wb") as f:
                    pickle.dump(i_map, f)
            except Exception as e:
                print(e)
                print("Couldn't save current i_map")
        
        

    expected_game_value /= _N_ITERATIONS
    print("\n", expected_game_value)

    temp_i_map = copy.copy(i_map)
    for key, val in i_map.items():
        temp_i_map[key] = [round(float(i), 3) for i in val.strategy]
    with open("trained_model.json", 'w') as f:
        json.dump(temp_i_map, f, default=custom_serializer, indent=4)

    display_results(expected_game_value, i_map)

def mccfr(i_map, start_from, infoset_key=";;", deck = Deck(13, 4), 
        p1_cards=((-1,-1), (-1,-1)), p2_cards=((-1,-1),(-1,-1)), co_cards=((-1,-1)),
        p1_hs_hist = '', p2_hs_hist = '',
        pr_1=1, pr_2=1, pr_c=1):
    """
    Monte Carlo Counterfactual regret minimization algorithm. With hand bucketing.

    Parameters
    ----------

    i_map: dict
        Dictionary of all information sets.
    infoset_key : A key to represent the game tree path we have taken
        Ex: "P1;1/3;aa/cc/rRc", "PlayerID; PreFlop Hand Strength; Flop Hand Strength; History"

        'f': fold
        'c': check/call
        '1r': small raise ($1)
        '2r': large raise ($2)
    pr_1 : (0, 1.0), float
        The probability that player 1 reaches `history`.
    pr_2 : (0, 1.0), float
        The probability that player 2 reaches `history`.
    pr_c: (0, 1.0), float
        The probability contribution of chance events to reach `history`.
    """

    # print(infoset_key)
    # if infoset_key == "P1;0;aa/c1r":
    #     print(p1_cards, p2_cards)
    #     print(estimate_hand_strength(p1_cards, (), deck))
    #     print(estimate_hand_strength(p2_cards, (), deck))
    # if infoset_key == "P2;0;aa/c":
    #     print(p1_cards, p2_cards)
    #     print(estimate_hand_strength(p1_cards, (), deck))
    #     print(estimate_hand_strength(p2_cards, (), deck))

    if is_terminal_node(infoset_key):
        player_id = infoset_key.split(";")[0]
        return terminal_util_handbuckets(infoset_key, p1_cards, p2_cards, co_cards, player_id, deck)
    
    if is_chance_node(infoset_key):
        return chance_util(i_map, start_from, infoset_key, deck, 
            p1_cards, p2_cards, co_cards,
            p1_hs_hist, p2_hs_hist,
            pr_1, pr_2, pr_c)

    player_id = infoset_key.split(";")[0]
    action_history = infoset_key.split(";")[-1].split('/')
    final_history = extract_actions(action_history[-1])
    
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
    
    for i, action in enumerate(valid_actions(infoset_key, _MAX_BET)):
        next_infoset_key = swap_players(infoset_key, p1_hs_hist, p2_hs_hist) + action
        
        if player_id == "P1":
            action_utils[i] = -1 * mccfr(i_map, start_from, next_infoset_key, deck, 
                                    p1_cards, p2_cards, co_cards,
                                    p1_hs_hist, p2_hs_hist,
                                    pr_1 * strategy[i], pr_2, pr_c)
        else:
            action_utils[i] = -1 * mccfr(i_map, start_from, next_infoset_key, deck,
                                    p1_cards, p2_cards, co_cards,
                                    p1_hs_hist, p2_hs_hist,
                                    pr_1, pr_2 * strategy[i], pr_c)        

    # Utility of information set.
    util = sum(action_utils * strategy)
    regrets = action_utils - util

    if player_id == "P1":
        info_set.regret_sum += pr_2 * pr_c * regrets
    else:
        info_set.regret_sum += pr_1 * pr_c * regrets

    return util

def swap_players(infoset_key, p1_hs_hist, p2_hs_hist):
    
    if infoset_key[1] == "1":
        infoset_key = infoset_key[0] + str(2) + infoset_key[2:]
        op_hs = p2_hs_hist
    else:
        infoset_key = infoset_key[0] + str(1) + infoset_key[2:]
        op_hs = p1_hs_hist
    
    infoset_key = infoset_key.split(";")
    infoset_key[1] = str(op_hs)

    return ";".join(infoset_key)

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
        self.strategy_sum += self.reach_pr * self.strategy
        self.num += 1
        #self.strategy_sum += self.strategy
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
        
        # CFR +
        self.regret_sum = self.make_positive(self.regret_sum)

        return strategy
        
    def get_average_strategy(self):
        """
        Calculate average strategy over all iterations. This is the
        Nash equilibrium strategy.
        """

        strategy = self.strategy_sum / self.reach_pr_sum
        # strategy = self.strategy_sum / self.num

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

def sort_cards(cards):
    """
    Returns a tuple of sorted cards based on the rank first and then suit. See
    the cards class for numbers associated with these two. Sorted from low to 
    high
    """
    sorted_cards = sorted(list(cards), key=lambda x: (x.rank, x.suit))
    return tuple(sorted_cards)

def estimate_hand_strength(player_cards, co_cards, deck):
    simulation_results = []

    for i in range(_NB_SIMULATION):
        cardnum = 2 + 5 - len(co_cards)
        selected_cards = list(random.sample(list(deck.cards), cardnum))
        opponents_cards = tuple(selected_cards[0:2])
        community_cards = co_cards + tuple(selected_cards[2:])
        dummy_history = ['cc']
        winner = evaluate_winner(player_cards, opponents_cards, community_cards, dummy_history)

        if winner == 0: # player wins
            result = 1
        elif winner == 2: # tie
            result = 0
        else: # player looses
            result = 0

        simulation_results.append(result)
    average_win_rate = sum(simulation_results) / len(simulation_results)
      
    return average_win_rate

def clamp_hand_strength(avg_win_rate):
    hand_strength = None
    for i in range(_HAND_BINS):
        if avg_win_rate < 1/_HAND_BINS:
            hand_strength = 0
        elif i > 0 and i/_HAND_BINS <= avg_win_rate < (i+1)/_HAND_BINS:
            hand_strength = i
        elif avg_win_rate == 1:
            hand_strength = i
    
    if hand_strength == None or hand_strength == _HAND_BINS:
        print("FAILED TO CLAMP HAND STRENGTH")
        print(f"FOR AVG WIN RATE OF {avg_win_rate}")
        quit()
    if hand_strength < _HAND_BINS:
        return hand_strength

def extract_actions(single_history):
    """
    Takes a single string of history (no /) and 
    returns a list of actions in a form such as
    ['c', '1r', '10r', 'f']
    """
    actions = []
    current_action = ''
    for item in single_history:
        if item in ['a', 'c', 'f']:
            actions.append(item)
            current_action = ''
        elif item.isdigit():
            current_action += item
        elif item == 'r':
            actions.append(current_action + item)
            current_action = ''
    return actions
            
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
        for key, action in enumerate(extract_actions(history)):
            if key % 2 == 0:
                if action == 'c':
                    p1_temp = p2_temp
                elif 'r' in action:
                    p1_temp = p2_temp + int(action.replace('r', ''))
                elif action == 'a':
                    p1_temp += 1
            else:
                if action == 'c':
                    p2_temp = p1_temp
                elif 'r' in action:
                    p2_temp = p1_temp + int(action.replace('r', ''))
                elif action == 'a':
                    p2_temp += 1
        p1_commited += p1_temp
        p2_commited += p2_temp
    
    return p1_commited, p2_commited

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
        for bet_amount in range(1, max_bet + 1 - p2_commited):
            actions.append(str(bet_amount) + 'r')

    if player_id == "P2":
        if p1_commited > p2_commited: # Being raised against
            actions = ['f'] + actions
        for bet_amount in range(1, max_bet + 1 - p1_commited):
            actions.append(str(bet_amount) + 'r')

    return actions

def is_chance_node(infoset_key):
    """
    Determine if we are at a chance node based on tree history.
    """
    action_history = infoset_key.split(";")[-1].split('/')
    if len(action_history) == 1 and len(action_history[0]) == 0:
        return True
    if len(action_history) < _BET_ROUNDS + 1: # Not in final betting round
        final_history = extract_actions(action_history[-1])
        if len(final_history) > 1:          
            final_moves = final_history[-2:] # Both players have taken at least one action
            if final_moves[-1] in ['a', 'c']:
                return True
    return False

def chance_util(i_map, start_from, infoset_key, deck, 
    p1_cards, p2_cards, co_cards, 
    p1_hs_hist, p2_hs_hist,
    pr_1, pr_2, pr_c):
    
    player_id = infoset_key.split(";")[0]
    action_history = infoset_key.split(";")[-1].split('/')
    
    if len(action_history) == 1 and len(action_history[0]) == 0: # Initializing
        expected_value = 0
        n_possibilities = math.comb(len(deck.cards), 4) * math.comb(4, 2)

        # for deal in deck.draw_combinations(2): # Good MC-CFR, regular amount of MC
        for deal in deck.draw_a_unique_deck(2): # Okay MC-CFR, a lot of MC
        # for deal in [deck.draw_random_cards(2)]: # Bad MC-CFR, too much MC
            p_cards, new_deck = deal
            s_cards, final_deck = new_deck.draw_random_cards(2)
            p_cards = sort_cards(p_cards) # Primary
            s_cards = sort_cards(s_cards) # Secondary
            co_cards = ()

            if start_from == "P1":
                p1_hand_strength = clamp_hand_strength(estimate_hand_strength(p_cards, co_cards, final_deck))
                p1_hs_hist = str(p1_hand_strength)
                p2_hand_strength = clamp_hand_strength(estimate_hand_strength(s_cards, co_cards, final_deck))
                p2_hs_hist = str(p2_hand_strength)
                infoset_key = f'P1;{p1_hand_strength};aa/'
                expected_value += mccfr(i_map, start_from, infoset_key, final_deck, 
                                p_cards, s_cards, co_cards,
                                p1_hs_hist, p2_hs_hist,
                                1, 1, 1 / n_possibilities)

            else:
                p2_hand_strength = clamp_hand_strength(estimate_hand_strength(p_cards, co_cards, final_deck))
                p2_hs_hist = str(p2_hand_strength)
                infoset_key = f'P2;{p2_hand_strength};aa/'
                
                p1_hand_strength = clamp_hand_strength(estimate_hand_strength(s_cards, co_cards, final_deck))
                p1_hs_hist = str(p1_hand_strength)
                p1_infoset_key = f"P1;{p1_hand_strength};aa/"
                info_set = get_info_set(i_map, p1_infoset_key)
                info_set.reach_pr += pr_1
                strategy = info_set.strategy

                for i, action in enumerate(valid_actions(p1_infoset_key, _MAX_BET)):
                    next_infoset_key = infoset_key + action
                    expected_value += strategy[i] * mccfr(i_map, start_from, next_infoset_key, final_deck, 
                        s_cards, p_cards, co_cards,
                        p1_hs_hist, p2_hs_hist,
                        strategy[i], 1, 1 / n_possibilities)

        return expected_value / n_possibilities
    

    ############################
    # Using MC for Hand Strength
    ############################
    if len(action_history) > 1:
        expected_value = 0
        
        if len(action_history) == 2:
            cards_to_draw = 3
        else:
            cards_to_draw = 1
        n_possibilities = math.comb(len(deck.cards), cards_to_draw)

        for deal in deck.draw_a_unique_deck(cards_to_draw):
            new_co_cards, new_deck = deal
            #new_co_cards, new_deck = deck.draw_random_cards(cards_to_draw)
            new_co_cards = sort_cards(new_co_cards + co_cards)

            p1_hand_strength = clamp_hand_strength(estimate_hand_strength(p1_cards, new_co_cards, new_deck))
            new_p1_hs_hist = p1_hs_hist + "/" + str(p1_hand_strength)

            p2_hand_strength = clamp_hand_strength(estimate_hand_strength(p2_cards, new_co_cards, new_deck))
            new_p2_hs_hist = p2_hs_hist + "/" + str(p2_hand_strength)

            new_infoset_key = infoset_key.split(";")
            if player_id == "P2":
                new_infoset_key[0] = "P1"
            new_infoset_key = [new_infoset_key[0]] + [new_p1_hs_hist] + [new_infoset_key[-1]]
            new_infoset_key = ';'.join(new_infoset_key) + "/"

            expected_value += mccfr(i_map, start_from, new_infoset_key, new_deck, 
                                    p1_cards, p2_cards, new_co_cards,
                                    new_p1_hs_hist, new_p2_hs_hist,
                                    pr_1, pr_2, pr_c * 1 / n_possibilities)
        
        return expected_value / n_possibilities

def is_terminal_node(infoset_key):
    action_history = infoset_key.split(";")[-1].split('/')
    if len(action_history) > 1:
        
        if len(action_history) < _BET_ROUNDS + 1: # +1 is for antee round
            p1_commited, p2_commited = player_money_bet(action_history)
            if p1_commited == _MAX_BET and p2_commited == _MAX_BET:
                return True # Players have all in before the final round
        
        final_history = extract_actions(action_history[-1]) # Only look at the latest round
        if len(final_history) > 0 and 'f' == final_history[-1]:
            return True # A player folded
        if len(final_history) > 1 and len(action_history) == _BET_ROUNDS + 1:
            final_moves = final_history[-2:] # Both players have taken at least one action
            if final_moves[-1] == 'c':
                return True
            
    return False

def terminal_util_handbuckets(infoset_key, p1_cards, p2_cards, co_cards, player_id, deck):
    """
    Returns the utility of a terminal history.
    All action has finished, payouts are determined.
    """
    action_history = infoset_key.split(";")[-1].split('/')

    p1_commited, p2_commited = player_money_bet(action_history)

    if len(co_cards) < 5:
        new_co_cards, _ = deck.draw_random_cards(5 - len(co_cards))
        co_cards = new_co_cards + co_cards

    winner = evaluate_winner(p1_cards, p2_cards, co_cards, action_history)

    if winner == 2: # Tie
        return 0

    if player_id == "P1":
        if winner == 0:
            return p2_commited
        else:
            return -1 * p1_commited
    if player_id == "P2":
        if winner == 1:
            return p1_commited
        else:
            return -1 * p2_commited

def evaluate_winner_slow(cards_1, cards_2, community_cards, history):
    """
    This function returns 0, 1, 2 for player 1 winning, player 2 winning
    or both players tieing.
    """
    
    # Check for insufficent number of community cards
    if len(community_cards) < 5:
        print("NOT ENOUGH COMMUNITY CARD TO DETERMINE WINNER")
        quit()

    # Check for folding
    if len(history) > 0 and len(history[-1]) > 0 and history[-1][-1] == 'f':
        final_history = extract_actions(history[-1])
        if len(final_history) % 2 == 0:
            return 0 # Player 2 folded
        else:
            return 1 # Player 1 folded

    if showdown_poker(cards_1, community_cards) > showdown_poker(cards_2, community_cards):
        return 0 # Player 1 wins
    elif showdown_poker(cards_2, community_cards) > showdown_poker(cards_1, community_cards):
        return 1 # Player 2 wins
    else:
        return 2 # Tie


def evaluate_winner(cards_1, cards_2, community_cards, history):
    """
    This function returns 0, 1, 2 for player 1 winning, player 2 winning
    or both players tieing.
    """
    
    # Check for insufficent number of cards
    if not(len(cards_1) == 2):
        print(cards_1)
        raise ValueError(f"Incorrect number of player 1 cards: {len(cards_1)}")
    if not(len(cards_2) == 2):
        print(cards_2)
        raise ValueError(f"Incorrect number of player 2 cards: {len(cards_2)}")
    if not(len(community_cards) == 5):
        print(community_cards)
        raise ValueError(f"Incorrect number of community cards: {len(community_cards)}")

    # Check for folding
    if len(history) > 0 and len(history[-1]) > 0 and history[-1][-1] == 'f':
        final_history = extract_actions(history[-1])
        if len(final_history) % 2 == 0:
            return 0 # Player 2 folded
        else:
            return 1 # Player 1 folded

    cards_1 = [(card.rank, card.suit) for card in cards_1]
    cards_2 = [(card.rank, card.suit) for card in cards_2]
    community_cards = [(card.rank, card.suit) for card in community_cards]

    result = mike_poker.showdownHands(cards_1, cards_2, community_cards)
    return result




def display_results(ev, i_map):
    print('player 1 expected value: {}'.format(ev))
    print('player 2 expected value: {}'.format(-1 * ev))

    print('\nplayer 1 strategies:')
    sorter_items = []
    for infoset_key, val in i_map.items():
        player_id = infoset_key.split(";")[0]
        action_history = infoset_key.split(";")[-1].split('/')

        if player_id == "P1":
            print(infoset_key, ['{:03.2f}'.format(x) for x in val.get_average_strategy()])
    
    print('\nplayer 2 strategies:')
    for infoset_key, val in i_map.items():
        player_id = infoset_key.split(";")[0]
        action_history = infoset_key.split(";")[-1].split('/')

        if player_id == "P2": 
            print(infoset_key, ['{:03.2f}'.format(x) for x in val.get_average_strategy()])

if __name__ == "__main__":
    main()
