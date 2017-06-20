/*
 * $Id: nlmcmrbtree_impl.h,v 1.2.8.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
/*@@NlmCmRBTree Impl Module

   Summary
   SUmmary goes here
 
   Description
   Description goes here
*/
#ifndef NLMPLATFORM_BCM
#include <nlmcmbasic.h>
#include <nlmcmdebug.h>
#include <nlmcmstring.h>
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmdebug.h>
#include <soc/kbp/common/nlmcmstring.h>
#endif

/* #include <nlmcmfile.h> */

/*
 * We know that no path in an rbtree can be longer than twice any other path
 * An rbtree of depth 30 is already very large. We set it to an twice that
 * number so that we cannot possibly fail when running non-recursively
 */
#undef  NLMCMRBMAX_DEPTH
#define NLMCMRBMAX_DEPTH            (30*2)

#ifndef NLMPLATFORM_BCM
#include <nlmcmexterncstart.h>
#else
#include <soc/kbp/common/nlmcmexterncstart.h>
#endif

/* The following are Node based macros. They are macros instead of functions for performance
   reasons. Also, this code has been ported from C++ class and the method names have
   been preserved for maintainability.
*/
#define NlmCmRBNode__GetKey(self)   (&((self)->m_data))
#define NlmCmRBNode__GetParCol(self)    ((nlm_value)((self)->m_parcol_p))
#define NlmCmRBNode__GetParColBit(self) (((nlm_value)((self)->m_parcol_p))&NlmCmRBNodeColorMask)
#define NlmCmRBNode__SetParCol(self,x)  (self)->m_parcol_p = (NlmCmRBNodeType*)(x)
#define NlmCmRBNode__GetColor(self) ((NlmCmRBNodeColor_t)(NlmCmRBNode__GetParColBit(self)))
#define NlmCmRBNode__IsRed(self)        (NlmCmRBNode__GetColor(self) != NlmCmRBNodeColorBlack)
#define NlmCmRBNode__IsBlack(self)  (NlmCmRBNode__GetColor(self) == NlmCmRBNodeColorBlack)

/* Ensure that errant clients can't inadvertently munge color or parent. */
#undef NlmCmRBNode__SetColor
#define NlmCmRBNode__SetColor(self,c)   NlmCmRBNode__SetParCol(self, (c) ? NlmCmRBNode__GetParCol(self)|NlmCmRBNodeColorMask : NlmCmRBNode__GetParCol(self)&~NlmCmRBNodeColorMask)
#undef NlmCmRBNode__SetParent
#define NlmCmRBNode__SetParent(self,p)  NlmCmRBNode__SetParCol(self, (nlm_value)p | NlmCmRBNode__GetColor(self))
#define NlmCmRBNode__Disowned(self) (self)->m_parcol_p = 0
#undef NlmCmRBNode__Print
#ifdef NlmCmRBNodeDataPrint
#define NlmCmRBNode__Print(tree,out,node)   NlmCmRBNodeDataPrint(out, NlmCmRBNode__GetKey(node));
#else
#define NlmCmRBNode__Print(tree,out,node)   (*tree->m_prtfunc_p)(out, NlmCmRBNode__GetKey(node));
#endif

#ifdef NlmCmRBNodeDataCmp
#define NlmCmRBNode__DoCmp(tree,a,b)    NlmCmRBNodeDataCmp(a,b)
#else
#define NlmCmRBNode__DoCmp(tree,a,b)    (*(tree->m_cmpfunc_p))(a,b)
#endif

#define NlmCmRBNode__Cmp(tree,a,b)  NlmCmRBNode__DoCmp(tree,a,b)
#define NlmCmRBNode__IsEqual(tree,a,b)  (NlmCmRBNode__Cmp(tree,a,b) == 0)
#define NlmCmRBNode__IsLess(tree,a,b)   (NlmCmRBNode__Cmp(tree,a,b) < 0)
#define NlmCmRBNode__GetParent(self)    ((NlmCmRBNodeType*)(NlmCmRBNode__GetParCol(self)&~NlmCmRBNodeColorMask))
#define NlmCmRBNode__GetGrandParent(self) NlmCmRBNode__GetParent(NlmCmRBNode__GetParent(self))

/* Macros specific for trees */
#define NlmCmRBTree__Nil(self)          (self)->m_nil_p
#define NlmCmRBTree__IsNotOurNil(self,x)        (x != NlmCmRBTree__Nil(self))

NlmCmRBNodeType *
NlmCmRBNodeMethod(ctor)(
    NlmCmRBNodeType *self
#ifdef NlmCmRBNodeCtorWithData
    , NlmCmRBNodeDataType data
#endif
    )
{
    /* WARNING!!! Keep this code consistent with the create method */

    NlmCmRBNode__CtorBody(self) ;

#ifdef NlmCmRBNodeCtorWithData
    self->m_data = data ;
#endif

#ifdef NlmCmRBNodeCtorHook
    NlmCmRBNodeCtorHook ;
#endif /*## NlmCmRBNodeCtorHook */

    /* RBTree will initialize other fields when the node is inserted. Set parent to 0 now
       to indicate node is not yet managed.
       <Code>
            self->m_left_p = self->u.m_right_p = 0 ;
       </Code>
     */
    return self ;
}

NlmCmRBNodeType *
NlmCmRBNodeMethod(create)(
    NlmCmAllocator *alloc
#ifdef NlmCmRBNodeCtorWithData
    , NlmCmRBNodeDataType data
#endif
    )
{
    NlmCmRBNodeType *node ;
    node = (NlmCmRBNodeType *)NlmCmAllocator__malloc(alloc, sizeof(NlmCmRBNodeType));

    /* We inline the body of the ctor here because this function is called very frequently.
    
       WARNING!!! Keep this code consistent with the ctor method.
    */
    NlmCmRBNode__CtorBody(node) ;

#ifdef NlmCmRBNodeCtorWithData
    node->m_data = data ;
#endif

#ifdef NlmCmRBNodeCtorHook
    NlmCmRBNodeCtorHook ;
#endif /*## NlmCmRBNodeCtorHook */

    return node ;
}

void NlmCmRBNodeMethod(destroy)(
    NlmCmRBNodeType *self,
    NlmCmAllocator *alloc)
{
    if (self) {
    assert(alloc != 0) ;
    assert(NlmCmRBNode__GetParent(self) == 0) ;

#ifdef NlmCmRBNodeDestroyHook
    if(NlmCmRBNode__IsValid(self))
        NlmCmRBNodeDestroyHook ;
#endif /* NlmCmRBNodeDestroyHook */

    NlmCmAllocator__free(alloc,self) ;
    }
}

