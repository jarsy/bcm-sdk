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
 
#ifndef INCLUDED_NLM11KDEVMGR_SS_H
#define INCLUDED_NLM11KDEVMGR_SS_H

/* include files */
#include "nlmcmbasic.h"
#include "nlmcmdebug.h"
#include "nlmcmallocator.h"
#include "nlmarch.h"
#include "nlmdevmgr.h"
#include "nlmcmexterncstart.h"


#define NLM11KDEV_SS_RMP_AB		8

#define NLM11KDEV_ST_BIX_ST        39
#define NLM11KDEV_ST_BIX_EN        61


#define	NLM11KDEV_SS_WIDBYT	    (NLM11KDEV_REG_LEN_IN_BYTES * 2)

#define NLM11KDEV_SS_SEP			2048
#define NLM11KDEV_SS_NUM			8

typedef enum Nlm11kDevSTWrTyp_e
{
	NLM11KDEV_ST_WRLO,
	NLM11KDEV_ST_WRHI,
	NLM11KDEV_ST_WRFU

}Nlm11kDevSTWrTyp;

typedef enum Nlm11kDevSSRmap_e
{
	NLM11KDEV_MAP_PE0 = 0,
	NLM11KDEV_MAP_PE1

} Nlm11kDevSSRmap;

typedef struct Nlm11kDevSTE_s
{
	nlm_u16	m_slr;
	nlm_u16	m_ssi_s;
	nlm_u8	m_ssi;
	nlm_u8	m_ss_abs;
	nlm_u8	m_abc;
	nlm_u8	m_ss_bsel;
	nlm_u32	m_bix;
	nlm_u8	m_bw;
	nlm_u8	m_lprv;
	nlm_u32	m_lpri;

} Nlm11kDevSTE;


typedef struct Nlm11kDevSSReg_s
{
	Nlm11kDevSSRmap	m_ss_result_map[NLM11KDEV_SS_RMP_AB];

} Nlm11kDevSSReg;


extern NlmErrNum_t Nlm11kDevMgr__STR(
	Nlm11kDev         *dev,
	nlm_u8		   st_bn,		
	nlm_u16		   st_ad,		
	Nlm11kDevSTE	   *o_dt,		
	NlmReasonCode  *o_reason
	);


extern NlmErrNum_t Nlm11kDevMgr__STW(
	Nlm11kDev       	*dev,
	nlm_u8			st_bn,		
	nlm_u16			st_ad,		
	Nlm11kDevSTE	    *st_p,		
	Nlm11kDevSTWrTyp   st_wt, 	    
	NlmReasonCode	*o_reason
	);


#if !defined NLM_12K_11K && !defined NLM_12K
/* 11K Specific APIs and Data structures with generic name i.e. "nlm" instead of "nlm11k"*/
#define NLMDEV_SS_RMP_AB		NLM11KDEV_SS_RMP_AB

#define NLMDEV_ST_BIX_ST        NLM11KDEV_ST_BIX_ST
#define NLMDEV_ST_BIX_EN        NLM11KDEV_ST_BIX_EN


#define	NLMDEV_SS_WIDBYT	    NLM11KDEV_SS_WIDBYT

#define NLMDEV_SS_SEP			NLM11KDEV_SS_SEP
#define NLMDEV_SS_NUM			NLM11KDEV_SS_NUM

typedef enum NlmDevSTWrTyp_e
{
	NLMDEV_ST_WRLO,
	NLMDEV_ST_WRHI,
	NLMDEV_ST_WRFU

}NlmDevSTWrTyp;

typedef enum NlmDevSSRmap_e
{
	NLMDEV_MAP_PE0 = 0,
	NLMDEV_MAP_PE1

} NlmDevSSRmap;

#define NlmDevSTE	Nlm11kDevSTE

#define NlmDevSSReg	Nlm11kDevSSReg


#define NlmDevMgr__STR	Nlm11kDevMgr__STR

#define NlmDevMgr__STW	Nlm11kDevMgr__STW

#endif


#include "nlmcmexterncend.h"

#endif /* INCLUDED_NLM11KDEVMGR_SS_H */


