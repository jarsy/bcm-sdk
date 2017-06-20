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
 


#ifndef INCLUDED_NLMCMBITSET_H
#define INCLUDED_NLMCMBITSET_H

#include <nlmcmbasic.h>
#include <nlmcmallocator.h>

#include <nlmcmexterncstart.h> /* this should be the final include */

/*@@NlmCmBitSet Module

   Summary
   Utility module that provides a variable length bit-set.

   Description
   NlmCmBitSet provides an array-like access to individual bits as well as
   access to bit-fields (a sequence of bits). The storage used by NlmCmBitSet
   is essentially one bit per value rounded to next byte.

   The NlmCmBitSet is a datatype for representing the arbitrary bit size
   data.  Bit index origin is 0.

   NOTE
   Bit sets can be of arbitrary size > 0.  However, the storage allocated
   and processed is always rounded up to a byte boundary.  Trailing bits
   will be processed, and perhaps corrupted.

   And, obviously, bit set storage always starts on a byte boundary.

   Trying to process and preserve those last bits was too error prone.
   Since we don't expect anyone to require it, don't do it.
*/



/*NlmCmBitSet Data Structure.
  Pointer to storage. Client owns storage unless m_alloc_p is present.
*/
typedef struct NlmCmBitSet
{
    nlm_u8		*m_bits_p;		/* Pointer to storage */
    size_t		m_size_in_bits;		/* Size in bits. */
    NlmCmAllocator	*m_alloc_p;		/* Allocator. Responsible for
						   storage pointed to by m_bits_p.
						*/
} NlmCmBitSet ;

#ifndef __doxygen__
NlmCmDECLARE_constcast(NlmCmBitSet)
#endif

/* Summary
   Cast away the const-ness of a type-safe manner.
   
   Details
   Implements a type-safe const cast for NlmCmBitSet in debug builds only.
   Release builds are not type safe, and remove all overhead.
*/
#define NlmCmBitSet__constcast(bs) NlmCmUSE_constcast(NlmCmBitSet, bs)


/* Summary
   Initialize the BitSet

   Description
   Given a pointer to an already allocated NlmCmBitSet object,
   initialize it to passed values.
   
   Returns
   NlmCmBitSet pointer on success, NULL on error.
   
   Parameters
   @param self The object to be initilized.
   @param bits Pointer to the bit string 
   @param size_in_bits The NlmCmBitSet size
   @param alloc NlmCmAllocator object pointer
   @param isInitialized TRUE iff bits are initialized
                        FALSE iff bits will be cleared to 0
  
   Note:
   As noted below, you may pass in storage (bits) or an allocator
   (alloc); NOT both.
  
   See Also
   NlmCmBitSet__dtor
*/
extern NlmCmBitSet* NlmCmBitSet__ctor(
    NlmCmBitSet		*self, 
    nlm_u8		*bits,		/*If bits is passed in, do NOT pass in alloc */
    size_t		size_in_bits,
    NlmCmAllocator	*alloc,		/*If alloc is passed in, do NOT pass in bits*/
    NlmBool		isInitialized	/* TRUE if bits are initialized,
					   FALSE => bits will be cleared to 0
					*/
    ) ;

/* Summary
   Allocates the BitSet object

   Description
   Returns a <LINK NlmCmBitSet Module> object of bit size specified.
 
   Returns 
   Pointer on success, NULL on error.
   
   Parameters
   @param size_in_bits The NlmCmBitSet size
   @param alloc NlmCmAllocator object pointer
 
   See Also
   NlmCmBitSet__destroy
 */
extern NlmCmBitSet* 
NlmCmBitSet__create(
    size_t		size_in_bits ,
    NlmCmAllocator	*alloc
    ) ;


/* Summary
   Cleans up the BitSet object.
 
   Description
   If the bitset was constructed with an allocator, then the
   storage (m_bit_p) will be freed when the dtor is called.
 
   Returns
   @return void 

   Parameters
   @param self NlmCmBitSet pointer

   See Also
   NlmCmBitSet__destroy
 */
