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
 




#ifndef INCLUDED_NLMCMPREFIX_H
#define INCLUDED_NLMCMPREFIX_H

#include <nlmcmbasic.h>
#include <nlmcmallocator.h>
#include <nlmcmbitset.h>

#include <nlmcmexterncstart.h>		/* this should be the final include */

/* Summary
   The NlmCmPrefix data structure.

   Description
   The m_data member is over-indexed.

   See Also 
   NlmCmPrefix__create
*/
typedef struct NlmCmPrefix {

    nlm_u16		m_avail;	/* Number of allocated bits available for storage.
					   Always a multiple of 8. */
#if defined(__cplusplus)

    /* g++ does not allow const: implies ctor required, and so object can not be
       part of simple union */

    nlm_u16		m_inuse;	/* Number of bits actually in use. */
#else
    const nlm_u16	m_inuse;	/* Number of bits actually in use. */
#endif

    nlm_u8		m_data[4];	/* Storage for the bits. This is
                                           <B>over-indexed!</B> Also, it
                                           is sized such that the
                                           structure is 4-byte aligned,
                                           and should remain this way
                                           for portability. */

} NlmCmPrefix;


/* Given a bit location determine the byte number (starting at 0) that contains that bit. */
#define		NlmCmPrefix__GetBitByte(n)		((n) >> 3)

/* Given a bit location determine the byte number (starting at 0) that contains that bit. */
#define		NlmCmPrefix__GetBitMask(n)		(0x80 >> ((n) & 7))

/* Given a bit location determine the byte number (starting at 0) that contains that bit. */
#define		NlmCmPrefix__GetNumBytes(numBits)	(((numBits) + 7) >> 3)

/* Summary
   Allocate and initialize a NlmCmPrefix.

   Description
   Use the provided allocator to allocate memory and initialize a new
   NlmCmPrefix structure.  The NlmCmPrefix data structure over-indexes its
   #m_data member. In order to support this, the create function over-allocates
   the NlmCmPrefix enough to support the storage of @a maxBitLength bits.

   Parameters
   @param allocator Allocator to use when creating the prefix.
   @param maxBitLength Number of bits the prefix is able to store;
                       internally rounded up to the nearest multiple of 8.
   @param initialDataBitLength Length in bits of the initial data. 
   @param initialData Pointer to the initial data. Copied into the newly
                      created prefix.

   See Also
   NlmCmPrefix__destroy
*/
extern NlmCmPrefix*
NlmCmPrefix__create(
    NlmCmAllocator *	allocator, 
    unsigned 		maxBitLength,
    unsigned 		initialDataBitLength, 
    const nlm_u8 *	initialData
    );

/* Summary
   Initialize a NlmCmPrefix into a buffer already created (typically on the stack)

   Description

   While NlmCmPrefix__create is a general purpose prefix creator, this
   function is provided for performance reasons.  This function can be
   used when a prefix is created on the stack (or into an already
   allocated chunk of memory). The supplied buffer must be large enough
   to hold the prefix (note that the prefix is over-allocated). The
   rule-of-thumb to use is to make sure that the buffer being supplied
   is at least NlmCmPrefix__GetStorageSize(maxBitLength) bytes long. Do
   not delete a prefix created using this function using
   NlmCmPrefix__destroy. The buffer being supplied must be properly
   aligned for the hardware architecture.

   Note
   This function should be used for prefixes that are transitional
   (never intended to be kept in memory permanently). It avoids a call
   to malloc (and perhaps a call later to free). This can help in
   performance critical situations.

   Important
   If the buffer is allocated on the stack, declare the said buffer as
   an array of nlm_u32's (with the size appropriately adjusted). For
   instance on x86 architectures no alignment is necessary (but not
   ideally aligned objects may incur a performance penalty) and on
   other processors an incorrect aligned can cause a crash. Declaring
   the buffer as an array of nlm_u32's rather than an array of bytes
   will prevent crashes and on tolerant hardware platforms such as
   x86, improves performance.

   Parameters
   @param buffer 		Buffer used for creating the prefix.
   @param bufSizeInBytes	Size of the buffer
   @param initialDataBitLength	Length in bits of the initial data.
   @param initialData		Pointer to the initial data. Copied into the newly
                                initialized prefix.

   See Also
   NlmCmPrefix__create
*/
extern NlmCmPrefix*
NlmCmPrefix__ctorIntoBuf(
    nlm_u8            *buffer,
    nlm_u32           bufSizeInBytes,
    nlm_u32           maxBitLength,
    nlm_u32           initialDataBitLength,
    const nlm_u8      *initialData);

