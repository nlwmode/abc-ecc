/**Function*************************************************************

  Synopsis    [Placement workspace writer for debugging]

  Description [See physDump.h.  Everything here is read-only with respect
               to the session: the cells come from the positions saved by
               &phys_place and the timing from the slacks saved by
               &phys_sta.]

  SideEffects [Creates directories and files]

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include "aig/gia/gia.h"
#include "phys/physMan.h"
#include "phys/lib/physLef.h"
#include "phys/lib/physLiberty.h"
#include "phys/place/physPlace.h"
#include "phys/place/physCong.h"
#include "phys/place/physDump.h"

ABC_NAMESPACE_IMPL_START


typedef struct Phys_DumpCell_t_
{
    int    Obj;        /* Gia object id                    */
    int    Row;        /* row index, -1 if outside the core */
    int    W, H;       /* size in DBU                       */
    float  X, Y;       /* center, um                        */
    float  X0, Y0;     /* lower-left corner, um             */
    Phys_LibCell_t * pCell;
} Phys_DumpCell_t;

typedef struct Phys_DumpNet_t_
{
    int        Driver;   /* Gia object driving the net       */
    Vec_Int_t * vSinks;  /* Gia objects of the sinks         */
    Vec_Int_t * vPins;   /* input pin of each sink, -1 for a PO */
    float      X0, Y0, X1, Y1;
    float      Hpwl;
} Phys_DumpNet_t;

/* color stops for a single-hue, colorblind-safe sequential ramp */
typedef struct Phys_DumpStop_t_
{
    float Pos;
    int   R, G, B;
} Phys_DumpStop_t;

/* cell density: one blue ramp, light (empty) to medium (full).  The
   ramp deliberately stops before the darkest blue so that the density
   stays a background wash and the cell boxes drawn on top keep the
   strongest contrast. */
static const Phys_DumpStop_t Phys_DumpDensityStops[] = {
    { 0.00f, 234, 243, 254 },
    { 0.25f, 205, 226, 251 },
    { 0.50f, 158, 197, 244 },
    { 0.75f, 109, 167, 236 },
    { 1.00f,  57, 135, 229 }
};

/* congestion: the next sequential hue, orange, light (empty) to dark */
static const Phys_DumpStop_t Phys_DumpCongStops[] = {
    { 0.00f, 253, 240, 232 },
    { 0.25f, 248, 201, 178 },
    { 0.50f, 242, 155, 114 },
    { 0.75f, 235, 104,  52 },
    { 1.00f, 168,  58,  18 }
};


static char * Phys_DumpStrdup( const char * p )
{
    char * pOut = ABC_ALLOC( char, strlen(p) + 1 );
    strcpy( pOut, p );
    return pOut;
}

void Phys_DumpSetDefault( Phys_DumpPars_t * p )
{
    p->pDir      = "phys_ws";
    p->pDesign   = NULL;
    p->pTag      = NULL;
    p->nMaxNets  = 2000;
    p->nBins     = 32;
    p->nCongBins = 64;
    p->fVerbose  = 0;
}

static int Phys_DumpMkdir( const char * pPath )
{
#ifdef _WIN32
    return _mkdir( pPath );
#else
    return mkdir( pPath, 0777 );
#endif
}

/* creates a directory and the missing ones above it */
static void Phys_DumpMkdirAll( const char * pPath )
{
    char * pTmp = Phys_DumpStrdup( pPath );
    int i, n = (int)strlen( pTmp );
    for ( i = 1; i <= n; i++ )
        if ( pTmp[i] == '/' || pTmp[i] == '\\' || pTmp[i] == 0 )
        {
            char c = pTmp[i];
            pTmp[i] = 0;
            if ( pTmp[0] != 0 )
                Phys_DumpMkdir( pTmp );
            pTmp[i] = c;
        }
    ABC_FREE( pTmp );
}

static char * Phys_DumpJoin2( const char * pA, const char * pB )
{
    char * pOut = ABC_ALLOC( char, strlen(pA) + strlen(pB) + 2 );
    sprintf( pOut, "%s/%s", pA, pB );
    return pOut;
}

static char * Phys_DumpJoin3( const char * pA, const char * pB, const char * pC )
{
    char * pOut = ABC_ALLOC( char, strlen(pA) + strlen(pB) + strlen(pC) + 3 );
    sprintf( pOut, "%s/%s/%s", pA, pB, pC );
    return pOut;
}

/* a snapshot file, "<dir>/<design>/<tag>.<ext>" */
static char * Phys_DumpSnapFile( const char * pDir, const char * pDesign, const char * pTag, const char * pExt )
{
    char * pName = ABC_ALLOC( char, strlen(pTag) + strlen(pExt) + 2 );
    char * pOut;
    sprintf( pName, "%s.%s", pTag, pExt );
    pOut = Phys_DumpJoin3( pDir, pDesign, pName );
    ABC_FREE( pName );
    return pOut;
}

