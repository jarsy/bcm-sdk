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
 
#ifndef INCLUDED_NLMTBLMGRREFAPP_H
#define INCLUDED_NLMTBLMGRREFAPP_H

#include "nlmcmbasic.h"
#include "nlmcmportable.h"
#include "nlmcmdevice.h"
#include "nlmcmutility.h"
#include "nlmcmstring.h"
#include "nlmgenerictblmgr.h"
#include "nlmrangemgr.h"
#include "nlmdevmgr.h"
#include "nlmarch.h"
#include "nlmsimxpt.h"
#include "nlmxpt.h"
#include <soc/sbx/caladan3/etu_xpt.h>

#define	RETURN_STATUS_OK	0
#define	RETURN_STATUS_FAIL	1
#define	RETURN_STATUS_ABORT	2

/* Range Manager definitions */
#define	NUM_OF_SRCPORT_DB_RANGES	12
#define	NUM_OF_DSTPORT_DB_RANGES	12

/* Generic Table Manager definitions */
#define	NUM_OF_TABLES               6
#define	TABLE_ID_MAX_LEN            9
#define TABLE_ID_LEN				(8)

#define	TABLE_ID_80B_0				"00000000"
#define	TABLE_WIDTH_80B_0			 NLM_TBL_WIDTH_80
#define	TABLE_SIZE_80B_0			 0
#define	START_GROUPID_80B_O          1
#define	END_GROUPID_80B_O            10

#define	TABLE_ID_80B_1				"00000001"
#define	TABLE_WIDTH_80B_1			 NLM_TBL_WIDTH_80
#define	TABLE_SIZE_80B_1			 0
#define	START_GROUPID_80B_1          2
#define	END_GROUPID_80B_1            11

#define	TABLE_ID_160B_2				"00000010"
#define	TABLE_WIDTH_160B_2			 NLM_TBL_WIDTH_160
#define	TABLE_SIZE_160B_2			 0
#define	START_GROUPID_160B_2         3
#define	END_GROUPID_160B_2           12

#define	TABLE_ID_160B_3				"00000011"
#define	TABLE_WIDTH_160B_3			 NLM_TBL_WIDTH_160
#define	TABLE_SIZE_160B_3			 0
#define	START_GROUPID_160B_3         8
#define	END_GROUPID_160B_3           18

#define	TABLE_ID_320B_4				"00000100"
#define	TABLE_WIDTH_320B_4			 NLM_TBL_WIDTH_320
#define	TABLE_SIZE_320B_4			 0
#define	START_GROUPID_320B_4         9
#define	END_GROUPID_320B_4           19

#define	TABLE_ID_640B_5				"00000101"
#define	TABLE_WIDTH_640B_5			 NLM_TBL_WIDTH_640
#define	TABLE_SIZE_640B_5			 0
#define	START_GROUPID_640B_5         10
#define	END_GROUPID_640B_5           20

#define	TABLE_WIDTH_IN_BYTES_80B	10
#define	TABLE_WIDTH_IN_BYTES_160B	20
#define	TABLE_WIDTH_IN_BYTES_320B	40
#define	TABLE_WIDTH_IN_BYTES_640B	80

#define	MAX_NUM_PARALLEL_SEARCHES	4
#define	NUM_SEARCHES				4

#define	DEBUG_PRINT_ITER_VAL		200

typedef enum XptIfType
{
	IFTYPE_CMODEL,
	IFTYPE_FPGA,
	IFTYPE_XLPXPT,
	IFTYPE_BCM_CALADAN3

}XptIfType;

typedef struct tableRecordInfo
{
	NlmGenericTblRecord	record;
	nlm_u16             groupId;
	nlm_u16             priority;
	NlmRecordIndex      index;
} tableRecordInfo;

typedef struct tableInfo
{
	nlm_8				tbl_id[ TABLE_ID_MAX_LEN ];
	NlmGenericTblWidth  tbl_width;
	nlm_u32         tbl_size;
	nlm_u32         max_recCount;
	nlm_u32         rec_count;
	nlm_u16         groupId_start;
	nlm_u16         groupId_end;
	tableRecordInfo *tblRecordInfo_p;
} tableInfo;





typedef struct genericTblMgrRefAppData
{
	/* Memory Allocator declarations */
	NlmCmAllocator  *alloc_p;
	NlmCmAllocator   alloc_bdy;

	/* Transport Layer declarations */
	NlmXpt			*xpt_p;
	XptIfType		 if_type;
	nlm_u32			 channel_id;
	nlm_u32			 request_queue_len;
	nlm_u32  		 result_queue_len;

	/* Device Manager declarations */
	NlmDevMgr 	*devMgr_p;
	NlmDev    	*cascadeDev_p;
	NlmDev		*dev_p;
	NlmDev_OperationMode  opr_mode;


	/* Range Manager declarations */
	NlmRangeMgr		*rangeMgr_p;
	NlmRangeDb     	*srcDb_p;
	NlmRangeDb     	*dstDb_p;
	NlmRange		 srcDb_ranges[ NUM_OF_SRCPORT_DB_RANGES ];
	NlmRange		 dstDb_ranges[ NUM_OF_DSTPORT_DB_RANGES ];		

	/* Table Manager declarations */
	NlmGenericTblMgr	*genericTblMgr_p;
	NlmGenericTbl		*tbl_p[ NUM_OF_TABLES ];
	tableInfo           tblInfo[ NUM_OF_TABLES ];

	/* Parallel Search Attributes */
	nlm_u8			ltr_num[ NUM_SEARCHES ];

	NlmGenericTblSearchAttributes search_attrs[NUM_SEARCHES];
	NlmRangeSrchAttrs range_attrs[NUM_SEARCHES];

    NlmGenericTblMgrBlksRange gtmBlkRange;

	NlmIndexChangedAppCb		indexChangeCB;

	nlm_u32             tbl_numRecords[ NUM_OF_TABLES ];  
} genericTblMgrRefAppData;

#include "nlmcmexterncend.h"

#endif /* INCLUDED_NLMTBLMGRREFAPP_H */
