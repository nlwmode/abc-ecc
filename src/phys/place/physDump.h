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
                   <dir>/<design>/<tag>.congestion.csv  RUDY demand and supply
                   <dir>/<design>/<tag>.rows.csv  row occupancy
                   <dir>/<design>/<tag>.critical_path.csv  critical path
                   <dir>/<design>/<tag>.routability.svg  routability picture
                   <dir>/<design>/snapshots.txt   tags written so far

               The placement SVG has four layers: a blue density heat map
               of the core, the standard cells as translucent boxes, the
               longest nets drawn as thin lines, and the LEF metal shapes
               of the pins those nets reach.  The cell boxes are the LEF
               macro sizes, and the nets run from the output pin of the
               driver to the input pin of every sink, not from the cell
               centers; the pin rectangles are the real PORT geometry of
               the macro, so a wide cell shows its pins spread over its
               shape, and every wire ends at the middle of the largest
               metal piece of its pin, which is the anchor the placer and
               the timing analysis use as well.  When &phys_sta has run,
               the critical path it traced is drawn on top of them as a
               thick saturated-magenta line with a white halo, pin to
               pin, its cells only lightly tinted, so the line is what
               stands out against the density and congestion colours;
               critical_path.csv lists the path
               objects in order with their positions, arrival, required
               time and slack.  The routability SVG
               is a separate RUDY (rectangular uniform wire density) map:
               every net contributes HPWL/(bbox area) to the bins of its
               bounding box, and the resulting wire demand is divided by
               the supply of the routing layers from the LEF.
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
    int    nCongBins; /* routability bins along the longer side     */
    int    fVerbose;
} Phys_DumpPars_t;

extern void Phys_DumpSetDefault( Phys_DumpPars_t * p );
extern int  Phys_DumpRun( Phys_Man_t * pMan, Gia_Man_t * pGia, Phys_DumpPars_t * pPars );

ABC_NAMESPACE_HEADER_END

#endif