/********************************************************************/

void
NlmCmRBTreeMethod(SetParanoia)(
    NlmCmRBTreeType *self,
    NlmBool val)
{
    NlmCmRBTree__SetParanoia(self, val);
}

NlmBool
NlmCmRBTreeMethod(GetParanoia)(
    const NlmCmRBTreeType *self)
{
    return NlmCmRBTree__GetParanoia(self);
}

NlmCmRBTreeType *
NlmCmRBTreeMethod(ctor)(
    NlmCmRBTreeType *self,
    NlmCmAllocator *alloc,
    NlmCmRBNodeRemove nodeRemoveFunc
    NLMCMRBCMPFUNC NLMCMRBPRTFUNC
    )
{
#ifdef NlmCmRBNodeCtorWithData
    NlmCmRBNodeDataType junk = NlmCmRBNodeCtorWithData ;
#endif

    self->m_itemCount = 0;
    self->m_paranoia = NlmFalse;
    self->m_alloc_p = alloc;
    self->m_remove_all_hook_p = nodeRemoveFunc ;

#ifndef NlmCmRBNodeDataCmp
    assert(cmpf != 0) ;
    self->m_cmpfunc_p = cmpf ;
#endif

#ifndef NlmCmRBNodeDataPrint
    self->m_prtfunc_p = prtf ;
#endif

#ifdef NlmCmRBNodeCtorWithData
    self->m_nil_p = NlmCmRBNodeMethod(create)(alloc, junk) ;
#else
    self->m_nil_p = NlmCmRBNodeMethod(create)(alloc) ;
#endif

    NlmCm__memset((void*)&self->m_nil_p->m_data, 0, sizeof self->m_nil_p->m_data); /* clear Nil data member */

    self->m_nil_p->m_left_p = 0 ;
    self->m_nil_p->u.m_tree_p = self ;

    /* sentinel Nil must be BLACK -- CLR, page 272 */
    NlmCmRBNode__SetColor(self->m_nil_p, NlmCmRBNodeColorBlack);

    /* Jskud: root must be Nil. */
    self->m_root_p = NlmCmRBTree__Nil(self);

#ifdef NlmCmRBTreeCtorHook
    NlmCmRBTreeCtorHook ;
#endif

    return self ;
}

void
NlmCmRBTreeMethod(dtor)(
    NlmCmRBTreeType *self)
{
#ifdef NlmCmRBTreeDtorHook
    NlmCmRBTreeDtorHook ;
#endif

    if (self->m_nil_p)
    NlmCmRBTreeMethod(RemoveAll)(self); 
    NlmCmRBNode__Disowned(self->m_nil_p);
    NlmCmRBNodeMethod(destroy)(self->m_nil_p, self->m_alloc_p);
}

NlmCmRBTreeType *
NlmCmRBTreeMethod(create)(
    NlmCmAllocator *alloc,
    NlmCmRBNodeRemove nodeRemoveFunc
    NLMCMRBCMPFUNC NLMCMRBPRTFUNC
    )
{
    NlmCmRBTreeType *tree ;

    tree = (NlmCmRBTreeType *)NlmCmAllocator__malloc(alloc, sizeof(NlmCmRBTreeType)) ;
    NlmCmRBTreeMethod(ctor)(tree, alloc, nodeRemoveFunc
#ifndef NlmCmRBNodeDataCmp
               , cmpf
#endif
#ifndef NlmCmRBNodeDataPrint
               , prtf
#endif
    ) ;
    return tree ;
}

void
NlmCmRBTreeMethod(destroy)(
    NlmCmRBTreeType *self)
{
    if (self) {
    NlmCmRBTreeMethod(dtor)(self) ;
    NlmCmAllocator__free(self->m_alloc_p, self) ;
    }
}

NlmCmRBNodeType *
NlmCmRBTreeMethod(GetMin)(
    const NlmCmRBNodeType * x)
{
    assert(NlmCmRBNode__IsValid(x));

    while (NlmCmRBNode__IsValid(x->m_left_p)) {
    x = x->m_left_p;
    }
    /* Intentional cast!  Just cast const away, to keep compiler quiet */
    return (NlmCmRBNodeType *)x;
}

NlmCmRBNodeType *
NlmCmRBTreeMethod(GetMax)(
    const NlmCmRBNodeType * x)
{
    assert(NlmCmRBNode__IsValid(x));

    while (NlmCmRBNode__IsValid(x->u.m_right_p)) {
    x = x->u.m_right_p;
    }
    /* Intentional cast!  Just cast const away, to keep compile quiet */
    return (NlmCmRBNodeType *)x;
}

nlm_u32
NlmCmRBTreeMethod(GetDepth)(
    const NlmCmRBNodeType * x)
{
    nlm_u32 debth = 1;
    assert(NlmCmRBNode__IsValid(x));
    
    while (NlmCmRBNode__IsValid(x->m_left_p)) {
    x = x->m_left_p;
    debth++;
    }
    return debth;
}


NlmCmRBNodeType *
NlmCmRBTreeMethod(GetNext)(
    const NlmCmRBNodeType * x)
{
    NlmCmRBNodeType *y;

    assert(NlmCmRBNode__IsValid(x));

    /* since we're doing an inorder traversal, we go right if we can */
    if (NlmCmRBNode__IsValid(x->u.m_right_p)) {
    return NlmCmRBTreeMethod(GetMin)(x->u.m_right_p);
    }

    /* Could not go right from here, so find ancestor for which we're down its left branch 
       (which is the same as not being down its right branch)
    */
    y = NlmCmRBNode__GetParent(x);
    while (NlmCmRBNode__IsValid(y) && x == y->u.m_right_p) {
    x = y;
    y = NlmCmRBNode__GetParent(y);
    }
    return y;
}

NlmCmRBNodeType *
NlmCmRBTreeMethod(GetPrev)(
    const NlmCmRBNodeType * x)
{
    /* This is the mirror image of GetNext, which see. */
    NlmCmRBNodeType *y;

    assert(NlmCmRBNode__IsValid(x));

    if (NlmCmRBNode__IsValid(x->m_left_p)) {
    return NlmCmRBTreeMethod(GetMax)(x->m_left_p);
    }

    /* Since we're going in reverse, and can't go immediately left, find our ancestor for
       which we're down the right branch, since that's the next node to visit.
    */
    y = NlmCmRBNode__GetParent(x);
    while (NlmCmRBNode__IsValid(y) && x == y->m_left_p) {
    x = y;
    y = NlmCmRBNode__GetParent(y);
    }
    return y;
}

