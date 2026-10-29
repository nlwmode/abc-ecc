/**Function*************************************************************

  Synopsis    [Commands for physical synthesis]

  Description [Technology/library readers and the command shell for the
               physical-synthesis flow.  The heavy lifting is kept in the
               decoupled phys/lib and phys/place modules; this file only
               connects them to the ABC command interpreter.

               Commands implemented here so far:
                 read_lef       - read a technology LEF (-t) or a cell LEF
                 read_tech_lef  - read a technology LEF (same as "read_lef -t")
                 read_liberty   - read a Liberty library with the new parser
                 set_signal_res - unit wire resistance for RC extraction
                 set_signal_cap - unit wire capacitance for RC extraction
                 print_phys     - report the data currently loaded
               These readers are intentionally outside the
               &phys_start/&phys_end session (as required).]

  SideEffects []

***********************************************************************/

#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "misc/extra/extra.h"
#include "aig/gia/gia.h"
#include "map/mio/mio.h"
#include "phys/phys.h"
#include "phys/physMan.h"
#include "phys/lib/physGenlib.h"
#include "phys/place/physPlace.h"
#include "phys/place/physDump.h"
#include "phys/sta/physSta.h"
#include "phys/map/physMap.h"
#include "abcPhys.h"

ABC_NAMESPACE_IMPL_START