extern void NlmCmBitSet__dtor(
    NlmCmBitSet		*self
    );

/* Summary
   Deallocates the memory of an BitSet object

   Returns
   @return void 

   Parameters
   @param self NlmCmBitSet pointer

   See Also
   NlmCmBitSet__dtor
 */
extern void NlmCmBitSet__destroy(
    NlmCmBitSet		*self
    );


/* Summary
   Copies a BitSet value to another BitSet object.

   Description
   Will only work if both bitsets are of the same size. If the bitsets
   differ in size, use Clone.

   Returns
   @return void 

   Parameters
   @param self NlmCmBitSet pointer
   @param src  Source NlmCmBitSet pointer 

   See Also
   NlmCmBitSet__Clone
 */
extern void NlmCmBitSet__Copy(
    NlmCmBitSet		*self,
    const NlmCmBitSet	*src
    );


/* Summary
   Copies bits from a nlm_u8 pointer to a BitSet object
  
   Description
   Will only work if both the bitset is size_in_bits wide.

   Returns
   @return void

   Parameters
   @param self NlmCmBitSet pointer
   @param bits bits to copy from
   @param size_in_bits length in bits
 */
extern void
NlmCmBitSet__CopyBits(
    NlmCmBitSet *self,
    const nlm_u8* bits,
    size_t size_in_bits
    ) ;

/* Summary
   Copy the bitsets of differing sizes.

   Description
   Clone will work exactly like a Copy if both bitsets are the same size.
   If the sizes differ AND self was constructed with an allocator, then
   self will free its current storage, and malloc new storage of the
   correct size, then perform a copy.

   You MUST call the dtor on the self bitset to free this storage when
   you no longer need it. If you do not a memory leak will occur.
  
   Returns
   @return NlmErrNum_t 

   Parameters
   @param self NlmCmBitSet pointer
   @param src  Source NlmCmBitSet pointer 
   
   See Also
   NlmCmBitSet__Copy
 */
extern NlmErrNum_t NlmCmBitSet__Clone(
    NlmCmBitSet		*self,
    const NlmCmBitSet	*src
    );

/* Summary
   Fills the NlmCmBitSet value with the specified uint8 value.

   Returns
   @return void

   Parameter
   @param self NlmCmBitSet pointer

   See Also
   NlmCmBitSet__Zero
   NlmCmBitSet__CopyZero
 */
extern void NlmCmBitSet__Fill(
    NlmCmBitSet		*self,
    nlm_u8		value
    ) ;

/* Summary
   Makes the NlmCmBitSet value to zeros.

   Returns
   @return void

   Parameter
   @param self NlmCmBitSet pointer

   See Also
   NlmCmBitSet__Fill
   NlmCmBitSet__CopyZero
 */
extern void NlmCmBitSet__Zero(
    NlmCmBitSet		*self
    ) ;

/* Summary
   Copy from source, truncating the source, or zero filling the target.

   Returns
   @return void

   Parameters  
   @param self NlmCmBitSet pointer
   @param src NlmCmBitSet pointer
  
   See Also
   NlmCmBitSet__Fill
   NlmCmBitSet__Zero
 */
extern void NlmCmBitSet__CopyZero(
    NlmCmBitSet		*self,
    const NlmCmBitSet	*src
    );

/* Summary
   Sets a bit value in NlmCmBitSet. The bitpos value is set to 1.

   Returns
   @return void

   Parameters
   @param self NlmCmBitSet pointer
   @param bitpos bit position
  
   See Also
   NlmCmBitSet__Clr
 */
extern void NlmCmBitSet__Set(
    NlmCmBitSet		*self,
    size_t		bitpos
    );

