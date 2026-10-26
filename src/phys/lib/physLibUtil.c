/**Function*************************************************************

  Synopsis    [Shared helpers for the physical-library readers]

  Description [A tiny open-addressing string hash used to look up LEF
               macros, layers, sites and Liberty cells by name.  The
               object vectors are built once by the parsers, so there is
               no need for incremental rehashing.]

  SideEffects []

***********************************************************************/

#include <string.h>
#include "phys/phys.h"

ABC_NAMESPACE_IMPL_START


/**Function*************************************************************

  Synopsis    [Reads the whole file into a null-terminated buffer]

  Description []

  SideEffects []

***********************************************************************/
char * Phys_ReadFileContents( const char * pFileName, int * pnSize )
{
    FILE * pFile;
    char * pData;
    long FileSize;
    size_t nRead;
    pFile = fopen( pFileName, "rb" );
    if ( pFile == NULL )
    {
        fprintf( stderr, "Error: cannot open file \"%s\".\n", pFileName );
        return NULL;
    }
    fseek( pFile, 0, SEEK_END );
    FileSize = ftell( pFile );
    fseek( pFile, 0, SEEK_SET );
    if ( FileSize <= 0 )
    {
        fprintf( stderr, "Error: file \"%s\" is empty.\n", pFileName );
        fclose( pFile );
        return NULL;
    }
    pData = ABC_ALLOC( char, (size_t)FileSize + 1 );
    nRead = fread( pData, 1, (size_t)FileSize, pFile );
    fclose( pFile );
    pData[nRead] = '\0';
    if ( pnSize )
        *pnSize = (int)nRead;
    return pData;
}


/**Function*************************************************************

  Synopsis    [FNV-1a string hash]

  Description []

  SideEffects []

***********************************************************************/
unsigned Phys_HashString( const char * pStr )
{
    unsigned Hash = 2166136261u;
    while ( *pStr )
    {
        Hash ^= (unsigned char)*pStr++;
        Hash *= 16777619u;
    }
    return Hash;
}


/**Function*************************************************************

  Synopsis    [Returns the name stored at the beginning of an object]

  Description [All objects placed in the hash vectors start with a
               char * pName field.]

  SideEffects []

***********************************************************************/
static char * Phys_HashKey( void * pObj )
{
    return *(char **)pObj;
}


/**Function*************************************************************

  Synopsis    [Builds a name hash for a vector of objects]

  Description []

  SideEffects [The previous hash, if any, is freed.]

***********************************************************************/
void Phys_HashBuild( Vec_Int_t ** pvHash, Vec_Ptr_t * vObjs )
{
    int nObjs = Vec_PtrSize( vObjs );
    int nSize = 16, i;
    while ( nSize < 2 * nObjs )
        nSize <<= 1;
    Vec_IntFreeP( pvHash );
    *pvHash = Vec_IntStart( nSize );
    for ( i = 0; i < nObjs; i++ )
    {
        char * pName = Phys_HashKey( Vec_PtrEntry( vObjs, i ) );
        unsigned Hash = Phys_HashString( pName );
        while ( Vec_IntEntry( *pvHash, (int)(Hash & (unsigned)(nSize-1)) ) )
            Hash++;
        Vec_IntWriteEntry( *pvHash, (int)(Hash & (unsigned)(nSize-1)), i + 1 );
    }
}


/**Function*************************************************************

  Synopsis    [Finds an object by name; returns -1 if not found]

  Description []

  SideEffects []

***********************************************************************/
int Phys_HashFind( Vec_Int_t * vHash, Vec_Ptr_t * vObjs, const char * pName )
{
    int nSize, i;
    unsigned Hash;
    if ( vHash == NULL || Vec_IntSize(vHash) == 0 )
        return -1;
    nSize = Vec_IntSize( vHash );
    Hash  = Phys_HashString( pName );
    for ( i = 0; i < nSize; i++ )
    {
        int Id = Vec_IntEntry( vHash, (int)((Hash + (unsigned)i) & (unsigned)(nSize-1)) );
        if ( Id == 0 )
            return -1;
        Id--;
        if ( !strcmp( Phys_HashKey(Vec_PtrEntry(vObjs, Id)), pName ) )
            return Id;
    }
    return -1;
}

ABC_NAMESPACE_IMPL_END

