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

/*! \file  obj.hpp
    \brief Provides the interface for local (cell-based) objective function computations.
*/

#ifndef PDEOPT_QOI_STOCHASTIC_STEFAN_BOLTZMANN_HPP
#define PDEOPT_QOI_STOCHASTIC_STEFAN_BOLTZMANN_HPP

#include "../TOOLS/qoi.hpp"
#include "pde_stoch_stefan_boltzmann.hpp"

template <class Real>
class QoI_StateCost : public QoI<Real> {
private:
  const Teuchos::RCP<FE<Real> > fe_;
  Teuchos::RCP<Intrepid::FieldContainer<Real> > weight_;
  Real xmid_;
  Real T_;

  Real weightFunc(const std::vector<Real> & x) const {
    return ((x[1] < xmid_) ? static_cast<Real>(0) : static_cast<Real>(1));
  }

  Real cost(const Real u, const int deriv = 0) const {
    if ( u > T_ ) {
      if ( deriv == 0 ) {
        return static_cast<Real>(0.5)*(u-T_)*(u-T_);
      }
      if ( deriv == 1 ) {
        return (u-T_);
      }
      if ( deriv == 2 ) {
        return static_cast<Real>(1);
      }
    }
    return static_cast<Real>(0);
  }

public:
  QoI_StateCost(const Teuchos::RCP<FE<Real> > &fe,
                Teuchos::ParameterList &parlist) : fe_(fe) {
    xmid_ = parlist.sublist("Geometry").get<Real>("Step height");
    T_    = parlist.sublist("Problem").get("Desired engine temperature",373.0);
    int c = fe_->cubPts()->dimension(0);
    int p = fe_->cubPts()->dimension(1);
    int d = fe_->cubPts()->dimension(2);
    std::vector<Real> pt(d);
    weight_ = Teuchos::rcp(new Intrepid::FieldContainer<Real>(c,p));
    for (int i = 0; i < c; ++i) {
      for (int j = 0; j < p; ++j) {
        for (int k = 0; k < d; ++k) {
          pt[k] = (*fe_->cubPts())(i,j,k);
        }
        (*weight_)(i,j) = weightFunc(pt);
      }
    }
  }

  Real value(Teuchos::RCP<Intrepid::FieldContainer<Real> > & val,
             const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & u_coeff,
             const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & z_coeff = Teuchos::null,
             const Teuchos::RCP<const std::vector<Real> > & z_param = Teuchos::null) {
    // Get relevant dimensions
    const int c = fe_->gradN()->dimension(0);
    const int p = fe_->gradN()->dimension(2);
    // Initialize output val
    val = Teuchos::rcp(new Intrepid::FieldContainer<Real>(c));
    // Evaluate state on FE basis
    Teuchos::RCP<Intrepid::FieldContainer<Real> > valU_eval
      = Teuchos::rcp(new Intrepid::FieldContainer<Real>(c, p));
    fe_->evaluateValue(valU_eval, u_coeff);
    // Compute cost
    Teuchos::RCP<Intrepid::FieldContainer<Real> > costU
      = Teuchos::rcp(new Intrepid::FieldContainer<Real>(c, p));
    for (int i = 0; i < c; ++i) {
      for (int j = 0; j < p; ++j) {
        (*costU)(i,j) = cost((*valU_eval)(i,j),0);
      }
    }
    // Compute squared L2-norm of diff
    fe_->computeIntegral(val,costU,weight_);
    return static_cast<Real>(0);
  }

