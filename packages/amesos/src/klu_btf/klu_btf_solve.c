/* ========================================================================== */
/* === klu_btf_solve ======================================================== */
/* ========================================================================== */

/* Solve Ax=b using the symbolic and numeric objects from klu_btf_analyze
 * (or klu_btf_analyze_given) and klu_btf_factor.  Note that no iterative
 * refinement is performed.  Uses Numeric->Xwork as workspace (undefined on
 * input and output), of size 4n double's (note that columns 2 to 4 of Xwork
 * overlap with Numeric->Iwork).
 *
 * TODO: add iterative refinement?
 * TODO: sparse right-hand-sides
 * TODO: add error checking of inputs (and allow it to be turned off)
 */  

#include "klu_btf_internal.h"

void klu_btf_solve
(
    /* inputs, not modified */
    klu_symbolic *Symbolic,
    klu_numeric *Numeric,
    int d,		    /* leading dimension of B */
    int nrhs,		    /* number of right-hand-sides */

    /* right-hand-side on input, overwritten with solution to Ax=b on output */
    double B [ ]	    /* size n*nrhs, in column-oriented form, with
			     * leading dimension d. */
)
{
    double x [4], offik, rs, s ;
    double *Singleton, *Offx, *Rs, *X ;
    double **Lbx, **Ubx ;
    int k1, k2, nk, k, block, pend, n, p, nblocks, chunk, nr, i, scale ;
    int *Q, *R, *Pnum, *Offp, *Offi ;
    int **Lbp, **Lbi, **Ubp, **Ubi ;

    /* ---------------------------------------------------------------------- */
    /* get the contents of the Symbolic object */
    /* ---------------------------------------------------------------------- */

    n = Symbolic->n ;
    nblocks = Symbolic->nblocks ;
    Q = Symbolic->Q ;
    R = Symbolic->R ;

    PRINTF (("klu_btf_solve:  n %d nblocks %d \n", n, nblocks)) ;

    /* ---------------------------------------------------------------------- */
    /* get the contents of the Numeric object */
    /* ---------------------------------------------------------------------- */

    ASSERT (nblocks == Numeric->nblocks) ;
    Pnum = Numeric->Pnum ;
    Offp = Numeric->Offp ;
    Offi = Numeric->Offi ;
    Offx = Numeric->Offx ;
    Singleton = Numeric->Singleton ;
    Lbp = Numeric->Lbp ;
    Lbi = Numeric->Lbi ;
    Lbx = Numeric->Lbx ;
    Ubp = Numeric->Ubp ;
    Ubi = Numeric->Ubi ;
    Ubx = Numeric->Ubx ;
    Rs = Numeric->Rs ;
    X = Numeric->Xwork ;
    ASSERT (klu_valid (n, Offp, Offi, Offx)) ;
    scale = Numeric->scale ;

    /* ---------------------------------------------------------------------- */
    /* solve in chunks of 4 columns at a time */
    /* ---------------------------------------------------------------------- */

    for (chunk = 0 ; chunk < nrhs ; chunk += 4)
    {

	/* ------------------------------------------------------------------ */
	/* get the size of the current chunk */
	/* ------------------------------------------------------------------ */

	nr = MIN (nrhs - chunk, 4) ;

	/* ------------------------------------------------------------------ */
	/* scale and permute the right hand side, X = P*(R\B) */
	/* ------------------------------------------------------------------ */

	if (scale == 0)
	{

	    /* no scaling */
	    switch (nr)
	    {

	    case 1:

		for (k = 0 ; k < n ; k++)
		{
		    X [k] = B [Pnum [k]] ;
		}
		break ;

	    case 2:

		for (k = 0 ; k < n ; k++)
		{
		    i = Pnum [k] ;
		    X [2*k    ] = B [i      ] ;
		    X [2*k + 1] = B [i + d  ] ;
		}
		break ;

	    case 3:

		for (k = 0 ; k < n ; k++)
		{
		    i = Pnum [k] ;
		    X [3*k    ] = B [i      ] ;
		    X [3*k + 1] = B [i + d  ] ;
		    X [3*k + 2] = B [i + d*2] ;
		}
		break ;

	    case 4:

		for (k = 0 ; k < n ; k++)
		{
		    i = Pnum [k] ;
		    X [4*k    ] = B [i      ] ;
		    X [4*k + 1] = B [i + d  ] ;
		    X [4*k + 2] = B [i + d*2] ;
		    X [4*k + 3] = B [i + d*3] ;
		}
		break ;
	    }

	}
	else
	{

	    switch (nr)
	    {

	    case 1:

		for (k = 0 ; k < n ; k++)
		{
#ifndef NRECIPROCAL
		    X [k] = B [Pnum [k]] * Rs [k] ;
#else
		    X [k] = B [Pnum [k]] / Rs [k] ;
#endif
		}
		break ;

	    case 2:

		for (k = 0 ; k < n ; k++)
		{
		    i = Pnum [k] ;
		    rs = Rs [k] ;
#ifndef NRECIPROCAL
		    X [2*k    ] = B [i      ] * rs ;
		    X [2*k + 1] = B [i + d  ] * rs ;
#else
		    X [2*k    ] = B [i      ] / rs ;
		    X [2*k + 1] = B [i + d  ] / rs ;
#endif
		}
		break ;

	    case 3:

		for (k = 0 ; k < n ; k++)
		{
		    i = Pnum [k] ;
		    rs = Rs [k] ;
#ifndef NRECIPROCAL
		    X [3*k    ] = B [i      ] * rs ;
		    X [3*k + 1] = B [i + d  ] * rs ;
		    X [3*k + 2] = B [i + d*2] * rs ;
#else
		    X [3*k    ] = B [i      ] / rs ;
		    X [3*k + 1] = B [i + d  ] / rs ;
		    X [3*k + 2] = B [i + d*2] / rs ;
#endif
		}
		break ;

	    case 4:

		for (k = 0 ; k < n ; k++)
		{
		    i = Pnum [k] ;
		    rs = Rs [k] ;
#ifndef NRECIPROCAL
		    X [4*k    ] = B [i      ] * rs ;
		    X [4*k + 1] = B [i + d  ] * rs ;
		    X [4*k + 2] = B [i + d*2] * rs ;
		    X [4*k + 3] = B [i + d*3] * rs ;
#else
		    X [4*k    ] = B [i      ] / rs ;
		    X [4*k + 1] = B [i + d  ] / rs ;
		    X [4*k + 2] = B [i + d*2] / rs ;
		    X [4*k + 3] = B [i + d*3] / rs ;
#endif
		}
		break ;
	    }
	}

	/* ------------------------------------------------------------------ */
	/* solve X = (L*U + Off)\X */
	/* ------------------------------------------------------------------ */

	for (block = nblocks-1 ; block >= 0 ; block--)
	{

	    /* -------------------------------------------------------------- */
	    /* the block of size nk is from rows/columns k1 to k2-1 */
	    /* -------------------------------------------------------------- */

	    k1 = R [block] ;
	    k2 = R [block+1] ;
	    nk = k2 - k1 ;
	    PRINTF (("solve %d, k1 %d k2-1 %d nk %d\n", block, k1,k2-1,nk)) ;

	    /* solve the block system */
	    if (nk == 1)
	    {
		s = Singleton [block] ;
		switch (nr)
		{

		case 1:
#ifndef NRECIPROCAL
		    X [  k1    ] *= s ;
#else
		    X [  k1    ] /= s ;
#endif
		    break ;

		case 2:
#ifndef NRECIPROCAL
		    X [2*k1    ] *= s ;
		    X [2*k1 + 1] *= s ;
#else
		    X [2*k1    ] /= s ;
		    X [2*k1 + 1] /= s ;
#endif
		    break ;

		case 3: 
#ifndef NRECIPROCAL
		    X [3*k1    ] *= s ;
		    X [3*k1 + 1] *= s ;
		    X [3*k1 + 2] *= s ;
#else
		    X [3*k1    ] /= s ;
		    X [3*k1 + 1] /= s ;
		    X [3*k1 + 2] /= s ;
#endif
		    break ;

		case 4:
#ifndef NRECIPROCAL
		    X [4*k1    ] *= s ;
		    X [4*k1 + 1] *= s ;
		    X [4*k1 + 2] *= s ;
		    X [4*k1 + 3] *= s ;
#else
		    X [4*k1    ] /= s ;
		    X [4*k1 + 1] /= s ;
		    X [4*k1 + 2] /= s ;
		    X [4*k1 + 3] /= s ;
#endif
		    break ;

		}
	    }
	    else
	    {
		klu_lsolve (nk, Lbp [block], Lbi [block], Lbx [block], nr,
			X + nr*k1) ;
		klu_usolve (nk, Ubp [block], Ubi [block], Ubx [block], nr,
			X + nr*k1) ;
	    }

	    /* -------------------------------------------------------------- */
	    /* block back-substitution for the off-diagonal-block entries */
	    /* -------------------------------------------------------------- */

	    if (block > 0)
	    {
		switch (nr)
		{

		case 1:

		    for (k = k1 ; k < k2 ; k++)
		    {
			pend = Offp [k+1] ;
			x [0] = X [k] ;
			for (p = Offp [k] ; p < pend ; p++)
			{
			    X [Offi [p]] -= Offx [p] * x [0] ;
			}
		    }
		    break ;

		case 2:

		    for (k = k1 ; k < k2 ; k++)
		    {
			pend = Offp [k+1] ;
			x [0] = X [2*k    ] ;
			x [1] = X [2*k + 1] ;
			for (p = Offp [k] ; p < pend ; p++)
			{
			    i = Offi [p] ;
			    offik = Offx [p] ;
			    X [2*i    ] -= offik * x [0] ;
			    X [2*i + 1] -= offik * x [1] ;
			}
		    }
		    break ;

		case 3:

		    for (k = k1 ; k < k2 ; k++)
		    {
			pend = Offp [k+1] ;
			x [0] = X [3*k    ] ;
			x [1] = X [3*k + 1] ;
			x [2] = X [3*k + 2] ;
			for (p = Offp [k] ; p < pend ; p++)
			{
			    i = Offi [p] ;
			    offik = Offx [p] ;
			    X [3*i    ] -= offik * x [0] ;
			    X [3*i + 1] -= offik * x [1] ;
			    X [3*i + 2] -= offik * x [2] ;
			}
		    }
		    break ;

		case 4:

		    for (k = k1 ; k < k2 ; k++)
		    {
			pend = Offp [k+1] ;
			x [0] = X [4*k    ] ;
			x [1] = X [4*k + 1] ;
			x [2] = X [4*k + 2] ;
			x [3] = X [4*k + 3] ;
			for (p = Offp [k] ; p < pend ; p++)
			{
			    i = Offi [p] ;
			    offik = Offx [p] ;
			    X [4*i    ] -= offik * x [0] ;
			    X [4*i + 1] -= offik * x [1] ;
			    X [4*i + 2] -= offik * x [2] ;
			    X [4*i + 3] -= offik * x [3] ;
			}
		    }
		    break ;
		}
	    }
	}

	/* ------------------------------------------------------------------ */
	/* permute the result, B = Q*X */
	/* ------------------------------------------------------------------ */

	switch (nr)
	{

	case 1:

	    for (k = 0 ; k < n ; k++)
	    {
		B [Q [k]] = X [k] ;
	    }
	    break ;

	case 2:

	    for (k = 0 ; k < n ; k++)
	    {
		i = Q [k] ;
		B [i      ] = X [2*k    ] ;
		B [i + d  ] = X [2*k + 1] ;
	    }
	    break ;

	case 3:

	    for (k = 0 ; k < n ; k++)
	    {
		i = Q [k] ;
		B [i      ] = X [3*k    ] ;
		B [i + d  ] = X [3*k + 1] ;
		B [i + d*2] = X [3*k + 2] ;
	    }
	    break ;

	case 4:

	    for (k = 0 ; k < n ; k++)
	    {
		i = Q [k] ;
		B [i      ] = X [4*k    ] ;
		B [i + d  ] = X [4*k + 1] ;
		B [i + d*2] = X [4*k + 2] ;
		B [i + d*3] = X [4*k + 3] ;
	    }
	    break ;
	}

	/* ------------------------------------------------------------------ */
	/* go to the next chunk of B */
	/* ------------------------------------------------------------------ */

	B += d*4 ;
    }
}
