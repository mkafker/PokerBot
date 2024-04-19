from pypokerengine.players import BasePokerPlayer
from pypokerengine.utils.card_utils import gen_cards, estimate_hole_card_win_rate
from pypokerengine.api.game import setup_config, start_poker
import numpy as np

NB_SIMULATION = 1000

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
        # if win_rate >= 0.5:
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
        # fold_threshold = 1.0 / self.nb_player
        fold_threshold = 0.35
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
                min_raise = raise_action[0]['amount']['min']
                max_raise = raise_action[0]['amount']['max']

                # Happens when we haven't busted, but don't have enough to raise a big blind
                if min_raise < 0 or max_raise < 0:
                    print("Not enough money to raise, choosing to either fold or call")
                    if len(valid_actions) == 3: # Can Still just Call
                        action = valid_actions[1] # Call
                    else:
                        action = valid_actions[0] # Fold
                else:
                    action['amount'] = action['amount']['min']

            else:
                action = valid_actions[1]  # default to call if raise is not available

            # Appears to work
            # action = valid_actions[2] 
            # action['amount'] = 15

        print(f"SharkMinPlayer: action['action'] = {action['action']}, action['amount'] = {action['amount']}")

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
        # fold_threshold = 1.0 / self.nb_player
        fold_threshold = 0.35
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
        # fold_threshold = 1.0 / self.nb_player
        fold_threshold = 0.35
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
        #print(win_rate, wr_std)
        
        
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
            #print(raise_action)
            if raise_action:
                # action = raise_action[0]
                
                # action['amount'] = raise_action[0]['amount']['min']
                # #print(action['amount'])
                action = raise_action[0]
                min_raise = raise_action[0]['amount']['min']
                max_raise = raise_action[0]['amount']['max']

                # Happens when we haven't busted, but don't have enough to raise a big blind
                if min_raise < 0 or max_raise < 0:
                    print("Not enough money to raise, choosing to either fold or call")
                    if len(valid_actions) == 3: # Can Still just Call
                        action = valid_actions[1] # Call
                    else:
                        action = valid_actions[0] # Fold
                else:
                    action['amount'] = action['amount']['min']

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
                #print(action['amount'])
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

