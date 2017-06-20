/*
 * $Id: cfp.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <assert.h>
#include <soc/types.h>
#include <soc/error.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/drv_if.h>
#include <soc/cfp.h>
#include "thunderbolt/fp_tb.h"
#include "robo_tbx.h"
#ifdef BCM_VO_SUPPORT
#include <soc/robo/voyager_service.h>
#endif /* BCM_VO_SUPPORT */

/* Slice 0~3 qualify set */
/* Since the slice format was decides by slice ID and L3 framing */
/* We combined the l3 framinng and slice id as "slice id for upper layer */
/* L3 Framing         Slice ID              Slice ID value */
/* b00 (IPv4)              b00                  b0000(0)
                           b01                  b0001(1)
                           b10                  b0010(2)
   IPv4 chain              b11                  b0011(3)

   b01(IPv6)               b00                  b0100(4)
                           b01                  b0101(5)
                           b10                  b0110(6)
   IPv6 chain              b11                  b0111(7)

   b10(chain slice)                               b1011(11)

   b11(Non-IP)             b00                  b1100(12)
                           b01                  b1101(13)
                           b10                  b1110(14) 
   Non-IP chain            b11                  b1111(15)

                    
*/



/* IPv4    SLICEs */
static int s0_qset[] = { 
    DRV_CFP_QUAL_SRC_PROFILE,
    DRV_CFP_QUAL_SRC_PORT,
    DRV_CFP_QUAL_SPTAG,
    DRV_CFP_QUAL_1QTAG,
    DRV_CFP_QUAL_L2_FRM_FORMAT,
    DRV_CFP_QUAL_L3_FRM_FORMAT,
    DRV_CFP_QUAL_IP_TOS,
    DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,  /* DRV_CFP_QUAL_IP_TOS */
    DRV_CFP_QUAL_IP_PROTO,
    DRV_CFP_QUAL_IP6_NEXT_HEADER, /* DRV_CFP_QUAL_IP_PROTO */
    DRV_CFP_QUAL_IP_FRGA,
    DRV_CFP_QUAL_IP_NON_FIRST_FRGA,
    DRV_CFP_QUAL_IP_AUTH,
    DRV_CFP_QUAL_IP_TTL,
    DRV_CFP_QUAL_IP6_HOP_LIMIT, /* DRV_CFP_QUAL_IP_TTL */
    DRV_CFP_QUAL_SP_PRI,
    DRV_CFP_QUAL_SP_CFI,
    DRV_CFP_QUAL_SP_VID,
    DRV_CFP_QUAL_USR_PRI,
    DRV_CFP_QUAL_USR_CFI,
    DRV_CFP_QUAL_USR_VID,
    DRV_CFP_QUAL_IP_DA,
    DRV_CFP_QUAL_IP_SA,
    DRV_CFP_QUAL_UDFA0,
    DRV_CFP_QUAL_UDFA1,
    DRV_CFP_QUAL_UDFA2,
    DRV_CFP_QUAL_UDFA3,
    DRV_CFP_QUAL_UDFA4,
    DRV_CFP_QUAL_INVALID
};
static int s1_qset[] = { 
    DRV_CFP_QUAL_SRC_PROFILE,
    DRV_CFP_QUAL_SRC_PORT,
    DRV_CFP_QUAL_SPTAG,
    DRV_CFP_QUAL_1QTAG,
    DRV_CFP_QUAL_L2_FRM_FORMAT,
    DRV_CFP_QUAL_L3_FRM_FORMAT,
    DRV_CFP_QUAL_IP_TOS,
    DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,  /* DRV_CFP_QUAL_IP_TOS */    
    DRV_CFP_QUAL_IP_PROTO,
    DRV_CFP_QUAL_IP6_NEXT_HEADER, /* DRV_CFP_QUAL_IP_PROTO */
    DRV_CFP_QUAL_IP_FRGA,
    DRV_CFP_QUAL_IP_NON_FIRST_FRGA,
    DRV_CFP_QUAL_IP_AUTH,
    DRV_CFP_QUAL_IP_TTL,
    DRV_CFP_QUAL_IP6_HOP_LIMIT, /* DRV_CFP_QUAL_IP_TTL */    
    DRV_CFP_QUAL_SP_PRI,
    DRV_CFP_QUAL_SP_CFI,
    DRV_CFP_QUAL_SP_VID,
    DRV_CFP_QUAL_USR_PRI,
    DRV_CFP_QUAL_USR_CFI,
    DRV_CFP_QUAL_USR_VID,
    DRV_CFP_QUAL_IP_DA,
    DRV_CFP_QUAL_IP_SA,
    DRV_CFP_QUAL_MAC_SA,
    DRV_CFP_QUAL_UDFA5,
    DRV_CFP_QUAL_UDFA6,
    DRV_CFP_QUAL_INVALID
};
static int s2_qset[] = {
    DRV_CFP_QUAL_SRC_PROFILE,
    DRV_CFP_QUAL_SRC_PORT,
    DRV_CFP_QUAL_SPTAG,
    DRV_CFP_QUAL_1QTAG,
    DRV_CFP_QUAL_L2_FRM_FORMAT,
    DRV_CFP_QUAL_L3_FRM_FORMAT,
    DRV_CFP_QUAL_IP_TOS,
    DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,  /* DRV_CFP_QUAL_IP_TOS */
    DRV_CFP_QUAL_IP_PROTO,
    DRV_CFP_QUAL_IP6_NEXT_HEADER, /* DRV_CFP_QUAL_IP_PROTO */
    DRV_CFP_QUAL_IP_FRGA,
    DRV_CFP_QUAL_IP_NON_FIRST_FRGA,
    DRV_CFP_QUAL_IP_AUTH,
    DRV_CFP_QUAL_IP_TTL,
    DRV_CFP_QUAL_IP6_HOP_LIMIT, /* DRV_CFP_QUAL_IP_TTL */
    DRV_CFP_QUAL_SP_PRI,
    DRV_CFP_QUAL_SP_CFI,
    DRV_CFP_QUAL_SP_VID,
    DRV_CFP_QUAL_USR_PRI,
    DRV_CFP_QUAL_USR_CFI,
    DRV_CFP_QUAL_USR_VID,
    DRV_CFP_QUAL_UDFA7,
    DRV_CFP_QUAL_UDFA8,
    DRV_CFP_QUAL_UDFA9,
    DRV_CFP_QUAL_UDFA10,
    DRV_CFP_QUAL_UDFA11,
    DRV_CFP_QUAL_UDFA12,
    DRV_CFP_QUAL_UDFA13,
    DRV_CFP_QUAL_UDFA14,
    DRV_CFP_QUAL_UDFA15,
    DRV_CFP_QUAL_INVALID
}; 

static int s3_qset[] = { 
    DRV_CFP_QUAL_SRC_PROFILE,
    DRV_CFP_QUAL_SRC_PORT,
    DRV_CFP_QUAL_SPTAG,
    DRV_CFP_QUAL_1QTAG,
    DRV_CFP_QUAL_L2_FRM_FORMAT,
    DRV_CFP_QUAL_L3_FRM_FORMAT,
    DRV_CFP_QUAL_IP_TOS,
    DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,  /* DRV_CFP_QUAL_IP_TOS */
    DRV_CFP_QUAL_IP_PROTO,
    DRV_CFP_QUAL_IP6_NEXT_HEADER, /* DRV_CFP_QUAL_IP_PROTO */
    DRV_CFP_QUAL_IP_FRGA,
    DRV_CFP_QUAL_IP_NON_FIRST_FRGA,
    DRV_CFP_QUAL_IP_AUTH,
    DRV_CFP_QUAL_IP_TTL,
    DRV_CFP_QUAL_IP6_HOP_LIMIT, /* DRV_CFP_QUAL_IP_TTL */
    DRV_CFP_QUAL_SP_PRI,
    DRV_CFP_QUAL_SP_CFI,
    DRV_CFP_QUAL_SP_VID,
    DRV_CFP_QUAL_USR_PRI,
    DRV_CFP_QUAL_USR_CFI,
    DRV_CFP_QUAL_USR_VID,
    DRV_CFP_QUAL_IP_DA,
    DRV_CFP_QUAL_IP_SA,
    DRV_CFP_QUAL_UDFA0,
    DRV_CFP_QUAL_UDFA1,
    DRV_CFP_QUAL_UDFA2,
    DRV_CFP_QUAL_UDFA3,
    DRV_CFP_QUAL_UDFA4,
    DRV_CFP_QUAL_CHAIN_ID,
    DRV_CFP_QUAL_UDFD0,
    DRV_CFP_QUAL_UDFD1,
    DRV_CFP_QUAL_UDFD2,
    DRV_CFP_QUAL_UDFD3,
    DRV_CFP_QUAL_UDFD4,
    DRV_CFP_QUAL_UDFD5,
    DRV_CFP_QUAL_UDFD6,
    DRV_CFP_QUAL_UDFD7,
    DRV_CFP_QUAL_UDFD8,
    DRV_CFP_QUAL_UDFD9,
    DRV_CFP_QUAL_UDFD10,
    DRV_CFP_QUAL_UDFD11,
    DRV_CFP_QUAL_INVALID
};


/* IPv6 SLICEs */
static int s4_qset[] = { 
    DRV_CFP_QUAL_SRC_PROFILE,
    DRV_CFP_QUAL_SRC_PORT,
    DRV_CFP_QUAL_SPTAG,
    DRV_CFP_QUAL_1QTAG,
    DRV_CFP_QUAL_L2_FRM_FORMAT,
    DRV_CFP_QUAL_L3_FRM_FORMAT,
    DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,
    DRV_CFP_QUAL_IP_TOS,  /* DRV_CFP_QUAL_IP6_TRAFFIC_CLASS */    
    DRV_CFP_QUAL_IP6_NEXT_HEADER,
    DRV_CFP_QUAL_IP_PROTO, /* DRV_CFP_QUAL_IP6_NEXT_HEADER */
    DRV_CFP_QUAL_IP_FRGA,
    DRV_CFP_QUAL_IP_NON_FIRST_FRGA,
    DRV_CFP_QUAL_IP_AUTH,
    DRV_CFP_QUAL_IP6_HOP_LIMIT,
    DRV_CFP_QUAL_IP_TTL, /* DRV_CFP_QUAL_IP6_HOP_LIMIT */
    DRV_CFP_QUAL_SP_PRI,
    DRV_CFP_QUAL_SP_CFI,
    DRV_CFP_QUAL_SP_VID,
    DRV_CFP_QUAL_USR_PRI,
    DRV_CFP_QUAL_USR_CFI,
    DRV_CFP_QUAL_USR_VID,
    DRV_CFP_QUAL_IP6_SA,
    DRV_CFP_QUAL_UDFB0,
    DRV_CFP_QUAL_INVALID
};
static int s5_qset[] = {
    DRV_CFP_QUAL_SRC_PROFILE,
    DRV_CFP_QUAL_SRC_PORT,
    DRV_CFP_QUAL_SPTAG,
    DRV_CFP_QUAL_1QTAG,
    DRV_CFP_QUAL_L2_FRM_FORMAT,
    DRV_CFP_QUAL_L3_FRM_FORMAT,
    DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,
    DRV_CFP_QUAL_IP_TOS,  /* DRV_CFP_QUAL_IP6_TRAFFIC_CLASS */    
    DRV_CFP_QUAL_IP6_NEXT_HEADER,
    DRV_CFP_QUAL_IP_PROTO, /* DRV_CFP_QUAL_IP6_NEXT_HEADER */
    DRV_CFP_QUAL_IP_FRGA,
    DRV_CFP_QUAL_IP_NON_FIRST_FRGA,
    DRV_CFP_QUAL_IP_AUTH,
    DRV_CFP_QUAL_IP6_HOP_LIMIT,
    DRV_CFP_QUAL_IP_TTL, /* DRV_CFP_QUAL_IP6_HOP_LIMIT */
    DRV_CFP_QUAL_IP6_SA,
    DRV_CFP_QUAL_MAC_SA,
    DRV_CFP_QUAL_USR_VID,
    DRV_CFP_QUAL_INVALID
};

static int s6_qset[] = { 
    DRV_CFP_QUAL_SRC_PROFILE,
    DRV_CFP_QUAL_SRC_PORT,
    DRV_CFP_QUAL_SPTAG,
    DRV_CFP_QUAL_1QTAG,
    DRV_CFP_QUAL_L2_FRM_FORMAT,
    DRV_CFP_QUAL_L3_FRM_FORMAT,
    DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,
    DRV_CFP_QUAL_IP_TOS,  /* DRV_CFP_QUAL_IP6_TRAFFIC_CLASS */        
    DRV_CFP_QUAL_IP6_NEXT_HEADER,
    DRV_CFP_QUAL_IP_PROTO, /* DRV_CFP_QUAL_IP6_NEXT_HEADER */
    DRV_CFP_QUAL_IP_FRGA,
    DRV_CFP_QUAL_IP_NON_FIRST_FRGA,
    DRV_CFP_QUAL_IP_AUTH,
    DRV_CFP_QUAL_IP6_HOP_LIMIT,
    DRV_CFP_QUAL_IP_TTL, /* DRV_CFP_QUAL_IP6_HOP_LIMIT */
    DRV_CFP_QUAL_SP_PRI,
    DRV_CFP_QUAL_SP_CFI,
    DRV_CFP_QUAL_SP_VID,
    DRV_CFP_QUAL_USR_PRI,
    DRV_CFP_QUAL_USR_CFI,
    DRV_CFP_QUAL_USR_VID,
    DRV_CFP_QUAL_UDFB1,
    DRV_CFP_QUAL_UDFB2,
    DRV_CFP_QUAL_UDFB3,
    DRV_CFP_QUAL_UDFB4,
    DRV_CFP_QUAL_UDFB5,
    DRV_CFP_QUAL_UDFB6,
    DRV_CFP_QUAL_UDFB7,
    DRV_CFP_QUAL_UDFB8,
    DRV_CFP_QUAL_UDFB9,
    DRV_CFP_QUAL_INVALID
};
    
