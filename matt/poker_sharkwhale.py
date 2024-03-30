from pypokerengine.players import BasePokerPlayer
from pypokerengine.utils.card_utils import gen_cards, estimate_hole_card_win_rate
from pypokerengine.api.game import setup_config, start_poker
import numpy as np


# Fish player always calls
class FishPlayer(BasePokerPlayer):  # Do not forget to make parent class as "BasePokerPlayer"

    #  we define the logic to make an action through this method. (so this method would be the core of your AI)
    def declare_action(self, valid_actions, hole_card, round_state):
        # valid_actions format => [raise_action_info, call_action_info, fold_action_info]
        call_action_info = valid_actions[1]
        action, amount = call_action_info["action"], call_action_info["amount"]
        return action, amount   # action returned here is sent to the poker engine

    def receive_game_start_message(self, game_info):
        pass

    def receive_round_start_message(self, round_count, hole_card, seats):
        pass

    def receive_street_start_message(self, street, round_state):
        pass

    def receive_game_update_message(self, action, round_state):
        pass

    def receive_round_result_message(self, winners, hand_info, round_state):
        pass



NB_SIMULATION = 1000

# Use a built-in function to estimate the win rate, then call if the win rate is above a certain amount
class HonestPlayer(BasePokerPlayer):

    def declare_action(self, valid_actions, hole_card, round_state):
        community_card = round_state['community_card']
        win_rate = estimate_hole_card_win_rate(
                nb_simulation=NB_SIMULATION,
                nb_player=self.nb_player,
                hole_card=gen_cards(hole_card),
                community_card=gen_cards(community_card)
                )
        if win_rate >= 1.0 / self.nb_player:
            action = valid_actions[1]  # fetch CALL action info
        else:
            action = valid_actions[0]  # fetch FOLD action info
        return action['action'], action['amount']

    def receive_game_start_message(self, game_info):
        self.nb_player = game_info['player_num']

    def receive_round_start_message(self, round_count, hole_card, seats):
        pass

    def receive_street_start_message(self, street, round_state):
        pass

    def receive_game_update_message(self, action, round_state):
        pass

    def receive_round_result_message(self, winners, hand_info, round_state):
        pass


class SharkPlayerMin(BasePokerPlayer):

    def declare_action(self, valid_actions, hole_card, round_state):
        community_card = round_state['community_card']
        win_rate = estimate_hole_card_win_rate(
            nb_simulation=NB_SIMULATION,
            nb_player=self.nb_player,
            hole_card=gen_cards(hole_card),
            community_card=gen_cards(community_card)
        )
        
        # Customize these thresholds as needed
        fold_threshold = 1.0 / self.nb_player
        call_threshold = 0.5
        raise_threshold = 0.7
        
        if win_rate < fold_threshold:
            action = valid_actions[0]  # fold
        elif win_rate < call_threshold:
            action = valid_actions[1]  # call
        elif win_rate < raise_threshold:
            # Moderate confidence, might still want to call to see the next card
            action = valid_actions[1]  # call
        else:
            # action = valid_actions[0]  # fold ####### TEMP


            # High confidence, try to raise
            # Check if raising is a valid action
            # [print(act) for act in valid_actions]
            # assert False
            raise_action = [act for act in valid_actions if act['action'] == 'raise']
            print(raise_action)
            if raise_action:
                action = raise_action[0]
                # action['amount'] = int(raise_action[0]['amount']['min'] + np.random.rand()*(raise_action[0]['amount']['max'] - raise_action[0]['amount']['min'])//1)
                action['amount'] = raise_action[0]['amount']['min']
                print(action['amount'])
                # action['amount'] = 15 # This works

            else:
                action = valid_actions[1]  # default to call if raise is not available

            # Appears to work
            # action = valid_actions[2] 
            # action['amount'] = 15


        return action['action'], action['amount']

    def receive_game_start_message(self, game_info):
        self.nb_player = game_info['player_num']

    def receive_round_start_message(self, round_count, hole_card, seats):
        pass

    def receive_street_start_message(self, street, round_state):
        pass

    def receive_game_update_message(self, action, round_state):
        pass

    def receive_round_result_message(self, winners, hand_info, round_state):
        pass

