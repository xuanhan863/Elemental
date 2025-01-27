import El

# NOTE: Going above polynomials of order 50 leads to *very* large coefficients
#       alpha^n for n > 50, even if alpha is close to 1.
#
#       A different scheme is likely warranted than simply looking for 
#       Z-linear dependence of { alpha^j }_{j=0}^{n-1}
#
alpha=El.ComplexDouble(1.002,1.003)
n=50
tag=El.zTag
progress=False
timeLLL=False
outputAll=False
outputCoeff=False

print "alpha=", alpha

ctrl=El.LLLCtrl_d()
ctrl.progress = progress
ctrl.time = timeLLL

NSqrt=100000.

# NOTE: The coefficients become orders of magnitude higher for 'weak'
#       reductions and seem to reliably lead to an exception being thrown,
#       so these tests will only use strong LLL reductions

B=El.Matrix()
for presort, smallestFirst in (True,True), (True,False), (False,False):
  for deltaLower in 0.5, 0.75, 0.95, 0.98, 0.99:
    for weak in False,:

      print "weak=%r, presort=%r, smallestFirst=%r, deltaLower=%f" % \
        (weak,presort,smallestFirst,deltaLower)

      ctrl.delta = deltaLower
      ctrl.weak = weak 
      ctrl.presort = presort
      ctrl.smallestFirst = smallestFirst

      # Search for the linear dependence
      startTime = El.mpi.Time()
      numExact, B, U = El.AlgebraicRelationSearch(alpha,n,NSqrt,ctrl)
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
