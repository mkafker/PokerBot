import poker
import scipy
import numpy

def optfunc(inplist = [1,2,3,4,5,6,7,8,9]):
  return poker.MCGames(10000, inplist)
def printstuff(x, f, context):
  print("x="+str(x))
  print("f="+str(f))

#ret = scipy.optimize.dual_annealing(optfunc,  bounds=list(zip([0]*9, [9]*9)), x0 =handrankToBetList, initial_temp=1, no_local_search=True, callback=printstuff)
rranges = [(1,10)]*(4*2)
print(rranges)
ret = scipy.optimize.differential_evolution(optfunc, bounds=rranges, mutation=0.1, workers=6, integrality=True, disp=True)
print(ret.fun)
print(ret.x)