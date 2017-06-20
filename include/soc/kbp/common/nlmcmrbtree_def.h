/*
 * $Id: nlmcmrbtree_def.h,v 1.2.8.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
/*@@NlmCmRBTree Define Module

   Summary
   This module is sort of a template that can be used to create multiple implementations of
   NlmCmRBTrees. As such, we do not protect ourselves from multiple includes.

   Description 
   It is required that for any given tree, it and all of its nodes share an allocator. Any
   calls to a tree or its nodes' destroy function must be passed to the allocator they were
   allocated with.

   To use this template, the following macros are needed (some are optional):

    * NlmCmRBNodeDataType - This macro represents the data type stored in each element
      of the tree.

    * NlmCmRBNameSuffix - This macro is used to give the implemenation a suffix. This
      is used to generate all typedef/struct and function names. The value of this macro must
      be a simple string that can be appended to some prefix to together form a valid C name.
 
    * NlmCmRBNamePrefix (optional) - By default all types, functions, etc. defined here will have
      a suffix NlmCmRB. However, this can be overridden using this macro.
    
    * NlmCmRBNodeCtorWithData (optional) - If this macro is defined the ctor/create functions for
      the node will accept an additional argument for the data. This is useful if the data type
      is small (easily passed on the stack). If the data type is not a pointer or a simple
      scalar, do not set this macro. The value of this macro must be the default value that can
      be used in the creation of a sentinel (it can be anything so long as it is the right type)

    * NlmCmRBNodeDataCmp(a,b) (optional) - This is an optional macro or function that compares
      two items. The result must be similar to strcmp (0 for equal, >0 means greater, etc.). This
      macro/function is given two arguments for comparison. The signature for this is as follows:
      <Code>
            int NlmCmRBNodeDataCmp(const NlmCmRBNodeDataType *a, 
                      const NlmCmRBNodeDataType *b) ;</Code>
      <PRE>
      It may be easier to use a #define to redirect the comparison to an actual function if the
      comparison cannot be done inline. Example:
      </PRE>
      <Code>
            #define NlmCmRBNodeDataCmp(a,b) myfunc((a), (b))</Code>
      <PRE>
      Of course if the NlmCmRBNodeDataType is a simple type the entire comparison can be done
      inline. if the datatype is an int for example:
      </PRE>
      <Code>
            #define NlmCmRBNodeDataCmp(a,b) ((*(a) == *(b))? 0 : ((*(a) < *(b)) ? -1 : 1))</Code>
      <PRE>
      If this macro is not defined, then the tree ctor/create will have an extra argument that
      is a compare function. This must be supplied at runtime
      </PRE>

    * NlmCmRBNodeDataPrint(f,a) (optional) - This is an optional macro to print the data. The signature for this is as follows:
      <Code>
            void NlmCmRBNodeDataPrint(NlmCmFile *f, const NlmCmRBNodeDataType *b) ;</Code>
      <PRE>
      For example, if the NlmCmRBNodeDataType is an int, you could do
      </PRE>
      <Code>
            #define NlmCmRBNodeDataPrint(f,a) NlmCmFile__fprintf(f,"%d\n", (a)->m_data)</Code>
      <PRE>
      If this macro is not defined, printing will not be supported.
      </PRE>

     * NlmCmRBNodeCtorHook (optional) - Use this macro to insert your own code into the auto-generated node constructor. Your code
       will be inserted at the end of the normal ctor code. The variable 'self' which is the
       pointer to the node structure is available to this macro (available in scope). For example:
       <Code>
            #define NlmCmRBNodeCtorHook   nodeInit(self)</Code>

     * NlmCmRBNodeDestroyHook (optional) - Use this macro to insert your own code into the auto-generated node destroy function.
       Your code will be inserted at the beginning of the normal destroy code. The variables
       'self', which is the pointer to the node structure, and 'alloc', which is the pointer to
       a NlmCmAllocator are available to this macro (available in scope).

     * NlmCmRBTreeCustomFields (optional) - If this macro is defined, any custom fields will be literally inserted. You can use this
       to create custom fields for your implementation of the tree. The default implementation
       does not touch these fields.

     * NlmCmRBTreeCtorHook (optional) - Use this macro to insert your own code into the auto-generated constructor. Your code
       will be placed at the end of the normal ctor code. The variable 'self' which is the
       pointer to the tree structure is available to this macro (available in scope). For example:
       <Code>
            #define NlmCmRBTreeCtorHook myInit(self)</Code>

     * NlmCmRBTreeDtorHook (optional) - Use this macro to insert your own code into the auto-generated destructor. Your code will
       be placed at the beginning of the normal dtor code. The variable 'self' which is the
       pointer to the tree structure is available to this macro (available in scope).

   These macros are undefined at the bottom of these files unless the macro
   IMPLEMENTING_NLMCMRBTREE is defined. This is done so that this template can
   be used by multiple tree implementations. However, the macro
   IMPLEMENTING_NLMCMRBTREE must be defined when the implementation C code
   itself is being compiled.
  
   See Also
   NlmCmrbtree_uint32.[hc] for an example implementation
  
   Note:
   We could have used virtual functions (func. ptrs) registered with the tree but since this
   tree code is typically used in a performance sensitive area, we chose to use macros for
   specialization.

   Once this template is used, you get a set of functions with the following prefixes (assume
   that the tree node suffix is XXX. See NlmCmRBNameSuffix above).
   <PRE>
    NlmCmRBNodeXXX__   -- functions that operate on a node
    NlmCmRBTreeXXX__   -- functions that operate on the tree
    NlmCmRBItorXXX__   -- functions that can iterate on NlmCmRBTreeXXX
   </PRE>

   Associated with the above mentioned functions, you have the corresponding types NlmCmRBNodeXXX,
   NlmCmRBTreeXXX, and NlmCmRBItorXXX.
 */