#ifndef __doxygen__

/* Export these private methods for friends, including NlmNcpTbl__AllocRqtPfx */
extern NlmCmPrefix*
NlmCmPrefix__pvt_ctor(
    NlmCmPrefix		*self,
    nlm_u16		initialAvail,
    nlm_u16		dataLength,
    const nlm_u8	*initialData
    );

#define NlmCmPrefix__pvt_ctorIntoBuf	NlmCmPrefix__ctorIntoBuf

NlmCmDECLARE_identity(extern, NlmCmPrefix)

#endif /*## __doxygen__ */


#ifdef __doxygen__

/* Clone a NlmCmPrefix. */
NlmCmPrefix* 
NlmCmPrefix__Clone(
    const NlmCmPrefix*	source,
    NlmCmAllocator	*allocator) ;
#else
#define NlmCmPrefix__Clone(src, alloc) NlmCmPrefix__create(alloc, src->m_inuse, src->m_inuse, src->m_data) 

#endif /*## __doxygen__ */

/* Summary
   Clean up after and free a NlmCmPrefix.

   Description
   Use when finished with NlmCmPrefixes created by NlmCmPrefix__create.

   Parameters
   @param self The NlmCmPrefix to destroy.
   @param alloc The allocator used to create the prefix.

   See Also
   NlmCmPrefix__create
*/
extern void NlmCmPrefix__destroy(
    NlmCmPrefix		*self,
    NlmCmAllocator	*alloc
    );

/* Summary
   Set the value of a prefix.

   Description
   Does not reallocate memory, but does perform bounds checking against m_avail.

   Parameters
   @param self The prefix to set.
   @param dataLength The length of the new data.
   @param data The data to copy into the prefix.

   See Also
   * NlmCmPrefix__SetString
   * NlmCmPrefix__SetBit
   * NlmCmPrefix__Append
*/
extern void NlmCmPrefix__Set(
    NlmCmPrefix		*self,           		    
    unsigned		dataLength,	    /* Length of data stream */
    const nlm_u8	*data		    /* Data stream */
    ) ;

/* Summary
   Set the value of a prefix given an ASCII string.

   Description
   Set the value of a prefix based on a NULL terminated ASCII
   string. The zero character '0' and one character '1' are
   interpreted as bits. In release-mode all non '0' characters are
   treated as '1' but in debug mode, it asserts.

   The prefix is not resized to fit, but does perform bounds checking
   against m_avail. Each byte of the prefix which is set by this
   function is first zeroed out, and then written to one bit at a
   time, leaving any unset bits at 0.

   Parameters
   @param self The prefix to set.
   @param data The string to interpret.

   See Also
   * NlmCmPrefix__Set
   * NlmCmPrefix__AppendString
 */
extern void NlmCmPrefix__SetString(
    NlmCmPrefix		*self,
    const char		*data
    );

/* Summary
   Append bits to the end of a prefix.

   Description
   Append the given bits to the end of a prefix. Does not allocate additional
   room, but does perform bounds checking.

   Parameters
   @param self The prefix to append to.
   @param dataLength Length of the data to append.
   @param data Pointer to the data to append.

   See Also
   * NlmCmPrefix__Set
   * NlmCmPrefix__AppendString
*/
extern void NlmCmPrefix__Append(
    NlmCmPrefix		*self,		    
    unsigned		dataLength,
    nlm_u8		*data
    );

/* Summary
   Append bits to a prefix based on an ASCII string.

   Description
   Append bits to a prefix as per NlmCmPrefix__Append, except the source data
   is a NULL terminated ASCII string as per NlmCmPrefix__SetString. Bounds
   checking is performed.

   Parameters
   @param self The prefix to append to.
   @param data Pointer to the data to append.

   See Also
   * NlmCmPrefix__AppendString
   * NlmCmPrefix__SetString
*/
extern void NlmCmPrefix__AppendString(
    NlmCmPrefix		*self,             
    const char		*data		    /* Zero terminated data string */
    ) ;

