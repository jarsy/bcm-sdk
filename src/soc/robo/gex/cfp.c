/*
 * $Id: cfp.c,v 1.9 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <assert.h>
#include <soc/types.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/drv_if.h>
#include <soc/cfp.h>
#include <soc/robo/gex_service.h>

/* Ram select code for bcm53115 */
#define CFP_53115_RAM_SEL_OB_STAT 0x10
#define CFP_53115_RAM_SEL_IB_STAT 0x8
#define CFP_53115_RAM_SEL_METER 0x4
#define CFP_53115_RAM_SEL_ACT 0x2
#define CFP_53115_RAM_SEL_TCAM 0x1

/* OP code */
#define CFP_53115_OP_NONE 0x0
#define CFP_53115_OP_READ 0x1
#define CFP_53115_OP_WRITE 0x2
#define CFP_53115_OP_SEARCH_VALID 0x4

/* Flood and drop value for BCM53115 */
#define CFP_53115_DEST_FLOOD 0x7f
#define CFP_53115_DEST_DROP 0x0


#define CFP_53115_L3_FRAM_IPV4 0x0
#define CFP_53115_L3_FRAM_IPV6 0x1
#define CFP_53115_L3_FRAM_NONIP 0x3
#define CFP_53115_SLICE_MAX_ID 15 /* L3 fram + slice id */

/* User defined fields for BCM53115 */
#define CFP_53115_UDF_NUM_MAX 93
#define CFP_53115_UDF_OFFSET_MAX 96

/* 1Gb */
#define CFP_53115_METER_RATE_MAX (1000 * 1000)
/* 64Kbits/sec */
#define CFP_53115_METER_RATE_MIN 64
/* 128Mb */
#define CFP_53115_METER_BURST_MAX (128 * 1000)   
/* 128K bits */
#define CFP_53115_METER_BURST_MIN 128



#define CFP_53115_IPV6_CHAIN_SLICE_ID   0x7
#define CFP_53115_IPV6_MAIN_SLICE_ID    0x4



/* Slice 0~4 qualify set */
/* Since the slice format was decides by slice ID and L3 framing */
/* We combined the l3 framinng and slice id as "slice id for upper layer */
/* L3 Framing         Slice ID              Slice ID value */
/* b00 (IPv4)                b00               b0000(0)
                                    b01               b0001(1)
                                    b10               b0010(2)
     b01(IPv6)                 b00               b0100(4)
                                    b01               b0101(5)
                                    b10               b0110(6)
                                    b11               b0111(7)
     b11(Non-IP)             b00               b1100(12)
                                    b01               b1101(13)
                                    b10               b1110(14) */

/* IPv4    SLICEs */
static int s0_qset[] = { DRV_CFP_QUAL_SRC_PBMP,
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_L2_FRM_FORMAT,
                                  DRV_CFP_QUAL_L3_FRM_FORMAT,
                                  DRV_CFP_QUAL_IPV4,
                                  DRV_CFP_QUAL_IP_TOS,
                                  DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,
                                  DRV_CFP_QUAL_IP_PROTO,
                                  DRV_CFP_QUAL_IP6_NEXT_HEADER,
                                  DRV_CFP_QUAL_IP_FRGA,
                                  DRV_CFP_QUAL_IP_NON_FIRST_FRGA,
                                  DRV_CFP_QUAL_IP_AUTH,
                                  DRV_CFP_QUAL_IP_TTL,
                                  DRV_CFP_QUAL_IP6_HOP_LIMIT,
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                                  DRV_CFP_QUAL_PPPOE_SESSION_FRM,
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
                                  DRV_CFP_QUAL_SP_VID,
                                  DRV_CFP_QUAL_SP_CFI,
                                  DRV_CFP_QUAL_SP_PRI,
                                  DRV_CFP_QUAL_USR_VID,
                                  DRV_CFP_QUAL_USR_CFI,
                                  DRV_CFP_QUAL_USR_PRI,
                                  DRV_CFP_QUAL_UDFA0,
                                  DRV_CFP_QUAL_UDFA1,
                                  DRV_CFP_QUAL_UDFA2,
                                  DRV_CFP_QUAL_UDFA3,
                                  DRV_CFP_QUAL_UDFA4,
                                  DRV_CFP_QUAL_UDFA5,
                                  DRV_CFP_QUAL_UDFA6,
                                  DRV_CFP_QUAL_UDFA7,
                                  DRV_CFP_QUAL_UDFA8,
                                 DRV_CFP_QUAL_INVALID};
static int s1_qset[] = { DRV_CFP_QUAL_SRC_PBMP,
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_L2_FRM_FORMAT,
                                  DRV_CFP_QUAL_L3_FRM_FORMAT,
                                  DRV_CFP_QUAL_IPV4,
                                  DRV_CFP_QUAL_IP_TOS,
                                  DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,
                                  DRV_CFP_QUAL_IP_PROTO,
                                  DRV_CFP_QUAL_IP6_NEXT_HEADER,
                                  DRV_CFP_QUAL_IP_FRGA,
                                  DRV_CFP_QUAL_IP_NON_FIRST_FRGA,
                                  DRV_CFP_QUAL_IP_AUTH,
                                  DRV_CFP_QUAL_IP_TTL,
                                  DRV_CFP_QUAL_IP6_HOP_LIMIT,
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHRER3_SUPPORT)
                                  DRV_CFP_QUAL_PPPOE_SESSION_FRM,
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_SRARFIGHTER3_SUPPORT */
                                  DRV_CFP_QUAL_SP_VID,
                                  DRV_CFP_QUAL_SP_CFI,
                                  DRV_CFP_QUAL_SP_PRI,
                                  DRV_CFP_QUAL_USR_VID,
                                  DRV_CFP_QUAL_USR_CFI,
                                  DRV_CFP_QUAL_USR_PRI,
                                  DRV_CFP_QUAL_UDFA9,
                                  DRV_CFP_QUAL_UDFA10,
                                  DRV_CFP_QUAL_UDFA11,
                                  DRV_CFP_QUAL_UDFA12,
                                  DRV_CFP_QUAL_UDFA13,
                                  DRV_CFP_QUAL_UDFA14,
                                  DRV_CFP_QUAL_UDFA15,
                                  DRV_CFP_QUAL_UDFA16,
                                  DRV_CFP_QUAL_UDFA17,
                                  DRV_CFP_QUAL_CLASS_ID,
                                 DRV_CFP_QUAL_INVALID};
static int s2_qset[] = { DRV_CFP_QUAL_SRC_PBMP,
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_L2_FRM_FORMAT,
                                  DRV_CFP_QUAL_L3_FRM_FORMAT,
                                  DRV_CFP_QUAL_IPV4,
                                  DRV_CFP_QUAL_IP_TOS,
                                  DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,
                                  DRV_CFP_QUAL_IP_PROTO,
                                  DRV_CFP_QUAL_IP6_NEXT_HEADER,
                                  DRV_CFP_QUAL_IP_FRGA,
                                  DRV_CFP_QUAL_IP_NON_FIRST_FRGA,
                                  DRV_CFP_QUAL_IP_AUTH,
                                  DRV_CFP_QUAL_IP_TTL,
                                  DRV_CFP_QUAL_IP6_HOP_LIMIT,
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                                  DRV_CFP_QUAL_PPPOE_SESSION_FRM,
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
                                  DRV_CFP_QUAL_SP_VID,
                                  DRV_CFP_QUAL_SP_CFI,
                                  DRV_CFP_QUAL_SP_PRI,
                                  DRV_CFP_QUAL_USR_VID,
                                  DRV_CFP_QUAL_USR_CFI,
                                  DRV_CFP_QUAL_USR_PRI,
                                  DRV_CFP_QUAL_UDFA18,
                                  DRV_CFP_QUAL_UDFA19,
                                  DRV_CFP_QUAL_UDFA20,
                                  DRV_CFP_QUAL_UDFA21,
                                  DRV_CFP_QUAL_UDFA22,
                                  DRV_CFP_QUAL_UDFA23,
                                  DRV_CFP_QUAL_UDFA24,
                                  DRV_CFP_QUAL_UDFA25,
                                  DRV_CFP_QUAL_UDFA26,
                                  DRV_CFP_QUAL_CLASS_ID,
                                 DRV_CFP_QUAL_INVALID};



/* IPv6 SLICEs */
static int s4_qset[] = { DRV_CFP_QUAL_SRC_PBMP,
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_L2_FRM_FORMAT,
                                  DRV_CFP_QUAL_L3_FRM_FORMAT,
                                  DRV_CFP_QUAL_IPV6,
                                  DRV_CFP_QUAL_IP_TOS,
                                  DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,
                                  DRV_CFP_QUAL_IP_PROTO,
                                  DRV_CFP_QUAL_IP6_NEXT_HEADER,
                                  DRV_CFP_QUAL_IP_FRGA,
                                  DRV_CFP_QUAL_IP_NON_FIRST_FRGA,
                                  DRV_CFP_QUAL_IP_AUTH,
                                  DRV_CFP_QUAL_IP_TTL,
                                  DRV_CFP_QUAL_IP6_HOP_LIMIT,
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                                  DRV_CFP_QUAL_PPPOE_SESSION_FRM,
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
                                  DRV_CFP_QUAL_SP_VID,
                                  DRV_CFP_QUAL_SP_CFI,
                                  DRV_CFP_QUAL_SP_PRI,
                                  DRV_CFP_QUAL_USR_VID,
                                  DRV_CFP_QUAL_USR_CFI,
                                  DRV_CFP_QUAL_USR_PRI,
                                  DRV_CFP_QUAL_UDFB0,
                                  DRV_CFP_QUAL_UDFB1,
                                  DRV_CFP_QUAL_UDFB2,
                                  DRV_CFP_QUAL_UDFB3,
                                  DRV_CFP_QUAL_UDFB4,
                                  DRV_CFP_QUAL_UDFB5,
                                  DRV_CFP_QUAL_UDFB6,
                                  DRV_CFP_QUAL_UDFB7,
                                  DRV_CFP_QUAL_UDFB8,
                                 DRV_CFP_QUAL_INVALID};
static int s5_qset[] = { DRV_CFP_QUAL_SRC_PBMP,
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_L2_FRM_FORMAT,
                                  DRV_CFP_QUAL_L3_FRM_FORMAT,
                                  DRV_CFP_QUAL_IPV6,
                                  DRV_CFP_QUAL_IP_TOS,
                                  DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,
                                  DRV_CFP_QUAL_IP_PROTO,
                                  DRV_CFP_QUAL_IP6_NEXT_HEADER,
                                  DRV_CFP_QUAL_IP_FRGA,
                                  DRV_CFP_QUAL_IP_NON_FIRST_FRGA,
                                  DRV_CFP_QUAL_IP_AUTH,
                                  DRV_CFP_QUAL_IP_TTL,
                                  DRV_CFP_QUAL_IP6_HOP_LIMIT,
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                                  DRV_CFP_QUAL_PPPOE_SESSION_FRM,
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
                                  DRV_CFP_QUAL_SP_VID,
                                  DRV_CFP_QUAL_SP_CFI,
                                  DRV_CFP_QUAL_SP_PRI,
                                  DRV_CFP_QUAL_USR_VID,
                                  DRV_CFP_QUAL_USR_CFI,
                                  DRV_CFP_QUAL_USR_PRI,
                                  DRV_CFP_QUAL_UDFB9,
                                  DRV_CFP_QUAL_UDFB10,
                                  DRV_CFP_QUAL_UDFB11,
                                  DRV_CFP_QUAL_UDFB12,
                                  DRV_CFP_QUAL_UDFB13,
                                  DRV_CFP_QUAL_UDFB14,
                                  DRV_CFP_QUAL_UDFB15,
                                  DRV_CFP_QUAL_UDFB16,
                                  DRV_CFP_QUAL_UDFB17,
                                  DRV_CFP_QUAL_CLASS_ID,
                                 DRV_CFP_QUAL_INVALID};
static int s6_qset[] = { DRV_CFP_QUAL_SRC_PBMP,
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_L2_FRM_FORMAT,
                                  DRV_CFP_QUAL_L3_FRM_FORMAT,
                                  DRV_CFP_QUAL_IPV6,
                                  DRV_CFP_QUAL_IP_TOS,
                                  DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,
                                  DRV_CFP_QUAL_IP_PROTO,
                                  DRV_CFP_QUAL_IP6_NEXT_HEADER,
                                  DRV_CFP_QUAL_IP_FRGA,
                                  DRV_CFP_QUAL_IP_NON_FIRST_FRGA,
                                  DRV_CFP_QUAL_IP_AUTH,
                                  DRV_CFP_QUAL_IP_TTL,
                                  DRV_CFP_QUAL_IP6_HOP_LIMIT,
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                                  DRV_CFP_QUAL_PPPOE_SESSION_FRM,
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
                                  DRV_CFP_QUAL_SP_VID,
                                  DRV_CFP_QUAL_SP_CFI,
                                  DRV_CFP_QUAL_SP_PRI,
                                  DRV_CFP_QUAL_USR_VID,
                                  DRV_CFP_QUAL_USR_CFI,
                                  DRV_CFP_QUAL_USR_PRI,
                                  DRV_CFP_QUAL_UDFB18,
                                  DRV_CFP_QUAL_UDFB19,
                                  DRV_CFP_QUAL_UDFB20,
                                  DRV_CFP_QUAL_UDFB21,
                                  DRV_CFP_QUAL_UDFB22,
                                  DRV_CFP_QUAL_UDFB23,
                                  DRV_CFP_QUAL_UDFB24,
                                  DRV_CFP_QUAL_UDFB25,
                                  DRV_CFP_QUAL_UDFB26,
                                  DRV_CFP_QUAL_CLASS_ID,
                                 DRV_CFP_QUAL_INVALID};
static int s7_qset[] = { DRV_CFP_QUAL_CHAIN_ID,
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
                                  DRV_CFP_QUAL_CLASS_ID,
                                 DRV_CFP_QUAL_INVALID};

/* Non-IP SLICEs */
static int s12_qset[] = { DRV_CFP_QUAL_SRC_PBMP,
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_L2_FRM_FORMAT,
                                  DRV_CFP_QUAL_L3_FRM_FORMAT,
                                  DRV_CFP_QUAL_ETYPE,
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                                  DRV_CFP_QUAL_PPPOE_SESSION_FRM,
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPOPORT */
                                  DRV_CFP_QUAL_SP_VID,
                                  DRV_CFP_QUAL_SP_CFI,
                                  DRV_CFP_QUAL_SP_PRI,
                                  DRV_CFP_QUAL_USR_VID,
                                  DRV_CFP_QUAL_USR_CFI,
                                  DRV_CFP_QUAL_USR_PRI,
                                  DRV_CFP_QUAL_UDFC0,
                                  DRV_CFP_QUAL_UDFC1,
                                  DRV_CFP_QUAL_UDFC2,
                                  DRV_CFP_QUAL_UDFC3,
                                  DRV_CFP_QUAL_UDFC4,
                                  DRV_CFP_QUAL_UDFC5,
                                  DRV_CFP_QUAL_UDFC6,
                                  DRV_CFP_QUAL_UDFC7,
                                  DRV_CFP_QUAL_UDFC8,
                                 DRV_CFP_QUAL_INVALID};
static int s13_qset[] = { DRV_CFP_QUAL_SRC_PBMP,
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_L2_FRM_FORMAT,
                                  DRV_CFP_QUAL_L3_FRM_FORMAT,
                                  DRV_CFP_QUAL_ETYPE,
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                                  DRV_CFP_QUAL_PPPOE_SESSION_FRM,
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
                                  DRV_CFP_QUAL_SP_VID,
                                  DRV_CFP_QUAL_SP_CFI,
                                  DRV_CFP_QUAL_SP_PRI,
                                  DRV_CFP_QUAL_USR_VID,
                                  DRV_CFP_QUAL_USR_CFI,
                                  DRV_CFP_QUAL_USR_PRI,
                                  DRV_CFP_QUAL_UDFC9,
                                  DRV_CFP_QUAL_UDFC10,
                                  DRV_CFP_QUAL_UDFC11,
                                  DRV_CFP_QUAL_UDFC12,
                                  DRV_CFP_QUAL_UDFC13,
                                  DRV_CFP_QUAL_UDFC14,
                                  DRV_CFP_QUAL_UDFC15,
                                  DRV_CFP_QUAL_UDFC16,
                                  DRV_CFP_QUAL_UDFC17,
                                  DRV_CFP_QUAL_CLASS_ID,
                                 DRV_CFP_QUAL_INVALID};
static int s14_qset[] = { DRV_CFP_QUAL_SRC_PBMP,
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_L2_FRM_FORMAT,
                                  DRV_CFP_QUAL_L3_FRM_FORMAT,
                                  DRV_CFP_QUAL_ETYPE,
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                                  DRV_CFP_QUAL_PPPOE_SESSION_FRM,
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
                                  DRV_CFP_QUAL_SP_VID,
                                  DRV_CFP_QUAL_SP_CFI,
                                  DRV_CFP_QUAL_SP_PRI,
                                  DRV_CFP_QUAL_USR_VID,
                                  DRV_CFP_QUAL_USR_CFI,
                                  DRV_CFP_QUAL_USR_PRI,
                                  DRV_CFP_QUAL_UDFC18,
                                  DRV_CFP_QUAL_UDFC19,
                                  DRV_CFP_QUAL_UDFC20,
                                  DRV_CFP_QUAL_UDFC21,
                                  DRV_CFP_QUAL_UDFC22,
                                  DRV_CFP_QUAL_UDFC23,
                                  DRV_CFP_QUAL_UDFC24,
                                  DRV_CFP_QUAL_UDFC25,
                                  DRV_CFP_QUAL_UDFC26,
                                  DRV_CFP_QUAL_CLASS_ID,
                                 DRV_CFP_QUAL_INVALID};

#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? BYTES2WORDS((m)->bytes)-1-(v) : (v))


typedef struct drv_slice_info_s {
    int slice_id;
    SHR_BITDCL  qset[_SHR_BITDCLSIZE(DRV_CFP_QUAL_COUNT)];
    int slice_udf_free;
}drv_slice_info_t;

static drv_slice_info_t drv53115_slice_info[CFP_53115_SLICE_MAX_ID +1]; 


typedef struct drv_udf_info_s {
    int valid;
    int self_field;
    int valid_field;
    int length;
    int offset_value;
    int offset_base;
    int slice_id;
    int sub_qual;
    int sub_field;
    int order;
}drv_udf_info_t;

static drv_udf_info_t drv53115_udf_info[CFP_53115_UDF_NUM_MAX];


typedef struct drv_cfp_qual_udf_translate_s {
    int qual;
    int field;
    int udf_num;
    int udf_base;
    int udf_off[8];
} drv_cfp_qual_udf_translate_t;
static drv_cfp_qual_udf_translate_t drv53115_qual_udf_translate_info[] = {
    {DRV_CFP_QUAL_MAC_DA, DRV_CFP_FIELD_MAC_DA, 
        3, DRV_CFP_UDF_OFFSET_BASE_START_OF_FRAME, 
        {4, 2, 0, 0, 0, 0, 0, 0}},
    {DRV_CFP_QUAL_MAC_SA, DRV_CFP_FIELD_MAC_SA, 
        3, DRV_CFP_UDF_OFFSET_BASE_START_OF_FRAME, 
        {10, 8, 6, 0, 0, 0, 0, 0}},
    {DRV_CFP_QUAL_IP_SA, DRV_CFP_FIELD_IP_SA, 
        2, DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR, 
        {14, 12, 0, 0, 0, 0, 0, 0}},
    {DRV_CFP_QUAL_IP_DA, DRV_CFP_FIELD_IP_DA, 
        2, DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR, 
        {18, 16, 0, 0, 0, 0, 0, 0}},
    {DRV_CFP_QUAL_IP6_SA, DRV_CFP_FIELD_IP6_SA,
        8, DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR, 
        {22, 20, 18, 16, 14, 12, 10, 8}},
    {DRV_CFP_QUAL_IP6_DA, DRV_CFP_FIELD_IP6_DA,
        8, DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR, 
        {38, 36, 34, 32, 30, 28, 26, 24}},
    {DRV_CFP_QUAL_L4_SRC, DRV_CFP_FIELD_L4SRC, 
        1, DRV_CFP_UDF_OFFSET_BASE_END_OF_L3_HDR, 
        {0, 0, 0, 0, 0, 0, 0, 0}},
    {DRV_CFP_QUAL_L4_DST, DRV_CFP_FIELD_L4DST,
        1, DRV_CFP_UDF_OFFSET_BASE_END_OF_L3_HDR, 
        {2, 0, 0, 0, 0, 0, 0, 0}},
    {DRV_CFP_QUAL_TCP_FLAG, DRV_CFP_FIELD_TCP_FLAG,
        1, DRV_CFP_UDF_OFFSET_BASE_END_OF_L3_HDR, 
        {13, 0, 0, 0, 0, 0, 0, 0}},
    {DRV_CFP_QUAL_SNAP_HEADER, DRV_CFP_FIELD_SNAP_HEADER,
        3, DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR, 
        {4, 2, 0, 0, 0, 0, 0, 0}},
    {DRV_CFP_QUAL_LLC_HEADER, DRV_CFP_FIELD_LLC_HEADER,
        2, DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR, 
        {3, 1, 0, 0, 0, 0, 0, 0}},
    {DRV_CFP_QUAL_IP6_FLOW_ID, DRV_CFP_FIELD_IP6_FLOW_ID,
        2, DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR, 
        {2, 0, 0, 0, 0, 0, 0, 0}},
    {DRV_CFP_QUAL_INVALID, 0,
        0, 0, {0, 0, 0, 0, 0, 0, 0, 0}} /* Indicate last qualify */
};


