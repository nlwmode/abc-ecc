/**Function*************************************************************

  Synopsis    [Liberty reader]

  Description [Liberty parser built on flex/bison.  The grammar lives in
               physLibertyLex.l and physLibertyYacc.y; the generated
               files are included at the end of this file.  The reader
               stores the NLDM data needed by the physical-synthesis flow
               in the Phys_Lib_t structures and is independent from
               sclLiberty.c.

               Supported: library, units, lu_table_template, cell, area,
               dont_use, pin, direction, capacitance, max_capacitance,
               max_transition, clock, function, timing, related_pin,
               timing_sense, timing_type, cell_rise, cell_fall,
               rise_transition, fall_transition, index_1, index_2,
               values.  Unknown constructs are skipped.]

  SideEffects []

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "phys/phys.h"

ABC_NAMESPACE_IMPL_START


/*------------------------------------------------------------------------*/
/* small string helpers                                                   */
/*------------------------------------------------------------------------*/

static char * Lib_StrDup( const char * pStr )
{
    char * pNew;
    if ( pStr == NULL )
        pStr = "";
    pNew = ABC_ALLOC( char, strlen(pStr) + 1 );
    strcpy( pNew, pStr );
    return pNew;
}

static char * Lib_StrCat( const char * pFirst, const char * pSecond )
{
    char * pNew;
    int nFirst  = pFirst  ? (int)strlen(pFirst)  : 0;
    int nSecond = pSecond ? (int)strlen(pSecond) : 0;
    pNew = ABC_ALLOC( char, nFirst + nSecond + 2 );
    if ( nFirst )
        memcpy( pNew, pFirst, (size_t)nFirst );
    if ( nFirst && nSecond )
        pNew[nFirst++] = ' ';
    if ( nSecond )
        memcpy( pNew + nFirst, pSecond, (size_t)nSecond );
    pNew[nFirst + nSecond] = '\0';
    return pNew;
}

static char * Lib_StrJoin( const char * pFirst, const char * pSecond )
{
    char * pNew;
    int nFirst  = pFirst  ? (int)strlen(pFirst)  : 0;
    int nSecond = pSecond ? (int)strlen(pSecond) : 0;
    pNew = ABC_ALLOC( char, nFirst + nSecond + 3 );
    memcpy( pNew, pFirst ? pFirst : "", (size_t)nFirst );
    if ( nFirst )
    {
        pNew[nFirst++] = ',';
        pNew[nFirst++] = ' ';
    }
    memcpy( pNew + nFirst, pSecond ? pSecond : "", (size_t)nSecond );
    pNew[nFirst + nSecond] = '\0';
    return pNew;
}

static char * Lib_StrWrap( const char * pPre, const char * pMid, const char * pPost )
{
    char * pNew;
    int n0 = (int)strlen(pPre);
    int n1 = pMid  ? (int)strlen(pMid)  : 0;
    int n2 = (int)strlen(pPost);
    pNew = ABC_ALLOC( char, n0 + n1 + n2 + 3 );
    memcpy( pNew, pPre, (size_t)n0 );
    pNew[n0] = ' ';
    memcpy( pNew + n0 + 1, pMid ? pMid : "", (size_t)n1 );
    pNew[n0 + 1 + n1] = ' ';
    memcpy( pNew + n0 + 1 + n1 + 1, pPost, (size_t)n2 );
    pNew[n0 + 1 + n1 + 1 + n2] = '\0';
    return pNew;
}

static char * Lib_StrNum( double Value )
{
    char Buffer[64];
    snprintf( Buffer, sizeof(Buffer), "%.15g", Value );
    return Lib_StrDup( Buffer );
}

static char * Lib_FirstWord( const char * pStr )
{
    const char * pBeg, * pEnd;
    char * pNew;
    int nLen;
    if ( pStr == NULL )
        return Lib_StrDup( "" );
    while ( *pStr && isspace((unsigned char)*pStr) )
        pStr++;
    pBeg = pStr;
    while ( *pStr && !isspace((unsigned char)*pStr) && *pStr != ',' )
        pStr++;
    pEnd = pStr;
    nLen = (int)(pEnd - pBeg);
    pNew = ABC_ALLOC( char, nLen + 1 );
    memcpy( pNew, pBeg, (size_t)nLen );
    pNew[nLen] = '\0';
    return pNew;
}

static char * Lib_Unquote( const char * pQuoted )
{
    char * pOut;
    int i, k = 0, n = (int)strlen(pQuoted);
    if ( n < 2 )
        return Lib_StrDup( "" );
    pOut = ABC_ALLOC( char, n + 1 );
    for ( i = 1; i < n - 1; i++ )
    {
        if ( pQuoted[i] == '\\' && i + 1 < n - 1 )
        {
            i++;
            if ( pQuoted[i] == '\n' )
                continue;
            if ( pQuoted[i] == '\r' )
            {
                if ( i + 1 < n - 1 && pQuoted[i+1] == '\n' )
                    i++;
                continue;
            }
            pOut[k++] = pQuoted[i];
        }
        else
            pOut[k++] = pQuoted[i];
    }
    pOut[k] = '\0';
    return pOut;
}


/*------------------------------------------------------------------------*/
/* numeric helpers                                                        */
/*------------------------------------------------------------------------*/

static void Lib_ParseFloatList( const char * pText, Vec_Flt_t * vValues )
{
    const char * p = pText;
    char * pEnd;
    double Value;
    while ( *p )
    {
        Value = strtod( p, &pEnd );
        if ( pEnd == p )
        {
            p++;
            continue;
        }
        Vec_FltPush( vValues, (float)Value );
        p = pEnd;
    }
}