  void gradient_1(Teuchos::RCP<Intrepid::FieldContainer<Real> > & grad,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & u_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & z_coeff = Teuchos::null,
                  const Teuchos::RCP<const std::vector<Real> > & z_param = Teuchos::null) {
    // Get relevant dimensions
    const int c = fe_->gradN()->dimension(0);
    const int f = fe_->gradN()->dimension(1);
    const int p = fe_->gradN()->dimension(2);
    // Initialize output grad
    grad = Teuchos::rcp(new Intrepid::FieldContainer<Real>(c, f));
    // Evaluate state on FE basis
    Teuchos::RCP<Intrepid::FieldContainer<Real> > valU_eval
      = Teuchos::rcp(new Intrepid::FieldContainer<Real>(c, p));
    fe_->evaluateValue(valU_eval, u_coeff);
    // Compute cost
    Teuchos::RCP<Intrepid::FieldContainer<Real> > costU
      = Teuchos::rcp(new Intrepid::FieldContainer<Real>(c, p));
    for (int i = 0; i < c; ++i) {
      for (int j = 0; j < p; ++j) {
        (*costU)(i,j) = cost((*valU_eval)(i,j),1);
      }
    }
    // Multiply cost and weight
    Intrepid::FieldContainer<Real> WcostU(c, p);
    Intrepid::FunctionSpaceTools::scalarMultiplyDataData<Real>(WcostU,
                                                               *weight_,
                                                               *costU);
    // Compute gradient of squared L2-norm of diff
    Intrepid::FunctionSpaceTools::integrate<Real>(*grad,
                                                  WcostU,
                                                  *(fe_->NdetJ()),
                                                  Intrepid::COMP_CPP, false);
  }

  void gradient_2(Teuchos::RCP<Intrepid::FieldContainer<Real> > & grad,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & u_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & z_coeff = Teuchos::null,
                  const Teuchos::RCP<const std::vector<Real> > & z_param = Teuchos::null) {
    throw Exception::Zero(">>> QoI_StateCost::gradient_2 is zero.");
  }

  void HessVec_11(Teuchos::RCP<Intrepid::FieldContainer<Real> > & hess,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & v_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & u_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & z_coeff = Teuchos::null,
                  const Teuchos::RCP<const std::vector<Real> > & z_param = Teuchos::null) {
    // Get relevant dimensions
    const int c = fe_->gradN()->dimension(0);
    const int f = fe_->gradN()->dimension(1);
    const int p = fe_->gradN()->dimension(2);
    // Initialize output grad
    hess = Teuchos::rcp(new Intrepid::FieldContainer<Real>(c, f));
    // Evaluate state on FE basis
    Teuchos::RCP<Intrepid::FieldContainer<Real> > valU_eval
      = Teuchos::rcp(new Intrepid::FieldContainer<Real>(c, p));
    fe_->evaluateValue(valU_eval, u_coeff);
    // Evaluate direction on FE basis
    Teuchos::RCP<Intrepid::FieldContainer<Real> > valV_eval
      = Teuchos::rcp(new Intrepid::FieldContainer<Real>(c, p));
    fe_->evaluateValue(valV_eval, v_coeff);
    // Compute cost
    Teuchos::RCP<Intrepid::FieldContainer<Real> > costU
      = Teuchos::rcp(new Intrepid::FieldContainer<Real>(c, p));
    for (int i = 0; i < c; ++i) {
      for (int j = 0; j < p; ++j) {
        (*costU)(i,j) = cost((*valU_eval)(i,j),2);
      }
    }
    // Multiply cost and weight
    Intrepid::FieldContainer<Real> WcostU(c, p);
    Intrepid::FunctionSpaceTools::scalarMultiplyDataData<Real>(WcostU,
                                                               *weight_,
                                                               *costU);
    // Multiply weighted cost and direction
    Intrepid::FieldContainer<Real> WcostUV(c, p);
    Intrepid::FunctionSpaceTools::scalarMultiplyDataData<Real>(WcostUV,
                                                               WcostU,
                                                               *valV_eval);
    // Compute gradient of squared L2-norm of diff
    Intrepid::FunctionSpaceTools::integrate<Real>(*hess,
                                                  WcostUV,
                                                  *(fe_->NdetJ()),
                                                  Intrepid::COMP_CPP, false);
  }

