/**Function*************************************************************

  Synopsis    [Common types and unit handling for physical synthesis]

  Description [All physical-synthesis data is kept in a self-contained,
               decoupled layer.  Internally we use the following units:
               - geometry / placement : LEF database units (DBU)
               - time                 : picoseconds (ps)
               - capacitance          : picofarads (pF)
               - resistance           : kilo-ohms (kOhm)
               - area                 : square microns (um^2)
               The parser converts the values found in Liberty/LEF files
               into these internal units.]

  SideEffects []

***********************************************************************/

#ifndef ABC__phys__lib__physTypes_h
#define ABC__phys__lib__physTypes_h

#include "misc/vec/vec.h"

ABC_NAMESPACE_HEADER_START

/* unit information shared by the LEF and Liberty readers */
typedef struct Phys_Units_t_
{
    int    Dbu;        /* LEF database units per micron (default 1000)     */
    float  TimeScale;  /* multiply file value by this to get ps            */
    float  CapScale;   /* multiply file value by this to get pF            */
    float  ResScale;   /* multiply file value by this to get kOhm          */
    char * pTimeUnit;  /* original Liberty time unit string, if any        */
    char * pCapUnit;   /* original Liberty cap unit string, if any         */
    char * pResUnit;   /* original Liberty res unit string, if any         */
} Phys_Units_t;

static inline void Phys_UnitsInit( Phys_Units_t * p )
{
    p->Dbu       = 1000;
    p->TimeScale = 1000.0f; /* 1 ns -> ps */
    p->CapScale  = 1.0f;    /* 1 pF -> pF */
    p->ResScale  = 1.0f;    /* 1 kOhm -> kOhm */
    p->pTimeUnit = NULL;
    p->pCapUnit  = NULL;
    p->pResUnit  = NULL;
}

static inline void Phys_UnitsClear( Phys_Units_t * p )
{
    ABC_FREE( p->pTimeUnit );
    ABC_FREE( p->pCapUnit );
    ABC_FREE( p->pResUnit );
}

/* file helper */
extern char *   Phys_ReadFileContents( const char * pFileName, int * pnSize );

/* generic string hash table helper (stores index+1 in an int vector) */
extern unsigned Phys_HashString( const char * pStr );
extern void     Phys_HashBuild( Vec_Int_t ** pvHash, Vec_Ptr_t * vObjs );
extern int      Phys_HashFind( Vec_Int_t * vHash, Vec_Ptr_t * vObjs, const char * pName );

ABC_NAMESPACE_HEADER_END

#endif