static void Lib_ScaleFloats( Vec_Flt_t * vValues, float Scale )
{
    int i;
    float * pArray;
    if ( vValues == NULL || Scale == 1.0f )
        return;
    pArray = Vec_FltArray( vValues );
    for ( i = 0; i < Vec_FltSize(vValues); i++ )
        pArray[i] *= Scale;
}

static void Lib_AppendFloats( Vec_Flt_t * vDst, Vec_Flt_t * vSrc )
{
    int i;
    if ( vSrc == NULL )
        return;
    for ( i = 0; i < Vec_FltSize(vSrc); i++ )
        Vec_FltPush( vDst, Vec_FltEntry(vSrc, i) );
}

static int Lib_StrHas( const char * pStr, const char * pSub )
{
    return pStr != NULL && strstr( pStr, pSub ) != NULL;
}

static int Lib_VarIsTime( const char * pVar )
{
    return Lib_StrHas( pVar, "transition" ) || Lib_StrHas( pVar, "time" ) ||
           Lib_StrHas( pVar, "slew" );
}

static int Lib_VarIsCap( const char * pVar )
{
    return Lib_StrHas( pVar, "capacitance" ) || Lib_StrHas( pVar, "load" ) ||
           Lib_StrHas( pVar, "cap" );
}

static float Lib_ParseUnitScale( const char * pText, const char * pWhat )
{
    double Value = 0.0;
    char * pEnd;
    Value = strtod( pText, &pEnd );
    if ( pEnd == pText )
        Value = 1.0;
    if ( !strcmp( pWhat, "time" ) )
    {
        if ( strstr( pEnd, "fs" ) ) return (float)(Value * 1e-3);
        if ( strstr( pEnd, "ps" ) ) return (float)(Value * 1.0);
        if ( strstr( pEnd, "ns" ) ) return (float)(Value * 1e3);
        if ( strstr( pEnd, "us" ) ) return (float)(Value * 1e6);
        return (float)(Value * 1e3); /* default: ns */
    }
    if ( !strcmp( pWhat, "cap" ) )
    {
        if ( strstr( pEnd, "ff" ) ) return (float)(Value * 1e-3);
        if ( strstr( pEnd, "pf" ) ) return (float)(Value * 1.0);
        if ( strstr( pEnd, "nf" ) ) return (float)(Value * 1e3);
        if ( strstr( pEnd, "uf" ) ) return (float)(Value * 1e6);
        return (float)Value; /* default: pF */
    }
    if ( !strcmp( pWhat, "res" ) )
    {
        if ( strstr( pEnd, "kohm" ) ) return (float)Value;
        if ( strstr( pEnd, "ohm" ) ) return (float)(Value * 1e-3);
        return (float)Value; /* default: kOhm */
    }
    return (float)Value;
}


/*------------------------------------------------------------------------*/
/* data structures                                                        */
/*------------------------------------------------------------------------*/

static Phys_Template_t * Phys_TemplateAlloc( const char * pName )
{
    Phys_Template_t * p = ABC_CALLOC( Phys_Template_t, 1 );
    p->pName   = Lib_StrDup( pName );
    p->vIndex1 = Vec_FltAlloc( 8 );
    p->vIndex2 = Vec_FltAlloc( 8 );
    return p;
}

static void Phys_TemplateFree( Phys_Template_t * p )
{
    if ( p == NULL ) return;
    ABC_FREE( p->pName );
    ABC_FREE( p->pVar1 );
    ABC_FREE( p->pVar2 );
    Vec_FltFree( p->vIndex1 );
    Vec_FltFree( p->vIndex2 );
    ABC_FREE( p );
}

static Phys_Template_t * Lib_FindTemplate( Phys_Lib_t * pLib, const char * pName )
{
    Phys_Template_t * pTemp;
    int i;
    if ( pName == NULL || pName[0] == '\0' )
        return NULL;
    Vec_PtrForEachEntry( Phys_Template_t *, pLib->vTemplates, pTemp, i )
        if ( !strcmp( pTemp->pName, pName ) )
            return pTemp;
    return NULL;
}

static Phys_Template_t * Lib_FindTemplateByBase( Phys_Lib_t * pLib, const char * pName )
{
    Phys_Template_t * pTemp;
    int i;
    if ( pName == NULL || pName[0] == '\0' )
        return NULL;
    Vec_PtrForEachEntry( Phys_Template_t *, pLib->vTemplates, pTemp, i )
    {
        const char * pBar = strchr( pTemp->pName, '|' );
        int nBase = pBar ? (int)(pBar - pTemp->pName) : (int)strlen(pTemp->pName);
        if ( (int)strlen(pName) == nBase && !strncmp( pTemp->pName, pName, (size_t)nBase ) )
            return pTemp;
    }
    return NULL;
}

static Phys_Surface_t * Phys_SurfaceAlloc( void )
{
    Phys_Surface_t * p = ABC_CALLOC( Phys_Surface_t, 1 );
    p->vIndex0 = Vec_FltAlloc( 8 );
    p->vIndex1 = Vec_FltAlloc( 8 );
    p->vData   = Vec_FltAlloc( 64 );
    return p;
}

static void Phys_SurfaceFree( Phys_Surface_t * p )
{
    if ( p == NULL ) return;
    Vec_FltFree( p->vIndex0 );
    Vec_FltFree( p->vIndex1 );
    Vec_FltFree( p->vData );
    ABC_FREE( p );
}

static Phys_TimingArc_t * Phys_ArcAlloc( void )
{
    return ABC_CALLOC( Phys_TimingArc_t, 1 );
}