from better_showdown import showdown_poker
import json
import random
random.seed(101)
import my_bot
STARTING_STACK = 100
class CFRPlayer(BasePokerPlayer):
    def __init__(self):
        self.infoset_key = ";;"
        self.current_street = None
        with open("trained_model.json", "r") as f:
            self.i_map = json.load(f)

    def determine_pips(self, round_state):
        street_actions = round_state["action_histories"][round_state["street"]]
        if len(street_actions) == 0:
            return 0, 0
        my_actions = [x['amount'] for x in street_actions if x['uuid'] == self.uuid]
        opp_actions = [x['amount'] for x in street_actions if x['uuid'] != self.uuid]
        if len(my_actions) < 1:
            my_actions = [0]
        if len(opp_actions) < 1:
            opp_actions = [0]
        return max(my_actions), max(opp_actions)

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
        # print(self.infoset_key)
        hole_cards = card_utils.gen_cards(hole_card)
        community_cards = card_utils.gen_cards(community_card)
        #print(self.uuid)
        print(round_state)
        if round_state["dealer_btn"] == round_state["next_player"]:
            player_id = "P2"
        else:
            player_id = "P1"

        hand_strength, hand_std = self.estimate_hand_strength(
            NB_SIMULATION,
            self.nb_player,
            hole_cards,
            community_cards
        )
        hand_strength = my_bot.clamp_hand_strength(hand_strength)

        if round_state['street'] == 'preflop':
            if player_id == "P1" and len(round_state['action_histories']['preflop']) == 2:
                print("New Round, resetting infoset_key...")
                self.infoset_key = ";;"
            if player_id == "P2" and len(round_state['action_histories']['preflop']) == 3:
                print("New Round, resetting infoset_key...")
                self.infoset_key = ";;"

        if round_state['street'] != self.current_street:
            self.current_street = round_state['street']
            if round_state['street'] != 'preflop' and self.infoset_key[-1] != "/": # Opp called to end the last street
                self.infoset_key += "c/"

        # "pip" is how much the player has put in to the pot so far this hand
        if round_state['street'] == 'preflop':
            if len(self.infoset_key) == 2 and player_id == "P1":
                self.infoset_key = f'{player_id};{hand_strength};aa/'
            else:
                
                p1_commited, p2_commited = my_bot.player_money_bet(self.infoset_key.split(';')[-1].split('/'))
                my_pip, opp_pip = self.determine_pips(round_state)

                if opp_pip == 0: # opponent has check called
                    opp_move = 'c'
                elif opp_pip == my_pip:
                    opp_move = 'c'
                else: # opponent has raised
                    if 0 < opp_pip < STARTING_STACK / 4:
                        opp_move = 1
                    if STARTING_STACK / 4 < opp_pip < STARTING_STACK / 2:
                        opp_move = 2
                    if STARTING_STACK / 2 < opp_pip < 3 * STARTING_STACK / 4:
                        opp_move = 3
                    if 3 * STARTING_STACK / 4 < opp_pip:
                        opp_move = 4

                    if player_id == "P2":
                        action_history = (self.infoset_key.split(';')[-1] + str(opp_move) + 'r').split('/')
                        if 'aa' not in action_history:
                            action_history = ['aa'] + action_history
                        print(action_history)
                        p1_temp_comit, p2_temp_comit = my_bot.player_money_bet(action_history)
                        if p1_temp_comit > 5:
                            opp_move = "" # This is a fucking stupid bodge
                        else:
                            opp_move = str(opp_move) + 'r'
                    if player_id == "P1":
                        action_history = (self.infoset_key.split(';')[-1] + str(opp_move) + 'r').split('/')
                        if 'aa' not in action_history:
                            action_history = ['aa'] + action_history
                        print(action_history)
                        p1_temp_comit, p2_temp_comit = my_bot.player_money_bet(action_history)
                        if p2_temp_comit > 5:
                            opp_move = "" # This is a fucking stupid bodge
                        else:
                            opp_move = str(opp_move) + 'r'

                    print(opp_move)

                if len(self.infoset_key) == 2:
                    self.infoset_key = f'{player_id};{hand_strength};aa/{opp_move}'
                else:
                    if opp_move:
                        self.infoset_key += opp_move
                    else:
                        if self.infoset_key[-1] == 'c':
                            self.infoset_key = self.infoset_key[0:-1]
                        else:
                            self.infoset_key = self.infoset_key[0:-2]
        else:
            temp_infoset_key = self.infoset_key.split(";")
            actions = my_bot.extract_actions(temp_infoset_key[-1].split('/'))
            if len(actions) == 0 and player_id =="P1":
                temp_infoset_key[1] += f"/{hand_strength}"
            elif len(actions) == 1 and player_id =="P2":
                temp_infoset_key[1] += f"/{hand_strength}"
            self.infoset_key = ";".join(temp_infoset_key)

            if len(actions) == 0 and player_id == "P1":
                pass
            else:
                p1_commited, p2_commited = my_bot.player_money_bet(self.infoset_key.split(';')[-1].split('/'))
                my_pip, opp_pip = self.determine_pips(round_state)
                if opp_pip == 0: # opponent has check called
                    opp_move = 'c'
                elif opp_pip == my_pip:
                    opp_move = 'c'

                else: # opponent has raised
                    if 0 < opp_pip < STARTING_STACK / 4:
                        opp_move = 1
                    if STARTING_STACK / 4 <= opp_pip < STARTING_STACK / 2:
                        opp_move = 2
                    if STARTING_STACK / 2 <= opp_pip < 3 * STARTING_STACK / 4:
                        opp_move = 3
                    if 3 * STARTING_STACK / 4 <= opp_pip:
                        opp_move = 4

                    if player_id == "P2":
                        action_history = (self.infoset_key.split(';')[-1] + str(opp_move) + 'r').split('/')
                        if 'aa' not in action_history:
                            action_history = ['aa'] + action_history
                        print(action_history)
                        p1_temp_comit, p2_temp_comit = my_bot.player_money_bet(action_history)
                        if p1_temp_comit > 5:
                            opp_move = "" # This is a fucking stupid bodge
                        else:
                            opp_move = str(opp_move) + 'r'
                    if player_id == "P1":
                        action_history = (self.infoset_key.split(';')[-1] + str(opp_move) + 'r').split('/')
                        if 'aa' not in action_history:
                            action_history = ['aa'] + action_history
                        print(action_history)
                        p1_temp_comit, p2_temp_comit = my_bot.player_money_bet(action_history)
                        if p2_temp_comit > 5:
                            opp_move = "" # This is a fucking stupid bodge
                        else:
                            opp_move = str(opp_move) + 'r'

                print(opp_move)
                print(opp_move, player_id, p1_commited, p2_commited)
                if opp_move:
                    self.infoset_key += opp_move
                else:
                    if self.infoset_key[-1] == 'c':
                        self.infoset_key = self.infoset_key[0:-1]
                    else:
                        self.infoset_key = self.infoset_key[0:-2]
    
        print(self.infoset_key)

        if "-" in self.infoset_key:
            print("Something has gone terribly wrong")
            print(round_state)
            print(self.infoset_key)
            quit()
        
        max_bet = 5

        try:
            my_valid_actions = my_bot.valid_actions(self.infoset_key, max_bet)
        except:
            # Fuck it, I'm just playing fish when I can't solve stuff
            action = valid_actions[1]
            return action['action'], action['amount']
            return 

        if self.infoset_key in self.i_map:
            action_choice = random.choices(my_valid_actions, weights = self.i_map[self.infoset_key], k = 1)
            action_choice = action_choice[0]
        else:
            # raise ValueError(f"Current action history {self.infoset_key} does not exist in i_map")
            # quit()
            action_choice = random.choices(my_valid_actions, k=1)[0]
        
        self.infoset_key += action_choice
        if action_choice == 'f':
            action = valid_actions[0] # fold
            self.infoset_key += "/"
            return action['action'], action['amount']
        elif action_choice == 'c':
            action = valid_actions[1] # call
            if len(self.infoset_key.split(';')[-1].split('/')[-1]) > 1:
                self.infoset_key += "/"
            return action['action'], action['amount']
        else:
            action = valid_actions[2] # raise
        
        min_raise = action['amount']['min']
        max_raise = action['amount']['max']

        # Happens when we haven't busted, but don't have enough to raise a big blind
        if min_raise < 0 or max_raise < 0:
            print("Not enough money to raise, choosing to call")
            action = valid_actions[1] # Call
            self.infoset_key = self.infoset_key[0:-2]
            self.infoset_key += 'c/'
            return action['action'], action['amount']

        if int(action_choice[0]) == 1:
            raise_value = min_raise
        elif int(action_choice[0]) == 2:
            raise_value = min_raise + int(1 * (max_raise - min_raise) / 3)
        elif int(action_choice[0]) == 3:
            raise_value = min_raise + int(2 * (max_raise - min_raise) / 3)
        elif int(action_choice[0]) == 4:
            raise_value = min_raise + int(3 * (max_raise - min_raise) / 3)
        else:
            print("Can't find raise amount!!!!")
            quit()
        
        action['amount'] = raise_value

        if action['amount'] < 0:
            print("Tried to raise negative money!")
            print(round_state)
            print(action)
            print(min_raise, max_raise)
            print(action['amount'])
            quit()
            
        print(f"CFRPlayer: action['action'] = {action['action']}, action['amount'] = {action['amount']}")

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

