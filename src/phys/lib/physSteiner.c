/**Function*************************************************************

  Synopsis    [Rectilinear Steiner tree for the interconnect model]

  Description [See physSteiner.h.  The small-net solver is the iterated
               1-Steiner heuristic over the Hanan grid of the net; the
               large-net solver splits the net at the median of the
               longer side of its bounding box, solves the groups and
               joins the group trees by their shortest connecting edge.
               Every net also gets the plain minimum spanning tree as a
               reference, and the Steiner tree is used only when it is
               shorter.]

  SideEffects []

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "phys/lib/physSteiner.h"

ABC_NAMESPACE_IMPL_START

/* the largest group solved by the iterated 1-Steiner search; a larger
   net is broken into groups of at most this many terminals */
#define PHYS_STREE_GROUP 12
/* an improvement smaller than this is not worth another Steiner point */
#define PHYS_STREE_EPS   1e-4f

static float Phys_StreeDist( float x0, float y0, float x1, float y1 )
{
    float dx = x0 - x1, dy = y0 - y1;
    return (dx < 0.0f ? -dx : dx) + (dy < 0.0f ? -dy : dy);
}

/* Prim's algorithm over n points, rooted at point zero; fills the parent
   of every point (-1 at the root) and the length of the edge to it, and
   returns the total length of the spanning tree */
static float Phys_StreeMst( float * pX, float * pY, int n, int * pParent, float * pLen )
{
    float * pBest;
    float Total = 0.0f;
    int i, j, iBest;
    if ( n <= 0 )
        return 0.0f;
    pBest = ABC_ALLOC( float, n );
    for ( i = 0; i < n; i++ )
    {
        pParent[i] = i ? 0 : -1;
        pLen[i]    = 0.0f;
        pBest[i]   = i ? Phys_StreeDist( pX[0], pY[0], pX[i], pY[i] ) : -1.0f;
    }
    for ( i = 1; i < n; i++ )
    {
        iBest = -1;
        for ( j = 0; j < n; j++ )
            if ( pBest[j] >= 0.0f && (iBest < 0 || pBest[j] < pBest[iBest]) )
                iBest = j;
        if ( iBest < 0 )
            break;
        pLen[iBest]  = pBest[iBest];
        Total       += pBest[iBest];
        pBest[iBest] = -1.0f;
        for ( j = 0; j < n; j++ )
            if ( pBest[j] >= 0.0f )
            {
                float Dist = Phys_StreeDist( pX[iBest], pY[iBest], pX[j], pY[j] );
                if ( Dist < pBest[j] )
                {
                    pBest[j]   = Dist;
                    pParent[j] = iBest;
                }
            }
    }
    ABC_FREE( pBest );
    return Total;
}

/* copies the unique values of a coordinate array into a vector */
static void Phys_StreeUnique( float * p, int n, Vec_Flt_t * vValues )
{
    int i, nOut = 0;
    Vec_FltClear( vValues );
    for ( i = 0; i < n; i++ )
        Vec_FltPush( vValues, p[i] );
    Vec_FltSort( vValues, 0 );
    for ( i = 0; i < Vec_FltSize(vValues); i++ )
        if ( nOut == 0 || Vec_FltEntry(vValues, i) != Vec_FltEntry(vValues, nOut-1) )
            Vec_FltWriteEntry( vValues, nOut++, Vec_FltEntry(vValues, i) );
    Vec_FltShrink( vValues, nOut );
}

/* the iterated 1-Steiner heuristic on one group of points; pX/pY have
   room for n extra points and receive the Steiner points after the n
   terminals.  Fills the minimum spanning tree over the final point set
   and returns its total length. */
