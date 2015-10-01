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

/** \file
    \brief  Contains definitions for Poisson material inversion.
    \author Created by D. Ridzal and D. Kouri.
 */

#ifndef USE_HESSVEC 
#define USE_HESSVEC 1
#endif

#ifndef ROL_POISSONINVERSION_HPP
#define ROL_POISSONINVERSION_HPP

#include "ROL_StdVector.hpp"
#include "ROL_Objective.hpp"
#include "ROL_HelperFunctions.hpp"

#include "Teuchos_getConst.hpp"
#include "Teuchos_LAPACK.hpp"

namespace ROL {
namespace ZOO {

  /** \brief Poisson material inversion.
   */
  template<class Real>
  class Objective_PoissonInversion : public Objective<Real> {
  
    typedef std::vector<Real> vector;
    typedef Vector<Real>      V;
    typedef StdVector<Real>   SV;
  
  private:
    int nu_;
    int nz_;

    Real hu_;
    Real hz_;

    Real alpha_;

    Real eps_;
    int  reg_type_;

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

    /* CONSTRUCTOR */
    Objective_PoissonInversion(int nz = 32, Real alpha = 1.e-4) : nz_(nz), alpha_(alpha) {
      nu_       = nz_-1;
      hu_       = 1.0/((Real)nu_+1.0);
      hz_       = hu_; 
      eps_      = 1.e-4;
      reg_type_ = 2;
    }

    /* REGULARIZATION DEFINITIONS */
    Real reg_value(const Vector<Real> &z) {
      using Teuchos::RCP;

      RCP<const vector> zp = getVector(z);

      Real val = 0.0;
      for (int i = 0; i < nz_; i++) {
        if ( reg_type_ == 2 ) {
          val += alpha_/2.0 * hz_ * (*zp)[i]*(*zp)[i];
        }
        else if ( reg_type_ == 1 ) {
          val += alpha_ * hz_ * std::sqrt((*zp)[i]*(*zp)[i] + eps_);
        }
        else if ( reg_type_ == 0 ) {
          if ( i < nz_-1 ) {
            val += alpha_ * std::sqrt(std::pow((*zp)[i]-(*zp)[i+1],2.0)+eps_);
          }
        }
      }
      return val;
    }

    void reg_gradient(Vector<Real> &g, const Vector<Real> &z) {
      using Teuchos::RCP;     

      if ( reg_type_ == 2 ) {
        g.set(z);
        g.scale(alpha_*hz_);    
      } 
      else if ( reg_type_ == 1 ) {
        RCP<const vector> zp = getVector(z);
        RCP<vector >      gp = getVector(g);

        for (int i = 0; i < nz_; i++) {
          (*gp)[i] = alpha_ * hz_ * (*zp)[i]/std::sqrt(std::pow((*zp)[i],2.0)+eps_);
        }
      }
      else if ( reg_type_ == 0 ) {
        RCP<const vector> zp = getVector(z);
        RCP<vector>       gp = getVector(g);

        Real diff = 0.0;
        for (int i = 0; i < nz_; i++) {
          if ( i == 0 ) {
            diff     = (*zp)[i]-(*zp)[i+1];
            (*gp)[i] = alpha_ * diff/std::sqrt(std::pow(diff,2.0)+eps_);
          }
          else if ( i == nz_-1 ) {
            diff     = (*zp)[i-1]-(*zp)[i];
            (*gp)[i] = -alpha_ * diff/std::sqrt(std::pow(diff,2.0)+eps_);
          }
          else {
            diff      = (*zp)[i]-(*zp)[i+1];
            (*gp)[i]  = alpha_ * diff/std::sqrt(std::pow(diff,2.0)+eps_);
            diff      = (*zp)[i-1]-(*zp)[i];
            (*gp)[i] -= alpha_ * diff/std::sqrt(std::pow(diff,2.0)+eps_);
          }
        }
      }
    }

