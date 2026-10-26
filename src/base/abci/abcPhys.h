/**Function*************************************************************

  Synopsis    [Commands for physical synthesis]

  Description []

  SideEffects []

***********************************************************************/

#ifndef ABC__base__abci__abcPhys_h
#define ABC__base__abci__abcPhys_h

#include "base/main/main.h"

ABC_NAMESPACE_HEADER_START

extern int Abc_CommandReadTechLef( Abc_Frame_t * pAbc, int argc, char ** argv );
extern int Abc_CommandReadLef( Abc_Frame_t * pAbc, int argc, char ** argv );
extern int Abc_CommandReadLiberty( Abc_Frame_t * pAbc, int argc, char ** argv );
extern int Abc_CommandSetSignalRes( Abc_Frame_t * pAbc, int argc, char ** argv );
extern int Abc_CommandSetSignalCap( Abc_Frame_t * pAbc, int argc, char ** argv );
extern int Abc_CommandPrintPhys( Abc_Frame_t * pAbc, int argc, char ** argv );
extern int Abc_CommandPhysStart( Abc_Frame_t * pAbc, int argc, char ** argv );
extern int Abc_CommandPhysEnd( Abc_Frame_t * pAbc, int argc, char ** argv );
extern int Abc_CommandPhysBind( Abc_Frame_t * pAbc, int argc, char ** argv );
extern int Abc_CommandPhysPlace( Abc_Frame_t * pAbc, int argc, char ** argv );
extern int Abc_CommandPhysSta( Abc_Frame_t * pAbc, int argc, char ** argv );
extern int Abc_CommandPhysDump( Abc_Frame_t * pAbc, int argc, char ** argv );
extern int Abc_CommandPhysMap( Abc_Frame_t * pAbc, int argc, char ** argv );

ABC_NAMESPACE_HEADER_END

#endif