  void HessVec_12(Teuchos::RCP<Intrepid::FieldContainer<Real> > & hess,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & v_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & u_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & z_coeff = Teuchos::null,
                  const Teuchos::RCP<const std::vector<Real> > & z_param = Teuchos::null) {
    throw Exception::Zero(">>> QoI_StateCost::HessVec_12 is zero.");
  }

  void HessVec_21(Teuchos::RCP<Intrepid::FieldContainer<Real> > & hess,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & v_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & u_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & z_coeff = Teuchos::null,
                  const Teuchos::RCP<const std::vector<Real> > & z_param = Teuchos::null) {
    throw Exception::Zero(">>> QoI_StateCost::HessVec_21 is zero.");
  }

  void HessVec_22(Teuchos::RCP<Intrepid::FieldContainer<Real> > & hess,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & v_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & u_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & z_coeff = Teuchos::null,
                  const Teuchos::RCP<const std::vector<Real> > & z_param = Teuchos::null) {
    throw Exception::Zero(">>> QoI_StateCost::HessVec_22 is zero.");
  }

}; // QoI_L2Tracking

template <class Real>
class QoI_ControlCost : public QoI<Real> {
private:
  const Teuchos::RCP<FE<Real> > fe_vol_;
  const std::vector<Teuchos::RCP<FE<Real> > > fe_bdry_;
  const std::vector<std::vector<int> > bdryCellLocIds_;
  std::vector<Teuchos::RCP<Intrepid::FieldContainer<Real> > > weight_;
  Real T_;

  Real weightFunc(const std::vector<Real> & x) const {
    return static_cast<Real>(1);
  }

  Real cost(const Real z, const int deriv = 0) const {
    if ( deriv == 0 ) {
      return static_cast<Real>(0.5)*(z-T_)*(z-T_);
    }
    if ( deriv == 1 ) {
      return (z-T_);
    }
    if ( deriv == 2 ) {
      return static_cast<Real>(1);
    }
    return static_cast<Real>(0);
  }

  Teuchos::RCP<Intrepid::FieldContainer<Real> > getBoundaryCoeff(
      const Intrepid::FieldContainer<Real> & cell_coeff,
      int locSideId) const {
    std::vector<int> bdryCellLocId = bdryCellLocIds_[locSideId];
    const int numCellsSide = bdryCellLocId.size();
    const int f = fe_vol_->N()->dimension(1);
    
    Teuchos::RCP<Intrepid::FieldContainer<Real > > bdry_coeff = 
      Teuchos::rcp(new Intrepid::FieldContainer<Real > (numCellsSide, f));
    for (int i = 0; i < numCellsSide; ++i) {
      for (int j = 0; j < f; ++j) {
        (*bdry_coeff)(i, j) = cell_coeff(bdryCellLocId[i], j);
      }
    }
    return bdry_coeff;
  }

public:
  QoI_ControlCost(const Teuchos::RCP<FE<Real> > &fe_vol,
                  const std::vector<Teuchos::RCP<FE<Real> > > &fe_bdry,
                  const std::vector<std::vector<int> > &bdryCellLocIds,
                  Teuchos::ParameterList &parlist)
    : fe_vol_(fe_vol), fe_bdry_(fe_bdry), bdryCellLocIds_(bdryCellLocIds) {
    T_ = parlist.sublist("Problem").get("Desired control temperature",293.0);
    const int numLocSides = bdryCellLocIds_.size();
    const int d = fe_vol_->gradN()->dimension(3);
    std::vector<Real> pt(d);
    weight_.resize(numLocSides);
    for (int l = 0; l < numLocSides; ++l) {
      const int numCellsSide  = bdryCellLocIds_[l].size();
      if ( numCellsSide ) {
        const int numCubPerSide = fe_bdry_[l]->cubPts()->dimension(1);
        weight_[l] = Teuchos::rcp(new Intrepid::FieldContainer<Real>(numCellsSide, numCubPerSide));
        for (int i = 0; i < numCellsSide; ++i) {
          for (int j = 0; j < numCubPerSide; ++j) {
            for (int k = 0; k < d; ++k) {
              pt[k] = (*fe_bdry_[l]->cubPts())(i,j,k);
            }
            (*weight_[l])(i,j) = weightFunc(pt);
          }
        }
      }
    }
  }