/* Summary
   Truncate a prefix to a given length.

   Description
   Truncate a prefix to a given length by adjusting the m_inuse field,
   padding the data with zero's if needed.
    * Does not return any data.
    * Does not free any memory.
 
   Parameters
   @param self The prefix to shorten.
   @param numBitsToUse Number of bits after truncation.
*/
extern void NlmCmPrefix__TruncateInUse(
    NlmCmPrefix		*self,
    nlm_u16		numBitsToUse
    );


/*@@NlmCmPrefix__GetBitRaw
   Summary
   Get the value of a particular bit in a prefix.

   Description
   The get bit methods return a bit at a given location. The return value is
   a zero or nonzero. This is slightly faster than GetBit because it avoids
   adjusting the value to produce a zero or a one.

   Parameters
   @param self The prefix to examine.
   @param location the bit-position to examine.

   See Also
   * NlmCmPrefix__GetBit
   * NlmCmPrefix__GetBitChar
   * NlmCmPrefix__SetBit
*/
#ifdef __doxygen__
extern nlm_u8 NlmCmPrefix__GetBitRaw(
    const NlmCmPrefix	*self,
    unsigned		location
    );
#else
#if NETL_DEBUG_LEVEL >= NETL_DEBUG_ALL	/* PHDBG */
extern nlm_u8 NlmCmPrefix__GetBitRaw(
    const NlmCmPrefix	*self,
    unsigned		location
    );
#else
#define NlmCmPrefix__GetBitRaw(self,IX) (NlmCmPrefix__pvt_GetBitRaw(self->m_data, self->m_inuse, IX))
#endif
#if NETL_DEBUG_LEVEL >= NETL_DEBUG_ALL	/* PHDBG */
extern nlm_u8 NlmCmPrefix__pvt_GetBitRaw(
    const nlm_u8* 	data,
    nlm_u32 		len,
    unsigned		location
    );
#else
#define NlmCmPrefix__pvt_GetBitRaw(data,len,IX) (NlmCmAssert_((IX)==(IX), "side-effect") \
						(nlm_u8)(data[(IX)>>3] & (0x80>>((IX)&7))))
#endif
#endif


/*@@NlmCmPrefix__GetBit
   Summary
   Get the value of a particular bit in a prefix.

   Description
   The get bit methods return a bit at a given location. The return value is
   either (nlm_u8)0 or (nlm_u8)0. The return value is a copy of the
   original data. If the requested bit is beyond m_inuse, the method asserts.
   Use NlmCmPrefix__GetBitRaw for faster performance

   Parameters
   @param self The prefix to examine.
   @param location the bit-position to examine.

   See Also
   * NlmCmPrefix__GetBitRaw
   * NlmCmPrefix__GetBitChar
   * NlmCmPrefix__SetBit
*/
#ifdef __doxygen__
extern nlm_u8 NlmCmPrefix__GetBit(
    const NlmCmPrefix	*self,
    unsigned		location
    );
#else
#if NETL_DEBUG_LEVEL >= NETL_DEBUG_ALL	/* PHDBG */
extern nlm_u8 NlmCmPrefix__GetBit(
    const NlmCmPrefix	*self,
    unsigned		location
    );
#else
#define NlmCmPrefix__GetBit(self,ix)		(NlmCmPrefix__pvt_GetBit(self->m_data, self->m_inuse, ix))
#endif
#if NETL_DEBUG_LEVEL >= NETL_DEBUG_ALL	/* PHDBG */
extern nlm_u8 NlmCmPrefix__pvt_GetBit(
    const nlm_u8* 	data,
    nlm_u32 		len,
    unsigned		location
    );
#else
#define NlmCmPrefix__pvt_GetBit(data, len, location) (nlm_u8)(NlmCmPrefix__pvt_GetBitRaw(data, len, location) != 0)
#endif
#endif


/*@@NlmCmPrefix__GetBitChar
   Summary
   Get the value if a particular bit in a prefix as a character

   Description
   Like NlmCmPrefix__GetBit, except the return value is one of (char)'0' or
   (char)'1'.

   Parameters
   @param self The prefix to examine.
   @param location The bit-position to examine.

   See Also
   * NlmCmPrefix__GetBit
   * NlmCmPrefix__SetBitChar
*/
#ifdef __doxygen__
extern char NlmCmPrefix__GetBitChar(
    NlmCmPrefix		*self,
    unsigned		location
    );
