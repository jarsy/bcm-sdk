#ifndef __GU_ELIBCT_H__
#define __GU_ELIBCT_H__
/**
 * @file sbG2EplibCt.h Egress Class Table Initialization Functions
 *
 * <pre>
 * ==================================================================
 * ==  sbG2Eplib.h - QE2000 Egress Class Table Initialization Functions ==
 * ==================================================================
 *
 * WORKING REVISION: $Id: sbG2EplibCt.h,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MODULE NAME:
 *
 *     sbG2EplibCt.h
 *
 * ABSTRACT:
 *
 *     Egress Class Table Initialization Functions
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

#include <sbQe2000Elib.h>

/**
 *
 * Enumerated Type that describes IP hw types. In hardware, these will define
 * the expected header on a per-class basis. This allows the instruction
 * stream to do header-specific editing operations
 *
 **/
typedef enum sbG2EplibClassHwTypes_s {
  EN_CLS_HW_TYPE_VLAN = 0,   /**< Vlan-aware Bridging */
  EN_CLS_HW_TYPE_LI,         /**< Logical Interface */
  /* leave as last */
  EN_CLS_HW_TYPES
} sbG2EplibClassHwTypes_t;

/**
 *
 *  Setup the IP memory segment registers. We define the data width,
 *  the segment size, and the starting address of the segment.
 *
 *  @param  pCtxt   - Gu Eplib Context.
 *  @return status  - status (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibSegmentInit(sbG2EplibCtxt_pst pCtxt);

/**
 *
 *  Setup Class Resolution Table
 *
 *  @param  EpHandle - Pointer to Context Structure for Qe2k Eplib
 *  @return status   - status (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibCRTInit(SB_QE2000_ELIB_HANDLE EpHandle);

/**
 *
 *  Setup Class Instruction Table
 *
 *  @param  pEgCtxt - Pointer to Context Structure for Qe2k Eplib
 *  @return status   - status (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibCITInit(sbG2EplibCtxt_pst pEgCtxt);

/**
 *
 *  Configure the Expected header types on a per-class basis. This is used
 *  by the hardware to make assumptions about what type of headers will be
 *  arriving in each of the instruction streams.
 *
 *  @param  pHalCtx    - Pointer to Hal Context Structure for Qe2k Register Set
 *  @param  eHwClsType - Array of Enumerated Types of the different headers
 *  @return status     - status (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibClassTypesSet(void *pHalCtx, sbG2EplibClassHwTypes_t eHwClsType[]);

/**
 *
 *  Retrieve the Expected header types on a per-class basis. This is used
 *  by the hardware to make assumptions about what type of headers will be
 *  arriving in each of the instruction streams.
 *
 *  @param  pHalCtx    - Pointer to Hal Context Structure for Qe2k Register Set
 *  @return eHwClsType - Array of Enumerated Types of the different headers
 *  @return status     - status (zero on success)
 *
 **/
sbElibStatus_et
sbG2EplibClassTypesGet(void *pHalCtx, sbG2EplibClassHwTypes_t eHwClsType[]);

#endif /* __GU_ELIBCT_H__ */
