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
    p->Util         = 0.60f;
    p->Aspect       = 1.00f;
    p->nIterCG      = 300;
    p->nIterNest    = 400;
    p->StopOverflow = 0.10f;
    p->fVerbose     = 0;
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

  Synopsis    [Nonlinear global placement: bin grid and Poisson field]

  Description [Implements the density model of RePlAce/DREAMPlace.
               The core is divided into bins and every cell contributes
               the area of its overlap with each bin it touches, computed
               with the triangle density function.  The normalized
               density map is the right-hand side of a Poisson equation
               with Neumann boundary conditions, which is solved in the
               DCT-II domain where the discrete Laplacian is diagonal.
               With laplacian(F) = -rho the electric field E = -grad F
               points away from dense regions, so descending along
               lambda * E spreads the cells.]

  SideEffects []

***********************************************************************/

typedef struct Phys_PlaceBins_t_
{
    int     nBinsX, nBinsY;   /* bin grid                                    */
    double  X0, Y0;           /* lower-left corner of the core               */
    double  BinW, BinH;       /* bin dimensions                              */
    double  Target;           /* target density (utilization)                */
    double  TotalArea;        /* total movable area, normalizes the overflow */
    double * pDens;           /* density map, nBinsX * nBinsY                */
    double * pPot;            /* electrostatic potential                     */
    double * pFieldX;         /* electric field, x component                 */
    double * pFieldY;         /* electric field, y component                 */
    double * pSpec;           /* DCT spectrum, scratch                       */
    double * pTmp;            /* DCT scratch                                 */
    double * pCosX;           /* cosine tables of the DCT                    */
    double * pCosY;
    double * pEigX;           /* eigenvalues of the discrete Laplacian       */
    double * pEigY;
} Phys_PlaceBins_t;

#define PHYS_PLACE_PI 3.14159265358979323846

static void Phys_PlaceBinsInit( Phys_PlaceBins_t * b, Phys_Man_t * p, double x0, double y0,
                                double W, double H, double Target, double TotalArea )
{
    int M, N, i, j, nBins;
    memset( b, 0, sizeof(Phys_PlaceBins_t) );
    /* a few sites per bin, but never more than 64 bins per direction */
    M = (int)( W / (4.0 * (double)p->SiteW) );
    N = (int)( H / (2.0 * (double)p->SiteH) );
    if ( M < 8 ) M = 8;
    if ( N < 8 ) N = 8;
    if ( M > 64 ) M = 64;
    if ( N > 64 ) N = 64;
    b->nBinsX    = M;
    b->nBinsY    = N;
    b->X0        = x0;
    b->Y0        = y0;
    b->BinW      = W / M;
    b->BinH      = H / N;
    b->Target    = Target;
    b->TotalArea = TotalArea;
    nBins = M * N;
    b->pDens   = ABC_CALLOC( double, nBins );
    b->pPot    = ABC_CALLOC( double, nBins );
    b->pFieldX = ABC_CALLOC( double, nBins );
    b->pFieldY = ABC_CALLOC( double, nBins );
    b->pSpec   = ABC_CALLOC( double, nBins );
    b->pTmp    = ABC_CALLOC( double, nBins );
    b->pCosX   = ABC_ALLOC( double, M*M );
    b->pCosY   = ABC_ALLOC( double, N*N );
    b->pEigX   = ABC_ALLOC( double, M );
    b->pEigY   = ABC_ALLOC( double, N );
    for ( i = 0; i < M; i++ )
        for ( j = 0; j < M; j++ )
            b->pCosX[i*M+j] = cos( PHYS_PLACE_PI * j * (2.0*i + 1.0) / (2.0*M) );
    for ( i = 0; i < N; i++ )
        for ( j = 0; j < N; j++ )
            b->pCosY[i*N+j] = cos( PHYS_PLACE_PI * j * (2.0*i + 1.0) / (2.0*N) );
    /* eigenvalues of the cell-centered Neumann Laplacian */
    for ( j = 0; j < M; j++ )
        b->pEigX[j] = (2.0 - 2.0*cos(PHYS_PLACE_PI*j/M)) / (b->BinW*b->BinW);
    for ( j = 0; j < N; j++ )
        b->pEigY[j] = (2.0 - 2.0*cos(PHYS_PLACE_PI*j/N)) / (b->BinH*b->BinH);
}

static void Phys_PlaceBinsFree( Phys_PlaceBins_t * b )
{
    ABC_FREE( b->pDens );
    ABC_FREE( b->pPot );
    ABC_FREE( b->pFieldX );
    ABC_FREE( b->pFieldY );
    ABC_FREE( b->pSpec );
    ABC_FREE( b->pTmp );
    ABC_FREE( b->pCosX );
    ABC_FREE( b->pCosY );
    ABC_FREE( b->pEigX );
    ABC_FREE( b->pEigY );
}