static int s7_qset[] = { 
    DRV_CFP_QUAL_SRC_PROFILE,
    DRV_CFP_QUAL_SRC_PORT,
    DRV_CFP_QUAL_SPTAG,
    DRV_CFP_QUAL_1QTAG,
    DRV_CFP_QUAL_L2_FRM_FORMAT,
    DRV_CFP_QUAL_L3_FRM_FORMAT,
    DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,
    DRV_CFP_QUAL_IP_TOS,  /* DRV_CFP_QUAL_IP6_TRAFFIC_CLASS */    
    DRV_CFP_QUAL_IP6_NEXT_HEADER,
    DRV_CFP_QUAL_IP_PROTO, /* DRV_CFP_QUAL_IP6_NEXT_HEADER */
    DRV_CFP_QUAL_IP_FRGA,
    DRV_CFP_QUAL_IP_NON_FIRST_FRGA,
    DRV_CFP_QUAL_IP_AUTH,
    DRV_CFP_QUAL_IP6_HOP_LIMIT,
    DRV_CFP_QUAL_IP_TTL, /* DRV_CFP_QUAL_IP6_HOP_LIMIT */
    DRV_CFP_QUAL_SP_PRI,
    DRV_CFP_QUAL_SP_CFI,
    DRV_CFP_QUAL_SP_VID,
    DRV_CFP_QUAL_USR_PRI,
    DRV_CFP_QUAL_USR_CFI,
    DRV_CFP_QUAL_USR_VID,
    DRV_CFP_QUAL_IP6_SA,
    DRV_CFP_QUAL_UDFB0,
    DRV_CFP_QUAL_CHAIN_ID,
    DRV_CFP_QUAL_UDFD0,
    DRV_CFP_QUAL_UDFD1,
    DRV_CFP_QUAL_UDFD2,
    DRV_CFP_QUAL_UDFD3,
    DRV_CFP_QUAL_UDFD4,
    DRV_CFP_QUAL_UDFD5,
    DRV_CFP_QUAL_UDFD6,
    DRV_CFP_QUAL_UDFD7,
    DRV_CFP_QUAL_UDFD8,
    DRV_CFP_QUAL_UDFD9,
    DRV_CFP_QUAL_UDFD10,
    DRV_CFP_QUAL_UDFD11,
    DRV_CFP_QUAL_INVALID
};

/* CHIAN SLICE*/
static int s11_qset[] = { 
    DRV_CFP_QUAL_SRC_PROFILE,
    DRV_CFP_QUAL_SRC_PORT,
    DRV_CFP_QUAL_CHAIN_ID,
    DRV_CFP_QUAL_UDFD0,
    DRV_CFP_QUAL_UDFD1,
    DRV_CFP_QUAL_UDFD2,
    DRV_CFP_QUAL_UDFD3,
    DRV_CFP_QUAL_UDFD4,
    DRV_CFP_QUAL_UDFD5,
    DRV_CFP_QUAL_UDFD6,
    DRV_CFP_QUAL_UDFD7,
    DRV_CFP_QUAL_UDFD8,
    DRV_CFP_QUAL_UDFD9,
    DRV_CFP_QUAL_UDFD10,
    DRV_CFP_QUAL_UDFD11,
    DRV_CFP_QUAL_INVALID
};

/* Non-IP SLICEs */
static int s12_qset[] = { 
    DRV_CFP_QUAL_SRC_PROFILE,
    DRV_CFP_QUAL_SRC_PORT,
    DRV_CFP_QUAL_SPTAG,
    DRV_CFP_QUAL_1QTAG,
    DRV_CFP_QUAL_L2_FRM_FORMAT,
    DRV_CFP_QUAL_L3_FRM_FORMAT,
    DRV_CFP_QUAL_ETYPE,
    DRV_CFP_QUAL_SP_PRI,
    DRV_CFP_QUAL_SP_CFI,
    DRV_CFP_QUAL_SP_VID,
    DRV_CFP_QUAL_USR_PRI,
    DRV_CFP_QUAL_USR_CFI,
    DRV_CFP_QUAL_USR_VID,
    DRV_CFP_QUAL_MAC_DA,
    DRV_CFP_QUAL_MAC_SA,
    DRV_CFP_QUAL_UDFC0,
    DRV_CFP_QUAL_UDFC1,
    DRV_CFP_QUAL_UDFC2,
    DRV_CFP_QUAL_INVALID
};
static int s13_qset[] = { 
    DRV_CFP_QUAL_SRC_PROFILE,
    DRV_CFP_QUAL_SRC_PORT,
    DRV_CFP_QUAL_SPTAG,
    DRV_CFP_QUAL_1QTAG,
    DRV_CFP_QUAL_L2_FRM_FORMAT,
    DRV_CFP_QUAL_L3_FRM_FORMAT,
    DRV_CFP_QUAL_ETYPE,
    DRV_CFP_QUAL_SP_PRI,
    DRV_CFP_QUAL_SP_CFI,
    DRV_CFP_QUAL_SP_VID,
    DRV_CFP_QUAL_USR_PRI,
    DRV_CFP_QUAL_USR_CFI,
    DRV_CFP_QUAL_USR_VID,
    DRV_CFP_QUAL_MAC_DA,
    DRV_CFP_QUAL_MAC_SA,
    DRV_CFP_QUAL_UDFC3,
    DRV_CFP_QUAL_UDFC4,
    DRV_CFP_QUAL_UDFC5,
    DRV_CFP_QUAL_INVALID
};
static int s14_qset[] = { 
    DRV_CFP_QUAL_SRC_PROFILE,
    DRV_CFP_QUAL_SRC_PORT,
    DRV_CFP_QUAL_SPTAG,
    DRV_CFP_QUAL_1QTAG,
    DRV_CFP_QUAL_L2_FRM_FORMAT,
    DRV_CFP_QUAL_L3_FRM_FORMAT,
    DRV_CFP_QUAL_ETYPE,
    DRV_CFP_QUAL_SP_PRI,
    DRV_CFP_QUAL_SP_CFI,
    DRV_CFP_QUAL_SP_VID,
    DRV_CFP_QUAL_USR_PRI,
    DRV_CFP_QUAL_USR_CFI,
    DRV_CFP_QUAL_USR_VID,
    DRV_CFP_QUAL_MAC_DA,
    DRV_CFP_QUAL_MAC_SA,
    DRV_CFP_QUAL_UDFC6,
    DRV_CFP_QUAL_UDFC7,
    DRV_CFP_QUAL_UDFC8,
    DRV_CFP_QUAL_INVALID
};
static int s15_qset[] = { 
    DRV_CFP_QUAL_SRC_PROFILE,
    DRV_CFP_QUAL_SRC_PORT,
    DRV_CFP_QUAL_SPTAG,
    DRV_CFP_QUAL_1QTAG,
    DRV_CFP_QUAL_L2_FRM_FORMAT,
    DRV_CFP_QUAL_L3_FRM_FORMAT,
    DRV_CFP_QUAL_ETYPE,
    DRV_CFP_QUAL_SP_PRI,
    DRV_CFP_QUAL_SP_CFI,
    DRV_CFP_QUAL_SP_VID,
    DRV_CFP_QUAL_USR_PRI,
    DRV_CFP_QUAL_USR_CFI,
    DRV_CFP_QUAL_USR_VID,
    DRV_CFP_QUAL_MAC_DA,
    DRV_CFP_QUAL_MAC_SA,
    DRV_CFP_QUAL_UDFC0,
    DRV_CFP_QUAL_UDFC1,
    DRV_CFP_QUAL_UDFC2,
    DRV_CFP_QUAL_CHAIN_ID,
    DRV_CFP_QUAL_UDFD0,
    DRV_CFP_QUAL_UDFD1,
    DRV_CFP_QUAL_UDFD2,
    DRV_CFP_QUAL_UDFD3,
    DRV_CFP_QUAL_UDFD4,
    DRV_CFP_QUAL_UDFD5,
    DRV_CFP_QUAL_UDFD6,
    DRV_CFP_QUAL_UDFD7,
    DRV_CFP_QUAL_UDFD8,
    DRV_CFP_QUAL_UDFD9,
    DRV_CFP_QUAL_UDFD10,
    DRV_CFP_QUAL_UDFD11,
    DRV_CFP_QUAL_INVALID
};


#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? BYTES2WORDS((m)->bytes)-1-(v) : (v))


typedef struct drv_slice_info_s {
    int slice_id;
    SHR_BITDCL  qset[_SHR_BITDCLSIZE(DRV_CFP_QUAL_COUNT)];
    int slice_udf_free;
}drv_slice_info_t;

static drv_slice_info_t drv53280_slice_info[CFP_53280_SLICE_MAX_ID +1]; 

typedef struct drv_udf_reg_info_s {
    int udf_id;
    int cfp_qual;
    int cfp_field;
    int cfp_field_valid;
    uint32 reg_id;
    uint32 reg_index;
    int slice_id;
}drv_udf_reg_info_t;
 
static  drv_udf_reg_info_t drv53280_udf_reg_info[] = {
{ 0,  DRV_CFP_QUAL_UDFA0,  DRV_CFP_FIELD_UDFA0_0, DRV_CFP_FIELD_UDF0_VALID, INDEX(CFP_UDF_0_Ar), 0, 0},   
{ 1,  DRV_CFP_QUAL_UDFA1,  DRV_CFP_FIELD_UDFA1_0, DRV_CFP_FIELD_UDF1_VALID, INDEX(CFP_UDF_0_Ar), 1, 0},   
{ 2,  DRV_CFP_QUAL_UDFA2,  DRV_CFP_FIELD_UDFA2_0, DRV_CFP_FIELD_UDF2_VALID, INDEX(CFP_UDF_0_Ar), 2, 0},   
{ 3,  DRV_CFP_QUAL_UDFA3,  DRV_CFP_FIELD_UDFA3_0, DRV_CFP_FIELD_UDF3_VALID, INDEX(CFP_UDF_0_Ar), 3, 0},   
{ 4,  DRV_CFP_QUAL_UDFA4,  DRV_CFP_FIELD_UDFA4_0, DRV_CFP_FIELD_UDF4_VALID, INDEX(CFP_UDF_0_Ar), 4, 0},   
{ 5,  DRV_CFP_QUAL_UDFA5,  DRV_CFP_FIELD_UDFA0_1, DRV_CFP_FIELD_UDF0_VALID, INDEX(CFP_UDF_1_Ar), 0, 1},   
{ 6,  DRV_CFP_QUAL_UDFA6,  DRV_CFP_FIELD_UDFA1_1, DRV_CFP_FIELD_UDF1_VALID, INDEX(CFP_UDF_1_Ar), 1, 1},   
{ 7,  DRV_CFP_QUAL_UDFA7,  DRV_CFP_FIELD_UDFA0_2, DRV_CFP_FIELD_UDF0_VALID, INDEX(CFP_UDF_2_Ar), 0, 2},   
{ 8,  DRV_CFP_QUAL_UDFA8,  DRV_CFP_FIELD_UDFA1_2, DRV_CFP_FIELD_UDF1_VALID, INDEX(CFP_UDF_2_Ar), 1, 2},   
{ 9,  DRV_CFP_QUAL_UDFA9,  DRV_CFP_FIELD_UDFA2_2, DRV_CFP_FIELD_UDF2_VALID, INDEX(CFP_UDF_2_Ar), 2, 2},   
{ 10, DRV_CFP_QUAL_UDFA10, DRV_CFP_FIELD_UDFA3_2, DRV_CFP_FIELD_UDF3_VALID, INDEX(CFP_UDF_2_Ar), 3, 2},   
{ 11, DRV_CFP_QUAL_UDFA11, DRV_CFP_FIELD_UDFA4_2, DRV_CFP_FIELD_UDF4_VALID, INDEX(CFP_UDF_2_Ar), 4, 2},   
{ 12, DRV_CFP_QUAL_UDFA12, DRV_CFP_FIELD_UDFA5_2, DRV_CFP_FIELD_UDF5_VALID, INDEX(CFP_UDF_2_Ar), 5, 2},   
{ 13, DRV_CFP_QUAL_UDFA13, DRV_CFP_FIELD_UDFA6_2, DRV_CFP_FIELD_UDF6_VALID, INDEX(CFP_UDF_2_Ar), 6, 2},   
{ 14, DRV_CFP_QUAL_UDFA14, DRV_CFP_FIELD_UDFA7_2, DRV_CFP_FIELD_UDF7_VALID, INDEX(CFP_UDF_2_Ar), 7, 2},   
{ 15, DRV_CFP_QUAL_UDFA15, DRV_CFP_FIELD_UDFA8_2, DRV_CFP_FIELD_UDF8_VALID, INDEX(CFP_UDF_2_Ar), 8, 2},   
{ 16, DRV_CFP_QUAL_UDFB0,  DRV_CFP_FIELD_UDFB8_0, DRV_CFP_FIELD_UDF8_VALID, INDEX(CFP_UDF_0_B08r),0, 4},  
{ 17, DRV_CFP_QUAL_UDFB1,  DRV_CFP_FIELD_UDFB0_2, DRV_CFP_FIELD_UDF0_VALID, INDEX(CFP_UDF_2_Br), 0, 6},   
{ 18, DRV_CFP_QUAL_UDFB2,  DRV_CFP_FIELD_UDFB1_2, DRV_CFP_FIELD_UDF1_VALID, INDEX(CFP_UDF_2_Br), 1, 6},   
{ 19, DRV_CFP_QUAL_UDFB3,  DRV_CFP_FIELD_UDFB2_2, DRV_CFP_FIELD_UDF2_VALID, INDEX(CFP_UDF_2_Br), 2, 6},   
{ 20, DRV_CFP_QUAL_UDFB4,  DRV_CFP_FIELD_UDFB3_2, DRV_CFP_FIELD_UDF3_VALID, INDEX(CFP_UDF_2_Br), 3, 6},   
{ 21, DRV_CFP_QUAL_UDFB5,  DRV_CFP_FIELD_UDFB4_2, DRV_CFP_FIELD_UDF4_VALID, INDEX(CFP_UDF_2_Br), 4, 6},   
{ 22, DRV_CFP_QUAL_UDFB6,  DRV_CFP_FIELD_UDFB5_2, DRV_CFP_FIELD_UDF5_VALID, INDEX(CFP_UDF_2_Br), 5, 6},   
{ 23, DRV_CFP_QUAL_UDFB7,  DRV_CFP_FIELD_UDFB6_2, DRV_CFP_FIELD_UDF6_VALID, INDEX(CFP_UDF_2_Br), 6, 6},   
{ 24, DRV_CFP_QUAL_UDFB8,  DRV_CFP_FIELD_UDFB7_2, DRV_CFP_FIELD_UDF7_VALID, INDEX(CFP_UDF_2_Br), 7, 6},   
{ 25, DRV_CFP_QUAL_UDFB9,  DRV_CFP_FIELD_UDFB8_2, DRV_CFP_FIELD_UDF8_VALID, INDEX(CFP_UDF_2_Br), 8, 6},   
{ 26, DRV_CFP_QUAL_UDFC0,  DRV_CFP_FIELD_UDFC0, DRV_CFP_FIELD_UDF0_VALID, INDEX(CFP_UDF_0_Cr), 0, 12},     
{ 27, DRV_CFP_QUAL_UDFC1,  DRV_CFP_FIELD_UDFC1, DRV_CFP_FIELD_UDF1_VALID, INDEX(CFP_UDF_0_Cr), 1, 12},     
{ 28, DRV_CFP_QUAL_UDFC2,  DRV_CFP_FIELD_UDFC2, DRV_CFP_FIELD_UDF2_VALID, INDEX(CFP_UDF_0_Cr), 2, 12},     
{ 29, DRV_CFP_QUAL_UDFC3,  DRV_CFP_FIELD_UDFC0, DRV_CFP_FIELD_UDF0_VALID, INDEX(CFP_UDF_1_Cr), 0, 13},     
{ 30, DRV_CFP_QUAL_UDFC4,  DRV_CFP_FIELD_UDFC1, DRV_CFP_FIELD_UDF1_VALID, INDEX(CFP_UDF_1_Cr), 1, 13},     
{ 31, DRV_CFP_QUAL_UDFC5,  DRV_CFP_FIELD_UDFC2, DRV_CFP_FIELD_UDF2_VALID, INDEX(CFP_UDF_1_Cr), 2, 13},     
{ 32, DRV_CFP_QUAL_UDFC6,  DRV_CFP_FIELD_UDFC0, DRV_CFP_FIELD_UDF0_VALID, INDEX(CFP_UDF_2_Cr), 0, 14},     
{ 33, DRV_CFP_QUAL_UDFC7,  DRV_CFP_FIELD_UDFC1, DRV_CFP_FIELD_UDF1_VALID, INDEX(CFP_UDF_2_Cr), 1, 14},     
{ 34, DRV_CFP_QUAL_UDFC8,  DRV_CFP_FIELD_UDFC2, DRV_CFP_FIELD_UDF2_VALID, INDEX(CFP_UDF_2_Cr), 2, 14},     
{ 35, DRV_CFP_QUAL_UDFD0,  DRV_CFP_FIELD_UDFD0,  DRV_CFP_FIELD_UDF0_VALID, INDEX(CFP_UDF_3_Dr),  0, 11},     
{ 36, DRV_CFP_QUAL_UDFD1,  DRV_CFP_FIELD_UDFD1,  DRV_CFP_FIELD_UDF1_VALID, INDEX(CFP_UDF_3_Dr),  1, 11},     
{ 37, DRV_CFP_QUAL_UDFD2,  DRV_CFP_FIELD_UDFD2,  DRV_CFP_FIELD_UDF2_VALID, INDEX(CFP_UDF_3_Dr),  2, 11},     
{ 38, DRV_CFP_QUAL_UDFD3,  DRV_CFP_FIELD_UDFD3,  DRV_CFP_FIELD_UDF3_VALID, INDEX(CFP_UDF_3_Dr),  3, 11},     
{ 39, DRV_CFP_QUAL_UDFD4,  DRV_CFP_FIELD_UDFD4,  DRV_CFP_FIELD_UDF4_VALID, INDEX(CFP_UDF_3_Dr),  4, 11},     
{ 40, DRV_CFP_QUAL_UDFD5,  DRV_CFP_FIELD_UDFD5,  DRV_CFP_FIELD_UDF5_VALID, INDEX(CFP_UDF_3_Dr),  5, 11},     
{ 41, DRV_CFP_QUAL_UDFD6,  DRV_CFP_FIELD_UDFD6,  DRV_CFP_FIELD_UDF6_VALID, INDEX(CFP_UDF_3_Dr),  6, 11},     
{ 42, DRV_CFP_QUAL_UDFD7,  DRV_CFP_FIELD_UDFD7,  DRV_CFP_FIELD_UDF7_VALID, INDEX(CFP_UDF_3_Dr),  7, 11},     
{ 43, DRV_CFP_QUAL_UDFD8,  DRV_CFP_FIELD_UDFD8,  DRV_CFP_FIELD_UDF8_VALID, INDEX(CFP_UDF_3_Dr),  8, 11},     
{ 44, DRV_CFP_QUAL_UDFD9,  DRV_CFP_FIELD_UDFD9,  DRV_CFP_FIELD_UDF9_VALID, INDEX(CFP_UDF_3_Dr),  9, 11},     
{ 45, DRV_CFP_QUAL_UDFD10, DRV_CFP_FIELD_UDFD10, DRV_CFP_FIELD_UDF10_VALID, INDEX(CFP_UDF_3_Dr), 10, 11},    
{ 46, DRV_CFP_QUAL_UDFD11, DRV_CFP_FIELD_UDFD11, DRV_CFP_FIELD_UDF11_VALID, INDEX(CFP_UDF_3_Dr), 11, 11}     
};

