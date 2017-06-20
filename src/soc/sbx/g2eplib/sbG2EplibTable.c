/**
 *
 * ====================================================================
 * ==  sbG2EplibTable.c - Logical Access to tables in IP memory
 * ====================================================================
 *
 * WORKING REVISION: $Id: sbG2EplibTable.c,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MODULE NAME:
 *
 *     sbG2EplibTable.c
 *
 * ABSTRACT:
 *
 *     Logical Access to tables in IP memory
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     Josh Weissman
 *     Travis B. Sawyer
 *
 * CREATION DATE:
 *
 *     30-March-2005
 *
 */

#include <shared/bsl.h>

#include "sbQe2000ElibMem.h"
#include "sbG2EplibTable.h"
#include "sbWrappers.h"


#if !defined(SAND_BIG_ENDIAN_HOST)
#define BSWAP32(__X) (__X)
#else
#define BSWAP32(__X) SAND_SWAP_32(__X)
#endif



/**--------------------------------------------------------------------------
 * Module-Scoped User Defined Types
 * --------------------------------------------------------------------------*/

/* SMAC tables descriptors */
typedef struct smac_table_s {
    uint32 ulBase;            /**< Start of Table In IP memory */
    uint32 ulEnd;             /**< End of Table In IP memory */
} sbG2EplibSMACTable_t;

/* IP segment descriptors */
typedef struct seg_desc_s {
    uint32 ulBase;            /**< Start of Table In IP memory */
    uint32 ulEnd;             /**< End of Table In IP memory */
    uint32 ulWidth;           /**< Log2n() size of Entry in Bits */
    uint32 ulEntrySize;       /**< Size of Entry in Bits */
} sbG2EplibSegDesc_t;

/**--------------------------------------------------------------------------
 * Forward Declarations of Module-Scoped Functions
 * --------------------------------------------------------------------------*/
static sbElibStatus_et
sbG2EplibIpTableWriteMultiple(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et eSeg,
			     uint32 nIdx, uint32 nData[]);
static sbElibStatus_et
sbG2EplibIpTableReadMultiple(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et eSeg,
			    uint32 nIdx, uint32 nData[]);

/**--------------------------------------------------------------------------
 *
 * IP Memory Segments
 *
 * Note: IP memory segment setup requires the memory width to be
 *       a power of two less than or equal to 64. The following
 *       are valid data widths.
 *
 *  Width = (1, 2, 4, 8, 16, 32, 64, 64)
 *  Log2n = (0, 1, 2, 3,  4,  5,  6, 7)
 *
 * ATTENTION:  The table below is the default table and can be modified
 * ATTENTION:  by the USER before INITIALIZATION.
 *
 *
 * Additional Notes:
 *
 *  - The physical memory width on the chip is 64 bits
 *  - The width table above is correct - The chip handles 6 and 7 the same (64-bits)
 *  - The IPv4 Encap table has 160b of payload that are kept in a 192b entry that spans
 *    three consecutive 64-bit words in memory
 *  - Segments MUST start on 8 word boundaries (IE: Bottom three bits MUST BE ZERO)
 *
 *                                                     64-bit
 * Seg | Start  | End    | Dat-Sz | Ent-Sz | Entries | Words | Name
 * ----+--------+--------+--------+--------+---------+-------+-------------------------
 * HW  | 0x0000 | 0x003f |  48b   |   64b  |    64   |   64  | VRID == 0, Per Port SMAC
 * HW  | 0x0040 | 0x0147 |  48b   |   64b  |    64   |   64  | VRID != 0, VRID SMAC
 * ----+--------+--------+--------+--------+---------+-------+-------------------------
 * 0   | 0x0148 | 0x02D7 |   8b   |    8b  |  3200   |  400  | Tos Remap
 * 1   | 0x02D8 | 0x039F |   3b   |    4b  |  3200   |  200  | Exp Remap
 * 2   | 0x03A0 | 0x069F | 160b   |  192b  |   256   |  768  | IPv4 Encap
 * 3   | 0x06A0 | 0x0767 |   3b   |    4b  |  3200   |  200  | Pri Remap
 * 4   | 0x0768 | 0x0777 |  64b   |   64b  |    16   |   16  | Counters
 * 5   | 0x0778 | 0x07F7 |   4b   |    4b  |  2048   |  128  | VPLS Knockout Color
 * 6   | 0x07F8 | 0x0bF7 |  64b   |   64b  |  1024   | 1024  | MPLS Label Record
 * 7   | 0x0bf8 | 0x25f7 |  64b   |   64b  |  6656   | 6656  | Tci DMAC
 * 8   | 0x25f8 | 0x3FF8 |  64b   |   64b  |  6656   | 6656  | Tci DMAC Counters
 *
 * --------------------------------------------------------------------------*/