static NlmCmRBNodeType *
NlmCmRBTreeMethod(LocateHead_pvt)(
    const NlmCmRBTreeType *self,
    NlmCmRBNodeType * x,
    const NlmCmRBNodeDataType * k)
{
    NlmCmRBNodeType * prior = 0;
    int cmp ;

    if (NlmCmRBTree__GetParanoia(self))
    NlmCmRBTreeMethod(AssertValid)(self) ;

    while (NlmCmRBNode__IsValid(x) && (0 != (cmp = NlmCmRBNode__Cmp(self, k, NlmCmRBNode__GetKey(x))))) {
    /* We are valid and not equal */
    prior = x;
    if (cmp < 0) {              /* IsLess */
        x = x->m_left_p;
    } else {
        x = x->u.m_right_p;
    }
    }

    if (NlmCmRBNode__IsValid(x)) return x;  /* equal if valid, so return it */
    if (prior == 0) return 0;           /* no valid prior, so no possible head */

    if (NlmCmRBNode__IsLess(self, k, NlmCmRBNode__GetKey(prior)))
    return prior;               /* must be Least Upper Bound */

    prior = NlmCmRBTreeMethod(GetNext)(prior);  /* could be Greatest Lower Bound */
    if (NlmCmRBNode__IsValid(prior) && NlmCmRBNode__IsLess(self, k, NlmCmRBNode__GetKey(prior)))
    return prior;

    return 0;
}

static void
NlmCmRBTreeMethod(TreeInsert)(
    NlmCmRBTreeType *self,
    NlmCmRBNodeType * z)
{
    NlmCmRBNodeType * y = NlmCmRBTree__Nil(self);
    NlmCmRBNodeType * x = self->m_root_p;

    while (NlmCmRBNode__IsValid(x)) {

    /* This is a fatal error, so always check, even in production (e.g., the pins file 
       may be corrupt, but only used in release mode.
    */
    int cmp = NlmCmRBNode__Cmp(self, NlmCmRBNode__GetKey(z), (NlmCmRBNode__GetKey(x))) ;

#if NLM_CMN_DC
    NlmBool ok = (cmp != 0) ;   /* Must not be equal */
PHMD
    if ( !ok ) {
        NlmCmFile__fprintf(NlmCm__stderr, "NlmRBTree Internal Error (brought to you by Mr. Clean)\n");
        NlmCmFile__fprintf(NlmCm__stderr, "         client is attempting to insert duplicate key\n");
        NlmCmRBNode__Print(self, NlmCm__stderr, z);
        NlmCmRBNode__Print(self, NlmCm__stderr, x);
        assert( ok );
    }
#endif  

    y = x;
    if (cmp < 0) {
        x = x->m_left_p;
    } else {
        x = x->u.m_right_p;
    }
    }

    NlmCmRBNode__SetParent(z, y);
    if (NlmCmRBNode__IsNull(y)) {
    self->m_root_p = z;
    } else {
    if (NlmCmRBNode__IsLess(self, NlmCmRBNode__GetKey(z), NlmCmRBNode__GetKey(y))) {
        y->m_left_p = z;
    } else {
        y->u.m_right_p = z;
    }
    }

    /* Jskud: set the pointers to sentinel Nil. Do it here, rather than in constructor,
       since ctor does not know tree, and Nil is tree specific.
    */
    z->m_left_p = z->u.m_right_p = NlmCmRBTree__Nil(self);
}

void
NlmCmRBTreeMethod(LeftRotate)(
    NlmCmRBTreeType *self,
    NlmCmRBNodeType *x)
{
    /* OnEntry:  x's right child is y; AtExit:  x is left child of y */
    NlmCmRBNodeType * y = x->u.m_right_p;
    NlmCmRBNodeType *px;

    /* move y's left child to x's right child */
    x->u.m_right_p = y->m_left_p;
    if (NlmCmRBNode__IsValid(y->m_left_p)) {
    NlmCmRBNode__SetParent(y->m_left_p, x);
    }

    px = NlmCmRBNode__GetParent(x); /* px is parent of x */

    NlmCmRBNode__SetParent(y, px);  /* hook y to x's old parent */

    if (NlmCmRBNode__IsNull(px))        /* point x's parent to y */
    self->m_root_p = y;
    else {
    if (x == px->m_left_p)  {
        px->m_left_p = y;
    } else {
        px->u.m_right_p = y;
    }
    }

    y->m_left_p = x;            /* hook x up as left child of y */
    NlmCmRBNode__SetParent(x, y);
}

void
NlmCmRBTreeMethod(RightRotate)(
    NlmCmRBTreeType *self,
    NlmCmRBNodeType * y)
{
    /* OnEntry:  x is left child of y; OnExit:  x's right child is y */
    NlmCmRBNodeType * x = y->m_left_p;
    NlmCmRBNodeType *py ;

    /* move x's right child to y's left child */
    y->m_left_p = x->u.m_right_p;
    if (NlmCmRBNode__IsValid(y->m_left_p)) {
    NlmCmRBNode__SetParent(y->m_left_p, y);
    }

    py = NlmCmRBNode__GetParent(y); /* py is parent of y */

    NlmCmRBNode__SetParent(x, py);  /* hook x to y's old parent */

    if (NlmCmRBNode__IsNull(py))        /* point y's parent to x */
    self->m_root_p = x;
    else {
    if (y == py->m_left_p)  {
        py->m_left_p = x;
    } else {
        py->u.m_right_p = x;
    }
    }

    x->u.m_right_p = y;         /* hook y up as right child of x */
    NlmCmRBNode__SetParent(y, x);
}

void
NlmCmRBTreeMethod(Adjust)(
    NlmCmRBTreeType *self,
    NlmCmRBNodeType * x)
{
    /* avoid AssertValid() while node may be out of place */
    NlmBool isParanoid = NlmCmRBTree__GetParanoia(self);
    NlmCmRBTree__SetParanoia(self, NlmFalse);
    NlmCmRBTreeMethod(Remove)(self, x);
    NlmCmRBTree__SetParanoia(self, isParanoid);

    NlmCmRBTreeMethod(Insert)(self, x);
}

