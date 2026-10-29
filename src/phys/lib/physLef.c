/**Function*************************************************************

  Synopsis    [LEF reader]

  Description [LEF parser built on flex/bison.  The grammar lives in
               physLefLex.l and physLefYacc.y; the generated files are
               included at the end of this file.  The reader keeps the
               technology data (units, sites, layers with RC data, vias)
               and the macro data (size, class, site, pin geometry and
               obstructions) needed by the physical-synthesis flow.
               Coordinates are converted to LEF database units (DBU).
               Unknown statements are skipped by the grammar's error
               recovery, so the reader tolerates LEF constructs outside
               the supported subset.]

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

static char * Lef_StrDup( const char * pStr )
{
    char * pNew;
    if ( pStr == NULL )
        pStr = "";
    pNew = ABC_ALLOC( char, strlen(pStr) + 1 );
    strcpy( pNew, pStr );
    return pNew;
}

static char * Lef_StrCat( const char * pFirst, const char * pSecond )
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

static char * Lef_StrWrap( const char * pPre, const char * pMid, const char * pPost )
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

static char * Lef_StrNum( double Value )
{
    char Buffer[64];
    snprintf( Buffer, sizeof(Buffer), "%.15g", Value );
    return Lef_StrDup( Buffer );
}

static char * Lef_FirstWord( const char * pStr )
{
    const char * pBeg, * pEnd;
    char * pNew;
    int nLen;
    if ( pStr == NULL )
        return Lef_StrDup( "" );
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

static char * Lef_Unquote( const char * pQuoted )
{
    char * pOut;
    int i, k = 0, n = (int)strlen(pQuoted);
    if ( n < 2 )
        return Lef_StrDup( "" );
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
/* data structures                                                        */
/*------------------------------------------------------------------------*/

static int Lef_ToDbu( Phys_LefTech_t * p, double Value )
{
    int Dbu = p ? p->Units.Dbu : 1000;
    if ( Dbu <= 0 )
        Dbu = 1000;
    return (int)( Value * Dbu + (Value >= 0.0 ? 0.5 : -0.5) );
}

static Phys_LefPin_t * Phys_LefPinAlloc( const char * pName )
{
    Phys_LefPin_t * p = ABC_CALLOC( Phys_LefPin_t, 1 );
    p->pName  = Lef_StrDup( pName );
    p->vRects = Vec_IntAlloc( 4 );
    return p;
}

static void Phys_LefPinFree( Phys_LefPin_t * p )
{
    if ( p == NULL ) return;
    ABC_FREE( p->pName );
    Vec_IntFree( p->vRects );
    ABC_FREE( p );
}

static Phys_LefMacro_t * Phys_LefMacroAlloc( const char * pName )
{
    Phys_LefMacro_t * p = ABC_CALLOC( Phys_LefMacro_t, 1 );
    p->pName  = Lef_StrDup( pName );
    p->vPins  = Vec_PtrAlloc( 16 );
    p->vObs   = Vec_IntAlloc( 16 );
    p->Id     = -1;
    return p;
}

static void Phys_LefMacroFree( Phys_LefMacro_t * p )
{
    Phys_LefPin_t * pPin;
    int i;
    if ( p == NULL ) return;
    ABC_FREE( p->pName );
    ABC_FREE( p->pClass );
    ABC_FREE( p->pSite );
    Vec_PtrForEachEntry( Phys_LefPin_t *, p->vPins, pPin, i )
        Phys_LefPinFree( pPin );
    Vec_PtrFree( p->vPins );
    Vec_IntFree( p->vObs );
    ABC_FREE( p );
}

static Phys_LefLayer_t * Phys_LefLayerAlloc( const char * pName )
{
    Phys_LefLayer_t * p = ABC_CALLOC( Phys_LefLayer_t, 1 );
    p->pName = Lef_StrDup( pName );
    return p;
}

static void Phys_LefLayerFree( Phys_LefLayer_t * p )
{
    if ( p == NULL ) return;
    ABC_FREE( p->pName );
    ABC_FREE( p->pType );
    ABC_FREE( p->pDir );
    ABC_FREE( p );
}

static Phys_LefSite_t * Phys_LefSiteAlloc( const char * pName )
{
    Phys_LefSite_t * p = ABC_CALLOC( Phys_LefSite_t, 1 );
    p->pName = Lef_StrDup( pName );
    return p;
}

static void Phys_LefSiteFree( Phys_LefSite_t * p )
{
    if ( p == NULL ) return;
    ABC_FREE( p->pName );
    ABC_FREE( p->pClass );
    ABC_FREE( p );
}

