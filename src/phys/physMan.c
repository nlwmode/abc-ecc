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

void Phys_ManClearPlace( Phys_Man_t * p )
{
    if ( p == NULL )
        return;
    Vec_IntFreeP( &p->vObjX );
    Vec_IntFreeP( &p->vObjY );
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
    p->Period = 0.0f;
    p->TMax = 0.0f;
    p->fTimed = 0;
}

void Phys_ManStop( Phys_Man_t ** pp )
{
    Phys_Man_t * p;
    Vec_Ptr_t * vPins;
    int i;
    if ( pp == NULL || *pp == NULL )
        return;
    p = *pp;
    *pp = NULL;
    Vec_PtrForEachEntry( Vec_Ptr_t *, p->vObjToPins, vPins, i )
        if ( vPins != NULL )
            Vec_PtrFree( vPins );
    Vec_PtrFree( p->vObjToCell );
    Vec_PtrFree( p->vObjToPins );
    Vec_IntFreeP( &p->vObjCellLit );
    Phys_ManClearPlace( p );
    Phys_ManClearTiming( p );
    ABC_FREE( p );
}

void Phys_ManResize( Phys_Man_t * p, int nObjs )
{
    Vec_Ptr_t * vPins;
    int i;
    if ( p == NULL )
        return;
    Vec_PtrForEachEntry( Vec_Ptr_t *, p->vObjToPins, vPins, i )
        if ( vPins != NULL )
            Vec_PtrFree( vPins );
    Vec_PtrClear( p->vObjToCell );
    Vec_PtrClear( p->vObjToPins );
    Vec_PtrFill( p->vObjToCell, nObjs, NULL );
    Vec_PtrFill( p->vObjToPins, nObjs, NULL );
    if ( p->vObjCellLit != NULL )
        Vec_IntFree( p->vObjCellLit );
    p->vObjCellLit = Vec_IntStartFull( nObjs );
    p->nObjs = nObjs;
    Phys_ManClearPlace( p );
    Phys_ManClearTiming( p );
}

ABC_NAMESPACE_IMPL_END
