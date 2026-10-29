/**Function*************************************************************

  Synopsis    [Supergate construction from a Liberty library]

  Description [A supergate is the cell that realizes one of the eight
               two-input AIG patterns

                   f(a,b) = (p0 ? !a : a) & (p1 ? !b : b) ^ q

               It is either a library cell that implements the pattern
               directly (a fused inverter is free when the cell has an
               inverting input pin) or the minimum-area composition of
               the library's one- and two-input cells and inverters.

               The composition is found by a Viterbi search over the
               sixteen two-input Boolean functions: the two fanin nets
               have cost zero, and applying a cell to two known functions
               costs the cell area.  The cheapest supergate is then
               turned into a synthetic Phys_LibCell_t:

               - Area is the sum over the cells of the composition;
                 the placer sizes it from the area because it has no LEF
                 macro (Phys_PlaceCellSize falls back to sites);
               - the two input pins carry the capacitance of the internal
                 pins they drive;
               - the output pin carries one arc per input pin, with the
                 unateness of the pattern and rise/fall/transition tables
                 obtained by simulating the composition on the grid of
                 the terminal cell's arc.

               The simulation follows physSta.c: only the pins reached by
               the switching input are sensitized, the other external
               input is static, and the delay of a pin follows the
               timing_sense of its arc.  This gives a supergate that the
               placer and the timing analysis accept without changes.]

  SideEffects []

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "phys/phys.h"

ABC_NAMESPACE_IMPL_START

#define PHYS_SUPER_MAX_INST 256
#define PHYS_SUPER_INF      1e30f

/*------------------------------------------------------------------------*/
/* Boolean expressions of the Liberty cells                               */
/*------------------------------------------------------------------------*/

typedef struct Phys_Expr_t_
{
    const char * p;
    const char * pVar0;
    const char * pVar1;
    int          fOk;
} Phys_Expr_t;

static void Phys_ExprSkip( Phys_Expr_t * e )
{
    while ( *e->p == ' ' || *e->p == '\t' || *e->p == '\n' || *e->p == '\r' )
        e->p++;
}

static int Phys_ExprOr( Phys_Expr_t * e );

/* a variable, a constant or a parenthesized expression, with an optional
   postfix negation; the value is a 4-bit truth table over (a,b), where
   bit (a*2+b) is f(a,b) */
static int Phys_ExprAtom( Phys_Expr_t * e )
{
    int Mask;
    Phys_ExprSkip( e );
    if ( *e->p == '(' )
    {
        e->p++;
        Mask = Phys_ExprOr( e );
        Phys_ExprSkip( e );
        if ( !e->fOk || *e->p != ')' )
        {
            e->fOk = 0;
            return 0;
        }
        e->p++;
    }
    else if ( *e->p >= '0' && *e->p <= '9' )
        Mask = ( *e->p++ == '0' ) ? 0x0 : 0xF;
    else if ( isalpha((unsigned char)*e->p) || *e->p == '_' )
    {
        char Name[64];
        int n = 0;
        while ( (isalnum((unsigned char)*e->p) || *e->p == '_' ||
                 *e->p == '[' || *e->p == ']') && n < 63 )
            Name[n++] = *e->p++;
        Name[n] = 0;
        if ( e->pVar1 != NULL && !strcmp(Name, e->pVar1) )
            Mask = 0xA;   /* b: f(0,0)=0, f(0,1)=1, f(1,0)=0, f(1,1)=1 */
        else if ( !strcmp(Name, e->pVar0) )
            Mask = 0xC;   /* a: f(0,0)=0, f(0,1)=0, f(1,0)=1, f(1,1)=1 */
        else
        {
            e->fOk = 0;
            return 0;
        }
    }
    else
    {
        e->fOk = 0;
        return 0;
    }
    Phys_ExprSkip( e );
    if ( *e->p == '\'' )
    {
        e->p++;
        Mask = (~Mask) & 0xF;
    }
    return Mask;
}

static int Phys_ExprUnary( Phys_Expr_t * e )
{
    Phys_ExprSkip( e );
    if ( *e->p == '!' || *e->p == '~' )
    {
        e->p++;
        return (~Phys_ExprUnary( e )) & 0xF;
    }
    return Phys_ExprAtom( e );
}

static int Phys_ExprAnd( Phys_Expr_t * e )
{
    int Mask = Phys_ExprUnary( e );
    for ( ;; )
    {
        Phys_ExprSkip( e );
        if ( !e->fOk || (*e->p != '*' && *e->p != '&' && *e->p != '^') )
            return Mask;
        {
            char Op = *e->p++;
            int Other = Phys_ExprUnary( e );
            Mask = Op == '^' ? (Mask ^ Other) : (Mask & Other);
        }
    }
}

static int Phys_ExprOr( Phys_Expr_t * e )
{
    int Mask = Phys_ExprAnd( e );
    for ( ;; )
    {
        Phys_ExprSkip( e );
        if ( !e->fOk || (*e->p != '+' && *e->p != '|') )
            return Mask;
        e->p++;
        Mask |= Phys_ExprAnd( e );
    }
}

/*------------------------------------------------------------------------*/
/* cell inspection                                                        */
/*------------------------------------------------------------------------*/

