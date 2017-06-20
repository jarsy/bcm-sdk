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
 
/*@@Nlm_GenericTableManager Module
   Summary:
	This module manages the generic tables contaning ACLs(Access control lists) 
	database into the device and decides the contents of various registers for 
	search operations. Managing the database is a fully automated system and 
	intelligent enough to make the optimal use of database space. 
	
	This file works as API between user and the module. The file also works as an 
	upper level integration layer for all Processors. It defines user friendly 
	data structures for instructions of device and API functions 
	declarations.
*/

#ifndef INCLUDED_NLMGENERICTBLMGR_H
#define INCLUDED_NLMGENERICTBLMGR_H

#if defined NLM_MT_OLD || defined NLM_MT
#include <nlmcmmt.h>
#endif

#include "nlmcmbasic.h"
#include "nlmcmallocator.h"
#include "nlmcmdbllinklist.h"
#include "nlmerrorcodes.h"
#include "nlmarch.h"
#include "nlmcmdevice.h"
#include "nlmcmexterncstart.h"

#define	NLM_GTM_INVALID_INDEX	(0xFFFFFFFF)

#define NLM_GTM_BLK_RANGE_WHOLE_DEVICE (NLMDEV_NUM_ARRAY_BLOCKS) 
#define NLM_GTM_BLK_RANGE_NO_BLK_USED (0xFF)

#define NLM_GTM_NUM_KEYS					(NLMDEV_NUM_KEYS)
#define NLM_GTM_NUM_PARALLEL_SEARCHES       (NLMDEV_NUM_PARALLEL_SEARCHES)
#define NLM_GTM_NUM_KEY_CONSTRUCT_SEGMENTS  (NLMDEV_NUM_OF_SEGMENTS_PER_KEY)

/* 
enum for permissible width of table. Table can have width of 80 bit, 160 bit,
320 bit or 640 bit.
*/
typedef enum NlmGenericTblWidth
{
	NLM_TBL_WIDTH_80  = 80,
	NLM_TBL_WIDTH_160 = 160,
	NLM_TBL_WIDTH_320 = 320,
	NLM_TBL_WIDTH_640 = 640,

	NLM_TBL_WIDTH_END
}NlmGenericTblWidth;


typedef struct NlmGenericTblMgrBlksRange
{
    nlm_u8 m_startBlkNum;  /* Initializing start blk or end blk to NLM_GTM_BLK_RANGE_WHOLE_DEVICE
                           indicates all blocks of the device are allocated to GTM 
                           whereas initializing it to NLM_GTM_BLK_RANGE_NO_BLK_USED indicates
                           no block of the device are allocated to GTM*/ 

    nlm_u8 m_endBlkNum;     /* Note: Ensure that Start Blk < End Blk; Start Blk and End blk should be at super blk boundary */  
} NlmGenericTblMgrBlksRange;



/* 
Structure for Generic Table Manager.
Generic Table Manager consists of its name which is a string, devType(device type)
for which this GTM will be created, doubly link list of generic tables which is
maintained by GTM itself, TableCount(maintained by GTM itself), flag ConfigLocked,
tblMgr pointer which is currently a Table_Manager pointer, assigned and used
internally by GTM. vtbl(Virtual Table maintained and internally used by GTM itself).
m_client_p is a client pointer which must be provided by user. When calling the call
back functions of client, m_client_p will be required and used by user.
*/
typedef struct NlmGenericTblMgr
{
	NlmCmAllocator  *m_alloc_p;           /*  Memory allocator */    
	
    NlmDevType       m_devType;           /*  type of device GenericTblMgr is operating on */
    
    NlmGenericTblMgrBlksRange *m_genericTblBlksRange;
	
    NlmCmDblLinkList *m_genericTbl_list_p; /*  List of generic tables associated
                                              with GenericTblMgr                           */
    nlm_u16          m_genericTbl_count;  /*  Number of generic tables associated
                                              with GenericTblMgr                           */
	NlmBool			 m_IsConfigLocked;

	void*			 m_tblMgr_p;		/* This is a void pointer so that neither the application nor
									  Generic Table Manager knowns about the internal
									  data structures. */

	void*			m_vtbl_p;        /*Application should not know about the vtbl layout */
	void*			m_client_p;    

    nlm_u8          m_tblIdStrLen;   /* Length of Tbl Ids used; should be same for all tables*/

#if defined NLM_MT_OLD || defined NLM_MT

	NlmCmMtSpinlock 	m_spinLock; /* Spin lock to protect data consistency with multi-threading*/
#endif


} NlmGenericTblMgr;

/*
Structure for implementing a doubly link list of tables searched
in parallel
*/
typedef struct NlmPsTblList
{
	#include "nlmcmdbllinklistdata.h"
	nlm_8*	m_tblId;

}NlmPsTblList;