static sbZfG2EplibIpSegment_t s_IpSegmentsDefault[SEG_SEGMENTS] = {
    /* start  end   width EntSz */
    {0x0,   0x7ffe,  0x4, 0x10},    /**< H-QoS Remap */
    {0x0,   0x3ff,   0x4, 0x10},    /**< VLAN Remap */
    {0x400, 0x40f,   0x4, 0x10}     /**< Port  Encap */
};

/**--------------------------------------------------------------------------
 * These are utility functions to access the descriptor tables. These provide
 * access to the table and impose a read-only interface to the module-scoped
 * access functions and descriptor data.
 * --------------------------------------------------------------------------*/
uint32
sbG2EplibGetSegBase(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et segment) {
    SB_ASSERT(pCtxt);
    return pCtxt->tIpSegment[segment].start;
}

uint32
sbG2EplibGetSegWidth(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et segment) {
    SB_ASSERT(pCtxt);
    return pCtxt->tIpSegment[segment].width;
}

uint32
sbG2EplibGetSegEntrySize(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et segment) {
    SB_ASSERT(pCtxt);
    return pCtxt->tIpSegment[segment].entrysize;
}

uint32
sbG2EplibGetSegEnd(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et segment) {
    SB_ASSERT(pCtxt);
    return pCtxt->tIpSegment[segment].end;
}

uint32
sbG2EplibGetSegBaseDefault(sbG2EplibIpSegs_et segment) {
    return s_IpSegmentsDefault[segment].start;
}

uint32
sbG2EplibGetSegWidthDefault(sbG2EplibIpSegs_et segment) {
    return s_IpSegmentsDefault[segment].width;
}
uint32
sbG2EplibGetSegEntrySizeDefault(sbG2EplibIpSegs_et segment) {
  return s_IpSegmentsDefault[segment].entrysize;
}

uint32
sbG2EplibGetSegEndDefault(sbG2EplibIpSegs_et segment) {
    return s_IpSegmentsDefault[segment].end;
}