  Real value(Teuchos::RCP<Intrepid::FieldContainer<Real> > & val,
             const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & u_coeff,
             const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & z_coeff = Teuchos::null,
             const Teuchos::RCP<const std::vector<Real> > & z_param = Teuchos::null) {
    const int c = fe_vol_->gradN()->dimension(0);
    // Initialize output val
    val = Teuchos::rcp(new Intrepid::FieldContainer<Real>(c));
    // Compute cost integral
    const int numLocSides = bdryCellLocIds_.size();
    for (int l = 0; l < numLocSides; ++l) {
      const int numCellsSide  = bdryCellLocIds_[l].size();
      if ( numCellsSide ) {
        const int numCubPerSide = fe_bdry_[l]->cubPts()->dimension(1);
        // Evaluate control on FE basis
        Teuchos::RCP<Intrepid::FieldContainer<Real> > z_coeff_bdry
          = getBoundaryCoeff(*z_coeff, l);
        Teuchos::RCP<Intrepid::FieldContainer<Real> > valZ_eval
          = Teuchos::rcp(new Intrepid::FieldContainer<Real>(numCellsSide, numCubPerSide));
        fe_bdry_[l]->evaluateValue(valZ_eval, z_coeff_bdry);
        // Compute cost
        Teuchos::RCP<Intrepid::FieldContainer<Real> > costZ
          = Teuchos::rcp(new Intrepid::FieldContainer<Real>(numCellsSide, numCubPerSide));
        for (int i = 0; i < numCellsSide; ++i) {
          for (int j = 0; j < numCubPerSide; ++j) {
            (*costZ)(i,j) = cost((*valZ_eval)(i,j),0);
          }
        }
        // Integrate cell cost
        Teuchos::RCP<Intrepid::FieldContainer<Real> > intVal
          = Teuchos::rcp(new Intrepid::FieldContainer<Real>(numCellsSide));
        fe_bdry_[l]->computeIntegral(intVal,costZ,weight_[l]);
        // Add to integral value
        for (int i = 0; i < numCellsSide; ++i) {
          int cidx = bdryCellLocIds_[l][i];
          (*val)(cidx) += (*intVal)(i);
        }
      }
    }
    return static_cast<Real>(0);
  }

  void gradient_1(Teuchos::RCP<Intrepid::FieldContainer<Real> > & grad,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & u_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & z_coeff = Teuchos::null,
                  const Teuchos::RCP<const std::vector<Real> > & z_param = Teuchos::null) {
    throw Exception::Zero(">>> QoI_ControlCost::gradient_1 is zero.");
  }