static Phys_LibPin_t * Phys_SuperDrvPin( Phys_LibCell_t * pCell )
{
    Phys_LibPin_t * pPin;
    int i;
    Vec_PtrForEachEntry( Phys_LibPin_t *, pCell->vPins, pPin, i )
        if ( pPin->Dir == 1 )
            return pPin;
    return NULL;
}

static Phys_LibPin_t * Phys_SuperOutPin( Phys_LibCell_t * pCell )
{
    Phys_LibPin_t * pPin;
    int i;
    Vec_PtrForEachEntry( Phys_LibPin_t *, pCell->vPins, pPin, i )
        if ( pPin->Dir == 1 && pPin->pFunc != NULL && pPin->pFunc[0] )
            return pPin;
    return NULL;
}

static Phys_LibPin_t * Phys_SuperInPin( Phys_LibCell_t * pCell, int k )
{
    Phys_LibPin_t * pPin;
    int i, Count = 0;
    Vec_PtrForEachEntry( Phys_LibPin_t *, pCell->vPins, pPin, i )
        if ( pPin->Dir == 0 && Count++ == k )
            return pPin;
    return NULL;
}

static int Phys_SuperInNum( Phys_LibCell_t * pCell )
{
    Phys_LibPin_t * pPin;
    int i, Count = 0;
    Vec_PtrForEachEntry( Phys_LibPin_t *, pCell->vPins, pPin, i )
        if ( pPin->Dir == 0 )
            Count++;
    return Count;
}

static Phys_TimingArc_t * Phys_SuperArc( Phys_LibCell_t * pCell, int k )
{
    Phys_LibPin_t * pOut = Phys_SuperDrvPin( pCell );
    Phys_LibPin_t * pIn  = Phys_SuperInPin( pCell, k );
    if ( pOut == NULL || pIn == NULL )
        return NULL;
    return Phys_LibPinFindArc( pOut, pIn->pName );
}

/* the truth table of a one- or two-input cell over its input pins in
   pin order, or -1 when the function is missing or not expressible */
static int Phys_SuperTruth( Phys_LibCell_t * pCell )
{
    Phys_LibPin_t * pOut = Phys_SuperOutPin( pCell );
    Phys_LibPin_t * pIn0 = Phys_SuperInPin( pCell, 0 );
    Phys_LibPin_t * pIn1 = Phys_SuperInPin( pCell, 1 );
    Phys_Expr_t e;
    int Mask;
    if ( pOut == NULL || pIn0 == NULL )
        return -1;
    if ( pIn1 != NULL && Phys_SuperInNum(pCell) != 2 )
        return -1;
    e.p = pOut->pFunc;
    e.pVar0 = pIn0->pName;
    e.pVar1 = pIn1 != NULL ? pIn1->pName : NULL;
    e.fOk = 1;
    Mask = Phys_ExprOr( &e );
    if ( !e.fOk )
        return -1;
    Phys_ExprSkip( &e );
    if ( *e.p != 0 )
        return -1;
    if ( pIn1 == NULL )
        /* a one-input cell depends on the first bit only; keep bits 0,1 */
        Mask = (Mask & 1) | (((Mask >> 2) & 1) << 1);
    return Mask;
}

/*------------------------------------------------------------------------*/
/* minimum-area synthesis over the two-input Boolean functions            */
/*------------------------------------------------------------------------*/

typedef struct Phys_SuperCand_t_
{
    Phys_LibCell_t * pCell;
    int              Mask;    /* truth table over the input pins         */
    int              nIn;     /* 1 or 2                                  */
    int              fArc;    /* has a timing arc for at least one pin   */
} Phys_SuperCand_t;

typedef struct Phys_SuperNode_t_
{
    float Cost;                /* accumulated area                        */
    int   Depth;
    int   fValid;
    int   iCand;               /* chosen cell, -1 for a fanin             */
    int   Tab0;                /* operand tables                          */
    int   Tab1;                /* -1 for a one-input cell                 */
} Phys_SuperNode_t;

/* bits of the truth table: bit i is f(a,b) with i = a*2 + b; variable a
   is 0xC, variable b is 0xA */
#define PHYS_SUPER_TAB_A 0xC
#define PHYS_SUPER_TAB_B 0xA

static int Phys_SuperCompose1( int fCell, int fIn )
{
    int i, Out = 0;
    for ( i = 0; i < 4; i++ )
        Out |= ((fCell >> ((fIn >> i) & 1)) & 1) << i;
    return Out;
}

static int Phys_SuperCompose2( int fCell, int fIn0, int fIn1 )
{
    int i, Out = 0;
    for ( i = 0; i < 4; i++ )
        Out |= ((fCell >> (((fIn0 >> i) & 1) * 2 + ((fIn1 >> i) & 1))) & 1) << i;
    return Out;
}

