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
 


/*@@NlmCmMath Module
   Summary
   Integral math routines. NO FLOATING POINT!

   Description
   Description goes here
*/

#ifndef INCLUDED_NLMCMMATH_H
#define INCLUDED_NLMCMMATH_H

#include <nlmcmbasic.h>
#include <nlmcmexterncstart.h> /* this should be the final include */

/* Assumes long is 32 bit 2's complement integer
   FYI: C++ rules for integer constants do not allow signs;
       so rather than -2147483648, would need to use Ox80000000
       if doing signed longs instead of doubles 
*/

#define	NlmCmMath__int16MaxLong	((signed long)0x00007FFF)
#define	NlmCmMath__uint16MaxLong	((signed long)0x0000FFFF)
#define	NlmCmMath__longMaxLong	((signed long)0x7FFFFFFF)
#define	NlmCmMath__longMinLong	((signed long)0x80000000) /*NB: cast required */

extern nlm_u32 NlmCmMath__FloorSqrt(nlm_u32 x);

extern nlm_u32 NlmCmMath__GCD(nlm_u32 x, nlm_u32 y);/* greatest common denominator */
extern int	NlmCmMath__ReverseInt8(int x) ;
extern nlm_u16	NlmCmMath__ReverseInt16(nlm_u16 val) ;
extern int	NlmCmMath__ReverseInt32(nlm_32 val) ;

/* Keep it simple: range: [longMin,longMax] */

extern char *NlmCmMath__itoa(int value, char *buffer, int index) ;


/* Summary
   Return a random number between 0 and NlmCmMath__RandomMax inclusive.

   Notes:
   Provide a good portable replacement for stdlib's rand().

   @a state is also the seed value for the random number generator -- don't modify it
   between calls, unless you want to (re)set the generator.  

   @a state is a legitimate UNSIGNED 32-bit random value; you can use it if needed.
   It's updated by every call to NlmCmMath__Random().

   Returns
   @return a signed integer value between 0 and NlmCmMath__RandomMax inclusive
 */
extern nlm_32 NlmCmMath__Random(nlm_u32 *state) ;

#define NlmCmMath__RandomMax NlmCmMath__longMaxLong


/* Summary
   Generate a uniform random value in the range [0, range).
*/
extern nlm_u32 NlmCmMath__RandomInRange(nlm_u32 *state, nlm_u32 range);


/* Summary
   return the next odd prime greater than or equal to @a lower_bound.
 */
extern nlm_u32 NlmCmMath__FindNextPrime(nlm_u32 lower_bound);

/**********************************************************************/

extern void	NlmCmMath__Verify(void);

/**********************************************************************/

#include <nlmcmexterncend.h>

#endif