/* 
Structure representing Generic Table. 
A generic table consists of its its table_ID which is a string of 
string length specified in GTM Init, width of table records(A table can not have records
with varying lengths), list of tables which are searched in parallel and
tableMaxSize(maximum number of records the table can have). Table should have
Table manager pointer to which the table belongs to. privateData is assigned
and used for internal purpose. If the maximum number of table records are not
limited then tableMaxSize should be 0.
*/
typedef struct NlmGenericTbl
{
	NlmGenericTblMgr *m_genericTblMgr_p;  /*  NlmGenericTblMgr to which this table belongs to */
	NlmCmAllocator   *m_alloc_p;          /*  Memory allocator                                */
	nlm_8            *m_id_str_p;         /*  Table id assigned to this table                 */	
	NlmGenericTblWidth    m_width;            /*  Table width, record length                  */
	nlm_u32           m_num_records;      /*  Number of records currently in the table        */
	
	NlmPsTblList	 *m_psTblList_p;      /*  List of tables searched in parallel with
	                                          this table                                      */
	nlm_u32			 m_tableMaxSize;	  /*  maximum number of records, table will have if   
											  table is static. If table is dynamic, the MaxSize
											  will be ZERO	
											  */
	NlmBool			 m_isLtrConfigured;	  /*   True if atleast one ltr is configured for the table */
	void*			 m_privateData;
	
} NlmGenericTbl;


/*
Generic table list structure.
Doubly link list is created by inheriting nlmcmdbllinklist. 
One instance of the structure is one node contaning one generic table.
*/
typedef struct NlmGenericTblList
{
	#include "nlmcmdbllinklistdata.h"
	NlmGenericTbl* m_gt_p;

}NlmGenericTblList;



/* This structure defines key construction mapping for a parallel search.
Each key can be formed by picking upto 10 different segments of compare data
with each segment being from 1-16 bytes. A value of "0" in m_segmentNumOfBytes 
indicates that the next segments are invalid and needs to be ignored; 
Value greater than 79 for m_segmentStartByte is invalid. 
 */
typedef struct NlmGenericTblKeyConstructionMap
{	
	nlm_u8    m_segmentStartByte[NLM_GTM_NUM_KEY_CONSTRUCT_SEGMENTS];
    nlm_u8    m_segmentNumOfBytes[NLM_GTM_NUM_KEY_CONSTRUCT_SEGMENTS];
} NlmGenericTblKeyConstructionMap;

/*
This structure maps each parallel search with a result sequence number, key number and
key construct mappings.
*/
typedef struct NlmGenericTblParallelSearchInfo
{
	nlm_8  *m_tblId_p;
    nlm_u8 m_rsltPortNum;
    nlm_u8 m_keyNum;	
	NlmGenericTblKeyConstructionMap   m_kcm;
}NlmGenericTblParallelSearchInfo;

/* This structure contains the attributes associated with Generic Tables searches that are
performed for a specified LTR.
Upto 4 parallel searches can be performed per LTR
*/
typedef struct NlmGenericTblSearchAttributes
{
	nlm_u8 m_numOfParallelSrches; 
    NlmGenericTblParallelSearchInfo m_psInfo[NLM_GTM_NUM_PARALLEL_SEARCHES];    
} NlmGenericTblSearchAttributes;

typedef struct NlmGenericTblRecord
{
	nlm_u8    *m_data; /* Data portion of the record */
	nlm_u8    *m_mask; /* Mask portion of the record */
	nlm_u16    m_len;  /* Record length in bits */
} NlmGenericTblRecord;


/* Add a callback function to indicate that index has changed  */
/* Callback registered by the application. This callback is called when
the index of a record changes so that the application can make changes in ADM 
client_p is the ptr passed by the appliccation during init so that data 
needed by the application can be accessed in the callback 
*/
typedef void (*NlmIndexChangedAppCb)(
				void* client_p,   
				NlmGenericTbl* genericTbl_p, /* ptr to table whose record is shuffled  */ 
				NlmRecordIndex oldIndex,  /* old index of the record that was shuffled */
				NlmRecordIndex newIndex   /* new index of the record that was shuffled */
				);


