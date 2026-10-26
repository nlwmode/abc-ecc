/**Function*************************************************************

  Synopsis    [Static timing analysis for the physical-synthesis flow]

  Description [Forward propagation of arrival times and slews over the
               mapped Gia netlist.  Cell delays and output slews use the
               Liberty NLDM tables.  Interconnect delays come from an
               Elmore model of an RC tree built from the placement: the
               driver and the sinks of a net are connected by a
               rectilinear minimum spanning tree over their placed pin
               centers, every tree edge becomes a resistance R*l in
               series with half of its own capacitance C*l, and each
               sink adds its Liberty input capacitance at its node.  The
               Elmore delay of a sink is the sum over the edges of its
               path from the driver of R_e * (C_e/2 + C_subtree(e)),
               where C_subtree(e) is everything hanging below the edge.
               The whole tree capacitance, not the sum of point-to-point
               wires, is the load that the driver's NLDM table sees, and
               the wire also degrades the slew on the way to each sink.
               Both rise and fall transitions are tracked.]

  SideEffects []

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "aig/gia/gia.h"
#include "phys/physMan.h"
#include "phys/lib/physLef.h"
#include "phys/lib/physLiberty.h"
#include "phys/sta/physSta.h"

ABC_NAMESPACE_IMPL_START


/* the ics55 technology LEF gives only sheet resistance, no capacitance;
   use a typical value of 0.2 fF per micron of wire when nothing else is
   available, so that wire delays are not purely resistive */
#define PHYS_STA_DEFAULT_CAP 0.0002f

/* the 10%-to-90% transition time of a first-order RC step is 2.2 time
   constants; the slew at the far end of a wire is the slew at the driver
   degraded by that much, combined with it in quadrature */
#define PHYS_STA_SLEW_RC 2.2f

void Phys_StaSetDefault( Phys_StaPars_t * p )
{
    p->Period    = 0.0f;
    p->InputSlew = 10.0f;
    p->pLayer    = NULL;
    p->WireRes   = 0.0f;
    p->WireCap   = 0.0f;
    p->fVerbose  = 0;
}

static float Phys_StaObjX( Phys_Man_t * p, int iObj )
{
    int Coord;
    if ( !p->fPlaced || iObj < 0 || iObj >= Vec_IntSize(p->vObjX) )
        return 0.0f;
    Coord = Vec_IntEntry( p->vObjX, iObj );
    return Coord < 0 ? 0.0f : (float)Coord;
}
static float Phys_StaObjY( Phys_Man_t * p, int iObj )
{
    int Coord;
    if ( !p->fPlaced || iObj < 0 || iObj >= Vec_IntSize(p->vObjY) )
        return 0.0f;
    Coord = Vec_IntEntry( p->vObjY, iObj );
    return Coord < 0 ? 0.0f : (float)Coord;
}
static float Phys_StaPinCap( Vec_Ptr_t * vPins, int k )
{
    Phys_LibPin_t * pPin;
    if ( vPins == NULL || k < 0 || k >= Vec_PtrSize(vPins) )
        return 0.0f;
    pPin = (Phys_LibPin_t *)Vec_PtrEntry( vPins, k );
    return pPin ? pPin->Cap : 0.0f;
}
static Phys_LibPin_t * Phys_StaOutPin( Phys_LibCell_t * pCell )
{
    Phys_LibPin_t * pPin;
    int k;
    Vec_PtrForEachEntry( Phys_LibPin_t *, pCell->vPins, pPin, k )
        if ( pPin->Dir == 1 )
            return pPin;
    return NULL;
}
static float Phys_StaMax( float a, float b )
{
    return a > b ? a : b;
}
static float Phys_StaManhattan( float x0, float y0, float x1, float y1 )
{
    float dx = x0 - x1, dy = y0 - y1;
    return (dx < 0.0f ? -dx : dx) + (dy < 0.0f ? -dy : dy);
}
/* the slew at the far end of a wire of Elmore delay Delay */
static float Phys_StaSlewAfter( float Slew, float Delay )
{
    float Add = PHYS_STA_SLEW_RC * Delay;
    return sqrtf( Slew * Slew + Add * Add );
}