#ifndef NLMPLATFORM_BCM
#include <nlmcmbasic.h>
#include <nlmcmallocator.h>
#include <nlmcmfile.h>
#include <nlmcmexterncstart.h> /* this should be the final include */
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmallocator.h>
#include <soc/kbp/common/nlmcmfile.h>
#include <soc/kbp/common/nlmcmexterncstart.h>
#endif



#ifndef NLMCMRBTREE_ONCE_DEFINED
#define NLMCMRBTREE_ONCE_DEFINED    1

/* We need to define these macros/types once and they will be availale for every implementation
   of this template.
 */
typedef enum NlmCmRBNodeColor_t
{
  /* NB: these specific encodings are hardwired below -- don't change, unless you change the
     color related methods.
     */
  NlmCmRBNodeColorBlack     = 0,
  NlmCmRBNodeColorRed       = 1,
  NlmCmRBNodeColorMask  = 0x1
} NlmCmRBNodeColor_t ;

#define NlmCmRBConcat2(a,b)     NlmCmConcat2_(a,b)
#define NlmCmRBConcat3(a,b,c)       NlmCmConcat3_(a,b,c)

#define NlmCmRBNodeMake(type)       NlmCmRBConcat3(NlmCmRBNamePrefix,Node,type)
#define NlmCmRBTreeMake(type)       NlmCmRBConcat3(NlmCmRBNamePrefix,Tree,type)
#define NlmCmRBItorMake(type)       NlmCmRBConcat3(NlmCmRBNamePrefix,Itor,type)

#define NlmCmRBNode__IsNull(x)      ((x)->m_left_p == 0)
#define NlmCmRBNode__IsValid(x)     ((x)->m_left_p != 0)

#define NlmCmRBNode__CtorBody(x)        (x)->m_parcol_p = 0

#define NlmCmRBNode__Left(x)        (x)->m_left_p
#define NlmCmRBNode__Right(x)       (x)->u.m_right_p

#endif
/* End of one-time definitions */

#ifndef NlmCmRBNamePrefix
#define NlmCmRBNamePrefix       NlmCmRB
#endif

#define NlmCmRBNodeType         NlmCmRBNodeMake(NlmCmRBNameSuffix)
#define NlmCmRBTreeType         NlmCmRBTreeMake(NlmCmRBNameSuffix)
#define NlmCmRBItorType         NlmCmRBItorMake(NlmCmRBNameSuffix)
#define NlmCmRBNodeCmpType      NlmCmRBConcat2(NlmCmRBNodeType,Cmp_t)
#define NlmCmRBNodePrintType        NlmCmRBConcat2(NlmCmRBNodeType,Print_t)
#define NlmCmRBNodeRemove       NlmCmRBConcat2(NlmCmRBNodeType,DestroyHook_t)