#define CFP_UDF_GET(unit, index, val) \
    DRV_REG_READ(unit, DRV_REG_ADDR(unit, \
        drv53280_udf_reg_info[index].reg_id, 0, \
        drv53280_udf_reg_info[index].reg_index),\
        val, DRV_REG_LENGTH_GET(unit, drv53280_udf_reg_info[index].reg_id))

#define CFP_UDF_SET(unit, index, val) \
    DRV_REG_WRITE(unit, DRV_REG_ADDR(unit, \
        drv53280_udf_reg_info[index].reg_id, 0, \
        drv53280_udf_reg_info[index].reg_index),\
        val, DRV_REG_LENGTH_GET(unit, drv53280_udf_reg_info[index].reg_id))


#define REV_A0 0
#define REV_B0 1

static drv_cfp_field_map_t  drv_cfp_field_map_table[] = {
    {DRV_CFP_FIELD_VALID, INDEX(VALIDf), -1},
    {DRV_CFP_FIELD_SRC_PROFILE, INDEX(SRC_PROFILEf), -1},
    {DRV_CFP_FIELD_SRC_PORT, INDEX(SRC_PORTf), -1},
    {DRV_CFP_FIELD_SPTAGGED, INDEX(S_TAGGEDf), -1},
    {DRV_CFP_FIELD_1QTAGGED, INDEX(C_TAGGEDf), -1},
    {DRV_CFP_FIELD_L2_FRM_FORMAT, INDEX(L2_FRAMINGf), -1},
    {DRV_CFP_FIELD_L3_FRM_FORMAT, INDEX(L3_FRAMINGf), -1},
    {DRV_CFP_FIELD_IP_TOS, INDEX(IP_TOSf), -1},
    {DRV_CFP_FIELD_IP6_TRAFFIC_CLASS, INDEX(IP_TRAFFICf), -1},
    {DRV_CFP_FIELD_ETYPE, INDEX(ETHERTYPE_SAPf), -1},
    {DRV_CFP_FIELD_IP_PROTO, INDEX(IP_PROTOf), -1},
    {DRV_CFP_FIELD_IP6_NEXT_HEADER, INDEX(IP_NEXT_HEADERf), -1},
    {DRV_CFP_FIELD_IP_FRAG, INDEX(IP_FRAGf), -1},
    {DRV_CFP_FIELD_IP_NON_FIRST_FRAG, INDEX(NON_FIRST_FRAGf), -1},
    {DRV_CFP_FIELD_IP_AUTH, INDEX(IP_AUTHf), -1},
    {DRV_CFP_FIELD_IP_TTL, INDEX(TTL_RANGEf), -1},
    {DRV_CFP_FIELD_IP6_HOP_LIMIT, INDEX(HOP_LIMIT_RANGEf), -1},
    {DRV_CFP_FIELD_CHAIN_ID, INDEX(CHAIN_IDf), -1},
    {DRV_CFP_FIELD_UDF_ALL_VALID, INDEX(ALL_VLDf), -1},
    {DRV_CFP_FIELD_UDF11_VALID, INDEX(UDF11_VLDf), -1},
    {DRV_CFP_FIELD_UDF10_VALID, INDEX(UDF10_VLDf), -1},
    {DRV_CFP_FIELD_UDF9_VALID, INDEX(UDF9_VLDf), -1},
    {DRV_CFP_FIELD_UDF8_VALID, INDEX(UDF8_VLDf), -1},
    {DRV_CFP_FIELD_UDF7_VALID, INDEX(UDF7_VLDf), -1},
    {DRV_CFP_FIELD_UDF6_VALID, INDEX(UDF6_VLDf), -1},
    {DRV_CFP_FIELD_UDF5_VALID, INDEX(UDF5_VLDf), -1},
    {DRV_CFP_FIELD_UDF4_VALID, INDEX(UDF4_VLDf), -1},
    {DRV_CFP_FIELD_UDF3_VALID, INDEX(UDF3_VLDf), -1},
    {DRV_CFP_FIELD_UDF2_VALID, INDEX(UDF2_VLDf), -1},
    {DRV_CFP_FIELD_UDF1_VALID, INDEX(UDF1_VLDf), -1},                      
    {DRV_CFP_FIELD_UDF0_VALID, INDEX(UDF0_VLDf), -1},
    {DRV_CFP_FIELD_SP_PRI, INDEX(SP_PRIf), -1},
    {DRV_CFP_FIELD_SP_CFI, INDEX(SP_CFIf), -1},
    {DRV_CFP_FIELD_SP_VID, INDEX(SP_VIDf), -1},
    {DRV_CFP_FIELD_USR_PRI, INDEX(USR_PRIf), -1},
    {DRV_CFP_FIELD_USR_CFI, INDEX(USR_CFIf), -1},
    {DRV_CFP_FIELD_USR_VID, INDEX(USR_VIDf), -1},
    {DRV_CFP_FIELD_IP_DA, INDEX(IPV4_DAf), -1},
    {DRV_CFP_FIELD_IP_SA, INDEX(IPV4_SAf), -1},
    {DRV_CFP_FIELD_IP6_SA, INDEX(IPV6_SAf), -1},
    {DRV_CFP_FIELD_MAC_SA, INDEX(MAC_SAf), -1},
    {DRV_CFP_FIELD_MAC_DA, INDEX(MAC_DAf), -1},
    {DRV_CFP_FIELD_UDFA4_0, INDEX(UDF_0_A4f), -1},
    {DRV_CFP_FIELD_UDFA3_0, INDEX(UDF_0_A3f), -1},
    {DRV_CFP_FIELD_UDFA2_0, INDEX(UDF_0_A2f), -1},
    {DRV_CFP_FIELD_UDFA1_0, INDEX(UDF_0_A1f), -1},
    {DRV_CFP_FIELD_UDFA0_0, INDEX(UDF_0_A0f), -1},
    {DRV_CFP_FIELD_UDFA1_1, INDEX(UDF_1_A1f), -1},
    {DRV_CFP_FIELD_UDFA0_1, INDEX(UDF_1_A0f), -1},
    {DRV_CFP_FIELD_UDFA8_2 , INDEX(UDF_2_A8f), -1},
    {DRV_CFP_FIELD_UDFA7_2 , INDEX(UDF_2_A7f), -1},
    {DRV_CFP_FIELD_UDFA6_2 , INDEX(UDF_2_A6f), -1},
    {DRV_CFP_FIELD_UDFA5_2 , INDEX(UDF_2_A5f), -1},
    {DRV_CFP_FIELD_UDFA4_2 , INDEX(UDF_2_A4f), -1},
    {DRV_CFP_FIELD_UDFA3_2 , INDEX(UDF_2_A3f), -1},
    {DRV_CFP_FIELD_UDFA2_2 , INDEX(UDF_2_A2f), -1},
    {DRV_CFP_FIELD_UDFA1_2 , INDEX(UDF_2_A1f), -1},
    {DRV_CFP_FIELD_UDFA0_2 , INDEX(UDF_2_A0f), -1},
    {DRV_CFP_FIELD_UDFB8_0 , INDEX(UDF_0_B8f), -1},
    {DRV_CFP_FIELD_UDFB8_2 , INDEX(UDF_2_B8f), -1},                                        
    {DRV_CFP_FIELD_UDFB7_2 , INDEX(UDF_2_B7f), -1},
    {DRV_CFP_FIELD_UDFB6_2 , INDEX(UDF_2_B6f), -1},
    {DRV_CFP_FIELD_UDFB5_2 , INDEX(UDF_2_B5f), -1},
    {DRV_CFP_FIELD_UDFB4_2 , INDEX(UDF_2_B4f), -1},
    {DRV_CFP_FIELD_UDFB3_2 , INDEX(UDF_2_B3f), -1},
    {DRV_CFP_FIELD_UDFB2_2 , INDEX(UDF_2_B2f), -1},
    {DRV_CFP_FIELD_UDFB1_2 , INDEX(UDF_2_B1f), -1},
    {DRV_CFP_FIELD_UDFB0_2 , INDEX(UDF_2_B0f), -1},
    {DRV_CFP_FIELD_UDFC2 , INDEX(UDF_N_C2f), -1},
    {DRV_CFP_FIELD_UDFC1 , INDEX(UDF_N_C1f), -1},
    {DRV_CFP_FIELD_UDFC0, INDEX(UDF_N_C0f), -1},
    {DRV_CFP_FIELD_UDFD11, INDEX(UDF_D11f), -1},
    {DRV_CFP_FIELD_UDFD10, INDEX(UDF_D10f), -1},
    {DRV_CFP_FIELD_UDFD9, INDEX(UDF_D9f), -1},
    {DRV_CFP_FIELD_UDFD8, INDEX(UDF_D8f), -1},
    {DRV_CFP_FIELD_UDFD7, INDEX(UDF_D7f), -1},
    {DRV_CFP_FIELD_UDFD6, INDEX(UDF_D6f), -1},
    {DRV_CFP_FIELD_UDFD5, INDEX(UDF_D5f), -1},
    {DRV_CFP_FIELD_UDFD4, INDEX(UDF_D4f), -1},
    {DRV_CFP_FIELD_UDFD3, INDEX(UDF_D3f), -1},
    {DRV_CFP_FIELD_UDFD2, INDEX(UDF_D2f), -1},
    {DRV_CFP_FIELD_UDFD1, INDEX(UDF_D1f), -1},
    {DRV_CFP_FIELD_UDFD0, INDEX(UDF_D0f), -1},
    {DRV_CFP_FIELD_SLICE_ID, INDEX(SC_N_IDf), -1},

    /* METER */    
    {DRV_CFP_FIELD_RATE, INDEX(RM_ENf), REV_A0},
    {DRV_CFP_FIELD_BUCKET_SIZE, INDEX(BKTSIZEf), REV_A0},
    {DRV_CFP_FIELD_REF_CNT, INDEX(RFSHCNTf), REV_A0},
     /* STAT */
    {DRV_CFP_FIELD_IB_CNT, INDEX(IN_BAND_CNTf), REV_A0},
    {DRV_CFP_FIELD_OB_CNT, INDEX(OUT_BAND_CNTf), REV_A0}
};