    void reg_hessVec(Vector<Real> &hv, const Vector<Real> &v, const Vector<Real> &z) {

      using Teuchos::RCP;

      if ( reg_type_ == 2 ) {
        hv.set(v);
        hv.scale(alpha_*hz_);
      }
      else if ( reg_type_ == 1 ) {
        RCP<const vector> zp  = getVector(z);
        RCP<const vector> vp  = getVector(v);
        RCP<vector>       hvp = getVector(hv);

        for (int i = 0; i < nz_; i++) {
          (*hvp)[i] = alpha_*hz_*(*vp)[i]*eps_/std::pow(std::pow((*zp)[i],2.0)+eps_,3.0/2.0);
        }
      }
      else if ( reg_type_ == 0 ) {
        RCP<const vector> zp  = getVector(z);
        RCP<const vector> vp  = getVector(v);
        RCP<vector>       hvp = getVector(hv);

        Real diff1 = 0.0;
        Real diff2 = 0.0;
        for (int i = 0; i < nz_; i++) {
          if ( i == 0 ) {
            diff1 = (*zp)[i]-(*zp)[i+1];
            diff1 = eps_/std::pow(std::pow(diff1,2.0)+eps_,3.0/2.0);
            (*hvp)[i] = alpha_* ((*vp)[i]*diff1 - (*vp)[i+1]*diff1);
          }
          else if ( i == nz_-1 ) {
            diff2 = (*zp)[i-1]-(*zp)[i];
            diff2 = eps_/std::pow(std::pow(diff2,2.0)+eps_,3.0/2.0);
            (*hvp)[i] = alpha_* (-(*vp)[i-1]*diff2 + (*vp)[i]*diff2);
          }
          else {
            diff1 = (*zp)[i]-(*zp)[i+1];
            diff1 = eps_/std::pow(std::pow(diff1,2.0)+eps_,3.0/2.0);
            diff2 = (*zp)[i-1]-(*zp)[i];
            diff2 = eps_/std::pow(std::pow(diff2,2.0)+eps_,3.0/2.0);
            (*hvp)[i] = alpha_* (-(*vp)[i-1]*diff2 + (*vp)[i]*(diff1 + diff2) - (*vp)[i+1]*diff1);
          }
        }
      }
    }

    /* FINITE ELEMENT DEFINTIONS */
    void apply_mass(Vector<Real> &Mf, const Vector<Real> &f ) {

      using Teuchos::RCP;
      RCP<const vector> fp  = getVector(f);
      RCP<vector>       Mfp = getVector(Mf);

      for (int i = 0; i < nu_; i++) {
        if ( i == 0 ) {
          (*Mfp)[i] = hu_/6.0*(4.0*(*fp)[i] + (*fp)[i+1]);
        }
        else if ( i == nu_-1 ) {
          (*Mfp)[i] = hu_/6.0*((*fp)[i-1] + 4.0*(*fp)[i]);
        }
        else {
          (*Mfp)[i] = hu_/6.0*((*fp)[i-1] + 4.0*(*fp)[i] + (*fp)[i+1]);
        }
      }
    }

    void solve_poisson(Vector<Real> &u, const Vector<Real> &z, Vector<Real> &b) {

      using Teuchos::RCP;
      RCP<const vector> zp = getVector(z);
      RCP<vector>       up = getVector(u);
      RCP<vector>       bp = getVector(b);

      // Get Diagonal and Off-Diagonal Entries of PDE Jacobian
      vector d(nu_,1.0);
      vector o(nu_-1,1.0);
      for ( int i = 0; i < nu_; i++ ) {
        d[i] = (std::exp((*zp)[i]) + std::exp((*zp)[i+1]))/hu_;
        if ( i < nu_-1 ) {
          o[i] *= -std::exp((*zp)[i+1])/hu_;
        }
      }

      // Solve Tridiagonal System Using LAPACK's SPD Tridiagonal Solver
      Teuchos::LAPACK<int,Real> lp;
      int info;
      int ldb  = nu_;
      int nhrs = 1;
      lp.PTTRF(nu_,&d[0],&o[0],&info);
      lp.PTTRS(nu_,nhrs,&d[0],&o[0],&(*bp)[0],ldb,&info);
      u.set(b);
    }

    Real evaluate_target(Real x) {
      return x*(1.0-x);
    }

