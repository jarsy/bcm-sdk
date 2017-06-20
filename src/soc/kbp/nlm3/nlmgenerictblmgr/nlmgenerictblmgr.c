/*
 * $Id: nlmgenerictblmgr.c,v 1.1.6.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */ 
 
/*******************************************************************************/
/*
File Description:
This file provides an interface between User Application and Generic Table 
Manager(GTM). This module manages the generic tables contaning ACLs(Access control
lists) database into the device and decides the contents of various registers for 
search operations. Managing the database is a fully automated system and intelligent 
enough to make the optimal use of database space. 
This file contains implementations of all GTM APIs. The APIs are

1. kbp_gtm_init
2. kbp_gtm_destroy
3. kbp_gtm_create_table
4. kbp_gtm_destroy_table
5. kbp_gtm_config_search
6. kbp_gtm_lock_config
7. kbp_gtm_add_record
8. kbp_gtm_update_record
9. kbp_gtm_delete_record


Above APIs are being used by user application for GTM.

Device instruction APIs (read, write, compare) takes input from the GTM
or user application in a format(say F1) which is not compatible to input 
format(say F2) what trnsport layer expects. These APIs serve this purpose ie.
converting format F1 to format F2 for transport layer. Transport layer takes
instructions as input in a request structure which is in similar format
as actual device accept, ie. Instruction_Bus, Data_Bus, Context_Address.
For more details about this format please see the data sheet and transport
layer module documents/comments. Please see the function declarations/
definitions for details of the above APIs. 

Apart form the APIs given above the file contains some private utility
functions declaration/implementation which are being used by the above APIs.
The private functions are


Please see the function declarations/definitions for details of the above
private functions.

Methodologies and Algorithms:
The file serves as a simple interface between Application/GTM  and 
transport_layer, so it need not to define/use any algorithm. For 
managing devices in a cascade system a link list of all devices is 
maintained in NlmDevMgr structure. This file does not handle multi 
channel case, so one Device Manager can handle only one cascade system. 
Multiple channels must be managed in GTM/User_Application.

File position in the full integrated system:

|-----------------------|       |-----------------------|       |-----------------------|
|                       |       |                       |       |                       |
|Generic Table Manager  |       |   User Application    |       |   Range Manager       |
|                       |       |                       |       |                       |
|-----------------------|       |-----------------------|       |-----------------------|
            |                               |                               |   
            |                               |                               |
            |                               |                               |
            |                   ____________v____________                   |
            |-----------------> |                       |<------------------|
                                |Device Manager.h/.c    |
                                |                       |
                                |_______________________|
                                            |
                                            v
                                |-----------------------|
                                |                       |
                                |   Transport Layer     |
                                |                       |
                                |-----------------------|

*/
/*******************************************************************************/

#include "nlmgenerictblmgr.h"
#include "nlmarch.h"

#ifdef NLM_12K_11K
#include <arch/nlmarch.h>

#endif

#include "nlmcmstring.h"
#include "nlmxktblmgr.h"
#include "nlmtblmgr.h"

/* destructure for Generic Table Manager */
static void NlmGenericTblMgr__dtor(
    NlmGenericTblMgr *self
    );

/* destructure for generic table */
static void NlmGenericTbl__dtor(
    NlmGenericTbl *self
    );

/*
Function: NlmGenericTblList__Init
Parameters: NlmCmAllocator* alloc_p
Return Type: NlmGenericTblList*
Description: Initializes empty generic table list by creating "head" node.
"head" contains "NlmGenericTbl*" = NULL. Parameter alloc_p is
used for memory allocation. Function returns list pointer which is "head".
*/
NlmGenericTblList* NlmGenericTblList__Init(NlmCmAllocator* alloc_p)
{

    NlmGenericTblList* head = (NlmGenericTblList*)NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmGenericTblList));

    if(!head) return 0;

    NlmCmDblLinkList__Init((NlmCmDblLinkList*)head);

    return head;
}

/*
Function: NlmGenericTblList__Insert
Parameters: NlmGenericTblList* head, NlmGenericTbl* gt_p, NlmCmAllocator* alloc_p
Return Type: NlmGenericTblList*
Description: Creates a generic table list node containing a "NlmGenericTbl"
pointer of newly created table and inserts this node into the generic table list.
It takes "head" as input and returns the newly created node pointer.
*/
NlmGenericTblList* NlmGenericTblList__Insert(NlmGenericTblList* head,
                                      NlmGenericTbl* gt_p, NlmCmAllocator* alloc_p)
{
    NlmGenericTblList* node = (NlmGenericTblList*)NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmGenericTblList));

    if(!node) return 0;
    
    node->m_gt_p = gt_p;

    NlmCmDblLinkList__Insert((NlmCmDblLinkList*)head, (NlmCmDblLinkList*)node);

    return node;
}

/*
Function: NlmGenericTblList__Search
Parameters: NlmGenericTblList* head, nlm_u8 tblId
Return Type: NlmGenericTblList*
Description: Searches a node in the generic table list based on the table_ID.
It takes list address "head" and tableID as input and returns node containing
table pointer which has the same tableID as input. If table is not found, it
returns NULL.
*/
NlmGenericTblList* NlmGenericTblList__Search( NlmGenericTblList* head,
                                                    nlm_u8 tblId )
{
    NlmGenericTblList *node;

    /* First node in the list does not contain any table, advance to next node */
    node = (NlmGenericTblList*)head->m_next_p;

    while( node != head )
    {
        if(node->m_gt_p->m_tblId == tblId )
            return node;

        node = (NlmGenericTblList*)node->m_next_p;
    }

    /* Not found */
    return NULL;
}
                                    
/*
Function: NlmGenericTblList__destroyNode
Parameters: NlmCmDblLinkList* node, void* arg
Return Type: void
Description: Deletes a node and free the memory held by the node. It calls the 
destructor of generic table to free all the memory held by the generic table. 
This function just deletes the node and do not update the doubly link list.
It takes the node pointer to be deleted as input and returns nothing. Second
input parameter "arg" is nothing but the memory allocator.
*/
void NlmGenericTblList__destroyNode(NlmCmDblLinkList* node, void* arg)
{
    NlmGenericTblList* gTbList = (NlmGenericTblList*)node;

    NlmGenericTbl__dtor(gTbList->m_gt_p);

    NlmCmAllocator__free((NlmCmAllocator*)arg, gTbList->m_gt_p);

    NlmCmAllocator__free((NlmCmAllocator*)arg, gTbList);
}

/*
Function: NlmGenericTblList__Remove
Parameters: NlmGenericTblList* node, NlmCmAllocator* alloc_p
Return Type: void
Description: This function removes a generic table list node from the list.
It actually calls "NlmCmDblLinkList__Remove" which frees the node from the list
and update the list and then it calls "NlmGenericTblList__destroyNode" to free
the memory held by the node. It takes the node pointer to be deleted as input 
and returns nothing.
*/
void NlmGenericTblList__Remove(NlmGenericTblList* node, NlmCmAllocator* alloc_p)
{
    NlmCmDblLinkList__Remove((NlmCmDblLinkList*)node,
                            NlmGenericTblList__destroyNode,
                            alloc_p); 
    
}

/*
Function: NlmGenericTblList__Destroy
Parameters: NlmGenericTblList* head, NlmCmAllocator* alloc_p
Return Type: void
Description: This function destroys the whole generic table list. It takes the 
list pointer which is "head" as input and returns nothing.
*/
void NlmGenericTblList__Destroy(NlmGenericTblList* head, NlmCmAllocator* alloc_p)
{
    NlmCmDblLinkList__Destroy((NlmCmDblLinkList*)head,
                                NlmGenericTblList__destroyNode,
                                alloc_p);

}

/*
Function: NlmPsTblList__Init
Parameters: NlmCmAllocator* alloc_p
Return Type: NlmPsTblList*
Description: Initializes empty parallel search table list by creating "head" node.
"head" contains m_tblId = NULL. Parameter alloc_p is used for memory allocation. 
Function returns list pointer which is "head".
*/

NlmPsTblList* NlmPsTblList__Init(NlmCmAllocator* alloc_p)
{

    NlmPsTblList* head = (NlmPsTblList*)NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmPsTblList));

    if(!head) return 0;

    NlmCmDblLinkList__Init((NlmCmDblLinkList*)head);

    return head;
}

/*
Function: NlmPsTblList__InsertUnique
Parameters: NlmPsTblList* head, nlm_u8 tableId , NlmCmAllocator* alloc_p
Return Type: NlmPsTblList*
Description: Creates a parallel search table list node containing a m_tblId
of newly created table and inserts this node into the parallel search table list.
It takes "head" as input and returns the newly created node pointer.
*/