/**--------------------------------------------------------------------------
 * sbG2EplibIpTableWrite()
 *
 *  Provide logical access to the different tables within the QE2k's IP
 *  memory. All table entries are accessed with a given IP segment that
 *  denotes the table, and an index within that table.
 *
 *  pCtxt   - Pointer to the Egress context structure
 *  eSeg    - Enumerated Type describing which table to access
 *  nIdx    - Index of the entry within that table
 *  nData   - Array (Pointer) to Packed word data
 *  returns - error code (zero on success)
 * --------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibIpTableWrite(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et eSeg,
		     uint32 nIdx, uint32 nData[])
{
    sbElibStatus_et rVal;

    rVal = sbG2EplibIpTableWriteMultiple(pCtxt, eSeg, nIdx, nData);

    return rVal;
}

/**--------------------------------------------------------------------------
 * sbG2EplibIpTableRead()
 *
 *  Provide logical access to the different tables within the QE2k's IP
 *  memory. All table entries are accessed with a given IP segment that
 *  denotes the table, and an index within that table.
 *
 *  pCtxt   - Pointer to the Egress context structure
 *  eSeg    - Enumerated Type describing which table to access
 *  nIdx    - Index of the entry within that table
 *  nData   - Array (Pointer) to Packed Word data
 *  returns - error code (zero on success)
 * --------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibIpTableRead(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et eSeg,
		    uint32 nIdx, uint32 nData[])
{
    sbElibStatus_et rVal;

    rVal = sbG2EplibIpTableReadMultiple(pCtxt, eSeg, nIdx, nData);

    return rVal;
}

/**--------------------------------------------------------------------------
 * sbG2EplibIpTableWriteMultiple()
 *
 *  This module-scoped function is used to write entries equal or greater
 *  than one-word in size. (Entry Size in bits = {64, 128}). This function
 *  deals with possibly Writing Multiple words in memory. This should never
 *  be called by the user, and should only provide utility for the public
 *  sbG2EplibIpTableWrite() function.
 *
 *  pCtxt   - Pointer to the Egress context structure
 *  eTble   - Enumerated Type selecting table
 *  nIdx    - Index of entry within that table
 *  nData   - Array (pointer) of packed data to be written
 *  returns - error code (zero on success)
 * --------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibIpTableWriteMultiple(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et eSeg,
			     uint32 nIdx, uint32 nData[])
{
    uint32 nAddr, nBase, nEnd, nStride, nESize, nOff;
    sbElibStatus_et nStatus;

    /* get segment descriptor information */
    nBase  = sbG2EplibGetSegBase(pCtxt, eSeg);
    nEnd   = sbG2EplibGetSegEnd(pCtxt, eSeg);
    nESize = sbG2EplibGetSegEntrySize(pCtxt, eSeg);

    /* determine number of rows */
    nStride = (nESize / 64);

    /* determine physical start address */
    nAddr = nBase + nESize * nIdx / 64;

    /* if the new address is past the end of segment, throw error */
    if (nAddr > nEnd) {
	LOG_INFO(BSL_LS_SOC_COMMON,
                 (BSL_META("address out of range (0x%08x -> 0x%08x): 0x%08x\n"),
                  nBase, nEnd, nAddr));
	return SB_ELIB_BAD_ARGS;
    }

    /* go through and write out the words */
    for (nOff = 0; nOff <= nStride; nOff++) {
	nStatus = sbQe2000ElibIpMemWrite(pCtxt->pHalCtx, nAddr + nOff,
					 nData[nOff * 2 + 1],
					 nData[nOff * 2 + 0]);
	if (nStatus)
	    return nStatus;
    }

    return SB_ELIB_OK;
}

/**--------------------------------------------------------------------------
 * sbG2EplibIpTableReadMultiple()
 *
 *  This module-scoped function is used to read entries equal or greater
 *  than one-word in size. (Entry Size in bits = {64, 128}). This function
 *  deals with possibly Reading Multiple words in memory. This should never
 *  be called by the user, and should only provide utility for the public
 *  sbG2EplibIpTableWrite() function.
 *
 *  pCtxt   - Pointer to the Egress context structure
 *  eTble   - Enumerated Type selecting table
 *  nIdx    - Index of entry within that table
 *  nData   - Array (pointer) of packed data to be written
 *  returns - error code (zero on success)
 * --------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibIpTableReadMultiple(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et eSeg,
			    uint32 nIdx, uint32 nData[])
{
    uint32 nAddr, nBase, nEnd, nStride, nESize, nOff, nRegData[4];
    sbElibStatus_et nStatus;


    /* get segment descriptor information */
    nBase  = sbG2EplibGetSegBase(pCtxt, eSeg);
    nEnd   = sbG2EplibGetSegEnd(pCtxt, eSeg);
    nESize = sbG2EplibGetSegEntrySize(pCtxt, eSeg);

    /* determine number of rows */
    nStride = (nESize / 64);

    /* determine physical start address */
    nAddr = nBase + nESize * nIdx / 64;

    /* if the new address is past the end of segment, throw error */
    if (nAddr > nEnd) {
	LOG_INFO(BSL_LS_SOC_COMMON,
                 (BSL_META("address out of range (0x%08x -> 0x%08x): 0x%08x\n"),
                  nBase, nEnd, nAddr));
	return SB_ELIB_BAD_ARGS;
    }

    /* go through and read out the words */
    for (nOff = 0; nOff <= nStride; nOff++) {
	nStatus = sbQe2000ElibIpMemRead(pCtxt->pHalCtx, nAddr + nOff, 0,
					&nRegData[nOff * 2 + 1],
					&nRegData[nOff * 2 + 0]);
	if (nStatus)
	    return nStatus;
    }

    /* export appropriate amount of the data */
    if (nESize == 128) {
	nData[3] = nRegData[3];
	nData[2] = nRegData[2];
    }
    nData[1] = nRegData[1];
    nData[0] = nRegData[0];

    return SB_ELIB_OK;
}