/* Summary
   Clears a bit value in NlmCmBitSet. The bitpos value is set to 0.

   Returns
   @return void

   Parameters
   @param self NlmCmBitSet pointer
   @param bitpos bit position
  
   See Also
   NlmCmBitSet__Clr
 */
extern void NlmCmBitSet__Clr(
    NlmCmBitSet		*self,
    size_t		bitpos
    );


/* Summary
   Sets a bit value to specified value. The bitpos value is set to value (0/1).

   Returns
   @return void

   Parameters
   @param self NlmCmBitSet pointer
   @param bitpos bit position
   @param bit bit value
  
   See Also
   NlmCmBitSet__Get
 */
extern void NlmCmBitSet__Jam(
    NlmCmBitSet		*self,
    size_t		bitpos,
    NlmBit		bit
    );

/* Summary
   The value of the bit specified is returned.

   Returns
   @return NlmBit

   Parameters
   @param self NlmCmBitSet pointer
   @param bitpos bit position
  
   See Also
   NlmCmBitSet__GetField
 */
extern NlmBit NlmCmBitSet__Get(
    const NlmCmBitSet	*self,
    size_t		bitpos
    );

/* Formats the contents of a BitSet into a buffer in a human readable form.

   Returns
   @return void

   See Also
   NlmCmBitSet__Print
 */
extern NlmBool
NlmCmBitSet__Format(
    const NlmCmBitSet	*self,	/* NlmCmBitSet pointer */
    NlmBool reverse,		/* if TRUE, reverse the order of the bits */
    char *buffer,		/* buffer to store characters */
    nlm_u32 size		/* number of characters in the buffer */
    ) ;

/* Summary
   Displays the value of the BitSet to the file or screen.

   Returns
   @return void
 
   Parameters
   @param self NlmCmBitSet pointer
   @param fp file pointer
  
   See Also
   NlmCmBitSet__GetField
 */
extern void NlmCmBitSet__Print(
    const NlmCmBitSet	*self,
    NlmCmFile		*fp
    );
/* repair original improper naming */
#define NlmCmBitSet__print NlmCmBitSet__Print

/* Summary
   Displays the value of the BitSet to the file or screen.

   Returns
   @return void

   Parameters
   @param bs NlmCmBitSet pointer
   @param fp file pointer

   See Also
   NlmCmBitSet__GetField
 */
extern void NlmCmBitSet__PrintAsBits(
    const NlmCmBitSet	*bs,
    NlmCmFile		*fp
    );
/* repair improper naming */
#define NlmCmBitSet__printAsBits NlmCmBitSet__PrintAsBits

/* Summary
   Shift the bits in the bitset to the right to the next byte boundary; 
   zero fill vacated positions;
   results go into the field pointed to by result.

   Returns
   size in bits of results

   Useful for printing results in bitsets with MSB on the LHS.
*/
extern size_t NlmCmBitSet__ShiftTo8(
    const NlmCmBitSet	*bs ,
    nlm_u8		*result ,
    size_t		result_size_in_bytes ) ;

/* Summary
   Finds the Largest Contiguous Zero Set

   Returns
   @return void

   Parameters
   @param self NlmCmBitSet pointer
   @param o_pos_p Pointer to the position value
   @param o_size_p Pointer to the size value

   Note:
   Keep the order consistent. It's always (position, size).

   See Also
   NlmCmBitSet__FindNextContiguousZeroSet
 */
extern void NlmCmBitSet__FindLargestContiguousZeroSet(
    const NlmCmBitSet *	self,
    size_t *		o_pos_p,	/* 0 => nothing found */
    size_t *		o_size_p	/* 0 => nothing found */
    ) ;

/* Summary
   Finds the next Largest Contiguous Zero Set

   Returns
   @return void

   Parameters
   @param self NlmCmBitSet pointer
   @param start size
   @param min_size minimum size
   @param o_pos_p Pointer to the position value
   @param o_size_p Pointer to the size value
  
   See Also
   NlmCmBitSet__FindContiguousZeroSet
 */
