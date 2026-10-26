/**Function*************************************************************

  Synopsis    [Analytical placement for the physical-synthesis flow]

  Description [See physPlace.h.  Positions are cell centers in LEF
               database units (DBU).]

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
#include "phys/place/physPlace.h"

ABC_NAMESPACE_IMPL_START


typedef struct Phys_PNode_t_
{
    int   Obj;       /* Gia object id                              */
    int   Fixed;     /* PIs and POs are fixed                      */
    int   W, H;      /* size in DBU                                */
    float X, Y;      /* center in DBU                              */
} Phys_PNode_t;

typedef struct Phys_RowCell_t_
{
    float X, Y;
    int   Idx;
} Phys_RowCell_t;

static int Phys_CompareByX( const void * a, const void * b );


void Phys_PlaceSetDefault( Phys_PlacePars_t * p )
{
    p->Util        = 0.60f;
    p->Aspect      = 1.00f;
    p->nIterCG     = 300;
    p->nIterRefine = 20;
    p->fVerbose    = 0;
}

static float Phys_PlaceRand( unsigned * pSeed )
{
    *pSeed = (*pSeed) * 1103515245u + 12345u;
    return (float)((*pSeed >> 8) & 0xFFFF) / 65535.0f;
}

static Phys_LefMacro_t * Phys_PlaceMacro( Phys_Man_t * p, Phys_LibCell_t * pCell )
{
    if ( p->pLef == NULL || pCell->MacroId < 0 || pCell->MacroId >= Vec_PtrSize(p->pLef->vMacros) )
        return NULL;
    return (Phys_LefMacro_t *)Vec_PtrEntry( p->pLef->vMacros, pCell->MacroId );
}

static void Phys_PlaceCellSize( Phys_Man_t * p, Phys_LibCell_t * pCell, int * pw, int * ph )
{
    Phys_LefMacro_t * pMacro = Phys_PlaceMacro( p, pCell );
    if ( pMacro != NULL && pMacro->Width > 0 && pMacro->Height > 0 )
    {
        *pw = pMacro->Width;
        *ph = pMacro->Height;
        return;
    }
    /* fall back to the Liberty area assuming single-row cells */
    {
        double Dbu = (double)(p->pLef ? p->pLef->Units.Dbu : 1000);
        double AreaDbu = (double)(pCell->Area > 0.01f ? pCell->Area : 1.0f) * Dbu * Dbu;
        int nSites = (int)(AreaDbu / ((double)p->SiteW * p->SiteH) + 0.999);
        if ( nSites < 1 ) nSites = 1;
        *pw = nSites * p->SiteW;
        *ph = p->SiteH;
    }
}

/* the size of the cell bound to a Gia object, zero if it is not a cell */
void Phys_PlaceCellSizeObj( Phys_Man_t * p, int iObj, int * pw, int * ph )
{
    Phys_LibCell_t * pCell;
    *pw = *ph = 0;
    if ( p == NULL || p->vObjToCell == NULL || iObj < 0 || iObj >= Vec_PtrSize(p->vObjToCell) )
        return;
    pCell = (Phys_LibCell_t *)Vec_PtrEntry( p->vObjToCell, iObj );
    if ( pCell != NULL )
        Phys_PlaceCellSize( p, pCell, pw, ph );
}

/* returns the site dimensions found in the LEF, or defaults */
static void Phys_PlaceSite( Phys_Man_t * p )
{
    Phys_LefSite_t * pSite = NULL;
    int i;
    if ( p->pLef != NULL )
        Vec_PtrForEachEntry( Phys_LefSite_t *, p->pLef->vSites, pSite, i )
        {
            if ( pSite->Width > 0 && pSite->Height > 0 )
                break;
            pSite = NULL;
        }
    if ( pSite != NULL )
    {
        p->SiteW = pSite->Width;
        p->SiteH = pSite->Height;
    }
    if ( p->SiteW <= 0 ) p->SiteW = 500;
    if ( p->SiteH <= 0 ) p->SiteH = 2400;
}


/**Function*************************************************************

  Synopsis    [Sparse matrix-vector product for the quadratic system]

  Description [Multiplies the symmetric matrix given by its diagonal
               and its off-diagonal (i, j, w) entries by the vector X.]

  SideEffects []

***********************************************************************/
static void Phys_PlaceMatVec( int n, Vec_Flt_t * vDiag, Vec_Int_t * vEI, Vec_Int_t * vEJ,
                              Vec_Flt_t * vEW, Vec_Flt_t * vX, Vec_Flt_t * vY )
{
    int e, i, j;
    Vec_FltFill( vY, n, 0.0f );
    Vec_IntForEachEntryTwo( vEI, vEJ, i, j, e )
    {
        float d = Vec_FltEntry(vEW, e) * (Vec_FltEntry(vX, i) - Vec_FltEntry(vX, j));
        Vec_FltAddToEntry( vY, i,  d );
        Vec_FltAddToEntry( vY, j, -d );
    }
    for ( i = 0; i < n; i++ )
        Vec_FltAddToEntry( vY, i, Vec_FltEntry(vDiag, i) * Vec_FltEntry(vX, i) );
}

static float Phys_PlaceDot( Vec_Flt_t * vA, Vec_Flt_t * vB )
{
    float Sum = 0.0f;
    int i;
    for ( i = 0; i < Vec_FltSize(vA); i++ )
        Sum += Vec_FltEntry(vA, i) * Vec_FltEntry(vB, i);
    return Sum;
}

