from pypokerengine.players import BasePokerPlayer
from pypokerengine.utils.card_utils import gen_cards, estimate_hole_card_win_rate
import numpy as np

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

class HonestPlayer(BasePokerPlayer):

    def declare_action(self, valid_actions, hole_card, round_state):
        community_card = round_state['community_card']
        win_rate = estimate_hole_card_win_rate(
                nb_simulation=1000,
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

##########################################################################################################

class FishPlayer_modified(BasePokerPlayer):  # Do not forget to make parent class as "BasePokerPlayer"

    #  we define the logic to make an action through this method. (so this method would be the core of your AI)
    def declare_action(self, valid_actions, hole_card, round_state):
        # valid_actions format => [raise_action_info, call_action_info, fold_action_info]
        if round_state['street'] == 'preflop':
            call_action_info = valid_actions[1]
            action, amount = call_action_info["action"], call_action_info["amount"]
        else:
            action, amount = valid_actions[2]['action'], valid_actions[2]['amount']['min']
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

class HonestPlayer_modified(BasePokerPlayer):

    def declare_action(self, valid_actions, hole_card, round_state):
        # print(round_state)
        # print(valid_actions)
        # print(hole_card)
        # print(self.invested_this_street(round_state))
        community_card = round_state['community_card']
        win_rate = estimate_hole_card_win_rate(
                nb_simulation=1000,
                nb_player=self.nb_player,
                hole_card=gen_cards(hole_card),
                community_card=gen_cards(community_card)
                )
        
        if valid_actions[1]['amount'] > self.invested_this_street(round_state):
            if win_rate >= 1.0 / self.nb_player:
                action = valid_actions[1] # fetch CALL action info
            else:
                action = valid_actions[0] # fetch FOLD action info
        if valid_actions[1]['amount'] == self.invested_this_street(round_state):
            if win_rate >= 1.0 / self.nb_player:
                action = valid_actions[2] # fetch RAISE action info
                action['amount'] = action['amount']['min']
            else:
                action = valid_actions[1] # fetch CALL action info
        else:
            action = valid_actions[1] # fetch CALL action info
        return action['action'], action['amount']

    def invested_this_street(self, round_state):
        """
        Amount that the player has added to the pot so far this street.
        """
        total = 0
        for action in round_state['action_histories'][round_state['street']]:
            if action['uuid'] == self.uuid:
                if action['action'] in ['SMALLBLIND', 'BIGBLIND']:
                    total += action['amount']
                else:
                    total += action['paid']
        return total

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

from pypokerengine.api.game import setup_config, start_poker

config = setup_config(max_round=10, initial_stack=1000, small_blind_amount=20)
config.register_player(name="p1", algorithm=FishPlayer_modified())
config.register_player(name="p2", algorithm=HonestPlayer_modified())
game_result = start_poker(config, verbose=1)
for player_info in game_result["players"]:
    print(player_info)