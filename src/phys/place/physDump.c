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
    float      X0, Y0, X1, Y1;
    float      Hpwl;
} Phys_DumpNet_t;


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

/* white for an empty bin, orange for half full, red for full */
static void Phys_DumpColor( float Ratio, char * pStr )
{
    int R, G, B;
    if ( Ratio < 0.0f ) Ratio = 0.0f;
    if ( Ratio > 1.0f ) Ratio = 1.0f;
    if ( Ratio < 0.5f )
    {
        float u = Ratio / 0.5f;
        R = 255;
        G = 255 - (int)(120.0f * u);
        B = 255 - (int)(210.0f * u);
    }
    else
    {
        float u = (Ratio - 0.5f) / 0.5f;
        R = 255 - (int)(28.0f * u);
        G = 135 - (int)(80.0f * u);
        B = 45  + (int)(20.0f * u);
    }
    sprintf( pStr, "#%02x%02x%02x", R, G, B );
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

static void Phys_DumpNetAdd( Vec_Ptr_t * vNets, int iDriver, int iSink )
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
        Vec_PtrWriteEntry( vNets, iDriver, pNet );
    }
    Vec_IntPushUnique( pNet->vSinks, iSink );
}

/* the same net model as the placer: one net per driver, listing its sinks */
static Vec_Ptr_t * Phys_DumpBuildNets( Phys_Man_t * p, Gia_Man_t * pGia, int nObjs )
{
    Vec_Ptr_t * vNets = Vec_PtrStart( nObjs );
    int i, k;
    for ( i = 1; i < nObjs; i++ )
    {
        Gia_Obj_t * pObj = Gia_ManObj( pGia, i );
        int iLit;
        if ( Gia_ObjIsPo(pGia, pObj) )
        {
            Phys_DumpNetAdd( vNets, Abc_Lit2Var(Gia_ObjFaninLit0(pObj, i)), i );
            continue;
        }
        if ( Vec_PtrEntry(p->vObjToCell, i) == NULL )
            continue;
        iLit = Vec_IntEntry( p->vObjCellLit, i );
        if ( iLit < 0 )
            continue;
        for ( k = 0; k < Gia_ObjCellSize(pGia, iLit); k++ )
            Phys_DumpNetAdd( vNets, Abc_Lit2Var(Gia_ObjCellFanin(pGia, iLit, k)), i );
    }
    return vNets;
}

