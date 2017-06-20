/*
 * $Id: nlmgenerictblmgr.c,v 1.1.6.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef INCLUDED_NLMTBLMGRREFAPP_H
#define INCLUDED_NLMTBLMGRREFAPP_H


#include "nlmcmbasic.h"
#include "nlmcmportable.h"
#include "nlmcmutility.h"
#include "nlmcmstring.h"
#include "nlmgenerictblmgr.h"
#include "nlmrangemgr.h"
#include "nlmdevmgr.h"
#include "nlmarch.h"
#include "nlmsimxpt.h"
#include "nlmxpt.h"

#define	RETURN_STATUS_OK	0
#define	RETURN_STATUS_FAIL	1
#define	RETURN_STATUS_ABORT	2

/* #define WITH_INDEX_SHUFFLE  */ /* define to show the index shuffles during ACL insertions */
/* #define RAND_SEARCH_KEYS    */ /* define to show the random search keys between 2K */

/* Generic Table Manager definitions */
#define	NUM_OF_TABLES					2

#define	TABLE_ID_80B_0					0
#define	TABLE_WIDTH_80B_0				NLM_TBL_WIDTH_80
#define TABLE_AD_WIDTH_80B_0			NLM_TBL_ADLEN_64B
#define	TABLE_SIZE_80B_0				2048
#define	START_GROUPID_80B_0				0
#define	END_GROUPID_80B_0				20

#define	TABLE_ID_80B_1					1
#define	TABLE_WIDTH_80B_1				NLM_TBL_WIDTH_80
#define TABLE_AD_WIDTH_80B_1			NLM_TBL_ADLEN_32B
#define	TABLE_SIZE_80B_1				2048
#define	START_GROUPID_80B_1				0
#define	END_GROUPID_80B_1				20

#define	TABLE_WIDTH_IN_BYTES_80B	10
#define	TABLE_WIDTH_IN_BYTES_160B	20
#define	TABLE_WIDTH_IN_BYTES_320B	40
#define	TABLE_WIDTH_IN_BYTES_640B	80

#define	MAX_NUM_PARALLEL_SEARCHES	2
#define	NUM_SEARCHES				2

#define	DEBUG_PRINT_ITER_VAL		200


#define	NLM_GTM_REFAPP_SMT0_LTR_NUM		(0)

#define NLM_GTM_REFAPP_NOSMT_80B_UDA_START_SB       0
#define NLM_GTM_REFAPP_NOSMT_80B_UDA_END_SB         7
#define NLM_GTM_REFAPP_NOSMT_80B_DBA_START_SB       0
#define NLM_GTM_REFAPP_NOSMT_80B_DBA_END_SB         7


typedef enum XptIfType
{
	IFTYPE_CMODEL,
	IFTYPE_FPGA,
	IFTYPE_XLPXPT

}XptIfType;

typedef struct tableRecordInfo
{
	NlmGenericTblRecord	record;
	nlm_u16             groupId;
	nlm_u16             priority;
	NlmRecordIndex      index;
	nlm_u8				assoData[8]; /* max associcated data 64b*/
} tableRecordInfo;

typedef struct tableInfo
{
	nlm_u8				tbl_id;
	NlmGenericTblWidth  tbl_width;
	NlmGenericTblAssoWidth  tbl_assoWidth;
	nlm_u32         tbl_size;
	nlm_u32         max_recCount;
	nlm_u32         rec_count;
	nlm_u16         groupId_start;
	nlm_u16         groupId_end;
	tableRecordInfo *tblRecordInfo_p;
} tableInfo;


typedef struct globalGTMInfo_s
{
	/* Table Manager declarations */
	NlmGenericTbl		*tbl_p;
	tableInfo           tblInfo;

	/* Parallel Search Attributes */
	nlm_u8			ltr_num;

	NlmGenericTblSearchAttributes search_attrs;

    NlmGenericTblMgrSBRange udaSBRange;
	NlmGenericTblMgrSBRange dbaSBRange;
	NlmBankNum bankNum;

} globalGTMInfo;



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

	globalGTMInfo	 g_gtmInfo[NUM_OF_TABLES];

	/* GTM manager*/
	NlmGenericTblMgr	*genericTblMgr_p;

	/* Device Manager declarations */
	NlmDevMgr 	*devMgr_p;
	NlmDev    	*cascadeDev_p;
	NlmDev		*dev_p;

	NlmIndexChangedAppCb		indexChangeCB;

	NlmSMTMode  smtMode;
	NlmPortMode portMode;

} genericTblMgrRefAppData;

#include "nlmcmexterncend.h"

#endif /* INCLUDED_NLMTBLMGRREFAPP_H */
