import El

n=1000
tag=El.zTag
progress=False
timeLLL=False
outputAll=False
outputCoeff=False

# Initially draw z out of a uniform ball of radius five about 10+0i
z = El.Matrix(tag)
El.Uniform(z,n,1,10.,5.)

# Compute a (hidden) Gaussian integer vector aHidden
aHidden = El.Matrix(tag)
El.Uniform(aHidden,n-1,1,0.,5.)
El.Round(aHidden)

# Force the last entry of z to be the above integer linear combination of the
# first n-1 entries
zetaLast=El.Dotu(aHidden,z[0:n-1,:])
z.Set(n-1,0,zetaLast)

if outputAll or outputCoeff:
  El.Print( aHidden, "aHidden" )
  El.Print( z, "z" )

ctrl=El.LLLCtrl_d()
ctrl.progress = progress
ctrl.time = timeLLL

NSqrt=1000000.

# NOTE: The coefficients become orders of magnitude higher for 'weak'
#       reductions and seem to reliably lead to an exception being thrown,
#       so these tests will only use strong LLL reductions

B=El.Matrix()
weak = False
ctrl.weak = weak 
for presort, smallestFirst in (True,True), (True,False), (False,False):
  for deltaLower in 0.5, 0.75, 0.95, 0.98, 0.99:
    for deep in False,True:

      print "deep=%r, presort=%r, smallestFirst=%r, deltaLower=%f" % \
        (deep,presort,smallestFirst,deltaLower)

      ctrl.delta = deltaLower
      ctrl.deep = deep
      ctrl.presort = presort
      ctrl.smallestFirst = smallestFirst

      # Search for the linear dependence
      startTime = El.mpi.Time()
      numExact, B, U = El.ZDependenceSearch(z,NSqrt,ctrl)
      runTime = El.mpi.Time() - startTime
      print "  runtime: %f seconds" % runTime
      print "  num \"exact\": ", numExact
      if outputAll:
        El.Print( B, "B" )
        El.Print( U, "U" )
      elif outputCoeff:
        El.Print( U[:,0], "u0" )
      print "approximate zero: %e+%ei" % \
        (B.GetRealPart(n,0)/NSqrt,B.GetImagPart(n,0)/NSqrt)
