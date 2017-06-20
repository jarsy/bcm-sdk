#ifndef __GU_ELIBCONTEXT_H__
#define __GU_ELIBCONTEXT_H__
/**
 * @file sbMkEplibContext.h - Define handle used for reference
 *
 * <pre>
 * =======================================================
 * ==  sbMkEplibContext.h - Define handle used for reference ==
 * =======================================================
 *
 * WORKING REVISION: $Id: sbG2EplibContext.h,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MODULE NAME:
 *
 *     sbMkEplibContext.h
 *
 * ABSTRACT:
 *
 *     Define handle used for reference
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     Travis B. Sawyer
 *     Josh Weissman
 *
 * CREATION DATE:
 *
 *     14-February-2005
 * </pre>
 */

#include "sbQe2000Elib.h"
#include "sbZfG2EplibIpSegment.hx"

typedef enum {
    G2EPLIB_UCODE = 0,
    G2EPLIB_QESS_UCODE = 1
}sbG2EplibUcode_t;

/* elib context struct */
typedef struct sbG2EplibCtxt_s {
    SB_QE2000_ELIB_HANDLE EpHandle;     /**< Handle to elib */
    void                  *pHalCtx;     /**< HAL Context */
    bool_t                bPCTCounts;   /**< Poll the PCT */
    sbZfG2EplibIpSegment_t tIpSegment[SB_QE2000_ELIB_NUM_SEGMENTS_K]; /**< IP Segment Memory Information */
    sbG2EplibUcode_t     eUcode;
} sbG2EplibCtxt_st, *sbG2EplibCtxt_pst;

#endif /* __GU_ELIBCONTEXT_H__ */
