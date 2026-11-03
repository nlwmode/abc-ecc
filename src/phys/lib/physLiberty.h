/**Function*************************************************************

  Synopsis    [Liberty reader and data structures]

  Description [A self-contained Liberty parser storing the NLDM timing
               data needed by the physical-synthesis flow.  The parser is
               generated with flex/bison (physLibertyLex.l and
               physLibertyYacc.y) and is deliberately independent from
               sclLiberty.c so that it can be improved/replaced without
               affecting existing ABC commands.

               The following subset is supported:
                 library, time_unit, capacitive_load_unit,
                 pulling_resistance_unit, lu_table_template,
                 cell, area, dont_use,
                 pin, direction, capacitance, rise_capacitance,
                 fall_capacitance, max_capacitance, max_transition,
                 clock, function, timing, related_pin, timing_sense,
                 timing_type, cell_rise, cell_fall, rise_transition,
                 fall_transition, index_1, index_2, values
               Everything else is skipped gracefully.]

  SideEffects []

***********************************************************************/

#ifndef ABC__phys__lib__physLiberty_h
#define ABC__phys__lib__physLiberty_h

#include "phys/lib/physTypes.h"

ABC_NAMESPACE_HEADER_START

typedef struct Phys_Surface_t_
{
    Vec_Flt_t * vIndex0;   /* variable_1 (usually input transition)      */
    Vec_Flt_t * vIndex1;   /* variable_2 (usually output load)           */
    Vec_Flt_t * vData;     /* n0 * n1 values, row-major                  */
    int         n0, n1;
} Phys_Surface_t;

typedef struct Phys_TimingArc_t_
{
    char *          pRelatedPin;
    int             Sense;     /* 0=non-unate, 1=positive, -1=negative  */
    int             Type;      /* 0=combinational, 1=rising_edge, ...   */
    Phys_Surface_t * pRise;
    Phys_Surface_t * pFall;
    Phys_Surface_t * pRiseTrans;
    Phys_Surface_t * pFallTrans;
} Phys_TimingArc_t;

typedef struct Phys_LibPin_t_
{
    char *      pName;
    char *      pFunc;     /* Boolean function (output pins), or NULL     */
    int         Dir;       /* 0=input 1=output 2=inout 3=internal        */
    float       Cap;       /* pF                                         */
    float       RiseCap;   /* pF                                         */
    float       FallCap;   /* pF                                         */
    float       MaxCap;    /* pF                                         */
    float       MaxTrans;  /* ps                                         */
    int         fClock;
    Vec_Ptr_t * vArcs;     /* Phys_TimingArc_t *                         */
} Phys_LibPin_t;

typedef struct Phys_LibCell_t_
{
    char *      pName;
    char *      pFormula;   /* supergate structure, NULL for library cells */
    float       Area;       /* um^2                                      */
    int         fSequential;
    int         fDontUse;
    int         nInputs;
    int         nOutputs;
    Vec_Ptr_t * vPins;      /* Phys_LibPin_t *                           */
    Vec_Ptr_t * vArcs;      /* all Phys_TimingArc_t * (owned by pins)    */
    int         CellId;     /* Mio cell id, -1 if unbound                */
    int         MacroId;    /* LEF macro id, -1 if unbound               */
    /* geometry of a synthetic supergate, which has no LEF macro; zero
       when the cell is a library cell */
    int         Width;      /* DBU                                       */
    int         Height;
    int *       pPinDx;     /* input-pin offsets from the center, DBU    */
    int *       pPinDy;
    int         DxOut;      /* output-pin offset from the center, DBU    */
    int         DyOut;
} Phys_LibCell_t;

typedef struct Phys_Template_t_
{
    char *      pName;
    char *      pVar1;      /* variable_1, e.g. input_net_transition    */
    char *      pVar2;      /* variable_2, e.g. total_output_net_capacitance */
    Vec_Flt_t * vIndex1;    /* raw values as found in the file           */
    Vec_Flt_t * vIndex2;
} Phys_Template_t;