typedef struct drv_udf_reg_info_s {
    int udf_id;
    uint32 reg_id;
    uint32 reg_index;
    uint32 reg_field_id;
}drv_udf_reg_info_t;

static  drv_udf_reg_info_t drv53115_udf_reg_info[] = {
    { 0, INDEX(UDF_0_A_0_8r), 0, INDEX(CFG_UDF_0_A_0_8f)},
    { 1, INDEX(UDF_0_A_0_8r), 1, INDEX(CFG_UDF_0_A_0_8f)},
    { 2, INDEX(UDF_0_A_0_8r), 2, INDEX(CFG_UDF_0_A_0_8f)},
    { 3, INDEX(UDF_0_A_0_8r), 3, INDEX(CFG_UDF_0_A_0_8f)},
    { 4, INDEX(UDF_0_A_0_8r), 4, INDEX(CFG_UDF_0_A_0_8f)},
    { 5, INDEX(UDF_0_A_0_8r), 5, INDEX(CFG_UDF_0_A_0_8f)},
    { 6, INDEX(UDF_0_A_0_8r), 6, INDEX(CFG_UDF_0_A_0_8f)},
    { 7, INDEX(UDF_0_A_0_8r), 7, INDEX(CFG_UDF_0_A_0_8f)},
    { 8, INDEX(UDF_0_A_0_8r), 8, INDEX(CFG_UDF_0_A_0_8f)},
    { 9, INDEX(UDF_1_A_0_8r), 0, INDEX(CFG_UDF_1_A_0_8f)},
    { 10, INDEX(UDF_1_A_0_8r), 1, INDEX(CFG_UDF_1_A_0_8f)},
    { 11, INDEX(UDF_1_A_0_8r), 2, INDEX(CFG_UDF_1_A_0_8f)},
    { 12, INDEX(UDF_1_A_0_8r), 3, INDEX(CFG_UDF_1_A_0_8f)},
    { 13, INDEX(UDF_1_A_0_8r), 4, INDEX(CFG_UDF_1_A_0_8f)},
    { 14, INDEX(UDF_1_A_0_8r), 5, INDEX(CFG_UDF_1_A_0_8f)},
    { 15, INDEX(UDF_1_A_0_8r), 6, INDEX(CFG_UDF_1_A_0_8f)},
    { 16, INDEX(UDF_1_A_0_8r), 7, INDEX(CFG_UDF_1_A_0_8f)},
    { 17, INDEX(UDF_1_A_0_8r), 8, INDEX(CFG_UDF_1_A_0_8f)},
    { 18, INDEX(UDF_2_A_0_8r), 0, INDEX(CFG_UDF_2_A_0_8f)},
    { 19, INDEX(UDF_2_A_0_8r), 1, INDEX(CFG_UDF_2_A_0_8f)},
    { 20, INDEX(UDF_2_A_0_8r), 2, INDEX(CFG_UDF_2_A_0_8f)},
    { 21, INDEX(UDF_2_A_0_8r), 3, INDEX(CFG_UDF_2_A_0_8f)},
    { 22, INDEX(UDF_2_A_0_8r), 4, INDEX(CFG_UDF_2_A_0_8f)},
    { 23, INDEX(UDF_2_A_0_8r), 5, INDEX(CFG_UDF_2_A_0_8f)},
    { 24, INDEX(UDF_2_A_0_8r), 6, INDEX(CFG_UDF_2_A_0_8f)},
    { 25, INDEX(UDF_2_A_0_8r), 7, INDEX(CFG_UDF_2_A_0_8f)},
    { 26, INDEX(UDF_2_A_0_8r), 8, INDEX(CFG_UDF_2_A_0_8f)},
    { 27, INDEX(UDF_0_B_0_8r), 0, INDEX(CFG_UDF_0_B_0_8f)},
    { 28, INDEX(UDF_0_B_0_8r), 1, INDEX(CFG_UDF_0_B_0_8f)},
    { 29, INDEX(UDF_0_B_0_8r), 2, INDEX(CFG_UDF_0_B_0_8f)},
    { 30, INDEX(UDF_0_B_0_8r), 3, INDEX(CFG_UDF_0_B_0_8f)},
    { 31, INDEX(UDF_0_B_0_8r), 4, INDEX(CFG_UDF_0_B_0_8f)},
    { 32, INDEX(UDF_0_B_0_8r), 5, INDEX(CFG_UDF_0_B_0_8f)},
    { 33, INDEX(UDF_0_B_0_8r), 6, INDEX(CFG_UDF_0_B_0_8f)},
    { 34, INDEX(UDF_0_B_0_8r), 7, INDEX(CFG_UDF_0_B_0_8f)},
    { 35, INDEX(UDF_0_B_0_8r), 8, INDEX(CFG_UDF_0_B_0_8f)},
    { 36, INDEX(UDF_1_B_0_8r), 0, INDEX(CFG_UDF_1_B_0_8f)},
    { 37, INDEX(UDF_1_B_0_8r), 1, INDEX(CFG_UDF_1_B_0_8f)},
    { 38, INDEX(UDF_1_B_0_8r), 2, INDEX(CFG_UDF_1_B_0_8f)},
    { 39, INDEX(UDF_1_B_0_8r), 3, INDEX(CFG_UDF_1_B_0_8f)},
    { 40, INDEX(UDF_1_B_0_8r), 4, INDEX(CFG_UDF_1_B_0_8f)},
    { 41, INDEX(UDF_1_B_0_8r), 5, INDEX(CFG_UDF_1_B_0_8f)},
    { 42, INDEX(UDF_1_B_0_8r), 6, INDEX(CFG_UDF_1_B_0_8f)},
    { 43, INDEX(UDF_1_B_0_8r), 7, INDEX(CFG_UDF_1_B_0_8f)},
    { 44, INDEX(UDF_1_B_0_8r), 8, INDEX(CFG_UDF_1_B_0_8f)},
    { 45, INDEX(UDF_2_B_0_8r), 0, INDEX(CFG_UDF_2_B_0_8f)},
    { 46, INDEX(UDF_2_B_0_8r), 1, INDEX(CFG_UDF_2_B_0_8f)},
    { 47, INDEX(UDF_2_B_0_8r), 2, INDEX(CFG_UDF_2_B_0_8f)},
    { 48, INDEX(UDF_2_B_0_8r), 3, INDEX(CFG_UDF_2_B_0_8f)},
    { 49, INDEX(UDF_2_B_0_8r), 4, INDEX(CFG_UDF_2_B_0_8f)},
    { 50, INDEX(UDF_2_B_0_8r), 5, INDEX(CFG_UDF_2_B_0_8f)},
    { 51, INDEX(UDF_2_B_0_8r), 6, INDEX(CFG_UDF_2_B_0_8f)},
    { 52, INDEX(UDF_2_B_0_8r), 7, INDEX(CFG_UDF_2_B_0_8f)},
    { 53, INDEX(UDF_2_B_0_8r), 8, INDEX(CFG_UDF_2_B_0_8f)},
    { 54, INDEX(UDF_0_C_0_8r), 0, INDEX(CFG_UDF_0_C_0_8f)},
    { 55, INDEX(UDF_0_C_0_8r), 1, INDEX(CFG_UDF_0_C_0_8f)},
    { 56, INDEX(UDF_0_C_0_8r), 2, INDEX(CFG_UDF_0_C_0_8f)},
    { 57, INDEX(UDF_0_C_0_8r), 3, INDEX(CFG_UDF_0_C_0_8f)},
    { 58, INDEX(UDF_0_C_0_8r), 4, INDEX(CFG_UDF_0_C_0_8f)},
    { 59, INDEX(UDF_0_C_0_8r), 5, INDEX(CFG_UDF_0_C_0_8f)},
    { 60, INDEX(UDF_0_C_0_8r), 6, INDEX(CFG_UDF_0_C_0_8f)},
    { 61, INDEX(UDF_0_C_0_8r), 7, INDEX(CFG_UDF_0_C_0_8f)},
    { 62, INDEX(UDF_0_C_0_8r), 8, INDEX(CFG_UDF_0_C_0_8f)},
    { 63, INDEX(UDF_1_C_0_8r), 0, INDEX(CFG_UDF_1_C_0_8f)},
    { 64, INDEX(UDF_1_C_0_8r), 1, INDEX(CFG_UDF_1_C_0_8f)},
    { 65, INDEX(UDF_1_C_0_8r), 2, INDEX(CFG_UDF_1_C_0_8f)},
    { 66, INDEX(UDF_1_C_0_8r), 3, INDEX(CFG_UDF_1_C_0_8f)},
    { 67, INDEX(UDF_1_C_0_8r), 4, INDEX(CFG_UDF_1_C_0_8f)},
    { 68, INDEX(UDF_1_C_0_8r), 5, INDEX(CFG_UDF_1_C_0_8f)},
    { 69, INDEX(UDF_1_C_0_8r), 6, INDEX(CFG_UDF_1_C_0_8f)},
    { 70, INDEX(UDF_1_C_0_8r), 7, INDEX(CFG_UDF_1_C_0_8f)},
    { 71, INDEX(UDF_1_C_0_8r), 8, INDEX(CFG_UDF_1_C_0_8f)},
    { 72, INDEX(UDF_2_C_0_8r), 0, INDEX(CFG_UDF_1_C_0_8f)},
    { 73, INDEX(UDF_2_C_0_8r), 1, INDEX(CFG_UDF_1_C_0_8f)},
    { 74, INDEX(UDF_2_C_0_8r), 2, INDEX(CFG_UDF_1_C_0_8f)},
    { 75, INDEX(UDF_2_C_0_8r), 3, INDEX(CFG_UDF_1_C_0_8f)},
    { 76, INDEX(UDF_2_C_0_8r), 4, INDEX(CFG_UDF_1_C_0_8f)},
    { 77, INDEX(UDF_2_C_0_8r), 5, INDEX(CFG_UDF_1_C_0_8f)},
    { 78, INDEX(UDF_2_C_0_8r), 6, INDEX(CFG_UDF_1_C_0_8f)},
    { 79, INDEX(UDF_2_C_0_8r), 7, INDEX(CFG_UDF_1_C_0_8f)},
    { 80, INDEX(UDF_2_C_0_8r), 8, INDEX(CFG_UDF_1_C_0_8f)},
    { 81, INDEX(UDF_0_D_0_11r), 0, INDEX(CFG_UDF_0_D_0_11f)},
    { 82, INDEX(UDF_0_D_0_11r), 1, INDEX(CFG_UDF_0_D_0_11f)},
    { 83, INDEX(UDF_0_D_0_11r), 2, INDEX(CFG_UDF_0_D_0_11f)},
    { 84, INDEX(UDF_0_D_0_11r), 3, INDEX(CFG_UDF_0_D_0_11f)},
    { 85, INDEX(UDF_0_D_0_11r), 4, INDEX(CFG_UDF_0_D_0_11f)},
    { 86, INDEX(UDF_0_D_0_11r), 5, INDEX(CFG_UDF_0_D_0_11f)},
    { 87, INDEX(UDF_0_D_0_11r), 6, INDEX(CFG_UDF_0_D_0_11f)},
    { 88, INDEX(UDF_0_D_0_11r), 7, INDEX(CFG_UDF_0_D_0_11f)},
    { 89, INDEX(UDF_0_D_0_11r), 8, INDEX(CFG_UDF_0_D_0_11f)},
    { 90, INDEX(UDF_0_D_0_11r), 9, INDEX(CFG_UDF_0_D_0_11f)},
    { 91, INDEX(UDF_0_D_0_11r), 10, INDEX(CFG_UDF_0_D_0_11f)},
    { 92, INDEX(UDF_0_D_0_11r), 11, INDEX(CFG_UDF_0_D_0_11f)}
};


static drv_cfp_field_map_t  drv_cfp53115_tcam_field_map_table[] = {
    {DRV_CFP_FIELD_VALID, INDEX(VALID_Rf), -1},
    {DRV_CFP_FIELD_SLICE_ID, INDEX(SC_N_IDf), -1},
    {DRV_CFP_FIELD_IN_PBMP, INDEX(SRC_PMAPf), -1},
    {DRV_CFP_FIELD_1QTAGGED, INDEX(C_TAGGEDf), -1},
    {DRV_CFP_FIELD_SPTAGGED, INDEX(S_TAGGEDf), -1},
    {DRV_CFP_FIELD_L2_FRM_FORMAT, INDEX(L2_FRAMINGf), -1},
    {DRV_CFP_FIELD_L3_FRM_FORMAT, INDEX(L3_FRAMINGf), -1},
    {DRV_CFP_FIELD_IP_TOS, INDEX(IP_TOSf), -1},
    {DRV_CFP_FIELD_IP_PROTO, INDEX(IP_PROTOf), -1},
    {DRV_CFP_FIELD_IP_FRAG, INDEX(IP_FRAGf), -1},
    {DRV_CFP_FIELD_IP_NON_FIRST_FRAG, INDEX(NON_FIRST_FRAGf), -1},
    {DRV_CFP_FIELD_IP_AUTH, INDEX(IP_AUTHf), -1},
    {DRV_CFP_FIELD_IP_TTL, INDEX(TTL_RANGEf), -1},
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIHGTER3_SUPPORT)
    {DRV_CFP_FIELD_PPPOE_SESSION_FRM, INDEX(PPPOE_SESSIONf), -1},
#endif /* BCM_NORTHSTARPLUS_SUPPORTT || BCM_STARFIGHTER3_SUPPORT */
    {DRV_CFP_FIELD_SP_PRI, INDEX(SP_PRIf), -1},
    {DRV_CFP_FIELD_SP_CFI, INDEX(SP_CFIf), -1},
    {DRV_CFP_FIELD_SP_VID, INDEX(SP_VIDf), -1},
    {DRV_CFP_FIELD_USR_PRI, INDEX(USR_PRIf), -1},
    {DRV_CFP_FIELD_USR_CFI, INDEX(USR_CFIf), -1},
    {DRV_CFP_FIELD_USR_VID, INDEX(USR_VIDf), -1},
    {DRV_CFP_FIELD_UDFA0_VALID, INDEX(UDF0_VLDf), -1},
    {DRV_CFP_FIELD_UDFA1_VALID, INDEX(UDF1_VLDf), -1},
    {DRV_CFP_FIELD_UDFA2_VALID, INDEX(UDF2_VLDf), -1},
    {DRV_CFP_FIELD_UDFA3_VALID, INDEX(UDF3_VLDf), -1},
    {DRV_CFP_FIELD_UDFA4_VALID, INDEX(UDF4_VLDf), -1},
    {DRV_CFP_FIELD_UDFA5_VALID, INDEX(UDF5_VLDf), -1},
    {DRV_CFP_FIELD_UDFA6_VALID, INDEX(UDF6_VLDf), -1},
    {DRV_CFP_FIELD_UDFA7_VALID, INDEX(UDF7_VLDf), -1},
    {DRV_CFP_FIELD_UDFA8_VALID, INDEX(UDF8_VLDf), -1},
    {DRV_CFP_FIELD_UDFA0, INDEX(UDF_N_A0f), -1},
    {DRV_CFP_FIELD_UDFA1, INDEX(UDF_N_A1f), -1},
    {DRV_CFP_FIELD_UDFA2, INDEX(UDF_N_A2f), -1},
    {DRV_CFP_FIELD_UDFA3, INDEX(UDF_N_A3f), -1},
    {DRV_CFP_FIELD_UDFA4, INDEX(UDF_N_A4f), -1},
    {DRV_CFP_FIELD_UDFA5, INDEX(UDF_N_A5f), -1},
    {DRV_CFP_FIELD_UDFA6, INDEX(UDF_N_A6f), -1},
    {DRV_CFP_FIELD_UDFA7, INDEX(UDF_N_A7f), -1},
    {DRV_CFP_FIELD_UDFA8, INDEX(UDF_N_A8f), -1},
    {DRV_CFP_FIELD_UDFB0_VALID, INDEX(UDF0_VLDf), -1},
    {DRV_CFP_FIELD_UDFB1_VALID, INDEX(UDF1_VLDf), -1},
    {DRV_CFP_FIELD_UDFB2_VALID, INDEX(UDF2_VLDf), -1},
    {DRV_CFP_FIELD_UDFB3_VALID, INDEX(UDF3_VLDf), -1},
    {DRV_CFP_FIELD_UDFB4_VALID, INDEX(UDF4_VLDf), -1},
    {DRV_CFP_FIELD_UDFB5_VALID, INDEX(UDF5_VLDf), -1},
    {DRV_CFP_FIELD_UDFB6_VALID, INDEX(UDF6_VLDf), -1},
    {DRV_CFP_FIELD_UDFB7_VALID, INDEX(UDF7_VLDf), -1},
    {DRV_CFP_FIELD_UDFB8_VALID, INDEX(UDF8_VLDf), -1},
    {DRV_CFP_FIELD_UDFB0, INDEX(UDF_N_B0f), -1},
    {DRV_CFP_FIELD_UDFB1, INDEX(UDF_N_B1f), -1},
    {DRV_CFP_FIELD_UDFB2, INDEX(UDF_N_B2f), -1},
    {DRV_CFP_FIELD_UDFB3, INDEX(UDF_N_B3f), -1},
    {DRV_CFP_FIELD_UDFB4, INDEX(UDF_N_B4f), -1},
    {DRV_CFP_FIELD_UDFB5, INDEX(UDF_N_B5f), -1},
    {DRV_CFP_FIELD_UDFB6, INDEX(UDF_N_B6f), -1},
    {DRV_CFP_FIELD_UDFB7, INDEX(UDF_N_B7f), -1},
    {DRV_CFP_FIELD_UDFB8, INDEX(UDF_N_B8f), -1},
    {DRV_CFP_FIELD_ETYPE, INDEX(ETHERTYPE_SAPf), -1},
    {DRV_CFP_FIELD_UDFC0_VALID, INDEX(UDF0_VLDf), -1},
    {DRV_CFP_FIELD_UDFC1_VALID, INDEX(UDF1_VLDf), -1},
    {DRV_CFP_FIELD_UDFC2_VALID, INDEX(UDF2_VLDf), -1},
    {DRV_CFP_FIELD_UDFC3_VALID, INDEX(UDF3_VLDf), -1},
    {DRV_CFP_FIELD_UDFC4_VALID, INDEX(UDF4_VLDf), -1},
    {DRV_CFP_FIELD_UDFC5_VALID, INDEX(UDF5_VLDf), -1},
    {DRV_CFP_FIELD_UDFC6_VALID, INDEX(UDF6_VLDf), -1},
    {DRV_CFP_FIELD_UDFC7_VALID, INDEX(UDF7_VLDf), -1},
    {DRV_CFP_FIELD_UDFC8_VALID, INDEX(UDF8_VLDf), -1},
    {DRV_CFP_FIELD_UDFC0, INDEX(UDF_N_C0f), -1},
    {DRV_CFP_FIELD_UDFC1, INDEX(UDF_N_C1f), -1},
    {DRV_CFP_FIELD_UDFC2, INDEX(UDF_N_C2f), -1},
    {DRV_CFP_FIELD_UDFC3, INDEX(UDF_N_C3f), -1},
    {DRV_CFP_FIELD_UDFC4, INDEX(UDF_N_C4f), -1},
    {DRV_CFP_FIELD_UDFC5, INDEX(UDF_N_C5f), -1},
    {DRV_CFP_FIELD_UDFC6, INDEX(UDF_N_C6f), -1},
    {DRV_CFP_FIELD_UDFC7, INDEX(UDF_N_C7f), -1},
    {DRV_CFP_FIELD_UDFC8, INDEX(UDF_N_C8f), -1},
    {DRV_CFP_FIELD_CHAIN_ID, INDEX(CHAIN_IDf), -1},
    {DRV_CFP_FIELD_UDFD0_VALID, INDEX(UDF0_VLDf), -1},
    {DRV_CFP_FIELD_UDFD1_VALID, INDEX(UDF1_VLDf), -1},
    {DRV_CFP_FIELD_UDFD2_VALID, INDEX(UDF2_VLDf), -1},
    {DRV_CFP_FIELD_UDFD3_VALID, INDEX(UDF3_VLDf), -1},
    {DRV_CFP_FIELD_UDFD4_VALID, INDEX(UDF4_VLDf), -1},
    {DRV_CFP_FIELD_UDFD5_VALID, INDEX(UDF5_VLDf), -1},
    {DRV_CFP_FIELD_UDFD6_VALID, INDEX(UDF6_VLDf), -1},
    {DRV_CFP_FIELD_UDFD7_VALID, INDEX(UDF7_VLDf), -1},
    {DRV_CFP_FIELD_UDFD8_VALID, INDEX(UDF8_VLDf), -1},
    {DRV_CFP_FIELD_UDFD9_VALID, INDEX(UDF9_VLDf), -1},
    {DRV_CFP_FIELD_UDFD10_VALID, INDEX(UDF10_VLDf), -1},
    {DRV_CFP_FIELD_UDFD11_VALID, INDEX(UDF11_VLDf), -1},
    {DRV_CFP_FIELD_UDFD0, INDEX(UDF_N_D0f), -1},
    {DRV_CFP_FIELD_UDFD1, INDEX(UDF_N_D1f), -1},
    {DRV_CFP_FIELD_UDFD2, INDEX(UDF_N_D2f), -1},
    {DRV_CFP_FIELD_UDFD3, INDEX(UDF_N_D3f), -1},
    {DRV_CFP_FIELD_UDFD4, INDEX(UDF_N_D4f), -1},
    {DRV_CFP_FIELD_UDFD5, INDEX(UDF_N_D5f), -1},
    {DRV_CFP_FIELD_UDFD6, INDEX(UDF_N_D6f), -1},
    {DRV_CFP_FIELD_UDFD7, INDEX(UDF_N_D7f), -1},
    {DRV_CFP_FIELD_UDFD8, INDEX(UDF_N_D8f), -1},
    {DRV_CFP_FIELD_UDFD9, INDEX(UDF_N_D9f), -1},
    {DRV_CFP_FIELD_UDFD10, INDEX(UDF_N_D10f), -1},
    {DRV_CFP_FIELD_UDFD11, INDEX(UDF_N_D11f), -1}
};