static void Lef_PinComputeCenter( Phys_LefPin_t * pPin, Phys_LefMacro_t * pMacro )
{
    if ( Vec_IntSize(pPin->vRects) > 0 )
    {
        /* The anchor of a pin is the middle of its largest metal
           rectangle.  A pin often consists of several staircase or
           L-shaped pieces, and the middle of their bounding box can
           fall outside the metal, so the largest piece is used. */
        int k, x1 = 0, y1 = 0, x2 = 0, y2 = 0, Best = -1;
        for ( k = 0; k < Vec_IntSize(pPin->vRects); k += 4 )
        {
            int rx1 = Vec_IntEntry(pPin->vRects, k+0);
            int ry1 = Vec_IntEntry(pPin->vRects, k+1);
            int rx2 = Vec_IntEntry(pPin->vRects, k+2);
            int ry2 = Vec_IntEntry(pPin->vRects, k+3);
            int Area = (rx2 - rx1) * (ry2 - ry1);
            if ( Best < 0 || Area > Best )
            {
                Best = Area;
                x1 = rx1; y1 = ry1; x2 = rx2; y2 = ry2;
            }
        }
        pPin->x = (x1 + x2) / 2;
        pPin->y = (y1 + y2) / 2;
    }
    else
    {
        pPin->x = pMacro ? pMacro->Width / 2 : 0;
        pPin->y = pMacro ? pMacro->Height / 2 : 0;
    }
    pPin->nRects = Vec_IntSize(pPin->vRects) / 4;
}


/*------------------------------------------------------------------------*/
/* parse context                                                          */
/*------------------------------------------------------------------------*/

enum {
    LEF_FRM_MACRO = 0,
    LEF_FRM_PIN,
    LEF_FRM_PORT,
    LEF_FRM_OBS,
    LEF_FRM_LAYER,
    LEF_FRM_SITE,
    LEF_FRM_VIA,
    LEF_FRM_UNITS,
    LEF_FRM_IGNORE
};

typedef struct Lef_Frame_t_
{
    int      Type;
    void *   pObj;
    void *   pParent;
    char *   pName;
} Lef_Frame_t;

typedef struct Lef_Ctx_t_
{
    Phys_LefTech_t * pTech;
    Vec_Ptr_t *      vStack;
    int              fTech;
    int              nErrors;
} Lef_Ctx_t;

static Lef_Ctx_t * s_pLefCtx = NULL;

static Lef_Ctx_t * Lef_ParseCtx( void ) { return s_pLefCtx; }

static void Lef_ParseError( const char * pMsg )
{
    extern int Lef_lineno;
    fprintf( stderr, "LEF parse error near line %d: %s\n", Lef_lineno, pMsg );
    if ( s_pLefCtx )
        s_pLefCtx->nErrors++;
}

static void Lef_ParseRecover( Lef_Ctx_t * g )
{
    /* the grammar discards one bad statement; keep going */
    (void)g;
}

static Lef_Frame_t * Lef_FrameTop( Lef_Ctx_t * g )
{
    if ( g == NULL || Vec_PtrSize(g->vStack) == 0 )
        return NULL;
    return (Lef_Frame_t *)Vec_PtrEntry( g->vStack, Vec_PtrSize(g->vStack) - 1 );
}

static void Lef_PushFrame( Lef_Ctx_t * g, int Type, void * pObj, void * pParent, const char * pName )
{
    Lef_Frame_t * f = ABC_CALLOC( Lef_Frame_t, 1 );
    f->Type    = Type;
    f->pObj    = pObj;
    f->pParent = pParent;
    f->pName   = pName ? Lef_StrDup( pName ) : NULL;
    Vec_PtrPush( g->vStack, f );
}

static Lef_Frame_t * Lef_PopFrame( Lef_Ctx_t * g )
{
    if ( g == NULL || Vec_PtrSize(g->vStack) == 0 )
        return NULL;
    return (Lef_Frame_t *)Vec_PtrPop( g->vStack );
}

static void Lef_FrameFree( Lef_Frame_t * f )
{
    if ( f == NULL ) return;
    ABC_FREE( f->pName );
    ABC_FREE( f );
}

static void Lef_CheckEndName( Lef_Frame_t * f, const char * pEndName )
{
    if ( f == NULL || f->pName == NULL || pEndName == NULL )
        return;
    if ( pEndName[0] && strcmp( f->pName, pEndName ) )
        fprintf( stderr, "LEF: warning: group \"%s\" ended with \"%s\".\n", f->pName, pEndName );
}