extern void NlmCmBitSet__FindNextContiguousZeroSet(
    const NlmCmBitSet *	self,
    size_t 		start,
    size_t 		min_size,
    size_t *		o_pos_p,	/* 0 => nothing found */
    size_t *		o_size_p	/* 0 => nothing found */
    ) ;


/* Summary
   Hash a NlmCmBitSet

   Description
   Generate a hash value from a NlmCmBitSet object.
  
   Returns
   @return hash value
 
   Parameters
   @param self NlmCmBitSet pointer
  
   See Also
   NlmCmBitSet__HashBitSet
 */
extern nlm_u32 NlmCmBitSet__Hash(
    const NlmCmBitSet*	self
    ) ;


/* Summary
   Hash a set of bits.

   Returns
   @return hash value

   Parameters
   @param bits pointer to byte array
   @param size_in_bits size in bites of byte array
  
   See Also
   NlmCmBitSet__Hash
 */
extern nlm_u32 NlmCmBitSet__HashBitSet(
    const nlm_u8*	bits,
    size_t		size_in_bits
    ) ;

/* Summary
   compute a 32 bit CRC on the input
 */
extern nlm_u32
NlmCmBitSet__CRC32(
    nlm_u32		crc,		/* starting CRC value */
    const nlm_u8	*bp,		/* pointer to input */
    size_t		len		/* length of input in bytes */
    );

/* Summary
   Faster hash of a NlmCmBitSet

   Description
   Generate a hash value from a NlmCmBitSet object.
  
   Returns
   @return hash value
 
   Parameters
   @param self NlmCmBitSet pointer
  
   See Also
   NlmCmBitSet__FastHashBitSet
 */
extern nlm_u32 NlmCmBitSet__FastHash(
    const NlmCmBitSet*	self
    ) ;


/* Summary
   Faster hash a set of bits.

   Returns
   @return hash value

   Parameters
   @param bits pointer to byte array
   @param size_in_bits size in bites of byte array
  
   See Also
   NlmCmBitSet__FastHash
 */
extern nlm_u32 NlmCmBitSet__FastHashBitSet(
    const nlm_u8*	bits,
    size_t		size_in_bits
    ) ;

/* Summary
   Returns NlmTrue if the bit set is all zero.

   Description
   Examines the bits in the bit set and returns NlmTrue
   if all of the bits are zero.

   Returns
   NlmTrue if all bits are zero; NlmFalse if any bits are one.

   Parameters
   @param self NlmCmBitSet pointer
 */
extern NlmBool NlmCmBitSet__IsZero(
    const NlmCmBitSet *self
    ) ;

/* Summary
   Returns NlmTrue if the bit set is all one.

   Description
   Examines the bits in the bit set and returns NlmTrue
   if all of the bits are one.

   Returns
   NlmTrue if all bits are one; NlmFalse if any bits are zero

   Parameters
   @param self NlmCmBitSet pointer
 */
extern NlmBool NlmCmBitSet__IsOnes(
    const NlmCmBitSet *self
    ) ;

/* Summary
   Compare two NlmCmBit object values. 

   Description
   Compares two NlmCmBit objects and returns the diff
   value.

   Returns
   0 if equal; <0 if self < that; >0 if self > that.

   Parameters
   @param self NlmCmBitSet pointer
   @param that NlmCmBitSet pointer to be compared with

   See Also
   NlmCmBitSet__Operate
 */
extern int  NlmCmBitSet__Compare(
    const NlmCmBitSet	*self, 
    const NlmCmBitSet	*that
    ) ;	