#else
#if NETL_DEBUG_LEVEL >= NETL_DEBUG_ALL	/* PHDBG */
extern char NlmCmPrefix__GetBitChar(
    NlmCmPrefix		*self,
    unsigned		location
    );
#else
#define NlmCmPrefix__GetBitChar(self,ix)		((char)(NlmCmPrefix__GetBitRaw(self,ix) ? '1' : '0'))
#endif
#endif


/*@@NlmCmPrefix__SetBit
   Summary
   Set the value of a particular bit position in a prefix.

   Description
   Value of 0 is treated as false and non-zeros are treated as trues.
   If the position is beyond the current m_inuse, the function asserts.

   Parameters
   @param self The prefix to alter.
   @param location The bit-location to set.
   @param newValue The new value to set.

   See Also
   * NlmCmPrefix__SetBitChar
   * NlmCmPrefix__GetBit
*/
#ifdef __doxygen__
extern void NlmCmPrefix__SetBit(
    NlmCmPrefix		*self,
    unsigned		location,
    nlm_u8		newValue
    );
#else
#if NETL_DEBUG_LEVEL >= NETL_DEBUG_ALL	/* PHDBG */
extern void NlmCmPrefix__SetBit(
    NlmCmPrefix		*self,
    unsigned		location,
    nlm_u8		newValue
    );
#else
#define NlmCmPrefix__SetBit(self,ix,v)		{ nlm_u32 __x = (ix) ; nlm_u8 __m = \
						  (nlm_u8)NlmCmPrefix__GetBitMask(__x), \
						  *ch = (self)->m_data+NlmCmPrefix__GetBitByte(__x) ;\
						  *ch = (nlm_u8)((v) ? (*ch |= __m) : (*ch & ~__m)) ; }
#endif
#endif


/*@@NlmCmPrefix__SetBitChar
   Summary
   Set the value of a particular bit position in a prefix.

   Description
   Set the value of a particular bit in a prefix as per NlmCmPrefix__SetBit,
   except the accepted set of values includes (char)'0' and (char)'1'. It asserts
   for all other values (but non-zero values are treated as ones in release mode)

   Parameters
   @param self The prefix to alter.
   @param location The bit-position to set.
   @param newValue The new value to set.

   See Also
   * NlmCmPrefix__SetBit
   * NlmCmPrefix__GetBitChar
*/
#ifdef __doxygen__
extern void NlmCmPrefix__SetBitChar(
    NlmCmPrefix		*self,
    unsigned		location,
    char		newValue
    );
#else
#if NETL_DEBUG_LEVEL >= NETL_DEBUG_ALL	/* PHDBG */
extern void NlmCmPrefix__SetBitChar(
    NlmCmPrefix		*self,
    unsigned		location,
    char		newValue
    );
#else
#define NlmCmPrefix__SetBitChar(self,ix,v)	NlmCmPrefix__SetBit(self,ix,(v) != '0')
#endif
#endif

/*@@NlmCmPrefix__Select
   Summary
   Initialize a prefix based on a subset of the bits in an existing prefix.

   Description
   Take a subset of bits in a prefix, and use them to initialize another
   existing prefix. Bounds checking is performed on both the source
   and destination prefix.

   Parameters
   @param self The source prefix.
   @param target The target prefix.
   @param start The starting bit position.
   @param length The number of bits to copy.
*/
extern NlmCmPrefix* NlmCmPrefix__Select(
    const NlmCmPrefix	*self,
    NlmCmPrefix*		target,
    unsigned		start,
    unsigned		length
    );


/* Summary
   Compare two prefixes lexicographically. 
   
   Description
   Lexicographically compares prefixes. That is, "10" is greater than 
   "001011". If two prefixes are lexicographically equal (e.g., "10" and
   "100" due to padding in trailing bits), then the shorter prefix wins. 
   That is, "10" is less than "100."

   Return
   Return less than 0 if prefix1 is less than prefix2, 0 if prefix1
   equals prefix2, or greater than 0 if prefix1 is greater than prefix2.

   Parameters
   @param prefix1 The first prefix to compare.
   @param prefix2 The second prefix to compare.
*/
extern int NlmCmPrefix__Compare(
    const NlmCmPrefix	*prefix1,
    const NlmCmPrefix	*prefix2
    );