NlmPsTblList* 
NlmPsTblList__InsertUnique(
        NlmPsTblList* head_p,
        nlm_u8 tableId , 
        NlmCmAllocator* alloc_p)
{
    /*First search if the table is already present.  */
    NlmPsTblList *curNode_p = (NlmPsTblList*)head_p->m_next_p;
    NlmPsTblList *newNode_p = NULL;

    while(curNode_p != head_p)
    {
        if(curNode_p->m_tblId == tableId)
        {
            /*The table is already present in the list. So return */
            return curNode_p;
        }
        curNode_p = (NlmPsTblList*)curNode_p->m_next_p;
    }

    
    /*If the table is not present in the list, then add it to the list */
    newNode_p = (NlmPsTblList*)NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmPsTblList));

    if(!newNode_p) 
        return NULL;
    
    newNode_p->m_tblId = tableId;

    NlmCmDblLinkList__Insert((NlmCmDblLinkList*)head_p, (NlmCmDblLinkList*)newNode_p);

    return newNode_p;
}

/*
Function: NlmPsTblList__destroyNode
Parameters: NlmCmDblLinkList* node, void* arg
Return Type: void
Description: Deletes a node and free the memory held by the node. It calls the 
destructor of parallel search table to free all the memory held by the parallel 
search table. This function just deletes the node and do not update the doubly 
link list. It takes the node pointer to be deleted as input and returns nothing. 
Second input parameter "arg" is nothing but the memory allocator.
*/

void NlmPsTblList__destroyNode(NlmCmDblLinkList* node, void* arg)
{
    NlmPsTblList* gTbPsList = (NlmPsTblList*)node;

    NlmCmAllocator__free((NlmCmAllocator*)arg, gTbPsList);
}



/*
Function: NlmPsTblList__Destroy
Parameters: NlmPsTblList* head, NlmCmAllocator* alloc_p
Return Type: void
Description: This function destroys the whole parallel search table list.
It takes the list pointer which is "head" as input and returns nothing.
*/
void NlmPsTblList__Destroy(NlmPsTblList* head, NlmCmAllocator* alloc_p)
{
    NlmCmDblLinkList__Destroy((NlmCmDblLinkList*)head,
                                NlmPsTblList__destroyNode,
                                alloc_p);

}

/*=====================================================================================*/

/* 
Function: NlmGenericTblMgr__ctor
Parameters: 
    NlmGenericTblMgr *self,
    NlmDevType       devType,
    NlmBankNum      bankNum,
    NlmCmAllocator   *alloc_p
Return Type: NlmGenericTblMgr*
Description: constructor of Generic Table Manager assigns the table manager name,
device type and memory allocator. It initializes table count to zero, an empty
generic table list and memory for VTable. It sets the GTM cofiguration unlocked.
It takes the Generic Table Manager pointer,  device type and memory 
allocator as input and returns NlmErrNum.
*/
NlmErrNum_t 
NlmGenericTblMgr__ctor(
    NlmGenericTblMgr *self,
    NlmDevType       devType,
    NlmBankNum      bankNum,
    NlmCmAllocator   *alloc_p,
    NlmReasonCode    *o_reason_p)
{
    NlmCmAssert((alloc_p != NULL), "Invalid memory allocator provided.\n");
    NlmCmAssert((self != NULL), "Invalid self pointer.\n");
    
    self->m_alloc_p    = alloc_p;
    
    self->m_devType = devType;

    self->m_genericTbl_list_p = NULL;
    self->m_genericTbl_count = 0;
    self->m_tblMgr_p = NULL;
    self->m_IsConfigLocked = NLMFALSE;
    self->m_bankType = bankNum;
    self->m_vtbl_p = NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmTblMgr__pvt_vtbl));
    
    if(!self->m_vtbl_p)
    {
        if(o_reason_p)
            *o_reason_p = NLMRSC_LOW_MEMORY;
        return NLMERR_FAIL;
    }
    

    /* Initialize the table list   */
    self->m_genericTbl_list_p = (NlmCmDblLinkList*)NlmGenericTblList__Init(alloc_p);
    NlmCmAssert( self->m_genericTbl_list_p, "Memory alloc for table list failed\n" );

    return NLMERR_OK;
}

/* 
Function: NlmGenericTblMgr__dtor
Parameters: NlmGenericTblMgr *self
Return Type: void
Description: destructor of Generic Table Manager deletes the whole generic 
table list. It frees the memory for table list, VTable and GTM name. It takes 
Generic Table Manager pointer as input and returns nothing.
*/
void NlmGenericTblMgr__dtor(
    NlmGenericTblMgr *self
    )
{
    NlmCmAllocator *alloc_p = NULL;

    if(self == NULL)
        return;

    alloc_p = self->m_alloc_p;

    if(NULL !=self->m_genericTbl_list_p)
        NlmGenericTblList__Destroy((NlmGenericTblList*)self->m_genericTbl_list_p, alloc_p);

    if(self->m_genericTblSBRange)
    {
        NlmCmAllocator__free(alloc_p, self->m_genericTblSBRange);
        self->m_genericTblSBRange = NULL;
    }
    if(self->m_genericTblBlksRange)
    {
        NlmCmAllocator__free(alloc_p, self->m_genericTblBlksRange); 
        self->m_genericTblBlksRange = NULL;
    }
    if(self->m_udaSBRange)
    {
        NlmCmAllocator__free(alloc_p, self->m_udaSBRange);  
        self->m_udaSBRange = NULL;
    }

    NlmCmAllocator__free(alloc_p, self->m_vtbl_p);
}

/*
Function: NlmGenericTbl__ctor
Parameters: 
    NlmGenericTbl *self,
    NlmCmAllocator   *alloc_p,
    NlmGenericTblMgr *genericTblMgr_p,
    nlm_u8            tblId,
    nlm_u16           width,
    nlm_u32           genericTblSize
Return Type: NlmGenericTbl*
Description: constructor of generic table assigns the memory allocator, address
of Generic Table Manager to which this table belongs to, table ID,
table width and table size. if this table is static, genericTblSize should not be 
zero. If it is zero then table is dynamic. It returns the same generic table 
pointer which it takes as input.
*/
static NlmGenericTbl* NlmGenericTbl__ctor(
    NlmGenericTbl *self,
    NlmCmAllocator   *alloc_p,
    NlmGenericTblMgr *genericTblMgr_p,
    nlm_u8            tblId,
    NlmGenericTblWidth width,
    nlm_u32           genericTblSize
    )
{
    NlmCmAssert((alloc_p != NULL), "Invalid memory allocator provided.\n");
    
    self->m_alloc_p    = alloc_p;
    self->m_genericTblMgr_p = genericTblMgr_p;
    self->m_tblId = tblId;
    self->m_width = width;
    self->m_tableMaxSize = genericTblSize;

    self->m_num_records = 0;
    self->m_psTblList_p = NULL;
    self->m_iterHandle = NULL;

    /* Initialize Parallel Search Dependence list   */
    self->m_psTblList_p = NlmPsTblList__Init(self->m_alloc_p);
    NlmCmAssert( self->m_psTblList_p, "Memory alloc for PS Dependency failed.\n" );

    return self;
}

/*
Function: NlmGenericTbl__dtor
Parameters: NlmGenericTbl *self
Return Type: void
Description: destructor of generic table deletes the memory held by the its
table_ID string and its parallel search table list.
It takes generic table pointer as input and returns nothing.
*/
static void NlmGenericTbl__dtor(
    NlmGenericTbl *self
    )
{
    NlmCmAllocator *alloc_p = NULL;

    if(self == NULL)
        return;
        
    alloc_p = self->m_alloc_p;

    if(NULL !=self->m_psTblList_p)
        NlmPsTblList__Destroy(self->m_psTblList_p, alloc_p);
}
/*wrapper*/
NlmGenericTblMgr *
NlmGenericTblMgr__Init(
        NlmCmAllocator                  *alloc_p,
        void                            *devMgr_p,
        NlmDevType                      devType,
        NlmBankNum                      bankNum,
        nlm_u8                          numOfDevices,
        NlmGenericTblMgrSBRange         *dbaSbRange,
        NlmGenericTblMgrSBRange         *udaSbRange,
        NlmIndexChangedAppCb            indexChangedAppCb,         
        void*                           client_p,
        NlmReasonCode                   *o_reason
        )
{
    return kbp_gtm_init(alloc_p,devMgr_p,devType,bankNum,numOfDevices,dbaSbRange,udaSbRange,indexChangedAppCb,client_p,o_reason);
}

NlmGenericTbl*
NlmGenericTblMgr__CreateTable(
        NlmGenericTblMgr    *genericTblMgr,
        NlmPortNum          portNum,
        nlm_u8              genericTblId, 
        NlmGenericTblWidth  genericTblWidth,
        NlmGenericTblAssoWidth      adWidth,
        nlm_u32             genericTblSize,
        NlmReasonCode       *o_reason
        )
{
    return kbp_gtm_create_table(
        genericTblMgr,
        portNum,
        genericTblId, 
        genericTblWidth,
        adWidth,
        genericTblSize,
        o_reason
        );
}