static Phys_LefPin_t * Lef_CurrentPin( Lef_Ctx_t * g )
{
    int i;
    for ( i = Vec_PtrSize(g->vStack) - 1; i >= 0; i-- )
    {
        Lef_Frame_t * f = (Lef_Frame_t *)Vec_PtrEntry( g->vStack, i );
        if ( f->Type == LEF_FRM_PIN )
            return (Phys_LefPin_t *)f->pObj;
    }
    return NULL;
}


/*------------------------------------------------------------------------*/
/* semantic actions                                                       */
/*------------------------------------------------------------------------*/

static void Lef_ParsePushMacro( Lef_Ctx_t * g, const char * pName )
{
    Lef_PushFrame( g, LEF_FRM_MACRO, Phys_LefMacroAlloc( pName ), NULL, pName );
}

static void Lef_ParsePopMacro( Lef_Ctx_t * g, const char * pEndName )
{
    Lef_Frame_t * f = Lef_PopFrame( g );
    Phys_LefMacro_t * pMacro;
    if ( f == NULL || f->Type != LEF_FRM_MACRO )
    {
        Lef_FrameFree( f );
        return;
    }
    Lef_CheckEndName( f, pEndName );
    pMacro = (Phys_LefMacro_t *)f->pObj;
    pMacro->nObs  = Vec_IntSize( pMacro->vObs ) / 4;
    pMacro->nPins = Vec_PtrSize( pMacro->vPins );
    if ( g->fTech )
        Phys_LefMacroFree( pMacro );
    else
    {
        pMacro->Id = Vec_PtrSize( g->pTech->vMacros );
        Vec_PtrPush( g->pTech->vMacros, pMacro );
    }
    Lef_FrameFree( f );
}

static void Lef_ParsePushPin( Lef_Ctx_t * g, const char * pName )
{
    Lef_Frame_t * pParent = Lef_FrameTop( g );
    if ( pParent != NULL && pParent->Type == LEF_FRM_MACRO )
        Lef_PushFrame( g, LEF_FRM_PIN, Phys_LefPinAlloc( pName ), pParent->pObj, pName );
    else
        Lef_PushFrame( g, LEF_FRM_IGNORE, NULL, NULL, pName );
}

static void Lef_ParsePopPin( Lef_Ctx_t * g, const char * pEndName )
{
    Lef_Frame_t * f = Lef_PopFrame( g );
    if ( f == NULL )
        return;
    Lef_CheckEndName( f, pEndName );
    if ( f->Type == LEF_FRM_PIN )
    {
        Phys_LefPin_t * pPin = (Phys_LefPin_t *)f->pObj;
        Phys_LefMacro_t * pMacro = (Phys_LefMacro_t *)f->pParent;
        Lef_PinComputeCenter( pPin, pMacro );
        if ( pMacro != NULL )
            Vec_PtrPush( pMacro->vPins, pPin );
        else
            Phys_LefPinFree( pPin );
    }
    Lef_FrameFree( f );
}

static void Lef_ParsePushPort( Lef_Ctx_t * g )
{
    Lef_Frame_t * pParent = Lef_FrameTop( g );
    Phys_LefPin_t * pPin = (pParent && pParent->Type == LEF_FRM_PIN) ?
        (Phys_LefPin_t *)pParent->pObj : NULL;
    Lef_PushFrame( g, pPin ? LEF_FRM_PORT : LEF_FRM_IGNORE, NULL, pPin, NULL );
}

static void Lef_ParsePopPort( Lef_Ctx_t * g )
{
    Lef_FrameFree( Lef_PopFrame(g) );
}

static void Lef_ParsePushObs( Lef_Ctx_t * g )
{
    Lef_Frame_t * pParent = Lef_FrameTop( g );
    Phys_LefMacro_t * pMacro = (pParent && pParent->Type == LEF_FRM_MACRO) ?
        (Phys_LefMacro_t *)pParent->pObj : NULL;
    Lef_PushFrame( g, pMacro ? LEF_FRM_OBS : LEF_FRM_IGNORE, NULL, pMacro, NULL );
}

static void Lef_ParsePopObs( Lef_Ctx_t * g )
{
    Lef_FrameFree( Lef_PopFrame(g) );
}

static void Lef_ParsePushLayer( Lef_Ctx_t * g, const char * pName )
{
    Lef_PushFrame( g, LEF_FRM_LAYER, Phys_LefLayerAlloc( pName ), NULL, pName );
}