/* orthonormal DCT-II of an M x N array, used as the forward Poisson transform */
static void Phys_PlaceDct( int M, int N, const double * pIn,
                           const double * pCosX, const double * pCosY,
                           double * pOut, double * pTmp )
{
    int i, j, u, v;
    memset( pTmp, 0, sizeof(double)*M*N );
    for ( i = 0; i < M; i++ )
        for ( j = 0; j < N; j++ )
        {
            double Val = pIn[i*N+j];
            if ( Val == 0.0 ) continue;
            for ( u = 0; u < M; u++ )
                pTmp[u*N+j] += Val * pCosX[i*M+u];
        }
    for ( u = 0; u < M; u++ )
    {
        double cu = (u == 0) ? sqrt(1.0/M) : sqrt(2.0/M);
        for ( v = 0; v < N; v++ )
            pOut[u*N+v] = 0.0;
        for ( j = 0; j < N; j++ )
        {
            double Val = pTmp[u*N+j];
            if ( Val == 0.0 ) continue;
            for ( v = 0; v < N; v++ )
                pOut[u*N+v] += Val * pCosY[j*N+v];
        }
        for ( v = 0; v < N; v++ )
        {
            double cv = (v == 0) ? sqrt(1.0/N) : sqrt(2.0/N);
            pOut[u*N+v] *= cu * cv;
        }
    }
}

/* orthonormal inverse DCT-II */
static void Phys_PlaceIdct( int M, int N, const double * pIn,
                            const double * pCosX, const double * pCosY,
                            double * pOut, double * pTmp )
{
    int i, j, u, v;
    memset( pTmp, 0, sizeof(double)*M*N );
    for ( u = 0; u < M; u++ )
    {
        double cu = (u == 0) ? sqrt(1.0/M) : sqrt(2.0/M);
        for ( j = 0; j < N; j++ )
        {
            double Sum = 0.0;
            for ( v = 0; v < N; v++ )
            {
                double cv = (v == 0) ? sqrt(1.0/N) : sqrt(2.0/N);
                Sum += cu * cv * pIn[u*N+v] * pCosY[j*N+v];
            }
            pTmp[u*N+j] = Sum;
        }
    }
    for ( i = 0; i < M; i++ )
        for ( j = 0; j < N; j++ )
        {
            double Sum = 0.0;
            for ( u = 0; u < M; u++ )
                Sum += pTmp[u*N+j] * pCosX[i*M+u];
            pOut[i*N+j] = Sum;
        }
}

/* solves laplacian(F) = -density and stores E = -grad F */
static void Phys_PlaceField( Phys_PlaceBins_t * b )
{
    int M = b->nBinsX, N = b->nBinsY, i, j, u, v;
    double hx = b->BinW, hy = b->BinH;
    Phys_PlaceDct( M, N, b->pDens, b->pCosX, b->pCosY, b->pSpec, b->pTmp );
    for ( u = 0; u < M; u++ )
        for ( v = 0; v < N; v++ )
        {
            double D = b->pEigX[u] + b->pEigY[v];
            /* laplacian(F) = -rho is  -D*F = -rho  in the DCT domain; the
               constant mode is a free additive constant and is dropped */
            b->pSpec[u*N+v] = (u == 0 && v == 0) ? 0.0 : b->pSpec[u*N+v] / D;
        }
    Phys_PlaceIdct( M, N, b->pSpec, b->pCosX, b->pCosY, b->pPot, b->pTmp );
    for ( i = 0; i < M; i++ )
        for ( j = 0; j < N; j++ )
        {
            double Fx, Fy;
            /* the Neumann condition mirrors the ghost bin, so the
               centered difference at the boundary uses the boundary bin
               itself on the outside */
            if ( i == 0 )
                Fx = (b->pPot[(i+1)*N+j] - b->pPot[i*N+j]) / (2.0*hx);
            else if ( i == M-1 )
                Fx = (b->pPot[i*N+j] - b->pPot[(i-1)*N+j]) / (2.0*hx);
            else
                Fx = (b->pPot[(i+1)*N+j] - b->pPot[(i-1)*N+j]) / (2.0*hx);
            if ( j == 0 )
                Fy = (b->pPot[i*N+j+1] - b->pPot[i*N+j]) / (2.0*hy);
            else if ( j == N-1 )
                Fy = (b->pPot[i*N+j] - b->pPot[i*N+j-1]) / (2.0*hy);
            else
                Fy = (b->pPot[i*N+j+1] - b->pPot[i*N+j-1]) / (2.0*hy);
            b->pFieldX[i*N+j] = -Fx;
            b->pFieldY[i*N+j] = -Fy;
        }
}

/* overlap of an interval with a bin, the triangle density function */
static double Phys_PlaceTri( double lo, double size, double BinLo, double BinHi )
{
    double Hi = lo + size < BinHi ? lo + size : BinHi;
    double Lo = lo > BinLo ? lo : BinLo;
    return Hi > Lo ? Hi - Lo : 0.0;
}