#define NlmCmRBNodeMethod(m)        NlmCmRBConcat3(NlmCmRBNodeType,__,m)
#define NlmCmRBTreeMethod(m)        NlmCmRBConcat3(NlmCmRBTreeType,__,m)
#define NlmCmRBItorMethod(m)        NlmCmRBConcat3(NlmCmRBItorType,__,m)

typedef struct NlmCmRBNodeType NlmCmRBNodeType ;
typedef struct NlmCmRBTreeType NlmCmRBTreeType ;
typedef struct NlmCmRBItorType NlmCmRBItorType ;


typedef int  (*NlmCmRBNodeCmpType)(const NlmCmRBNodeDataType *a, const NlmCmRBNodeDataType *b);

typedef void (*NlmCmRBNodePrintType)(NlmCmFile *, const NlmCmRBNodeDataType *a);

/* Summary
   Used to specialize an implementation of a NlmCmRBTree for a particular data type.
   
   Description
   As a for instance, common/base contains an implementation of a NlmCmRBTree that stores
   pointers (see nlmcmrbtree_ptr.h). Since these pointers are void*, they can point to arbitrary
   data. If someone used the pointer tree, they might put in pointers to data types that need
   to be cleaned up. That user would then define a function matching the signature of
   NlmCmRBNodeRemove and pass it to the create function of the NlmCmRBTree. The NlmCmRBNodeRemove
   function the user defined would then be used as an additional helper function to tear down
   a nodes data as the node is removed from the tree.
 */
typedef void (*NlmCmRBNodeRemove)(NlmCmRBTreeType *, NlmCmRBNodeType *a);

struct NlmCmRBNodeType
{
  NlmCmRBNodeType *     m_parcol_p; /* unusual name to thwart bogus access */
  NlmCmRBNodeType *     m_left_p;   /* Left child */

  /* Summary
     Sentinel node to represent Nil (at the leaves).
       
     Description
     For the sentinel node, we use the convention that the left child is NULL and the space
     for the right-child is shared by a ptr to the tree that this sentinel belongs to. Note
     that for a non NULL node, the left and the right children are always other nodes
     (sentinels or otherwise).
      
     NB: sentinel parent pointer updated during tree manipulations;
     therefore, node must be Disowned() prior to dtor.
  */
  union {
    NlmCmRBNodeType *   m_right_p;  /* Right child */
    NlmCmRBTreeType *   m_tree_p ;  /* Only used by the sentinel */
  } u ;

  NlmCmRBNodeDataType       m_data ;    /* The data for this node */
};

#ifdef NlmCmRBNodeCtorWithData
extern NlmCmRBNodeType *    NlmCmRBNodeMethod(ctor)(NlmCmRBNodeType *self, NlmCmRBNodeDataType data) ;
extern NlmCmRBNodeType *    NlmCmRBNodeMethod(create)(NlmCmAllocator *alloc, NlmCmRBNodeDataType data) ;
#else
extern NlmCmRBNodeType *    NlmCmRBNodeMethod(ctor)(NlmCmRBNodeType *self) ;
extern NlmCmRBNodeType *    NlmCmRBNodeMethod(create)(NlmCmAllocator *alloc) ;
#endif
extern void     NlmCmRBNodeMethod(destroy)(NlmCmRBNodeType *self, NlmCmAllocator *alloc) ;

/*******************************************************************************/

struct NlmCmRBTreeType
{
  /* FYI: If m_nil were an embedded member, we might need to start
     serializing with the tree (and not at a node), so that m_nil is written
     only once (depending on how carefully MFC persistence is implemented).
     But it's worse than that -- there's an extra sentinel created when
     loading NlmCmRBTree when m_nil is embedded, and that causes the nil
     pointer to not be our nil pointer; and our tree is invalid.  So rework,
     and define our m_nil as a pointer.  Sigh.
  */

  NlmCmRBNodeType * m_root_p;   /* The root of the tree */
  NlmCmRBNodeType * m_nil_p;    /* The sentinel (created once) */

