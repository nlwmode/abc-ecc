/**Function*************************************************************

  Synopsis    [Rectilinear Steiner tree for the interconnect model]

  Description [Builds a rectilinear Steiner tree connecting the pins of
               one net.  The terminals are the output pin of the driver
               and the input pins of the sinks, given as coordinates in
               microns.  The tree is rooted at terminal zero (the
               driver), and every node keeps its parent and the length
               of the edge to it, so the caller can accumulate the
               Elmore delay of the RC tree.

               Small nets are solved with the iterated 1-Steiner
               heuristic: starting from the rectilinear minimum spanning
               tree, the Hanan grid point that shortens the tree the
               most is added as a Steiner point, and the search repeats
               until no point helps.  Larger nets are broken into
               groups by a median split of the longer side of the
               bounding box; every group is solved on its own and the
               group trees are joined by the shortest edge between
               them.  A result that is longer than the minimum spanning
               tree of the terminals is discarded in favor of that
               tree, so the construction is never worse than the plain
               spanning tree model.]

  SideEffects []

***********************************************************************/

#ifndef ABC__phys__lib__physSteiner_h
#define ABC__phys__lib__physSteiner_h

#include "phys/lib/physTypes.h"

ABC_NAMESPACE_HEADER_START

typedef struct Phys_Stree_t_
{
    int     nNodes;    /* terminals followed by the Steiner points      */
    int     nTerms;    /* the number of terminals, set by the caller    */
    int     nSteiner;  /* the number of Steiner points                  */
    int     fRun;      /* the Steiner search ran on this net            */
    int     fUsed;     /* its tree was shorter than the spanning tree   */
    float * pX;        /* node x coordinates, microns                   */
    float * pY;        /* node y coordinates, microns                   */
    int   * pParent;   /* parent of every node, -1 at the driver        */
    float * pLen;      /* length of the edge to the parent, microns     */
    int   * pTermNode; /* tree node of every terminal, 0..nTerms-1      */
    float   TotalLen;  /* total wire length of the tree, microns        */
} Phys_Stree_t;

/* builds the tree over nTerms points; the first point is the driver.
   fSteiner = 0 reproduces the plain minimum spanning tree */
extern Phys_Stree_t * Phys_StreeBuild( float * pX, float * pY, int nTerms, int fSteiner );
extern void           Phys_StreeFree( Phys_Stree_t * p );

ABC_NAMESPACE_HEADER_END

#endif