typedef struct Phys_Lib_t_
{
    char *        pName;
    char *        pCorner;     /* corner name ("read_liberty -k", or the
                                  PVT name when -k is omitted)             */
    /* PVT of the library, scanned from nom_process/nom_voltage/
       nom_temperature and the default operating condition.  pProc is the
       process word of the operating condition (tt, ss, ff); fPvt is set
       when all three are known.  A corner read without "-k" is named
       "<pProc>_<voltage>_<temperature>" and the corners are ordered by
       these values. */
    float         PvtProcess;
    float         PvtVoltage;
    float         PvtTemp;
    char *        pProc;
    int           fPvt;
    Phys_Units_t  Units;
    Vec_Ptr_t *   vCells;      /* Phys_LibCell_t *                        */
    Vec_Ptr_t *   vTemplates;  /* Phys_Template_t *                       */
    Vec_Int_t *   vCellHash;
    Vec_Ptr_t *   vSuperCells; /* eight supergates, by pattern index      */
    int           fSuperBuilt; /* set when physSuper.c has run            */
} Phys_Lib_t;

/* reading / memory */
extern Phys_Lib_t *      Phys_LibertyRead( const char * pFileName, int fVerbose );
extern void              Phys_LibFree( Phys_Lib_t ** pp );
extern void              Phys_LibFinish( Phys_Lib_t * p );

/* corners: one Phys_Lib_t holds the cells of one corner.  Phys_LibMerge
   moves the cells of pSrc into pDst, so that several Liberty files of the
   same corner (for example, several standard-cell families with distinct
   cell names) can be read into one library.  Returns 1 on success and 0
   when a cell name is already present in pDst; pSrc keeps its templates
   and is released by the caller. */
extern void              Phys_LibSetCorner( Phys_Lib_t * p, const char * pCorner );
extern int               Phys_LibMerge( Phys_Lib_t * pDst, Phys_Lib_t * pSrc );

/* PVT: the corner name derived from the values read by the parser when
   "read_liberty" is called without "-k".  Phys_LibPvtName writes
   "<process>_<voltage>_<temperature>", for example "tt_1.2_25", and
   returns 0 when the file carries no complete PVT.  Phys_LibPvtRank
   orders the process words: 0 = fast (ff), 1 = typical (tt), 2 = slow
   (ss), 3 = PVT known but an unrecognized process, 4 = no PVT. */
extern int               Phys_LibPvtName( Phys_Lib_t * p, char * pBuf, int nBuf );
extern int               Phys_LibPvtRank( Phys_Lib_t * p );

/* lookups */
extern Phys_LibCell_t *  Phys_LibFindCell( Phys_Lib_t * p, const char * pName );
extern Phys_LibPin_t *   Phys_LibCellFindPin( Phys_LibCell_t * pCell, const char * pName );
extern Phys_LibPin_t *   Phys_LibCellFindOutputPin( Phys_LibCell_t * pCell, const char * pName );
extern Phys_TimingArc_t * Phys_LibPinFindArc( Phys_LibPin_t * pPin, const char * pRelated );
extern Phys_TimingArc_t * Phys_LibCellFindArc( Phys_LibCell_t * pCell, const char * pIn, const char * pOut );

/* table lookup (bilinear interpolation, clamped at the table boundary) */
extern float             Phys_SurfaceLookup( Phys_Surface_t * p, float v0, float v1 );
extern float             Phys_SurfaceLookupConst( Phys_Surface_t * p, int i0, int i1 );

/* surface allocation, used by physSuper.c to compose timing arcs */
extern Phys_Surface_t *  Phys_SurfaceAlloc( void );
extern void              Phys_SurfaceFree( Phys_Surface_t * p );
extern void              Phys_LibCellFree( Phys_LibCell_t * p );

/* reporting */
extern void              Phys_LibPrint( Phys_Lib_t * p );
extern void              Phys_LibCellPrint( Phys_Lib_t * p, Phys_LibCell_t * pCell );

ABC_NAMESPACE_HEADER_END

#endif

