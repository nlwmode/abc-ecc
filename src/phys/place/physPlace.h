/**Function*************************************************************

  Synopsis    [Analytical placement for the physical-synthesis flow]

  Description [A DreamPlace-style quadratic placer, simplified for the
               first working version of the flow:
                 1) build the netlist of mapped cells, PIs, and POs
                 2) minimize quadratic wirelength (clique net model)
                    with a Jacobi-preconditioned conjugate gradient
                 3) spread the cells to uniform density by mapping the
                    cumulative cell width onto the core along each axis
                 4) legalize the cells into standard-cell rows
                 5) refine by alternating a quadratic solve, anchored at
                    the current placement, with legalization, keeping the
                    best wirelength seen
               The placer works on the Gia network only through the
               bindings stored in Phys_Man_t, so the cost model can be
               replaced without touching the command layer.]

  SideEffects []

***********************************************************************/

#ifndef ABC__phys__place__physPlace_h
#define ABC__phys__place__physPlace_h

#include "aig/gia/gia.h"
#include "phys/physMan.h"

ABC_NAMESPACE_HEADER_START

typedef struct Phys_PlacePars_t_
{
    float Util;        /* target core utilization                  */
    float Aspect;      /* core height / width                      */
    int   nIterCG;     /* conjugate-gradient iterations            */
    int   nIterRefine; /* wirelength-refinement rounds             */
    int   fVerbose;
} Phys_PlacePars_t;

extern void Phys_PlaceSetDefault( Phys_PlacePars_t * p );
extern void Phys_PlaceCellSizeObj( Phys_Man_t * p, int iObj, int * pw, int * ph );
extern int  Phys_PlaceRun( Phys_Man_t * p, Gia_Man_t * pGia, Phys_PlacePars_t * pPars );
extern void Phys_PlacePrint( Phys_Man_t * p );

ABC_NAMESPACE_HEADER_END

#endif