class SharkPlayerMax(BasePokerPlayer):

    def declare_action(self, valid_actions, hole_card, round_state):
        community_card = round_state['community_card']
        win_rate = estimate_hole_card_win_rate(
            nb_simulation=NB_SIMULATION,
            nb_player=self.nb_player,
            hole_card=gen_cards(hole_card),
            community_card=gen_cards(community_card)
        )
        
        # Customize these thresholds as needed
        fold_threshold = 1.0 / self.nb_player
        call_threshold = 0.5
        raise_threshold = 0.7
        
        if win_rate < fold_threshold:
            action = valid_actions[0]  # fold
        elif win_rate < call_threshold:
            action = valid_actions[1]  # call
        elif win_rate < raise_threshold:
            # Moderate confidence, might still want to call to see the next card
            action = valid_actions[1]  # call
        else:
            # action = valid_actions[0]  # fold ####### TEMP


            # High confidence, try to raise
            # Check if raising is a valid action
            # [print(act) for act in valid_actions]
            # assert False
            raise_action = [act for act in valid_actions if act['action'] == 'raise']
            print(raise_action)
            if raise_action:
                action = raise_action[0]
                # action['amount'] = int(raise_action[0]['amount']['min'] + np.random.rand()*(raise_action[0]['amount']['max'] - raise_action[0]['amount']['min'])//1)
                action['amount'] = raise_action[0]['amount']['max']
                print(action['amount'])
                # action['amount'] = 15 # This works

            else:
                action = valid_actions[1]  # default to call if raise is not available

            # Appears to work
            # action = valid_actions[2] 
            # action['amount'] = 15


        return action['action'], action['amount']

    def receive_game_start_message(self, game_info):
        self.nb_player = game_info['player_num']

    def receive_round_start_message(self, round_count, hole_card, seats):
        pass

    def receive_street_start_message(self, street, round_state):
        pass

    def receive_game_update_message(self, action, round_state):
        pass

    def receive_round_result_message(self, winners, hand_info, round_state):
        pass

class SharkPlayerLinear(BasePokerPlayer):

    def declare_action(self, valid_actions, hole_card, round_state):
        community_card = round_state['community_card']
        win_rate = estimate_hole_card_win_rate(
            nb_simulation=NB_SIMULATION,
            nb_player=self.nb_player,
            hole_card=gen_cards(hole_card),
            community_card=gen_cards(community_card)
        )
        
        # Customize these thresholds as needed
        fold_threshold = 1.0 / self.nb_player
        call_threshold = 0.5
        raise_threshold = 0.7
        
        if win_rate < fold_threshold:
            action = valid_actions[0]  # fold
        elif win_rate < call_threshold:
            action = valid_actions[1]  # call
        elif win_rate < raise_threshold:
            # Moderate confidence, might still want to call to see the next card
            action = valid_actions[1]  # call
        else:
            raise_action = [act for act in valid_actions if act['action'] == 'raise']
            print(raise_action)
            if raise_action:
                action = raise_action[0]
                # action['amount'] = int(raise_action[0]['amount']['min'] + np.random.rand()*(raise_action[0]['amount']['max'] - raise_action[0]['amount']['min'])//1)
                action['amount'] = (1-win_rate)/(1-raise_threshold)*raise_action[0]['amount']['min'] + (1-(1-win_rate)/(1-raise_threshold))*raise_action[0]['amount']['max']
                print(action['amount'])
                # action['amount'] = 15 # This works

            else:
                action = valid_actions[1]  # default to call if raise is not available

            # Appears to work
            # action = valid_actions[2] 
            # action['amount'] = 15


        return action['action'], action['amount']

    def receive_game_start_message(self, game_info):
        self.nb_player = game_info['player_num']

    def receive_round_start_message(self, round_count, hole_card, seats):
        pass

    def receive_street_start_message(self, street, round_state):
        pass

    def receive_game_update_message(self, action, round_state):
        pass

    def receive_round_result_message(self, winners, hand_info, round_state):
        pass