/**Function*************************************************************

  Synopsis    [Jacobi-preconditioned conjugate gradient]

  Description [Solves A x = b.  The matrix is symmetric positive
               definite, given by its diagonal and its edge list.]

  SideEffects []

***********************************************************************/
static void Phys_PlaceCg( int n, Vec_Flt_t * vDiag, Vec_Int_t * vEI, Vec_Int_t * vEJ,
                          Vec_Flt_t * vEW, Vec_Flt_t * vB, Vec_Flt_t * vX, int nIter )
{
    Vec_Flt_t * vR  = Vec_FltStart( n );
    Vec_Flt_t * vZ  = Vec_FltStart( n );
    Vec_Flt_t * vP  = Vec_FltStart( n );
    Vec_Flt_t * vAp = Vec_FltStart( n );
    float rz, rzNew, alpha, beta;
    int it, i;
    if ( n == 0 )
    {
        Vec_FltFree( vR ); Vec_FltFree( vZ ); Vec_FltFree( vP ); Vec_FltFree( vAp );
        return;
    }
    Phys_PlaceMatVec( n, vDiag, vEI, vEJ, vEW, vX, vAp );
    for ( i = 0; i < n; i++ )
    {
        Vec_FltWriteEntry( vR, i, Vec_FltEntry(vB, i) - Vec_FltEntry(vAp, i) );
        Vec_FltWriteEntry( vZ, i, Vec_FltEntry(vR, i) / Vec_FltEntry(vDiag, i) );
        Vec_FltWriteEntry( vP, i, Vec_FltEntry(vZ, i) );
    }
    rz = Phys_PlaceDot( vR, vZ );
    for ( it = 0; it < nIter; it++ )
    {
        Phys_PlaceMatVec( n, vDiag, vEI, vEJ, vEW, vP, vAp );
        alpha = rz / Phys_PlaceDot( vP, vAp );
        for ( i = 0; i < n; i++ )
        {
            Vec_FltAddToEntry( vX, i,  alpha * Vec_FltEntry(vP, i) );
            Vec_FltAddToEntry( vR, i, -alpha * Vec_FltEntry(vAp, i) );
        }
        if ( Phys_PlaceDot( vR, vR ) < 1e-6f )
            break;
        for ( i = 0; i < n; i++ )
            Vec_FltWriteEntry( vZ, i, Vec_FltEntry(vR, i) / Vec_FltEntry(vDiag, i) );
        rzNew = Phys_PlaceDot( vR, vZ );
        beta  = rzNew / rz;
        rz    = rzNew;
        for ( i = 0; i < n; i++ )
            Vec_FltWriteEntry( vP, i, Vec_FltEntry(vZ, i) + beta * Vec_FltEntry(vP, i) );
    }
    Vec_FltFree( vR ); Vec_FltFree( vZ ); Vec_FltFree( vP ); Vec_FltFree( vAp );
}

/**Function*************************************************************

  Synopsis    [Solves a wirelength problem with a quadratic anchor]

  Description [Solves (L + Lambda I) x = b_wl + Lambda x_ref, where L is
               the wirelength Laplacian, b_wl collects the contributions
               of the fixed terminals, and x_ref is the reference
               placement.  The anchor weight Lambda controls how far the
               result may move away from the reference: with a small
               Lambda the result is the plain quadratic solution, with a
               large one it stays near the reference.  Interleaving this
               solve with density spreading realizes a proximal-gradient
               iteration on wirelength plus a density penalty.]

  SideEffects []

***********************************************************************/
static void Phys_PlaceSolve( int n, Vec_Flt_t * vDiag, Vec_Int_t * vEI, Vec_Int_t * vEJ,
                             Vec_Flt_t * vEW, Vec_Flt_t * vB, Vec_Flt_t * vRef,
                             float Lambda, Vec_Flt_t * vX, int nIter )
{
    Vec_Flt_t * vDiagE, * vBe;
    int i;
    if ( n == 0 )
        return;
    vDiagE = Vec_FltStart( n );
    vBe    = Vec_FltStart( n );
    for ( i = 0; i < n; i++ )
    {
        Vec_FltWriteEntry( vDiagE, i, Vec_FltEntry(vDiag, i) + Lambda );
        Vec_FltWriteEntry( vBe, i, Vec_FltEntry(vB, i) + Lambda * Vec_FltEntry(vRef, i) );
    }
    Phys_PlaceCg( n, vDiagE, vEI, vEJ, vEW, vBe, vX, nIter );
    Vec_FltFree( vDiagE );
    Vec_FltFree( vBe );
}


/**Function*************************************************************

  Synopsis    [Density spreading of the quadratic solution]

  Description [The quadratic solver ignores cell sizes, so it packs the
               cells into a small fraction of the core, where the density
               is far above what legalization can handle.  This step turns
               the solution into a placement of uniform density without
               changing the order of the cells along either axis: the cells
               are sorted by X, the cumulative cell width is mapped onto the
               core width, and the same is done for Y.  Because the order is
               preserved the solve's connectivity information survives, but
               the under-dispersion and the clustering near the inputs are
               removed, and the legalizer then has almost nothing to fix.
               The result is written back to the solution vectors, which are
               also the reference for the refinement.]

  SideEffects []

***********************************************************************/
static void Phys_PlaceScale( Phys_PNode_t * pNodes, Vec_Int_t * vFree, Vec_Flt_t * vSolX,
                             Vec_Flt_t * vSolY, float x0, float y0, float W, float H,
                             double TotalArea, float Util )
{
    Phys_RowCell_t * pX, * pY;
    double SumW = 0.0;
    int n = Vec_IntSize( vFree ), k, i;
    (void)TotalArea;
    (void)Util;
    if ( n == 0 )
        return;
    pX = ABC_ALLOC( Phys_RowCell_t, n );
    pY = ABC_ALLOC( Phys_RowCell_t, n );
    Vec_IntForEachEntry( vFree, i, k )
    {
        pX[k].X = pNodes[i].X; pX[k].Idx = i;
        pY[k].X = pNodes[i].Y; pY[k].Idx = i;
        SumW += (double)pNodes[i].W;
    }
    if ( SumW <= 0.0 )
    {
        ABC_FREE( pX ); ABC_FREE( pY );
        return;
    }
    qsort( pX, n, sizeof(Phys_RowCell_t), Phys_CompareByX );
    qsort( pY, n, sizeof(Phys_RowCell_t), Phys_CompareByX );
    /* Re-place the cells so that the cumulative width is a uniform
       function of the coordinate, keeping the order of the solution along
       each axis.  This is a monotone warp: the cells that were neighbors
       stay neighbors, but the density, which the quadratic solver leaves
       concentrated near the inputs, becomes uniform.  The vertical warp
       uses the width rather than the height as its weight: all cells are
       one row tall, so a row's occupancy is the width it holds, and
       balancing width per unit of height balances the row occupancy. */
    {
        double Cur = 0.0;
        for ( k = 0; k < n; k++ )
        {
            i = pX[k].Idx;
            pNodes[i].X = x0 + (float)( W * (Cur + 0.5 * pNodes[i].W) / SumW );
            Cur += (double)pNodes[i].W;
        }
        Cur = 0.0;
        for ( k = 0; k < n; k++ )
        {
            i = pY[k].Idx;
            pNodes[i].Y = y0 + (float)( H * (Cur + 0.5 * pNodes[i].W) / SumW );
            Cur += (double)pNodes[i].W;
        }
    }
    Vec_IntForEachEntry( vFree, i, k )
    {
        Vec_FltWriteEntry( vSolX, k, pNodes[i].X );
        Vec_FltWriteEntry( vSolY, k, pNodes[i].Y );
    }
    ABC_FREE( pX ); ABC_FREE( pY );
}