/* 
 * Returns the first bit position of the prefix data that differs      
 * If one prefix compleltely encompassed by the larger prefix,
 * then  (length of shorter prefix + 1) will be returned. If the both the
 * prefixes are same then -1  is returned.
 */ 
extern nlm_32
NlmCmPrefix__FirstDifferBit(
    const nlm_u8* pfx1data,
    nlm_u32 pfx1len,
    const nlm_u8* pfx2data,
    nlm_u32 pfx2len);

/* Compare two prefixes to check for equality.

   Return
   NlmTrue for equality and NlmFalse otherwise ;

   Parameters
   @param prefix1 The first prefix to compare.
   @param prefix2 The second prefix to compare.
*/
#if NETL_DEBUG_LEVEL >= NETL_DEBUG_ALL	/* PHDBG */
extern NlmBool NlmCmPrefix__IsEqual(
    const NlmCmPrefix	*prefix1,
    const NlmCmPrefix	*prefix2
    );
#else
#define NlmCmPrefix__IsEqual(prefix1, prefix2) (NlmCmPrefix__pvt_IsEqual((prefix1)->m_data, \
						(prefix1)->m_inuse, (prefix2)->m_data, (prefix2)->m_inuse))
#endif
#ifndef __doxygen__
/* same as IsEqual() but with different signature. IsEqual() calls this method */

#if NETL_DEBUG_LEVEL >= NETL_DEBUG_ALL	/* PHDBG */
extern NlmBool
NlmCmPrefix__pvt_IsEqual( 
    const nlm_u8* pfx1data,
    nlm_u32 pfx1len,
    const nlm_u8* pfx2data,
    nlm_u32 pfx2len);
#else
#define	NlmCmPrefix__pvt_IsEqual(pfx1data, pfx1len, pfx2data, pfx2len) \
    NlmCmAssert_((pfx1len)==(pfx1len),"side-effect") \
    ((pfx1len == pfx2len) && (0 == NlmCm__memcmp(pfx1data, pfx2data, NlmCmPrefix__GetNumBytes(pfx1len))))
#endif
#endif

/* Determine if the first prefix is more 'specific' than the second.

   Return
   Return NlmTRUE if prefix1 is more 'specific' than prefix2. That is, if
   prefix2 is a proper subset of prefix1, return NlmTRUE; otherwise, return
   NlmFALSE.

   Parameters
   @param prefix1 The prefix which may be more specific than prefix2
   @param prefix2 The prefix which may be a proper subset of prefix1
*/
#if NETL_DEBUG_LEVEL >= NETL_DEBUG_ALL	/* PHDBG */
extern NlmBool NlmCmPrefix__IsMoreSpecific(
    const NlmCmPrefix	*prefix1,
    const NlmCmPrefix	*prefix2
    );
#else
#define NlmCmPrefix__IsMoreSpecific(prefix1, prefix2) ((prefix1->m_inuse <= prefix2->m_inuse ? NlmFALSE : \
	NlmCmPrefix__pvt_IsMoreSpecificEqual((prefix1)->m_data, \
	(prefix1)->m_inuse, (prefix2)->m_data, (prefix2)->m_inuse)))
#endif

/* Same as NlmCmPrefix__IsMoreSpecific, but with different signature.
   NlmCmPrefix__IsMoreSpecific calls this method.
 */
#if NETL_DEBUG_LEVEL >= NETL_DEBUG_ALL	/* PHDBG */
extern NlmBool NlmCmPrefix__IsMoreSpecificEqual(
    const NlmCmPrefix	*prefix1,
    const NlmCmPrefix	*prefix2
    );
#else
#define NlmCmPrefix__IsMoreSpecificEqual(prefix1, prefix2) ((prefix1->m_inuse < prefix2->m_inuse) ? NlmFALSE : \
	NlmCmPrefix__pvt_IsMoreSpecificEqual((prefix1)->m_data, \
	(prefix1)->m_inuse, (prefix2)->m_data, (prefix2)->m_inuse))