static void Lef_ParsePopLayer( Lef_Ctx_t * g, const char * pEndName )
{
    Lef_Frame_t * f = Lef_PopFrame( g );
    if ( f == NULL || f->Type != LEF_FRM_LAYER )
    {
        Lef_FrameFree( f );
        return;
    }
    Lef_CheckEndName( f, pEndName );
    if ( g->fTech )
        Vec_PtrPush( g->pTech->vLayers, f->pObj );
    else
        Phys_LefLayerFree( (Phys_LefLayer_t *)f->pObj );
    Lef_FrameFree( f );
}

static void Lef_ParsePushSite( Lef_Ctx_t * g, const char * pName )
{
    Lef_PushFrame( g, LEF_FRM_SITE, Phys_LefSiteAlloc( pName ), NULL, pName );
}

static void Lef_ParsePopSite( Lef_Ctx_t * g, const char * pEndName )
{
    Lef_Frame_t * f = Lef_PopFrame( g );
    if ( f == NULL || f->Type != LEF_FRM_SITE )
    {
        Lef_FrameFree( f );
        return;
    }
    Lef_CheckEndName( f, pEndName );
    if ( g->fTech )
        Vec_PtrPush( g->pTech->vSites, f->pObj );
    else
        Phys_LefSiteFree( (Phys_LefSite_t *)f->pObj );
    Lef_FrameFree( f );
}

static void Lef_ParsePushVia( Lef_Ctx_t * g, const char * pName )
{
    Lef_PushFrame( g, LEF_FRM_VIA, NULL, NULL, pName );
}

static void Lef_ParsePopVia( Lef_Ctx_t * g, const char * pEndName )
{
    Lef_Frame_t * f = Lef_PopFrame( g );
    if ( f == NULL )
        return;
    Lef_CheckEndName( f, pEndName );
    if ( f->Type == LEF_FRM_VIA && g->fTech && f->pName != NULL )
        Vec_PtrPush( g->pTech->vVias, Lef_StrDup( f->pName ) );
    Lef_FrameFree( f );
}

static void Lef_ParsePushUnits( Lef_Ctx_t * g )
{
    Lef_PushFrame( g, LEF_FRM_UNITS, NULL, NULL, NULL );
}

static void Lef_ParsePopUnits( Lef_Ctx_t * g )
{
    Lef_FrameFree( Lef_PopFrame(g) );
}

static void Lef_ParsePushIgnore( Lef_Ctx_t * g )
{
    Lef_PushFrame( g, LEF_FRM_IGNORE, NULL, NULL, NULL );
}

static void Lef_ParsePopIgnore( Lef_Ctx_t * g )
{
    Lef_FrameFree( Lef_PopFrame(g) );
}

static Vec_Int_t * Lef_RectTarget( Lef_Ctx_t * g )
{
    Lef_Frame_t * f = Lef_FrameTop( g );
    if ( f == NULL )
        return NULL;
    switch ( f->Type )
    {
    case LEF_FRM_PIN:
        return ((Phys_LefPin_t *)f->pObj)->vRects;
    case LEF_FRM_PORT:
        return f->pParent ? ((Phys_LefPin_t *)f->pParent)->vRects : NULL;
    case LEF_FRM_OBS:
        return f->pParent ? ((Phys_LefMacro_t *)f->pParent)->vObs : NULL;
    default:
        return NULL;
    }
}

static void Lef_ParseRect( Lef_Ctx_t * g, double x1, double y1, double x2, double y2 )
{
    Vec_Int_t * vRects = Lef_RectTarget( g );
    if ( vRects == NULL )
        return;
    Vec_IntPush( vRects, Lef_ToDbu( g->pTech, x1 ) );
    Vec_IntPush( vRects, Lef_ToDbu( g->pTech, y1 ) );
    Vec_IntPush( vRects, Lef_ToDbu( g->pTech, x2 ) );
    Vec_IntPush( vRects, Lef_ToDbu( g->pTech, y2 ) );
}

static void Lef_ParseRectPoint( Lef_Ctx_t * g, double x, double y )
{
    Lef_ParseRect( g, x, y, x, y );
}