/* bin range touched by the cell footprint, clamped to the bin grid */
static void Phys_PlaceBinRange( Phys_PlaceBins_t * b, double lo, double size,
                                int * pLo, int * pHi, int nBins, double Origin, double BinSize )
{
    int Low  = (int)floor( (lo - Origin) / BinSize );
    int High = (int)floor( (lo + size - Origin) / BinSize ) + 1;
    if ( Low < 0 ) Low = 0;
    if ( High > nBins ) High = nBins;
    if ( High < Low ) High = Low;
    *pLo = Low;
    *pHi = High;
}

static void Phys_PlaceDensityMap( Phys_PlaceBins_t * b, Phys_PNode_t * pNodes, int nFree,
                                  Vec_Int_t * vFree, double * pX, double * pY, double * pDens )
{
    int M = b->nBinsX, N = b->nBinsY, k, kk, hh;
    double InvArea = 1.0 / (b->BinW * b->BinH);
    memset( pDens, 0, sizeof(double)*M*N );
    for ( k = 0; k < nFree; k++ )
    {
        int iNode = Vec_IntEntry( vFree, k );
        double w = (double)pNodes[iNode].W;
        double h = (double)pNodes[iNode].H;
        double xl = pX[k] - 0.5*w;
        double yl = pY[k] - 0.5*h;
        int kl, kh, bl, bh;
        Phys_PlaceBinRange( b, xl, w, &kl, &kh, M, b->X0, b->BinW );
        Phys_PlaceBinRange( b, yl, h, &bl, &bh, N, b->Y0, b->BinH );
        for ( kk = kl; kk < kh; kk++ )
        {
            double Bxl = b->X0 + kk*b->BinW;
            double px = Phys_PlaceTri( xl, w, Bxl, Bxl + b->BinW );
            if ( px <= 0.0 ) continue;
            for ( hh = bl; hh < bh; hh++ )
            {
                double Byl = b->Y0 + hh*b->BinH;
                double py = Phys_PlaceTri( yl, h, Byl, Byl + b->BinH );
                if ( py <= 0.0 ) continue;
                pDens[kk*N+hh] += px*py*InvArea;
            }
        }
    }
}

/* adds the gradient of the density penalty to the gradient of every free
   cell; the electric force pushes a cell out of a crowded bin, so the
   gradient of the penalty is its negative */
static void Phys_PlaceDensityForce( Phys_PlaceBins_t * b, Phys_PNode_t * pNodes, int nFree,
                                    Vec_Int_t * vFree, double * pX, double * pY,
                                    double Lambda, double * pGx, double * pGy )
{
    int M = b->nBinsX, N = b->nBinsY, k, kk, hh;
    for ( k = 0; k < nFree; k++ )
    {
        int iNode = Vec_IntEntry( vFree, k );
        double w = (double)pNodes[iNode].W;
        double h = (double)pNodes[iNode].H;
        double xl = pX[k] - 0.5*w;
        double yl = pY[k] - 0.5*h;
        double Gx = 0.0, Gy = 0.0;
        int kl, kh, bl, bh;
        Phys_PlaceBinRange( b, xl, w, &kl, &kh, M, b->X0, b->BinW );
        Phys_PlaceBinRange( b, yl, h, &bl, &bh, N, b->Y0, b->BinH );
        for ( kk = kl; kk < kh; kk++ )
        {
            double Bxl = b->X0 + kk*b->BinW;
            double px = Phys_PlaceTri( xl, w, Bxl, Bxl + b->BinW );
            if ( px <= 0.0 ) continue;
            for ( hh = bl; hh < bh; hh++ )
            {
                double Byl = b->Y0 + hh*b->BinH;
                double py = Phys_PlaceTri( yl, h, Byl, Byl + b->BinH );
                int Idx;
                if ( py <= 0.0 ) continue;
                Idx = kk*N + hh;
                Gx += px*py*b->pFieldX[Idx];
                Gy += px*py*b->pFieldY[Idx];
            }
        }
        pGx[k] -= Lambda * Gx;
        pGy[k] -= Lambda * Gy;
    }
}

/* density overflow: the area above the target, relative to the cell area */
static double Phys_PlaceOverflow( Phys_PlaceBins_t * b, double * pDens, double * pMaxDens )
{
    double Over = 0.0, Max = 0.0;
    int t, nBins = b->nBinsX * b->nBinsY;
    for ( t = 0; t < nBins; t++ )
    {
        if ( pDens[t] > b->Target )
            Over += pDens[t] - b->Target;
        if ( pDens[t] > Max )
            Max = pDens[t];
    }
    if ( pMaxDens != NULL )
        *pMaxDens = Max;
    return b->TotalArea > 0.0 ? Over * b->BinW * b->BinH / b->TotalArea : 0.0;
}


