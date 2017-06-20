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
 

/*@@NlmCmPfxBundle Module

   Summary
   This module creates a prefix bundle factory.

   Description
   A prefix bundle is essentially the essential parts of a prefix (see <LINK NlmCmPrefix Module>),
   its index, and an optional associated data value. The size of the associated data is
   specified during contruction and should not be changed afterwards!!!

   A prefix bundle (NlmCmPfxBundle) is always associated with a storage factory. You cannot
   access the contents of the prefix-bundle with its associated factory. This module is
   designed assuming that a factory is used to create a variety of prefix-bundles.

   To store a 32 bit prefix with no associated data, using this factory, you would take
   (4 + 4) bytes rounded to 8 byte increments gives you 8 bytes of storage with almost no
   malloc overhead (usually an additional 8 bytes). ALSO READ DOCUMENTATION IN
   NlmCmAllocator FOR MORE DETAILS.

   If you do not use this factory then the storage needed is 16 bytes for a ptr to the prefix
   itself and an additional 12 bytes for the index/prefix/assoc data structure (assuming that
   you are not allocating individual bundles and putting the bundles in an array of structs
   rather than an array of ptrs which would require an additional 12 bytes). If the assoc data
   is larger than 4 bytes, then the exact amount needed by the associated data size (rounded up
   to 2 bytes) whereas with the other scheme, we may need an additional 12 bytes per prefix.

   If you have a million prefixes, this translates into several megabytes of savings. If your
   malloc overhead is 8 bytes (which on most systems it is), your savings can be as high as 50%
   and if your assoc data is larger than 4-bytes or <= 2 bytes, then the savings are even higher.

   WE DO NOT RECOMMEND USING NlmCmPfxBundle's IF THE NUMBER OF PREFIXES YOU ARE DEALING WITH IS
   SMALL (say less than 100,000). IMPROPER USE CAN ACTUALLY LEAD TO WASTING MORE MEMORY
   THAN USING STRAIGHT MALLOC/FREE.

   Notes:
   The storage factory supplied to NlmCmAllocator can be used to allocate other types of
   objects in the same factory as well. You can safely do this as long as the size of other
   stuff is either fixed, or you have a large number of them or if your sizes are comparable to
   the size of the NlmCmPfxBundle (index + assoc + pfx-size + pfx-data).

   When creating the storage factory, it is safe to use '4' for alignment as this will create an
   efficient (both memory and performance) prefix bundle. Recommended chunk size is about 1024
   bytes.
*/

#ifndef INCLUDED_NLMCMPFXBUNDLE_H
#define INCLUDED_NLMCMPFXBUNDLE_H

#include <nlmcmbasic.h>
#include <nlmcmprefix.h>

#include <nlmcmexterncstart.h>

/* Storage for associated data and the prefix itself is created by overallocating from the
   factory. The associated data is also stored at the end of the prefix data (it is two byte)
   aligned.
*/
typedef struct NlmCmPfxBundle
{
#if defined NLM_MT_OLD || defined NLM_MT
    volatile nlm_u32	m_nIndex ;
#else
	nlm_u32	m_nIndex;
#endif

	nlm_u32 m_dirtyBit:1;
	nlm_u32 m_nSeqNum:31;
    nlm_u16	m_nPfxSize ;
    nlm_u8	m_data[2] ;		/* DO NOT ACCESS THIS */
} NlmCmPfxBundle ;

extern NlmCmPfxBundle *	NlmCmPfxBundle__create(
    NlmCmAllocator *,	/* What factory does this bundle belong to? */
    NlmCmPrefix* prefix,		/* Prefix to use to create bundle */
    nlm_u32 	ix,		/* Index to set */
    nlm_u32 	assocSize,	/* number of bytes for assoc data */
	nlm_u32 seqNum); /* Sequence number */

