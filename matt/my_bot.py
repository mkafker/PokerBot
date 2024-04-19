import random
import itertools
import copy
import time
import math
import json
import numpy as np
import pickle
import re

from helper_objects import Card, Deck
from better_showdown import showdown_poker

_NB_SIMULATION = 200
_HAND_BINS = 5

suit_to_num = {
    's': 1,
    'h': 2,
    'd': 3,
    'c': 4
}
rank_to_num = {
    '2': 2,
    '3': 3,
    '4': 4,
    '5': 5,
    '6': 6,
    '7': 7,
    '8': 8,
    '9': 9,
    'T': 10,
    'J': 11,
    'Q': 12,
    'K': 13,
    'A': 14,
}

def make_cards(cards):
    new_cards = []
    for card in cards:
        rank = rank_to_num[card[0]]
        suit = suit_to_num[card[1]]
        new_cards.append(Card(rank, suit))
    return tuple(new_cards)

def make_deck(my_cards, board_cards):
    cards = my_cards + board_cards
    cards = make_cards(cards)
    deck = Deck(13, 4)
    for card in cards:
        deck.cards.remove(card)
    return deck

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