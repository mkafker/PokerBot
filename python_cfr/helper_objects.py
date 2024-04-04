import random
import itertools
import copy
import math
import json
import numpy as np
import re

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
        return "(%s, %s)" % (self.rank, self.suit)
    
    def pretty_repr(self):
        return "%s%s" % (self.RANK_TO_STRING[self.rank], self.SUIT_TO_STRING[self.suit])
    
    def __eq__(self, other):
        return (isinstance(other, self.__class__) and self.rank == other.rank and self.suit == other.suit)
    
    def __lt__(self, other):
        if self.rank == other.rank:
            return self.suit < other.suit
        return self.rank < other.rank

    def __gt__(self, other):
        if self.rank == other.rank:
            return self.suit > other.suit
        return self.rank > other.rank

    def __hash__(self):
        return hash((self.rank, self.suit))

class Deck:
    def __init__(self, ranks, suits):
        self.cards = self.build_deck(ranks, suits)

    def build_deck(self, ranks, suits):
        return {Card(rank, suit) for rank, suit in itertools.product([2+x for x in range(ranks)], [1+x for x in range(suits)])}

    def shuffle(self):
        import random
        self.cards = set(random.sample(list(self.cards), len(self.cards)))

    def draw_combinations(self, cardnum):
        combinations = []
        for combination in itertools.combinations(self.cards, cardnum):
            remaining_deck = Deck(1, 1) # dummy deck
            remaining_deck.cards = copy.copy(self.cards)
            for card in combination:
                remaining_deck.cards.remove(card)
            combinations.append((tuple(sorted(combination)), remaining_deck))
        return combinations
    
    def draw_a_unique_deck(self, cardnum):
        combinations = []
        remaining_deck = Deck(1, 1) # dummy deck
        remaining_deck.cards = copy.copy(self.cards)
        while len(remaining_deck.cards) > cardnum:
            cards, remaining_deck = remaining_deck.draw_random_cards(cardnum)
            final_deck = Deck(1, 1) # dummy deck
            final_deck.cards = copy.copy(self.cards)
            for card in cards:
                final_deck.cards.remove(card)
            combinations.append((tuple(cards), final_deck))
        return combinations

    def draw_random_cards(self, cardnum):
        selected_cards = sorted(random.sample(list(self.cards), cardnum))
        remaining_deck = Deck(1, 1)
        remaining_deck.cards = copy.copy(self.cards)
        for card in selected_cards:
            remaining_deck.cards.remove(card)
        return tuple(selected_cards), remaining_deck