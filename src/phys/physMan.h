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

#include "phys/phys.h"
#include "phys/lib/physLiberty.h"

ABC_NAMESPACE_HEADER_START

typedef struct Phys_Man_t_
{
    Phys_LefTech_t * pLef;         /* borrowed from the ABC frame          */
    Phys_Lib_t *     pLib;         /* borrowed from the ABC frame          */
    Vec_Ptr_t *      vObjToCell;   /* per Gia object: Phys_LibCell_t *     */
    Vec_Ptr_t *      vObjToPins;   /* per Gia object: Vec_Ptr_t * of
                                      Phys_LibPin_t * in fanin order       */
    Vec_Int_t *      vObjCellLit;  /* per Gia object: mapping literal of
                                      the mapped cell, -1 if not a cell   */
    int              nObjs;        /* size of the per-object vectors       */
    int              fVerbose;
    /* placement (all coordinates are centers, in DBU) */
    Vec_Int_t *      vObjX;
    Vec_Int_t *      vObjY;
    int              SiteW, SiteH; /* standard-cell site, DBU              */
    int              CoreX0, CoreY0, CoreW, CoreH;
    float            Hpwl;         /* total wirelength of the placement, um */
    int              fPlaced;
    /* timing (ps); populated by &phys_sta */
    Vec_Flt_t *      vObjArrRise;
    Vec_Flt_t *      vObjArrFall;
    Vec_Flt_t *      vObjSlewRise;
    Vec_Flt_t *      vObjSlewFall;
    Vec_Flt_t *      vObjSlack;    /* PO endpoints only, -1 elsewhere     */
    float            Period;
    float            TMax;
    int              fTimed;
} Phys_Man_t;

extern Phys_Man_t * Phys_ManStart( Phys_LefTech_t * pLef, Phys_Lib_t * pLib, int fVerbose );
extern void         Phys_ManStop( Phys_Man_t ** pp );
extern void         Phys_ManResize( Phys_Man_t * p, int nObjs );
extern void         Phys_ManClearPlace( Phys_Man_t * p );
extern void         Phys_ManClearTiming( Phys_Man_t * p );

ABC_NAMESPACE_HEADER_END

#endif