static float Phys_StreeGroup( float * pX, float * pY, int n,
                              int * pParent, float * pLen, int * pnOut )
{
    int nPts = n, iter, i, j, nX, nY, fExist, k;
    int * pTmpParent = ABC_ALLOC( int, 2 * n + 2 );
    float * pTmpLen = ABC_ALLOC( float, 2 * n + 2 );
    Vec_Flt_t * vXs = Vec_FltAlloc( 2 * n + 2 );
    Vec_Flt_t * vYs = Vec_FltAlloc( 2 * n + 2 );
    float Total = Phys_StreeMst( pX, pY, nPts, pParent, pLen );
    /* the Hanan grid points that are not already in the set are the
       candidates; the search stops as soon as none of them helps */
    for ( iter = 0; iter < n; iter++ )
    {
        float BestTotal = Total, BestX = 0.0f, BestY = 0.0f;
        int fBest = 0;
        Phys_StreeUnique( pX, nPts, vXs );
        Phys_StreeUnique( pY, nPts, vYs );
        nX = Vec_FltSize( vXs );
        nY = Vec_FltSize( vYs );
        for ( i = 0; i < nX; i++ )
        for ( j = 0; j < nY; j++ )
        {
            float cx = Vec_FltEntry( vXs, i );
            float cy = Vec_FltEntry( vYs, j );
            float Cand;
            for ( fExist = 0, k = 0; k < nPts; k++ )
                if ( pX[k] == cx && pY[k] == cy )
                {
                    fExist = 1;
                    break;
                }
            if ( fExist )
                continue;
            pX[nPts] = cx;
            pY[nPts] = cy;
            Cand = Phys_StreeMst( pX, pY, nPts + 1, pTmpParent, pTmpLen );
            if ( Cand < BestTotal - PHYS_STREE_EPS )
                BestTotal = Cand, BestX = cx, BestY = cy, fBest = 1;
        }
        if ( !fBest )
            break;
        pX[nPts] = BestX;
        pY[nPts] = BestY;
        nPts++;
        Total = BestTotal;
    }
    Total = Phys_StreeMst( pX, pY, nPts, pParent, pLen );
    *pnOut = nPts;
    Vec_FltFree( vXs );
    Vec_FltFree( vYs );
    ABC_FREE( pTmpParent );
    ABC_FREE( pTmpLen );
    return Total;
}

/* the sort of the point ids used by the median split; the recursion is
   depth-first, so the file-scope keys are set before the sort and no
   recursive search runs while they are in use */
static float * s_pSortX;
static float * s_pSortY;
static int     s_fSortX;
static int Phys_StreeSortCmp( const void * p1, const void * p2 )
{
    int i = *(const int *)p1;
    int j = *(const int *)p2;
    float a = s_fSortX ? s_pSortX[i] : s_pSortY[i];
    float b = s_fSortX ? s_pSortX[j] : s_pSortY[j];
    if ( a < b ) return -1;
    if ( a > b ) return  1;
    return i - j;
}

/* the tree under construction: node coordinates and an edge list */
typedef struct Phys_StreeCtx_t_
{
    float * pX, * pY;
    int     nNodes, nCap;
    int   * pE0, * pE1;
    float * pELen;
    int     nEdges, nEdgeCap;
} Phys_StreeCtx_t;

static int Phys_StreeAddNode( Phys_StreeCtx_t * p, float x, float y )
{
    assert( p->nNodes < p->nCap );
    p->pX[p->nNodes] = x;
    p->pY[p->nNodes] = y;
    return p->nNodes++;
}
static void Phys_StreeAddEdge( Phys_StreeCtx_t * p, int u, int v, float Len )
{
    assert( p->nEdges < p->nEdgeCap );
    p->pE0[p->nEdges]   = u;
    p->pE1[p->nEdges]   = v;
    p->pELen[p->nEdges] = Len;
    p->nEdges++;
}

/* builds the tree over the node ids in vIds and returns the nodes of the
   resulting component, terminals first in the order of vIds */
