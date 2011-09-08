// @HEADER
// ***********************************************************************
// 
//    Thyra: Interfaces and Support for Abstract Numerical Algorithms
//                 Copyright (2004) Sandia Corporation
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
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ***********************************************************************
// @HEADER

#ifndef THYRA_DIAGONAL_LINEAR_OP_WITH_SOLVE_HPP
#define THYRA_DIAGONAL_LINEAR_OP_WITH_SOLVE_HPP

#include "Thyra_DefaultDiagonalLinearOpWithSolve_decl.hpp"
#include "Thyra_DefaultDiagonalLinearOp.hpp"
#include "Thyra_MultiVectorStdOps.hpp"
#include "Thyra_VectorBase.hpp"
#include "Thyra_TestingTools.hpp" // ToDo: I need to have a better way to get eps()!
#include "Teuchos_Assert.hpp"


namespace Thyra {


// Constructors/initializers/accessors


template<class Scalar>
DefaultDiagonalLinearOpWithSolve<Scalar>::DefaultDiagonalLinearOpWithSolve()
{}


template<class Scalar>
DefaultDiagonalLinearOpWithSolve<Scalar>::DefaultDiagonalLinearOpWithSolve(
  const RCP<const VectorBase<Scalar> >   &diag
  )
{
  initialize(diag);
}


// protected


// Overridden from LinearOpWithSolveBase


template<class Scalar>
bool
DefaultDiagonalLinearOpWithSolve<Scalar>::solveSupportsImpl(
  EOpTransp M_trans) const
{
  typedef Teuchos::ScalarTraits<Scalar> ST;
  return (ST::isComplex ? M_trans==NOTRANS || M_trans==TRANS : true);
}


template<class Scalar>
bool
DefaultDiagonalLinearOpWithSolve<Scalar>::solveSupportsSolveMeasureTypeImpl(
  EOpTransp M_trans, const SolveMeasureType& solveMeasureType) const
{
  return this->solveSupportsImpl(M_trans); // I am a direct solver!
}


template<class Scalar>
SolveStatus<Scalar>
DefaultDiagonalLinearOpWithSolve<Scalar>::solveImpl(
  const EOpTransp transp,
  const MultiVectorBase<Scalar> &B,
  const Ptr<MultiVectorBase<Scalar> > &X,
  const Ptr<const SolveCriteria<Scalar> > solveCriteria
  ) const
{

#ifdef THYRA_DEBUG
  TEUCHOS_ASSERT(this->solveSupportsImpl(transp));
#endif

  typedef Teuchos::ScalarTraits<Scalar> ST;

  assign(X, ST::zero());
  
  const Ordinal numCols = B.domain()->dim();
  SolveStatus<Scalar> overallSolveStatus;

  for (Ordinal col_j = 0; col_j < numCols; ++col_j) {

    const RCP<const VectorBase<Scalar> > b = B.col(col_j);
    const RCP<VectorBase<Scalar> > x = X->col(col_j);

    ele_wise_divide( ST::one(), *b, *this->getDiag(), x.ptr() );

  }

  SolveStatus<Scalar> solveStatus;
  solveStatus.solveStatus =
    (nonnull(solveCriteria) && !solveCriteria->solveMeasureType.useDefault()
      ? SOLVE_STATUS_CONVERGED : SOLVE_STATUS_UNKNOWN );
  solveStatus.achievedTol = SolveStatus<Scalar>::unknownTolerance();
  return solveStatus;

}


}	// end namespace Thyra


#endif	// THYRA_DIAGONAL_LINEAR_OP_WITH_SOLVE_HPP
