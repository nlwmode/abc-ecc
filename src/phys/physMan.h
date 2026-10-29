/**Function*************************************************************

  Synopsis    [Physical-synthesis session manager]

  Description [Holds the data shared by the commands that are valid
               between &phys_start and &phys_end.  The manager keeps
               only pointers to the LEF/Liberty databases owned by the
               ABC frame and per-object bindings produced by &phys_bind.
               It does not depend on the Gia data structures; the
               command layer fills the vectors using the Gia APIs.]

  SideEffects []

***********************************************************************/

#ifndef ABC__phys__physMan_h
#define ABC__phys__physMan_h

#include "aig/gia/gia.h"
#include "phys/phys.h"
#include "phys/lib/physLiberty.h"

ABC_NAMESPACE_HEADER_START

typedef struct Phys_Cong_t_ Phys_Cong_t;

typedef struct Phys_Man_t_
{
    Phys_LefTech_t * pLef;         /* borrowed from the ABC frame          */
    Phys_Lib_t *     pLib;         /* borrowed from the ABC frame          */
    Vec_Ptr_t *      vObjToCell;   /* per Gia object: Phys_LibCell_t *     */
    Vec_Ptr_t *      vObjToPins;   /* per Gia object: Vec_Ptr_t * of
                                      Phys_LibPin_t * in fanin order       */
    Vec_Int_t *      vObjCellLit;  /* per Gia object: mapping literal of
                                      the mapped cell; negative when the
                                      node is not a cell or is a virtual
                                      (pre-mapping) cell, whose fanins are
                                      then the Gia fanins                  */
    /* Cell instances, one per driven output literal.  A Gia object can
       drive both of its literals (a gate plus an output inverter, or two
       gates for the two phases), so the placer and the timing analysis
       work on this literal-indexed view, not on the objects.  The pins
       are owned here; vObjToPins only borrows them. */
    Vec_Ptr_t *      vLitToCell;   /* per literal: Phys_LibCell_t *        */
    Vec_Ptr_t *      vLitToPins;   /* per literal: Vec_Ptr_t * of pins     */
    Vec_Int_t *      vLitX;        /* per literal: placed center X, DBU    */
    Vec_Int_t *      vLitY;        /* per literal: placed center Y, DBU    */
    int              nLits;        /* size of the literal vectors          */
    int              fLitMapped;   /* 1 = real mapping, 0 = virtual cells  */
    int              nObjs;        /* size of the per-object vectors       */
    int              fVerbose;
    /* placement (all coordinates are centers, in DBU) */
    Vec_Int_t *      vObjX;
    Vec_Int_t *      vObjY;
    int              SiteW, SiteH; /* standard-cell site, DBU              */
    int              CoreX0, CoreY0, CoreW, CoreH;
    float            Hpwl;         /* total wirelength of the placement, um */
    int              fPlaced;
    /* congestion / routability, built by &phys_place from the placement */
    Phys_Cong_t *    pCong;        /* owned; freed by Phys_ManClearCong    */
    Vec_Flt_t *      vObjCong;     /* per object: bin demand / supply      */
    Vec_Flt_t *      vObjRout;     /* per object: min(1, supply/demand)    */
    Vec_Flt_t *      vObjDens;     /* per object: cell density of the bin  */
    /* timing (ps); populated by &phys_sta */
    Vec_Flt_t *      vObjArrRise;
    Vec_Flt_t *      vObjArrFall;
    Vec_Flt_t *      vObjSlewRise;
    Vec_Flt_t *      vObjSlewFall;
    Vec_Flt_t *      vObjSlack;    /* PO endpoints only, -1 elsewhere     */
    Vec_Flt_t *      vObjReq;      /* per object: required arrival, ps     */
    Vec_Flt_t *      vObjCrit;     /* per object: criticality in [0,1]     */
    Vec_Flt_t *      vNetCap;      /* per driver: total net cap, pF        */
    Vec_Flt_t *      vNetDelay;    /* per driver: max sink Elmore, ps      */
    Vec_Flt_t *      vNetLen;      /* per driver: total wire length, um    */
    Vec_Int_t *      vPinOff;      /* per object: offset into pin arrays   */
    Vec_Flt_t *      vPinWireDelay;/* per input pin: Elmore delay, ps      */
    Vec_Flt_t *      vPinGateDelay;/* per input pin: arc delay, ps         */
    Vec_Flt_t *      vObjInWire;   /* per object: wire delay into POs, ps  */
    Vec_Int_t *      vPathObjs;    /* critical path: objects, input -> output */
    Vec_Int_t *      vObjPath;     /* per object: 1 if on the critical path   */
    float            PathDelay;    /* arrival at the critical endpoint, ps    */
    int              PathEnd;      /* critical endpoint (PO object), -1 if none */
    float            Period;
    float            TMax;
    float            RcR, RcC;     /* unit RC used by the last STA run     */
    float            Dbu;          /* LEF database units per micron        */
    float            InputSlew;    /* input slew used by the last STA run  */
    int              fTimed;
} Phys_Man_t;