    void apply_linearized_control_operator( Vector<Real> &Bd, const Vector<Real> &z, 
                                      const Vector<Real> &d,  const Vector<Real> &u ) {

      using Teuchos::RCP;
      
      RCP<const vector> zp  = getVector(z);
      RCP<const vector> up  = getVector(u);
      RCP<const vector> dp  = getVector(d);
      RCP<vector>       Bdp = getVector(Bd);

      for (int i = 0; i < nu_; i++) {
        if ( i == 0 ) {
          (*Bdp)[i] = 1.0/hu_*( std::exp((*zp)[i])*(*up)[i]*(*dp)[i] 
                                    + std::exp((*zp)[i+1])*((*up)[i]-(*up)[i+1])*(*dp)[i+1] );
        }
        else if ( i == nu_-1 ) {
          (*Bdp)[i] = 1.0/hu_*( std::exp((*zp)[i])*((*up)[i]-(*up)[i-1])*(*dp)[i] 
                                    + std::exp((*zp)[i+1])*(*up)[i]*(*dp)[i+1] );
        }
        else {
          (*Bdp)[i] = 1.0/hu_*( std::exp((*zp)[i])*((*up)[i]-(*up)[i-1])*(*dp)[i] 
                                    + std::exp((*zp)[i+1])*((*up)[i]-(*up)[i+1])*(*dp)[i+1] );
        }
      }
    }

    void apply_transposed_linearized_control_operator( Vector<Real> &Bd, const Vector<Real> &z,
                                                 const Vector<Real> &d,  const Vector<Real> &u ) {
      using Teuchos::RCP;

      RCP<const vector> zp  = getVector(z);
      RCP<const vector> up  = getVector(u);
      RCP<const vector> dp  = getVector(d);
      RCP<vector>       Bdp = getVector(Bd);

      for (int i = 0; i < nz_; i++) {
        if ( i == 0 ) {
          (*Bdp)[i] = std::exp((*zp)[i])/hu_*(*up)[i]*(*dp)[i];
        }
        else if ( i == nz_-1 ) {
          (*Bdp)[i] = std::exp((*zp)[i])/hu_*(*up)[i-1]*(*dp)[i-1];
        }
        else {
          (*Bdp)[i] = std::exp((*zp)[i])/hu_*( ((*up)[i]-(*up)[i-1])*((*dp)[i]-(*dp)[i-1]) );
        }
      }
    }
    
    void apply_transposed_linearized_control_operator_2( Vector<Real> &Bd, const Vector<Real> &z, const Vector<Real> &v,
                                                   const Vector<Real> &d,  const Vector<Real> &u ) {
      using Teuchos::RCP;
      RCP<const vector> zp  = getVector(z);
      RCP<const vector> vp  = getVector(v);
      RCP<const vector> up  = getVector(u);
      RCP<const vector> dp  = getVector(d);
      RCP<vector>       Bdp = getVector(Bd);

      for (int i = 0; i < nz_; i++) {
        if ( i == 0 ) {
          (*Bdp)[i] = (*vp)[i]*std::exp((*zp)[i])/hu_*(*up)[i]*(*dp)[i];
        }
        else if ( i == nz_-1 ) {
          (*Bdp)[i] = (*vp)[i]*std::exp((*zp)[i])/hu_*(*up)[i-1]*(*dp)[i-1];
        }
        else {
          (*Bdp)[i] = (*vp)[i]*std::exp((*zp)[i])/hu_*( ((*up)[i]-(*up)[i-1])*((*dp)[i]-(*dp)[i-1]) );
        }
      }
    }

    /* STATE AND ADJOINT EQUATION DEFINTIONS */
    void solve_state_equation(Vector<Real> &u, const Vector<Real> &z) {
    
      using Teuchos::RCP;
      using Teuchos::rcp;

      Real k1 = 1.0;
      Real k2 = 2.0;
      // Right Hand Side
      RCP<vector> bp = rcp( new vector(nu_, 0.0) );
      for ( int i = 0; i < nu_; i++ ) {
        if ( (Real)(i+1)*hu_ < 0.5 ) {
         (*bp)[i] = 2.0*k1*hu_;
        }
        else if ( std::abs((Real)(i+1)*hu_ - 0.5) < ROL_EPSILON ) {
         (*bp)[i] = (k1+k2)*hu_;
        }
        else if ( (Real)(i+1)*hu_ > 0.5 ) {
         (*bp)[i] = 2.0*k2*hu_;
        }
      }
     
      SV b(bp);
      // Solve Equation
      solve_poisson(u,z,b);
    }