static int Phys_CompareByX( const void * a, const void * b )
{
    float x0 = ((const Phys_RowCell_t *)a)->X;
    float x1 = ((const Phys_RowCell_t *)b)->X;
    if ( x0 < x1 ) return -1;
    if ( x0 > x1 ) return  1;
    return 0;
}

/* half-perimeter wirelength of the current node positions, in DBU */
static double Phys_PlaceHpwl( Phys_PNode_t * pNodes, Vec_Int_t * vNode2Idx, Vec_Ptr_t * vSinks, int nObjs )
{
    double TotalHpwl = 0.0;
    int q, k, Entry;
    for ( q = 0; q < nObjs; q++ )
    {
        Vec_Int_t * vList = (Vec_Int_t *)Vec_PtrEntry( vSinks, q );
        int iDriver;
        float xMin, xMax, yMin, yMax;
        if ( vList == NULL )
            continue;
        iDriver = Vec_IntEntry( vNode2Idx, q );
        if ( iDriver < 0 )
            continue;
        xMin = xMax = pNodes[iDriver].X;
        yMin = yMax = pNodes[iDriver].Y;
        Vec_IntForEachEntry( vList, Entry, k )
        {
            xMin = Abc_MinFloat( xMin, pNodes[Entry].X );
            xMax = Abc_MaxFloat( xMax, pNodes[Entry].X );
            yMin = Abc_MinFloat( yMin, pNodes[Entry].Y );
            yMax = Abc_MaxFloat( yMax, pNodes[Entry].Y );
        }
        TotalHpwl += (double)(xMax - xMin) + (double)(yMax - yMin);
    }
    return TotalHpwl;
}
/* wirelength of the internal (free-to-free) nets only, split into x and y */
static void Phys_PlaceHpwlInternal( Phys_PNode_t * pNodes, Vec_Int_t * vNode2Idx, Vec_Int_t * vFreeIdx,
                                    Vec_Ptr_t * vSinks, int nObjs, double * pX, double * pY )
{
    double HpwlX = 0.0, HpwlY = 0.0;
    int q, k, Entry;
    for ( q = 0; q < nObjs; q++ )
    {
        Vec_Int_t * vList = (Vec_Int_t *)Vec_PtrEntry( vSinks, q );
        int iDriver, fDriver, fSink, fAnyFixed = 0;
        float xMin, xMax, yMin, yMax;
        if ( vList == NULL )
            continue;
        iDriver = Vec_IntEntry( vNode2Idx, q );
        if ( iDriver < 0 )
            continue;
        fDriver = Vec_IntEntry( vFreeIdx, iDriver );
        if ( fDriver < 0 )
            continue;
        xMin = xMax = pNodes[iDriver].X;
        yMin = yMax = pNodes[iDriver].Y;
        Vec_IntForEachEntry( vList, Entry, k )
        {
            fSink = Vec_IntEntry( vFreeIdx, Entry );
            if ( fSink < 0 )
            {
                fAnyFixed = 1;
                break;
            }
            xMin = Abc_MinFloat( xMin, pNodes[Entry].X );
            xMax = Abc_MaxFloat( xMax, pNodes[Entry].X );
            yMin = Abc_MinFloat( yMin, pNodes[Entry].Y );
            yMax = Abc_MaxFloat( yMax, pNodes[Entry].Y );
        }
        if ( fAnyFixed )
            continue;
        HpwlX += (double)(xMax - xMin);
        HpwlY += (double)(yMax - yMin);
    }
    *pX = HpwlX;
    *pY = HpwlY;
}
/* wirelength of the internal (free-to-free) nets only */
static double Phys_PlaceHpwlFree( Phys_PNode_t * pNodes, Vec_Int_t * vNode2Idx, Vec_Int_t * vFreeIdx,
                                  Vec_Ptr_t * vSinks, int nObjs )
{
    double HpwlX = 0.0, HpwlY = 0.0;
    Phys_PlaceHpwlInternal( pNodes, vNode2Idx, vFreeIdx, vSinks, nObjs, &HpwlX, &HpwlY );
    return HpwlX + HpwlY;
}
/* extent of the free node positions, for diagnostics */
static void Phys_PlaceExtent( Phys_PNode_t * pNodes, Vec_Int_t * vFree, float * pMinX, float * pMaxX, float * pMinY, float * pMaxY )
{
    int k, i;
    *pMinX = *pMinY = 1e30f;
    *pMaxX = *pMaxY = -1e30f;
    Vec_IntForEachEntry( vFree, i, k )
    {
        *pMinX = Abc_MinFloat( *pMinX, pNodes[i].X );
        *pMaxX = Abc_MaxFloat( *pMaxX, pNodes[i].X );
        *pMinY = Abc_MinFloat( *pMinY, pNodes[i].Y );
        *pMaxY = Abc_MaxFloat( *pMaxY, pNodes[i].Y );
    }
}

static void Phys_PlaceReport( const char * pStage, Phys_PNode_t * pNodes, Vec_Int_t * vFree,
                              Vec_Int_t * vNode2Idx, Vec_Int_t * vFreeIdx, Vec_Ptr_t * vSinks,
                              int nObjs, float Dbu )
{
    double IntX, IntY;
    float MinX, MaxX, MinY, MaxY;
    Phys_PlaceExtent( pNodes, vFree, &MinX, &MaxX, &MinY, &MaxY );
    Phys_PlaceHpwlInternal( pNodes, vNode2Idx, vFreeIdx, vSinks, nObjs, &IntX, &IntY );
    fprintf( stdout, "Placement: %-11s internal hpwl %8.1f um (x %8.1f, y %8.1f), "
             "extent x [%6.1f, %6.1f] y [%6.1f, %6.1f] um.\n",
             pStage, (IntX + IntY) / Dbu, IntX / Dbu, IntY / Dbu,
             MinX / Dbu, MaxX / Dbu, MinY / Dbu, MaxY / Dbu );
}