static drv_cfp_field_map_t  drv_cfp53115_field_map_table[] = {
    {DRV_CFP_FIELD_CHANGE_DSCP_IB_EN, INDEX(CHANGE_DSCP_IBf), -1},
    {DRV_CFP_FIELD_NEW_DSCP_IB, INDEX(NEW_DSCP_IBf), -1},
    {DRV_CFP_FIELD_CHANGE_DSCP_OB_EN, INDEX(CHANGE_DSCP_OBf), -1},
    {DRV_CFP_FIELD_NEW_DSCP_OB, INDEX(NEW_DSCP_OBf), -1},
    {DRV_CFP_FIELD_CHANGE_FWD_IB_EN, INDEX(CHANGE_FWRD_MAP_IBf), -1},
    {DRV_CFP_FIELD_NEW_FWD_IB, INDEX(DST_MAP_IBf), -1},
    {DRV_CFP_FIELD_CHANGE_FWD_OB_EN, INDEX(CHANGE_FWRD_MAP_OBf), -1},
    {DRV_CFP_FIELD_NEW_FWD_OB, INDEX(DST_MAP_OBf), -1},
    {DRV_CFP_FIELD_CHANGE_TC, INDEX(CHANGE_TCf), -1},
    {DRV_CFP_FIELD_NEW_TC, INDEX(NEW_TCf), -1},
    {DRV_CFP_FIELD_ACTION_CHAIN, INDEX(CHAIN_IDf), -1},
    {DRV_CFP_FIELD_ACTION_LOOPBACK_EN, INDEX(LOOP_BK_ENf), -1},
    {DRV_CFP_FIELD_ACTION_REASON, INDEX(REASON_CODEf), -1},
    {DRV_CFP_FIELD_ACTION_STP_BYPASS, INDEX(STP_BYPf), -1},
    {DRV_CFP_FIELD_ACTION_EAP_BYPASS, INDEX(EAP_BYPf), -1},
    {DRV_CFP_FIELD_ACTION_VLAN_BYPASS, INDEX(VLAN_BYPf), -1},
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    {DRV_CFP_FIELD_ACTION_WRED_DEFAULT, INDEX(RED_DEFAULTf), -1},
    {DRV_CFP_FIELD_ACTION_CHANGE_COLOR, INDEX(CHANGE_COLORf), -1},
    {DRV_CFP_FIELD_ACTION_NEW_COLOR, INDEX(CFP_COLORf), -1},
    {DRV_CFP_FIELD_ACTION_CPCP_RMK_DISABLE, INDEX(CPCP_RMK_DISABLEf), -1},
    {DRV_CFP_FIELD_ACTION_SPCP_RMK_DISABLE, INDEX(SPCP_RMK_DISABLEf), -1},
    {DRV_CFP_FIELD_ACTION_DEI_RMK_DISABLE, INDEX(DEI_RMK_DISABLEf), -1},
    {DRV_CFP_FIELD_ACTION_CPCP_RMK_DISABLE, INDEX(CPCP_RMK_DISABLEf), -1},
    {DRV_CFP_FIELD_ACTION_CHANGE_TC_IN, INDEX(CHANGE_TC_If), -1},
    {DRV_CFP_FIELD_ACTION_NEW_TC_IN, INDEX(NEW_TC_If), -1},
    {DRV_CFP_FIELD_ACTION_CHANGE_TC_OUT, INDEX(CHANGE_TC_Of), -1},
    {DRV_CFP_FIELD_ACTION_NEW_TC_OUT, INDEX(NEW_TC_Of), -1},
    {DRV_CFP_FIELD_ACTION_MAC_LIMIT_BYPASS, INDEX(MAC_LIMIT_BYPASSf), -1},
    {DRV_CFP_FIELD_COLOR_MODE, INDEX(COLOR_BLINDf), -1},
    {DRV_CFP_FIELD_POLICER_ACT, INDEX(POLICER_ACTf), -1},
    {DRV_CFP_FIELD_COUPLING_EN, INDEX(COUPLING_ENf), -1},
    {DRV_CFP_FIELD_POLICER_MODE, INDEX(POLICER_MODEf), -1},
    {DRV_CFP_FIELD_EIR_QUOTA, INDEX(E_QUOTAf), -1},
    {DRV_CFP_FIELD_EIR_BUCKET_SIZE, INDEX(E_LIMITf), -1},
    {DRV_CFP_FIELD_EIR_RATE, INDEX(EIRf), -1},
    {DRV_CFP_FIELD_CIR_QUOTA, INDEX(C_QUOTAf), -1},
    {DRV_CFP_FIELD_CIR_BUCKET_SIZE, INDEX(C_LIMITf), -1},
    {DRV_CFP_FIELD_CIR_RATE, INDEX(CIRf), -1},
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
    {DRV_CFP_FIELD_CURR_QUOTA, INDEX(CURR_QUOTAf), -1},
    {DRV_CFP_FIELD_RATE_REFRESH_EN, INDEX(RATE_REFRESH_ENf), -1},
    {DRV_CFP_FIELD_REF_CAP, INDEX(REF_CAPf), -1},
    {DRV_CFP_FIELD_RATE, INDEX(TOKEN_NUMf), -1},
    {DRV_CFP_FIELD_IB_CNT, INDEX(IN_BAND_CNTf), -1},
    {DRV_CFP_FIELD_OB_CNT, INDEX(OUT_BAND_CNTf), -1},
#if defined(BCM_STARFIGHTER3_SUPPORT)
     /* New definitions to incorporate field name changes in SF3 */
    {DRV_CFP_FIELD_CHANGE_FWD_IB_EN_SF3, INDEX(CHANGE_FWD_IBf), -1},
    {DRV_CFP_FIELD_CHANGE_FWD_OB_EN_SF3, INDEX(CHANGE_FWD_OBf), -1},
    {DRV_CFP_FIELD_ACTION_LOOPBACK_EN_SF3, INDEX(LOOPBACK_ENf), -1},
    {DRV_CFP_FIELD_ACTION_STP_BYPASS_SF3, INDEX(STP_BYPASSf), -1},
    {DRV_CFP_FIELD_ACTION_EAP_BYPASS_SF3, INDEX(EAP_BYPASSf), -1},
    {DRV_CFP_FIELD_ACTION_VLAN_BYPASS_SF3, INDEX(VLAN_BYPASSf), -1},
    {DRV_CFP_FIELD_ACTION_PCP_REMARKING_CTRL, INDEX(PCP_REMARKING_CTRLf), -1},
#endif
};


#if defined(BCM_STARFIGHTER3_SUPPORT)
soc_field_info_t sf3_cfp_act_fields[] = {
    { CFP_COLORf_ROBO, 2, 61, SOCF_LE },
    { CHAIN_IDf_ROBO, 8, 52, SOCF_LE },
    { CHANGE_COLORf_ROBO, 1, 60, 0 },
    { CHANGE_DSCP_IBf_ROBO, 1, 32, 0 },
    { CHANGE_DSCP_OBf_ROBO, 1, 51, 0 },
    { CHANGE_FWD_IBf_ROBO, 2, 24, SOCF_LE },
    { CHANGE_FWD_OBf_ROBO, 2, 43, SOCF_LE },
    { CHANGE_TC_If_ROBO, 1, 13, 0 },
    { CHANGE_TC_Of_ROBO, 1, 65, 0 },
    { DST_MAP_IBf_ROBO, 10, 14, SOCF_LE },
    { DST_MAP_OBf_ROBO, 10, 33, SOCF_LE },
    { EAP_BYPASSf_ROBO, 1, 1, 0 },
    { ECCf_ROBO, 3, 72, SOCF_LE },
    { LOOPBACK_ENf_ROBO, 1, 9, 0 },
    { MAC_LIMIT_BYPASSf_ROBO, 1, 64, 0 },
    { NEW_DSCP_IBf_ROBO, 6, 26, SOCF_LE },
    { NEW_DSCP_OBf_ROBO, 6, 45, SOCF_LE },
    { NEW_TC_If_ROBO, 3, 10, SOCF_LE },
    { NEW_TC_Of_ROBO, 3, 66, SOCF_LE },
    { PARITYf_ROBO, 1, 75, 0 },
    { PCP_REMARKING_CTRLf_ROBO, 3, 69, SOCF_LE },
    { REASON_CODEf_ROBO, 6, 3, SOCF_LE },
    { RED_DEFAULTf_ROBO, 1, 63, 0 },
    { STP_BYPASSf_ROBO, 1, 2, 0 },
    { VLAN_BYPASSf_ROBO, 1, 0, 0 }
};
soc_field_info_t sf3_cfp_meter_fields[] = {
    { CIRf_ROBO, 19, 192, SOCF_LE },
    { COLOR_BLINDf_ROBO, 1, 0, 0 },
    { COUPLING_ENf_ROBO, 1, 2, 0 },
    { C_LIMITf_ROBO, 20, 160, SOCF_LE },
    { C_QUOTAf_ROBO, 23, 128, SOCF_LE },
    { EIRf_ROBO, 19, 96, SOCF_LE },
    { E_LIMITf_ROBO, 20, 64, SOCF_LE },
    { E_QUOTAf_ROBO, 23, 32, SOCF_LE },
    { POLICER_ACTf_ROBO, 1, 1, 0 },
    { POLICER_MODEf_ROBO, 2, 3, SOCF_LE },
    { RESERVED0f_ROBO, 27, 5, SOCF_LE|SOCF_RES },
    { RESERVED1f_ROBO, 9, 55, SOCF_LE|SOCF_RES },
    { RESERVED2f_ROBO, 12, 84, SOCF_LE|SOCF_RES },
    { RESERVED3f_ROBO, 13, 115, SOCF_LE|SOCF_RES },
    { RESERVED4f_ROBO, 9, 151, SOCF_LE|SOCF_RES },
    { RESERVED5f_ROBO, 12, 180, SOCF_LE|SOCF_RES },
    { RESERVED6f_ROBO, 13, 211, SOCF_LE|SOCF_RES }
};

static int
_drv_sf3_cfp_fieldinfo_get(int unit, uint32 mem_type, uint32 field_id, uint32 *fieldinfo_ptr)
{
    int i;
    int rv = SOC_E_NONE;

    if (mem_type ==  DRV_CFP_RAM_ACT) {
        for(i = 0; i < COUNTOF(sf3_cfp_act_fields) ; i++) {
            if (sf3_cfp_act_fields[i].field == field_id) {
                *fieldinfo_ptr = (uint32)&sf3_cfp_act_fields[i];
                break;
            }
        }
        if ( i == COUNTOF(sf3_cfp_act_fields)) {
            rv = SOC_E_BADID;
        }
    } else if (mem_type == DRV_CFP_RAM_METER) {
        for(i = 0; i < COUNTOF(sf3_cfp_meter_fields) ; i++) {
            if (sf3_cfp_meter_fields[i].field == field_id) {
                *fieldinfo_ptr = (uint32)&sf3_cfp_meter_fields[i];
                break;
            }
        }
        if ( i == COUNTOF(sf3_cfp_meter_fields)) {
            rv = SOC_E_BADID;
        }
    }
    return rv;
}
#endif




/*
 * Function: _drv_gex_cfp_field_mapping
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
_drv_gex_cfp_field_mapping(int unit, uint32 mem_type, uint32 field_type, uint32 *field_id)
{
    int i;
    int rv = SOC_E_NONE;

    if ((mem_type ==  DRV_CFP_RAM_TCAM) || (mem_type == DRV_CFP_RAM_TCAM_MASK)) {
        for(i = 0; i < COUNTOF(drv_cfp53115_tcam_field_map_table) ; i++) {           
            if (drv_cfp53115_tcam_field_map_table[i].cfp_field_type == field_type) {
                *field_id = drv_cfp53115_tcam_field_map_table[i].field_id;
                break;
            }
        }
        if ( i == COUNTOF(drv_cfp53115_tcam_field_map_table)) {
            rv = SOC_E_BADID;
        }
    } else {
        for(i = 0; i < COUNTOF(drv_cfp53115_field_map_table) ; i++) {
            if (drv_cfp53115_field_map_table[i].cfp_field_type == field_type) {
                *field_id = drv_cfp53115_field_map_table[i].field_id;
                break;
            }
        }
        if ( i == COUNTOF(drv_cfp53115_field_map_table)) {
            rv = SOC_E_BADID;
        }
    }

    return rv;
}


int
_drv_gex_cfp_qual_by_udf(int unit, int *slice_id, uint32 field_type)
{
    int i;

    for(i=0; i < CFP_53115_UDF_NUM_MAX; i++) {
        if ((drv53115_udf_info[i].valid) && 
            (drv53115_udf_info[i].sub_field == field_type) && 
            (drv53115_udf_info[i].slice_id == *slice_id)) {
            return TRUE;
        }
    }

    if (*slice_id == CFP_53115_IPV6_CHAIN_SLICE_ID) {
        /* Check if this qualifier in main slice (0) */
        *slice_id = 4;
        for(i=0; i < CFP_53115_UDF_NUM_MAX; i++) {
            if ((drv53115_udf_info[i].valid) && 
                (drv53115_udf_info[i].sub_field == field_type) && 
                (drv53115_udf_info[i].slice_id == *slice_id)) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

int
_drv_gex_cfp_field_in_chain_slice(uint32 field_type)
{
    int rv = FALSE;
    
    switch (field_type) {
        case DRV_CFP_FIELD_CHAIN_ID:
        case DRV_CFP_FIELD_UDFD0_VALID:
        case DRV_CFP_FIELD_UDFD0:
        case DRV_CFP_FIELD_UDFD1_VALID:
        case DRV_CFP_FIELD_UDFD1:
        case DRV_CFP_FIELD_UDFD2_VALID:
        case DRV_CFP_FIELD_UDFD2:
        case DRV_CFP_FIELD_UDFD3_VALID:
        case DRV_CFP_FIELD_UDFD3:
        case DRV_CFP_FIELD_UDFD4_VALID:
        case DRV_CFP_FIELD_UDFD4:
        case DRV_CFP_FIELD_UDFD5_VALID:
        case DRV_CFP_FIELD_UDFD5:
        case DRV_CFP_FIELD_UDFD6_VALID:
        case DRV_CFP_FIELD_UDFD6:
        case DRV_CFP_FIELD_UDFD7_VALID:
        case DRV_CFP_FIELD_UDFD7:
        case DRV_CFP_FIELD_UDFD8_VALID:
        case DRV_CFP_FIELD_UDFD8:
        case DRV_CFP_FIELD_UDFD9_VALID:
        case DRV_CFP_FIELD_UDFD9:
        case DRV_CFP_FIELD_UDFD10_VALID:
        case DRV_CFP_FIELD_UDFD10:
        case DRV_CFP_FIELD_UDFD11_VALID:
        case DRV_CFP_FIELD_UDFD11:
            rv = TRUE;
            break;
        default:
            break;
    }

    return rv;
}

/*
 * Function: _drv_gex_cfp_meter_rate2chip
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
_drv_gex_cfp_meter_rate2chip(int unit, uint32 kbits_sec, uint32 *chip_val)
{
    /* case 1 : greater than 100M */
    if (kbits_sec > 1000 * 100 ) {
        *chip_val = (kbits_sec / (1000 * 8)) + 115;
        if (*chip_val > 240) {
            *chip_val = 240; /* max value */
        }
    } else if (kbits_sec > 1792) {
    /* case 2 : greater than 1792K */
        *chip_val = (kbits_sec / 1000 ) + 27;
    } else {
    /* case 3 : the rest (64K ~ 1792K) */
        *chip_val = kbits_sec / 64;
    }
}

/*
 * Function: _drv_gex_cfp_meter_chip2rate
 *
 * Purpose:
 *     Translate the register value to driver rate value.
 *
 * Parameters:
 *     unit - BCM device number
 *     kbits_sec(OUT) - driver rate value
 *     chip_val - register value
 *
 * Returns:
 *     Nothing
 */
void
_drv_gex_cfp_meter_chip2rate(int unit, uint32 *kbits_sec, uint32 chip_val)
{
    if (chip_val > 127) {
        *kbits_sec = (chip_val - 115) * 8 * 1000;
    } else if (chip_val > 28) {
        *kbits_sec = (chip_val - 27) * 1000;
    } else {
        *kbits_sec = chip_val * 64;
    }
}

/*
 * Function: _drv_gex_cfp_meter_burst2chip
 *
 * Purpose:
 *     Translate the driver burst value to register value.
 *
 * Parameters:
 *     unit - BCM device number
 *     kbits_burst - driver burst value
 *     chip_val(OUT) - chip value
 *
 * Returns:
 *     Nothing
 */
void
_drv_gex_cfp_meter_burst2chip(int unit, uint32 kbits_burst, uint32 *chip_val)
{
    uint32  burst;
    int i;
    
    burst = CFP_53115_METER_BURST_MAX;
    for (i = 0; i < 11; i++) {
        if (kbits_burst >= burst) {
            *chip_val = i;
            break;
        }
        burst = burst / 2;
    }
}

/*
 * Function: _drv_gex_cfp_meter_chip2burst
 *
 * Purpose:
 *     Translate the register value to driver burst value.
 *
 * Parameters:
 *     unit - BCM device number
 *     kbits_burst(OUT) - driver burst value
 *     chip_val - register value
 *
 * Returns:
 *     Nothing
 */
void
_drv_gex_cfp_meter_chip2burst(int unit, uint32 *kbits_burst, uint32 chip_val)
{
    uint32 burst = CFP_53115_METER_BURST_MAX;
    int i;

    for (i = 0; i < chip_val ; i++) {
        burst = burst / 2;
    }
    *kbits_burst = burst ;
}

/*
 * Function: _drv_gex_cfp_read
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
_drv_gex_cfp_read(int unit, uint32 ram_type, 
                         uint32 index, drv_cfp_entry_t *entry)
{
    int i, rv = SOC_E_NONE;
    uint32  mem_id  = 0;
    int ram_val, retry;
    uint32 reg_val, fld_val;
    int index_max, mem_len;
    uint32  *data_p, *mask_p, data_reg_addr;

    assert(entry);
    
    switch (ram_type) {
        case DRV_CFP_RAM_ACT:
            if (SOC_IS_STARFIGHTER3(unit)) {
                mem_id = INDEX(CFP_ACTm);
            } else {
                mem_id = INDEX(CFP_ACT_POLm);
            }
            ram_val = CFP_53115_RAM_SEL_ACT;
            break;
        case DRV_CFP_RAM_METER:
            mem_id = INDEX(CFP_METERm);
            ram_val = CFP_53115_RAM_SEL_METER;
            break;
        case DRV_CFP_RAM_TCAM:
            assert(entry->tcam_mask);
            mem_id = INDEX(CFP_TCAM_IPV4_SCm);
            ram_val = CFP_53115_RAM_SEL_TCAM;
            break;
        default:
            rv = SOC_E_PARAM;
            return rv;
    }

    index_max = soc_robo_mem_index_max(unit, mem_id);
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "_drv_cfp_read : mem_id = %d, ram_val = %d, index_max = %d\n"),
                 mem_id, ram_val, index_max));
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /*
     * Perform TCAM read operation 
     */

    MEM_LOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
    if (REG_READ_CFP_ACCr(unit, &reg_val) < 0) {
        goto cfp_read_exit;
    }
    fld_val = CFP_53115_OP_READ;
    soc_CFP_ACCr_field_set(unit, &reg_val, 
        OP_SELf, &fld_val);

    fld_val = ram_val;
    soc_CFP_ACCr_field_set(unit, &reg_val, 
        RAM_SELf, &fld_val);

    fld_val = index;
    soc_CFP_ACCr_field_set(unit, &reg_val, 
        XCESS_ADDRf, &fld_val);

    fld_val = 1;
    soc_CFP_ACCr_field_set(unit, &reg_val, 
        OP_STR_DONEf, &fld_val);

    if (REG_WRITE_CFP_ACCr(unit, &reg_val) < 0) {
        goto cfp_read_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if (REG_READ_CFP_ACCr(unit, &reg_val) < 0) {
            goto cfp_read_exit;
        }
        soc_CFP_ACCr_field_get(unit, &reg_val, 
            OP_STR_DONEf, &fld_val);
        if (!fld_val) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto cfp_read_exit;
    }

    switch (ram_type) {
    case DRV_CFP_RAM_TCAM:
        mem_len = soc_mem_entry_words(unit, mem_id);
        if (entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_MAIN) {
            data_p = entry->cfp_chain->tcam_data;
            mask_p = entry->cfp_chain->tcam_mask;
        } else {
            data_p = entry->tcam_data;
            mask_p = entry->tcam_mask;
        }
        for (i = 0; i < mem_len; i++) {
            if ((rv = REG_READ_CFP_DATAr(unit, i, &reg_val)) < 0) {
                goto cfp_read_exit;
            }
            *(data_p + i) = reg_val;
            
            if ((rv = REG_READ_CFP_MASKr(unit, i, &reg_val)) < 0) {
                goto cfp_read_exit;
            }
            *(mask_p + i) = reg_val;
        }
        break;
    case DRV_CFP_RAM_ACT:
#ifdef BCM_STARFIGHTER3_SUPPORT
        if (SOC_IS_STARFIGHTER3(unit)) {
            mem_len = CFP_53134_RAM_ACT_REG_SIZE;
        } else {
#endif /* BCM_STARFIGHTER3_SUPPORT */
            mem_len = soc_mem_entry_words(unit, mem_id);
#ifdef BCM_STARFIGHTER3_SUPPORT
        }
#endif /* BCM_STARFIGHTER3_SUPPORT */
        if (entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_MAIN) {
            data_p = entry->cfp_chain->act_data;
        } else {
            data_p = entry->act_data;
        }
        data_reg_addr = DRV_REG_ADDR(
            unit, INDEX(ACT_POL_DATA0r), 0, 0);
        for (i = 0; i < mem_len; i++) {
            if ((rv = DRV_REG_READ(unit, 
                (data_reg_addr + i * 4), &reg_val, 4)) < 0) {
                goto cfp_read_exit;
            }
            *(data_p + i) = reg_val;
        }
        break;
    case DRV_CFP_RAM_METER:
#ifdef BCM_STARFIGHTER3_SUPPORT
        if (SOC_IS_STARFIGHTER3(unit)) {
            mem_len = CFP_53134_RAM_METER_REG_SIZE;
        } else {
#endif /* BCM_STARFIGHTER3_SUPPORT */
            mem_len = soc_mem_entry_words(unit, mem_id);
#ifdef BCM_STARFIGHTER3_SUPPORT
        }
#endif /* BCM_STARFIGHTER3_SUPPORT */
        data_p = entry->meter_data;
        data_reg_addr = DRV_REG_ADDR(
            unit, INDEX(RATE_METER0r), 0, 0);
        for (i = 0; i < mem_len; i++) {
            if ((rv = DRV_REG_READ(unit, 
                (data_reg_addr + i * 4), &reg_val, 4)) < 0) {
                goto cfp_read_exit;
            }
            *(data_p + i) = reg_val;
        }
        break;
    }

    cfp_read_exit:
        MEM_UNLOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
    return rv;
}