static drv_cfp_field_map_t  drv_cfp_action_field_map_table[] = 
{
        /* ACT_POLm */
    {DRV_CFP_ACT_FILTER_ALL, INDEX(FLT_BYPf), -1},
    {DRV_CFP_ACT_FILTER_SA, INDEX(FLT_BYPf), -1},
    {DRV_CFP_ACT_FILTER_EGRESS_VLAN, INDEX(FLT_BYPf), -1},
    {DRV_CFP_ACT_FILTER_INGRESS_VLAN, INDEX(FLT_BYPf), -1},
    {DRV_CFP_ACT_FILTER_EAP, INDEX(FLT_BYPf), -1},
    {DRV_CFP_ACT_FILTER_STP, INDEX(FLT_BYPf), -1},
    {DRV_CFP_ACT_FILTER_PORT_MASK, INDEX(FLT_BYPf), -1},
    {DRV_CFP_ACT_FILTER_TAGGED, INDEX(FLT_BYPf), -1},
    {DRV_CFP_ACT_FILTER_LAG, INDEX(FLT_BYPf), -1},        
    {DRV_CFP_ACT_DIS_LRN, INDEX(LRN_DISf), -1},
    {DRV_CFP_ACT_DIS_LRN_CANCEL, INDEX(LRN_DISf), -1},
    {DRV_CFP_ACT_DA_KNOWN, INDEX(DA_KNOWNf), -1},
    {DRV_CFP_ACT_DA_KNOWN_CANCEL, INDEX(DA_KNOWNf), -1},
    {DRV_CFP_ACT_MIRROR_COPY, INDEX(MIR_COPYf), -1},
    {DRV_CFP_ACT_MIRROR_COPY_CANCEL, INDEX(MIR_COPYf), -1},
    {DRV_CFP_ACT_CPU_COPY, INDEX(CPU_COPYf), -1},
    {DRV_CFP_ACT_CPU_COPY_CANCEL, INDEX(CPU_COPYf), -1},
    {DRV_CFP_ACT_COSQ_CPU_NEW, INDEX(COS2CPUf), -1},
    {DRV_CFP_ACT_RATE_VIOLATE_DROP, INDEX(DROP_ON_RVf), REV_A0},
    {DRV_CFP_ACT_RATE_VIOLATE_DROP, INDEX(DROP_ON_YELLOWf), REV_B0},
    {DRV_CFP_ACT_RATE_VIOLATE_DROP_CANCEL, INDEX(DROP_ON_RVf), REV_A0},
    {DRV_CFP_ACT_RATE_VIOLATE_DROP_CANCEL, INDEX(DROP_ON_YELLOWf), REV_B0},
    {DRV_CFP_ACT_REDIRECT_MGID, INDEX(DEST_IDf), -1},
    {DRV_CFP_ACT_REDIRECT_VPORT_PORT, INDEX(DEST_IDf), -1},
    {DRV_CFP_ACT_DROP, INDEX(DEST_IDf), -1},
    {DRV_CFP_ACT_LOOPBACK, INDEX(DEST_IDf), -1},
    {DRV_CFP_ACT_CHANGE_FWD, INDEX(CHANGE_FWDf), -1},
    {DRV_CFP_ACT_CHANGE_FWD_CANCEL, INDEX(CHANGE_FWDf), -1},
    {DRV_CFP_ACT_OB_CHANGE_DP, INDEX(CHANGE_DP_OBf), REV_A0},
    {DRV_CFP_ACT_OB_CHANGE_DP, INDEX(CHANGE_DP_YELLOWf), REV_B0},
    {DRV_CFP_ACT_OB_CHANGE_DP_CANCEL, INDEX(CHANGE_DP_OBf), REV_A0},
    {DRV_CFP_ACT_OB_CHANGE_DP_CANCEL, INDEX(CHANGE_DP_YELLOWf), REV_B0},
    {DRV_CFP_ACT_OB_NEW_DP, INDEX(NEW_DP_OBf), REV_A0},
    {DRV_CFP_ACT_OB_NEW_DP, INDEX(NEW_DP_YELLOWf), REV_B0},
    {DRV_CFP_ACT_IB_CHANGE_DP, INDEX(CHANGE_DP_IBf), REV_A0},
    {DRV_CFP_ACT_IB_CHANGE_DP, INDEX(CHANGE_DP_GREENf), REV_B0},
    {DRV_CFP_ACT_IB_CHANGE_DP_CANCEL, INDEX(CHANGE_DP_IBf), REV_A0},
    {DRV_CFP_ACT_IB_CHANGE_DP_CANCEL, INDEX(CHANGE_DP_GREENf), REV_B0},
    {DRV_CFP_ACT_IB_NEW_DP, INDEX(NEW_DP_IBf), REV_A0},
    {DRV_CFP_ACT_IB_NEW_DP, INDEX(NEW_DP_GREENf), REV_B0},
    {DRV_CFP_ACT_CHANGE_TC,INDEX(CHANGE_TCf), -1},
    {DRV_CFP_ACT_CHANGE_TC_CANCEL,INDEX(CHANGE_TCf), -1},
    {DRV_CFP_ACT_NEW_TC, INDEX(NEW_TCf), -1},
    {DRV_CFP_ACT_CHANGE_TC_CANCEL, INDEX(CHANGE_TCf), -1},
    {DRV_CFP_ACT_NEW_VLAN, INDEX(NEW_VIDf), -1},
    {DRV_CFP_ACT_CHAIN_ID, INDEX(FLOW_OR_CHAIN_SELf), -1},
    {DRV_CFP_ACT_CHAIN_ID_CANCEL, INDEX(FLOW_OR_CHAIN_SELf), -1},
    {DRV_CFP_ACT_CLASSFICATION_ID, INDEX(FLOW_OR_CHAIN_IDf), -1}
};

/*
 * Function: _drv_tbx_cfp_field_mapping
 *
 * Purpose:
 *     Translate the driver field type to chip field index.
 *
 * Parameters:
 *     unit - BCM device number
 *     field_type - driver field value
 *     field_id (OUT) - chip field index
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_BADID - unknown driver field value
 */
int
_drv_tbx_cfp_field_mapping(int unit, uint32 mem_type, uint32 field_type, uint32 *field_id)
{
    int rv = SOC_E_NONE;
    int i, rev;

    if (SOC_IS_TB_AX(unit)){
        rev = REV_A0;
    } else {
        rev = REV_B0;
    }
    
    if (mem_type ==  DRV_CFP_RAM_ACT) {
        for(i = 0; i < COUNTOF(drv_cfp_action_field_map_table) ; i++) {           
            if (drv_cfp_action_field_map_table[i].cfp_field_type == field_type) {
                if ((drv_cfp_action_field_map_table[i].rev_id == -1) ||
                    (drv_cfp_action_field_map_table[i].rev_id == rev)){
                    *field_id = drv_cfp_action_field_map_table[i].field_id;
                    break;
                }
            }
        }
    } else {
        for(i = 0; i < COUNTOF(drv_cfp_field_map_table) ; i++) {
            if (drv_cfp_field_map_table[i].cfp_field_type == field_type) {
                if ((drv_cfp_field_map_table[i].rev_id == -1) ||
                    (drv_cfp_field_map_table[i].rev_id == rev)){       
                    *field_id = drv_cfp_field_map_table[i].field_id;
                    break;
                }
            }
        }
    }
    return rv;
}

#ifdef BCM_TB_SUPPORT
int
_drv_tb_cfp_chain_slice_id_mapping(int unit,uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* slice_id)
{
    uint32 id, map;


    id =  entry->slice_id;
    map = 1 << id;

    if (map & CFP_53280_SLICE_MAP_CHAIN) {
        if (entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN){
            id = CFP_53280_SLICE_ID_CHAIN_SLICE;
        } else {
            id = id - 3;
        }
    }
    *slice_id = id;
    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s slice_id %d entry->flags %x\n"),
               FUNCTION_NAME(),id, entry->flags));    
    return SOC_E_NONE;
}

#endif /* BCM_TB_SUPPORT */

/*
 * Function: _drv_cfp_read
 *
 * Purpose:
 *     Read the CFP raw data by ram type from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     ram_type - ram type (TCAM/METER/ACT/POLICY)
 *     index -entry index
 *     entry(OUT) -CFP entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv_tbx_cfp_read(int unit, uint32 ram_type, 
                         uint32 index, drv_cfp_entry_t *cfp_entry)
{
    int rv = SOC_E_NONE;
    uint32 *mem_entry;
    int size, mem_len;

    assert(cfp_entry);

    switch (ram_type) {
        case DRV_MEM_CFP_ACT:
            mem_len = soc_mem_entry_words(unit, INDEX(CFP_ACT_POLm));
            break;
        case DRV_MEM_CFP_METER:
            if (SOC_IS_TB_AX(unit)) {
                mem_len = soc_mem_entry_words(unit, INDEX(CFP_METERm));
            } else {
                mem_len = soc_mem_entry_words(unit, INDEX(CFP_RATE_METERm));
            }
            break;
        case DRV_MEM_CFP_DATA_MASK:
            mem_len = soc_mem_entry_words(unit, INDEX(CFP_DATA_MASKm));
            break;      
        default:
            rv = SOC_E_PARAM;
            return rv;
    }
    size = mem_len * sizeof(uint32);

    mem_entry = (uint32 *) sal_alloc(size, "cfp mem entry");
    if (mem_entry == NULL) {
        return SOC_E_RESOURCE;
    }

    sal_memset(mem_entry, 0, size);

    rv = DRV_MEM_READ(unit, ram_type, index, 1, mem_entry);

    if (SOC_FAILURE(rv)) {
        sal_free(mem_entry);
        return rv;
    }

    switch (ram_type) {
        case DRV_MEM_CFP_ACT:
            if (cfp_entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_MAIN) {
                sal_memcpy(cfp_entry->cfp_chain->act_data, mem_entry,size);
            } else {                
                sal_memcpy(cfp_entry->act_data, mem_entry,size);
            }
            break;
        case DRV_MEM_CFP_METER:
            sal_memcpy(cfp_entry->meter_data, mem_entry, size);
            break;
        case DRV_MEM_CFP_DATA_MASK:
            if (cfp_entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_MAIN) {
                sal_memcpy(cfp_entry->cfp_chain->tcam_data, mem_entry, 8*sizeof(uint32));
                sal_memcpy(cfp_entry->cfp_chain->tcam_mask, &mem_entry[8], 8*sizeof(uint32));
            } else {
                sal_memcpy(cfp_entry->tcam_data, mem_entry, 8*sizeof(uint32));
                sal_memcpy(cfp_entry->tcam_mask, &mem_entry[8], 8*sizeof(uint32));
            }
            break;
    }

    sal_free(mem_entry);
    return rv;
}

/*
 * Function: _drv_cfp_write
 *
 * Purpose:
 *     Write the CFP raw data by ram type to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     ram_type - ram type (TCAM/METER/ACT/POLICY)
 *     index -entry index
 *     entry -CFP entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv_tbx_cfp_write(int unit, uint32 ram_type, 
                              uint32 index, drv_cfp_entry_t *cfp_entry)
{
    int rv = SOC_E_NONE;
    uint32 *mem_entry;
    int size, mem_len;

    assert(cfp_entry);

    switch (ram_type) {
        case DRV_MEM_CFP_ACT:
            mem_len = soc_mem_entry_words(unit, INDEX(CFP_ACT_POLm));
            break;
        case DRV_MEM_CFP_METER:
            if (SOC_IS_TB_AX(unit)) {
                mem_len = soc_mem_entry_words(unit, INDEX(CFP_METERm));
            } else {
                mem_len = soc_mem_entry_words(unit, INDEX(CFP_RATE_METERm));
            }
            break;
#ifdef BCM_TB_SUPPORT            
        case DRV_MEM_CFP_DATA_MASK:
            mem_len = soc_mem_entry_words(unit, INDEX(CFP_DATA_MASKm));
            break;   
#endif /* BCM_TB_SUPPORT */            
        default:
            rv = SOC_E_PARAM;
            return rv;
    }
    size = mem_len * sizeof(uint32);
    mem_entry = (uint32 *) sal_alloc(size, "cfp mem entry");
    if (mem_entry == NULL) {
        return SOC_E_RESOURCE;
    }
    sal_memset(mem_entry, 0, size);

    switch (ram_type) {
        case DRV_MEM_CFP_ACT:
            if (cfp_entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_MAIN) {
                sal_memcpy(mem_entry, cfp_entry->cfp_chain->act_data, size);
            } else {                
                sal_memcpy(mem_entry, cfp_entry->act_data, size);
            }
            break;
        case DRV_MEM_CFP_METER:
            sal_memcpy(mem_entry, cfp_entry->meter_data, size);
            break;
#ifdef BCM_TB_SUPPORT            
        case DRV_MEM_CFP_DATA_MASK:
            if (cfp_entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_MAIN) {
                sal_memcpy(mem_entry, cfp_entry->cfp_chain->tcam_data, 8*sizeof(uint32));
                sal_memcpy(&mem_entry[8], cfp_entry->cfp_chain->tcam_mask,  8*sizeof(uint32));
            } else {
                sal_memcpy(mem_entry, cfp_entry->tcam_data, 8*sizeof(uint32));
                sal_memcpy(&mem_entry[8], cfp_entry->tcam_mask,  8*sizeof(uint32));
            }
            break;
#endif /* BCM_TB_SUPPORT */            
    }
    rv = DRV_MEM_WRITE(unit, ram_type, index, 1, mem_entry);
    if (SOC_FAILURE(rv)) {
        sal_free(mem_entry);
        return rv;
    }
    sal_free(mem_entry);
    return rv;
}

/*
 * Function: drv_tbx_cfp_init
 *
 * Purpose:
 *     Initialize the CFP module. 
 *
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 */
int 
drv_tbx_cfp_init(int unit)
{
    int port, i, slice_id;
    pbmp_t pbm;
    uint32  slice[(DRV_CFP_QUAL_COUNT / 32) + 1];
    int     *qset;
    int slice_udf_free;

    /* Enable CFP */
    pbm = PBMP_E_ALL(unit);
    _SHR_PBMP_PORT_REMOVE(pbm, CMIC_PORT(unit));
    PBMP_ITER(pbm, port) {
        DRV_CFP_CONTROL_SET(unit, DRV_CFP_ENABLE, port, 1);
    }
    /* Clear TCAM Table */
    DRV_CFP_CONTROL_SET(unit, DRV_CFP_TCAM_RESET, 0, 0);
    
    /* Clear Other RAM Table */
    DRV_CFP_CONTROL_SET(unit, DRV_CFP_OTHER_RAM_CLEAR, 0, 0);

    /* Slice information initialization */
    sal_memset(&drv53280_slice_info[0], 0,
        sizeof(drv_slice_info_t) * (CFP_53280_SLICE_MAX_ID + 1));
   
    for (slice_id = 0; slice_id <= CFP_53280_SLICE_MAX_ID; slice_id++) {
        switch(slice_id) {
            case 0:
                qset = s0_qset;
                slice_udf_free =5;
                break;
            case 1:
                qset = s1_qset;
                slice_udf_free = 2;
                break;
            case 2:
                qset = s2_qset;
                slice_udf_free = 9;
                break;
            case 3:
                qset = s3_qset;
                slice_udf_free = 17;
                break;
            case 4:
                qset = s4_qset;
                slice_udf_free = 1;
                break;
            case 5:
                qset = s5_qset;
                slice_udf_free = 0;
                break;
            case 6:
                qset = s6_qset;
                slice_udf_free = 9;
                break;
            case 7:
                qset = s7_qset;
                slice_udf_free = 13;
                break;
            case 11:
                qset = s11_qset;
                slice_udf_free = 12;
                break;
            case 12:
                qset = s12_qset;
                slice_udf_free = 3;
                break;
            case 13:
                qset = s13_qset;
                slice_udf_free = 3;
                break;
            case 14:
                qset = s14_qset;
                slice_udf_free = 3;
                break;
            case 15:
                qset = s15_qset;
                slice_udf_free = 15;
                break;
            default:
                continue;
        }
        for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
            slice[i] = 0;
        }
        i = 0;
        while (qset[i] != DRV_CFP_QUAL_INVALID) {
            slice[(qset[i]/32)] |= (0x1 << (qset[i] % 32));
            i++;
        }
        for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
            drv53280_slice_info[slice_id].qset[i] = slice[i];
        }

        drv53280_slice_info[slice_id].slice_id = slice_id;
        drv53280_slice_info[slice_id].slice_udf_free = slice_udf_free;
        
    }
    
    return SOC_E_NONE;
}