/**Function*************************************************************

  Synopsis    [Legalizes the cells of one row]

  Description [Assigns non-overlapping x positions to the cells of a
               row.  The cells are sorted by their desired x coordinate
               and first pushed right to remove overlaps, then pushed
               left again from the right edge.  When there is enough
               room, a cell whose desired position fits keeps it, so the
               spreading solution is preserved as much as possible.  The
               cells that cannot fit in this row are returned in
               vUnplaced.  Returns the total width occupied by the row.]

  SideEffects []

***********************************************************************/
static float Phys_PlaceLegalizeRow( Vec_Int_t * vRow, Phys_PNode_t * pNodes, float rowY,
                                    float x0, float W, Vec_Int_t * vUnplaced )
{
    Phys_RowCell_t * pCells;
    float Cursor, Used = 0.0f;
    int n = Vec_IntSize( vRow );
    int k, i;
    if ( n == 0 )
        return 0.0f;
    pCells = ABC_ALLOC( Phys_RowCell_t, n );
    for ( k = 0; k < n; k++ )
    {
        i = Vec_IntEntry( vRow, k );
        pCells[k].X = pNodes[i].X;
        pCells[k].Idx = i;
    }
    qsort( pCells, n, sizeof(Phys_RowCell_t), Phys_CompareByX );
    /* forward pass: keep the desired x if it does not overlap the left neighbor */
    Cursor = x0;
    for ( k = 0; k < n; k++ )
    {
        i = pCells[k].Idx;
        if ( pNodes[i].X < Cursor + 0.5f * pNodes[i].W )
            pNodes[i].X = Cursor + 0.5f * pNodes[i].W;
        Cursor = pNodes[i].X + 0.5f * pNodes[i].W;
    }
    /* backward pass: push back from the right edge */
    Cursor = x0 + W;
    for ( k = n - 1; k >= 0; k-- )
    {
        i = pCells[k].Idx;
        if ( pNodes[i].X > Cursor - 0.5f * pNodes[i].W )
            pNodes[i].X = Cursor - 0.5f * pNodes[i].W;
        Cursor = pNodes[i].X - 0.5f * pNodes[i].W;
    }
    /* record the result; keep the row only if the cells fit */
    Vec_IntClear( vRow );
    for ( k = 0; k < n; k++ )
    {
        i = pCells[k].Idx;
        pNodes[i].Y = rowY;
        if ( pNodes[i].X - 0.5f * pNodes[i].W < x0 - 0.5f )
            Vec_IntPush( vUnplaced, i );
        else
            Vec_IntPush( vRow, i ), Used += (float)pNodes[i].W;
    }
    ABC_FREE( pCells );
    return Used;
}

/**Function*************************************************************

  Synopsis    [Row legalization]

  Description [Assigns every cell to the row nearest to its y position
               and resolves the horizontal overlaps within each row.  A
               cell that does not fit in its own row is moved to the
               nearest row with enough free width.  Unlike a global
               re-packing of the rows, this keeps the cells close to the
               positions produced by the spreading step, and therefore
               preserves the wirelength structure of the solution.]

  SideEffects []

***********************************************************************/
static int Phys_PlaceLegalize( Phys_Man_t * p, Phys_PNode_t * pNodes, Vec_Int_t * vFree,
                               float x0, float y0, float W, float H, int fVerbose )
{
    int nRows = (int)(H / p->SiteH);
    int n = Vec_IntSize( vFree );
    Vec_Ptr_t * vRows;
    Vec_Int_t * vUnplaced, * vNext, * vRow2;
    float * pUsed;
    float * pOldX, * pOldY;
    double SumDx = 0.0, SumDy = 0.0, MaxDx = 0.0, MaxDy = 0.0;
    int iNode, f, k, r, Pass, nOverlaps = 0;
    if ( nRows < 1 ) nRows = 1;
    if ( n == 0 )
        return 0;
    pOldX = ABC_ALLOC( float, n );
    pOldY = ABC_ALLOC( float, n );
    Vec_IntForEachEntry( vFree, iNode, f )
    {
        pOldX[f] = pNodes[iNode].X;
        pOldY[f] = pNodes[iNode].Y;
    }
    vRows = Vec_PtrAlloc( nRows );
    for ( r = 0; r < nRows; r++ )
        Vec_PtrPush( vRows, Vec_IntAlloc( 16 ) );
    /* assign each cell to the row that contains its y position */
    Vec_IntForEachEntry( vFree, iNode, f )
    {
        r = (int)floor( (pNodes[iNode].Y - y0) / (float)p->SiteH );
        if ( r < 0 ) r = 0;
        if ( r >= nRows ) r = nRows - 1;
        Vec_IntPush( (Vec_Int_t *)Vec_PtrEntry(vRows, r), iNode );
    }
    pUsed     = ABC_CALLOC( float, nRows );
    vUnplaced = Vec_IntAlloc( 64 );
    vNext     = Vec_IntAlloc( 64 );
    if ( fVerbose )
    {
        float Dbu = p->pLef ? (float)p->pLef->Units.Dbu : 1000.0f;
        double SumW = 0.0;
        Vec_IntForEachEntry( vFree, iNode, f )
            SumW += (double)pNodes[iNode].W;
        fprintf( stdout, "Placement: legalize: %d cells, total width %.1f um, "
                 "%d rows of %.1f um, capacity %.1f um (%.0f%% used), core %.1f x %.1f um.\n",
                 n, SumW / Dbu, nRows, W / Dbu, (double)nRows * W / Dbu,
                 100.0 * SumW / ((double)nRows * W), W / Dbu, H / Dbu );
    }
    for ( Pass = 0; Pass < 4; Pass++ )
    {
        int nOverfull = 0;
        Vec_IntClear( vUnplaced );
        if ( fVerbose )
            for ( r = 0; r < nRows; r++ )
            {
                float RowW = 0.0f;
                Vec_Int_t * vRowTmp = (Vec_Int_t *)Vec_PtrEntry(vRows, r);
                Vec_IntForEachEntry( vRowTmp, iNode, k )
                    RowW += (float)pNodes[iNode].W;
                if ( RowW > W )
                    nOverfull++;
            }
        for ( r = 0; r < nRows; r++ )
            pUsed[r] = Phys_PlaceLegalizeRow( (Vec_Int_t *)Vec_PtrEntry(vRows, r), pNodes,
                                              y0 + r * p->SiteH + 0.5f * p->SiteH, x0, W, vUnplaced );
        if ( fVerbose )
            fprintf( stdout, "Placement: legalize pass %d: %d/%d rows overfull, %d cells unplaced.\n",
                     Pass, nOverfull, nRows, Vec_IntSize(vUnplaced) );
        if ( Vec_IntSize(vUnplaced) == 0 )
            break;
        /* move the cells that did not fit to the nearest row with room */
        Vec_IntClear( vNext );
        Vec_IntForEachEntry( vUnplaced, iNode, k )
        {
            int rOrig = (int)floor( (pNodes[iNode].Y - y0) / (float)p->SiteH );
            int Best = -1, BestDist = 1 << 30, Dist;
            if ( rOrig < 0 ) rOrig = 0;
            if ( rOrig >= nRows ) rOrig = nRows - 1;
            for ( r = 0; r < nRows; r++ )
            {
                if ( pUsed[r] + pNodes[iNode].W > W + 0.5f )
                    continue;
                Dist = r > rOrig ? r - rOrig : rOrig - r;
                if ( Dist < BestDist )
                    BestDist = Dist, Best = r;
            }
            if ( Best < 0 )
                continue;   /* no row has room; will be placed with overlap */
            Vec_IntPush( (Vec_Int_t *)Vec_PtrEntry(vRows, Best), iNode );
            pUsed[Best] += (float)pNodes[iNode].W;
            Vec_IntPush( vNext, iNode );
        }
        if ( Vec_IntSize(vNext) == 0 )
            break;
    }
    /* any cell still unplaced is put at the right edge of its row */
    Vec_IntForEachEntry( vUnplaced, iNode, k )
    {
        r = (int)floor( (pNodes[iNode].Y - y0) / (float)p->SiteH );
        if ( r < 0 ) r = 0;
        if ( r >= nRows ) r = nRows - 1;
        pNodes[iNode].X = x0 + W - 0.5f * pNodes[iNode].W;
        pNodes[iNode].Y = y0 + r * p->SiteH + 0.5f * p->SiteH;
        nOverlaps++;
    }
    if ( fVerbose )
    {
        int nUsedRows = 0;
        for ( r = 0; r < nRows; r++ )
            if ( Vec_IntSize( (Vec_Int_t *)Vec_PtrEntry(vRows, r) ) > 0 )
                nUsedRows++;
        Vec_IntForEachEntry( vFree, iNode, f )
        {
            double Dx = fabs( pNodes[iNode].X - pOldX[f] );
            double Dy = fabs( pNodes[iNode].Y - pOldY[f] );
            SumDx += Dx; SumDy += Dy;
            MaxDx = MaxDx > Dx ? MaxDx : Dx;
            MaxDy = MaxDy > Dy ? MaxDy : Dy;
        }
        {
            float Dbu = p->pLef ? (float)p->pLef->Units.Dbu : 1000.0f;
            fprintf( stdout, "Placement: legalize: %d of %d rows used; displacement mean %.2f x %.2f, max %.2f x %.2f um.\n",
                     nUsedRows, nRows, SumDx / n / Dbu, SumDy / n / Dbu, MaxDx / Dbu, MaxDy / Dbu );
        }
    }
    Vec_PtrForEachEntry( Vec_Int_t *, vRows, vRow2, r )
        Vec_IntFree( vRow2 );
    Vec_PtrFree( vRows );
    Vec_IntFree( vUnplaced );
    Vec_IntFree( vNext );
    ABC_FREE( pUsed );
    ABC_FREE( pOldX );
    ABC_FREE( pOldY );
    return nOverlaps;
}

