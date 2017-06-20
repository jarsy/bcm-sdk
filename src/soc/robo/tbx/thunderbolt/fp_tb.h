/*
 * $Id: fp_tb.h,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _FP_TB_H
#define _FP_TB_H

#include "../robo_tbx.h"

#define CFP_53280_L3_FRAME_IPV4 0x0
#define CFP_53280_L3_FRAME_IPV6 0x1
#define CFP_53280_L3_FRAME_NONIP 0x3
#define CFP_53280_L3_FRAME_CHAIN 0x2

#define CFP_53280_L3_FRAME(slice_id) \
    (((slice_id) & 0xc) >> 2)

#define CFP_53280_SLICE_ID(slice_id) \
    ((slice_id) & 0x3)

#define CFP_53280_SLICE_ID_MAP(slice_id) \
    (0x1 << (slice_id))

#define CFP_53280_SLICE_ID_IPV4_BASE        0
#define CFP_53280_SLICE_ID_IPV6_BASE        4
#define CFP_53280_SLICE_ID_NONIP_BASE       12

#define CFP_53280_SLICE_ID_WITH_CHAIN       3
#define CFP_53280_SLICE_ID_ALLOW_CHAIN     0 
#define CFP_53280_SLICE_ID_CHAIN_SLICE      11

#define CFP_53280_SLICE_MAP_FRAME_IPV4       0x0007
#define CFP_53280_SLICE_MAP_FRAME_IPV6       0x0070
#define CFP_53280_SLICE_MAP_FRAME_NONIP    0x7000

#define CFP_53280_SLICE_MAP_FRAME_IPV4_ALL       0x000f
#define CFP_53280_SLICE_MAP_FRAME_IPV6_ALL       0x00f0
#define CFP_53280_SLICE_MAP_FRAME_NONIP_ALL    0xf000
#define CFP_53280_SLICE_MAP_FRAME_CHAIN      0x800

#define CFP_53280_SLICE_MAP_FRAME_ANY 0xf0df
#define CFP_53280_SLICE_MAP_FRAME_IP 0x00ff

#define CFP_53280_SLICE_MAP_CHAIN  0x8088

#define CFP_53280_SLICE_CHAIN_UDF_BASE 35

#define CFP_53280_SLICE_PRI_LEVEL0     3
#define CFP_53280_SLICE_PRI_LEVEL1     5


#define CFP_53280_SLICE_MAX_ID 15 /* L3 fram + slice id */


/* User defined fields for BCM53280 */
#define CFP_53280_UDF_NUM_MAX 47
#define CFP_53280_UDF_OFFSET_MAX 96


#define CFP_53280_METER_RATE_MAX(unit) \
        (TB_RATE_MAX_REF_CNTS(unit) * TB_RATE_REF_COUNT_GRANULARITY(unit))

#define CFP_53280_METER_RATE_MIN(unit) 0

#define CFP_53280_METER_RATE_MAX_BUCKET_UNIT(unit)  \
    (SOC_IS_TB_AX(unit) ? (1<<9)-1 : (1<<20)-1)


/* max burst size (bytes)  */
#define CFP_53280_METER_RATE_MAX_BUCKET_SIZE(unit)   \
    (CFP_53280_METER_RATE_MAX_BUCKET_UNIT(unit) * TB_RATE_BUCKET_UNIT_SIZE(unit))

#define CFP_53280_METER_BURST_MAX(unit) \
    ((CFP_53280_METER_RATE_MAX_BUCKET_SIZE(unit) * 8)/1000)

#define CFP_53280_METER_BURST_MIN(unit) 0

#define CFP_53280_BYPASS_LAG    1 << 0
#define CFP_53280_BYPASS_TGF    1 << 1 
#define CFP_53280_BYPASS_PMF    1 << 2 
#define CFP_53280_BYPASS_STP    1 << 3
#define CFP_53280_BYPASS_EAP    1 << 4 
#define CFP_53280_BYPASS_VIF    1 << 5
#define CFP_53280_BYPASS_VEF    1 << 6 
#define CFP_53280_BYPASS_SA     1 << 7
#define CFP_53280_BYPASS_ALL 0xFF

typedef enum fp_cfp_fix_udf_map_e{
CFP_53280_UDF_DIP,
CFP_53280_UDF_SIP,
CFP_53280_UDF_SIP6,
CFP_53280_UDF_SIP6HI,
CFP_53280_UDF_SIP6LO,
CFP_53280_UDF_INVID,
CFP_53280_UDF_DA,
CFP_53280_UDF_SA
}fp_cfp_fix_udf_map_t;

#define CFP_53280_FIX_UDF_SET(qual) \
    (0x10000 << (qual)) 

#define TEST_CFP_53280_FIX_UDF(drv, qual) \
    ((drv->flags) & CFP_53280_FIX_UDF_SET(qual))

#endif  /* _FP_53280_H */