static int Phys_SuperCollect( Phys_Lib_t * pLib, Phys_SuperCand_t * pCands, int nMax )
{
    Phys_LibCell_t * pCell;
    int i, nCands = 0;
    Vec_PtrForEachEntry( Phys_LibCell_t *, pLib->vCells, pCell, i )
    {
        int Mask, nIn;
        if ( nCands >= nMax )
            break;
        if ( pCell->fSequential || pCell->fDontUse || pCell->Area <= 0.0f )
            continue;
        if ( pCell->nOutputs != 1 )
            continue;
        nIn = Phys_SuperInNum( pCell );
        if ( nIn < 1 || nIn > 2 )
            continue;
        Mask = Phys_SuperTruth( pCell );
        if ( Mask < 0 )
            continue;
        /* a cell whose output does not depend on its input(s) is a
           constant and cannot help; a buffer only adds area */
        if ( nIn == 1 && (Mask == 0x0 || Mask == 0x3) )
            continue;
        if ( nIn == 2 && (Mask == 0x0 || Mask == 0xF) )
            continue;
        pCands[nCands].pCell = pCell;
        pCands[nCands].Mask  = Mask;
        pCands[nCands].nIn   = nIn;
        pCands[nCands].fArc  = Phys_SuperArc( pCell, 0 ) != NULL ||
                               (nIn == 2 && Phys_SuperArc( pCell, 1 ) != NULL);
        nCands++;
    }
    return nCands;
}

static int Phys_SuperNodeUpdate( Phys_SuperNode_t * pNode, float Cost, int Depth,
                                 int iCand, int Tab0, int Tab1 )
{
    if ( pNode->fValid && Cost > pNode->Cost + 1e-6f )
        return 0;
    if ( pNode->fValid && Cost > pNode->Cost - 1e-6f && Depth >= pNode->Depth )
        return 0;
    pNode->fValid = 1;
    pNode->Cost   = Cost;
    pNode->Depth  = Depth;
    pNode->iCand  = iCand;
    pNode->Tab0   = Tab0;
    pNode->Tab1   = Tab1;
    return 1;
}

static void Phys_SuperSynthesize( Phys_SuperCand_t * pCands, int nCands, Phys_SuperNode_t * pNodes )
{
    int i, t, u, fChanged = 1;
    for ( i = 0; i < 16; i++ )
    {
        pNodes[i].Cost   = PHYS_SUPER_INF;
        pNodes[i].Depth  = 1 << 30;
        pNodes[i].fValid = 0;
        pNodes[i].iCand  = -1;
        pNodes[i].Tab0   = -1;
        pNodes[i].Tab1   = -1;
    }
    pNodes[PHYS_SUPER_TAB_A].fValid = pNodes[PHYS_SUPER_TAB_B].fValid = 1;
    pNodes[PHYS_SUPER_TAB_A].Cost = pNodes[PHYS_SUPER_TAB_B].Cost = 0.0f;
    pNodes[PHYS_SUPER_TAB_A].Depth = pNodes[PHYS_SUPER_TAB_B].Depth = 0;
    while ( fChanged )
    {
        fChanged = 0;
        for ( i = 0; i < nCands; i++ )
        {
            Phys_LibCell_t * pCell = pCands[i].pCell;
            /* a cell without an arc has no delay model: only use it when
               nothing cheaper is available */
            float Area = (pCands[i].fArc ? 0.0f : 1e-4f) + pCell->Area;
            if ( pCands[i].nIn == 1 )
            {
                for ( t = 0; t < 16; t++ )
                {
                    int New;
                    if ( !pNodes[t].fValid )
                        continue;
                    New = Phys_SuperCompose1( pCands[i].Mask, t );
                    fChanged |= Phys_SuperNodeUpdate( &pNodes[New], pNodes[t].Cost + Area,
                                                      pNodes[t].Depth + 1, i, t, -1 );
                }
            }
            else
            {
                for ( t = 0; t < 16; t++ )
                {
                    if ( !pNodes[t].fValid )
                        continue;
                    for ( u = 0; u < 16; u++ )
                    {
                        int New;
                        if ( !pNodes[u].fValid )
                            continue;
                        New = Phys_SuperCompose2( pCands[i].Mask, t, u );
                        fChanged |= Phys_SuperNodeUpdate( &pNodes[New],
                                                          pNodes[t].Cost + pNodes[u].Cost + Area,
                                                          (pNodes[t].Depth > pNodes[u].Depth ?
                                                           pNodes[t].Depth : pNodes[u].Depth) + 1,
                                                          i, t, u );
                    }
                }
            }
        }
    }
}

/*------------------------------------------------------------------------*/
/* expansion of a synthesis solution into instances                       */
/*------------------------------------------------------------------------*/

typedef struct Phys_SuperInst_t_
{
    Phys_LibCell_t * pCell;
    int              nIn;     /* 1 or 2                                  */
    int              Op[2];   /* source index: 0,1 = fanin, 2+ = net     */
    int              iOut;    /* index of this instance's output         */
} Phys_SuperInst_t;

typedef struct Phys_SuperSolu_t_
{
    Phys_SuperNode_t * pNodes;
    Phys_SuperCand_t * pCands;
    Phys_SuperInst_t   pInsts[PHYS_SUPER_MAX_INST];
    int                nInsts;
    int                nIdx;   /* 2 + number of internal nets             */
    int                iRoot;  /* index of the output net                 */
    int                fOk;
} Phys_SuperSolu_t;