static Vec_Int_t * Phys_StreeBuildRec( Phys_StreeCtx_t * p, Vec_Int_t * vIds )
{
    int n = Vec_IntSize( vIds ), i, nA, nOut;
    Vec_Int_t * vRes;
    if ( n <= PHYS_STREE_GROUP )
    {
        float * pX   = ABC_ALLOC( float, 2 * n + 2 );
        float * pY   = ABC_ALLOC( float, 2 * n + 2 );
        int * pPar   = ABC_ALLOC( int, 2 * n + 2 );
        float * pLen = ABC_ALLOC( float, 2 * n + 2 );
        int * pMap   = ABC_ALLOC( int, 2 * n + 2 );
        for ( i = 0; i < n; i++ )
        {
            int id = Vec_IntEntry( vIds, i );
            pX[i] = p->pX[id];
            pY[i] = p->pY[id];
        }
        Phys_StreeGroup( pX, pY, n, pPar, pLen, &nOut );
        vRes = Vec_IntAlloc( nOut );
        for ( i = 0; i < n; i++ )
            pMap[i] = Vec_IntEntry( vIds, i ), Vec_IntPush( vRes, pMap[i] );
        for ( i = n; i < nOut; i++ )
            pMap[i] = Phys_StreeAddNode( p, pX[i], pY[i] ), Vec_IntPush( vRes, pMap[i] );
        for ( i = 1; i < nOut; i++ )
            Phys_StreeAddEdge( p, pMap[pPar[i]], pMap[i], pLen[i] );
        ABC_FREE( pX );
        ABC_FREE( pY );
        ABC_FREE( pPar );
        ABC_FREE( pLen );
        ABC_FREE( pMap );
        return vRes;
    }
    /* split the longer side of the bounding box at the median */
    {
        float X0 = p->pX[Vec_IntEntry(vIds,0)], X1 = X0;
        float Y0 = p->pY[Vec_IntEntry(vIds,0)], Y1 = Y0;
        Vec_Int_t * vA, * vB, * rA, * rB;
        int * pSort = ABC_ALLOC( int, n );
        int iA, iB, uBest = -1, vBest = -1;
        float Best = 1e30f;
        for ( i = 1; i < n; i++ )
        {
            int id = Vec_IntEntry( vIds, i );
            X0 = Abc_MinFloat( X0, p->pX[id] );
            X1 = Abc_MaxFloat( X1, p->pX[id] );
            Y0 = Abc_MinFloat( Y0, p->pY[id] );
            Y1 = Abc_MaxFloat( Y1, p->pY[id] );
        }
        for ( i = 0; i < n; i++ )
            pSort[i] = Vec_IntEntry( vIds, i );
        s_pSortX = p->pX;
        s_pSortY = p->pY;
        s_fSortX = (X1 - X0) >= (Y1 - Y0);
        qsort( pSort, n, sizeof(int), Phys_StreeSortCmp );
        nA = n / 2;
        vA = Vec_IntAlloc( nA );
        vB = Vec_IntAlloc( n - nA );
        for ( i = 0; i < nA; i++ )
            Vec_IntPush( vA, pSort[i] );
        for ( ; i < n; i++ )
            Vec_IntPush( vB, pSort[i] );
        ABC_FREE( pSort );
        rA = Phys_StreeBuildRec( p, vA );
        rB = Phys_StreeBuildRec( p, vB );
        Vec_IntFree( vA );
        Vec_IntFree( vB );
        /* join the two trees by the shortest edge between their nodes */
        for ( iA = 0; iA < Vec_IntSize(rA); iA++ )
        {
            int u = Vec_IntEntry( rA, iA );
            for ( iB = 0; iB < Vec_IntSize(rB); iB++ )
            {
                int v = Vec_IntEntry( rB, iB );
                float Dist = Phys_StreeDist( p->pX[u], p->pY[u], p->pX[v], p->pY[v] );
                if ( Dist < Best )
                    Best = Dist, uBest = u, vBest = v;
            }
        }
        Phys_StreeAddEdge( p, uBest, vBest, Best );
        Vec_IntAppend( rA, rB );
        Vec_IntFree( rB );
        return rA;
    }
}

/* roots the edge list at node zero and stores the tree in pTree */
static void Phys_StreeFinish( Phys_StreeCtx_t * p, Phys_Stree_t * pTree )
{
    int n = p->nNodes, e = p->nEdges, nArc = 2 * e > 0 ? 2 * e : 1;
    int * pHead = ABC_ALLOC( int, n );
    int * pNext = ABC_ALLOC( int, nArc );
    int * pTo   = ABC_ALLOC( int, nArc );
    float * pELen = ABC_ALLOC( float, nArc );
    int * pStack = ABC_ALLOC( int, n );
    int i, j, nTop = 0, nVisited = 0;
    float Total = 0.0f;
    for ( i = 0; i < n; i++ )
        pHead[i] = -1;
    for ( i = 0; i < e; i++ )
    {
        int u = p->pE0[i], v = p->pE1[i];
        pNext[2*i]   = pHead[u];
        pTo[2*i]     = v;
        pELen[2*i]   = p->pELen[i];
        pHead[u]     = 2*i;
        pNext[2*i+1] = pHead[v];
        pTo[2*i+1]   = u;
        pELen[2*i+1] = p->pELen[i];
        pHead[v]     = 2*i+1;
        Total       += p->pELen[i];
    }
    for ( i = 0; i < n; i++ )
        pTree->pParent[i] = -2;
    pTree->pParent[0] = -1;
    pTree->pLen[0]    = 0.0f;
    pStack[nTop++]    = 0;
    while ( nTop )
    {
        int v = pStack[--nTop];
        nVisited++;
        for ( j = pHead[v]; j >= 0; j = pNext[j] )
        {
            int w = pTo[j];
            if ( pTree->pParent[w] != -2 )
                continue;
            pTree->pParent[w] = v;
            pTree->pLen[w]    = pELen[j];
            pStack[nTop++]    = w;
        }
    }
    assert( nVisited == n );
    pTree->TotalLen = Total;
    ABC_FREE( pHead );
    ABC_FREE( pNext );
    ABC_FREE( pTo );
    ABC_FREE( pELen );
    ABC_FREE( pStack );
}