/*
 * Function: drv_tbx_cfp_action_get
 *
 * Purpose:
 *     Get the CFP action type and parameters value from 
 *     the raw data of ACTION/POLICY ram.
 *
 * Parameters:
 *     unit - BCM device number
 *     action(IN/OUT) - driver action type
 *     entry -cfp entry
 *     act_param(OUT) - action paramter (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown action type
 *
 */
int
drv_tbx_cfp_action_get(int unit, uint32* action, 
            drv_cfp_entry_t* entry, uint32* act_param)
{
    int rv = SOC_E_NONE;
    uint32  fld_val;

    rv = DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, *action, entry, &fld_val);
    SOC_IF_ERROR_RETURN(rv);
    
    switch(*action) {
    case DRV_CFP_ACT_FILTER_ALL:
        *act_param = fld_val & CFP_53280_BYPASS_ALL;
        break;
    case DRV_CFP_ACT_FILTER_SA:
        if (fld_val & CFP_53280_BYPASS_SA) {
            *act_param = FALSE;
        } else {
            *act_param = TRUE;
        }
        break;
    case DRV_CFP_ACT_FILTER_EGRESS_VLAN:
        if (fld_val & CFP_53280_BYPASS_VEF) {
            *act_param = FALSE;
        } else {
            *act_param = TRUE;
        }
        break;
    case DRV_CFP_ACT_FILTER_INGRESS_VLAN:
        if (fld_val & CFP_53280_BYPASS_VIF) {
            *act_param = FALSE;
        } else {
            *act_param = TRUE;
        }
        break;
    case DRV_CFP_ACT_FILTER_EAP:
        if (fld_val & CFP_53280_BYPASS_EAP) {
            *act_param = FALSE;
        } else {
            *act_param = TRUE;
        }
        break;
    case DRV_CFP_ACT_FILTER_STP:
        if (fld_val & CFP_53280_BYPASS_STP) {
            *act_param = FALSE;
        } else {
            *act_param = TRUE;
        }
        break;
    case DRV_CFP_ACT_FILTER_PORT_MASK:
        if (fld_val & CFP_53280_BYPASS_PMF) {
            *act_param = FALSE;
        } else {
            *act_param = TRUE;
        }
        break;
    case DRV_CFP_ACT_FILTER_TAGGED:
        if (fld_val & CFP_53280_BYPASS_TGF) {
            *act_param = FALSE;
        } else {
            *act_param = TRUE;
        }
        break;
    case DRV_CFP_ACT_FILTER_LAG:
        if (fld_val & CFP_53280_BYPASS_LAG) {
            *act_param = FALSE;
        } else {
            *act_param = TRUE;
        }
        break;
    case DRV_CFP_ACT_RATE_VIOLATE_DROP:
    case DRV_CFP_ACT_CPU_COPY:
    case DRV_CFP_ACT_DA_KNOWN:
    case DRV_CFP_ACT_DIS_LRN:
    case DRV_CFP_ACT_CHANGE_FWD:
    case DRV_CFP_ACT_OB_CHANGE_DP:
    case DRV_CFP_ACT_IB_CHANGE_DP:
    case DRV_CFP_ACT_CHANGE_TC:
    case DRV_CFP_ACT_CHAIN_ID:
    case DRV_CFP_ACT_MIRROR_COPY:        
        if ( fld_val ) {
            *act_param = TRUE;
        } else {
            *act_param = FALSE;
        }
        break;
    case DRV_CFP_ACT_CHANGE_FWD_CANCEL:            
    case DRV_CFP_ACT_RATE_VIOLATE_DROP_CANCEL:
    case DRV_CFP_ACT_CPU_COPY_CANCEL:
    case DRV_CFP_ACT_DA_KNOWN_CANCEL:            
    case DRV_CFP_ACT_DIS_LRN_CANCEL:
    case DRV_CFP_ACT_OB_CHANGE_DP_CANCEL:
    case DRV_CFP_ACT_IB_CHANGE_DP_CANCEL:
    case DRV_CFP_ACT_CHANGE_TC_CANCEL:
    case DRV_CFP_ACT_MIRROR_COPY_CANCEL:
        if ( fld_val ) {
            *act_param = FALSE;
        } else {
            *act_param = TRUE;
        }
        break;

    case DRV_CFP_ACT_COSQ_CPU_NEW:
    case DRV_CFP_ACT_NEW_VLAN:
    case DRV_CFP_ACT_CLASSFICATION_ID:
        *act_param = fld_val;
        break;
    case DRV_CFP_ACT_REDIRECT_MGID:
    case DRV_CFP_ACT_REDIRECT_VPORT_PORT:
    case DRV_CFP_ACT_DROP:        
    case DRV_CFP_ACT_LOOPBACK:
        if (fld_val == 0x801) {
            *action = DRV_CFP_ACT_LOOPBACK;
        } else if (fld_val == 0x800) {
            *action = DRV_CFP_ACT_DROP;
        } else if (fld_val & 0x1000) {
            *action = DRV_CFP_ACT_REDIRECT_MGID;
        } else {
            *action = DRV_CFP_ACT_REDIRECT_VPORT_PORT;
        }
        *act_param  = fld_val;
        break;
    case DRV_CFP_ACT_OB_NEW_DP:
    case DRV_CFP_ACT_IB_NEW_DP:
    case DRV_CFP_ACT_NEW_TC:
        *act_param  = fld_val;
        break;
    }
    return rv;
}

/*
 * Function: drv_tbx_cfp_action_set
 *
 * Purpose:
 *     Set the CFP action type and parameters value to 
 *     the raw data of ACTION/POLICY ram.
 *
 * Parameters:
 *     unit - BCM device number
 *     action - driver action type
 *     entry(OUT) -cfp entry
 *     act_param - action paramter (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown action type
 *
 * Note:
 *     action                              act_param
 *
 */
int
drv_tbx_cfp_action_set(int unit, uint32 action, 
            drv_cfp_entry_t* entry, uint32 act_param1, uint32 act_param2)
{
    int rv = SOC_E_NONE;
    int fld_value;

    assert(entry);
        
    fld_value = 0;

    if ((action == DRV_CFP_ACT_FILTER_LAG) || (action == DRV_CFP_ACT_FILTER_TAGGED) ||
        (action == DRV_CFP_ACT_FILTER_PORT_MASK) || (action == DRV_CFP_ACT_FILTER_STP) ||
        (action == DRV_CFP_ACT_FILTER_EAP) || (action == DRV_CFP_ACT_FILTER_INGRESS_VLAN) ||
        (action == DRV_CFP_ACT_FILTER_EGRESS_VLAN) ||(action == DRV_CFP_ACT_FILTER_SA)){
    
        rv = DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, DRV_CFP_ACT_FILTER_ALL, 
            entry, (uint32 *)&fld_value);
        SOC_IF_ERROR_RETURN(rv);

    }

    switch (action) {
        /* DRV_CFP_ACT_FILTER_XXX actions, 
        *   act_param1 = 1 => apply the Filter, do not bypass the filter. 
        */
        case DRV_CFP_ACT_FILTER_SA:
            if (act_param1) {
                fld_value &= ~(CFP_53280_BYPASS_SA);
             } else {
                fld_value |= (CFP_53280_BYPASS_SA);
            }
            break;
        case DRV_CFP_ACT_FILTER_EGRESS_VLAN:
            if (act_param1) {
                fld_value &= ~(CFP_53280_BYPASS_VEF);
             } else {
                fld_value |= (CFP_53280_BYPASS_VEF);
            }
            break;
        case DRV_CFP_ACT_FILTER_INGRESS_VLAN:
            if (act_param1) {
                fld_value &= ~(CFP_53280_BYPASS_VIF);
             } else {
                fld_value |= (CFP_53280_BYPASS_VIF);
            }
            break;
        case DRV_CFP_ACT_FILTER_EAP:
            if (act_param1) {
                fld_value &= ~(CFP_53280_BYPASS_EAP);
             } else {
                fld_value |= (CFP_53280_BYPASS_EAP);
            }
            break;
        case DRV_CFP_ACT_FILTER_STP:
            if (act_param1) {
                fld_value &= ~(CFP_53280_BYPASS_STP);
             } else {
                fld_value |= (CFP_53280_BYPASS_STP);
            }
            break;
        case DRV_CFP_ACT_FILTER_PORT_MASK:
            if (act_param1) {
                fld_value &= ~(CFP_53280_BYPASS_PMF);
             } else {
                fld_value |= (CFP_53280_BYPASS_PMF);
            }
            break;
        case DRV_CFP_ACT_FILTER_TAGGED:
            if (act_param1) {
                fld_value &= ~(CFP_53280_BYPASS_TGF);
             } else {
                fld_value |= (CFP_53280_BYPASS_TGF);
            }
            break;
        case DRV_CFP_ACT_FILTER_LAG:
            if (act_param1) {
                fld_value &= ~(CFP_53280_BYPASS_LAG);
             } else {
                fld_value |= (CFP_53280_BYPASS_LAG);
            }
            break;            
        case DRV_CFP_ACT_FILTER_ALL:
            if (act_param1) {
                fld_value = ~(CFP_53280_BYPASS_ALL);
             } else {
                fld_value = (CFP_53280_BYPASS_ALL);
            }
            break;    
        case DRV_CFP_ACT_RATE_VIOLATE_DROP:
        case DRV_CFP_ACT_CPU_COPY:
        case DRV_CFP_ACT_DA_KNOWN:
        case DRV_CFP_ACT_DIS_LRN:
        case DRV_CFP_ACT_CHANGE_FWD:
        case DRV_CFP_ACT_OB_CHANGE_DP:
        case DRV_CFP_ACT_IB_CHANGE_DP:
        case DRV_CFP_ACT_CHANGE_TC:
        case DRV_CFP_ACT_CHAIN_ID:
        case DRV_CFP_ACT_MIRROR_COPY:
            fld_value = 1;
            break;
        case DRV_CFP_ACT_CHANGE_FWD_CANCEL:            
        case DRV_CFP_ACT_RATE_VIOLATE_DROP_CANCEL:
        case DRV_CFP_ACT_CPU_COPY_CANCEL:
        case DRV_CFP_ACT_DA_KNOWN_CANCEL:            
        case DRV_CFP_ACT_DIS_LRN_CANCEL:
        case DRV_CFP_ACT_OB_CHANGE_DP_CANCEL:
        case DRV_CFP_ACT_IB_CHANGE_DP_CANCEL:
        case DRV_CFP_ACT_CHANGE_TC_CANCEL:
        case DRV_CFP_ACT_MIRROR_COPY_CANCEL:
            fld_value = 0;
            break;
        case DRV_CFP_ACT_COSQ_CPU_NEW:
        case DRV_CFP_ACT_NEW_VLAN:
        case DRV_CFP_ACT_CLASSFICATION_ID:
            fld_value = act_param1;
            break;
        case DRV_CFP_ACT_REDIRECT_MGID:
            fld_value = 0x1000;
            fld_value |= act_param1;
            break;
        case DRV_CFP_ACT_REDIRECT_VPORT_PORT:
            fld_value = (act_param1 << 6) | (act_param2);
            fld_value &= 0x7ff;
            break;
        case DRV_CFP_ACT_DROP:
            fld_value = 0x800;
            break;
        case DRV_CFP_ACT_LOOPBACK:
            fld_value = 0x801;
            break;
        case DRV_CFP_ACT_OB_NEW_DP:
        case DRV_CFP_ACT_IB_NEW_DP:
        case DRV_CFP_ACT_NEW_TC:
            fld_value = act_param1;
            break;
    }

    rv = DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, action, entry, 
        (uint32 *)&fld_value);
    return rv;
}

/*
 * Function: drv_tbx_cfp_control_get
 *
 * Purpose:
 *     Get the CFP control paramters.
 *
 * Parameters:
 *     unit - BCM device number
 *     control_type - CFP control type
 *     param1 -control paramter 1 (if need)
 *     param2(OUT) -control parameter 2 (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     control_type     param1     param2
 *     DRV_CFP_ENABLE, port number, enable
 *     DRV_CFP_FLOOD_TRUNK, XXX, enable
 *     DRV_CFP_FLOOD_VLAN, XXX, enable
 *     DRV_CFP_ISPVLAN_DELIMITER, XXX, ISP delimter
 *     DRV_CFP_LLC_ENCAP, XXX, DSAP+SSAP+Control field
 *     DRV_CFP_SLICE_SELECT, port number, slice id map
 *     DRV_CFP_TCAM_RESET, XXX, XXX
 *     
 */
int
drv_tbx_cfp_control_get(int unit, uint32 control_type, uint32 param1, 
            uint32 *param2)
{
    int rv = SOC_E_NONE;
    uint32  fld_32, reg_val;

    switch (control_type) {
        case DRV_CFP_ENABLE:
            rv = REG_READ_CFP_CTL_REGr(unit, &reg_val);
            SOC_IF_ERROR_RETURN(rv);

            rv = soc_CFP_CTL_REGr_field_get(unit, &reg_val, 
                CFP_EN_MAPf, (uint32 *)&fld_32);
            SOC_IF_ERROR_RETURN(rv);
            
            if ((fld_32 & (0x1 << param1))  != 0) {
                *param2 = TRUE;
            } else {
                *param2 = FALSE;
            }
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }

    return rv;
}