    void solve_adjoint_equation(Vector<Real> &p, const Vector<Real> &u, const Vector<Real> &z) {
 
      using Teuchos::RCP;
      using Teuchos::rcp;

      RCP<const vector> up = getVector(u);
      RCP<vector> rp = rcp( new vector(nu_,0.0) );
      SV res(rp);

      for (int i = 0; i < nu_; i++) {
        (*rp)[i] = -((*up)[i]-evaluate_target((Real)(i+1)*hu_));
      }
      StdVector<Real> Mres( Teuchos::rcp( new std::vector<Real>(nu_,0.0) ) );
      apply_mass(Mres,res);
      solve_poisson(p,z,Mres);
    }

    void solve_state_sensitivity_equation(Vector<Real> &w, const Vector<Real> &v, 
                                          const Vector<Real> &u, const Vector<Real> &z) {
      using Teuchos::rcp;
      SV b( rcp( new vector(nu_,0.0) ) );
      apply_linearized_control_operator(b,z,v,u);
      solve_poisson(w,z,b);
    }

    void solve_adjoint_sensitivity_equation(Vector<Real> &q, const Vector<Real> &w, const Vector<Real> &v,
                                            const Vector<Real> &p, const Vector<Real> &u, const Vector<Real> &z) {

      using Teuchos::rcp;

      SV res( rcp( new vector(nu_,0.0) ) );
      apply_mass(res,w);
      SV res1( rcp( new vector(nu_,0.0) ) );
      apply_linearized_control_operator(res1,z,v,p);
      res.axpy(-1.0,res1);
      solve_poisson(q,z,res);
    }

    /* OBJECTIVE FUNCTION DEFINITIONS */
    Real value( const Vector<Real> &z, Real &tol ) {

      using Teuchos::RCP;
      using Teuchos::rcp;

      // SOLVE STATE EQUATION
      RCP<vector> up = rcp( new vector(nu_,0.0) );
      SV u( up );

      solve_state_equation(u,z);

      // COMPUTE MISFIT
      RCP<vector> rp = rcp( new vector(nu_,0.0) );
      SV res( rp );

      for (int i = 0; i < nu_; i++) {
        (*rp)[i] = ((*up)[i]-evaluate_target((Real)(i+1)*hu_));
      }

      RCP<V> Mres = res.clone();
      apply_mass(*Mres,res);
      return 0.5*Mres->dot(res) + reg_value(z);
    } 

    void gradient( Vector<Real> &g, const Vector<Real> &z, Real &tol ) {

      using Teuchos::rcp; 

      // SOLVE STATE EQUATION
      SV u( rcp( new vector(nu_,0.0) ) );
      solve_state_equation(u,z);

      // SOLVE ADJOINT EQUATION
      SV p( Teuchos::rcp( new std::vector<Real>(nu_,0.0) ) );
      solve_adjoint_equation(p,u,z);

      // Apply Transpose of Linearized Control Operator
      apply_transposed_linearized_control_operator(g,z,p,u);
     
      // Regularization gradient
      SV g_reg( rcp( new vector(nz_,0.0) ) );
      reg_gradient(g_reg,z); 

      // Build Gradient
      g.plus(g_reg);
    }
#if USE_HESSVEC
    void hessVec( Vector<Real> &hv, const Vector<Real> &v, const Vector<Real> &z, Real &tol ) {
 
      using Teuchos::rcp;

      // SOLVE STATE EQUATION
      SV u( rcp( new vector(nu_,0.0) ) );
      solve_state_equation(u,z);

      // SOLVE ADJOINT EQUATION
      SV p( rcp( new vector(nu_,0.0) ) );
      solve_adjoint_equation(p,u,z);

      // SOLVE STATE SENSITIVITY EQUATION
      SV w( rcp( new vector(nu_,0.0) ) );
      solve_state_sensitivity_equation(w,v,u,z);

      // SOLVE ADJOINT SENSITIVITY EQUATION
      SV q( rcp( new vector(nu_,0.0) ) );
      solve_adjoint_sensitivity_equation(q,w,v,p,u,z);

      // Apply Transpose of Linearized Control Operator
      apply_transposed_linearized_control_operator(hv,z,q,u);
    
      // Apply Transpose of Linearized Control Operator
      SV tmp( rcp( new vector(nz_,0.0) ) );
      apply_transposed_linearized_control_operator(tmp,z,w,p);
      hv.axpy(-1.0,tmp); 

      // Apply Transpose of 2nd Derivative of Control Operator
      tmp.zero();
      apply_transposed_linearized_control_operator_2(tmp,z,v,p,u);
      hv.plus(tmp);

      // Regularization hessVec
      SV hv_reg( rcp( new vector(nz_,0.0) ) );
      reg_hessVec(hv_reg,v,z);

      // Build hessVec
      hv.plus(hv_reg);
    }
#endif