/*
 * Function: _drv_gex_cfp_stat_read
 *
 * Purpose:
 *     Read the counter raw data from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     counter type - in-band/ out-band
 *     index -entry index
 *     entry(OUT) -counter raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv_gex_cfp_stat_read(int unit, uint32 counter_type, 
                                uint32 index, uint32 *counter)
{
    int rv = SOC_E_NONE;
    uint32  mem_id  = 0;
    int ram_val, retry;
    uint32 reg_val, fld_val;
    int index_max;

    assert(counter);
    
    switch (counter_type) {
        case DRV_CFP_RAM_STAT_IB:
            mem_id = INDEX(CFP_STAT_IBm);
            ram_val = CFP_53115_RAM_SEL_IB_STAT;
            break;
        case DRV_CFP_RAM_STAT_OB:
            mem_id = INDEX(CFP_STAT_OBm);
            ram_val = CFP_53115_RAM_SEL_OB_STAT;
            break;
        default:
            rv = SOC_E_PARAM;
            return rv;
    }

    index_max = soc_robo_mem_index_max(unit, mem_id);
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /*
     * Perform read operation 
     */

    MEM_LOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
    if (REG_READ_CFP_ACCr(unit, &reg_val) < 0) {
        goto cfp_stat_read_exit;
    }
    fld_val = CFP_53115_OP_READ;
    soc_CFP_ACCr_field_set(unit, &reg_val, 
        OP_SELf, &fld_val);
    
    fld_val = ram_val;
    soc_CFP_ACCr_field_set(unit, &reg_val, 
        RAM_SELf, &fld_val);

    fld_val = index;
    soc_CFP_ACCr_field_set(unit, &reg_val, 
        XCESS_ADDRf, &fld_val);

    fld_val = 1;
    soc_CFP_ACCr_field_set(unit, &reg_val, 
        OP_STR_DONEf, &fld_val);

    if (REG_WRITE_CFP_ACCr(unit, &reg_val) < 0) {
        goto cfp_stat_read_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if (REG_READ_CFP_ACCr(unit, &reg_val) < 0) {
            goto cfp_stat_read_exit;
        }
        soc_CFP_ACCr_field_get(unit, &reg_val, 
            OP_STR_DONEf, &fld_val);
        if (!fld_val) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto cfp_stat_read_exit;
    }
    
    switch (counter_type) {
    case DRV_CFP_RAM_STAT_IB:
        if ((rv = REG_READ_RATE_INBANDr(unit,&reg_val)) < 0) {
            goto cfp_stat_read_exit;
        }
        *counter = reg_val;
        break;
    case DRV_CFP_RAM_STAT_OB:
        if ((rv = REG_READ_RATE_OUTBANDr(unit,&reg_val)) < 0) {
            goto cfp_stat_read_exit;
        }
        *counter = reg_val;
        break;
    }

    cfp_stat_read_exit:
        MEM_UNLOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
    return rv;
}

/*
 * Function: _drv_gex_cfp_stat_write
 *
 * Purpose:
 *     Set the counter raw data to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     counter type - in-band/ out-band
 *     index -entry index
 *     entry -counter raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv_gex_cfp_stat_write(int unit, uint32 counter_type, 
                                      uint32 index, uint32 counter)
{
    int rv = SOC_E_NONE;
    uint32  mem_id  = 0;
    int ram_val, retry;
    uint32 reg_val, fld_val;
    int index_max;
    uint32  data_reg_val;
    
    switch (counter_type) {
        case DRV_CFP_RAM_STAT_IB:
            mem_id = INDEX(CFP_STAT_IBm);
            ram_val = CFP_53115_RAM_SEL_IB_STAT;
            break;
        case DRV_CFP_RAM_STAT_OB:
            mem_id = INDEX(CFP_STAT_OBm);
            ram_val = CFP_53115_RAM_SEL_OB_STAT;
            break;
        default:
            rv = SOC_E_PARAM;
            return rv;
    }

    index_max = soc_robo_mem_index_max(unit, mem_id);
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /*
     * Perform write operation 
     */

    MEM_LOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
    if (REG_READ_CFP_ACCr(unit, &reg_val) < 0) {
        goto cfp_stat_write_exit;
    }
    fld_val = CFP_53115_OP_WRITE;
    soc_CFP_ACCr_field_set(unit, &reg_val, 
        OP_SELf, &fld_val);
    
    fld_val = ram_val;
    soc_CFP_ACCr_field_set(unit, &reg_val, 
        RAM_SELf, &fld_val);

    fld_val = index;
    soc_CFP_ACCr_field_set(unit, &reg_val, 
        XCESS_ADDRf, &fld_val);

    fld_val = 1;
    soc_CFP_ACCr_field_set(unit, &reg_val, 
        OP_STR_DONEf, &fld_val);

    /* Set counter value */
    data_reg_val = counter;
    switch (counter_type) {
    case DRV_CFP_RAM_STAT_IB:
        if ((rv = REG_WRITE_RATE_INBANDr(unit,&data_reg_val)) < 0) {
            goto cfp_stat_write_exit;
        }
        break;
    case DRV_CFP_RAM_STAT_OB:
        if ((rv = REG_WRITE_RATE_OUTBANDr(unit,&data_reg_val)) < 0) {
            goto cfp_stat_write_exit;
        }
        break;
    }

    if (REG_WRITE_CFP_ACCr(unit, &reg_val) < 0) {
        goto cfp_stat_write_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if (REG_READ_CFP_ACCr(unit, &reg_val) < 0) {
            goto cfp_stat_write_exit;
        }
        soc_CFP_ACCr_field_get(unit, &reg_val, 
            OP_STR_DONEf, &fld_val);
        if (!fld_val) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto cfp_stat_write_exit;
    }

    cfp_stat_write_exit:
        MEM_UNLOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
    return rv;
}


/*
 * Function: _drv_gex_cfp_tcam_search
 *
 * Purpose:
 *     Search the Valid TCAM raw data
 *
 * Parameters:
 *     unit - BCM device number
 *     flags - search flags
 *     index(OUT) -entry index
 *     entry(OUT) -CFP entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv_gex_cfp_tcam_search(int unit, uint32 flags, uint32* index, 
            drv_cfp_entry_t *entry)
{
    int i, rv = SOC_E_NONE;
    uint32 reg_val, fld_val = 0;
    int mem_len;
    uint32  *data_p, *mask_p;

    assert(entry);
    assert(index);

    MEM_LOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
    if ((rv = REG_READ_CFP_ACCr(unit, &reg_val)) < 0) {
        goto cfp_search_exit;
    }

    /* Get SEARCH DONE bit */
    if (flags & DRV_CFP_SEARCH_DONE) {
        soc_CFP_ACCr_field_get(unit, &reg_val, 
            OP_STR_DONEf, &fld_val);
        *index = (fld_val)? 0 : 1;
        goto cfp_search_exit;
    }

    /* Get valid TCAM entry */
    if(flags & DRV_CFP_SEARCH_GET) {
        /*
         * set error times ???
         */
         
        soc_CFP_ACCr_field_get(unit, &reg_val, 
            SERCH_STSf, &fld_val);
        if (fld_val) {
            soc_CFP_ACCr_field_get(unit, &reg_val, 
                XCESS_ADDRf, &fld_val);
            *index = fld_val;
            mem_len = soc_mem_entry_words(unit, INDEX(CFP_TCAM_IPV4_SCm));
            data_p = entry->tcam_data;
            mask_p = entry->tcam_mask;
            
            for (i = 0; i < mem_len; i++) {
                if ((rv = REG_READ_CFP_DATAr(unit, i, &reg_val)) < 0) {
                    goto cfp_search_exit;
                }
                *(mask_p + i) = reg_val;
                if ((rv = REG_READ_CFP_MASKr(unit, i, &reg_val)) < 0) {
                    goto cfp_search_exit;
                }
                *(data_p + i) = reg_val;
            }
            rv = SOC_E_NONE;
        } else {
            rv = SOC_E_EMPTY;
        }
        goto cfp_search_exit;
    }


    if (flags & DRV_CFP_SEARCH_START) {
        /* Set search op code */
        fld_val = CFP_53115_OP_SEARCH_VALID;
        soc_CFP_ACCr_field_set(unit, &reg_val, 
            OP_SELf, &fld_val);
        /* Set TCAM */
        fld_val = CFP_53115_RAM_SEL_TCAM;
        soc_CFP_ACCr_field_set(unit, &reg_val, 
            RAM_SELf, &fld_val);
        /* Set initial search address */
        fld_val = 0;
        soc_CFP_ACCr_field_set(unit, &reg_val, 
            XCESS_ADDRf, &fld_val);

        /* start search */
        fld_val = 1;
        soc_CFP_ACCr_field_set(unit, &reg_val, 
            OP_STR_DONEf, &fld_val);
        if ((rv = REG_WRITE_CFP_ACCr(unit, &reg_val)) < 0) {
            goto cfp_search_exit;
        }
    }
    
    cfp_search_exit:
        MEM_UNLOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
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
_drv_gex_cfp_write(int unit, uint32 ram_type, 
                        uint32 index, drv_cfp_entry_t *entry, 
                        int bypass_index_check)
{
    int i, rv = SOC_E_NONE;
    uint32  mem_id = 0;
    int ram_val, retry;
    uint32 reg_acc, reg_val, fld_val;
    int index_max, mem_len;
    uint32  *data_p, *mask_p, data_reg_addr;

    assert(entry);
    
    switch (ram_type) {
        case DRV_CFP_RAM_ACT:
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                mem_id = INDEX(CFP_ACTm);
            } else 
#endif /* BCM_STARFIGHTER3_SUPPORT */
            {
                mem_id = INDEX(CFP_ACT_POLm);
            }
            ram_val = CFP_53115_RAM_SEL_ACT;
            break;
        case DRV_CFP_RAM_METER:
            mem_id = INDEX(CFP_METERm);
            ram_val = CFP_53115_RAM_SEL_METER;
            break;
        case DRV_CFP_RAM_STAT_IB:
            mem_id = INDEX(CFP_STAT_IBm);
            ram_val = CFP_53115_RAM_SEL_IB_STAT;
            break;
        case DRV_CFP_RAM_STAT_OB:
            mem_id = INDEX(CFP_STAT_OBm);
            ram_val = CFP_53115_RAM_SEL_OB_STAT;
            break;
        case DRV_CFP_RAM_TCAM:
        case DRV_CFP_RAM_TCAM_INVALID:
            mem_id = INDEX(CFP_TCAM_IPV4_SCm);
            ram_val = CFP_53115_RAM_SEL_TCAM;
            break;
        default:
            rv = SOC_E_UNAVAIL;
            return rv;
    }

    if (!bypass_index_check) {
        index_max = soc_robo_mem_index_max(unit, mem_id);
        if (index > index_max) {
            rv = SOC_E_PARAM;
            return rv;
        }
    }

    /*
     * Perform TCAM read operation 
     */

    MEM_LOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
    if ((rv = REG_READ_CFP_ACCr(unit, &reg_acc)) < 0) {
        goto cfp_write_exit;
    }
    fld_val = CFP_53115_OP_WRITE;
    soc_CFP_ACCr_field_set(unit, &reg_acc, 
        OP_SELf, &fld_val);

    fld_val = index;
    soc_CFP_ACCr_field_set(unit, &reg_acc, 
        XCESS_ADDRf, &fld_val);

    switch (ram_type) {
    case DRV_CFP_RAM_TCAM:
        mem_len = soc_mem_entry_words(unit, mem_id);
        if (entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_MAIN) {
            data_p = entry->cfp_chain->tcam_data;
            mask_p = entry->cfp_chain->tcam_mask;
        } else {
            data_p = entry->tcam_data;
            mask_p = entry->tcam_mask;
        }
        for (i = 0; i < mem_len; i++) {
            reg_val = *(data_p + i);
            if ((REG_WRITE_CFP_DATAr(unit, i, &reg_val)) < 0) {
                goto cfp_write_exit;
            }
            reg_val = *(mask_p + i);
            if ((REG_WRITE_CFP_MASKr(unit, i, &reg_val)) < 0) {
                goto cfp_write_exit;
            }
        }
        break;
    case DRV_CFP_RAM_TCAM_INVALID:
        mem_len = soc_mem_entry_words(unit, mem_id);
        data_p = entry->tcam_data;
        mask_p = entry->tcam_mask;
        /* write the first word which contain the valid field */
        reg_val = *(data_p);
        if ((REG_WRITE_CFP_DATAr(unit, 0, &reg_val)) < 0) {
            goto cfp_write_exit;
        }
        reg_val = *(mask_p);
        if ((REG_WRITE_CFP_MASKr(unit, 0, &reg_val)) < 0) {
            goto cfp_write_exit;
        }
        break;
    case DRV_CFP_RAM_ACT:
#ifdef BCM_STARFIGHTER3_SUPPORT
        if (SOC_IS_STARFIGHTER3(unit)) {
            mem_len = CFP_53134_RAM_ACT_REG_SIZE;
        } else {
#endif /* BCM_STARFIGHTER3_SUPPORT */
            mem_len = soc_mem_entry_words(unit, mem_id);
#if defined(BCM_STARFIGHTER3_SUPPORT)
        }
#endif /* BCM_STARFIGHTER3_SUPPORT */
        if (entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_MAIN) {
            data_p = entry->cfp_chain->act_data;
        } else {
            data_p = entry->act_data;
        }
        data_reg_addr = DRV_REG_ADDR(unit, INDEX(ACT_POL_DATA0r), 0, 0);
        for (i = 0; i < mem_len; i++) {
            reg_val = *(data_p + i);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (data_reg_addr + i * 4), &reg_val, 4)) < 0) {
                goto cfp_write_exit;
            }
        }
        break;
    case DRV_CFP_RAM_METER:
        data_p = entry->meter_data;
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                mem_len = CFP_53134_RAM_METER_REG_SIZE;
            } else {
#endif /* BCM_STARFIGHTER3_SUPPORT */
                mem_len = soc_mem_entry_words(unit, mem_id);
#if defined(BCM_STARFIGHTER3_SUPPORT)
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            data_reg_addr = DRV_REG_ADDR(unit, INDEX(RATE_METER0r), 0, 0);
            for (i = 0; i < mem_len; i++) {
                reg_val = *(data_p + i);
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, (data_reg_addr + i * 4), &reg_val, 4)) < 0) {
                    goto cfp_write_exit;
                }
            }
           
        } else {
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
            reg_val = *data_p;
            if ((rv = REG_WRITE_RATE_METER0r(unit, &reg_val)) < 0) {
                goto cfp_write_exit;
            }
            reg_val = *(data_p + 1);
            /* 
             * This bit controls global rate meter refresh.
             * It should be always enabled. 
             */
            fld_val = 1;
            soc_RATE_METER1r_field_set(unit, &reg_val, 
                RATE_REFRESH_ENf,  &fld_val);
            if ((rv = REG_WRITE_RATE_METER1r(unit, &reg_val)) < 0) {
                goto cfp_write_exit;
            }
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        }
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
        break;
    default:
        rv = SOC_E_PARAM;
        goto cfp_write_exit;
    }

    fld_val = ram_val;
    soc_CFP_ACCr_field_set(unit, &reg_acc, 
        RAM_SELf, &fld_val);
    fld_val = 1;
    soc_CFP_ACCr_field_set(unit, &reg_acc, 
        OP_STR_DONEf, &fld_val);

    if ((rv = REG_WRITE_CFP_ACCr(unit, &reg_acc)) < 0) {
        goto cfp_write_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_CFP_ACCr(unit, &reg_acc)) < 0) {
            goto cfp_write_exit;
        }
        soc_CFP_ACCr_field_get(unit, &reg_acc, 
            OP_STR_DONEf, &fld_val);
        if (!fld_val) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto cfp_write_exit;
    }

    cfp_write_exit:
        MEM_UNLOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
    return rv;
}

/*
 * Function: drv53115_cfp_init
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
drv_gex_cfp_init(int unit)
{
    int port, i, slice_id;
    pbmp_t pbm;
    uint32  slice[(DRV_CFP_QUAL_COUNT / 32) + 1];
    int     *qset;
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    uint32 temp, reg_val = 0;
    int rv, index_max;
    drv_cfp_entry_t null_entry;
#endif /* BCM_53125 || BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT
        */

    /* Enable CFP */
    pbm = PBMP_E_ALL(unit);
    _SHR_PBMP_PORT_REMOVE(pbm, CMIC_PORT(unit));
    PBMP_ITER(pbm, port) {
         (DRV_SERVICES(unit)->cfp_control_set)
            (unit, DRV_CFP_ENABLE, port, 1);
    }
    /* Clear TCAM Table */
    (DRV_SERVICES(unit)->cfp_control_set)
        (unit, DRV_CFP_TCAM_RESET, 0, 0);
    /* Clear Other RAM Table */
    (DRV_SERVICES(unit)->cfp_control_set)
        (unit, DRV_CFP_OTHER_RAM_CLEAR, 0, 0);

#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) || 
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        /* 
         * The TCAM should be filled first 
         * otherwise it will cause the parity error 
         */
        sal_memset(&null_entry, 0, sizeof(null_entry));
        index_max = soc_robo_mem_index_max(unit, INDEX(CFP_TCAM_IPV4_SCm));
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)
        if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
            /* Polar has 256 TCAM entries, but expose 128 entries to customer only.
            * For TCAM parity check, we still need to clear all 256 entries before enabling 
            *  the TCAM checksum function. */
            index_max = 255;
            for (i = 0; i <= index_max; i++) {
                _drv_gex_cfp_write(unit,
                    DRV_CFP_RAM_TCAM, i, &null_entry, TRUE);
            }
        } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */        
        {
            for (i = 0; i <= index_max; i++) {
                _drv_gex_cfp_write(unit,
                    DRV_CFP_RAM_TCAM, i, &null_entry, FALSE);
            }
        }
        rv = REG_READ_TCAM_CTRLr(unit, &reg_val);
        if (rv< 0) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_drv_cfp_init : Failed to read the TCAM control regsiter.\n")));
        }
        /* Enable TCAM checksum */
        temp = 1;
        soc_TCAM_CTRLr_field_set(unit, &reg_val, 
            EN_TCAM_CHKSUMf, &temp);
        rv = REG_WRITE_TCAM_CTRLr(unit, &reg_val);
        if (rv< 0) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_drv_cfp_init : Failed to write the TCAM control regsiter.\n")));
        }
    }