void
NlmCmRBTreeMethod(Insert)(
    NlmCmRBTreeType *self,
    NlmCmRBNodeType * x)
{
    NlmCmRBNodeType * y;

    assert( x && NlmCmRBNode__GetParent(x)==0 && NlmCmRBTree__IsNotOurNil(self,x) );

    if (NlmCmRBTree__GetParanoia(self))
    NlmCmRBTreeMethod(AssertValid)(self) ;

    self->m_itemCount++;
    NlmCmRBTreeMethod(TreeInsert)(self, x);
    NlmCmRBNode__SetColor(x, NlmCmRBNodeColorRed);

    while (x != self->m_root_p && NlmCmRBNode__IsRed(NlmCmRBNode__GetParent(x))) {
    if (NlmCmRBNode__GetParent(x) == NlmCmRBNode__GetGrandParent(x)->m_left_p) {
        y = NlmCmRBNode__GetGrandParent(x)->u.m_right_p;
        if (NlmCmRBNode__IsRed(y)) {
        NlmCmRBNode__SetColor(NlmCmRBNode__GetParent(x), NlmCmRBNodeColorBlack);
        NlmCmRBNode__SetColor(y, NlmCmRBNodeColorBlack);
        NlmCmRBNode__SetColor(NlmCmRBNode__GetGrandParent(x), NlmCmRBNodeColorRed);
        x = NlmCmRBNode__GetGrandParent(x);
        } else {
        if (x == NlmCmRBNode__GetParent(x)->u.m_right_p) {
            x = NlmCmRBNode__GetParent(x);
            NlmCmRBTreeMethod(LeftRotate)(self, x);
        }
        NlmCmRBNode__SetColor(NlmCmRBNode__GetParent(x), NlmCmRBNodeColorBlack);
        NlmCmRBNode__SetColor(NlmCmRBNode__GetGrandParent(x), NlmCmRBNodeColorRed);
        NlmCmRBTreeMethod(RightRotate)(self, NlmCmRBNode__GetGrandParent(x));
        }
    } else {
        y = NlmCmRBNode__GetGrandParent(x)->m_left_p;
        if (NlmCmRBNode__IsRed(y)) {
        NlmCmRBNode__SetColor(NlmCmRBNode__GetParent(x), NlmCmRBNodeColorBlack);
        NlmCmRBNode__SetColor(y, NlmCmRBNodeColorBlack);
        NlmCmRBNode__SetColor(NlmCmRBNode__GetGrandParent(x), NlmCmRBNodeColorRed);
        x = NlmCmRBNode__GetGrandParent(x);
        } else {
        if (x == NlmCmRBNode__GetParent(x)->m_left_p) {
            x = NlmCmRBNode__GetParent(x);
            NlmCmRBTreeMethod(RightRotate)(self, x);
        }
        NlmCmRBNode__SetColor(NlmCmRBNode__GetParent(x), NlmCmRBNodeColorBlack);
        NlmCmRBNode__SetColor(NlmCmRBNode__GetGrandParent(x), NlmCmRBNodeColorRed);
        NlmCmRBTreeMethod(LeftRotate)(self, NlmCmRBNode__GetGrandParent(x));
        }
    }
    }

    NlmCmRBNode__SetColor(self->m_root_p, NlmCmRBNodeColorBlack);

    if (NlmCmRBTree__GetParanoia(self))
    NlmCmRBTreeMethod(AssertValid)(self) ;
}

static void
NlmCmRBTreeMethod(RemoveAllFromNodeRecursive)(
    NlmCmRBTreeType *self,
    NlmCmRBNodeType * x)
{
    if (NlmCmRBNode__IsValid(x)) {
    NlmCmRBTreeMethod(RemoveAllFromNodeRecursive)(self, x->m_left_p);
    NlmCmRBTreeMethod(RemoveAllFromNodeRecursive)(self, x->u.m_right_p);
    NlmCmRBNode__Disowned(x);
    if (self->m_remove_all_hook_p)
        (*self->m_remove_all_hook_p)(self, x) ;
    NlmCmRBNodeMethod(destroy)(x, self->m_alloc_p);
    }
}

static void
NlmCmRBTreeMethod(RemoveAllFromNode)(
    NlmCmRBTreeType *self,
    NlmCmRBNodeType * x)
{

    if (x && NlmCmRBNode__IsValid(x)) {
    NlmCmAllocator * alloc     = self->m_alloc_p ;
    NlmCmRBNodeType ** deferred = (NlmCmRBNodeType **)
        NlmCmAllocator__malloc(alloc, sizeof(NlmCmRBNodeType *) * NLMCMRBMAX_DEPTH) ;

    if (deferred == 0) {
        NlmCmRBTreeMethod(RemoveAllFromNodeRecursive)(self, x) ;
    } else {
        nlm_u32 n = 0 ;

        /*
         * This loop is essentially emulating a post-order traversal and
         * destruction of the nodes.
         * [Note: actually, looks more like an inorder traversal to me.]
         */

        while (x) {
        NlmCmRBNodeType *tmp ;

        again:
        /* Find the left-most child of this sub-root */
        tmp = x->m_left_p ;
        for ( ; NlmCmRBNode__IsValid(tmp) ; tmp = x->m_left_p ) {

            /* FIRST we use the old value, THEN we increment it;
               therefore, the following assert is correct to
               ensure we don't overindex on the subsequent push
            */
            assert(n < NLMCMRBMAX_DEPTH) ;
            deferred[n++] = x ;         /* Push current */
            x = tmp ;               /* New Current */
        }

        for ( ;; ) {
            tmp = x->u.m_right_p ;
            NlmCmRBNode__Disowned(x);
            if (self->m_remove_all_hook_p)
            (*self->m_remove_all_hook_p)(self, x) ;
            NlmCmRBNodeMethod(destroy)(x, self->m_alloc_p);
            assert(self->m_itemCount-- > 0);    /* ensure we don't delete too many nodes */

            /* This is not strictly Post-order because the parent
             * of this node if any is already deleted
             */
            if (NlmCmRBNode__IsValid(tmp)) {    /* This is new sub-root */
            x = tmp ;
            goto again ;
            }

            if (n) {
            x = deferred[--n] ;     /* Pop */
            } else {
            x = 0 ;
            break ;
            }
        }
        }

        NlmCmAllocator__free(alloc, deferred) ;
        assert(0 == self->m_itemCount);
    }
    }
}