  void gradient_2(Teuchos::RCP<Intrepid::FieldContainer<Real> > & grad,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & u_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & z_coeff = Teuchos::null,
                  const Teuchos::RCP<const std::vector<Real> > & z_param = Teuchos::null) {
    const int c = fe_vol_->gradN()->dimension(0);
    const int f = fe_vol_->gradN()->dimension(1);
    // Initialize output val
    grad = Teuchos::rcp(new Intrepid::FieldContainer<Real>(c, f));
    // Compute cost integral
    const int numLocSides = bdryCellLocIds_.size();
    for (int l = 0; l < numLocSides; ++l) {
      const int numCellsSide  = bdryCellLocIds_[l].size();
      if ( numCellsSide ) {
        const int numCubPerSide = fe_bdry_[l]->cubPts()->dimension(1);
        // Evaluate control on FE basis
        Teuchos::RCP<Intrepid::FieldContainer<Real> > z_coeff_bdry
          = getBoundaryCoeff(*z_coeff, l);
        Teuchos::RCP<Intrepid::FieldContainer<Real> > valZ_eval
          = Teuchos::rcp(new Intrepid::FieldContainer<Real>(numCellsSide, numCubPerSide));
        fe_bdry_[l]->evaluateValue(valZ_eval, z_coeff_bdry);
        // Compute cost
        Teuchos::RCP<Intrepid::FieldContainer<Real> > costZ
          = Teuchos::rcp(new Intrepid::FieldContainer<Real>(numCellsSide, numCubPerSide));
        for (int i = 0; i < numCellsSide; ++i) {
          for (int j = 0; j < numCubPerSide; ++j) {
            (*costZ)(i,j) = cost((*valZ_eval)(i,j),1);
          }
        }
        // Multiply cost and weight
        Intrepid::FieldContainer<Real> WcostZ(numCellsSide, numCubPerSide);
        Intrepid::FunctionSpaceTools::scalarMultiplyDataData<Real>(WcostZ,
                                                                   *(weight_[l]),
                                                                   *costZ);
        // Compute gradient of squared L2-norm of diff
        Teuchos::RCP<Intrepid::FieldContainer<Real> > intGrad
          = Teuchos::rcp(new Intrepid::FieldContainer<Real>(numCellsSide, f));
        Intrepid::FunctionSpaceTools::integrate<Real>(*intGrad,
                                                      WcostZ,
                                                      *(fe_bdry_[l]->NdetJ()),
                                                      Intrepid::COMP_CPP, false);
        // Add to integral value
        for (int i = 0; i < numCellsSide; ++i) {
          int cidx = bdryCellLocIds_[l][i];
          for (int j = 0; j < f; ++j) {
            (*grad)(cidx,j) += (*intGrad)(i,j);
          }
        }
      }
    }
  }

  void HessVec_11(Teuchos::RCP<Intrepid::FieldContainer<Real> > & hess,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & v_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & u_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & z_coeff = Teuchos::null,
                  const Teuchos::RCP<const std::vector<Real> > & z_param = Teuchos::null) {
    throw Exception::Zero(">>> QoI_ControlCost::HessVec_11 is zero.");
  }

  void HessVec_12(Teuchos::RCP<Intrepid::FieldContainer<Real> > & hess,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & v_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & u_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & z_coeff = Teuchos::null,
                  const Teuchos::RCP<const std::vector<Real> > & z_param = Teuchos::null) {
    throw Exception::Zero(">>> QoI_ControlCost::HessVec_12 is zero.");
  }

  void HessVec_21(Teuchos::RCP<Intrepid::FieldContainer<Real> > & hess,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & v_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & u_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & z_coeff = Teuchos::null,
                  const Teuchos::RCP<const std::vector<Real> > & z_param = Teuchos::null) {
    throw Exception::Zero(">>> QoI_ControlCost::HessVec_21 is zero.");
  }

