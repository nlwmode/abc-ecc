/**Function*************************************************************

  Synopsis    [Choice classes of a Gia network with sibling nodes]

  Description [A network built by "&choice" or "&st; &dch" carries a list
               of functionally equivalent subcircuits of which the AIG
               keeps all, while only one is the representative of the
               netlist.  This module turns that list into classes for the
               physical flow, so that the placer and the timing analysis
               can count exactly one alternative per class.

               The generators differ in the list they leave behind: "&st;
               &dch" writes a sibling chain into pSibls, while "&choice"
               writes an equivalence class into pReprs/pNexts, whose head
               is itself a member.  Both are read here, and the rest of
               the flow sees only the members of a class.

               The members are equivalent up to inversion, they have no
               fanout inside the class (the classes ABC builds are
               independent, so no member is in the cone of another), and
               each member therefore owns the nodes that exist only
               because of it: its MFFC.  That private
               cone is the subgraph of the alternative, and the nodes
               outside every cone are the shared logic -- the inputs, the
               output and the internal logic the alternatives have in
               common -- which is active whatever the selection is.  This
               is what makes the alternatives comparable: they all see
               the same boundary.

               The representative of a fresh class is the member the
               network already uses, i.e. the one with a fanout outside
               the class ("&choice" keeps the head and drops the siblings
               with fanouts, "&st; &dch" may keep the fanouts elsewhere),
               with the first member as the fallback.  A selection
               replaces it: the cones of the other members become
               inactive and leave the netlist view, the cone of the
               selected member comes back.

               A reference to a class member is ambiguous once the
               selection can move, so the physical flow resolves it:
               a fanin of a node outside the class is redirected to the
               selected member, while a fanin inside the member's own
               cone is kept, which preserves the internal nets of the
               alternative under evaluation.]

  SideEffects []

  SeeAlso     []

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aig/gia/gia.h"
#include "phys/phys.h"
#include "phys/physMan.h"

ABC_NAMESPACE_IMPL_START

/* the MFFC walk of one alternative is capped, so that a class built on a
   large common cone does not sweep the whole design into one local
   evaluation; a mapped alternative stays far below the cap */
#define PHYS_CHOICE_MFFC_MAX 4000

typedef struct Phys_ChoiceClass_t_
{
    int   nAlts;       /* alternatives of the class                    */
    int   iActive;     /* index of the selected alternative            */
    Vec_Int_t * vAlts; /* class members as positive literals           */
    Vec_Ptr_t * vCones;/* per alternative: Vec_Int_t * of the MFFC,
                          in topological (ascending object) order      */
} Phys_ChoiceClass_t;


/* the nodes the object would take with it, from the shared reference
   counts: the walk stops where some other node still refers to a fanin */
static Vec_Int_t * Phys_ChoiceCone( Gia_Man_t * pGia, int iMember )
{
    Vec_Int_t * vNodes  = Vec_IntAlloc( 16 );
    Vec_Int_t * vLeaves = Vec_IntAlloc( 16 );
    Vec_Int_t * vInners = Vec_IntAlloc( 16 );
    Gia_ObjCheckMffc( pGia, Gia_ManObj(pGia, Abc_Lit2Var(iMember)), PHYS_CHOICE_MFFC_MAX,
                      vNodes, vLeaves, vInners );
    Vec_IntFree( vNodes );
    Vec_IntFree( vLeaves );
    return vInners;
}

/* the member the network uses: one with a fanout of the whole network.
   The chains ABC builds are independent, so the fanouts of a member are
   outside its class and the reference count is the criterion.  The
   return value is an index into vAlts, not a literal. */
static int Phys_ChoicePickRepr( Gia_Man_t * pGia, Vec_Int_t * vAlts )
{
    int i, k, iBest = 0, nBest = -1;
    Vec_IntForEachEntry( vAlts, i, k )
        if ( Gia_ObjRefNumId( pGia, Abc_Lit2Var(i) ) > nBest )
            nBest = Gia_ObjRefNumId( pGia, Abc_Lit2Var(i) ), iBest = k;
    return iBest;
}

/* marks the objects of the alternatives that are not selected; the shared
   nodes stay active in every selection.  The selected cones are marked in
   a second pass, so a node that two alternatives of one class happen to
   share stays active when either of them is selected. */
