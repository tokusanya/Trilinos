/*!
   \mainpage %Belos: A Block Linear Solvers Package

   \section intro Introduction

   %Belos is an extensible and interoperable framework for large-scale, iterative methods for
solving systems of linear equations with multiple right-hand sides.  The 
motivation for this framework is to provide a generic 
interface to a collection of algorithms for solving large-scale linear systems.
Belos is interoperable because both the matrix and vectors are considered to be opaque objects---only 
knowledge of the matrix and vectors via elementary operations is necessary. An implementation of Belos
is accomplished via the use of interfaces. Current interfaces available include
Epetra and so any libraries that understand Epetra matrices and vectors (such
as AztecOO) may also be used in conjunction with Belos.

One of the goals of Belos is to allow the user the flexibility to specify the
data representation for the matrix and vectors and so leverage any existing software
investment.  The algorithms that are currently available through Belos are Block GMRES (Generalized 
Minimal RESidual) and Block CG (Conjugate-Gradient). 

   \section contributors Belos Contributors

   The following people have contributed to the development of %Belos:

   <ul>
	<li> Teri Barth, Sandia National Labs, tlbarth@sandia.gov
 	<li> Rich Lehoucq, Sandia National Labs, rblehou@sandia.gov
	<li> Heidi Thornquist, Sandia National Labs, hkthorn@sandia.gov
   </ul>

*/
