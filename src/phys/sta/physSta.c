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

static float Phys_StaLitX( Phys_Man_t * p, int iLit )
{
    int Coord;
    if ( !p->fPlaced || p->vLitX == NULL || iLit < 0 || iLit >= Vec_IntSize(p->vLitX) )
        return 0.0f;
    Coord = Vec_IntEntry( p->vLitX, iLit );
    return Coord < 0 ? 0.0f : (float)Coord;
}
static float Phys_StaLitY( Phys_Man_t * p, int iLit )
{
    int Coord;
    if ( !p->fPlaced || p->vLitY == NULL || iLit < 0 || iLit >= Vec_IntSize(p->vLitY) )
        return 0.0f;
    Coord = Vec_IntEntry( p->vLitY, iLit );
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
/* point-to-point Elmore wire delay, in ps: R*d*(C + C_unit*d/2) */
float Phys_StaWireDelay( Phys_Man_t * p, float x0, float y0, float x1, float y1, float Cload )
{
    float Dist;
    if ( p == NULL || p->RcR <= 0.0f )
        return 0.0f;
    Dist = Phys_StaManhattan( x0, y0, x1, y1 );
    return p->RcR * Dist * ( Cload + 0.5f * p->RcC * Dist );
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
    if ( iDriver <= 1 || iDriver >= nObjs )
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
    int nLits;
    Vec_Flt_t * vArrR, * vArrF, * vSlewR, * vSlewF, * vSlack;
    Vec_Flt_t * vPinDelay, * vNetCap;
    Vec_Flt_t * vNetDelay, * vNetLen, * vPinGateDelay;
    Vec_Flt_t * vObjReq, * vObjCrit, * vObjInWire;
    Vec_Flt_t * vObjArrR, * vObjArrF, * vObjSlewR, * vObjSlewF;
    Vec_Flt_t * vObjNetCap, * vObjNetDelay, * vObjNetLen;
    Vec_Flt_t * vInWire, * vReq;
    Vec_Int_t * vPinOff;
    Vec_Int_t * vPred, * vPath, * vObjPath;
    Vec_Ptr_t * vSinkObj, * vSinkPin;
    Vec_Int_t * vEnds;
    Vec_Flt_t * vEndArr, * vEndSlack;
    float PathDelay = -1.0f;
    int PathEnd = -1;
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
    nLits = Phys_ManLitNum( p );
    if ( nLits != 2 * nObjs )
    {
        fprintf( stdout, "STA: the literal cell view has %d entries for %d objects; "
                         "run \"&phys_bind\" first.\n", nLits, nObjs );
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
    vArrR  = Vec_FltStart( nLits );
    vArrF  = Vec_FltStart( nLits );
    vPred  = Vec_IntStartFull( nLits );
    vSlewR = Vec_FltStart( nLits );
    vSlewF = Vec_FltStart( nLits );
    vSlack = Vec_FltStart( nObjs );
    vEnds     = Vec_IntAlloc( 64 );
    vEndArr   = Vec_FltAlloc( 64 );
    vEndSlack = Vec_FltAlloc( 64 );
    Vec_FltFill( vSlewR, nLits, pPars->InputSlew );
    Vec_FltFill( vSlewF, nLits, pPars->InputSlew );
    for ( i = 0; i < nObjs; i++ )
        Vec_FltWriteEntry( vSlack, i, -1.0f );
    /* primary inputs start at time zero (at their positive literal) */
    for ( i = 1; i < nObjs; i++ )
        if ( Gia_ObjIsPi(pGia, Gia_ManObj(pGia, i)) )
            Vec_FltWriteEntry( vArrR, 2*i, 0.0f ), Vec_FltWriteEntry( vArrF, 2*i, 0.0f );
    /* collect the sinks of every net */
    vSinkObj  = Vec_PtrStart( nLits );
    vSinkPin  = Vec_PtrStart( nLits );
    vPinOff   = Vec_IntStartFull( nLits );
    vPinDelay = Vec_FltAlloc( 0 );
    vNetCap   = Vec_FltStart( nLits );
    vNetDelay = Vec_FltStart( nLits );
    vNetLen   = Vec_FltStart( nLits );
    vInWire   = Vec_FltStart( nLits );
    vPinGateDelay = Vec_FltAlloc( 0 );
    vReq      = Vec_FltStart( nLits );
    Vec_FltFill( vReq, nLits, 1e30f );
    vObjReq   = Vec_FltStart( nObjs );
    Vec_FltFill( vObjReq, nObjs, 1e30f );
    vObjCrit  = Vec_FltStart( nObjs );
    vObjInWire= Vec_FltStart( nObjs );
    for ( i = 0; i < nLits; i++ )
    {
        Vec_Ptr_t * vPins;
        int d, nPins, nFans;
        if ( !Phys_ManLitIsCell( p, i ) )
            continue;
        vPins = Phys_ManLitPins( p, i );
        if ( vPins == NULL )
            continue;
        nPins = Vec_PtrSize( vPins );
        nFans = Phys_ManLitFaninNum( p, pGia, i );
        if ( nPins > nFans )
            nPins = nFans;
        Vec_IntWriteEntry( vPinOff, i, Vec_FltSize(vPinDelay) );
        for ( k = 0; k < nPins; k++ )
            Vec_FltPush( vPinDelay, 0.0f ), Vec_FltPush( vPinGateDelay, 0.0f );
        for ( k = 0; k < nPins; k++ )
        {
            d = Phys_ManLitFaninLit( p, pGia, i, k );
            if ( d > 1 && d < nLits )
                Phys_StaNetAdd( vSinkObj, vSinkPin, d, i, k, nLits );
        }
    }
    for ( i = 1; i < nObjs; i++ )
    {
        Gia_Obj_t * pObj = Gia_ManObj( pGia, i );
        int d;
        if ( !Gia_ObjIsPo(pGia, pObj) )
            continue;
        d = Phys_ManDriverLit( p, pGia, Gia_ObjFaninLit0p(pGia, pObj) );
        if ( d > 1 && d < nLits )
            Phys_StaNetAdd( vSinkObj, vSinkPin, d, 2*i, -1, nLits );
    }
    /* build the RC tree of every net and evaluate the Elmore delay of each
       sink pin from the placement coordinates */
    for ( i = 0; i < nLits; i++ )
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
        pX[0] = Phys_StaLitX( p, i ) / Dbu;
        pY[0] = Phys_StaLitY( p, i ) / Dbu;
        for ( q = 0; q < Vec_IntSize(vObj); q++ )
        {
            int iSink = Vec_IntEntry( vObj, q );
            int iPin  = Vec_IntEntry( vPin, q );
            Vec_Ptr_t * vPins = Phys_ManLitPins( p, iSink );
            pX[q+1]   = Phys_StaLitX( p, iSink ) / Dbu;
            pY[q+1]   = Phys_StaLitY( p, iSink ) / Dbu;
            pCap[q+1] = Phys_StaPinCap( vPins, iPin );
        }
        Phys_StaElmoreNet( pX, pY, pCap, nPins, RcR, RcC, pDel, &TotalCap, &TotalLen );
        Vec_FltWriteEntry( vNetCap, i, TotalCap );
        Vec_FltWriteEntry( vNetLen, i, TotalLen );
        TotalWire    += TotalLen;
        TotalWireCap += RcC * TotalLen;
        for ( q = 0; q < Vec_IntSize(vObj); q++ )
        {
            int iSink = Vec_IntEntry( vObj, q );
            int iPin  = Vec_IntEntry( vPin, q );
            if ( NetMax < pDel[q+1] )
                NetMax = pDel[q+1];
            if ( iPin < 0 )
                Vec_FltWriteEntry( vInWire, iSink, pDel[q+1] );
            else if ( Vec_IntEntry(vPinOff, iSink) >= 0 )
                Vec_FltWriteEntry( vPinDelay, Vec_IntEntry(vPinOff, iSink) + iPin, pDel[q+1] );
        }
        Vec_FltWriteEntry( vNetDelay, i, NetMax );
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
    /* forward propagation over the cell instances, in literal order: the
       fanin literals of an instance always belong to lower Gia objects,
       and the two phases of one object are visited positive first */
    for ( i = 0; i < nLits; i++ )
    {
        Phys_LibCell_t * pCell = Phys_ManLitCell( p, i );
        if ( pCell == NULL )
        {
            int iObj = Abc_Lit2Var( i );
            Gia_Obj_t * pObj = Gia_ManObj( pGia, iObj );
            if ( (i & 1) == 0 && Gia_ObjIsPo(pGia, pObj) )
            {
                int d = Phys_ManDriverLit( p, pGia, Gia_ObjFaninLit0p(pGia, pObj) );
                float Delay = d > 1 && d < nLits ? Vec_FltEntry( vInWire, i ) : 0.0f;
                float ArrR = d > 1 && d < nLits ? Vec_FltEntry( vArrR, d ) + Delay : 0.0f;
                float ArrF = d > 1 && d < nLits ? Vec_FltEntry( vArrF, d ) + Delay : 0.0f;
                float Arr  = Phys_StaMax( ArrR, ArrF );
                Vec_FltWriteEntry( vArrR, i, ArrR );
                Vec_FltWriteEntry( vArrF, i, ArrF );
                if ( d > 1 && d < nLits )
                {
                    Vec_FltWriteEntry( vSlewR, i, Phys_StaSlewAfter( Vec_FltEntry(vSlewR, d), Delay ) );
                    Vec_FltWriteEntry( vSlewF, i, Phys_StaSlewAfter( Vec_FltEntry(vSlewF, d), Delay ) );
                    Vec_IntWriteEntry( vPred, i, d );
                }
                Vec_IntPush( vEnds, iObj );
                Vec_FltPush( vEndArr, Arr );
                if ( TMax < Arr )
                    TMax = Arr;
            }
            continue;
        }
        nCells++;
        {
            Phys_LibPin_t * pOut = Phys_StaOutPin( pCell );
            Vec_Ptr_t * vPins = Phys_ManLitPins( p, i );
            float OutR = 0.0f, OutF = 0.0f, SlewR = pPars->InputSlew, SlewF = pPars->InputSlew;
            float Load = Vec_FltEntry( vNetCap, i );
            float BestCand = -1.0f;
            int BestPred = -1, fAny = 0, nFans = Phys_ManLitFaninNum( p, pGia, i );
            if ( vPins != NULL && nFans > Vec_PtrSize(vPins) )
                nFans = Vec_PtrSize(vPins);
            for ( k = 0; k < nFans; k++ )
            {
                int d = Phys_ManLitFaninLit( p, pGia, i, k );
                Phys_LibPin_t * pInPin = vPins != NULL && k < Vec_PtrSize(vPins) ?
                                        (Phys_LibPin_t *)Vec_PtrEntry(vPins, k) : NULL;
                Phys_TimingArc_t * pArc;
                float InR, InF, InSlewR, InSlewF, Delay;
                float CandR, CandF, TransR, TransF;
                float GateR = 0.0f, GateF = 0.0f;
                nIns++;
                if ( d <= 1 || d >= nLits )
                {
                    InR = InF = 0.0f;
                    InSlewR = InSlewF = pPars->InputSlew;
                }
                else
                {
                    Delay   = Vec_IntEntry(vPinOff, i) >= 0 ?
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
                    if ( Phys_StaMax( InR, InF ) > BestCand )
                        BestCand = Phys_StaMax( InR, InF ), BestPred = d;
                    fAny = 1;
                    continue;
                }
                if ( pArc->Sense < 0 )
                {
                    GateR  = Phys_SurfaceLookup( pArc->pRise, InSlewF, Load );
                    GateF  = Phys_SurfaceLookup( pArc->pFall, InSlewR, Load );
                    CandR  = InF + GateR;
                    CandF  = InR + GateF;
                    TransR = Phys_SurfaceLookup( pArc->pRiseTrans, InSlewF, Load );
                    TransF = Phys_SurfaceLookup( pArc->pFallTrans, InSlewR, Load );
                }
                else
                {
                    GateR  = Phys_SurfaceLookup( pArc->pRise, InSlewR, Load );
                    GateF  = Phys_SurfaceLookup( pArc->pFall, InSlewF, Load );
                    CandR  = InR + GateR;
                    CandF  = InF + GateF;
                    TransR = Phys_SurfaceLookup( pArc->pRiseTrans, InSlewR, Load );
                    TransF = Phys_SurfaceLookup( pArc->pFallTrans, InSlewF, Load );
                }
                /* keep the arc delay for the backward required-time pass */
                if ( Vec_IntEntry(vPinOff, i) >= 0 )
                    Vec_FltWriteEntry( vPinGateDelay, Vec_IntEntry(vPinOff, i) + k,
                                       Phys_StaMax( GateR, GateF ) );
                OutR  = Phys_StaMax( OutR,  CandR );
                OutF  = Phys_StaMax( OutF,  CandF );
                SlewR = Phys_StaMax( SlewR, TransR );
                SlewF = Phys_StaMax( SlewF, TransF );
                if ( Phys_StaMax( CandR, CandF ) > BestCand )
                    BestCand = Phys_StaMax( CandR, CandF ), BestPred = d;
                fAny = 1;
            }
            if ( fAny )
            {
                if ( BestPred > 1 )
                    Vec_IntWriteEntry( vPred, i, BestPred );
                Vec_FltWriteEntry( vArrR, i, OutR );
                Vec_FltWriteEntry( vArrF, i, OutF );
                Vec_FltWriteEntry( vSlewR, i, SlewR );
                Vec_FltWriteEntry( vSlewF, i, SlewF );
            }
        }
    }
    /* arrival per Gia object, for the mapper and the dump: the largest of
       the mapped phases.  An object with no cell keeps the zero-delay AIG
       view the mapper uses as a ranking hint for the leaves that fall
       inside a mapped cell. */
    vObjArrR    = Vec_FltStart( nObjs );
    vObjArrF    = Vec_FltStart( nObjs );
    vObjSlewR   = Vec_FltStart( nObjs );
    vObjSlewF   = Vec_FltStart( nObjs );
    vObjNetCap  = Vec_FltStart( nObjs );
    vObjNetDelay= Vec_FltStart( nObjs );
    vObjNetLen  = Vec_FltStart( nObjs );
    Vec_FltFill( vObjSlewR, nObjs, pPars->InputSlew );
    Vec_FltFill( vObjSlewF, nObjs, pPars->InputSlew );
    for ( i = 1; i < nObjs; i++ )
    {
        Gia_Obj_t * pObj = Gia_ManObj( pGia, i );
        int l0 = 2 * i, l1 = l0 + 1;
        int f0 = Phys_ManLitIsCell( p, l0 ), f1 = Phys_ManLitIsCell( p, l1 );
        float r = 0.0f, f = 0.0f, sR = pPars->InputSlew, sF = pPars->InputSlew;
        float cap = 0.0f, dly = 0.0f, len = 0.0f;
        if ( f0 )
        {
            r = Phys_StaMax( r, Vec_FltEntry(vArrR, l0) );
            f = Phys_StaMax( f, Vec_FltEntry(vArrF, l0) );
            sR = Phys_StaMax( sR, Vec_FltEntry(vSlewR, l0) );
            sF = Phys_StaMax( sF, Vec_FltEntry(vSlewF, l0) );
            cap = Phys_StaMax( cap, Vec_FltEntry(vNetCap, l0) );
            dly = Phys_StaMax( dly, Vec_FltEntry(vNetDelay, l0) );
            len = Phys_StaMax( len, Vec_FltEntry(vNetLen, l0) );
        }
        if ( f1 )
        {
            r = Phys_StaMax( r, Vec_FltEntry(vArrR, l1) );
            f = Phys_StaMax( f, Vec_FltEntry(vArrF, l1) );
            sR = Phys_StaMax( sR, Vec_FltEntry(vSlewR, l1) );
            sF = Phys_StaMax( sF, Vec_FltEntry(vSlewF, l1) );
            cap = Phys_StaMax( cap, Vec_FltEntry(vNetCap, l1) );
            dly = Phys_StaMax( dly, Vec_FltEntry(vNetDelay, l1) );
            len = Phys_StaMax( len, Vec_FltEntry(vNetLen, l1) );
        }
        if ( Gia_ObjIsPo(pGia, pObj) )
        {
            r = Vec_FltEntry( vArrR, l0 );
            f = Vec_FltEntry( vArrF, l0 );
            sR = Vec_FltEntry( vSlewR, l0 );
            sF = Vec_FltEntry( vSlewF, l0 );
            Vec_FltWriteEntry( vObjInWire, i, Vec_FltEntry(vInWire, l0) );
        }
        else if ( !f0 && !f1 && Gia_ObjIsAnd(pObj) )
        {
            int nFansI = Gia_ObjIsBuf(pObj) ? 1 : (Gia_ObjIsMuxId(pGia, i) ? 3 : 2);
            int d0 = Gia_ObjFaninId0(pObj, i);
            int d1 = Gia_ObjFaninId1(pObj, i);
            for ( k = 0; k < nFansI; k++ )
            {
                int d = k == 0 ? d0 : k == 1 ? d1 : Gia_ObjFaninId2(pGia, i);
                if ( d <= 0 || d >= nObjs )
                    continue;
                if ( r < Vec_FltEntry(vObjArrR, d) )
                    r = Vec_FltEntry(vObjArrR, d), sR = Vec_FltEntry(vObjSlewR, d);
                if ( f < Vec_FltEntry(vObjArrF, d) )
                    f = Vec_FltEntry(vObjArrF, d), sF = Vec_FltEntry(vObjSlewF, d);
            }
        }
        Vec_FltWriteEntry( vObjArrR, i, r );
        Vec_FltWriteEntry( vObjArrF, i, f );
        Vec_FltWriteEntry( vObjSlewR, i, sR );
        Vec_FltWriteEntry( vObjSlewF, i, sF );
        Vec_FltWriteEntry( vObjNetCap, i, cap );
        Vec_FltWriteEntry( vObjNetDelay, i, dly );
        Vec_FltWriteEntry( vObjNetLen, i, len );
    }
    /* Trace the critical path, for the dump.  The forward pass recorded
       the fanin literal that produced the arrival of every instance; an
       unmapped AIG node has no instance, so it falls back to the fanin
       object with the later arrival, the same zero-delay view the object
       arrays use.  Walk back from the endpoint with the latest arrival;
       the object ids strictly decrease, so the walk always terminates. */
    vPath    = Vec_IntAlloc( 64 );
    vObjPath = Vec_IntStart( nObjs );
    {
        int nEnds = Vec_IntSize( vEnds ), iEnd = -1, l;
        for ( i = 0; i < nEnds; i++ )
            if ( Vec_FltEntry( vEndArr, i ) > PathDelay )
                PathDelay = Vec_FltEntry( vEndArr, i ), iEnd = i;
        if ( iEnd >= 0 )
        {
            PathEnd = Vec_IntEntry( vEnds, iEnd );
            l = 2 * PathEnd;
            while ( l > 1 && l < nLits && Vec_IntSize(vPath) < nObjs )
            {
                int iObj = Abc_Lit2Var( l );
                int d;
                if ( Vec_IntEntry( vObjPath, iObj ) )
                    break;
                Vec_IntWriteEntry( vObjPath, iObj, 1 );
                Vec_IntPush( vPath, iObj );
                d = Vec_IntEntry( vPred, l );
                if ( d <= 1 && Gia_ObjIsAnd(Gia_ManObj(pGia, iObj)) )
                {
                    Gia_Obj_t * pObj = Gia_ManObj( pGia, iObj );
                    int d0 = Gia_ObjFaninId0( pObj, iObj );
                    int d1 = Gia_ObjFaninId1( pObj, iObj );
                    float A0 = d0 > 0 && d0 < nObjs ?
                        Phys_StaMax( Vec_FltEntry(vObjArrR,d0), Vec_FltEntry(vObjArrF,d0) ) : -1.0f;
                    float A1 = !Gia_ObjIsBuf(pObj) && d1 > 0 && d1 < nObjs ?
                        Phys_StaMax( Vec_FltEntry(vObjArrR,d1), Vec_FltEntry(vObjArrF,d1) ) : -1.0f;
                    d = A1 > A0 ? 2 * d1 : A0 >= 0.0f ? 2 * d0 : -1;
                }
                l = d;
            }
            /* the walk went from the endpoint towards the inputs */
            Vec_IntReverseOrder( vPath );
        }
        else
            PathDelay = 0.0f;
    }
    /* Backward required times.  Start at the PO endpoints with the period,
       then walk the cell instances in reverse literal order and push the
       required time through the arcs (gate + wire).  The object view takes
       the required time of the mapped phase. */
    {
        float PeriodUsed = pPars->Period > 0.0f ? pPars->Period : TMax;
        if ( PeriodUsed <= 0.0f )
            PeriodUsed = 1.0f;
        for ( i = 1; i < nObjs; i++ )
        {
            Gia_Obj_t * pObj = Gia_ManObj( pGia, i );
            int d;
            if ( !Gia_ObjIsPo(pGia, pObj) )
                continue;
            d = Phys_ManDriverLit( p, pGia, Gia_ObjFaninLit0p(pGia, pObj) );
            Vec_FltWriteEntry( vReq, 2*i, PeriodUsed );
            if ( d > 1 && d < nLits && Vec_FltEntry(vReq, d) > PeriodUsed - Vec_FltEntry(vInWire, 2*i) )
                Vec_FltWriteEntry( vReq, d, PeriodUsed - Vec_FltEntry(vInWire, 2*i) );
        }
        for ( i = nLits - 1; i >= 2; i-- )
        {
            Phys_LibCell_t * pCell = Phys_ManLitCell( p, i );
            Vec_Ptr_t * vPins;
            float Req = Vec_FltEntry( vReq, i );
            int nFans, off;
            if ( pCell == NULL || !(Req < 1e29f) )
                continue;
            off = Vec_IntEntry( vPinOff, i );
            nFans = Phys_ManLitFaninNum( p, pGia, i );
            vPins = Phys_ManLitPins( p, i );
            if ( vPins != NULL && nFans > Vec_PtrSize(vPins) )
                nFans = Vec_PtrSize(vPins);
            for ( k = 0; k < nFans; k++ )
            {
                int d = Phys_ManLitFaninLit( p, pGia, i, k );
                float Need = Req;
                if ( d <= 1 || d >= nLits || off < 0 || off + k >= Vec_FltSize(vPinDelay) )
                    continue;
                Need -= Vec_FltEntry( vPinDelay, off + k );
                if ( off + k < Vec_FltSize(vPinGateDelay) )
                    Need -= Vec_FltEntry( vPinGateDelay, off + k );
                if ( Vec_FltEntry(vReq, d) > Need )
                    Vec_FltWriteEntry( vReq, d, Need );
            }
        }
        for ( i = 1; i < nObjs; i++ )
        {
            Gia_Obj_t * pObj = Gia_ManObj( pGia, i );
            if ( Phys_ManLitIsCell( p, 2*i ) )
                Vec_FltWriteEntry( vObjReq, i, Vec_FltEntry(vReq, 2*i) );
            else if ( Phys_ManLitIsCell( p, 2*i+1 ) )
                Vec_FltWriteEntry( vObjReq, i, Vec_FltEntry(vReq, 2*i+1) );
            else if ( Gia_ObjIsPo(pGia, pObj) )
                Vec_FltWriteEntry( vObjReq, i, Vec_FltEntry(vReq, 2*i) );
        }
        /* zero-delay view of the unmapped internal nodes */
        for ( i = nObjs - 1; i > 0; i-- )
        {
            Gia_Obj_t * pObj = Gia_ManObj( pGia, i );
            float Req = Vec_FltEntry( vObjReq, i );
            int d0, d1;
            /* push the hint through the AIG even across mapped cells, so
               that the leaves inside a mapped cell still see a required
               time; the mapped fanins keep their STA value and are not
               overwritten */
            if ( !Gia_ObjIsAnd(pObj) || !(Req < 1e29f) )
                continue;
            d0 = Gia_ObjFaninId0( pObj, i );
            d1 = Gia_ObjFaninId1( pObj, i );
            if ( d0 > 0 && d0 < nObjs && !Phys_ManLitIsCell(p, 2*d0) &&
                 !Phys_ManLitIsCell(p, 2*d0+1) && Vec_FltEntry(vObjReq, d0) > Req )
                Vec_FltWriteEntry( vObjReq, d0, Req );
            if ( !Gia_ObjIsBuf(pObj) && d1 > 0 && d1 < nObjs &&
                 !Phys_ManLitIsCell(p, 2*d1) && !Phys_ManLitIsCell(p, 2*d1+1) &&
                 Vec_FltEntry(vObjReq, d1) > Req )
                Vec_FltWriteEntry( vObjReq, d1, Req );
        }
        for ( i = 1; i < nObjs; i++ )
        {
            float Arr = Phys_StaMax( Vec_FltEntry(vObjArrR, i), Vec_FltEntry(vObjArrF, i) );
            float Req = Vec_FltEntry(vObjReq, i);
            float Crit = 0.0f;
            if ( Req < 1e29f )
            {
                Crit = 1.0f - ( Req - Arr ) / PeriodUsed;
                if ( Crit < 0.0f ) Crit = 0.0f;
                if ( Crit > 1.0f ) Crit = 1.0f;
            }
            Vec_FltWriteEntry( vObjCrit, i, Crit );
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
        p->vObjArrRise  = vObjArrR;
        p->vObjArrFall  = vObjArrF;
        p->vObjSlewRise = vObjSlewR;
        p->vObjSlewFall = vObjSlewF;
        p->vObjSlack    = vSlack;
        p->vObjReq      = vObjReq;
        p->vObjCrit     = vObjCrit;
        p->vNetCap      = vObjNetCap;
        p->vNetDelay    = vObjNetDelay;
        p->vNetLen      = vObjNetLen;
        p->vPinOff      = vPinOff;
        p->vPinWireDelay= vPinDelay;
        p->vPinGateDelay= vPinGateDelay;
        p->vObjInWire   = vObjInWire;
        p->vPathObjs    = vPath;
        p->vObjPath     = vObjPath;
        p->PathDelay    = PathDelay > 0.0f ? PathDelay : 0.0f;
        p->PathEnd      = PathEnd;
        p->RcR          = RcR;
        p->RcC          = RcC;
        p->Dbu          = Dbu;
        p->InputSlew    = pPars->InputSlew;
        p->TMax         = TMax;
        p->fTimed       = 1;
        if ( pPars->fVerbose )
        {
            fprintf( stdout, "STA: %d cells, %d input pins, %d nets, %d sinks, %d endpoints.\n",
                     nCells, nIns, nNets, nSinks, nEndpoints );
            fprintf( stdout, "STA: wire %.2f um, %.3f pF, R=%.3f ohm/um, C=%.6f pF/um.\n",
                     TotalWire, TotalWireCap, RcR, RcC );
            if ( iWorstNet >= 0 )
                fprintf( stdout, "STA: slowest net: driver literal %d, %d sinks, %.2f ps of Elmore delay.\n",
                         iWorstNet, nWorstSinks, MaxNetDelay );
            fprintf( stdout, "STA: longest path %.2f ps, period %.2f ps, WNS %.2f ps, TNS %.2f ps.\n",
                     TMax, Period, WNS, TNS );
            if ( Vec_IntSize(vPath) > 1 )
                fprintf( stdout, "STA: critical path: %d objects from an input to PO object %d "
                                 "(%.2f ps).\n", Vec_IntSize(vPath), PathEnd, PathDelay );
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
    /* free the per-literal data that is no longer needed */
    for ( i = 0; i < nLits; i++ )
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
    Vec_FltFree( vArrR ); Vec_FltFree( vArrF );
    Vec_FltFree( vSlewR ); Vec_FltFree( vSlewF );
    Vec_FltFree( vNetCap ); Vec_FltFree( vNetDelay ); Vec_FltFree( vNetLen );
    Vec_FltFree( vInWire );
    Vec_IntFree( vPred );
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