/* Summary
   Converts a right aligned hex string to a left aligned byte array

   Parameters
   @param alloc		NlmCmAllocator to use for internal buffers
   @param str		const char* hex string to convert.  Must be
			of the format 0xFF and null terminated.
   @param bytearr	nlm_u8* byte array to save into
   
   Return
   NlmTrue on success, NlmFalse on error

*/
extern NlmBool
NlmCmBitSet__RHexStrToLByteArr(
    NlmCmAllocator* alloc,	/* allocator for buffer */
    const char* str,		/* Null terminated */
    nlm_u8*	bytearr,	/* Where to write */
    nlm_u32	bytearrlen,	/* Length of byte array (in bytes) */
    nlm_u32	lengthInBits);	/* Number of bits to use */

/* Summary
   Perform logical operations between two NlmCmBitSet values.

   Description
   Performs one of the logical operations &(and) |(or) ^(xor) ~(not) 
   specified.
  
   Returns
   @return void

   Parameters
   @param self NlmCmBitSet pointer
   @param xyz  NlmCmBitSet pointer to be second operand
   @param op   Logical operator
  
   See Also
   NlmCmBitSet__Compare
 */
extern void NlmCmBitSet__Operate(
    NlmCmBitSet		*self,
    const NlmCmBitSet	*xyz,
    char op			    /* op:  &(and) |(or) ^(xor) ~(not) */
    ) ;

/* Summary
   Set bitset from signed integer value.

   Description
   Bits in a bitset are keep in the order {0 1 2 .. 31 ...}.
   But sometimes we want to treat them like an integer,
   with bit 0 being the least significant bit.
   This is a utility routine to convert representations.
  
   NOTE:
   Bit sets are treating as UNSIGNED integers.
   This routine truncates or 0 fills, as needed.

   Returns
   @return void

   Parameters
   @param self NlmCmBitSet pointer
   @param val  Integer value 
 
   See Also
   NlmCmBitSet__GetAsInt
   NlmCmBitSet__GetAsUInt
   NlmCmBitSet__SetAsInt
   NlmCmBitSet__SetAsUInt
 */   
extern void NlmCmBitSet__SetAsInt(
    NlmCmBitSet		*self,
    nlm_32		val
    ) ;

/* Summary
   Set bitset from unsigned integer value.

   Description
   Bits in a bitset are keep in the order {0 1 2 .. 31 ...}.
   But sometimes we want to treat them like an integer,
   with bit 0 being the least significant bit.
   This is a utility routine to convert representations.

   NOTE:
   Bit sets are treating as UNSIGNED integers. This routine
   truncates or 0 fills, as needed.

   Returns
   @return void

   Parameters
   @param self NlmCmBitSet pointer
   @param val  Integer value 
  
   See Also
   NlmCmBitSet__GetAsInt
   NlmCmBitSet__GetAsUInt
   NlmCmBitSet__SetAsInt
   NlmCmBitSet__SetAsUInt
 */   
extern void NlmCmBitSet__SetAsUInt(
    NlmCmBitSet		*self,
    nlm_u32		val
    ) ;

/* Summary
   Get the NlmCmBitSet value as a signed integer.

   Description
   Bits in a bitset are keep in the order {0 1 2 .. 31 ...}.
   But sometimes we want to treat them like an integer,
   with bit 0 being the least significant bit.
   This is a utility routine to convert representations.
  
   NOTE:
   Bit sets are treating as UNSIGNED integers. This routine
   truncates or 0 fills, as needed.

   Returns
   @return int NlmCmBitSet value

   Parameters
   @param self NlmCmBitSet pointer

   See Also
   NlmCmBitSet__GetAsInt
   NlmCmBitSet__GetAsUInt
   NlmCmBitSet__SetAsInt
   NlmCmBitSet__SetAsUInt
 */   
extern nlm_32 NlmCmBitSet__GetAsInt(
    const NlmCmBitSet	*self
    ) ;

