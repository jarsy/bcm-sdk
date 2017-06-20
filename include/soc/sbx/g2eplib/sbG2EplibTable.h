#ifndef __GU_ELIBTABLE_H__
#define __GU_ELIBTABLE_H__
/**
 * @file sbG2EplibTable.h  Logical Table Access Functions
 *
 * <pre>
 * ===================================================
 * ==  sbG2EplibTable.h - Logical Table Access Functions ==
 * ===================================================
 *
 * WORKING REVISION: $Id: sbG2EplibTable.h,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MODULE NAME:
 *
 *     sbG2EplibTable.h
 *
 * ABSTRACT:
 *
 *      Logical Table Access Functions
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     Josh Weissman
 *
 * CREATION DATE:
 *
 *     30-March-2005
 * </pre>
 */

#include "sbQe2000Elib.h"
#include "sbElibStatus.h"
#include "sbG2Eplib.h"

/**
 * Enumerated Type of different counters within the IP Counter
 * segment memory.
 */
typedef enum sbG2EplibIpCntIdx_es {
    SEG_CNT_TOPROC = 0,
    SEG_CNT_FROMPROC = 1,
    SEG_CNT_DROPCLASS = 2,
    /* leave as last */
    SEG_CNT_COUNTS
} sbG2EplibIpCntIdx_et;

/**
 *
 * Enumerated Type of the different SMAC tables that are at the bottom of
 * IP memory before the first IP segment. These are all physically in the
 * same memory portion but we still maintain the distinction of between them.
 *
 **/
typedef enum sbG2EplibSMACTbls_et {
    SMAC_TBL_PORT = 0,   /**< Port-Based SMAC Table */
    SMAC_TBL_VRID,       /**< Vrid-Based SMAC Table */
    /* leave as last */
    SMAC_TBL_TABLES
} sbG2EplibSMACTbls_et;

/**
 *
 *  Provide logical write access to the different tables within the QE2k's IP
 *  memory. All table entries are accessed with a given IP segment that
 *  denotes the table, and an index within that table.
 *
 *  @param  pCtxt   - Pointer to the Egress context structure
 *  @param  eSeg    - Enumerated Type describing which table to access
 *  @param  nIdx    - Index of the entry within that table
 *  @param  nData   - Array (Pointer) to Packed word data
 *  @return status  - error code (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibIpTableWrite(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et eSeg,
		     uint32 nIdx, uint32 nData[]);

/**
 *
 *  Provide logical read access to the different tables within the QE2k's IP
 *  memory. All table entries are accessed with a given IP segment that
 *  denotes the table, and an index within that table.
 *
 *  @param  pCtxt   - Pointer to the Egress context structure
 *  @param  eSeg    - Enumerated Type describing which table to access
 *  @param  nIdx    - Index of the entry within that table
 *  @return nData   - Array (Pointer) to Packed Word data
 *  @return status  - error code (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibIpTableRead(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et eSeg,
		    uint32 nIdx, uint32 nData[]);

/**
 *
 *  Define an entry in one of the SMAC tables located at the bottom of
 *  IP memory. This access function is shared between the different tables
 *  that reside, an enumerated type selects which table to access.
 *
 *  @param  pCtxt   - Pointer to the Egress context structure
 *  @param  eTble   - Enumerated Type selecting table
 *  @param  nIdx    - Index of entry within that table
 *  @param  nData   - Array (pointer) of packed data to be written
 *  @return status  - error code (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibIpSMACWrite(sbG2EplibCtxt_pst pCtxt, sbG2EplibSMACTbls_et eTbl,
		    uint32 nIdx, uint32 nData[]);

/**
 *
 *  Retrieve an entry in one of the SMAC tables located at the bottom of
 *  IP memory. This access function is shared between the different tables
 *  that reside, an enumerated type selects which table to access.
 *
 *  @param  pCtxt   - Pointer to the Egress context structure
 *  @param  eTble   - Enumerated Type selecting table
 *  @param  nIdx    - Index of entry within that table
 *  @return nData   - Array (pointer) where packed data is exported
 *  @return status  - error code (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibIpSMACRead(sbG2EplibCtxt_pst pCtxt, sbG2EplibSMACTbls_et eTbl,
		   uint32 nIdx, uint32 nData[]);

/**
 *
 * Utility functions to access the descriptor tables. These provide
 * access to the table and impose a read-only interface to the module-scoped
 * access functions and descriptor data.
 *
 **/
uint32 sbG2EplibGetSegBaseDefault(sbG2EplibIpSegs_et segment);
uint32 sbG2EplibGetSegWidthDefault(sbG2EplibIpSegs_et segment);
uint32 sbG2EplibGetSegEntrySizeDefault(sbG2EplibIpSegs_et segment);
uint32 sbG2EplibGetSegEndDefault(sbG2EplibIpSegs_et segment);

uint32 sbG2EplibGetSegBase(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et segment);
uint32 sbG2EplibGetSegWidth(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et segment);
uint32 sbG2EplibGetSegEntrySize(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et segment);
uint32 sbG2EplibGetSegEnd(sbG2EplibCtxt_pst pCtxt, sbG2EplibIpSegs_et segment);

uint32 sbG2EplibGetSMACBase(sbG2EplibSMACTbls_et tbl);
uint32 sbG2EplibGetSMACEnd(sbG2EplibSMACTbls_et tbl);

#endif /* __GU_ELIBTABLE_H__ */
