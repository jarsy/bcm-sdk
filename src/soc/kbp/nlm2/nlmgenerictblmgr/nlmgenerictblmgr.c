/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
/*
 **************************************************************************************
 Copyright 2009-2012 Broadcom Corporation

 This program is the proprietary software of Broadcom Corporation and/or its licensors,
 and may only be used, duplicated, modified or distributed pursuant to the terms and 
 conditions of a separate, written license agreement executed between you and 
 Broadcom (an "Authorized License").Except as set forth in an Authorized License, 
 Broadcom grants no license (express or implied),right to use, or waiver of any kind 
 with respect to the Software, and Broadcom expressly reserves all rights in and to 
 the Software and all intellectual property rights therein.  
 IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY 
 WAY,AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization, constitutes the 
    valuable trade secrets of Broadcom, and you shall use all reasonable efforts to 
    protect the confidentiality thereof,and to use this information only in connection
    with your use of Broadcom integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH 
    ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER 
    EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM 
    SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, 
    NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. 
    YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS 
    BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES 
    WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE 
    THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; 
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF 
    OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING 
    ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 **************************************************************************************
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

1. NlmGenericTblMgr__Init
2. NlmGenericTblMgr__Destroy
3. NlmGenericTblMgr__CreateTable
4. NlmGenericTblMgr__DestroyTable
5. NlmGenericTblMgr__ConfigSearch
6. NlmGenericTblMgr__LockConfiguration
7. NlmGenericTblMgr__AddRecord
8. NlmGenericTblMgr__UpdateRecord
9. NlmGenericTblMgr__DeleteRecord


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

|-----------------------|		|-----------------------|		|-----------------------|
|						|		|						|		|						|
|Generic Table Manager	|		|	User Application	|		|	Range Manager		|
|						|		|						|		|						|
|-----------------------|		|-----------------------|		|-----------------------|
			|								|								|	
			|								|								|
			|								|								|
			|					____________v____________					|
			|----------------->	|						|<------------------|
								|Device Manager.h/.c	|
								|						|
								|_______________________|
											|
											v
								|-----------------------|
								|						|
								|	Transport Layer		|
								|						|
								|-----------------------|

*/
/*******************************************************************************/

#include "nlmgenerictblmgr.h"
#include "nlmarch.h"
#include "nlmcmstring.h"
#include "nlmxktblmgr.h"
#include "nlmtblmgr.h"


/**** private function declarations ****/
/* constructor of Generic Table Manager */
static NlmErrNum_t NlmGenericTblMgr__ctor(
    NlmGenericTblMgr *self,
	NlmDevType		 devType,
    NlmCmAllocator   *alloc_p,
	NlmReasonCode	 *o_reason_p);

/* destructure for Generic Table Manager */
static void NlmGenericTblMgr__dtor(
    NlmGenericTblMgr *self
	);

/* constructor of generic table */
static NlmGenericTbl* NlmGenericTbl__ctor(
    NlmGenericTbl *self,
    NlmCmAllocator   *alloc_p,
	NlmGenericTblMgr *genericTblMgr_p,
	nlm_8           *id_str_p,
	nlm_u16			width,
	nlm_u32			genericTblSize  
	);

/* destructure for generic table */
static void NlmGenericTbl__dtor(
    NlmGenericTbl *self
	);


static NlmGenericTbl*
NlmGenericTblMgr__pvt_CreateTable(
		NlmGenericTblMgr	*genericTblMgr,
		nlm_8				*genericTblId_str, 
		NlmGenericTblWidth  genericTblWidth,
		nlm_u32				genericTblSize,
		NlmReasonCode		*o_reason
		);

static NlmErrNum_t	
NlmGenericTblMgr__pvt_DestroyTable(
	NlmGenericTblMgr  *genericTblMgr,
	NlmGenericTbl     *genericTbl,
	NlmReasonCode	  *o_reason
	);