static int Phys_SuperExpandRec( Phys_SuperSolu_t * p, int Tab )
{
    Phys_SuperNode_t * pNode = &p->pNodes[Tab];
    int Op0, Op1;
    if ( !pNode->fValid || p->nInsts >= PHYS_SUPER_MAX_INST )
    {
        p->fOk = 0;
        return -1;
    }
    if ( pNode->iCand < 0 )
        return Tab == PHYS_SUPER_TAB_A ? 0 : 1;
    Op0 = Phys_SuperExpandRec( p, pNode->Tab0 );
    if ( !p->fOk )
        return -1;
    Op1 = pNode->Tab1 >= 0 ? Phys_SuperExpandRec( p, pNode->Tab1 ) : -1;
    if ( !p->fOk )
        return -1;
    {
        Phys_SuperInst_t * pInst = &p->pInsts[p->nInsts++];
        pInst->pCell  = p->pCands[pNode->iCand].pCell;
        pInst->nIn    = pNode->Tab1 >= 0 ? 2 : 1;
        pInst->Op[0]  = Op0;
        pInst->Op[1]  = Op1;
        pInst->iOut   = p->nIdx++;
        return pInst->iOut;
    }
}

static void Phys_SuperExpand( Phys_SuperSolu_t * p, int Tab )
{
    p->nInsts = 0;
    p->nIdx   = 2;
    p->iRoot  = -1;
    p->fOk    = 1;
    p->iRoot  = Phys_SuperExpandRec( p, Tab );
    if ( !p->fOk || p->nInsts == 0 )
        p->fOk = 0;
}

/*------------------------------------------------------------------------*/
/* physical model: capacitances and composed timing arcs                  */
/*------------------------------------------------------------------------*/

/* the capacitance seen by every net of the composition: the two external
   inputs and the internal nets */
static void Phys_SuperCaps( Phys_SuperSolu_t * p, float * pCaps )
{
    int i, k;
    for ( i = 0; i < p->nIdx; i++ )
        pCaps[i] = 0.0f;
    for ( i = 0; i < p->nInsts; i++ )
        for ( k = 0; k < p->pInsts[i].nIn; k++ )
        {
            Phys_LibPin_t * pPin = Phys_SuperInPin( p->pInsts[i].pCell, k );
            if ( pPin != NULL )
                pCaps[p->pInsts[i].Op[k]] += pPin->Cap;
        }
}

/* propagates one switching input through the composition: only the pins
   reachable from that input are sensitized, the other input is static */
static void Phys_SuperSim( Phys_SuperSolu_t * p, float * pCaps, int iIn, float Slew,
                           float Load, int fRise,
                           float * pArrR, float * pArrF, float * pSlewR, float * pSlewF )
{
    int i, k;
    for ( i = 0; i < p->nIdx; i++ )
    {
        pArrR[i] = pArrF[i] = -1.0f;
        pSlewR[i] = pSlewF[i] = Slew;
    }
    if ( fRise )
        pArrR[iIn] = 0.0f;
    else
        pArrF[iIn] = 0.0f;
    for ( i = 0; i < p->nInsts; i++ )
    {
        Phys_SuperInst_t * pInst = &p->pInsts[i];
        float NetLoad = pCaps[pInst->iOut] + (pInst->iOut == p->iRoot ? Load : 0.0f);
        float BestR = -1.0f, BestF = -1.0f, BestSlewR = Slew, BestSlewF = Slew;
        for ( k = 0; k < pInst->nIn; k++ )
        {
            int iSrc = pInst->Op[k];
            Phys_TimingArc_t * pArc = Phys_SuperArc( pInst->pCell, k );
            float InR = pArrR[iSrc], InF = pArrF[iSrc];
            float CandR, CandF, TransR, TransF;
            if ( pArc == NULL || pArc->pRise == NULL || pArc->pFall == NULL )
            {
                /* no delay data: pass the arrival through */
                if ( InR > BestR )
                    BestR = InR, BestSlewR = pSlewR[iSrc];
                if ( InF > BestF )
                    BestF = InF, BestSlewF = pSlewF[iSrc];
                continue;
            }
            if ( pArc->Sense < 0 )
            {
                CandR  = InF >= 0.0f ? InF + Phys_SurfaceLookup( pArc->pRise, pSlewF[iSrc], NetLoad ) : -1.0f;
                CandF  = InR >= 0.0f ? InR + Phys_SurfaceLookup( pArc->pFall, pSlewR[iSrc], NetLoad ) : -1.0f;
                TransR = Phys_SurfaceLookup( pArc->pRiseTrans, pSlewF[iSrc], NetLoad );
                TransF = Phys_SurfaceLookup( pArc->pFallTrans, pSlewR[iSrc], NetLoad );
            }
            else
            {
                CandR  = InR >= 0.0f ? InR + Phys_SurfaceLookup( pArc->pRise, pSlewR[iSrc], NetLoad ) : -1.0f;
                CandF  = InF >= 0.0f ? InF + Phys_SurfaceLookup( pArc->pFall, pSlewF[iSrc], NetLoad ) : -1.0f;
                TransR = Phys_SurfaceLookup( pArc->pRiseTrans, pSlewR[iSrc], NetLoad );
                TransF = Phys_SurfaceLookup( pArc->pFallTrans, pSlewF[iSrc], NetLoad );
            }
            if ( CandR > BestR )
                BestR = CandR, BestSlewR = TransR;
            if ( CandF > BestF )
                BestF = CandF, BestSlewF = TransF;
        }
        pArrR[pInst->iOut]  = BestR;
        pArrF[pInst->iOut]  = BestF;
        pSlewR[pInst->iOut] = BestSlewR;
        pSlewF[pInst->iOut] = BestSlewF;
    }
}