#endif /* BCM_STARFIGHTER_SUPPORT || BCM_POLAR_SUPPORT || 
        * BCM_NORTHSTAR _SUPPORT ||  BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT
        */
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {

        /* Disable all meters */
        index_max = soc_robo_mem_index_max(unit, INDEX(CFP_METERm));
        temp = 0x3; /* Disable mode */

        /* coverity[callee_ptr_arith] */
        SOC_IF_ERROR_RETURN(
            drv_gex_cfp_field_set(unit, DRV_CFP_RAM_METER, 
                DRV_CFP_FIELD_POLICER_MODE, &null_entry, &temp));
        for (i=0; i < index_max; i++) {
            SOC_IF_ERROR_RETURN(
                drv_gex_cfp_entry_write(unit, i, DRV_CFP_RAM_METER ,
                &null_entry));
        }
        
        /* Enable flobal meter */
        SOC_IF_ERROR_RETURN(
            REG_READ_RATE_METER_GLOBAL_CTLr(unit, &reg_val));
        temp = 1;
        soc_RATE_METER_GLOBAL_CTLr_field_set(unit, &reg_val,
            RATE_REFRESH_ENf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_RATE_METER_GLOBAL_CTLr(unit, &reg_val));
        
    }
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */

    /* Slice information initialization */
    sal_memset(&drv53115_slice_info[0], 0,
        sizeof(drv_slice_info_t) * (CFP_53115_SLICE_MAX_ID + 1));

    

    for (slice_id = 0; slice_id <= CFP_53115_SLICE_MAX_ID; slice_id++) {
        switch(slice_id) {
            case 0:
                qset = s0_qset;
                break;
            case 1:
                qset = s1_qset;
                break;
            case 2:
                qset = s2_qset;
                break;
            case 4:
                qset = s4_qset;
                break;
            case 5:
                qset = s5_qset;
                break;
            case 6:
                qset = s6_qset;
                break;
            case 7:
                qset = s7_qset;
                break;
            case 12:
                qset = s12_qset;
                break;
            case 13:
                qset = s13_qset;
                break;
            case 14:
                qset = s14_qset;
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
            drv53115_slice_info[slice_id].qset[i] = slice[i];
        }

        drv53115_slice_info[slice_id].slice_id = slice_id;

        if (slice_id == 7) { /* Chain Slice */
            drv53115_slice_info[slice_id].slice_udf_free = 12;
        } else {
            drv53115_slice_info[slice_id].slice_udf_free = 9;
        }
        
    }

    /* UDFs initialization */
    for (i = 0; i < CFP_53115_UDF_NUM_MAX; i++) {
        /* coverity[negative_sink] */
        drv53115_udf_info[i].valid = 0;
        drv53115_udf_info[i].length = 16; /* 16 Bits */
        /* coverity[check_after_sink] */
        if ((i >= 0) && (i < 9)) {
            drv53115_udf_info[i].slice_id = 0;
            drv53115_udf_info[i].self_field = (DRV_CFP_FIELD_UDFA0 + (i % 9));
            drv53115_udf_info[i].valid_field = (DRV_CFP_FIELD_UDFA0_VALID+ (i % 9));
        } else if ((i >= 9) && (i < 18)) {
            drv53115_udf_info[i].slice_id = 1;
            drv53115_udf_info[i].self_field = (DRV_CFP_FIELD_UDFA0 + (i % 9));
            drv53115_udf_info[i].valid_field = (DRV_CFP_FIELD_UDFA0_VALID+ (i % 9));
        } else if ((i >= 18) && (i < 27)) {
            drv53115_udf_info[i].slice_id = 2;
            drv53115_udf_info[i].self_field = (DRV_CFP_FIELD_UDFA0 + (i % 9));
            drv53115_udf_info[i].valid_field = (DRV_CFP_FIELD_UDFA0_VALID+ (i % 9));
        } else if ((i >= 27) && (i < 36)) {
            drv53115_udf_info[i].slice_id = 4;
            drv53115_udf_info[i].self_field = (DRV_CFP_FIELD_UDFB0 + (i % 9));
            drv53115_udf_info[i].valid_field = (DRV_CFP_FIELD_UDFB0_VALID+ (i % 9));
        } else if ((i >= 36) && (i < 45)) {
            drv53115_udf_info[i].slice_id = 5;
            drv53115_udf_info[i].self_field = (DRV_CFP_FIELD_UDFB0 + (i % 9));
            drv53115_udf_info[i].valid_field = (DRV_CFP_FIELD_UDFB0_VALID+ (i % 9));
        } else if ((i >= 45) && (i < 54)) {
            drv53115_udf_info[i].slice_id = 6;
            drv53115_udf_info[i].self_field = (DRV_CFP_FIELD_UDFB0 + (i % 9));
            drv53115_udf_info[i].valid_field = (DRV_CFP_FIELD_UDFB0_VALID+ (i % 9));
        } else if ((i >= 54) && (i < 63)) {
            drv53115_udf_info[i].slice_id = 12;
            drv53115_udf_info[i].self_field = (DRV_CFP_FIELD_UDFC0 + (i % 9));
            drv53115_udf_info[i].valid_field = (DRV_CFP_FIELD_UDFC0_VALID+ (i % 9));
        } else if ((i >= 63) && (i < 72)) {
            drv53115_udf_info[i].slice_id = 13;
            drv53115_udf_info[i].self_field = (DRV_CFP_FIELD_UDFC0 + (i % 9));
            drv53115_udf_info[i].valid_field = (DRV_CFP_FIELD_UDFC0_VALID+ (i % 9));
        } else if ((i >= 72) && (i < 81)) {
            drv53115_udf_info[i].slice_id = 14;
            drv53115_udf_info[i].self_field = (DRV_CFP_FIELD_UDFC0 + (i % 9));
            drv53115_udf_info[i].valid_field = (DRV_CFP_FIELD_UDFC0_VALID+ (i % 9));
        } else if ((i >= 81) && (i < 93)) {
            drv53115_udf_info[i].slice_id = 7;
            drv53115_udf_info[i].self_field = (DRV_CFP_FIELD_UDFD0 + (i - 81));
            drv53115_udf_info[i].valid_field = (DRV_CFP_FIELD_UDFD0_VALID+ (i - 81));
        }
    }

    
    return SOC_E_NONE;
}

/*
 * Function: drv_gex_cfp_action_get
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
 * Note:
 *     The action types were divided into 2 groups.
 *     One is to changed priority and the other is switch behavior 
 *     (drop/flood/append/redirect). So the user configure the input paramter,
 *     'action', to decide which action type you need to get. 
 *     action                              act_param
 *     DRV_CFP_ACT_IB_MOD_INT_PRI     Priority
 *     DRV_CFP_ACT_IB_MOD_INT_PRI_CANCEL     XXX
 *     DRV_CFP_ACT_IB_NONE           XXX
 *     DRV_CFP_ACT_IB_REDIRECT     port number
 *     DRV_CFP_ACT_IB_APPEND     port number
 *     DRV_CFP_ACT_IB_FLOOD     XXX
 *     DRV_CFP_ACT_IB_DROP     XXX
 */
int
drv_gex_cfp_action_get(int unit, uint32* action, 
            drv_cfp_entry_t* entry, uint32* act_param)
{
    int rv = SOC_E_NONE;
    uint32  fld_val = 0;
    uint32 cfp_field_type;
    switch (*action) {
        case DRV_CFP_ACT_IB_NONE:
        case DRV_CFP_ACT_IB_REDIRECT:
        case DRV_CFP_ACT_IB_APPEND:
        case DRV_CFP_ACT_IB_FLOOD:
        case DRV_CFP_ACT_IB_DROP:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        case DRV_CFP_ACT_IB_REMOVE:
#endif /* BCM_POLAR_SUPPORT | BCM_NORTHSTAR_SUPPORT | NS+ | SF3 */
            cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_IB_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_IB_EN_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            switch (fld_val) {
                case 0: /* None action */
                    *action = DRV_CFP_ACT_IB_NONE;
                    break;
                case 1 : /* Remove */
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                    if(SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                        *action = DRV_CFP_ACT_IB_REMOVE;
                        (DRV_SERVICES(unit)->cfp_field_get)(unit,  
                            DRV_CFP_RAM_ACT, DRV_CFP_FIELD_NEW_FWD_IB, 
                            entry, &fld_val);
                        if (fld_val & 0x80) {
                            /* Bit 7 indicated IMP port */
                            *act_param = fld_val | 0x100;
                        } else {
                            *act_param = fld_val;
                        }
                    } else 
#endif /* BCM_POLAR_SUPPORT | BCM_NORTHSTAR_SUPPORT | NS+ | SF3 */
                    {
                        rv = SOC_E_INTERNAL;
                    }
                    break;
                case 2: /* Replace */
                    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                        DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
                    if (fld_val == CFP_53115_DEST_DROP) {
                        *action = DRV_CFP_ACT_IB_DROP;
                    } else if (fld_val == CFP_53115_DEST_FLOOD) {
                        *action = DRV_CFP_ACT_IB_FLOOD;
                    } else {
                        *action = DRV_CFP_ACT_IB_REDIRECT;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                        if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))
                            && (fld_val & 0x80)) {
                            /* Bit 7 indicated IMP port */
                            *act_param = fld_val | 0x100;
                        } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ || SF3 */
                        if (fld_val & 0x40) {
                            /* Bit 6 indicated IMP port */
                            *act_param = fld_val | 0x100;
                        } else {
                            *act_param = fld_val;
                        }
                    }
                    break;
                case 3: /* Append */
                    *action = DRV_CFP_ACT_IB_APPEND;
                    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                        DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))
                        && (fld_val & 0x80)) {
                        /* Bit 7 indicated IMP port */
                        *act_param = fld_val | 0x100;
                    } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ || SF3 */
                    if (fld_val & 0x40) {
                        /* Bit 6 indicated IMP port */
                        *act_param = fld_val | 0x100;
                    } else {
                        *act_param = fld_val;
                    }
                    break;
                default:
                    rv = SOC_E_INTERNAL;
                    break;
            }
            break;
            
        case DRV_CFP_ACT_OB_NONE:
        case DRV_CFP_ACT_OB_REDIRECT:
        case DRV_CFP_ACT_OB_APPEND:
        case DRV_CFP_ACT_OB_FLOOD:
        case DRV_CFP_ACT_OB_DROP:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        case DRV_CFP_ACT_OB_REMOVE:
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ || SF3*/
            cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_OB_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_OB_EN_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            switch (fld_val) {
                case 0: /* None action */
                    *action = DRV_CFP_ACT_OB_NONE;
                    break;
                case 1 : /* Remove */
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                    if(SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                        *action = DRV_CFP_ACT_OB_REMOVE;
                        (DRV_SERVICES(unit)->cfp_field_get)(unit,  
                            DRV_CFP_RAM_ACT, DRV_CFP_FIELD_NEW_FWD_OB, 
                            entry, &fld_val);
                        if (fld_val & 0x80) {
                            /* Bit 7 indicated IMP port */
                            *act_param = fld_val | 0x100;
                        } else {
                            *act_param = fld_val;
                        }
                    } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ || SF3 */
                    {
                        rv = SOC_E_INTERNAL;
                    }
                    break;
                case 2: /* Replace */
                    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                        DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
                    if (fld_val == CFP_53115_DEST_DROP) {
                        *action = DRV_CFP_ACT_OB_DROP;
                    } else if (fld_val == CFP_53115_DEST_FLOOD) {
                        *action = DRV_CFP_ACT_OB_FLOOD;
                    } else {
                        *action = DRV_CFP_ACT_OB_REDIRECT;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                        if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))
                            && (fld_val & 0x80)) {
                            /* Bit 7 indicated IMP port */
                            *act_param = fld_val | 0x100;
                        } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ || SF3 */
                        if (fld_val & 0x40) {
                            /* Bit 6 indicated IMP port */
                            *act_param = fld_val | 0x100;
                        } else {
                            *act_param = fld_val;
                        }
                    }
                    break;
                case 3: /* Append */
                    *action = DRV_CFP_ACT_OB_APPEND;
                    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                        DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))
                        && (fld_val & 0x80)) {
                        /* Bit 7 indicated IMP port */
                        *act_param = fld_val | 0x100;
                    } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT  || NS+ || SF3 */
                    if (fld_val & 0x40) {
                        /* Bit 6 indicated IMP port */
                        *act_param = fld_val | 0x100;
                    } else {
                        *act_param = fld_val;
                    }
                    break;
                default:
                    rv = SOC_E_INTERNAL;
                    break;
            }
            break;
            
        case DRV_CFP_ACT_CHANGE_TC:
        case DRV_CFP_ACT_CHANGE_TC_CANCEL:
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT,
                    DRV_CFP_FIELD_ACTION_CHANGE_TC_IN, entry, &fld_val);
            } else {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT,
                    DRV_CFP_FIELD_CHANGE_TC, entry, &fld_val);
            }
            if (fld_val == 0) {
                *action = DRV_CFP_ACT_CHANGE_TC_CANCEL; 
            } else {
                *action = DRV_CFP_ACT_CHANGE_TC;
                if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT,
                        DRV_CFP_FIELD_NEW_TC, entry, &fld_val);
                } else {
                    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT,
                            DRV_CFP_FIELD_NEW_TC, entry, &fld_val);
                }
                *act_param = fld_val;
            }
            break;
            
        case DRV_CFP_ACT_CHAIN_ID:
        case DRV_CFP_ACT_CLASSFICATION_ID:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_CHAIN, entry, &fld_val);
            *act_param = fld_val;
            break;

        case DRV_CFP_ACT_LOOPBACK:
            cfp_field_type = DRV_CFP_FIELD_ACTION_LOOPBACK_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_ACTION_LOOPBACK_EN_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            if (fld_val) {
                *act_param = TRUE;
            } else {
                *act_param = FALSE;
            }
            break;
            
        case DRV_CFP_ACT_REASON_CODE:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_REASON, entry, &fld_val);
            *act_param = fld_val;
            break;
            
        case DRV_CFP_ACT_STP_BYPASS:
            cfp_field_type = DRV_CFP_FIELD_ACTION_STP_BYPASS;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_ACTION_STP_BYPASS_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            if (fld_val) {
                *act_param = TRUE;
            } else {
                *act_param = FALSE;
            }
            break;
            
        case DRV_CFP_ACT_EAP_BYPASS:
            cfp_field_type = DRV_CFP_FIELD_ACTION_EAP_BYPASS;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_ACTION_EAP_BYPASS_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            if (fld_val) {
                *act_param = TRUE;
            } else {
                *act_param = FALSE;
            }
            break;
            
        case DRV_CFP_ACT_VLAN_BYPASS:
            cfp_field_type = DRV_CFP_FIELD_ACTION_VLAN_BYPASS;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_ACTION_VLAN_BYPASS_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            if (fld_val) {
                *act_param = TRUE;
            } else {
                *act_param = FALSE;
            }
            break;
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        case DRV_CFP_ACT_IB_DSCP_NEW:
        case DRV_CFP_ACT_IB_DSCP_CANCEL:
            DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_DSCP_IB_EN, entry, &fld_val);
            if (fld_val) {
                DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_DSCP_IB, entry, &fld_val);
                *act_param = fld_val;
            } else {
                *act_param = DRV_CFP_ACT_IB_DSCP_CANCEL;
            }
            break;
        case DRV_CFP_ACT_OB_DSCP_NEW:
        case DRV_CFP_ACT_OB_DSCP_CANCEL:
            DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_DSCP_OB_EN, entry, &fld_val);
            if (fld_val) {
                DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_DSCP_OB, entry, &fld_val);
                *act_param = fld_val;
            } else {
                *act_param = DRV_CFP_ACT_OB_DSCP_CANCEL;
            }
            break;
        case DRV_CFP_ACT_WRED_DEFAULT:
            DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_WRED_DEFAULT, entry, &fld_val);
            *act_param = fld_val;
            break;
        case DRV_CFP_ACT_CHANGE_COLOR:
        case DRV_CFP_ACT_CHANGE_COLOR_CANCEL:
            DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_CHANGE_COLOR, entry, &fld_val);
            if (fld_val) {
                DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_NEW_COLOR, entry, &fld_val);
                if (fld_val == 0x0) {
                    *act_param = DRV_FIELD_COLOR_GREEN;
                } else if (fld_val == 0x1) {
                    *act_param = DRV_FIELD_COLOR_YELLOW;
                } else if (fld_val == 0x2) {
                    *act_param = DRV_FIELD_COLOR_RED;
                } else {
                    return (SOC_E_INTERNAL);
                }
            } else {
                *act_param = DRV_CFP_ACT_CHANGE_COLOR_CANCEL;
            }
            break;
        case DRV_CFP_ACT_DEI_RMK_DISABLE:
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_PCP_REMARKING_CTRL, entry, &fld_val);
                if (fld_val & CFP_53134_DEI_RMK_BIT) {
                    *act_param = TRUE;
                } else {
                    *act_param = FALSE;
                }
            } else
#endif /* BCM_STARFIGHTER3_SUPPORT */
            {
                DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_DEI_RMK_DISABLE, entry, &fld_val);
                if (fld_val) {
                    *act_param = TRUE;
                } else {
                    *act_param = FALSE;
                }
            }
            break;
        case DRV_CFP_ACT_CPCP_RMK_DISABLE:
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_PCP_REMARKING_CTRL, entry, &fld_val);
                if (fld_val & CFP_53134_CPCP_RMK_BIT) {
                    *act_param = TRUE;
                } else {
                    *act_param = FALSE;
                }
            } else
#endif /* BCM_STARFIGHTER3_SUPPORT */
            {
                DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_CPCP_RMK_DISABLE, entry, &fld_val);
                if (fld_val) {
                    *act_param = TRUE;
                } else {
                    *act_param = FALSE;
                }
            }
            break;
        case DRV_CFP_ACT_SPCP_RMK_DISABLE:
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_PCP_REMARKING_CTRL, entry, &fld_val);
                if (fld_val & CFP_53134_SPCP_RMK_BIT) {
                    *act_param = TRUE;
                } else {
                    *act_param = FALSE;
                }
            } else
#endif /* BCM_STARFIGHTER3_SUPPORT */
            {
                DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_SPCP_RMK_DISABLE, entry, &fld_val);
                if (fld_val) {
                    *act_param = TRUE;
                } else {
                    *act_param = FALSE;
                }
            }
            break;
        case DRV_CFP_ACT_CHANGE_TC_OUTPUT:
        case DRV_CFP_ACT_CHANGE_TC_OUTPUT_CANCEL:
            DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_CHANGE_TC_OUT, entry, &fld_val);
            if (fld_val) {
                DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_NEW_TC_OUT, entry, &fld_val);
                *act_param = fld_val;
            } else {
                *act_param = DRV_CFP_ACT_CHANGE_TC_OUTPUT_CANCEL;
            }
            break;
        case DRV_CFP_ACT_MAC_LIMIT_DROP_BYPASS:
            DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_MAC_LIMIT_BYPASS, entry, &fld_val);
            if (fld_val) {
                *act_param = TRUE;
            } else {
                *act_param = FALSE;
            }
            break;
#endif /* NS+ || SF3 */
            
        default:
            rv = SOC_E_PARAM;
    }
    
    return rv;
}

/*
 * Function: drv_gex_cfp_action_set
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
 *     DRV_CFP_ACT_IB_MOD_INT_PRI     Priority
 *     DRV_CFP_ACT_IB_MOD_INT_PRI_CANCEL     XXX
 *     DRV_CFP_ACT_IB_NONE           XXX
 *     DRV_CFP_ACT_IB_REDIRECT     port number
 *     DRV_CFP_ACT_IB_APPEND     port number
 *     DRV_CFP_ACT_IB_FLOOD     XXX
 *     DRV_CFP_ACT_IB_DROP     XXX
 */
int
drv_gex_cfp_action_set(int unit, uint32 action, 
            drv_cfp_entry_t* entry, uint32 act_param1, uint32 act_param2)
{
    int rv = SOC_E_NONE;
    uint32  fld_val, tmp_act;
    uint32 cfp_field_type;
    assert(entry);
    
    switch (action) {
        case DRV_CFP_ACT_IB_NONE:
            fld_val = 0;
            cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_IB_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_IB_EN_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            /* action */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_REMOVE:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_IB_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_IB_EN_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                fld_val = 1;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    cfp_field_type, entry, &fld_val);
                if (act_param1 & 0x100) {
                    /* Bit 7 indicate port 8. (IMP port) */
                    fld_val = (act_param1 | 0x80) & 0xff;
                } else {
                    fld_val = act_param1;
                }
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
            } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ || SF3 */
            {
                rv = SOC_E_UNAVAIL;
            }
            break;            
        case DRV_CFP_ACT_IB_REDIRECT:
            fld_val = 2;
            cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_IB_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_IB_EN_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))
                && (act_param1 & 0x100)) {
                /* Bit 7 indicate port 8. (IMP port) */
                fld_val = (act_param1 | 0x80) & 0xff;
            } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ || SF3 */
            if (act_param1 & 0x100) {
                /* Bit 6 indicate port 8. (IMP port) */
                fld_val = (act_param1 | 0x40) & 0xff;
            } else {
                fld_val = act_param1;
            }
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
            break;
            
        case DRV_CFP_ACT_IB_APPEND:
            fld_val = 3;
            cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_IB_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_IB_EN_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit)|| SOC_IS_STARFIGHTER3(unit))
                && (act_param1 & 0x100)) {
                /* Bit 7 indicate port 8. (IMP port) */
                fld_val = (act_param1 | 0x80) & 0xff;
            } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ || SF3 */
            if (act_param1 & 0x100) {
                /* Bit 6 indicate port 8. (IMP port) */
                fld_val = (act_param1 | 0x40) & 0xff;
            } else {
                fld_val = act_param1;
            }
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_FLOOD:
            fld_val = 2;
            cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_IB_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_IB_EN_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            fld_val = CFP_53115_DEST_FLOOD;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
            break;
            
        case DRV_CFP_ACT_IB_DROP:
            fld_val = 2;
            cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_IB_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_IB_EN_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            fld_val = CFP_53115_DEST_DROP;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_OB_NONE:
            fld_val = 0;
            cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_OB_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_OB_EN_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            /* action */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            break;
        case DRV_CFP_ACT_OB_REMOVE:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                fld_val = 1;
                cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_OB_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
                if (SOC_IS_STARFIGHTER3(unit)) {
                    cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_OB_EN_SF3;
                }
