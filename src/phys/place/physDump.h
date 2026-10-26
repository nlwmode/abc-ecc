/**Function*************************************************************

  Synopsis    [Placement workspace writer for debugging]

  Description [Writes a snapshot of the current placement into a
               workspace directory, so that the placement can be looked
               at without re-running the flow.  The workspace layout is

                   <dir>/index.html          index of all designs
                   <dir>/designs.txt         names of the designs seen
                   <dir>/<design>/placement.log   one record per dump
                   <dir>/<design>/<tag>.svg       picture of the placement
                   <dir>/<design>/<tag>.cells.csv cell geometries
                   <dir>/<design>/<tag>.nets.csv  net bounding boxes
                   <dir>/<design>/<tag>.density.csv  bin occupancy
                   <dir>/<design>/<tag>.rows.csv  row occupancy
                   <dir>/<design>/snapshots.txt   tags written so far

               The SVG has three layers: a density heat map of the core,
               the cell rectangles, and the longest nets drawn as lines.
               The command layer only passes strings; the writer uses the
               placements saved by &phys_place and the timing saved by
               &phys_sta.]

  SideEffects [Creates directories and files]

***********************************************************************/

#ifndef ABC__phys__place__physDump_h
#define ABC__phys__place__physDump_h

#include "aig/gia/gia.h"
#include "phys/physMan.h"

ABC_NAMESPACE_HEADER_START

typedef struct Phys_DumpPars_t_
{
    char * pDir;      /* workspace directory                        */
    char * pDesign;   /* design name, or NULL to use the network's  */
    char * pTag;      /* snapshot tag, or NULL for "placement"      */
    int    nMaxNets;  /* longest nets to draw, 0 to draw none       */
    int    nBins;     /* density bins along the longer side         */
    int    fVerbose;
} Phys_DumpPars_t;

extern void Phys_DumpSetDefault( Phys_DumpPars_t * p );
extern int  Phys_DumpRun( Phys_Man_t * pMan, Gia_Man_t * pGia, Phys_DumpPars_t * pPars );

ABC_NAMESPACE_HEADER_END

#endif