/* which external input feeds which output pin of every instance */
static void Phys_SuperSources( Phys_SuperSolu_t * p, int * pSrc )
{
    int i, k;
    for ( i = 0; i < p->nInsts; i++ )
    {
        int Mask = 0;
        for ( k = 0; k < p->pInsts[i].nIn; k++ )
        {
            int iSrc = p->pInsts[i].Op[k];
            Mask |= iSrc < 2 ? (1 << iSrc) : pSrc[iSrc - 2];
        }
        pSrc[i] = Mask;
    }
}

/* the terminal cell's arc to be used as the sampling grid */
static Phys_Surface_t * Phys_SuperGrid( Phys_Lib_t * pLib, Phys_SuperSolu_t * p,
                                        int iIn, int * pSrc )
{
    Phys_SuperInst_t * pInst = &p->pInsts[p->nInsts - 1];
    int k;
    for ( k = 0; k < pInst->nIn; k++ )
    {
        int iSrc = pInst->Op[k];
        int Mask = iSrc < 2 ? (1 << iSrc) : pSrc[iSrc - 2];
        Phys_TimingArc_t * pArc;
        if ( (Mask & (1 << iIn)) == 0 )
            continue;
        pArc = Phys_SuperArc( pInst->pCell, k );
        if ( pArc != NULL && pArc->pRise != NULL && pArc->pRise->n0 > 0 && pArc->pRise->n1 > 0 )
            return pArc->pRise;
    }
    /* fall back to any arc of the library, then to a fixed grid */
    {
        Phys_LibCell_t * pCell;
        int i;
        Vec_PtrForEachEntry( Phys_LibCell_t *, pLib->vCells, pCell, i )
        {
            Phys_TimingArc_t * pArc;
            int c;
            Vec_PtrForEachEntry( Phys_TimingArc_t *, pCell->vArcs, pArc, c )
                if ( pArc->pRise != NULL && pArc->pRise->n0 > 0 && pArc->pRise->n1 > 0 )
                    return pArc->pRise;
            if ( i > 64 )
                break;
        }
    }
    return NULL;
}

static Phys_Surface_t * Phys_SuperSurface( int n0, int n1, float * pSlew, float * pLoad, float * pData )
{
    Phys_Surface_t * p = Phys_SurfaceAlloc();
    int i;
    for ( i = 0; i < n0; i++ )
        Vec_FltPush( p->vIndex0, pSlew[i] );
    for ( i = 0; i < n1; i++ )
        Vec_FltPush( p->vIndex1, pLoad[i] );
    for ( i = 0; i < n0 * n1; i++ )
        Vec_FltPush( p->vData, pData[i] );
    p->n0 = n0;
    p->n1 = n1;
    return p;
}

static char * Phys_SuperStrDup( const char * pStr )
{
    char * pNew = ABC_ALLOC( char, strlen(pStr) + 1 );
    strcpy( pNew, pStr );
    return pNew;
}

/* the structure of the composition, with A and B for the input pins */
static char * Phys_SuperFormula( Phys_SuperSolu_t * p, int Tab )
{
    Phys_SuperNode_t * pNode = &p->pNodes[Tab];
    char * pName, * pOp0, * pOp1, * pRes;
    int nSize;
    if ( pNode->iCand < 0 )
        return Phys_SuperStrDup( Tab == PHYS_SUPER_TAB_A ? "A" : "B" );
    pName = p->pCands[pNode->iCand].pCell->pName;
    pOp0 = Phys_SuperFormula( p, pNode->Tab0 );
    if ( pNode->Tab1 < 0 )
    {
        nSize = (int)strlen(pName) + (int)strlen(pOp0) + 4;
        pRes = ABC_ALLOC( char, nSize );
        snprintf( pRes, nSize, "%s(%s)", pName, pOp0 );
        ABC_FREE( pOp0 );
        return pRes;
    }
    pOp1 = Phys_SuperFormula( p, pNode->Tab1 );
    nSize = (int)strlen(pName) + (int)strlen(pOp0) + (int)strlen(pOp1) + 6;
    pRes = ABC_ALLOC( char, nSize );
    snprintf( pRes, nSize, "%s(%s, %s)", pName, pOp0, pOp1 );
    ABC_FREE( pOp0 );
    ABC_FREE( pOp1 );
    return pRes;
}