/*
 * The following is ideal for a a pfx-bundle to be created on the stack
 * (or into a buffer dynamically allocated by caller)...
 * but it is a loaded gun. Make sure that your buffer size is long enough
 * to hold the entire bundle. Note that you cannot destroy a bundle created
 * this way since nothing was actually allocated.
 *
 * Note: You cannot clone an item created this way. It is guaranteed to
 * crash
 *
 * Return value is the same as the buffer passed in
 */
extern NlmCmPfxBundle *	NlmCmPfxBundle__ctorIntoBuf(
    nlm_u8 *	buffer,		/* Storage for this prefix */
    nlm_u32	bufSizeInBytes,	/* Size of this storage (for debug purposes) */
    nlm_u8 *	pfxData,	/* Prefix Data.. can be NULL */
    nlm_u32	pfxLenInBits,	/* Number of bits */
    nlm_u32 	assocSize) ;	/* number of bytes for assoc data */

#define NlmCmPfxBundle__destroy(self,alloc) NlmCmAllocator__free(alloc,self)

/* Summary
   Clones a bundle.

   Description
   Since we do not store the size of the assoc-data, we have to search the factory to
   determine the size of the bundle. It will be a little slower than CloneFast. If you know
   for sure the size of the assoc-data being cloned, and you are very certain of that,
   then use CloneFast.
*/
extern NlmCmPfxBundle *	NlmCmPfxBundle__Clone(
    NlmCmPfxBundle* self,		/* Prefix bundle to clone */
    NlmCmAllocator *) ;		/* What factory does this bundle belong to? */

/* Summary
   A faster version of NlmCmPfxBundle__Clone

   Description
   Unlike NlmCmPfxBundle__Clone, NlmCmPfxBundle__CloneFast gets additional information about the
   assocSize to help determine the size of the bundle fast. Note however, that the associated
   data sizes for the original and the cloned item is expected to be the same (will assert in
   debug mode)

   Note
   Clone is safer because it guarantees an accurate copy.
*/
extern NlmCmPfxBundle * NlmCmPfxBundle__CloneFast(
    NlmCmPfxBundle* bundle,		/* Prefix bundle to clone */
    NlmCmAllocator *,		/* What factory does this bundle belong to? */
    nlm_u32 assocSize) ;		/* number of bytes for assoc data */

extern NlmCmPfxBundle * NlmCmPfxBundle__CreateFromString(
    NlmCmAllocator *,		/* What factory does this bundle belong to? */
    const nlm_u8 *prefix,		/* Prefix to use to create bundle, Can be NULL */
    nlm_u32 numbits,			/* Number of bits to use */
    nlm_u32 ix,		/* Index to set */
    nlm_u32 assocSize) ;		/* number of bytes for assoc data */

/* Converts (allocates) a Prefix Bundle into a whole prefix. */
extern NlmCmPrefix * NlmCmPfxBundle__CvtToPrefix(
    const NlmCmPfxBundle* self,
    NlmCmAllocator *) ;

/* Initialize an existing pfx from a prefix bundle */
extern NlmCmPrefix * NlmCmPfxBundle__CvtToPrefixInline(
    const NlmCmPfxBundle* self,
    NlmCmPrefix* pfx) ;

/* Converts a Prefix Bundle into a whole prefix using existing raw storage of
   appropriate size */
extern NlmCmPrefix * NlmCmPfxBundle__CvtToPrefixBuf(
    const NlmCmPfxBundle* self,
    nlm_u8* buffer,
    nlm_u32 bufSizeInBytes) ;

/* Compare two prefixes based on prefix information only. Use NlmCmPfxBundle__IsEqual if all
   you want is an equality check because it is much faster than a full blown Compare

   Returns
   0, <0 or >0 depending on whether self==other, self<other or self>other
*/
extern int		NlmCmPfxBundle__Compare(
    const NlmCmPfxBundle* self,
    const NlmCmPfxBundle* other) ;

extern NlmBool NlmCmPfxBundle__ComparePfxBundleAndPfx(
	const NlmCmPfxBundle* pfxbundle,
	const NlmCmPrefix* prefix);


/*
 * Specialized hash function for a prefix (any trailing bits are expected to be zero
 * in the last byte)
 */