/**Function*************************************************************

  Synopsis    [Maximum row occupancy of a placement]

  Description [Computes the largest total cell width of any row, relative
               to the row width.  A value above 1.0 means that the row
               cannot hold its cells and legalization has to spill them,
               so it is a direct measure of how far the placement is from
               being legalizable.]

  SideEffects []

***********************************************************************/
static float Phys_PlaceMaxRowOcc( Phys_PNode_t * pNodes, Vec_Int_t * vFree,
                                  float y0, float H, int SiteH, float W )
{
    float * pUsed;
    float MaxOcc = 0.0f;
    int nRows = (int)(H / SiteH), iNode, f, r;
    if ( nRows < 1 || W <= 0.0f )
        return 0.0f;
    pUsed = ABC_CALLOC( float, nRows );
    Vec_IntForEachEntry( vFree, iNode, f )
    {
        r = (int)floor( (pNodes[iNode].Y - y0) / (float)SiteH );
        if ( r < 0 ) r = 0;
        if ( r >= nRows ) r = nRows - 1;
        pUsed[r] += pNodes[iNode].W;
    }
    for ( r = 0; r < nRows; r++ )
        MaxOcc = Abc_MaxFloat( MaxOcc, pUsed[r] / W );
    ABC_FREE( pUsed );
    return MaxOcc;
}

/**Function*************************************************************

  Synopsis    [Number of rows that hold at least one cell]

  Description [Together with the maximum row occupancy this tells how
               evenly the cells use the core height: a placement that
               concentrates in part of the rows has a higher density
               there than the target utilization suggests.]

  SideEffects []

***********************************************************************/
static int Phys_PlaceRowsUsed( Phys_PNode_t * pNodes, Vec_Int_t * vFree,
                               float y0, float H, int SiteH )
{
    char * pUsed;
    int nRows = (int)(H / SiteH), iNode, f, r, nUsed = 0;
    if ( nRows < 1 )
        return 0;
    pUsed = ABC_CALLOC( char, nRows );
    Vec_IntForEachEntry( vFree, iNode, f )
    {
        r = (int)floor( (pNodes[iNode].Y - y0) / (float)SiteH );
        if ( r < 0 ) r = 0;
        if ( r >= nRows ) r = nRows - 1;
        pUsed[r] = 1;
    }
    for ( r = 0; r < nRows; r++ )
        nUsed += pUsed[r];
    ABC_FREE( pUsed );
    return nUsed;
}