results = []
for i in range(20):
    if i % 10 == 0:
        print("##################")
        print(f'Iteration {i}')
        print("##################")
    config = setup_config(max_round=10, initial_stack=STARTING_STACK, small_blind_amount=5)
    # config.register_player(name="f1", algorithm=FishPlayer())
    config.register_player(name="cfr1", algorithm=CFRPlayer())
    

    # config.register_player(name="f2", algorithm=FishPlayer())
    # config.register_player(name="h1", algorithm=HonestPlayer())
    # config.register_player(name="h2", algorithm=HonestPlayer())
    # config.register_player(name="smin", algorithm=SharkPlayerMin())
    config.register_player(name="smax", algorithm=SharkPlayerMax())
    # config.register_player(name="slin", algorithm=SharkPlayerLinear())
    # config.register_player(name="w1", algorithm=WhalePlayer())
    # config.register_player(name="w1", algorithm=WhalePlayer())

    


    game_result = start_poker(config, verbose=0)
    winner_player = None
    winner_stack = -10000
    for player_info in game_result["players"]:
        if player_info['stack'] > winner_stack:
            winner_stack = player_info['stack']
            winner_player = player_info['name']
        print(player_info)
    print(f"WINNER: {winner_player}")

    if winner_player == 'cfr1':
        results.append(1)
    else:
        results.append(0)

print(f'Percentage of games won: {round(sum(results) / len(results) * 100, 4)} %')
    