static void Phys_ArcFree( Phys_TimingArc_t * p )
{
    if ( p == NULL ) return;
    ABC_FREE( p->pRelatedPin );
    Phys_SurfaceFree( p->pRise );
    Phys_SurfaceFree( p->pFall );
    Phys_SurfaceFree( p->pRiseTrans );
    Phys_SurfaceFree( p->pFallTrans );
    ABC_FREE( p );
}

static Phys_LibPin_t * Phys_LibPinAlloc( const char * pName )
{
    Phys_LibPin_t * p = ABC_CALLOC( Phys_LibPin_t, 1 );
    p->pName = Lib_StrDup( pName );
    p->vArcs = Vec_PtrAlloc( 4 );
    p->Dir   = -1;
    return p;
}

static void Phys_LibPinFree( Phys_LibPin_t * p )
{
    Phys_TimingArc_t * pArc;
    int i;
    if ( p == NULL ) return;
    ABC_FREE( p->pName );
    ABC_FREE( p->pFunc );
    Vec_PtrForEachEntry( Phys_TimingArc_t *, p->vArcs, pArc, i )
        Phys_ArcFree( pArc );
    Vec_PtrFree( p->vArcs );
    ABC_FREE( p );
}

static int Lib_PinDirFromString( const char * pStr )
{
    if ( !strcmp( pStr, "input" ) )  return 0;
    if ( !strcmp( pStr, "output" ) ) return 1;
    if ( !strcmp( pStr, "inout" ) )  return 2;
    if ( !strcmp( pStr, "internal" ) ) return 3;
    return -1;
}

static int Lib_ParseTimingType( const char * pStr )
{
    if ( !strcmp( pStr, "combinational" ) ||
         !strcmp( pStr, "combinational_rise" ) ||
         !strcmp( pStr, "combinational_fall" ) ) return 0;
    if ( !strcmp( pStr, "rising_edge" ) )  return 1;
    if ( !strcmp( pStr, "falling_edge" ) ) return 2;
    if ( strstr( pStr, "setup" ) )  return 3;
    if ( strstr( pStr, "hold" ) )   return 4;
    if ( strstr( pStr, "recovery" ) ) return 5;
    if ( strstr( pStr, "removal" ) )  return 6;
    return 7;
}

static int Lib_ParseTimingSense( const char * pStr )
{
    if ( !strcmp( pStr, "positive_unate" ) ) return 1;
    if ( !strcmp( pStr, "negative_unate" ) ) return -1;
    return 0;
}

static Phys_LibCell_t * Phys_LibCellAlloc( const char * pName )
{
    Phys_LibCell_t * p = ABC_CALLOC( Phys_LibCell_t, 1 );
    p->pName   = Lib_StrDup( pName );
    p->vPins   = Vec_PtrAlloc( 16 );
    p->vArcs   = Vec_PtrAlloc( 16 );
    p->CellId  = -1;
    p->MacroId = -1;
    return p;
}

static void Phys_LibCellFree( Phys_LibCell_t * p )
{
    Phys_LibPin_t * pPin;
    int i;
    if ( p == NULL ) return;
    ABC_FREE( p->pName );
    Vec_PtrForEachEntry( Phys_LibPin_t *, p->vPins, pPin, i )
        Phys_LibPinFree( pPin );
    Vec_PtrFree( p->vPins );
    Vec_PtrFree( p->vArcs );
    ABC_FREE( p );
}


/*------------------------------------------------------------------------*/
/* parse context and semantic actions                                     */
/*------------------------------------------------------------------------*/

enum {
    LIB_FRM_LIBRARY = 0,
    LIB_FRM_CELL,
    LIB_FRM_PIN,
    LIB_FRM_TIMING,
    LIB_FRM_TEMPLATE,
    LIB_FRM_TABLE,
    LIB_FRM_SEQ,
    LIB_FRM_OTHER
};

typedef struct Lib_Frame_t_
{
    int      Type;
    void *   pObj;
    void *   pParent;
    int      TableKind;    /* 0=rise, 1=fall, 2=rise_trans, 3=fall_trans */
    char *   pTemplate;
} Lib_Frame_t;

typedef struct Lib_Ctx_t_
{
    Phys_Lib_t * pLib;
    Vec_Ptr_t *  vStack;
    int          nErrors;
    int          fSeenLibrary;
} Lib_Ctx_t;

static Lib_Ctx_t * s_pLibCtx = NULL;

static Lib_Ctx_t * Lib_ParseCtx( void ) { return s_pLibCtx; }

static void Lib_ParseError( const char * pMsg )
{
    extern int Lib_lineno;
    fprintf( stderr, "Liberty parse error near line %d: %s\n", Lib_lineno, pMsg );
    if ( s_pLibCtx )
        s_pLibCtx->nErrors++;
}

static Lib_Frame_t * Lib_FrameTop( Lib_Ctx_t * g )
{
    if ( g == NULL || Vec_PtrSize(g->vStack) == 0 )
        return NULL;
    return (Lib_Frame_t *)Vec_PtrEntry( g->vStack, Vec_PtrSize(g->vStack) - 1 );
}

static void Lib_PushFrame( Lib_Ctx_t * g, int Type, void * pObj, void * pParent )
{
    Lib_Frame_t * f = ABC_CALLOC( Lib_Frame_t, 1 );
    f->Type   = Type;
    f->pObj   = pObj;
    f->pParent= pParent;
    Vec_PtrPush( g->vStack, f );
}

static void Lib_FrameFree( Lib_Frame_t * f )
{
    if ( f == NULL ) return;
    ABC_FREE( f->pTemplate );
    ABC_FREE( f );
}