/**Function*************************************************************

  Synopsis    [Log-sum-exp wirelength and its gradient]

  Description [For every net, the bounding box is replaced by
               gamma*(log(sum exp((x-xmax)/gamma)) +
                      log(sum exp((xmin-x)/gamma))) + xmax - xmin
               and the same in y.  The exponentials are shifted by the
               extremes, so they never overflow, and the gradient of a
               pin is exp((x-xmax)/gamma)/sum_x - exp((xmin-x)/gamma)/sum_nx.
               This is the NTUPlace3 smooth approximation used by
               DREAMPlace and RePlAce.]

  SideEffects []

***********************************************************************/
static double Phys_PlaceLse( int * pNetStart, int * pNetPins, int nNets,
                             double * pAX, double * pAY, Vec_Int_t * vFreeIdx,
                             double Gamma, double * pGx, double * pGy )
{
    double Inv = 1.0 / Gamma, Wl = 0.0;
    int net, j;
    for ( net = 0; net < nNets; net++ )
    {
        int Start = pNetStart[net], End = pNetStart[net+1];
        double xmin = 1e300, xmax = -1e300, ymin = 1e300, ymax = -1e300;
        double sx = 0.0, snx = 0.0, sy = 0.0, sny = 0.0;
        if ( End - Start < 2 )
            continue;
        for ( j = Start; j < End; j++ )
        {
            int nd = pNetPins[j];
            xmin = xmin < pAX[nd] ? xmin : pAX[nd];
            xmax = xmax > pAX[nd] ? xmax : pAX[nd];
            ymin = ymin < pAY[nd] ? ymin : pAY[nd];
            ymax = ymax > pAY[nd] ? ymax : pAY[nd];
        }
        for ( j = Start; j < End; j++ )
        {
            int nd = pNetPins[j];
            sx  += exp( (pAX[nd] - xmax) * Inv );
            snx += exp( (xmin - pAX[nd]) * Inv );
            sy  += exp( (pAY[nd] - ymax) * Inv );
            sny += exp( (ymin - pAY[nd]) * Inv );
        }
        Wl += Gamma * (log(sx) + log(snx)) + xmax - xmin
            + Gamma * (log(sy) + log(sny)) + ymax - ymin;
        for ( j = Start; j < End; j++ )
        {
            int nd = pNetPins[j];
            int f = Vec_IntEntry( vFreeIdx, nd );
            if ( f < 0 )
                continue;
            pGx[f] += exp( (pAX[nd] - xmax) * Inv ) / sx - exp( (xmin - pAX[nd]) * Inv ) / snx;
            pGy[f] += exp( (pAY[nd] - ymax) * Inv ) / sy - exp( (ymin - pAY[nd]) * Inv ) / sny;
        }
    }
    return Wl;
}

/* gradient of wirelength + lambda * density at the given free positions */
static double Phys_PlaceEval( Phys_PlaceBins_t * b, Phys_PNode_t * pNodes, int nNodes, int nFree,
                              Vec_Int_t * vFree, Vec_Int_t * vFreeIdx,
                              int * pNetStart, int * pNetPins, int nNets,
                              double * pAX, double * pAY, double * pX, double * pY,
                              double Gamma, double Lambda,
                              double * pGx, double * pGy,
                              double * pOverflow, double * pMaxDens )
{
    int k, iNode;
    double Wl;
    for ( iNode = 0; iNode < nNodes; iNode++ )
        if ( pNodes[iNode].Fixed )
        {
            pAX[iNode] = (double)pNodes[iNode].X;
            pAY[iNode] = (double)pNodes[iNode].Y;
        }
    for ( k = 0; k < nFree; k++ )
    {
        iNode = Vec_IntEntry( vFree, k );
        pAX[iNode] = pX[k];
        pAY[iNode] = pY[k];
    }
    memset( pGx, 0, sizeof(double)*nFree );
    memset( pGy, 0, sizeof(double)*nFree );
    Wl = Phys_PlaceLse( pNetStart, pNetPins, nNets, pAX, pAY, vFreeIdx, Gamma, pGx, pGy );
    Phys_PlaceDensityMap( b, pNodes, nFree, vFree, pX, pY, b->pDens );
    *pOverflow = Phys_PlaceOverflow( b, b->pDens, pMaxDens );
    if ( Lambda != 0.0 )
    {
        Phys_PlaceField( b );
        Phys_PlaceDensityForce( b, pNodes, nFree, vFree, pX, pY, Lambda, pGx, pGy );
    }
    return Wl;
}

