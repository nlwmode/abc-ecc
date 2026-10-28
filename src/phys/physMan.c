/**Function*************************************************************

  Synopsis    [Physical-synthesis session manager]

  Description []

  SideEffects []

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "phys/phys.h"
#include "phys/physMan.h"
#include "phys/place/physCong.h"

ABC_NAMESPACE_IMPL_START


Phys_Man_t * Phys_ManStart( Phys_LefTech_t * pLef, Phys_Lib_t * pLib, int fVerbose )
{
    Phys_Man_t * p = ABC_CALLOC( Phys_Man_t, 1 );
    p->pLef = pLef;
    p->pLib = pLib;
    p->vObjToCell = Vec_PtrAlloc( 1024 );
    p->vObjToPins = Vec_PtrAlloc( 1024 );
    p->nObjs = 0;
    p->fVerbose = fVerbose;
    p->SiteW = 1000;
    p->SiteH = 2400;
    p->fPlaced = 0;
    p->fTimed = 0;
    return p;
}

void Phys_ManClearCong( Phys_Man_t * p )
{
    if ( p == NULL )
        return;
    Phys_CongFree( &p->pCong );
    Vec_FltFreeP( &p->vObjCong );
    Vec_FltFreeP( &p->vObjRout );
    Vec_FltFreeP( &p->vObjDens );
}

void Phys_ManClearPlace( Phys_Man_t * p )
{
    if ( p == NULL )
        return;
    Vec_IntFreeP( &p->vObjX );
    Vec_IntFreeP( &p->vObjY );
    Vec_IntFreeP( &p->vLitX );
    Vec_IntFreeP( &p->vLitY );
    Phys_ManClearCong( p );
    p->fPlaced = 0;
}

void Phys_ManClearTiming( Phys_Man_t * p )
{
    if ( p == NULL )
        return;
    Vec_FltFreeP( &p->vObjArrRise );
    Vec_FltFreeP( &p->vObjArrFall );
    Vec_FltFreeP( &p->vObjSlewRise );
    Vec_FltFreeP( &p->vObjSlewFall );
    Vec_FltFreeP( &p->vObjSlack );
    Vec_FltFreeP( &p->vObjReq );
    Vec_FltFreeP( &p->vObjCrit );
    Vec_FltFreeP( &p->vNetCap );
    Vec_FltFreeP( &p->vNetDelay );
    Vec_FltFreeP( &p->vNetLen );
    Vec_IntFreeP( &p->vPinOff );
    Vec_FltFreeP( &p->vPinWireDelay );
    Vec_FltFreeP( &p->vPinGateDelay );
    Vec_FltFreeP( &p->vObjInWire );
    Vec_IntFreeP( &p->vPathObjs );
    Vec_IntFreeP( &p->vObjPath );
    p->PathDelay = 0.0f;
    p->PathEnd = -1;
    p->Period = 0.0f;
    p->TMax = 0.0f;
    p->RcR = p->RcC = 0.0f;
    p->Dbu = 0.0f;
    p->InputSlew = 0.0f;
    p->fTimed = 0;
}

/* frees the pin vectors stored in the cells; the container itself is
   released by the caller with Vec_PtrFreeP, which also clears the field */
static void Phys_ManFreePins( Vec_Ptr_t * vCells )
{
    Vec_Ptr_t * vPins;
    int i;
    if ( vCells == NULL )
        return;
    Vec_PtrForEachEntry( Vec_Ptr_t *, vCells, vPins, i )
        if ( vPins != NULL )
            Vec_PtrFree( vPins );
}

void Phys_ManStop( Phys_Man_t ** pp )
{
    Phys_Man_t * p;
    if ( pp == NULL || *pp == NULL )
        return;
    p = *pp;
    *pp = NULL;
    Phys_ManFreePins( p->vObjToPins );
    Phys_ManFreePins( p->vLitToPins );
    Vec_PtrFreeP( &p->vObjToPins );
    Vec_PtrFreeP( &p->vLitToPins );
    Vec_PtrFreeP( &p->vObjToCell );
    Vec_PtrFreeP( &p->vLitToCell );
    Vec_IntFreeP( &p->vObjCellLit );
    Phys_ManClearPlace( p );
    Phys_ManClearTiming( p );
    ABC_FREE( p );
}