static Phys_LibCell_t * Lib_CurrentCell( Lib_Ctx_t * g )
{
    int i;
    for ( i = Vec_PtrSize(g->vStack) - 1; i >= 0; i-- )
    {
        Lib_Frame_t * f = (Lib_Frame_t *)Vec_PtrEntry( g->vStack, i );
        if ( f->Type == LIB_FRM_CELL )
            return (Phys_LibCell_t *)f->pObj;
    }
    return NULL;
}

static void Lib_PushLibrary( Lib_Ctx_t * g, const char * pArgs )
{
    char * pName = Lib_FirstWord( pArgs );
    if ( g->pLib->pName == NULL && pName[0] )
        g->pLib->pName = Lib_StrDup( pName );
    g->fSeenLibrary = 1;
    ABC_FREE( pName );
    Lib_PushFrame( g, LIB_FRM_LIBRARY, NULL, NULL );
}

static void Lib_PushCell( Lib_Ctx_t * g, const char * pArgs )
{
    char * pName = Lib_FirstWord( pArgs );
    Phys_LibCell_t * pCell = Phys_LibCellAlloc( pName );
    ABC_FREE( pName );
    Lib_PushFrame( g, LIB_FRM_CELL, pCell, NULL );
}

static void Lib_PushPin( Lib_Ctx_t * g, const char * pArgs )
{
    Lib_Frame_t * pParent = Lib_FrameTop( g );
    if ( pParent != NULL && pParent->Type == LIB_FRM_CELL )
    {
        char * pName = Lib_FirstWord( pArgs );
        Phys_LibPin_t * pPin = Phys_LibPinAlloc( pName );
        ABC_FREE( pName );
        Lib_PushFrame( g, LIB_FRM_PIN, pPin, pParent->pObj );
    }
    else
        Lib_PushFrame( g, LIB_FRM_OTHER, NULL, NULL );
}

static void Lib_PushTiming( Lib_Ctx_t * g )
{
    Lib_Frame_t * pParent = Lib_FrameTop( g );
    if ( pParent != NULL && pParent->Type == LIB_FRM_PIN )
        Lib_PushFrame( g, LIB_FRM_TIMING, Phys_ArcAlloc(), pParent->pObj );
    else
        Lib_PushFrame( g, LIB_FRM_OTHER, NULL, NULL );
}

static void Lib_PushTemplate( Lib_Ctx_t * g, const char * pArgs )
{
    char * pName = Lib_FirstWord( pArgs );
    Phys_Template_t * pTemp = Phys_TemplateAlloc( pName );
    ABC_FREE( pName );
    Lib_PushFrame( g, LIB_FRM_TEMPLATE, pTemp, NULL );
}

static void Lib_PushTable( Lib_Ctx_t * g, int Kind, const char * pArgs )
{
    Lib_Frame_t * pParent = Lib_FrameTop( g );
    if ( pParent != NULL && pParent->Type == LIB_FRM_TIMING )
    {
        Lib_Frame_t * f;
        Lib_PushFrame( g, LIB_FRM_TABLE, Phys_SurfaceAlloc(), pParent->pObj );
        f = Lib_FrameTop( g );
        f->TableKind = Kind;
        f->pTemplate = Lib_FirstWord( pArgs );
    }
    else
        Lib_PushFrame( g, LIB_FRM_OTHER, NULL, NULL );
}

static void Lib_PushSequential( Lib_Ctx_t * g )
{
    Phys_LibCell_t * pCell = Lib_CurrentCell( g );
    if ( pCell != NULL )
        pCell->fSequential = 1;
    Lib_PushFrame( g, LIB_FRM_SEQ, NULL, NULL );
}

static void Lib_PushIgnore( Lib_Ctx_t * g )
{
    Lib_PushFrame( g, LIB_FRM_OTHER, NULL, NULL );
}

static void Lib_FinalizeSurface( Lib_Ctx_t * g, Lib_Frame_t * f )
{
    Phys_Surface_t * pSurf = (Phys_Surface_t *)f->pObj;
    Phys_Template_t * pTemp = NULL;
    Phys_TimingArc_t * pArc = (Phys_TimingArc_t *)f->pParent;
    const char * pVar1 = NULL;
    const char * pVar2 = NULL;
    int fTime1, fTime2, fCap1, fCap2;
    float Scale1, Scale2;

    if ( pSurf == NULL )
        return;
    if ( f->pTemplate && f->pTemplate[0] )
    {
        pTemp = Lib_FindTemplate( g->pLib, f->pTemplate );
        if ( pTemp == NULL )
            pTemp = Lib_FindTemplateByBase( g->pLib, f->pTemplate );
    }
    if ( pTemp != NULL )
    {
        if ( Vec_FltSize(pSurf->vIndex0) == 0 )
            Lib_AppendFloats( pSurf->vIndex0, pTemp->vIndex1 );
        if ( Vec_FltSize(pSurf->vIndex1) == 0 )
            Lib_AppendFloats( pSurf->vIndex1, pTemp->vIndex2 );
        pVar1 = pTemp->pVar1;
        pVar2 = pTemp->pVar2;
    }
    fTime1 = pTemp ? Lib_VarIsTime( pVar1 ) : 1;
    fTime2 = pTemp ? Lib_VarIsTime( pVar2 ) : 0;
    fCap1  = pTemp ? Lib_VarIsCap( pVar1 )  : 0;
    fCap2  = pTemp ? Lib_VarIsCap( pVar2 )  : 1;
    Scale1 = fTime1 ? g->pLib->Units.TimeScale : (fCap1 ? g->pLib->Units.CapScale : 1.0f);
    Scale2 = fTime2 ? g->pLib->Units.TimeScale : (fCap2 ? g->pLib->Units.CapScale : 1.0f);
    Lib_ScaleFloats( pSurf->vIndex0, Scale1 );
    Lib_ScaleFloats( pSurf->vIndex1, Scale2 );
    Lib_ScaleFloats( pSurf->vData, g->pLib->Units.TimeScale );

    pSurf->n0 = Vec_FltSize( pSurf->vIndex0 );
    pSurf->n1 = Vec_FltSize( pSurf->vIndex1 );
    if ( pSurf->n1 == 0 && pSurf->n0 > 0 )
        pSurf->n1 = 1;
    if ( pSurf->n0 == 0 && pSurf->n1 > 0 )
        pSurf->n0 = 1;
    if ( pSurf->n0 == 0 && Vec_FltSize(pSurf->vData) > 0 )
        pSurf->n0 = 1;

    if ( pArc != NULL )
    {
        Phys_Surface_t ** ppSlot = NULL;
        switch ( f->TableKind )
        {
        case 0: ppSlot = &pArc->pRise;      break;
        case 1: ppSlot = &pArc->pFall;      break;
        case 2: ppSlot = &pArc->pRiseTrans; break;
        case 3: ppSlot = &pArc->pFallTrans; break;
        default: break;
        }
        if ( ppSlot != NULL )
        {
            Phys_SurfaceFree( *ppSlot );
            *ppSlot = pSurf;
            return;
        }
    }
    Phys_SurfaceFree( pSurf );
}