static NlmErrNum_t	
NlmGenericTblMgr__pvt_ConfigSearch(
		NlmGenericTblMgr  *genericTblMgr,
		nlm_u8             ltrNum, 
		NlmGenericTblSearchAttributes  *searchAttrs,
		NlmReasonCode    *o_reason
		);


static NlmErrNum_t  
NlmGenericTblMgr__pvt_LockConfiguration (
			NlmGenericTblMgr  *genericTblMgr,
			NlmReasonCode    *o_reason
			);

static NlmErrNum_t	
NlmGenericTblMgr__pvt_AddRecord(
	NlmGenericTbl       *genericTbl,
	NlmGenericTblRecord *tblRecord,
	nlm_u16				 groupId,
	nlm_u16              recordPriority, 
	NlmRecordIndex      *o_recordIndex,
	NlmReasonCode		*o_reason
	);

static NlmErrNum_t	
NlmGenericTblMgr__pvt_UpdateRecord(
	NlmGenericTbl       *genericTbl,
	NlmGenericTblRecord *tblRecord,
	NlmRecordIndex      recordIndex,
	NlmReasonCode		*o_reason
	); 

static NlmErrNum_t	
NlmGenericTblMgr__pvt_DeleteRecord(
	NlmGenericTbl       *genericTbl,
	NlmRecordIndex		recordIndex,
	NlmReasonCode		*o_reason
	);


/*=====================================================================================*/

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

	NlmGenericTblList* head = NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmGenericTblList));

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
	NlmGenericTblList* node = NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmGenericTblList));

	if(!node) return 0;
	
	node->m_gt_p = gt_p;

	NlmCmDblLinkList__Insert((NlmCmDblLinkList*)head, (NlmCmDblLinkList*)node);

	return node;
}

/*
Function: NlmGenericTblList__Search
Parameters: NlmGenericTblList* head, nlm_8* tblId_p
Return Type: NlmGenericTblList*
Description: Searches a node in the generic table list based on the table_ID.
It takes list address "head" and tableID as input and returns node containing
table pointer which has the same tableID as input. If table is not found, it
returns NULL.
*/
NlmGenericTblList* NlmGenericTblList__Search( NlmGenericTblList* head,
													nlm_8* tblId_p )
{
	NlmGenericTblList *node;

	/* First node in the list does not contain any table, advance to next node */
	node = head->m_next_p;

	while( node != head )
	{
		if( !NlmCm__strcmp( node->m_gt_p->m_id_str_p, tblId_p ) )
			return node;

		node = node->m_next_p;
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

	NlmPsTblList* head = NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmPsTblList));

	if(!head) return 0;

	NlmCmDblLinkList__Init((NlmCmDblLinkList*)head);

	return head;
}

/*
Function: NlmPsTblList__InsertUnique
Parameters: NlmPsTblList* head, nlm_8* tableId , NlmCmAllocator* alloc_p
Return Type: NlmPsTblList*
Description: Creates a parallel search table list node containing a m_tblId as string
pointer of newly created table and inserts this node into the parallel search table list.
It takes "head" as input and returns the newly created node pointer.
*/

NlmPsTblList* 
NlmPsTblList__InsertUnique(
		NlmPsTblList* head_p,
		nlm_8* tableId , 
		NlmCmAllocator* alloc_p)
{
	/*First search if the table is already present.  */
	NlmPsTblList *curNode_p = head_p->m_next_p;
	NlmPsTblList *newNode_p = NULL;

	while(curNode_p != head_p)
	{
		if(curNode_p->m_tblId == tableId)
		{
			/*The table is already present in the list. So return */
			return curNode_p;
		}
		curNode_p = curNode_p->m_next_p;
	}

	
	/*If the table is not present in the list, then add it to the list */
	newNode_p = NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmPsTblList));

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
	NlmDevType		 devType,
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
	NlmDevType		 devType,
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
	self->m_vtbl_p = NlmCmAllocator__calloc(alloc_p, sizeof(NlmTblMgr__pvt_vtbl), 1);
	
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

	NlmCmAllocator__free(alloc_p, self->m_vtbl_p);
}

