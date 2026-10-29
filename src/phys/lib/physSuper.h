/**Function*************************************************************

  Synopsis    [Library-driven supergate construction]

  Description [Before technology mapping every AIG node has to be bound
               to a cell.  The AIG node computes

                   (p0 ? !a : a) & (p1 ? !b : b) ^ q

               where (p0,p1) is the input-phase pattern and q the polarity
               of the literal the node drives - eight cases in all.  This
               module realizes all eight from the cells of a Liberty
               library: a single cell when the library has one, otherwise
               the minimum-area composition of the library's one- and
               two-input cells and inverters.  The result is called a
               supergate; it is a synthetic Phys_LibCell_t with two input
               pins, one output pin, composed area, composed pin
               capacitances and composed timing arcs, so the placer and
               the timing analysis can treat it exactly like a library
               cell even though it has no LEF macro.

               The composition is synthesized by a shortest-path (Viterbi)
               search over the sixteen two-input Boolean functions; the
               cost is area.  The delay arcs are obtained by simulating
               the internal netlist at every point of the grid of the
               terminal cell's arc, with the same rise/fall model that
               physSta.c uses.]

  SideEffects []

***********************************************************************/

#ifndef ABC__phys__lib__physSuper_h
#define ABC__phys__lib__physSuper_h

#include "phys/lib/physTypes.h"
#include "phys/lib/physLef.h"

ABC_NAMESPACE_HEADER_START

/* builds the eight supergate cells of the library, stores them in
   pLib->vSuperCells and returns the number of patterns realized; when
   the LEF is given, every supergate also gets a footprint and pin
   offsets composed from the cells of its composition */
extern int              Phys_SuperBuild( Phys_Lib_t * pLib, Phys_LefTech_t * pLef );

/* the supergate realizing input phases (p0,p1) and output phase q;
   NULL when the library cannot realize the pattern */
extern Phys_LibCell_t * Phys_SuperCell( Phys_Lib_t * pLib, int p0, int p1, int q );

/* prints the eight-pattern table: area, footprint and structure */
extern void             Phys_SuperPrint( Phys_Lib_t * pLib, Phys_LefTech_t * pLef );

/* reports the pattern index (0..7) of a supergate, -1 for a real cell */
extern int              Phys_SuperPattern( Phys_Lib_t * pLib, Phys_LibCell_t * pCell );

ABC_NAMESPACE_HEADER_END

#endif