static void Lib_ParsePop( Lib_Ctx_t * g )
{
    Lib_Frame_t * f;
    if ( Vec_PtrSize(g->vStack) == 0 )
        return;
    f = (Lib_Frame_t *)Vec_PtrEntry( g->vStack, Vec_PtrSize(g->vStack) - 1 );
    Vec_PtrPop( g->vStack );
    switch ( f->Type )
    {
    case LIB_FRM_CELL:
    {
        Phys_LibCell_t * pCell = (Phys_LibCell_t *)f->pObj;
        Phys_LibPin_t * pPin;
        int i;
        pCell->nInputs = pCell->nOutputs = 0;
        Vec_PtrForEachEntry( Phys_LibPin_t *, pCell->vPins, pPin, i )
        {
            Phys_TimingArc_t * pArc;
            int k;
            if ( pPin->Dir == 0 )
                pCell->nInputs++;
            else if ( pPin->Dir == 1 )
                pCell->nOutputs++;
            Vec_PtrForEachEntry( Phys_TimingArc_t *, pPin->vArcs, pArc, k )
                Vec_PtrPush( pCell->vArcs, pArc );
        }
        Vec_PtrPush( g->pLib->vCells, pCell );
        break;
    }
    case LIB_FRM_PIN:
    {
        Phys_LibPin_t * pPin = (Phys_LibPin_t *)f->pObj;
        Phys_LibCell_t * pCell = (Phys_LibCell_t *)f->pParent;
        if ( pPin->Dir < 0 )
            pPin->Dir = 0;
        if ( pCell )
            Vec_PtrPush( pCell->vPins, pPin );
        else
            Phys_LibPinFree( pPin );
        break;
    }
    case LIB_FRM_TIMING:
    {
        Phys_TimingArc_t * pArc = (Phys_TimingArc_t *)f->pObj;
        Phys_LibPin_t * pPin = (Phys_LibPin_t *)f->pParent;
        if ( pArc->pRelatedPin != NULL && pPin != NULL )
            Vec_PtrPush( pPin->vArcs, pArc );
        else
            Phys_ArcFree( pArc );
        break;
    }
    case LIB_FRM_TEMPLATE:
        Vec_PtrPush( g->pLib->vTemplates, (Phys_Template_t *)f->pObj );
        break;
    case LIB_FRM_TABLE:
        Lib_FinalizeSurface( g, f );
        break;
    default:
        break;
    }
    Lib_FrameFree( f );
}