/*
Function: NlmGenericTbl__ctor
Parameters: 
	NlmGenericTbl *self,
    NlmCmAllocator   *alloc_p,
	NlmGenericTblMgr *genericTblMgr_p,
	nlm_8            *id_str_p,
	nlm_u16           width,
	nlm_u32			  genericTblSize
Return Type: NlmGenericTbl*
Description: constructor of generic table assigns the memory allocator, address
of Generic Table Manager to which this table belongs to, table ID, table ID length,
table width and table size. if this table is static, genericTblSize should not be 
zero. If it is zero then table is dynamic. It returns the same generic table 
pointer which it takes as input.
*/
static NlmGenericTbl* NlmGenericTbl__ctor(
    NlmGenericTbl *self,
    NlmCmAllocator   *alloc_p,
	NlmGenericTblMgr *genericTblMgr_p,
	nlm_8            *id_str_p,
	nlm_u16           width,
	nlm_u32			  genericTblSize
	)
{
	NlmCmAssert((alloc_p != NULL), "Invalid memory allocator provided.\n");
    
	self->m_alloc_p    = alloc_p;
	self->m_genericTblMgr_p = genericTblMgr_p;
	self->m_id_str_p = NlmCmAllocator__calloc(alloc_p, (NlmCm__strlen(id_str_p)+1), sizeof(nlm_8));
	NlmCm__strcpy(self->m_id_str_p, id_str_p);
	self->m_width = width;
	self->m_tableMaxSize = genericTblSize;

	self->m_num_records = 0;
	self->m_psTblList_p = NULL;

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

	NlmCmAllocator__free(alloc_p, self->m_id_str_p);

	if(NULL !=self->m_psTblList_p)
		NlmPsTblList__Destroy(self->m_psTblList_p, alloc_p);
}