static void Phys_ChoiceActiveInt( Phys_Man_t * p )
{
    int c, a, k, iObj;
    for ( iObj = 0; iObj < Vec_IntSize(p->vObjActive); iObj++ )
        Vec_IntWriteEntry( p->vObjActive, iObj, 1 );
    for ( c = 0; c < p->nClasses; c++ )
    for ( a = 0; a < p->pClasses[c].nAlts; a++ )
    {
        if ( a == p->pClasses[c].iActive )
            continue;
        Vec_IntForEachEntry( (Vec_Int_t *)Vec_PtrEntry(p->pClasses[c].vCones, a), iObj, k )
            Vec_IntWriteEntry( p->vObjActive, iObj, 0 );
    }
    for ( c = 0; c < p->nClasses; c++ )
        Vec_IntForEachEntry( Phys_ChoiceClassCone(p, c, p->pClasses[c].iActive), iObj, k )
            Vec_IntWriteEntry( p->vObjActive, iObj, 1 );
}

void Phys_ChoiceFree( Phys_Man_t * p )
{
    int c, k;
    if ( p == NULL )
        return;
    for ( c = 0; p->pClasses != NULL && c < p->nClasses; c++ )
    {
        Vec_Int_t * pCone;
        Vec_PtrForEachEntry( Vec_Int_t *, p->pClasses[c].vCones, pCone, k )
            Vec_IntFree( pCone );
        Vec_PtrFreeP( &p->pClasses[c].vCones );
        Vec_IntFreeP( &p->pClasses[c].vAlts );
    }
    ABC_FREE( p->pClasses );
    p->nClasses = 0;
    p->fHasChoices = 0;
    Vec_IntFreeP( &p->vObjClass );
    Vec_IntFreeP( &p->vObjAlt );
    Vec_IntFreeP( &p->vObjActive );
    Vec_IntFreeP( &p->vObjSiblLit );
    Vec_IntFreeP( &p->vObjPhase );
}

/* Fills vAlts with the members of the class the object iHead belongs to.
   The two representations of a choice network are distinct: "&st; &dch"
   leaves a sibling chain in pSibls, and "&choice" leaves an equivalence
   class in pReprs/pNexts, whose head is a member rather than a sibling.
   Both are walked here, so the rest of the flow sees only vAlts. */
static int Phys_ChoiceChain( Gia_Man_t * pGia, int iHead, Vec_Int_t * vAlts, int nObjs )
{
    int iMember;
    Vec_IntClear( vAlts );
    if ( pGia->pSibls != NULL )
    {
        for ( iMember = iHead; iMember > 0 && iMember < nObjs; iMember = Gia_ObjSibl(pGia, iMember) )
        {
            if ( Vec_IntSize(vAlts) > nObjs )
                return 0;
            Vec_IntPush( vAlts, 2 * iMember );
        }
        return 1;
    }
    /* an equivalence class: the head and every member the next chain
       reaches.  Gia_ObjIsHead asserts above, so the head is a member. */
    if ( !Gia_ObjIsHead(pGia, iHead) )
        return 0;
    for ( iMember = iHead; iMember > 0 && iMember < nObjs; iMember = Gia_ObjNext(pGia, iMember) )
    {
        if ( Vec_IntSize(vAlts) > nObjs )
            return 0;
        Vec_IntPush( vAlts, 2 * iMember );
    }
    return 1;
}