static void Lef_ParseClass( Lef_Ctx_t * g, const char * pClass )
{
    Lef_Frame_t * f = Lef_FrameTop( g );
    char * pWord;
    if ( f == NULL )
        return;
    pWord = Lef_FirstWord( pClass );
    if ( f->Type == LEF_FRM_MACRO )
    {
        Phys_LefMacro_t * pMacro = (Phys_LefMacro_t *)f->pObj;
        ABC_FREE( pMacro->pClass );
        pMacro->pClass = pWord;
    }
    else if ( f->Type == LEF_FRM_SITE )
    {
        Phys_LefSite_t * pSite = (Phys_LefSite_t *)f->pObj;
        ABC_FREE( pSite->pClass );
        pSite->pClass = pWord;
    }
    else
        ABC_FREE( pWord );
}

static void Lef_ParseSiteRef( Lef_Ctx_t * g, const char * pSite )
{
    Lef_Frame_t * f = Lef_FrameTop( g );
    Phys_LefMacro_t * pMacro;
    if ( f == NULL || f->Type != LEF_FRM_MACRO )
        return;
    pMacro = (Phys_LefMacro_t *)f->pObj;
    ABC_FREE( pMacro->pSite );
    pMacro->pSite = Lef_StrDup( pSite );
}

static void Lef_ParseSize( Lef_Ctx_t * g, double Width, double Height )
{
    Lef_Frame_t * f = Lef_FrameTop( g );
    if ( f == NULL )
        return;
    if ( f->Type == LEF_FRM_MACRO )
    {
        Phys_LefMacro_t * pMacro = (Phys_LefMacro_t *)f->pObj;
        pMacro->Width  = Lef_ToDbu( g->pTech, Width );
        pMacro->Height = Lef_ToDbu( g->pTech, Height );
    }
    else if ( f->Type == LEF_FRM_SITE )
    {
        Phys_LefSite_t * pSite = (Phys_LefSite_t *)f->pObj;
        pSite->Width  = Lef_ToDbu( g->pTech, Width );
        pSite->Height = Lef_ToDbu( g->pTech, Height );
    }
}

static void Lef_ParseOrigin( Lef_Ctx_t * g, double X, double Y )
{
    Lef_Frame_t * f = Lef_FrameTop( g );
    Phys_LefMacro_t * pMacro;
    if ( f == NULL || f->Type != LEF_FRM_MACRO )
        return;
    pMacro = (Phys_LefMacro_t *)f->pObj;
    pMacro->OriginX = Lef_ToDbu( g->pTech, X );
    pMacro->OriginY = Lef_ToDbu( g->pTech, Y );
}

static void Lef_ParseDirection( Lef_Ctx_t * g, const char * pDir )
{
    Lef_Frame_t * f = Lef_FrameTop( g );
    if ( f == NULL )
        return;
    if ( f->Type == LEF_FRM_PIN )
    {
        Phys_LefPin_t * pPin = (Phys_LefPin_t *)f->pObj;
        if ( !strcmp( pDir, "INPUT" ) || !strcmp( pDir, "FEEDTHRU" ) )
            pPin->Dir = 0;
        else if ( !strcmp( pDir, "OUTPUT" ) )
            pPin->Dir = 1;
        else
            pPin->Dir = 2;
    }
    else if ( f->Type == LEF_FRM_LAYER )
    {
        Phys_LefLayer_t * pLayer = (Phys_LefLayer_t *)f->pObj;
        ABC_FREE( pLayer->pDir );
        pLayer->pDir = Lef_StrDup( pDir );
    }
}

static void Lef_ParseUse( Lef_Ctx_t * g, const char * pUse )
{
    Lef_Frame_t * f = Lef_FrameTop( g );
    Phys_LefPin_t * pPin;
    if ( f == NULL || f->Type != LEF_FRM_PIN )
        return;
    pPin = (Phys_LefPin_t *)f->pObj;
    if ( !strcmp( pUse, "POWER" ) )
        pPin->Dir = 3;
    else if ( !strcmp( pUse, "GROUND" ) )
        pPin->Dir = 4;
    else if ( !strcmp( pUse, "CLOCK" ) )
        pPin->Dir = 0;
    /* USE SIGNAL does not override DIRECTION */
}

static void Lef_ParseLayerType( Lef_Ctx_t * g, const char * pType )
{
    Lef_Frame_t * f = Lef_FrameTop( g );
    Phys_LefLayer_t * pLayer;
    if ( f == NULL || f->Type != LEF_FRM_LAYER )
        return;
    pLayer = (Phys_LefLayer_t *)f->pObj;
    ABC_FREE( pLayer->pType );
    pLayer->pType = Lef_StrDup( pType );
    pLayer->fRouting = !strcmp( pType, "ROUTING" );
}

