// @HEADER
// ****************************************************************************
//                Tempus: Copyright (2017) Sandia Corporation
//
// Distributed under BSD 3-clause license (See accompanying file Copyright.txt)
// ****************************************************************************
// @HEADER

#ifndef TEMPUS_TEST_VANDERPOL_IMEX_EXPLICITMODEL_DECL_HPP
#define TEMPUS_TEST_VANDERPOL_IMEX_EXPLICITMODEL_DECL_HPP

#include "Thyra_ModelEvaluator.hpp" // Interface
#include "Thyra_StateFuncModelEvaluatorBase.hpp" // Implementation

#include "Teuchos_ParameterListAcceptorDefaultBase.hpp"
#include "Teuchos_ParameterList.hpp"

namespace Tempus_Test {

/** \brief van der Pol model formulated for IMEX.
 *
 *  This is a canonical equation of a nonlinear oscillator (Hairer, Norsett,
 *  and Wanner, pp. 111-115, and Hairer and Wanner, pp. 4-5) for an electrical
 *  circuit.  In implicit ODE form, \f$ \mathcal{F}(\dot{x},x,t) = 0 \f$,
 *  the scaled problem can be written as
 *  \f{eqnarray*}{
 *    \dot{x}_0(t) - x_1(t) & = & 0 \\
 *    \dot{x}_1(t) - [(1-x_0^2)x_1-x_0]/\epsilon & = & 0
 *  \f}
 *  where the initial conditions are
 *  \f{eqnarray*}{
 *    x_0(t_0=0) & = & 2 \\
 *    x_1(t_0=0) & = & 0
 *  \f}
 *  and the initial time derivatives are
 *  \f{eqnarray*}{
 *    \dot{x}_0(t_0=0) & = & x_1(t_0=0) = 0 \\
 *    \dot{x}_1(t_0=0) & = & [(1-x_0^2)x_1-x_0]/\epsilon = -2/\epsilon
 *  \f}
 *  For an IMEX time stepper, we need to rewrite this in the following form
 *  \f{eqnarray*}{
 *    M(x,t)\, \dot{x}(x,t) + G(x,t) + F(x,t) & = & 0, \\
 *    \mathcal{G}(\dot{x},x,t) + F(x,t) & = & 0,
 *  \f}
 *  where \f$\mathcal{G}(\dot{x},x,t) = M(x,t)\, \dot{x} + G(x,t)\f$,
 *  \f$M(x,t)\f$ is the mass matrix, \f$F(x,t)\f$ is the operator
 *  representing the "slow" physics (and evolved explicitly), and
 *  \f$G(x,t)\f$ is the operator representing the "fast" physics.
 *  For the van der Pol problem, we can separate the terms as follows
 *  \f[
 *    x      = \left[\begin{array}{c} x_0 \\ x_1 \end{array}\right],\;
 *    F(x,t) = \left[\begin{array}{c} -x_1 \\ x_0/\epsilon\end{array}\right],
 *    \mbox{ and }
 *    G(x,t) = \left[\begin{array}{c} 0 \\
 *                   -(1-x_0^2)x_1/\epsilon \end{array}\right]
 *  \f]
 *  where \f$M(x,t)=I\f$ is the identity matrix.
 *
 *  Thus the explicit van der Pol model (VanDerPol_IMEX_ExplicitModel)
 *  formulated for IMEX is
 *  \f{eqnarray*}{
 *    \mathcal{F}_0 & = & \dot{x}_0(t) - x_1(t) = 0 \\
 *    \mathcal{F}_1 & = & \dot{x}_1(t) + x_0/\epsilon = 0
 *  \f}
 *  and the implicit van der Pol model (VanDerPol_IMEX_ImplicitModel)
 *  formulated for IMEX is
 *  \f{eqnarray*}{
 *    \mathcal{G}_0 & = & \dot{x}_0(t) = 0 \\
 *    \mathcal{G}_1 & = & \dot{x}_1(t) - (1-x_0^2)x_1/\epsilon = 0
 *  \f}
 *
 *  Recalling the defintion of the iteration matrix, \f$W\f$,
 *  \f[
 *    W_{ij} \equiv \frac{d\mathcal{F}_i}{dx_j} =
 *      \alpha \frac{\partial\mathcal{F}_i}{\partial \dot{x}_j}
 *    + \beta \frac{\partial\mathcal{F}_i}{\partial x_j}
 *  \f]
 *  where
 *  \f[
 *    \alpha = \left\{
 *      \begin{array}{cl}
 *        \frac{\partial\dot{x}_i}{\partial x_j} & \mbox{ if } i = j \\
 *        0 & \mbox{ if } i \neq j
 *      \end{array} \right.
 *    \;\;\;\; \mbox{ and } \;\;\;\;
 *    \beta = 1
 *  \f]
 *  we can write for the explicit van der Pol model
 *  (VanDerPol_IMEX_ExplicitModel)
 *  \f{eqnarray*}{
 *    W_{00} = \alpha \frac{\partial\mathcal{F}_0}{\partial \dot{x}_0}
 *            + \beta \frac{\partial\mathcal{F}_0}{\partial x_0}
 *         & = & \alpha \\
 *    W_{01} = \alpha \frac{\partial\mathcal{F}_0}{\partial \dot{x}_1}
 *            + \beta \frac{\partial\mathcal{F}_0}{\partial x_1}
 *         & = & -\beta \\
 *    W_{10} = \alpha \frac{\partial\mathcal{F}_1}{\partial \dot{x}_0}
 *            + \beta \frac{\partial\mathcal{F}_1}{\partial x_0}
 *         & = & 1/\epsilon \\
 *    W_{11} = \alpha \frac{\partial\mathcal{F}_1}{\partial \dot{x}_1}
 *            + \beta \frac{\partial\mathcal{F}_1}{\partial x_1}
 *         & = & \alpha \\
 *  \f}
 */

template<class Scalar>
class VanDerPol_IMEX_ExplicitModel
  : public Thyra::StateFuncModelEvaluatorBase<Scalar>,
    public Teuchos::ParameterListAcceptorDefaultBase
{
  public:

  // Constructor
  VanDerPol_IMEX_ExplicitModel(
    Teuchos::RCP<Teuchos::ParameterList> pList = Teuchos::null);

  /** \name Public functions overridden from ModelEvaluator. */
  //@{
  Teuchos::RCP<const Thyra::VectorSpaceBase<Scalar> > get_x_space() const;
  Teuchos::RCP<const Thyra::VectorSpaceBase<Scalar> > get_f_space() const;
  Thyra::ModelEvaluatorBase::InArgs<Scalar> getNominalValues() const;
  Thyra::ModelEvaluatorBase::InArgs<Scalar> createInArgs() const;

  Teuchos::RCP<const Thyra::VectorSpaceBase<Scalar> > get_p_space(int l) const;
  Teuchos::RCP<const Teuchos::Array<std::string> > get_p_names(int l) const;
  Teuchos::RCP<const Thyra::VectorSpaceBase<Scalar> > get_g_space(int j) const;
  //@}

  /** \name Public functions overridden from ParameterListAcceptor. */
  //@{
  void setParameterList(Teuchos::RCP<Teuchos::ParameterList> const& paramList);
  Teuchos::RCP<const Teuchos::ParameterList> getValidParameters() const;
  //@}

private:

  void setupInOutArgs_() const;

  /** \name Private functions overridden from ModelEvaluatorDefaultBase. */
  //@{
  Thyra::ModelEvaluatorBase::OutArgs<Scalar> createOutArgsImpl() const;
  void evalModelImpl(
    const Thyra::ModelEvaluatorBase::InArgs<Scalar> &inArgs_bar,
    const Thyra::ModelEvaluatorBase::OutArgs<Scalar> &outArgs_bar
    ) const;
  //@}

  int dim_;         ///< Number of state unknowns (2)
  int Np_;          ///< Number of parameter vectors (1)
  int np_;          ///< Number of parameters in this vector (1)
  int Ng_;          ///< Number of observation functions (0)
  int ng_;          ///< Number of elements in this observation function (0)
  bool haveIC_;     ///< false => no nominal values are provided (default=true)
  bool acceptModelParams_; ///< Changes inArgs to require parameters
  mutable bool isInitialized_;
  mutable Thyra::ModelEvaluatorBase::InArgs<Scalar>  inArgs_;
  mutable Thyra::ModelEvaluatorBase::OutArgs<Scalar> outArgs_;
  mutable Thyra::ModelEvaluatorBase::InArgs<Scalar>  nominalValues_;
  Teuchos::RCP<const Thyra::VectorSpaceBase<Scalar> > x_space_;
  Teuchos::RCP<const Thyra::VectorSpaceBase<Scalar> > f_space_;
  Teuchos::RCP<const Thyra::VectorSpaceBase<Scalar> > p_space_;
  Teuchos::RCP<const Thyra::VectorSpaceBase<Scalar> > g_space_;

  // Parameters for the model:
  Scalar epsilon_; ///< This is a model parameter
  Scalar t0_ic_;   ///< initial time
  Scalar x0_ic_;   ///< initial condition for x0
  Scalar x1_ic_;   ///< initial condition for x1
};


} // namespace Tempus_Test
#endif // TEMPUS_TEST_VANDERPOL_IMEX_EXPLICITMODEL_DECL_HPP
