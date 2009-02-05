// @HEADER
// ***********************************************************************
// 
//    Thyra: Interfaces and Support for Abstract Numerical Algorithms
//                 Copyright (2004) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ***********************************************************************
// @HEADER

#ifndef THYRA_PRODUCT_VECTOR_SPACE_DECL_HPP
#define THYRA_PRODUCT_VECTOR_SPACE_DECL_HPP

#include "Thyra_ProductVectorSpaceBase.hpp"
#include "Thyra_VectorSpaceDefaultBase.hpp"
#include "Teuchos_implicit_cast.hpp"

namespace Thyra {

/** \brief Standard concrete implementation of a product vector space.
 *
 * This subclass allows <tt>%VectorSpaceBase</tt> objects to be built out
 * of one or more other vector space objects to form a product space.
 * The specific type of vector created by
 * <tt>this->createMember()</tt> is of type <tt>DefaultProductVector</tt> but
 * the client need not ever know this or deal with this type directly.
 * However, a client may want to dynamic_cast to access the
 * <tt>ProductVectorBase</tt> interface.
 *
 * To demonstrate how to use this class, suppose one has <tt>p</tt>
 * vector spaces <tt>V[k]</tt> for <tt>k = 0...p-1</tt> and one wants
 * to form a concatenated vector space <tt>Z</tt> containing all of
 * these vector spaces stacked on top of each other to form:

 \verbatim

     [ V[0]   ]
 Z = [ V[1]   ]
     [ .      ]
     [ V[p-1] ]
 \endverbatim

 * Such a vector space can be constructed out of an array of
 * <tt>p</tt> constituent <tt>VectorSpaceBase</tt> objects as shown in the
 * following function:

 \code
 void constructProductSpace(
   const Teuchos::RCP<const VectorSpaceBase<Scalar> > V[], int p
   ,Teuchos::RCP<const VectorSpaceBase<Scalar> > *Z
   )
 {
   *Z = Teuchos::rcp(new DefaultProductVectorSpace<Scalar>(V,p));
 }
 \endcode

 * Or, a product space can be constructed out of <tt>p</tt>
 * copies of the same <tt>VectorSpaceBase</tt> object as follows:

 \code
 void constructProductSpace(
   const Teuchos::RCP<const VectorSpaceBase<Scalar> > V, int p
   ,Teuchos::RCP<const VectorSpaceBase<Scalar> > *Z
   )
 {
   Teuchos::Array<Teuchos::RCP<const VectorSpaceBase<Scalar> > > vecSpaces(p);
   for( int k = 0; k < p; ++k ) vecSpaces[k] = V;
   *Z = Teuchos::rcp(new DefaultProductVectorSpace<Scalar>(&vecSpaces[0],p);
 }
 \endcode

 * Once a <tt>%DefaultProductVectorSpace</tt> object is initialized, it can
 * be used just like any other <tt>%VectorSpaceBase</tt> object.  The
 * method <tt>createMember()</tt> will create <tt>DefaultProductVector</tt>
 * objects containing vector members from the constituent vector
 * spaces.  The method <tt>createMembers()</tt> will create
 * <tt>ProductMultiVector</tt> objects containing multi-vector members
 * from the constituent vector spaces.
 *
 * There are several methods that can be used by clients that need to
 * work with the individual constituent vector spaces.  The method
 * <tt>numBlocks()</tt> give the number of constituent vector spaces
 * while <tt>vecSpaces()</tt> returns a pointer to a copy of the array
 * of the constituent vector spaces passed to <tt>initialize()</tt>.
 * Some other useful utility methods are also defined.  The method
 * <tt>vecSpacesOffsets()</tt> returns a pointer to an array that
 * gives the offset of each constituent vector in the overall
 * composite product vector.  For example, the (zero-based)
 * <tt>kth</tt> vector space <tt>this->%vecSpaces()[k]</tt> owns the
 * element indexes <tt>this->%vecSpacesOffsets()[k]</tt> to
 * <tt>this->%vecSpacesOffsets()[k+1]-1</tt>.  Determining which
 * constituent vector space owns a particular element index can be
 * found out by calling <tt>getVecSpcPoss()</tt>.
 *
 * The default assignment operator is allowed since it has the correct
 * semantics for shallow copy.  The default copy constructor is also
 * allowed but only performs a shallow copy of the constituent vector
 * space objects.  If you want to copy the constituent vector space
 * objects also you need to use the <tt>clone()</tt> method.  The
 * default constructor is not allowed (declared private) to avoid
 * accidents.
 *
 * \ingroup Thyra_Op_Vec_ANA_Development_grp
 */
template<class Scalar>
class DefaultProductVectorSpace
  : virtual public ProductVectorSpaceBase<Scalar>
  , virtual protected VectorSpaceDefaultBase<Scalar>
{
public:

  /** @name Constructors/initializers/accessors */
  //@{

  /// Construct to an initialized state (calls <tt>initialize</tt>)
  DefaultProductVectorSpace(
    const int                                                       numBlocks
    ,const Teuchos::RCP<const VectorSpaceBase<Scalar> >     vecSpaces[]
    );
  
  /** \brief Initialize with a list of constituent vector spaces.
   *
   * @param  numBlocks [in] The number of constituent vector spaces.
   * @param  vecSpaces [in] If <tt>vecSpaces!=NULL</tt> then <tt>vecSpaces</tt>
   *                   must point to an array of length <tt>this->numBlocks</tt>
   *                   and on output <tt>vecSpace[i]</tt> will be set to <tt>this->vecSpaces()[i]</tt>
   *                   for <tt>i=0,..,this->numBlocks()-1</tt>.
   *
   * Preconditions:<ul>
   * <li> <tt>numBlocks > 0</tt>
   * <li> <tt>vecSpaces != NULL</tt>
   * <li> <tt>vecSpaces[i].get() != NULL, i=0,...,numBlocks</tt>
   * <li> The vector space create by <tt><tt>vecSpace[i]->smallVecSpcFcty()->createVecSpc(k)</tt>
   *      must be compatible.  In other words,
   *      <tt>vecSpace[i]->smallVecSpcFcty()->createVecSpc(k)->isCompatible(
   *        *vecSpace[j]->smallVecSpcFcty()->createVecSpc(k)
   *        ) == true </tt> for all <tt>i=[0,numBlocks]</tt>, <tt>j=[0,numBlocks]</tt>
   *        and valid <tt>k > 0</tt>.
   *      This is required to insure that product multi-vectors can be created
   *      with constituent multi-vector blocks that have compatible <tt>domain()</tt>
   *      vector spaces.
   * </ul>
   *
   * Postconditions:<ul>
   * <li> <tt>this->dim() == sum( vecSpaces[i]->dim(), i=0,...,numBlocks-1 )</tt>
   * <li> <tt>this->numBlocks()==numBlocks</tt>
   * <li> <tt>getBlock(i).get() == vecSpaces[i].get(), i=0,...,numBlocks-1</tt>
   * <li> <tt>vecSpaces()[i].get() == vecSpaces[i].get(), i=0,...,numBlocks-1</tt>
   * </ul>
   */
  virtual void initialize(
    const int                                                       numBlocks
    ,const Teuchos::RCP<const VectorSpaceBase<Scalar> >     vecSpaces[]
    );

  /** \brief Return if <tt>this</tt> vector space was cloned.
   *
   * If this function returns <tt>true</tt> then the client needs to be careful
   * about how the constituent vector spaces returned from <tt>uninitialize()</tt>
   * will be used.
   */
  bool hasBeenCloned() const;

  /** \brief Uninitialize.
   *
   * @param  numBlocks [out] If <tt>numBlocks!=NULL</tt> then on output <tt>*numBlocks</tt>
   *                   will be set to <tt>this->numBlocks()</tt>.
   * @param  vecSpaces [out] If <tt>vecSpaces!=NULL</tt> then <tt>vecSpaces</tt>
   *                   must point to an array of length <tt>this->numBlocks</tt>
   *                   and on output <tt>vecSpace[i]</tt> will be set to <tt>this->vecSpaces()[i]</tt>
   *                   for <tt>i=0,..,this->numBlocks()-1</tt>.
   *
   * Postconditions:<ul>
   * <li> <tt>this->numBlocks()==0</tt>
   * <li> <tt>vecSpaces()==NULL</tt>
   * </ul>
   *
   * <b>Warning!</b> If <tt>this->hasBeenCloned()==true</tt> then the client
   * had better not mess with the constituent vector spaces returned in
   * <tt>vecSpaces[]</tt> since another <tt>DefaultProductVectorSpace</tt> object is
   * still using them.
   */
  virtual void uninitialize(
    int                                                             *numBlocks  = NULL
    ,Teuchos::RCP<const VectorSpaceBase<Scalar> >           vecSpaces[] = NULL
    );

  /** \brief Returns a pointer to an array (of length <tt>this->numBlocks()</tt>)
   * to the constituent vector spaces.
   *
   * Postconditions:<ul>
   * <li> [<tt>this->numBlocks() == 0</tt>] <tt>return == NULL</tt>
   * <li> [<tt>this->numBlocks() > 0</tt>] <tt>return != NULL</tt>
   * </ul>
   */
  virtual const Teuchos::RCP<const VectorSpaceBase<Scalar> >* vecSpaces() const;

  /** \brief Returns a pointer to an array (of length <tt>this->numBlocks()+1</tt>)
   * of offset into each constituent vector space.
   *
   * Postconditions:<ul>
   * <li> [<tt>this->numBlocks() == 0</tt>] <tt>return == NULL</tt>
   * <li> [<tt>this->numBlocks() > 0</tt>] <tt>return != NULL</tt>
   * </ul>
   */
  virtual const Index* vecSpacesOffsets() const;

  /** \brief Get the position of the vector space object and its offset into
   * a composite vector that owns the <tt>ith</tt> index in the
   * composite vector.
   *
   * Preconditions:<ul>
   * <li> <tt>0 <= i < this->dim()</tt>
   * </ul>
   *
   * Postconditions:<ul>
   * <li> <tt>kth_global_offset == this->vecSpacesoffsets()[kth_vector-space]</tt>
   * <li> <tt>kth_global_offset <= i <= kth_global_offset + this->vecSpaces()[kth_vector_space]->dim() - 1</tt>
   * </ul>
   *
   * @param  i    [in] The index offset of the element to find the vector space object for.
   * @param  kth_vector_space
   *              [out] The index for <tt>this->vectorSpaces()[kth_vector_space]</tt> that owns the element <tt>i</tt>.
   * @param  kth_global_offset
   *              [out] The global offset for <tt>this->vectorSpaces()[kth_vector_space]</tt> in the composite
   *              vector.
   */
  void getVecSpcPoss( Index i, int* kth_vector_space, Index* kth_global_offset ) const;
  
  //@}

  /** @name Overridden from DefaultProductVectorSpace */
  //@{

  /** \brief . */
  int numBlocks() const;
  /** \brief . */
  Teuchos::RCP<const VectorSpaceBase<Scalar> > getBlock(const int k) const; 

  //@}

  /** @name Overridden from VectorSpaceBase */
  //@{
  /// Returns the summation of the constituent vector spaces.
  Index dim() const;
  /// Returns true only if also a product vector space and all constituent vectors are compatible
  bool isCompatible( const VectorSpaceBase<Scalar>& vecSpc ) const;
  /// Returns a <tt>DefaultProductVector</tt> object
  Teuchos::RCP< VectorBase<Scalar> > createMember() const;
  /// Returns the sum of the scalar products of the constituent vectors
  Scalar scalarProd(
    const VectorBase<Scalar>& x, const VectorBase<Scalar>& y ) const;
  /// Returns the sum of the scalar products of each of the columns of the constituent multi-vectors
  void scalarProdsImpl(
    const MultiVectorBase<Scalar>& X, const MultiVectorBase<Scalar>& Y,
    const ArrayView<Scalar> &scalarProds ) const;
  /// Returns true if all of the constituent vector spaces return true
  bool hasInCoreView(const Range1D& rng, const EViewType viewType, const EStrideType strideType) const;
  /// Returns <tt>getBlock(0)->smallVecSpcFcty()</tt>
  Teuchos::RCP< const VectorSpaceFactoryBase<Scalar> > smallVecSpcFcty() const;
  /// Returns a <tt>DefaultProductMultiVector</tt> object
  Teuchos::RCP< MultiVectorBase<Scalar> > createMembers(int numMembers) const;
  /// Clones the object as promised
  Teuchos::RCP< const VectorSpaceBase<Scalar> > clone() const;
  //@}


  /** @name Overridden from Teuchos::Describable */
  //@{
                                                
  /** \brief Prints just the name <tt>DefaultProductVectorSpace</tt> along
   * with the overall dimension and the number of blocks.
   */
  std::string description() const;

  /** \brief Prints the details about the constituent vector spaces.
   *
   * This function outputs different levels of detail based on the value passed in
   * for <tt>verbLevel</tt>:
   *
   * ToDo: Finish documentation!
   */
  void describe(
    Teuchos::FancyOStream &out,
    const Teuchos::EVerbosityLevel verbLevel
    ) const;

  //@}
  
protected:

  // ///////////////////////////////////
  // Protected member functions

  /** \brief Added to allow TSFExtended DefaultProductVectorSpace to derive
   * from this. */
  DefaultProductVectorSpace() : numBlocks_(0), dim_(0) {}

private:
 
  // ///////////////////////////////////
  // Private types

  typedef Teuchos::Array<Teuchos::RCP<const VectorSpaceBase<Scalar> > > vecSpaces_t;
  typedef Teuchos::Array<Index> vecSpacesOffsets_t;
 
  // ///////////////////////////////////
  // Private data members

  int                                        numBlocks_;
  Teuchos::RCP<vecSpaces_t>          vecSpaces_;
  Teuchos::RCP<vecSpacesOffsets_t>   vecSpacesOffsets_;
  // cached info
  Index                                      dim_;
  bool                                       isInCore_;

  // ///////////////////////////////////
  // Private member functions

  void assertInitialized() const;

};


/** \brief Nonmember constructor that takes an array of vector spaces.
 *
 * \relates DefaultProductVectorSpace
 */
template<class Scalar>
inline
Teuchos::RCP<DefaultProductVectorSpace<Scalar> >
productVectorSpace(
  const Teuchos::Array<Teuchos::RCP<const VectorSpaceBase<Scalar> > > &vecSpaces
  )
{
  return Teuchos::rcp(
    new DefaultProductVectorSpace<Scalar>(vecSpaces.size(),&vecSpaces[0])
    );
}


/** \brief Nonmember constructor that duplicates a block vector space
 * <tt>numBlock</tt> times to form a product space.
 *
 * \relates DefaultProductVectorSpace
 */
template<class Scalar>
inline
Teuchos::RCP<DefaultProductVectorSpace<Scalar> >
productVectorSpace(
  const Teuchos::RCP<const VectorSpaceBase<Scalar> > &vecSpace,
  const int numBlocks
  )
{
  using Teuchos::Array;
  using Teuchos::RCP;
  Array<RCP<const VectorSpaceBase<Scalar> > > vecSpaceBlocks;
  for ( int i = 0; i < numBlocks; ++i )
    vecSpaceBlocks.push_back(vecSpace);
  return productVectorSpace(vecSpaceBlocks);
}


// /////////////////////////////////
// Inline members

template<class Scalar>
inline const Teuchos::RCP<const VectorSpaceBase<Scalar> >*
DefaultProductVectorSpace<Scalar>::vecSpaces() const
{
  return ( dim_ ? &(*vecSpaces_)[0] : NULL );
}

template<class Scalar>
inline const Index* DefaultProductVectorSpace<Scalar>::vecSpacesOffsets() const
{
  return ( dim_ ? &(*vecSpacesOffsets_)[0] : NULL );
}

template<class Scalar>
inline bool DefaultProductVectorSpace<Scalar>::hasBeenCloned() const
{
  return vecSpaces_.count() > 1;
}

template<class Scalar>
inline
void DefaultProductVectorSpace<Scalar>::assertInitialized() const
{
  using Teuchos::implicit_cast;
#ifdef TEUCHOS_DEBUG
  TEST_FOR_EXCEPT( is_null(vecSpaces_) );
#endif
}

} // namespace Thyra

#endif // THYRA_PRODUCT_VECTOR_SPACE_DECL_HPP
