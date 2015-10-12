// @HEADER
// ************************************************************************
//
//               Rapid Optimization Library (ROL) Package
//                 Copyright (2014) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//  
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY 
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact lead developers:
//              Drew Kouri   (dpkouri@sandia.gov) and
//              Denis Ridzal (dridzal@sandia.gov)
//
// ************************************************************************
// @HEADER

#ifndef ROL_MINIMAX2_HPP
#define ROL_MINIMAX2_HPP

#include "ROL_Objective.hpp"
#include "ROL_Vector.hpp"
#include "ROL_StdVector.hpp"

#include "Teuchos_RCP.hpp"

namespace ROL {

template<class Real>
class Minimax2 : public Objective<Real> {
 
  typedef std::vector<Real> vector;
  typedef Vector<Real>      V;
  typedef StdVector<Real>   SV;

private:

  Teuchos::RCP<const vector> getVector( const V& x ) {
    using Teuchos::dyn_cast;
    using Teuchos::getConst;
    return dyn_cast<const SV>(getConst(x)).getVector();
  }

  Teuchos::RCP<vector> getVector( V& x ) {
    using Teuchos::dyn_cast;
    return dyn_cast<SV>(x).getVector();
  }

  

public:
  Minimax2(void) {}

  Real value(const Vector<Real> &x, Real &tol) {

    using Teuchos::RCP;
    RCP<const vector> xp = getVector(x);
    Real f1 = std::pow((*xp)[0],4.0) + std::pow((*xp)[1],2.0);
    Real f2 = std::pow(2.0-(*xp)[0],2.0) + std::pow(2.0-(*xp)[1],2.0);
    Real f3 = 2.0*std::exp(-(*xp)[0] + (*xp)[1]);
    return std::max(f1,std::max(f2,f3));
  }

  void gradient(Vector<Real> &g, const Vector<Real> &x, Real &tol) {

    using Teuchos::RCP;
    RCP<const vector> xp = getVector(x);
    RCP<vector> gp = getVector(g);

    Real f1 = std::pow((*xp)[0],4.0) + std::pow((*xp)[1],2.0);
    Real f2 = std::pow(2.0-(*xp)[0],2.0) + std::pow(2.0-(*xp)[1],2.0);
    Real f3 = 2.0*std::exp(-(*xp)[0] + (*xp)[1]);

    if ( f1 >= std::max(f2,f3) ) {
      (*gp)[0] = 4.0*std::pow((*xp)[0],3.0);
      (*gp)[1] = 2.0*(*xp)[1];
    }
    else if ( f2 >= std::max(f1,f3) ) {
      (*gp)[0] = 2.0*(*xp)[0]-4.0;
      (*gp)[1] = 2.0*(*xp)[1]-4.0;
    }
    else if ( f3 >= std::max(f1,f2) ) {
      (*gp)[0] = -2.0*std::exp(-(*xp)[0]+(*xp)[1]);
      (*gp)[1] = 2.0*std::exp(-(*xp)[0]+(*xp)[1]);
    } 
  }
}; // class Minimax2

} // namespace ROL

#endif
