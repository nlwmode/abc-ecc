/**Function*************************************************************

  Synopsis    [Shared routing congestion and routability model]

  Description [The placer builds the RUDY map once the cells are legal,
               stores it in Phys_Man_t, and both &phys_dump and
               &phys_map consume it.  The per-object views are what the
               mapper uses: the congestion of the bin containing a node
               and its routability (supply / demand), both in the same
               units as the RUDY demand.]

  SideEffects []

***********************************************************************/

#ifndef ABC__phys__place__physCong_h
#define ABC__phys__place__physCong_h

#include "aig/gia/gia.h"
#include "phys/physMan.h"

ABC_NAMESPACE_HEADER_START

typedef struct Phys_Cong_t_
{
    int     nX, nY;      /* number of routing bins                */
    float   X0, Y0;      /* lower-left corner of the core, um     */
    float   W, H;        /* core size, um                         */
    float   BinW, BinH;  /* bin size, um                          */
    float * pDemand;     /* per bin: RUDY wire demand, um         */
    float * pCong;       /* per bin: demand / supply              */
    float   Supply;      /* per bin: routing supply, um           */
    float   MaxCong;     /* largest congestion                    */
    float   TotalDemand; /* wire demand over the core, um         */
    float   TotalSupply; /* routing supply over the core, um      */
    int     nOverflow;   /* bins whose demand exceeds supply      */
} Phys_Cong_t;

/* Builds the RUDY map from the placement in pMan and stores it there,
   together with per-object congestion/routability/density.  Returns the
   map, or NULL when the placement has no nets or no core. */
extern Phys_Cong_t * Phys_CongBuild( Phys_Man_t * pMan, Gia_Man_t * pGia, int nGrid );
extern void          Phys_CongFree( Phys_Cong_t ** pp );
/* Routing supply of one bin, in um of wire. */
extern float         Phys_CongSupply( Phys_Man_t * pMan, float Dbu, float BinW, float BinH );

ABC_NAMESPACE_HEADER_END

#endif