/*
Function: NlmGenericTblMgr__Init
Parameters: 
		NlmCmAllocator   *alloc_p,
		void			 *devMgr_p,
		NlmDevType		 devType,
		nlm_u8			 numOfDevices,
        NlmGenericTblMgrBlksRange      *gtmBlksRange,
		nlm_u8              tblIdStrLen,
		NlmIndexChangedAppCb indexChangedAppCb, 
		void*				client_p,
		NlmReasonCode    *o_reason
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
NlmGenericTblMgr__Init(
		NlmCmAllocator   *alloc_p,
		void			 *devMgr_p,
		NlmDevType		 devType,
		nlm_u8			 numOfDevices,
        NlmGenericTblMgrBlksRange      *gtmBlksRange,
		nlm_u8              tblIdStrLen,
		NlmIndexChangedAppCb indexChangedAppCb,         
		void*				client_p,
		NlmReasonCode    *o_reason
		)
{
	NlmGenericTblMgr *self = NULL;
	NlmErrNum_t errNum = NLMERR_OK;

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;

    if(alloc_p == NULL)    
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_MEMALLOC_PTR;
		return NULL;
	}

    self = NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmGenericTblMgr));
    
	if(!self )
	{
		if(o_reason)
			*o_reason = NLMRSC_LOW_MEMORY;
		return NULL;
	}
    
	errNum = NlmGenericTblMgr__ctor(self,  devType, alloc_p, o_reason); 
	
	if(errNum != NLMERR_OK)
	{
		NlmCmAllocator__free(alloc_p, self);
		return NULL;
	}

	self->m_client_p = client_p;
	self->m_tblIdStrLen = tblIdStrLen;

#if defined NLM_MT_OLD || defined NLM_MT
    {
        nlm_32 ret;

	ret = NlmCmMt__SpinInit(&self->m_spinLock, 
					"NlmGtm_Kbp_SpinLock",
					NlmCmMtFlag);
        if(ret != 0)
        {
            *o_reason = NLMRSC_MT_SPINLOCK_INIT_FAILED;

            return NULL;
        }
    }
#endif

	switch(devType)
	{
		case NLM_DEVTYPE_2:
		case NLM_DEVTYPE_2_S:
        {
			/* Allocate memory for genericTableMgr_p*/
			if(NLMERR_OK != NlmTblMgr__Init(
								self,
								alloc_p,
								devMgr_p,
								numOfDevices,
                                gtmBlksRange,
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
Function: NlmGenericTblMgr__CreateTable
Parameters: 
		NlmGenericTblMgr	*genericTblMgr,
		nlm_8				*genericTblId_str, 
		NlmGenericTblWidth  genericTblWidth,
		nlm_u32				genericTblSize,
		NlmReasonCode		*o_reason
Return Type: NlmGenericTbl*
Description: Adds a new generic table to the Generic Table Manager. A table id 
is assigned, which is string of 0s and 1s. Key searched within this table must 
contain this table id at the MSB portion of the key. A table can be configured 
with width of 80, 160, 320 or 640. Table size can be static or dynamic. If table
is static Table_Size must have a non zero value. If table is dynamic Table_Size 
must have zero value. Return value of 0 indicates success, and anything else is 
failure. Table pointer is returned, which can be used for further function calls.
*/
NlmGenericTbl*
NlmGenericTblMgr__CreateTable(
		NlmGenericTblMgr	*genericTblMgr,
		nlm_8				*genericTblId_str, 
		NlmGenericTblWidth  genericTblWidth,
		nlm_u32				genericTblSize,
		NlmReasonCode		*o_reason
		)
{
	NlmGenericTbl* genericTbl_p = NULL;

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinLock(&genericTblMgr->m_spinLock);
#endif

	genericTbl_p = NlmGenericTblMgr__pvt_CreateTable(genericTblMgr,
										genericTblId_str, 
										genericTblWidth,
										genericTblSize,
										o_reason);
#if defined NLM_MT_OLD || defined NLM_MT
		NlmCmMt__SpinUnlock(&genericTblMgr->m_spinLock);
#endif
		return genericTbl_p;
}




static NlmGenericTbl*
NlmGenericTblMgr__pvt_CreateTable(
		NlmGenericTblMgr	*genericTblMgr,
		nlm_8				*genericTblId_str, 
		NlmGenericTblWidth  genericTblWidth,
		nlm_u32				genericTblSize,
		NlmReasonCode		*o_reason
		)
{
	NlmErrNum_t err = NLMERR_OK;
	NlmGenericTbl *self = NULL;
	NlmCmAllocator* alloc_p = NULL;
	NlmTblMgr__pvt_vtbl* vtbl_p = NULL;

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;

	if(NULL == genericTblMgr)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_GENERIC_TM;
		return NULL;
	}

	vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTblMgr->m_vtbl_p;

	/* Make sure table list is not NULL   */
	NlmCmAssert( genericTblMgr->m_genericTbl_list_p, "Table List NULL!!\n" );
    if(genericTblMgr->m_genericTbl_list_p == NULL)
	{
        if(o_reason)
			*o_reason = NLMRSC_INVALID_POINTER;
		return NULL;
	}
	
	if(NLMTRUE == genericTblMgr->m_IsConfigLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_CONFIGURATION_LOCKED;
		return NULL;
	}

	if(NULL == genericTblId_str ||
		NlmCm__strlen(genericTblId_str) != genericTblMgr->m_tblIdStrLen)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_TABLEID;
		return NULL;
	}
	
	
	/* memory allocation for new table */
	alloc_p = genericTblMgr->m_alloc_p;
	self = NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmGenericTbl));
    
	if(!self )
	{
		if(o_reason)
			*o_reason = NLMRSC_LOW_MEMORY;
		return NULL;
	}

	/* constructor of generic table */
    self = NlmGenericTbl__ctor(
							self, 
							alloc_p,
							genericTblMgr, 
							genericTblId_str, 
							genericTblWidth,
							genericTblSize
							);
	
	/* virtual function for creating table for some specific Table Manager*/
	err = vtbl_p->CreateTable(genericTblMgr,
							self,
							o_reason
							);
	if(NLMERR_OK != err)
	{ /* if table is not being created then call table_destructor and free the memory */
		NlmGenericTbl__dtor(self);
		NlmCmAllocator__free(self->m_alloc_p, self);
		return NULL;
	}

	/* insert the table created into doubly link list */
	if( NULL == NlmGenericTblList__Insert((NlmGenericTblList*)genericTblMgr->m_genericTbl_list_p,
											self, alloc_p))
	{	/* if can not insert into list, call table_destructor and free the memory */
		NlmGenericTbl__dtor(self);
		NlmCmAllocator__free(alloc_p, self);

		if(o_reason)
			*o_reason = NLMRSC_LOW_MEMORY;
		return NULL;
	}

	genericTblMgr->m_genericTbl_count++;
	return self;
}