static void Lib_ParseAttr( Lib_Ctx_t * g, const char * pKey, const char * pVal )
{
    Lib_Frame_t * f = Lib_FrameTop( g );
    char * pWord;

    /* library-wide units */
    if ( !strcmp( pKey, "time_unit" ) )
    {
        g->pLib->Units.TimeScale = Lib_ParseUnitScale( pVal, "time" );
        return;
    }
    if ( !strcmp( pKey, "capacitive_load_unit" ) )
    {
        g->pLib->Units.CapScale = Lib_ParseUnitScale( pVal, "cap" );
        return;
    }
    if ( !strcmp( pKey, "pulling_resistance_unit" ) )
    {
        g->pLib->Units.ResScale = Lib_ParseUnitScale( pVal, "res" );
        return;
    }
    if ( f == NULL )
        return;

    switch ( f->Type )
    {
    case LIB_FRM_CELL:
    {
        Phys_LibCell_t * pCell = (Phys_LibCell_t *)f->pObj;
        if ( !strcmp( pKey, "area" ) )
            pCell->Area = (float)atof( pVal );
        else if ( !strcmp( pKey, "dont_use" ) || !strcmp( pKey, "dont_touch" ) )
        {
            pWord = Lib_FirstWord( pVal );
            pCell->fDontUse = !strcmp( pWord, "true" ) || !strcmp( pWord, "1" );
            ABC_FREE( pWord );
        }
        break;
    }
    case LIB_FRM_PIN:
    {
        Phys_LibPin_t * pPin = (Phys_LibPin_t *)f->pObj;
        if ( !strcmp( pKey, "direction" ) )
        {
            pWord = Lib_FirstWord( pVal );
            pPin->Dir = Lib_PinDirFromString( pWord );
            ABC_FREE( pWord );
        }
        else if ( !strcmp( pKey, "capacitance" ) )
            pPin->Cap = (float)atof( pVal ) * g->pLib->Units.CapScale;
        else if ( !strcmp( pKey, "rise_capacitance" ) )
            pPin->RiseCap = (float)atof( pVal ) * g->pLib->Units.CapScale;
        else if ( !strcmp( pKey, "fall_capacitance" ) )
            pPin->FallCap = (float)atof( pVal ) * g->pLib->Units.CapScale;
        else if ( !strcmp( pKey, "max_capacitance" ) )
            pPin->MaxCap = (float)atof( pVal ) * g->pLib->Units.CapScale;
        else if ( !strcmp( pKey, "max_transition" ) )
            pPin->MaxTrans = (float)atof( pVal ) * g->pLib->Units.TimeScale;
        else if ( !strcmp( pKey, "clock" ) )
        {
            pWord = Lib_FirstWord( pVal );
            pPin->fClock = !strcmp( pWord, "true" ) || !strcmp( pWord, "1" );
            ABC_FREE( pWord );
        }
        else if ( !strcmp( pKey, "function" ) )
        {
            ABC_FREE( pPin->pFunc );
            pPin->pFunc = Lib_StrDup( pVal );
        }
        break;
    }
    case LIB_FRM_TIMING:
    {
        Phys_TimingArc_t * pArc = (Phys_TimingArc_t *)f->pObj;
        if ( !strcmp( pKey, "related_pin" ) )
        {
            pWord = Lib_FirstWord( pVal );
            ABC_FREE( pArc->pRelatedPin );
            pArc->pRelatedPin = pWord;
        }
        else if ( !strcmp( pKey, "timing_sense" ) )
        {
            pWord = Lib_FirstWord( pVal );
            pArc->Sense = Lib_ParseTimingSense( pWord );
            ABC_FREE( pWord );
        }
        else if ( !strcmp( pKey, "timing_type" ) )
        {
            pWord = Lib_FirstWord( pVal );
            pArc->Type = Lib_ParseTimingType( pWord );
            ABC_FREE( pWord );
        }
        break;
    }
    case LIB_FRM_TEMPLATE:
    {
        Phys_Template_t * pTemp = (Phys_Template_t *)f->pObj;
        if ( !strcmp( pKey, "variable_1" ) )
        {
            ABC_FREE( pTemp->pVar1 );
            pTemp->pVar1 = Lib_FirstWord( pVal );
        }
        else if ( !strcmp( pKey, "variable_2" ) )
        {
            ABC_FREE( pTemp->pVar2 );
            pTemp->pVar2 = Lib_FirstWord( pVal );
        }
        break;
    }
    default:
        break;
    }
}

static void Lib_ParseCall( Lib_Ctx_t * g, const char * pKey, const char * pArgs )
{
    if ( !strcmp( pKey, "capacitive_load_unit" ) )
        g->pLib->Units.CapScale = Lib_ParseUnitScale( pArgs, "cap" );
    else if ( !strcmp( pKey, "time_unit" ) )
        g->pLib->Units.TimeScale = Lib_ParseUnitScale( pArgs, "time" );
}

static void Lib_ParseIndex( Lib_Ctx_t * g, int iIndex, const char * pArgs )
{
    Lib_Frame_t * f = Lib_FrameTop( g );
    if ( f == NULL )
        return;
    if ( f->Type == LIB_FRM_TEMPLATE )
    {
        Phys_Template_t * pTemp = (Phys_Template_t *)f->pObj;
        Lib_ParseFloatList( pArgs, iIndex == 0 ? pTemp->vIndex1 : pTemp->vIndex2 );
    }
    else if ( f->Type == LIB_FRM_TABLE )
    {
        Phys_Surface_t * pSurf = (Phys_Surface_t *)f->pObj;
        Lib_ParseFloatList( pArgs, iIndex == 0 ? pSurf->vIndex0 : pSurf->vIndex1 );
    }
}

static void Lib_ParseValues( Lib_Ctx_t * g, const char * pArgs )
{
    Lib_Frame_t * f = Lib_FrameTop( g );
    if ( f != NULL && f->Type == LIB_FRM_TABLE )
        Lib_ParseFloatList( pArgs, ((Phys_Surface_t *)f->pObj)->vData );
}

static void Lib_ParseRecover( Lib_Ctx_t * g )
{
    /* the grammar discards one bad statement; keep going */
    (void)g;
}


/*------------------------------------------------------------------------*/
/* generated parser / scanner entry points                                */
/*------------------------------------------------------------------------*/

int Lib_parse( void );
int Lib_lex( void );
int Lib_error( const char * pMsg );
void Lib_SetInput( const char * pData, int nSize );
void Lib_Cleanup( void );


/*------------------------------------------------------------------------*/
/* public API                                                             */
/*------------------------------------------------------------------------*/