/* keeps every free cell inside the core */
static void Phys_PlaceClamp( double * pX, double * pY, Phys_PNode_t * pNodes, Vec_Int_t * vFree,
                             int nFree, double x0, double y0, double W, double H )
{
    int k, iNode;
    for ( k = 0; k < nFree; k++ )
    {
        double HalfW, HalfH;
        iNode = Vec_IntEntry( vFree, k );
        HalfW = 0.5 * (double)pNodes[iNode].W;
        HalfH = 0.5 * (double)pNodes[iNode].H;
        if ( pX[k] < x0 + HalfW ) pX[k] = x0 + HalfW;
        if ( pX[k] > x0 + W - HalfW ) pX[k] = x0 + W - HalfW;
        if ( pY[k] < y0 + HalfH ) pY[k] = y0 + HalfH;
        if ( pY[k] > y0 + H - HalfH ) pY[k] = y0 + H - HalfH;
    }
}

/* the smoothing parameter: held at Gamma0 for the first fifth of the run,
   while the density force spreads the cells, then annealed linearly */
static double Phys_PlaceGamma( double Gamma0, double GammaEnd, int itGlobal, int nTot )
{
    int Hold = nTot / 5;
    double t;
    if ( itGlobal <= Hold || nTot <= Hold )
        return Gamma0;
    t = (double)(itGlobal - Hold) / (double)(nTot - Hold);
    if ( t > 1.0 ) t = 1.0;
    return Gamma0 + (GammaEnd - Gamma0) * t;
}