int Phys_ChoiceDerive( Phys_Man_t * p, Gia_Man_t * pGia )
{
    int nObjs, i, c, a, k, iObj;
    Vec_Int_t * vHasNext;
    int fSibls;
    if ( p == NULL || pGia == NULL )
        return 0;
    Phys_ChoiceFree( p );
    fSibls = ( pGia->pSibls != NULL );
    if ( !fSibls && ( pGia->pReprs == NULL || pGia->pNexts == NULL ) )
        return 0;
    nObjs = Gia_ManObjNum( pGia );
    p->vObjClass   = Vec_IntStartFull( nObjs );
    p->vObjAlt     = Vec_IntStartFull( nObjs );
    p->vObjActive  = Vec_IntStart( nObjs );
    p->vObjSiblLit = Vec_IntStartFull( nObjs );
    p->pClasses    = ABC_CALLOC( Phys_ChoiceClass_t, nObjs );
    vHasNext       = Vec_IntStart( nObjs );
    if ( fSibls )
    for ( i = 1; i < nObjs; i++ )
    {
        int iNext = Gia_ObjSibl( pGia, i );
        Vec_IntWriteEntry( p->vObjSiblLit, i, iNext > 0 ? 2 * iNext : -1 );
        if ( iNext > 0 && iNext < nObjs )
            Vec_IntWriteEntry( vHasNext, iNext, 1 );
    }
    /* A chain head is a member that no other member points at; walking
       each member once enumerates the chains.  For a sibling chain the
       head is the member with no predecessor in the list; for an
       equivalence class it is the representative, which is a member with
       no representative of its own.  A cycle, which the choice generators
       do not produce, is not entered twice because vHasNext marks every
       member that some chain reaches. */
    for ( i = 1; i < nObjs; i++ )
    {
        Phys_ChoiceClass_t * pClass;
        if ( fSibls )
        {
            if ( Gia_ObjSibl( pGia, i ) == 0 || Vec_IntEntry( vHasNext, i ) )
                continue;
        }
        else
        {
            if ( !Gia_ObjIsHead( pGia, i ) )
                continue;
        }
        pClass = p->pClasses + p->nClasses;
        pClass->vAlts = Vec_IntAlloc( 4 );
        pClass->vCones = Vec_PtrAlloc( 4 );
        if ( !Phys_ChoiceChain( pGia, i, pClass->vAlts, nObjs ) ||
             Vec_IntSize( pClass->vAlts) < 2 )
        {
            Vec_IntFreeP( &pClass->vAlts );
            Vec_PtrFreeP( &pClass->vCones );
            continue;
        }
        Vec_IntForEachEntry( pClass->vAlts, iObj, a )
            Vec_IntWriteEntry( vHasNext, Abc_Lit2Var(iObj), 1 );
        pClass->nAlts = Vec_IntSize( pClass->vAlts );
        p->nClasses++;
    }
    Vec_IntFree( vHasNext );
    if ( p->nClasses == 0 )
    {
        Phys_ChoiceFree( p );
        return 0;
    }
    /* The members of a class are equivalent up to inversion, so the
       redirection of a fanin has to carry the phase difference: the value
       of the node on one input pattern, which is the same for every
       pattern because the members are equivalent.  The all-zero pattern
       is used here, computed fresh rather than read from fPhase, which
       other passes reuse for their own purposes.  Gia object ids are
       topological, so one forward sweep suffices, and a member of a
       class sees its fanins already valued because the classes are
       independent. */
    p->vObjPhase = Vec_IntStart( nObjs );
    for ( i = 0; i < nObjs; i++ )
    {
        Gia_Obj_t * pObj = Gia_ManObj( pGia, i );
        int ph;
        if ( Gia_ObjIsAnd(pObj) )
        {
            int f0 = Vec_IntEntry(p->vObjPhase, Gia_ObjFaninId0p(pGia, pObj)) ^ Gia_ObjFaninC0(pObj);
            int f1 = Vec_IntEntry(p->vObjPhase, Gia_ObjFaninId1p(pGia, pObj)) ^ Gia_ObjFaninC1(pObj);
            if ( Gia_ObjIsMuxId(pGia, i) )
            {
                int f2 = Vec_IntEntry(p->vObjPhase, Gia_ObjFaninId2p(pGia, pObj)) ^ Gia_ObjFaninC2(pGia, pObj);
                ph = (f2 && f1) || (!f2 && f0);
            }
            else
                ph = f0 & f1;
        }
        else if ( Gia_ObjIsCo(pObj) )
            ph = Vec_IntEntry(p->vObjPhase, Gia_ObjFaninId0p(pGia, pObj)) ^ Gia_ObjFaninC0(pObj);
        else
            ph = 0;
        Vec_IntWriteEntry( p->vObjPhase, i, ph );
    }
    /* every member is a root of its own cone; the class and alternative
       index of a node is what tells an internal reference of an
       alternative from a reference to the class */
    if ( pGia->pRefs == NULL )
        Gia_ManCreateRefs( pGia );
    for ( c = 0; c < p->nClasses; c++ )
    {
        Phys_ChoiceClass_t * pClass = p->pClasses + c;
        for ( a = 0; a < pClass->nAlts; a++ )
        {
            Vec_Int_t * vCone = Phys_ChoiceCone( pGia, Vec_IntEntry(pClass->vAlts, a) );
            Vec_PtrPush( pClass->vCones, vCone );
            Vec_IntForEachEntry( vCone, iObj, k )
            {
                Vec_IntWriteEntry( p->vObjClass, iObj, c );
                Vec_IntWriteEntry( p->vObjAlt,   iObj, a );
            }
            Vec_IntWriteEntry( p->vObjClass, Abc_Lit2Var(Vec_IntEntry(pClass->vAlts, a)), c );
            Vec_IntWriteEntry( p->vObjAlt,   Abc_Lit2Var(Vec_IntEntry(pClass->vAlts, a)), a );
        }
        pClass->iActive = Phys_ChoicePickRepr( pGia, pClass->vAlts );
    }
    p->fHasChoices = 1;
    Phys_ChoiceActiveInt( p );
    return 1;
}

