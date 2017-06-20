/*
 * $Id: nlmcmdbllinklist.c,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#include "nlmcmdbllinklist.h"



NlmCmDblLinkList*
NlmCmDblLinkList__Init(NlmCmDblLinkList* head)
{
    head->m_back_p = head ;
    head->m_next_p = head ;

    return head ;
}

void
NlmCmDblLinkList__Insert(
    NlmCmDblLinkList*   self, 
    NlmCmDblLinkList*   node )
{
    NlmCmDblLinkList* next = (NlmCmDblLinkList*)self->m_next_p ;

    self->m_next_p = node ;
    node->m_back_p = self ;

    next->m_back_p = node ;
    node->m_next_p = next ;
}

void
NlmCmDblLinkList__Remove(
    NlmCmDblLinkList*   node,
    NlmCmDblLinkList__destroyNode_t destroyNode,
    void* arg)
{
    NlmCmDblLinkList*   back = (NlmCmDblLinkList*)node->m_back_p ;
    NlmCmDblLinkList*   next = (NlmCmDblLinkList*)node->m_next_p ;

    back->m_next_p = next ;
    next->m_back_p = back ;

    destroyNode(node,arg);
}


void
NlmCmDblLinkList__Destroy(
    NlmCmDblLinkList*       head,
    NlmCmDblLinkList__destroyNode_t destroyNode,
    void* arg)
{
    NlmCmDblLinkList*   here = (NlmCmDblLinkList*)head->m_next_p ;
    NlmCmDblLinkList*   next ;

    while (here != head) 
    {
        next = (NlmCmDblLinkList*)here->m_next_p ;
        destroyNode(here, arg);

        here = next ;
    }

    destroyNode(head,arg);
}












