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
               while a session is open.]

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

  Synopsis    [Binds the cells of the current Gia mapping to Liberty cells
               and LEF macros]

  Description [The Gia network must have a cell mapping produced by
               &nf (or by &phys_map).  The genlib cell names are matched
               against Liberty cell names, the pins of each gate are
               matched by name against the Liberty pins in fanin order,
               and the LEF macro is looked up by the Liberty cell name.]

  SideEffects []

***********************************************************************/
static int Abc_PhysBind( Abc_Frame_t * pAbc, int fVerbose )
{
    Phys_Man_t * pMan = (Phys_Man_t *)pAbc->pPhysMan;
    Gia_Man_t * pGia = pAbc->pGia;
    Mio_Library_t * pMio;
    Mio_Cell2_t * pCells = NULL;
    Vec_Mem_t * vTtMem = NULL;
    Vec_Wec_t * vTt2Match = NULL;
    int nObjs = Gia_ManObjNum( pGia );
    int nCells = 0, i, c, nBound = 0, nDup = 0, nSkip = 0, nNoLib = 0, nNoMacro = 0;
    pMio = (Mio_Library_t *)Abc_FrameReadLibGen();
    Mio_LibraryMatchesFetch( pMio, &vTtMem, &vTt2Match, &pCells, &nCells, 0, 0, 0 );
    if ( pCells == NULL )
    {
        Abc_Print( -1, "The current genlib library has no matches.\n" );
        return 1;
    }
    Phys_ManResize( pMan, 2 * nObjs );
    /* a node can be mapped in both polarities (two cells at the same Gia
       object); the per-object model keeps the first one, preferring the
       non-inverted polarity, and reports the other ones */
    for ( c = 0; c < 2; c++ )
    for ( i = 2 + c; i < 2 * nObjs; i += 2 )
    {
        Phys_LibCell_t * pCell;
        Phys_LefMacro_t * pMacro;
        Vec_Ptr_t * vPins;
        Mio_Gate_t * pGate;
        int iObj, iCell, k, nFanins, nPins;
        if ( Vec_IntEntry( pGia->vCellMapping, i ) <= 0 )
            continue;
        iObj = Abc_Lit2Var( i );
        if ( Vec_PtrEntry( pMan->vObjToCell, iObj ) != NULL )
        {
            nDup++;
            continue;
        }
        iCell = Gia_ObjCellId( pGia, i );
        if ( iCell < 0 || iCell >= nCells )
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
        nFanins = Gia_ObjCellSize( pGia, i );
        nPins   = Mio_GateReadPinNum( pGate );
        if ( nFanins > nPins )
        {
            Abc_Print( -1, "Internal error: cell \"%s\" has %d fanins but %d pins.\n",
                       pCell->pName, nFanins, nPins );
            return 1;
        }
        vPins = Vec_PtrAlloc( nFanins );
        for ( k = 0; k < nFanins; k++ )
            Vec_PtrPush( vPins, Phys_LibCellFindPin( pCell, Mio_GateReadPinName(pGate, k) ) );
        Vec_PtrWriteEntry( pMan->vObjToCell, iObj, pCell );
        Vec_PtrWriteEntry( pMan->vObjToPins, iObj, vPins );
        Vec_IntWriteEntry( pMan->vObjCellLit, iObj, i );
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
    if ( fVerbose && nDup )
        Abc_Print( 1, "Binding: %d additional cells share a node with another cell and are not modeled yet.\n", nDup );
    Abc_Print( 1, "Bound %d cells to Liberty/LEF (%d not found in Liberty, %d bad cell ids, %d without LEF macro).\n",
               nBound, nNoLib, nSkip, nNoMacro );
    return 0;
}

int Abc_CommandPhysBind( Abc_Frame_t * pAbc, int argc, char ** argv )
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
    if ( pAbc->pGia->vCellMapping == NULL )
    {
        Abc_Print( -1, "The Gia network has no cell mapping. Run \"&nf\" (or \"&phys_map\") first.\n" );
        return 1;
    }
    if ( Abc_FrameReadLibGen() == NULL )
    {
        Abc_Print( -1, "The genlib library is not installed. Run \"&phys_start\" first.\n" );
        return 1;
    }
    return Abc_PhysBind( pAbc, fVerbose );

usage:
    Abc_Print( -2, "usage: &phys_bind [-vh]\n" );
    Abc_Print( -2, "\t          binds the mapped Gia cells to Liberty cells and LEF macros\n" );
    Abc_Print( -2, "\t          (the Gia network must be mapped by \"&nf\" first)\n" );
    Abc_Print( -2, "\t-v      : print detailed information [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}


/**Function*************************************************************

  Synopsis    [&phys_place command]

  Description [Analytically places the mapped and bound Gia netlist.
               The placer is a simplified DreamPlace-style quadratic
               solver followed by density spreading, row legalization,
               and wirelength refinement.]

  SideEffects []

***********************************************************************/
int Abc_CommandPhysPlace( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    Phys_PlacePars_t Pars;
    int c, fVerbose = 0;
    Phys_PlaceSetDefault( &Pars );
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "u:a:i:s:vh" )) != EOF )
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
            Pars.nIterRefine = atoi( globalUtilOptarg );
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
    Abc_Print( -2, "usage: &phys_place [-u num] [-a num] [-i num] [-s num] [-vh]\n" );
    Abc_Print( -2, "\t          places the mapped and bound Gia netlist\n" );
    Abc_Print( -2, "\t-u num  : target core utilization [default = %.2f]\n", 0.60 );
    Abc_Print( -2, "\t-a num  : core height / width [default = %.2f]\n", 1.00 );
    Abc_Print( -2, "\t-i num  : conjugate-gradient iterations [default = %d]\n", 300 );
    Abc_Print( -2, "\t-s num  : wirelength-refinement rounds, 0 to skip [default = %d]\n", 20 );
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
    Phys_StaPars_t Pars;
    int c, fVerbose = 0;
    Phys_StaSetDefault( &Pars );
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
    Abc_Print( -2, "\t-s num  : input slew in ps [default = %.2f]\n", 10.0 );
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
               in the per-design log.  See phys/place/physDump.h for the
               layout of the workspace.]

  SideEffects [Creates directories and files]

