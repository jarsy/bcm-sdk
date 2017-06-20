/*
 * $Id: nlmcmdbllinklist.h,v 1.2.8.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#ifndef INCLUDED_NLMCMDBLLINKLIST_H
#define INCLUDED_NLMCMDBLLINKLIST_H

#ifndef NLMPLATFORM_BCM
#include "nlmcmexterncstart.h"
#else
#include <soc/kbp/common/nlmcmexterncstart.h>
#endif

typedef struct NlmCmDblLinkList NlmCmDblLinkList ;


struct NlmCmDblLinkList
{
#ifndef NLMPLATFORM_BCM 
	#include "nlmcmdbllinklistdata.h"
#else
	#include <soc/kbp/common/nlmcmdbllinklistdata.h>
#endif
} ;

typedef void (*NlmCmDblLinkList__destroyNode_t)(NlmCmDblLinkList* node, void* arg);

NlmCmDblLinkList*
NlmCmDblLinkList__Init(NlmCmDblLinkList*);

void
NlmCmDblLinkList__Insert(
    NlmCmDblLinkList*   self, 
    NlmCmDblLinkList*   node 
    );

void
NlmCmDblLinkList__Remove(
    NlmCmDblLinkList*   node,
    NlmCmDblLinkList__destroyNode_t destroyNode,
    void* arg);

void
NlmCmDblLinkList__Destroy(
    NlmCmDblLinkList*       head,
    NlmCmDblLinkList__destroyNode_t destroyNode,
    void* arg
    );

/*NlmBool
NlmCmDblLinkList__IsEmpty(
    const NlmCmDblLinkList*     self 
    );*/
#ifndef NLMPLATFORM_BCM
#include "nlmcmexterncend.h"
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif

#endif

