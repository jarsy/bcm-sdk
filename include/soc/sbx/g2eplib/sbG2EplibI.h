#ifndef __GU_ELIBI_H__
#define __GU_ELIBI_H__
/**
 * @file sbG2EplibI.h Definitions, types and macros local to the elib
 *
 * <pre>
 * =================================================================
 * ==  sbG2EplibI.h - Definitions, types and macros local to the eplib
 * =================================================================
 *
 * WORKING REVISION: $Id: sbG2EplibI.h,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MODULE NAME:
 *
 *     sbG2EplibI.h
 *
 * ABSTRACT:
 *
 *     Definitions, types and macros local to the eplib
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


/* macro to calculate segment size in in 8-word segments */
#define SEG_BSIZE(start, end) (((end) + 1 - (start)) / SB_QE2000_ELIB_SEGMENT_SIZ_DIV_K)

/* Division, round up */
#define DIVUP(p,q) (((p) + (q) - 1) / (q))

/* Calculate Words required to hold zframe */
#define ZF_WDSIZE(x) \
 (DIVUP(SAND_HAL_FRAME_##x##_SIZE_IN_BYTES, sizeof(uint32)))

/* Single Bit Mask */
#define BIT_N(n)       (0x1 << (n))
#define GET_BIT_N(x,n) (((x) >> (n)) & 0x1)
#define MSKN_32(n)     (0xFFFFFFFF >> (32 - (n)))
#define MSKN_64(n)     (0xFFFFFFFFFFFFFFFFULL >> (64 - (n)))

/*
 * Compose Index into the Egress Tos/Exp Remap tables
 * Index = {Egress-Port, Cos, Dp, E}
 */
#define RMP_IDX(port, coss, dp, e) \
 ((((port) & 0x3f) << 6) | (((coss) & 0x7) << 3) | (((dp) & 0x3) << 1) | ((e) & 0x1))

#ifdef DENTER
#undef DENTER
#endif

#ifdef DEXIT
#undef DEXIT
#endif

/* debugging tracepoints */
#if defined (DEBUG_PRINT)
#include "sbWrappers.h"
#define DENTER() \
    LOG_WARN(BSL_LS_SOC_COMMON, \
             (BSL_META("%s: enter\n"), \
              FUNCTION_NAME()));
#define DBG(x)   x
#define DEXIT()  \
    LOG_WARN(BSL_LS_SOC_COMMON, \
             (BSL_META("%s: line: %d:  exit status %d\n"), \
              FUNCTION_NAME(), __LINE__, _code));
#else
#define DENTER()
#define DBG(x)
#define DEXIT(_code)
#endif

#endif /* __GU_ELIBI_H__ */