static void Lef_ParseWidth( Lef_Ctx_t * g, double Width )
{
    Lef_Frame_t * f = Lef_FrameTop( g );
    if ( f == NULL || f->Type != LEF_FRM_LAYER )
        return;
    ((Phys_LefLayer_t *)f->pObj)->Width = Lef_ToDbu( g->pTech, Width );
}

static void Lef_ParsePitch( Lef_Ctx_t * g, double X, double Y, int fTwo )
{
    Lef_Frame_t * f = Lef_FrameTop( g );
    Phys_LefLayer_t * pLayer;
    if ( f == NULL || f->Type != LEF_FRM_LAYER )
        return;
    pLayer = (Phys_LefLayer_t *)f->pObj;
    pLayer->PitchX = Lef_ToDbu( g->pTech, X );
    pLayer->PitchY = fTwo ? Lef_ToDbu( g->pTech, Y ) : pLayer->PitchX;
}

static void Lef_ParseResistance( Lef_Ctx_t * g, const char * pQual, double Value )
{
    Lef_Frame_t * f = Lef_FrameTop( g );
    if ( f == NULL || f->Type != LEF_FRM_LAYER )
        return;
    if ( pQual != NULL && strcmp( pQual, "RPERSQ" ) )
        return;
    ((Phys_LefLayer_t *)f->pObj)->ResPerSq = (float)Value;
}

static void Lef_ParseCapacitance( Lef_Ctx_t * g, const char * pQual, double Value )
{
    Lef_Frame_t * f = Lef_FrameTop( g );
    if ( f == NULL || f->Type != LEF_FRM_LAYER )
        return;
    if ( pQual != NULL && strcmp( pQual, "CPERSQ" ) && strcmp( pQual, "CPERSQDIST" ) )
        return;
    ((Phys_LefLayer_t *)f->pObj)->CapPerSq = (float)Value;
}

static void Lef_ParseEdgeCapacitance( Lef_Ctx_t * g, double Value )
{
    Lef_Frame_t * f = Lef_FrameTop( g );
    if ( f == NULL || f->Type != LEF_FRM_LAYER )
        return;
    ((Phys_LefLayer_t *)f->pObj)->EdgeCap = (float)Value;
}

static void Lef_ParseDatabase( Lef_Ctx_t * g, const char * pUnit, double Value )
{
    Lef_Frame_t * f = Lef_FrameTop( g );
    if ( f == NULL || f->Type != LEF_FRM_UNITS || !g->fTech )
        return;
    if ( strcmp( pUnit, "MICRONS" ) || Value <= 0.0 )
        return;
    g->pTech->Units.Dbu = (int)( Value + 0.5 );
}

static void Lef_ParseGrid( Lef_Ctx_t * g, double Value )
{
    if ( Value > 0.0 )
        g->pTech->ManufacturingGrid = (float)Value;
}


/*------------------------------------------------------------------------*/
/* generated parser / scanner entry points                                */
/*------------------------------------------------------------------------*/

int Lef_parse( void );
int Lef_lex( void );
int Lef_error( const char * pMsg );
void Lef_SetInput( const char * pData, int nSize );
void Lef_Cleanup( void );


/*------------------------------------------------------------------------*/
/* file reader                                                            */
/*------------------------------------------------------------------------*/

static int Lef_ParseFile( Phys_LefTech_t * pTech, const char * pFileName, int fTech, int fVerbose )
{
    Lef_Ctx_t Ctx, * g = &Ctx;
    char * pData;
    int nSize = 0, Ret;

    pData = Phys_ReadFileContents( pFileName, &nSize );
    if ( pData == NULL )
        return 0;

    memset( g, 0, sizeof(*g) );
    g->pTech  = pTech;
    g->vStack = Vec_PtrAlloc( 32 );
    g->fTech  = fTech;
    s_pLefCtx = g;

    Lef_SetInput( pData, nSize );
    Ret = Lef_parse();
    Lef_Cleanup();
    s_pLefCtx = NULL;

    while ( Vec_PtrSize(g->vStack) > 0 )
        Lef_FrameFree( (Lef_Frame_t *)Vec_PtrPop(g->vStack) );
    Vec_PtrFree( g->vStack );
    ABC_FREE( pData );

    if ( Ret != 0 || g->nErrors != 0 )
    {
        fprintf( stderr, "LEF: failed to parse \"%s\".\n", pFileName );
        return 0;
    }
    if ( fVerbose )
        fprintf( stdout, "LEF: parsed %s: %d layers, %d sites, %d macros, %d vias (DBU=%d).\n",
                 fTech ? "technology" : "cell", Vec_PtrSize(pTech->vLayers),
                 Vec_PtrSize(pTech->vSites), Vec_PtrSize(pTech->vMacros),
                 Vec_PtrSize(pTech->vVias), pTech->Units.Dbu );
    return 1;
}


