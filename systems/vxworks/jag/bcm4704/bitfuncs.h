/*
    EXTERNAL SOURCE RELEASE on 12/03/2001 3.0 - Subject to change without notice.

*/
/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * bit manipulation utility functions
 *
 * Copyright 2000 Broadcom, Inc.
 * $Id: bitfuncs.h,v 1.1 2004/02/24 07:47:00 csm Exp $
 */

#ifndef _BITFUNCS_H
#define _BITFUNCS_H

#include <hnbutypedefs.h>

/* local prototypes */
static INLINE uint32 find_msbit(uint32 x);


/*
 * find_msbit: returns index of most significant set bit in x, with index
 *   range defined as 0-31.  NOTE: returns zero if input is zero.
 */

#if defined(USE_PENTIUM_BSR) && defined(__GNUC__)

/*
 * Implementation for Pentium processors and gcc.  Note that this
 * instruction is actually very slow on some processors (e.g., family 5,
 * model 2, stepping 12, "Pentium 75 - 200"), so we use the generic
 * implementation instead.
 */
static INLINE uint32 find_msbit(uint32 x)
{
	uint msbit;
        __asm__("bsrl %1,%0"
                :"=r" (msbit)
                :"r" (x));
        return msbit;
}

#else

/*
 * Generic Implementation
 */

#define DB_POW_MASK16	0xffff0000
#define DB_POW_MASK8	0x0000ff00
#define DB_POW_MASK4	0x000000f0
#define DB_POW_MASK2	0x0000000c
#define DB_POW_MASK1	0x00000002

static INLINE uint32 find_msbit(uint32 x)
{
	uint32 temp_x = x;
	uint msbit = 0;
	if (temp_x & DB_POW_MASK16) {
		temp_x >>= 16;
		msbit = 16;
	}
	if (temp_x & DB_POW_MASK8) {
		temp_x >>= 8;
		msbit += 8;
	}
	if (temp_x & DB_POW_MASK4) {
		temp_x >>= 4;
		msbit += 4;
	}
	if (temp_x & DB_POW_MASK2) {
		temp_x >>= 2;
		msbit += 2;
	}
	if (temp_x & DB_POW_MASK1) {
		msbit += 1;
	}
	return(msbit);
}

#endif

#endif /* _BITFUNCS_H */