#endif

#ifndef __doxygen__
/* same as IsMoreSpecific() but with different signature. IsMoreSpecific() calls this method */

#define NlmCmPrefix__pvt_IsMoreSpecific(pfx1data, pfx1len, pfx2data, pfx2len) ( (pfx1len <= pfx2len) ? NlmFALSE : \
	NlmCmPrefix__pvt_IsMoreSpecificEqual(pfx1data, pfx1len, pfx2data, pfx2len))

/* IsMoreSpecific that also supports the case when the prefixes are of the same length */
NlmBool
NlmCmPrefix__pvt_IsMoreSpecificEqual(
    const nlm_u8* 	pfx1data,
    nlm_u32 		pfx1len,
    const nlm_u8* 	pfx2data,
    nlm_u32 		pfx2len);
#endif

#ifdef __doxygen__

/* Hash a prefix. */
nlm_u32 NlmCmPrefix__Hash(
    const NlmCmPrefix*	self
    ) ;
#else
#define NlmCmPrefix__Hash(self) (NlmCmBitSet__HashBitSet((self)->m_data, (self)->m_inuse))

#endif /*## __doxygen__ */

#ifdef __doxygen__
/*@@NlmCmPrefix__AssertTrailingBits
   Summary
   Asserts in debug builds that the trailing bits are set to zero

   Description
   This is a macro that gets compiled away in non-debug builds
   Most prefix related functions assume that any trailing bits are set to
   zero. This can be asserted in debug builds using this function. This macro
   essentially uses NlmCmPrefix__CheckTrailingBits to perform this check.

   Parameters
   @param pfxdata	The prefix data
   @param inuse_bits	Number of bits in use
   @param avail_bits	Number of bits allocated

   See Also 
   NlmCmPrefix__CheckTrailingBits
 */
extern void NlmCmPrefix__AssertTrailingBits(
    const nlm_u8 *	pfxdata,
    nlm_u32 		inuse_bits,
    nlm_u32 		alloc_bits) ;
#else
#define NlmCmPrefix__AssertTrailingBits(data, inuse, avail) \
    assert(NlmCmPrefix__CheckTrailingBits(data, inuse, avail))
#endif

/*@@NlmCmPrefix__CheckTrailingBits
   Summary
   Check to make sure that any trailing bits are zero.

   Description
   The assumption that any trailing bits are set to zero is made by
   all functions dealing with prefixes (very important for performance reasons)
   This function is provided so that any data that can represent a prefix
   can be checked to see if it conforms to the conventions of a prefix.
   This can be used by other data structures that also have NlmCmPrefix like
   assumptions about trailing bits.

   Note that this is not a macro/function that gets removed in a production
   build. Refer to NlmCmPrefix__AssertTrailingBits for a check that can be done
   only in debug builds

   Parameters
   @param pfxdata	The prefix data
   @param inuse_bits	Number of bits in use
   @param avail_bits	Number of bits allocated

   See Also
   NlmCmPrefix__AssertTrailingBits
*/
extern NlmBool NlmCmPrefix__CheckTrailingBits(
    const nlm_u8* 	pfxdata,
    nlm_u32 		inuse_bits,
    nlm_u32 		avail_bits) ;

#ifdef __doxygen__
/*@@NlmCmPrefix__AssertValid
   Summary
   Asserts in debug builds that the trailing bits are set to zero

   Description
   This is a macro that gets compiled away in non-debug builds
   Most prefix related functions assume that any trailing bits are set to
   zero. This can be asserted in debug builds using this function. This macro
   essentially uses NlmCmPrefix__CheckTrailingBits to perform this check.

   Parameters
   @param self		The prefix to check

   See Also 
   * NlmCmPrefix__CheckTrailingBits
   * NlmCmPrefix__AssertTrailingBits
   * NlmCmPrefix__IsValid
 */
extern void NlmCmPrefix__AssertValid(
    const NlmCmPrefix	*self) ;
#else
#define NlmCmPrefix__AssertValid(self) \
    assert(NlmCmPrefix__CheckTrailingBits(self->m_data, self->m_inuse, self->m_avail))
#endif