/*
 * Function: drv_tbx_cfp_control_set
 *
 * Purpose:
 *     Set the CFP control paramters.
 *
 * Parameters:
 *     unit - BCM device number
 *     control_type - CFP control type
 *     param1 -control paramter 1 (if need)
 *     param2 -control parameter 2 (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     control_type     param1     param2
 *     DRV_CFP_ENABLE, port number, enable
 *     DRV_CFP_FLOOD_TRUNK, XXX, enable
 *     DRV_CFP_FLOOD_VLAN, XXX, enable
 *     DRV_CFP_ISPVLAN_DELIMITER, XXX, ISP demimeter
 *     DRV_CFP_LLC_ENCAP, XXX, DSAP+SSAP+Control field
 *     DRV_CFP_SLICE_SELECT, port number, slice id map
 *     DRV_CFP_TCAM_RESET, XXX, XXX
 *     
 */
int
drv_tbx_cfp_control_set(int unit, uint32 control_type, uint32 param1, 
            uint32 param2)
{
    int rv = SOC_E_NONE;
    uint32  fld_32, reg_val;
    uint32 temp;

    switch (control_type) {
        case DRV_CFP_ENABLE:
            /* Read CFP control register */
            rv = REG_READ_CFP_EN_CTRLr(unit, &reg_val);
            SOC_IF_ERROR_RETURN(rv);

            rv = soc_CFP_EN_CTRLr_field_get(unit, &reg_val, CFP_EN_CTRLf, 
                (uint32 *)&fld_32);
            SOC_IF_ERROR_RETURN(rv);

            /* Set CFP ENABLE PORT bitmap */
            temp = 0x1 << param1;
             if (param2) {
                fld_32 |= temp;
            } else {
                fld_32 &= ~temp;
            }
            rv = soc_CFP_EN_CTRLr_field_set(unit, &reg_val, CFP_EN_CTRLf, 
                (uint32 *)&fld_32);
            SOC_IF_ERROR_RETURN(rv);
            
            /* Write CFP control register */
            rv = REG_WRITE_CFP_EN_CTRLr(unit, &reg_val);
            break;
            
        case DRV_CFP_TCAM_RESET:
            MEM_LOCK(unit, INDEX(CFP_TCAM_IPV4_S0m));
            DRV_MEM_CLEAR(unit, DRV_MEM_CFP_DATA_MASK);
            MEM_UNLOCK(unit, INDEX(CFP_TCAM_IPV4_S0m));
            break;
            
        case DRV_CFP_OTHER_RAM_CLEAR:
            MEM_LOCK(unit, INDEX(CFP_TCAM_IPV4_S0m));
            DRV_MEM_CLEAR(unit, DRV_MEM_CFP_ACT);
            DRV_MEM_CLEAR(unit, DRV_MEM_CFP_METER);            
            DRV_MEM_CLEAR(unit, DRV_MEM_CFP_STAT);            
            MEM_UNLOCK(unit, INDEX(CFP_TCAM_IPV4_S0m));
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }

    return rv;
}


/*
 * Function: drv_tbx_cfp_entry_read
 *
 * Purpose:
 *     Read the TCAM/ACTION/POLICY/METER raw data from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     index - CFP entry index
 *     ram_type -TCAM, ACTION/POLICT and METER
 *     entry(OUT) - chip entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown ram type
 *
 */
int
drv_tbx_cfp_entry_read(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry)
{
    int rv = SOC_E_NONE;

    switch (ram_type) {
        case DRV_CFP_RAM_ALL:
            rv = SOC_E_UNAVAIL;
#ifdef BCM_VO_SUPPORT
            if (SOC_IS_VO(unit)) {
                rv = _drv_vo_cfp_data_mask_read(unit, DRV_MEM_CFP_DATA_MASK, 
                    index, entry);
            }
#endif /* BCM_VO_SUPPORT */
#ifdef BCM_TB_SUPPORT
            if (SOC_IS_TB(unit)) {
                rv = _drv_tbx_cfp_read(unit, DRV_MEM_CFP_DATA_MASK, 
                    index, entry);
            }
#endif /* BCM_TB_SUPPORT */           
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : \
                                      failed to read TCAM with index = 0x%x, rv = %d. \n"), 
                           index, rv));
                return rv;
            }
            rv = _drv_tbx_cfp_read(unit, DRV_MEM_CFP_ACT, index, entry);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : \
                                      failed to read action ram with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            rv = _drv_tbx_cfp_read(unit, DRV_MEM_CFP_METER, index, entry);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : \
                                      failed to read meter ram with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_TCAM:
            rv = SOC_E_UNAVAIL;
#ifdef BCM_VO_SUPPORT
            if (SOC_IS_VO(unit)) {
                rv = _drv_vo_cfp_data_mask_read(unit, DRV_MEM_CFP_DATA_MASK, 
                    index, entry);
            }
#endif /* BCM_VO_SUPPORT */
#ifdef BCM_TB_SUPPORT
            if (SOC_IS_TB(unit)) {
                rv = _drv_tbx_cfp_read(unit, DRV_MEM_CFP_DATA_MASK,
                    index, entry);
            }
#endif /* BCM_TB_SUPPORT */            
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : \
                                      failed to read TCAM with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_ACT:
            rv = _drv_tbx_cfp_read(unit, DRV_MEM_CFP_ACT, index, entry);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : \
                                      failed to read action ram with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_METER:
            rv = _drv_tbx_cfp_read(unit, DRV_MEM_CFP_METER, index, entry);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : \
                                      failed to read action ram with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv_tbx_cfp_entry_write
 *
 * Purpose:
 *     Write the TCAM/ACTION/POLICY/METER raw data to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     index - CFP entry index
 *     ram_type -TCAM, ACTION/POLICT and METER
 *     entry - chip entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown ram type
 *
 * Note:
 *     
 */
int
drv_tbx_cfp_entry_write(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry)
{
    int rv;

    switch (ram_type) {
        case DRV_CFP_RAM_ALL:
            rv = SOC_E_UNAVAIL;
#ifdef BCM_VO_SUPPORT
            if (SOC_IS_VO(unit)) {
                rv = _drv_vo_cfp_data_mask_write(unit, DRV_MEM_CFP_DATA_MASK,
                    index, entry);
            } 
#endif /* BCM_VO_SUPPORT */
#ifdef BCM_TB_SUPPORT
            if (SOC_IS_TB(unit)) {
                rv = _drv_tbx_cfp_write(unit, DRV_MEM_CFP_DATA_MASK, 
                    index, entry);
            }
#endif /* BCM_TB_SUPPORT */            
            if (SOC_FAILURE(rv)){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write : failed to write TCAM with index \
                                      = 0x%x, rv = %d. \n"), index, rv));
                return rv;
            }
            rv = _drv_tbx_cfp_write(unit, DRV_MEM_CFP_ACT, index, entry);
            if (SOC_FAILURE(rv)){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write:failed to write action ram with index \
                                      = 0x%x, rv = %d. \n"), index, rv));
                return rv;
            }
            rv = _drv_tbx_cfp_write(unit, DRV_MEM_CFP_METER, index, entry);
            if (SOC_FAILURE(rv)){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write :failed to write meter ram with index \
                                      = 0x%x, rv = %d. \n"), index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_TCAM:
            rv = SOC_E_UNAVAIL;
#ifdef BCM_VO_SUPPORT
            if (SOC_IS_VO(unit)) {
                rv = _drv_vo_cfp_data_mask_write(unit, DRV_MEM_CFP_DATA_MASK, index, entry);
            } 
#endif
#ifdef BCM_TB_SUPPORT
            if (SOC_IS_TB(unit)) {
                rv = _drv_tbx_cfp_write(unit, DRV_MEM_CFP_DATA_MASK, index, entry);
            }
#endif /* BCM_TB_SUPPORT */            
            if (SOC_FAILURE(rv)){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write : failed to write TCAM with index \
                                      = 0x%x, rv = %d. \n"), index, rv));
                return rv;
            }
            entry->id = index; 
            break;
        case DRV_CFP_RAM_ACT:
            rv = _drv_tbx_cfp_write(unit, DRV_MEM_CFP_ACT, index, entry);
            if (SOC_FAILURE(rv)){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write : failed to write ACT ram with index \
                                      = 0x%x, rv = %d. \n"), index, rv));
                return rv;
            }
            entry->id = index; 
            break;
        case DRV_CFP_RAM_METER:
            rv = _drv_tbx_cfp_write(unit, DRV_MEM_CFP_METER, index, entry);
            if (SOC_FAILURE(rv)){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write :failed to write METER ram with index \
                                      = 0x%x, rv = %d. \n"), index, rv));
                return rv;
            }
            entry->id = index; 
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv_tbx_cfp_field_get
 *
 * Purpose:
 *     Get the field value from the CFP entry raw data.
 *
 * Parameters:
 *     unit - BCM device number
 *     mem_type - driver ram type (TCAM/Meter/Act/Policy)
 *     field_type -driver CFP field type
 *     entry -cfp entry
 *     fld_val(OUT) - field value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv_tbx_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val)
{
    int rv = SOC_E_NONE;
    int     mem_id;
    uint32  fld_id=0, slice_id;
    uint32  mask = -1;
    uint32  mask_hi, mask_lo;
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    int i, wp, bp, len;
    uint32  *data_p=NULL;

    assert(entry);
    assert(fld_val);
    mem_id = -1;
    switch (mem_type) {
    case DRV_CFP_RAM_TCAM:
    case DRV_CFP_RAM_TCAM_MASK:
#ifdef BCM_VO_SUPPORT
        if (SOC_IS_VO(unit)) {
            return _drv_vo_cfp_field_get(unit, mem_type, field_type, entry, 
                fld_val);
        }
#endif /* BCM_VO_SUPPORT */
#ifdef BCM_TB_SUPPORT
        if (SOC_IS_TB(unit)) {
            rv = _drv_tb_cfp_chain_slice_id_mapping(unit, field_type, entry, &slice_id);
            SOC_IF_ERROR_RETURN(rv);
            switch (slice_id) {
                case 0:
                    mem_id = INDEX(CFP_TCAM_IPV4_S0m);
                    break;
                case 1:                    
                    mem_id = INDEX(CFP_TCAM_IPV4_S1m);
                    break;
                case 2:                    
                    mem_id = INDEX(CFP_TCAM_IPV4_S2m);
                    break;            
                case 4:
                    mem_id = INDEX(CFP_TCAM_IPV6_S0m);
                    break;
                case 5:
                    mem_id = INDEX(CFP_TCAM_IPV6_S1m);
                    break;
                case 6:   
                    mem_id = INDEX(CFP_TCAM_IPV6_S2m);
                    break;                
                case 11:
                    mem_id = INDEX(CFP_TCAM_CHAIN_SCm);
                    break;                                    
                case 12:
                case 13:
                case 14:                    
                    mem_id = INDEX(CFP_TCAM_NONIP_MASKm);
                    break;                    
                default:
                    break;
            }
 
            if (mem_id == -1) {
                return SOC_E_PARAM;
            }
            if (entry->flags & _DRV_CFP_SLICE_CHAIN){
                if (entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN){
                    if (mem_type == DRV_CFP_RAM_TCAM) {
                        data_p = entry->tcam_data;
                    } else { /* mask data */
                        data_p = entry->tcam_mask;
                    }
                } else {
                    if (mem_type == DRV_CFP_RAM_TCAM) {
                        data_p = entry->cfp_chain->tcam_data;
                    } else { /* mask data */
                        data_p = entry->cfp_chain->tcam_mask;
                    }
                }
            } else {
                if (mem_type == DRV_CFP_RAM_TCAM) {
                    data_p = entry->tcam_data;
                } else { /* mask data */
                    data_p = entry->tcam_mask;
                }
            }
            break;
        }
#endif /* BCM_TB_SUPPORT */        
        rv = SOC_E_UNAVAIL;
        break;
    case DRV_CFP_RAM_ACT:
        mem_id = INDEX(CFP_ACT_POLm);
        if (entry->flags & _DRV_CFP_SLICE_CHAIN){
            if (entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_MAIN){
               data_p = entry->cfp_chain->act_data;
            } else {
               data_p = entry->act_data;
            }
        } else {
            data_p = entry->act_data;
        }
        break;
    case DRV_CFP_RAM_METER:
       if (SOC_IS_TB_AX(unit)) {
            mem_id = INDEX(CFP_METERm);
        } else {
            mem_id = INDEX(CFP_RATE_METERm);       
        }
        data_p = entry->meter_data;
        break;
    default:
        rv = SOC_E_PARAM;
        return rv;
    }
    SOC_IF_ERROR_RETURN(rv);
    if (( rv = _drv_tbx_cfp_field_mapping(unit, mem_type, field_type, &fld_id)) < 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "drv_cfp_field_get : UNKNOW FIELD ID. \n")));
        return rv;
    }

    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);
    SOC_FIND_FIELD(fld_id, meminfo->fields,
                             meminfo->nFields, fieldinfo);
    assert(fieldinfo);
    bp = fieldinfo->bp;

    wp = bp / 32;
    bp = bp & (32 - 1);
    len = fieldinfo->len;

    /* field is 1-bit wide */
    if (len == 1) {
        fld_val[0] = ((data_p[FIX_MEM_ORDER_E(wp, meminfo)] >> bp) & 1);
    } else {
        if (fieldinfo->flags & SOCF_LE) {
            for (i = 0; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
                if (len < 32) {
                    mask = (1 << len) - 1;
                } else {
                    mask = 0xffffffff;
                }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
                mask_lo = mask;
                mask_hi = 0;
                /* if field is not aligned with 32-bit word boundary */
                /* adjust hi and lo masks accordingly. */
                if (bp) {
                    mask_lo = mask << bp;
                    mask_hi = mask >> (32 - bp);
                }
                /* get field value --- 32 bits each time */
                fld_val[i] = (data_p[FIX_MEM_ORDER_E(wp++, meminfo)] 
                    & mask_lo) >> bp;
                if (mask_hi) {
                    fld_val[i] |= (data_p[FIX_MEM_ORDER_E(wp, meminfo)] 
                        & mask_hi) << (32 - bp);
                }
                i++;
            }
        } else {
            i = (len - 1) / 32;

            while (len > 0) {
                assert(i >= 0);
                fld_val[i] = 0;
                do {
                    fld_val[i] = (fld_val[i] << 1) |
                    ((data_p[FIX_MEM_ORDER_E(bp / 32, meminfo)] >>
                    (bp & (32 - 1))) & 1);
                    len--;
                    bp++;
                } while (len & (32 - 1));
                i--;
            }
        }
    }
    return rv;
}

