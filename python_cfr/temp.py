import random
import itertools
import copy
import time
import math
import json
import numpy as np
import pickle
import re

import matplotlib.pyplot as plt

from datetime import datetime

import sys
sys.path.append('../')
from cpp_temp import poker as mike_poker

from helper_objects import Card, Deck
from better_showdown import showdown_poker

_NB_SIMULATION = 10000
random.seed(1)

def estimate_hand_strength(player_cards, co_cards, deck):
    simulation_results = []

    for i in range(_NB_SIMULATION):
        cardnum = 2 + 5 - len(co_cards)
        selected_cards = list(random.sample(list(deck.cards), cardnum))
        opponents_cards = sorted(tuple(selected_cards[0:2]))
        community_cards = sorted(co_cards + tuple(selected_cards[2:]))
        dummy_history = ['cc']
        winner = evaluate_winner(player_cards, opponents_cards, community_cards, dummy_history)
        #winner_slow = evaluate_winner_slow(player_cards, opponents_cards, community_cards, dummy_history)       

        if winner == 0: # player wins
            simulation_results.append(1)
        elif winner == 2: # tie
            simulation_results.append(0)
        else: # player looses
            simulation_results.append(0)

    average_win_rate = sum(simulation_results) / len(simulation_results)

    return average_win_rate

def estimate_hand_strength_slow(player_cards, co_cards, deck):
    simulation_results = []

    for i in range(_NB_SIMULATION):
        cardnum = 2 + 5 - len(co_cards)
        selected_cards = list(random.sample(list(deck.cards), cardnum))
        opponents_cards = sorted(tuple(selected_cards[0:2]))
        community_cards = sorted(co_cards + tuple(selected_cards[2:]))
        dummy_history = ['cc']
        winner = evaluate_winner_slow(player_cards, opponents_cards, community_cards, dummy_history)

        if winner == 0: # player wins
            simulation_results.append(1)
        elif winner == 2: # tie
            simulation_results.append(0)
        else: # player looses
            simulation_results.append(0)

        
    average_win_rate = sum(simulation_results) / len(simulation_results)

    return average_win_rate

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

    # print(cards_1)
    # print(cards_2)
    # print(community_cards)

    result = mike_poker.showdownHands(cards_1, cards_2, community_cards)
    # print(result)
    return result

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

deck = Deck(13, 4)
hand_strengths = []
for deal in deck.draw_combinations(2):
    player_cards, new_dekc = deal
    hand_strengths.append(estimate_hand_strength(player_cards, (), new_deck))


plt.hist(hand_strengths)
plt.show()
quit()


cards = (Card(2,2), Card(3,1))
for card in cards:
    deck.cards.remove(card)
print(estimate_hand_strength(cards, (), deck))
print(estimate_hand_strength_slow(cards, (), deck))