  NlmCmRBNodeType *m_maxNode_p; /* The node containing prefixes with the maximum prefix length*/
  NlmCmRBNodeType *m_minNode_p; /* The node containing prefixes with the minimum prefix length*/

  unsigned      m_itemCount:31;
  unsigned      m_paranoia:1 ;

  NlmCmAllocator *  m_alloc_p ;
  NlmCmRBNodeRemove m_remove_all_hook_p ;

#ifndef NlmCmRBNodeDataCmp
#   define NLMCMRBCMPFUNC , NlmCmRBNodeCmpType cmpf
  NlmCmRBNodeCmpType    m_cmpfunc_p ;
#else
#   define NLMCMRBCMPFUNC
#endif

#ifndef NlmCmRBNodeDataPrint
#   define NLMCMRBPRTFUNC , NlmCmRBNodePrintType prtf
  NlmCmRBNodePrintType  m_prtfunc_p ;
#else
#   define NLMCMRBPRTFUNC
#endif

#ifdef NlmCmRBTreeCustomFields
  NlmCmRBTreeCustomFields
#endif
} ;

/* The following functions will be defined for each unique NlmCmRBTreeType. */

extern NlmCmRBTreeType *    NlmCmRBTreeMethod(ctor)(NlmCmRBTreeType *self, NlmCmAllocator *alloc,
                           NlmCmRBNodeRemove nodeRemoveAllHook
                           NLMCMRBCMPFUNC NLMCMRBPRTFUNC) ;
extern void     NlmCmRBTreeMethod(dtor)(NlmCmRBTreeType *self);
extern NlmCmRBTreeType *    NlmCmRBTreeMethod(create)(NlmCmAllocator *alloc,
                         NlmCmRBNodeRemove nodeRemoveAllHook
                         NLMCMRBCMPFUNC NLMCMRBPRTFUNC) ;
extern void     NlmCmRBTreeMethod(destroy)(NlmCmRBTreeType *self);

/* Use these macros for speed */
#ifndef NlmCmRBTree__GetItemCount
#define         NlmCmRBTree__GetItemCount(self)     (self)->m_itemCount
#define         NlmCmRBTree__GetParanoia(self)      (self)->m_paranoia
#define         NlmCmRBTree__SetParanoia(self,val)  (self)->m_paranoia = ((val) != 0)
#define         NlmCmRBTree__GetRoot(self)      (self)->m_root_p
#endif

/* Insert an item into the tree. Remember that duplicates are not allowed. Unless you are sure,
   test with Locate and then Insert.
 */
extern void     NlmCmRBTreeMethod(Insert)(NlmCmRBTreeType *self,NlmCmRBNodeType *);

/* Adjust allows you to get a node that is already in the tree, modify it and then re-balance
   the tree. The tree is balanced according to the contents of the modified node. You do not
   need to call Adjust if you are manually Remove'ing the node from the tree and re-Insert'ing 
   back into the tree.
 */
extern void     NlmCmRBTreeMethod(Adjust)(NlmCmRBTreeType *self,NlmCmRBNodeType *);

/* Remove a node from the tree. (The node must already be in the tree) You must not call
   Remove on a node that does not already exist in the tree.
 */
extern void     NlmCmRBTreeMethod(Remove)(NlmCmRBTreeType *self, NlmCmRBNodeType *);

/* Given a key, find the node within the tree. Only the m_data field of the key
   'k' needs to be initialized for the search
 */
extern NlmCmRBNodeType *    NlmCmRBTreeMethod(Locate)(
                         const NlmCmRBTreeType *self, const NlmCmRBNodeDataType * k) ;

/* return the first element GreaterThanOrEqual to key; if key is 0, then return
   LocateFirst(); may return 0
 */
extern NlmCmRBNodeType *    NlmCmRBTreeMethod(LocateHead)(
                             const NlmCmRBTreeType *self, const NlmCmRBNodeDataType * k) ;

/* Locate the first (smallest) element in the tree; may return 0. */
extern NlmCmRBNodeType *    NlmCmRBTreeMethod(LocateFirst)(
                              const NlmCmRBTreeType *self);

/* RemoveAll removes all the nodes in the tree. If a remove-all-hook was supplied during 
   construction, it is called at each nodes removal
 */
