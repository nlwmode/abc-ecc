/**Function*************************************************************

  Synopsis    [Genlib generation from Liberty data]

  Description [Converts the parsed Liberty library into the genlib
               format consumed by the ABC standard-cell mapper and by
               &nf.  Only combinational single-output cells with a
               Boolean function are emitted; unsupported functions are
               skipped.  The generated text is self-contained and ends
               with ".end".]

  SideEffects []

***********************************************************************/

#ifndef ABC__phys__lib__physGenlib_h
#define ABC__phys__lib__physGenlib_h

#include "phys/lib/physTypes.h"
#include "phys/lib/physLiberty.h"

ABC_NAMESPACE_HEADER_START

typedef struct Phys_GenlibStats_t_
{
    int nGates;
    int nSkipped;
    int nSeq;
    int nMultiOut;
    int nNoFunc;
} Phys_GenlibStats_t;

/* Build a genlib description of the library.  The returned string is
   allocated with ABC_ALLOC() and should be freed by the caller. */
extern char * Phys_GenlibBuild( Phys_Lib_t * pLib, Phys_GenlibStats_t * pStats, int fVerbose );

ABC_NAMESPACE_HEADER_END

#endif