/* builds the synthetic cell of one pattern from its solution */
static Phys_LibCell_t * Phys_SuperMakeCell( Phys_Lib_t * pLib, Phys_SuperSolu_t * p,
                                            int p0, int p1, int q, int Tab )
{
    Phys_LibCell_t * pCell;
    Phys_LibPin_t * pPinA, * pPinB, * pPinY;
    float * pCaps;
    int * pSrc;
    float * pArrR, * pArrF, * pSlewR, * pSlewF;
    int i, k;
    float Area = 0.0f;
    char Name[64];

    pCaps = ABC_ALLOC( float, p->nIdx );
    pSrc  = ABC_ALLOC( int, p->nInsts + 1 );
    pArrR = ABC_ALLOC( float, p->nIdx );
    pArrF = ABC_ALLOC( float, p->nIdx );
    pSlewR = ABC_ALLOC( float, p->nIdx );
    pSlewF = ABC_ALLOC( float, p->nIdx );

    Phys_SuperCaps( p, pCaps );
    Phys_SuperSources( p, pSrc );
    for ( i = 0; i < p->nInsts; i++ )
        Area += p->pInsts[i].pCell->Area;

    pCell = ABC_CALLOC( Phys_LibCell_t, 1 );
    snprintf( Name, sizeof(Name), "SUPER_%d%d_%d", p0, p1, q );
    pCell->pName    = Phys_SuperStrDup( Name );
    pCell->pFormula = Phys_SuperFormula( p, Tab );
    pCell->Area     = Area;
    pCell->nInputs  = 2;
    pCell->nOutputs = 1;
    pCell->CellId   = -1;
    pCell->MacroId  = -1;
    pCell->vPins    = Vec_PtrAlloc( 3 );
    pCell->vArcs    = Vec_PtrAlloc( 2 );

    pPinA = ABC_CALLOC( Phys_LibPin_t, 1 );
    pPinA->pName = Phys_SuperStrDup( "A" );
    pPinA->Dir   = 0;
    pPinA->Cap   = pCaps[0];
    pPinA->vArcs = Vec_PtrAlloc( 0 );
    pPinB = ABC_CALLOC( Phys_LibPin_t, 1 );
    pPinB->pName = Phys_SuperStrDup( "B" );
    pPinB->Dir   = 0;
    pPinB->Cap   = pCaps[1];
    pPinB->vArcs = Vec_PtrAlloc( 0 );
    pPinY = ABC_CALLOC( Phys_LibPin_t, 1 );
    pPinY->pName = Phys_SuperStrDup( "Y" );
    pPinY->Dir   = 1;
    pPinY->vArcs = Vec_PtrAlloc( 2 );
    Vec_PtrPush( pCell->vPins, pPinA );
    Vec_PtrPush( pCell->vPins, pPinB );
    Vec_PtrPush( pCell->vPins, pPinY );

    for ( k = 0; k < 2; k++ )
    {
        Phys_Surface_t * pGrid = Phys_SuperGrid( pLib, p, k, pSrc );
        float DefaultSlew[5] = { 5.0f, 10.0f, 20.0f, 40.0f, 80.0f };
        float DefaultLoad[5] = { 0.0005f, 0.001f, 0.002f, 0.004f, 0.008f };
        float * pSlew = pGrid != NULL ? pGrid->vIndex0->pArray : DefaultSlew;
        float * pLoad = pGrid != NULL ? pGrid->vIndex1->pArray : DefaultLoad;
        int nSlew = pGrid != NULL ? pGrid->n0 : 5;
        int nLoad = pGrid != NULL ? pGrid->n1 : 5;
        float * pRise, * pFall, * pRiseT, * pFallT;
        int a, b;
        if ( nSlew < 1 ) nSlew = 1;
        if ( nLoad < 1 ) nLoad = 1;
        pRise  = ABC_ALLOC( float, nSlew * nLoad );
        pFall  = ABC_ALLOC( float, nSlew * nLoad );
        pRiseT = ABC_ALLOC( float, nSlew * nLoad );
        pFallT = ABC_ALLOC( float, nSlew * nLoad );
        for ( a = 0; a < nSlew; a++ )
            for ( b = 0; b < nLoad; b++ )
            {
                int iPoint = a * nLoad + b;
                float Slew = pSlew[a], Load = pLoad[b];
                float Rise, Fall, RiseT, FallT;
                Phys_SuperSim( p, pCaps, k, Slew, Load, 1, pArrR, pArrF, pSlewR, pSlewF );
                Rise  = pArrR[p->iRoot] >= 0.0f ? pArrR[p->iRoot] : -1.0f;
                RiseT = pArrR[p->iRoot] >= 0.0f ? pSlewR[p->iRoot] : -1.0f;
                Fall  = pArrF[p->iRoot] >= 0.0f ? pArrF[p->iRoot] : -1.0f;
                FallT = pArrF[p->iRoot] >= 0.0f ? pSlewF[p->iRoot] : -1.0f;
                Phys_SuperSim( p, pCaps, k, Slew, Load, 0, pArrR, pArrF, pSlewR, pSlewF );
                if ( Rise < 0.0f )
                    Rise = pArrR[p->iRoot], RiseT = pSlewR[p->iRoot];
                if ( Fall < 0.0f )
                    Fall = pArrF[p->iRoot], FallT = pSlewF[p->iRoot];
                pRise[iPoint]  = Rise < 0.0f ? 0.0f : Rise;
                pFall[iPoint]  = Fall < 0.0f ? 0.0f : Fall;
                pRiseT[iPoint] = RiseT < 0.0f ? Slew : RiseT;
                pFallT[iPoint] = FallT < 0.0f ? Slew : FallT;
            }
        {
            Phys_TimingArc_t * pArc = ABC_CALLOC( Phys_TimingArc_t, 1 );
            pArc->pRelatedPin = Phys_SuperStrDup( k == 0 ? "A" : "B" );
            /* the pattern is unate in each fanin: a complemented fanin
               inverts the arc, and so does an inverting output phase */
            pArc->Sense = ((k == 0 ? p0 : p1) ^ q) ? -1 : 1;
            pArc->Type  = 0;
            pArc->pRise      = Phys_SuperSurface( nSlew, nLoad, pSlew, pLoad, pRise );
            pArc->pFall      = Phys_SuperSurface( nSlew, nLoad, pSlew, pLoad, pFall );
            pArc->pRiseTrans = Phys_SuperSurface( nSlew, nLoad, pSlew, pLoad, pRiseT );
            pArc->pFallTrans = Phys_SuperSurface( nSlew, nLoad, pSlew, pLoad, pFallT );
            Vec_PtrPush( pPinY->vArcs, pArc );
            Vec_PtrPush( pCell->vArcs, pArc );
        }
        ABC_FREE( pRise );
        ABC_FREE( pFall );
        ABC_FREE( pRiseT );
        ABC_FREE( pFallT );
    }
    ABC_FREE( pCaps );
    ABC_FREE( pSrc );
    ABC_FREE( pArrR );
    ABC_FREE( pArrF );
    ABC_FREE( pSlewR );
    ABC_FREE( pSlewF );
    return pCell;
}

