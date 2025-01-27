/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {

template<typename T> 
void Hankel( Matrix<T>& A, Int m, Int n, const vector<T>& a )
{
    DEBUG_ONLY(CSE cse("Hankel"))
    const Int length = m+n-1;
    if( a.size() != (Unsigned)length )
        LogicError("a was the wrong size");
    A.Resize( m, n );
    auto hankelFill = [&]( Int i, Int j ) { return a[i+j]; };
    IndexDependentFill( A, function<T(Int,Int)>(hankelFill) );
}

template<typename T>
void Hankel( AbstractDistMatrix<T>& A, Int m, Int n, const vector<T>& a )
{
    DEBUG_ONLY(CSE cse("Hankel"))
    const Int length = m+n-1;
    if( a.size() != (Unsigned)length )
        LogicError("a was the wrong size");
    A.Resize( m, n );
    auto hankelFill = [&]( Int i, Int j ) { return a[i+j]; };
    IndexDependentFill( A, function<T(Int,Int)>(hankelFill) );
}

#define PROTO(T) \
  template void Hankel \
  ( Matrix<T>& A, Int m, Int n, const vector<T>& a ); \
  template void Hankel \
  ( AbstractDistMatrix<T>& A, Int m, Int n, const vector<T>& a );

#define EL_ENABLE_QUAD
#define EL_ENABLE_BIGFLOAT
#include "El/macros/Instantiate.h"

} // namespace El