/**Function*************************************************************

  Synopsis    [Builds the RC tree of one net and evaluates its Elmore delays]

  Description [pX/pY hold the placed pin centers in microns.  Entry zero is
               the driver; entries 1..nPins-1 are the sinks, and pCap holds
               the input capacitance of every sink.  The pins are joined by
               a rectilinear minimum spanning tree computed by Prim's
               algorithm and rooted at the driver.  pDelay receives the
               Elmore delay of every pin and the total net capacitance and
               the total wire length are returned in pTotalCap/pTotalLen.]

  SideEffects []

***********************************************************************/
static void Phys_StaElmoreNet( float * pX, float * pY, float * pCap, int nPins,
                               float RcR, float RcC, float * pDelay,
                               float * pTotalCap, float * pTotalLen )
{
    float * pBest, * pLen, * pSub;
    int * pParent, * pOrder;
    int nAdded, i, j, iBest;
    *pTotalCap = 0.0f;
    *pTotalLen = 0.0f;
    if ( nPins <= 1 )
    {
        if ( nPins == 1 )
            pDelay[0] = 0.0f;
        return;
    }
    pParent = ABC_ALLOC( int, nPins );
    pOrder  = ABC_ALLOC( int, nPins );
    pBest   = ABC_ALLOC( float, nPins );
    pLen    = ABC_CALLOC( float, nPins );
    pSub    = ABC_CALLOC( float, nPins );
    /* Prim's algorithm from the driver; the parent pointers form the
       tree rooted at the driver */
    for ( i = 1; i < nPins; i++ )
    {
        pParent[i] = 0;
        pBest[i]   = Phys_StaManhattan( pX[0], pY[0], pX[i], pY[i] );
    }
    pBest[0] = -1.0f;
    pOrder[0] = 0;
    nAdded = 1;
    while ( nAdded < nPins )
    {
        iBest = -1;
        for ( j = 1; j < nPins; j++ )
            if ( pBest[j] >= 0.0f && (iBest < 0 || pBest[j] < pBest[iBest]) )
                iBest = j;
        if ( iBest < 0 )
            break;
        pLen[iBest]  = pBest[iBest];
        pBest[iBest] = -1.0f;
        pOrder[nAdded++] = iBest;
        for ( j = 1; j < nPins; j++ )
            if ( pBest[j] >= 0.0f )
            {
                float Dist = Phys_StaManhattan( pX[iBest], pY[iBest], pX[j], pY[j] );
                if ( Dist < pBest[j] )
                {
                    pBest[j]   = Dist;
                    pParent[j] = iBest;
                }
            }
    }
    /* the capacitance hanging below every node: its own pin plus the
       complete subtrees of its children, wire capacitance included */
    for ( i = 0; i < nPins; i++ )
        pSub[i] = pCap[i];
    for ( i = nAdded - 1; i > 0; i-- )
    {
        int v = pOrder[i];
        pSub[pParent[v]] += pSub[v] + RcC * pLen[v];
    }
    /* the driver's NLDM tables see the whole net, and the Elmore delay of
       a node accumulates R_e * (C_e/2 + C_subtree(e)) along its path */
    for ( i = 1; i < nPins; i++ )
    {
        *pTotalCap += pCap[i] + RcC * pLen[i];
        *pTotalLen += pLen[i];
    }
    pDelay[0] = 0.0f;
    for ( i = 1; i < nAdded; i++ )
    {
        int v = pOrder[i];
        pDelay[v] = pDelay[pParent[v]] +
                    RcR * pLen[v] * ( pSub[v] + 0.5f * RcC * pLen[v] );
    }
    ABC_FREE( pParent );
    ABC_FREE( pOrder );
    ABC_FREE( pBest );
    ABC_FREE( pLen );
    ABC_FREE( pSub );
}
/* adds a sink pin to the list of the net driven by iDriver */
static void Phys_StaNetAdd( Vec_Ptr_t * vSinkObj, Vec_Ptr_t * vSinkPin,
                            int iDriver, int iSink, int iPin, int nObjs )
{
    Vec_Int_t * vObj, * vPin;
    if ( iDriver <= 0 || iDriver >= nObjs )
        return;
    vObj = (Vec_Int_t *)Vec_PtrEntry( vSinkObj, iDriver );
    if ( vObj == NULL )
    {
        Vec_PtrWriteEntry( vSinkObj, iDriver, vObj = Vec_IntAlloc(4) );
        Vec_PtrWriteEntry( vSinkPin, iDriver, vPin = Vec_IntAlloc(4) );
    }
    else
        vPin = (Vec_Int_t *)Vec_PtrEntry( vSinkPin, iDriver );
    Vec_IntPush( vObj, iSink );
    Vec_IntPush( vPin, iPin );
}


