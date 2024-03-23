from pypokerengine.players import BasePokerPlayer


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

from pypokerengine.utils.card_utils import gen_cards, estimate_hole_card_win_rate

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

from pypokerengine.api.game import setup_config, start_poker

#config = setup_config(max_round=10, initial_stack=100, small_blind_amount=5)
#config.register_player(name="p1", algorithm=FishPlayer())
#config.register_player(name="p2", algorithm=FishPlayer())
#config.register_player(name="p3", algorithm=HonestPlayer())
#game_result = start_poker(config, verbose=1)

from pypokerengine.api.game import setup_config, start_poker
config = setup_config(max_round=10, initial_stack=100, small_blind_amount=5)
config.register_player(name="fish_player", algorithm=FishPlayer())
config.register_player(name="honest_player", algorithm=HonestPlayer())
game_result = start_poker(config, verbose=1)
for player_info in game_result["players"]:
    print(player_info)
