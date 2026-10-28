/**Function*************************************************************

  Synopsis    [Builds the physical ranking data of &phys_map]

  Description [The mapper ranks cuts and matches lexicographically by

                   critical-path timing, area, congestion, routability,

               where the timing key is the predicted arrival of the cut
               (placement-aware wire delay plus gate delay) checked against
               the required time, the area is the true area flow, the
               congestion is the routing overflow demand of the cut wires
               and the routability is the worst local supply/demand ratio
               touched by the cut.  This file converts the placement
               (coordinates, RUDY congestion, routability) and the STA
               results (arrival, required, net capacitance, unit RC) into
               the flat per-object arrays of Jf_Phys_t.

               Every array is indexed by Gia object id.  Missing data is
               filled with neutral values (zero congestion, routability
               one, zero criticality, +INF required), so the mapper never
               reads uninitialized memory when only part of the flow has
               been run.]

  SideEffects []

***********************************************************************/

#include <string.h>
#include "phys/phys.h"
#include "phys/physMan.h"
#include "phys/map/physMap.h"

ABC_NAMESPACE_IMPL_START

static inline float Phys_MapAbs( float x )
{
    return x < 0.0f ? -x : x;
}
static inline float Phys_MapVecFlt( Vec_Flt_t * v, int i, float Def )
{
    if ( v == NULL || i < 0 || i >= Vec_FltSize(v) )
        return Def;
    return Vec_FltEntry( v, i );
}
static inline int Phys_MapVecInt( Vec_Int_t * v, int i, int Def )
{
    if ( v == NULL || i < 0 || i >= Vec_IntSize(v) )
        return Def;
    return Vec_IntEntry( v, i );
}
/* Placement coordinates of every object, in um.  The placed cells have
   their own coordinates; the internal AIG nodes that do not correspond to
   a placed cell inherit the average of their fanins. */
static void Phys_MapCoords( Phys_Man_t * pMan, Gia_Man_t * pGia, float * pX, float * pY )
{
    int nObjs = Gia_ManObjNum( pGia );
    float Dbu = pMan->Dbu > 0.0f ? pMan->Dbu :
                (float)(pMan->pLef ? pMan->pLef->Units.Dbu : 1000);
    int i, k, nFans;
    for ( i = 0; i < nObjs; i++ )
        pX[i] = pY[i] = -1.0f;
    for ( i = 1; i < nObjs; i++ )
    {
        Gia_Obj_t * pObj = Gia_ManObj( pGia, i );
        int x = Phys_MapVecInt( pMan->vObjX, i, -1 );
        if ( pMan->fPlaced && x >= 0 )
        {
            pX[i] = (float)x / Dbu;
            pY[i] = (float)Phys_MapVecInt( pMan->vObjY, i, 0 ) / Dbu;
            continue;
        }
        if ( Gia_ObjIsPi(pGia, pObj) )
        {
            pX[i] = pY[i] = 0.0f;
            continue;
        }
        if ( !Gia_ObjIsAnd(pObj) )
            continue;
        nFans = Gia_ObjIsMuxId( pGia, i ) ? 3 : 2;
        {
            float SumX = 0.0f, SumY = 0.0f;
            int nKnown = 0;
            for ( k = 0; k < nFans; k++ )
            {
                int d = k == 0 ? Gia_ObjFaninId0(pObj, i) :
                        k == 1 ? Gia_ObjFaninId1(pObj, i) :
                                 Gia_ObjFaninId2(pGia, i);
                if ( d > 0 && d < nObjs && pX[d] >= 0.0f )
                    SumX += pX[d], SumY += pY[d], nKnown++;
            }
            if ( nKnown > 0 )
                pX[i] = SumX / nKnown, pY[i] = SumY / nKnown;
        }
    }
    for ( i = 0; i < nObjs; i++ )
        if ( pX[i] < 0.0f )
            pX[i] = pY[i] = 0.0f;
}
int Phys_MapBuildPhys( Phys_Man_t * pMan, Gia_Man_t * pGia,
                       int fCong, Jf_Phys_t * pPhys )
{
    int nObjs = Gia_ManObjNum( pGia );
    int i, fTimed;
    memset( pPhys, 0, sizeof(Jf_Phys_t) );
    if ( pMan == NULL || !pMan->fPlaced || nObjs < 1 )
        return 0;
    pPhys->fEnable = 1;
    pPhys->fCong   = fCong;
    pPhys->nObjs   = nObjs;
    pPhys->RcR     = pMan->RcR;
    pPhys->RcC     = pMan->RcC;
    pPhys->pX      = ABC_ALLOC( float, nObjs );
    pPhys->pY      = ABC_ALLOC( float, nObjs );
    pPhys->pCong   = ABC_ALLOC( float, nObjs );
    pPhys->pRout   = ABC_ALLOC( float, nObjs );
    pPhys->pArr    = ABC_ALLOC( float, nObjs );
    pPhys->pReq    = ABC_ALLOC( float, nObjs );
    pPhys->pCrit   = ABC_ALLOC( float, nObjs );
    pPhys->pNetCap = ABC_ALLOC( float, nObjs );
    Phys_MapCoords( pMan, pGia, pPhys->pX, pPhys->pY );
    fTimed = pMan->fTimed && pMan->vObjArrRise != NULL && pMan->vObjArrFall != NULL;
    for ( i = 0; i < nObjs; i++ )
    {
        pPhys->pCong[i] = fCong ? Phys_MapVecFlt( pMan->vObjCong, i, 0.0f ) : 0.0f;
        pPhys->pRout[i] = fCong ? Phys_MapVecFlt( pMan->vObjRout, i, 1.0f ) : 1.0f;
        if ( fTimed )
        {
            float Rise = Phys_MapVecFlt( pMan->vObjArrRise, i, 0.0f );
            float Fall = Phys_MapVecFlt( pMan->vObjArrFall, i, 0.0f );
            pPhys->pArr[i] = Abc_MaxFloat( Rise, Fall );
        }
        else
            pPhys->pArr[i] = 0.0f;
        pPhys->pReq[i]    = fTimed ? Phys_MapVecFlt( pMan->vObjReq, i, 1e30f ) : 1e30f;
        pPhys->pCrit[i]   = fTimed ? Phys_MapVecFlt( pMan->vObjCrit, i, 0.0f ) : 0.0f;
        pPhys->pNetCap[i] = fTimed ? Phys_MapVecFlt( pMan->vNetCap, i, 0.0f ) : 0.0f;
        if ( !(pPhys->pCrit[i] >= 0.0f) )
            pPhys->pCrit[i] = 0.0f;
        if ( !(pPhys->pNetCap[i] >= 0.0f) )
            pPhys->pNetCap[i] = 0.0f;
    }
    return 1;
}
void Phys_MapFreePhys( Jf_Phys_t * pPhys )
{
    if ( pPhys == NULL )
        return;
    ABC_FREE( pPhys->pX );
    ABC_FREE( pPhys->pY );
    ABC_FREE( pPhys->pCong );
    ABC_FREE( pPhys->pRout );
    ABC_FREE( pPhys->pArr );
    ABC_FREE( pPhys->pReq );
    ABC_FREE( pPhys->pCrit );
    ABC_FREE( pPhys->pNetCap );
    memset( pPhys, 0, sizeof(Jf_Phys_t) );
}

ABC_NAMESPACE_IMPL_END