void
NlmCmRBTreeMethod(RemoveAll)(
    NlmCmRBTreeType *self)
{
    /* delete each node in post-order traversal */
    NlmCmRBTreeMethod(RemoveAllFromNode)(self, self->m_root_p);
    self->m_root_p = NlmCmRBTree__Nil(self);
    self->m_itemCount = 0;
}

static void
NlmCmRBTreeMethod(RemoveFixup)(
    NlmCmRBTreeType *self,
    NlmCmRBNodeType *x)
{
    NlmCmRBNodeType *w;
    while (x != self->m_root_p && NlmCmRBNode__IsBlack(x)) {
    if (x == NlmCmRBNode__GetParent(x)->m_left_p) {

        

        w = NlmCmRBNode__GetParent(x)->u.m_right_p;
        if (NlmCmRBNode__IsRed(w)) {
        /* Case 1 */
        NlmCmRBNode__SetColor(w, NlmCmRBNodeColorBlack);
        NlmCmRBNode__SetColor(NlmCmRBNode__GetParent(x), NlmCmRBNodeColorRed);
        NlmCmRBTreeMethod(LeftRotate)(self, NlmCmRBNode__GetParent(x));
        w = NlmCmRBNode__GetParent(x)->u.m_right_p;
        }
        if (NlmCmRBNode__IsBlack(w->m_left_p) && NlmCmRBNode__IsBlack(w->u.m_right_p)) {
        /* Case 2 */
        NlmCmRBNode__SetColor(w, NlmCmRBNodeColorRed);
        x = NlmCmRBNode__GetParent(x);
        } else {
        if (NlmCmRBNode__IsBlack(w->u.m_right_p)) {
            /* Case 3 */
            NlmCmRBNode__SetColor(w->m_left_p, NlmCmRBNodeColorBlack);
            NlmCmRBNode__SetColor(w, NlmCmRBNodeColorRed);
            NlmCmRBTreeMethod(RightRotate)(self, w);
            w = NlmCmRBNode__GetParent(x)->u.m_right_p;
        }
        /* Case 4 */
        NlmCmRBNode__SetColor(w, NlmCmRBNode__GetColor(NlmCmRBNode__GetParent(x)));
        NlmCmRBNode__SetColor(NlmCmRBNode__GetParent(x), NlmCmRBNodeColorBlack);
        NlmCmRBNode__SetColor(w->u.m_right_p, NlmCmRBNodeColorBlack);
        NlmCmRBTreeMethod(LeftRotate)(self, NlmCmRBNode__GetParent(x));
        x = self->m_root_p;
        }
        
    }
    else {
        
        w = NlmCmRBNode__GetParent(x)->m_left_p;
        if (NlmCmRBNode__IsRed(w)) {
        /* Case 1 */
        NlmCmRBNode__SetColor(w, NlmCmRBNodeColorBlack);
        NlmCmRBNode__SetColor(NlmCmRBNode__GetParent(x), NlmCmRBNodeColorRed);
        NlmCmRBTreeMethod(RightRotate)(self, NlmCmRBNode__GetParent(x));
        w = NlmCmRBNode__GetParent(x)->m_left_p;
        }
        if (NlmCmRBNode__IsBlack(w->u.m_right_p) && NlmCmRBNode__IsBlack(w->m_left_p)) {
        /* Case 2 */
        NlmCmRBNode__SetColor(w, NlmCmRBNodeColorRed);
        x = NlmCmRBNode__GetParent(x);
        } else {
        if (NlmCmRBNode__IsBlack(w->m_left_p)) {
            /* Case 3 */
            NlmCmRBNode__SetColor(w->u.m_right_p, NlmCmRBNodeColorBlack);
            NlmCmRBNode__SetColor(w, NlmCmRBNodeColorRed);
            NlmCmRBTreeMethod(LeftRotate)(self, w);
            w = NlmCmRBNode__GetParent(x)->m_left_p;
        }
        /* Case 4 */
        NlmCmRBNode__SetColor(w, NlmCmRBNode__GetColor(NlmCmRBNode__GetParent(x)));
        NlmCmRBNode__SetColor(NlmCmRBNode__GetParent(x), NlmCmRBNodeColorBlack);
        NlmCmRBNode__SetColor(w->m_left_p, NlmCmRBNodeColorBlack);
        NlmCmRBTreeMethod(RightRotate)(self, NlmCmRBNode__GetParent(x));
        x = self->m_root_p;
        }
        
    }
    }
    NlmCmRBNode__SetColor(x, NlmCmRBNodeColorBlack);
}

void
NlmCmRBTreeMethod(Remove)(
    NlmCmRBTreeType *self,
    NlmCmRBNodeType * z)
{
    NlmCmRBNodeType * y;
    NlmCmRBNodeType * x;
    NlmCmRBNodeType *py;
    NlmCmRBNodeColor_t yColor ;

    /* Amazing that Remove() is key insensitive (other than AssertValid())!  We depend
       on this to implement Adjust().
    */
    assert(z && NlmCmRBNode__IsValid(z));
    if (NlmCmRBTree__GetParanoia(self))
    NlmCmRBTreeMethod(AssertValid)(self) ;

    self->m_itemCount--;

    /* determine y, the node we'll splice out */
    if (NlmCmRBNode__IsNull(z->m_left_p) || NlmCmRBNode__IsNull(z->u.m_right_p)) {
    /* z has few enough children that we can remove it */
    y = z;
    }
    else {
    /* z is full, so we'll replace z in the tree with y; y is easy to remove: it's z's
       successor, and has no left child
    */
    y = NlmCmRBTreeMethod(GetNext)(z);
    }

    /* invariant: y has at most one child, so it's easy to splice y out; set x to the child
       which will fill in for y
    */
    if (NlmCmRBNode__IsValid(y->m_left_p)) {
    x = y->m_left_p;
    } else {
    x = y->u.m_right_p;
    }

    /* Jskud: change from book -- grab y's parent now, but don't update x's parent yet -- need
       to delay since using sentinel (see below), and may need to change apparent parent
       (since splicing).
    */
    py = NlmCmRBNode__GetParent(y);

    /* Adjust the ancestor pointer to x, the new child. */
    if (NlmCmRBNode__IsNull(py)) {
    self->m_root_p = x;
    } else {
    if (y == py->m_left_p) {
        py->m_left_p = x;
    } else {
        py->u.m_right_p = x;
    }
    }

    /* We may not change the value of the key in a node -- others will be pointing at that
       value. Changing the pointers <B>within</B> the tree may not change the pointers
       <B>into</B> the tree.
       
       Therefore, we must deviate from the book (which would overwrite the key and leave z
       in the tree); rather than overwrite z with y's key, we splice y in place of z, so that
       y's key remains in the same place. Cf. CLR, page 254, Exercise 13.3-5.
    */
    yColor = NlmCmRBNode__GetColor(y);

    if (y != z) {
    NlmCmRBNodeType *pz ;

    /* have y take on z's values */
    NlmCmRBNode__SetColor(y, NlmCmRBNode__GetColor(z));
    NlmCmRBNode__SetParent(y, NlmCmRBNode__GetParent(z));
    y->m_left_p = z->m_left_p;
    y->u.m_right_p = z->u.m_right_p;

    /* update bidirectional links */
    pz = NlmCmRBNode__GetParent(z);
    if (NlmCmRBNode__IsNull(pz)) {
        self->m_root_p = y;
    } else {
        if (z == pz->m_left_p){
        pz->m_left_p = y;
        } else {
        pz->u.m_right_p = y;
        }
    }
    NlmCmRBNode__SetParent(y->m_left_p, y);
    NlmCmRBNode__SetParent(y->u.m_right_p, y);

    /* since y is taking the place of z, if we were pointing to z, we must now point
       to y (thank goodness for testing)
    */
    if (py == z) py = y;
    }

    
    NlmCmRBNode__SetParent(x, py);

    if (yColor == NlmCmRBNodeColorBlack) {
    NlmCmRBTreeMethod(RemoveFixup)(self, x);
    }

    if (NlmCmRBTree__GetParanoia(self))
    NlmCmRBTreeMethod(AssertValid)(self) ;
    
    /* Disown to indicate that removed, allowing dtor to fire cleanly, or to allow
       subsequent Insert().
    */
    NlmCmRBNode__Disowned(z);
}