NlmErrNum_t NlmGenericTblMgr__DestroyTable(
    NlmGenericTblMgr  *genericTblMgr,
    NlmPortNum          portNum,
    NlmGenericTbl     *genericTbl,
    NlmReasonCode     *o_reason
    )
{
    return kbp_gtm_destroy_table(
    genericTblMgr,
    portNum,
    genericTbl,
    o_reason
    );
}


/*
Function: NlmGenericTblMgr__pvt_ConfigParallelSearchDep
Parameters: 
    NlmGenericTblMgr  *genericTblMgr,
    nlm_u8 tbl1_id,
    nlm_u8 tbl2_id,
    NlmReasonCode     *o_reason
Return Type: NlmErrNum_t
Description: Configure the parallel search dependency between the tables.
If two tables are configured in the same LTR then they form a 
parallel search dependency pair 
*/
NlmErrNum_t 
NlmGenericTblMgr__pvt_ConfigParallelSearchDep(
        NlmGenericTblMgr  *genericTblMgr,
        nlm_u8 tbl1_id,
        nlm_u8 tbl2_id,
        NlmReasonCode    *o_reason
        )
{
    NlmGenericTbl *tbl1_p = NULL, *tbl2_p = NULL;
    NlmGenericTblList *tbl1Node_p = NULL, *tbl2Node_p = NULL;

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    
    /* It's not valid if tables passed are same */
    if(tbl1_id == tbl2_id)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PS_DEPEND_ATTRIBUTES;

        return NLMERR_FAIL;
    }

    /* Check whether these two tables really exist or not */
    if( NULL == ( tbl1Node_p = NlmGenericTblList__Search(
                                    (NlmGenericTblList *)genericTblMgr->m_genericTbl_list_p,
                                     tbl1_id ) ) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PS_DEPEND_ATTRIBUTES;

        return NLMERR_FAIL;
    }

    if( NULL == ( tbl2Node_p = NlmGenericTblList__Search(
                                    (NlmGenericTblList *)genericTblMgr->m_genericTbl_list_p,
                                     tbl2_id ) ) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PS_DEPEND_ATTRIBUTES;

        return NLMERR_FAIL;
    }

    tbl1_p = tbl1Node_p->m_gt_p;
    tbl2_p = tbl2Node_p->m_gt_p;
    
    /* Make sure PS Dependency list is not NULL */
    if(tbl1_p->m_psTblList_p == NULL || tbl2_p->m_psTblList_p == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_INTERNAL_ERROR;
        return NLMERR_FAIL;
    }

    /* Insert table2 into table1's PS Dependency list   */
    if( NULL == NlmPsTblList__InsertUnique( tbl1_p->m_psTblList_p, tbl2_p->m_tblId, tbl1_p->m_alloc_p ) )
    {
        if(o_reason)
            *o_reason = NLMRSC_LOW_MEMORY;
        return NLMERR_FAIL;
    }

    /* Insert table1 into table2's PS Dependency list */
    if( NULL == NlmPsTblList__InsertUnique( tbl2_p->m_psTblList_p, tbl1_p->m_tblId, tbl2_p->m_alloc_p ) )
    {
        if(o_reason)
            *o_reason = NLMRSC_LOW_MEMORY;
        return NLMERR_FAIL;
    }
    
    return NLMERR_OK;
}


NlmErrNum_t 
NlmGenericTblMgr__ConfigSearch(
        NlmGenericTblMgr  *genericTblMgr,
        NlmPortNum          portNum,
        nlm_u8             ltrNum, 
        NlmGenericTblSearchAttributes  *searchAttrs,
        NlmReasonCode    *o_reason
        )
{
    return kbp_gtm_config_search(
        genericTblMgr,
        portNum,
        ltrNum, 
        searchAttrs,
        o_reason
        );
}

NlmErrNum_t  
NlmGenericTblMgr__LockConfiguration (
            NlmGenericTblMgr  *genericTblMgr,
            NlmPortNum          portNum,
            NlmReasonCode    *o_reason
            )
{
    return kbp_gtm_lock_config (
            genericTblMgr,
            portNum,
            o_reason
            );
}

NlmErrNum_t 
NlmGenericTblMgr__AddRecord(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8              *assocData,
    nlm_u16              groupId,
    nlm_u16              recordPriority, 
    NlmRecordIndex      *o_recordIndex,
    NlmReasonCode       *o_reason
    )  
{
    return kbp_gtm_add_record(
    genericTbl,
    portNum,
    tblRecord,
    assocData,
    groupId,
    recordPriority, 
    o_recordIndex,
    o_reason
    );
}

NlmErrNum_t 
NlmGenericTblMgr__UpdateRecord(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8              *assocData,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
    ) 
{
    return kbp_gtm_update_record(
    genericTbl,
    portNum,
    tblRecord,
    assocData,
    recordIndex,
    o_reason
    ) ;
}

NlmErrNum_t 
NlmGenericTblMgr__DeleteRecord(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
    ) 
{
    return kbp_gtm_delete_record(
    genericTbl,
    portNum,
    recordIndex,
    o_reason
    );
}

NlmErrNum_t  
NlmGenericTblMgr__Destroy(
            NlmGenericTblMgr *genericTblMgr,    
            NlmPortNum       portNum,
            NlmReasonCode    *o_reason
            )
{
    return kbp_gtm_destroy(
            genericTblMgr,  
            portNum,
            o_reason
            );
}


/*
Function: kbp_gtm_init
Parameters: 
        NlmCmAllocator                  *alloc_p,
        void                            *devMgr_p,
        NlmDevType                      devType,
        NlmBankNum                      bankNum,
        nlm_u8                          numOfDevices,
        NlmGenericTblMgrSBRange         *dbaSbRange,
        NlmGenericTblMgrSBRange         *udaSbRange,
        nlm_u8                          tblIdStrLen,
        NlmIndexChangedAppCb            indexChangedAppCb, 
        void*                           client_p,
        NlmReasonCode                   *o_reason
Return Type: NlmGenericTblMgr*
Description: Creates and initializes the Generic Table Manager inside the control plane software.
The Generic Table Manager needs to be given a target device it should work with.
The Generic Table Manager is designed with any generic Processor architectures.
Only one Generic Table Manager can be instantiated per set of devices.
Return value of 0 indicates success, and anything else is failure. The initialized
Generic Table Manager is returned, and the pointer has to be used for further
function calls. client_p is the pointer passed from the application, that is passed back in 
the index changed callback to the application (in case, the application wants
to access its specific data structures in the callback.
*/
NlmGenericTblMgr *
kbp_gtm_init(
        NlmCmAllocator                  *alloc_p,
        void                            *devMgr_p,
        NlmDevType                      devType,
        NlmBankNum                      bankNum,
        nlm_u8                          numOfDevices,
        NlmGenericTblMgrSBRange         *dbaSbRange,
        NlmGenericTblMgrSBRange         *udaSbRange,
        NlmIndexChangedAppCb            indexChangedAppCb,         
        void*                           client_p,
        NlmReasonCode                   *o_reason
        )
{
    NlmGenericTblMgr *self = NULL;
    NlmErrNum_t errNum = NLMERR_OK;
    NlmGenericTblMgrBlksRange gtmBlksRange[NLMDEV_MAX_DEV_NUM];
    nlm_u32 devNum = 0;

#ifndef NLM_12K_11K
    NlmDevMgr *devMgr_ptr  = NULL;
#else
    NlmGenericTblMgrSBRange dummyUdaSbRange;    
    if(udaSbRange == NULL)
        udaSbRange = &dummyUdaSbRange;
#endif


    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    if(alloc_p == NULL)    
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_MEMALLOC_PTR;
        return NULL;
    }

    if(devMgr_p == NULL)    
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NULL;
    }
    if(dbaSbRange == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_GTM_SB_BLKS_RANGE;
        return NULL;
    }
    if(udaSbRange == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_UDA_SB_BLKS_RANGE;
        return NULL;
    }

    if(numOfDevices > NLMDEV_MAX_DEV_NUM)
    {
          if(o_reason)
            *o_reason = NLMRSC_INVALID_NUM_OF_DEVICES;
        return NULL;
    }

#ifndef NLM_12K_11K
    if( (bankNum != NLMDEV_BANK_0) && (bankNum != NLMDEV_BANK_1) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_BANK_NUM;
        return NULL;
    }
    
    /* check for the bank types */
    devMgr_ptr = (NlmDevMgr*)devMgr_p;
    if(devMgr_ptr->m_smtMode == NLMDEV_DUAL_SMT_MODE)
    {
        /* in NLMDEV_DUAL_SMT_MODE bank-0 and bank-1 are supported */
        if( bankNum != NLMDEV_BANK_0 && bankNum != NLMDEV_BANK_1 )
        {
            if(o_reason)
                *o_reason = NLMRSC_INVALID_BANK_NUM;
            return NULL;
        }
    }
    else
    {
        /* in NLMDEV_NO_SMT_MODE only bank-0 supported */
        if( bankNum != NLMDEV_BANK_0 )
        {
            if(o_reason)
                *o_reason = NLMRSC_INVALID_BANK_NUM;
            return NULL;
        }
    }
#endif

    /* check for the super block ranges */
    for(devNum = 0; devNum < numOfDevices; devNum++)
    {
        /* ============ DBA Blocks Checkings ============ */
        /* check for the super block ranges */
        if( dbaSbRange[devNum].m_stSBNum > NLMDEV_NUM_SUPER_BLOCKS  ||
            dbaSbRange[devNum].m_endSBNum > NLMDEV_NUM_SUPER_BLOCKS )
        {
            if(o_reason)
                *o_reason = NLMRSC_INVALID_GTM_SB_BLKS_RANGE;
            return NULL;
        }
        
        if( dbaSbRange[devNum].m_stSBNum != NLM_GTM_DBA_SB_BLK_RANGE_WHOLE_DEVICE && 
                dbaSbRange[devNum].m_endSBNum != NLM_GTM_DBA_SB_BLK_RANGE_WHOLE_DEVICE)
        {
            /* check for the super block ranges:  start SB < end SB*/
            if( dbaSbRange[devNum].m_stSBNum > dbaSbRange[devNum].m_endSBNum )
            {
                if(o_reason)
                    *o_reason = NLMRSC_INVALID_GTM_SB_BLKS_RANGE;
                return NULL;
            }
#ifndef NLM_12K_11K
            /* super block range must be at 16block boundary: minimum 2 SB 
            * st  SB must start with EVEN  
            * end SB must start with ODD 
            */
            if( ((dbaSbRange[devNum].m_stSBNum % 2) != 0) || ((dbaSbRange[devNum].m_endSBNum % 2) != 1) )
            {
                if(o_reason)
                    *o_reason = NLMRSC_INVALID_GTM_SB_BLKS_RANGE;
                return NULL;
            }
#endif
        }

#ifndef NLM_12K_11K
        /* ============ UDA Blocks Checkings ============ */
        /* check for the super block ranges */
        if( udaSbRange[devNum].m_stSBNum == NLM_GTM_UDA_SB_BLK_RANGE_NO_BLK_USED || 
            udaSbRange[devNum].m_endSBNum == NLM_GTM_UDA_SB_BLK_RANGE_NO_BLK_USED )
        {
            continue;
        }
        if( udaSbRange[devNum].m_stSBNum > NLMDEV_NUM_SRAM_SUPER_BLOCKS  ||
            udaSbRange[devNum].m_endSBNum > NLMDEV_NUM_SRAM_SUPER_BLOCKS )
        {
            if(o_reason)
                *o_reason = NLMRSC_INVALID_UDA_SB_BLKS_RANGE;
            return NULL;
        }

        if( udaSbRange[devNum].m_stSBNum != NLM_GTM_UDA_SB_BLK_RANGE_WHOLE_DEVICE && 
                udaSbRange[devNum].m_endSBNum != NLM_GTM_UDA_SB_BLK_RANGE_WHOLE_DEVICE)
        {
            /* check for the super block ranges:  start SB < end SB*/
            if( udaSbRange[devNum].m_stSBNum > udaSbRange[devNum].m_endSBNum )
            {
                if(o_reason)
                    *o_reason = NLMRSC_INVALID_UDA_SB_BLKS_RANGE;
                return NULL;
            }
        }   
#else
    udaSbRange[devNum].m_stSBNum = NLM_GTM_UDA_SB_BLK_RANGE_NO_BLK_USED;
    udaSbRange[devNum].m_endSBNum = NLM_GTM_UDA_SB_BLK_RANGE_NO_BLK_USED;
#endif
    }

    
    self = (NlmGenericTblMgr*)NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmGenericTblMgr));
     
     if(!self )
     {
         if(o_reason)
             *o_reason = NLMRSC_LOW_MEMORY;
         return NULL;
     }
    
    errNum = NlmGenericTblMgr__ctor(self,  devType, bankNum, alloc_p, o_reason); 
    
    if(errNum != NLMERR_OK)
    {
        NlmCmAllocator__free(alloc_p, self);
        return NULL;
    }

    self->m_client_p = client_p;

    for(devNum = 0; devNum < numOfDevices; devNum++)
    {
        /* ============ DBA Blocks Checkings ============ */
        /* check for the super block ranges */
        if( dbaSbRange[devNum].m_stSBNum == NLM_GTM_DBA_SB_BLK_RANGE_NO_BLK_USED || 
            dbaSbRange[devNum].m_endSBNum == NLM_GTM_DBA_SB_BLK_RANGE_NO_BLK_USED )
        {
            gtmBlksRange[devNum].m_startBlkNum = NLM_GTM_BLK_RANGE_NO_BLK_USED;
            gtmBlksRange[devNum].m_endBlkNum   = NLM_GTM_BLK_RANGE_NO_BLK_USED; 
        }
        else if( dbaSbRange[devNum].m_stSBNum == NLM_GTM_DBA_SB_BLK_RANGE_WHOLE_DEVICE || 
                dbaSbRange[devNum].m_endSBNum == NLM_GTM_DBA_SB_BLK_RANGE_WHOLE_DEVICE)
        {
            /* all blocks are allocated to bank */
            gtmBlksRange[devNum].m_startBlkNum = 0;
#ifndef NLM_12K_11K
            gtmBlksRange[devNum].m_endBlkNum   = (devMgr_ptr->m_numOfAbs - 1); 
#else
            gtmBlksRange[devNum].m_endBlkNum   = (NLM11KDEV_NUM_ARRAY_BLOCKS - 1); 
#endif
        } 
        else
        {
#ifndef NLM_12K_11K
            /* assign block ranges: get it from SB ranges */
            gtmBlksRange[devNum].m_startBlkNum = 
                (dbaSbRange[devNum].m_stSBNum * devMgr_ptr->m_numOfABsPerSB);

            gtmBlksRange[devNum].m_endBlkNum   = 
                ((dbaSbRange[devNum].m_endSBNum * devMgr_ptr->m_numOfABsPerSB) - 1);
#else
        /* assign block ranges: get it from SB ranges */
        gtmBlksRange[devNum].m_startBlkNum = 
            (dbaSbRange[devNum].m_stSBNum * NLM11KDEV_NUM_BLKS_PER_SUPER_BLOCK);
        
        gtmBlksRange[devNum].m_endBlkNum   = 
            ((dbaSbRange[devNum].m_endSBNum * NLM11KDEV_NUM_BLKS_PER_SUPER_BLOCK) - 1);
#endif

        }

#ifndef NLM_12K_11K

        /* ============ UDA Blocks Checkings ============ */
        /* check for the super block ranges */
        if( udaSbRange[devNum].m_stSBNum == NLM_GTM_UDA_SB_BLK_RANGE_NO_BLK_USED || 
            udaSbRange[devNum].m_endSBNum == NLM_GTM_UDA_SB_BLK_RANGE_NO_BLK_USED )
        {
            udaSbRange[devNum].m_stSBNum = NLM_GTM_UDA_SB_BLK_RANGE_NO_BLK_USED;
            udaSbRange[devNum].m_endSBNum   = NLM_GTM_UDA_SB_BLK_RANGE_NO_BLK_USED; 
        }
        else if( udaSbRange[devNum].m_stSBNum == NLM_GTM_UDA_SB_BLK_RANGE_WHOLE_DEVICE || 
                udaSbRange[devNum].m_endSBNum == NLM_GTM_UDA_SB_BLK_RANGE_WHOLE_DEVICE)
        {
            /* all blocks are allocated to bank */
            udaSbRange[devNum].m_stSBNum    = 0;
            udaSbRange[devNum].m_endSBNum   = (NLMDEV_NUM_SRAM_SUPER_BLOCKS - 1); 
        }
#endif

    }

#if defined NLM_MT_OLD || defined NLM_MT
    {
        nlm_32 ret;

    ret = NlmCmMt__SpinInit(&self->m_spinLock, 
                    "NlmGtm_Kbp_SpinLock",
                    NlmCmMtFlag);
        if(ret != 0)
        {
            *o_reason = NLMRSC_MT_SPINLOCK_INIT_FAILED;
        NlmCmAllocator__free(alloc_p, self);
        
            return NULL;
        }
    }
#endif

    switch(devType)
    {
#ifndef NLM_12K_11K
        case NLM_DEVTYPE_3:
        case NLM_DEVTYPE_3_N:
        case NLM_DEVTYPE_3_40M:
        case NLM_DEVTYPE_3_N_40M:
#else
        case NLM_DEVTYPE_2:
        case NLM_DEVTYPE_2_S:
#endif
        {
            /* Allocate memory for genericTableMgr_p*/
            if(NLMERR_OK != NlmTblMgr__Init(
                                self,
                                alloc_p,
                                devMgr_p,
                                numOfDevices,
                                &gtmBlksRange[0],
                                udaSbRange,
                                indexChangedAppCb,
                                client_p,
                                o_reason
                                ))
            {
                NlmGenericTblMgr__dtor(self);
                NlmCmAllocator__free(alloc_p, self);

                return NULL;
            }
            break;
        }
        default:
        {
            NlmGenericTblMgr__dtor(self);
            NlmCmAllocator__free(alloc_p, self);

            if(o_reason)
                *o_reason = NLMRSC_INVALID_DEVICE_TYPE;
            return NULL;
        }
    }
    return self;
}
/*
Function: kbp_gtm_create_table
Parameters: 
        NlmGenericTblMgr    *genericTblMgr,
        NlmPortNum          portNum,
        nlm_u8              genericTblId, 
        NlmGenericTblWidth  genericTblWidth,
        NlmGenericTblAssoWidth      adWidth,
        nlm_u32             genericTblSize,
        NlmReasonCode       *o_reason
Return Type: NlmGenericTbl*
Description: Adds a new generic table to the Generic Table Manager. A table id 
is assigned. Key searched within this table must contain this table id at the MSB 
portion of the key. A table can be configured with DBA width of 80, 160, 320 or 640. 
A table can be configured to have associated data also of width 32, 64, 128 or 256.
Table size can be static or dynamic. If table is static Table_Size must have a non zero value. 
If table is dynamic Table_Size must have zero value. 
Return value of 0 indicates success, and anything else is failure. Table pointer is returned, which can be used for further function calls.
*/
NlmGenericTbl*
kbp_gtm_create_table(
        NlmGenericTblMgr    *genericTblMgr,
        NlmPortNum          portNum,
        nlm_u8              genericTblId, 
        NlmGenericTblWidth  genericTblWidth,
        NlmGenericTblAssoWidth      adWidth,
        nlm_u32             genericTblSize,
        NlmReasonCode       *o_reason
        )
{
    NlmGenericTbl* genericTbl_p = NULL;
    NlmErrNum_t err = NLMERR_OK;
    NlmCmAllocator* alloc_p = NULL;
    NlmTblMgr__pvt_vtbl* vtbl_p = NULL;
    NlmReasonCode dummyReasonCode;

    if(o_reason == NULL)
        o_reason = &dummyReasonCode;
    
    *o_reason = NLMRSC_REASON_OK;

    if(NULL == genericTblMgr)
    {
        *o_reason = NLMRSC_INVALID_GENERIC_TM;
        return NULL;
    }

#ifndef NLM_12K_11K 

    if( (portNum != NLMDEV_PORT_0) && (portNum != NLMDEV_PORT_1) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NULL;
    }

#endif

    if( genericTblWidth >= NLM_TBL_WIDTH_END)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_TABLE_WIDTH;
        return NULL;
    }
    vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTblMgr->m_vtbl_p;

    /* Make sure table list is not NULL   */
    NlmCmAssert( genericTblMgr->m_genericTbl_list_p, "Table List NULL!!\n" );
    if(genericTblMgr->m_genericTbl_list_p == NULL)
    {
        *o_reason = NLMRSC_INVALID_POINTER;
        return NULL;
    }
    
    if(NLMTRUE == genericTblMgr->m_IsConfigLocked)
    {
        *o_reason = NLMRSC_CONFIGURATION_LOCKED;
        return NULL;
    }

#ifndef NLM_12K_11K
    if(adWidth >= NLM_TBL_ADLEN_END)
    {
        *o_reason = NLMRSC_INVALID_TABLE_ASSO_WIDTH;
        return NULL;
    }
#else
    if(adWidth > NLM_TBL_ADLEN_ZERO)
    {
        *o_reason = NLMRSC_INVALID_TABLE_ASSO_WIDTH;
        return NULL;
    }

#endif
    
    
    /* memory allocation for new table */
    alloc_p = genericTblMgr->m_alloc_p;

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMt__SpinLock(&genericTblMgr->m_spinLock);
#endif

    genericTbl_p = (NlmGenericTbl*)NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmGenericTbl));
    
    if(!genericTbl_p )
    {
        *o_reason = NLMRSC_LOW_MEMORY;
#if defined NLM_MT_OLD || defined NLM_MT
        NlmCmMt__SpinUnlock(&genericTblMgr->m_spinLock);
#endif
        return NULL;
    }

    /* constructor of generic table */
    genericTbl_p = NlmGenericTbl__ctor(
                            genericTbl_p, 
                            alloc_p,
                            genericTblMgr, 
                            genericTblId, 
                            genericTblWidth,
                            genericTblSize
                            );
    
    /* virtual function for creating table for some specific Table Manager*/
    err = vtbl_p->CreateTable(genericTblMgr, portNum, genericTbl_p, o_reason);
    if(NLMERR_OK != err)
    { /* if table is not being created then call table_destructor and free the memory */
        NlmGenericTbl__dtor(genericTbl_p);
        NlmCmAllocator__free(genericTbl_p->m_alloc_p, genericTbl_p);
#if defined NLM_MT_OLD || defined NLM_MT
        NlmCmMt__SpinUnlock(&genericTblMgr->m_spinLock);
#endif
        return NULL;
    }

    /* insert the table created into doubly link list */
    if( NULL == NlmGenericTblList__Insert((NlmGenericTblList*)genericTblMgr->m_genericTbl_list_p,
                                            genericTbl_p, alloc_p))
    {   /* if can not insert into list, call table_destructor and free the memory */
        NlmGenericTbl__dtor(genericTbl_p);
        NlmCmAllocator__free(alloc_p, genericTbl_p);

        *o_reason = NLMRSC_LOW_MEMORY;
#if defined NLM_MT_OLD || defined NLM_MT
        NlmCmMt__SpinUnlock(&genericTblMgr->m_spinLock);
#endif
        return NULL;
    }

    genericTblMgr->m_genericTbl_count++;

    genericTbl_p->m_adWidth = adWidth;

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMt__SpinUnlock(&genericTblMgr->m_spinLock);
#endif

    return genericTbl_p;
}
/*
Function: kbp_gtm_destroy_table
Parameters: 
    NlmGenericTblMgr  *genericTblMgr,
    NlmPortNum          portNum,
    NlmGenericTbl     *genericTbl,
    NlmReasonCode     *o_reason
Return Type: NlmErrNum_t
Description: This function destroys a table identified by genericTbl.
Once a table is destroyed, it can not participate in search and all table 
records associated with the table will deleted from device database.
Return value of 0 indicates success, and anything else is failure.
*/
NlmErrNum_t kbp_gtm_destroy_table(
    NlmGenericTblMgr  *genericTblMgr,
    NlmPortNum          portNum,
    NlmGenericTbl     *genericTbl,
    NlmReasonCode     *o_reason
    )
{
    NlmGenericTblList* node = NULL;
    NlmErrNum_t errNum = NLMERR_OK;
    NlmTblMgr__pvt_vtbl* vtbl_p = NULL;
    NlmReasonCode dummyReasonCode;

    if(o_reason == NULL)
        o_reason = &dummyReasonCode;

    *o_reason = NLMRSC_REASON_OK;

    if(NULL == genericTblMgr)
    {
        *o_reason = NLMRSC_INVALID_GENERIC_TM;
        return NLMERR_NULL_PTR;
    }

#ifndef NLM_12K_11K

    if( (portNum != NLMDEV_PORT_0) && (portNum != NLMDEV_PORT_1) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_NULL_PTR;
    }
#endif
    if(NULL == genericTbl)
    {
        *o_reason = NLMRSC_INVALID_GENERIC_TABLE;
        return NLMERR_NULL_PTR;
    }

    vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTblMgr->m_vtbl_p;

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMt__SpinLock(&genericTblMgr->m_spinLock);
#endif
    /* Free the Iter Handle */
    if(genericTbl->m_iterHandle != NULL)
    {
        genericTbl->m_iterHandle->m_tbl_p = NULL;
        NlmCmAllocator__free(genericTbl->m_alloc_p,genericTbl->m_iterHandle);
    }

    errNum = vtbl_p->DestroyTable(genericTblMgr,
                        portNum, genericTbl, o_reason);
    if(NLMERR_OK !=errNum) {
#if defined NLM_MT_OLD || defined NLM_MT
        NlmCmMt__SpinUnlock(&genericTblMgr->m_spinLock);
#endif
        return errNum;
    }

    node = (NlmGenericTblList*)genericTblMgr->m_genericTbl_list_p->m_next_p;
    while(node->m_gt_p)
    {
        if(node->m_gt_p == genericTbl)
            break;

        /* Move to the next node */
        node = (NlmGenericTblList*)node->m_next_p;
    }
    
    if(node->m_gt_p)
        NlmGenericTblList__Remove(node, genericTbl->m_alloc_p);
    else
    {
        *o_reason = NLMRSC_INVALID_GENERIC_TABLE;
#if defined NLM_MT_OLD || defined NLM_MT
        NlmCmMt__SpinUnlock(&genericTblMgr->m_spinLock);
#endif
        return NLMERR_FAIL;
    }

    genericTblMgr->m_genericTbl_count--;

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMt__SpinUnlock(&genericTblMgr->m_spinLock);
#endif

    return errNum;
}



/*
Function: kbp_gtm_config_search
Parameters: 
        NlmGenericTblMgr  *genericTblMgr, 
        NlmPortNum          portNum,
        nlm_u8             ltrNum, 
        NlmGenericTblSearchAttributes  *searchAttrs,
        NlmReasonCode    *o_reason
Return Type: NlmErrNum_t
Description: This function configures an ltr with the searches attributes. 
It is required to configure a search only after tables participating in the 
search are created.  If more than one table is configured for an LTR, then
these tables are said to be parallely searched
Return value of NLMERR_OK indicates success, and anything else is failure.
*/
NlmErrNum_t 
kbp_gtm_config_search(
        NlmGenericTblMgr  *genericTblMgr,
        NlmPortNum          portNum,
        nlm_u8             ltrNum, 
        NlmGenericTblSearchAttributes  *searchAttrs,
        NlmReasonCode    *o_reason
        )
{
    NlmErrNum_t err = NLMERR_OK;
    NlmTblMgr__pvt_vtbl* vtbl_p = NULL;
    nlm_32 i = 0, j = 0;

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    if(NULL == genericTblMgr)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_GENERIC_TM;
        return NLMERR_NULL_PTR;
    }

#ifndef NLM_12K_11K

    if( (portNum != NLMDEV_PORT_0) && (portNum != NLMDEV_PORT_1) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_NULL_PTR;
    }
#endif

    if(NLMTRUE == genericTblMgr->m_IsConfigLocked)
    {
        if(o_reason)
            *o_reason = NLMRSC_CONFIGURATION_LOCKED;
        return NLMERR_FAIL;
    }

    vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTblMgr->m_vtbl_p;

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMt__SpinLock(&genericTblMgr->m_spinLock);
#endif

    /* virtual function for search configuration of some specific Table Manager */
    err = vtbl_p->ConfigSearch(genericTblMgr, portNum,
                                ltrNum, 
                                searchAttrs,
                                o_reason
                                );

    if(err != NLMERR_OK){
#if defined NLM_MT_OLD || defined NLM_MT
        NlmCmMt__SpinUnlock(&genericTblMgr->m_spinLock);
#endif
        return err;
    }


    /*configure the parallel search dependency between the tables.
    If two tables are configured in the same LTR then they form a 
    parallel search dependency pair */
    
    /*Iterate through all possible pairs of tables which are configured for
    the LTR and set the parallel search dependency for them */
    for(i = 0; i < searchAttrs->m_numOfParallelSrches; ++i)
    {
        for(j = i+1; j < searchAttrs->m_numOfParallelSrches; ++j)
        {
            err= NlmGenericTblMgr__pvt_ConfigParallelSearchDep(
                                    genericTblMgr,
                                    searchAttrs->m_psInfo[i].m_tblId,
                                    searchAttrs->m_psInfo[j].m_tblId,
                                    o_reason);

            if(err != NLMERR_OK) {
#if defined NLM_MT_OLD || defined NLM_MT
                NlmCmMt__SpinUnlock(&genericTblMgr->m_spinLock);
#endif
                return err;
            }
        }
    }

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMt__SpinUnlock(&genericTblMgr->m_spinLock);
#endif

    
    return err;
}
/*
Function: kbp_gtm_lock_config
Parameters: 
        NlmGenericTblMgr  *genericTblMgr,
        NlmPortNum          portNum,
        NlmReasonCode    *o_reason
Return Type: NlmErrNum_t
Description: This function locks the configuration of the Table Manager.
After creating all the tables and setting the dependencies if any, this
API needs to be called. After locking the configuration, user will not be able
to change the search attributes of any table. Return value of 0 indicates 
success, and anything else is failure.
*/
NlmErrNum_t  
kbp_gtm_lock_config (
            NlmGenericTblMgr  *genericTblMgr,
            NlmPortNum          portNum,
            NlmReasonCode    *o_reason
            )
{
    NlmErrNum_t err = NLMERR_OK;
    NlmTblMgr__pvt_vtbl* vtbl_p = NULL;

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    if(NULL == genericTblMgr)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_GENERIC_TM;
        return NLMERR_FAIL;
    }

#ifndef NLM_12K_11K

    if( (portNum != NLMDEV_PORT_0) && (portNum != NLMDEV_PORT_1) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_NULL_PTR;
    }
#endif
    vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTblMgr->m_vtbl_p;

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMt__SpinLock(&genericTblMgr->m_spinLock);
#endif

    err = vtbl_p->LockConfiguration(genericTblMgr, portNum, o_reason);
    if(NLMERR_OK != err)
        return err;

    genericTblMgr->m_IsConfigLocked = NLMTRUE;

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMt__SpinUnlock(&genericTblMgr->m_spinLock);
#endif


    return NLMERR_OK;
}

/*
Function: kbp_gtm_add_record
Parameters: 
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8          *assocData,
    nlm_u16              groupId,
    nlm_u16              recordPriority, 
    NlmRecordIndex      *o_recordIndex,
    NlmReasonCode       *o_reason
Return Type: NlmErrNum_t
Description: This function adds a record to a table identified with 'genericTbl'.
It is necessary to provide the explicit priority for the record. If table is configured to have AD, 
then associated data should also be present. Index assigned 
to the record is given as output in 'o_recordIndex' Return value of 0 indicates 
success, and anything else is failure.
*/
NlmErrNum_t 
kbp_gtm_add_record(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8              *assocData,
    nlm_u16              groupId,
    nlm_u16              recordPriority, 
    NlmRecordIndex      *o_recordIndex,
    NlmReasonCode       *o_reason
    )  
{
    NlmErrNum_t errNum = NLMERR_OK;
    NlmTblMgr__pvt_vtbl* vtbl_p = NULL;
    NlmReasonCode dummyReasonCode; 

    if(!o_reason)
        o_reason = &dummyReasonCode;
    
    *o_reason = NLMRSC_REASON_OK;

    if(NULL == genericTbl)
    {
        *o_reason = NLMRSC_INVALID_GENERIC_TABLE;
        return NLMERR_FAIL;
    }
    
#ifndef NLM_12K_11K

    if( (portNum != NLMDEV_PORT_0) && (portNum != NLMDEV_PORT_1) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_NULL_PTR;
    }
#endif
    if((tblRecord == NULL) || 
        ((genericTbl->m_adWidth != NLM_TBL_ADLEN_ZERO) && (NULL == assocData)))
    {
        *o_reason = NLMRSC_INVALID_RECORD;
        return NLMERR_FAIL;
    }

    vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTbl->m_genericTblMgr_p->m_vtbl_p;  


#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMt__SpinLock(&(genericTbl->m_genericTblMgr_p->m_spinLock));
#endif


    /* if table is static TableMaxSize can not be zero. If it is zero then table is dynamic */
    if((0 !=genericTbl->m_tableMaxSize)&&(genericTbl->m_num_records >= genericTbl->m_tableMaxSize))
    {
        *o_reason = NLMRSC_TABLE_LIMIT_EXCEEDED;
#if defined NLM_MT_OLD || defined NLM_MT
        NlmCmMt__SpinUnlock(&(genericTbl->m_genericTblMgr_p->m_spinLock));
#endif
        return NLMERR_FAIL;
    }
        
    errNum =  vtbl_p->AddRecord(genericTbl, portNum,
                                tblRecord, assocData,
                                recordPriority,
                                o_recordIndex,
                                groupId,
                                o_reason
                                );

    if(errNum == NLMERR_OK)
        (genericTbl->m_num_records)++;

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMt__SpinUnlock(&(genericTbl->m_genericTblMgr_p->m_spinLock));
#endif


    return errNum;
}


/*
Function: kbp_gtm_find_record
Parameters: 
    NlmGenericTbl       *genericTbl,
    NlmGenericTblRecord *tblRecord,
    NlmRecordIndex      *o_recordIndex,
    NlmReasonCode       *o_reason
    
Return Type: NlmErrNum_t

This function finds a record identified by tblRecord in the Shadow Memory.
It is necessary to provide the tblRecord in D-M format.
Index associated with the record is given as output in 'o_recordIndex'
Return value of 0 indicates success, and anything else is failure.
*/
NlmErrNum_t 
kbp_gtm_find_record(
    NlmGenericTbl       *genericTbl,
    NlmGenericTblRecord *tblRecord,
    NlmRecordIndex      *o_recordIndex,
    NlmReasonCode       *o_reason
    )  
{
    NlmErrNum_t errNum = NLMERR_OK;
    NlmTblMgr__pvt_vtbl* vtbl_p = NULL;
    NlmReasonCode dummyReasonCode; 

    if(!o_reason)
        o_reason = &dummyReasonCode;
    
    *o_reason = NLMRSC_REASON_OK;

    if(NULL == genericTbl)
    {
        *o_reason = NLMRSC_INVALID_GENERIC_TABLE;
        return NLMERR_FAIL;
    }
    
	if( tblRecord == NULL )
    {
        *o_reason = NLMRSC_INVALID_RECORD;
        return NLMERR_FAIL;
    }

	vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTbl->m_genericTblMgr_p->m_vtbl_p;  

	errNum =  vtbl_p->FindRecord(genericTbl,tblRecord, 
								o_recordIndex,
                                o_reason
                                );
    return errNum;
}


NlmErrNum_t
kbp_gtm_get_ad_info(
    NlmGenericTbl       *genericTbl,/*table pointer : input param */
	NlmGenericTblRecord *tblRecord, /*Record information : input Param */
    NlmGenericTblAdInfoReadType readType, /*Read from H/W or S/W : input Param */
    NlmGenericTblAdInfo *adInfo, /*AD information : out Param */
    NlmReasonCode *o_reason  
    )
{
	NlmErrNum_t errNum = NLMERR_OK;
    NlmReasonCode dummyReasonCode;
	NlmRecordIndex dbaIndex = 0;
	NlmTblMgr__pvt_vtbl* vtbl_p = NULL;
	NlmTblAssoDataWidth adWidth;
	
	/*check for the error */ 
    if(!o_reason)
        o_reason = &dummyReasonCode;
    
    *o_reason = NLMRSC_REASON_OK;

    if(NULL == genericTbl)
    {
        *o_reason = NLMRSC_INVALID_GENERIC_TABLE;
        return NLMERR_FAIL;
    }
    
	if( tblRecord == NULL )
    {
        *o_reason = NLMRSC_INVALID_RECORD;
        return NLMERR_FAIL;
    }

	if((readType != GTM_AD_READ_FROM_SW) && (readType != GTM_AD_READ_FROM_HW))
	{
		*o_reason = NLMRSC_INVALID_PARAM;
		return NLMERR_FAIL;
	}
	adWidth = genericTbl->m_adWidth;

	/*if no AD for the table, return */
	if(adWidth == NLM_TBL_ADLEN_ZERO)
	{
		*o_reason = NLMRSC_INVALID_GENERIC_TABLE;
        return NLMERR_FAIL;
	}

	/*Get the DBA index of the record */
	errNum = kbp_gtm_find_record(genericTbl,tblRecord,&dbaIndex,o_reason);
	if(errNum != NLMERR_OK)
		return NLMERR_FAIL;

	if(dbaIndex == 0xFFFFFFFF)
	{
		*o_reason = NLMRSC_INVALID_RECORD;
		return NLMERR_FAIL;
	}

	/*get the virtual tbl pointer */
	vtbl_p = (NlmTblMgr__pvt_vtbl*)genericTbl->m_genericTblMgr_p->m_vtbl_p;

	/*Get the AD info */
	errNum = vtbl_p->GetAdInfo(genericTbl->m_genericTblMgr_p,
								adWidth,
								dbaIndex,
								readType,
								adInfo,
								o_reason);
	if(errNum != NLMERR_OK)
		return NLMERR_FAIL;

	return NLMERR_OK;
}



/*
Function: kbp_gtm_update_record
Parameters: 
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8          *assocData,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
Return Type: NlmErrNum_t
Description: This function updates an existing record, at given recordIndex, 
with the input valid record. If table is having AD also then table record or assoc data or both 
cab be updated. Return value of 0 indicates success, 
and anything else is failure.
*/
NlmErrNum_t 
kbp_gtm_update_record(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8              *assocData,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
    ) 
{
    NlmErrNum_t errNum = NLMERR_OK;
    NlmTblMgr__pvt_vtbl* vtbl_p = NULL;

    NlmReasonCode dummyReasonCode; 

    if(!o_reason)
        o_reason = &dummyReasonCode;

    
    *o_reason = NLMRSC_REASON_OK;

    if(NULL == genericTbl)
    {
        *o_reason = NLMRSC_INVALID_GENERIC_TABLE;
        return NLMERR_FAIL;
    }
    
#ifndef NLM_12K_11K

    if( (portNum != NLMDEV_PORT_0) && (portNum != NLMDEV_PORT_1) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_NULL_PTR;
    }
#endif

    if((tblRecord == NULL) && (NULL == assocData))
    {
        *o_reason = NLMRSC_INVALID_RECORD;
        return NLMERR_FAIL;
    }

    if((tblRecord == NULL) && (genericTbl->m_adWidth == NLM_TBL_ADLEN_ZERO))
    {
        *o_reason = NLMRSC_INVALID_RECORD;
        return NLMERR_FAIL;
    }
        

    vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTbl->m_genericTblMgr_p->m_vtbl_p;  

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMt__SpinLock( &(genericTbl->m_genericTblMgr_p->m_spinLock));
#endif

    errNum =  vtbl_p->UpdateRecord(genericTbl, portNum,
                                tblRecord, assocData,
                                recordIndex,
                                o_reason
                                );

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMt__SpinUnlock(&(genericTbl->m_genericTblMgr_p->m_spinLock));
#endif


    return errNum;
}
/*
Function: kbp_gtm_delete_record
Parameters: 
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
Return Type: NlmErrNum_t
Description: This function deletes a record from the table identified with 'genericTbl'.
The record will be deleted from the device database so this particular
record can not be searched or read afterwards. Return value of 0 indicates 
success, and anything else is failure.
*/
NlmErrNum_t 
kbp_gtm_delete_record(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
    ) 
{
    NlmErrNum_t errNum = NLMERR_OK;
    NlmTblMgr__pvt_vtbl* vtbl_p = NULL;

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    if(NULL == genericTbl)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_GENERIC_TABLE;
        return NLMERR_FAIL;
    }

#ifndef NLM_12K_11K

    if( (portNum != NLMDEV_PORT_0) && (portNum != NLMDEV_PORT_1) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_NULL_PTR;
    }
#endif

    vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTbl->m_genericTblMgr_p->m_vtbl_p;

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMt__SpinLock(&(genericTbl->m_genericTblMgr_p->m_spinLock));
#endif


    errNum = vtbl_p->DeleteRecord(genericTbl, portNum,
                                recordIndex,
                                o_reason
                                );

    if(errNum == NLMERR_OK)
        --(genericTbl->m_num_records);

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMt__SpinUnlock(&(genericTbl->m_genericTblMgr_p->m_spinLock));
#endif

    return errNum;
}

/*
Function: kbp_gtm_destroy
Parameters: 
        NlmGenericTblMgr *genericTblMgr,    
            NlmPortNum          portNum,
            NlmReasonCode    *o_reason
Return Type: NlmErrNum_t
Description: Destroys the GTM moduleand frees all its associated memory.
Before destroying the GTM, user should should destroy all existing tables.
If a table exist, GTM can not be destroyed. Return value of 0 indicates 
success, and anything else is failure.
*/
NlmErrNum_t  
kbp_gtm_destroy(
            NlmGenericTblMgr *genericTblMgr,    
            NlmPortNum       portNum,
            NlmReasonCode    *o_reason
            )
{
    NlmErrNum_t errorCode = 0;
    NlmCmAllocator *alloc_p = NULL;
    NlmTblMgr__pvt_vtbl* vtbl_p = NULL;
    NlmGenericTblList *gTblList = NULL;
    NlmGenericTblList *gTblListNode = NULL;

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    if(NULL == genericTblMgr)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_GENERIC_TM;
        return NLMERR_FAIL;
    }

#ifndef NLM_12K_11K
    if( (portNum != NLMDEV_PORT_0) && (portNum != NLMDEV_PORT_1) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_NULL_PTR;
    }
#endif

    gTblList = (NlmGenericTblList*)(genericTblMgr->m_genericTbl_list_p)->m_next_p;
    while(gTblList->m_gt_p)
    {
        gTblListNode = (NlmGenericTblList*)gTblList->m_next_p;
        kbp_gtm_destroy_table(genericTblMgr,  
                    portNum, gTblList->m_gt_p, o_reason);
        gTblList = gTblListNode;
    }
    
    vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTblMgr->m_vtbl_p;

    errorCode = vtbl_p->Destroy(genericTblMgr, portNum, o_reason);

    /*Free the Generic Table Manager */
    if(NULL !=genericTblMgr)
    {
        alloc_p = genericTblMgr->m_alloc_p;
        NlmGenericTblMgr__dtor(genericTblMgr);

#if defined NLM_MT_OLD || defined NLM_MT
        NlmCmMt__SpinDestroy( &genericTblMgr->m_spinLock, "NlmGtm_Kbp_SpinLock" );
#endif

        NlmCmAllocator__free(alloc_p, genericTblMgr);
    }

    return errorCode;
}