/**Function*************************************************************

  Synopsis    [Runs analytical placement]

  Description [Returns 1 on success and 0 on failure.]

  SideEffects []

***********************************************************************/
int Phys_PlaceRun( Phys_Man_t * p, Gia_Man_t * pGia, Phys_PlacePars_t * pPars )
{
    int nObjs = Gia_ManObjNum( pGia );
    Phys_PNode_t * pNodes;
    Vec_Int_t * vNode2Idx, * vFreeIdx, * vFree;
    Vec_Ptr_t * vSinks;
    Vec_Int_t * vEI, * vEJ;
    Vec_Flt_t * vEW, * vDiag, * vBx, * vBy, * vSolX, * vSolY, * vRefX, * vRefY;
    double TotalArea = 0.0;
    float x0, y0, W, H, Dbu, Anchor, AvgDiag;
    int i, k, q, nNodes = 0, nFree = 0, nEdges = 0, nOverlaps;
    unsigned Seed = 12345;
    if ( p == NULL || pGia == NULL )
        return 0;
    if ( p->vObjToCell == NULL || Vec_PtrSize(p->vObjToCell) < nObjs )
    {
        fprintf( stdout, "Placement: the network has not been bound. Run \"&phys_bind\" first.\n" );
        return 0;
    }
    if ( p->pLef == NULL )
        fprintf( stdout, "Placement: no LEF data; using default site and RC assumptions.\n" );
    Phys_PlaceSite( p );
    Dbu = (float)(p->pLef ? p->pLef->Units.Dbu : 1000);
    pNodes    = ABC_CALLOC( Phys_PNode_t, nObjs );
    vNode2Idx = Vec_IntStartFull( nObjs );
    vFreeIdx  = Vec_IntAlloc( 1024 );
    vFree     = Vec_IntAlloc( 1024 );
    vSinks    = Vec_PtrStart( nObjs );
    /* create nodes */
    for ( i = 1; i < nObjs; i++ )
    {
        Phys_LibCell_t * pCell = (Phys_LibCell_t *)Vec_PtrEntry( p->vObjToCell, i );
        Phys_PNode_t * pNode = pNodes + nNodes;
        if ( pCell == NULL && !Gia_ObjIsPi(pGia, Gia_ManObj(pGia, i)) &&
                              !Gia_ObjIsPo(pGia, Gia_ManObj(pGia, i)) )
            continue;
        pNode->Obj = i;
        if ( pCell != NULL )
        {
            Phys_PlaceCellSize( p, pCell, &pNode->W, &pNode->H );
            pNode->Fixed = 0;
            TotalArea += (double)pNode->W * (double)pNode->H;
            Vec_IntPush( vFree, nNodes );
        }
        else
        {
            pNode->W = 0;
            pNode->H = 0;
            pNode->Fixed = 1;
        }
        Vec_IntWriteEntry( vNode2Idx, i, nNodes );
        nNodes++;
    }
    nFree = Vec_IntSize( vFree );
    if ( nFree == 0 || TotalArea <= 0.0 )
    {
        fprintf( stdout, "Placement: there are no mapped cells to place.\n" );
        ABC_FREE( pNodes );
        Vec_IntFree( vNode2Idx );
        Vec_IntFree( vFreeIdx );
        Vec_IntFree( vFree );
        Vec_PtrFree( vSinks );
        return 0;
    }
    Vec_IntGrow( vFreeIdx, nNodes );
    Vec_IntFill( vFreeIdx, nNodes, -1 );
    Vec_IntForEachEntry( vFree, i, k )
        Vec_IntWriteEntry( vFreeIdx, i, k );
    /* core area */
    {
        double CoreArea = TotalArea / (double)(pPars->Util > 0.01f ? pPars->Util : 0.6f);
        float Aspect = pPars->Aspect > 0.01f ? pPars->Aspect : 1.0f;
        W = (float)sqrt( CoreArea / Aspect );
        H = Aspect * W;
        W = (float)(int)( W / p->SiteW + 0.999 ) * p->SiteW;
        H = (float)(int)( H / p->SiteH + 0.999 ) * p->SiteH;
        if ( W < 10 * p->SiteW ) W = 10 * p->SiteW;
        if ( H < 4  * p->SiteH ) H = 4  * p->SiteH;
        x0 = 0.0f;
        y0 = 0.0f;
    }
    /* initial positions */
    for ( i = 0; i < nNodes; i++ )
    {
        Phys_PNode_t * pNode = pNodes + i;
        if ( pNode->Fixed )
            continue;
        pNode->X = x0 + 0.5f*pNode->W + Phys_PlaceRand(&Seed) * (W - pNode->W);
        pNode->Y = y0 + 0.5f*pNode->H + Phys_PlaceRand(&Seed) * (H - pNode->H);
    }
    /* PIs on the left, POs on the right */
    {
        int nPi = 0, nPo = 0, nPiTot = 0, nPoTot = 0;
        for ( i = 0; i < nNodes; i++ )
        {
            if ( Gia_ObjIsPi(pGia, Gia_ManObj(pGia, pNodes[i].Obj)) ) nPiTot++;
            if ( Gia_ObjIsPo(pGia, Gia_ManObj(pGia, pNodes[i].Obj)) ) nPoTot++;
        }
        for ( i = 0; i < nNodes; i++ )
        {
            Phys_PNode_t * pNode = pNodes + i;
            if ( Gia_ObjIsPi(pGia, Gia_ManObj(pGia, pNode->Obj)) )
                pNode->X = x0, pNode->Y = y0 + H * (0.5f + nPi++) / (nPiTot + 1);
            else if ( Gia_ObjIsPo(pGia, Gia_ManObj(pGia, pNode->Obj)) )
                pNode->X = x0 + W, pNode->Y = y0 + H * (0.5f + nPo++) / (nPoTot + 1);
        }
    }
    /* collect nets: vSinks[d] lists the sink nodes driven by object d */
    for ( i = 0; i < nNodes; i++ )
    {
        int iObj = pNodes[i].Obj;
        Gia_Obj_t * pObj = Gia_ManObj( pGia, iObj );
        if ( Gia_ObjIsPo(pGia, pObj) )
        {
            int d = Abc_Lit2Var( Gia_ObjFaninLit0(pObj, iObj) );
            Vec_Int_t * vList;
            if ( d <= 0 || Vec_IntEntry(vNode2Idx, d) < 0 )
                continue;
            vList = (Vec_Int_t *)Vec_PtrEntry( vSinks, d );
            if ( vList == NULL )
                Vec_PtrWriteEntry( vSinks, d, vList = Vec_IntAlloc(4) );
            Vec_IntPush( vList, i );
            continue;
        }
        if ( Vec_PtrEntry(p->vObjToCell, iObj) == NULL )
            continue;
        {
            int iLit = Vec_IntEntry( p->vObjCellLit, iObj );
            if ( iLit < 0 )
                continue;
            for ( k = 0; k < Gia_ObjCellSize(pGia, iLit); k++ )
            {
                int d = Abc_Lit2Var( Gia_ObjCellFanin(pGia, iLit, k) );
                Vec_Int_t * vList;
                if ( d <= 0 || Vec_IntEntry(vNode2Idx, d) < 0 )
                    continue;
                vList = (Vec_Int_t *)Vec_PtrEntry( vSinks, d );
                if ( vList == NULL )
                    Vec_PtrWriteEntry( vSinks, d, vList = Vec_IntAlloc(4) );
                Vec_IntPush( vList, i );
            }
        }
    }
    /* build the quadratic system over free nodes */
    vEI = Vec_IntAlloc( 1024 );
    vEJ = Vec_IntAlloc( 1024 );
    vEW = Vec_FltAlloc( 1024 );
    vDiag = Vec_FltStart( nFree );
    vBx   = Vec_FltStart( nFree );
    vBy   = Vec_FltStart( nFree );
    {
        Vec_Int_t * vPinIdx = Vec_IntAlloc( 16 );
        for ( q = 0; q < nObjs; q++ )
        {
            Vec_Int_t * vList = (Vec_Int_t *)Vec_PtrEntry( vSinks, q );
            int iDriver, nPins, a, b;
            float w;
            if ( vList == NULL )
                continue;
            iDriver = Vec_IntEntry( vNode2Idx, q );
            if ( iDriver < 0 )
                continue;
            Vec_IntClear( vPinIdx );
            Vec_IntPush( vPinIdx, iDriver );
            Vec_IntAppend( vPinIdx, vList );
            nPins = Vec_IntSize( vPinIdx );
            if ( nPins < 2 )
                continue;
            w = 1.0f / (float)nPins;
            for ( a = 0; a < nPins; a++ )
            for ( b = a + 1; b < nPins; b++ )
            {
                int ia = Vec_IntEntry( vPinIdx, a );
                int ib = Vec_IntEntry( vPinIdx, b );
                int fa = ia >= 0 ? Vec_IntEntry( vFreeIdx, ia ) : -1;
                int fb = ib >= 0 ? Vec_IntEntry( vFreeIdx, ib ) : -1;
                if ( fa >= 0 && fb >= 0 )
                {
                    Vec_IntPush( vEI, fa );
                    Vec_IntPush( vEJ, fb );
                    Vec_FltPush( vEW, w );
                    Vec_FltAddToEntry( vDiag, fa, w );
                    Vec_FltAddToEntry( vDiag, fb, w );
                    nEdges++;
                }
                else if ( fa >= 0 )
                {
                    Vec_FltAddToEntry( vDiag, fa, w );
                    Vec_FltAddToEntry( vBx,   fa, w * pNodes[ia].X );
                    Vec_FltAddToEntry( vBy,   fa, w * pNodes[ia].Y );
                }
                else if ( fb >= 0 )
                {
                    Vec_FltAddToEntry( vDiag, fb, w );
                    Vec_FltAddToEntry( vBx,   fb, w * pNodes[ib].X );
                    Vec_FltAddToEntry( vBy,   fb, w * pNodes[ib].Y );
                }
            }
        }
        Vec_IntFree( vPinIdx );
    }
    vSolX = Vec_FltStart( nFree );
    vSolY = Vec_FltStart( nFree );
    vRefX = Vec_FltStart( nFree );
    vRefY = Vec_FltStart( nFree );
    {
        float Avg = 0.0f;
        for ( k = 0; k < nFree; k++ )
            Avg += Vec_FltEntry( vDiag, k );
        AvgDiag = Avg / (float)nFree + 1.0f;
        /* just enough anchoring to make the system non-singular */
        Anchor = 1e-3f * AvgDiag;
    }
    for ( k = 0; k < nFree; k++ )
    {
        Vec_FltWriteEntry( vRefX, k, x0 + 0.5f * W );
        Vec_FltWriteEntry( vRefY, k, y0 + 0.5f * H );
    }
    if ( pPars->fVerbose )
        fprintf( stdout, "Placement: %d nodes (%d free), %d quadratic edges, core %.1f x %.1f um.\n",
                 nNodes, nFree, nEdges, W / Dbu, H / Dbu );
    if ( pPars->fVerbose )
        Phys_PlaceReport( "random init:", pNodes, vFree, vNode2Idx, vFreeIdx, vSinks, nObjs, Dbu );
    Phys_PlaceSolve( nFree, vDiag, vEI, vEJ, vEW, vBx, vRefX, Anchor, vSolX, pPars->nIterCG );
    Phys_PlaceSolve( nFree, vDiag, vEI, vEJ, vEW, vBy, vRefY, Anchor, vSolY, pPars->nIterCG );
    Vec_IntForEachEntry( vFree, i, k )
    {
        pNodes[i].X = Vec_FltEntry( vSolX, k );
        pNodes[i].Y = Vec_FltEntry( vSolY, k );
    }
    if ( pPars->fVerbose )
        Phys_PlaceReport( "after CG:", pNodes, vFree, vNode2Idx, vFreeIdx, vSinks, nObjs, Dbu );
    Phys_PlaceScale( pNodes, vFree, vSolX, vSolY, x0, y0, W, H, TotalArea, pPars->Util );
    if ( pPars->fVerbose )
        Phys_PlaceReport( "after scale:", pNodes, vFree, vNode2Idx, vFreeIdx, vSinks, nObjs, Dbu );
    /* Legalize the equalized solution. */
    nOverlaps = Phys_PlaceLegalize( p, pNodes, vFree, x0, y0, W, H, pPars->fVerbose );
    if ( pPars->fVerbose )
        Phys_PlaceReport( "after legal:", pNodes, vFree, vNode2Idx, vFreeIdx, vSinks, nObjs, Dbu );
    /* Wirelength refinement.  Spreading alone cannot recover the
       wirelength that the density constraint destroys, and solving only
       for wirelength collapses the placement back into the compact
       quadratic blob, so alternate the two: solve the quadratic problem
       with a proximal term that anchors every cell to where it currently
       is, then legalize.  The anchor weight decays geometrically, which
       lets the cells gradually trade density for connectivity, and the
       best legal placement seen is kept, so the refinement can only make
       the result better.  A round that pushes any row to the occupancy
       limit is rejected without retrying, because a smaller anchor weight
       would only pack the rows further. */
    {
        double BestHpwl = Phys_PlaceHpwlFree( pNodes, vNode2Idx, vFreeIdx, vSinks, nObjs ) / Dbu;
        Vec_Flt_t * vBestX = Vec_FltStart( nFree );
        Vec_Flt_t * vBestY = Vec_FltStart( nFree );
        float OccLimit = 0.95f;
        int iRound, nFail = 0;
        float Lambda = 50.0f * AvgDiag;
        Vec_IntForEachEntry( vFree, i, k )
        {
            Vec_FltWriteEntry( vBestX, k, pNodes[i].X );
            Vec_FltWriteEntry( vBestY, k, pNodes[i].Y );
        }
        for ( iRound = 0; iRound < pPars->nIterRefine; iRound++ )
        {
            double CurHpwl;
            float MaxOcc, LambdaUsed = Lambda;
            Vec_IntForEachEntry( vFree, i, k )
            {
                Vec_FltWriteEntry( vRefX, k, pNodes[i].X );
                Vec_FltWriteEntry( vRefY, k, pNodes[i].Y );
                Vec_FltWriteEntry( vSolX, k, pNodes[i].X );
                Vec_FltWriteEntry( vSolY, k, pNodes[i].Y );
            }
            Phys_PlaceSolve( nFree, vDiag, vEI, vEJ, vEW, vBx, vRefX, Lambda, vSolX, pPars->nIterCG );
            Phys_PlaceSolve( nFree, vDiag, vEI, vEJ, vEW, vBy, vRefY, Lambda, vSolY, pPars->nIterCG );
            Vec_IntForEachEntry( vFree, i, k )
            {
                pNodes[i].X = Vec_FltEntry( vSolX, k );
                pNodes[i].Y = Vec_FltEntry( vSolY, k );
            }
            Phys_PlaceLegalize( p, pNodes, vFree, x0, y0, W, H, 0 );
            CurHpwl = Phys_PlaceHpwlFree( pNodes, vNode2Idx, vFreeIdx, vSinks, nObjs ) / Dbu;
            MaxOcc  = Phys_PlaceMaxRowOcc( pNodes, vFree, y0, H, p->SiteH, W );
            if ( CurHpwl < BestHpwl * (1.0 - 1e-4) && MaxOcc <= OccLimit )
            {
                BestHpwl = CurHpwl;
                Vec_IntForEachEntry( vFree, i, k )
                {
                    Vec_FltWriteEntry( vBestX, k, pNodes[i].X );
                    Vec_FltWriteEntry( vBestY, k, pNodes[i].Y );
                }
                Lambda *= 0.7f;
                nFail = 0;
            }
            else
            {
                double Rel = (CurHpwl - BestHpwl) / BestHpwl;
                Vec_IntForEachEntry( vFree, i, k )
                {
                    pNodes[i].X = Vec_FltEntry( vBestX, k );
                    pNodes[i].Y = Vec_FltEntry( vBestY, k );
                }
                if ( MaxOcc > OccLimit )
                {
                    if ( pPars->fVerbose )
                        fprintf( stdout, "Placement: refinement stopped at round %d: "
                                 "row occupancy %.2f exceeds %.2f.\n", iRound, MaxOcc, OccLimit );
                    break;
                }
                Lambda *= 0.7f;
                /* A round that legalizes back to the same placement means
                   the anchor is still too stiff, not that the schedule is
                   exhausted, so keep decaying the anchor instead of
                   counting it against the failure budget. */
                if ( Rel > 1e-4 && ++nFail >= 3 )
                    break;
            }
            if ( pPars->fVerbose )
            {
                char pStr[72];
                sprintf( pStr, "after round %d (lambda %.0f, row occ %.2f):", iRound, LambdaUsed, MaxOcc );
                Phys_PlaceReport( pStr, pNodes, vFree, vNode2Idx, vFreeIdx, vSinks, nObjs, Dbu );
            }
        }
        Vec_FltFree( vBestX );
        Vec_FltFree( vBestY );
        if ( pPars->fVerbose )
            fprintf( stdout, "Placement: refinement finished at wirelength %.1f um.\n", BestHpwl );
    }
    /* save the result */
    Phys_ManClearPlace( p );
    p->vObjX = Vec_IntStartFull( nObjs );
    p->vObjY = Vec_IntStartFull( nObjs );
    for ( i = 0; i < nNodes; i++ )
    {
        Vec_IntWriteEntry( p->vObjX, pNodes[i].Obj, (int)(pNodes[i].X + 0.5f) );
        Vec_IntWriteEntry( p->vObjY, pNodes[i].Obj, (int)(pNodes[i].Y + 0.5f) );
    }
    p->CoreX0  = 0;
    p->CoreY0  = 0;
    p->CoreW   = (int)(W + 0.5f);
    p->CoreH   = (int)(H + 0.5f);
    p->fPlaced = 1;
    Phys_ManClearTiming( p );
    /* total half-perimeter wirelength of the placed netlist */
    p->Hpwl = (float)( Phys_PlaceHpwl( pNodes, vNode2Idx, vSinks, nObjs ) / Dbu );
    if ( pPars->fVerbose )
        fprintf( stdout, "Placement: %d cells placed, %d overlaps remain, utilization %.2f, "
                 "%d/%d rows used, max row occupancy %.2f, wirelength %.1f um.\n",
                 nFree, nOverlaps, TotalArea / ((double)W * (double)H),
                 Phys_PlaceRowsUsed( pNodes, vFree, y0, H, p->SiteH ),
                 (int)(H / p->SiteH),
                 Phys_PlaceMaxRowOcc( pNodes, vFree, y0, H, p->SiteH, W ), p->Hpwl );
    ABC_FREE( pNodes );
    Vec_IntFree( vNode2Idx );
    Vec_IntFree( vFreeIdx );
    Vec_IntFree( vFree );
    for ( q = 0; q < nObjs; q++ )
        if ( Vec_PtrEntry(vSinks, q) != NULL )
            Vec_IntFree( (Vec_Int_t *)Vec_PtrEntry(vSinks, q) );
    Vec_PtrFree( vSinks );
    Vec_IntFree( vEI ); Vec_IntFree( vEJ );
    Vec_FltFree( vEW ); Vec_FltFree( vDiag );
    Vec_FltFree( vBx ); Vec_FltFree( vBy );
    Vec_FltFree( vSolX ); Vec_FltFree( vSolY );
    Vec_FltFree( vRefX ); Vec_FltFree( vRefY );
    return 1;
}

void Phys_PlacePrint( Phys_Man_t * p )
{
    if ( p == NULL || !p->fPlaced )
    {
        fprintf( stdout, "Placement: no placement data.\n" );
        return;
    }
    fprintf( stdout, "Placement: core %d x %d DBU, site %d x %d DBU.\n",
             p->CoreW, p->CoreH, p->SiteW, p->SiteH );
}

ABC_NAMESPACE_IMPL_END
