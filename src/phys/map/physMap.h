/**Function*************************************************************

  Synopsis    [Physical data for the technology mapper]

  Description [Builds the per-object arrays consumed by the cut ranking
               of &phys_map (Jf_Phys_t in gia.h) from the placement and
               the timing analysis stored in Phys_Man_t.  The ranking
               compares the critical-path timing, the area, the congestion
               and the routability in that order, so the arrays carry the
               predicted arrival and required time, the RUDY congestion,
               the routability and the geometry/RC for the wire delay.
               The mapper only sees plain float arrays, so the Gia/Nf code
               does not depend on the physical-synthesis structures.

  SideEffects []

***********************************************************************/

#ifndef ABC__phys__map__physMap_h
#define ABC__phys__map__physMap_h

#include "aig/gia/gia.h"
#include "phys/physMan.h"

ABC_NAMESPACE_HEADER_START

/* Fill pPhys with the placement (coordinates, congestion) and timing
   (arrival, required, criticality, net capacitance, RC) of pMan.  The
   arrays are allocated here and released by Phys_MapFreePhys().  Returns
   1 on success, 0 if the design has not been placed. */
extern int  Phys_MapBuildPhys( Phys_Man_t * pMan, Gia_Man_t * pGia,
                               int fCong, Jf_Phys_t * pPhys );
extern void Phys_MapFreePhys( Jf_Phys_t * pPhys );

ABC_NAMESPACE_HEADER_END

#endif