#if NLM_CMN_DC
static void
NlmCmRBTreeMethod(PrintNodeRecursive)(
    const NlmCmRBTreeType *self,
    const NlmCmRBNodeType *x,
    NlmCmFile *out,
    int indent)
{
    int i ;
    if (NlmCmRBNode__IsNull(x)) return;

    NlmCmRBTreeMethod(PrintNodeRecursive)(self, x->m_left_p, out, indent+4);

    for (i = 0; i < indent; i++) NlmCmFile__fputc(out,' ');
    NlmCmFile__fprintf(out, "%s", (NlmCmRBNode__IsRed(x) ? "Red   " : "Black "));
    NlmCmRBNode__Print(self, out, x);
    NlmCmFile__fputc(out,'\n');

    NlmCmRBTreeMethod(PrintNodeRecursive)(self, x->u.m_right_p, out, indent+4);
}

static void
NlmCmRBTreeMethod(PrintNode)(
    const NlmCmRBTreeType *self,
    const NlmCmRBNodeType *x,
    NlmCmFile *out,
    int indent)
{
    if (x && !NlmCmRBNode__IsNull(x)) {
    typedef struct stkItem
    {
        const NlmCmRBNodeType *x ;
        int indent ;
    } stkItem ;
    NlmCmAllocator * alloc     = self->m_alloc_p ;
    stkItem * deferred = (stkItem *)
        NlmCmAllocator__malloc(alloc, sizeof(stkItem) * NLMCMRBMAX_DEPTH) ;

/*  NlmCmRBTreeMethod(PrintNodeRecursive) (self, x, out, indent) ;*/
/*  fprintf(out, "-------------------------------\n") ;*/
    if (deferred == 0) {
        NlmCmRBTreeMethod(PrintNodeRecursive) (self, x, out, indent) ;
    } else {
        nlm_u32 n = 0 ;
        int i ;

        /*
         * This loop is essentially emulating a in-order traversal and
         * printing of the nodes
         */
        while (x) {
        const NlmCmRBNodeType *l ;
        
        assert(n < NLMCMRBMAX_DEPTH) ;
        deferred[n].x = x ; deferred[n].indent = indent ; n++   ;       /* PUSH */

        l = x->m_left_p ;
        while (NlmCmRBNode__IsValid(l)) {
            assert(n < NLMCMRBMAX_DEPTH) ;
            indent += 4 ;
            deferred[n].x = l ; deferred[n].indent = indent ; n++ ;     /* PUSH */
            l = l->m_left_p ;
        }

        do {
            n-- ; x = deferred[n].x ; indent = deferred[n].indent ;     /* POP */
            for (i = 0; i < indent; i++) NlmCmFile__fputc(out, ' ');
            NlmCmFile__fprintf(out, "%s", (NlmCmRBNode__IsRed(x) ? "Red   " : "Black "));
            NlmCmRBNode__Print(self, out, x);
            NlmCmFile__fputc(out, '\n');

            x = x->u.m_right_p ;
            if (NlmCmRBNode__IsValid(x)) {
            indent += 4 ;
            break ;
            } else
            x = 0 ;
        } while (n) ;
        }
        NlmCmAllocator__free(alloc, (void *)deferred) ;
    }
    }
}

void
NlmCmRBTreeMethod(Print)(
    const NlmCmRBTreeType *self,
    NlmCmFile *out)
{
#ifndef NlmCmRBNodeDataPrint
    if (!self->m_prtfunc_p) {
    assert(0) ;     /* No function for printing given!!! */
    return ;
    }
#endif
    
    NlmCmFile__fprintf(out, ">>> ---NlmCmRBTree__PrintTree starting--- <<<\n");
    NlmCmRBTreeMethod(PrintNode)(self, self->m_root_p, out, 0);
    NlmCmFile__fprintf(out, "<<< ---NlmCmRBTree__PrintTree finished--- >>>\n");
}

#endif /* #if 0 */

static int
NlmCmRBTreeMethod(VerifyHeight)(
    int bh, int refbh)
{
    if (refbh == -1) refbh = bh;
    if (!(refbh == bh)) {
    NlmCmFile__printf("ERROR: black heights differ; expect: %d; actual: %d\n",
           refbh, bh) ;
    }
    return refbh ;
}