extern void     NlmCmRBTreeMethod(RemoveAll)(NlmCmRBTreeType *self);

/* The following four methods return a ptr that can be a Nil ptr but, since our Nil ptr is
   actually a sentinel node, you have to use the NlmCmRBNode__IsValid() to determine if it is a
   valid node.
 */
extern NlmCmRBNodeType *    NlmCmRBTreeMethod(GetNext)(const NlmCmRBNodeType * x) ;
extern NlmCmRBNodeType *    NlmCmRBTreeMethod(GetPrev)(const NlmCmRBNodeType * x) ;
extern NlmCmRBNodeType *    NlmCmRBTreeMethod(GetMax)(const NlmCmRBNodeType * x) ;
extern NlmCmRBNodeType *    NlmCmRBTreeMethod(GetMin)(const NlmCmRBNodeType * x) ;

/* Will return a valid random node from the tree. Multiple calls on the same tree
 * with the same random seed will result in the same node returned.
 */

extern nlm_u32  NlmCmRBTreeMethod(GetDepth)(const NlmCmRBNodeType * x) ;
extern void     NlmCmRBTreeMethod(AssertValid)(const NlmCmRBTreeType *self);
extern void     NlmCmRBTreeMethod(SetParanoia)(NlmCmRBTreeType *self, NlmBool val);
extern NlmBool      NlmCmRBTreeMethod(GetParanoia)(const NlmCmRBTreeType *self);
extern void     NlmCmRBTreeMethod(Print)(const NlmCmRBTreeType *, NlmCmFile *out);


/* This is a trick. We locate the smallest value from here; we know that it has a Nil left
   pointer; we know the Nil pointer is our sentinel node which points to us; so we just grab
   that pointer.
 */
extern NlmCmRBTreeType *    NlmCmRBTreeMethod(GetTree)(const NlmCmRBNodeType * x);


/********************************************************************/
struct NlmCmRBItorType
{
  /* NB: clients depend on being able to delete an element to which an iterator is *not*
     positioned -- fortunately, we splice elements (via update key fields), which makes this
     work. We won one!
     */
  const NlmCmRBTreeType *   m_base_p;
  NlmCmRBNodeType *     m_here_p;
} ;

/* Note: Their is no dtor for a NlmCmRBItor. This was a consciencous decision. In the case of the
   itor, the dtor function would be a no-op. We could write a funciton that simply took in a
   'self' pointer and then cast it to a void*, but we don't want to suffer the call overhead in
   a performance sensitive area such as the RB tree.

   If we wrote a macro to accomplish the no-op, it would look similar to the following:
   <Code>
            #define NlmCmRBItorMethod(dtor)(self) (void*)self</Code>

   But, the above has a macro embedded in a macro. C compilers are not able to expand macros
   within macros, so the compiler would think we are trying to redefine the NlmCmRBItorMethod
   macro, which is not something we can do.
 */
extern NlmCmRBItorType *    NlmCmRBItorMethod(ctor)(NlmCmRBItorType *self, const NlmCmRBTreeType *base);
extern NlmCmRBItorType *    NlmCmRBItorMethod(create)(NlmCmAllocator *alloc, const NlmCmRBTreeType *base);
extern void     NlmCmRBItorMethod(destroy)(NlmCmRBItorType *self, NlmCmAllocator *alloc);
extern void     NlmCmRBItorMethod(SetHere)(NlmCmRBItorType *self, const NlmCmRBNodeType *location);
extern NlmCmRBNodeType *    NlmCmRBItorMethod(Next)(NlmCmRBItorType *self);
extern void             NlmCmRBItorMethod(Reset)(NlmCmRBItorType *self);

#ifndef NlmCmRBItor__Here
#define         NlmCmRBItor__Here(self) (self)->m_here_p
#define         NlmCmRBItor__Base(self) (self)->m_base_p
#endif

/********************************************************************/

#ifndef IMPLEMENTING_NLMCMRBTREE
#ifndef NLMPLATFORM_BCM
#include <nlmcmrbtree_udef.h>
#else
#include <soc/kbp/common/nlmcmrbtree_udef.h>
#endif
#endif

#ifndef NLMPLATFORM_BCM
#include <nlmcmexterncend.h>
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif

