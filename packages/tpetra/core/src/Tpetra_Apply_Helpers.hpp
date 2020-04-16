// @HEADER
// ***********************************************************************
//
//          Tpetra: Templated Linear Algebra Services Package
//                 Copyright (2008) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
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
// ************************************************************************
// @HEADER

#ifndef TPETRA_APPLY_HELPERS_HPP
#define TPETRA_APPLY_HELPERS_HPP
#include <type_traits>
#include "Teuchos_ScalarTraits.hpp"
#include "Tpetra_CrsMatrix.hpp"
#include "Tpetra_Details_Profiling.hpp"

namespace Tpetra {
  //! @name Methods performing multiple matrix-vector products at once
  //@{
  /// \brief Does multiply matrix apply() calls with a single X vector
  ///
  /// Computes Y[i] = Matrices[i] * X, for i=1,...,N
  ///
  /// This routine only communicates the interprocessor portions of X once,
  /// if such a thing is possible (aka the ColMap's of the matrices match). 
  ///
  /// \param Matrices [in] - [std::vector|Teuchos::Array|Teuchos::ArrayRCP] of Tpetra::CrsMatrix objects.
  ///                        These matrices can different numbers of rows. 
  /// \param X [in]        - Tpetra::MultiVector or Tpetra::Vector object.
  /// \param Y [out]       - [std::vector|Teuchos::Array|Teuchos::ArrayRCP] of Tpetra::MultiVector or Tpetra::Vector objects.
  ///                        These must have the same number of vectors as X.
  template <class MatrixArray, class MultiVectorArray> 
  void batchedApply(const MatrixArray &Matrices, 
                    const typename std::remove_pointer<typename MultiVectorArray::value_type>::type &X,
                    MultiVectorArray &Y,
                    typename std::remove_pointer<typename MatrixArray::value_type>::type::scalar_type alpha = Teuchos::ScalarTraits< typename std::remove_pointer<typename MatrixArray::value_type>::type::scalar_type>::one(),
                    typename std::remove_pointer<typename MatrixArray::value_type>::type::scalar_type beta  = Teuchos::ScalarTraits< typename std::remove_pointer<typename MatrixArray::value_type>::type::scalar_type>::zero() ) {
    Tpetra::Details::ProfilingRegion regionImport ("Tpetra::batchedApply");

    using size_type   = typename MatrixArray::size_type;
    using matrix_type = typename std::remove_pointer<typename MatrixArray::value_type>::type;
    using map_type    = typename matrix_type::map_type;
    using import_type = typename matrix_type::import_type;
    using export_type = typename matrix_type::export_type;
    using MV          = typename matrix_type::MV;
    using scalar_type = typename matrix_type::scalar_type;

    using Teuchos::RCP;
    using Teuchos::rcp_const_cast;

    const scalar_type ONE  = Teuchos::ScalarTraits<scalar_type>::one();
    const scalar_type ZERO = Teuchos::ScalarTraits<scalar_type>::zero();

    // FIXME: Does not work for replicated Y's
    // FIXME: Pointer aliasing of X and Y doesn't work, unless you only alias the last Y

    size_type N = Matrices.size();
    if(N==0) return;

    RCP<const map_type> compare_domainMap = Matrices[0]->getDomainMap();
    RCP<const map_type> compare_colMap    = Matrices[0]->getColMap();
    RCP<const import_type> importer       = Matrices[0]->getGraph()->getImporter();
    bool can_batch = (importer.is_null() || N==1) ? false :true;

    // Do the domain/column maps all match?
    for(size_type i=1; i<N && can_batch; i++) {
      if(&*Matrices[i]->getColMap()    != &*compare_colMap || 
         &*Matrices[i]->getDomainMap() != &*compare_domainMap){
        can_batch=false;
      }
    }

    if(can_batch) {
      /* Batching path: Guarantees an existing importer and N>1 */

      // Special case for alpha = 0
      if (alpha == ZERO) {
        if (beta == ZERO) {
          for(size_type i=0; i<N; i++) Y[i]->putScalar(ZERO);
        } else if (beta != ONE) {
          for(size_type i=0; i<N; i++) Y[i]->scale(beta);
        }
        return;
      }
      
      const bool Y_is_overwritten = (beta == ZERO);

      // Start by importing X to Matrices[0]'s temporary
      RCP<const MV> X_colMap;
      {
        Tpetra::Details::ProfilingRegion regionImport ("Tpetra::batchedApply: Import");
        RCP<MV> X_colMapNonConst = Matrices[0]->getColumnMapMultiVector(X);

        // Import from the domain Map MV to the column Map MV.
        X_colMapNonConst->doImport(X, *importer, INSERT);
        X_colMap = rcp_const_cast<const MV>(X_colMapNonConst);
      }

      for(size_type i=0; i<N; i++) {
        RCP<const export_type> exporter = Matrices[i]->getGraph()->getExporter();

        // Temporary MV for doExport (if needed),
        RCP<MV> Y_rowMap = Matrices[i]->getRowMapMultiVector(*Y[i]);
        if (!exporter.is_null()) {
          Matrices[i]->localApply(*X_colMap, *Y_rowMap, Teuchos::NO_TRANS, alpha, beta);
          {
            Tpetra::Details::ProfilingRegion regionExport ("Tpetra::batchedApply: Export");             
            if (Y_is_overwritten) {
              Y[i]->putScalar(ZERO);
            }
            else {
              Y[i]->scale (beta);
            }
            Y[i]->doExport(*Y_rowMap, *exporter, ADD);
          }
        }
        else { // Don't do an Export: row Map and range Map are the same.
          // Check for aliasing
          if (! Y[i]->isConstantStride() || X_colMap.getRawPtr() == Y[i]) {
            Y_rowMap = Matrices[i]->getRowMapMultiVector(*Y[i], true);
            if (beta != ZERO) {
              Tpetra::deep_copy (*Y_rowMap, *Y[i]);
            }

            Matrices[i]->localApply(*X_colMap, *Y_rowMap, Teuchos::NO_TRANS, alpha, beta);
            Tpetra::deep_copy(*Y[i], *Y_rowMap);
          }
          else {
            Matrices[i]->localApply(*X_colMap, *Y[i], Teuchos::NO_TRANS, alpha, beta);
          }
        }        
      }
    } 
    else {
      /* Non-batching path */
      for(size_type i=0; i<N; i++) {
        Matrices[i]->apply(X,*Y[i],Teuchos::NO_TRANS, alpha, beta);
      }
    }
  }
  //@}

}// namespace Tpetra

#endif // TPETRA_APPLY_HELPERS_HPP