/**Function*************************************************************

  Synopsis    [Nonlinear global placement with Nesterov acceleration]

  Description [Minimizes the log-sum-exp wirelength plus the density
               penalty with Nesterov's accelerated gradient method and a
               Barzilai-Borwein step size, following DREAMPlace's
               NesterovAcceleratedGradientOptimizer.  The smoothing
               parameter gamma decays linearly from a few bins to a
               fraction of a bin, and the density weight starts from the
               ratio of the two gradient norms and is doubled whenever
               the density overflow stops improving.  The iterations are
               grouped into stages that share a weight; positions are
               clamped to the core after every step.]

  SideEffects []

***********************************************************************/
static void Phys_PlaceNesterov( Phys_Man_t * p, Phys_PNode_t * pNodes, int nNodes, int nFree,
                                Vec_Int_t * vFree, Vec_Int_t * vFreeIdx,
                                int * pNetStart, int * pNetPins, int nNets,
                                double x0, double y0, double W, double H, double TotalArea,
                                double Dbu, Phys_PlacePars_t * pPars,
                                double * pOverflowOut, double * pMaxDensOut )
{
    Phys_PlaceBins_t Bins, * b = &Bins;
    double * pX, * pY, * pUx, * pUy, * pVx, * pVy, * pGx, * pGy, * pDx, * pDy;
    double * pAX, * pAY;
    double Gamma0, GammaEnd, Gamma, Lambda, NormWL, NormD, MaxGrad, MaxDim;
    double Step, Alpha, A, MaxMove, Overflow, MaxDens, Wl, BestOverflow;
    int k, it, itGlobal, Stage, nStage, nInner, nTot, iNode, fStop, fConverged, fLambdaChanged;

    Phys_PlaceBinsInit( b, p, x0, y0, W, H, (double)pPars->Util, TotalArea );
    pX  = ABC_ALLOC( double, nFree );
    pY  = ABC_ALLOC( double, nFree );
    pUx = ABC_ALLOC( double, nFree );
    pUy = ABC_ALLOC( double, nFree );
    pVx = ABC_ALLOC( double, nFree );
    pVy = ABC_ALLOC( double, nFree );
    pGx = ABC_ALLOC( double, nFree );
    pGy = ABC_ALLOC( double, nFree );
    pDx = ABC_ALLOC( double, nFree );
    pDy = ABC_ALLOC( double, nFree );
    pAX = ABC_ALLOC( double, nNodes );
    pAY = ABC_ALLOC( double, nNodes );
    for ( k = 0; k < nNodes; k++ )
        if ( pNodes[k].Fixed )
        {
            pAX[k] = (double)pNodes[k].X;
            pAY[k] = (double)pNodes[k].Y;
        }
    for ( k = 0; k < nFree; k++ )
    {
        iNode = Vec_IntEntry( vFree, k );
        pX[k] = (double)pNodes[iNode].X;
        pY[k] = (double)pNodes[iNode].Y;
    }
    Phys_PlaceClamp( pX, pY, pNodes, vFree, nFree, x0, y0, W, H );
    MaxDim   = W > H ? W : H;
    Gamma0   = 4.0 * b->BinW;
    GammaEnd = 0.25 * b->BinW;
    if ( GammaEnd < 0.5 * (double)p->SiteW )
        GammaEnd = 0.5 * (double)p->SiteW;
    if ( GammaEnd > Gamma0 )
        GammaEnd = Gamma0;
    /* start from a small fraction of the weight that balances the two
       gradient norms; the schedule below raises it as needed */
    Wl = Phys_PlaceEval( b, pNodes, nNodes, nFree, vFree, vFreeIdx, pNetStart, pNetPins, nNets,
                         pAX, pAY, pX, pY, Gamma0, 0.0, pGx, pGy, &Overflow, &MaxDens );
    NormWL = 0.0;
    for ( k = 0; k < nFree; k++ )
        NormWL += pGx[k]*pGx[k] + pGy[k]*pGy[k];
    Phys_PlaceEval( b, pNodes, nNodes, nFree, vFree, vFreeIdx, pNetStart, pNetPins, nNets,
                    pAX, pAY, pX, pY, Gamma0, 1.0, pDx, pDy, &Overflow, &MaxDens );
    NormD = 0.0;
    for ( k = 0; k < nFree; k++ )
        NormD += pDx[k]*pDx[k] + pDy[k]*pDy[k];
    if ( NormD > 0.0 && NormWL > 0.0 && isfinite(NormWL) && isfinite(NormD) )
        Lambda = sqrt( NormWL / NormD );
    else
        Lambda = 1.0;
    if ( !(Lambda > 0.0) || !isfinite(Lambda) )
        Lambda = 1.0;
    /* one small gradient step gives the previous iterate for the first
       Barzilai-Borwein estimate */
    Wl = Phys_PlaceEval( b, pNodes, nNodes, nFree, vFree, vFreeIdx, pNetStart, pNetPins, nNets,
                         pAX, pAY, pX, pY, Gamma0, Lambda, pGx, pGy, &Overflow, &MaxDens );
    MaxGrad = 0.0;
    for ( k = 0; k < nFree; k++ )
    {
        double G = sqrt( pGx[k]*pGx[k] + pGy[k]*pGy[k] );
        if ( G > MaxGrad ) MaxGrad = G;
    }
    Step = MaxGrad > 0.0 ? 0.01 * MaxDim / MaxGrad : 0.0;
    for ( k = 0; k < nFree; k++ )
    {
        pVx[k] = pX[k] - Step * pGx[k];
        pVy[k] = pY[k] - Step * pGy[k];
        pUx[k] = pX[k];
        pUy[k] = pY[k];
    }
    Phys_PlaceClamp( pVx, pVy, pNodes, vFree, nFree, x0, y0, W, H );
    Phys_PlaceEval( b, pNodes, nNodes, nFree, vFree, vFreeIdx, pNetStart, pNetPins, nNets,
                    pAX, pAY, pVx, pVy, Gamma0, Lambda, pDx, pDy, &Overflow, &MaxDens );
    Alpha = 0.0;
    {
        double Num = 0.0, Den = 0.0;
        for ( k = 0; k < nFree; k++ )
        {
            double Sx = pX[k] - pVx[k], Sy = pY[k] - pVy[k];
            double Yx = pGx[k] - pDx[k], Yy = pGy[k] - pDy[k];
            Num += Sx*Sx + Sy*Sy;
            Den += Yx*Yx + Yy*Yy;
        }
        Alpha = Den > 1e-300 ? sqrt( Num / Den ) : Step;
    }
    if ( !(Alpha > 0.0) || !isfinite(Alpha) )
        Alpha = MaxDim;
    if ( Alpha > MaxDim )
        Alpha = MaxDim;
    A = 1.0;
    nTot   = pPars->nIterNest;
    nStage = 20;
    if ( nStage > nTot )
        nStage = nTot;
    nInner = nTot / nStage;
    if ( nInner < 1 )
        nInner = 1;
    if ( pPars->fVerbose )
        fprintf( stdout, "Placement: nonlinear placement on a %d x %d bin grid "
                 "(%.1f x %.1f um bins), %d stages of %d iterations, initial lambda %.3g.\n",
                 b->nBinsX, b->nBinsY, b->BinW / Dbu, b->BinH / Dbu, nStage, nInner, Lambda );
    Gamma      = Gamma0;
    itGlobal   = 0;
    fStop      = 0;
    fLambdaChanged = 0;
    /* the overflow of the best iterate so far, used to decide whether the
       density weight still buys any spreading */
    BestOverflow = 1e30;
    for ( Stage = 0; Stage < nStage && !fStop && itGlobal < nTot; Stage++ )
    {
        fConverged = 0;
        /* a new density weight invalidates the gradient and the difference
           pair the Barzilai-Borwein step is built from */
        if ( Stage > 0 && fLambdaChanged )
        {
            Gamma = Phys_PlaceGamma( Gamma0, GammaEnd, itGlobal, nTot );
            Wl = Phys_PlaceEval( b, pNodes, nNodes, nFree, vFree, vFreeIdx, pNetStart, pNetPins, nNets,
                                 pAX, pAY, pX, pY, Gamma, Lambda, pGx, pGy, &Overflow, &MaxDens );
            for ( k = 0; k < nFree; k++ )
            {
                pVx[k] = pX[k];
                pVy[k] = pY[k];
                pDx[k] = pGx[k];
                pDy[k] = pGy[k];
            }
        }
        for ( it = 0; it < nInner && itGlobal < nTot; it++, itGlobal++ )
        {
            double SS = 0.0, SY = 0.0, YY = 0.0, BB, Lip, ANext, Coef;
            double MaxG = 0.0;
            for ( k = 0; k < nFree; k++ )
            {
                double Sx = pX[k] - pVx[k], Sy = pY[k] - pVy[k];
                double Yx = pGx[k] - pDx[k], Yy = pGy[k] - pDy[k];
                SS += Sx*Sx + Sy*Sy;
                SY += Sx*Yx + Sy*Yy;
                YY += Yx*Yx + Yy*Yy;
            }
            /* Barzilai-Borwein short step with DREAMPlace's fallbacks */
            BB = YY > 1e-300 ? SY / YY : 0.0;
            if ( !(BB > 0.0) || !isfinite(BB) )
            {
                Lip = YY > 1e-300 ? sqrt( SS / YY ) : Alpha;
                BB = Lip < Alpha ? Lip : Alpha;
            }
            if ( !(BB > 0.0) || !isfinite(BB) )
                BB = Alpha;
            for ( k = 0; k < nFree; k++ )
            {
                double G = sqrt( pGx[k]*pGx[k] + pGy[k]*pGy[k] );
                if ( G > MaxG ) MaxG = G;
            }
            /* keep the largest-gradient cell from jumping across a bin:
               a step that big cannot be trusted by the bin-based density
               model */
            if ( MaxG > 0.0 )
            {
                double StepHi = 0.25 * b->BinW / MaxG;
                if ( BB > StepHi ) BB = StepHi;
            }
            ANext = 0.5 * (1.0 + sqrt( 4.0*A*A + 1.0 ));
            Coef  = (A - 1.0) / ANext;
            MaxMove = 0.0;
            for ( k = 0; k < nFree; k++ )
            {
                double OldUx = pUx[k], OldUy = pUy[k];
                double NewUx = pX[k] - BB * pGx[k];
                double NewUy = pY[k] - BB * pGy[k];
                double NewVx = NewUx + Coef * (NewUx - OldUx);
                double NewVy = NewUy + Coef * (NewUy - OldUy);
                double MoveX = fabs( NewVx - pX[k] ), MoveY = fabs( NewVy - pY[k] );
                if ( MoveX > MaxMove ) MaxMove = MoveX;
                if ( MoveY > MaxMove ) MaxMove = MoveY;
                pUx[k] = NewUx;
                pUy[k] = NewUy;
                pVx[k] = pX[k];
                pVy[k] = pY[k];
                pDx[k] = pGx[k];
                pDy[k] = pGy[k];
                pX[k]  = NewVx;
                pY[k]  = NewVy;
            }
            Phys_PlaceClamp( pX, pY, pNodes, vFree, nFree, x0, y0, W, H );
            A     = ANext;
            Alpha = BB;
            Gamma = Phys_PlaceGamma( Gamma0, GammaEnd, itGlobal, nTot );
            Wl = Phys_PlaceEval( b, pNodes, nNodes, nFree, vFree, vFreeIdx, pNetStart, pNetPins, nNets,
                                 pAX, pAY, pX, pY, Gamma, Lambda, pGx, pGy, &Overflow, &MaxDens );
            if ( pPars->fVerbose )
                fprintf( stdout, "Placement: iter %4d (stage %2d): gamma %.3f um, lambda %.3g, "
                         "overflow %.4f, max density %.3f, lse wirelength %.1f um, "
                         "step %.3g, max |g| %.3g, max move %.1f DBU.\n",
                         itGlobal, Stage, Gamma / Dbu, Lambda, Overflow, MaxDens, Wl / Dbu,
                         BB, MaxG, MaxMove );
            if ( MaxMove < 0.01 * (double)p->SiteW )
            {
                fConverged = 1;
                break;
            }
        }
        /* refresh the overflow of the final iterate of this stage */
        Phys_PlaceDensityMap( b, pNodes, nFree, vFree, pX, pY, b->pDens );
        Overflow = Phys_PlaceOverflow( b, b->pDens, &MaxDens );
        if ( pPars->fVerbose )
        {
            int t, tMax = 0, nBins = b->nBinsX * b->nBinsY;
            double BinX, BinY;
            for ( t = 1; t < nBins; t++ )
                if ( b->pDens[t] > b->pDens[tMax] )
                    tMax = t;
            BinX = (b->X0 + ((tMax / b->nBinsY) + 0.5) * b->BinW) / Dbu;
            BinY = (b->Y0 + ((tMax % b->nBinsY) + 0.5) * b->BinH) / Dbu;
            fprintf( stdout, "Placement: stage %d summary: gamma %.2f um, lambda %.3g, "
                     "overflow %.3f, max density %.2f at (%.1f, %.1f) um, lse wirelength %.1f um.\n",
                     Stage, Gamma / Dbu, Lambda, Overflow, MaxDens, BinX, BinY, Wl / Dbu );
        }
        fLambdaChanged = 0;
        if ( Overflow < pPars->StopOverflow * 0.5 )
            ;   /* already spread enough: keep the weight */
        else if ( Overflow < BestOverflow * 0.98 )
            BestOverflow = Overflow;
        else
        {
            Lambda *= 2.0;
            BestOverflow = Overflow;
            fLambdaChanged = 1;
        }
        /* even when the density is fine, keep going until the smoothing
           parameter is nearly annealed, because that is what tightens the
           wirelength */
        if ( Overflow < pPars->StopOverflow && itGlobal * 10 >= nTot * 9 )
            fStop = 1;
        if ( fConverged && Overflow < pPars->StopOverflow )
            fStop = 1;
    }
    for ( k = 0; k < nFree; k++ )
    {
        iNode = Vec_IntEntry( vFree, k );
        pNodes[iNode].X = (float)pX[k];
        pNodes[iNode].Y = (float)pY[k];
    }
    if ( pOverflowOut != NULL ) *pOverflowOut = Overflow;
    if ( pMaxDensOut  != NULL ) *pMaxDensOut  = MaxDens;
    ABC_FREE( pX ); ABC_FREE( pY );
    ABC_FREE( pUx ); ABC_FREE( pUy );
    ABC_FREE( pVx ); ABC_FREE( pVy );
    ABC_FREE( pGx ); ABC_FREE( pGy );
    ABC_FREE( pDx ); ABC_FREE( pDy );
    ABC_FREE( pAX ); ABC_FREE( pAY );
    Phys_PlaceBinsFree( b );
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
    if ( p == NULL || pGia == NULL || pPars == NULL )
        return 0;
    if ( pPars->Util <= 0.0f || pPars->Util > 1.0f )
    {
        fprintf( stdout, "Placement: target utilization %.3f should be in (0, 1].\n", pPars->Util );
        return 0;
    }
    if ( pPars->Aspect <= 0.0f )
    {
        fprintf( stdout, "Placement: the core aspect ratio %.3f should be positive.\n", pPars->Aspect );
        return 0;
    }
    if ( pPars->nIterCG < 1 || pPars->nIterNest < 1 )
    {
        fprintf( stdout, "Placement: the numbers of conjugate-gradient and nonlinear iterations should be positive.\n" );
        return 0;
    }
    if ( pPars->StopOverflow < 0.0f || pPars->StopOverflow >= 1.0f )
    {
        fprintf( stdout, "Placement: the stopping overflow %.3f should be in [0, 1).\n", pPars->StopOverflow );
        return 0;
    }
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
    /* Build the CSR net list of the nonlinear placement.  The driver of
       every net comes first, followed by its sinks; the pins index into
       the node array, so the fixed terminals are addressed the same way
       as the movable cells. */
    {
        Vec_Int_t * vNetStart = Vec_IntAlloc( nObjs + 1 );
        Vec_Int_t * vNetPins  = Vec_IntAlloc( 4 * nObjs + 16 );
        double Overflow = 0.0, MaxDens = 0.0;
        Vec_IntPush( vNetStart, 0 );
        for ( q = 0; q < nObjs; q++ )
        {
            Vec_Int_t * vList = (Vec_Int_t *)Vec_PtrEntry( vSinks, q );
            int iDriver;
            if ( vList == NULL )
                continue;
            iDriver = Vec_IntEntry( vNode2Idx, q );
            if ( iDriver < 0 )
                continue;
            Vec_IntPush( vNetPins, iDriver );
            Vec_IntAppend( vNetPins, vList );
            Vec_IntPush( vNetStart, Vec_IntSize(vNetPins) );
        }
        Phys_PlaceNesterov( p, pNodes, nNodes, nFree, vFree, vFreeIdx,
                            Vec_IntArray(vNetStart), Vec_IntArray(vNetPins), Vec_IntSize(vNetStart)-1,
                            x0, y0, W, H, TotalArea, Dbu, pPars, &Overflow, &MaxDens );
        if ( pPars->fVerbose )
            fprintf( stdout, "Placement: nonlinear placement over %d nets finished at "
                     "overflow %.3f and maximum bin density %.2f.\n",
                     Vec_IntSize(vNetStart)-1, Overflow, MaxDens );
        Vec_IntFree( vNetStart );
        Vec_IntFree( vNetPins );
    }
    if ( pPars->fVerbose )
        Phys_PlaceReport( "after nonlinear:", pNodes, vFree, vNode2Idx, vFreeIdx, vSinks, nObjs, Dbu );
    /* Legalize the spread solution. */
    nOverlaps = Phys_PlaceLegalize( p, pNodes, vFree, x0, y0, W, H, pPars->fVerbose );
    if ( pPars->fVerbose )
        Phys_PlaceReport( "after legal:", pNodes, vFree, vNode2Idx, vFreeIdx, vSinks, nObjs, Dbu );
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