import pypokerengine.utils.card_utils as card_utils
class WhalePlayer(BasePokerPlayer):

    def estimate_hand_strength(self, nb_simulation, nb_player, hole_card, community_card):
        # Utilize PyPokerEngine's functions for simulation and evaluation
        outcomes = [
            card_utils._montecarlo_simulation(nb_player, hole_card, community_card)
            for _ in range(nb_simulation)
        ]
        sr = np.array(outcomes)
        return np.mean(sr),np.std(sr)
    
    def declare_action(self, valid_actions, hole_card, round_state):
        community_card = round_state['community_card']
        hole_cards = card_utils.gen_cards(hole_card)
        community_cards = card_utils.gen_cards(community_card)
        
        win_rate, wr_std = self.estimate_hand_strength(
            NB_SIMULATION,
            self.nb_player,
            hole_cards,
            community_cards
        )
        print(win_rate, wr_std)
        
        
        # Customize these thresholds as needed
        call_threshold = 1.0 / self.nb_player
        raise_threshold = 0.5
        aggressive_threshold = 0.7
        std_threshold = 0.4

        optimistic_rate = np.max([win_rate+wr_std,1])
        
        if win_rate < call_threshold:
            action = valid_actions[0]  # fold
        elif win_rate < raise_threshold and wr_std < std_threshold:
            action = valid_actions[1]  # call
        elif win_rate < raise_threshold and wr_std >= std_threshold:
            raise_action = [act for act in valid_actions if act['action'] == 'raise']
            print(raise_action)
            if raise_action:
                action = raise_action[0]
                
                action['amount'] = raise_action[0]['amount']['min']
                print(action['amount'])

            else:
                action = valid_actions[1]  # default to call if raise is not available
        else:

            raise_action = [act for act in valid_actions if act['action'] == 'raise']
            # print(raise_action)
            if raise_action:
                action = raise_action[0]
                # action['amount'] = int(raise_action[0]['amount']['min'] + np.random.rand()*(raise_action[0]['amount']['max'] - raise_action[0]['amount']['min'])//1)
                # action['amount'] = 0.5*(raise_action[0]['amount']['min'] + raise_action[0]['amount']['max'])
                action['amount'] = (1-win_rate)/(1-raise_threshold)*raise_action[0]['amount']['min'] + (1-(1-win_rate)/(1-raise_threshold))*raise_action[0]['amount']['max']
                print(action['amount'])
                # action['amount'] = 15 # This works

            else:
                action = valid_actions[1]  # default to call if raise is not available
                
        print(f"WhalePlayer: action['action'] = {action['action']}, action['amount'] = {action['amount']}, win_rate ={win_rate}, std = {wr_std}")


        return action['action'], action['amount']

    def receive_game_start_message(self, game_info):
        self.nb_player = game_info['player_num']

    def receive_round_start_message(self, round_count, hole_card, seats):
        pass

    def receive_street_start_message(self, street, round_state):
        pass

    def receive_game_update_message(self, action, round_state):
        pass

    def receive_round_result_message(self, winners, hand_info, round_state):
        pass

    




config = setup_config(max_round=10, initial_stack=100, small_blind_amount=5)
config.register_player(name="f1", algorithm=FishPlayer())
config.register_player(name="f2", algorithm=FishPlayer())
config.register_player(name="h1", algorithm=HonestPlayer())
config.register_player(name="h2", algorithm=HonestPlayer())
config.register_player(name="smin", algorithm=SharkPlayerMin())
config.register_player(name="smax", algorithm=SharkPlayerMax())
config.register_player(name="slin", algorithm=SharkPlayerLinear())
config.register_player(name="w1", algorithm=WhalePlayer())
# config.register_player(name="w1", algorithm=WhalePlayer())


game_result = start_poker(config, verbose=0)
for player_info in game_result["players"]:
    print(player_info)