/*------------------------------------------------------------------------*/
/* footprint and pin geometry of a composed cell                          */
/*------------------------------------------------------------------------*/

/* The members of a supergate are placed left to right in one row.  The
   input pins of the supergate sit where the first member pin they drive
   sits; the output sits where the root member's output sits.  Everything
   is measured from the center of the composed cell, the same convention
   Phys_ManLitPinOffset uses for a LEF macro.  When any member has no LEF
   macro, the geometry is left empty and the placer falls back to the
   Liberty area. */
static void Phys_SuperGeom( Phys_LefTech_t * pLef, Phys_SuperSolu_t * p, Phys_LibCell_t * pCell )
{
    Phys_LefMacro_t * pMacro;
    int * pX, nTot = 0, H = 0, i, k;
    if ( pLef == NULL )
        return;
    pX = ABC_ALLOC( int, p->nInsts + 1 );
    for ( i = 0; i < p->nInsts; i++ )
    {
        pMacro = Phys_LefFindMacro( pLef, p->pInsts[i].pCell->pName );
        if ( pMacro == NULL || pMacro->Width <= 0 || pMacro->Height <= 0 )
        {
            ABC_FREE( pX );
            return;
        }
        pX[i] = nTot;
        nTot += pMacro->Width;
        if ( H < pMacro->Height )
            H = pMacro->Height;
    }
    pCell->Width  = nTot;
    pCell->Height = H;
    pCell->pPinDx = ABC_CALLOC( int, 2 );
    pCell->pPinDy = ABC_CALLOC( int, 2 );
    for ( k = 0; k < 2; k++ )
    {
        int iInst = -1, nPin = 0;
        for ( i = 0; i < p->nInsts && iInst < 0; i++ )
            for ( nPin = 0; nPin < p->pInsts[i].nIn; nPin++ )
                if ( p->pInsts[i].Op[nPin] == k )
                {
                    iInst = i;
                    break;
                }
        if ( iInst < 0 )
            continue;
        pMacro = Phys_LefFindMacro( pLef, p->pInsts[iInst].pCell->pName );
        {
            Phys_LibPin_t * pInLib = Phys_SuperInPin( p->pInsts[iInst].pCell, nPin );
            Phys_LefPin_t * pPin = pInLib ? Phys_LefMacroFindPin( pMacro, pInLib->pName ) : NULL;
            if ( pPin == NULL )
                continue;
            pCell->pPinDx[k] = pX[iInst] + pPin->x - pMacro->OriginX - pCell->Width / 2;
            pCell->pPinDy[k] = pPin->y - pMacro->OriginY - pCell->Height / 2;
        }
    }
    /* the output pin of the root instance */
    {
        int iRoot = p->nInsts - 1;
        Phys_LibPin_t * pOutLib = Phys_SuperDrvPin( p->pInsts[iRoot].pCell );
        pMacro = Phys_LefFindMacro( pLef, p->pInsts[iRoot].pCell->pName );
        if ( pOutLib != NULL && pMacro != NULL )
        {
            Phys_LefPin_t * pPin = Phys_LefMacroFindPin( pMacro, pOutLib->pName );
            if ( pPin != NULL )
            {
                pCell->DxOut = pX[iRoot] + pPin->x - pMacro->OriginX - pCell->Width / 2;
                pCell->DyOut = pPin->y - pMacro->OriginY - pCell->Height / 2;
            }
        }
    }
    ABC_FREE( pX );
}