Phys_Lib_t * Phys_LibertyRead( const char * pFileName, int fVerbose )
{
    Lib_Ctx_t Ctx, * g = &Ctx;
    Phys_Lib_t * pLib;
    char * pData;
    int nSize = 0, Ret;

    pData = Phys_ReadFileContents( pFileName, &nSize );
    if ( pData == NULL )
        return NULL;

    pLib = ABC_CALLOC( Phys_Lib_t, 1 );
    Phys_UnitsInit( &pLib->Units );
    pLib->vCells     = Vec_PtrAlloc( 1024 );
    pLib->vTemplates = Vec_PtrAlloc( 64 );

    memset( g, 0, sizeof(*g) );
    g->pLib   = pLib;
    g->vStack = Vec_PtrAlloc( 32 );
    s_pLibCtx = g;

    Lib_SetInput( pData, nSize );
    Ret = Lib_parse();
    Lib_Cleanup();
    s_pLibCtx = NULL;

    while ( Vec_PtrSize(g->vStack) > 0 )
    {
        Lib_Frame_t * f = (Lib_Frame_t *)Vec_PtrPop(g->vStack);
        Lib_FrameFree( f );
    }
    Vec_PtrFree( g->vStack );
    ABC_FREE( pData );

    if ( Ret != 0 || g->nErrors != 0 )
    {
        fprintf( stderr, "Liberty: failed to parse \"%s\".\n", pFileName );
        Phys_LibFree( &pLib );
        return NULL;
    }

    Phys_LibFinish( pLib );
    if ( fVerbose )
        fprintf( stdout, "Liberty: parsed library \"%s\": %d cells, %d templates "
                 "(time=%.1f ps/unit, cap=%.4f pF/unit, res=%.4f kOhm/unit).\n",
                 pLib->pName ? pLib->pName : "", Vec_PtrSize(pLib->vCells),
                 Vec_PtrSize(pLib->vTemplates), pLib->Units.TimeScale,
                 pLib->Units.CapScale, pLib->Units.ResScale );
    return pLib;
}

void Phys_LibFinish( Phys_Lib_t * p )
{
    int i;
    if ( p == NULL ) return;
    for ( i = 0; i < Vec_PtrSize(p->vCells); i++ )
        ((Phys_LibCell_t *)Vec_PtrEntry(p->vCells, i))->CellId = i;
    Phys_HashBuild( &p->vCellHash, p->vCells );
}

void Phys_LibFree( Phys_Lib_t ** pp )
{
    Phys_Lib_t * p;
    Phys_LibCell_t * pCell;
    Phys_Template_t * pTemp;
    int i;
    if ( pp == NULL || *pp == NULL )
        return;
    p = *pp;
    *pp = NULL;
    Vec_PtrForEachEntry( Phys_LibCell_t *, p->vCells, pCell, i )
        Phys_LibCellFree( pCell );
    Vec_PtrForEachEntry( Phys_Template_t *, p->vTemplates, pTemp, i )
        Phys_TemplateFree( pTemp );
    Vec_PtrFree( p->vCells );
    Vec_PtrFree( p->vTemplates );
    Vec_IntFreeP( &p->vCellHash );
    ABC_FREE( p->pName );
    Phys_UnitsClear( &p->Units );
    ABC_FREE( p );
}

Phys_LibCell_t * Phys_LibFindCell( Phys_Lib_t * p, const char * pName )
{
    int Id;
    if ( p == NULL || pName == NULL )
        return NULL;
    Id = Phys_HashFind( p->vCellHash, p->vCells, pName );
    return Id >= 0 ? (Phys_LibCell_t *)Vec_PtrEntry(p->vCells, Id) : NULL;
}

Phys_LibPin_t * Phys_LibCellFindPin( Phys_LibCell_t * pCell, const char * pName )
{
    Phys_LibPin_t * pPin;
    int i;
    if ( pCell == NULL || pName == NULL )
        return NULL;
    Vec_PtrForEachEntry( Phys_LibPin_t *, pCell->vPins, pPin, i )
        if ( !strcmp( pPin->pName, pName ) )
            return pPin;
    return NULL;
}

Phys_LibPin_t * Phys_LibCellFindOutputPin( Phys_LibCell_t * pCell, const char * pName )
{
    Phys_LibPin_t * pPin;
    int i;
    if ( pCell == NULL )
        return NULL;
    Vec_PtrForEachEntry( Phys_LibPin_t *, pCell->vPins, pPin, i )
        if ( (pName == NULL || !strcmp( pPin->pName, pName )) && pPin->Dir == 1 )
            return pPin;
    return NULL;
}

Phys_TimingArc_t * Phys_LibPinFindArc( Phys_LibPin_t * pPin, const char * pRelated )
{
    Phys_TimingArc_t * pArc;
    int i;
    if ( pPin == NULL || pRelated == NULL )
        return NULL;
    Vec_PtrForEachEntry( Phys_TimingArc_t *, pPin->vArcs, pArc, i )
        if ( pArc->pRelatedPin && !strcmp( pArc->pRelatedPin, pRelated ) )
            return pArc;
    return NULL;
}

Phys_TimingArc_t * Phys_LibCellFindArc( Phys_LibCell_t * pCell, const char * pIn, const char * pOut )
{
    Phys_LibPin_t * pPin = Phys_LibCellFindOutputPin( pCell, pOut );
    return Phys_LibPinFindArc( pPin, pIn );
}


/*------------------------------------------------------------------------*/
/* table lookup                                                           */
/*------------------------------------------------------------------------*/

static void Phys_FindInterval( Vec_Flt_t * v, float x, int * pi0, int * pi1, float * pt )
{
    int n = Vec_FltSize( v );
    int lo = 0, hi = n - 1, mid;
    if ( n <= 1 )
    {
        *pi0 = *pi1 = 0;
        *pt = 0.0f;
        return;
    }
    if ( x <= Vec_FltEntry( v, 0 ) )
    {
        *pi0 = *pi1 = 0;
        *pt = 0.0f;
        return;
    }
    if ( x >= Vec_FltEntry( v, n - 1 ) )
    {
        *pi0 = *pi1 = n - 1;
        *pt = 0.0f;
        return;
    }
    while ( lo + 1 < hi )
    {
        mid = (lo + hi) >> 1;
        if ( Vec_FltEntry( v, mid ) <= x )
            lo = mid;
        else
            hi = mid;
    }
    *pi0 = lo;
    *pi1 = lo + 1;
    {
        float t0 = Vec_FltEntry( v, lo );
        float t1 = Vec_FltEntry( v, lo + 1 );
        *pt = ( t1 > t0 ) ? (x - t0) / (t1 - t0) : 0.0f;
    }
}

