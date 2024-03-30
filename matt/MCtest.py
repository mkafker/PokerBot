import pypokerengine.utils.card_utils as card_utils

print(dir(card_utils))

# Example hole cards and community cards
hole_card_str = ['SA', 'DK']  # Ace of Spades, King of Diamonds
community_card_str = ['C2', 'C3', 'C4']  # 2, 3, 4 of Clubs

# Convert card strings to Card objects
hole_cards = card_utils.gen_cards(hole_card_str)
community_cards = card_utils.gen_cards(community_card_str)

# Number of players and simulations
nb_player = 3  # Including the player
nb_simulation = 1  # For simplicity, just one simulation

# Call the _montecarlo_simulation function
win = card_utils._montecarlo_simulation(nb_player, hole_cards, community_cards)

# Print the result
print("Win:", win)