/*------------------------------------------------------------------------*/
/* public API                                                             */
/*------------------------------------------------------------------------*/

Phys_LefTech_t * Phys_LefTechAlloc( void )
{
    Phys_LefTech_t * p = ABC_CALLOC( Phys_LefTech_t, 1 );
    Phys_UnitsInit( &p->Units );
    p->ManufacturingGrid = 0.001f;
    p->vLayers = Vec_PtrAlloc( 64 );
    p->vSites  = Vec_PtrAlloc( 8 );
    p->vMacros = Vec_PtrAlloc( 1024 );
    p->vVias   = Vec_PtrAlloc( 64 );
    return p;
}

void Phys_LefTechFree( Phys_LefTech_t ** pp )
{
    Phys_LefTech_t * p;
    Phys_LefLayer_t * pLayer;
    Phys_LefSite_t * pSite;
    Phys_LefMacro_t * pMacro;
    char * pName;
    int i;
    if ( pp == NULL || *pp == NULL )
        return;
    p = *pp;
    *pp = NULL;
    Vec_PtrForEachEntry( Phys_LefLayer_t *, p->vLayers, pLayer, i )
        Phys_LefLayerFree( pLayer );
    Vec_PtrForEachEntry( Phys_LefSite_t *, p->vSites, pSite, i )
        Phys_LefSiteFree( pSite );
    Vec_PtrForEachEntry( Phys_LefMacro_t *, p->vMacros, pMacro, i )
        Phys_LefMacroFree( pMacro );
    Vec_PtrForEachEntry( char *, p->vVias, pName, i )
        ABC_FREE( pName );
    Vec_PtrFree( p->vLayers );
    Vec_PtrFree( p->vSites );
    Vec_PtrFree( p->vMacros );
    Vec_PtrFree( p->vVias );
    Vec_IntFreeP( &p->vMacroHash );
    Phys_UnitsClear( &p->Units );
    ABC_FREE( p );
}

int Phys_LefReadTech( Phys_LefTech_t * p, const char * pFileName, int fVerbose )
{
    return Lef_ParseFile( p, pFileName, 1, fVerbose );
}

int Phys_LefReadMacros( Phys_LefTech_t * p, const char * pFileName, int fVerbose )
{
    return Lef_ParseFile( p, pFileName, 0, fVerbose );
}

void Phys_LefFinish( Phys_LefTech_t * p )
{
    int i;
    if ( p == NULL ) return;
    for ( i = 0; i < Vec_PtrSize(p->vMacros); i++ )
        ((Phys_LefMacro_t *)Vec_PtrEntry(p->vMacros, i))->Id = i;
    Phys_HashBuild( &p->vMacroHash, p->vMacros );
}

Phys_LefMacro_t * Phys_LefFindMacro( Phys_LefTech_t * p, const char * pName )
{
    int Id;
    if ( p == NULL || pName == NULL )
        return NULL;
    Id = Phys_HashFind( p->vMacroHash, p->vMacros, pName );
    return Id >= 0 ? (Phys_LefMacro_t *)Vec_PtrEntry(p->vMacros, Id) : NULL;
}

Phys_LefLayer_t * Phys_LefFindLayer( Phys_LefTech_t * p, const char * pName )
{
    Phys_LefLayer_t * pLayer;
    int i;
    if ( p == NULL || pName == NULL )
        return NULL;
    Vec_PtrForEachEntry( Phys_LefLayer_t *, p->vLayers, pLayer, i )
        if ( !strcmp( pLayer->pName, pName ) )
            return pLayer;
    return NULL;
}

Phys_LefSite_t * Phys_LefFindSite( Phys_LefTech_t * p, const char * pName )
{
    Phys_LefSite_t * pSite;
    int i;
    if ( p == NULL || pName == NULL )
        return NULL;
    Vec_PtrForEachEntry( Phys_LefSite_t *, p->vSites, pSite, i )
        if ( !strcmp( pSite->pName, pName ) )
            return pSite;
    return NULL;
}

Phys_LefPin_t * Phys_LefMacroFindPin( Phys_LefMacro_t * pMacro, const char * pName )
{
    Phys_LefPin_t * pPin;
    int i;
    if ( pMacro == NULL || pName == NULL )
        return NULL;
    Vec_PtrForEachEntry( Phys_LefPin_t *, pMacro->vPins, pPin, i )
        if ( !strcmp( pPin->pName, pName ) )
            return pPin;
    return NULL;
}

