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
 
/*@@Nlm11kDevMgr shadow memory
Summary:
Shadow device can be considered as software copy of actual device.
It contains data of all registers and memory in the actual device.
The role of the shadow memory is to avoid read of various locations from the
device required in order to update the data at various locations in the device.

Note:
Device Manager only creates and destroys the shadow memory and will be using the
data in some of the registers to perform some operations correctly;

Device manager does not update the shadow memory in any of the operations;
Its the responsibility of Device Manager User to maintain the correct data in
the shadow memory; Basically user needs to update the shadow memory whenever a
write operation is performed */

#ifndef INCLUDED_NLMDEVMGR_SHADOW_H
#define INCLUDED_NLMDEVMGR_SHADOW_H

#include "nlmdevmgr.h"
#include "nlmdevmgr_ss.h"
#include "nlmcmexterncstart.h"

/* Ltr registers.  */
typedef struct Nlm11kDevShadowLtr_s
{
	Nlm11kDevBlkSelectReg		  m_blockSelect[NLM11KDEV_NUM_ARRAY_BLOCKS/64];
    Nlm11kDevSuperBlkKeyMapReg   m_superBlkKeyMap;
	Nlm11kDevParallelSrchReg	  m_parallelSrch[NLM11KDEV_NUM_ARRAY_BLOCKS/32];
	Nlm11kDevRangeInsertion0Reg  m_rangeInsert0;
    Nlm11kDevRangeInsertion1Reg  m_rangeInsert1;
    Nlm11kDevMiscelleneousReg    m_miscelleneous;
    Nlm11kDevSSReg               m_ssReg;
    Nlm11kDevKeyConstructReg     m_keyConstruct[NLM11KDEV_NUM_KEYS * NLM11KDEV_NUM_OF_KCR_PER_KEY];

}Nlm11kDevShadowLtr;

/* Contents of Array Block */
typedef struct Nlm11kDevShadowAB_s
{
	Nlm11kDevABEntry			m_abEntry[NLM11KDEV_AB_DEPTH];
	Nlm11kDevBlockConfigReg	m_blkConfig;
	Nlm11kDevBlockMaskReg		m_bmr[NLM11KDEV_NUM_OF_BMRS_PER_BLK][NLM11KDEV_NUM_OF_80B_SEGMENTS_PER_BMR];

}Nlm11kDevShadowAB;

typedef struct Nlm11kDevShadowCB_s
{
	nlm_u8 m_cbData[NLM11KDEV_CB_WIDTH_IN_BYTES];

}Nlm11kDevShadowCB;

typedef struct Nlm11kDevShadowST_s
{
	Nlm11kDevSTE	m_row[NLM11KDEV_SS_SEP];

} Nlm11kDevShadowST;




/* Shadow chip structure - contains everything as actual chip except global registers and range registers */
typedef struct Nlm11kDevShadowDevice_s
{
	Nlm11kDevShadowLtr	 *m_ltr;		/* pointer to an array of "NLM11KDEV_NUM_LTR_SET" Nlm11kDevShadowLtr */
	Nlm11kDevShadowAB   *m_arrayBlock;	/* pointer to an array of "NLM11KDEV_NUM_ARRAY_BLOCKS" Nlm11kDevShadowAB */
    Nlm11kDevShadowST   *m_st;         /* pointer to an advanced search memory */
	Nlm11kDevRangeReg   *m_rangeReg;	/* pointer to range registers */

}Nlm11kDevShadowDevice;

/* Some macros which helps application/devmgr to access Shadow Device */

#define NLM11K_GET_SHADOW_MEM_FROM_DEV_PTR(dev) (Nlm11kDevShadowDevice*)(dev->m_shadowDevice_p)
#define NLM11K_GET_SHADOW_MEM_FROM_DEVMGR_PTR(devMgr, devId) (Nlm11kDevShadowDevice*)(((Nlm11kDev*)(devMgr->m_devList_pp)[devId])->m_shadowDevice_p)


#if !defined NLM_12K_11K && !defined NLM_12K
/* Ltr registers.  */

#define	NlmDevShadowLtr	Nlm11kDevShadowLtr
#define	NlmDevShadowAB	Nlm11kDevShadowAB
#define	NlmDevShadowCB	Nlm11kDevShadowCB
#define	NlmDevShadowST		Nlm11kDevShadowST

#define	NlmDevShadowDevice		Nlm11kDevShadowDevice


/* Some macros which helps application/devmgr to access Shadow Device */

#define NLM_GET_SHADOW_MEM_FROM_DEV_PTR	 NLM11K_GET_SHADOW_MEM_FROM_DEV_PTR
#define NLM_GET_SHADOW_MEM_FROM_DEVMGR_PTR	NLM11K_GET_SHADOW_MEM_FROM_DEVMGR_PTR


#endif

#include "nlmcmexterncend.h"
#endif /* INCLUDED_NLMDEVMGR_SHADOW_H */