/* Summary
   Get the NlmCmBitSet value as an unsigned integer.

   Description
   Bits in a bitset are keep in the order {0 1 2 .. 31 ...}.
   But sometimes we want to treat them like an integer,
   with bit 0 being the least significant bit.
   This is a utility routine to convert representations.
  
   NOTE:
   Bit sets are treating as UNSIGNED integers. This routine
   truncates or 0 fills, as needed.

   Returns
   @return int NlmCmBitSet value

   Parameters
   @param self NlmCmBitSet pointer

   See Also
   NlmCmBitSet__GetAsInt
   NlmCmBitSet__GetAsUInt
   NlmCmBitSet__SetAsInt
   NlmCmBitSet__SetAsUInt
 */   
extern nlm_u32 NlmCmBitSet__GetAsUInt(
    const NlmCmBitSet	*self
    ) ;

/* Summary
   Make a field for a NlmCmBitSet object.

   Description
   Creates the scalar value used to define a field.
   Fields in NlmCmBitSet go from low to high bit positions.

   Returns
   @return the field representation

   See Also
   NlmCmBitSet__SetField
 */
extern nlm_u32 NlmCmBitSet__MakeField(
    int head,				/* first bit position */
    int tail				/* last bit position +1 */
    ) ;

/* Summary
   Get a set of bits from the NlmCmBitSet object.

   Description
   Copies a subset of bits from one NlmCmBitSet object
   to another. The start and end position parameters are
   each of nlm_u32 type, and field specification of longer length
   than 16 bits is possible.

   Returns
   @return void

   Parameters
   @param self NlmCmBitSet pointer
   @param data destination NlmCmBitSet pointer 
   @param start_pos will represent the start position 
   @param end_pos will represent the end position 
                
   See Also
   NlmCmBitSet__SetField
 */   
extern void NlmCmBitSet__GetLongField(
    const NlmCmBitSet	*self,
    NlmCmBitSet		*data,
    size_t		start_pos,
    size_t		end_pos
    );

/* Summary
   Get a field from the NlmCmBitSet object.

   Description
   Gets (that is, extracts) a field from one CmCmBitSet
   to overwrite the entire contents of another NlmCmBitSet.

   Returns
   @return void

   Parameters
   @param self NlmCmBitSet pointer
   @param data destination NlmCmBitSet pointer 
   @param field The field should be a single "short-packed" int. 
                Bits 0-31 will represent the start position and 
                Bits 16-31 will represent 1 position past the end 
                position.
  
   See Also
   NlmCmBitSet__SetField
 */   
extern void NlmCmBitSet__GetField(
    const NlmCmBitSet	*self,
    NlmCmBitSet		*data,
    size_t		field
    );


/* Summary
   Set a field in the NlmCmBitSet object.

   Description
   Sets (that is, replaces) a field in one CmCmBitSet
   with the contents of another NlmCmBitSet,
   starting with the least significant bits.
   Excess bits in the source are ignored.

   Returns
   @return void

   Parameters
   @param self destination NlmCmBitSet pointer
   @param data source NlmCmBitSet pointer 
   @param field The field should be a single "short-packed" int. 
                Bits 0-15 will represent the start position and 
                Bits 16-31 will represent 1 position past the end 
                position.
  
   See Also
   NlmCmBitSet__SetFieldMask
 */   
extern void NlmCmBitSet__SetField(
    NlmCmBitSet		*self,
    const NlmCmBitSet	*data,
    size_t		field
    );


/* Summary
   Set a field in a bitset to a desired value.

   Description
   Copy the lower order bits from the reference bitset into the specified
   field within the bitset object. This method uses separate parameters to
   specify the starting and ending positions, so that larger bitsets can be
   handled.

   Returns
   @return void

   Parameters
   @param self      NlmCmBitSet pointer
   @param data      Source NlmCmBitSet pointer
   @param start_pos Starting bit position of the field
   @param ending    Ending bit position (+1) of the field

   See Also
   NlmCmBitSet__GetLongField
   NlmCmBitSet__SetField
 */
extern void
NlmCmBitSet__SetLongField(
    NlmCmBitSet		*self,
    const NlmCmBitSet	*data,
    size_t		start_pos,
    size_t		end_pos
    );