int Phys_LefMacroPinIndex( Phys_LefMacro_t * pMacro, const char * pName )
{
    Phys_LefPin_t * pPin;
    int i;
    if ( pMacro == NULL || pName == NULL )
        return -1;
    Vec_PtrForEachEntry( Phys_LefPin_t *, pMacro->vPins, pPin, i )
        if ( !strcmp( pPin->pName, pName ) )
            return i;
    return -1;
}

Phys_LefLayer_t * Phys_LefFindRcLayer( Phys_LefTech_t * p, const char * pName )
{
    Phys_LefLayer_t * pLayer;
    int i;
    if ( p == NULL )
        return NULL;
    if ( pName != NULL && strcmp( pName, "auto" ) )
        return Phys_LefFindLayer( p, pName );
    Vec_PtrForEachEntry( Phys_LefLayer_t *, p->vLayers, pLayer, i )
        if ( pLayer->fRouting && pLayer->Width > 0 &&
             pLayer->ResPerSq > 0.0f && pLayer->CapPerSq > 0.0f )
            return pLayer;
    Vec_PtrForEachEntry( Phys_LefLayer_t *, p->vLayers, pLayer, i )
        if ( pLayer->fRouting && pLayer->Width > 0 )
            return pLayer;
    return NULL;
}

void Phys_LefLayerRc( Phys_LefTech_t * p, Phys_LefLayer_t * pLayer, float * pResPerMicron, float * pCapPerMicron )
{
    float WidthMicron;
    if ( pLayer == NULL )
    {
        if ( pResPerMicron ) *pResPerMicron = 0.0f;
        if ( pCapPerMicron ) *pCapPerMicron = 0.0f;
        return;
    }
    WidthMicron = (float)pLayer->Width / (float)(p ? p->Units.Dbu : 1000);
    if ( WidthMicron <= 0.0f )
        WidthMicron = 0.1f;
    if ( pResPerMicron )
        *pResPerMicron = pLayer->ResPerSq / WidthMicron;   /* ohm/um   */
    if ( pCapPerMicron )
        *pCapPerMicron = pLayer->CapPerSq * WidthMicron + 2.0f * pLayer->EdgeCap; /* pF/um */
}

void Phys_LefTechPrint( Phys_LefTech_t * p )
{
    Phys_LefLayer_t * pLayer;
    int i;
    if ( p == NULL ) return;
    fprintf( stdout, "LEF technology: DBU=%d, grid=%.4f um, %d layers, %d sites, %d macros.\n",
             p->Units.Dbu, p->ManufacturingGrid, Vec_PtrSize(p->vLayers),
             Vec_PtrSize(p->vSites), Vec_PtrSize(p->vMacros) );
    Vec_PtrForEachEntry( Phys_LefLayer_t *, p->vLayers, pLayer, i )
        if ( pLayer->fRouting )
            fprintf( stdout, "  layer %-12s width=%d pitch=(%d,%d) Rpersq=%.4f Cpersq=%.6f EdgeC=%.6f\n",
                     pLayer->pName, pLayer->Width, pLayer->PitchX, pLayer->PitchY,
                     pLayer->ResPerSq, pLayer->CapPerSq, pLayer->EdgeCap );
}

void Phys_LefMacroPrint( Phys_LefTech_t * p, Phys_LefMacro_t * pMacro )
{
    Phys_LefPin_t * pPin;
    int i;
    if ( pMacro == NULL ) return;
    fprintf( stdout, "MACRO %s class=%s site=%s size=%dx%d DBU, %d pins, %d obs rects.\n",
             pMacro->pName, pMacro->pClass ? pMacro->pClass : "",
             pMacro->pSite ? pMacro->pSite : "", pMacro->Width, pMacro->Height,
             pMacro->nPins, pMacro->nObs );
    Vec_PtrForEachEntry( Phys_LefPin_t *, pMacro->vPins, pPin, i )
        fprintf( stdout, "  pin %-10s dir=%d center=(%d,%d) rects=%d\n",
                 pPin->pName, pPin->Dir, pPin->x, pPin->y, pPin->nRects );
    (void)p;
}

ABC_NAMESPACE_IMPL_END

/* The generated scanner/parser are compiled as part of this translation
   unit so that they can use the static helpers above. */
#include "physLefYacc.c"
#include "physLefLex.c"
