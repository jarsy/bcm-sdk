/*
 * $Id: nlmcmrbtree_udef.h,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 

/*@@NlmCmCBTree Undefine Module
   Summary
   This module is used to undefine specific macros.

   Description
   If we are being included by a normal app, then we need to undefine all the important
   macros so that multiple tree implementations can co-exist in an app.

   We also must undefine Types, Methods, and FUNCs because the real names must be used for
   implementation purposes rather than the #define's we use. Otherwise, they may interfere
   with apps or become incompatible/unusable with multiple uses of this template.
*/


/*## Undefine important macros for multiple tree implementations. */
#undef NlmCmRBNodeDataType
#undef NlmCmRBNodeDataCmp
#undef NlmCmRBNodeDataPrint
#undef NlmCmRBNodeCtorWithData
#undef NlmCmRBNameSuffix
#undef NlmCmRBNamePrefix
#undef NlmCmRBNodeCtorHook
#undef NlmCmRBNodeDestroyHook
#undef NlmCmRBTreeCustomFields
#undef NlmCmRBTreeCtorHook
#undef NlmCmRBTreeDtorHook

/*## Undefine these to use the real names instead. */
#undef NlmCmRBNodeType
#undef NlmCmRBTreeType
#undef NlmCmRBItorType
#undef NlmCmRBNodeMethod
#undef NlmCmRBTreeMethod
#undef NlmCmRBItorMethod
#undef NlmCmRBNodeCmpType
#undef NlmCmRBNodePrintType
#undef NLMCMRBCMPFUNC
#undef NLMCMRBPRTFUNC