/*------------------------------------------------------------------------*/
/* public interface                                                       */
/*------------------------------------------------------------------------*/

int Phys_SuperBuild( Phys_Lib_t * pLib, Phys_LefTech_t * pLef )
{
    Phys_SuperCand_t * pCands;
    Phys_SuperNode_t Nodes[16];
    Phys_SuperSolu_t Solu, * p = &Solu;
    int nCands, i, p0, p1, q, Count = 0;
    if ( pLib == NULL )
        return 0;
    if ( pLib->vSuperCells == NULL )
        pLib->vSuperCells = Vec_PtrAlloc( 8 );
    if ( pLib->fSuperBuilt )
    {
        for ( i = 0; i < 8; i++ )
            Count += Vec_PtrEntry( pLib->vSuperCells, i ) != NULL;
        return Count;
    }
    pLib->fSuperBuilt = 1;
    while ( Vec_PtrSize(pLib->vSuperCells) < 8 )
        Vec_PtrPush( pLib->vSuperCells, NULL );
    pCands = ABC_ALLOC( Phys_SuperCand_t, Vec_PtrSize(pLib->vCells) + 1 );
    nCands = Phys_SuperCollect( pLib, pCands, Vec_PtrSize(pLib->vCells) );
    Phys_SuperSynthesize( pCands, nCands, Nodes );

    p->pNodes = Nodes;
    p->pCands = pCands;
    for ( p0 = 0; p0 < 2; p0++ )
        for ( p1 = 0; p1 < 2; p1++ )
            for ( q = 0; q < 2; q++ )
            {
                int Tab = 0, a, b, iPat = (p0 << 2) | (p1 << 1) | q;
                for ( a = 0; a < 2; a++ )
                    for ( b = 0; b < 2; b++ )
                        if ( ((a ^ p0) & (b ^ p1)) ^ q )
                            Tab |= 1 << (a * 2 + b);
                if ( !Nodes[Tab].fValid || Nodes[Tab].iCand < 0 )
                    continue;
                Phys_SuperExpand( p, Tab );
                if ( !p->fOk )
                    continue;
                {
                    Phys_LibCell_t * pCell = Phys_SuperMakeCell( pLib, p, p0, p1, q, Tab );
                    Phys_SuperGeom( pLef, p, pCell );
                    Vec_PtrWriteEntry( pLib->vSuperCells, iPat, pCell );
                }
                Count++;
            }
    ABC_FREE( pCands );
    return Count;
}

Phys_LibCell_t * Phys_SuperCell( Phys_Lib_t * pLib, int p0, int p1, int q )
{
    int iPat;
    if ( pLib == NULL || pLib->vSuperCells == NULL )
        return NULL;
    iPat = (p0 << 2) | (p1 << 1) | q;
    if ( iPat < 0 || iPat >= Vec_PtrSize(pLib->vSuperCells) )
        return NULL;
    return (Phys_LibCell_t *)Vec_PtrEntry( pLib->vSuperCells, iPat );
}

int Phys_SuperPattern( Phys_Lib_t * pLib, Phys_LibCell_t * pCell )
{
    int i;
    if ( pLib == NULL || pLib->vSuperCells == NULL || pCell == NULL )
        return -1;
    for ( i = 0; i < Vec_PtrSize(pLib->vSuperCells); i++ )
        if ( Vec_PtrEntry(pLib->vSuperCells, i) == pCell )
            return i;
    return -1;
}

void Phys_SuperPrint( Phys_Lib_t * pLib, Phys_LefTech_t * pLef )
{
    int p0, p1, q, iPat, nReal = 0;
    double Dbu = pLef != NULL ? (double)pLef->Units.Dbu : 1000.0;
    Phys_LibCell_t * pCell;
    if ( pLib == NULL || pLib->vSuperCells == NULL )
        return;
    for ( iPat = 0; iPat < Vec_PtrSize(pLib->vSuperCells); iPat++ )
        nReal += Vec_PtrEntry( pLib->vSuperCells, iPat ) != NULL;
    printf( "Supergates of the library (%d of 8 patterns realized):\n", nReal );
    printf( "  %-4s %-4s %-4s | %8s %11s | %s\n", "p0", "p1", "out", "area", "w x h (um)", "structure" );
    for ( p0 = 0; p0 < 2; p0++ )
        for ( p1 = 0; p1 < 2; p1++ )
            for ( q = 0; q < 2; q++ )
            {
                iPat = (p0 << 2) | (p1 << 1) | q;
                pCell = (Phys_LibCell_t *)Vec_PtrEntry( pLib->vSuperCells, iPat );
                if ( pCell == NULL )
                {
                    printf( "  %-4d %-4d %-4d | %8s %11s | unrealizable\n", p0, p1, q, "-", "-" );
                    continue;
                }
                printf( "  %-4d %-4d %-4d | %8.2f %5.2f x %4.2f | %s\n", p0, p1, q, pCell->Area,
                        pCell->Width / Dbu, pCell->Height / Dbu,
                        pCell->pFormula ? pCell->pFormula : "" );
            }
}

ABC_NAMESPACE_IMPL_END
