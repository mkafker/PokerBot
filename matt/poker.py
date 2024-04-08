from pypokerengine.players import BasePokerPlayer
from pypokerengine.api.game import setup_config, start_poker
from pypokerengine.utils.card_utils import gen_cards, estimate_hole_card_win_rate

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
            print(valid_actions)
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

def estimate_hand_strength(nb_simulation, nb_player, hole_card, community_card):
    simulation_results = []
    for i in range(nb_simulation):
        opponents_cards = []
        for j in range(nb_player-1):  # nb_opponents = nb_player - 1
            opponents_cards.append(draw_cards_from_deck(num=2))
        nb_need_community = 5 - len(community_card)
        community_card.append(draw_cards_from_deck(num=nb_need_community))
        result = observe_game_result(hole_card, community_card, opponents_cards)  # return 1 if win else 0
        simulation_results.append(result)
    average_win_rate = 1.0 * sum(simulation_results) / len(simulation_results)
    return average_win_rate

#config = setup_config(max_round=10, initial_stack=100, small_blind_amount=5)
#config.register_player(name="p1", algorithm=FishPlayer())
#config.register_player(name="p2", algorithm=FishPlayer())
#config.register_player(name="p3", algorithm=FishPlayer())
#game_result = start_poker(config, verbose=1)

#from pypokerengine.utils.card_utils import gen_cards, estimate_hole_card_win_rate
#hole_card = gen_cards(['H5', 'D7'])
#community_card = gen_cards(['D3', 'C5', 'C6'])
#print(estimate_hole_card_win_rate(nb_simulation=1000, nb_player=12, hole_card=hole_card, community_card=community_card))

#nb_simulation = 1000
#nb_player = 3
#hole_card = ['H4', 'D7']
#community_card = ['D3', 'C5', 'C6']

from pypokerengine.api.game import setup_config, start_poker
config = setup_config(max_round=1, initial_stack=100, small_blind_amount=5)
config.register_player(name="fish_player", algorithm=FishPlayer())
config.register_player(name="honest_player", algorithm=HonestPlayer())
config.register_player(name="honest_player_2", algorithm=HonestPlayer())
game_result = start_poker(config, verbose=1)
#for player_info in game_result["players"]:
#    print(player_info)
