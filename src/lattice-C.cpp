/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"
#include "El.h"
using namespace El;

extern "C" {

ElError ElLLLCtrlDefault_s( ElLLLCtrl_s* ctrl )
{
    float eps = limits::Epsilon<float>();

    ctrl->delta = 0.75f;
    ctrl->eta = 0.5f + Pow(eps,0.9f);
    ctrl->weak = false;
    ctrl->deep = false;
    ctrl->presort = true;
    ctrl->smallestFirst = true;
    ctrl->reorthogTol = 0;
    ctrl->numOrthog = 1;
    ctrl->zeroTol = Pow(eps,0.9f);
    ctrl->progress = false;
    ctrl->time = false;
    return EL_SUCCESS;
}

ElError ElLLLCtrlDefault_d( ElLLLCtrl_d* ctrl )
{
    double eps = limits::Epsilon<double>();

    ctrl->delta = 0.75;
    ctrl->eta = 0.5 + Pow(eps,0.5);
    ctrl->weak = false;
    ctrl->deep = false;
    ctrl->presort = true;
    ctrl->smallestFirst = true;
    ctrl->reorthogTol = 0;
    ctrl->numOrthog = 1;
    ctrl->zeroTol = Pow(eps,0.9f);
    ctrl->progress = false;
    ctrl->time = false;
    return EL_SUCCESS;
}

#define C_PROTO(SIG,SIGBASE,F) \
  ElError ElLLL_ ## SIG \
  ( ElMatrix_ ## SIG B, \
    ElLLLCtrl_ ## SIGBASE ctrl, \
    ElLLLInfo_ ## SIGBASE * infoC ) \
  { EL_TRY( \
      auto info = LLL( *CReflect(B), CReflect(ctrl) );\
      *infoC = CReflect(info); \
    ) } \
  ElError ElLLLFormR_ ## SIG \
  ( ElMatrix_ ## SIG B, \
    ElMatrix_ ## SIG R, \
    ElLLLCtrl_ ## SIGBASE ctrl, \
    ElLLLInfo_ ## SIGBASE * infoC ) \
  { EL_TRY( \
      auto info = LLL( *CReflect(B), *CReflect(R), CReflect(ctrl) );\
      *infoC = CReflect(info); \
    ) } \
  ElError ElLLLFull_ ## SIG \
  ( ElMatrix_ ## SIG B, \
    ElMatrix_ ## SIG U, \
    ElMatrix_ ## SIG UInv, \
    ElMatrix_ ## SIG R, \
    ElLLLCtrl_ ## SIGBASE ctrl, \
    ElLLLInfo_ ## SIGBASE * infoC ) \
  { EL_TRY( \
      auto info = \
        LLL( *CReflect(B), *CReflect(U), *CReflect(UInv), *CReflect(R), \
             CReflect(ctrl) ); \
      *infoC = CReflect(info); \
    ) } \
  ElError ElLatticeImageAndKernel_ ## SIG \
  ( ElMatrix_ ## SIG B, \
    ElMatrix_ ## SIG M, \
    ElMatrix_ ## SIG K, \
    ElLLLCtrl_ ## SIGBASE ctrl ) \
  { EL_TRY( LatticeImageAndKernel( \
      *CReflect(B), *CReflect(M), *CReflect(K), CReflect(ctrl) ) ) } \
  ElError ElLatticeKernel_ ## SIG \
  ( ElMatrix_ ## SIG B, \
    ElMatrix_ ## SIG K, \
    ElLLLCtrl_ ## SIGBASE ctrl ) \
  { EL_TRY( LatticeKernel( *CReflect(B), *CReflect(K), CReflect(ctrl) ) ) } \
  ElError ElZDependenceSearch_ ## SIG \
  ( ElConstMatrix_ ## SIG z, \
    Base<F> NSqrt, \
    ElMatrix_ ## SIG B, \
    ElMatrix_ ## SIG U, \
    ElLLLCtrl_ ## SIGBASE ctrl, \
    ElInt* numFound ) \
  { EL_TRY( *numFound = \
      ZDependenceSearch \
      ( *CReflect(z), NSqrt, *CReflect(B), *CReflect(U), CReflect(ctrl) ) ) } \
  ElError ElAlgebraicRelationSearch_ ## SIG \
  ( CREFLECT(F) alpha, \
    ElInt n, \
    Base<F> NSqrt, \
    ElMatrix_ ## SIG B, \
    ElMatrix_ ## SIG U, \
    ElLLLCtrl_ ## SIGBASE ctrl, \
    ElInt* numFound ) \
  { EL_TRY( *numFound = \
      AlgebraicRelationSearch \
      ( CReflect(alpha), n, NSqrt, \
        *CReflect(B), *CReflect(U), CReflect(ctrl) ) ) }

#define EL_NO_INT_PROTO
#include "El/macros/CInstantiate.h"

} // extern "C"
