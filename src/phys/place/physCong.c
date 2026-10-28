/**Function*************************************************************

  Synopsis    [Shared routing congestion and routability model]

  Description [See physCong.h.  The RUDY demand of a net is its HPWL
               spread uniformly over the bins of its bounding box; the
               routing supply of a bin is the total track length the
               routing layers can hold.  The per-object views sample the
               bin that contains the cell center.]

  SideEffects []

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "aig/gia/gia.h"
#include "phys/physMan.h"
#include "phys/lib/physLef.h"
#include "phys/place/physPlace.h"
#include "phys/place/physCong.h"

ABC_NAMESPACE_IMPL_START


/**Function*************************************************************

  Synopsis    [Routing supply of one bin]

  Description [Every routing layer contributes bin_area / pitch of
               wire: the number of tracks that cross the bin in the
               direction perpendicular to the layer's preferred
               direction is (bin side)/pitch, and each of those tracks
               spans the other side of the bin.  When the LEF has no
               routing layers, assume two 0.2 um-pitch layers.]

  SideEffects []

***********************************************************************/

float Phys_CongSupply( Phys_Man_t * p, float Dbu, float BinW, float BinH )
{
    float Cap = 0.0f;
    if ( p->pLef != NULL )
    {
        Phys_LefLayer_t * pLayer;
        int i;
        Vec_PtrForEachEntry( Phys_LefLayer_t *, p->pLef->vLayers, pLayer, i )
        {
            float Pitch;
            int fHoriz;
            if ( !pLayer->fRouting || pLayer->Width <= 0 )
                continue;
            fHoriz = pLayer->pDir != NULL && (pLayer->pDir[0] == 'H' || pLayer->pDir[0] == 'h');
            Pitch  = (float)(fHoriz ? pLayer->PitchY : pLayer->PitchX);
            if ( Pitch <= 0.0f )
                Pitch = 2.0f * (float)pLayer->Width;
            if ( Pitch <= 0.0f )
                continue;
            Cap += BinW * BinH / (Pitch / Dbu);
        }
    }
    if ( Cap <= 0.0f )
        Cap = 2.0f * BinW * BinH / 0.2f;
    return Cap;
}

/* RUDY: spread the HPWL of every net uniformly over the bins of its
   bounding box.  A net that degenerates to a point or a line is widened
   to one cell height, the smallest believable wire bundle. */
static void Phys_CongRudy( float * pX0, float * pY0, float * pX1, float * pY1,
                           float * pHpwl, int nNets, float X0, float Y0,
                           int nBinsX, int nBinsY, float BinW, float BinH,
                           float MinDim, float * pDemand )
{
    int i, bx, by;
    for ( i = 0; i < nNets; i++ )
    {
        float NetX0 = pX0[i], NetX1 = pX1[i];
        float NetY0 = pY0[i], NetY1 = pY1[i];
        float dX, dY, Density;
        int bx0, bx1, by0, by1;
        if ( NetX1 - NetX0 < MinDim )
        {
            float c = 0.5f * (NetX0 + NetX1);
            NetX0 = c - 0.5f * MinDim;
            NetX1 = c + 0.5f * MinDim;
        }
        if ( NetY1 - NetY0 < MinDim )
        {
            float c = 0.5f * (NetY0 + NetY1);
            NetY0 = c - 0.5f * MinDim;
            NetY1 = c + 0.5f * MinDim;
        }
        dX = NetX1 - NetX0;
        dY = NetY1 - NetY0;
        if ( dX <= 0.0f || dY <= 0.0f )
            continue;
        Density = pHpwl[i] / (dX * dY);   /* um of wire per um^2 */
        bx0 = (int)floor( (NetX0 - X0) / BinW );
        bx1 = (int)floor( (NetX1 - X0) / BinW );
        by0 = (int)floor( (NetY0 - Y0) / BinH );
        by1 = (int)floor( (NetY1 - Y0) / BinH );
        if ( bx1 < 0 || by1 < 0 || bx0 >= nBinsX || by0 >= nBinsY )
            continue;
        if ( bx0 < 0 ) bx0 = 0;
        if ( by0 < 0 ) by0 = 0;
        if ( bx1 >= nBinsX ) bx1 = nBinsX - 1;
        if ( by1 >= nBinsY ) by1 = nBinsY - 1;
        for ( by = by0; by <= by1; by++ )
        {
            float Oy0 = Abc_MaxFloat( NetY0, Y0 + by * BinH );
            float Oy1 = Abc_MinFloat( NetY1, Y0 + (by + 1) * BinH );
            for ( bx = bx0; bx <= bx1; bx++ )
            {
                float Ox0 = Abc_MaxFloat( NetX0, X0 + bx * BinW );
                float Ox1 = Abc_MinFloat( NetX1, X0 + (bx + 1) * BinW );
                if ( Ox1 > Ox0 && Oy1 > Oy0 )
                    pDemand[by * nBinsX + bx] += Density * (Ox1 - Ox0) * (Oy1 - Oy0);
            }
        }
    }
}