/*
Function: NlmGenericTblMgr__DestroyTable
Parameters: 
	NlmGenericTblMgr  *genericTblMgr,
	NlmGenericTbl     *genericTbl,
	NlmReasonCode	  *o_reason
Return Type: NlmErrNum_t
Description: This function destroys a table identified by genericTbl.
Once a table is destroyed, it can not participate in search and all table 
records associated with the table will deleted from device database.
Return value of 0 indicates success, and anything else is failure.
*/
NlmErrNum_t	NlmGenericTblMgr__DestroyTable(
	NlmGenericTblMgr  *genericTblMgr,
	NlmGenericTbl     *genericTbl,
	NlmReasonCode	  *o_reason
	)
{
	NlmErrNum_t errNum = NLMERR_OK;

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinLock(&genericTblMgr->m_spinLock);
#endif

	errNum = NlmGenericTblMgr__pvt_DestroyTable(genericTblMgr,
										genericTbl,
										o_reason);

#if defined NLM_MT_OLD || defined NLM_MT
		NlmCmMt__SpinUnlock(&genericTblMgr->m_spinLock);
#endif
		return errNum;
}

static NlmErrNum_t	
NlmGenericTblMgr__pvt_DestroyTable(
	NlmGenericTblMgr  *genericTblMgr,
	NlmGenericTbl     *genericTbl,
	NlmReasonCode	  *o_reason
	)
{
	NlmGenericTblList* node = NULL;
	NlmErrNum_t errNum = NLMERR_OK;
	NlmTblMgr__pvt_vtbl* vtbl_p = NULL;

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;

	if(NULL == genericTblMgr)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_GENERIC_TM;
		return NLMERR_NULL_PTR;
	}
	if(NULL == genericTbl)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_GENERIC_TABLE;
		return NLMERR_NULL_PTR;
	}

	vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTblMgr->m_vtbl_p;

	errNum = vtbl_p->DestroyTable(genericTblMgr,
									genericTbl,
									o_reason);
	if(NLMERR_OK !=errNum)
		return errNum;

	node = genericTblMgr->m_genericTbl_list_p->m_next_p;
	while(node->m_gt_p)
	{
		if(node->m_gt_p == genericTbl)
			break;

		/* Move to the next node */
		node = node->m_next_p;
	}
	
	if(node->m_gt_p)
		NlmGenericTblList__Remove(node, genericTbl->m_alloc_p);
	else
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_GENERIC_TABLE;
		return NLMERR_FAIL;
	}

	genericTblMgr->m_genericTbl_count--;
	return errNum;
}