/*
 * Function: drv_tbx_cfp_field_set
 *
 * Purpose:
 *     Set the field value to the CFP entry raw data.
 *
 * Parameters:
 *     unit - BCM device number
 *     mem_type - driver ram type (TCAM/Meter/Act/Policy)
 *     field_type -driver CFP field type
 *     entry(OUT) -cfp entry
 *     fld_val - field value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv_tbx_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val)
{
    int rv = SOC_E_NONE;
    int     mem_id;
    uint32  fld_id=0, slice_id;
    uint32  mask, mask_hi, mask_lo;
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    int i, wp, bp, len;
    uint32  *data_p;

    assert(entry);
    assert(fld_val);
    mem_id = -1;
    data_p = NULL;
    switch (mem_type) {
    case DRV_CFP_RAM_TCAM:
    case DRV_CFP_RAM_TCAM_MASK:
#ifdef BCM_VO_SUPPORT
        if (SOC_IS_VO(unit)) {
            return _drv_vo_cfp_field_set(unit, mem_type, field_type, entry, 
                fld_val);
        }
#endif /* BCM_VO_SUPPORT */
#ifdef BCM_TB_SUPPORT
        if (SOC_IS_TB(unit)) {
            rv = _drv_tb_cfp_chain_slice_id_mapping(unit, field_type, entry, &slice_id);
            SOC_IF_ERROR_RETURN(rv);
            switch (slice_id) {
                case 0:
                    mem_id = INDEX(CFP_TCAM_IPV4_S0m);
                    break;
                case 1:                    
                    mem_id = INDEX(CFP_TCAM_IPV4_S1m);
                    break;
                case 2:                    
                    mem_id = INDEX(CFP_TCAM_IPV4_S2m);
                    break;            
                case 4:
                    mem_id = INDEX(CFP_TCAM_IPV6_S0m);
                    break;
                case 5:
                    mem_id = INDEX(CFP_TCAM_IPV6_S1m);
                    break;
                case 6:   
                    mem_id = INDEX(CFP_TCAM_IPV6_S2m);
                    break;                
                case 11:
                    mem_id = INDEX(CFP_TCAM_CHAIN_SCm);
                    break;                                    
                case 12:
                case 13:
                case 14:                    
                    mem_id = INDEX(CFP_TCAM_NONIP_MASKm);
                    break;                    
                default:
                    break;
            }

            if (mem_id == -1) {
                return SOC_E_PARAM;
            }
            if (entry->flags & _DRV_CFP_SLICE_CHAIN){
                if (entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN){
                    if (mem_type == DRV_CFP_RAM_TCAM) {
                        data_p = entry->tcam_data;
                    } else { /* mask data */
                        data_p = entry->tcam_mask;
                    }
                } else {
                    if (mem_type == DRV_CFP_RAM_TCAM) {
                        data_p = entry->cfp_chain->tcam_data;
                    } else { /* mask data */
                        data_p = entry->cfp_chain->tcam_mask;
                    }
                }
        } else {
                if (mem_type == DRV_CFP_RAM_TCAM) {
                    data_p = entry->tcam_data;
                } else { /* mask data */
                    data_p = entry->tcam_mask;
                }
            }        
            break;
        }
#endif /* BCM_TB_SUPPORT */        
        rv = SOC_E_UNAVAIL;           
        break;
    case DRV_CFP_RAM_ACT:
        mem_id = INDEX(CFP_ACT_POLm);
        if (entry->flags & _DRV_CFP_SLICE_CHAIN){
            if (entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_MAIN){
               data_p = entry->cfp_chain->act_data;
            } else {
               data_p = entry->act_data;
            }
        } else {
            data_p = entry->act_data;
        }
        break;
    case DRV_CFP_RAM_METER:
       if (SOC_IS_TB_AX(unit)) {
            mem_id = INDEX(CFP_METERm);
        } else {
            mem_id = INDEX(CFP_RATE_METERm);       
        }
        data_p = entry->meter_data;
        break;
    default:
        rv = SOC_E_PARAM;
        return rv;
    } 
    SOC_IF_ERROR_RETURN(rv);
    
    if (( rv = _drv_tbx_cfp_field_mapping(unit, mem_type, field_type, &fld_id)) < 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "drv_cfp_field_set : UNKNOW FIELD ID. \n")));
        return rv;
    }

    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);
    SOC_FIND_FIELD(fld_id, meminfo->fields,
                             meminfo->nFields, fieldinfo);
    assert(fieldinfo);
    /* 
     * Get the value to set into each entry's valid field. 
     * The valid value is depend on chips.
     */
    if ((fld_id == INDEX(VALIDf)) &&  *fld_val != 0){
        len = fieldinfo->len;
        *fld_val = 0;
        for (i = 0; i < len; i++) {
            *fld_val |= (0x1 << i);
        }
    }    

    if ((fld_id == INDEX(ALL_VLDf)) &&  *fld_val != 0){
        len = fieldinfo->len;
        *fld_val = 0;
        for (i = 0; i < len; i++) {
            *fld_val |= (0x1 << i);
        }
    }    
    bp = fieldinfo->bp;
    if (fieldinfo->flags & SOCF_LE) {
        wp = bp / 32;
        bp = bp & (32 - 1);
        i = 0;
        for (len = fieldinfo->len; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
            if (len >= 32) {
                mask = 0xffffffff;
            } else {
                mask = (1 << len) - 1;
            }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
            mask_lo = mask;
            mask_hi = 0;

            /* if field is not aligned with 32-bit word boundary */
            /* adjust hi and lo masks accordingly. */
            if (bp) {
                mask_lo = mask << bp;
                mask_hi = mask >> (32 - bp);
            }
            /* set field value --- 32 bits each time */
            data_p[FIX_MEM_ORDER_E(wp, meminfo)] &= ~mask_lo;
            data_p[FIX_MEM_ORDER_E(wp++, meminfo)] |= 
                ((fld_val[i] << bp) & mask_lo);
            if (mask_hi) {
                data_p[FIX_MEM_ORDER_E(wp, meminfo)] &= ~(mask_hi);
                data_p[FIX_MEM_ORDER_E(wp, meminfo)] |= 
                    ((fld_val[i] >> (32 - bp)) & mask_hi);
            }
            i++;
        }
    } else {                   
        /* Big endian: swap bits */
        len = fieldinfo->len;

        while (len > 0) {
            len--;
            data_p[FIX_MEM_ORDER_E(bp / 32, meminfo)] &= ~(1 << (bp & (32-1)));
            data_p[FIX_MEM_ORDER_E(bp / 32, meminfo)] |=
            (fld_val[len / 32] >> (len & (32-1)) & 1) << (bp & (32-1));
            bp++;
        }
    }
    return rv;
}

/*
 * Function: drv_tbx_cfp_meter_get
 *
 * Purpose:
 *     Get the meter value from CFP entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry -cfp entry
 *     kbits_sec(OUT) -meter rate (kbits per second)
 *     kbits_burst(OUT) -meter bucket size (kbits)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv_tbx_cfp_meter_get(int unit, drv_cfp_entry_t* entry, uint32 *kbits_sec, 
            uint32 *kbits_burst)    
{
    int rv = SOC_E_NONE;
    uint32 ref_cnt, bkt_size, enable;
    
    assert(entry);
    assert(kbits_sec);
    assert(kbits_burst);
        
    ref_cnt = 0;
    bkt_size = 0;
    enable = 0;
    DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_RATE, entry, &enable);
    if (enable) {
        DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_REF_CNT, entry, &ref_cnt);
        DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_BUCKET_SIZE, entry, &bkt_size);        
    }

    *kbits_sec = 64*ref_cnt;
    *kbits_burst = (256*8*bkt_size)/1000;

    return rv;
}

/*
 * Function: _drv_tbx_cfp_meter_rate2chip
 *
 * Purpose:
 *     Translate the driver rate value to register value.
 *
 * Parameters:
 *     unit - BCM device number
 *     kbits_sec - driver rate value
 *     chip_val(OUT) - register value
 *
 * Returns:
 *     Nothing
 */
void
_drv_tbx_cfp_meter_rate2chip(int unit, uint32 kbits_sec, uint32 *ref_cnt)
{
    *ref_cnt = kbits_sec / TB_RATE_REF_COUNT_GRANULARITY(unit);
    return;
}
int
drv_tbx_cfp_meter_rate_transform(int unit, uint32 kbits_sec, uint32 kbits_burst, 
                uint32 *bucket_size, uint32 * ref_cnt, uint32 *ref_unit)
{
    int rv = SOC_E_NONE;

    if (kbits_sec) {
    /*    coverity[unsigned_compare]    */
        if ((kbits_sec > CFP_53280_METER_RATE_MAX(unit)) ||
            (kbits_sec < CFP_53280_METER_RATE_MIN(unit))) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "drv_tbx_cfp_meter_rate_transform : rate unsupported. \n")));
            rv = SOC_E_UNAVAIL;
            return (rv);
        }
        _drv_tbx_cfp_meter_rate2chip(unit, kbits_sec, ref_cnt);
    } else {
        *ref_cnt = 0;
    }

    if (kbits_burst) {
    /*    coverity[unsigned_compare]    */
        if ((kbits_burst > CFP_53280_METER_BURST_MAX(unit)) ||
            (kbits_burst < CFP_53280_METER_BURST_MIN(unit))) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "drv_tbx_cfp_meter_rate_transform : burst size unsupported. \n")));
            rv = SOC_E_UNAVAIL;
            return (rv);
        }
        /* tranfer to Byte */
        *bucket_size = ((kbits_burst + 7) / 8)*1000; 
        /* tranfer value to unit, to set register */
        *bucket_size = *bucket_size / TB_RATE_BUCKET_UNIT_SIZE(unit); 
    } else {
        *bucket_size = 0;
    }

    return rv;
}

/*
 * Function: drv_tbx_cfp_meter_set
 *
 * Purpose:
 *     Set the meter value to CFP entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry(OUT) -cfp entry
 *     kbits_sec -meter rate (kbits per second)
 *     kbits_burst -meter bucket size (kbits)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv_tbx_cfp_meter_set(int unit, drv_cfp_entry_t* entry, uint32 kbits_sec, 
            uint32 kbits_burst)
{
    int rv = SOC_E_NONE;
    uint32  bkt_size;    
    uint32 ref_cnt, enable;

    assert(entry);
    enable = 0;
    if (kbits_sec) {
        enable = 1;
    /*    coverity[unsigned_compare]    */
        if ((kbits_sec > CFP_53280_METER_RATE_MAX(unit)) ||
            (kbits_sec < CFP_53280_METER_RATE_MIN(unit))) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "drv_cfp_meter_set : rate unsupported. \n")));
            rv = SOC_E_UNAVAIL;
            return (rv);
        }
        _drv_tbx_cfp_meter_rate2chip(unit, kbits_sec, &ref_cnt);
    } else {
        ref_cnt = 0;
    }
    DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_REF_CNT, entry, &ref_cnt);

    if (kbits_burst) {
        enable = 1;
    /*    coverity[unsigned_compare]    */
        if ((kbits_burst > CFP_53280_METER_BURST_MAX(unit)) ||
            (kbits_burst < CFP_53280_METER_BURST_MIN(unit))) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "drv_cfp_meter_set : burst size unsupported. \n")));
            rv = SOC_E_UNAVAIL;
            return (rv);
        }
        bkt_size = ((kbits_burst + 7) / 8)*1000; /* tranfer to Byte */
        bkt_size = bkt_size / 256; /* tranfer value to unit, to set register */
    } else {
        bkt_size = 0;
    }
    rv = DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_BUCKET_SIZE, entry, &bkt_size);
    SOC_IF_ERROR_RETURN(rv);

    rv = DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_RATE, entry, &enable);
    SOC_IF_ERROR_RETURN(rv);

    return rv;
}


/*
 * Function: drv_tbx_cfp_slice_id_select
 *
 * Purpose:
 *     According to this entry's fields to select which slice id used for this entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - driver cfp entry
 *     slice_id(OUT) - slice id for this entry
 *     flag- for TB, flag for bcmFieldGroupModexxx
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_RESOURCE - Can't found suitable slice id for this entry.
 */
int
drv_tbx_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, uint32 *slice_id, uint32 flags)
{
    int i;
    int match[CFP_53280_SLICE_MAX_ID+1];
    int slice, qual_num, match_num;
    int id_match, id, mul_id, final_id;

    match_num = 0;
    qual_num = 0;
    id_match = 0;
    mul_id = 0;
    entry->slice_id = -1;
    entry->slice_bmp = 0;
    /* get the qual_num from qset, we'll use qual_num to decide the slice_id */
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        qual_num +=_shr_popcount(entry->w[i]);
    }

    for (slice = 0; slice <= CFP_53280_SLICE_MAX_ID; slice++) {
        match[slice] = TRUE;
        for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
            if (entry->w[i] & ~drv53280_slice_info[slice].qset[i]) {
                match[slice] = FALSE;
                break;
            }
        }
        if (TRUE == match[slice]){
            id = CFP_53280_SLICE_ID(slice);
            id_match |= 1 << id;
            match_num++;
            entry->slice_bmp |= 1 << slice;
        }
    }

    if (!match_num) {
        return SOC_E_NOT_FOUND;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "entry->slice_bmp %x flags %d \n"),
                 entry->slice_bmp, flags));

    if (_shr_popcount(id_match) > 1) {
        mul_id = TRUE;
    }
    
    final_id = -1;
    
    if (flags == DRV_FIELD_QUAL_CHAIN) {
        /* double wide must be slice&0x3=0 or 3*/
        if (!((id_match & CFP_53280_SLICE_ID_MAP(CFP_53280_SLICE_ID_ALLOW_CHAIN))
            ||(id_match & CFP_53280_SLICE_ID_MAP(CFP_53280_SLICE_ID_WITH_CHAIN)))) {
            return SOC_E_NOT_FOUND;
        }
        if (id_match & CFP_53280_SLICE_ID_MAP(CFP_53280_SLICE_ID_ALLOW_CHAIN)) {
            if (entry->slice_bmp & CFP_53280_SLICE_ID_MAP(CFP_53280_SLICE_ID_IPV4_BASE)){
                entry->slice_bmp &= 0xfff8;
            }                    
            if (entry->slice_bmp & CFP_53280_SLICE_ID_MAP(CFP_53280_SLICE_ID_IPV6_BASE)) {
                entry->slice_bmp &= 0xff8f;
            }                    
            if (entry->slice_bmp & CFP_53280_SLICE_ID_MAP(CFP_53280_SLICE_ID_NONIP_BASE)) {
                entry->slice_bmp &= 0x8fff;
            }
        } 

        LOG_DEBUG(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "DRV_FP: slice_bmp %x\n"),
                   entry->slice_bmp));

        for (slice = CFP_53280_SLICE_MAX_ID; slice >= 0; slice--) {
            if (entry->slice_bmp & CFP_53280_SLICE_ID_MAP(slice)){
                *slice_id = slice;
                break;
            }
        }
        return SOC_E_NONE;            
    } else {
        if (entry->slice_bmp & CFP_53280_SLICE_MAP_FRAME_CHAIN) {
            *slice_id = CFP_53280_SLICE_ID_CHAIN_SLICE;
            return SOC_E_NONE;            
        }
        if (flags == DRV_FIELD_QUAL_SINGLE) {
            /* mode single won't accept slice&0x3 == 3*/
            id_match &= ~(CFP_53280_SLICE_ID_MAP(CFP_53280_SLICE_ID_WITH_CHAIN));
            if (_shr_popcount(id_match) == 0) {
                return SOC_E_NOT_FOUND;
            }            
            entry->slice_bmp &= ~CFP_53280_SLICE_MAP_CHAIN;
        } else {
            /* auto mode */
            if (entry->slice_bmp & ~(CFP_53280_SLICE_MAP_CHAIN)) {
                entry->slice_bmp &= ~CFP_53280_SLICE_MAP_CHAIN;
            } 
        }
    }

    if (mul_id) {
        if ( qual_num < CFP_53280_SLICE_PRI_LEVEL0) {
            /* choose id = 0 or the lowest */
            if (id_match & CFP_53280_SLICE_ID_MAP(0)){
                final_id = 0;
            } else {
                /* choose the lowest id*/
                for (i = 0; i < 2; i++){
                    if ((id_match >> i) & 0x1) {
                        final_id = i;
                        break;
                    }
                }
            }
        } else if ((CFP_53280_SLICE_PRI_LEVEL0 <= qual_num )
            && (qual_num < CFP_53280_SLICE_PRI_LEVEL1) ){
            /* choose id = 1 , then 2, then 0*/
            if (id_match & CFP_53280_SLICE_ID_MAP(1)){
                final_id = 1;
            } else {
                if (id_match & CFP_53280_SLICE_ID_MAP(2)) {
                    final_id = 2; 
                } else if (id_match & CFP_53280_SLICE_ID_MAP(0)){
                    final_id = 0;
                }
            }
        } else {
            /* choose id = 2, then 1, then 0*/
            if (id_match & CFP_53280_SLICE_ID_MAP(2)){
                final_id = 2;
            } else {
                if (id_match & CFP_53280_SLICE_ID_MAP(1)) {
                    final_id = 1; 
                } else if (id_match & CFP_53280_SLICE_ID_MAP(0)){
                    final_id = 0;
                }
            }
        }                
    }
    
    for (slice = CFP_53280_SLICE_MAX_ID; slice >= 0; slice--) {
        if (TRUE == match[slice]){
            id = CFP_53280_SLICE_ID(slice);
            if (final_id == -1){
                *slice_id = slice;
                break;
            }else if (final_id == id){
                *slice_id = slice;
                break;                                
            } 
        }
    }
    
    return SOC_E_NONE;    
}

