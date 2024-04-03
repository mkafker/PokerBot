import poker
import time

cardsA = [(14,1), (14,2)]
cardsB=[(12,4),(13,2)]
communityCards=[(4,4),(5,2),(6,2),(2,2),(8,4)]

N = 100000
start_time = time.time()
for i in range(N):
    poker.showdownHands(cardsA, cardsB, communityCards)

end_time = time.time()
el = end_time - start_time
print(N/el)
