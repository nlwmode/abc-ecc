/**Function*************************************************************

  Synopsis    [LEF reader and data structures]

  Description [A small, self-contained LEF parser.  It reads the parts
               of a technology LEF and a cell LEF that are needed by the
               physical-synthesis flow:
                 - UNITS / MANUFACTURINGGRID
                 - SITE
                 - LAYER (including RC information)
                 - VIA (names only; geometry is skipped)
                 - MACRO / PIN / PORT / RECT / OBS
               Coordinates are stored in LEF database units (DBU).
               The code is intentionally independent from the rest of
               ABC so that the parser can be replaced/improved freely.]

  SideEffects []

***********************************************************************/

#ifndef ABC__phys__lib__physLef_h
#define ABC__phys__lib__physLef_h

#include "phys/lib/physTypes.h"

ABC_NAMESPACE_HEADER_START

typedef struct Phys_LefPin_t_
{
    char *      pName;
    int         x, y;      /* center relative to macro origin, DBU       */
    int         Dir;       /* 0=input 1=output 2=inout 3=power 4=ground  */
    Vec_Int_t * vRects;    /* four ints per RECT                         */
    int         nRects;
} Phys_LefPin_t;

typedef struct Phys_LefMacro_t_
{
    char *      pName;
    char *      pClass;
    char *      pSite;
    int         OriginX, OriginY;
    int         Width, Height;
    Vec_Ptr_t * vPins;     /* Phys_LefPin_t *                           */
    Vec_Int_t * vObs;      /* four ints per obstruction RECT            */
    int         nObs;      /* number of obstruction RECTs               */
    int         nPins;
    int         Id;
} Phys_LefMacro_t;

typedef struct Phys_LefLayer_t_
{
    char *      pName;
    char *      pType;     /* ROUTING, CUT, OVERLAP, ...                 */
    char *      pDir;      /* HORIZONTAL, VERTICAL, ...                  */
    int         Width;
    int         PitchX, PitchY;
    float       ResPerSq;  /* ohms per square                            */
    float       CapPerSq;  /* pF per square micron                       */
    float       EdgeCap;   /* pF per micron of edge                      */
    int         fRouting;
} Phys_LefLayer_t;

typedef struct Phys_LefSite_t_
{
    char *      pName;
    char *      pClass;
    int         Width, Height;
} Phys_LefSite_t;

typedef struct Phys_LefTech_t_
{
    Phys_Units_t Units;
    float        ManufacturingGrid; /* microns                           */
    Vec_Ptr_t *  vLayers;   /* Phys_LefLayer_t *                         */
    Vec_Ptr_t *  vSites;    /* Phys_LefSite_t *                          */
    Vec_Ptr_t *  vMacros;   /* Phys_LefMacro_t *                         */
    Vec_Ptr_t *  vVias;     /* char * (names only)                       */
    Vec_Int_t *  vMacroHash;
} Phys_LefTech_t;

/* technology / macro database */
extern Phys_LefTech_t *  Phys_LefTechAlloc( void );
extern void              Phys_LefTechFree( Phys_LefTech_t ** pp );
extern int               Phys_LefReadTech( Phys_LefTech_t * p, const char * pFileName, int fVerbose );
extern int               Phys_LefReadMacros( Phys_LefTech_t * p, const char * pFileName, int fVerbose );
extern void              Phys_LefFinish( Phys_LefTech_t * p );

/* lookups */
extern Phys_LefMacro_t * Phys_LefFindMacro( Phys_LefTech_t * p, const char * pName );
extern Phys_LefLayer_t * Phys_LefFindLayer( Phys_LefTech_t * p, const char * pName );
extern Phys_LefSite_t *  Phys_LefFindSite( Phys_LefTech_t * p, const char * pName );
extern Phys_LefPin_t *   Phys_LefMacroFindPin( Phys_LefMacro_t * pMacro, const char * pName );
extern int               Phys_LefMacroPinIndex( Phys_LefMacro_t * pMacro, const char * pName );

/* RC helpers */
extern Phys_LefLayer_t * Phys_LefFindRcLayer( Phys_LefTech_t * p, const char * pName );
extern void              Phys_LefLayerRc( Phys_LefTech_t * p, Phys_LefLayer_t * pLayer, float * pResPerMicron, float * pCapPerMicron );

/* reporting */
extern void              Phys_LefTechPrint( Phys_LefTech_t * p );
extern void              Phys_LefMacroPrint( Phys_LefTech_t * p, Phys_LefMacro_t * pMacro );

ABC_NAMESPACE_HEADER_END

#endif

