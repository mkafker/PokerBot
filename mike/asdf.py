import poker
import time
import random


cardsA=[(4,4),(11,2)]
cardsB=[(3,4),(11,4)]
communityCards=[(2,2),(3,4),(7,3),(10,3)]


N = 100000
start_time = time.time()
poker.MCSingleHand(cardsA, communityCards, 1, N)
end_time = time.time()
el = end_time - start_time
print(N/el)