/**Function*************************************************************

  Synopsis    [read_lef command]

  Description [Reads a LEF file.  Without -t the file is expected to hold
               the MACRO definitions of the standard cells and the
               technology LEF has to be loaded already.  With -t the file
               is read as a technology LEF, which supplies the LEF units,
               the sites, the routing layers and the vias; it replaces any
               technology LEF loaded before.]

  SideEffects []

***********************************************************************/
int Abc_CommandReadLef( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    int c, fVerbose = 0, fTech = 0;
    Phys_LefTech_t * pLef;
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "tvh" )) != EOF )
    {
        switch ( c )
        {
        case 't':
            fTech ^= 1;
            break;
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if ( argc != globalUtilOptind + 1 )
        goto usage;
    if ( fTech )
    {
        if ( pAbc->pLef != NULL )
            Phys_LefTechFree( (Phys_LefTech_t **)&pAbc->pLef );
        pLef = Phys_LefTechAlloc();
        if ( !Phys_LefReadTech( pLef, argv[globalUtilOptind], 1 ) )
        {
            Phys_LefTechFree( &pLef );
            return 1;
        }
        Phys_LefFinish( pLef );
        pAbc->pLef = pLef;
        if ( fVerbose )
            Phys_LefTechPrint( pLef );
        return 0;
    }
    if ( pAbc->pLef == NULL )
    {
        Abc_Print( -1, "The technology LEF is not loaded. Run \"read_lef -t\" first.\n" );
        return 1;
    }
    pLef = (Phys_LefTech_t *)pAbc->pLef;
    if ( !Phys_LefReadMacros( pLef, argv[globalUtilOptind], 1 ) )
        return 1;
    Phys_LefFinish( pLef );
    if ( fVerbose )
    {
        Phys_LefMacro_t * pMacro;
        int i;
        Vec_PtrForEachEntry( Phys_LefMacro_t *, pLef->vMacros, pMacro, i )
            if ( i < 5 || i >= Vec_PtrSize(pLef->vMacros) - 5 )
                Phys_LefMacroPrint( pLef, pMacro );
    }
    return 0;

usage:
    Abc_Print( -2, "usage: read_lef [-t] [-vh] <file>\n" );
    Abc_Print( -2, "\t          reads a LEF file\n" );
    Abc_Print( -2, "\t-t      : read a technology LEF (units, sites, layers, vias)\n" );
    Abc_Print( -2, "\t          instead of a cell LEF (MACRO definitions)\n" );
    Abc_Print( -2, "\t-v      : print detailed information [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}


/**Function*************************************************************

  Synopsis    [read_tech_lef command]

  Description [Kept for compatibility: the same as "read_lef -t".]

  SideEffects []

***********************************************************************/
int Abc_CommandReadTechLef( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    int c, fVerbose = 0;
    Phys_LefTech_t * pLef;
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "vh" )) != EOF )
    {
        switch ( c )
        {
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if ( argc != globalUtilOptind + 1 )
        goto usage;
    if ( pAbc->pLef != NULL )
        Phys_LefTechFree( (Phys_LefTech_t **)&pAbc->pLef );
    pLef = Phys_LefTechAlloc();
    if ( !Phys_LefReadTech( pLef, argv[globalUtilOptind], 1 ) )
    {
        Phys_LefTechFree( &pLef );
        return 1;
    }
    Phys_LefFinish( pLef );
    pAbc->pLef = pLef;
    if ( fVerbose )
        Phys_LefTechPrint( pLef );
    return 0;

usage:
    Abc_Print( -2, "usage: read_tech_lef [-vh] <file>\n" );
    Abc_Print( -2, "\t          reads a technology LEF file (same as \"read_lef -t\")\n" );
    Abc_Print( -2, "\t-v      : print technology information [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}


/**Function*************************************************************

  Synopsis    [read_liberty command]

  Description []

  SideEffects []

***********************************************************************/
int Abc_CommandReadLiberty( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    int c, fVerbose = 0;
    Phys_Lib_t * pLib;
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "vh" )) != EOF )
    {
        switch ( c )
        {
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if ( argc != globalUtilOptind + 1 )
        goto usage;
    pLib = Phys_LibertyRead( argv[globalUtilOptind], 1 );
    if ( pLib == NULL )
        return 1;
    if ( pAbc->pPhysLib != NULL )
        Phys_LibFree( (Phys_Lib_t **)&pAbc->pPhysLib );
    pAbc->pPhysLib = pLib;
    if ( fVerbose )
        Phys_LibPrint( pLib );
    return 0;

usage:
    Abc_Print( -2, "usage: read_liberty [-vh] <file>\n" );
    Abc_Print( -2, "\t          reads a Liberty library with the physical-synthesis parser\n" );
    Abc_Print( -2, "\t-v      : print library information [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}


/**Function*************************************************************

  Synopsis    [set_signal_res command]

  Description [Sets the unit resistance of a signal wire.  The timing
               analysis multiplies it by the length of every edge of the
               RC tree that it extracts from the placed netlist.  Zero,
               the default, makes the analysis use the resistance of the
               routing layer read from the LEF.]

  SideEffects []

***********************************************************************/
int Abc_CommandSetSignalRes( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    float Res;
    int c, fVerbose = 0;
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "vh" )) != EOF )
    {
        switch ( c )
        {
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if ( argc == globalUtilOptind )
    {
        Abc_Print( 1, "Signal resistance: %.6f ohm/um%s.\n", Abc_FrameReadSignalRes(),
                   Abc_FrameReadSignalRes() > 0.0f ? "" : " (using the LEF routing layer)" );
        return 0;
    }
    if ( argc != globalUtilOptind + 1 )
        goto usage;
    Res = (float)atof( argv[globalUtilOptind] );
    if ( Res < 0.0f )
    {
        Abc_Print( -1, "The signal resistance cannot be negative.\n" );
        return 1;
    }
    Abc_FrameSetSignalRes( Res );
    if ( fVerbose )
        Abc_Print( 1, "Signal resistance: %.6f ohm/um.\n", Res );
    return 0;

usage:
    Abc_Print( -2, "usage: set_signal_res [-vh] [value]\n" );
    Abc_Print( -2, "\t          sets the unit resistance of a signal wire used to extract\n" );
    Abc_Print( -2, "\t          the RC of the placed nets\n" );
    Abc_Print( -2, "\tvalue   : resistance in ohm/um [default = LEF routing layer]\n" );
    Abc_Print( -2, "\t-v      : print the new value [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}


/**Function*************************************************************

  Synopsis    [set_signal_cap command]

  Description [Sets the unit capacitance of a signal wire.  The timing
               analysis multiplies it by the length of every edge of the
               RC tree that it extracts from the placed netlist.  Zero,
               the default, makes the analysis use the capacitance of the
               routing layer read from the LEF, or a typical value when
               the LEF does not define one.]

  SideEffects []

***********************************************************************/
int Abc_CommandSetSignalCap( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    float Cap;
    int c, fVerbose = 0;
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "vh" )) != EOF )
    {
        switch ( c )
        {
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if ( argc == globalUtilOptind )
    {
        Abc_Print( 1, "Signal capacitance: %.6f pF/um%s.\n", Abc_FrameReadSignalCap(),
                   Abc_FrameReadSignalCap() > 0.0f ? "" : " (using the LEF/default value)" );
        return 0;
    }
    if ( argc != globalUtilOptind + 1 )
        goto usage;
    Cap = (float)atof( argv[globalUtilOptind] );
    if ( Cap < 0.0f )
    {
        Abc_Print( -1, "The signal capacitance cannot be negative.\n" );
        return 1;
    }
    Abc_FrameSetSignalCap( Cap );
    if ( fVerbose )
        Abc_Print( 1, "Signal capacitance: %.6f pF/um.\n", Cap );
    return 0;

usage:
    Abc_Print( -2, "usage: set_signal_cap [-vh] [value]\n" );
    Abc_Print( -2, "\t          sets the unit capacitance of a signal wire used to extract\n" );
    Abc_Print( -2, "\t          the RC of the placed nets\n" );
    Abc_Print( -2, "\tvalue   : capacitance in pF/um [default = LEF routing layer]\n" );
    Abc_Print( -2, "\t-v      : print the new value [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}


/**Function*************************************************************

  Synopsis    [print_phys command]

  Description []

  SideEffects []

***********************************************************************/
int Abc_CommandPrintPhys( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    char * pCellName = NULL;
    char * pMacroName = NULL;
    int c, fVerbose = 0;
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "c:m:vh" )) != EOF )
    {
        switch ( c )
        {
        case 'c':
            if ( globalUtilOptarg == NULL )
                goto usage;
            pCellName = (char *)globalUtilOptarg;
            break;
        case 'm':
            if ( globalUtilOptarg == NULL )
                goto usage;
            pMacroName = (char *)globalUtilOptarg;
            break;
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if ( argc != globalUtilOptind )
        goto usage;
    if ( pAbc->pLef == NULL && pAbc->pPhysLib == NULL )
    {
        Abc_Print( 1, "No physical data is loaded.\n" );
        return 0;
    }
    if ( pAbc->pLef != NULL )
    {
        Phys_LefTech_t * pLef = (Phys_LefTech_t *)pAbc->pLef;
        Abc_Print( 1, "LEF: DBU=%d, grid=%.4f, %d layers, %d sites, %d macros, %d vias.\n",
                   pLef->Units.Dbu, pLef->ManufacturingGrid, Vec_PtrSize(pLef->vLayers),
                   Vec_PtrSize(pLef->vSites), Vec_PtrSize(pLef->vMacros), Vec_PtrSize(pLef->vVias) );
        if ( fVerbose )
            Phys_LefTechPrint( pLef );
        if ( pMacroName != NULL )
        {
            Phys_LefMacro_t * pMacro = Phys_LefFindMacro( pLef, pMacroName );
            if ( pMacro == NULL )
                Abc_Print( 0, "Cannot find LEF macro \"%s\".\n", pMacroName );
            else
                Phys_LefMacroPrint( pLef, pMacro );
        }
    }
    if ( pAbc->pPhysLib != NULL )
    {
        Phys_Lib_t * pLib = (Phys_Lib_t *)pAbc->pPhysLib;
        Abc_Print( 1, "Liberty: \"%s\", %d cells, %d templates, units time=%.1f ps cap=%.4f pF res=%.4f kOhm.\n",
                   pLib->pName ? pLib->pName : "", Vec_PtrSize(pLib->vCells),
                   Vec_PtrSize(pLib->vTemplates), pLib->Units.TimeScale,
                   pLib->Units.CapScale, pLib->Units.ResScale );
        if ( fVerbose )
            Phys_LibPrint( pLib );
        if ( pCellName != NULL )
        {
            Phys_LibCell_t * pCell = Phys_LibFindCell( pLib, pCellName );
            if ( pCell == NULL )
                Abc_Print( 0, "Cannot find Liberty cell \"%s\".\n", pCellName );
            else
                Phys_LibCellPrint( pLib, pCell );
        }
    }
    return 0;

usage:
    Abc_Print( -2, "usage: print_phys [-c cell] [-m macro] [-vh]\n" );
    Abc_Print( -2, "\t          reports the loaded LEF/Liberty data\n" );
    Abc_Print( -2, "\t-c cell : print information about the Liberty cell\n" );
    Abc_Print( -2, "\t-m macro: print information about the LEF macro\n" );
    Abc_Print( -2, "\t-v      : print detailed information [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}


/**Function*************************************************************

  Synopsis    [Derives genlib from Liberty and installs it as the mapping
               library of the frame]

  Description []

  SideEffects []

***********************************************************************/
static int Abc_PhysInstallGenlib( Abc_Frame_t * pAbc, int fVerbose )
{
    Phys_GenlibStats_t Stats;
    Mio_Library_t * pMio;
    char * pText, * pCopy;
    pText = Phys_GenlibBuild( (Phys_Lib_t *)pAbc->pPhysLib, &Stats, fVerbose );
    if ( pText == NULL )
        return 0;
    /* set PHYS_GENLIB_DUMP=<file> to dump the generated genlib for debugging */
    if ( getenv("PHYS_GENLIB_DUMP") )
    {
        FILE * pFile = fopen( getenv("PHYS_GENLIB_DUMP"), "w" );
        if ( pFile )
        {
            fputs( pText, pFile );
            fclose( pFile );
        }
    }
    if ( Stats.nGates == 0 )
    {
        Abc_Print( -1, "The Liberty library has no combinational cells usable by the mapper.\n" );
        ABC_FREE( pText );
        return 0;
    }
    /* Mio_LibraryRead modifies the buffer in place, so give it a private copy */
    pCopy = Abc_UtilStrsav( pText );
    pMio  = Mio_LibraryRead( "phys_genlib", pCopy, NULL, 0, fVerbose );
    ABC_FREE( pCopy );
    ABC_FREE( pText );
    if ( pMio == NULL )
    {
        Abc_Print( -1, "Installing the genlib derived from Liberty has failed.\n" );
        return 0;
    }
    Mio_UpdateGenlib( pMio );
    return 1;
}


/**Function*************************************************************

  Synopsis    [&phys_start command]

  Description [Opens a physical-synthesis session.  The loaded Liberty
               data is converted into genlib and installed as the
               standard-cell library, so that the existing mappers can
               be used unchanged.  The &phys_* commands are only valid
               while a session is open.

               A session on an unmapped AIG runs
                   &phys_start; &phys_bind; &phys_place; &phys_sta;
                   &phys_map; &phys_end
               &phys_bind puts a virtual cell on every AIG node, so
               &phys_place and &phys_sta give the unmapped network
               coordinates and a delay; &phys_map then maps it with the
               physical cut ranking.  To feed the result back, repeat
               &phys_place; &phys_sta; &phys_map inside the session.]

  SideEffects []

***********************************************************************/
int Abc_CommandPhysStart( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    int c, fVerbose = 0;
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "vh" )) != EOF )
    {
        switch ( c )
        {
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if ( argc != globalUtilOptind )
        goto usage;
    if ( pAbc->pPhysMan != NULL )
    {
        Abc_Print( -1, "A physical-synthesis session is already open. Run \"&phys_end\" first.\n" );
        return 1;
    }
    if ( pAbc->pPhysLib == NULL )
    {
        Abc_Print( -1, "The Liberty library is not loaded. Run \"read_liberty\" first.\n" );
        return 1;
    }
    if ( pAbc->pLef == NULL )
        Abc_Print( 1, "Warning: no LEF data is loaded. Run \"read_lef -t\" and \"read_lef\" "
                      "to enable placement and STA.\n" );
    if ( !Abc_PhysInstallGenlib( pAbc, fVerbose ) )
        return 1;
    pAbc->pPhysMan = Phys_ManStart( (Phys_LefTech_t *)pAbc->pLef,
                                    (Phys_Lib_t *)pAbc->pPhysLib, fVerbose );
    Abc_Print( 1, "Started physical-synthesis session: %d Liberty cells, %d genlib gates%s.\n",
               Vec_PtrSize( ((Phys_Lib_t *)pAbc->pPhysLib)->vCells ),
               Mio_LibraryReadGateNum( (Mio_Library_t *)Abc_FrameReadLibGen() ),
               pAbc->pLef ? ", LEF loaded" : "" );
    return 0;

usage:
    Abc_Print( -2, "usage: &phys_start [-vh]\n" );
    Abc_Print( -2, "\t          opens a physical-synthesis session\n" );
    Abc_Print( -2, "\t          (requires that \"read_liberty\" has been run)\n" );
    Abc_Print( -2, "\t          the flow is &phys_start; &phys_bind; &phys_place;\n" );
    Abc_Print( -2, "\t          &phys_sta; &phys_map; &phys_end\n" );
    Abc_Print( -2, "\t-v      : print detailed information [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}


/**Function*************************************************************

  Synopsis    [&phys_end command]

  Description [Closes the physical-synthesis session and releases all
               data owned by it.  The LEF/Liberty databases remain
               loaded in the frame.]

  SideEffects []

***********************************************************************/
int Abc_CommandPhysEnd( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    Phys_Man_t * pMan;
    int c, fVerbose = 0;
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "vh" )) != EOF )
    {
        switch ( c )
        {
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if ( argc != globalUtilOptind )
        goto usage;
    if ( pAbc->pPhysMan == NULL )
    {
        Abc_Print( -1, "There is no physical-synthesis session. Run \"&phys_start\" first.\n" );
        return 1;
    }
    pMan = (Phys_Man_t *)pAbc->pPhysMan;
    if ( fVerbose )
        Abc_Print( 1, "Closing the session with %d bound objects.\n", pMan->nObjs );
    Phys_ManStop( &pMan );
    pAbc->pPhysMan = NULL;
    if ( fVerbose )
        Abc_Print( 1, "Finished physical-synthesis session.\n" );
    return 0;

usage:
    Abc_Print( -2, "usage: &phys_end [-vh]\n" );
    Abc_Print( -2, "\t          closes the physical-synthesis session\n" );
    Abc_Print( -2, "\t-v      : print detailed information [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}


/**Function*************************************************************

  Synopsis    [Counts the usable inputs and outputs of a Liberty cell]

  Description [Returns 0 for a sequential or dont-use cell, which cannot
               model a combinational AIG node.  For the remaining cells
               the numbers of input and output pins are returned, together
               with a flag telling whether the last output has timing
               arcs, which the timing analysis needs to see a delay.]

  SideEffects []

***********************************************************************/
static int Abc_PhysCellPins( Phys_LibCell_t * pCell, int * pnIn, int * pnOut, int * pfArc )
{
    Phys_LibPin_t * pPin;
    int k, nIn = 0, nOut = 0, fArc = 0;
    *pnIn = *pnOut = *pfArc = 0;
    if ( pCell->fSequential || pCell->fDontUse )
        return 0;
    Vec_PtrForEachEntry( Phys_LibPin_t *, pCell->vPins, pPin, k )
    {
        if ( pPin->Dir == 0 )
            nIn++;
        else if ( pPin->Dir == 1 )
        {
            nOut++;
            fArc = pPin->vArcs != NULL && Vec_PtrSize(pPin->vArcs) > 0;
        }
    }
    *pnIn = nIn;
    *pnOut = nOut;
    *pfArc = fArc;
    return 1;
}


/**Function*************************************************************

  Synopsis    [Selects the cell that models an unmapped AIG node]

  Description [Before technology mapping every AIG node is bound to one
               copy of a small two-input combinational cell, so that the
               placer and the timing analysis have a geometry and a delay
               model for it.  The smallest cell with two inputs, one
               output and timing arcs wins; the choice only sets the
               relative positions of the AIG nodes and does not restrict
               the mapping.]

  SideEffects []

***********************************************************************/
static Phys_LibCell_t * Abc_PhysVirtualCell( Phys_Lib_t * pLib )
{
    Phys_LibCell_t * pBest = NULL, * pCell;
    int i, nIn, nOut, fArc, fBestArc = 0;
    if ( pLib == NULL )
        return NULL;
    Vec_PtrForEachEntry( Phys_LibCell_t *, pLib->vCells, pCell, i )
    {
        if ( !Abc_PhysCellPins( pCell, &nIn, &nOut, &fArc ) )
            continue;
        if ( nIn != 2 || nOut != 1 )
            continue;
        if ( pBest == NULL || (fArc && !fBestArc) ||
             (fArc == fBestArc && pCell->Area < pBest->Area) )
            pBest = pCell, fBestArc = fArc;
    }
    return pBest;
}


/**Function*************************************************************

  Synopsis    [Collects which literal phases the fanouts demand]

  Description [For every Gia object, vDemand[i] has bit 0 set when some
               fanout reads the positive literal of node i and bit 1 when
               some fanout reads the complemented literal.  The binder
               uses it to pick the cheaper of the two supergate phases.]

  SideEffects []

***********************************************************************/
static void Abc_PhysDemandMark( Vec_Int_t * vDemand, int iLit )
{
    int iObj = Abc_Lit2Var( iLit );
    if ( iObj <= 0 || iObj >= Vec_IntSize(vDemand) )
        return;
    Vec_IntWriteEntry( vDemand, iObj, Vec_IntEntry(vDemand, iObj) | (1 << Abc_LitIsCompl(iLit)) );
}

static void Abc_PhysDemandPhases( Gia_Man_t * pGia, Vec_Int_t * vDemand )
{
    Gia_Obj_t * pObj;
    int i;
    Gia_ManForEachObj( pGia, pObj, i )
    {
        if ( Gia_ObjIsAnd(pObj) )
        {
            Abc_PhysDemandMark( vDemand, Gia_ObjFaninLit0p(pGia, pObj) );
            Abc_PhysDemandMark( vDemand, Gia_ObjFaninLit1p(pGia, pObj) );
            if ( Gia_ObjIsMuxId(pGia, i) )
                Abc_PhysDemandMark( vDemand, Gia_ObjFaninLit2p(pGia, pObj) );
        }
        else if ( Gia_ObjIsCo(pObj) )
            Abc_PhysDemandMark( vDemand, Gia_ObjFaninLit0p(pGia, pObj) );
    }
}

/**Function*************************************************************

  Synopsis    [Picks the output phase of the supergate of an AIG node]

  Description [When only one literal phase is read, the supergate drives
               that phase and the pattern is realized without an extra
               inverter.  When both phases are read, the cheaper of the
               two supergates is used; the second phase is then read from
               the same driver, which is exact for placement and timing
               because inverting a net changes neither its geometry nor
               its arrival time.]

  SideEffects []

***********************************************************************/
static int Abc_PhysPickPhase( Phys_Lib_t * pLib, int p0, int p1, int Demand )
{
    Phys_LibCell_t * pPos = Phys_SuperCell( pLib, p0, p1, 0 );
    Phys_LibCell_t * pNeg = Phys_SuperCell( pLib, p0, p1, 1 );
    if ( Demand == 2 )
        return 1;
    if ( Demand != 3 || pPos == NULL || pNeg == NULL )
        return 0;
    return pNeg->Area < pPos->Area ? 1 : 0;
}


/**Function*************************************************************

  Synopsis    [Binds every AIG node to a virtual cell before mapping]

  Description [The mapping is not available yet, but &phys_place and
               &phys_sta need a netlist: every internal AIG node becomes
               one instance of the virtual cell, and the nets are the AIG
               nets.  The fanins of such an object are read from the AIG
               (see Phys_ManObjFaninNum); the cell literal stays negative,
               which is how the placer and the timing analysis tell a
               virtual instance from a mapped cell.]

  SideEffects []

***********************************************************************/
static int Abc_PhysBindVirtual( Abc_Frame_t * pAbc, const char * pCellName, int fVerbose )
{
    Phys_Man_t * pMan = (Phys_Man_t *)pAbc->pPhysMan;
    Gia_Man_t * pGia = pAbc->pGia;
    Phys_Lib_t * pLib = (Phys_Lib_t *)pMan->pLib;
    Phys_LibCell_t * pCell = NULL;
    Phys_LefMacro_t * pMacro = NULL;
    Vec_Int_t * vDemand = NULL;
    int nObjs = Gia_ManObjNum( pGia );
    int i, k, nBound = 0, nBuf = 0, nMux = 0, fSuper = 0;
    if ( pCellName != NULL )
    {
        int nIn, nOut, fArc;
        pCell = Phys_LibFindCell( (Phys_Lib_t *)pMan->pLib, pCellName );
        if ( pCell == NULL )
        {
            Abc_Print( -1, "Cannot find the Liberty cell \"%s\" to model the AIG nodes.\n", pCellName );
            return 1;
        }
        if ( pCell->fSequential || pCell->fDontUse )
        {
            Abc_Print( -1, "Cannot model the AIG nodes with \"%s\": the cell is %s.\n", pCell->pName,
                       pCell->fSequential ? "sequential" : "marked dont-use" );
            return 1;
        }
        Abc_PhysCellPins( pCell, &nIn, &nOut, &fArc );
        if ( nIn != 2 || nOut != 1 )
        {
            Abc_Print( -1, "Cannot model the AIG nodes with \"%s\": it has %d input(s) and %d output(s), "
                           "while the model needs two inputs and one output.\n", pCell->pName, nIn, nOut );
            return 1;
        }
    }
    else
    {
        int nSuper = Phys_SuperBuild( pLib, pMan->pLef );
        if ( nSuper == 8 )
            fSuper = 1;
        else
        {
            Abc_Print( 0, "Supergates: the library realizes %d of the 8 AIG patterns; "
                          "every node is modeled by one cell instead.\n", nSuper );
            pCell = Abc_PhysVirtualCell( pLib );
        }
    }
    if ( !fSuper && pCell == NULL )
    {
        Abc_Print( -1, "The Liberty library has no two-input combinational cell to model the AIG nodes.\n" );
        return 1;
    }
    Phys_ManResize( pMan, 2 * nObjs );
    Phys_ManLitResize( pMan, 2 * nObjs, 0 );
    if ( !fSuper && pMan->pLef != NULL )
    {
        pMacro = Phys_LefFindMacro( pMan->pLef, pCell->pName );
        if ( pMacro != NULL )
            pCell->MacroId = pMacro->Id;
    }
    if ( fSuper )
    {
        vDemand = Vec_IntStart( nObjs );
        Abc_PhysDemandPhases( pGia, vDemand );
    }
    for ( i = 1; i < nObjs; i++ )
    {
        Gia_Obj_t * pObj = Gia_ManObj( pGia, i );
        Phys_LibCell_t * pCellNode = pCell;
        Vec_Ptr_t * vPinsCopy;
        Phys_LibPin_t * pPin;
        int nPins = 0;
        if ( !Gia_ObjIsAnd(pObj) )
            continue;
        if ( Gia_ObjIsBuf(pObj) )
            nBuf++;
        else if ( Gia_ObjIsMuxId(pGia, i) )
            nMux++;
        if ( fSuper )
        {
            int p0 = Gia_ObjFaninC0(pObj);
            int p1 = Gia_ObjIsBuf(pObj) ? 0 : Gia_ObjFaninC1(pObj);
            int q  = Abc_PhysPickPhase( pLib, p0, p1, Vec_IntEntry(vDemand, i) );
            pCellNode = Phys_SuperCell( pLib, p0, p1, q );
            assert( pCellNode != NULL );
        }
        vPinsCopy = Vec_PtrAlloc( 2 );
        Vec_PtrForEachEntry( Phys_LibPin_t *, pCellNode->vPins, pPin, k )
            if ( pPin->Dir == 0 )
                Vec_PtrPush( vPinsCopy, pPin ), nPins++;
        assert( nPins == 2 );
        (void)nPins;
        Vec_PtrWriteEntry( pMan->vObjToCell, i, pCellNode );
        /* the virtual cell drives the positive literal of the AIG node;
           the pins are owned by the literal array */
        Vec_PtrWriteEntry( pMan->vLitToCell, 2 * i, pCellNode );
        Vec_PtrWriteEntry( pMan->vLitToPins, 2 * i, vPinsCopy );
        nBound++;
    }
    Vec_IntFreeP( &vDemand );
    if ( fVerbose && fSuper )
        Phys_SuperPrint( pLib, pMan->pLef );
    if ( fVerbose && pMacro != NULL )
        Abc_Print( 1, "Virtual cell \"%s\": LEF macro %d x %d DBU.\n",
                   pCell->pName, pMacro->Width, pMacro->Height );
    if ( fVerbose && nBuf )
        Abc_Print( 1, "Binding: %d buffers are modeled as one-input cells.\n", nBuf );
    if ( nMux )
        Abc_Print( 0, "Binding: the virtual two-input cell does not model the select input of "
                      "%d mux node(s); the placement sees all three fanins, the timing analysis "
                      "sees only two. Run \"&phys_map\" before relying on the mux timing.\n", nMux );
    if ( fSuper )
        Abc_Print( 1, "Bound %d AIG nodes to the eight supergate patterns of the library; "
                      "the network is mapped by \"&phys_map\".\n", nBound );
    else
        Abc_Print( 1, "Bound %d AIG nodes to the virtual cell \"%s\" (%.3f um^2); "
                      "the network is mapped by \"&phys_map\".\n", nBound, pCell->pName, pCell->Area );
    return 0;
}


/**Function*************************************************************

  Synopsis    [Binds the cells of the current Gia mapping to Liberty cells
               and LEF macros]

  Description [When the Gia network has a cell mapping produced by &nf
               (or by &phys_map), the genlib cell names are matched
               against Liberty cell names, the pins of each gate are
               matched by name against the Liberty pins in fanin order,
               and the LEF macro is looked up by the Liberty cell name.
               Without a mapping, every AIG node is bound to a virtual
               cell, so that the unmapped network can be placed and timed
               before &phys_map selects the gates.]

  SideEffects []

***********************************************************************/
static int Abc_PhysBind( Abc_Frame_t * pAbc, const char * pCellName, int fVerbose )
{
    Phys_Man_t * pMan = (Phys_Man_t *)pAbc->pPhysMan;
    Gia_Man_t * pGia = pAbc->pGia;
    Mio_Library_t * pMio;
    Mio_Cell2_t * pCells = NULL;
    Vec_Mem_t * vTtMem = NULL;
    Vec_Wec_t * vTt2Match = NULL;
    int nObjs = Gia_ManObjNum( pGia );
    int nCells = 0, i, nBound = 0, nInv = 0, nBuf = 0, nSkip = 0, nNoLib = 0, nNoMacro = 0;
    if ( pGia->vCellMapping == NULL )
        return Abc_PhysBindVirtual( pAbc, pCellName, fVerbose );
    if ( pCellName != NULL && fVerbose )
        Abc_Print( 1, "The network is mapped; the virtual-cell option \"%s\" is not used.\n", pCellName );
    pMio = (Mio_Library_t *)Abc_FrameReadLibGen();
    /* the mapping may have been produced with more than one gate per
       functionality; the cell ids then index that same table, so fetch it
       with the parameters stored next to the mapping */
    Mio_LibraryMatchesFetch( pMio, &vTtMem, &vTt2Match, &pCells, &nCells,
                             pGia->CellMatchPars[0], pGia->CellMatchPars[1],
                             pGia->CellMatchPars[2], pGia->CellMatchPars[3] );
    if ( pCells == NULL )
    {
        Abc_Print( -1, "The current genlib library has no matches.\n" );
        return 1;
    }
    Phys_ManResize( pMan, 2 * nObjs );
    Phys_ManLitResize( pMan, 2 * nObjs, 1 );
    /* Every non-zero entry of the cell mapping is one cell instance: a
       gate record (> 0), a buffer (-2) or an inverter (-1).  A Gia object
       can drive both of its literals (a gate plus an output inverter, or
       two gates for the two phases), so the instances are keyed by the
       output literal.  The object-level binding keeps one instance per
       object for the views that are still per object. */
    for ( i = 2; i < 2 * nObjs; i++ )
    {
        Phys_LibCell_t * pCell;
        Phys_LefMacro_t * pMacro;
        Vec_Ptr_t * vPins;
        Mio_Gate_t * pGate;
        int iCell, k, nFanins, nPins, Entry = Vec_IntEntry( pGia->vCellMapping, i );
        if ( Entry == 0 )
            continue;
        if ( Entry > 0 )
        {
            iCell = Gia_ObjCellId( pGia, i );
            if ( iCell < 0 || iCell >= nCells )
            {
                nSkip++;
                continue;
            }
            nFanins = Gia_ObjCellSize( pGia, i );
        }
        else if ( Entry == -1 || Entry == -2 )
        {
            /* the mapper adds the inverter and the buffer implicitly;
               slots 2 and 3 of the match table hold them */
            iCell = Entry == -1 ? 3 : 2;
            if ( iCell >= nCells )
            {
                nSkip++;
                continue;
            }
            nFanins = 1;
            if ( Entry == -1 ) nInv++;
            else               nBuf++;
        }
        else
        {
            nSkip++;
            continue;
        }
        pGate = (Mio_Gate_t *)pCells[iCell].pMioGate;
        pCell = Phys_LibFindCell( (Phys_Lib_t *)pMan->pLib, pCells[iCell].pName );
        if ( pCell == NULL )
        {
            nNoLib++;
            if ( fVerbose )
                Abc_Print( 0, "Cannot find Liberty cell \"%s\".\n", pCells[iCell].pName );
            continue;
        }
        nPins = Mio_GateReadPinNum( pGate );
        if ( nFanins > nPins )
        {
            Abc_Print( -1, "Internal error: cell \"%s\" has %d fanins but %d pins.\n",
                       pCell->pName, nFanins, nPins );
            return 1;
        }
        vPins = Vec_PtrAlloc( nFanins );
        for ( k = 0; k < nFanins; k++ )
            Vec_PtrPush( vPins, Phys_LibCellFindPin( pCell, Mio_GateReadPinName(pGate, k) ) );
        Vec_PtrWriteEntry( pMan->vLitToCell, i, pCell );
        Vec_PtrWriteEntry( pMan->vLitToPins, i, vPins );
        if ( pMan->pLef != NULL )
        {
            pMacro = Phys_LefFindMacro( pMan->pLef, pCell->pName );
            if ( pMacro == NULL )
                nNoMacro++;
            else
                pCell->MacroId = pMacro->Id;
        }
        nBound++;
    }
    for ( i = 1; i < nObjs; i++ )
    {
        int l0 = 2 * i, l1 = l0 + 1, lit = -1;
        if ( Vec_IntEntry( pGia->vCellMapping, l0 ) > 0 )
            lit = l0;
        else if ( Vec_IntEntry( pGia->vCellMapping, l1 ) > 0 )
            lit = l1;
        else if ( Vec_IntEntry( pGia->vCellMapping, l0 ) != 0 )
            lit = l0;
        else if ( Vec_IntEntry( pGia->vCellMapping, l1 ) != 0 )
            lit = l1;
        if ( lit < 0 || Vec_PtrEntry( pMan->vLitToCell, lit ) == NULL )
            continue;
        /* the pins stay owned by the literal arrays */
        Vec_PtrWriteEntry( pMan->vObjToCell, i, Vec_PtrEntry( pMan->vLitToCell, lit ) );
        Vec_IntWriteEntry( pMan->vObjCellLit, i, lit );
    }
    if ( fVerbose )
        Abc_Print( 1, "Binding: %d gate, %d inverter and %d buffer instances.\n",
                   nBound - nInv - nBuf, nInv, nBuf );
    Abc_Print( 1, "Bound %d cell instances to Liberty/LEF (%d not found in Liberty, %d bad cell ids, %d without LEF macro).\n",
               nBound, nNoLib, nSkip, nNoMacro );
    return 0;
}

int Abc_CommandPhysBind( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    char * pCellName = NULL;
    int c, fVerbose = 0;
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "c:vh" )) != EOF )
    {
        switch ( c )
        {
        case 'c':
            if ( globalUtilOptarg == NULL ) goto usage;
            pCellName = (char *)globalUtilOptarg;
            break;
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if ( argc != globalUtilOptind )
        goto usage;
    if ( pAbc->pPhysMan == NULL )
    {
        Abc_Print( -1, "There is no physical-synthesis session. Run \"&phys_start\" first.\n" );
        return 1;
    }
    if ( pAbc->pGia == NULL )
    {
        Abc_Print( -1, "There is no Gia network. Read one with \"&read\" first.\n" );
        return 1;
    }
    if ( Abc_FrameReadLibGen() == NULL )
    {
        Abc_Print( -1, "The genlib library is not installed. Run \"&phys_start\" first.\n" );
        return 1;
    }
    return Abc_PhysBind( pAbc, pCellName, fVerbose );

usage:
    Abc_Print( -2, "usage: &phys_bind [-c cell] [-vh]\n" );
    Abc_Print( -2, "\t          binds the Gia network to Liberty cells and LEF macros\n" );
    Abc_Print( -2, "\t          a mapped network binds its real cells; an unmapped AIG\n" );
    Abc_Print( -2, "\t          binds every node to a virtual cell, so that it can be\n" );
    Abc_Print( -2, "\t          placed and timed before \"&phys_map\"\n" );
    Abc_Print( -2, "\t-c cell : Liberty cell that models the AIG nodes before mapping\n" );
    Abc_Print( -2, "\t          [default = smallest two-input combinational cell]\n" );
    Abc_Print( -2, "\t-v      : print detailed information [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}


/**Function*************************************************************

  Synopsis    [&phys_place command]

  Description [Analytically places the bound Gia netlist: a mapped
               netlist after &nf/&phys_map, or the AIG nodes bound to
               virtual cells by &phys_bind before mapping.  A quadratic
               solver provides the starting point; the RePlAce/DREAMPlace
               nonlinear placement then minimizes the log-sum-exp
               wirelength plus an electrostatic density penalty, and the
               result is legalized into the rows.  The wirelength is taken
               between the LEF pin shapes of the cells: a net starts at
               the output pin of its driver and ends at the input pin of
               every sink, so the cell shapes and the pin positions of the
               LEF macro affect where the cells are placed, not just the
               cell centers.]

  SideEffects []

***********************************************************************/
int Abc_CommandPhysPlace( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    Phys_PlacePars_t Pars, Def;
    int c, fVerbose = 0;
    Phys_PlaceSetDefault( &Def );
    Pars = Def;
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "u:a:i:s:o:g:vh" )) != EOF )
    {
        switch ( c )
        {
        case 'u':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.Util = (float)atof( globalUtilOptarg );
            break;
        case 'a':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.Aspect = (float)atof( globalUtilOptarg );
            break;
        case 'i':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.nIterCG = atoi( globalUtilOptarg );
            break;
        case 's':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.nIterNest = atoi( globalUtilOptarg );
            break;
        case 'o':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.StopOverflow = (float)atof( globalUtilOptarg );
            break;
        case 'g':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.nCongBins = atoi( globalUtilOptarg );
            if ( Pars.nCongBins < 1 || Pars.nCongBins > 2048 )
            {
                Abc_Print( -1, "The number of congestion bins should be in [1, 2048].\n" );
                goto usage;
            }
            break;
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if ( argc != globalUtilOptind )
        goto usage;
    if ( pAbc->pPhysMan == NULL )
    {
        Abc_Print( -1, "There is no physical-synthesis session. Run \"&phys_start\" first.\n" );
        return 1;
    }
    if ( pAbc->pGia == NULL )
    {
        Abc_Print( -1, "There is no Gia network. Read one with \"&read\" first.\n" );
        return 1;
    }
    Pars.fVerbose = fVerbose;
    if ( !Phys_PlaceRun( (Phys_Man_t *)pAbc->pPhysMan, pAbc->pGia, &Pars ) )
        return 1;
    return 0;

usage:
    Abc_Print( -2, "usage: &phys_place [-u num] [-a num] [-i num] [-s num] [-o num] [-g num] [-vh]\n" );
    Abc_Print( -2, "\t          places the bound Gia netlist (virtual AIG cells before\n" );
    Abc_Print( -2, "\t          mapping, mapped standard cells afterwards)\n" );
    Abc_Print( -2, "\t-u num  : target core utilization, 0 < num <= 1 [default = %.2f]\n", Def.Util );
    Abc_Print( -2, "\t-a num  : core height / width [default = %.2f]\n", Def.Aspect );
    Abc_Print( -2, "\t-i num  : conjugate-gradient iterations [default = %d]\n", Def.nIterCG );
    Abc_Print( -2, "\t-s num  : nonlinear placement iterations [default = %d]\n", Def.nIterNest );
    Abc_Print( -2, "\t-o num  : density overflow at which to stop [default = %.2f]\n", Def.StopOverflow );
    Abc_Print( -2, "\t-g num  : RUDY bins along the longer core side [default = %d]\n", Def.nCongBins );
    Abc_Print( -2, "\t-v      : print detailed information [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}


/**Function*************************************************************

  Synopsis    [&phys_sta command]

  Description [Runs NLDM-based timing analysis with RC/Elmore wire
               delays derived from the current placement.]

  SideEffects []

***********************************************************************/
int Abc_CommandPhysSta( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    Phys_StaPars_t Pars, Def;
    int c, fVerbose = 0;
    Phys_StaSetDefault( &Def );
    Pars = Def;
    /* the unit RC set by "set_signal_res"/"set_signal_cap" is the default;
       -r and -c override it for this run only */
    Pars.WireRes = Abc_FrameReadSignalRes();
    Pars.WireCap = Abc_FrameReadSignalCap();
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "T:s:l:r:c:vh" )) != EOF )
    {
        switch ( c )
        {
        case 'T':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.Period = (float)atof( globalUtilOptarg );
            break;
        case 's':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.InputSlew = (float)atof( globalUtilOptarg );
            break;
        case 'l':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.pLayer = (char *)globalUtilOptarg;
            break;
        case 'r':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.WireRes = (float)atof( globalUtilOptarg );
            break;
        case 'c':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.WireCap = (float)atof( globalUtilOptarg );
            break;
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if ( argc != globalUtilOptind )
        goto usage;
    if ( pAbc->pPhysMan == NULL )
    {
        Abc_Print( -1, "There is no physical-synthesis session. Run \"&phys_start\" first.\n" );
        return 1;
    }
    if ( pAbc->pGia == NULL )
    {
        Abc_Print( -1, "There is no Gia network. Read one with \"&read\" first.\n" );
        return 1;
    }
    Pars.fVerbose = fVerbose;
    if ( !Phys_StaRun( (Phys_Man_t *)pAbc->pPhysMan, pAbc->pGia, &Pars ) )
        return 1;
    return 0;

usage:
    Abc_Print( -2, "usage: &phys_sta [-T num] [-s num] [-l layer] [-r num] [-c num] [-vh]\n" );
    Abc_Print( -2, "\t          reports timing with Elmore wire delays extracted from the placement\n" );
    Abc_Print( -2, "\t-T num  : clock period in ps [default = longest path]\n" );
    Abc_Print( -2, "\t-s num  : input slew in ps [default = %.2f]\n", Def.InputSlew );
    Abc_Print( -2, "\t-l layer: routing layer for RC, or \"auto\" [default = auto]\n" );
    Abc_Print( -2, "\t-r num  : unit wire resistance in ohm/um [default = set_signal_res, else LEF]\n" );
    Abc_Print( -2, "\t-c num  : unit wire capacitance in pF/um [default = set_signal_cap, else LEF]\n" );
    Abc_Print( -2, "\t-v      : print detailed information [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}


/**Function*************************************************************

  Synopsis    [&phys_dump command]

  Description [Writes a snapshot of the current placement into a debug
               workspace: a picture of the placement, the cell and net
               tables, the bin density, the row occupancy, and a record
               in the per-design log.  The picture draws the cell shapes
               from the LEF and anchors every net at the LEF pins: the
               wires start at the output pin of the driver and end at the
               input pin of each sink, and the pin metal shapes are drawn
               on top.  When "&phys_sta" has run, the critical path is
               traced and drawn in saturated magenta on top of the cells,
               pin to pin, and listed in critical_path.csv.  See
               phys/place/physDump.h for the layout of the workspace.]

  SideEffects [Creates directories and files]

***********************************************************************/
int Abc_CommandPhysDump( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    Phys_DumpPars_t Pars;
    int c, fVerbose = 0;
    Phys_DumpSetDefault( &Pars );
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "o:n:t:m:b:g:vh" )) != EOF )
    {
        switch ( c )
        {
        case 'o':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.pDir = (char *)globalUtilOptarg;
            break;
        case 'n':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.pDesign = (char *)globalUtilOptarg;
            break;
        case 't':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.pTag = (char *)globalUtilOptarg;
            break;
        case 'm':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.nMaxNets = atoi( globalUtilOptarg );
            break;
        case 'b':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.nBins = atoi( globalUtilOptarg );
            break;
        case 'g':
            if ( globalUtilOptarg == NULL ) goto usage;
            Pars.nCongBins = atoi( globalUtilOptarg );
            break;
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if ( argc != globalUtilOptind )
        goto usage;
    if ( pAbc->pPhysMan == NULL )
    {
        Abc_Print( -1, "There is no physical-synthesis session. Run \"&phys_start\" first.\n" );
        return 1;
    }
    if ( pAbc->pGia == NULL )
    {
        Abc_Print( -1, "There is no Gia network. Read one with \"&read\" first.\n" );
        return 1;
    }
    Pars.fVerbose = fVerbose;
    if ( !Phys_DumpRun( (Phys_Man_t *)pAbc->pPhysMan, pAbc->pGia, &Pars ) )
        return 1;
    return 0;

usage:
    Abc_Print( -2, "usage: &phys_dump [-o dir] [-n name] [-t tag] [-m num] [-b num] [-g num] [-vh]\n" );
    Abc_Print( -2, "\t          writes a snapshot of the placement into a debug workspace\n" );
    Abc_Print( -2, "\t          (the workspace and the design directory are created if missing)\n" );
    Abc_Print( -2, "\t-o dir  : workspace directory in the current directory [default = %s]\n", "phys_ws" );
    Abc_Print( -2, "\t-n name : design name, or the name of the network [default = %s]\n", "design" );
    Abc_Print( -2, "\t-t tag  : snapshot tag, giving the names of the files [default = %s]\n", "placement" );
    Abc_Print( -2, "\t-m num  : longest nets to draw in the picture, 0 for none [default = %d]\n", 2000 );
    Abc_Print( -2, "\t-b num  : density bins along the longer side [default = %d]\n", 32 );
    Abc_Print( -2, "\t-g num  : RUDY routability bins along the longer side [default = %d]\n", 64 );
    Abc_Print( -2, "\t-v      : print detailed information [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}


/**Function*************************************************************

  Synopsis    [&phys_map command]

  Description [Technology mapping with the cut ranking of "&nf" driven by
               the placement and the timing analysis.  The command is
               meant to be run after the unmapped AIG has been bound to
               virtual cells, placed and timed:
                   &phys_start; &phys_bind; &phys_place; &phys_sta;
                   &phys_map; &phys_end
               The mapper is "&nf" with one difference: cuts and matches
               are compared lexicographically by
                   critical-path timing, area, congestion, routability,
               without a weighted cost.  The timing key is the predicted
               arrival through the cut (STA arrival of its leaves plus the
               Elmore wire delay from &phys_sta and the gate delay) measured
               against the required time of the node, so it dominates on the
               critical path and yields to area where there is slack.  The
               area is the true area flow, the congestion is the routing
               overflow demand of the cut wires from the RUDY map of
               &phys_place, and the routability is the worst local
               supply/demand ratio touched by the cut.  The physical keys
               only steer the comparison; the area and arrival propagated
               to the fanouts stay true, so the mapper does not build up a
               fictitious cost over the levels of logic.
               With "-l" the command is exactly "&nf" (no physical data,
               no placement needed).]

  SideEffects []

***********************************************************************/
int Abc_CommandPhysMap( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    extern void Nf_ManSetDefaultPars( Jf_Par_t * pPars );
    extern Gia_Man_t * Nf_ManPerformMapping( Gia_Man_t * pGia, Jf_Par_t * pPars );
    Phys_Man_t * pMan = (Phys_Man_t *)pAbc->pPhysMan;
    Jf_Par_t Pars, * pPars = &Pars;
    Jf_Phys_t Phys;
    Gia_Man_t * pNew, * pGia;
    int c, fVerbose = 0, nObjs, nRelax = -1;
    int fLegacy = 0, fRootOnly = 0, fCong = 1;
    Nf_ManSetDefaultPars( pPars );
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "K:C:F:E:D:R:lsdvh" )) != EOF )
    {
        switch ( c )
        {
        case 'K':
            if ( globalUtilOptarg == NULL ) goto usage;
            pPars->nLutSize = atoi( globalUtilOptarg );
            break;
        case 'C':
            if ( globalUtilOptarg == NULL ) goto usage;
            pPars->nCutNum = atoi( globalUtilOptarg );
            break;
        case 'F':
            if ( globalUtilOptarg == NULL ) goto usage;
            pPars->nRounds = atoi( globalUtilOptarg );
            if ( pPars->nRounds < 0 )
            {
                Abc_Print( -1, "The number of area-flow rounds should be non-negative.\n" );
                goto usage;
            }
            break;
        case 'E':
            if ( globalUtilOptarg == NULL ) goto usage;
            pPars->nAreaTuner = atoi( globalUtilOptarg );
            if ( pPars->nAreaTuner < 0 || pPars->nAreaTuner > 100 )
            {
                Abc_Print( -1, "The area/edge tradeoff parameter %d is not in [0, 100].\n", pPars->nAreaTuner );
                goto usage;
            }
            break;
        case 'D':
            if ( globalUtilOptarg == NULL ) goto usage;
            pPars->DelayTarget = atoi( globalUtilOptarg );
            if ( pPars->DelayTarget <= 0 )
            {
                Abc_Print( -1, "The delay target should be a positive integer.\n" );
                goto usage;
            }
            break;
        case 'R':
            if ( globalUtilOptarg == NULL ) goto usage;
            nRelax = atoi( globalUtilOptarg );
            if ( nRelax < 0 || nRelax > 100 )
            {
                Abc_Print( -1, "The delay relaxation %d%% is not in [0, 100].\n", nRelax );
                goto usage;
            }
            break;
        case 'l':
            fLegacy = 1;
            break;
        case 's':
            fRootOnly = 1;
            break;
        case 'd':
            fCong = 0;
            break;
        case 'v':
            fVerbose ^= 1;
            break;
        case 'w':
            pPars->fVeryVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if ( argc != globalUtilOptind )
        goto usage;
    if ( pAbc->pGia == NULL )
    {
        Abc_Print( -1, "There is no Gia network. Read one with \"&read\" first.\n" );
        return 1;
    }
    if ( Abc_FrameReadLibGen() == NULL )
    {
        Abc_Print( -1, "The genlib library is not installed. Run \"&phys_start\" first.\n" );
        return 1;
    }
    if ( !fLegacy && pMan == NULL )
    {
        Abc_Print( -1, "There is no physical-synthesis session. Run \"&phys_start\" first,\n" );
        Abc_Print( -1, "or use \"&phys_map -l\" for the legacy &nf mapping.\n" );
        return 1;
    }
    nObjs = Gia_ManObjNum( pAbc->pGia );
    if ( !fLegacy )
    {
        if ( !pMan->fPlaced )
        {
            Abc_Print( -1, "The design is not placed. Run \"&phys_bind\" and \"&phys_place\" first,\n" );
            Abc_Print( -1, "or use \"&phys_map -l\" for the legacy &nf mapping.\n" );
            return 1;
        }
        if ( 2 * nObjs > Vec_IntSize(pMan->vObjCellLit) )
        {
            Abc_Print( -1, "The session data does not match the current Gia network. Run \"&phys_bind\" first.\n" );
            return 1;
        }
        if ( !pMan->fTimed && fVerbose )
            Abc_Print( 1, "Warning: there is no timing data, so the timing key falls back to\n"
                          "the arrival of the mapper, and the required times cannot\n"
                          "separate the candidates of the area track.\n" );
    }
    if ( pPars->nLutSize < 2 || pPars->nLutSize > pPars->nLutSizeMax )
    {
        Abc_Print( -1, "LUT size %d is not supported.\n", pPars->nLutSize );
        goto usage;
    }
    if ( pPars->nCutNum < 1 || pPars->nCutNum > pPars->nCutNumMax )
    {
        Abc_Print( -1, "The number of cuts %d is not supported.\n", pPars->nCutNum );
        goto usage;
    }
    if ( nObjs < 2 )
    {
        Abc_Print( -1, "The Gia network has no objects to map.\n" );
        return 1;
    }
    if ( fLegacy )
    {
        pPars->pPhys = NULL;
        pPars->fAllGates = 0;
        pPars->nRelaxRatio = 0;  /* "-l" is bit-for-bit "&nf": no relaxation */
    }
    else
    {
        if ( !Phys_MapBuildPhys( pMan, pAbc->pGia, fCong, &Phys ) )
        {
            Abc_Print( -1, "The physical data of the session is not available.\n" );
            return 1;
        }
        pPars->pPhys     = &Phys;
        pPars->fAllGates = !fRootOnly;  /* keep the drive strengths for physical mapping */
        pPars->fPinFilter = pPars->fAllGates ? 1 : 0;  /* prune dominated drives */
        pPars->fCoarsen  = 0;  /* coarsening renumbers the objects */
        /* With all gates, the delay-optimal mapping uses the fastest drive
           everywhere and the area recovery cannot undo it without a delay
           bound of its own.  "-R" relaxes that bound so the recovery has
           room; zero keeps the plain "&nf" behaviour. */
        pPars->nRelaxRatio = nRelax < 0 ? 0 : nRelax;
    }
    pPars->fVerbose = fVerbose;
    if ( fVerbose )
    {
        if ( fLegacy )
            Abc_Print( 1, "Legacy mapping: %d objects (same as &nf).\n", nObjs );
        else
            Abc_Print( 1, "Physical mapping: %d objects, ranking timing > area > congestion > routability, delay relaxation %d%%, congestion %s.\n",
                       nObjs, pPars->nRelaxRatio, fCong ? "on" : "off" );
    }
    pGia = pAbc->pGia;
    pNew = Nf_ManPerformMapping( pGia, pPars );
    if ( !fLegacy )
        Phys_MapFreePhys( &Phys );
    if ( pNew == NULL )
    {
        Abc_Print( -1, "Physical mapping has failed.\n" );
        return 1;
    }
    /* With timing boxes the mapper renumbers the network before mapping, so
       the per-object placement and timing data, which is indexed by the
       original ids, does not describe the mapped objects. */
    if ( !fLegacy && pNew != pGia )
        Abc_Print( 0, "Mapping: the network was renumbered for timing boxes, so the placement "
                      "and timing data may not match the mapped objects.\n" );
    Abc_FrameUpdateGia( pAbc, pNew );
    /* the mapping changed, so the old binding/placement/timing are gone */
    if ( pMan == NULL )
    {
        /* "-l" without a session is just "&nf": there is no physical
           binding to refresh, and Abc_PhysBind needs a session */
        if ( fVerbose )
            Abc_Print( 1, "No physical-synthesis session; the mapped network is left as is.\n" );
        return 0;
    }
    return Abc_PhysBind( pAbc, NULL, fVerbose );

usage:
    Abc_Print( -2, "usage: &phys_map [-K num] [-C num] [-F num] [-E num] [-D num] [-R num]\n" );
    Abc_Print( -2, "                [-l] [-s] [-d] [-vwh]\n" );
    Abc_Print( -2, "\t          physical-aware technology mapping (enhanced \"&nf\")\n" );
    Abc_Print( -2, "\t          maps the placed AIG; see &phys_bind/&phys_place/&phys_sta\n" );
    Abc_Print( -2, "\t          the cuts are ranked lexicographically by critical-path timing,\n" );
    Abc_Print( -2, "\t          area, congestion and routability, without any cost weights\n" );
    Abc_Print( -2, "\t-K num  : LUT size for the mapping (2 <= K <= %d) [default = %d]\n", pPars->nLutSizeMax, pPars->nLutSize );
    Abc_Print( -2, "\t-C num  : the max number of priority cuts (1 <= C <= %d) [default = %d]\n", pPars->nCutNumMax, pPars->nCutNum );
    Abc_Print( -2, "\t-F num  : the number of area flow rounds [default = %d]\n", pPars->nRounds );
    Abc_Print( -2, "\t-E num  : the area/edge tradeoff parameter (0 <= num <= 100) [default = %d]\n", pPars->nAreaTuner );
    Abc_Print( -2, "\t-D num  : sets the delay constraint for the mapping [default = best possible]\n" );
    Abc_Print( -2, "\t-R num  : relax the delay bound of the mapping by num percent, which\n" );
    Abc_Print( -2, "\t          gives the area recovery room to undo the fastest drives [default = %d]\n", 0 );
    Abc_Print( -2, "\t-l      : use the legacy &nf algorithm (no physical data)\n" );
    Abc_Print( -2, "\t-s      : use only the smallest root gate of each function\n" );
    Abc_Print( -2, "\t-d      : disable the congestion and routability keys\n" );
    Abc_Print( -2, "\t-v      : toggles verbose output [default = no]\n" );
    Abc_Print( -2, "\t-w      : toggles very verbose output [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}

ABC_NAMESPACE_IMPL_END