typedef struct Phys_CongBox_t_
{
    float X0, Y0, X1, Y1;
    int   nSinks;
} Phys_CongBox_t;

static void Phys_CongNetAdd( Phys_CongBox_t * pBox, float * pX, float * pY, int iDriver, int iSink )
{
    Phys_CongBox_t * p = pBox + iDriver;
    if ( pX[iSink] < 0.0f || pY[iSink] < 0.0f )
        return;
    if ( p->nSinks == 0 )
        p->X0 = p->X1 = pX[iSink], p->Y0 = p->Y1 = pY[iSink];
    else
    {
        p->X0 = Abc_MinFloat( p->X0, pX[iSink] );
        p->X1 = Abc_MaxFloat( p->X1, pX[iSink] );
        p->Y0 = Abc_MinFloat( p->Y0, pY[iSink] );
        p->Y1 = Abc_MaxFloat( p->Y1, pY[iSink] );
    }
    p->nSinks++;
}

void Phys_CongFree( Phys_Cong_t ** pp )
{
    Phys_Cong_t * p;
    if ( pp == NULL || *pp == NULL )
        return;
    p = *pp;
    *pp = NULL;
    ABC_FREE( p->pDemand );
    ABC_FREE( p->pCong );
    ABC_FREE( p );
}

Phys_Cong_t * Phys_CongBuild( Phys_Man_t * pMan, Gia_Man_t * pGia, int nGrid )
{
    Phys_Cong_t * pCong;
    Phys_CongBox_t * pBoxes;
    float * pX, * pY, * pArea;
    float Dbu, X0, Y0, W, H, Target, MinDim, BinW, BinH;
    int nObjs, nLits, nBins, i, k, bx, by, nNets = 0, nX, nY;
    if ( pMan == NULL || pGia == NULL || !pMan->fPlaced )
        return NULL;
    nObjs = Gia_ManObjNum( pGia );
    nLits = Phys_ManLitNum( pMan );
    if ( nLits != 2 * nObjs )
        return NULL;
    if ( pMan->vLitX == NULL || pMan->vLitY == NULL ||
         Vec_IntSize(pMan->vLitX) < nLits || Vec_IntSize(pMan->vLitY) < nLits ||
         pMan->vObjToCell == NULL || Vec_PtrSize(pMan->vObjToCell) < nObjs )
        return NULL;
    Dbu = (float)(pMan->pLef ? pMan->pLef->Units.Dbu : 1000);
    if ( Dbu <= 0.0f )
        Dbu = 1000.0f;
    X0 = (float)pMan->CoreX0 / Dbu;
    Y0 = (float)pMan->CoreY0 / Dbu;
    W  = (float)pMan->CoreW / Dbu;
    H  = (float)pMan->CoreH / Dbu;
    if ( W <= 0.0f || H <= 0.0f )
        return NULL;
    /* per-literal coordinates in microns, negative when the instance has none */
    pX = ABC_ALLOC( float, nLits );
    pY = ABC_ALLOC( float, nLits );
    for ( i = 0; i < nLits; i++ )
    {
        int x = Vec_IntEntry( pMan->vLitX, i );
        int y = Vec_IntEntry( pMan->vLitY, i );
        pX[i] = x < 0 ? -1.0f : (float)x / Dbu;
        pY[i] = y < 0 ? -1.0f : (float)y / Dbu;
    }
    /* the bounding box of every net, in the same model as the placer:
       one net per driving literal, with POs as extra sinks */
    pBoxes = ABC_CALLOC( Phys_CongBox_t, nLits );
    for ( i = 2; i < nLits; i++ )
    {
        int nFans;
        if ( !Phys_ManLitIsCell( pMan, i ) )
            continue;
        nFans = Phys_ManLitFaninNum( pMan, pGia, i );
        for ( k = 0; k < nFans; k++ )
        {
            int d = Phys_ManLitFaninLit( pMan, pGia, i, k );
            if ( d > 1 && d < nLits )
                Phys_CongNetAdd( pBoxes, pX, pY, d, i );
        }
    }
    for ( i = 1; i < nObjs; i++ )
    {
        Gia_Obj_t * pObj = Gia_ManObj( pGia, i );
        int d;
        if ( !Gia_ObjIsPo(pGia, pObj) )
            continue;
        d = Phys_ManDriverLit( pMan, pGia, Gia_ObjFaninLit0(pObj, i) );
        if ( d > 1 && d < nLits )
            Phys_CongNetAdd( pBoxes, pX, pY, d, 2*i );
    }
    /* count the nets that have a driver position and at least one sink */
    for ( i = 0; i < nLits; i++ )
        if ( pBoxes[i].nSinks > 0 && pX[i] >= 0.0f && pY[i] >= 0.0f )
            nNets++;
    pCong = ABC_CALLOC( Phys_Cong_t, 1 );
    Target = (W > H ? W : H) / (float)(nGrid < 1 ? 1 : nGrid);
    if ( Target <= 0.0f )
        Target = (W > H ? W : H);
    nX = (int)( W / Target + 0.999f );
    nY = (int)( H / Target + 0.999f );
    if ( nX < 1 ) nX = 1;
    if ( nY < 1 ) nY = 1;
    if ( nX > 2048 ) nX = 2048;
    if ( nY > 2048 ) nY = 2048;
    pCong->nX = nX;
    pCong->nY = nY;
    pCong->X0 = X0;
    pCong->Y0 = Y0;
    pCong->W  = W;
    pCong->H  = H;
    pCong->BinW = BinW = W / (float)nX;
    pCong->BinH = BinH = H / (float)nY;
    nBins = nX * nY;
    pCong->pDemand = ABC_CALLOC( float, nBins );
    pCong->pCong   = ABC_CALLOC( float, nBins );
    pArea          = ABC_CALLOC( float, nBins );
    if ( pCong->pDemand == NULL || pCong->pCong == NULL || pArea == NULL )
    {
        ABC_FREE( pX ); ABC_FREE( pY ); ABC_FREE( pBoxes ); ABC_FREE( pArea );
        Phys_CongFree( &pCong );
        return NULL;
    }
    pCong->Supply = Phys_CongSupply( pMan, Dbu, BinW, BinH );
    MinDim = (float)(pMan->SiteH > 2 * pMan->SiteW ? pMan->SiteH : 2 * pMan->SiteW) / Dbu;
    if ( MinDim < 0.1f )
        MinDim = 0.1f;
    if ( nNets > 0 )
    {
        float * pBx0 = ABC_ALLOC( float, nNets ), * pBy0 = ABC_ALLOC( float, nNets );
        float * pBx1 = ABC_ALLOC( float, nNets ), * pBy1 = ABC_ALLOC( float, nNets );
        float * pHpwl = ABC_ALLOC( float, nNets );
        int n = 0;
        for ( i = 0; i < nLits; i++ )
            if ( pBoxes[i].nSinks > 0 && pX[i] >= 0.0f && pY[i] >= 0.0f )
            {
                pBx0[n] = Abc_MinFloat( pBoxes[i].X0, pX[i] );
                pBx1[n] = Abc_MaxFloat( pBoxes[i].X1, pX[i] );
                pBy0[n] = Abc_MinFloat( pBoxes[i].Y0, pY[i] );
                pBy1[n] = Abc_MaxFloat( pBoxes[i].Y1, pY[i] );
                pHpwl[n] = (pBx1[n] - pBx0[n]) + (pBy1[n] - pBy0[n]);
                n++;
            }
        Phys_CongRudy( pBx0, pBy0, pBx1, pBy1, pHpwl, n, X0, Y0, nX, nY,
                       BinW, BinH, MinDim, pCong->pDemand );
        ABC_FREE( pBx0 ); ABC_FREE( pBy0 ); ABC_FREE( pBx1 ); ABC_FREE( pBy1 );
        ABC_FREE( pHpwl );
    }
    /* cell area per bin, for the density view, over the literal instances */
    for ( i = 1; i < nLits; i++ )
    {
        Phys_LibCell_t * pCell;
        int w, h;
        float Cx0, Cy0, Cx1, Cy1;
        int bx0, bx1, by0, by1;
        if ( (pCell = Phys_ManLitCell( pMan, i )) == NULL || pX[i] < 0.0f )
            continue;
        Phys_PlaceCellSizeCell( pMan, pCell, &w, &h );
        if ( w <= 0 || h <= 0 )
            continue;
        Cx0 = pX[i] - 0.5f * (float)w / Dbu;
        Cy0 = pY[i] - 0.5f * (float)h / Dbu;
        Cx1 = Cx0 + (float)w / Dbu;
        Cy1 = Cy0 + (float)h / Dbu;
        bx0 = (int)floor( (Cx0 - X0) / BinW );
        bx1 = (int)floor( (Cx1 - X0) / BinW );
        by0 = (int)floor( (Cy0 - Y0) / BinH );
        by1 = (int)floor( (Cy1 - Y0) / BinH );
        if ( bx0 < 0 ) bx0 = 0;
        if ( by0 < 0 ) by0 = 0;
        if ( bx1 >= nX ) bx1 = nX - 1;
        if ( by1 >= nY ) by1 = nY - 1;
        for ( by = by0; by <= by1; by++ )
        for ( bx = bx0; bx <= bx1; bx++ )
        {
            float Ox0 = Abc_MaxFloat( Cx0, X0 + bx * BinW );
            float Ox1 = Abc_MinFloat( Cx1, X0 + (bx + 1) * BinW );
            float Oy0 = Abc_MaxFloat( Cy0, Y0 + by * BinH );
            float Oy1 = Abc_MinFloat( Cy1, Y0 + (by + 1) * BinH );
            if ( Ox1 > Ox0 && Oy1 > Oy0 )
                pArea[by * nX + bx] += (Ox1 - Ox0) * (Oy1 - Oy0);
        }
    }
    for ( i = 0; i < nBins; i++ )
    {
        float Cong = pCong->Supply > 0.0f ? pCong->pDemand[i] / pCong->Supply : 0.0f;
        pCong->pCong[i] = Cong;
        if ( Cong > pCong->MaxCong )
            pCong->MaxCong = Cong;
        if ( Cong > 1.0f )
            pCong->nOverflow++;
        pCong->TotalDemand += pCong->pDemand[i];
    }
    pCong->TotalSupply = pCong->Supply * (float)nBins;
    /* per-object views used by the mapper: sample the bin of the primary
       instance of the object and take the worst density of its phases */
    Vec_FltFreeP( &pMan->vObjCong );
    Vec_FltFreeP( &pMan->vObjRout );
    Vec_FltFreeP( &pMan->vObjDens );
    pMan->vObjCong = Vec_FltStart( nObjs );
    pMan->vObjRout = Vec_FltStart( nObjs );
    pMan->vObjDens = Vec_FltStart( nObjs );
    for ( i = 0; i < nObjs; i++ )
    {
        int l0 = 2*i, l1 = l0 + 1;
        int iLit = Phys_ManLitIsCell(pMan, l0) ? l0 : (Phys_ManLitIsCell(pMan, l1) ? l1 : l0);
        float Cong = 0.0f, Rout = 1.0f, Dens = 0.0f;
        if ( pX[iLit] >= 0.0f )
        {
            int bi = (int)floor( (pX[iLit] - X0) / BinW );
            int bj = (int)floor( (pY[iLit] - Y0) / BinH );
            if ( bi < 0 ) bi = 0;
            if ( bj < 0 ) bj = 0;
            if ( bi >= nX ) bi = nX - 1;
            if ( bj >= nY ) bj = nY - 1;
            Cong = pCong->pCong[bj * nX + bi];
            Rout = pCong->pDemand[bj * nX + bi] > 0.0f ?
                   Abc_MinFloat( 1.0f, pCong->Supply / pCong->pDemand[bj * nX + bi] ) : 1.0f;
            Dens = pArea[bj * nX + bi] / (BinW * BinH);
        }
        if ( Phys_ManLitIsCell(pMan, l0) && pX[l0] >= 0.0f )
        {
            int bi = (int)floor( (pX[l0] - X0) / BinW );
            int bj = (int)floor( (pY[l0] - Y0) / BinH );
            if ( bi < 0 ) bi = 0;
            if ( bj < 0 ) bj = 0;
            if ( bi >= nX ) bi = nX - 1;
            if ( bj >= nY ) bj = nY - 1;
            if ( Dens < pArea[bj * nX + bi] / (BinW * BinH) )
                Dens = pArea[bj * nX + bi] / (BinW * BinH);
        }
        if ( Phys_ManLitIsCell(pMan, l1) && pX[l1] >= 0.0f )
        {
            int bi = (int)floor( (pX[l1] - X0) / BinW );
            int bj = (int)floor( (pY[l1] - Y0) / BinH );
            if ( bi < 0 ) bi = 0;
            if ( bj < 0 ) bj = 0;
            if ( bi >= nX ) bi = nX - 1;
            if ( bj >= nY ) bj = nY - 1;
            if ( Dens < pArea[bj * nX + bi] / (BinW * BinH) )
                Dens = pArea[bj * nX + bi] / (BinW * BinH);
        }
        Vec_FltWriteEntry( pMan->vObjCong, i, Cong );
        Vec_FltWriteEntry( pMan->vObjRout, i, Rout );
        Vec_FltWriteEntry( pMan->vObjDens, i, Dens );
    }
    Phys_CongFree( &pMan->pCong );
    pMan->pCong = pCong;
    ABC_FREE( pX );
    ABC_FREE( pY );
    ABC_FREE( pBoxes );
    ABC_FREE( pArea );
    return pCong;
}

ABC_NAMESPACE_IMPL_END
