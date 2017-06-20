/*
 * $Id: fp_vo.h,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _FP_VO_H
#define _FP_VO_H

/* sw slice id     chip slice id
    0                       0
    1                       1
    2                       2
    3                       0+3 (slice0 + slice chain)
    4                       3 (slice chain only)
*/
#define CFP_53600_SLICE_ID_BASE     0
#define CFP_53600_SLICE_ID_CHAIN_SLICE  4
#define CFP_53600_SLICE_ID_WITH_CHAIN   3


#define CFP_53600_SLICE_MAP_ALL         0x1f
#define CFP_53600_SLICE_MAP_SINGLE      0x7
#define CFP_53600_SLICE_MAP_DOUBLE_WIDE (1 << CFP_53600_SLICE_ID_WITH_CHAIN)



#define CFP_53600_SLICE_ID_MAP(slice_id) \
    (0x1 << (slice_id))



#define CFP_53600_SLICE_PRI_LEVEL0     3
#define CFP_53600_SLICE_PRI_LEVEL1     5

#define CFP_53600_SLICE_CHAIN_UDF_BASE 36

#define CFP_53600_UDF_NUM_MAX   55

#define CFP_53600_UDF_OFFSET_MAX    62


#endif