#define  NlmCmPfxBundle__HashInline(bits,numbits)	NlmCmBitSet__CRC32(numbits, bits, NlmCmPfxBundle__GetNumPfxBytes(numbits))

extern nlm_u32 NlmCmPfxBundle__Hash(
    const NlmCmPfxBundle* self) ;

extern void		NlmCmPfxBundle__Print(
    NlmCmPfxBundle *self,
    NlmCmFile* fp) ;

#define NlmCmPfxBundle__AssertValid(self)		\
    NlmCmPrefix__AssertTrailingBits(			\
        NlmCmPfxBundle__GetPfxData(self), 		\
	NlmCmPfxBundle__GetPfxSize(self), 		\
	8*NlmCmPfxBundle__GetNumPfxBytes((self)->m_nPfxSize))

/* Compare two prefixes based on prefix information only.

   Returns
   NlmFalse if the prefixes are not equal and NlmTrue if they are equal.
*/
#define NlmCmPfxBundle__IsEqual(A,B) \
    (NlmCmAssert_((A)==(A),"side-effect") NlmCmAssert_((B)==(B),"side-effect") \
    (((A)->m_nPfxSize == (B)->m_nPfxSize) && (0 == NlmCm__memcmp((A)->m_data, (B)->m_data, ((A)->m_nPfxSize+7) >> 3))))

extern void	NlmCmPfxBundle__Verify(void) ;

/* Returns the number of prefix bytes used to store the prefix. */
#define NlmCmPfxBundle__GetNumPfxBytes(x)	(((nlm_u32)(x) + 7) >> 3)
#define NlmCmPfxBundle__GetNumPfxBytes2(x)	((NlmCmPfxBundle__GetNumPfxBytes(x) + 1) & ~(nlm_u32)1)

/* Returns the index of the Prefix Bundle. */
#define NlmCmPfxBundle__GetIndex(self)		(self)->m_nIndex

/* Returns the pointer to the associated data. The returned data is a uint8 * but
   it is guaranteed to be 2-byte aligned.
 */
#define NlmCmPfxBundle__GetAssocPtr(SELF)	(NlmCmAssert_((SELF)==(SELF), "side-effect") \
                                                ((nlm_u8 *)(SELF)->m_data + NlmCmPfxBundle__GetNumPfxBytes2((SELF)->m_nPfxSize)))

/* Given a Prefix bundle and the factory from which it came from, returns the size of
   the prefix.
*/
#define NlmCmPfxBundle__GetPfxSize(self)		(self)->m_nPfxSize

/* Given a Prefix bundle and the factory from which it came from, returns the data
   for the prefix
*/
#define NlmCmPfxBundle__GetPfxData(self)		((nlm_u8 *)(self)->m_data)

#define NlmCmPfxBundle__GetBitRaw(self,IX)	(NlmCmAssert_((IX)==(IX), "side-effect") \
                                                (nlm_u8)((self)->m_data[(IX)>>3] & (0x80 >> ((IX)&7))))
#define NlmCmPfxBundle__GetBit(self,IX)		(nlm_u8)((NlmCmPfxBundle__GetBitRaw(self,IX) != 0))
#define NlmCmPfxBundle__GetBitChar(self,IX)	((char)(NlmCmPfxBundle__GetBitRaw(self,IX) ? '1' : '0'))

extern void NlmCmPfxBundle__SetBit(
    NlmCmPfxBundle *	self,
    nlm_u32		ix,
    NlmBool		bit) ;	/* Must be zero or non-zero, non-zero treated as one */

extern void NlmCmPfxBundle__SetBitChar(
    NlmCmPfxBundle *	self,
    nlm_u32		ix,
    nlm_u8		bit) ;	/* Must be '0' or '1' */

extern void NlmCmPfxBundle__SetString(
    NlmCmPfxBundle * 	self,
    const char * 	str) ;	/* Must be a string of '0' and '1's. Same as calling SetBitChar repeatedly */

#include <nlmcmexterncend.h>

#endif