/*
Function: NlmGenericTblMgr__pvt_ConfigParallelSearchDep
Parameters: 
	NlmGenericTblMgr  *genericTblMgr,
	nlm_8 *tbl1_id,
	nlm_8 *tbl2_id,
	NlmReasonCode	  *o_reason
Return Type: NlmErrNum_t
Description: Configure the parallel search dependency between the tables.
If two tables are configured in the same LTR then they form a 
parallel search dependency pair 
*/
NlmErrNum_t	
NlmGenericTblMgr__pvt_ConfigParallelSearchDep(
		NlmGenericTblMgr  *genericTblMgr,
		nlm_8 *tbl1_id,
		nlm_8 *tbl2_id,
		NlmReasonCode    *o_reason
		)
{
	NlmGenericTbl *tbl1_p = NULL, *tbl2_p = NULL;
	NlmGenericTblList *tbl1Node_p = NULL, *tbl2Node_p = NULL;

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;

	
	/* It's not valid if tables passed are same */
	if( !NlmCm__strcmp( tbl1_id, tbl2_id ) )
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
	if( NULL == NlmPsTblList__InsertUnique( tbl1_p->m_psTblList_p, tbl2_p->m_id_str_p, tbl1_p->m_alloc_p ) )
	{
		if(o_reason)
			*o_reason = NLMRSC_LOW_MEMORY;
		return NLMERR_FAIL;
	}

	/* Insert table1 into table2's PS Dependency list */
	if( NULL == NlmPsTblList__InsertUnique( tbl2_p->m_psTblList_p, tbl1_p->m_id_str_p, tbl2_p->m_alloc_p ) )
	{
		if(o_reason)
			*o_reason = NLMRSC_LOW_MEMORY;
		return NLMERR_FAIL;
	}
	
	return NLMERR_OK;
}





/*
Function: NlmGenericTblMgr__ConfigSearch
Parameters: 
		NlmGenericTblMgr  *genericTblMgr, 
		nlm_u8             ltrNum, 
		NlmGenericTblSearchAttributes  *psAttrs,
		NlmReasonCode    *o_reason
Return Type: NlmErrNum_t
Description: This function configures an ltr with the searches attributes. 
It is required to configure a search only after tables participating in the 
search are created.  If more than one table is configured for an LTR, then
these tables are said to be parallely searched
Return value of NLMERR_OK indicates success, and anything else is failure.
*/
NlmErrNum_t	
NlmGenericTblMgr__ConfigSearch(
		NlmGenericTblMgr  *genericTblMgr,
		nlm_u8             ltrNum, 
		NlmGenericTblSearchAttributes  *searchAttrs,
		NlmReasonCode    *o_reason
		)
{
	NlmErrNum_t errNum = NLMERR_OK;

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinLock(&genericTblMgr->m_spinLock);
#endif

	errNum = NlmGenericTblMgr__pvt_ConfigSearch(genericTblMgr,
													ltrNum, 
													searchAttrs,
													o_reason);
#if defined NLM_MT_OLD || defined NLM_MT
		NlmCmMt__SpinUnlock(&genericTblMgr->m_spinLock);
#endif

	return errNum;

}


static NlmErrNum_t	
NlmGenericTblMgr__pvt_ConfigSearch(
		NlmGenericTblMgr  *genericTblMgr,
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

	if(NLMTRUE == genericTblMgr->m_IsConfigLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_CONFIGURATION_LOCKED;
		return NLMERR_FAIL;
	}

	vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTblMgr->m_vtbl_p;

	/* virtual function for search configuration of some specific Table Manager */
	err = vtbl_p->ConfigSearch(genericTblMgr,
								ltrNum, 
								searchAttrs,
								o_reason
								);

	if(err != NLMERR_OK)
		return err;


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
									searchAttrs->m_psInfo[i].m_tblId_p,
									searchAttrs->m_psInfo[j].m_tblId_p,
									o_reason);

			if(err != NLMERR_OK)
				return err;
		}
	}
	
	return err;
}


/*
Function: NlmGenericTblMgr__LockConfiguration
Parameters: 
		NlmGenericTblMgr  *genericTblMgr,
		NlmReasonCode    *o_reason
Return Type: NlmErrNum_t
Description: This function locks the configuration of the Table Manager.
After creating all the tables and setting the dependencies if any, this
API needs to be called. After locking the configuration, user will not be able
to change the search attributes of any table. Return value of 0 indicates 
success, and anything else is failure.
*/
NlmErrNum_t  
NlmGenericTblMgr__LockConfiguration (
			NlmGenericTblMgr  *genericTblMgr,
			NlmReasonCode    *o_reason
			)
{
	NlmErrNum_t errNum = NLMERR_OK;

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinLock(&genericTblMgr->m_spinLock);
#endif

	errNum = NlmGenericTblMgr__pvt_LockConfiguration (genericTblMgr,
														o_reason);
#if defined NLM_MT_OLD || defined NLM_MT
		NlmCmMt__SpinUnlock(&genericTblMgr->m_spinLock);
#endif

	return errNum;
}

