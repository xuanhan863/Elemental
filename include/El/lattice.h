/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_LATTICE_C_H
#define EL_LATTICE_C_H

#ifdef __cplusplus
extern "C" {
#endif

/* Lenstra-Lenstra-Lovasz lattice reduction
   ======================================== */

typedef struct
{
    float delta;
    float eta;
    ElInt rank;
    ElInt nullity;    
    ElInt numSwaps;
    float logVol;
} ElLLLInfo_s;

typedef struct
{
    double delta;
    double eta;
    ElInt rank;
    ElInt nullity;    
    ElInt numSwaps;
    double logVol;
} ElLLLInfo_d;

typedef struct
{
    float delta;
    float eta;
    bool weak;
    bool deep;
    bool presort;
    bool smallestFirst;
    float reorthogTol;
    ElInt numOrthog;
    float zeroTol;
    bool progress;
    bool time;
} ElLLLCtrl_s;
EL_EXPORT ElError ElLLLCtrlDefault_s( ElLLLCtrl_s* ctrl );

typedef struct
{
    double delta;
    double eta;
    bool weak;
    bool deep;
    bool presort;
    bool smallestFirst;
    double reorthogTol;
    ElInt numOrthog;
    double zeroTol;
    bool progress;
    bool time;
} ElLLLCtrl_d;
EL_EXPORT ElError ElLLLCtrlDefault_d( ElLLLCtrl_d* ctrl );

EL_EXPORT ElError ElLLL_s( ElMatrix_s B, ElLLLCtrl_s ctrl, ElLLLInfo_s* info );
EL_EXPORT ElError ElLLL_d( ElMatrix_d B, ElLLLCtrl_d ctrl, ElLLLInfo_d* info );
EL_EXPORT ElError ElLLL_c( ElMatrix_c B, ElLLLCtrl_s ctrl, ElLLLInfo_s* info );
EL_EXPORT ElError ElLLL_z( ElMatrix_z B, ElLLLCtrl_d ctrl, ElLLLInfo_d* info );

EL_EXPORT ElError ElLLLFormR_s
( ElMatrix_s B, ElMatrix_s R, ElLLLCtrl_s ctrl, ElLLLInfo_s* info );
EL_EXPORT ElError ElLLLFormR_d
( ElMatrix_d B, ElMatrix_d R, ElLLLCtrl_d ctrl, ElLLLInfo_d* info );
EL_EXPORT ElError ElLLLFormR_c
( ElMatrix_c B, ElMatrix_c R, ElLLLCtrl_s ctrl, ElLLLInfo_s* info );
EL_EXPORT ElError ElLLLFormR_z
( ElMatrix_z B, ElMatrix_z R, ElLLLCtrl_d ctrl, ElLLLInfo_d* info );

EL_EXPORT ElError ElLLLFull_s
( ElMatrix_s B, ElMatrix_s U, ElMatrix_s UInv, ElMatrix_s R,
  ElLLLCtrl_s ctrl, ElLLLInfo_s* info );
EL_EXPORT ElError ElLLLFull_d
( ElMatrix_d B, ElMatrix_d U, ElMatrix_d UInv, ElMatrix_d R,
  ElLLLCtrl_d ctrl, ElLLLInfo_d* info );
EL_EXPORT ElError ElLLLFull_c
( ElMatrix_c B, ElMatrix_c U, ElMatrix_c UInv, ElMatrix_c R,
  ElLLLCtrl_s ctrl, ElLLLInfo_s* info );
EL_EXPORT ElError ElLLLFull_z
( ElMatrix_z B, ElMatrix_z U, ElMatrix_z UInv, ElMatrix_z R,
  ElLLLCtrl_d ctrl, ElLLLInfo_d* info );

/* Lattice image and kernel
   ======================== */
EL_EXPORT ElError ElLatticeImageAndKernel_s
( ElMatrix_s B, ElMatrix_s M, ElMatrix_s K, ElLLLCtrl_s ctrl );
EL_EXPORT ElError ElLatticeImageAndKernel_d
( ElMatrix_d B, ElMatrix_d M, ElMatrix_d K, ElLLLCtrl_d ctrl );
EL_EXPORT ElError ElLatticeImageAndKernel_c
( ElMatrix_c B, ElMatrix_c M, ElMatrix_c K, ElLLLCtrl_s ctrl );
EL_EXPORT ElError ElLatticeImageAndKernel_z
( ElMatrix_z B, ElMatrix_z M, ElMatrix_z K, ElLLLCtrl_d ctrl );

EL_EXPORT ElError ElLatticeKernel_s
( ElMatrix_s B, ElMatrix_s K, ElLLLCtrl_s ctrl );
EL_EXPORT ElError ElLatticeKernel_d
( ElMatrix_d B, ElMatrix_d K, ElLLLCtrl_d ctrl );
EL_EXPORT ElError ElLatticeKernel_c
( ElMatrix_c B, ElMatrix_c K, ElLLLCtrl_s ctrl );
EL_EXPORT ElError ElLatticeKernel_z
( ElMatrix_z B, ElMatrix_z K, ElLLLCtrl_d ctrl );

/* Search for Z-dependence
   ======================= */
EL_EXPORT ElError ElZDependenceSearch_s
( ElConstMatrix_s z, float NSqrt,
  ElMatrix_s B, ElMatrix_s U, ElLLLCtrl_s ctrl,
  ElInt* numExact );
EL_EXPORT ElError ElZDependenceSearch_d
( ElConstMatrix_d z, double NSqrt,
  ElMatrix_d B, ElMatrix_d U, ElLLLCtrl_d ctrl,
  ElInt* numExact );
EL_EXPORT ElError ElZDependenceSearch_c
( ElConstMatrix_c z, float NSqrt,
  ElMatrix_c B, ElMatrix_c U, ElLLLCtrl_s ctrl,
  ElInt* numExact );
EL_EXPORT ElError ElZDependenceSearch_z
( ElConstMatrix_z z, double NSqrt,
  ElMatrix_z B, ElMatrix_z U, ElLLLCtrl_d ctrl,
  ElInt* numExact );

/* Search for an algebraic relation
   ================================ */
EL_EXPORT ElError ElAlgebraicRelationSearch_s
( float alpha, ElInt n, float NSqrt,
  ElMatrix_s B, ElMatrix_s U, ElLLLCtrl_s ctrl,
  ElInt* numExact );
EL_EXPORT ElError ElAlgebraicRelationSearch_d
( double alpha, ElInt n, double NSqrt,
  ElMatrix_d B, ElMatrix_d U, ElLLLCtrl_d ctrl,
  ElInt* numExact );
EL_EXPORT ElError ElAlgebraicRelationSearch_c
( complex_float alpha, ElInt n, float NSqrt,
  ElMatrix_c B, ElMatrix_c U, ElLLLCtrl_s ctrl,
  ElInt* numExact );
EL_EXPORT ElError ElAlgebraicRelationSearch_z
( complex_double alpha, ElInt n, double NSqrt,
  ElMatrix_z B, ElMatrix_z U, ElLLLCtrl_d ctrl,
  ElInt* numExact );

#ifdef __cplusplus
}
#endif

#endif // ifndef EL_LATTICE_C_H
