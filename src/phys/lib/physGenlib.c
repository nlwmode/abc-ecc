/**Function*************************************************************

  Synopsis    [Genlib generation from Liberty data]

  Description [Produces the subset of genlib that the ABC standard-cell
               mapper needs.  Only combinational cells with one output
               and an explicit Boolean function are emitted.  Liberty
               functions using "&" and "|" are normalized to "*" and "+";
               functions containing other operators are skipped, since
               the genlib formula parser does not support them.]

  SideEffects []

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "phys/phys.h"
#include "phys/lib/physGenlib.h"

ABC_NAMESPACE_IMPL_START


static int Phys_GenlibIsNameChar( char c )
{
    return isalnum((unsigned char)c) || c == '_' || c == '[' || c == ']' ||
           c == '$' || c == '.';
}

static int Phys_GenlibIsOperator( char c )
{
    return c == '!' || c == '*' || c == '+' || c == '(' || c == ')';
}

static int Phys_GenlibIsInput( Phys_LibCell_t * pCell, const char * pName )
{
    Phys_LibPin_t * pPin;
    int i;
    if ( !strcmp( pName, "0" ) || !strcmp( pName, "1" ) )
        return 1;
    Vec_PtrForEachEntry( Phys_LibPin_t *, pCell->vPins, pPin, i )
        if ( pPin->Dir == 0 && !strcmp( pPin->pName, pName ) )
            return 1;
    return 0;
}

/* Convert a Liberty function into a genlib formula.  Returns NULL if the
   function contains unsupported operators or unknown identifiers. */
static char * Phys_GenlibFormula( Phys_LibCell_t * pCell, const char * pFunc, int * pfOk )
{
    Vec_Str_t * vStr = Vec_StrAlloc( 64 );
    char * pForm;
    const char * p = pFunc;
    while ( *p )
    {
        char c = *p++;
        if ( isspace((unsigned char)c) )
            continue;
        if ( c == '&' )
            c = '*';
        else if ( c == '|' )
            c = '+';
        if ( !Phys_GenlibIsNameChar(c) && !Phys_GenlibIsOperator(c) )
        {
            *pfOk = 0;
            Vec_StrFree( vStr );
            return NULL;
        }
        Vec_StrPush( vStr, c );
    }
    Vec_StrPush( vStr, '\0' );
    pForm = Vec_StrReleaseArray( vStr );
    Vec_StrFree( vStr );
    /* validate identifiers */
    p = pForm;
    while ( *p )
    {
        char Name[256];
        int n = 0;
        if ( !Phys_GenlibIsNameChar(*p) )
        {
            p++;
            continue;
        }
        while ( Phys_GenlibIsNameChar(*p) && n < (int)sizeof(Name) - 1 )
            Name[n++] = *p++;
        Name[n] = '\0';
        if ( !Phys_GenlibIsInput( pCell, Name ) )
        {
            *pfOk = 0;
            ABC_FREE( pForm );
            return NULL;
        }
    }
    *pfOk = 1;
    return pForm;
}

static const char * Phys_GenlibPhase( Phys_LibPin_t * pOut, Phys_LibPin_t * pIn )
{
    Phys_TimingArc_t * pArc = Phys_LibPinFindArc( pOut, pIn->pName );
    if ( pArc == NULL )
        return "UNKNOWN";
    if ( pArc->Sense > 0 )
        return "NONINV";
    if ( pArc->Sense < 0 )
        return "INV";
    return "UNKNOWN";
}

/* Representative value of a lookup-table axis (the middle index). */
static float Phys_GenlibMidIndex( Vec_Flt_t * vIndex )
{
    int n = Vec_FltSize( vIndex );
    if ( n == 0 )
        return 0.0f;
    return Vec_FltEntry( vIndex, n / 2 );
}

/* Delay increase per unit of output load, from the ends of the load axis. */
static float Phys_GenlibSlope( Phys_Surface_t * p, float Slew )
{
    int n = Vec_FltSize( p->vIndex1 );
    float L0, L1;
    if ( n < 2 )
        return 0.0f;
    L0 = Vec_FltEntry( p->vIndex1, 0 );
    L1 = Vec_FltEntry( p->vIndex1, n - 1 );
    if ( L1 <= L0 )
        return 0.0f;
    return ( Phys_SurfaceLookup(p, Slew, L1) - Phys_SurfaceLookup(p, Slew, L0) ) / (L1 - L0);
}

/* Genlib describes a gate by a constant intrinsic delay per input pin and
   a delay per unit of output load.  The Liberty NLDM tables are reduced to
   these two numbers at the middle point of each axis.  Delays are in ps,
   matching the internal units of the physical-synthesis layer. */