/* A bound object is either a cell of the current mapping, whose fanins are
   stored in the cell mapping, or a virtual cell bound before mapping, whose
   fanins are the fanins of the AIG node itself.  Both the placer and the
   timing analysis need the same view of the netlist, hence these accessors. */
static inline int Phys_ManObjIsVirtual( Phys_Man_t * p, int iObj )
{
    return p->vObjCellLit != NULL && iObj >= 0 && iObj < Vec_IntSize(p->vObjCellLit) &&
           Vec_IntEntry( p->vObjCellLit, iObj ) < 0;
}

int Phys_ManObjFaninNum( Phys_Man_t * p, Gia_Man_t * pGia, int iObj )
{
    Gia_Obj_t * pObj;
    int iLit;
    if ( p == NULL || pGia == NULL || p->vObjToCell == NULL ||
         iObj <= 0 || iObj >= Vec_PtrSize(p->vObjToCell) ||
         Vec_PtrEntry(p->vObjToCell, iObj) == NULL )
        return 0;
    if ( !Phys_ManObjIsVirtual(p, iObj) )
    {
        iLit = Vec_IntEntry( p->vObjCellLit, iObj );
        /* the bound literal can be an inverter (-1) or a buffer (-2), for
           which the cell mapping does not store a fanin count */
        return Phys_ManLitFaninNum( p, pGia, iLit );
    }
    pObj = Gia_ManObj( pGia, iObj );
    if ( pObj == NULL || !Gia_ObjIsAnd(pObj) )
        return 0;
    if ( Gia_ObjIsBuf(pObj) )
        return 1;
    /* a mux AIG node has a third (select) fanin; the mapper and the timing
       analysis must see it, otherwise the netlist model silently drops it */
    return Gia_ObjIsMuxId( pGia, iObj ) ? 3 : 2;
}

int Phys_ManObjFaninLit( Phys_Man_t * p, Gia_Man_t * pGia, int iObj, int k )
{
    Gia_Obj_t * pObj;
    int iLit, nFans = Phys_ManObjFaninNum( p, pGia, iObj );
    if ( k < 0 || k >= nFans )
        return 0;
    if ( !Phys_ManObjIsVirtual(p, iObj) )
    {
        iLit = Vec_IntEntry( p->vObjCellLit, iObj );
        return Phys_ManLitFaninLit( p, pGia, iLit, k );
    }
    pObj = Gia_ManObj( pGia, iObj );
    if ( k == 2 )
        return Gia_ObjFaninLit2( pGia, iObj );
    return Gia_ObjFaninLitp( pGia, pObj, k );
}

void Phys_ManResize( Phys_Man_t * p, int nObjs )
{
    if ( p == NULL )
        return;
    Phys_ManFreePins( p->vObjToPins );
    Phys_ManFreePins( p->vLitToPins );
    Vec_PtrFreeP( &p->vObjToPins );
    Vec_PtrFreeP( &p->vLitToPins );
    Vec_PtrFreeP( &p->vLitToCell );
    p->nLits = 0;
    p->fLitMapped = 0;
    if ( p->vObjToCell == NULL )
        p->vObjToCell = Vec_PtrAlloc( nObjs );
    Vec_PtrClear( p->vObjToCell );
    Vec_PtrFill( p->vObjToCell, nObjs, NULL );
    p->vObjToPins = Vec_PtrStart( nObjs );
    if ( p->vObjCellLit != NULL )
        Vec_IntFree( p->vObjCellLit );
    p->vObjCellLit = Vec_IntStartFull( nObjs );
    p->nObjs = nObjs;
    Phys_ManClearPlace( p );
    Phys_ManClearTiming( p );
}

