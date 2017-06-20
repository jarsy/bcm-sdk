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
 
#ifndef INCLUDED_NLMRANGEMGR_REFAPP3_H
#define INCLUDED_NLMRANGEMGR_REFAPP3_H

#include "nlmcmbasic.h"
#include "nlmcmportable.h"
#include "nlmcmdevice.h"
#include "nlmcmutility.h"
#include "nlmcmstring.h"
#include "nlmrangemgr.h"
#include "nlmdevmgr.h"
#include "nlmdevmgr_shadow.h"
#include "nlmarch.h"
#include "nlmsimxpt.h"
#include "nlmxpt.h"
#include <soc/sbx/caladan3/etu_xpt.h>

#ifdef NLM_XLP
#include "nlmxlpxpt.h"
#endif

#define	RETURN_STATUS_OK	0
#define	RETURN_STATUS_FAIL	1
#define	RETURN_STATUS_ABORT	2

/* Range Manager definitions */
#define	NUM_OF_RANGE_DATABASES              1
#define	NUM_OF_RANGES_PER_DATABASE      8

#define	NLMRANGEMGR_REFAPP_BLOCK_0  0
#define NLMRANGEMGR_REFAPP_TBLID_0  0

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

	/* Transport Layer declarations */
	NlmXpt			*xpt_p;
	XptIfType		 if_type;
	nlm_u32			 channel_id;
	nlm_u32			 request_queue_len;
	nlm_u32  		 result_queue_len;

	/* Device Manager declarations */
	NlmDevMgr 	*devMgr_p;
	NlmDev		*dev_p;
	NlmDevId      devId;
	NlmDev_OperationMode  oprMode;

	/* Range Manager declarations */
	NlmRangeMgr		*rangeMgr_p;
	NlmRangeDb     	*db_p[NUM_OF_RANGE_DATABASES];
	NlmRange		 db_ranges[NUM_OF_RANGE_DATABASES][ NUM_OF_RANGES_PER_DATABASE ];
	NlmRangeDbAttrSet     db_attrs[NUM_OF_RANGE_DATABASES];
	NlmRangeSrchAttrs     db_srchAttrs;

} NlmRangeMgrRefAppData;

#include "nlmcmexterncend.h"

#endif /* INCLUDED_NLMRANGEMGR_REFAPP3_H */

