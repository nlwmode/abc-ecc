/**Function*************************************************************

  Synopsis    [Analytical placement for the physical-synthesis flow]

  Description [A RePlAce/DREAMPlace-style analytical placer (CPU only):
                 1) build the netlist of mapped cells, PIs, and POs
                 2) minimize quadratic wirelength (clique net model)
                    with a Jacobi-preconditioned conjugate gradient and
                    equalize the cell density with a monotone warp
                 3) run nonlinear global placement: log-sum-exp
                    wirelength with an annealed smoothing parameter plus
                    the electrostatic density penalty, whose field comes
                    from a Poisson solve in the DCT domain over a bin
                    grid.  The iterations use Nesterov acceleration with
                    a Barzilai-Borwein step size, and the density weight
                    grows whenever the density overflow stops improving
                 4) optionally legalize the cells into standard-cell
                    rows (-l); by default the spread solution is kept,
                    so the cells may overlap and do not sit on sites
               The terminals are boundary conditions of the placement,
               and object order says nothing about connectivity, so by
               default the placement is solved first, every terminal is
               then moved to the side and to the slot where its
               connected cells are, and the placement is solved again
               with the new pins.  nPinMode = 0 keeps the object order
               on the left and right edges (one pass), 1 orders the two
               sides only, and 3 solves the two passes without moving
               the terminals, which is the control for 1 and 2.
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
    float Util;         /* target core utilization                 */
    float Aspect;       /* core height / width                     */
    int   nIterCG;      /* conjugate-gradient iterations           */
    int   nIterNest;    /* nonlinear placement iterations          */
    float StopOverflow; /* density overflow at which to stop       */
    int   nCongBins;    /* RUDY bins along the longer core side    */
    int   nPinMode;     /* 0 = object order, 1 = sorted on two sides,
                           2 = sorted and assigned to all four sides */
    float IoPitch;      /* minimum IO pin pitch in um, 0 = no limit */
    int   fLegalize;    /* 1 = legalize into rows, 0 = keep the spread
                           placement, whose cells may overlap        */
    int   nChoiceMode;  /* 0 = every alternative of a choice class is a
                           cell of the placement (the choice-blind
                           baseline), 1 = block coordinate descent over
                           the selection ("-c"): only the selected
                           alternative of every class is placed
                           globally, and every alternative is evaluated
                           at its class output anchor between the
                           rounds                                   */
    int   nChoiceRounds;/* selection rounds of the descent ("-r")     */
    float ChoiceTol;    /* relative cost gain that makes a new
                           alternative worth migrating to            */
    int   fVerbose;
} Phys_PlacePars_t;

extern void Phys_PlaceSetDefault( Phys_PlacePars_t * p );
extern void Phys_PlaceCellSizeObj( Phys_Man_t * p, int iObj, int * pw, int * ph );
extern void Phys_PlaceCellSizeCell( Phys_Man_t * p, Phys_LibCell_t * pCell, int * pw, int * ph );
extern int  Phys_PlaceRun( Phys_Man_t * p, Gia_Man_t * pGia, Phys_PlacePars_t * pPars );
extern void Phys_PlacePrint( Phys_Man_t * p );

ABC_NAMESPACE_HEADER_END

#endif