/* Allocates the literal-indexed instance arrays.  The pin vectors are
   owned by vLitToPins; vObjToPins only borrows them, so the two vectors
   are never freed twice. */
void Phys_ManLitResize( Phys_Man_t * p, int nLits, int fMapped )
{
    if ( p == NULL )
        return;
    Phys_ManFreePins( p->vLitToPins );
    Vec_PtrFreeP( &p->vLitToCell );
    Vec_PtrFreeP( &p->vLitToPins );
    p->vLitToCell = Vec_PtrStart( nLits );
    p->vLitToPins = Vec_PtrStart( nLits );
    p->nLits = nLits;
    p->fLitMapped = fMapped;
}

int Phys_ManLitNum( Phys_Man_t * p )
{
    return p != NULL && p->vLitToCell != NULL ? Vec_PtrSize(p->vLitToCell) : 0;
}

int Phys_ManLitIsCell( Phys_Man_t * p, int iLit )
{
    return p != NULL && p->vLitToCell != NULL &&
           iLit >= 0 && iLit < Vec_PtrSize(p->vLitToCell) &&
           Vec_PtrEntry( p->vLitToCell, iLit ) != NULL;
}

Phys_LibCell_t * Phys_ManLitCell( Phys_Man_t * p, int iLit )
{
    return Phys_ManLitIsCell( p, iLit ) ?
           (Phys_LibCell_t *)Vec_PtrEntry( p->vLitToCell, iLit ) : NULL;
}

Vec_Ptr_t * Phys_ManLitPins( Phys_Man_t * p, int iLit )
{
    return Phys_ManLitIsCell( p, iLit ) ?
           (Vec_Ptr_t *)Vec_PtrEntry( p->vLitToPins, iLit ) : NULL;
}

int Phys_ManLitFaninNum( Phys_Man_t * p, Gia_Man_t * pGia, int iLit )
{
    if ( !Phys_ManLitIsCell( p, iLit ) )
        return 0;
    if ( p->fLitMapped )
    {
        int Entry = Vec_IntEntry( pGia->vCellMapping, iLit );
        /* an inverter or a buffer has one input; only a gate record carries
           the fanin count in the cell mapping */
        if ( Entry < 0 )
            return Entry == 0 ? 0 : 1;
        return Gia_ObjCellSize( pGia, iLit );
    }
    return Phys_ManObjFaninNum( p, pGia, Abc_Lit2Var(iLit) );
}

int Phys_ManLitFaninLit( Phys_Man_t * p, Gia_Man_t * pGia, int iLit, int k )
{
    int Entry;
    if ( !Phys_ManLitIsCell( p, iLit ) || k < 0 )
        return 0;
    if ( !p->fLitMapped )
    {
        int iFan = Phys_ManObjFaninLit( p, pGia, Abc_Lit2Var(iLit), k );
        /* pre-mapping cells drive the positive literal of their AIG node,
           so a complemented AIG edge is still read from that driver */
        return Abc_Var2Lit( Abc_Lit2Var(iFan), 0 );
    }
    Entry = Vec_IntEntry( pGia->vCellMapping, iLit );
    if ( Entry == -1 )
        /* an inverter is fed by the other polarity of the same node */
        return iLit ^ 1;
    if ( Entry == -2 )
        /* a buffer is fed by the AIG fanin of its node */
        return Gia_ObjFaninLit0p( pGia, Gia_ManObj(pGia, Abc_Lit2Var(iLit)) );
    if ( Entry <= 0 )
        return 0;
    return Gia_ObjCellFanin( pGia, iLit, k );
}

int Phys_ManDriverLit( Phys_Man_t * p, Gia_Man_t * pGia, int iLit )
{
    if ( p == NULL || !p->fLitMapped )
        /* before mapping every AIG node has one cell at its positive
           literal, and a complemented AIG edge reads that driver */
        return Abc_Var2Lit( Abc_Lit2Var(iLit), 0 );
    return iLit;
}

ABC_NAMESPACE_IMPL_END