static NlmErrNum_t  
NlmGenericTblMgr__pvt_LockConfiguration (
			NlmGenericTblMgr  *genericTblMgr,
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
	vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTblMgr->m_vtbl_p;

	err = vtbl_p->LockConfiguration(genericTblMgr, o_reason);
	if(NLMERR_OK != err)
		return err;

	genericTblMgr->m_IsConfigLocked = NLMTRUE;

	return NLMERR_OK;
}

/*
Function: NlmGenericTblMgr__AddRecord
Parameters: 
	NlmGenericTbl       *genericTbl,
	NlmGenericTblRecord *tblRecord,
	nlm_u16				 groupId,
	nlm_u16              recordPriority, 
	NlmRecordIndex      *o_recordIndex,
	NlmReasonCode		*o_reason
Return Type: NlmErrNum_t
Description: This function adds a record to a table identified with 'genericTbl'.
It is necessary to provide the explicit priority for the record. Index assigned 
to the record is given as output in 'o_recordIndex' Return value of 0 indicates 
success, and anything else is failure.
*/
NlmErrNum_t	
NlmGenericTblMgr__AddRecord(
	NlmGenericTbl       *genericTbl,
	NlmGenericTblRecord *tblRecord,
	nlm_u16				 groupId,
	nlm_u16              recordPriority, 
	NlmRecordIndex      *o_recordIndex,
	NlmReasonCode		*o_reason
	)  
{
	NlmErrNum_t errNum = NLMERR_OK;

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinLock(&(genericTbl->m_genericTblMgr_p->m_spinLock));
#endif

	errNum = NlmGenericTblMgr__pvt_AddRecord(genericTbl,
											tblRecord,
											groupId,
											recordPriority, 
											o_recordIndex,
											o_reason);  
#if defined NLM_MT_OLD || defined NLM_MT
		NlmCmMt__SpinUnlock(&(genericTbl->m_genericTblMgr_p->m_spinLock));
#endif

	return errNum;

}

static NlmErrNum_t	
NlmGenericTblMgr__pvt_AddRecord(
	NlmGenericTbl       *genericTbl,
	NlmGenericTblRecord *tblRecord,
	nlm_u16				 groupId,
	nlm_u16              recordPriority, 
	NlmRecordIndex      *o_recordIndex,
	NlmReasonCode		*o_reason
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

	vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTbl->m_genericTblMgr_p->m_vtbl_p;	

	/* if table is static TableMaxSize can not be zero. If it is zero then table is dynamic */
	if((0 !=genericTbl->m_tableMaxSize)&&(genericTbl->m_num_records >= genericTbl->m_tableMaxSize))
	{
		if(o_reason) *o_reason = NLMRSC_TABLE_LIMIT_EXCEEDED;
		return NLMERR_FAIL;
	}
		
	errNum =  vtbl_p->AddRecord(genericTbl,
								tblRecord,
								recordPriority,
								o_recordIndex,
								groupId,
								o_reason
								);

	if(errNum == NLMERR_OK)
		(genericTbl->m_num_records)++;

	return errNum;
}