  void HessVec_22(Teuchos::RCP<Intrepid::FieldContainer<Real> > & hess,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & v_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & u_coeff,
                  const Teuchos::RCP<const Intrepid::FieldContainer<Real> > & z_coeff = Teuchos::null,
                  const Teuchos::RCP<const std::vector<Real> > & z_param = Teuchos::null) {
    const int c = fe_vol_->gradN()->dimension(0);
    const int f = fe_vol_->gradN()->dimension(1);
    // Initialize output val
    hess = Teuchos::rcp(new Intrepid::FieldContainer<Real>(c, f));
    // Compute cost integral
    const int numLocSides = bdryCellLocIds_.size();
    for (int l = 0; l < numLocSides; ++l) {
      const int numCellsSide  = bdryCellLocIds_[l].size();
      if ( numCellsSide ) {
        const int numCubPerSide = fe_bdry_[l]->cubPts()->dimension(1);
        // Evaluate control on FE basis
        Teuchos::RCP<Intrepid::FieldContainer<Real> > z_coeff_bdry
          = getBoundaryCoeff(*z_coeff, l);
        Teuchos::RCP<Intrepid::FieldContainer<Real> > valZ_eval
          = Teuchos::rcp(new Intrepid::FieldContainer<Real>(numCellsSide, numCubPerSide));
        fe_bdry_[l]->evaluateValue(valZ_eval, z_coeff_bdry);
        // Evaluate direction on FE basis
        Teuchos::RCP<Intrepid::FieldContainer<Real> > v_coeff_bdry
          = getBoundaryCoeff(*v_coeff, l);
        Teuchos::RCP<Intrepid::FieldContainer<Real> > valV_eval
          = Teuchos::rcp(new Intrepid::FieldContainer<Real>(numCellsSide, numCubPerSide));
        fe_bdry_[l]->evaluateValue(valV_eval, v_coeff_bdry);
        // Compute cost
        Teuchos::RCP<Intrepid::FieldContainer<Real> > costZ
          = Teuchos::rcp(new Intrepid::FieldContainer<Real>(numCellsSide, numCubPerSide));
        for (int i = 0; i < numCellsSide; ++i) {
          for (int j = 0; j < numCubPerSide; ++j) {
            (*costZ)(i,j) = cost((*valZ_eval)(i,j),2);
          }
        }
        // Multiply cost and weight
        Intrepid::FieldContainer<Real> WcostZ(numCellsSide, numCubPerSide);
        Intrepid::FunctionSpaceTools::scalarMultiplyDataData<Real>(WcostZ,
                                                                   *(weight_[l]),
                                                                   *costZ);
        // Multiply weighted cost and direction
        Intrepid::FieldContainer<Real> WcostZV(numCellsSide, numCubPerSide);
        Intrepid::FunctionSpaceTools::scalarMultiplyDataData<Real>(WcostZV,
                                                                   WcostZ,
                                                                   *valV_eval);
        // Compute hessian times a vector of cost
        Teuchos::RCP<Intrepid::FieldContainer<Real> > intHess
          = Teuchos::rcp(new Intrepid::FieldContainer<Real>(numCellsSide, f));
        Intrepid::FunctionSpaceTools::integrate<Real>(*intHess,
                                                      WcostZV,
                                                      *(fe_bdry_[l]->NdetJ()),
                                                      Intrepid::COMP_CPP, false);
        // Add to integral value
        for (int i = 0; i < numCellsSide; ++i) {
          int cidx = bdryCellLocIds_[l][i];
          for (int j = 0; j < f; ++j) {
            (*hess)(cidx,j) += (*intHess)(i,j);
          }
        }
      }
    }
  }

}; // QoI_L2Penalty

template <class Real>
class StochasticStefanBoltzmannStdObjective : public ROL::ParametrizedStdObjective<Real> {
private:
  Real alpha0_;
  Real alpha1_;

public:
  StochasticStefanBoltzmannStdObjective(Teuchos::ParameterList &parlist) {
    alpha0_ = parlist.sublist("Problem").get("State Cost",1.0);
    alpha1_ = parlist.sublist("Problem").get("Control Cost",1.0);
  }

  Real value(const std::vector<Real> &x, Real &tol) {
    return alpha0_*x[0] + alpha1_*x[1];
  }

  void gradient(std::vector<Real> &g, const std::vector<Real> &x, Real &tol) {
    g[0] = alpha0_;
    g[1] = alpha1_;
  }

  void hessVec(std::vector<Real> &hv, const std::vector<Real> &v, const std::vector<Real> &x, Real &tol) {
    const Real zero(0);
    hv[0] = zero;
    hv[1] = zero;
  }

}; // OBJ_SCALAR

#endif
