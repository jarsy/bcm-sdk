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
 


#ifndef INCLUDED_NLMCMRBTREE_STR_H
#define INCLUDED_NLMCMRBTREE_STR_H

/* We have to use a typedef or else we get compiler errors with 'const const' */
typedef const char * NlmCmRBCnstStr ;

#define NlmCmRBNodeDataType		NlmCmRBCnstStr
#define NlmCmRBNameSuffix		Str
#define NlmCmRBNodeDataCmp(a,b)		NlmCm__strcmp(*(a), *(b))
#define NlmCmRBNodeDataPrint(out,a)	NlmCmFile__fprintf(out, "%s", *(a))
#define NlmCmRBNodeCtorWithData		0

#include <nlmcmrbtree_def.h>

/*
 * The official type names created by nlmcmrbtree-def.h are
 *
 *	NlmCmRBNodePtr	-- This is the type for a tree node
 *	NlmCmRBTreePtr	-- This is the type for a tree itself
 *	NlmCmRBItorPtr	-- This is the type for a tree iterator
 *
 * For all the public functions available for a node and the tree, see
 * nlmcmrbtree-def.h. For an example of how to use this module, see
 * nlmcmrbtree-uint32_verify.c
 *
 * The tree must be created with two additonal arguments (besides alloc)
 * They are a compare function and a print function.
 *
 */

#define NlmCmRBTreeStr__GetItemCount(self)	NlmCmRBTree__GetItemCount(self)
#define NlmCmRBItorStr__Here(self)		NlmCmRBItor__Here(self)
#define	NlmCmRBItorStr__Base(self)		NlmCmRBItor__Base(self)

#endif
