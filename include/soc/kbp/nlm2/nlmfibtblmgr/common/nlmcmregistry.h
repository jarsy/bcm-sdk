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
 

/*@@NlmCmRegistry Module

   Summary
   Summary goes here
 
   Description
   Description goes here 
 */

#ifndef INCLUDED_NLMCMREGISTRY_H
#define INCLUDED_NLMCMREGISTRY_H

#include <nlmcmbasic.h>
#include <nlmcmexterncstart.h>	/* this should be the final include */

/* Need to describe what this is and/or does */
enum NlmCmModule_t {
    NLMCMMOD_BV, /* Basic Verify 		- FILE: various */
    NLMCMMOD_LL, /* Linked List			- FILE: "nlmcmlinkedlist"*/
    NLMCMMOD_PF, /* Prefix Factory		- FILE: "nlmcmprefixfactory"*/
    NLMCMMOD_PR, /* Prefix Formatter		- FILE: "nlmcmprefixformatter"*/
    NLMCMMOD_SA, /* S3a Simulation Architecture	- FILE: "cys3a" */
    NLMCMMOD_CP, /* Network Co-Processor API	- FILE: "cyncp" */
    NLMCMMOD_FF, /* finite fifo	    		- FILE: "nlmcmfinitefifo"*/
    NLMCMMOD_DP,	/* delay pipe			- FILE :"nlmcmdelay"*/
    NLMCMMOD_PG, /* parity generator		- FILE :"nlmcmparity"*/
    NLMCMMOD_MI, /* Misc Library 		- FILE: "cymisc"*/
    NLMCMMOD_END /* Must be last element */
};

/* Need to describe what this is and/or does */
typedef enum NlmCmModule_t NlmCmModule;

/* Need to describe what this is and/or does */
typedef enum NlmCmErrNum_t
{
    NLMCMERR_OK = 0,
    NLMCMERR_FAIL = (NLMSS_CM<<16),	/* Requested operation was unsuccessful */
    NLMCMERR_ALLOC,                      /* Allocation Error */
    NLMCMERR_TIMER,			/* Timer Verify did not pass without warnings */
    NLMCMERR_SEM_FAIL,                   /* NlmCmSemaphore general error */
    NLMCMERR_SEM_AGAIN, 			/* NlmCmSemaphore__trywait: wait would block */
    NLMCMERR_XML_BADTAG,			/* Invalid tag in XML file */
    NLMCMERR_XML_BADATTR,                /* Invalid attr in XML file */
    NLMCMERR_NOT_AVAIL,			/* Requested operation is not available */
    NLMCMERR_END				/* Must be last element */
} NlmCmErrNum_t;

#include <nlmcmexterncend.h>

#endif
