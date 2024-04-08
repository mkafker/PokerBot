import numpy as np
import matplotlib.pyplot as plt
import random

##Returns the adjusted strategy after an iteration
def getStrategy(regretSum,strategySum):
    actions = 3
    normalizingSum = 0
    strategy = [0,0,0]
    #Normalizingsum is the sum of positive regrets. 
    #This ensures do not 'over-adjust' and converge to equilibrium
    for i in range(0,actions):
        if regretSum[i] > 0:
            strategy[i] = regretSum[i]
        else:
            strategy[i] = 0
        normalizingSum += strategy[i]
    ##This loop normalizes our updated strategy
    for i in range(0,actions):
        if normalizingSum > 0:
            strategy[i] = strategy[i]/normalizingSum
        else:
            #Default to 33%
            strategy[i] = 1.0 / actions
        strategySum[i] += strategy[i]
    return (strategy,strategySum)

#Returns a random action according to the strategy
def getAction(strategy):
    r = random.uniform(0,1)
    if r >= 0 and r < strategy[0]:
        return 0
    elif r >= strategy[0] and r < strategy[0] + strategy[1]:
        return 1
    elif r >= strategy[0] + strategy[1] and r < sum(strategy):
        return 2
    else:
        return 0

def train(iterations,regretSum,oppStrategy):
    strategySum = [0,0,0]
    actions = 3
    for i in range(0,iterations):
        
        actionUtility = [0,0,0]

        ##Retrieve Actions
        t = getStrategy(regretSum,strategySum)

        strategy = t[0]
        strategySum = t[1]
        print("strategy", strategy)
        print("strategySum", strategySum)
        myaction = getAction(strategy)
        
        #Define an arbitrary opponent strategy from which to adjust
        otherAction = getAction(oppStrategy)

        print("myaction", myaction, "otherAction", otherAction)
        
        # [Rock, Paper, Scissors]
        # Opponent Chooses scissors
        if otherAction == actions - 1:
            actionUtility[0] = 1
            actionUtility[1] = -1
        # Opponent Chooses Rock
        elif otherAction == 0:
            actionUtility[actions - 1] = -1
            actionUtility[1] = 1
        # Opponent Chooses Paper
        else:
            actionUtility[0] = -1
            actionUtility[2] = 1
        
        print("regretsum before", regretSum)
        print("actionUtility", actionUtility)
        #Add the regrets from this decision
        for j in range(0, actions):
            regretSum[j] += actionUtility[j] - actionUtility[myaction]
        print("regretsum after", regretSum)
        print("i = ", i)
        print()

        if i > 500:
        	quit()
    return strategySum

def getAverageStrategy(iterations,oppStrategy):
    actions = 3
    strategySum = train(iterations,[0,0,0],oppStrategy)
    print(strategySum)
    avgStrategy = [0,0,0]
    normalizingSum = 0
    for i in range(0,actions):
        normalizingSum += strategySum[i]
    print(normalizingSum)
    for i in range(0,actions):
        if normalizingSum > 0:
            avgStrategy[i] = strategySum[i] / normalizingSum
        else:
            avgStrategy[i] = 1.0 / actions
    return avgStrategy

oppStrat = [.4,.3,.3]
print("Opponent's Strategy",oppStrat)
print("Maximally Exploitative Strat", getAverageStrategy(1000000,oppStrat))

quit()

#Two player training Function
def train2Player(iterations,regretSum1,regretSum2,p2Strat):
    ##Adapt Train Function for two players
    actions = 3
    actionUtility = [0,0,0]
    strategySum1 = [0,0,0]
    strategySum2 = [0,0,0]
    for i in range(0,iterations):
        ##Retrieve Actions
        t1 = getStrategy(regretSum1,strategySum1)
        strategy1 = t1[0]
        strategySum1 = t1[1]
        myaction = getAction(strategy1)
        t2 = getStrategy(regretSum2,p2Strat)
        strategy2 = t2[0]
        strategySum2 = t2[1]
        otherAction = getAction(strategy2)
        
        #Opponent Chooses scissors
        if otherAction == actions - 1:
            #Utility(Rock) = 1
            actionUtility[0] = 1
            #Utility(Paper) = -1
            actionUtility[1] = -1
        #Opponent Chooses Rock
        elif otherAction == 0:
            #Utility(Scissors) = -1
            actionUtility[actions - 1] = -1
            #Utility(Paper) = 1
            actionUtility[1] = 1
        #Opopnent Chooses Paper
        else:
            #Utility(Rock) = -1
            actionUtility[0] = -1
            #Utility(Scissors) = 1
            actionUtility[2] = 1
            
        #Add the regrets from this decision
        for i in range(0,actions):
            regretSum1[i] += actionUtility[i] - actionUtility[myaction]
            regretSum2[i] += -(actionUtility[i] - actionUtility[myaction])
    return (strategySum1, strategySum2)

#Returns a nash equilibrium reached by two opponents through CFRM
def RPStoNash(iterations,oppStrat):
    strats = train2Player(iterations,[0,0,0],[0,0,0],oppStrat)
    s1 = sum(strats[0])
    s2 = sum(strats[1])
    for i in range(3):
        if s1 > 0:
            strats[0][i] = strats[0][i]/s1
        if s2 > 0:
            strats[1][i] = strats[1][i]/s2
    return strats

print(RPStoNash(1000000,[.4,.3,.3]))