void Phys_ChoiceActive( Phys_Man_t * p, Gia_Man_t * pGia )
{
    (void)pGia;
    if ( p == NULL || !p->fHasChoices )
        return;
    Phys_ChoiceActiveInt( p );
}

void Phys_ChoiceSetActive( Phys_Man_t * p, Gia_Man_t * pGia, int iClass, int iAlt )
{
    (void)pGia;
    if ( p == NULL || !p->fHasChoices || iClass < 0 || iClass >= p->nClasses ||
         iAlt < 0 || iAlt >= p->pClasses[iClass].nAlts )
        return;
    if ( p->pClasses[iClass].iActive == iAlt )
        return;
    p->pClasses[iClass].iActive = iAlt;
    Phys_ChoiceActiveInt( p );
}

int Phys_ChoiceActiveNum( Phys_Man_t * p, int iClass )
{
    if ( p == NULL || !p->fHasChoices || iClass < 0 || iClass >= p->nClasses )
        return -1;
    return p->pClasses[iClass].iActive;
}

int Phys_ChoiceObjClass( Phys_Man_t * p, int iObj )
{
    if ( p == NULL || !p->fHasChoices || iObj < 0 || iObj >= Vec_IntSize(p->vObjClass) )
        return -1;
    return Vec_IntEntry( p->vObjClass, iObj );
}

int Phys_ChoiceObjAlt( Phys_Man_t * p, int iObj )
{
    if ( p == NULL || !p->fHasChoices || iObj < 0 || iObj >= Vec_IntSize(p->vObjAlt) )
        return -1;
    return Vec_IntEntry( p->vObjAlt, iObj );
}

int Phys_ChoiceObjActive( Phys_Man_t * p, int iObj )
{
    if ( p == NULL || !p->fHasChoices )
        return 1;
    if ( iObj < 0 || iObj >= Vec_IntSize(p->vObjActive) )
        return 1;
    return Vec_IntEntry( p->vObjActive, iObj );
}

int Phys_ChoiceClassSize( Phys_Man_t * p, int iClass )
{
    if ( p == NULL || !p->fHasChoices || iClass < 0 || iClass >= p->nClasses )
        return 0;
    return p->pClasses[iClass].nAlts;
}

Vec_Int_t * Phys_ChoiceClassCone( Phys_Man_t * p, int iClass, int iAlt )
{
    if ( p == NULL || !p->fHasChoices || iClass < 0 || iClass >= p->nClasses ||
         iAlt < 0 || iAlt >= p->pClasses[iClass].nAlts )
        return NULL;
    return (Vec_Int_t *)Vec_PtrEntry( p->pClasses[iClass].vCones, iAlt );
}

Vec_Int_t * Phys_ChoiceClassAlts( Phys_Man_t * p, int iClass )
{
    if ( p == NULL || !p->fHasChoices || iClass < 0 || iClass >= p->nClasses )
        return NULL;
    return p->pClasses[iClass].vAlts;
}

/* The literal the network drives for a fanin reference.  A reference from
   outside the class of a member that is not selected is redirected to the
   selected member, which is what keeps the fanouts of the class attached
   to whichever alternative the placement chose.  A reference inside the
   member's own cone is kept: it is an internal net of that alternative.

   Class members are equivalent only up to inversion, so the redirection
   also carries the complement bit: the selected member s equals the
   referenced member m when s = m ^ (phase(m) ^ phase(s)), and the phase
   is the value of the node on the all-zero input pattern, which the
   choice computation leaves in fPhase.  ResolveFanin therefore needs the
   phases of the network, which Phys_ChoiceDerive computes once. */
