/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#ifndef EL_LATTICE_LLL_UNBLOCKED_HPP
#define EL_LATTICE_LLL_UNBLOCKED_HPP

namespace El {
namespace lll {

// Put the k'th column of B into the k'th column of QR and then rotate
// said column with the first k-1 (scaled) Householder reflectors.

template<typename F>
void ExpandQR
( Int k,
  const Matrix<F>& B,
        Matrix<F>& QR,
  const Matrix<F>& t,
  const Matrix<Base<F>>& d,
  Int numOrthog,
  bool time )
{
    DEBUG_ONLY(CSE cse("lll::ExpandQR"))
    typedef Base<F> Real;
    const Int m = B.Height();
    const F* BBuf = B.LockedBuffer();
          F* QRBuf = QR.Buffer();  
    const F* tBuf = t.LockedBuffer();
    const Base<F>* dBuf = d.LockedBuffer();
    const Int BLDim = B.LDim();
    const Int QRLDim = QR.LDim();

    // Copy in the k'th column of B
    for( Int i=0; i<m; ++i )
        QRBuf[i+k*QRLDim] = BBuf[i+k*BLDim];

    if( time )
        applyHouseTimer.Start();
    for( Int orthog=0; orthog<numOrthog; ++orthog )
    {
        for( Int i=0; i<k; ++i )
        {
            // Apply the i'th Householder reflector
    
            // Temporarily replace QR(i,i) with 1
            const Real alpha = RealPart(QRBuf[i+i*QRLDim]);
            QRBuf[i+i*QRLDim] = 1;

            const F innerProd =
              blas::Dot
              ( m-i,
                &QRBuf[i+i*QRLDim], 1,
                &QRBuf[i+k*QRLDim], 1 );
            blas::Axpy
            ( m-i, -tBuf[i]*innerProd,
              &QRBuf[i+i*QRLDim], 1,
              &QRBuf[i+k*QRLDim], 1 );

            // Fix the scaling
            QRBuf[i+k*QRLDim] *= dBuf[i];

            // Restore H(i,i)
            QRBuf[i+i*QRLDim] = alpha; 
        }
    }
    if( time )
        applyHouseTimer.Stop();
}

template<typename F>
void HouseholderStep
( Int k,
  Matrix<F>& QR,
  Matrix<F>& t,
  Matrix<Base<F>>& d,
  bool time )
{
    DEBUG_ONLY(CSE cse("lll::HouseholderStep"))
    typedef Base<F> Real;

    // Perform the next step of Householder reduction
    F* QRBuf = QR.Buffer();
    const Int QRLDim = QR.LDim();
    F& rhokk = QRBuf[k+k*QRLDim]; 
    auto qr21 = QR( IR(k+1,END), IR(k) );
    F tau = LeftReflector( rhokk, qr21 );
    t.Set( k, 0, tau );
    if( RealPart(rhokk) < Real(0) )
    {
        d.Set( k, 0, -1 );
        rhokk *= -1;
    }
    else
        d.Set( k, 0, +1 );
}

template<typename F>
inline Base<F> LogPotential( const Matrix<F>& R )
{
    DEBUG_ONLY(CSE cse("lll::LogPotential"))
    typedef Base<F> Real;
    const Int n = R.Width();

    Real logPotential=0;
    for( Int j=0; j<n; ++j )
        logPotential += 2*(n-j)*Log(Abs(R.Get(j,j)));
    return logPotential;
}

// Return true if the new column is a zero vector
template<typename F>
bool Step
( Int k,
  Matrix<F>& B,
  Matrix<F>& U,
  Matrix<F>& UInv,
  Matrix<F>& QR,
  Matrix<F>& t,
  Matrix<Base<F>>& d,
  bool formU,
  bool formUInv,
  const LLLCtrl<Base<F>>& ctrl )
{
    DEBUG_ONLY(CSE cse("lll::Step"))
    typedef Base<F> Real;
    const Int m = B.Height();
    const Int n = B.Width();
    const Real eps = limits::Epsilon<Real>();

    F* BBuf = B.Buffer();
    F* UBuf = U.Buffer();
    F* UInvBuf = UInv.Buffer();
    F* QRBuf = QR.Buffer();
    const Int BLDim = B.LDim();
    const Int ULDim = U.LDim();
    const Int UInvLDim = UInv.LDim();
    const Int QRLDim = QR.LDim();

    while( true ) 
    {
        lll::ExpandQR( k, B, QR, t, d, ctrl.numOrthog, ctrl.time );

        const Real oldNorm = blas::Nrm2( m, &BBuf[k*BLDim], 1 );
        if( !limits::IsFinite(oldNorm) )
            RuntimeError("Encountered an unbounded norm; increase precision");
        if( oldNorm > Real(1)/eps )
            RuntimeError("Encountered norm greater than 1/eps, where eps=",eps);

        if( oldNorm <= ctrl.zeroTol )
        {
            for( Int i=0; i<m; ++i )
                BBuf[i+k*BLDim] = 0;
            for( Int i=0; i<m; ++i )
                QRBuf[i+k*QRLDim] = 0;
            t.Set( k, 0, Real(1)/Real(2) );
            d.Set( k, 0, Real(1) );
            return true;
        }

        if( ctrl.time )
            roundTimer.Start();
        if( ctrl.weak )
        {
            const Real rho_km1_km1 = RealPart(QRBuf[(k-1)+(k-1)*QRLDim]);
            // We should be able to assume R(k-1,k-1) >= 0
            if( rho_km1_km1 > ctrl.zeroTol )
            {
                F chi = QRBuf[(k-1)+k*QRLDim]/rho_km1_km1;
                if( Abs(RealPart(chi)) > ctrl.eta ||
                    Abs(ImagPart(chi)) > ctrl.eta )
                {
                    chi = Round(chi);
                    blas::Axpy
                    ( k, -chi,
                      &QRBuf[(k-1)*QRLDim], 1,
                      &QRBuf[ k   *QRLDim], 1 );

                    blas::Axpy
                    ( m, -chi,
                      &BBuf[(k-1)*BLDim], 1,
                      &BBuf[ k   *BLDim], 1 );

                    if( formU )
                        blas::Axpy
                        ( n, -chi,
                          &UBuf[(k-1)*ULDim], 1,
                          &UBuf[ k   *ULDim], 1 );
                    if( formUInv )
                        blas::Axpy
                        ( n, chi,
                          &UInvBuf[k  ], UInvLDim,
                          &UInvBuf[k-1], UInvLDim );
                }
            }
        }
        else
        {
            vector<F> xBuf(k);
            for( Int i=k-1; i>=0; --i )
            {
                if( Abs(QRBuf[i+i*QRLDim]) <= ctrl.zeroTol )
                {
                    xBuf[i] = 0;
                    continue;
                }
                F chi = QRBuf[i+k*QRLDim]/QRBuf[i+i*QRLDim];
                if( Abs(RealPart(chi)) > ctrl.eta ||
                    Abs(ImagPart(chi)) > ctrl.eta )
                {
                    chi = Round(chi);
                    blas::Axpy
                    ( i+1, -chi,
                      &QRBuf[i*QRLDim], 1,
                      &QRBuf[k*QRLDim], 1 );
                }
                else
                    chi = 0;
                xBuf[i] = chi;
            }
            blas::Gemv
            ( 'N', m, k,
              F(-1), BBuf,           BLDim,
                     &xBuf[0],       1,
              F(+1), &BBuf[k*BLDim], 1 );
            if( formU )
                blas::Gemv
                ( 'N', n, k,
                  F(-1), UBuf,           ULDim,
                         &xBuf[0],       1,
                  F(+1), &UBuf[k*ULDim], 1 );
            if( formUInv )
                blas::Geru
                ( k, n,
                  F(1), &xBuf[0],    1,
                        &UInvBuf[k], UInvLDim,
                        &UInvBuf[0], UInvLDim );
        }
        const Real newNorm = blas::Nrm2( m, &BBuf[k*BLDim], 1 );
        if( ctrl.time )
            roundTimer.Stop();
        if( !limits::IsFinite(newNorm) )
            RuntimeError("Encountered an unbounded norm; increase precision");
        if( newNorm > Real(1)/eps )
            RuntimeError("Encountered norm greater than 1/eps, where eps=",eps);

        if( newNorm > ctrl.reorthogTol*oldNorm )
        {
            break;
        }
        else if( ctrl.progress )
            Output
            ("  Reorthogonalizing with k=",k,
             " since oldNorm=",oldNorm," and newNorm=",newNorm);
    }

    lll::HouseholderStep( k, QR, t, d, ctrl.time );
    return false;
}

// Consider explicitly returning both Q and R rather than just R (in 'QR')
template<typename F>
LLLInfo<Base<F>> UnblockedAlg
( Matrix<F>& B,
  Matrix<F>& U,
  Matrix<F>& UInv,
  Matrix<F>& QR,
  bool formU,
  bool formUInv,
  const LLLCtrl<Base<F>>& ctrl )
{
    DEBUG_ONLY(CSE cse("lll::UnblockedAlg"))
    typedef Base<F> Real;
    if( ctrl.time )
    {
        applyHouseTimer.Reset();
        roundTimer.Reset();
    }

    const Int m = B.Height();
    const Int n = B.Width();
    const Int minDim = Min(m,n);
    Matrix<F> t;
    Matrix<Real> d;
    Zeros( QR, m, n );
    Zeros( d, minDim, 1 );
    Zeros( t, minDim, 1 );

    // Perform the first step of Householder reduction
    lll::ExpandQR( 0, B, QR, t, d, ctrl.numOrthog, ctrl.time );
    lll::HouseholderStep( 0, QR, t, d, ctrl.time );
    Int nullity = 0;
    {
        auto b0 = B(ALL,IR(0));
        if( FrobeniusNorm(b0) <= ctrl.zeroTol )
        {
            auto QR0 = QR(ALL,IR(0));
            Zero( b0 );
            Zero( QR0 );
            nullity = 1;
        }
    }

    Int k=1, numSwaps=0;
    while( k < n )
    {
        bool zeroVector =
          lll::Step( k, B, U, UInv, QR, t, d, formU, formUInv, ctrl );
        if( zeroVector )
            nullity = k+1;
        else
            nullity = Min(nullity,k);

        const Real rho_km1_km1 = QR.GetRealPart(k-1,k-1);
        const F rho_km1_k = QR.Get(k-1,k);
        const Real rho_k_k = QR.GetRealPart(k,k); 
        
        const Real leftTerm = Sqrt(ctrl.delta)*rho_km1_km1;
        const Real rightTerm = lapack::SafeNorm(rho_k_k,rho_km1_k);
        if( leftTerm <= rightTerm )
        {
            ++k;
        }
        else
        {
            ++numSwaps;
            if( ctrl.progress )
                Output("Dropping from k=",k," to ",Max(k-1,1)," since sqrt(delta)*R(k-1,k-1)=",leftTerm," > ",rightTerm);

            ColSwap( B, k-1, k );

            if( formU )
                ColSwap( U, k-1, k );
            if( formUInv )
                RowSwap( UInv, k-1, k );
            if( k == 1 )
            {
                // We must reinitialize since we keep k=1
                lll::ExpandQR( 0, B, QR, t, d, ctrl.numOrthog, ctrl.time );
                lll::HouseholderStep( 0, QR, t, d, ctrl.time );
                {
                    auto b0 = B(ALL,IR(0));
                    if( FrobeniusNorm(b0) <= ctrl.zeroTol )
                    {
                        auto QR0 = QR(ALL,IR(0));
                        Zero( b0 );
                        Zero( QR0 );
                        nullity = 1;
                    }
                    else
                        nullity = 0;
                }
            }
            else
            {
                k = k-1; 
            }
        }
    }

    if( ctrl.time )
    {
        Output("  Apply Householder time: ",applyHouseTimer.Total());
        Output("  Round time:             ",roundTimer.Total());
    }

    // Force R to be upper-trapezoidal
    MakeTrapezoidal( UPPER, QR );

    std::pair<Real,Real> achieved = lll::Achieved(QR,ctrl);
    Real logVol = lll::LogVolume(QR);

    LLLInfo<Base<F>> info;
    info.delta = achieved.first;
    info.eta = achieved.second;
    info.rank = n-nullity;
    info.nullity = nullity;
    info.numSwaps = numSwaps;
    info.logVol = logVol;

    return info;
}

template<typename F>
void DeepColSwap( Matrix<F>& B, Int i, Int k )
{
    const Int m = B.Height();
    auto bi = B( ALL, IR(i) );
    auto bk = B( ALL, IR(k) );
    auto bkCopy( bk );

    F* BBuf = B.Buffer();
    const Int BLDim = B.LDim();
    for( Int l=k-1; l>=i; --l )
        blas::Copy( m, &BBuf[l*BLDim], 1, &BBuf[(l+1)*BLDim], 1 );

    bi = bkCopy;
}

template<typename F>
void DeepRowSwap( Matrix<F>& B, Int i, Int k )
{
    const Int n = B.Width();
    auto bi = B( IR(i), ALL );
    auto bk = B( IR(k), ALL );
    auto bkCopy( bk );

    F* BBuf = B.Buffer();
    const Int BLDim = B.LDim();
    for( Int l=k-1; l>=i; --l )
        blas::Copy( n, &BBuf[l], BLDim, &BBuf[l+1], BLDim );

    bi = bkCopy;
}

template<typename F>
LLLInfo<Base<F>> UnblockedDeepAlg
( Matrix<F>& B,
  Matrix<F>& U,
  Matrix<F>& UInv,
  Matrix<F>& QR,
  bool formU,
  bool formUInv,
  const LLLCtrl<Base<F>>& ctrl )
{
    DEBUG_ONLY(CSE cse("lll::UnblockedDeepAlg"))
    typedef Base<F> Real;
    if( ctrl.time )
    {
        applyHouseTimer.Reset();
        roundTimer.Reset();
    }

    const Int m = B.Height();
    const Int n = B.Width();
    const Int minDim = Min(m,n);
    Matrix<F> t;
    Matrix<Real> d;
    Zeros( QR, m, n );
    Zeros( d, minDim, 1 );
    Zeros( t, minDim, 1 );

    // TODO: Move into a control structure
    //const bool alwaysRecomputeNorms = false;
    const bool alwaysRecomputeNorms = true;
    const Real updateTol = Sqrt(limits::Epsilon<Real>());

    // Perform the first step of Householder reduction
    lll::ExpandQR( 0, B, QR, t, d, ctrl.numOrthog, ctrl.time );
    lll::HouseholderStep( 0, QR, t, d, ctrl.time );
    Int nullity = 0;
    {
        auto b0 = B(ALL,IR(0));
        if( FrobeniusNorm(b0) <= ctrl.zeroTol )
        {
            auto QR0 = QR(ALL,IR(0));
            Zero( b0 );
            Zero( QR0 );
            nullity = 1;
        }
    }

    Int k=1, numSwaps=0;
    while( k < n )
    {
        bool zeroVector =
          lll::Step( k, B, U, UInv, QR, t, d, formU, formUInv, ctrl );
        if( zeroVector )
            nullity = k+1;
        else
            nullity = Min(nullity,k);

        bool swapped=false;
        // NOTE:
        // There appears to be a mistake in the "New Step 4" initialization of 
        // "c" in 
        //
        //   Schnorr and Euchner, "Lattice Basis Reduction: Improved Practical
        //   Algorithms and Solving Subset Sum Problems", 
        //
        // as "c" should be initialized to || b_k ||^2, not || b'_k ||^2,
        // where || b'_k ||_2 = R(k,k) and || b_k ||_2 = norm(R(1:k,k)),
        // if we count from one.
        Real origNorm = blas::Nrm2( k+1, QR.LockedBuffer(0,k), 1 );
        Real partialNorm = origNorm;
        for( Int i=0; i<k; ++i )
        {
            const Real rho_i_i = QR.GetRealPart(i,i);
            const Real leftTerm = Sqrt(ctrl.delta)*rho_i_i;
            if( leftTerm > partialNorm )
            {
                ++numSwaps;
                if( ctrl.progress )
                    Output("Deep inserting k=",k," into position i=",i," since sqrt(delta)*R(i,i)=",leftTerm," > ",partialNorm);

                DeepColSwap( B, i, k );
                if( formU )
                    DeepColSwap( U, i, k );
                if( formUInv )
                    DeepRowSwap( UInv, i, k );
                if( i == 0 )
                {
                    // We must reinitialize since we keep k=1
                    lll::ExpandQR( 0, B, QR, t, d, ctrl.numOrthog, ctrl.time );
                    lll::HouseholderStep( 0, QR, t, d, ctrl.time );
                    {
                        auto b0 = B(ALL,IR(0));
                        if( FrobeniusNorm(b0) <= ctrl.zeroTol )
                        {
                            auto QR0 = QR(ALL,IR(0));
                            Zero( b0 );
                            Zero( QR0 );
                            nullity = 1;
                        }
                        else
                            nullity = 0;
                    }
                    k=1;
                }
                else
                {
                    k = i;
                }
                swapped = true;
                break;
            }
            else
            {
                // Downdate the partial norm in the same manner as LAWN 176
                Real gamma = Abs(QR.Get(i,k)) / partialNorm;
                gamma = Max( Real(0), (Real(1)-gamma)*(Real(1)+gamma) );
                const Real ratio = partialNorm / origNorm; 
                const Real phi = gamma*(ratio*ratio);
                if( phi <= updateTol || alwaysRecomputeNorms )
                {
                    partialNorm = blas::Nrm2( k-i, QR.LockedBuffer(i+1,k), 1 );
                    origNorm = partialNorm;
                }
                else
                    partialNorm *= Sqrt(gamma);
            }
        }
        if( !swapped )
            ++k;
    }

    if( ctrl.time )
    {
        Output("  Apply Householder time: ",applyHouseTimer.Total());
        Output("  Round time:             ",roundTimer.Total());
    }

    // Force R to be upper-trapezoidal
    MakeTrapezoidal( UPPER, QR );

    std::pair<Real,Real> achieved = lll::Achieved(QR,ctrl);
    Real logVol = lll::LogVolume(QR);

    LLLInfo<Base<F>> info;
    info.delta = achieved.first;
    info.eta = achieved.second;
    info.rank = n-nullity;
    info.nullity = nullity;
    info.numSwaps = numSwaps;
    info.logVol = logVol;

    return info;
}

} // namespace lll
} // namespace El

#endif // ifndef EL_LATTICE_LLL_UNBLOCKED_HPP
