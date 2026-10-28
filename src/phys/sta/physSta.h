/**Function*************************************************************

  Synopsis    [Static timing analysis for the physical-synthesis flow]

  Description [NLDM-based timing analysis of the mapped Gia netlist.
               Cell delays and output slews come from the Liberty
               lookup tables.  Interconnect delays come from an Elmore
               model of an RC tree built from the placement: the pins of
               every net are joined by a rectilinear minimum spanning
               tree, the driver sees the whole tree capacitance as its
               load, and each sink gets the Elmore delay of its own path
               from the driver.  Both cell and wire timing use the
               internal units of the physical-synthesis layer (ps, pF,
               ohm).]

  SideEffects []

***********************************************************************/

#ifndef ABC__phys__sta__physSta_h
#define ABC__phys__sta__physSta_h

#include "aig/gia/gia.h"
#include "phys/physMan.h"

ABC_NAMESPACE_HEADER_START

typedef struct Phys_StaPars_t_
{
    float Period;      /* ps; <= 0.0 uses the longest path             */
    float InputSlew;   /* ps, slew at the primary inputs               */
    char * pLayer;     /* routing layer for RC, NULL/"auto" = auto     */
    float WireRes;     /* ohm/um; <= 0.0 uses the LEF layer value      */
    float WireCap;     /* pF/um; <= 0.0 uses the LEF/default value     */
    int   fVerbose;
} Phys_StaPars_t;

extern void Phys_StaSetDefault( Phys_StaPars_t * p );
extern int  Phys_StaRun( Phys_Man_t * p, Gia_Man_t * pGia, Phys_StaPars_t * pPars );
extern void Phys_StaPrint( Phys_Man_t * p );
/* Elmore delay of a point-to-point wire, in ps; uses the unit RC of the
   last STA run and the given load capacitance in pF. */
extern float Phys_StaWireDelay( Phys_Man_t * p, float x0, float y0, float x1, float y1, float Cload );

ABC_NAMESPACE_HEADER_END

#endif