/**Function*************************************************************

  Synopsis    [Runs static timing analysis]

  Description [Returns 1 on success and 0 on failure.]

  SideEffects []

***********************************************************************/
int Phys_StaRun( Phys_Man_t * p, Gia_Man_t * pGia, Phys_StaPars_t * pPars )
{
    int nObjs = Gia_ManObjNum( pGia );
    Vec_Flt_t * vArrR, * vArrF, * vSlewR, * vSlewF, * vSlack;
    Vec_Flt_t * vPinDelay, * vNetCap, * vPoDelay;
    Vec_Int_t * vPinOff;
    Vec_Ptr_t * vSinkObj, * vSinkPin;
    Vec_Int_t * vEnds;
    Vec_Flt_t * vEndArr, * vEndSlack;
    float RcR, RcC, Dbu, TMax = 0.0f, WNS = 0.0f, TNS = 0.0f;
    float TotalWire = 0.0f, TotalWireCap = 0.0f, MaxNetDelay = 0.0f;
    int i, k, nCells = 0, nIns = 0, nNets = 0, nSinks = 0, nEndpoints;
    int iWorstNet = -1, nWorstSinks = 0;
    if ( p == NULL || pGia == NULL || pPars == NULL )
        return 0;
    if ( pPars->Period < 0.0f || pPars->InputSlew < 0.0f ||
         pPars->WireRes < 0.0f || pPars->WireCap < 0.0f )
    {
        fprintf( stdout, "STA: negative timing or RC parameters are not supported.\n" );
        return 0;
    }
    if ( p->vObjToCell == NULL || Vec_PtrSize(p->vObjToCell) < nObjs )
    {
        fprintf( stdout, "STA: the network has not been bound. Run \"&phys_bind\" first.\n" );
        return 0;
    }
    Dbu = (float)(p->pLef ? p->pLef->Units.Dbu : 1000);
    if ( Dbu <= 0.0f )
        Dbu = 1000.0f;
    RcR = RcC = 0.0f;
    if ( p->pLef != NULL )
    {
        Phys_LefLayer_t * pLayer;
        if ( pPars->pLayer != NULL && strcmp( pPars->pLayer, "auto" ) )
        {
            pLayer = Phys_LefFindLayer( p->pLef, pPars->pLayer );
            if ( pLayer == NULL )
            {
                fprintf( stdout, "STA: cannot find layer \"%s\" in the LEF data.\n", pPars->pLayer );
                return 0;
            }
            if ( !pLayer->fRouting )
                fprintf( stdout, "STA: warning: layer \"%s\" is not a routing layer.\n", pPars->pLayer );
        }
        else
            pLayer = Phys_LefFindRcLayer( p->pLef, NULL );
        Phys_LefLayerRc( p->pLef, pLayer, &RcR, &RcC );
    }
    else if ( pPars->pLayer != NULL && strcmp( pPars->pLayer, "auto" ) )
        fprintf( stdout, "STA: no LEF data; the layer selection \"%s\" is ignored.\n", pPars->pLayer );
    if ( pPars->WireRes > 0.0f )
        RcR = pPars->WireRes;
    if ( pPars->WireCap > 0.0f )
        RcC = pPars->WireCap;
    else if ( RcC <= 0.0f )
    {
        RcC = PHYS_STA_DEFAULT_CAP;
        if ( pPars->fVerbose )
            fprintf( stdout, "STA: LEF defines no wire capacitance; using the default %.6f pF/um.\n", RcC );
    }
    if ( !p->fPlaced && pPars->fVerbose )
        fprintf( stdout, "STA: the design is not placed; wire parasitics are zero.\n" );
    vArrR  = Vec_FltStart( nObjs );
    vArrF  = Vec_FltStart( nObjs );
    vSlewR = Vec_FltStart( nObjs );
    vSlewF = Vec_FltStart( nObjs );
    vSlack = Vec_FltStart( nObjs );
    vEnds     = Vec_IntAlloc( 64 );
    vEndArr   = Vec_FltAlloc( 64 );
    vEndSlack = Vec_FltAlloc( 64 );
    Vec_FltFill( vSlewR, nObjs, pPars->InputSlew );
    Vec_FltFill( vSlewF, nObjs, pPars->InputSlew );
    for ( i = 0; i < nObjs; i++ )
        Vec_FltWriteEntry( vSlack, i, -1.0f );
    /* primary inputs start at time zero */
    for ( i = 1; i < nObjs; i++ )
        if ( Gia_ObjIsPi(pGia, Gia_ManObj(pGia, i)) )
            Vec_FltWriteEntry( vArrR, i, 0.0f ), Vec_FltWriteEntry( vArrF, i, 0.0f );
    /* collect the sinks of every net */
    vSinkObj  = Vec_PtrStart( nObjs );
    vSinkPin  = Vec_PtrStart( nObjs );
    vPinOff   = Vec_IntStartFull( nObjs );
    vPinDelay = Vec_FltAlloc( 0 );
    vNetCap   = Vec_FltStart( nObjs );
    vPoDelay  = Vec_FltStart( nObjs );
    for ( i = 1; i < nObjs; i++ )
    {
        Phys_LibCell_t * pCell = (Phys_LibCell_t *)Vec_PtrEntry( p->vObjToCell, i );
        Gia_Obj_t * pObj = Gia_ManObj( pGia, i );
        Vec_Ptr_t * vPins;
        int iLit, d, nPins;
        if ( pCell == NULL )
        {
            if ( Gia_ObjIsPo( pGia, pObj ) )
            {
                d = Abc_Lit2Var( Gia_ObjFaninLit0(pObj, i) );
                if ( d > 0 )
                    Phys_StaNetAdd( vSinkObj, vSinkPin, d, i, -1, nObjs );
            }
            continue;
        }
        iLit  = Vec_IntEntry( p->vObjCellLit, i );
        vPins = (Vec_Ptr_t *)Vec_PtrEntry( p->vObjToPins, i );
        if ( iLit < 0 || vPins == NULL )
            continue;
        nPins = Vec_PtrSize( vPins );
        Vec_IntWriteEntry( vPinOff, i, Vec_FltSize(vPinDelay) );
        for ( k = 0; k < nPins; k++ )
            Vec_FltPush( vPinDelay, 0.0f );
        if ( nPins > Gia_ObjCellSize(pGia, iLit) )
            nPins = Gia_ObjCellSize( pGia, iLit );
        for ( k = 0; k < nPins; k++ )
        {
            d = Abc_Lit2Var( Gia_ObjCellFanin(pGia, iLit, k) );
            if ( d > 0 )
                Phys_StaNetAdd( vSinkObj, vSinkPin, d, i, k, nObjs );
        }
    }
    /* build the RC tree of every net and evaluate the Elmore delay of each
       sink pin from the placement coordinates */
    for ( i = 1; i < nObjs; i++ )
    {
        Vec_Int_t * vObj = (Vec_Int_t *)Vec_PtrEntry( vSinkObj, i );
        Vec_Int_t * vPin = (Vec_Int_t *)Vec_PtrEntry( vSinkPin, i );
        float * pX, * pY, * pCap, * pDel;
        float TotalCap, TotalLen, NetMax = 0.0f;
        int nPins, q;
        if ( vObj == NULL )
            continue;
        nPins = 1 + Vec_IntSize( vObj );
        pX   = ABC_ALLOC( float, nPins );
        pY   = ABC_ALLOC( float, nPins );
        pCap = ABC_CALLOC( float, nPins );
        pDel = ABC_ALLOC( float, nPins );
        pX[0] = Phys_StaObjX( p, i ) / Dbu;
        pY[0] = Phys_StaObjY( p, i ) / Dbu;
        for ( q = 0; q < Vec_IntSize(vObj); q++ )
        {
            int iSink = Vec_IntEntry( vObj, q );
            int iPin  = Vec_IntEntry( vPin, q );
            Vec_Ptr_t * vPins = (Vec_Ptr_t *)Vec_PtrEntry( p->vObjToPins, iSink );
            pX[q+1]   = Phys_StaObjX( p, iSink ) / Dbu;
            pY[q+1]   = Phys_StaObjY( p, iSink ) / Dbu;
            pCap[q+1] = Phys_StaPinCap( vPins, iPin );
        }
        Phys_StaElmoreNet( pX, pY, pCap, nPins, RcR, RcC, pDel, &TotalCap, &TotalLen );
        Vec_FltWriteEntry( vNetCap, i, TotalCap );
        TotalWire    += TotalLen;
        TotalWireCap += RcC * TotalLen;
        for ( q = 0; q < Vec_IntSize(vObj); q++ )
        {
            int iSink = Vec_IntEntry( vObj, q );
            int iPin  = Vec_IntEntry( vPin, q );
            if ( NetMax < pDel[q+1] )
                NetMax = pDel[q+1];
            if ( iPin < 0 )
                Vec_FltWriteEntry( vPoDelay, iSink, pDel[q+1] );
            else if ( Vec_IntEntry(vPinOff, iSink) >= 0 )
                Vec_FltWriteEntry( vPinDelay, Vec_IntEntry(vPinOff, iSink) + iPin, pDel[q+1] );
        }
        if ( MaxNetDelay < NetMax )
        {
            MaxNetDelay  = NetMax;
            iWorstNet    = i;
            nWorstSinks  = Vec_IntSize( vObj );
        }
        nNets++;
        nSinks += Vec_IntSize( vObj );
        ABC_FREE( pX );
        ABC_FREE( pY );
        ABC_FREE( pCap );
        ABC_FREE( pDel );
    }
    /* forward propagation (Gia objects are in topological order) */
    for ( i = 1; i < nObjs; i++ )
    {
        Phys_LibCell_t * pCell = (Phys_LibCell_t *)Vec_PtrEntry( p->vObjToCell, i );
        Gia_Obj_t * pObj = Gia_ManObj( pGia, i );
        Phys_LibPin_t * pOut;
        Vec_Ptr_t * vPins;
        float OutR = 0.0f, OutF = 0.0f, SlewR = pPars->InputSlew, SlewF = pPars->InputSlew;
        float Load;
        int fAny = 0, iLit;
        if ( pCell == NULL )
        {
            if ( Gia_ObjIsPo(pGia, pObj) )
            {
                int d = Abc_Lit2Var( Gia_ObjFaninLit0(pObj, i) );
                float Delay = d > 0 ? Vec_FltEntry( vPoDelay, i ) : 0.0f;
                float ArrR = Vec_FltEntry( vArrR, d ) + Delay;
                float ArrF = Vec_FltEntry( vArrF, d ) + Delay;
                float Arr  = Phys_StaMax( ArrR, ArrF );
                Vec_FltWriteEntry( vArrR, i, ArrR );
                Vec_FltWriteEntry( vArrF, i, ArrF );
                Vec_FltWriteEntry( vSlewR, i, Phys_StaSlewAfter( Vec_FltEntry(vSlewR, d), Delay ) );
                Vec_FltWriteEntry( vSlewF, i, Phys_StaSlewAfter( Vec_FltEntry(vSlewF, d), Delay ) );
                Vec_IntPush( vEnds, i );
                Vec_FltPush( vEndArr, Arr );
                if ( TMax < Arr )
                    TMax = Arr;
            }
            continue;
        }
        nCells++;
        pOut  = Phys_StaOutPin( pCell );
        vPins = (Vec_Ptr_t *)Vec_PtrEntry( p->vObjToPins, i );
        iLit  = Vec_IntEntry( p->vObjCellLit, i );
        if ( iLit < 0 )
            continue;
        Load = Vec_FltEntry( vNetCap, i );
        for ( k = 0; k < Gia_ObjCellSize(pGia, iLit); k++ )
        {
            int d = Abc_Lit2Var( Gia_ObjCellFanin(pGia, iLit, k) );
            Phys_LibPin_t * pInPin = vPins && k < Vec_PtrSize(vPins) ? (Phys_LibPin_t *)Vec_PtrEntry(vPins, k) : NULL;
            Phys_TimingArc_t * pArc;
            float InR, InF, InSlewR, InSlewF, Delay;
            float CandR, CandF, TransR, TransF;
            nIns++;
            if ( d == 0 )
            {
                InR = InF = 0.0f;
                InSlewR = InSlewF = pPars->InputSlew;
            }
            else
            {
                Delay   = vPins && k < Vec_PtrSize(vPins) && Vec_IntEntry(vPinOff, i) >= 0 ?
                          Vec_FltEntry( vPinDelay, Vec_IntEntry(vPinOff, i) + k ) : 0.0f;
                InR     = Vec_FltEntry( vArrR, d ) + Delay;
                InF     = Vec_FltEntry( vArrF, d ) + Delay;
                InSlewR = Phys_StaSlewAfter( Vec_FltEntry(vSlewR, d), Delay );
                InSlewF = Phys_StaSlewAfter( Vec_FltEntry(vSlewF, d), Delay );
            }
            if ( pOut == NULL || pInPin == NULL || (pArc = Phys_LibPinFindArc( pOut, pInPin->pName )) == NULL )
            {
                OutR  = Phys_StaMax( OutR,  InR );
                OutF  = Phys_StaMax( OutF,  InF );
                SlewR = Phys_StaMax( SlewR, InSlewR );
                SlewF = Phys_StaMax( SlewF, InSlewF );
                fAny = 1;
                continue;
            }
            if ( pArc->Sense < 0 )
            {
                CandR  = InF + Phys_SurfaceLookup( pArc->pRise, InSlewF, Load );
                CandF  = InR + Phys_SurfaceLookup( pArc->pFall, InSlewR, Load );
                TransR = Phys_SurfaceLookup( pArc->pRiseTrans, InSlewF, Load );
                TransF = Phys_SurfaceLookup( pArc->pFallTrans, InSlewR, Load );
            }
            else
            {
                CandR  = InR + Phys_SurfaceLookup( pArc->pRise, InSlewR, Load );
                CandF  = InF + Phys_SurfaceLookup( pArc->pFall, InSlewF, Load );
                TransR = Phys_SurfaceLookup( pArc->pRiseTrans, InSlewR, Load );
                TransF = Phys_SurfaceLookup( pArc->pFallTrans, InSlewF, Load );
            }
            OutR  = Phys_StaMax( OutR,  CandR );
            OutF  = Phys_StaMax( OutF,  CandF );
            SlewR = Phys_StaMax( SlewR, TransR );
            SlewF = Phys_StaMax( SlewF, TransF );
            fAny = 1;
        }
        if ( fAny )
        {
            Vec_FltWriteEntry( vArrR, i, OutR );
            Vec_FltWriteEntry( vArrF, i, OutF );
            Vec_FltWriteEntry( vSlewR, i, SlewR );
            Vec_FltWriteEntry( vSlewF, i, SlewF );
        }
    }
    /* timing summary */
    {
        float Period = pPars->Period > 0.0f ? pPars->Period : TMax;
        nEndpoints = Vec_IntSize( vEnds );
        WNS = Period;
        for ( i = 0; i < nEndpoints; i++ )
        {
            int iPo = Vec_IntEntry( vEnds, i );
            float Slack = Period - Vec_FltEntry( vEndArr, i );
            Vec_FltWriteEntry( vSlack, iPo, Slack );
            Vec_FltPush( vEndSlack, Slack );
            if ( Slack < WNS )
                WNS = Slack;
            if ( Slack < 0.0f )
                TNS += Slack;
        }
        if ( nEndpoints == 0 )
            WNS = 0.0f;
        /* save the result */
        Phys_ManClearTiming( p );
        p->Period       = Period;
        p->vObjArrRise  = vArrR;
        p->vObjArrFall  = vArrF;
        p->vObjSlewRise = vSlewR;
        p->vObjSlewFall = vSlewF;
        p->vObjSlack    = vSlack;
        p->TMax         = TMax;
        p->fTimed       = 1;
        if ( pPars->fVerbose )
        {
            fprintf( stdout, "STA: %d cells, %d input pins, %d nets, %d sinks, %d endpoints.\n",
                     nCells, nIns, nNets, nSinks, nEndpoints );
            fprintf( stdout, "STA: wire %.2f um, %.3f pF, R=%.3f ohm/um, C=%.6f pF/um.\n",
                     TotalWire, TotalWireCap, RcR, RcC );
            if ( iWorstNet >= 0 )
                fprintf( stdout, "STA: slowest net: driver object %d, %d sinks, %.2f ps of Elmore delay.\n",
                         iWorstNet, nWorstSinks, MaxNetDelay );
            fprintf( stdout, "STA: longest path %.2f ps, period %.2f ps, WNS %.2f ps, TNS %.2f ps.\n",
                     TMax, Period, WNS, TNS );
            if ( nEndpoints > 0 )
            {
                int nWorst = nEndpoints < 10 ? nEndpoints : 10;
                int j;
                fprintf( stdout, "STA: worst endpoints:\n" );
                for ( j = 0; j < nWorst; j++ )
                {
                    int iBest = -1, b;
                    float Best = 1e30f;
                    for ( b = 0; b < nEndpoints; b++ )
                    {
                        float Slack = Vec_FltEntry( vEndSlack, b );
                        if ( Slack < Best )
                            Best = Slack, iBest = b;
                    }
                    if ( iBest < 0 )
                        break;
                    fprintf( stdout, "  PO object %6d : arrival %10.2f ps, slack %10.2f ps\n",
                             Vec_IntEntry(vEnds, iBest), Vec_FltEntry(vEndArr, iBest), Best );
                    Vec_FltWriteEntry( vEndSlack, iBest, 1e30f );
                }
            }
        }
        else
            fprintf( stdout, "STA: longest path %.2f ps, period %.2f ps, WNS %.2f ps, TNS %.2f ps.\n",
                     TMax, Period, WNS, TNS );
    }
    /* free the net data */
    for ( i = 0; i < nObjs; i++ )
    {
        Vec_Int_t * vObj = (Vec_Int_t *)Vec_PtrEntry( vSinkObj, i );
        Vec_Int_t * vPin = (Vec_Int_t *)Vec_PtrEntry( vSinkPin, i );
        if ( vObj != NULL )
            Vec_IntFree( vObj );
        if ( vPin != NULL )
            Vec_IntFree( vPin );
    }
    Vec_PtrFree( vSinkObj );
    Vec_PtrFree( vSinkPin );
    Vec_IntFree( vPinOff );
    Vec_FltFree( vPinDelay );
    Vec_FltFree( vNetCap );
    Vec_FltFree( vPoDelay );
    Vec_IntFree( vEnds );
    Vec_FltFree( vEndArr );
    Vec_FltFree( vEndSlack );
    return 1;
}

void Phys_StaPrint( Phys_Man_t * p )
{
    if ( p == NULL || !p->fTimed )
    {
        fprintf( stdout, "STA: no timing data.\n" );
        return;
    }
    fprintf( stdout, "STA: period %.2f ps, longest path %.2f ps.\n", p->Period, p->TMax );
}

ABC_NAMESPACE_IMPL_END