#endif /* BCM_STARFIGHTER3_SUPPORT */
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    cfp_field_type, entry, &fld_val);
                if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit))
                    && (act_param1 & 0x100)) {
                    /* Bit 7 indicate port 8. (IMP port) */
                    fld_val = (act_param1 | 0x80) & 0xff;
                } else {
                    fld_val = act_param1;
                }
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
            } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ || SF3 */
            {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_CFP_ACT_OB_REDIRECT:
            fld_val = 2;
            cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_OB_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_OB_EN_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))
                && (act_param1 & 0x100)) {
                /* Bit 7 indicate port 8. (IMP port) */
                fld_val = (act_param1 | 0x80) & 0xff;
            } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ || SF3 */
            if (act_param1 & 0x100) {
                /* Bit 6 indicate port 8. (IMP port) */
                fld_val = (act_param1 | 0x40) & 0xff;
            } else {
                fld_val = act_param1;
            }
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
            break;
            
        case DRV_CFP_ACT_OB_APPEND:
            fld_val = 3;
            cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_OB_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_OB_EN_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) &&
                (act_param1 & 0x100)) {
                /* Bit 7 indicate port 8. (IMP port) */
                fld_val = (act_param1 | 0x80) & 0xff;
            } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ || SF3 */
            if (act_param1 & 0x100) {
                /* Bit 6 indicate port 8. (IMP port) */
                fld_val = (act_param1 | 0x40) & 0xff;
            } else {
                fld_val = act_param1;
            }
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
            break;
            
        case DRV_CFP_ACT_OB_FLOOD:
            fld_val = 2;
            cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_OB_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_OB_EN_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            fld_val = CFP_53115_DEST_FLOOD;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_OB_DROP:
            fld_val = 2;
            cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_OB_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_CHANGE_FWD_OB_EN_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            fld_val = CFP_53115_DEST_DROP;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
            break;
            
        case DRV_CFP_ACT_IB_DSCP_NEW:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_DSCP_IB_EN, entry, &fld_val);
            fld_val = act_param1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_DSCP_IB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_DSCP_CANCEL:
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_DSCP_IB_EN, entry, &fld_val);
            break;
            
        case DRV_CFP_ACT_OB_DSCP_NEW:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_DSCP_OB_EN, entry, &fld_val);
            fld_val = act_param1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_DSCP_OB, entry, &fld_val);
            break;
            
        case DRV_CFP_ACT_OB_DSCP_CANCEL:
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_DSCP_OB_EN, entry, &fld_val);
            break;

        case DRV_CFP_ACT_CLASSFICATION_ID:
            fld_val = act_param1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_CHAIN, entry, &fld_val);
            break;
        case DRV_CFP_ACT_CHAIN_ID:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_CHAIN, entry, &fld_val);
            break;
            
        case DRV_CFP_ACT_CHANGE_TC:
            fld_val = 1;
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_CHANGE_TC_IN, entry, &fld_val);
                fld_val = act_param1;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_NEW_TC_IN, entry, &fld_val);
            } else {
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_CHANGE_TC, entry, &fld_val);
                fld_val = act_param1;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_TC, entry, &fld_val);
            }
            break;
            
        case DRV_CFP_ACT_CHANGE_TC_CANCEL:
            fld_val = 0;
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_CHANGE_TC_IN, entry, &fld_val);
            } else {
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_CHANGE_TC, entry, &fld_val);
            }
            break;
            
        case DRV_CFP_ACT_LOOPBACK:
            if (act_param1 == TRUE) {
                fld_val = 1;
            } else {
                fld_val = 0;
            }
            cfp_field_type = DRV_CFP_FIELD_ACTION_LOOPBACK_EN;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_ACTION_LOOPBACK_EN_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            break;
            
        case DRV_CFP_ACT_REASON_CODE:
            fld_val = act_param1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_REASON, entry, &fld_val);
            break;
            
        case DRV_CFP_ACT_STP_BYPASS:
            if (act_param1 == TRUE) {
                fld_val = 1;
            } else {
                fld_val = 0;
            }
            cfp_field_type = DRV_CFP_FIELD_ACTION_STP_BYPASS;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_ACTION_STP_BYPASS_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            break;
            
        case DRV_CFP_ACT_EAP_BYPASS:
            if (act_param1 == TRUE) {
                fld_val = 1;
            } else {
                fld_val = 0;
            }
            cfp_field_type = DRV_CFP_FIELD_ACTION_EAP_BYPASS;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_ACTION_EAP_BYPASS_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            break;
            
        case DRV_CFP_ACT_VLAN_BYPASS:
            if (act_param1 == TRUE) {
                fld_val = 1;
            } else {
                fld_val = 0;
            }
            cfp_field_type = DRV_CFP_FIELD_ACTION_VLAN_BYPASS;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_ACTION_VLAN_BYPASS_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            break;
            
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        case DRV_CFP_ACT_WRED_DEFAULT:
            fld_val = act_param1;
            DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_WRED_DEFAULT, entry, &fld_val);
            break;
        case DRV_CFP_ACT_CHANGE_COLOR:
            fld_val = 1;
            DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_CHANGE_COLOR, entry, &fld_val);
            switch (act_param1) {
                case DRV_FIELD_COLOR_GREEN:
                    fld_val = 0;
                    break;
                case DRV_FIELD_COLOR_YELLOW:
                    fld_val = 1;
                    break;
                case DRV_FIELD_COLOR_RED:
                    fld_val = 2;
                    break;
                default:
                    return (SOC_E_PARAM);
            }
            DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_NEW_COLOR, entry, &fld_val);
            break;
        case DRV_CFP_ACT_CHANGE_COLOR_CANCEL:
            fld_val = 0;
            DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_CHANGE_COLOR, entry, &fld_val);
            break;
        case DRV_CFP_ACT_DEI_RMK_DISABLE:
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                fld_val = 0;
                DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_PCP_REMARKING_CTRL, entry, &fld_val);
                if (act_param1) {
                    fld_val = fld_val | CFP_53134_DEI_RMK_BIT;
                } else {
                    /* Clear bit 2 of PCP_REMARKING_CTRL , i.e., DEI field */
                    fld_val = fld_val & (~CFP_53134_DEI_RMK_BIT &
                                CFP_53134_PCP_RMK_CTRL_MASK);
                }
            } else
#endif /* BCM_STARFIGHTER3_SUPPORT */
            { 
                fld_val = act_param1;
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_DEI_RMK_DISABLE, entry, &fld_val);
            }
            break;
        case DRV_CFP_ACT_CPCP_RMK_DISABLE:
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                fld_val = 0;
                DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_PCP_REMARKING_CTRL, entry, &fld_val);
                if (act_param1) {
                    fld_val = fld_val | CFP_53134_CPCP_RMK_BIT;
                } else {
                    /* Clear bit 1 of PCP_REMARKING_CTRL , i.e., CPCP field */
                    fld_val = fld_val & (~CFP_53134_CPCP_RMK_BIT &
                                CFP_53134_PCP_RMK_CTRL_MASK);
                }
            } else
#endif /* BCM_STARFIGHTER3_SUPPORT */
            {
                fld_val = act_param1;
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_CPCP_RMK_DISABLE, entry, &fld_val);
            }
            break;
        case DRV_CFP_ACT_SPCP_RMK_DISABLE:
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                fld_val = 0;
                DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_PCP_REMARKING_CTRL, entry, &fld_val);
                if (act_param1) {
                    fld_val = fld_val | CFP_53134_SPCP_RMK_BIT;
                } else {
                    /* Clear bit 0 of PCP_REMARKING_CTRL , i.e., SPCP field */
                    fld_val = fld_val & (~CFP_53134_SPCP_RMK_BIT &
                                CFP_53134_PCP_RMK_CTRL_MASK);
                }
            } else 
#endif /* BCM_STARFIGHTER3_SUPPORT */
            {
                fld_val = act_param1;
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ACTION_SPCP_RMK_DISABLE, entry, &fld_val);
            }
            break;
        case DRV_CFP_ACT_CHANGE_TC_OUTPUT:
            fld_val = 1;
            DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_CHANGE_TC_OUT, entry, &fld_val);
            fld_val = act_param1;
            DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_NEW_TC_OUT, entry, &fld_val);
            break;
        case DRV_CFP_ACT_CHANGE_TC_OUTPUT_CANCEL:
            fld_val = 0;
            DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_CHANGE_TC_OUT, entry, &fld_val);
            break;
        case DRV_CFP_ACT_MAC_LIMIT_DROP_BYPASS:
            fld_val = 1;
            DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ACTION_MAC_LIMIT_BYPASS, entry, &fld_val);
            break;
#endif /* NS+ || SF3 */
        default:
            rv = SOC_E_UNAVAIL;
    }

    /* Handle the bypss options */

    switch (action) {
        case DRV_CFP_ACT_IB_NONE:
            /* Check the out-band action */
            tmp_act = DRV_CFP_ACT_OB_NONE;
            drv_gex_cfp_action_get(unit, &tmp_act, entry, &fld_val);
            if (tmp_act == DRV_CFP_ACT_OB_NONE) {
                fld_val = 0;

                cfp_field_type = DRV_CFP_FIELD_ACTION_VLAN_BYPASS;
#if defined(BCM_STARFIGHTER3_SUPPORT)
                if (SOC_IS_STARFIGHTER3(unit)) {
                    cfp_field_type = DRV_CFP_FIELD_ACTION_VLAN_BYPASS_SF3;
                }
#endif /* BCM_STARFIGHTER3_SUPPORT */
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT,
                    cfp_field_type, entry, &fld_val);

                cfp_field_type = DRV_CFP_FIELD_ACTION_STP_BYPASS;
#if defined(BCM_STARFIGHTER3_SUPPORT)
                if (SOC_IS_STARFIGHTER3(unit)) {
                    cfp_field_type = DRV_CFP_FIELD_ACTION_STP_BYPASS_SF3;
                }
#endif /* BCM_STARFIGHTER3_SUPPORT */
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT,
                    cfp_field_type, entry, &fld_val);

                cfp_field_type = DRV_CFP_FIELD_ACTION_EAP_BYPASS;
#if defined(BCM_STARFIGHTER3_SUPPORT)
                if (SOC_IS_STARFIGHTER3(unit)) {
                    cfp_field_type = DRV_CFP_FIELD_ACTION_EAP_BYPASS_SF3;
                }
#endif /* BCM_STARFIGHTER3_SUPPORT */
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT,
                    cfp_field_type, entry, &fld_val);
            }
            break;
        case DRV_CFP_ACT_OB_NONE:
            /* Check the in-band action */
            tmp_act = DRV_CFP_ACT_IB_NONE;
            drv_gex_cfp_action_get(unit, &tmp_act, entry, &fld_val);
            if (tmp_act == DRV_CFP_ACT_IB_NONE) {
                fld_val = 0;

                cfp_field_type = DRV_CFP_FIELD_ACTION_VLAN_BYPASS;
#if defined(BCM_STARFIGHTER3_SUPPORT)
                if (SOC_IS_STARFIGHTER3(unit)) {
                    cfp_field_type = DRV_CFP_FIELD_ACTION_VLAN_BYPASS_SF3;
                }
#endif /* BCM_STARFIGHTER3_SUPPORT */
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT,
                    cfp_field_type, entry, &fld_val);

                cfp_field_type = DRV_CFP_FIELD_ACTION_STP_BYPASS;
#if defined(BCM_STARFIGHTER3_SUPPORT)
                if (SOC_IS_STARFIGHTER3(unit)) {
                    cfp_field_type = DRV_CFP_FIELD_ACTION_STP_BYPASS_SF3;
                }
#endif /* BCM_STARFIGHTER3_SUPPORT */
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT,
                    cfp_field_type, entry, &fld_val);

                cfp_field_type = DRV_CFP_FIELD_ACTION_EAP_BYPASS;
#if defined(BCM_STARFIGHTER3_SUPPORT)
                if (SOC_IS_STARFIGHTER3(unit)) {
                    cfp_field_type = DRV_CFP_FIELD_ACTION_EAP_BYPASS_SF3;
                }
#endif /* BCM_STARFIGHTER3_SUPPORT */
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT,
                    cfp_field_type, entry, &fld_val);
            }
            break;
        case DRV_CFP_ACT_IB_REDIRECT:
        case DRV_CFP_ACT_IB_APPEND:
        case DRV_CFP_ACT_IB_FLOOD:
        case DRV_CFP_ACT_OB_REDIRECT:
        case DRV_CFP_ACT_OB_APPEND:
        case DRV_CFP_ACT_OB_FLOOD:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        case DRV_CFP_ACT_IB_REMOVE:
        case DRV_CFP_ACT_OB_REMOVE:
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ || SF3 */
            fld_val = 1;

            cfp_field_type = DRV_CFP_FIELD_ACTION_VLAN_BYPASS;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_ACTION_VLAN_BYPASS_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);

            cfp_field_type = DRV_CFP_FIELD_ACTION_STP_BYPASS;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_ACTION_STP_BYPASS_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);

            cfp_field_type = DRV_CFP_FIELD_ACTION_EAP_BYPASS;
#if defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_STARFIGHTER3(unit)) {
                cfp_field_type = DRV_CFP_FIELD_ACTION_EAP_BYPASS_SF3;
            }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                cfp_field_type, entry, &fld_val);
            break;
        default:
            break;
    }

    return rv;
}

/*
 * Function: drv_gex_cfp_control_get
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
drv_gex_cfp_control_get(int unit, uint32 control_type, uint32 param1, 
            uint32 *param2)
{
    uint32  fld_32 = 0, reg_val;

    switch (control_type) {
        case DRV_CFP_ENABLE:
            SOC_IF_ERROR_RETURN(REG_READ_CFP_CTL_REGr(unit, &reg_val));
            soc_CFP_CTL_REGr_field_get
                (unit, &reg_val, CFP_EN_MAPf, (uint32 *)&fld_32);
            if ((fld_32 & (0x1 << param1))  != 0) {
                *param2 = TRUE;
            } else {
                *param2 = FALSE;
            }
            break;
        default:
            return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;
}

/*
 * Function: drv_gex_cfp_control_set
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
drv_gex_cfp_control_set(int unit, uint32 control_type, uint32 param1, 
            uint32 param2)
{
    int rv = SOC_E_NONE;
    uint32  fld_32 = 0, reg_val;
    uint32 temp;

    switch (control_type) {
        case DRV_CFP_ENABLE:
            /* Read CFP control register */
            SOC_IF_ERROR_RETURN(REG_READ_CFP_CTL_REGr(unit, &reg_val));

            /* Set CFP ENABLE PORT bitmap */
            soc_CFP_CTL_REGr_field_get
                (unit, &reg_val, CFP_EN_MAPf, (uint32 *)&fld_32);
            temp = 0x1 << param1;
             if (param2) {
                fld_32 |= temp;
            } else {
                fld_32 &= ~temp;
            }
            soc_CFP_CTL_REGr_field_set
                (unit, &reg_val, CFP_EN_MAPf, (uint32 *)&fld_32);

            /* Write CFP control register */
            SOC_IF_ERROR_RETURN(REG_WRITE_CFP_CTL_REGr(unit, &reg_val));
            break;
            
        case DRV_CFP_TCAM_RESET:
            MEM_LOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
            if ((rv = REG_READ_CFP_ACCr(unit, &reg_val)) < 0) {
                MEM_UNLOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
                return rv;
            }
            fld_32 = 1;
            soc_CFP_ACCr_field_set(unit, &reg_val, TCAM_RSTf, &fld_32);
            if ((rv = REG_WRITE_CFP_ACCr(unit, &reg_val)) < 0) {
                MEM_UNLOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
                return rv;
            }
            MEM_UNLOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
            break;
            
        case DRV_CFP_OTHER_RAM_CLEAR:
            MEM_LOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
            if ((rv = REG_READ_CFP_ACCr(unit, &reg_val)) < 0) {
                MEM_UNLOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
                return rv;
            }
            fld_32 = 1;
            soc_CFP_ACCr_field_set(unit, &reg_val, CFP_RAM_CLEARf, &fld_32);
            if ((rv = REG_WRITE_CFP_ACCr(unit, &reg_val)) < 0) {
                MEM_UNLOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
                return rv;
            }
            MEM_UNLOCK(unit, INDEX(CFP_TCAM_IPV4_SCm));
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }

    return rv;
}


/*
 * Function: drv_gex_cfp_entry_read
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
drv_gex_cfp_entry_read(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry)
{
    int rv = SOC_E_NONE;

    switch (ram_type) {
        case DRV_CFP_RAM_ALL:
            if ((rv = _drv_gex_cfp_read(unit, DRV_CFP_RAM_TCAM, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : failed to read TCAM with index = 0x%x, rv = %d. \n"), 
                           index, rv));
                return rv;
            }
            if ( (rv = _drv_gex_cfp_read(unit, DRV_CFP_RAM_ACT, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : failed to read action ram with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            if ((rv = _drv_gex_cfp_read(unit, DRV_CFP_RAM_METER, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : failed to read meter ram with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_TCAM:
            if ((rv = _drv_gex_cfp_read(unit, DRV_CFP_RAM_TCAM, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : failed to read TCAM with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_ACT:
            if ((rv = _drv_gex_cfp_read(unit, DRV_CFP_RAM_ACT, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : failed to read action ram with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_METER:
            if ((rv = _drv_gex_cfp_read(unit, DRV_CFP_RAM_METER, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : failed to read action ram with index = 0x%x, rv = %d. \n"),
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
 * Function: drv_gex_cfp_entry_search
 *
 * Purpose:
 *     Search the valid CFP entry from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     flags - search flags
 *     index(OUT) - CFP entry index
 *     entry(OUT) - chip entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *
 */
int
drv_gex_cfp_entry_search(int unit, uint32 flags, uint32 *index, 
            drv_cfp_entry_t *entry)
{
    int rv = SOC_E_NONE;

    if ((rv = _drv_gex_cfp_tcam_search(unit, flags, 
            index, entry)) != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "drv_cfp_entry_search : failed to serach TCAM rv = %d\n"),
                   rv));
        return rv;
    }
    /* If found valid TCAM entry, get the correspoding ACT and Meter data */
    if ((flags & DRV_CFP_SEARCH_GET) && (rv == SOC_E_NONE)) {
        if ((rv = _drv_gex_cfp_read(unit, DRV_CFP_RAM_ACT, *index, entry)) 
            != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "drv_cfp_entry_search:failed to read action ram with index = 0x%x, rv = %d. \n"),
                       *index, rv));
            return rv;
        }
        if ((rv = _drv_gex_cfp_read(unit, DRV_CFP_RAM_METER, *index, entry)) 
            != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "drv_cfp_entry_search :failed to read meter ram with index = 0x%x, rv = %d. \n"),
                       *index, rv));
            return rv;
        }
        entry->id = *index;
    }
    return rv;
}


/*
 * Function: drv_gex_cfp_entry_write
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
drv_gex_cfp_entry_write(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry)
{
    int rv = SOC_E_NONE;

    switch (ram_type) {
        case DRV_CFP_RAM_ALL:
            if ((rv = _drv_gex_cfp_write(unit, DRV_CFP_RAM_TCAM, 
                                index, entry, FALSE)) != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write : failed to write TCAM with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            if ((rv = _drv_gex_cfp_write(unit, DRV_CFP_RAM_ACT, 
                                index, entry, FALSE)) != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write:failed to write action ram with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            if ((rv = _drv_gex_cfp_write(unit, DRV_CFP_RAM_METER, 
                                index, entry, FALSE)) != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write :failed to write meter ram with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_TCAM:
             if ((rv = _drv_gex_cfp_write(unit, DRV_CFP_RAM_TCAM, 
                                index, entry, FALSE)) != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write : failed to write TCAM with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index; 
            break;
        case DRV_CFP_RAM_TCAM_INVALID:
             if ((rv = _drv_gex_cfp_write(unit, DRV_CFP_RAM_TCAM_INVALID, 
                                index, entry, FALSE)) != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write : failed to write invalid TCAM with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index; 
            break;
        case DRV_CFP_RAM_ACT:
             if ((rv = _drv_gex_cfp_write(unit, DRV_CFP_RAM_ACT, 
                                index, entry, FALSE)) != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write : failed to write ACT ram with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index; 
            break;
        case DRV_CFP_RAM_METER:
             if ((rv = _drv_gex_cfp_write(unit, DRV_CFP_RAM_METER, 
                                index, entry, FALSE)) != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write :failed to write METER ram with index = 0x%x, rv = %d. \n"),
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
 * Function: drv_gex_cfp_field_get
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
drv_gex_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val)
{
    int rv = SOC_E_NONE;
    int     mem_id;
    uint32  fld_id = 0;
    uint32  mask = -1;
    uint32  mask_hi, mask_lo;
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    int i, wp, bp, len;
    uint32  *data_p;
    int     slice_id = 0;

    assert(entry);
    assert(fld_val);
    *fld_val = 0;
   mem_id = -1;
    switch (mem_type) {
    case DRV_CFP_RAM_TCAM:
    case DRV_CFP_RAM_TCAM_MASK:
        slice_id = entry->slice_id;
        switch (slice_id) {
            case 0:
            case 1:                    
            case 2:                    
                mem_id = INDEX(CFP_TCAM_IPV4_SCm);
                break;
            case 7:
                /* Check this qualifier is reside in slice 0 or 3 */
                if (field_type == DRV_CFP_FIELD_VALID) {
                    /* get from chain slice */
                    entry->slice_id = CFP_53115_IPV6_CHAIN_SLICE_ID;
                    entry->flags = _DRV_CFP_SLICE_CHAIN |
                        _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;
                    mem_id = INDEX(CFP_TCAM_CHAIN_SCm);
                } else if (field_type == DRV_CFP_FIELD_SLICE_ID) {
                    /* get from chain slice */
                    entry->slice_id = CFP_53115_IPV6_CHAIN_SLICE_ID;
                    entry->flags = _DRV_CFP_SLICE_CHAIN |
                        _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;
                    mem_id = INDEX(CFP_TCAM_CHAIN_SCm);
                } else {
                    entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN |
                        _DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
                    if (!_drv_gex_cfp_field_in_chain_slice(field_type)) {
                        mem_id = INDEX(CFP_TCAM_CHAIN_SCm);
                        entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;
                    } else {
                        mem_id = INDEX(CFP_TCAM_IPV6_SCm);
                        entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_MAIN;
                    }
                }
                break;
            case 4:
            case 5:
            case 6:   
                mem_id = INDEX(CFP_TCAM_IPV6_SCm);
                break;                
            case 12:
            case 13:
            case 14:
                mem_id = INDEX(CFP_TCAM_NONIP_SCm);
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
    case DRV_CFP_RAM_ACT:
#if defined(BCM_STARFIGHTER3_SUPPORT)
        if (SOC_IS_STARFIGHTER3(unit)) {
            mem_id = INDEX(CFP_ACTm);
        } else
#endif /* BCM_STARFIGHTER3_SUPPORT */
        {
            mem_id = INDEX(CFP_ACT_POLm);
        }
        if (entry->flags & _DRV_CFP_SLICE_CHAIN){
            if (field_type == DRV_CFP_FIELD_ACTION_CHAIN){
               data_p = entry->cfp_chain->act_data;
            } else {
               data_p = entry->act_data;
            }
        } else {
            data_p = entry->act_data;
        }
        break;
    case DRV_CFP_RAM_METER:
        mem_id = INDEX(CFP_METERm);
        data_p = entry->meter_data;
        break;
    default:
        rv = SOC_E_PARAM;
        return rv;
    }

    /* Check if this qual has been replaced by UDfs */
    if (_drv_gex_cfp_qual_by_udf(unit, &slice_id, field_type)){
        uint32 temp_val;
        
        for (i = 0; i < CFP_53115_UDF_NUM_MAX; i++) {
            if ((drv53115_udf_info[i].valid) && 
                (drv53115_udf_info[i].sub_field == field_type) && 
                (drv53115_udf_info[i].slice_id == slice_id)) {
                wp = (drv53115_udf_info[i].order / 2);
                bp = (drv53115_udf_info[i].order % 2);
                rv = drv_gex_cfp_field_get(unit, mem_type, 
                    drv53115_udf_info[i].self_field, entry, 
                    &temp_val);
                if (rv  < 0) {
                    return rv;
                }
                if (bp == 0) {
                    *(fld_val + wp) |= temp_val & 0xffff; 
                } else {
                    *(fld_val + wp) |= (temp_val & 0xffff) << 16; 
                }
                
            }
        }
        return rv;
    }


    if (( rv = _drv_gex_cfp_field_mapping(unit, mem_type, field_type, &fld_id)) < 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "drv_cfp_field_get : UNKNOW FIELD ID. \n")));
        return rv;
    }

    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);
#if defined(BCM_STARFIGHTER3_SUPPORT)
    if((SOC_IS_STARFIGHTER3(unit)) &&((mem_type == DRV_CFP_RAM_ACT) ||
            (mem_type == DRV_CFP_RAM_METER))) {
        _drv_sf3_cfp_fieldinfo_get(unit, mem_type, fld_id, (uint32 *) &fieldinfo);
        if(mem_type == DRV_CFP_RAM_ACT) {
            meminfo->bytes = CFP_53134_RAM_ACT_REG_SIZE * 4;
        } else if (mem_type == DRV_CFP_RAM_METER) {
            meminfo->bytes = CFP_53134_RAM_METER_REG_SIZE *4;
        }
    } else
#endif /* BCM_STARFIGHTER3_SUPPORT  */
    {
        SOC_FIND_FIELD(fld_id, meminfo->fields,
                             meminfo->nFields, fieldinfo);
    }
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
 * Function: drv_gex_cfp_field_set
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
drv_gex_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val)
{
    int rv = SOC_E_NONE;
    int     mem_id;
    uint32  fld_id = 0;
    uint32  mask, mask_hi, mask_lo;
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    int i, wp, bp, len;
    uint32  *data_p;
    int     slice_id = 0;
    uint32  ipv6_slice_id = 0;

    assert(entry);
    assert(fld_val);
    mem_id = -1;
    switch (mem_type) {
    case DRV_CFP_RAM_TCAM:
    case DRV_CFP_RAM_TCAM_MASK:
        slice_id = entry->slice_id;
        switch (slice_id) {
            case 0:
            case 1:                    
            case 2:                    
                mem_id = INDEX(CFP_TCAM_IPV4_SCm);
                break;
            case 7:
                /* Check this qualifier is reside in slice 0 or 3 */
                if (field_type == DRV_CFP_FIELD_VALID) {
                    /* configure IPv6 slice first */
                    if (entry->cfp_chain != NULL) {
                        entry->slice_id = CFP_53115_IPV6_MAIN_SLICE_ID;
                        entry->flags = _DRV_CFP_SLICE_CHAIN |
                            _DRV_CFP_SLICE_CONFIG_SLICE_MAIN;
                        SOC_IF_ERROR_RETURN(
                            drv_gex_cfp_field_set(unit, mem_type, field_type,
                            entry, fld_val));
                        /* configure chain slice */
                        entry->slice_id = CFP_53115_IPV6_CHAIN_SLICE_ID;
                    }
                    entry->flags = _DRV_CFP_SLICE_CHAIN |
                        _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;
                    mem_id = INDEX(CFP_TCAM_CHAIN_SCm);
                } else if (field_type == DRV_CFP_FIELD_SLICE_ID) {
                    /* configure IPv6 slice first */
                    entry->slice_id = CFP_53115_IPV6_MAIN_SLICE_ID;
                    entry->flags = _DRV_CFP_SLICE_CHAIN |
                        _DRV_CFP_SLICE_CONFIG_SLICE_MAIN;
                    SOC_IF_ERROR_RETURN(
                        drv_gex_cfp_field_set(unit, mem_type, field_type,
                        entry, &ipv6_slice_id));
                    /* configure chain slice */
                    entry->slice_id = CFP_53115_IPV6_CHAIN_SLICE_ID;
                    entry->flags = _DRV_CFP_SLICE_CHAIN |
                        _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;
                    mem_id = INDEX(CFP_TCAM_CHAIN_SCm);
                } else {
                    entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN |
                        _DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
                    if (_drv_gex_cfp_field_in_chain_slice(field_type)) {
                        mem_id = INDEX(CFP_TCAM_CHAIN_SCm);
                        entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;
                    } else {
                        mem_id = INDEX(CFP_TCAM_IPV6_SCm);
                        entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_MAIN;
                    }
                }
                break;
            case 4:
            case 5:
            case 6:   
                mem_id = INDEX(CFP_TCAM_IPV6_SCm);
                break;                
            case 12:
            case 13:
            case 14:
                mem_id = INDEX(CFP_TCAM_NONIP_SCm);
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
    case DRV_CFP_RAM_ACT:
#if defined(BCM_STARFIGHTER3_SUPPORT)
        if (SOC_IS_STARFIGHTER3(unit)) {
            mem_id = INDEX(CFP_ACTm);
        } else
#endif /* BCM_STARFIGHTER3_SUPPORT */
        {
            mem_id = INDEX(CFP_ACT_POLm);
        }
        if (entry->flags & _DRV_CFP_SLICE_CHAIN){
            if (field_type == DRV_CFP_FIELD_ACTION_CHAIN){
               data_p = entry->cfp_chain->act_data;
            } else {
               data_p = entry->act_data;
            }
        } else {
            data_p = entry->act_data;
        }
        break;
    case DRV_CFP_RAM_METER:
        mem_id = INDEX(CFP_METERm);
        data_p = entry->meter_data;
        break;
    default:
        rv = SOC_E_PARAM;
        return rv;
    } 

    /* Check if this qual has been replaced by UDfs */
    if (_drv_gex_cfp_qual_by_udf(unit, &slice_id, field_type)){
        uint32 temp_val;
        
        for (i = 0; i < CFP_53115_UDF_NUM_MAX; i++) {
            if ((drv53115_udf_info[i].valid) && 
                (drv53115_udf_info[i].sub_field == field_type) && 
                (drv53115_udf_info[i].slice_id == slice_id)) {
                wp = (drv53115_udf_info[i].order / 2);
                bp = (drv53115_udf_info[i].order % 2);
                temp_val = *(fld_val + wp);
                if (bp == 0) {
                    temp_val = temp_val & 0xffff;
                } else {
                    temp_val = (temp_val >> 16) & 0xffff;
                }
                rv = drv_gex_cfp_field_set(unit, mem_type, 
                    drv53115_udf_info[i].self_field, entry, 
                    &temp_val);
                if (rv  < 0) {
                    LOG_CLI((BSL_META_U(unit,
                                        "%s FAIL on field set. \n"),FUNCTION_NAME()));
                    return rv;
                }
            
                /* set valid bit */
                temp_val = 1;
                rv = drv_gex_cfp_field_set(unit, mem_type, 
                    drv53115_udf_info[i].valid_field, entry, 
                    &temp_val);
                if (rv  < 0) {
                    LOG_CLI((BSL_META_U(unit,
                                        "%s FAIL on field set. \n"),FUNCTION_NAME()));
                    return rv;
                }
            }
        }
        return rv;
    }

    if (( rv = _drv_gex_cfp_field_mapping(unit, mem_type, field_type, &fld_id)) < 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "drv_cfp_field_set : UNKNOW FIELD ID. \n")));
        return rv;
    }

    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);
#if defined(BCM_STARFIGHTER3_SUPPORT)
    if((SOC_IS_STARFIGHTER3(unit)) &&((mem_type == DRV_CFP_RAM_ACT) ||
            (mem_type == DRV_CFP_RAM_METER))) {
        _drv_sf3_cfp_fieldinfo_get(unit, mem_type, fld_id, (uint32 *) &fieldinfo);
        if(mem_type == DRV_CFP_RAM_ACT) {
            meminfo->bytes = CFP_53134_RAM_ACT_REG_SIZE * 4;
        } else if (mem_type == DRV_CFP_RAM_METER) {
            meminfo->bytes = CFP_53134_RAM_METER_REG_SIZE *4;
        }
    } else
#endif /* BCM_STARFIGHTER3_SUPPORT  */
    {
        SOC_FIND_FIELD(fld_id, meminfo->fields,
                             meminfo->nFields, fieldinfo);
    }
    assert(fieldinfo);

    /* 
     * Get the value to set into each entry's valid field. 
     * The valid value is depend on chips.
     */
    if ((fld_id == INDEX(VALID_Rf)) &&  *fld_val != 0){
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
 * Function: drv_gex_cfp_meter_get
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
drv_gex_cfp_meter_get(int unit, drv_cfp_entry_t* entry, uint32 *kbits_sec, 
            uint32 *kbits_burst)    
{
    int rv = SOC_E_NONE;
    uint32  fld_val;

    assert(entry);
    assert(kbits_sec);
    assert(kbits_burst);
    
    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_METER, 
        DRV_CFP_FIELD_RATE_REFRESH_EN, entry, &fld_val);
    if (fld_val) {
        (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_RATE, entry, &fld_val);
        _drv_gex_cfp_meter_chip2rate(unit, kbits_sec, fld_val);
    } else {
        *kbits_sec = 0;
    }

    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_METER, 
        DRV_CFP_FIELD_REF_CAP, entry, &fld_val);
    _drv_gex_cfp_meter_chip2burst(unit, kbits_burst, fld_val);

    return rv;
}

/*
 * Function: drv_gex_cfp_meter_set
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
drv_gex_cfp_meter_set(int unit, drv_cfp_entry_t* entry, uint32 kbits_sec, 
            uint32 kbits_burst)
{
    int rv = SOC_E_NONE;
    uint32  fld_val;

    assert(entry);
    if (kbits_sec) {
        if ((kbits_sec > CFP_53115_METER_RATE_MAX) ||
            (kbits_sec < CFP_53115_METER_RATE_MIN)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "drv_cfp_meter_set : rate unsupported. \n")));
            rv = SOC_E_UNAVAIL;
            return rv;
        }
        _drv_gex_cfp_meter_rate2chip(unit, kbits_sec, &fld_val);
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
                DRV_CFP_FIELD_RATE, entry, &fld_val);
        fld_val = 1;
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_RATE_REFRESH_EN, entry, &fld_val);
    } else {
        fld_val = 0;
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_RATE_REFRESH_EN, entry, &fld_val);
    }

    if (kbits_burst) {
        if ((kbits_burst > CFP_53115_METER_BURST_MAX) ||
            (kbits_burst < CFP_53115_METER_BURST_MIN)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "drv_cfp_meter_set : burst size unsupported. \n")));
            rv = SOC_E_UNAVAIL;
            return rv;
        }
        _drv_gex_cfp_meter_burst2chip(unit, kbits_burst, &fld_val);
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
                DRV_CFP_FIELD_REF_CAP, entry, &fld_val);
    } else {
        _drv_gex_cfp_meter_burst2chip(unit, CFP_53115_METER_BURST_MIN, &fld_val);
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
                DRV_CFP_FIELD_REF_CAP, entry, &fld_val);
    }
    return rv;
}

int
drv_gex_cfp_meter_rate_transform(int unit, uint32 kbits_sec, uint32 kbits_burst, 
                uint32 *bucket_size, uint32 * ref_cnt, uint32 *ref_unit)
{
    int rv = SOC_E_NONE;

    if (kbits_sec) {
        /*    coverity[unsigned_compare]    */
        if ((kbits_sec > CFP_53115_METER_RATE_MAX) ||
            (kbits_sec < CFP_53115_METER_RATE_MIN)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "drv_cfp_meter_rate_transform : rate unsupported. \n")));
            rv = SOC_E_UNAVAIL;
            return rv;
        }
        _drv_gex_cfp_meter_rate2chip(unit, kbits_sec, ref_cnt);
    } else {
        *ref_cnt = 0;
    }

    if (kbits_burst) {
        /*    coverity[unsigned_compare]    */
        if ((kbits_burst > CFP_53115_METER_BURST_MAX) ||
            (kbits_burst < CFP_53115_METER_BURST_MIN)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "drv_cfp_meter_rate_transform : burst size unsupported. \n")));
            rv = SOC_E_UNAVAIL;
            return rv;
        }
        
        _drv_gex_cfp_meter_burst2chip(unit, kbits_burst, bucket_size);
    } else {
        *bucket_size = 0;
    }

    return rv;
}
                


int drv_gex_cfp_slice_id_select_udf(int unit, drv_cfp_entry_t * entry, uint32 * slice_id);
int _drv_gex_cfp_chain_slice_id_select(int unit, drv_cfp_entry_t * entry, 
            uint32 * slice_id, uint32 flags);

/*
 * Function: drv_gex_cfp_slice_id_select
 *
 * Purpose:
 *     According to this entry's fields to select which slice id used for this entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - driver cfp entry
 *     slice_id(OUT) - slice id for this entry
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_RESOURCE - Can't found suitable slice id for this entry.
 */
int
drv_gex_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, uint32 *slice_id, uint32 flags)
{
    uint32 i, j;
    int match;
    int slice;

    if (flags & DRV_FIELD_QUAL_CHAIN) {
        /* L3 frame type : IPv6 + SLICE ID */
        return _drv_gex_cfp_chain_slice_id_select(
            unit, entry, slice_id, flags);
    }
    
    if (flags & DRV_FIELD_QUAL_REPLACE_BY_UDF) {
        return drv_gex_cfp_slice_id_select_udf(
            unit, entry, slice_id);
    }

    for (j=0; j <= CFP_53115_SLICE_MAX_ID; j++) {
        /* Checking Non-IP first */
        slice = (j + 13) % (CFP_53115_SLICE_MAX_ID + 1);
        if (slice == CFP_53115_IPV6_CHAIN_SLICE_ID) {
            /* only for double wide mode(chain) */
            continue;
        }
        match = TRUE;
        for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
            if (entry->w[i] & ~drv53115_slice_info[slice].qset[i]) {
                match = FALSE;
                break;
            }
        }
        if (match) {
            *slice_id = drv53115_slice_info[slice].slice_id;
            return SOC_E_NONE;
        }
    }
    
    return SOC_E_RESOURCE;
    
}

int
drv_gex_cfp_slice_id_select_udf(int unit, drv_cfp_entry_t *entry, uint32 *slice_id)
{
    uint32 i, j, k;
    int match, slice;
    drv_slice_info_t old_slice_info;
    uint32 diff_qset;
    uint32 qual;

    for (k=0; k < (CFP_53115_SLICE_MAX_ID + 1); k++) {
        /* Checking Non-IP first */
        slice = (k + 13) % (CFP_53115_SLICE_MAX_ID + 1);
        sal_memcpy(&old_slice_info, &drv53115_slice_info[slice],
            sizeof(drv_slice_info_t));
        match = TRUE;
        for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
            diff_qset = entry->w[i] & ~drv53115_slice_info[slice].qset[i];
            if (diff_qset) {
                for (j = 0; j < 32; j++) {
                    if (diff_qset & 0x1 << j) {
                        qual = i * 32 + j;
                        switch (qual) {
                            case DRV_CFP_QUAL_MAC_SA:
                            case DRV_CFP_QUAL_MAC_DA:
                                if (old_slice_info.slice_udf_free >= 3) {
                                    old_slice_info.slice_udf_free -= 3;
                                } else {
                                    match = FALSE;
                                }
                                break;
                            case DRV_CFP_QUAL_IP_SA:
                            case DRV_CFP_QUAL_IP_DA:
                                if ((slice != 0) && (slice != 1) && 
                                    (slice !=2)) {
                                    match = FALSE;
                                    break;
                                }
                                if (old_slice_info.slice_udf_free >= 2) {
                                    old_slice_info.slice_udf_free -= 2;
                                } else {
                                    match = FALSE;
                                }
                                break;
                            case DRV_CFP_QUAL_IP6_SA:
                            case DRV_CFP_QUAL_IP6_DA:
                                if ((slice != 4) && (slice != 5) && 
                                    (slice !=6) && (slice !=7)) {
                                    match = FALSE;
                                    break;
                                }
                                if (old_slice_info.slice_udf_free >= 8) {
                                    old_slice_info.slice_udf_free -= 8;
                                } else {
                                    match = FALSE;
                                }
                                break;
                            case DRV_CFP_QUAL_IP6_FLOW_ID:
                                if ((slice != 4) && (slice != 5) && 
                                    (slice !=6) && (slice !=7)) {
                                    match = FALSE;
                                    break;
                                }
                                if (old_slice_info.slice_udf_free >= 1) {
                                    old_slice_info.slice_udf_free -= 1;
                                } else {
                                    match = FALSE;
                                }
                                break;
                            case DRV_CFP_QUAL_L4_SRC:
                            case DRV_CFP_QUAL_L4_DST:
                            case DRV_CFP_QUAL_TCP_FLAG:
                                if ((slice != 0) && (slice != 1) && 
                                    (slice !=2)) {
                                    match = FALSE;
                                    break;
                                }
                                if (old_slice_info.slice_udf_free >= 1) {
                                    old_slice_info.slice_udf_free -= 1;
                                } else {
                                    match = FALSE;
                                }
                                break;
                             case DRV_CFP_QUAL_SNAP_HEADER:
                                if ((slice != 12) && (slice != 13) && 
                                    (slice !=14)) {
                                    match = FALSE;
                                    break;
                                }
                                if (old_slice_info.slice_udf_free >= 3) {
                                    old_slice_info.slice_udf_free -= 3;
                                } else {
                                    match = FALSE;
                                }
                                break;
                            case DRV_CFP_QUAL_LLC_HEADER:
                                if ((slice != 12) && (slice != 13) && 
                                    (slice !=14)) {
                                    match = FALSE;
                                    break;
                                }
                                if (old_slice_info.slice_udf_free >= 2) {
                                    old_slice_info.slice_udf_free -= 2;
                                } else {
                                    match = FALSE;
                                }
                                break;
                            default:
                                match = FALSE;
                                break;
                        }
                    }
                }
            }
        }
        if (match) {
            *slice_id = old_slice_info.slice_id;
            return SOC_E_NONE;
        }
    }
    
    return SOC_E_RESOURCE;
    
}


int
_drv_gex_cfp_chain_slice_id_select(int unit, drv_cfp_entry_t *entry, uint32 *slice_id, uint32 flags)
{
    uint32 i, j;
    int match;
    drv_slice_info_t chain_slice_info;
    uint32 diff_qset, qual;


    /* Combine qualifier set of the slice 11 and 7 for chain slice */
    match = TRUE;
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        chain_slice_info.qset[i] = 
            drv53115_slice_info[CFP_53115_IPV6_CHAIN_SLICE_ID].qset[i] |
            drv53115_slice_info[CFP_53115_IPV6_MAIN_SLICE_ID].qset[i];
        if (entry->w[i] & ~chain_slice_info.qset[i]) {
                match = FALSE;
                break;
        }
    }
    if (match) {
        *slice_id = CFP_53115_IPV6_CHAIN_SLICE_ID;
        return SOC_E_NONE;
    }

    /* Using the UDFs to replace the required qualifiers */
    if (flags & DRV_FIELD_QUAL_REPLACE_BY_UDF) {
        chain_slice_info.slice_udf_free = 
            drv53115_slice_info[CFP_53115_IPV6_CHAIN_SLICE_ID].slice_udf_free +
            drv53115_slice_info[4].slice_udf_free;
        match = TRUE;
        for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
            diff_qset = entry->w[i] & ~chain_slice_info.qset[i];
            if (diff_qset) {
                for (j = 0; j < 32; j++) {
                    if (diff_qset & 0x1 << j) {
                        qual = i * 32 + j;
                        switch (qual) {
                            case DRV_CFP_QUAL_MAC_SA:
                            case DRV_CFP_QUAL_MAC_DA:
                                if (chain_slice_info.slice_udf_free >= 3) {
                                    chain_slice_info.slice_udf_free -= 3;
                                } else {
                                    match = FALSE;
                                }
                                break;
                            case DRV_CFP_QUAL_IP6_SA:
                            case DRV_CFP_QUAL_IP6_DA:
                                if (chain_slice_info.slice_udf_free >= 8) {
                                    chain_slice_info.slice_udf_free -= 8;
                                } else {
                                    match = FALSE;
                                }
                                break;
                            case DRV_CFP_QUAL_IP6_FLOW_ID:
                                if (chain_slice_info.slice_udf_free >= 1) {
                                    chain_slice_info.slice_udf_free -= 1;
                                } else {
                                    match = FALSE;
                                }
                                break;
                            default:
                                match = FALSE;
                                break;
                        }
                    }
                }
            }
            if (match == FALSE) {
                break;
            }
        }
        if (match) {
            *slice_id = CFP_53115_IPV6_CHAIN_SLICE_ID;
            return SOC_E_NONE;
        }
    }

    return SOC_E_RESOURCE;

}