static void Phys_GenlibPinDelay( Phys_LibPin_t * pOut, Phys_LibPin_t * pIn,
                                 float * pDelayRise, float * pDelayFall,
                                 float * pFanoutRise, float * pFanoutFall )
{
    Phys_TimingArc_t * pArc = Phys_LibPinFindArc( pOut, pIn->pName );
    Phys_Surface_t * pRise = pArc ? pArc->pRise : NULL;
    Phys_Surface_t * pFall = pArc ? pArc->pFall : NULL;
    float Slew;
    *pDelayRise = *pDelayFall = 0.0f;
    *pFanoutRise = *pFanoutFall = 0.0f;
    if ( pRise != NULL && Vec_FltSize(pRise->vIndex0) > 0 )
    {
        Slew = Phys_GenlibMidIndex( pRise->vIndex0 );
        *pDelayRise  = Phys_SurfaceLookup( pRise, Slew, Phys_GenlibMidIndex(pRise->vIndex1) );
        *pFanoutRise = Phys_GenlibSlope( pRise, Slew );
    }
    if ( pFall != NULL && Vec_FltSize(pFall->vIndex0) > 0 )
    {
        Slew = Phys_GenlibMidIndex( pFall->vIndex0 );
        *pDelayFall  = Phys_SurfaceLookup( pFall, Slew, Phys_GenlibMidIndex(pFall->vIndex1) );
        *pFanoutFall = Phys_GenlibSlope( pFall, Slew );
    }
}

char * Phys_GenlibBuild( Phys_Lib_t * pLib, Phys_GenlibStats_t * pStats, int fVerbose )
{
    Vec_Str_t * vStr;
    char * pResult;
    Phys_GenlibStats_t Stats;
    Phys_LibCell_t * pCell;
    int i;

    memset( &Stats, 0, sizeof(Stats) );
    vStr = Vec_StrAlloc( 1 << 20 );
    Vec_StrPrintF( vStr, "# genlib generated by ABC physical synthesis from Liberty \"%s\"\n",
                   pLib->pName ? pLib->pName : "" );

    Vec_PtrForEachEntry( Phys_LibCell_t *, pLib->vCells, pCell, i )
    {
        Phys_LibPin_t * pOut = NULL, * pPin;
        char * pForm;
        int fOk, k;
        if ( pCell->fDontUse )
            continue;
        if ( pCell->fSequential )
        {
            Stats.nSeq++;
            continue;
        }
        if ( pCell->nOutputs != 1 )
        {
            Stats.nMultiOut++;
            continue;
        }
        Vec_PtrForEachEntry( Phys_LibPin_t *, pCell->vPins, pPin, k )
            if ( pPin->Dir == 1 )
            {
                pOut = pPin;
                break;
            }
        if ( pOut == NULL || pOut->pFunc == NULL || pOut->pFunc[0] == '\0' )
        {
            Stats.nNoFunc++;
            continue;
        }
        /* Constant cells are required by the mapper (Mio_CollectRootsNew2
           looks for const0/const1, buffer, and inverter).  The genlib
           reader compares the formula text verbatim, so it has to be
           written without spaces around '=' and ';'. */
        {
            const char * pFunc = pOut->pFunc;
            while ( *pFunc == ' ' || *pFunc == '\t' )
                pFunc++;
            if ( (pFunc[0] == '0' || pFunc[0] == '1') && pFunc[1] == '\0' )
            {
                Vec_StrPrintF( vStr, "GATE %s %.6f %s =CONST%c;\n",
                               pCell->pName, pCell->Area, pOut->pName, pFunc[0] );
                Stats.nGates++;
                continue;
            }
        }
        pForm = Phys_GenlibFormula( pCell, pOut->pFunc, &fOk );
        if ( pForm == NULL )
        {
            Stats.nSkipped++;
            if ( fVerbose )
                fprintf( stdout, "Genlib: skipping cell \"%s\" (function \"%s\").\n",
                         pCell->pName, pOut->pFunc );
            continue;
        }
        Vec_StrPrintF( vStr, "GATE %s %.6f %s = %s ;\n",
                       pCell->pName, pCell->Area, pOut->pName, pForm );
        Vec_PtrForEachEntry( Phys_LibPin_t *, pCell->vPins, pPin, k )
        {
            float DelayRise, DelayFall, FanoutRise, FanoutFall;
            if ( pPin->Dir != 0 )
                continue;
            Phys_GenlibPinDelay( pOut, pPin, &DelayRise, &DelayFall, &FanoutRise, &FanoutFall );
            Vec_StrPrintF( vStr, "PIN %s %s %.6f %.6f %.6f %.6f %.6f %.6f\n",
                           pPin->pName, Phys_GenlibPhase(pOut, pPin),
                           pPin->Cap, pPin->MaxCap,
                           DelayRise, FanoutRise, DelayFall, FanoutFall );
        }
        ABC_FREE( pForm );
        Stats.nGates++;
    }
    Vec_StrPrintStr( vStr, ".end\n" );
    Vec_StrPush( vStr, '\0' );
    if ( pStats != NULL )
        *pStats = Stats;
    if ( fVerbose )
        fprintf( stdout, "Genlib: %d gates emitted (%d sequential, %d multi-output, "
                 "%d without function, %d unsupported functions skipped).\n",
                 Stats.nGates, Stats.nSeq, Stats.nMultiOut, Stats.nNoFunc, Stats.nSkipped );
    pResult = Vec_StrReleaseArray( vStr );
    Vec_StrFree( vStr );
    return pResult;
}

ABC_NAMESPACE_IMPL_END