***********************************************************************/
int Abc_CommandPhysDump( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    Phys_DumpPars_t Pars;
    int c, fVerbose = 0;
    Phys_DumpSetDefault( &Pars );
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "o:n:t:m:b:vh" )) != EOF )
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
    Abc_Print( -2, "usage: &phys_dump [-o dir] [-n name] [-t tag] [-m num] [-b num] [-vh]\n" );
    Abc_Print( -2, "\t          writes a snapshot of the placement into a debug workspace\n" );
    Abc_Print( -2, "\t          (the workspace and the design directory are created if missing)\n" );
    Abc_Print( -2, "\t-o dir  : workspace directory in the current directory [default = %s]\n", "phys_ws" );
    Abc_Print( -2, "\t-n name : design name, or the name of the network [default = %s]\n", "design" );
    Abc_Print( -2, "\t-t tag  : snapshot tag, giving the names of the files [default = %s]\n", "placement" );
    Abc_Print( -2, "\t-m num  : longest nets to draw in the picture, 0 for none [default = %d]\n", 2000 );
    Abc_Print( -2, "\t-b num  : density bins along the longer side [default = %d]\n", 32 );
    Abc_Print( -2, "\t-v      : print detailed information [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}


/**Function*************************************************************

  Synopsis    [Derives a coordinate for every Gia object]

  Description [Bound cells, primary inputs and primary outputs take the
               coordinates computed by the placement.  The internal AIG
               nodes, which do not have a position of their own, inherit
               the average of their fanins, so that every object used as
               a cut leaf has a finite coordinate.]

  SideEffects []

***********************************************************************/
static void Abc_PhysMapCoords( Phys_Man_t * pMan, Gia_Man_t * pGia, float * pX, float * pY )
{
    int nObjs = Gia_ManObjNum( pGia );
    float Dbu = (float)(pMan->pLef ? pMan->pLef->Units.Dbu : 1000);
    int i, k, nFans;
    for ( i = 0; i < nObjs; i++ )
        pX[i] = pY[i] = -1.0f;
    for ( i = 1; i < nObjs; i++ )
    {
        Gia_Obj_t * pObj = Gia_ManObj( pGia, i );
        if ( pMan->fPlaced && i < Vec_IntSize(pMan->vObjX) &&
             Vec_IntEntry(pMan->vObjX, i) >= 0 )
        {
            pX[i] = (float)Vec_IntEntry( pMan->vObjX, i ) / Dbu;
            pY[i] = (float)Vec_IntEntry( pMan->vObjY, i ) / Dbu;
            continue;
        }
        if ( Gia_ObjIsPi(pGia, pObj) )
        {
            pX[i] = pY[i] = 0.0f;
            continue;
        }
        if ( !Gia_ObjIsAnd(pObj) )
            continue;
        nFans = Gia_ObjIsMuxId( pGia, i ) ? 3 : 2;
        {
            float SumX = 0.0f, SumY = 0.0f;
            int nKnown = 0;
            for ( k = 0; k < nFans; k++ )
            {
                int d = k == 0 ? Gia_ObjFaninId0(pObj, i) :
                        k == 1 ? Gia_ObjFaninId1(pObj, i) : Gia_ObjFaninId2(pGia, i);
                if ( d > 0 && pX[d] >= 0.0f )
                    SumX += pX[d], SumY += pY[d], nKnown++;
            }
            if ( nKnown > 0 )
                pX[i] = SumX / nKnown, pY[i] = SumY / nKnown;
        }
    }
    for ( i = 0; i < nObjs; i++ )
        if ( pX[i] < 0.0f )
            pX[i] = pY[i] = 0.0f;
}


/**Function*************************************************************

  Synopsis    [&phys_map command]

  Description [Performs technology mapping like "&nf" but with the
               placement available from a previous &phys_bind/&phys_place
               iteration.  The physical spread of the leaves of each cut
               is added to its area flow and delay, so the cut ranking
               favors cuts whose fanins are placed close to each other.
               The resulting network is automatically bound.]

  SideEffects []

***********************************************************************/
int Abc_CommandPhysMap( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    extern void Nf_ManSetDefaultPars( Jf_Par_t * pPars );
    extern Gia_Man_t * Nf_ManPerformMapping( Gia_Man_t * pGia, Jf_Par_t * pPars );
    Phys_Man_t * pMan;
    Jf_Par_t Pars, * pPars = &Pars;
    Gia_Man_t * pNew;
    float * pX = NULL, * pY = NULL;
    float FlowW = 0.0f, DelayW = 0.05f;
    int c, fVerbose = 0, nObjs;
    Nf_ManSetDefaultPars( pPars );
    Extra_UtilGetoptReset();
    while ( (c = Extra_UtilGetopt( argc, argv, "K:C:F:E:D:x:y:vwh" )) != EOF )
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
            break;
        case 'E':
            if ( globalUtilOptarg == NULL ) goto usage;
            pPars->nAreaTuner = atoi( globalUtilOptarg );
            break;
        case 'D':
            if ( globalUtilOptarg == NULL ) goto usage;
            pPars->DelayTarget = atoi( globalUtilOptarg );
            break;
        case 'x':
            if ( globalUtilOptarg == NULL ) goto usage;
            FlowW = (float)atof( globalUtilOptarg );
            break;
        case 'y':
            if ( globalUtilOptarg == NULL ) goto usage;
            DelayW = (float)atof( globalUtilOptarg );
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
    pMan = (Phys_Man_t *)pAbc->pPhysMan;
    if ( !pMan->fPlaced )
    {
        Abc_Print( -1, "The design is not placed. Run \"&phys_bind\" and \"&phys_place\" first.\n" );
        return 1;
    }
    nObjs = Gia_ManObjNum( pAbc->pGia );
    if ( 2 * nObjs > Vec_IntSize(pMan->vObjCellLit) )
    {
        Abc_Print( -1, "The session data does not match the current Gia network. Run \"&phys_bind\" first.\n" );
        return 1;
    }
    if ( pPars->nLutSize < 2 || pPars->nLutSize > pPars->nLutSizeMax ||
         pPars->nCutNum < 1 || pPars->nCutNum > pPars->nCutNumMax )
        goto usage;
    pX = ABC_ALLOC( float, nObjs );
    pY = ABC_ALLOC( float, nObjs );
    Abc_PhysMapCoords( pMan, pAbc->pGia, pX, pY );
    if ( fVerbose )
    {
        float xMin = pX[1], xMax = pX[1], yMin = pY[1], yMax = pY[1];
        int k;
        for ( k = 2; k < nObjs; k++ )
        {
            xMin = Abc_MinFloat( xMin, pX[k] );  xMax = Abc_MaxFloat( xMax, pX[k] );
            yMin = Abc_MinFloat( yMin, pY[k] );  yMax = Abc_MaxFloat( yMax, pY[k] );
        }
        Abc_Print( 1, "Physical coordinates: x [%.2f, %.2f], y [%.2f, %.2f] um.\n",
                   xMin, xMax, yMin, yMax );
    }
    pPars->pPhysX     = pX;
    pPars->pPhysY     = pY;
    pPars->PhysFlowW  = FlowW;
    pPars->PhysDelayW = DelayW;
    pPars->fCoarsen   = 0;   /* coarsening renumbers the objects */
    pPars->fVerbose   = fVerbose;
    if ( fVerbose )
        Abc_Print( 1, "Physical mapping: %d objects, flow weight %.3f, delay weight %.3f.\n",
                   nObjs, FlowW, DelayW );
    pNew = Nf_ManPerformMapping( pAbc->pGia, pPars );
    ABC_FREE( pX );
    ABC_FREE( pY );
    if ( pNew == NULL )
    {
        Abc_Print( -1, "Physical mapping has failed.\n" );
        return 1;
    }
    Abc_FrameUpdateGia( pAbc, pNew );
    /* the mapping changed, so the old binding/placement/timing are gone */
    return Abc_PhysBind( pAbc, fVerbose );

usage:
    Abc_Print( -2, "usage: &phys_map [-KCFEDxy num] [-vwh]\n" );
    Abc_Print( -2, "\t          physical-aware technology mapping (cf. \"&nf\")\n" );
    Abc_Print( -2, "\t          requires a placement from a previous iteration\n" );
    Abc_Print( -2, "\t-K num  : LUT size for the mapping (2 <= K <= %d) [default = %d]\n", pPars->nLutSizeMax, pPars->nLutSize );
    Abc_Print( -2, "\t-C num  : the max number of priority cuts (1 <= C <= %d) [default = %d]\n", pPars->nCutNumMax, pPars->nCutNum );
    Abc_Print( -2, "\t-F num  : the number of area flow rounds [default = %d]\n", pPars->nRounds );
    Abc_Print( -2, "\t-E num  : the area/edge tradeoff parameter (0 <= num <= 100) [default = %d]\n", pPars->nAreaTuner );
    Abc_Print( -2, "\t-D num  : sets the delay constraint for the mapping [default = best possible]\n" );
    Abc_Print( -2, "\t-x num  : cost of the cut bounding box in area flow, in gate areas\n" );
    Abc_Print( -2, "\t          per um; 0 disables the area term [default = %.2f]\n", 0.00 );
    Abc_Print( -2, "\t-y num  : cost of the cut bounding box in delay, in ps per um\n" );
    Abc_Print( -2, "\t          [default = %.2f]\n", 0.05 );
    Abc_Print( -2, "\t-v      : toggles verbose output [default = no]\n" );
    Abc_Print( -2, "\t-w      : toggles very verbose output [default = no]\n" );
    Abc_Print( -2, "\t-h      : print the command usage\n" );
    return 1;
}

ABC_NAMESPACE_IMPL_END

