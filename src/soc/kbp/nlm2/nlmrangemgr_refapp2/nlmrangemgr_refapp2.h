/*
 * $Id: nlmrangemgr_refapp2.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef INCLUDED_NLMRANGEMGR_REFAPP_H
#define INCLUDED_NLMRANGEMGR_REFAPP_H

#include "nlmcmbasic.h"
#include "nlmcmportable.h"
#include "nlmcmdevice.h"
#include "nlmcmutility.h"
#include "nlmcmstring.h"
#include "nlmrangemgr.h"

#ifndef NLM_NO_DEVMGR
#include "nlmdevmgr.h"
#include "nlmdevmgr_shadow.h"
#include "nlmarch.h"
#include "nlmsimxpt.h"
#include "nlmxpt.h"
#include <soc/sbx/caladan3/etu_xpt.h>

#ifdef NLM_XLP
#include "nlmxlpxpt.h"
#endif

#endif

#define	RETURN_STATUS_OK	0
#define	RETURN_STATUS_FAIL	1
#define	RETURN_STATUS_ABORT	2

/* Range Manager definitions */
#define	NUM_OF_RANGE_DATABASES           4
#define	NUM_OF_RANGES_PER_DATABASE      10


#define	MAX_NUM_PARALLEL_SEARCHES	4
#define	NUM_SEARCHES				4

#define	NLMRANGEMGR_REFAPP_BLOCK_0  0
#define	NLMRANGEMGR_REFAPP_BLOCK_4	4
#define NLMRANGEMGR_REFAPP_TBLID_0  0
#define NLMRANGEMGR_REFAPP_TBLID_1  1


typedef enum XptIfType
{
	IFTYPE_CMODEL,
	IFTYPE_FPGA,
	IFTYPE_XLPXPT,
	IFTYPE_BCM_CALADAN3

}XptIfType;


typedef struct NlmRangeMgrRefAppData
{
	/* Memory Allocator declarations */
	NlmCmAllocator  *alloc_p;
	NlmCmAllocator   alloc_bdy;
#ifndef NLM_NO_DEVMGR
	/* Transport Layer declarations */
	NlmXpt			*xpt_p;
	XptIfType		 if_type;
	nlm_u32			 channel_id;
	nlm_u32			 request_queue_len;
	nlm_u32  		 result_queue_len;

	/* Device Manager declarations */	
	nlm_u32         devId;

	NlmDev_OperationMode oprMode;  
#endif    
    void	*dev_p;
    void 	*devMgr_p;   

	/* Range Manager declarations */
	NlmRangeMgr		*rangeMgr_p;
	NlmRangeDb     	*db_p[NUM_OF_RANGE_DATABASES];
	NlmRange		 db_ranges[NUM_OF_RANGE_DATABASES][ NUM_OF_RANGES_PER_DATABASE ];
	NlmRangeDbAttrSet     db_attrs[NUM_OF_RANGE_DATABASES];
	NlmRangeSrchAttrs     db_srchAttrs;

} NlmRangeMgrRefAppData;

#include "nlmcmexterncend.h"

#endif /* INCLUDED_NLMRANGEMGR_REFAPP_H */