static const NlmCmRBNodeType *
NlmCmRBTreeMethod(VerifyNodeRecursive)(
    NlmCmRBTreeType *self,
    NlmCmRBNodeType * x,
    int bh, int *refbh,
    const NlmCmRBNodeType * prior)
{
    NlmCmRBNodeType *child;

    if (NlmCmRBNode__IsNull(x)) {
    assert(x == NlmCmRBTree__Nil(self));        /* Every Nil is our unique Nil. */
    assert(NlmCmRBNode__IsBlack(x));            /* Every Nil node (only one) is black */
    *refbh=NlmCmRBTreeMethod(VerifyHeight)(bh,*refbh);  /* Every path has the same black height */
    return prior;
    }

    assert(NlmCmRBTreeMethod(GetTree(x))==self);    /* make sure we can find ourself */

    child = x->m_left_p;            /* ensure our left child points to us */
    if (!(NlmCmRBNode__IsNull(child) || NlmCmRBNode__GetParent(child) == x)) {
    NlmCmFile__printf("ERROR: child does not point to parent.\n");
    /* NlmCmRBTreeMethod(PrintNode)(self, x, NlmCm__stdout, 0); */
    }
    child = x->u.m_right_p;         /* ensure our right child points to us */
    assert(NlmCmRBNode__IsNull(child) || NlmCmRBNode__GetParent(child) == x);

    if (NlmCmRBNode__IsRed(x)) {
    /* Every child of a red node is black */
    if (!(NlmCmRBNode__IsBlack(x->m_left_p) && NlmCmRBNode__IsBlack(x->u.m_right_p))) {
        NlmCmFile__printf("ERROR: red node has non black children.\n");
        /* NlmCmRBTreeMethod(PrintNode)(self, x, NlmCm__stdout, 0); */
    }
    } else {
    bh++;
    }

    /* We do not have a way to find out if the node's contents are valid (we used to have
       this in MFC as a virtual function on every object)
       <Code>
       AssertValid(x);  // make sure this node itself is OK
       </Code>
    */

    prior = NlmCmRBTreeMethod(VerifyNodeRecursive)(self, x->m_left_p, bh, refbh, prior);

    /* Keys are in Binary Search Tree such that inorder walk visits in order.

       FYI: prior might not be set if this is the first node
    */


    /* Use this if we are allowed to have duplicates (which is not supported right now)
      
       if (prior)
       assert(NlmCmRBNode__IsLess(self, prior, NlmCmRBNode__GetKey(x)) ||
       NlmCmRBNode__IsEqual(self, prior, NlmCmRBNode__GetKey(x)));
    */

    if (prior) {
#if NETL_DEBUG_LEVEL >= NETL_DEBUG_ASSERT
    /* split up the assert, to avoid 
           warning: string length `587' is greater than the length `509'
           ISO C89 compilers are required to support 
    */
    const NlmCmRBNodeDataType *pkey, *xkey ;
    pkey = NlmCmRBNode__GetKey(prior) ;
    xkey = NlmCmRBNode__GetKey(x) ;
    assert(NlmCmRBNode__IsLess(self, pkey, xkey)) ;
#endif
    }

    return NlmCmRBTreeMethod(VerifyNodeRecursive)(self, x->u.m_right_p, bh, refbh, x);
}

static const NlmCmRBNodeType *
NlmCmRBTreeMethod(VerifyNode)(
    NlmCmRBTreeType *self,
    NlmCmRBNodeType * x,
    int bh, int *refbh,
    const NlmCmRBNodeType * prior)
{
    typedef struct stkItem
    {
    NlmCmRBNodeType * x ;
    int bh ;
    const NlmCmRBNodeType * prior ;
    } stkItem ;

/*
 * Uncommend the following to verify this non-recursive verify against the simple
 * recursive one.
 */
/*#define NLMRB_DOUBLE_VFY*/
#ifdef NLMRB_DOUBLE_VFY
    int refbhvfy = -1 ;
    const NlmCmRBNodeType * vfy ;
    static int Count = 0 ;
#endif

    NlmCmRBNodeType *child ;
    NlmCmAllocator * alloc     = self->m_alloc_p ;
    stkItem* deferred = (stkItem*)
    NlmCmAllocator__malloc(alloc, sizeof(stkItem) * NLMCMRBMAX_DEPTH) ;
    nlm_u32 n = 0 ;

    if (deferred == 0) {
    return NlmCmRBTreeMethod(VerifyNodeRecursive)(self, x, bh, refbh, prior) ;
    }

#ifdef NLMRB_DOUBLE_VFY
    Count++ ;
    if (Count == -1)
    NlmCmDebug__Break() ;
    vfy = NlmCmRBTreeMethod(VerifyNodeRecursive)(self, x, bh, &refbhvfy, prior) ;
#endif

    while (x) {
    again:
    if (NlmCmRBNode__IsNull(x)) {
        assert(x == NlmCmRBTree__Nil(self));        /* Every Nil is our unique Nil. */
        assert(NlmCmRBNode__IsBlack(x));        /* Every Nil node (only one) is black */
        *refbh = NlmCmRBTreeMethod(VerifyHeight)(bh,*refbh);    /* Every path has the same black height */
    } else {

        assert(NlmCmRBTreeMethod(GetTree(x))==self);    /* make sure we can find ourself */

        child = x->m_left_p;            /* ensure our left child points to us */
        if (!(NlmCmRBNode__IsNull(child) || NlmCmRBNode__GetParent(child) == x)) {
        NlmCmFile__printf("ERROR: child does not point to parent.\n");
        /* NlmCmRBTreeMethod(PrintNode)(self, x, NlmCm__stdout, 0); */
        }
        child = x->u.m_right_p;         /* ensure our right child points to us */
        assert(NlmCmRBNode__IsNull(child) || NlmCmRBNode__GetParent(child) == x);

        if (NlmCmRBNode__IsRed(x)) {
        /* Every child of a red node is black */
        if (!(NlmCmRBNode__IsBlack(x->m_left_p) && NlmCmRBNode__IsBlack(x->u.m_right_p))) {
            NlmCmFile__printf("ERROR: red node has non black children.\n");
            /* NlmCmRBTreeMethod(PrintNode)(self, x, NlmCm__stdout, 0); */
        }
        } else {
        bh++;
        }

        assert(n < NLMCMRBMAX_DEPTH) ;
        deferred[n].x = x ;
        deferred[n].bh = bh ;
        deferred[n].prior = prior ;
        n++ ;

        x = x->m_left_p ;
        goto again ;
    }

    if (n) {
        n-- ;
        x = deferred[n].x ;
        bh = deferred[n].bh ;
        /* We do not pop prior because that is the return value of the
         * left-child recursion
         */

#if NETL_DEBUG_LEVEL >= NETL_DEBUG_ASSERT
        if (prior) {
        /* split up the assert, to avoid 
           warning: string length `587' is greater than the length `509'
           ISO C89 compilers are required to support 
        */
        const NlmCmRBNodeDataType *pkey, *xkey ;
        pkey = NlmCmRBNode__GetKey(prior) ;
        xkey = NlmCmRBNode__GetKey(x) ;
        assert(NlmCmRBNode__IsLess(self, pkey, xkey)) ;
        }
#endif
        prior = x ;
        x = x->u.m_right_p ;
    } else
        break ;
    }
    NlmCmAllocator__free(alloc, deferred) ;

#ifdef NLMRB_DOUBLE_VFY
    assert(prior == vfy) ;
    assert(*refbh == refbhvfy) ;
#endif

    return prior ;
}