/*
 * Function: drv_tbx_cfp_slice_to_qset
 *
 * Purpose:
 *     According to slice id used for this entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry(OUT) - driver cfp entry
 *     slice_id - slice id 
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - un-support slice id.
 */
int
drv_tbx_cfp_slice_to_qset(int unit, uint32 slice_id, drv_cfp_entry_t *entry)
{
    uint32 i;

    if (slice_id > CFP_53280_SLICE_MAX_ID) {
        return SOC_E_PARAM;
    }
    
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        entry->w[i] = drv53280_slice_info[slice_id].qset[i];
    }
    return SOC_E_NONE;
    
}

/*
 * Function: drv_tbx_cfp_stat_get
 *
 * Purpose:
 *     Get the counter value from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     stat_type - In-band/Out-band counter/Both
 *     index -entry index
 *     counter(OUT) -counter value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
drv_tbx_cfp_stat_get(int unit, uint32 stat_type, uint32 index, uint32* counter)
{
    int rv = SOC_E_NONE;
    uint32 *mem_entry, temp;
    int mem_len, size;

    assert(counter);
   
    mem_len = soc_mem_entry_words(unit, INDEX(CFP_STATm));
    size = mem_len * sizeof(uint32);
    mem_entry = (uint32 *) sal_alloc(size, "cfp mem entry");
    if (mem_entry == NULL) {
        return SOC_E_RESOURCE;
    }
    sal_memset(mem_entry, 0, size);

    rv = DRV_MEM_READ(unit, DRV_MEM_CFP_STAT, index, 1, mem_entry);

    if (SOC_FAILURE(rv)) {
        sal_free(mem_entry);
        return rv;
    }
    switch (stat_type) {
        case DRV_CFP_STAT_INBAND:
            sal_memcpy(counter, mem_entry, 4);
            break;
        case DRV_CFP_STAT_OUTBAND:
            if (SOC_IS_TB_AX(unit)){
                sal_memcpy(counter, &mem_entry[1], 4);
            } else {
                sal_memcpy(counter, &mem_entry[2], 4);
            }
            break;
        case DRV_CFP_STAT_YELLOW:
            if (SOC_IS_TB_AX(unit)){
                rv = SOC_E_PARAM;
            } else {
                sal_memcpy(counter, &mem_entry[1], 4);
            }
            break;
        case DRV_CFP_STAT_ALL:
            sal_memcpy(&temp, mem_entry, 4);
            *counter = temp;
             sal_memcpy(&temp, &mem_entry[1], 4);
            *counter += temp;
            if (!SOC_IS_TB_AX(unit)){
                 sal_memcpy(&temp, &mem_entry[2], 4);
                *counter += temp;
            }
            break;
        default:
            rv = SOC_E_PARAM;
    }
    sal_free(mem_entry);
    return rv;
}

/*
 * Function: drv_tbx_cfp_stat_set
 *
 * Purpose:
 *     Set the CFP counter value to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     stat_type - In-band/Out-band counter/Both
 *     index -entry index
 *     counter -counter value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
drv_tbx_cfp_stat_set(int unit, uint32 stat_type, uint32 index, uint32 counter)
{           
    int rv = SOC_E_NONE;
    uint32 *mem_entry;
    int mem_len, size;
      
    mem_len = soc_mem_entry_words(unit, INDEX(CFP_STATm));
    size = mem_len * sizeof(uint32);
    mem_entry = (uint32 *) sal_alloc(size, "cfp mem entry");
    if (mem_entry == NULL) {
        return SOC_E_RESOURCE;
    }
    sal_memset(mem_entry, 0, size);

    switch (stat_type) {
        case DRV_CFP_STAT_INBAND:
            sal_memcpy(mem_entry, &counter, 4);
            break;
        case DRV_CFP_STAT_OUTBAND:
            if (SOC_IS_TB_AX(unit)){
                sal_memcpy(&mem_entry[1], &counter, 4);
            } else {
                sal_memcpy(&mem_entry[2], &counter, 4);
            }
            break;
        case DRV_CFP_STAT_YELLOW:
            if (SOC_IS_TB_AX(unit)){
                rv = SOC_E_PARAM;
            } else {
                sal_memcpy(&mem_entry[1], &counter, 4);
            }
            break;
        case DRV_CFP_STAT_ALL:
            sal_memcpy(mem_entry, &counter, 4);
            sal_memcpy(&mem_entry[1], &counter, 4);
            if (!SOC_IS_TB_AX(unit)){
                sal_memcpy(&mem_entry[2], &counter, 4);
            }
            break;
        default:
            rv = SOC_E_PARAM;
    }
    rv = DRV_MEM_WRITE(unit, DRV_MEM_CFP_STAT, index, 1, mem_entry);

    if (SOC_FAILURE(rv)) {
        sal_free(mem_entry);
        return rv;
    }

    sal_free(mem_entry);
    return rv;
}

/*
 * Function: drv_tbx_cfp_udf_get
 *
 * Purpose:
 *     Get the offset value of the User Defined fields.
 *
 * Parameters:
 *     unit - BCM device number
 *     port - For TB udfs are global settings not related to port.
 *               here, we use this parameter to differentiate the result 
 *               we get from the register offset or just the mapping between
 *               the bcm layer and the driver service layer.
 *               port = 0  (DRV_CFP_UDF_OFFSET_GET)
 *                           bcm udf_id <--> register level access
 *               port = DRV_CFP_UDF_QUAL_GET
 *                           bcm udf_id <--> driver service level mapping
 *     udf_index -the index of user defined fields
 *     offset(OUT) - when port = 0, offset value
 *                                  port != 0, DRV_CFP_QUAL_UDFxxx
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 *     
 */
int 
drv_tbx_cfp_udf_get(int unit, uint32 port, uint32 udf_index, 
    uint32 *offset, uint32 *base)
{
    int rv = SOC_E_NONE;
    uint32  temp, reg_32;

    if (udf_index >=  CFP_53280_UDF_NUM_MAX) {
        return SOC_E_CONFIG;
    }

    if (port == DRV_CFP_UDF_LENGTH_GET){
        *offset = 2; /* all the UDF length are 2-byte length */
        return SOC_E_NONE;
    }

    /* if  port !=0 get the cfp qual udf id */
    if (port == DRV_CFP_UDF_QUAL_GET) {        
        *offset = drv53280_udf_reg_info[udf_index].cfp_qual;
        return SOC_E_NONE;
    }
    if (port == DRV_CFP_UDF_FIELD_GET) {        
        *offset = drv53280_udf_reg_info[udf_index].cfp_field;
        *base = drv53280_udf_reg_info[udf_index].cfp_field_valid; 
        return SOC_E_NONE;
    }

    if (port == DRV_CFP_UDF_SLICE_ID_GET) {        
        *offset = drv53280_udf_reg_info[udf_index].slice_id;
        *base = CFP_53280_SLICE_ID_MAP(*offset);
        return SOC_E_NONE;
    }
    /* For BCM53280
     * 0 : slice0 A0,   5: slice1 A0,     7 : slice2 A0  
     * 1 : slice0 A1,   6: slice1 A1,     8 : slice2 A1
     * 2 : slice0 A2,                     9 : slice2 A2
     * 3 : slice0 A3,                    10 : slice2 A3
     * 4 : slice0 A4,                    11 : slice2 A4
     *                                   12 : slice2 A5
     *                                   13 : slice2 A6
     *                                   14 : slice2 A7
     *                                   15 : slice2 A8
     *
     * 16 : slice0 B8,                   17 : slice2 B0  
     *                                   18 : slice2 B1
     *                                   19 : slice2 B2
     *                                   20 : slice2 B3
     *                                   21 : slice2 B4
     *                                   22 : slice2 B5
     *                                   23 : slice2 B6
     *                                   24 : slice2 B7
     *                                   25 : slice2 B8
     *
     * 26 : slice0 C0,  29 : slice1 C0,  32 : slice2 C0  
     * 27 : slice0 C1,  30 : slice1 C1,  33 : slice2 C1
     * 28 : slice0 C2,  31 : slice1 C2,  34 : slice2 C2
     *
     * 35 : D0
     * 36 : D1
     * 37 : D2
     * 38 : D3
     * 39 : D4
     * 40 : D5
     * 41 : D6
     * 42 : D7
     * 43 : D8
     * 44 : D9
     * 45 : D10
     * 46 : D11
     */

    rv = CFP_UDF_GET(unit, udf_index, &reg_32);
    SOC_IF_ERROR_RETURN(rv);

    rv = soc_CFP_UDF_0_Ar_field_get(unit, &reg_32, UDF_N_X_OFFSETf, &temp);
    SOC_IF_ERROR_RETURN(rv);

    *offset = temp * 2;

    soc_CFP_UDF_0_Ar_field_get(unit, &reg_32, UDF_N_X_REFf, &temp);
    switch(temp) {
        case 0:
            *base = DRV_CFP_UDF_OFFSET_BASE_START_OF_FRAME;
            break;
        case 2:
            *base = DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR;
            break;
        case 3:
            *base = DRV_CFP_UDF_OFFSET_BASE_END_OF_L3_HDR;
            break;
    }

    return rv;
}

/*
 * Function: drv_tbx_cfp_udf_set
 *
 * Purpose:
 *     Set the offset value of the User Defined fields.
 *
 * Parameters:
 *     unit - BCM device number
 *     port - port numbrt
 *     udf_index -the index of user defined fields
 *     offset -offset value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int 
drv_tbx_cfp_udf_set(int unit, uint32 port, uint32 udf_index, 
    uint32 offset, uint32 base)
{
    int rv = SOC_E_NONE;
    uint32  reg_32, temp;

    if (udf_index >=  CFP_53280_UDF_NUM_MAX) {
        return SOC_E_CONFIG;
    }
    if (offset > CFP_53280_UDF_OFFSET_MAX) {
        return SOC_E_CONFIG;
    }

    /* For BCM53280
     * 0 : slice0 A0,   5: slice1 A0,     7 : slice2 A0  
     * 1 : slice0 A1,   6: slice1 A1,     8 : slice2 A1
     * 2 : slice0 A2,                     9 : slice2 A2
     * 3 : slice0 A3,                    10 : slice2 A3
     * 4 : slice0 A4,                    11 : slice2 A4
     *                                   12 : slice2 A5
     *                                   13 : slice2 A6
     *                                   14 : slice2 A7
     *                                   15 : slice2 A8
     *
     * 16 : slice0 B8,                   17 : slice2 B0  
     *                                   18 : slice2 B1
     *                                   19 : slice2 B2
     *                                   20 : slice2 B3
     *                                   21 : slice2 B4
     *                                   22 : slice2 B5
     *                                   23 : slice2 B6
     *                                   24 : slice2 B7
     *                                   25 : slice2 B8
     *
     * 26 : slice0 C0,  29 : slice1 C0,  32 : slice2 C0  
     * 27 : slice0 C1,  30 : slice1 C1,  33 : slice2 C1
     * 28 : slice0 C2,  31 : slice1 C2,  34 : slice2 C2
     *
     * 35 : D0
     * 36 : D1
     * 37 : D2
     * 38 : D3
     * 39 : D4
     * 40 : D5
     * 41 : D6
     * 42 : D7
     * 43 : D8
     * 44 : D9
     * 45 : D10
     * 46 : D11
     */
    rv = CFP_UDF_GET(unit, udf_index, &reg_32);

    SOC_IF_ERROR_RETURN(rv);

    temp = (offset /2 ) & 0x1f; /* the offset is 2N bytes based */

    soc_CFP_UDF_0_Ar_field_set(unit, &reg_32, UDF_N_X_OFFSETf, &temp);
    

    switch(base) {
        case DRV_CFP_UDF_OFFSET_BASE_START_OF_FRAME:
            temp = 0;
            break;
        case DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR:
            temp = 2;
            break;
        case DRV_CFP_UDF_OFFSET_BASE_END_OF_L3_HDR:
            temp = 3;
            break;
        default:
            return SOC_E_PARAM;
    }

    soc_CFP_UDF_0_Ar_field_set(unit, &reg_32, UDF_N_X_REFf, &temp);

    CFP_UDF_SET(unit, udf_index, &reg_32);    
    return rv;
}