/*
Function: NlmGenericTblMgr__UpdateRecord
Parameters: 
	NlmGenericTbl       *genericTbl,
	NlmGenericTblRecord *tblRecord,
	NlmRecordIndex      recordIndex,
	NlmReasonCode		*o_reason
Return Type: NlmErrNum_t
Description: This function updates an existing record, at given recordIndex, 
with the input valid record. Return value of 0 indicates success, 
and anything else is failure.
*/
NlmErrNum_t	
NlmGenericTblMgr__UpdateRecord(
	NlmGenericTbl       *genericTbl,
	NlmGenericTblRecord *tblRecord,
	NlmRecordIndex      recordIndex,
	NlmReasonCode		*o_reason
	) 
{
	NlmErrNum_t errNum = NLMERR_OK;

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinLock( &(genericTbl->m_genericTblMgr_p->m_spinLock));
#endif
	
	errNum = NlmGenericTblMgr__pvt_UpdateRecord(genericTbl,
												tblRecord,
												recordIndex,
												o_reason);
#if defined NLM_MT_OLD || defined NLM_MT
		NlmCmMt__SpinUnlock(&(genericTbl->m_genericTblMgr_p->m_spinLock));
#endif

	return errNum;

}

static NlmErrNum_t	
NlmGenericTblMgr__pvt_UpdateRecord(
	NlmGenericTbl       *genericTbl,
	NlmGenericTblRecord *tblRecord,
	NlmRecordIndex      recordIndex,
	NlmReasonCode		*o_reason
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

	vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTbl->m_genericTblMgr_p->m_vtbl_p;	

	errNum =  vtbl_p->UpdateRecord(genericTbl,
								tblRecord,
								recordIndex,
								o_reason
								);

	return errNum;
}


/*
Function: NlmGenericTblMgr__DeleteRecord
Parameters: 
	NlmGenericTbl       *genericTbl,
	NlmRecordIndex		recordIndex,
	NlmReasonCode		*o_reason
Return Type: NlmErrNum_t
Description: This function deletes a record from the table identified with 'genericTbl'.
The record will be deleted from the device database so this particular
record can not be searched or read afterwards. Return value of 0 indicates 
success, and anything else is failure.
*/
NlmErrNum_t	
NlmGenericTblMgr__DeleteRecord(
	NlmGenericTbl       *genericTbl,
	NlmRecordIndex		recordIndex,
	NlmReasonCode		*o_reason
	) 
{
	NlmErrNum_t	errNum = NLMERR_OK;

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinLock(&(genericTbl->m_genericTblMgr_p->m_spinLock));
#endif

	errNum = NlmGenericTblMgr__pvt_DeleteRecord(genericTbl,
											recordIndex,
											o_reason);

#if defined NLM_MT_OLD || defined NLM_MT
		NlmCmMt__SpinUnlock(&(genericTbl->m_genericTblMgr_p->m_spinLock));
#endif

	return errNum;
}

static NlmErrNum_t	
NlmGenericTblMgr__pvt_DeleteRecord(
	NlmGenericTbl       *genericTbl,
	NlmRecordIndex		recordIndex,
	NlmReasonCode		*o_reason
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

	vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTbl->m_genericTblMgr_p->m_vtbl_p;

	errNum = vtbl_p->DeleteRecord(genericTbl,
								recordIndex,
								o_reason
								);

	if(errNum == NLMERR_OK)
		--(genericTbl->m_num_records);

	return errNum;
}

/*
Function: NlmGenericTblMgr__Destroy
Parameters: 
		NlmGenericTblMgr *genericTblMgr,	
		NlmReasonCode    *o_reason
Return Type: NlmErrNum_t
Description: Destroys the GTM moduleand frees all its associated memory.
Before destroying the GTM, user should should destroy all existing tables.
If a table exist, GTM can not be destroyed. Return value of 0 indicates 
success, and anything else is failure.
*/
NlmErrNum_t  
NlmGenericTblMgr__Destroy(
			NlmGenericTblMgr *genericTblMgr,	
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

	gTblList = (NlmGenericTblList*)(genericTblMgr->m_genericTbl_list_p)->m_next_p;
	while(gTblList->m_gt_p)
	{
		gTblListNode = gTblList->m_next_p;
		NlmGenericTblMgr__DestroyTable(genericTblMgr, gTblList->m_gt_p, o_reason);
		gTblList = gTblListNode;
	}
	
	vtbl_p =(NlmTblMgr__pvt_vtbl*)genericTblMgr->m_vtbl_p;

	errorCode = vtbl_p->Destroy(genericTblMgr, o_reason);

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

