    void invHessVec( Vector<Real> &hv, const Vector<Real> &v, const Vector<Real> &x, Real &tol ) {

      using Teuchos::RCP;
      using Teuchos::rcp;

      // Cast hv and v vectors to std::vector.
      RCP<vector> hvp       = getVector(hv);

      std::vector<Real> vp(*getVector(v));

      int dim = static_cast<int>(vp.size());


      // Compute dense Hessian.
      Teuchos::SerialDenseMatrix<int, Real> H(dim, dim);
      Objective_PoissonInversion<Real> & obj = *this; 
      H = computeDenseHessian<Real>(obj, x);

      // Compute eigenvalues, sort real part.
      std::vector<vector> eigenvals = computeEigenvalues<Real>(H);
      std::sort((eigenvals[0]).begin(), (eigenvals[0]).end());

      // Perform 'inertia' correction.
      Real inertia = (eigenvals[0])[0];
      Real correction = 0.0;
      if ( inertia <= 0.0 ) {
        correction = 2.0*std::abs(inertia);
        if ( inertia == 0.0 ) {
          int cnt = 0;
          while ( eigenvals[0][cnt] == 0.0 ) {
            cnt++;
          }
          correction = 0.5*eigenvals[0][cnt];
          if ( cnt == dim-1 ) {
            correction = 1.0;
          }
        }
        for (int i=0; i<dim; i++) {
          H(i,i) += correction;
        }
      }

      // Compute dense inverse Hessian.
      Teuchos::SerialDenseMatrix<int, Real> invH = computeInverse<Real>(H);

      // Apply dense inverse Hessian.
      Teuchos::SerialDenseVector<int, Real> hv_teuchos(Teuchos::View, &((*hvp)[0]), dim);
      const Teuchos::SerialDenseVector<int, Real> v_teuchos(Teuchos::View, &(vp[0]), dim);
      hv_teuchos.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1.0, invH, v_teuchos, 0.0);
    }

  };

  template<class Real>
  void getPoissonInversion( Teuchos::RCP<Objective<Real> > &obj, Vector<Real> &x0, Vector<Real> &x ) {

    typedef std::vector<Real> vector;
    using Teuchos::RCP;
    using Teuchos::rcp;

    // Cast Initial Guess and Solution Vectors
    RCP<vector> x0p = getVector(x0);
    RCP<vector>  xp = getVector(x);

    int n = xp->size();
    // Resize Vectors
    n = 128;
    x0p->resize(n);
    xp->resize(n);
    // Instantiate Objective Function
    obj = rcp( new Objective_PoissonInversion<Real>(n,1.e-6) );
    // Get Initial Guess
    for (int i=0; i<n; i++) {
      (*x0p)[i] = 1.5;
    }
    // Get Solution
    Real h  = 1.0/((Real)n+1);
    Real pt = 0.0;
    Real k1 = 1.0;
    Real k2 = 2.0;
    for( int i=0; i<n; i++ ) {
      pt = (Real)(i+1)*h;
      if ( pt >= 0.0 && pt < 0.5 ) {
        (*xp)[i] = std::log(k1);
      }
      else if ( pt >= 0.5 && pt < 1.0 ) {
        (*xp)[i] = std::log(k2); 
      }
    }
  }

} // End ZOO Namespace
} // End ROL Namespace

#endif