#ifdef __doxygen__
/*@@NlmCmPrefix__IsValid
   Summary
   Returns true if trailing bits (if any) are zero.

   Description
   The assumption that any trailing bits are set to zero is made by
   all functions dealing with prefixes (very important for performance reasons)
   This function is provided so that any data that can represent a prefix
   can be checked to see if it conforms to the conventions of a prefix.
   This can be used by other data structures that also have NlmCmPrefix like
   assumptions about trailing bits

   Note that this is not a macro/function that gets removed in a production
   build. Refer to NlmCmPrefix__AssertValid for a check that can be done
   only in debug builds

   Parameters
   @param self	The prefix data

   See Also 
   * NlmCmPrefix__AssertValid
   * NlmCmPrefix__CheckTrailingBits
   * NlmCmPrefix__AssertTrailingBits
 */
extern NlmBool NlmCmPrefix__IsValid(
    const NlmCmPrefix	*self) ;
#else
#define NlmCmPrefix__IsValid(self)	\
    NlmCmPrefix__CheckTrailingBits(self->m_data, self->m_inuse, self->m_avail)
#endif

/* Summary
   Print a prefix.

   Description
   Print the given prefix to the given NlmCmFile*.
   
   Parameters
   @param self The prefix to print.
   @param fp The NlmCmFile* to print to.
*/
extern void NlmCmPrefix__Print(
    NlmCmPrefix		*self,
    NlmCmFile*		fp
    );

/* Summary
   Print a prefix bitwise, with separator _ after every byte.

   Description
   Print the given prefix to the given NlmCmFile*.
   
   Parameters
   @param self The prefix to print.
   @param fp The NlmCmFile* to print to.
*/
extern void NlmCmPrefix__PrintWithSeparator(
    NlmCmPrefix		*self,
    NlmCmFile*		fp
    );

/* Summary
   Print the prefix to a string.

   Description
   Print the given prefix to the given string.
   
   Parameters
   @param self The prefix to print.
   @param str The char* to print to.
*/
extern void NlmCmPrefix__PrintToStr(
    NlmCmPrefix		*self,
    char		*str		
    );

/* Summary
   Print a prefix in IP notation (XXX.XXX.XXX.XXX/LENGTH)

   Description
   Print the given prefix to the given NlmCmFile * in IP notation.

   Parameters
   @param self The prefix to print
   @param fp The NlmCmFile * to print to
*/
extern void NlmCmPrefix__PrintAsIp(
    NlmCmPrefix		*self,
    NlmCmFile		*fp
    ) ;

/* Print the given bit to stdout.

   Parameters
   @param bit The bit to print. Must be one of nlm_u8(1, 0).
*/

/* Summary
   Print a prefix in IP notation (XXX.XXX.XXX.XXX/LENGTH) to a
   given string. 

   NOTE: This function assumes (char* str) the given string buffer is a 
   initialized zero terminated string otherwise the result is undetermined.  

   Description
   Print the given prefix to the given char * in IP notation.

   Parameters
   @param self The prefix to print
   @param fp The char * to print to
*/
extern void NlmCmPrefix__PrintToStrAsIp(
    NlmCmPrefix		*self,
    char		*str
    ) ;


extern void NlmCmPrefix__PrintBit(
    nlm_u8		bit
    );

/* Summary
   Split out the active length and data as a comma separated pair

    Note
    This is a macro which returns "token, token" for substitution
    into argument lists -- it is a lexical construct, not a simple expression

*/
#define NlmCmPrefix__SubLenData(pfx) \
    ( NlmCmAssert_(0 != NlmCmPrefix__Identity(pfx), "non-null typesafe value") \
      NlmCmAssert_((pfx) == (pfx), "stable macro argument") \
      pfx->m_inuse ) , \
    pfx->m_data

#ifndef __doxygen__

#define	NlmCmPrefix__GetStorageSize(nBits) (NlmCmDbgOnly_(NlmCmUint16__Identity(nBits)) \
    NLMCMMAX((sizeof(NlmCmPrefix) + NlmCmPrefix__GetNumBytes(nBits) - sizeof(((NlmCmPrefix*)0)->m_data)),\
	    (sizeof(NlmCmPrefix))))

extern NlmErrNum_t	NlmCmPrefix__Verify(void);

#endif /*## __doxygen__ */

#include <nlmcmexterncend.h>

#endif

/*[]*/