/* keep only characters that are portable in a file name */
static char * Phys_DumpCleanName( const char * pIn )
{
    char * pOut = ABC_ALLOC( char, strlen(pIn) + 1 );
    int i, n = 0;
    for ( i = 0; pIn[i]; i++ )
    {
        char c = pIn[i];
        if ( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
             (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-' )
            pOut[n++] = c;
        else
            pOut[n++] = '_';
    }
    if ( n == 0 )
        pOut[n++] = 'd';
    pOut[n] = 0;
    return pOut;
}

static const char * Phys_DumpBaseName( const char * pPath )
{
    const char * pSlash = strrchr( pPath, '/' );
    const char * pBack  = strrchr( pPath, '\\' );
    const char * p = pPath;
    if ( pSlash != NULL )
        p = pSlash + 1;
    if ( pBack != NULL && pBack + 1 > p )
        p = pBack + 1;
    return p;
}

/* read a text file into a vector of lines; returns an empty vector if the
   file does not exist */
static Vec_Ptr_t * Phys_DumpReadLines( const char * pFile )
{
    Vec_Ptr_t * vLines = Vec_PtrAlloc( 16 );
    char Line[1024];
    FILE * f = fopen( pFile, "r" );
    if ( f == NULL )
        return vLines;
    while ( fgets( Line, sizeof(Line), f ) != NULL )
    {
        int n = (int)strlen( Line );
        while ( n > 0 && (Line[n-1] == '\n' || Line[n-1] == '\r') )
            Line[--n] = 0;
        if ( n == 0 )
            continue;
        Vec_PtrPush( vLines, Phys_DumpStrdup(Line) );
    }
    fclose( f );
    return vLines;
}

/* appends a line to a list of strings unless it is already there */
static void Phys_DumpAddUnique( Vec_Ptr_t * vLines, const char * pLine )
{
    char * pEntry;
    int i;
    Vec_PtrForEachEntry( char *, vLines, pEntry, i )
        if ( strcmp( pEntry, pLine ) == 0 )
            return;
    Vec_PtrPush( vLines, Phys_DumpStrdup(pLine) );
}

static void Phys_DumpWriteLines( const char * pFile, Vec_Ptr_t * vLines )
{
    FILE * f = fopen( pFile, "w" );
    char * pEntry;
    int i;
    if ( f == NULL )
        return;
    Vec_PtrForEachEntry( char *, vLines, pEntry, i )
        fprintf( f, "%s\n", pEntry );
    fclose( f );
}

static void Phys_DumpFreeLines( Vec_Ptr_t * vLines )
{
    char * pEntry;
    int i;
    Vec_PtrForEachEntry( char *, vLines, pEntry, i )
        ABC_FREE( pEntry );
    Vec_PtrFree( vLines );
}

/* interpolates a one-hue ramp; Ratio is clamped to [0, 1] */
static void Phys_DumpRamp( const Phys_DumpStop_t * pStops, int nStops, float Ratio, char * pStr )
{
    int i;
    float R, G, B, u;
    if ( Ratio < 0.0f ) Ratio = 0.0f;
    if ( Ratio > 1.0f ) Ratio = 1.0f;
    for ( i = 1; i < nStops; i++ )
        if ( Ratio <= pStops[i].Pos || i == nStops - 1 )
            break;
    u = (Ratio - pStops[i-1].Pos) / (pStops[i].Pos - pStops[i-1].Pos);
    if ( u < 0.0f ) u = 0.0f;
    if ( u > 1.0f ) u = 1.0f;
    R = pStops[i-1].R + u * (float)(pStops[i].R - pStops[i-1].R);
    G = pStops[i-1].G + u * (float)(pStops[i].G - pStops[i-1].G);
    B = pStops[i-1].B + u * (float)(pStops[i].B - pStops[i-1].B);
    sprintf( pStr, "#%02x%02x%02x", (int)(R + 0.5f), (int)(G + 0.5f), (int)(B + 0.5f) );
}

static void Phys_DumpColorDensity( float Ratio, char * pStr )
{
    Phys_DumpRamp( Phys_DumpDensityStops,
                   (int)(sizeof(Phys_DumpDensityStops) / sizeof(Phys_DumpDensityStops[0])),
                   Ratio, pStr );
}

static void Phys_DumpColorCongestion( float Ratio, char * pStr )
{
    Phys_DumpRamp( Phys_DumpCongStops,
                   (int)(sizeof(Phys_DumpCongStops) / sizeof(Phys_DumpCongStops[0])),
                   Ratio, pStr );
}

/* the coordinates of an object, if it has one */
static int Phys_DumpObjPos( Phys_Man_t * p, int iObj, float Dbu, float * pX, float * pY )
{
    int x, y;
    if ( iObj <= 0 || iObj >= Vec_IntSize(p->vObjX) )
        return 0;
    x = Vec_IntEntry( p->vObjX, iObj );
    y = Vec_IntEntry( p->vObjY, iObj );
    if ( x < 0 || y < 0 )
        return 0;
    *pX = (float)x / Dbu;
    *pY = (float)y / Dbu;
    return 1;
}

/* the cell instance that gave an object its position: the positive phase
   when it is mapped, otherwise the negative one */
static int Phys_DumpObjLit( Phys_Man_t * p, int iObj )
{
    if ( iObj <= 0 || 2 * iObj + 1 >= Phys_ManLitNum(p) )
        return -1;
    if ( Phys_ManLitIsCell( p, 2 * iObj ) )
        return 2 * iObj;
    if ( Phys_ManLitIsCell( p, 2 * iObj + 1 ) )
        return 2 * iObj + 1;
    return -1;
}

/* the position of a pin of an object: negative pin index selects the
   output pin, a non-negative one an input pin in fanin order.  When the
   pin has no LEF geometry the cell center is the anchor. */
static int Phys_DumpObjPinPos( Phys_Man_t * p, int iObj, int iPin, float Dbu,
                               float * pX, float * pY )
{
    int iLit = Phys_DumpObjLit( p, iObj ), dx = 0, dy = 0;
    if ( !Phys_DumpObjPos( p, iObj, Dbu, pX, pY ) )
        return 0;
    if ( iLit >= 0 )
        Phys_ManLitPinOffset( p, iLit, iPin, &dx, &dy );
    *pX += (float)dx / Dbu;
    *pY += (float)dy / Dbu;
    return 1;
}

/* the input pin of iSinkObj that iDriverObj drives, or -1 when the two
   objects are not directly connected */
static int Phys_DumpFaninPin( Phys_Man_t * p, Gia_Man_t * pGia, int iDriverObj, int iSinkObj )
{
    int k, nFans = Phys_ManObjFaninNum( p, pGia, iSinkObj );
    for ( k = 0; k < nFans; k++ )
        if ( Abc_Lit2Var(Phys_ManObjFaninLit( p, pGia, iSinkObj, k )) == iDriverObj )
            return k;
    return -1;
}

/* the LEF metal shapes of one pin, mapped into the picture.  Drawing the
   real pin geometry makes it obvious where the wires of a net start and
   end, and shows that a wide cell has its pins spread over its shape. */
static void Phys_DumpPinRects( FILE * f, Phys_Man_t * p, int iObj, int iPin,
                               float Dbu, float MarginX, float TopY,
                               float X0, float Y0, float H, float Scale )
{
    int iLit = Phys_DumpObjLit( p, iObj ), k;
    Phys_LefMacro_t * pMacro;
    Phys_LefPin_t * pPin;
    float Cx, Cy, Ox, Oy;
    if ( iLit < 0 || !Phys_DumpObjPos( p, iObj, Dbu, &Cx, &Cy ) )
        return;
    pMacro = Phys_ManLitMacro( p, iLit );
    pPin   = Phys_ManLitLefPin( p, iLit, iPin );
    if ( pMacro == NULL || pPin == NULL || Vec_IntSize(pPin->vRects) == 0 )
        return;
    /* the macro origin of the pin coordinates, in um */
    Ox = Cx - ((float)pMacro->OriginX + 0.5f * (float)pMacro->Width) / Dbu;
    Oy = Cy - ((float)pMacro->OriginY + 0.5f * (float)pMacro->Height) / Dbu;
    for ( k = 0; k < Vec_IntSize(pPin->vRects); k += 4 )
    {
        float x1 = Ox + (float)Vec_IntEntry(pPin->vRects, k+0) / Dbu;
        float y1 = Oy + (float)Vec_IntEntry(pPin->vRects, k+1) / Dbu;
        float x2 = Ox + (float)Vec_IntEntry(pPin->vRects, k+2) / Dbu;
        float y2 = Oy + (float)Vec_IntEntry(pPin->vRects, k+3) / Dbu;
        fprintf( f, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\"/>\n",
                 MarginX + (x1 - X0) * Scale, TopY + (Y0 + H - y2) * Scale,
                 (x2 - x1) * Scale, (y2 - y1) * Scale );
    }
}

/* the edges of the critical path: one line per pair of consecutive path
   objects that both have a placement position, from the output pin of
   the driver to the input pin of the sink.  The style lives on the
   enclosing group, so the same helper draws the halo and the stroke. */
static void Phys_DumpPathEdges( FILE * f, Phys_Man_t * p, Gia_Man_t * pGia, float Dbu, float MarginX,
                                float TopY, float X0, float Y0, float H, float Scale )
{
    int fPrev = 0, k, iObj, iPrevObj = -1;
    if ( p->vPathObjs == NULL )
        return;
    for ( k = 0; k < Vec_IntSize(p->vPathObjs); k++ )
    {
        float Cx, Cy, Px, Py, Sx, Sy;
        int iPin;
        iObj = Vec_IntEntry( p->vPathObjs, k );
        if ( !Phys_DumpObjPos( p, iObj, Dbu, &Cx, &Cy ) )
            /* an object inside a mapped cell has no position of its own;
               the path just passes through it to the next placed object */
            continue;
        if ( fPrev )
        {
            Phys_DumpObjPinPos( p, iPrevObj, -1, Dbu, &Px, &Py );
            iPin = Phys_DumpFaninPin( p, pGia, iPrevObj, iObj );
            if ( iPin >= 0 )
                Phys_DumpObjPinPos( p, iObj, iPin, Dbu, &Sx, &Sy );
            else
                Sx = Cx, Sy = Cy;
            fprintf( f, "<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\"/>\n",
                     MarginX + (Px - X0) * Scale, TopY + (Y0 + H - Py) * Scale,
                     MarginX + (Sx - X0) * Scale, TopY + (Y0 + H - Sy) * Scale );
        }
        iPrevObj = iObj;
        fPrev = 1;
    }
}

static void Phys_DumpNetAdd( Vec_Ptr_t * vNets, int iDriver, int iSink, int iPin )
{
    Phys_DumpNet_t * pNet;
    if ( iDriver <= 0 )
        return;
    pNet = (Phys_DumpNet_t *)Vec_PtrEntry( vNets, iDriver );
    if ( pNet == NULL )
    {
        pNet = ABC_CALLOC( Phys_DumpNet_t, 1 );
        pNet->Driver = iDriver;
        pNet->vSinks = Vec_IntAlloc( 4 );
        pNet->vPins  = Vec_IntAlloc( 4 );
        Vec_PtrWriteEntry( vNets, iDriver, pNet );
    }
    if ( Vec_IntFind( pNet->vSinks, iSink ) >= 0 )
        return;
    Vec_IntPush( pNet->vSinks, iSink );
    Vec_IntPush( pNet->vPins, iPin );
}

/* the same net model as the placer: one net per driver, listing its
   sinks and the input pin every sink uses */
static Vec_Ptr_t * Phys_DumpBuildNets( Phys_Man_t * p, Gia_Man_t * pGia, int nObjs )
{
    Vec_Ptr_t * vNets = Vec_PtrStart( nObjs );
    int i, k;
    for ( i = 1; i < nObjs; i++ )
    {
        Gia_Obj_t * pObj = Gia_ManObj( pGia, i );
        int nFans;
        if ( Gia_ObjIsPo(pGia, pObj) )
        {
            Phys_DumpNetAdd( vNets, Abc_Lit2Var(Gia_ObjFaninLit0(pObj, i)), i, -1 );
            continue;
        }
        if ( Vec_PtrEntry(p->vObjToCell, i) == NULL )
            continue;
        nFans = Phys_ManObjFaninNum( p, pGia, i );
        for ( k = 0; k < nFans; k++ )
            Phys_DumpNetAdd( vNets, Abc_Lit2Var(Phys_ManObjFaninLit( p, pGia, i, k )), i, k );
    }
    return vNets;
}

static int Phys_DumpCompareNets( const void * p1, const void * p2 )
{
    float a = ((const Phys_DumpNet_t *)p1)->Hpwl;
    float b = ((const Phys_DumpNet_t *)p2)->Hpwl;
    return a < b ? 1 : a > b ? -1 : 0;
}


static void Phys_DumpCongWriteCsv( const char * pFile, Phys_Cong_t * pCong )
{
    FILE * f = fopen( pFile, "w" );
    int bx, by;
    if ( f == NULL )
        return;
    fprintf( f, "bin_x,bin_y,x0_um,y0_um,x1_um,y1_um,rudy_um,routing_um,congestion,overflow\n" );
    for ( by = 0; by < pCong->nY; by++ )
    for ( bx = 0; bx < pCong->nX; bx++ )
    {
        int i = by * pCong->nX + bx;
        fprintf( f, "%d,%d,%.3f,%.3f,%.3f,%.3f,%.5f,%.5f,%.5f,%d\n",
                 bx, by,
                 pCong->X0 + bx * pCong->BinW, pCong->Y0 + by * pCong->BinH,
                 pCong->X0 + (bx + 1) * pCong->BinW, pCong->Y0 + (by + 1) * pCong->BinH,
                 pCong->pDemand[i], pCong->Supply, pCong->pCong[i],
                 pCong->pCong[i] > 1.0f ? 1 : 0 );
    }
    fclose( f );
}

/**Function*************************************************************

  Synopsis    [Writes the RUDY routability picture]

  Description [The bins are colored with the orange one-hue ramp used for
               the second sequential context (the placement background
               uses blue).  Bins whose demand exceeds the routing supply
               get a critical-red outline, so overflow is not carried by
               color alone.  A color bar makes the scale readable.]

  SideEffects [Creates a file]

***********************************************************************/

static void Phys_DumpCongWriteSvg( const char * pFile, const char * pDesign,
                                   const char * pTag, Phys_Cong_t * pCong )
{
    FILE * f = fopen( pFile, "w" );
    float Scale, MarginX = 40.0f, TopY = 82.0f, BottomY = 66.0f;
    float CoreW, CoreH, MaxRatio, BarX, BarY, BarW, BarH;
    int ImgW, ImgH, bx, by, s, nSeg = 48;
    char Color[16];
    if ( f == NULL )
        return;
    Scale  = 1000.0f / (pCong->W > pCong->H ? pCong->W : pCong->H);
    CoreW  = pCong->W * Scale;
    CoreH  = pCong->H * Scale;
    ImgW   = (int)( CoreW + 2 * MarginX + 0.5f );
    ImgH   = (int)( CoreH + TopY + BottomY + 0.5f );
    MaxRatio = pCong->MaxCong > 1e-9f ? pCong->MaxCong : 0.0f;
    fprintf( f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
    fprintf( f, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" "
                "viewBox=\"0 0 %d %d\">\n", ImgW, ImgH, ImgW, ImgH );
    fprintf( f, "<rect width=\"100%%\" height=\"100%%\" fill=\"#fcfcfb\"/>\n" );
    fprintf( f, "<text x=\"8\" y=\"22\" font-family=\"sans-serif\" font-size=\"14\" fill=\"#0b0b0b\">"
                "%s &#8211; %s: RUDY routability, max congestion %.2f</text>\n",
                pDesign, pTag, pCong->MaxCong );
    fprintf( f, "<text x=\"8\" y=\"42\" font-family=\"sans-serif\" font-size=\"11\" fill=\"#52514e\">"
                "%d x %d bins; wire demand %.1f um, routing supply %.1f um; "
                "%d bin(s) over capacity</text>\n",
                pCong->nX, pCong->nY, pCong->TotalDemand, pCong->TotalSupply, pCong->nOverflow );
    if ( pCong->nOverflow > 0 )
        fprintf( f, "<text x=\"8\" y=\"58\" font-family=\"sans-serif\" font-size=\"11\" fill=\"#898781\">"
                    "orange: uniformly spread wire demand; red outline: demand above routing supply</text>\n" );
    else
        fprintf( f, "<text x=\"8\" y=\"58\" font-family=\"sans-serif\" font-size=\"11\" fill=\"#898781\">"
                    "orange: uniformly spread wire demand; no bin exceeds the routing supply</text>\n" );
    /* the bins */
    fprintf( f, "<g id=\"congestion\">\n" );
    for ( by = 0; by < pCong->nY; by++ )
    for ( bx = 0; bx < pCong->nX; bx++ )
    {
        int i = by * pCong->nX + bx;
        if ( pCong->pDemand[i] <= 0.0f )
            continue;
        Phys_DumpColorCongestion( MaxRatio > 0.0f ? pCong->pCong[i] / MaxRatio : 0.0f, Color );
        fprintf( f, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" fill=\"%s\"/>\n",
                 MarginX + bx * pCong->BinW * Scale,
                 TopY + (pCong->H - (by + 1) * pCong->BinH) * Scale,
                 pCong->BinW * Scale + 0.5f, pCong->BinH * Scale + 0.5f, Color );
    }
    fprintf( f, "</g>\n" );
    if ( pCong->nOverflow > 0 )
    {
        fprintf( f, "<g id=\"overflow\" fill=\"none\" stroke=\"#d03b3b\" "
                    "stroke-width=\"0.8\" stroke-opacity=\"0.9\">\n" );
        for ( by = 0; by < pCong->nY; by++ )
        for ( bx = 0; bx < pCong->nX; bx++ )
            if ( pCong->pCong[by * pCong->nX + bx] > 1.0f )
                fprintf( f, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\"/>\n",
                         MarginX + bx * pCong->BinW * Scale,
                         TopY + (pCong->H - (by + 1) * pCong->BinH) * Scale,
                         pCong->BinW * Scale, pCong->BinH * Scale );
        fprintf( f, "</g>\n" );
    }
    fprintf( f, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" "
                "fill=\"none\" stroke=\"#52514e\" stroke-width=\"1\"/>\n",
             MarginX, TopY, CoreW, CoreH );
    /* the color bar */
    BarX = MarginX;
    BarY = TopY + CoreH + 16.0f;
    BarW = CoreW < 280.0f ? CoreW : 280.0f;
    BarH = 10.0f;
    fprintf( f, "<text x=\"%.2f\" y=\"%.2f\" font-family=\"sans-serif\" font-size=\"10\" "
                "fill=\"#52514e\">congestion (demand / supply)</text>\n", BarX, BarY - 5.0f );
    for ( s = 0; s < nSeg; s++ )
    {
        Phys_DumpColorCongestion( (s + 0.5f) / (float)nSeg, Color );
        fprintf( f, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" fill=\"%s\"/>\n",
                 BarX + BarW * s / (float)nSeg, BarY,
                 BarW / (float)nSeg + 0.3f, BarH, Color );
    }
    fprintf( f, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" "
                "fill=\"none\" stroke=\"#c3c2b7\" stroke-width=\"0.8\"/>\n",
             BarX, BarY, BarW, BarH );
    fprintf( f, "<text x=\"%.2f\" y=\"%.2f\" font-family=\"sans-serif\" font-size=\"10\" "
                "fill=\"#52514e\">0</text>\n", BarX, BarY + BarH + 12.0f );
    fprintf( f, "<text x=\"%.2f\" y=\"%.2f\" font-family=\"sans-serif\" font-size=\"10\" "
                "fill=\"#52514e\" text-anchor=\"middle\">%.3f</text>\n",
             BarX + 0.5f * BarW, BarY + BarH + 12.0f, 0.5f * MaxRatio );
    fprintf( f, "<text x=\"%.2f\" y=\"%.2f\" font-family=\"sans-serif\" font-size=\"10\" "
                "fill=\"#52514e\" text-anchor=\"end\">%.3f</text>\n",
             BarX + BarW, BarY + BarH + 12.0f, MaxRatio );
    fprintf( f, "</svg>\n" );
    fclose( f );
}


/**Function*************************************************************

  Synopsis    [Writes the workspace index]

  Description [Lists every design found in designs.txt and every snapshot
               listed in its snapshots.txt.]

  SideEffects []

***********************************************************************/

static void Phys_DumpWriteIndex( const char * pDir )
{
    char * pFile = Phys_DumpJoin2( pDir, "index.html" );
    char * pList = Phys_DumpJoin2( pDir, "designs.txt" );
    Vec_Ptr_t * vDesigns = Phys_DumpReadLines( pList );
    FILE * f = fopen( pFile, "w" );
    char * pDesign, * pTag;
    int i;
    if ( f == NULL )
    {
        ABC_FREE( pFile ); ABC_FREE( pList );
        Phys_DumpFreeLines( vDesigns );
        return;
    }
    fprintf( f, "<!DOCTYPE html>\n<html><head><meta charset=\"utf-8\">\n" );
    fprintf( f, "<title>Physical synthesis workspace</title>\n" );
    fprintf( f, "<style>body{font-family:sans-serif;margin:24px;}\n" );
    fprintf( f, "table{border-collapse:collapse;}td,th{padding:4px 12px;text-align:left;}\n" );
    fprintf( f, "tr:nth-child(even){background:#f4f4f4;}code{color:#555;}</style></head><body>\n" );
    fprintf( f, "<h1>Physical synthesis workspace</h1>\n" );
    fprintf( f, "<p>Every design has a <code>placement.log</code> with one record per dump, "
                "and one snapshot per tag: a placement picture with translucent standard-cell "
                "boxes, a RUDY routability map, and CSV files with the cell geometries, the net "
                "bounding boxes, the bin density, the per-bin congestion, and the row occupancy. "
                "When the snapshot has timing data, the critical path traced by "
                "<code>&amp;phys_sta</code> is drawn in magenta on top of the placement, with "
                "its cells repainted; the path objects are listed in "
                "<code>critical_path.csv</code>.</p>\n" );
    fprintf( f, "<table><tr><th>design</th><th>snapshots</th><th>log</th></tr>\n" );
    Vec_PtrForEachEntry( char *, vDesigns, pDesign, i )
    {
        char * pSnaps = Phys_DumpJoin3( pDir, pDesign, "snapshots.txt" );
        char * pRun = Phys_DumpJoin3( pDir, pDesign, "run.log" );
        Vec_Ptr_t * vSnaps = Phys_DumpReadLines( pSnaps );
        FILE * fRun = fopen( pRun, "r" );
        int k;
        fprintf( f, "<tr><td><b>%s</b></td><td>", pDesign );
        Vec_PtrForEachEntry( char *, vSnaps, pTag, k )
        {
            char * pRoute = Phys_DumpSnapFile( pDir, pDesign, pTag, "routability.svg" );
            char * pCong  = Phys_DumpSnapFile( pDir, pDesign, pTag, "congestion.csv" );
            char * pPath  = Phys_DumpSnapFile( pDir, pDesign, pTag, "critical_path.csv" );
            FILE * fRoute = fopen( pRoute, "r" );
            FILE * fCong  = fopen( pCong, "r" );
            FILE * fPath  = fopen( pPath, "r" );
            fprintf( f, "%s<b>%s</b>: <a href=\"%s/%s.svg\">placement</a>",
                     k ? "<br>" : "", pTag, pDesign, pTag );
            if ( fRoute != NULL )
            {
                fprintf( f, " &middot; <a href=\"%s/%s.routability.svg\">routability</a>",
                         pDesign, pTag );
                fclose( fRoute );
            }
            if ( fCong != NULL )
            {
                fprintf( f, " &middot; <a href=\"%s/%s.congestion.csv\">congestion.csv</a>",
                         pDesign, pTag );
                fclose( fCong );
            }
            if ( fPath != NULL )
            {
                fprintf( f, " &middot; <a href=\"%s/%s.critical_path.csv\">critical_path.csv</a>",
                         pDesign, pTag );
                fclose( fPath );
            }
            ABC_FREE( pRoute );
            ABC_FREE( pCong );
            ABC_FREE( pPath );
        }
        fprintf( f, "</td><td><a href=\"%s/placement.log\">placement.log</a>", pDesign );
        if ( fRun != NULL )
        {
            fprintf( f, " &middot; <a href=\"%s/run.log\">run.log</a>", pDesign );
            fclose( fRun );
        }
        fprintf( f, "</td></tr>\n" );
        Phys_DumpFreeLines( vSnaps );
        ABC_FREE( pSnaps ); ABC_FREE( pRun );
    }
    fprintf( f, "</table>\n</body></html>\n" );
    fclose( f );
    ABC_FREE( pFile ); ABC_FREE( pList );
    Phys_DumpFreeLines( vDesigns );
}


/**Function*************************************************************

  Synopsis    [Writes a snapshot of the placement into the workspace]

  Description []

  SideEffects [Creates directories and files]

***********************************************************************/

int Phys_DumpRun( Phys_Man_t * pMan, Gia_Man_t * pGia, Phys_DumpPars_t * pPars )
{
    Phys_DumpCell_t * pCells;
    Phys_DumpNet_t * pNets, * pNet;
    Phys_Cong_t Cong = {0}, * pCongSrc;
    Vec_Ptr_t * vNets, * vSnaps, * vDesigns;
    float * pBinArea;
    float * pRowUsed;
    int * pRowCells;
    char * pDir, * pDesign, * pTag, * pDesignDir;
    char * pLog, * pSvg, * pCellsFile, * pNetsFile, * pDensity, * pRows, * pSnapsFile;
    char * pCongFile, * pRouteSvg, * pPathFile;
    char Color[16], TimeStr[32];
    int fCong, fPath, nPathCells = 0;
    FILE * f;
    float Dbu, X0, Y0, W, H, TotalArea = 0.0f, MaxRowOcc = 0.0f, PathLen = 0.0f;
    float BinW, BinH, Scale, Margin = 40.0f, HpwlSum = 0.0f;
    int nObjs, nCells = 0, nNets = 0, nRows, nRowsUsed = 0, nBinsX, nBinsY;
    int nDrawNets, i, k, x, y, w, h;
    time_t Now;
    struct tm * pTm;

    if ( pPars == NULL || pPars->pDir == NULL )
    {
        fprintf( stdout, "Dump: no workspace directory is given.\n" );
        return 0;
    }
    if ( pMan == NULL || !pMan->fPlaced )
    {
        fprintf( stdout, "Dump: the network has not been placed. Run \"&phys_place\" first.\n" );
        return 0;
    }
    nObjs = Gia_ManObjNum( pGia );
    if ( pMan->vObjX == NULL || pMan->vObjY == NULL || Vec_IntSize(pMan->vObjX) < nObjs ||
         pMan->vObjToCell == NULL || pMan->vObjCellLit == NULL || Vec_PtrSize(pMan->vObjToCell) < nObjs )
    {
        fprintf( stdout, "Dump: the placement does not match the current network. Place it again.\n" );
        return 0;
    }
    Dbu = (float)(pMan->pLef ? pMan->pLef->Units.Dbu : 1000);
    /* names and directories */
    pDir = Phys_DumpStrdup( pPars->pDir );
    for ( i = (int)strlen(pDir) - 1; i > 0 && pDir[i] == '/'; i-- )
        pDir[i] = 0;
    if ( pPars->pDesign != NULL )
        pDesign = Phys_DumpCleanName( pPars->pDesign );
    else if ( Gia_ManName(pGia) != NULL && Gia_ManName(pGia)[0] != 0 )
        pDesign = Phys_DumpCleanName( Phys_DumpBaseName(Gia_ManName(pGia)) );
    else
        pDesign = Phys_DumpStrdup( "design" );
    pTag = Phys_DumpCleanName( pPars->pTag ? pPars->pTag : "placement" );
    /* create the workspace first, then put everything below it */
    Phys_DumpMkdirAll( pDir );
    pDesignDir = Phys_DumpJoin2( pDir, pDesign );
    Phys_DumpMkdirAll( pDesignDir );
    /* collect the placed cells */
    pCells = ABC_ALLOC( Phys_DumpCell_t, nObjs );
    for ( i = 1; i < nObjs; i++ )
    {
        Phys_LibCell_t * pCell = (Phys_LibCell_t *)Vec_PtrEntry( pMan->vObjToCell, i );
        if ( pCell == NULL )
            continue;
        x = Vec_IntEntry( pMan->vObjX, i );
        y = Vec_IntEntry( pMan->vObjY, i );
        if ( x < 0 || y < 0 )
            continue;
        Phys_PlaceCellSizeObj( pMan, i, &w, &h );
        pCells[nCells].Obj   = i;
        pCells[nCells].pCell = pCell;
        pCells[nCells].W     = w;
        pCells[nCells].H     = h;
        pCells[nCells].X     = (float)x / Dbu;
        pCells[nCells].Y     = (float)y / Dbu;
        pCells[nCells].X0    = pCells[nCells].X - 0.5f * (float)w / Dbu;
        pCells[nCells].Y0    = pCells[nCells].Y - 0.5f * (float)h / Dbu;
        TotalArea += (float)w * (float)h / (Dbu * Dbu);
        nCells++;
    }
    if ( nCells == 0 )
    {
        fprintf( stdout, "Dump: there are no placed cells to dump.\n" );
        ABC_FREE( pCells ); ABC_FREE( pDir ); ABC_FREE( pDesign ); ABC_FREE( pTag ); ABC_FREE( pDesignDir );
        return 0;
    }
    /* the core, in um */
    X0 = (float)pMan->CoreX0 / Dbu;
    Y0 = (float)pMan->CoreY0 / Dbu;
    W  = (float)pMan->CoreW / Dbu;
    H  = (float)pMan->CoreH / Dbu;
    if ( W <= 0.0f || H <= 0.0f || pMan->SiteH <= 0 )
    {
        fprintf( stdout, "Dump: the core is empty. Place the network again.\n" );
        ABC_FREE( pCells ); ABC_FREE( pDir ); ABC_FREE( pDesign ); ABC_FREE( pTag ); ABC_FREE( pDesignDir );
        return 0;
    }
    /* rows */
    nRows = pMan->CoreH / pMan->SiteH;
    if ( nRows < 1 ) nRows = 1;
    pRowUsed  = ABC_CALLOC( float, nRows );
    pRowCells = ABC_CALLOC( int, nRows );
    for ( i = 0; i < nCells; i++ )
    {
        int r = (int)floor( ((double)Vec_IntEntry(pMan->vObjY, pCells[i].Obj) - pMan->CoreY0) / pMan->SiteH );
        if ( r < 0 || r >= nRows )
            pCells[i].Row = -1;
        else
        {
            pCells[i].Row = r;
            pRowUsed[r]  += (float)pCells[i].W / Dbu;
            pRowCells[r]++;
        }
    }
    for ( k = 0; k < nRows; k++ )
    {
        float Occ = pRowUsed[k] / W;
        if ( pRowUsed[k] > 0.0f )
            nRowsUsed++;
        if ( MaxRowOcc < Occ )
            MaxRowOcc = Occ;
    }
    /* nets */
    vNets = Phys_DumpBuildNets( pMan, pGia, nObjs );
    pNets = ABC_ALLOC( Phys_DumpNet_t, nObjs );
    Vec_PtrForEachEntry( Phys_DumpNet_t *, vNets, pNet, k )
    {
        float fX, fY;
        int fValid = 1;
        if ( pNet == NULL || Vec_IntSize(pNet->vSinks) == 0 )
            continue;
        if ( !Phys_DumpObjPinPos( pMan, pNet->Driver, -1, Dbu, &fX, &fY ) )
            continue;
        pNet->X0 = pNet->X1 = fX;
        pNet->Y0 = pNet->Y1 = fY;
        Vec_IntForEachEntry( pNet->vSinks, x, i )
        {
            int iPin = Vec_IntEntry( pNet->vPins, i );
            if ( !Phys_DumpObjPinPos( pMan, x, iPin, Dbu, &fX, &fY ) )
            {
                fValid = 0;
                break;
            }
            pNet->X0 = Abc_MinFloat( pNet->X0, fX );
            pNet->X1 = Abc_MaxFloat( pNet->X1, fX );
            pNet->Y0 = Abc_MinFloat( pNet->Y0, fY );
            pNet->Y1 = Abc_MaxFloat( pNet->Y1, fY );
        }
        if ( !fValid )
            continue;
        pNet->Hpwl = (pNet->X1 - pNet->X0) + (pNet->Y1 - pNet->Y0);
        HpwlSum += pNet->Hpwl;
        pNets[nNets++] = *pNet;
    }
    qsort( pNets, nNets, sizeof(Phys_DumpNet_t), Phys_DumpCompareNets );
    /* density bins: square bins, never finer than one standard-cell row,
       because a bin smaller than a row only shows that the row covers it */
    {
        float BinTarget = (W > H ? W : H) / (float)(pPars->nBins < 1 ? 1 : pPars->nBins);
        float MinBin = (float)pMan->SiteH / Dbu;
        if ( BinTarget < MinBin )
            BinTarget = MinBin;
        nBinsX = (int)( W / BinTarget + 0.999f );
        nBinsY = (int)( H / BinTarget + 0.999f );
        if ( nBinsX > pPars->nBins ) nBinsX = pPars->nBins;
        if ( nBinsY > nRows ) nBinsY = nRows;
        if ( nBinsX < 1 ) nBinsX = 1;
        if ( nBinsY < 1 ) nBinsY = 1;
    }
    BinW = W / (float)nBinsX;
    BinH = H / (float)nBinsY;
    pBinArea = ABC_CALLOC( float, nBinsX * nBinsY );
    for ( i = 0; i < nCells; i++ )
    {
        float Cx0 = pCells[i].X0, Cy0 = pCells[i].Y0;
        float Cx1 = Cx0 + (float)pCells[i].W / Dbu;
        float Cy1 = Cy0 + (float)pCells[i].H / Dbu;
        int bx0 = (int)floor( (Cx0 - X0) / BinW ), bx1 = (int)floor( (Cx1 - X0) / BinW );
        int by0 = (int)floor( (Cy0 - Y0) / BinH ), by1 = (int)floor( (Cy1 - Y0) / BinH );
        int bx, by;
        if ( bx0 < 0 ) bx0 = 0;
        if ( by0 < 0 ) by0 = 0;
        if ( bx1 >= nBinsX ) bx1 = nBinsX - 1;
        if ( by1 >= nBinsY ) by1 = nBinsY - 1;
        for ( by = by0; by <= by1; by++ )
        for ( bx = bx0; bx <= bx1; bx++ )
        {
            float Ox0 = Abc_MaxFloat( Cx0, X0 + bx * BinW );
            float Ox1 = Abc_MinFloat( Cx1, X0 + (bx + 1) * BinW );
            float Oy0 = Abc_MaxFloat( Cy0, Y0 + by * BinH );
            float Oy1 = Abc_MinFloat( Cy1, Y0 + (by + 1) * BinH );
            if ( Ox1 > Ox0 && Oy1 > Oy0 )
                pBinArea[by * nBinsX + bx] += (Ox1 - Ox0) * (Oy1 - Oy0);
        }
    }
    /* RUDY routability: per-bin wire demand divided by routing supply.
       The placer already built this map; reuse it so the picture and the
       mapper see exactly the same numbers. */
    pCongSrc = pMan->pCong != NULL ? pMan->pCong : Phys_CongBuild( pMan, pGia, pPars->nCongBins );
    fCong = pCongSrc != NULL;
    if ( fCong )
        Cong = *pCongSrc;
    /* the critical path traced by the last &phys_sta, if any */
    fPath = pMan->fTimed && pMan->vPathObjs != NULL && pMan->vObjPath != NULL &&
            Vec_IntSize(pMan->vPathObjs) > 1 && Vec_IntSize(pMan->vObjPath) >= nObjs;
    if ( fPath )
    {
        int iPrevObj = -1;
        Vec_IntForEachEntry( pMan->vPathObjs, x, k )
        {
            float Px, Py, Sx, Sy;
            int iPin;
            if ( Vec_PtrEntry(pMan->vObjToCell, x) != NULL )
                nPathCells++;
            if ( !Phys_DumpObjPos( pMan, x, Dbu, &Px, &Py ) )
                continue;
            if ( iPrevObj >= 0 )
            {
                /* the same pin-to-pin line the SVG draws */
                Phys_DumpObjPinPos( pMan, iPrevObj, -1, Dbu, &Sx, &Sy );
                iPin = Phys_DumpFaninPin( pMan, pGia, iPrevObj, x );
                if ( iPin >= 0 )
                    Phys_DumpObjPinPos( pMan, x, iPin, Dbu, &Px, &Py );
                PathLen += fabs( Px - Sx ) + fabs( Py - Sy );
            }
            iPrevObj = x;
        }
    }
    /* the CSV files of this snapshot */
    pCellsFile   = Phys_DumpSnapFile( pDir, pDesign, pTag, "cells.csv" );
    pNetsFile    = Phys_DumpSnapFile( pDir, pDesign, pTag, "nets.csv" );
    pDensity     = Phys_DumpSnapFile( pDir, pDesign, pTag, "density.csv" );
    pCongFile    = Phys_DumpSnapFile( pDir, pDesign, pTag, "congestion.csv" );
    pRows        = Phys_DumpSnapFile( pDir, pDesign, pTag, "rows.csv" );
    pPathFile    = Phys_DumpSnapFile( pDir, pDesign, pTag, "critical_path.csv" );
    pSvg         = Phys_DumpSnapFile( pDir, pDesign, pTag, "svg" );
    pRouteSvg    = Phys_DumpSnapFile( pDir, pDesign, pTag, "routability.svg" );
    pLog         = Phys_DumpJoin3( pDir, pDesign, "placement.log" );
    pSnapsFile   = Phys_DumpJoin3( pDir, pDesign, "snapshots.txt" );
    if ( (f = fopen( pCellsFile, "w" )) != NULL )
    {
        fprintf( f, "obj,cell,x_um,y_um,w_um,h_um,x0_um,y0_um,row\n" );
        for ( i = 0; i < nCells; i++ )
            fprintf( f, "%d,%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d\n",
                     pCells[i].Obj, pCells[i].pCell->pName ? pCells[i].pCell->pName : "",
                     pCells[i].X, pCells[i].Y, (float)pCells[i].W / Dbu, (float)pCells[i].H / Dbu,
                     pCells[i].X0, pCells[i].Y0, pCells[i].Row );
        fclose( f );
    }
    if ( (f = fopen( pNetsFile, "w" )) != NULL )
    {
        fprintf( f, "driver_obj,driver_cell,n_sinks,x0_um,y0_um,x1_um,y1_um,hpwl_um\n" );
        for ( i = 0; i < nNets; i++ )
        {
            Phys_LibCell_t * pCell = (Phys_LibCell_t *)Vec_PtrEntry( pMan->vObjToCell, pNets[i].Driver );
            fprintf( f, "%d,%s,%d,%.3f,%.3f,%.3f,%.3f,%.3f\n",
                     pNets[i].Driver, pCell && pCell->pName ? pCell->pName : "",
                     Vec_IntSize(pNets[i].vSinks),
                     pNets[i].X0, pNets[i].Y0, pNets[i].X1, pNets[i].Y1, pNets[i].Hpwl );
        }
        fclose( f );
    }
    if ( (f = fopen( pDensity, "w" )) != NULL )
    {
        fprintf( f, "bin_x,bin_y,x0_um,y0_um,x1_um,y1_um,cell_area_um2,util\n" );
        for ( k = 0; k < nBinsY; k++ )
        for ( i = 0; i < nBinsX; i++ )
            fprintf( f, "%d,%d,%.3f,%.3f,%.3f,%.3f,%.4f,%.4f\n",
                     i, k, X0 + i * BinW, Y0 + k * BinH, X0 + (i + 1) * BinW, Y0 + (k + 1) * BinH,
                     pBinArea[k * nBinsX + i], pBinArea[k * nBinsX + i] / (BinW * BinH) );
        fclose( f );
    }
    if ( (f = fopen( pRows, "w" )) != NULL )
    {
        fprintf( f, "row,y_um,cells,used_um,capacity_um,occ\n" );
        for ( k = 0; k < nRows; k++ )
            fprintf( f, "%d,%.3f,%d,%.3f,%.3f,%.4f\n",
                     k, Y0 + (k + 0.5) * pMan->SiteH / Dbu, pRowCells[k], pRowUsed[k], W, pRowUsed[k] / W );
        fclose( f );
    }
    if ( fCong )
        Phys_DumpCongWriteCsv( pCongFile, &Cong );
    if ( fPath && (f = fopen( pPathFile, "w" )) != NULL )
    {
        fprintf( f, "step,obj,cell,x_um,y_um,arrival_ps,required_ps,slack_ps\n" );
        Vec_IntForEachEntry( pMan->vPathObjs, x, k )
        {
            Phys_LibCell_t * pCell = (Phys_LibCell_t *)Vec_PtrEntry( pMan->vObjToCell, x );
            float Px = 0.0f, Py = 0.0f;
            float Arr = 0.0f, Req = 1e30f, Slack = -1.0f;
            int fPos = Phys_DumpObjPos( pMan, x, Dbu, &Px, &Py );
            if ( x < Vec_FltSize(pMan->vObjArrRise) && x < Vec_FltSize(pMan->vObjArrFall) )
                Arr = Abc_MaxFloat( Vec_FltEntry(pMan->vObjArrRise, x),
                                    Vec_FltEntry(pMan->vObjArrFall, x) );
            if ( x < Vec_FltSize(pMan->vObjReq) )
                Req = Vec_FltEntry( pMan->vObjReq, x );
            if ( x < Vec_FltSize(pMan->vObjSlack) )
                Slack = Vec_FltEntry( pMan->vObjSlack, x );
            fprintf( f, "%d,%d,%s,", k, x, pCell && pCell->pName ? pCell->pName : "" );
            if ( fPos )
                fprintf( f, "%.3f,%.3f", Px, Py );
            else
                fprintf( f, "," );
            fprintf( f, ",%.2f,", Arr );
            if ( Req < 1e29f )
                fprintf( f, "%.2f", Req );
            fprintf( f, "," );
            if ( Slack >= 0.0f )
                fprintf( f, "%.2f", Slack );
            fprintf( f, "\n" );
        }
        fclose( f );
    }
    else if ( !fPath )
        remove( pPathFile );
    /* the picture */
    Scale  = 1100.0f / (W > H ? W : H);
    nDrawNets = pPars->nMaxNets < nNets ? pPars->nMaxNets : nNets;
    if ( (f = fopen( pSvg, "w" )) != NULL )
    {
        float MarginX = Margin, TopY = fPath ? 90.0f : 68.0f, BottomY = 58.0f;
        float CoreW = W * Scale, CoreH = H * Scale;
        float BarX, BarY, BarW, BarH;
        int ImgW = (int)( CoreW + 2 * MarginX + 0.5f );
        int ImgH = (int)( CoreH + TopY + BottomY + 0.5f );
        int s, nSeg = 48;
        fprintf( f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
        fprintf( f, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" "
                    "viewBox=\"0 0 %d %d\">\n", ImgW, ImgH, ImgW, ImgH );
        fprintf( f, "<rect width=\"100%%\" height=\"100%%\" fill=\"#fcfcfb\"/>\n" );
        fprintf( f, "<text x=\"8\" y=\"22\" font-family=\"sans-serif\" font-size=\"14\" fill=\"#0b0b0b\">"
                    "%s &#8211; %s: %d cells, utilization %.2f, wirelength %.1f um, "
                    "longest net %.1f um</text>\n",
                    pDesign, pTag, nCells, TotalArea / (W * H), HpwlSum,
                    nNets ? pNets[0].Hpwl : 0.0f );
        fprintf( f, "<text x=\"8\" y=\"42\" font-family=\"sans-serif\" font-size=\"11\" fill=\"#52514e\">"
                    "background: cell density per bin (blue, empty to full); "
                    "boxes: standard cells (translucent); lines: %d longest nets; "
                    "teal shapes: LEF pin metal (wires run pin to pin)</text>\n", nDrawNets );
        if ( fCong )
            fprintf( f, "<text x=\"8\" y=\"58\" font-family=\"sans-serif\" font-size=\"11\" fill=\"#898781\">"
                        "routability map: %s.routability.svg, max congestion %.2f</text>\n",
                        pTag, Cong.MaxCong );
        if ( fPath )
        {
            fprintf( f, "<rect x=\"8\" y=\"65\" width=\"11\" height=\"11\" fill=\"#e0226e\" "
                        "fill-opacity=\"0.22\" stroke=\"#c2185b\" stroke-width=\"0.7\"/>\n" );
            fprintf( f, "<line x1=\"24\" y1=\"70.5\" x2=\"46\" y2=\"70.5\" stroke=\"#ffffff\" "
                        "stroke-width=\"6\" stroke-linecap=\"round\"/>\n" );
            fprintf( f, "<line x1=\"24\" y1=\"70.5\" x2=\"46\" y2=\"70.5\" stroke=\"#c2185b\" "
                        "stroke-width=\"2.8\" stroke-linecap=\"round\"/>\n" );
            fprintf( f, "<text x=\"52\" y=\"76\" font-family=\"sans-serif\" font-size=\"12\" "
                        "fill=\"#c2185b\" font-weight=\"bold\">critical path: %d cells, %d objects, "
                        "%.1f um of flight length, arrival %.1f ps (critical_path.csv)</text>\n",
                        nPathCells, Vec_IntSize(pMan->vPathObjs), PathLen, pMan->PathDelay );
        }
        /* density */
        fprintf( f, "<g id=\"density\">\n" );
        for ( k = 0; k < nBinsY; k++ )
        for ( i = 0; i < nBinsX; i++ )
        {
            float Ratio = pBinArea[k * nBinsX + i] / (BinW * BinH);
            if ( Ratio < 0.01f )
                continue;
            Phys_DumpColorDensity( Ratio, Color );
            fprintf( f, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" fill=\"%s\"/>\n",
                     MarginX + i * BinW * Scale, TopY + (H - (k + 1) * BinH) * Scale,
                     BinW * Scale + 0.5f, BinH * Scale + 0.5f, Color );
        }
        fprintf( f, "</g>\n" );
        /* the row grid */
        fprintf( f, "<g id=\"rows\" stroke=\"#e1e0d9\" stroke-width=\"0.5\">\n" );
        for ( k = 0; k <= nRows; k++ )
            fprintf( f, "<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\"/>\n",
                     MarginX, TopY + (H - k * (float)pMan->SiteH / Dbu) * Scale,
                     MarginX + CoreW, TopY + (H - k * (float)pMan->SiteH / Dbu) * Scale );
        fprintf( f, "</g>\n" );
        /* the longest nets: from the output pin of the driver to the
           input pin of every sink, so the wires start where they will be
           routed, not at the cell centers */
        fprintf( f, "<g id=\"nets\" stroke=\"#52514e\" stroke-opacity=\"0.16\" stroke-width=\"0.9\">\n" );
        for ( i = 0; i < nDrawNets; i++ )
        {
            float Dx, Dy;
            if ( !Phys_DumpObjPinPos( pMan, pNets[i].Driver, -1, Dbu, &Dx, &Dy ) )
                continue;
            Vec_IntForEachEntry( pNets[i].vSinks, x, k )
            {
                float Sx, Sy;
                int iPin = Vec_IntEntry( pNets[i].vPins, k );
                if ( !Phys_DumpObjPinPos( pMan, x, iPin, Dbu, &Sx, &Sy ) )
                    continue;
                fprintf( f, "<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\"/>\n",
                         MarginX + (Dx - X0) * Scale, TopY + (Y0 + H - Dy) * Scale,
                         MarginX + (Sx - X0) * Scale, TopY + (Y0 + H - Sy) * Scale );
            }
        }
        fprintf( f, "</g>\n" );
        /* the standard cells: light translucent boxes, so the density, the
           nets and the critical path stay clearly above them */
        fprintf( f, "<g id=\"cells\" fill=\"#52514e\" fill-opacity=\"0.10\" "
                    "stroke=\"#0b0b0b\" stroke-opacity=\"0.40\" stroke-width=\"0.45\">\n" );
        for ( i = 0; i < nCells; i++ )
        {
            float Cw = (float)pCells[i].W / Dbu * Scale;
            float Ch = (float)pCells[i].H / Dbu * Scale;
            if ( Cw < 0.6f ) Cw = 0.6f;
            if ( Ch < 0.6f ) Ch = 0.6f;
            fprintf( f, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\"/>\n",
                     MarginX + (pCells[i].X0 - X0) * Scale,
                     TopY + (Y0 + H - pCells[i].Y0 - (float)pCells[i].H / Dbu) * Scale,
                     Cw, Ch );
        }
        fprintf( f, "</g>\n" );
        /* the LEF pin shapes at the ends of the drawn wires, and on the
           critical path, so the pin-anchored routing is visible */
        fprintf( f, "<g id=\"pins\" fill=\"#0f5f6b\" fill-opacity=\"0.60\" "
                    "stroke=\"#0b3f47\" stroke-opacity=\"0.70\" stroke-width=\"0.25\">\n" );
        for ( i = 0; i < nDrawNets; i++ )
        {
            Phys_DumpPinRects( f, pMan, pNets[i].Driver, -1, Dbu, MarginX, TopY, X0, Y0, H, Scale );
            Vec_IntForEachEntry( pNets[i].vSinks, x, k )
                Phys_DumpPinRects( f, pMan, x, Vec_IntEntry(pNets[i].vPins, k),
                                   Dbu, MarginX, TopY, X0, Y0, H, Scale );
        }
        if ( fPath )
        {
            int iPrevObj = -1;
            Vec_IntForEachEntry( pMan->vPathObjs, x, k )
            {
                int iPin;
                if ( iPrevObj < 0 )
                {
                    iPrevObj = x;
                    continue;
                }
                Phys_DumpPinRects( f, pMan, iPrevObj, -1, Dbu, MarginX, TopY, X0, Y0, H, Scale );
                iPin = Phys_DumpFaninPin( pMan, pGia, iPrevObj, x );
                if ( iPin >= 0 )
                    Phys_DumpPinRects( f, pMan, x, iPin, Dbu, MarginX, TopY, X0, Y0, H, Scale );
                iPrevObj = x;
            }
        }
        fprintf( f, "</g>\n" );
        /* the critical path.  The cells it crosses only get a light tint,
           and are drawn first, so that the magenta line with its white
           halo is what the eye follows; the halo keeps the line readable
           over the density map, the cells and the nets. */
        if ( fPath )
        {
            fprintf( f, "<g id=\"critcells\" fill=\"#e0226e\" fill-opacity=\"0.22\" "
                        "stroke=\"#c2185b\" stroke-opacity=\"0.75\" stroke-width=\"0.7\">\n" );
            for ( i = 0; i < nCells; i++ )
            {
                float Cw, Ch;
                if ( !Vec_IntEntry(pMan->vObjPath, pCells[i].Obj) )
                    continue;
                Cw = (float)pCells[i].W / Dbu * Scale;
                Ch = (float)pCells[i].H / Dbu * Scale;
                if ( Cw < 0.6f ) Cw = 0.6f;
                if ( Ch < 0.6f ) Ch = 0.6f;
                fprintf( f, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\"/>\n",
                         MarginX + (pCells[i].X0 - X0) * Scale,
                         TopY + (Y0 + H - pCells[i].Y0 - (float)pCells[i].H / Dbu) * Scale,
                         Cw, Ch );
            }
            fprintf( f, "</g>\n" );
            fprintf( f, "<g id=\"critpath-halo\" stroke=\"#ffffff\" stroke-opacity=\"0.85\" "
                        "stroke-width=\"6\" stroke-linecap=\"round\" fill=\"none\">\n" );
            Phys_DumpPathEdges( f, pMan, pGia, Dbu, MarginX, TopY, X0, Y0, H, Scale );
            fprintf( f, "</g>\n" );
            fprintf( f, "<g id=\"critpath\" stroke=\"#c2185b\" stroke-opacity=\"0.95\" "
                        "stroke-width=\"2.8\" stroke-linecap=\"round\" fill=\"none\">\n" );
            Phys_DumpPathEdges( f, pMan, pGia, Dbu, MarginX, TopY, X0, Y0, H, Scale );
            fprintf( f, "</g>\n" );
        }
        fprintf( f, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" "
                    "fill=\"none\" stroke=\"#52514e\" stroke-width=\"1\"/>\n",
                 MarginX, TopY, CoreW, CoreH );
        /* density color bar */
        BarX  = MarginX;
        BarY  = TopY + CoreH + 16.0f;
        BarW  = CoreW < 280.0f ? CoreW : 280.0f;
        BarH  = 10.0f;
        fprintf( f, "<text x=\"%.2f\" y=\"%.2f\" font-family=\"sans-serif\" font-size=\"10\" "
                    "fill=\"#52514e\">cell density per bin (empty to full)</text>\n",
                 BarX, BarY - 5.0f );
        for ( s = 0; s < nSeg; s++ )
        {
            Phys_DumpColorDensity( (s + 0.5f) / (float)nSeg, Color );
            fprintf( f, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" fill=\"%s\"/>\n",
                     BarX + BarW * s / (float)nSeg, BarY,
                     BarW / (float)nSeg + 0.3f, BarH, Color );
        }
        fprintf( f, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" "
                    "fill=\"none\" stroke=\"#c3c2b7\" stroke-width=\"0.8\"/>\n",
                 BarX, BarY, BarW, BarH );
        fprintf( f, "<text x=\"%.2f\" y=\"%.2f\" font-family=\"sans-serif\" font-size=\"10\" "
                    "fill=\"#52514e\">0</text>\n", BarX, BarY + BarH + 12.0f );
        fprintf( f, "<text x=\"%.2f\" y=\"%.2f\" font-family=\"sans-serif\" font-size=\"10\" "
                    "fill=\"#52514e\" text-anchor=\"middle\">0.5</text>\n",
                 BarX + 0.5f * BarW, BarY + BarH + 12.0f );
        fprintf( f, "<text x=\"%.2f\" y=\"%.2f\" font-family=\"sans-serif\" font-size=\"10\" "
                    "fill=\"#52514e\" text-anchor=\"end\">1.0</text>\n",
                 BarX + BarW, BarY + BarH + 12.0f );
        fprintf( f, "</svg>\n" );
        fclose( f );
    }
    if ( fCong )
        Phys_DumpCongWriteSvg( pRouteSvg, pDesign, pTag, &Cong );
    /* the log record */
    Now = time( NULL );
    pTm = localtime( &Now );
    strftime( TimeStr, sizeof(TimeStr), "%Y-%m-%d %H:%M:%S", pTm );
    if ( (f = fopen( pLog, "a+" )) != NULL )
    {
        long Size;
        fseek( f, 0, SEEK_END );
        Size = ftell( f );
        if ( Size == 0 )
        {
            fprintf( f, "# physical-synthesis workspace log\n" );
            fprintf( f, "# %-19s %-12s %-10s %6s %6s %10s %10s %9s %6s %5s %6s %10s %8s %7s\n",
                     "time", "tag", "design", "cells", "nets", "area_um2", "hpwl_um",
                     "longest_net", "rows", "used", "maxocc", "tmax_ps", "maxcong", "over%" );
        }
        fprintf( f, "  %-19s %-12s %-10s %6d %6d %10.1f %10.1f %9.1f %6d %5d %6.2f",
                 TimeStr, pTag, pDesign, nCells, nNets, TotalArea, HpwlSum,
                 nNets ? pNets[0].Hpwl : 0.0f, nRows, nRowsUsed, MaxRowOcc );
        if ( pMan->fTimed )
            fprintf( f, " %10.1f (period %.1f)", pMan->TMax, pMan->Period );
        else
            fprintf( f, " %10s", "-" );
        if ( fCong )
            fprintf( f, " %8.2f %6.2f\n", Cong.MaxCong,
                     100.0f * (float)Cong.nOverflow / (float)(Cong.nX * Cong.nY) );
        else
            fprintf( f, " %8s %6s\n", "-", "-" );
        fclose( f );
    }
    /* the snapshot list and the index */
    vSnaps = Phys_DumpReadLines( pSnapsFile );
    Phys_DumpAddUnique( vSnaps, pTag );
    Phys_DumpWriteLines( pSnapsFile, vSnaps );
    Phys_DumpFreeLines( vSnaps );
    vDesigns = Phys_DumpReadLines( Phys_DumpJoin2( pDir, "designs.txt" ) );
    {
        char * pList = Phys_DumpJoin2( pDir, "designs.txt" );
        Phys_DumpAddUnique( vDesigns, pDesign );
        Phys_DumpWriteLines( pList, vDesigns );
        Phys_DumpWriteIndex( pDir );
        ABC_FREE( pList );
    }
    Phys_DumpFreeLines( vDesigns );
    if ( pPars->fVerbose )
    {
        fprintf( stdout, "Dump: %s/%s: %d cells, %d nets, %d rows, max row occupancy %.2f, "
                 "wirelength %.1f um", pDesign, pTag, nCells, nNets, nRows, MaxRowOcc, HpwlSum );
        if ( fCong )
            fprintf( stdout, ", max congestion %.2f (%d bins over capacity)",
                     Cong.MaxCong, Cong.nOverflow );
        fprintf( stdout, ".\n" );
    }
    fprintf( stdout, "Dump: wrote workspace \"%s/%s\" (%d cells, %d nets, %.1f um wirelength%s).\n",
             pDir, pDesign, nCells, nNets, HpwlSum, pMan->fTimed ? ", with timing" : "" );
    /* cleanup */
    Vec_PtrForEachEntry( Phys_DumpNet_t *, vNets, pNet, k )
        if ( pNet != NULL )
        {
            Vec_IntFree( pNet->vSinks );
            Vec_IntFree( pNet->vPins );
            ABC_FREE( pNet );
        }
    Vec_PtrFree( vNets );
    ABC_FREE( pCells );
    ABC_FREE( pNets );
    ABC_FREE( pBinArea );
    ABC_FREE( pRowUsed );
    ABC_FREE( pRowCells );
    ABC_FREE( pDir ); ABC_FREE( pDesign ); ABC_FREE( pTag ); ABC_FREE( pDesignDir );
    ABC_FREE( pCellsFile ); ABC_FREE( pNetsFile ); ABC_FREE( pDensity );
    ABC_FREE( pCongFile ); ABC_FREE( pRows ); ABC_FREE( pSvg ); ABC_FREE( pRouteSvg );
    ABC_FREE( pPathFile );
    ABC_FREE( pLog ); ABC_FREE( pSnapsFile );
    return 1;
}

ABC_NAMESPACE_IMPL_END