/*
 * Function: drv_gex_cfp_slice_to_qset
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
drv_gex_cfp_slice_to_qset(int unit, uint32 slice_id, drv_cfp_entry_t *entry)
{
    uint32 i;

    if (slice_id > CFP_53115_SLICE_MAX_ID) {
        return SOC_E_PARAM;
    }

    if (slice_id == CFP_53115_IPV6_CHAIN_SLICE_ID) {
        for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
            entry->w[i] = 
                drv53115_slice_info[CFP_53115_IPV6_CHAIN_SLICE_ID].qset[i] |
                drv53115_slice_info[CFP_53115_IPV6_MAIN_SLICE_ID].qset[i];
        }
    } else {
        for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
            entry->w[i] = drv53115_slice_info[slice_id].qset[i];
        }
    }
    return SOC_E_NONE;
    
}

/*
 * Function: drv_gex_cfp_stat_get
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
drv_gex_cfp_stat_get(int unit, uint32 stat_type, uint32 index, uint32* counter)
{
    int rv = SOC_E_NONE;
    uint32  index_max, temp;
    
    assert(counter);

   index_max = soc_robo_mem_index_max(unit, INDEX(CFP_STAT_IBm));
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }
    
    switch (stat_type) {
        case DRV_CFP_STAT_INBAND:
            _drv_gex_cfp_stat_read(unit, DRV_CFP_RAM_STAT_IB, index, 
                counter);
            break;
        case DRV_CFP_STAT_OUTBAND:
            _drv_gex_cfp_stat_read(unit, DRV_CFP_RAM_STAT_OB, index, 
                counter);
            break;
        case DRV_CFP_STAT_ALL:
            _drv_gex_cfp_stat_read(unit, DRV_CFP_RAM_STAT_IB, index, 
                &temp);
            *counter = temp;
            _drv_gex_cfp_stat_read(unit, DRV_CFP_RAM_STAT_OB, index, 
                &temp);
            *counter += temp;
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv_gex_cfp_stat_set
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
drv_gex_cfp_stat_set(int unit, uint32 stat_type, uint32 index, uint32 counter)
{
    int rv = SOC_E_NONE;
    uint32  index_max;

   index_max = soc_robo_mem_index_max(unit, INDEX(CFP_STAT_IBm));
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }
    
    switch (stat_type) {
        case DRV_CFP_STAT_INBAND:
            _drv_gex_cfp_stat_write(unit, DRV_CFP_RAM_STAT_IB, index, 
                counter);
            break;
        case DRV_CFP_STAT_OUTBAND:
            _drv_gex_cfp_stat_write(unit, DRV_CFP_RAM_STAT_OB, index, 
                counter);
            break;
        case DRV_CFP_STAT_ALL:
            _drv_gex_cfp_stat_write(unit, DRV_CFP_RAM_STAT_IB, index, 
                counter);
            _drv_gex_cfp_stat_write(unit, DRV_CFP_RAM_STAT_OB, index, 
                counter);
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv_gex_cfp_udf_get
 *
 * Purpose:
 *     Get the offset value of the User Defined fields.
 *
 * Parameters:
 *     unit - BCM device number
 *     port - port numbrt
 *     udf_index -the index of user defined fields
 *     offset(OUT) -offset value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     For BCM5348, there are 10 UDFs.
 *     
 */
int 
drv_gex_cfp_udf_get(int unit, uint32 port, uint32 udf_index, 
    uint32 *offset, uint32 *base)
{
    int rv = SOC_E_NONE;
    uint32  reg_addr, reg_len, temp;
    uint32  reg_32;
    drv_udf_reg_info_t udf_reg_info;

    if (udf_index >=  CFP_53115_UDF_NUM_MAX) {
        return SOC_E_CONFIG;
    }

    if (port == DRV_CFP_UDF_LENGTH_GET){
        *offset = 2; /* all the UDF length are 2-byte length */
        return SOC_E_NONE;
    }

    /* For BCM53115 
     * 0 : slice0 A0,  9 : slice1 A0,  18 : slice2 A0  
     * 1 : slice0 A1,  10 : slice1 A1,  19 : slice2 A1
     * 2 : slice0 A2,  11 : slice1 A2,  20 : slice2 A2
     * 3 : slice0 A3,  12 : slice1 A3,  21 : slice2 A3
     * 4 : slice0 A4,  13 : slice1 A4,  22 : slice2 A4
     * 5 : slice0 A5,  14 : slice1 A5,  23 : slice2 A5
     * 6 : slice0 A6,  15 : slice1 A6,  24 : slice2 A6
     * 7 : slice0 A7,  16 : slice1 A7,  25 : slice2 A7
     * 8 : slice0 A8,  17 : slice1 A8,  26 : slice2 A8
     *
     * 27 : slice0 B0,  36 : slice1 B0,  45 : slice2 B0  
     * 28 : slice0 B1,  37 : slice1 B1,  46 : slice2 B1
     * 29 : slice0 B2,  38 : slice1 B2,  47 : slice2 B2
     * 30 : slice0 B3,  39 : slice1 B3,  48 : slice2 B3
     * 31 : slice0 B4,  40 : slice1 B4,  49 : slice2 B4
     * 32 : slice0 B5,  41 : slice1 B5,  50 : slice2 B5
     * 33 : slice0 B6,  42 : slice1 B6,  51 : slice2 B6
     * 34 : slice0 B7,  43 : slice1 B7,  52 : slice2 B7
     * 35 : slice0 B8,  44 : slice1 B8,  53 : slice2 B8
     *
     * 54 : slice0 C0,  63 : slice1 C0,  72 : slice2 C0  
     * 55 : slice0 C1,  64 : slice1 C1,  73 : slice2 C1
     * 56 : slice0 C2,  65 : slice1 C2,  74 : slice2 C2
     * 57 : slice0 C3,  66 : slice1 C3,  75 : slice2 C3
     * 58 : slice0 C4,  67 : slice1 C4,  76 : slice2 C4
     * 59 : slice0 C5,  68 : slice1 C5,  77 : slice2 C5
     * 60 : slice0 C6,  69 : slice1 C6,  78 : slice2 C6
     * 61 : slice0 C7,  70 : slice1 C7,  79 : slice2 C7
     * 62 : slice0 C8,  71 : slice1 C8,  80 : slice2 C8
     *
     * 81 : D0
     * 82 : D1
     * 83 : D2
     * 84 : D3
     * 85 : D4
     * 86 : D5
     * 87 : D6
     * 88 : D7
     * 89 : D8
     * 90 : D9
     * 91 : D10
     * 92 : D11
     */

    sal_memset(&udf_reg_info, 0, sizeof(drv_udf_reg_info_t)); 

    sal_memcpy(&udf_reg_info, &drv53115_udf_reg_info[udf_index],
        sizeof(drv_udf_reg_info_t));
    
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, udf_reg_info.reg_id, 0, udf_reg_info.reg_index);
    
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, udf_reg_info.reg_id);
    
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_32, reg_len)) < 0) {
        return rv;
    }
    (DRV_SERVICES(unit)->reg_field_get)
        (unit, udf_reg_info.reg_id, &reg_32, 
        udf_reg_info.reg_field_id, &temp);
    
    *offset = (temp & 0x1f) * 2;

    temp &= 0x7;
    temp >>= 5;
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
 * Function: drv_gex_cfp_udf_set
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
drv_gex_cfp_udf_set(int unit, uint32 port, uint32 udf_index, 
    uint32 offset, uint32 base)
{
    int rv = SOC_E_NONE;
    uint32  reg_addr, reg_len, temp;
    uint32  reg_32;
    drv_udf_reg_info_t udf_reg_info;

    if (udf_index >=  CFP_53115_UDF_NUM_MAX) {
        return SOC_E_CONFIG;
    }
    if (offset > CFP_53115_UDF_OFFSET_MAX) {
        return SOC_E_CONFIG;
    }

    /* For BCM53115 
     * 0 : slice0 A0,  9 : slice1 A0,  18 : slice2 A0  
     * 1 : slice0 A1,  10 : slice1 A1,  19 : slice2 A1
     * 2 : slice0 A2,  11 : slice1 A2,  20 : slice2 A2
     * 3 : slice0 A3,  12 : slice1 A3,  21 : slice2 A3
     * 4 : slice0 A4,  13 : slice1 A4,  22 : slice2 A4
     * 5 : slice0 A5,  14 : slice1 A5,  23 : slice2 A5
     * 6 : slice0 A6,  15 : slice1 A6,  24 : slice2 A6
     * 7 : slice0 A7,  16 : slice1 A7,  25 : slice2 A7
     * 8 : slice0 A8,  17 : slice1 A8,  26 : slice2 A8
     *
     * 27 : slice0 B0,  36 : slice1 B0,  45 : slice2 B0  
     * 28 : slice0 B1,  37 : slice1 B1,  46 : slice2 B1
     * 29 : slice0 B2,  38 : slice1 B2,  47 : slice2 B2
     * 30 : slice0 B3,  39 : slice1 B3,  48 : slice2 B3
     * 31 : slice0 B4,  40 : slice1 B4,  49 : slice2 B4
     * 32 : slice0 B5,  41 : slice1 B5,  50 : slice2 B5
     * 33 : slice0 B6,  42 : slice1 B6,  51 : slice2 B6
     * 34 : slice0 B7,  43 : slice1 B7,  52 : slice2 B7
     * 35 : slice0 B8,  44 : slice1 B8,  53 : slice2 B8
     *
     * 54 : slice0 C0,  63 : slice1 C0,  72 : slice2 C0  
     * 55 : slice0 C1,  64 : slice1 C1,  73 : slice2 C1
     * 56 : slice0 C2,  65 : slice1 C2,  74 : slice2 C2
     * 57 : slice0 C3,  66 : slice1 C3,  75 : slice2 C3
     * 58 : slice0 C4,  67 : slice1 C4,  76 : slice2 C4
     * 59 : slice0 C5,  68 : slice1 C5,  77 : slice2 C5
     * 60 : slice0 C6,  69 : slice1 C6,  78 : slice2 C6
     * 61 : slice0 C7,  70 : slice1 C7,  79 : slice2 C7
     * 62 : slice0 C8,  71 : slice1 C8,  80 : slice2 C8
     *
     * 81 : D0
     * 82 : D1
     * 83 : D2
     * 84 : D3
     * 85 : D4
     * 86 : D5
     * 87 : D6
     * 88 : D7
     * 89 : D8
     * 90 : D9
     * 91 : D10
     * 92 : D11
     */

    sal_memset(&udf_reg_info, 0, sizeof(drv_udf_reg_info_t)); 

    sal_memcpy(&udf_reg_info, &drv53115_udf_reg_info[udf_index],
        sizeof(drv_udf_reg_info_t));

    
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, udf_reg_info.reg_id, 0, udf_reg_info.reg_index);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
        (unit, udf_reg_info.reg_id);
    
    temp = (offset /2 ) & 0x1f; /* the offset is 2N bytes based */

    switch(base) {
        case DRV_CFP_UDF_OFFSET_BASE_START_OF_FRAME:
            temp |= (0 << 5);
            break;
        case DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR:
            temp |= (2 << 5);
            break;
        case DRV_CFP_UDF_OFFSET_BASE_END_OF_L3_HDR:
            temp |= (3 << 5);
            break;
        default:
            return SOC_E_PARAM;
    }

    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_32, reg_len)) < 0) {
        return rv;
    }
    
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, udf_reg_info.reg_id, &reg_32, 
        udf_reg_info.reg_field_id, &temp);
    
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_32, reg_len)) < 0) {
        return rv;
    }
    
    return rv;
}


int 
_drv_gex_cfp_chain_sub_qual_by_udf(int unit, int enable, int slice_id, uint32 sub_qual, 
    drv_cfp_qual_udf_info_t * qual_udf_info)
{
    int i;
    int match = FALSE;
    drv_cfp_qual_udf_translate_t translate_info;
    int udf_used;
    int base, shift;
    int sub_slice_id;

    if (enable ) {
        /* Check if this qualify can be replaced by UDF */
        i = 0;
        
        while (drv53115_qual_udf_translate_info[i].qual !=
            DRV_CFP_QUAL_INVALID) {
            if (drv53115_qual_udf_translate_info[i].qual == 
                sub_qual) {
                sal_memcpy(&translate_info, &drv53115_qual_udf_translate_info[i],
                    sizeof(drv_cfp_qual_udf_translate_t));
                match = TRUE;
                break;
            }
            i++;
        }

        if (match == FALSE) {
            return SOC_E_UNAVAIL;
        }

        /* Check where the UDFs from */
        if (drv53115_slice_info[CFP_53115_IPV6_MAIN_SLICE_ID].slice_udf_free >=
            translate_info.udf_num) {
            /* same slice id means all UDFs comes from one slice */
            sub_slice_id = CFP_53115_IPV6_MAIN_SLICE_ID;
        } else if (drv53115_slice_info[CFP_53115_IPV6_CHAIN_SLICE_ID].slice_udf_free >=
            translate_info.udf_num) {
            /* same slice id means all UDFs comes from one slice */
            sub_slice_id = CFP_53115_IPV6_CHAIN_SLICE_ID;
        } else {
            /* UDFs comes from different slice is not supported */
            return SOC_E_RESOURCE;
        }


        /* Update the slice and UDF information */
        udf_used = 0;
        for (i=0; i < CFP_53115_UDF_NUM_MAX; i++) {
            
            if ((drv53115_udf_info[i].valid == 0) && 
                (drv53115_udf_info[i].slice_id == sub_slice_id)) {
                drv53115_udf_info[i].valid = 1;
                drv53115_udf_info[i].sub_qual = sub_qual;
                drv53115_udf_info[i].sub_field = translate_info.field;
                drv53115_udf_info[i].offset_base = translate_info.udf_base;
                drv53115_udf_info[i].offset_value = translate_info.udf_off[udf_used];
                drv53115_udf_info[i].order = udf_used;
                drv_gex_cfp_udf_set(unit, -1, i, drv53115_udf_info[i].offset_value,
                    drv53115_udf_info[i].offset_base);
                qual_udf_info->qual = sub_qual;
                qual_udf_info->udf_index[udf_used] = i;
                qual_udf_info->udf_off[udf_used] = drv53115_udf_info[i].offset_value;
                
                drv53115_slice_info[sub_slice_id].slice_udf_free--;
                udf_used++;
                if (udf_used == translate_info.udf_num) {
                    /* Done */
                    qual_udf_info->valid = 1;
                    qual_udf_info->udf_num = translate_info.udf_num;
                    qual_udf_info->udf_base = translate_info.udf_base;
                    base = sub_qual /32;
                    shift = sub_qual % 32;
                    drv53115_slice_info[sub_slice_id].qset[base] |= 
                        (0x1 << shift);
                    return SOC_E_NONE;
                }
            }

        }

        /* Failed or Removed*/
        udf_used = 0;
        for (i=0; i < CFP_53115_UDF_NUM_MAX; i++) {
            if ((drv53115_udf_info[i].valid) && 
                (drv53115_udf_info[i].slice_id == sub_slice_id)&& 
                (drv53115_udf_info[i].sub_qual == sub_qual)) {
                drv53115_udf_info[i].valid = 0;
                drv53115_udf_info[i].sub_qual = 0;
                drv53115_udf_info[i].sub_field = 0;
                udf_used++;
            }
        }

        drv53115_slice_info[sub_slice_id].slice_udf_free += udf_used;

        return SOC_E_FAIL;
        
    } else { /* Disable */
        udf_used = 0;
        sub_slice_id = CFP_53115_IPV6_CHAIN_SLICE_ID;
        for (i=0; i < CFP_53115_UDF_NUM_MAX; i++) {
            if ((drv53115_udf_info[i].valid) && 
                (drv53115_udf_info[i].slice_id == sub_slice_id)) {
                base = drv53115_udf_info[i].sub_qual /32;
                shift = drv53115_udf_info[i].sub_qual % 32;
                drv53115_slice_info[slice_id].qset[base] &= ~(0x1 << shift);
                drv53115_udf_info[i].valid = 0;
                drv53115_udf_info[i].sub_qual = 0;
                drv53115_udf_info[i].sub_field = 0;
                udf_used++;
            }
        }

        drv53115_slice_info[sub_slice_id].slice_udf_free += udf_used;
    }
    
    return SOC_E_NONE;
}




int 
drv_gex_cfp_sub_qual_by_udf(int unit, int enable, int slice_id, uint32 sub_qual, 
    drv_cfp_qual_udf_info_t * qual_udf_info)
{
    int i;
    int match = FALSE;
    drv_cfp_qual_udf_translate_t translate_info;
    int udf_used;
    int base, shift;

    /* check if chain slice */
    if (slice_id == CFP_53115_IPV6_CHAIN_SLICE_ID) {
        return _drv_gex_cfp_chain_sub_qual_by_udf(
            unit, enable, slice_id, sub_qual, qual_udf_info);
    }

    if (enable ) {
        /* Check if this qualify can be replaced by UDF */
        i = 0;
        
        while (drv53115_qual_udf_translate_info[i].qual !=
            DRV_CFP_QUAL_INVALID) {
            if (drv53115_qual_udf_translate_info[i].qual == 
                sub_qual) {
                sal_memcpy(&translate_info, &drv53115_qual_udf_translate_info[i],
                    sizeof(drv_cfp_qual_udf_translate_t));
                match = TRUE;
                break;
            }
            i++;
        }

        if (match == FALSE) {
            return SOC_E_UNAVAIL;
        }

        if (drv53115_slice_info[slice_id].slice_udf_free < 
            translate_info.udf_num) {
            return SOC_E_RESOURCE;
        }


        /* Update the slice and UDF information */
        udf_used = 0;
        for (i=0; i < CFP_53115_UDF_NUM_MAX; i++) {
            
            if ((drv53115_udf_info[i].valid == 0) && 
                (drv53115_udf_info[i].slice_id == slice_id)) {
                drv53115_udf_info[i].valid = 1;
                drv53115_udf_info[i].sub_qual = sub_qual;
                drv53115_udf_info[i].sub_field = translate_info.field;
                drv53115_udf_info[i].offset_base = translate_info.udf_base;
                drv53115_udf_info[i].offset_value = translate_info.udf_off[udf_used];
                drv53115_udf_info[i].order = udf_used;
                drv_gex_cfp_udf_set(unit, -1, i, drv53115_udf_info[i].offset_value,
                    drv53115_udf_info[i].offset_base);
                qual_udf_info->qual = sub_qual;
                qual_udf_info->udf_index[udf_used] = i;
                qual_udf_info->udf_off[udf_used] = drv53115_udf_info[i].offset_value;
                
                drv53115_slice_info[slice_id].slice_udf_free--;
                udf_used++;
                if (udf_used == translate_info.udf_num) {
                    /* Done */
                    qual_udf_info->valid = 1;
                    qual_udf_info->udf_num = translate_info.udf_num;
                    qual_udf_info->udf_base = translate_info.udf_base;
                    base = sub_qual /32;
                    shift = sub_qual % 32;
                    drv53115_slice_info[slice_id].qset[base] |= (0x1 << shift);
                    return SOC_E_NONE;
                }
            }

        }

        /* Failed or Removed*/
        udf_used = 0;
        for (i=0; i < CFP_53115_UDF_NUM_MAX; i++) {
            if ((drv53115_udf_info[i].valid) && 
                (drv53115_udf_info[i].slice_id == slice_id) && 
                (drv53115_udf_info[i].sub_qual == sub_qual)) {
                drv53115_udf_info[i].valid = 0;
                drv53115_udf_info[i].sub_qual = 0;
                drv53115_udf_info[i].sub_field = 0;
                udf_used++;
            }
        }

        drv53115_slice_info[slice_id].slice_udf_free += udf_used;

        return SOC_E_FAIL;
        
    } else { /* Disable */
        udf_used = 0;
        for (i=0; i < CFP_53115_UDF_NUM_MAX; i++) {
            if ((drv53115_udf_info[i].valid) && 
                (drv53115_udf_info[i].slice_id == slice_id)) {
                base = drv53115_udf_info[i].sub_qual /32;
                shift = drv53115_udf_info[i].sub_qual % 32;
                drv53115_slice_info[slice_id].qset[base] &= ~(0x1 << shift);
                drv53115_udf_info[i].valid = 0;
                drv53115_udf_info[i].sub_qual = 0;
                drv53115_udf_info[i].sub_field = 0;
                udf_used++;
            }
        }

        drv53115_slice_info[slice_id].slice_udf_free += udf_used;
    }
    
    return SOC_E_NONE;
}