extern Phys_Man_t * Phys_ManStart( Phys_LefTech_t * pLef, Phys_Lib_t * pLib, int fVerbose );
extern void         Phys_ManStop( Phys_Man_t ** pp );
extern void         Phys_ManResize( Phys_Man_t * p, int nObjs );
extern void         Phys_ManClearPlace( Phys_Man_t * p );
extern void         Phys_ManClearCong( Phys_Man_t * p );
extern void         Phys_ManClearTiming( Phys_Man_t * p );

/* Fanins of a bound object.  For a mapped cell they come from the cell
   mapping; for a virtual cell bound before technology mapping they are the
   fanins of the AIG node.  The literal keeps the complement bit, as the
   Gia cell fanins do. */
extern int          Phys_ManObjFaninNum( Phys_Man_t * p, Gia_Man_t * pGia, int iObj );
extern int          Phys_ManObjFaninLit( Phys_Man_t * p, Gia_Man_t * pGia, int iObj, int k );

/* Cell instances keyed by the output literal.  An inverter or a second
   phase cell attached to the same Gia object has its own instance, so the
   placer and the timing analysis see the netlist the mapper produced.
   The fanin accessors return the driver literal of every input: for a
   real mapping the literal of the cell record, for a virtual pre-mapping
   cell the positive literal of the AIG fanin. */
extern void         Phys_ManLitResize( Phys_Man_t * p, int nLits, int fMapped );
extern int          Phys_ManLitNum( Phys_Man_t * p );
extern int          Phys_ManLitIsCell( Phys_Man_t * p, int iLit );
extern Phys_LibCell_t * Phys_ManLitCell( Phys_Man_t * p, int iLit );
extern Vec_Ptr_t *  Phys_ManLitPins( Phys_Man_t * p, int iLit );
/* Pin geometry from the LEF macro of the cell instance.  The pin index is
   the input index in fanin order, or negative for the output pin.  The
   offset is from the cell center in DBU and points at the middle of the
   largest metal rectangle of the pin, so it stays on the pin's metal even
   for staircase-shaped ports; it is zero when the macro or the pin is
   missing, so the cell center remains the fallback anchor. */
extern Phys_LefMacro_t * Phys_ManLitMacro( Phys_Man_t * p, int iLit );
extern Phys_LefPin_t *   Phys_ManLitLefPin( Phys_Man_t * p, int iLit, int k );
extern int          Phys_ManLitPinOffset( Phys_Man_t * p, int iLit, int k, int * pDx, int * pDy );
extern int          Phys_ManLitFaninNum( Phys_Man_t * p, Gia_Man_t * pGia, int iLit );
extern int          Phys_ManLitFaninLit( Phys_Man_t * p, Gia_Man_t * pGia, int iLit, int k );
/* literal of the instance that drives an arbitrary signal literal */
extern int          Phys_ManDriverLit( Phys_Man_t * p, Gia_Man_t * pGia, int iLit );

ABC_NAMESPACE_HEADER_END

#endif