/*
Creates and initializes the Generic Table Manager inside the control plane software.
The Generic Table Manager needs to be given a target device it should work with.
The Generic Table Manager is designed with any generic Processor architectures.
Only one Generic Table Manager can be instantiated per set of devices.
Return value of 0 indicates success, and anything else is failure. The initialized
Generic Table Manager is returned, and the pointer has to be used for further
function calls.
client_p is the pointer passed from the application, that is passed back in 
the index changed callback to the application (in case, the application wants
to access its specific data structures in the callback. 
*/
extern NlmGenericTblMgr*
NlmGenericTblMgr__Init(
		NlmCmAllocator   *alloc_p,
		void			 *devMgr_p,		
		NlmDevType		 devType,	
		nlm_u8			 numOfDevices,
        NlmGenericTblMgrBlksRange      *gtmBlksRange,
        nlm_u8              tblIdStrLen,
		NlmIndexChangedAppCb indexChangedAppCb,
		void			 *client_p,
		NlmReasonCode    *o_reason
		);

/*
Destroys the GTM moduleand frees all its associated memory.
Before destroying the GTM, user should should destroy all existing tables.
If a table exist, GTM can not be destroyed. Return value of 0 indicates 
success, and anything else is failure.
*/
extern NlmErrNum_t  
NlmGenericTblMgr__Destroy(
			NlmGenericTblMgr *genericTblMgr,	
			NlmReasonCode    *o_reason
			);

/* 
Adds a new generic table to the Generic Table Manager. A table id is assigned, which
is string of 0s and 1s. Key searched within this table must contain this table id
at the MSB portion of the key. A table can be configured with width of 80, 160, 320
or 640. Table size can be static or dynamic. If table is static Table_Size must have
a non zero value. If table is dynamic Table_Size must have zero value. Return value of 
0 indicates success, and anything else is failure.
Table pointer is returned, which can be used for further function calls.
*/
extern NlmGenericTbl*
NlmGenericTblMgr__CreateTable(
		NlmGenericTblMgr	*genericTblMgr,
		nlm_8				*genericTblIdStr, 
		NlmGenericTblWidth  genericTblWidth,
		nlm_u32				genericTblSize,
		NlmReasonCode		*o_reason
		);


/* 
This function destroys a table identified by genericTbl.
Once a table is destroyed, it can not participate in search and all table 
records associated with the table will deleted from device database.
Return value of 0 indicates success, and anything else is failure.
*/
extern NlmErrNum_t	
NlmGenericTblMgr__DestroyTable(
	NlmGenericTblMgr  *genericTblMgr,
	NlmGenericTbl     *genericTbl,
	NlmReasonCode	  *o_reason
	);

/*
This function configures an ltr with the searches attributes. 
It is required to configure a search only after tables participating in the 
search are created.  If more than one table is configured for an LTR, then
these tables are said to be parallely searched
Return value of NLMERR_OK indicates success, and anything else is failure.
*/
extern NlmErrNum_t	
NlmGenericTblMgr__ConfigSearch(
		NlmGenericTblMgr  *genericTblMgr,
		nlm_u8             ltrNum, 
		NlmGenericTblSearchAttributes  *searchAttrs,
		NlmReasonCode    *o_reason
		);

/*
This function locks the configuration of the Table Manager.
After creating all the tables and setting the dependencies if any, this
API needs to be called. After locking the configuration, user will not be able
to change the search attributes of any table. Return value of 0 indicates 
success, and anything else is failure.
*/
extern NlmErrNum_t  
NlmGenericTblMgr__LockConfiguration (
			NlmGenericTblMgr  *genericTblMgr,
			NlmReasonCode    *o_reason
			);

/* This function adds a record to a table identified with 'genericTbl'.
 * It is necessary to provide the explicit priority for the record.
 * Index assigned to the record is given as output in 'o_recordIndex'
 * Return value of 0 indicates success, and anything else is failure.
 */
extern NlmErrNum_t	
NlmGenericTblMgr__AddRecord(
	NlmGenericTbl       *genericTbl,
	NlmGenericTblRecord *tblRecord,
	nlm_u16				 groupId,
	nlm_u16              recordPriority, 
	NlmRecordIndex      *o_recordIndex,
	NlmReasonCode		*o_reason
	);

/* This function updates an existing record, at given recordIndex, 
with the input valid record. Return value of 0 indicates success, 
and anything else is failure.
*/

NlmErrNum_t	
NlmGenericTblMgr__UpdateRecord(
	NlmGenericTbl       *genericTbl,
	NlmGenericTblRecord *tblRecord,
	NlmRecordIndex      recordIndex,
	NlmReasonCode		*o_reason
	);

/* 
This function deletes a record from the table identified with 'genericTbl'.
The record will be deleted from the device database so this particular
record can not be searched or read afterwards. Return value of 0 indicates 
success, and anything else is failure.
*/
extern NlmErrNum_t	
NlmGenericTblMgr__DeleteRecord(
	NlmGenericTbl       *genericTbl,
	NlmRecordIndex		recordIndex,
	NlmReasonCode		*o_reason
	);



#include "nlmcmexterncend.h"
#endif /* INCLUDED_NLMGENERICTBLMGR_H */