void
NlmCmRBTreeMethod(AssertValid)(
    const NlmCmRBTreeType *self)
{
    /* NB: must avoid premature AssertValid() -- tree considered complete before root node
       has been full loaded; eg, consider this scenario:

       * propset loading name
       * name loading namenode
       * namenode loading parent
       * some ... parent namenode loading (root) namenode
       * (root) namenode loading ... children
       * some ... child loading sentinel
       * sentinel loading tree
       * tree points to root (in progress, considered complete)
       * tree points to sentinel (in progress, considered complete)
       * sentinel loading tree considered complete,
       so validates tree (with incomplete root node)

       So, add an extra field to determine if loading -- client must reset when
       entire tree known to be loaded.
     */

    NlmCmRBTreeType* Self = (NlmCmRBTreeType*)(self);
    int refbh = -1 ;

    assert(NlmCmRBNode__IsBlack(self->m_root_p));   /* root must be black
                               could be Nil, but Nil is black too */

    assert(NlmCmRBNode__IsNull(NlmCmRBTree__Nil(self)));    /* Nil must be NlmCmRBNode__IsNull() */
    assert(!NlmCmRBNode__IsValid(NlmCmRBTree__Nil(self)));/* and not valid */
    assert(NlmCmRBNode__IsBlack(NlmCmRBTree__Nil(self)));   /* and black */

    /* root is Nil, or root's parent pointer is Nil */
    assert(NlmCmRBNode__IsNull(self->m_root_p) || NlmCmRBNode__IsNull(NlmCmRBNode__GetParent(self->m_root_p)));

    NlmCmRBTreeMethod(VerifyNode)(Self, self->m_root_p, 0, &refbh, 0);
}


NlmCmRBNodeType *
NlmCmRBTreeMethod(Locate)(
    const NlmCmRBTreeType *self,
    const NlmCmRBNodeDataType* k)
{
    int cmp ;
    NlmCmRBNodeType * x = self->m_root_p ;
    if (NlmCmRBTree__GetParanoia(self))
    NlmCmRBTreeMethod(AssertValid)(self) ;

    while (NlmCmRBNode__IsValid(x) && (0 != (cmp = NlmCmRBNode__Cmp(self, k, NlmCmRBNode__GetKey(x))))) {
    /* We are valid and not equal */
    if (cmp < 0) {              /* IsLess */
        x = x->m_left_p;
    } else {
        x = x->u.m_right_p;
    }
    }

    /* Jskud: Nil is tree specific; translate to global NULL for clients */
    return NlmCmRBNode__IsValid(x) ? x : 0;
}

NlmCmRBNodeType *
NlmCmRBTreeMethod(LocateHead)(
    const NlmCmRBTreeType *self,
    const NlmCmRBNodeDataType* k)
{
    return (k == 0) ? NlmCmRBTreeMethod(LocateFirst)(self) :
    NlmCmRBTreeMethod(LocateHead_pvt)(self, self->m_root_p, k) ;
}

NlmCmRBNodeType *
NlmCmRBTreeMethod(LocateFirst)(
    const NlmCmRBTreeType *self)
{
    return NlmCmRBNode__IsValid(self->m_root_p) ? NlmCmRBTreeMethod(GetMin)(self->m_root_p) : 0;
}

NlmCmRBTreeType *
NlmCmRBTreeMethod(GetTree)(
    const NlmCmRBNodeType* x)
{
    return (NlmCmRBTreeMethod(GetMin)(x)->m_left_p)->u.m_tree_p;
}

/*********************************************************************/
NlmCmRBItorType *
NlmCmRBItorMethod(ctor)(
    NlmCmRBItorType *self,
    const NlmCmRBTreeType *base)
{
    self->m_base_p = base ;
    self->m_here_p = 0 ;
    return self;
}

NlmCmRBItorType *
NlmCmRBItorMethod(create)(
    NlmCmAllocator *alloc,
    const NlmCmRBTreeType *base)
{
    NlmCmRBItorType *itor ;

    itor = (NlmCmRBItorType *)NlmCmAllocator__malloc(alloc, sizeof(NlmCmRBItorType)) ;
    return NlmCmRBItorMethod(ctor)(itor, base) ;
}

void
NlmCmRBItorMethod(destroy)(
    NlmCmRBItorType *self,
    NlmCmAllocator *alloc)
{
    NlmCmAllocator__free(alloc, self) ;
}

void
NlmCmRBItorMethod(SetHere)(
    NlmCmRBItorType *self,
    const NlmCmRBNodeType *location)
{
    if  (location == 0) {
    self->m_here_p = 0;
    } else {
    NlmBool ok = NlmCmRBTreeMethod(GetTree)(location) == self->m_base_p ;
    assert(ok);
    if (ok) self->m_here_p = (NlmCmRBNodeType *)(location);
    }
}

NlmCmRBNodeType *
NlmCmRBItorMethod(Next)(
    NlmCmRBItorType *self)
{
    if (self->m_here_p == 0) {
    self->m_here_p = NlmCmRBTreeMethod(LocateFirst)(self->m_base_p);
    } else {
    self->m_here_p = NlmCmRBTreeMethod(GetNext)(self->m_here_p);
    /* if we've run our course, then return a clean 0 */
    if (NlmCmRBNode__IsNull(self->m_here_p))
        self->m_here_p = 0;
    }
    return self->m_here_p;
}

void
NlmCmRBItorMethod(Reset)(
    NlmCmRBItorType *self)
{
    self->m_here_p = 0 ;
}
#ifndef NLMPLATFORM_BCM
#include <nlmcmexterncend.h>
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif

