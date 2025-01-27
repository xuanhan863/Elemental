/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_LATTICE_HPP
#define EL_LATTICE_HPP

namespace El {

// Lenstra-Lenstra-Lovasz (LLL) lattice reduction
// ==============================================
// A reduced basis, say D, is an LLL(delta) reduction of an m x n matrix B if
//
//    B U = D = Q R,
//
// where U is unimodular (integer-valued with absolute determinant of 1)
// and Q R is a floating-point QR factorization of D that satisfies the three
//  properties:
//
//   1. R has non-negative diagonal
//
//   2. R is (eta) size-reduced:
//
//        | R(i,j) / R(i,i) | < phi(F) eta,  for all i < j, and
//
//      where phi(F) is 1 for a real field F or sqrt(2) for a complex
//      field F, and
//
//   3. R is (delta) Lovasz reduced:
//
//        delta R(i,i)^2 <= R(i+1,i+1)^2 + |R(i,i+1)|^2,  for all i.
//
// Please see
//
//   Henri Cohen, "A course in computational algebraic number theory"
// 
// for more information on the "MLLL" variant of LLL used by Elemental to 
// handle linearly dependent vectors (the algorithm was originally suggested by
// Mike Pohst).
//

template<typename Real>
struct LLLInfo
{
    Real delta;
    Real eta; 
    Int rank;
    Int nullity; 
    Int numSwaps;
    Real logVol;
};

// Return the Gaussian estimate of the minimum-length vector
// 
//   GH(L) = (1/sqrt(pi)) Gamma(n/2+1)^{1/n} |det(L)|^{1/n}.
//
// where n is the rank of the lattice L.
template<typename Real,typename=EnableIf<IsReal<Real>>>
Real LatticeGaussianHeuristic( Int n, Real logVol )
{
    return Exp((LogGamma(Real(n)/Real(2)+Real(1))+logVol)/Real(n))/
           Sqrt(Pi<Real>());
}

template<typename Real>
struct LLLCtrl
{
    Real delta=Real(3)/Real(4);
    Real eta=Real(1)/Real(2) + Pow(limits::Epsilon<Real>(),Real(0.9));

    // A 'weak' LLL reduction only ensures that | R(i,i+1) / R(i,i) | is
    // bounded above by eta (or, for complex data, by sqrt(2) eta)
    bool weak=false;

    // LLL with deep insertion requires more work but tends to produce shorter
    // vectors
    bool deep=false;

    // Preprocessing with a "rank-obscuring" column-pivoted QR factorization
    // (in the manner suggested by Wubben et al.) tends to greatly decrease
    // the number of swaps within LLL
    bool presort=true;
    bool smallestFirst=true;

    // If the size-reduced column has a two-norm that is less than or
    // equal to `reorthogTol` times the  original two-norm, then reorthog.
    Real reorthogTol=0;

    // The number of times to execute the orthogonalization
    Int numOrthog=1;

    // If a size-reduced column has a two-norm less than or equal to 'zeroTol',
    // then it is interpreted as a zero vector (and forced to zero)
    Real zeroTol=Pow(limits::Epsilon<Real>(),Real(0.9));

    bool progress=false;
    bool time=false;
};

// TODO: Maintain B in BigInt form

template<typename F>
LLLInfo<Base<F>> LLL
( Matrix<F>& B,
  const LLLCtrl<Base<F>>& ctrl=LLLCtrl<Base<F>>() );

// TODO: Also return Q?
template<typename F>
LLLInfo<Base<F>> LLL
( Matrix<F>& B,
  Matrix<F>& R,
  const LLLCtrl<Base<F>>& ctrl=LLLCtrl<Base<F>>() );

// TODO: Also return Q?
template<typename F>
LLLInfo<Base<F>> LLL
( Matrix<F>& B,
  Matrix<F>& U,
  Matrix<F>& UInv,
  Matrix<F>& R,
  const LLLCtrl<Base<F>>& ctrl=LLLCtrl<Base<F>>() );

// Perform a tree reduction of subsets of the original basis in order to 
// expose parallelism and perform as much work as possible in double-precision
// (which is often possible even for the SVP Challenge).
// This will not be substantially faster than the above LLL until Elemental
// supports different MPFR precisions simultaneously
template<typename F>
LLLInfo<Base<F>> RecursiveLLL
( Matrix<F>& B,
  Int cutoff=10,
  const LLLCtrl<Base<F>>& ctrl=LLLCtrl<Base<F>>() );

// Overwrite B, fill M with its (quasi-reduced) image of B, and fill K with the
// LLL-reduced basis for the kernel of B.
//
// This is essentially Algorithm 2.7.1 from Cohen's
// "A course in computational algebraic number theory". The main difference
// is that we avoid solving the normal equations and call a least squares
// solver.
// 
template<typename F>
void LatticeImageAndKernel
( Matrix<F>& B,
  Matrix<F>& M,
  Matrix<F>& K,
  const LLLCtrl<Base<F>>& ctrl=LLLCtrl<Base<F>>() );

// Overwrite B and fill K with the LLL-reduced basis for the kernel of B.
// This will eventually mirror Algorithm 2.7.2 from Cohen's
// "A course in computational algebraic number theory".
template<typename F>
void LatticeKernel
( Matrix<F>& B,
  Matrix<F>& K,
  const LLLCtrl<Base<F>>& ctrl=LLLCtrl<Base<F>>() );

// Search for Z-dependence
// =======================
// Search for Z-dependence of a vector of real or complex numbers, z, via
// the quadratic form
//
//   Q(a) = || a ||_2^2 + N | z^T a |^2,
//
// which is generated by the basis matrix
//
//   
//   B = [I; sqrt(N) z^T],
//
// as Q(a) = a^T B^T B a = || B a ||_2^2. Cohen has advice for the choice of
// the (large) parameter N within subsection 2.7.2 within his book. 
//
// The number of (nearly) exact Z-dependences detected is returned.
//
template<typename F>
Int ZDependenceSearch
( const Matrix<F>& z,
        Base<F> NSqrt,
        Matrix<F>& B,
        Matrix<F>& U, 
  const LLLCtrl<Base<F>>& ctrl=LLLCtrl<Base<F>>() );

// Search for an algebraic relation
// ================================
// Search for the (Gaussian) integer coefficients of a polynomial of alpha
// that is (nearly) zero.
template<typename F>
Int AlgebraicRelationSearch
( F alpha,
  Int n,
  Base<F> NSqrt,
  Matrix<F>& B,
  Matrix<F>& U, 
  const LLLCtrl<Base<F>>& ctrl=LLLCtrl<Base<F>>() );

} // namespace El

#endif // ifndef EL_LATTICE_HPP
