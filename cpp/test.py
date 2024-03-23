import poker
import scipy
import numpy

def optfunc(inplist):
  return 100.0-poker.MCGames(100000, [int(x) for x in inplist])
def printstuff(x, f, context):
  print("x="+str(x))
  print("f="+str(f))

#ret = scipy.optimize.dual_annealing(optfunc,  bounds=list(zip([0]*9, [9]*9)), x0 =handrankToBetList, initial_temp=1, no_local_search=True, callback=printstuff)
rranges = [(0,10)]*(9*13) # 9 ranks, 13 cards
print(rranges)
ret = scipy.optimize.differential_evolution(optfunc, bounds=rranges, mutation=0.1, workers=1, integrality=True, disp=True)
print(ret.fun)
print(ret.x)