static int Phys_DumpCompareNets( const void * p1, const void * p2 )
{
    float a = ((const Phys_DumpNet_t *)p1)->Hpwl;
    float b = ((const Phys_DumpNet_t *)p2)->Hpwl;
    return a < b ? 1 : a > b ? -1 : 0;
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
                "and one snapshot per tag: an SVG picture of the placement, plus CSV files "
                "with the cell geometries, the net bounding boxes, the bin density, and the "
                "row occupancy.</p>\n" );
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
            fprintf( f, "%s<a href=\"%s/%s.svg\">%s</a>",
                     k ? " &middot; " : "", pDesign, pTag, pTag );
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
    Vec_Ptr_t * vNets, * vSnaps, * vDesigns;
    float * pBinArea;
    float * pRowUsed;
    int * pRowCells;
    char * pDir, * pDesign, * pTag, * pDesignDir;
    char * pLog, * pSvg, * pCellsFile, * pNetsFile, * pDensity, * pRows, * pSnapsFile;
    char Color[16], TimeStr[32];
    FILE * f;
    float Dbu, X0, Y0, W, H, TotalArea = 0.0f, MaxRowOcc = 0.0f;
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
        if ( !Phys_DumpObjPos( pMan, pNet->Driver, Dbu, &fX, &fY ) )
            continue;
        pNet->X0 = pNet->X1 = fX;
        pNet->Y0 = pNet->Y1 = fY;
        Vec_IntForEachEntry( pNet->vSinks, x, i )
        {
            if ( !Phys_DumpObjPos( pMan, x, Dbu, &fX, &fY ) )
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
    /* density bins: square bins, never finer than a few rows, because a
       bin smaller than a cell only shows that the cell covers it */
    {
        float BinTarget = (W > H ? W : H) / (float)(pPars->nBins < 1 ? 1 : pPars->nBins);
        float MinBin = 4.0f * (float)pMan->SiteH / Dbu;
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
    /* the CSV files of this snapshot */
    pCellsFile   = Phys_DumpSnapFile( pDir, pDesign, pTag, "cells.csv" );
    pNetsFile    = Phys_DumpSnapFile( pDir, pDesign, pTag, "nets.csv" );
    pDensity     = Phys_DumpSnapFile( pDir, pDesign, pTag, "density.csv" );
    pRows        = Phys_DumpSnapFile( pDir, pDesign, pTag, "rows.csv" );
    pSvg         = Phys_DumpSnapFile( pDir, pDesign, pTag, "svg" );
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
    /* the picture */
    Scale  = 1100.0f / (W > H ? W : H);
    nDrawNets = pPars->nMaxNets < nNets ? pPars->nMaxNets : nNets;
    if ( (f = fopen( pSvg, "w" )) != NULL )
    {
        float MarginX = Margin, MarginY = Margin + 16.0f;
        int ImgW = (int)( W * Scale + 2 * MarginX + 0.5f );
        int ImgH = (int)( H * Scale + 2 * MarginY + 0.5f );
        fprintf( f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
        fprintf( f, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" "
                    "viewBox=\"0 0 %d %d\">\n", ImgW, ImgH, ImgW, ImgH );
        fprintf( f, "<rect width=\"100%%\" height=\"100%%\" fill=\"#ffffff\"/>\n" );
        fprintf( f, "<text x=\"8\" y=\"20\" font-family=\"sans-serif\" font-size=\"14\" fill=\"#222\">"
                    "%s &#8211; %s: %d cells, utilization %.2f, wirelength %.1f um, "
                    "longest net %.1f um</text>\n",
                    pDesign, pTag, nCells, TotalArea / (W * H), HpwlSum,
                    nNets ? pNets[0].Hpwl : 0.0f );
        fprintf( f, "<text x=\"8\" y=\"36\" font-family=\"sans-serif\" font-size=\"11\" fill=\"#666\">"
                    "background: cell density per bin (white empty, red full); "
                    "thin lines: %d longest nets</text>\n", nDrawNets );
        /* density */
        fprintf( f, "<g id=\"density\">\n" );
        for ( k = 0; k < nBinsY; k++ )
        for ( i = 0; i < nBinsX; i++ )
        {
            float Ratio = pBinArea[k * nBinsX + i] / (BinW * BinH);
            if ( Ratio < 0.01f )
                continue;
            Phys_DumpColor( Ratio, Color );
            fprintf( f, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" fill=\"%s\"/>\n",
                     MarginX + i * BinW * Scale, MarginY + (H - (k + 1) * BinH) * Scale,
                     BinW * Scale + 0.5f, BinH * Scale + 0.5f, Color );
        }
        fprintf( f, "</g>\n" );
        /* the row grid */
        fprintf( f, "<g id=\"rows\" stroke=\"#c8c8c8\" stroke-width=\"0.5\">\n" );
        for ( k = 0; k <= nRows; k++ )
            fprintf( f, "<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\"/>\n",
                     MarginX, MarginY + (H - k * (float)pMan->SiteH / Dbu) * Scale,
                     MarginX + W * Scale, MarginY + (H - k * (float)pMan->SiteH / Dbu) * Scale );
        fprintf( f, "</g>\n" );
        /* the longest nets */
        fprintf( f, "<g id=\"nets\" stroke=\"#1f4e79\" stroke-opacity=\"0.18\" stroke-width=\"1\">\n" );
        for ( i = 0; i < nDrawNets; i++ )
        {
            float Dx, Dy;
            if ( !Phys_DumpObjPos( pMan, pNets[i].Driver, Dbu, &Dx, &Dy ) )
                continue;
            Vec_IntForEachEntry( pNets[i].vSinks, x, k )
            {
                float Sx, Sy;
                if ( !Phys_DumpObjPos( pMan, x, Dbu, &Sx, &Sy ) )
                    continue;
                fprintf( f, "<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\"/>\n",
                         MarginX + (Dx - X0) * Scale, MarginY + (Y0 + H - Dy) * Scale,
                         MarginX + (Sx - X0) * Scale, MarginY + (Y0 + H - Sy) * Scale );
            }
        }
        fprintf( f, "</g>\n" );
        /* the cells */
        fprintf( f, "<g id=\"cells\" fill=\"#2f5597\" fill-opacity=\"0.75\">\n" );
        for ( i = 0; i < nCells; i++ )
        {
            float Cw = (float)pCells[i].W / Dbu * Scale;
            float Ch = (float)pCells[i].H / Dbu * Scale;
            if ( Cw < 0.6f ) Cw = 0.6f;
            if ( Ch < 0.6f ) Ch = 0.6f;
            fprintf( f, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\"/>\n",
                     MarginX + (pCells[i].X0 - X0) * Scale,
                     MarginY + (Y0 + H - pCells[i].Y0 - (float)pCells[i].H / Dbu) * Scale,
                     Cw, Ch );
        }
        fprintf( f, "</g>\n" );
        fprintf( f, "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" "
                    "fill=\"none\" stroke=\"#222\" stroke-width=\"1\"/>\n",
                 MarginX, MarginY, W * Scale, H * Scale );
        fprintf( f, "</svg>\n" );
        fclose( f );
    }
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
            fprintf( f, "# %-19s %-12s %-10s %6s %6s %10s %10s %9s %6s %5s %6s %10s\n",
                     "time", "tag", "design", "cells", "nets", "area_um2", "hpwl_um",
                     "longest_net", "rows", "used", "maxocc", "tmax_ps" );
        }
        fprintf( f, "  %-19s %-12s %-10s %6d %6d %10.1f %10.1f %9.1f %6d %5d %6.2f",
                 TimeStr, pTag, pDesign, nCells, nNets, TotalArea, HpwlSum,
                 nNets ? pNets[0].Hpwl : 0.0f, nRows, nRowsUsed, MaxRowOcc );
        if ( pMan->fTimed )
            fprintf( f, " %10.1f (period %.1f)\n", pMan->TMax, pMan->Period );
        else
            fprintf( f, " %10s\n", "-" );
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
        fprintf( stdout, "Dump: %s/%s: %d cells, %d nets, %d rows, max row occupancy %.2f, "
                 "wirelength %.1f um.\n", pDesign, pTag, nCells, nNets, nRows, MaxRowOcc, HpwlSum );
    fprintf( stdout, "Dump: wrote workspace \"%s/%s\" (%d cells, %d nets, %.1f um wirelength%s).\n",
             pDir, pDesign, nCells, nNets, HpwlSum, pMan->fTimed ? ", with timing" : "" );
    /* cleanup */
    Vec_PtrForEachEntry( Phys_DumpNet_t *, vNets, pNet, k )
        if ( pNet != NULL )
        {
            Vec_IntFree( pNet->vSinks );
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
    ABC_FREE( pRows ); ABC_FREE( pSvg ); ABC_FREE( pLog ); ABC_FREE( pSnapsFile );
    return 1;
}

ABC_NAMESPACE_IMPL_END