int Phys_ChoiceResolveFanin( Phys_Man_t * p, int iSinkObj, int iLit )
{
    int v = Abc_Lit2Var( iLit ), iClass, iAlt, iSel, fCompl;
    if ( p == NULL || !p->fHasChoices || v <= 1 || v >= Vec_IntSize(p->vObjClass) )
        return iLit;
    iClass = Vec_IntEntry( p->vObjClass, v );
    if ( iClass < 0 || Vec_IntEntry( p->vObjActive, v ) )
        return iLit;
    iAlt = Vec_IntEntry( p->vObjAlt, v );
    if ( iSinkObj >= 0 && Vec_IntEntry(p->vObjClass, iSinkObj) == iClass &&
         Vec_IntEntry(p->vObjAlt, iSinkObj) == iAlt )
        return iLit;
    iSel = Abc_Lit2Var( Vec_IntEntry(p->pClasses[iClass].vAlts, p->pClasses[iClass].iActive) );
    fCompl = Abc_LitIsCompl( iLit ) ^
             ( Vec_IntEntry(p->vObjPhase, v) ^ Vec_IntEntry(p->vObjPhase, iSel) );
    return Abc_Var2Lit( iSel, fCompl );
}

/* Duplicates the network with the alternatives that are not selected
   deleted: a node of an inactive cone is replaced by the selected member
   of its class, and every fanin that refers to such a node is redirected
   the way Phys_ChoiceResolveFanin reads it.  The result is the netlist
   the selection describes -- what a flow that cannot select would see
   after the choice graph is reduced -- so its cells are, one for one,
   the active nodes of the placement view.

   The traversal starts from every active node in topological order, so
   the duplicate is in topological order even when a redirected fanin
   points at a member with a larger object id than its sink, and the
   active nodes that no output reaches are kept as well: the placement
   places them, so the two netlists are only comparable cell for cell if
   the duplicate has them too.  The nodes are appended without structural
   hashing, so the cell count is the count of active nodes. */
static void Phys_ChoiceReduce_rec( Gia_Man_t * pNew, Gia_Man_t * p, Phys_Man_t * pMan,
                                   Gia_Obj_t * pObj, Vec_Int_t * vMap )
{
    int iObj = Gia_ObjId( p, pObj );
    int iLit0, iLit1;
    Gia_Obj_t * pFan0, * pFan1;
    if ( !Phys_ChoiceObjActive( pMan, iObj ) )
    {
        /* a sink outside the class of a member that is not selected; the
           phase difference is part of the literal the function returns */
        int iSel = Phys_ChoiceResolveFanin( pMan, -1, 2 * iObj );
        Gia_Obj_t * pSel = Gia_ManObj( p, Abc_Lit2Var(iSel) );
        Phys_ChoiceReduce_rec( pNew, p, pMan, pSel, vMap );
        pObj->Value = Abc_LitNotCond( pSel->Value, Abc_LitIsCompl(iSel) );
        return;
    }
    if ( ~pObj->Value )
        return;
    assert( Gia_ObjIsAnd(pObj) );
    if ( Gia_ObjIsBuf(pObj) )
    {
        iLit0 = Phys_ChoiceResolveFanin( pMan, iObj, Gia_ObjFaninLit0p(p, pObj) );
        pFan0 = Gia_ManObj( p, Abc_Lit2Var(iLit0) );
        Phys_ChoiceReduce_rec( pNew, p, pMan, pFan0, vMap );
        pObj->Value = Gia_ManAppendBuf( pNew, Abc_LitNotCond( pFan0->Value, Abc_LitIsCompl(iLit0) ) );
    }
    else
    {
        iLit0 = Phys_ChoiceResolveFanin( pMan, iObj, Gia_ObjFaninLit0p(p, pObj) );
        iLit1 = Phys_ChoiceResolveFanin( pMan, iObj, Gia_ObjFaninLit1p(p, pObj) );
        pFan0 = Gia_ManObj( p, Abc_Lit2Var(iLit0) );
        pFan1 = Gia_ManObj( p, Abc_Lit2Var(iLit1) );
        Phys_ChoiceReduce_rec( pNew, p, pMan, pFan0, vMap );
        Phys_ChoiceReduce_rec( pNew, p, pMan, pFan1, vMap );
        iLit0 = Abc_LitNotCond( pFan0->Value, Abc_LitIsCompl(iLit0) );
        iLit1 = Abc_LitNotCond( pFan1->Value, Abc_LitIsCompl(iLit1) );
        /* the redirection can make the two fanins of a node the same literal */
        if ( iLit0 == iLit1 )
            pObj->Value = Gia_ManAppendBuf( pNew, iLit0 );
        else if ( iLit0 == Abc_LitNot(iLit1) )
            pObj->Value = 0;
        else
            pObj->Value = Gia_ManAppendAnd( pNew, iLit0, iLit1 );
    }
    if ( vMap != NULL && Abc_Lit2Var(pObj->Value) > 0 )
        Vec_IntWriteEntry( vMap, Abc_Lit2Var(pObj->Value), iObj );
}