NlmErrNum_t
kbp_gtm_iter_next(
    NlmGenericTblIterHandle *iterHandle,
    NlmGenericTblIterData  *iterInfo,
    NlmReasonCode    *reasoncode
    )
{
    NlmErrNum_t errNum = NLMERR_OK;
    NlmTblMgr__pvt_vtbl* vtbl_p = NULL;

    /*Check for errors */
    if(reasoncode)
        *reasoncode = NLMRSC_REASON_OK;

    if(NULL == iterHandle)
    {
        if(reasoncode)
            *reasoncode = NLMRSC_INVALID_INPUT;
        return NLMERR_FAIL;
    }

    if(NULL == iterInfo)
    {
        if(reasoncode)
            *reasoncode = NLMRSC_INVALID_INPUT;
        return NLMERR_FAIL;
    }

    /*Get Virtual table pointer */
    vtbl_p =(NlmTblMgr__pvt_vtbl*)iterHandle->m_tbl_p->m_genericTblMgr_p->m_vtbl_p;

    iterHandle->isRecsAvailable = 1;
    
    /*call the table manager functions */
    errNum = vtbl_p->iter_next(iterHandle,iterInfo);

    if(iterHandle->isRecsAvailable == 0)
    {
        *reasoncode = NLMRSC_NO_RECORDS_FOUND;
        return NLMERR_FAIL;
    }
    
    return errNum;
}


NlmErrNum_t
kbp_gtm_iter_init(
    NlmGenericTbl       *genericTbl,
    NlmGenericTblIterHandle **iterHandle)
{

    if(genericTbl == NULL)
        return NLMERR_FAIL;

    /* Allocate memory for Handler */
    if(genericTbl->m_iterHandle == NULL)
    {
        genericTbl->m_iterHandle = NlmCmAllocator__calloc(genericTbl->m_alloc_p,1,sizeof(NlmGenericTblIterHandle));

        if(genericTbl->m_iterHandle == NULL)
            return NLMERR_FAIL;
        
        genericTbl->m_iterHandle->m_tbl_p = genericTbl;

    }
    genericTbl->m_iterHandle->curBlock = 0;
    genericTbl->m_iterHandle->CurRecOffset = 0;
    genericTbl->m_iterHandle->recCounter = 0;
    genericTbl->m_iterHandle->isRecsAvailable = 1;

    (*iterHandle) = genericTbl->m_iterHandle;

    return NLMERR_OK;
}

/************************************************
 * Function :
 * kbp_gtm_set_attribute
 * 
 * Parameters:
 *  NlmGenericTbl           * genericTbl,
 *  NlmGtmAttribType    attribType,
 *  nlm_u32             attribValue,
 *  NlmReasonCode   *o_reason
 *
 * Summary:
 * kbp_gtm_set_attribute sets the value of the given attribute for the given table.
 * It reuruns ok if successful otherwise returns failure.
 *
 ***********************************************/
extern NlmErrNum_t
kbp_gtm_set_attribute(
    NlmGenericTbl       *genericTbl,
    NlmGtmAttribType    attribType,     /* Attribute type*/
    nlm_u32  attribValue,       /* Attribute value*/
    NlmReasonCode *o_reason  
    )
{
    (void)attribValue;

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    if(NULL == genericTbl)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_GENERIC_TABLE;
        return NLMERR_NULL_PTR;
    }

    if(NLMTRUE == genericTbl->m_genericTblMgr_p->m_IsConfigLocked)
    {
        if(o_reason)
            *o_reason = NLMRSC_CONFIGURATION_LOCKED;
        return NLMERR_FAIL;
    }

    switch(attribType)
    {
        case NLM_GTM_ATTRIB_DEFAULT:
            genericTbl->m_keepSeparate = 0;
            break;
            
         case NLM_GTM_ATTRIB_SEPARATE_BLOCK:
            genericTbl->m_keepSeparate = 1;
            break;

        default:
            genericTbl->m_keepSeparate = 0;
            break;
    }

    return NLMERR_OK;
}

#ifndef NLMPLATFORM_BCM

NlmGenericTblMgr*
bcm_kbp_gtm_init(
        NlmCmAllocator              *alloc_p,
        void                        *devMgr_p,      
        NlmDevType                  devType,
        NlmBankNum                  bankNum,
        nlm_u8                      numOfDevices,
        NlmGenericTblMgrSBRange     *dbaSbRange,
        NlmGenericTblMgrSBRange     *udaSbRange,
        NlmIndexChangedAppCb        indexChangedAppCb,
        void                        *client_p,
        NlmReasonCode               *o_reason
        )
{
    return kbp_gtm_init(alloc_p,
        devMgr_p,      
        devType,
        bankNum,
        numOfDevices,
        dbaSbRange,
        udaSbRange,
        indexChangedAppCb,
        client_p,
        o_reason);
}


NlmErrNum_t  
bcm_kbp_gtm_destroy(
            NlmGenericTblMgr *genericTblMgr,    
            NlmPortNum          portNum,
            NlmReasonCode    *o_reason
            )
{
    return kbp_gtm_destroy(genericTblMgr,    
            portNum,
            o_reason);
}


NlmGenericTbl*
bcm_kbp_gtm_create_table(
        NlmGenericTblMgr    *genericTblMgr,
        NlmPortNum          portNum,
        nlm_u8              genericTblId, 
        NlmGenericTblWidth  genericTblWidth,        
        NlmGenericTblAssoWidth      adWidth,
        nlm_u32             genericTblSize,
        NlmReasonCode       *o_reason
        )
{
    return kbp_gtm_create_table(
        genericTblMgr,
        portNum,
        genericTblId, 
        genericTblWidth,        
        adWidth,
        genericTblSize,
        o_reason
        );
}



NlmErrNum_t  
bcm_kbp_gtm_destroy_table(
    NlmGenericTblMgr  *genericTblMgr,
    NlmPortNum        portNum,
    NlmGenericTbl     *genericTbl,
    NlmReasonCode     *o_reason
    )
{
    return kbp_gtm_destroy_table(
    genericTblMgr,
    portNum,
    genericTbl,
    o_reason
    );
}

NlmErrNum_t  
bcm_kbp_gtm_config_search(
        NlmGenericTblMgr  *genericTblMgr,
        NlmPortNum          portNum,
        nlm_u8             ltrNum, 
        NlmGenericTblSearchAttributes  *searchAttrs,
        NlmReasonCode    *o_reason
        )
{
    return kbp_gtm_config_search(
        genericTblMgr,
        portNum,
        ltrNum, 
        searchAttrs,
        o_reason
        );
}

NlmErrNum_t  
bcm_kbp_gtm_lock_config (
            NlmGenericTblMgr  *genericTblMgr,
            NlmPortNum          portNum,
            NlmReasonCode    *o_reason
            )
{
    return kbp_gtm_lock_config (
            genericTblMgr,
            portNum,
            o_reason
            );
}


NlmErrNum_t  
bcm_kbp_gtm_add_record(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8          *assocData,
    nlm_u16              groupId,
    nlm_u16              recordPriority, 
    NlmRecordIndex      *o_recordIndex,
    NlmReasonCode       *o_reason
    )
{
    return kbp_gtm_add_record(
    genericTbl,
    portNum,
    tblRecord,
    assocData,
    groupId,
    recordPriority, 
    o_recordIndex,
    o_reason
    );

}
NlmErrNum_t  
bcm_kbp_gtm_find_record(
    NlmGenericTbl       *genericTbl,
    NlmGenericTblRecord *tblRecord,
    NlmRecordIndex      *o_recordIndex,
    NlmReasonCode       *o_reason
    )
{
    return kbp_gtm_find_record(
    genericTbl,
    tblRecord,
    o_recordIndex,
    o_reason
    );

}


NlmErrNum_t 
bcm_kbp_gtm_update_record(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8          *assocData,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
    )
{
    return kbp_gtm_update_record(
    genericTbl,
    portNum,
    tblRecord,
    assocData,
    recordIndex,
    o_reason
    );
}

NlmErrNum_t  
bcm_kbp_gtm_delete_record(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
    )
{
    return kbp_gtm_delete_record(
    genericTbl,
    portNum,
    recordIndex,
    o_reason
    );
}

NlmErrNum_t
bcm_kbp_gtm_iter_next(
    NlmGenericTblIterHandle *iterHandle,
    NlmGenericTblIterData  *iterInfo,
    NlmReasonCode    *reasoncode
    )
{
    return kbp_gtm_iter_next(
    iterHandle,
    iterInfo,
    reasoncode
    );
}

NlmErrNum_t
bcm_kbp_gtm_iter_init(
    NlmGenericTbl       *genericTbl,
    NlmGenericTblIterHandle **iterHandle)
{
    return kbp_gtm_iter_init(
    genericTbl,
    iterHandle);
}


NlmErrNum_t
bcm_kbp_gtm_set_attribute(
    NlmGenericTbl       *genericTbl,
    NlmGtmAttribType    attribType,     /* Attribute type*/
    nlm_u32  attribValue,       /* Attribute value*/
    NlmReasonCode *o_reason  
    )
{
    return kbp_gtm_set_attribute(
    genericTbl,
    attribType,
    attribValue,
    o_reason  
    );
}

#endif