/* Summary
   Set a field in a bitset to a desired value with a mask

   Description
   Copy the lower order bits from the reference bitset into the specified
   field within the bitset object. This method uses separate parameters to
   specify the starting and ending positions, so that larger bitsets can be
   handled.

   Returns
   @return void

   Parameters
   @param self      NlmCmBitSet pointer
   @param data      Source NlmCmBitSet pointer
   @param mask	    Mask for the set operation
   @param start_pos Starting bit position of the field
   @param ending    Ending bit position (+1) of the field

   See Also
   NlmCmBitSet__GetLongField
   NlmCmBitSet__SetField
 */
extern void
NlmCmBitSet__SetLongFieldMask(
    NlmCmBitSet		*self,
    const NlmCmBitSet	*data,
    const NlmCmBitSet    *mask,
    size_t		start_pos,
    size_t		end_pos
    );

/* Summary
   Set a field in the NlmCmBitSet object using a mask.

   Description
   Copies a subset of bits from one CmCmBitSet object
   to another using a mask. Only masked bits will be 
   updated.

   Returns
   @return void

   Parameters
   @param self destination NlmCmBitSet pointer
   @param mask mask NlmCmBitSet pointer
   @param data source NlmCmBitSet pointer 
   @param field The field should be a single "short-packed" int. 
                Bits 0-15 will represent the start position and 
                Bits 16-31 will represent 1 position past the end 
                position.
  
   See Also
   NlmCmBitSet__SetField
 */   
extern void NlmCmBitSet__SetFieldMask(
    NlmCmBitSet		*self,
    const NlmCmBitSet	*mask,
    const NlmCmBitSet	*data,
    size_t		field
    );

/* Summary
   Set a field in the NlmCmBitSet object with the value passed in
   as an integer.

   Description
   Sets a field in the NlmCmBitSet object.  The field is set based
   on the field coordiates and the value given in val.

   Returns
   @return void

   Parameters
   @param self destination NlmCmBitSet pointer
   @param val data value for the field
   @param field The field should be a single "short-packed" int. 
                Bits 0-15 will represent the start position and 
                Bits 16-31 will represent 1 position past the end 
                position.
  
   See Also
   NlmCmBitSet__SetField
   NlmCmBitSet__GetFieldAsInt
 */   

extern void NlmCmBitSet__SetFieldAsInt(
    NlmCmBitSet		*self,
    nlm_u32		val,
    size_t		field
    );

/* Summary
   Get a field in the NlmCmBitSet object from a range of bits

   Description
   Gets a field in the NlmCmBitSet object.  The range of bits is
   encoded in the field parameter

   Returns
   @return nlm_u32

   Parameters
   @param self destination NlmCmBitSet pointer
   @param val data value for the field
   @param field The field should be a single "short-packed" int. 
                Bits 0-15 will represent the start position and 
                Bits 16-31 will represent 1 position past the end 
                position.
  
   See Also
   NlmCmBitSet__GetField
   NlmCmBitSet__SetFieldAsInt
 */   
extern nlm_u32 NlmCmBitSet__GetFieldAsInt(
    const NlmCmBitSet	*self,
    size_t		field
    );

/* Summary
   Reverse the bits in a NlmCmBitSet

   Description
   Reverse the bits in a bitset. Only works for bitsets with a size
   that is a multiple of 8.
*/
extern void NlmCmBitSet__Reverse(
    NlmCmBitSet		*self);

/* Summary
   Reverse the bits in a byte array

   Description
   Reverse the bits in a byte array.
*/
extern void NlmCmBitSet__ReverseBytes(
    size_t		size_in_bytes,
    nlm_u8*		bits ) ;

#ifndef __doxygen__
extern NlmErrNum_t NlmCmBitSet__Verify(void);
#endif /*## __doxygen__ */

#include <nlmcmexterncend.h>

#endif

/* [] */