float Phys_SurfaceLookupConst( Phys_Surface_t * p, int i0, int i1 )
{
    int n1;
    if ( p == NULL || Vec_FltSize(p->vData) == 0 )
        return 0.0f;
    n1 = Vec_FltSize( p->vIndex1 );
    if ( n1 < 1 ) n1 = 1;
    if ( i0 < 0 ) i0 = 0;
    if ( i1 < 0 ) i1 = 0;
    return Vec_FltEntry( p->vData, i0 * n1 + i1 );
}

float Phys_SurfaceLookup( Phys_Surface_t * p, float v0, float v1 )
{
    int i0, i1, j0, j1;
    float t0, t1, d00, d01, d10, d11;
    if ( p == NULL )
        return 0.0f;
    if ( Vec_FltSize(p->vIndex0) == 0 || Vec_FltSize(p->vData) == 0 )
        return Phys_SurfaceLookupConst( p, 0, 0 );
    Phys_FindInterval( p->vIndex0, v0, &i0, &i1, &t0 );
    if ( Vec_FltSize(p->vIndex1) == 0 )
    {
        int n1 = 1;
        if ( i1 * n1 >= Vec_FltSize(p->vData) )
            i1 = i0;
        return (1.0f - t0) * Vec_FltEntry(p->vData, i0*n1) +
               t0 * Vec_FltEntry(p->vData, i1*n1);
    }
    Phys_FindInterval( p->vIndex1, v1, &j0, &j1, &t1 );
    {
        int n1 = Vec_FltSize(p->vIndex1);
        int nData = Vec_FltSize(p->vData);
        if ( (i0 * n1 + j0) >= nData )
            return Vec_FltEntry( p->vData, nData - 1 );
        if ( (i1 * n1 + j1) >= nData )
            i1 = i0, j1 = j0;
        d00 = Vec_FltEntry( p->vData, i0 * n1 + j0 );
        d01 = Vec_FltEntry( p->vData, i0 * n1 + j1 );
        d10 = Vec_FltEntry( p->vData, i1 * n1 + j0 );
        d11 = Vec_FltEntry( p->vData, i1 * n1 + j1 );
    }
    return (1.0f - t0) * (1.0f - t1) * d00 +
           (1.0f - t0) * t1 * d01 +
           t0 * (1.0f - t1) * d10 +
           t0 * t1 * d11;
}


/*------------------------------------------------------------------------*/
/* reporting                                                              */
/*------------------------------------------------------------------------*/

void Phys_LibPrint( Phys_Lib_t * p )
{
    Phys_LibCell_t * pCell;
    int i;
    if ( p == NULL ) return;
    fprintf( stdout, "Liberty library \"%s\": %d cells, units time=%.2f ps, cap=%.4f pF, res=%.4f kOhm.\n",
             p->pName ? p->pName : "", Vec_PtrSize(p->vCells), p->Units.TimeScale,
             p->Units.CapScale, p->Units.ResScale );
    Vec_PtrForEachEntry( Phys_LibCell_t *, p->vCells, pCell, i )
        fprintf( stdout, "  cell %-24s area=%g in=%d out=%d seq=%d arcs=%d\n",
                 pCell->pName, pCell->Area, pCell->nInputs, pCell->nOutputs,
                 pCell->fSequential, Vec_PtrSize(pCell->vArcs) );
}

void Phys_LibCellPrint( Phys_Lib_t * p, Phys_LibCell_t * pCell )
{
    Phys_LibPin_t * pPin;
    Phys_TimingArc_t * pArc;
    int i, k;
    if ( pCell == NULL ) return;
    fprintf( stdout, "cell %s: area=%g inputs=%d outputs=%d sequential=%d\n",
             pCell->pName, pCell->Area, pCell->nInputs, pCell->nOutputs, pCell->fSequential );
    Vec_PtrForEachEntry( Phys_LibPin_t *, pCell->vPins, pPin, i )
    {
        fprintf( stdout, "  pin %-10s dir=%d cap=%.5f maxcap=%.5f arcs=%d%s%s\n",
                 pPin->pName, pPin->Dir, pPin->Cap, pPin->MaxCap, Vec_PtrSize(pPin->vArcs),
                 pPin->pFunc ? " func=" : "", pPin->pFunc ? pPin->pFunc : "" );
        Vec_PtrForEachEntry( Phys_TimingArc_t *, pPin->vArcs, pArc, k )
            fprintf( stdout, "    arc <- %-8s sense=%d type=%d rise=%dx%d fall=%dx%d "
                             "rise00=%.4f fall00=%.4f\n",
                     pArc->pRelatedPin ? pArc->pRelatedPin : "?", pArc->Sense, pArc->Type,
                     pArc->pRise ? pArc->pRise->n0 : 0, pArc->pRise ? pArc->pRise->n1 : 0,
                     pArc->pFall ? pArc->pFall->n0 : 0, pArc->pFall ? pArc->pFall->n1 : 0,
                     Phys_SurfaceLookupConst( pArc->pRise, 0, 0 ),
                     Phys_SurfaceLookupConst( pArc->pFall, 0, 0 ) );
    }
    (void)p;
}

ABC_NAMESPACE_IMPL_END

/* The generated scanner/parser are compiled as part of this translation
   unit so that they can use the static helpers above. */
#include "physLibertyYacc.c"
#include "physLibertyLex.c"