/* vMap, when given, receives the object of the original network every
   node of the duplicate was built from, so that the binding of the
   choice graph can be carried over to the reduced netlist */
Gia_Man_t * Phys_ChoiceReduce( Phys_Man_t * p, Gia_Man_t * pGia, Vec_Int_t * vMap )
{
    Gia_Man_t * pNew;
    Gia_Obj_t * pObj;
    int i;
    if ( p == NULL || pGia == NULL || !p->fHasChoices )
        return NULL;
    if ( vMap != NULL )
        Vec_IntFill( vMap, Gia_ManObjNum(pGia), -1 );
    pNew = Gia_ManStart( Gia_ManObjNum(pGia) );
    pNew->pName = Abc_UtilStrsav( pGia->pName );
    pNew->pSpec = Abc_UtilStrsav( pGia->pSpec );
    Gia_ManFillValue( pGia );
    Gia_ManConst0(pGia)->Value = 0;
    Gia_ManForEachCi( pGia, pObj, i )
    {
        pObj->Value = Gia_ManAppendCi( pNew );
        if ( vMap != NULL )
            Vec_IntWriteEntry( vMap, Abc_Lit2Var(pObj->Value), i );
    }
    /* Every active node is materialized, including the ones no output
       reaches: the placement's active set is the cone-based one, so a
       class whose only consumers are alternatives that were not selected
       stays in it (its cones are "orphans" of the selected netlist).  The
       sweep in topological order appends them, and a redirection to a
       selected member of a larger id is resolved by the recursion. */
    for ( i = 1; i < Gia_ManObjNum(pGia); i++ )
    {
        pObj = Gia_ManObj( pGia, i );
        if ( Gia_ObjIsAnd(pObj) && !~pObj->Value && Phys_ChoiceObjActive( p, i ) )
            Phys_ChoiceReduce_rec( pNew, pGia, p, pObj, vMap );
    }
    Gia_ManForEachCo( pGia, pObj, i )
    {
        int iLit = Phys_ChoiceResolveFanin( p, Gia_ObjId(pGia, pObj), Gia_ObjFaninLit0p(pGia, pObj) );
        Gia_Obj_t * pFan = Gia_ManObj( pGia, Abc_Lit2Var(iLit) );
        Phys_ChoiceReduce_rec( pNew, pGia, p, pFan, vMap );
        pObj->Value = Gia_ManAppendCo( pNew, Abc_LitNotCond( pFan->Value, Abc_LitIsCompl(iLit) ) );
    }
    Gia_ManSetRegNum( pNew, Gia_ManRegNum(pGia) );
    return pNew;
}

void Phys_ChoicePrint( Phys_Man_t * p, int fVerbose )
{
    int c, a, nAlts = 0, nNodes = 0, nInactive = 0, nSel = 0;
    Vec_Int_t * vCone;
    if ( p == NULL || !p->fHasChoices )
    {
        if ( p != NULL && fVerbose )
            fprintf( stdout, "Choices: the network has no sibling classes.\n" );
        return;
    }
    for ( c = 0; c < p->nClasses; c++ )
    {
        nAlts += p->pClasses[c].nAlts;
        for ( a = 0; a < p->pClasses[c].nAlts; a++ )
        {
            Vec_Int_t * vCone = Phys_ChoiceClassCone( p, c, a );
            nNodes += Vec_IntSize( vCone );
            if ( a == p->pClasses[c].iActive )
                nSel += Vec_IntSize( vCone );
        }
    }
    nInactive = nNodes - nSel;
    fprintf( stdout, "Choices: %d classes, %d alternatives, %d nodes in the cones, "
                     "%d of them inactive (%d of the classes by fanout).\n",
             p->nClasses, nAlts, nNodes, nInactive, p->nClasses );
    if ( !fVerbose )
        return;
    for ( c = 0; c < p->nClasses && c < 10; c++ )
    {
        fprintf( stdout, "Choices: class %d: %d alternatives, selected %d:",
                 c, p->pClasses[c].nAlts, p->pClasses[c].iActive );
        for ( a = 0; a < p->pClasses[c].nAlts; a++ )
        {
            Vec_Int_t * vCone = Phys_ChoiceClassCone( p, c, a );
            fprintf( stdout, " %d%s", Vec_IntSize(vCone),
                     a == p->pClasses[c].iActive ? "*" : "" );
        }
        fprintf( stdout, "\n" );
    }
    (void)vCone;
}

ABC_NAMESPACE_IMPL_END
