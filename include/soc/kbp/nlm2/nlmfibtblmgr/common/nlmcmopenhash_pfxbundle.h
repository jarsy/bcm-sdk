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
 


#ifndef INCLUDED_NLMCMOPENHASH_PFXBUNDLE_H
#define INCLUDED_NLMCMOPENHASH_PFXBUNDLE_H

#include <nlmcmpfxbundle.h>
#include <nlmcmstring.h>
#include <nlmcmopenhash_udef.h>

typedef NlmCmPfxBundle * NlmCmPfxBundlePtr ;

#define NlmCmOpenHashPfxBundle__Hash(pfx) \
    NlmCmPfxBundle__HashInline(NlmCmPfxBundle__GetPfxData(pfx), NlmCmPfxBundle__GetPfxSize(pfx))

/* The following macros ARE available for general use */

/*
 * Remember that hash table has three states for a given slot within it
 *
 * NULL slot -- marks the terminiation of a probe sequence
 * Invalid slot -- marks a slot previously occupied by something valid
 * Others -- contains items actually inserted into the hash table
 *
 * The argument 'x' here is a NlmCmPfxBundlePtr. The argument 'slot' is
 * a pointer to NlmCmPfxBundlePtr
 */
#define NlmCmOpenHashPfxBundle__NullValue 		((NlmCmPfxBundlePtr)0)
#define NlmCmOpenHashPfxBundle__InvalidValue		((NlmCmPfxBundlePtr)(nlm_value)1)
#define NlmCmOpenHashPfxBundle__IsNullValue(x) 		((x) == NlmCmOpenHashPfxBundle__NullValue)
#define NlmCmOpenHashPfxBundle__IsInvalidValue(x)	((x) == NlmCmOpenHashPfxBundle__InvalidValue)
#define NlmCmOpenHashPfxBundle__IsValidItem(x)		((x) > NlmCmOpenHashPfxBundle__InvalidValue)
#define NlmCmOpenHashPfxBundle__Invalidate(slot)		*(slot) = NlmCmOpenHashPfxBundle__InvalidValue

/*
 * The following macros are not available for general use 
 *
 * The arguments 'x', 'a' and 'b' here is a 'NlmCmPfxBundlePtr *'
 * Note that the following macros disappear once the hash is completely defined
 */

#define NlmCmOpenHashElementType		NlmCmPfxBundlePtr
#define NlmCmOpenHashNameSuffix		PfxBundle
#define NlmCmOpenHashHashFunc(a)		NlmCmOpenHashPfxBundle__Hash(*(a))
#define NlmCmOpenHashNullValue		NlmCmOpenHashPfxBundle__NullValue
#define NlmCmOpenHashInvalidValue	NlmCmOpenHashPfxBundle__InvalidValue
#define NlmCmOpenHashIsNullValue(x)	NlmCmOpenHashPfxBundle__IsNullValue(*(x))
#define NlmCmOpenHashIsInvalidValue(x)	NlmCmOpenHashPfxBundle__IsInvalidValue(*(x))
#define NlmCmOpenHashIsValidItem(x)      NlmCmOpenHashPfxBundle__IsValidItem(*(x))
#define NlmCmOpenHashIsEqual(a,b)	NlmCmPfxBundle__IsEqual(*(a), *(b))
#define NlmCmOpenHashUseMemSet

#if 0  /* Enable the following to get probe-counts using Locate */
#define NlmCmOpenHashCountProbes
#define NlmCmOpenHashPfxBundle__Enabled
#endif

#include <nlmcmopenhash_def.h>

/*
 * The official type name created by nlmcmrarray_def.h is
 *
 *	NlmCmOpenHashPfxBundle	-- This is the type for a tree node
 *
 * See nlmcmarray_def.h for the methods created with the NlmCmOpenHashPfxBundle__
 * suffix as well as the generic NlmCmOpenHash methods
 *
 */
#define	NlmCmOpenHashPfxBundle__GetSize(self)		NlmCmOpenHash__GetSize(self)
#define	NlmCmOpenHashPfxBundle__GetCount(self)		NlmCmOpenHash__GetCount(self)
#define	NlmCmOpenHashPfxBundle__GetMaxSize(self)		NlmCmOpenHash__GetMaxSize(self)
#define	NlmCmOpenHashPfxBundle__GetAtRef(self,ix)	NlmCmOpenHash__GetAtRef(self,ix)
#define	NlmCmOpenHashPfxBundle__GetAt(self,ix)		NlmCmOpenHash__GetAt(self,ix)

#include <nlmcmexterncstart.h>

extern NlmCmPfxBundle **
NlmCmOpenHashPfxBundle__LocateWithBytes(
    NlmCmOpenHashPfxBundle * 	self,
    const nlm_u8 *		pfxdata,
    nlm_u32 			pfxlen) ;

extern NlmBool
NlmCmOpenHashPfxBundle__Locate2WithBytes(
    NlmCmOpenHashPfxBundle * 	self,
    const nlm_u8 *		pfxdata,
    nlm_u32 			pfxlen,
    NlmCmPfxBundle *** 		ret) ;

/* If hash table contains lots of deleted slots then reconstruct the hash table */
extern void
NlmCmOpenHashPfxBundle__CheckAndReconst(
    NlmCmOpenHashPfxBundle * 	self);

#include<nlmcmexterncend.h>
#endif