Phys_Stree_t * Phys_StreeBuild( float * pX, float * pY, int nTerms, int fSteiner )
{
    Phys_StreeCtx_t Ctx, * p = &Ctx;
    Phys_Stree_t * pTree;
    int * pPar;
    float * pLen;
    float LenSteiner = 0.0f, LenMst;
    int i, fRun, fUsed;
    if ( nTerms <= 0 )
        return NULL;
    p->nCap     = 2 * nTerms + 8;
    p->nEdgeCap = 4 * nTerms + 8;
    p->nNodes   = nTerms;
    p->nEdges   = 0;
    p->pX    = ABC_ALLOC( float, p->nCap );
    p->pY    = ABC_ALLOC( float, p->nCap );
    p->pE0   = ABC_ALLOC( int,   p->nEdgeCap );
    p->pE1   = ABC_ALLOC( int,   p->nEdgeCap );
    p->pELen = ABC_ALLOC( float, p->nEdgeCap );
    for ( i = 0; i < nTerms; i++ )
        p->pX[i] = pX[i], p->pY[i] = pY[i];
    if ( fSteiner && nTerms >= 4 )
    {
        Vec_Int_t * vIds = Vec_IntAlloc( nTerms );
        Vec_Int_t * vRes;
        for ( i = 0; i < nTerms; i++ )
            Vec_IntPush( vIds, i );
        vRes = Phys_StreeBuildRec( p, vIds );
        Vec_IntFree( vRes );
        Vec_IntFree( vIds );
        for ( i = 0; i < p->nEdges; i++ )
            LenSteiner += p->pELen[i];
    }
    /* the plain spanning tree is both the fallback and the reference */
    pPar = ABC_ALLOC( int, nTerms );
    pLen = ABC_ALLOC( float, nTerms );
    LenMst = Phys_StreeMst( p->pX, p->pY, nTerms, pPar, pLen );
    fRun  = fSteiner && nTerms >= 4;
    fUsed = fRun && p->nEdges > 0 && LenSteiner <= LenMst + PHYS_STREE_EPS;
    if ( !fUsed )
    {
        p->nNodes = nTerms;
        p->nEdges = 0;
        for ( i = 1; i < nTerms; i++ )
            Phys_StreeAddEdge( p, pPar[i], i, pLen[i] );
    }
    ABC_FREE( pPar );
    ABC_FREE( pLen );
    pTree = ABC_CALLOC( Phys_Stree_t, 1 );
    pTree->nNodes   = p->nNodes;
    pTree->nTerms   = nTerms;
    pTree->nSteiner = p->nNodes - nTerms;
    pTree->fRun     = fRun;
    pTree->fUsed    = fUsed;
    pTree->pX       = ABC_ALLOC( float, p->nNodes );
    pTree->pY       = ABC_ALLOC( float, p->nNodes );
    pTree->pParent  = ABC_ALLOC( int,   p->nNodes );
    pTree->pLen     = ABC_ALLOC( float, p->nNodes );
    pTree->pTermNode= ABC_ALLOC( int,   nTerms );
    for ( i = 0; i < p->nNodes; i++ )
        pTree->pX[i] = p->pX[i], pTree->pY[i] = p->pY[i];
    for ( i = 0; i < nTerms; i++ )
        pTree->pTermNode[i] = i;
    Phys_StreeFinish( p, pTree );
    ABC_FREE( p->pX );
    ABC_FREE( p->pY );
    ABC_FREE( p->pE0 );
    ABC_FREE( p->pE1 );
    ABC_FREE( p->pELen );
    return pTree;
}

void Phys_StreeFree( Phys_Stree_t * p )
{
    if ( p == NULL )
        return;
    ABC_FREE( p->pX );
    ABC_FREE( p->pY );
    ABC_FREE( p->pParent );
    ABC_FREE( p->pLen );
    ABC_FREE( p->pTermNode );
    ABC_FREE( p );
}

ABC_NAMESPACE_IMPL_END
