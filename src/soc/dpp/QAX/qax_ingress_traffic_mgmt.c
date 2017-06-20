/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: qax_ingress_traffic_mgmt.c
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

/*************
 * INCLUDES  *
 *************/
#include <shared/bsl.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/QAX/qax_ingress_traffic_mgmt.h>
#include <soc/dpp/ARAD/arad_ingress_traffic_mgmt.h>
#include <soc/dpp/JER/jer_ingress_traffic_mgmt.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dpp/mbcm.h>

/***********
 * DEFINES *
 * {       *
 ***********/

/* VSQ man-exp field's number of mantissa bits */
#define CGM_ITM_NOF_MANTISSA_BITS  8
/* VSQ Words 16B resolution */
#define CGM_ITM_VSQ_WORDS_RESOLUTION    16

#define QAX_ITM_GRNT_BYTES_MAX (0xFFFFFFFF) /*(16 * 256 * 1024 * 1024 -1)*/ /* 2^28 words -- in bytes */
#define QAX_ITM_GRNT_SRAM_BYTES_MAX (16 * 256 * 1024) /* 2^18 words -- in bytes */
#define QAX_ITM_GRNT_SRAM_PDS_MAX (32 * 1024) /* 2^15 */

#define QAX_ITM_QUEUE_SIZE_BYTES_MAX (0xFFFFFFFF) /*(16 * 256 * 1024 * 1024 - 1)*/ /* 2^28 words -- in bytes */
#define QAX_ITM_QUEUE_SIZE_SRAM_BYTES_MAX (16 * 256 * 1024) /* 2^18 words -- in bytes */
#define QAX_ITM_QUEUE_SIZE_SRAM_PDS_MAX (32 * 1024) /* 2^15 */

#define QAX_ITM_WRED_QT_DP_INFO_MAX_PROBABILITY_MAX 100
#define QAX_ITM_Q_WRED_INFO_MIN_AVRG_TH_MAX (3 * 0x80000000)
#define QAX_ITM_Q_WRED_INFO_MAX_AVRG_TH_MAX (3 * 0x80000000)


/* Up to 192M Words (Word=16B) */
/* Number of buffers * buffer size in bytes / word size */
#define CGM_ITM_WORDS_SIZE_MAX(unit, core)      (((core == 0) ? \
                                                      (SOC_DPP_CONFIG(unit)->jer->dbuffs.dbuffs_bdries.mnmc_0.size) /* Number of buffers */ \
                                                      : (SOC_DPP_CONFIG(unit)->jer->dbuffs.dbuffs_bdries.mnmc_1.size)) \
                                                     * (SOC_DPP_CONFIG(unit)->arad->init.dram.dbuff_size /* buffer size in bytes -- 4K for QAX */ \
                                                     / CGM_ITM_VSQ_WORDS_RESOLUTION))
/* Words - Number of bytes / word size */
#define CGM_ITM_VSQ_WORDS_SIZE_MAX(unit)   (SOC_DPP_DEFS_GET(unit, hw_dram_interfaces_max) * 1024 * 1024 * 1024 \
                                                / CGM_ITM_VSQ_WORDS_RESOLUTION - 1)
/* 16K SRAM-Buffers (Buffer=256B) */
#define CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX(unit) (SOC_DPP_DEFS_GET(unit, ocb_memory_size) \
                                                    * (1024 * 1024 / 8 /* MBits -> Bytes */ \
                                                        / SOC_DPP_CONFIG(unit)->arad->init.ocb.databuffer_size) /* buffer size in bytes */ \
                                                        - 1)
/* 32K SRAM-PDs */
#define CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX      (32 * 1024 - 1)
/* 16K SRAM-PDBs (PDB=2*PD) */
#define CGM_ITM_VSQ_SRAM_PDBS_SIZE_MAX     (CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX / 2)
/* 96K DRAM-BDBs (BDB=8BD) */
#define CGM_ITM_VSQ_DRAM_BDBS_SIZE_MAX     (96 * 1024 - 1)

#define ITM_FADT_MAX_ALPHA  (7)
#define ITM_FADT_MIN_ALPHA  (-7)

#define QAX_ITM_VSQ_PG_OFFSET_FIELD_SIZE  48

/* PB VSQ Flow Control have 2 priorities - LP, HP */
#define CGM_ITM_PB_VSQ_POOL_FC_NOF_PRIO (2)


/* scheduler delta range */
#define QAX_ITM_SCHEDULER_DELTA_MAX                 (127)
#define QAX_ITM_SCHEDULER_DELTA_MIN                 (-128)
#define QAX_ITM_SCHEDULER_DELTA_NOF_BITS            (8)
#define CGM_HAP_NOF_ENTRIES                     (256)


typedef struct {
    soc_mem_t mem_id;
    soc_field_t field_id;
    int mantissa_bits; 
    int exp_bits;
    int factor;
} itm_mantissa_exp_threshold_info;

/*
 * Those defines are used as keys to enter the CGM_<VOQ|VSQ>_GRNTD_RJCT_MASKm, 
 * more details are available in the register file. if an API will be needed in 
 * the future those defines should move from C file to relevant header, currently 
 * they are internal.
 */
#define QAX_SRAM_PDS_IN_GUARANTEED       0x1
#define QAX_SRAM_WORDS_IN_GUARANTEED     0x2
#define QAX_WORDS_IN_GUARANTEED          0x4

/*
 * This enum is used to define each bit in the CGM_<..>_RJCT_MASKm and is currently internal, 
 * if in the future an API is needed to control those Reject masks the enum needs to move from 
 * C file to relevant header. 
 */
typedef enum {
    QAX_VOQ_SRAM_PDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT          = 0,
    QAX_VOQ_SRAM_PDS_MAX_SIZE_REJECT_BIT                        = 1,
    QAX_VOQ_SRAM_WORDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT        = 2,
    QAX_VOQ_SRAM_WORDS_MAX_SIZE_REJECT_BIT                      = 3,
    QAX_VOQ_WORDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT             = 4,
    QAX_VOQ_WORDS_MAX_SIZE_REJECT_BIT                           = 5,
    QAX_VOQ_SYSTEM_RED_REJECT_BIT                               = 6,
    QAX_VOQ_WRED_REJECT_BIT                                     = 7,
    QAX_VOQ_DRAM_BLOCK_REJECT_BIT                               = 8,
    QAX_PB_VSQ_SRAM_PDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT       = 9,
    QAX_PB_VSQ_SRAM_PDS_MAX_SIZE_REJECT_BIT                     = 10,
    QAX_VSQ_D_SRAM_PDS_MAX_SIZE_REJECT_BIT                      = 11,
    QAX_VSQ_C_SRAM_PDS_MAX_SIZE_REJECT_BIT                      = 12,
    QAX_VSQ_B_SRAM_PDS_MAX_SIZE_REJECT_BIT                      = 13,
    QAX_VSQ_A_SRAM_PDS_MAX_SIZE_REJECT_BIT                      = 14,
    QAX_PB_VSQ_SRAM_BUFFERS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT   = 15,
    QAX_PB_VSQ_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT                 = 16,
    QAX_VSQ_D_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT                  = 17,
    QAX_VSQ_C_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT                  = 18,
    QAX_VSQ_B_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT                  = 19,
    QAX_VSQ_A_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT                  = 20,
    QAX_PB_VSQ_WORDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT          = 21,
    QAX_PB_VSQ_WORDS_MAX_SIZE_REJECT_BIT                        = 22,
    QAX_VSQ_D_WORDS_MAX_SIZE_REJECT_BIT                         = 23,
    QAX_VSQ_C_WORDS_MAX_SIZE_REJECT_BIT                         = 24,
    QAX_VSQ_B_WORDS_MAX_SIZE_REJECT_BIT                         = 25,
    QAX_VSQ_A_WORDS_MAX_SIZE_REJECT_BIT                         = 26,
    QAX_VSQ_F_WORDS_WRED_REJECT_BIT                             = 27,
    QAX_VSQ_E_WORDS_WRED_REJECT_BIT                             = 28,
    QAX_VSQ_D_WORDS_WRED_REJECT_BIT                             = 29,
    QAX_VSQ_C_WORDS_WRED_REJECT_BIT                             = 30,
    QAX_VSQ_B_WORDS_WRED_REJECT_BIT                             = 31,
    QAX_VSQ_A_WORDS_WRED_REJECT_BIT                             = 32,
    QAX_DRAM_BDBS_OCCUPANCY_REJECT_BIT                          = 33,
    QAX_SRAM_BUFFERS_OCCUPANCY_REJECT_BIT                       = 34,
    QAX_SRAM_PDBS_OCCUPANCY_REJECT_BIT                          = 35,
    QAX_CFG_MAX_DP_LEVEL_REJECT_BIT                             = 36,
    QAX_CGM_NOF_ADMISSION_TESTS                                 = 37
} cgm_reject_admission_tests_e;

/* 
 * The tables CGM_PB_VSQ_<WORDS|SRAM_BUFFERS|SRAM_PDS>_RJCT_MAP
 * have bitmaps, where each bit correspond to a specific scenario
 * in which the packet should be dropped for PB-VSQ.
 * Each case represent a bit in the number of the resulting bit in the bitmap,
 * so combining several cases results in a number.
 * Example:
 *  Lets take the case where guaranteed area is blocked (full) and also the
 *  total shared is blocked (full), and we don't have headroom. In that case
 *  we want to reject the packet.
 *  This scenario is mapped in the bitmap into bit = 
 *  CGM_ITM_WORDS_VSQ_GRNTD_BLOCKED | CGM_ITM_WORDS_TOTAL_SHARED_BLOCKED = 
 *  0x40 | 0x1 = 0x41 = 65 --> Need to set bit 65 in the bitmap.
 *
 * See tables definition for for information.
 */
/* Bits representation for Words */
#define CGM_ITM_WORDS_TOTAL_SHARED_BLOCKED          0x1
#define CGM_ITM_WORDS_PORT_PG_SHARED_BLOCKED        0x2
#define CGM_ITM_WORDS_TOTAL_HEADROOM_BLOCKED        0x4
#define CGM_ITM_WORDS_PORT_PG_HEADROOM_BLOCKED      0x8
#define CGM_ITM_WORDS_VOQ_WORDS_IN_GRNTD            0x10
#define CGM_ITM_WORDS_VSQ_GRNTD_PRE_BLOCKED         0x20
#define CGM_ITM_WORDS_VSQ_GRNTD_BLOCKED             0x40

/* Bits representation for SRAM-Buffers & SRAM-PDs */
#define CGM_ITM_SRAM_TOTAL_SHARED_BLOCKED          0x1
#define CGM_ITM_SRAM_PORT_PG_SHARED_BLOCKED        0x2
#define CGM_ITM_SRAM_HEADROOM_EXTENSION_BLOCKED    0x4
#define CGM_ITM_SRAM_TOTAL_HEADROOM_BLOCKED        0x8
#define CGM_ITM_SRAM_PORT_HEADROOM_BLOCKED         0x10
#define CGM_ITM_SRAM_PG_HEADROOM_NOMINAL_BLOCKED   0x20
#define CGM_ITM_SRAM_VOQ_IN_GRNTD                  0x40
#define CGM_ITM_SRAM_VSQ_GRNTD_PRE_BLOCKED         0x80
#define CGM_ITM_SRAM_VSQ_GRNTD_BLOCKED             0x100

/* 
 * When guaranteed DP mask bit is set, the guaranteed area will be blocked
 * for the corresponding DP, so it won't be able to use it.
 * Guaranteed DP mask should be set for DP2 & DP3 (Architecture feedback),
 * but because of a HW bug this feature doesn't work for QAX.
 */
#define CGM_ITM_GRNTD_DP_MASK   0x0

/*
 * If set, resource will be defined as blocked when free space smaller than
 * packet size. Otherwise packet can use this resource in case admitted.
 * Consider moving this to a soc property, so user can define if he wants
 * resource overflow or not.
 */
#define CGM_ITM_RESOURCE_LIMIT_IS_STRICT  0

/***********
 * DEFINES *
 * }       *
 ***********/

/********************
 * STATIC FUNCTIONS *
 * {                *
 ********************/

STATIC  uint32 
  _qax_itm_alpha_to_field(
     int     unit,
     int     alpha
  );

STATIC int 
  _qax_itm_field_to_alpha(
     int     unit,
     uint32 alpha_field
  );

STATIC int 
qax_itm_fc_global_thresholds_init(int unit);

STATIC int
qax_itm_drop_global_thresholds_init(int unit);

STATIC void
  _qax_itm_hw_data_to_wred_info (
    SOC_SAND_IN  int                                                  unit,
    SOC_SAND_IN  uint32                                               min_avrg_th,
    SOC_SAND_IN  uint32                                               max_avrg_th,
    SOC_SAND_IN  uint32                                               c1,
    SOC_SAND_IN  uint32                                               c2,
    SOC_SAND_IN  uint32                                               c3,
    SOC_SAND_OUT SOC_TMC_ITM_WRED_QT_DP_INFO                          *wred_param
  );

STATIC void
  _qax_itm_wred_info_to_hw_data (
     SOC_SAND_IN   int unit,
     SOC_SAND_IN   SOC_TMC_ITM_WRED_QT_DP_INFO                          *wred_param,
     SOC_SAND_IN   uint32                                               min_avrg_th_exact,
     SOC_SAND_IN   uint32                                               max_avrg_th_exact,
     SOC_SAND_OUT  uint32*                                              c1,
     SOC_SAND_OUT  uint32*                                              c2,
     SOC_SAND_OUT  uint32*                                              c3
  );

STATIC uint32 
  _qax_itm_mantissa_exp_field_set(
    int                  unit,
    itm_mantissa_exp_threshold_info* info,
    int round_up,
    void *data,
    uint32 threshold, 
    uint32* result_threshold
    );

STATIC int
  qax_itm_vsq_rjct_man_exp_set(
    int                                 unit,
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
    soc_mem_t                           mem,
    soc_field_t                         field,
    uint32                              entry_offset,
    uint32                              threshold,
    uint32                              *result_threshold
  );

STATIC int
  qax_itm_vsq_rjct_man_exp_get(
    int                                 unit,
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
    soc_mem_t                           mem,
    soc_field_t                         field,
    uint32                              entry_offset,
    uint32                              *result_threshold
  );

STATIC int
  qax_itm_vsq_rjct_fadt_set(
    int                              unit,
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type,
    soc_mem_t                        mem,
    SOC_TMC_ITM_VSQ_FADT_FIELDS_INFO fadt_fields,
    uint32                           entry_offset,
    SOC_TMC_FADT_INFO                fadt_info,
    SOC_TMC_FADT_INFO                *exact_fadt_info
  );

STATIC int
  qax_itm_vsq_rjct_fadt_get(
    int                              unit,
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type,
    soc_mem_t                        mem,
    SOC_TMC_ITM_VSQ_FADT_FIELDS_INFO fadt_fields,
    uint32                           entry_offset,
    SOC_TMC_FADT_INFO                *exact_fadt_info
  );

STATIC int
  qax_itm_vsq_pg_headroom_rjct_set(
    int                                 unit,
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
    soc_mem_t                           mem,
    uint32                              entry_offset,
    SOC_TMC_ITM_VSQ_PG_HDRM_INFO        headroom_info,
    SOC_TMC_ITM_VSQ_PG_HDRM_INFO        *exact_headroom_info
  );

STATIC int
  qax_itm_vsq_pg_headroom_rjct_get(
    int                                 unit,
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
    soc_mem_t                           mem,
    uint32                              entry_offset,
    SOC_TMC_ITM_VSQ_PG_HDRM_INFO        *exact_headroom_info
  );

STATIC int
  qax_itm_vsq_qt_rt_cls_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_NDX    vsq_in_group_ndx
  );

STATIC int 
  qax_itm_category_rngs_verify( 
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  SOC_TMC_ITM_CATEGORY_RNGS *info 
  ); 

STATIC int
  qax_itm_glob_rcs_drop_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  SOC_TMC_ITM_GLOB_RCS_DROP_TH *info
  );

STATIC int
  soc_qax_itm_reserved_resource_init(
    int unit
  );

STATIC int
  qax_itm_resource_allocation_set(
    SOC_SAND_IN int                     unit,
    SOC_SAND_IN int                     core_id,
    SOC_SAND_IN SOC_TMC_ITM_INGRESS_CONGESTION_MGMT *ingress_congestion_mgmt
  ); 

STATIC int
  qax_itm_resource_allocation_get(
    SOC_SAND_IN int                     unit,
    SOC_SAND_IN int                     core_id,
    SOC_SAND_OUT SOC_TMC_ITM_INGRESS_CONGESTION_MGMT *ingress_congestion_mgmt
  ); 

STATIC int
  qax_itm_resource_allocation_verify(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int core,
    SOC_SAND_IN SOC_TMC_ITM_INGRESS_CONGESTION_MGMT *ingress_congestion_mgmt
  );

STATIC int
  qax_itm_vsq_fc_verify(
    SOC_SAND_IN int                        unit,
    SOC_SAND_IN SOC_TMC_ITM_VSQ_GROUP      vsq_group_ndx,
    SOC_SAND_IN uint32                     vsq_rt_cls_ndx,
    SOC_SAND_IN int                        pool_id,
    SOC_SAND_IN SOC_TMC_ITM_VSQ_FC_INFO    *info
  );

/********************
 * STATIC FUNCTIONS *
 * }                *
 ********************/




itm_mantissa_exp_threshold_info voq_guaranteed_th_mant_exp[] = {
    {/* words */
        CGM_VOQ_GRNTD_PRMSm,
        GRNTD_WORDS_THf,
        8, /* mantissa_bits */
        5, /* exp_bits */
        16 /* factor */
    },
    {/* sram_words */
        CGM_VOQ_GRNTD_PRMSm,
        GRNTD_SRAM_WORDS_THf,
        8, /* mantissa_bits */
        4, /* exp_bits */
        16 /* factor */
    },
    
    {/* sram_pds */
        CGM_VOQ_GRNTD_PRMSm,
        GRNTD_SRAM_PDS_THf,
        8, /* mantissa_bits */
        4, /* exp_bits */
        1 /* factor */
    }
};

itm_mantissa_exp_threshold_info voq_fadt_max_th_mant_exp[] = {
    {/* max words */
        CGM_VOQ_WORDS_RJCT_PRMSm,
        VOQ_FADT_MAX_THf,
        8, /* mantissa_bits */
        5, /* exp_bits */
        16 /* factor */
    },
    {/* max sram_words */
        CGM_VOQ_SRAM_WORDS_RJCT_PRMSm,
        VOQ_FADT_MAX_THf,
        8, /* mantissa_bits */
        5, /* exp_bits */
        16 /* factor */
    },
    
    {/* max sram_pds */
        CGM_VOQ_SRAM_PDS_RJCT_PRMSm,
        VOQ_FADT_MAX_THf,
        8, /* mantissa_bits */
        5, /* exp_bits */
        1 /* factor */
    }
};

itm_mantissa_exp_threshold_info voq_fadt_min_th_mant_exp[] = {
   
    { /* min words */ 
         CGM_VOQ_WORDS_RJCT_PRMSm,
         VOQ_FADT_MIN_THf,
         8, /* mantissa_bits */
         5, /* exp_bits */
         16 /* factor */
    },
    
    { /* min sram_words */
        CGM_VOQ_SRAM_WORDS_RJCT_PRMSm,
        VOQ_FADT_MIN_THf,
        8, /* mantissa_bits */
        5, /* exp_bits */
        16 /* factor */
    },
    
    {/* min sram_pds */
        CGM_VOQ_SRAM_PDS_RJCT_PRMSm,
        VOQ_FADT_MIN_THf,
        8, /* mantissa_bits */
        5, /* exp_bits */
        1  /* factor */
    }
};

itm_mantissa_exp_threshold_info voq_wred_ecn_max_th_mant_exp[] = 
{
    { /* wred max threshold */ 
        CGM_VOQ_WORDS_RJCT_PRMSm,
        VOQ_WRED_MAX_THf,
        8, /* mantissa_bits */
        5, /* exp_bits */
        16 /* factor */
    },
    { /* ecn max threshold */ 
        CGM_CNI_PRMSm,
        WRED_MAX_THf,
        8, /* mantissa_bits */
        5, /* exp_bits */
        16 /* factor */
    }
};

itm_mantissa_exp_threshold_info voq_wred_ecn_min_th_mant_exp[] = 
{
    { /* wred min threshold */ 
         CGM_VOQ_WORDS_RJCT_PRMSm,
         VOQ_WRED_MIN_THf,
         8, /* mantissa_bits */
         5, /* exp_bits */
         16 /* factor */
    },
    { /* ecn min threshold */ 
        CGM_CNI_PRMSm,
        WRED_MIN_THf,
        8, /* mantissa_bits */
        5, /* exp_bits */
        16 /* factor */
    }
};

itm_mantissa_exp_threshold_info voq_wred_ecn_max_size_th_mant_exp = 
{ /* ecn max size threshold */ 
        CGM_CNI_PRMSm,
        MAX_SIZE_THf,
        8, /* mantissa_bits */
        5, /* exp_bits */
        16 /* factor */
};

/* -------------------------------------
   DRAM bound 
   -------------------------------------
*/

itm_mantissa_exp_threshold_info voq_dram_bound_qsize_recovery_th_mant_exp = 
{
    CGM_VOQ_DRAM_BOUND_PRMSm,
    QSIZE_RECOVERY_THf,
    8, /* mantissa_bits */
    4, /* exp_bits */
    16 /* factor */
};




STATIC  uint32 _qax_itm_alpha_to_field(
     int     unit,
     int     alpha)
{
    uint32 value_mask = 0x7;
    uint32 sign;
    uint32 alpha_field;

    /*            DESC          => "If AdjustFactor3 is set,  
     Dynamic-Max-Th = Free-Resource << AdjustFactor2:0 
 Otherwise, 
     Dynamic-Max-Th = Free-Resource >> AdjustFactor2:0"
    */
    if (alpha >= 0) {
        sign = 1;
    }
    else {
        sign = 0;
        alpha = -alpha;
    }
    alpha_field = (sign << 3) | (alpha & value_mask);

    return alpha_field;
}

STATIC int _qax_itm_field_to_alpha(
     int     unit,
     uint32 alpha_field)
{
    uint32 value_mask = 0x7;
    uint32 sign_mask = 1 << 3;
    int alpha;

    /*            DESC          => "If AdjustFactor3 is set,  
     Dynamic-Max-Th = Free-Resource << AdjustFactor2:0 
 Otherwise, 
     Dynamic-Max-Th = Free-Resource >> AdjustFactor2:0"
    */
 
    alpha = alpha_field & value_mask;
    if ((alpha_field & sign_mask) == 0) {
        alpha = -(alpha);
    }

    return alpha;
}


STATIC uint32  _qax_itm_mantissa_exp_field_set(
    int                  unit,
    itm_mantissa_exp_threshold_info* info,
    int round_up,
    void *data,
    uint32 threshold, 
    uint32* result_threshold
    )
{
    uint32    exp_man, mnt = 0, exp = 0;
    
    SOCDNX_INIT_FUNC_DEFS;

    if (round_up) {
        SOCDNX_SAND_IF_ERR_EXIT(soc_sand_break_to_mnt_exp_round_up(
                                    SOC_SAND_DIV_ROUND_UP(threshold, info->factor),
                                    info->mantissa_bits,
                                    info->exp_bits,
                                    0, &mnt, &exp
                                    ));
    } else {
        SOCDNX_SAND_IF_ERR_EXIT(soc_sand_break_to_mnt_exp_round_down(
                                    threshold/info->factor,
                                    info->mantissa_bits,
                                    info->exp_bits,
                                    0, &mnt, &exp
                                    ));
    }

    /* Write them according to the rate class entry */
    arad_iqm_mantissa_exponent_set(unit, mnt, exp, info->mantissa_bits, &exp_man);
    soc_mem_field32_set(unit, info->mem_id, data, info->field_id, exp_man);
    *result_threshold = mnt * (info->factor << exp);

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC void  _qax_itm_mantissa_exp_field_get(
    int                  unit,
    itm_mantissa_exp_threshold_info* info,
    void *data,
    uint32* result_threshold
    )
{
    uint32    exp_man, mnt = 0, exp = 0;
    
    exp_man = soc_mem_field32_get(unit, info->mem_id, data, info->field_id);
    arad_iqm_mantissa_exponent_get(unit, exp_man, info->mantissa_bits, &mnt, &exp);
    *result_threshold = mnt * (info->factor << exp);
}



/*! ****************************************************
* \brief
*   Generate a reject mask for relevant CGM memories.
*
*  \par DATE: 
*       28/JUNE/2016 \n
*  \par DIRECT INPUT:
*    \param [in] unit 
*    \param [in] is_voq: true if voq, false if vsq.
*    \param [in] is_over_voq: true if is_over_voq.
*    \param [in] is_over_vsq: true if is_over_vsq.
*    \param [in] words_in_guaranteed: true if words_in_guaranteed.
*    \param [in] sram_words_in_guaranteed: true if sram_words_in_guaranteed.
*    \param [in] sram_pds_in_guaranteed: true if sram_pds_in_guaranteed.
*    \param [in-out] mem_mask: ptr to bit-map that will contain result mask.
*  \par DIRECT OUTPUT:
*    int - SOC_E_XXX error code
*  \par INDIRECT INPUT:
*   * None 
*  \par INDIRECT OUTPUT:
*    return by reference the result mask to be set in the relevant memory.
*  \remark
*    should be used to create entries for CGM_<VOQ|VSQ>_GRNTD_RJCT_MASKm memories
*  \see
*   * None
*****************************************************/
int qax_itm_cgm_guaranteed_reject_mask_create(int unit, int is_voq, int is_over_voq, int is_over_vsq, int words_in_guaranteed, int sram_words_in_guaranteed, int sram_pds_in_guaranteed, SHR_BITDCL* mem_mask)
{
    int in_guaranteed = words_in_guaranteed || sram_words_in_guaranteed || sram_pds_in_guaranteed;

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_NULL_CHECK(mem_mask);

    SHR_BITCLR_RANGE(mem_mask, 0, QAX_CGM_NOF_ADMISSION_TESTS);
    SHR_BITWRITE(mem_mask, QAX_VOQ_SRAM_PDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT,          (!((is_over_voq && in_guaranteed) || sram_pds_in_guaranteed)));
    SHR_BITWRITE(mem_mask, QAX_VOQ_SRAM_PDS_MAX_SIZE_REJECT_BIT,                        (!((is_over_voq && in_guaranteed) || sram_pds_in_guaranteed)));
    SHR_BITWRITE(mem_mask, QAX_VOQ_SRAM_WORDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT,        (!((is_over_voq && in_guaranteed) || sram_words_in_guaranteed)));
    SHR_BITWRITE(mem_mask, QAX_VOQ_SRAM_WORDS_MAX_SIZE_REJECT_BIT,                      (!((is_over_voq && in_guaranteed) || sram_words_in_guaranteed)));
    SHR_BITWRITE(mem_mask, QAX_VOQ_WORDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT,             (!((is_over_voq && in_guaranteed) || words_in_guaranteed)));
    SHR_BITWRITE(mem_mask, QAX_VOQ_WORDS_MAX_SIZE_REJECT_BIT,                           (!((is_over_voq && in_guaranteed) || words_in_guaranteed)));
    SHR_BITWRITE(mem_mask, QAX_VOQ_SYSTEM_RED_REJECT_BIT,                               1);
    SHR_BITWRITE(mem_mask, QAX_VOQ_WRED_REJECT_BIT,                                     (!(is_voq && in_guaranteed)));
    SHR_BITWRITE(mem_mask, QAX_VOQ_DRAM_BLOCK_REJECT_BIT,                               1);
    SHR_BITWRITE(mem_mask, QAX_PB_VSQ_SRAM_PDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT,       (!(is_voq ? (is_over_vsq && sram_pds_in_guaranteed) : ((is_over_vsq && in_guaranteed) || sram_pds_in_guaranteed))));
    SHR_BITWRITE(mem_mask, QAX_PB_VSQ_SRAM_PDS_MAX_SIZE_REJECT_BIT,                     (!(is_voq ? (is_over_vsq && sram_pds_in_guaranteed) : ((is_over_vsq && in_guaranteed) || sram_pds_in_guaranteed))));
    SHR_BITWRITE(mem_mask, QAX_VSQ_D_SRAM_PDS_MAX_SIZE_REJECT_BIT,                      !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_VSQ_C_SRAM_PDS_MAX_SIZE_REJECT_BIT,                      !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_VSQ_B_SRAM_PDS_MAX_SIZE_REJECT_BIT,                      !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_VSQ_A_SRAM_PDS_MAX_SIZE_REJECT_BIT,                      !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_PB_VSQ_SRAM_BUFFERS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT,   (!(is_voq ? (is_over_vsq && sram_words_in_guaranteed) : ((is_over_vsq && in_guaranteed) || sram_words_in_guaranteed))));
    SHR_BITWRITE(mem_mask, QAX_PB_VSQ_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT,                 (!(is_voq ? (is_over_vsq && sram_words_in_guaranteed) : ((is_over_vsq && in_guaranteed) || sram_words_in_guaranteed))));
    SHR_BITWRITE(mem_mask, QAX_VSQ_D_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT,                  !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_VSQ_C_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT,                  !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_VSQ_B_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT,                  !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_VSQ_A_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT,                  !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_PB_VSQ_WORDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT,          (!(is_voq ? (is_over_vsq && words_in_guaranteed) : ((is_over_vsq && in_guaranteed) || words_in_guaranteed))));
    SHR_BITWRITE(mem_mask, QAX_PB_VSQ_WORDS_MAX_SIZE_REJECT_BIT,                        (!(is_voq ? (is_over_vsq && words_in_guaranteed) : ((is_over_vsq && in_guaranteed) || words_in_guaranteed))));
    SHR_BITWRITE(mem_mask, QAX_VSQ_D_WORDS_MAX_SIZE_REJECT_BIT,                         !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_VSQ_C_WORDS_MAX_SIZE_REJECT_BIT,                         !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_VSQ_B_WORDS_MAX_SIZE_REJECT_BIT,                         !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_VSQ_A_WORDS_MAX_SIZE_REJECT_BIT,                         !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_VSQ_F_WORDS_WRED_REJECT_BIT,                             !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_VSQ_E_WORDS_WRED_REJECT_BIT,                             !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_VSQ_D_WORDS_WRED_REJECT_BIT,                             !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_VSQ_C_WORDS_WRED_REJECT_BIT,                             !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_VSQ_B_WORDS_WRED_REJECT_BIT,                             !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_VSQ_A_WORDS_WRED_REJECT_BIT,                             !in_guaranteed);
    SHR_BITWRITE(mem_mask, QAX_DRAM_BDBS_OCCUPANCY_REJECT_BIT,                          1);
    SHR_BITWRITE(mem_mask, QAX_SRAM_BUFFERS_OCCUPANCY_REJECT_BIT,                       1);
    SHR_BITWRITE(mem_mask, QAX_SRAM_PDBS_OCCUPANCY_REJECT_BIT,                          1);
    SHR_BITWRITE(mem_mask, QAX_CFG_MAX_DP_LEVEL_REJECT_BIT,                             1);
        
exit:
    SOCDNX_FUNC_RETURN;
}

/*! ****************************************************
* \brief
*   Sets ITM admission tests defaults.
*
*  \par DATE: 
*       28/JUNE/2016 \n
*  \par DIRECT INPUT:
*    \param [in] unit 
*  \par DIRECT OUTPUT:
*    int - SOC_E_XXX error code
*  \par INDIRECT INPUT:
*    * None 
*  \par INDIRECT OUTPUT:
*    configures defaults for CGM admission tests according to pre defined logic 
*    decided by the Arch team.
*  \remark
*    * None
*  \see
*    * None
*****************************************************/
int qax_itm_admission_tests_defaults_set (int unit)
{
    int enforce_admission_test_loosly = 0; 
    int voq_over_voq;
    int voq_over_vsq;
    int vsq_over_voq;
    int vsq_over_vsq;
    int words_in_guaranteed;
    int sram_words_in_guaranteed;
    int sram_pds_in_guaranteed;
    int mem_index;
    int max_mem_index;
    soc_mem_t mem;
    SHR_BITDCLNAME(mem_mask, QAX_CGM_NOF_ADMISSION_TESTS);

    SOCDNX_INIT_FUNC_DEFS;

    enforce_admission_test_loosly = (SOC_DPP_CONFIG(unit)->jer->tm.cgm_mgmt_guarantee_mode == SOC_TMC_ITM_CGM_MGMT_GUARANTEE_LOOSE);
    /* 
     * in the future we will might want to make desicions with a finer resolution regarding the admission test. 
     * therefore a preperation for such an ocassion is allready present here in the form of voq_over_voq etc 
     */
    voq_over_voq = voq_over_vsq = vsq_over_voq = vsq_over_vsq = enforce_admission_test_loosly;

    /* Set reject mask for CGM_VOQ_GRNTD_RJCT_MASK */
    max_mem_index = soc_mem_index_max(unit, CGM_VOQ_GRNTD_RJCT_MASKm);
    mem = CGM_VOQ_GRNTD_RJCT_MASKm;
    for (mem_index = 0; mem_index <= max_mem_index; ++mem_index) 
    {
        words_in_guaranteed = mem_index & QAX_WORDS_IN_GUARANTEED;
        sram_words_in_guaranteed = mem_index & QAX_SRAM_WORDS_IN_GUARANTEED;
        sram_pds_in_guaranteed = mem_index & QAX_SRAM_PDS_IN_GUARANTEED;
        SOCDNX_IF_ERR_EXIT( qax_itm_cgm_guaranteed_reject_mask_create(unit, 1, voq_over_voq, voq_over_vsq, words_in_guaranteed, sram_words_in_guaranteed, sram_pds_in_guaranteed, mem_mask));
        soc_mem_write(unit, mem, MEM_BLOCK_ALL, mem_index, mem_mask);
    }

    /* Set reject mask for CGM_VSQ_GRNTD_RJCT_MASK */
    max_mem_index = soc_mem_index_max(unit, CGM_VSQ_GRNTD_RJCT_MASKm);
    mem = CGM_VSQ_GRNTD_RJCT_MASKm;
    for (mem_index = 0; mem_index <= max_mem_index; ++mem_index) 
    {
        words_in_guaranteed = mem_index & QAX_WORDS_IN_GUARANTEED;
        sram_words_in_guaranteed = mem_index & QAX_SRAM_WORDS_IN_GUARANTEED;
        sram_pds_in_guaranteed = mem_index & QAX_SRAM_PDS_IN_GUARANTEED;
        SOCDNX_IF_ERR_EXIT( qax_itm_cgm_guaranteed_reject_mask_create(unit, 0, vsq_over_voq, vsq_over_vsq, words_in_guaranteed, sram_words_in_guaranteed, sram_pds_in_guaranteed, mem_mask));
        soc_mem_write(unit, mem, MEM_BLOCK_ALL, mem_index, mem_mask);
    }

    /* Set Lossless (8) and default (0) */
    SHR_BITSET_RANGE(mem_mask, 0, QAX_CGM_NOF_ADMISSION_TESTS);
    WRITE_CGM_PB_VSQ_RJCT_MASKm(unit, MEM_BLOCK_ALL, 0, mem_mask);
    SHR_BITCLR_RANGE(mem_mask, 0, QAX_DRAM_BDBS_OCCUPANCY_REJECT_BIT);
    SHR_BITSET(mem_mask, QAX_PB_VSQ_SRAM_PDS_MAX_SIZE_REJECT_BIT);
    SHR_BITSET(mem_mask, QAX_PB_VSQ_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT);
    SHR_BITSET(mem_mask, QAX_PB_VSQ_WORDS_MAX_SIZE_REJECT_BIT);
    WRITE_CGM_PB_VSQ_RJCT_MASKm(unit, MEM_BLOCK_ALL, 8, mem_mask);

    /* Set PP Admission profiles ECN/ECI (1) and default (0) */
    SHR_BITSET_RANGE(mem_mask, 0, QAX_CGM_NOF_ADMISSION_TESTS);
    WRITE_CGM_PP_RJCT_MASKm(unit, MEM_BLOCK_ALL, 0, mem_mask);
    SHR_BITCLR_RANGE(mem_mask, 0, QAX_CGM_NOF_ADMISSION_TESTS);
    SHR_BITSET_RANGE(mem_mask, 0, QAX_CGM_NOF_ADMISSION_TESTS);
    SHR_BITCLR(mem_mask, QAX_VOQ_WRED_REJECT_BIT);
    SHR_BITCLR(mem_mask, QAX_VSQ_A_WORDS_MAX_SIZE_REJECT_BIT);
    SHR_BITCLR(mem_mask, QAX_VSQ_B_WORDS_MAX_SIZE_REJECT_BIT);
    SHR_BITCLR(mem_mask, QAX_VSQ_C_WORDS_MAX_SIZE_REJECT_BIT);
    SHR_BITCLR(mem_mask, QAX_VSQ_D_WORDS_MAX_SIZE_REJECT_BIT);
    SHR_BITCLR(mem_mask, QAX_VSQ_A_WORDS_WRED_REJECT_BIT);
    SHR_BITCLR(mem_mask, QAX_VSQ_B_WORDS_WRED_REJECT_BIT);
    SHR_BITCLR(mem_mask, QAX_VSQ_C_WORDS_WRED_REJECT_BIT);
    SHR_BITCLR(mem_mask, QAX_VSQ_D_WORDS_WRED_REJECT_BIT);
    SHR_BITCLR(mem_mask, QAX_VSQ_E_WORDS_WRED_REJECT_BIT);
    SHR_BITCLR(mem_mask, QAX_VSQ_F_WORDS_WRED_REJECT_BIT);
    WRITE_CGM_PP_RJCT_MASKm(unit, MEM_BLOCK_ALL, 1, mem_mask);

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Configuration of CGM_<VOQ|PB_VSQ>_RJCT_SETTINGS according to
 * Architecture feedback.
 * These configurations are used by admission tests.
 */
STATIC
int qax_itm_rjct_setting_defaults_set(int unit)
{
    uint32 data = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(READ_CGM_VOQ_RJCT_SETTINGSr(unit, &data));
    /* Set grntd mask for VOQ - which DPs are not allowed to use the guaranteed area */
    soc_reg_field_set(unit, CGM_VOQ_RJCT_SETTINGSr, &data, VOQ_GRNTD_DP_MASKf, CGM_ITM_GRNTD_DP_MASK);
    /* Set for each VOQ resource if it allows to overflow or not */
    soc_reg_field_set(unit, CGM_VOQ_RJCT_SETTINGSr, &data, VOQ_WORDS_IN_GRNTD_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    soc_reg_field_set(unit, CGM_VOQ_RJCT_SETTINGSr, &data, VOQ_SRAM_WORDS_IN_GRNTD_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    SOCDNX_IF_ERR_EXIT(WRITE_CGM_VOQ_RJCT_SETTINGSr(unit, data));

    SOCDNX_IF_ERR_EXIT(READ_CGM_PB_VSQ_RJCT_SETTINGSr(unit, &data));
    /* Set grntd mask for VSQ - which DPs are not allowed to use the guaranteed area */
    soc_reg_field_set(unit, CGM_PB_VSQ_RJCT_SETTINGSr, &data, PB_VSQ_GRNTD_DP_MASKf, CGM_ITM_GRNTD_DP_MASK);
    /* Set for each VSQ resource if it allows to overflow or not */
    soc_reg_field_set(unit, CGM_PB_VSQ_RJCT_SETTINGSr, &data, PB_VSQ_WORDS_GRNTD_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    soc_reg_field_set(unit, CGM_PB_VSQ_RJCT_SETTINGSr, &data, PB_VSQ_SRAM_BUFFERS_GRNTD_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    soc_reg_field_set(unit, CGM_PB_VSQ_RJCT_SETTINGSr, &data, PB_VSQ_PORT_WORDS_SHRD_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    soc_reg_field_set(unit, CGM_PB_VSQ_RJCT_SETTINGSr, &data, PB_VSQ_PORT_WORDS_HDRM_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    soc_reg_field_set(unit, CGM_PB_VSQ_RJCT_SETTINGSr, &data, PB_VSQ_PG_WORDS_SHRD_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    soc_reg_field_set(unit, CGM_PB_VSQ_RJCT_SETTINGSr, &data, PB_VSQ_PG_WORDS_HDRM_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    soc_reg_field_set(unit, CGM_PB_VSQ_RJCT_SETTINGSr, &data, PB_VSQ_WORDS_SHRD_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    soc_reg_field_set(unit, CGM_PB_VSQ_RJCT_SETTINGSr, &data, PB_VSQ_WORDS_HDRM_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    soc_reg_field_set(unit, CGM_PB_VSQ_RJCT_SETTINGSr, &data, PB_VSQ_PORT_SRAM_BUFFERS_SHRD_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    soc_reg_field_set(unit, CGM_PB_VSQ_RJCT_SETTINGSr, &data, PB_VSQ_PORT_SRAM_BUFFERS_HDRM_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    soc_reg_field_set(unit, CGM_PB_VSQ_RJCT_SETTINGSr, &data, PB_VSQ_PG_SRAM_BUFFERS_SHRD_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    soc_reg_field_set(unit, CGM_PB_VSQ_RJCT_SETTINGSr, &data, PB_VSQ_PG_SRAM_BUFFERS_HDRM_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    soc_reg_field_set(unit, CGM_PB_VSQ_RJCT_SETTINGSr, &data, PB_VSQ_SRAM_BUFFERS_SHRD_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    soc_reg_field_set(unit, CGM_PB_VSQ_RJCT_SETTINGSr, &data, PB_VSQ_SRAM_BUFFERS_HDRM_IS_STRICTf, CGM_ITM_RESOURCE_LIMIT_IS_STRICT);
    SOCDNX_IF_ERR_EXIT(WRITE_CGM_PB_VSQ_RJCT_SETTINGSr(unit, data));

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Configure default values for PB-VSQ reject bitmaps.
 * The bits we set represent a scenario on which we want to
 * reject a packet.
 * The bit to set is dependent on several cases. Each case is represented
 * by a bit in the number the comprise the resulting bit.
 * Combining the cases bits together gives us the resulting bit number.
 */
STATIC
int qax_itm_rjct_map_defaults_set(int unit)
{
    int reject_bit = -1;
    int headroom_bits = 0;
    int sram_headroom_bits = 0;
    int shared_bits = 0;
    int sram_shared_bits = 0;
    int is_resource_limit_strict = CGM_ITM_RESOURCE_LIMIT_IS_STRICT;
    soc_reg_above_64_val_t rjct_bitmap;
    soc_reg_above_64_val_t sram_rjct_bitmap;
    SOCDNX_INIT_FUNC_DEFS;


    /*****************************/
    /* Set reject bits for Words */
    /*****************************/
    if (SOC_DPP_CONFIG(unit)->arad->init.dram.nof_drams != 0) {
        SOC_REG_ABOVE_64_CLEAR(rjct_bitmap);

        /* Set reject bits for lossy (Total-Headroom-State & Port-PG-Headroom-State are BLOCKED in HW) */
        headroom_bits = CGM_ITM_WORDS_TOTAL_HEADROOM_BLOCKED | CGM_ITM_WORDS_PORT_PG_HEADROOM_BLOCKED;

        if (1 == is_resource_limit_strict) {
            /* 
             * Coverity explanation: this code is currently dead, but we plan
             * give the user the option to choose between strict/loose mode via
             * soc property, and we want this section to work when we will do it.
             */
            /* coverity[dead_error_begin:FALSE] */
            reject_bit = CGM_ITM_WORDS_VSQ_GRNTD_PRE_BLOCKED | CGM_ITM_WORDS_TOTAL_SHARED_BLOCKED | headroom_bits;
            SHR_BITSET(rjct_bitmap, reject_bit);
            reject_bit = CGM_ITM_WORDS_VSQ_GRNTD_PRE_BLOCKED | CGM_ITM_WORDS_TOTAL_SHARED_BLOCKED | CGM_ITM_WORDS_PORT_PG_SHARED_BLOCKED | headroom_bits;
            SHR_BITSET(rjct_bitmap, reject_bit);
        }

        reject_bit = CGM_ITM_WORDS_VSQ_GRNTD_BLOCKED | CGM_ITM_WORDS_PORT_PG_SHARED_BLOCKED | headroom_bits;
        SHR_BITSET(rjct_bitmap, reject_bit);
        reject_bit = CGM_ITM_WORDS_VSQ_GRNTD_BLOCKED | CGM_ITM_WORDS_TOTAL_SHARED_BLOCKED | headroom_bits;
        SHR_BITSET(rjct_bitmap, reject_bit);
        reject_bit = CGM_ITM_WORDS_VSQ_GRNTD_BLOCKED | CGM_ITM_WORDS_TOTAL_SHARED_BLOCKED | CGM_ITM_WORDS_PORT_PG_SHARED_BLOCKED | headroom_bits;
        SHR_BITSET(rjct_bitmap, reject_bit);

        /* Set reject bits for the case of lossless and in flow control (Total-Shared-State & Port-PG-Shared-State are BLOCKED in HW) */
        shared_bits = CGM_ITM_WORDS_TOTAL_SHARED_BLOCKED | CGM_ITM_WORDS_PORT_PG_SHARED_BLOCKED;
        reject_bit = CGM_ITM_WORDS_VSQ_GRNTD_BLOCKED | CGM_ITM_WORDS_TOTAL_HEADROOM_BLOCKED | shared_bits;
        SHR_BITSET(rjct_bitmap, reject_bit);
        reject_bit = CGM_ITM_WORDS_VSQ_GRNTD_BLOCKED | CGM_ITM_WORDS_PORT_PG_HEADROOM_BLOCKED | shared_bits;
        SHR_BITSET(rjct_bitmap, reject_bit);
        reject_bit = CGM_ITM_WORDS_VSQ_GRNTD_BLOCKED | CGM_ITM_WORDS_PORT_PG_HEADROOM_BLOCKED | CGM_ITM_WORDS_TOTAL_HEADROOM_BLOCKED | shared_bits;
        SHR_BITSET(rjct_bitmap, reject_bit);

        /* 
         * Each index represent PG.Admit-Profile.
         * PG.Admit-Profile == 0 by default, and currently we don't configure
         * other PG.Admit-Profiles, so change only profile 0.
         */
        SOCDNX_IF_ERR_EXIT(WRITE_CGM_PB_VSQ_WORDS_RJCT_MAPm(unit, MEM_BLOCK_ALL, 0, rjct_bitmap));
    }

    /*************************************************/
    /* Set reject bits for SRAM-Buffers and SRAM-PDs */
    /*************************************************/
    SOC_REG_ABOVE_64_CLEAR(sram_rjct_bitmap);
    /* 
     * Set reject bits for lossy (Total-Shared-State | Total-Headroom-Ext-State | PG-Headroom-Ext-State
     *         & Total-Headroom-State & Port-Headroom-State & PG-Headroom-State are BLOCKED in HW)
     */
    sram_headroom_bits = CGM_ITM_SRAM_HEADROOM_EXTENSION_BLOCKED | CGM_ITM_SRAM_TOTAL_HEADROOM_BLOCKED | CGM_ITM_SRAM_PORT_HEADROOM_BLOCKED | CGM_ITM_SRAM_PG_HEADROOM_NOMINAL_BLOCKED;

    if (1 == is_resource_limit_strict) {
        /* 
         * Coverity explanation: this code is currently dead, but we plan
         * give the user the option to choose between strict/loose mode via
         * soc property, and we want this section to work when we will do it.
         */
        /* coverity[dead_error_begin:FALSE] */
        reject_bit = CGM_ITM_SRAM_VSQ_GRNTD_PRE_BLOCKED | CGM_ITM_SRAM_TOTAL_SHARED_BLOCKED | sram_headroom_bits;
        SHR_BITSET(sram_rjct_bitmap, reject_bit);
        reject_bit = CGM_ITM_SRAM_VSQ_GRNTD_PRE_BLOCKED | CGM_ITM_SRAM_PORT_PG_SHARED_BLOCKED | CGM_ITM_SRAM_TOTAL_SHARED_BLOCKED | sram_headroom_bits;
        SHR_BITSET(sram_rjct_bitmap, reject_bit);
    }
    

    reject_bit = CGM_ITM_SRAM_VSQ_GRNTD_BLOCKED | CGM_ITM_SRAM_PORT_PG_SHARED_BLOCKED | sram_headroom_bits;
    SHR_BITSET(sram_rjct_bitmap, reject_bit);
    reject_bit = CGM_ITM_SRAM_VSQ_GRNTD_BLOCKED | CGM_ITM_SRAM_TOTAL_SHARED_BLOCKED | sram_headroom_bits;
    SHR_BITSET(sram_rjct_bitmap, reject_bit);
    reject_bit = CGM_ITM_SRAM_VSQ_GRNTD_BLOCKED | CGM_ITM_SRAM_PORT_PG_SHARED_BLOCKED | CGM_ITM_SRAM_TOTAL_SHARED_BLOCKED | sram_headroom_bits;
    SHR_BITSET(sram_rjct_bitmap, reject_bit);

    /* Set reject bit for lossless and in flow control (Total-Shared-State & Port-PG-Shared-State are BLOCKED in HW) */
    sram_shared_bits = CGM_ITM_SRAM_TOTAL_SHARED_BLOCKED | CGM_ITM_SRAM_PORT_PG_SHARED_BLOCKED;
    reject_bit = CGM_ITM_SRAM_VSQ_GRNTD_BLOCKED | CGM_ITM_SRAM_TOTAL_HEADROOM_BLOCKED | sram_shared_bits;
    SHR_BITSET(sram_rjct_bitmap, reject_bit);
    reject_bit = CGM_ITM_SRAM_VSQ_GRNTD_BLOCKED | CGM_ITM_SRAM_PORT_HEADROOM_BLOCKED | sram_shared_bits;
    SHR_BITSET(sram_rjct_bitmap, reject_bit);
    reject_bit = CGM_ITM_SRAM_VSQ_GRNTD_BLOCKED | CGM_ITM_SRAM_PG_HEADROOM_NOMINAL_BLOCKED | CGM_ITM_SRAM_HEADROOM_EXTENSION_BLOCKED | sram_shared_bits;
    SHR_BITSET(sram_rjct_bitmap, reject_bit);
    reject_bit = CGM_ITM_SRAM_VSQ_GRNTD_BLOCKED | CGM_ITM_SRAM_PORT_HEADROOM_BLOCKED | CGM_ITM_SRAM_PG_HEADROOM_NOMINAL_BLOCKED | CGM_ITM_SRAM_HEADROOM_EXTENSION_BLOCKED | sram_shared_bits;
    SHR_BITSET(sram_rjct_bitmap, reject_bit);
    reject_bit = CGM_ITM_SRAM_VSQ_GRNTD_BLOCKED | CGM_ITM_WORDS_TOTAL_HEADROOM_BLOCKED | CGM_ITM_SRAM_PORT_HEADROOM_BLOCKED | sram_shared_bits;
    SHR_BITSET(sram_rjct_bitmap, reject_bit);
    reject_bit = CGM_ITM_SRAM_VSQ_GRNTD_BLOCKED | CGM_ITM_WORDS_TOTAL_HEADROOM_BLOCKED | CGM_ITM_SRAM_PORT_HEADROOM_BLOCKED | CGM_ITM_SRAM_PG_HEADROOM_NOMINAL_BLOCKED | CGM_ITM_SRAM_HEADROOM_EXTENSION_BLOCKED | sram_shared_bits;
    SHR_BITSET(sram_rjct_bitmap, reject_bit);

    /* 
     * Each index represent PG.Admit-Profile.
     * PG.Admit-Profile == 0 by default, and currently we don't configure
     * other PG.Admit-Profiles, so change only profile 0.
     */
    SOCDNX_IF_ERR_EXIT(WRITE_CGM_PB_VSQ_SRAM_BUFFERS_RJCT_MAPm(unit, MEM_BLOCK_ALL, 0, sram_rjct_bitmap));

    /* Set SRAM-PDs reject bitmap the same as SRAM-Buffers reject bitmap */
    SOCDNX_IF_ERR_EXIT(WRITE_CGM_PB_VSQ_SRAM_PDS_RJCT_MAPm(unit, MEM_BLOCK_ALL, 0, sram_rjct_bitmap));

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int qax_itm_scheduler_compensation_init(
    SOC_SAND_IN  int unit
    )
{
    int index;
    uint32 table_data[10];
    uint32 data;
    uint64 data64;
    int cgm_delta_signs_reg;

    SOCDNX_INIT_FUNC_DEFS;

    /* Scheduler compensation init
     * Only 8 first profiles are used so credit class is mapped 1-1 to credit class profile
     */
    sal_memset(table_data, 0x0, sizeof(table_data));
    for (index = 0; index < 32; index++) { /* enable all types for all profiles, including those we don't plan to use */
        /* when compensation tag exists(new mechanism) we enable all compensations types */
        if (SOC_TMC_ITM_COMPENSATION_LEGACY_MODE(unit)) { 
            /* legacy mode (credit discount profiles) */

            /* in future devices this should be allowed per queue (credit profile) and not globally */
            soc_mem_field32_set(unit, CGM_SCH_HCMm, table_data, MASKf, 0x4); /* enable per queue compensation only -- bit #2 */
        } else {
            
            /* 
               bit 0 - If reset, mask Header-Truncate delta 
               bit 1 - If reset, mask IRPP header delta 
               bit 2 - If reset, mask In-PP-Port header delta 
               bit 3 - If reset, mask Header-Append delta"
            */
            /* read is not necessary - single field */
            soc_mem_field32_set(unit, CGM_SCH_HCMm, table_data, MASKf, 0xf); /* enable all compensation types */
        } 
        SOCDNX_IF_ERR_EXIT(WRITE_CGM_SCH_HCMm(unit, MEM_BLOCK_ALL, index, table_data));
    }

    for (index = 0; index < 8; index++) { /* we are going to use 8 first profiles only to allow 1-1 mapping on the second mapping level */

        /* map VOQ credit class to compensation profile  (1-1 mapping)*/
        
        SOCDNX_IF_ERR_EXIT(READ_CGM_VOQ_HCPm(unit, MEM_BLOCK_ANY , index, table_data));
        soc_mem_field32_set(unit, CGM_VOQ_HCPm, table_data, SCH_PROFILEf, index); 
        SOCDNX_IF_ERR_EXIT(WRITE_CGM_VOQ_HCPm(unit, MEM_BLOCK_ALL, index, table_data));
    }

    /* negate delta - packet size compensation feature */
    if (SOC_IS_QUX(unit)) {
        cgm_delta_signs_reg = CGM_REG_04F2r; /* CGM_DELTA_SIGNS */
    } else {
        cgm_delta_signs_reg = CGM_REG_04F5r; /* CGM_DELTA_SIGNS */
    }

    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, cgm_delta_signs_reg, REG_PORT_ANY, 0, &data)); /* CGM_DELTA_SIGNS */
    soc_reg_field_set(unit, cgm_delta_signs_reg, &data, FIELD_7_7f, TRUE); /* CreditHdrTruncateNeg */
    soc_reg_field_set(unit, cgm_delta_signs_reg, &data, FIELD_8_8f, TRUE); /* CreditIrppDeltaNeg */
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, cgm_delta_signs_reg, REG_PORT_ANY, 0, data));

    /* Set Header-Truncate & Header-Append-Ptr compensation modes to get
     * information about system headers and IRPP editing from PP */
    /* 
       -   Nwk-Header-Truncate(8) = (Cfg-Truncate-Mode == 0) ? 0 :
          (Cfg-Truncate-Mode == 1) ? User-Header-1[23:16] :  
          PPH.FWD_HEADER_OFFSET
 
          -   Nwk-Header-Append(8) =   (Cfg-Append-Mode == 0) ? 0 :
          (Cfg-Append-Mode == 1) ? User-Header-1[15:8] :  {3'b0, Out-LIF[17:13]}
    */
    COMPILER_64_ZERO(data64);
    SOCDNX_IF_ERR_EXIT(READ_IHB_LBP_GENERAL_CONFIG_0r(unit, SOC_CORE_ALL, &data64)); 
    soc_reg64_field32_set(unit, IHB_LBP_GENERAL_CONFIG_0r, &data64, NWK_HDR_TRUNCATE_MODEf, 0x2);
    soc_reg64_field32_set(unit, IHB_LBP_GENERAL_CONFIG_0r, &data64, NWK_HDR_APPEND_MODEf, 0x2);
    SOCDNX_IF_ERR_EXIT(WRITE_IHB_LBP_GENERAL_CONFIG_0r(unit, SOC_CORE_ALL, data64));

exit:
    SOCDNX_FUNC_RETURN;

}


STATIC int qax_itm_crps_compensation_init(
    SOC_SAND_IN  int unit
    )
{
    uint32 data32=0, data = 0, data1, data2;
    int core_index, index;
    int cgm_delta_signs_reg;
    
    SOCDNX_INIT_FUNC_DEFS;
    /* call Soc to configure CGM_IRPP_CTR_HCM_A/B */
    /* For IQM, all compensation activities are active, except Header_truncate - therefore, set the value 0xE */
    /* For IRPP,all compensation activities are active, except Header_truncate, which is determine by config file */
    data1 = (SOC_DPP_CONFIG(unit)->jer->tm.truncate_delta_in_pp_counter[0] == 0) ? 0xE : 0xF;
    data2 = (SOC_DPP_CONFIG(unit)->jer->tm.truncate_delta_in_pp_counter[1] == 0) ? 0xE : 0xF;             
    BCM_DPP_CORES_ITER(BCM_CORE_ALL, core_index)
    {
        for (index = 0; index < 32; index++) {
            /* Counter A*/
            SOCDNX_IF_ERR_EXIT(READ_CGM_TM_CTR_HCMm(unit, CGM_BLOCK(unit, core_index),index, &data32));
            soc_mem_field32_set(unit, CGM_TM_CTR_HCMm, &data32, MASKf, 0xE);
            SOCDNX_IF_ERR_EXIT(WRITE_CGM_TM_CTR_HCMm(unit, CGM_BLOCK(unit, core_index),index, &data32));

            /* Counter B*/
            SOCDNX_IF_ERR_EXIT(READ_CGM_TM_CTR_HCMm(unit, CGM_BLOCK(unit, core_index),index+32, &data32));
            soc_mem_field32_set(unit, CGM_TM_CTR_HCMm, &data32, MASKf, 0xE);
            SOCDNX_IF_ERR_EXIT(WRITE_CGM_TM_CTR_HCMm(unit, CGM_BLOCK(unit, core_index),index+32, &data32));

            /* Counter A*/
            SOCDNX_IF_ERR_EXIT(READ_CGM_IRPP_CTR_HCMm(unit, CGM_BLOCK(unit, core_index),index, &data));
            soc_mem_field32_set(unit, CGM_IRPP_CTR_HCMm, &data, MASKf, data1);
            SOCDNX_IF_ERR_EXIT(WRITE_CGM_IRPP_CTR_HCMm(unit, CGM_BLOCK(unit, core_index), index, &data));

            /* Counter B*/
            SOCDNX_IF_ERR_EXIT(READ_CGM_IRPP_CTR_HCMm(unit, CGM_BLOCK(unit, core_index), index+32, &data));
            soc_mem_field32_set(unit, CGM_IRPP_CTR_HCMm, &data, MASKf, data2);
            SOCDNX_IF_ERR_EXIT(WRITE_CGM_IRPP_CTR_HCMm(unit, CGM_BLOCK(unit, core_index), index+32, &data));
        }
    }

    /* negate delta - packet size compensation feature */
    if (SOC_IS_QUX(unit)) {
        cgm_delta_signs_reg = CGM_REG_04F2r; /* CGM_DELTA_SIGNS */
    } else {
        cgm_delta_signs_reg = CGM_REG_04F5r; /* CGM_DELTA_SIGNS */
    }
    data = 0;
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, cgm_delta_signs_reg, REG_PORT_ANY, 0, &data)); /* CGM_DELTA_SIGNS */
    soc_reg_field_set(unit, cgm_delta_signs_reg, &data, FIELD_3_3f, TRUE); /* CrpsHdrTruncateNeg */
    soc_reg_field_set(unit, cgm_delta_signs_reg, &data, FIELD_4_4f, TRUE); /* CrpsIrppDeltaNeg */
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, cgm_delta_signs_reg, REG_PORT_ANY, 0, data));

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int qax_itm_stif_compensation_init(
    SOC_SAND_IN  int unit
    )
{
    uint32 data32;
    int index;
    int cgm_delta_signs_reg;
    SOCDNX_INIT_FUNC_DEFS;
    /*Statistic interface compensation default configuration*/
    /* 
      bit 0 - masks Header-Truncate delta 
      bit 1 - masks IRPP header delta 
      bit 2 - masks VOQ x In-PP-Port header delta 
      bit 3 - masks Header-Append delta"
    */
    for (index = 0; index < 32; index++) {
            SOCDNX_IF_ERR_EXIT(READ_CGM_STAT_HCMm(unit, CGM_BLOCK(unit, SOC_CORE_ALL),index, &data32));
            soc_mem_field32_set(unit, CGM_STAT_HCMm, &data32, MASKf, 0xF);
            SOCDNX_IF_ERR_EXIT(WRITE_CGM_STAT_HCMm(unit, CGM_BLOCK(unit, SOC_CORE_ALL),index, &data32));
    }

    /* negate delta - packet size compensation feature */
    if (SOC_IS_QUX(unit)) {
        cgm_delta_signs_reg = CGM_REG_04F2r; /* CGM_DELTA_SIGNS */
    } else {
        cgm_delta_signs_reg = CGM_REG_04F5r; /* CGM_DELTA_SIGNS */
    }
    data32 = 0;
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, cgm_delta_signs_reg, REG_PORT_ANY, 0, &data32)); /* CGM_DELTA_SIGNS */
    soc_reg_field_set(unit, cgm_delta_signs_reg, &data32, FIELD_5_5f, TRUE); /* StatHdrTruncateNeg */
    soc_reg_field_set(unit, cgm_delta_signs_reg, &data32, FIELD_6_6f, TRUE); /* StatIrppDeltaNeg */
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, cgm_delta_signs_reg, REG_PORT_ANY, 0, data32));

exit:
    SOCDNX_FUNC_RETURN;
}


int
  qax_itm_init(
    SOC_SAND_IN  int  unit
  )
{
    uint32                 data = 0;
    soc_reg_above_64_val_t above64;
    uint64                 val64;
    int                    res;
    int                    core_index;
    SOC_TMC_ITM_GUARANTEED_RESOURCE guaranteed_resources; 
    SOC_TMC_ITM_INGRESS_CONGESTION_MGMT ingress_congestion_mgmt;
    SOC_TMC_ITM_INGRESS_CONGESTION_RESOURCE *resource = NULL;
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type;

    SOCDNX_INIT_FUNC_DEFS;

    data = 0x0;
    if (!SOC_IS_QUX(unit)) {
    soc_mem_field32_set(unit, IPS_CRVSm, &data, CR_VAL_BMPf, 0x0); 
    res = arad_fill_table_with_entry(unit, IPS_CRVSm, MEM_BLOCK_ANY, &data);
    SOCDNX_IF_ERR_EXIT(res);

    data = 0;
    res = READ_IPS_CREDIT_CONFIGr(unit, &data); 
    SOCDNX_IF_ERR_EXIT(res);
    soc_reg_field_set(unit, IPS_CREDIT_CONFIGr, &data, CR_VAL_SEL_ENABLEf, TRUE);
    res = WRITE_IPS_CREDIT_CONFIGr(unit, data); 
    SOCDNX_IF_ERR_EXIT(res);
    }

    SOC_REG_ABOVE_64_CLEAR(above64); /* set IPS_STORED_CREDITS_USAGE_CONFIGURATIONr */
    soc_reg_above_64_field32_set(unit, IPS_STORED_CREDITS_USAGE_CONFIGURATIONr, above64, MUL_PKT_DEQf, 0xf0f0);
    soc_reg_above_64_field32_set(unit, IPS_STORED_CREDITS_USAGE_CONFIGURATIONr, above64, MUL_PKT_DEQ_BYTESf, 0x10);
    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, IPS_STORED_CREDITS_USAGE_CONFIGURATIONr, REG_PORT_ANY, 0, above64));

    /* init CGM counters */
    COMPILER_64_SET(val64, 0xFFFFF, 0xFFFFFFFF);
    SOCDNX_IF_ERR_EXIT(WRITE_CGM_PRG_CTR_SETTINGSr(unit, val64));

    /* Enable stamping Fabric Header */
    SOCDNX_IF_ERR_EXIT(READ_ITE_STAMPING_FABRIC_HEADER_ENABLEr(unit, &val64));
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, STAMP_FAP_PORTf, 1);
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, STAMP_FWD_ACTIONf, 1);
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, STAMP_LB_KEY_EXT_ENf, 1);
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, STAMP_FWDACTION_TDMf, 1);
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, STAMP_DP_ENf, 0xff);
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, SNP_STAMP_TRAP_CODEf, 1);
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, STAMP_CNI_BITf, 1);
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, STAMP_USER_DEFINED_LSBf, (64-9)*8+4); /* (64 - FTMH)*8 + OutLIF offset */
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, BACKWARD_TC_ENf, 0x0);
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, BACKWARD_DP_ENf, 0x0);
    if (SOC_DPP_CONFIG(unit)->arad->init.mirror_stamp_sys_dsp_ext) 
    {
        /* Enable DSP-Ext stamping for mirrored/snooped packets */
        soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, STAMP_DSP_EXT_ENf, 0);
    }
    SOCDNX_IF_ERR_EXIT(WRITE_ITE_STAMPING_FABRIC_HEADER_ENABLEr(unit, val64));

    /* 
     * Read CGM minimum occupancy registers (watermark) in order for 
     * future reads to be correct (the registers set to maximum on read, 
     * but first reads are always 0).
     */
    SOCDNX_IF_ERR_EXIT(READ_CGM_SRAM_BUFFERS_FREE_MIN_STATUSr(unit, &data));
    SOCDNX_IF_ERR_EXIT(READ_CGM_SRAM_PDBS_FREE_MIN_STATUSr(unit, &data));
    SOCDNX_IF_ERR_EXIT(READ_CGM_DRAM_BDBS_FREE_MIN_STATUSr(unit, &data));

    /*
     * FIELD_0_0 should be 0 in both registers (ASIC feedback)
     */
    SOCDNX_IF_ERR_EXIT(READ_IPS_AVOID_DRAM_CAM_FULLr(unit, &data));
    soc_reg_field_set(unit, IPS_AVOID_DRAM_CAM_FULLr, &data, FIELD_0_0f, 0);
    SOCDNX_IF_ERR_EXIT(WRITE_IPS_AVOID_DRAM_CAM_FULLr(unit, data));

    SOCDNX_IF_ERR_EXIT(READ_IPS_AVOID_FABRIC_CAM_FULLr(unit, &data));
    soc_reg_field_set(unit, IPS_AVOID_FABRIC_CAM_FULLr, &data, FIELD_0_0f, 0);
    SOCDNX_IF_ERR_EXIT(WRITE_IPS_AVOID_FABRIC_CAM_FULLr(unit, data));

    /* set the total of the guarenteed VOQ resource, by default the dynamic shared space should be equal to the total memory */
    BCM_DPP_CORES_ITER(BCM_CORE_ALL, core_index) {
        res = sw_state_access[unit].dpp.soc.qax.tm.guaranteed_info.get(unit, core_index, &guaranteed_resources);
        SOCDNX_IF_ERR_EXIT(res);

        /* Words */
        guaranteed_resources.guaranteed[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].total = CGM_ITM_WORDS_SIZE_MAX(unit, core_index);

        /* SRAM words */
        guaranteed_resources.guaranteed[SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES].total = SOC_DPP_DEFS_GET(unit, ocb_memory_size) 
            * (1024 * 1024 / 8 /* MBits -> Bytes */
               / 16);          /* word size */

        /* SRAM PDs*/
        guaranteed_resources.guaranteed[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].total = SOC_DPP_DEFS_GET(unit, pdm_size);

        /* update warm boot data */
        res = sw_state_access[unit].dpp.soc.qax.tm.guaranteed_info.set(unit, core_index, &guaranteed_resources);
        SOCDNX_IF_ERR_EXIT(res);
    }

    /* PG-TC configuration */
    SOCDNX_IF_ERR_EXIT(READ_CGM_VSQ_SETTINGSr(unit, &data));
    /* Take TC from TM-CMD.TC */
    soc_reg_field_set(unit, CGM_VSQ_SETTINGSr, &data, PB_VSQ_MC_COUNT_ONCEf, 1);
    /* Set PG-TC width to 3b */
    soc_reg_field_set(unit, CGM_VSQ_SETTINGSr, &data, PB_VSQ_PG_TC_WIDTHf, 0x3);
    SOCDNX_IF_ERR_EXIT(WRITE_CGM_VSQ_SETTINGSr(unit, data));

    /* create sw_state var for reserved resource */
    res = soc_qax_itm_reserved_resource_init(unit);
    SOCDNX_IF_ERR_EXIT(res);

    /* set source based VSQs (VSQ-E/F) resources */
    SOC_DPP_CORES_ITER(SOC_CORE_ALL, core_index) {
        res = sw_state_access[unit].dpp.soc.qax.tm.lossless_pool_id.set(unit, core_index, 0);
        SOCDNX_IF_ERR_EXIT(res);

        ingress_congestion_mgmt.global[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].total = CGM_ITM_WORDS_SIZE_MAX(unit, core_index);
        ingress_congestion_mgmt.global[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].total = CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX(unit);
        ingress_congestion_mgmt.global[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].total = CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX;

        /* Set defaults */
        for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
            resource = &ingress_congestion_mgmt.global[rsrc_type];
            resource->pool_0 = resource->total; /* Set pool 0 to take all resources */
            resource->pool_1 = 0;
            resource->headroom = 0;
            resource->nominal_headroom = 0;
            resource->reserved = 0;
        }

        res = qax_itm_global_resource_allocation_set(unit, core_index, &ingress_congestion_mgmt);
        SOCDNX_IF_ERR_EXIT(res);
    }
	res = jer_itm_setup_dp_map(unit);
    SOCDNX_IF_ERR_EXIT(res);  

    /* Init default global reject thresholds */
    SOCDNX_IF_ERR_EXIT(qax_itm_drop_global_thresholds_init(unit));
	
    /* Init default global flow control reject thresholds */
    SOCDNX_IF_ERR_EXIT(qax_itm_fc_global_thresholds_init(unit));

    /* Configure Admission tests defaults */
    SOCDNX_IF_ERR_EXIT(qax_itm_rjct_setting_defaults_set(unit));
    SOCDNX_IF_ERR_EXIT(qax_itm_rjct_map_defaults_set(unit));
    SOCDNX_IF_ERR_EXIT(qax_itm_admission_tests_defaults_set(unit));

    /* Init Ingress Compensation */
    SOCDNX_IF_ERR_EXIT(qax_itm_scheduler_compensation_init(unit));
    SOCDNX_IF_ERR_EXIT(qax_itm_crps_compensation_init(unit));
    SOCDNX_IF_ERR_EXIT(qax_itm_stif_compensation_init(unit));

    /* Init TAR fifos thresholds */
    SOCDNX_IF_ERR_EXIT(READ_CGM_TAR_FIFO_RJCT_THm(unit, MEM_BLOCK_ANY, 0, &data));
    soc_mem_field32_set(unit, CGM_TAR_FIFO_RJCT_THm, &data, MIRROR_RJCT_THf, 0x3ff);
    soc_mem_field32_set(unit, CGM_TAR_FIFO_RJCT_THm, &data, SNOOP_RJCT_THf, 0x3ff);
    soc_mem_field32_set(unit, CGM_TAR_FIFO_RJCT_THm, &data, FWD_RJCT_THf, 0x3ff);
    SOCDNX_IF_ERR_EXIT(arad_fill_table_with_entry(unit, CGM_TAR_FIFO_RJCT_THm, MEM_BLOCK_ALL, &data));

exit:
    SOCDNX_FUNC_RETURN;
}

/* init default flow control thresholds */
STATIC
int qax_itm_fc_global_thresholds_init(int unit)
{
    SOC_TMC_ITM_GLOB_RCS_FC_TH glbl_fc, glbl_fc_exact;
    int res = SOC_E_NONE;
    soc_reg_above_64_val_t above64;
    ARAD_MGMT_INIT *init;
    int index = 0, priority = 0;
    uint32 threshold = 0, dummy = 0;
    itm_mantissa_exp_threshold_info fc_info;

    SOCDNX_INIT_FUNC_DEFS;

    /* clear struct */
    SOC_TMC_ITM_GLOB_RCS_FC_TH_clear(&glbl_fc);
    SOC_TMC_ITM_GLOB_RCS_FC_TH_clear(&glbl_fc_exact);

    init = &(SOC_DPP_CONFIG(unit)->arad->init);

    /* If we are in ocb_only mode, we need to set all ITM FC registers to 0*/
    if (init->dram.nof_drams != 0) {
        /* Flow-Control based on Free BDB thresholds - HP and LP. */
        glbl_fc.bdbs.hp.set       = 192;
        glbl_fc.bdbs.hp.clear     = 202;
        glbl_fc.bdbs.lp.set       = 256;
        glbl_fc.bdbs.lp.clear     = 266;
    }

    /* Flow-Control based on Free OCB thresholds - HP and LP. */
    glbl_fc.ocb.hp.set = 220;
    glbl_fc.ocb.hp.clear = 270;
    glbl_fc.ocb.lp.set = 270;
    glbl_fc.ocb.lp.clear = 320;

    glbl_fc.ocb_pdb.hp.set = 220;
    glbl_fc.ocb_pdb.hp.clear = 270;
    glbl_fc.ocb_pdb.lp.set = 270;
    glbl_fc.ocb_pdb.lp.clear = 320;

    /* Flow-Control based on Free Headroom thresholds. */
    glbl_fc.hdrm.hp.set = 220;
    glbl_fc.hdrm.hp.clear = 270;

    glbl_fc.hdrm_pd.hp.set = 220;
    glbl_fc.hdrm_pd.hp.clear = 270;

    res = arad_itm_glob_rcs_fc_set_unsafe(unit, &glbl_fc, &glbl_fc_exact);
    SOCDNX_SAND_IF_ERR_EXIT(res);
    
    /* 
     * set global source based VSQs flow control thresholds to maximum (disable), this way losless traffic will use shared space.
     * set pool0 and pool1 for each priority (LP, HP).
     */
    for (priority = 0; priority < CGM_ITM_PB_VSQ_POOL_FC_NOF_PRIO; ++priority) {

        fc_info.mem_id = CGM_PB_VSQ_POOL_FC_THm;
        fc_info.mantissa_bits = CGM_ITM_NOF_MANTISSA_BITS;
        fc_info.factor = 1;

        /* 
         * Config pool 0 
         */
        index = 2 * priority;

        SOCDNX_IF_ERR_EXIT(READ_CGM_PB_VSQ_POOL_FC_THm(unit, MEM_BLOCK_ALL, index, above64));

        /* Words FC thresholds */
        threshold = SOC_DPP_DEFS_GET(unit, itm_glob_rcs_fc_p0_byte_size_max);

        fc_info.field_id = WORDS_SET_THf;
        fc_info.exp_bits = soc_mem_field_length(unit, fc_info.mem_id, fc_info.field_id) - fc_info.mantissa_bits;
        SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &fc_info, 1, above64, threshold, &dummy));

        fc_info.field_id = WORDS_CLR_THf;
        fc_info.exp_bits = soc_mem_field_length(unit, fc_info.mem_id, fc_info.field_id) - fc_info.mantissa_bits;
        SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &fc_info, 1, above64, threshold, &dummy));

        /* SRAM-Buffers FC thresholds */
        threshold = SOC_DPP_DEFS_GET(unit, itm_glob_rcs_fc_p0_size_max);

        fc_info.field_id = SRAM_BUFFERS_SET_THf;
        fc_info.exp_bits = soc_mem_field_length(unit, fc_info.mem_id, fc_info.field_id) - fc_info.mantissa_bits;
        SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &fc_info, 1, above64, threshold, &dummy));

        fc_info.field_id = SRAM_BUFFERS_CLR_THf;
        fc_info.exp_bits = soc_mem_field_length(unit, fc_info.mem_id, fc_info.field_id) - fc_info.mantissa_bits;
        SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &fc_info, 1, above64, threshold, &dummy));

        /* SRAM-PDs FC thresholds */
        threshold = SOC_DPP_DEFS_GET(unit, itm_glob_rcs_fc_p0_pd_size_max);

        fc_info.field_id = SRAM_PDS_SET_THf;
        fc_info.exp_bits = soc_mem_field_length(unit, fc_info.mem_id, fc_info.field_id) - fc_info.mantissa_bits;
        SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &fc_info, 1, above64, threshold, &dummy));

        fc_info.field_id = SRAM_PDS_CLR_THf;
        fc_info.exp_bits = soc_mem_field_length(unit, fc_info.mem_id, fc_info.field_id) - fc_info.mantissa_bits;
        SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &fc_info, 1, above64, threshold, &dummy));

        SOCDNX_IF_ERR_EXIT(WRITE_CGM_PB_VSQ_POOL_FC_THm(unit, MEM_BLOCK_ALL, index, above64));


        /* 
         * Config pool 1 
         */
        index = 2 * priority + 1;

        SOCDNX_IF_ERR_EXIT(READ_CGM_PB_VSQ_POOL_FC_THm(unit, MEM_BLOCK_ALL, index, above64));

        /* Words FC thresholds */
        threshold = SOC_DPP_DEFS_GET(unit, itm_glob_rcs_fc_p1_byte_size_max);

        fc_info.field_id = WORDS_SET_THf;
        fc_info.exp_bits = soc_mem_field_length(unit, fc_info.mem_id, fc_info.field_id) - fc_info.mantissa_bits;
        SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &fc_info, 1, above64, threshold, &dummy));

        fc_info.field_id = WORDS_CLR_THf;
        fc_info.exp_bits = soc_mem_field_length(unit, fc_info.mem_id, fc_info.field_id) - fc_info.mantissa_bits;
        SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &fc_info, 1, above64, threshold, &dummy));

        /* SRAM-Buffers FC thresholds */
        threshold = SOC_DPP_DEFS_GET(unit, itm_glob_rcs_fc_p1_size_max);

        fc_info.field_id = SRAM_BUFFERS_SET_THf;
        fc_info.exp_bits = soc_mem_field_length(unit, fc_info.mem_id, fc_info.field_id) - fc_info.mantissa_bits;
        SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &fc_info, 1, above64, threshold, &dummy));

        fc_info.field_id = SRAM_BUFFERS_CLR_THf;
        fc_info.exp_bits = soc_mem_field_length(unit, fc_info.mem_id, fc_info.field_id) - fc_info.mantissa_bits;
        SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &fc_info, 1, above64, threshold, &dummy));

        /* SRAM-PDs FC thresholds */
        threshold = SOC_DPP_DEFS_GET(unit, itm_glob_rcs_fc_p1_pd_size_max);

        fc_info.field_id = SRAM_PDS_SET_THf;
        fc_info.exp_bits = soc_mem_field_length(unit, fc_info.mem_id, fc_info.field_id) - fc_info.mantissa_bits;
        SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &fc_info, 1, above64, threshold, &dummy));

        fc_info.field_id = SRAM_PDS_CLR_THf;
        fc_info.exp_bits = soc_mem_field_length(unit, fc_info.mem_id, fc_info.field_id) - fc_info.mantissa_bits;
        SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &fc_info, 1, above64, threshold, &dummy));

        SOCDNX_IF_ERR_EXIT(WRITE_CGM_PB_VSQ_POOL_FC_THm(unit, MEM_BLOCK_ALL, index, above64));
    }

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Set Global reject thresholds.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit - 
 *      Unit id.
 * \remark
 *  Need to get precise thresholds from AEs. \n
 *  Currently thresholds are set to max.
 */
STATIC
int qax_itm_drop_global_thresholds_init(int unit)
{
    SOC_TMC_ITM_GLOB_RCS_DROP_TH glbl_drop, glbl_drop_exact;
    int core_index = 0;
    /*uint32 voq_threshold = 0, vsq_threshold = 0;*/
    int rv = SOC_E_NONE;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_DPP_CORES_ITER(SOC_DPP_CORE_ALL(unit), core_index) {

        /* clear structs */
        SOC_TMC_ITM_GLOB_RCS_DROP_TH_clear(&glbl_drop);
        SOC_TMC_ITM_GLOB_RCS_DROP_TH_clear(&glbl_drop_exact);

        /* Reject based on Global Free DRAM-BDBs thresholds - per drop-precedence. */
        /* values were approximately taken from Jericho. */
        /* TBD: Get accurate values from AEs */
        glbl_drop.global_free_dram[0].set   = 128; /* dp 0 */
        glbl_drop.global_free_dram[0].clear = 130;
        glbl_drop.global_free_dram[1].set   = 156; /* dp 1 */
        glbl_drop.global_free_dram[1].clear = 158;
        glbl_drop.global_free_dram[2].set   = 192; /* dp 2 */
        glbl_drop.global_free_dram[2].clear = 194;
        glbl_drop.global_free_dram[3].set   = 232; /* dp 3 */
        glbl_drop.global_free_dram[3].clear = 234;

        /* Reject base on Free SRAM-PDBs - per drop-precedence. */
        /* For normal VOQs */
        glbl_drop.global_free_sram[0][0].set   = 70; /* dp 0 */
        glbl_drop.global_free_sram[0][0].clear = 120;
        glbl_drop.global_free_sram[0][1].set   = 130; /* dp 1 */
        glbl_drop.global_free_sram[0][1].clear = 180;
        glbl_drop.global_free_sram[0][2].set   = 190; /* dp 2 */
        glbl_drop.global_free_sram[0][2].clear = 240;
        glbl_drop.global_free_sram[0][3].set   = 250; /* dp 3 */
        glbl_drop.global_free_sram[0][3].clear = 300;
        /* For OCB-Only VOQs */
        glbl_drop.global_free_sram_only[0][0].set   = 70; /* dp 0 */
        glbl_drop.global_free_sram_only[0][0].clear = 120;
        glbl_drop.global_free_sram_only[0][1].set   = 130; /* dp 1 */
        glbl_drop.global_free_sram_only[0][1].clear = 180;
        glbl_drop.global_free_sram_only[0][2].set   = 190; /* dp 2 */
        glbl_drop.global_free_sram_only[0][2].clear = 240;
        glbl_drop.global_free_sram_only[0][3].set   = 250; /* dp 3 */
        glbl_drop.global_free_sram_only[0][3].clear = 300;

        /* Reject base on Free SRAM-Buffers - per drop-precedence. */
        /* For normal VOQs */
        glbl_drop.global_free_sram[1][0].set   = 70; /* dp 0 */
        glbl_drop.global_free_sram[1][0].clear = 120;
        glbl_drop.global_free_sram[1][1].set   = 130; /* dp 1 */
        glbl_drop.global_free_sram[1][1].clear = 180;
        glbl_drop.global_free_sram[1][2].set   = 190; /* dp 2 */
        glbl_drop.global_free_sram[1][2].clear = 240;
        glbl_drop.global_free_sram[1][3].set   = 250; /* dp 3 */
        glbl_drop.global_free_sram[1][3].clear = 300;
        /* For OCB-Only VOQs */
        glbl_drop.global_free_sram_only[1][0].set   = 70; /* dp 0 */
        glbl_drop.global_free_sram_only[1][0].clear = 120;
        glbl_drop.global_free_sram_only[1][1].set   = 130; /* dp 1 */
        glbl_drop.global_free_sram_only[1][1].clear = 180;
        glbl_drop.global_free_sram_only[1][2].set   = 190; /* dp 2 */
        glbl_drop.global_free_sram_only[1][2].clear = 240;
        glbl_drop.global_free_sram_only[1][3].set   = 250; /* dp 3 */
        glbl_drop.global_free_sram_only[1][3].clear = 300;

        rv = qax_itm_glob_rcs_drop_set(unit, core_index, &glbl_drop, &glbl_drop_exact);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit: 
    SOCDNX_FUNC_RETURN; 
}

int
  qax_itm_per_queue_info_set(
    SOC_SAND_IN   int             unit,
    SOC_SAND_IN   int             core,
    SOC_SAND_IN   uint32          queue,
    SOC_SAND_IN   ARAD_IQM_STATIC_TBL_DATA* IQM_static_tbl_data
  )
{
    soc_reg_above_64_val_t data_above_64, data2_above_64;
    uint32 offset;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data_above_64);
    SOC_REG_ABOVE_64_CLEAR(data2_above_64);

    offset = queue / 4; /* each entry handles 4 queues */
    /* read */
    SOCDNX_IF_ERR_EXIT(READ_CGM_VOQ_PROFILESm(unit, CGM_BLOCK(unit, core), offset, &data_above_64));
    SOCDNX_IF_ERR_EXIT(READ_CGM_VOQ_VSQS_PRMSm(unit, CGM_BLOCK(unit, core), offset, &data2_above_64));

    switch (queue % 4) {
        /* modify */
        case 0:
            soc_CGM_VOQ_PROFILESm_field32_set(unit, &data_above_64, RATE_CLASSf, IQM_static_tbl_data->rate_class);
            soc_CGM_VOQ_PROFILESm_field32_set(unit, &data_above_64, CREDIT_CLASSf, IQM_static_tbl_data->credit_class);
            soc_CGM_VOQ_VSQS_PRMSm_field32_set(unit, &data2_above_64, TRAFFIC_CLASSf, IQM_static_tbl_data->traffic_class);
            soc_CGM_VOQ_VSQS_PRMSm_field32_set(unit, &data2_above_64, CONNECTION_CLASSf, IQM_static_tbl_data->connection_class);
            break;
        case 1:
            soc_CGM_VOQ_PROFILESm_field32_set(unit, &data_above_64, RATE_CLASS_1f, IQM_static_tbl_data->rate_class);
            soc_CGM_VOQ_PROFILESm_field32_set(unit, &data_above_64, CREDIT_CLASS_1f, IQM_static_tbl_data->credit_class);
            soc_CGM_VOQ_VSQS_PRMSm_field32_set(unit, &data2_above_64, TRAFFIC_CLASS_1f, IQM_static_tbl_data->traffic_class);
            soc_CGM_VOQ_VSQS_PRMSm_field32_set(unit, &data2_above_64, CONNECTION_CLASS_1f, IQM_static_tbl_data->connection_class);
            break;
        case 2:
            soc_CGM_VOQ_PROFILESm_field32_set(unit, &data_above_64, RATE_CLASS_2f, IQM_static_tbl_data->rate_class);
            soc_CGM_VOQ_PROFILESm_field32_set(unit, &data_above_64, CREDIT_CLASS_2f, IQM_static_tbl_data->credit_class);
            soc_CGM_VOQ_VSQS_PRMSm_field32_set(unit, &data2_above_64, TRAFFIC_CLASS_2f, IQM_static_tbl_data->traffic_class);
            soc_CGM_VOQ_VSQS_PRMSm_field32_set(unit, &data2_above_64, CONNECTION_CLASS_2f, IQM_static_tbl_data->connection_class);
            break;
        case 3:
            soc_CGM_VOQ_PROFILESm_field32_set(unit, &data_above_64, RATE_CLASS_3f, IQM_static_tbl_data->rate_class);
            soc_CGM_VOQ_PROFILESm_field32_set(unit, &data_above_64, CREDIT_CLASS_3f, IQM_static_tbl_data->credit_class);
            soc_CGM_VOQ_VSQS_PRMSm_field32_set(unit, &data2_above_64, TRAFFIC_CLASS_3f, IQM_static_tbl_data->traffic_class);
            soc_CGM_VOQ_VSQS_PRMSm_field32_set(unit, &data2_above_64, CONNECTION_CLASS_3f, IQM_static_tbl_data->connection_class);
            break;
    }
    /* write */
    SOCDNX_IF_ERR_EXIT(WRITE_CGM_VOQ_PROFILESm(unit, CGM_BLOCK(unit, core), offset, &data_above_64));
    SOCDNX_IF_ERR_EXIT(WRITE_CGM_VOQ_VSQS_PRMSm(unit, CGM_BLOCK(unit, core), offset, &data2_above_64));

exit:
    SOCDNX_FUNC_RETURN;
}

int
  qax_itm_per_queue_info_get(
    SOC_SAND_IN   int             unit,
    SOC_SAND_IN   int             core,
    SOC_SAND_IN   uint32          queue,
    SOC_SAND_OUT   ARAD_IQM_STATIC_TBL_DATA* IQM_static_tbl_data
  )
{
    soc_reg_above_64_val_t data_above_64, data2_above_64;
    uint32 offset;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data_above_64);
    SOC_REG_ABOVE_64_CLEAR(data2_above_64);
    IQM_static_tbl_data->que_signature = 0; /* no signature in QAX */

    offset = queue / 4; /* each entry handles 4 queues */
    /* read */
    SOCDNX_IF_ERR_EXIT(READ_CGM_VOQ_PROFILESm(unit, CGM_BLOCK(unit, core), offset, &data_above_64));
    SOCDNX_IF_ERR_EXIT(READ_CGM_VOQ_VSQS_PRMSm(unit, CGM_BLOCK(unit, core), offset, &data2_above_64));

    switch (queue % 4) {
        /* modify */
        case 0:
            IQM_static_tbl_data->rate_class = soc_CGM_VOQ_PROFILESm_field32_get(unit, &data_above_64, RATE_CLASSf);
            IQM_static_tbl_data->credit_class = soc_CGM_VOQ_PROFILESm_field32_get(unit, &data_above_64, CREDIT_CLASSf);
            IQM_static_tbl_data->traffic_class = soc_CGM_VOQ_VSQS_PRMSm_field32_get(unit, &data2_above_64, TRAFFIC_CLASSf);
            IQM_static_tbl_data->connection_class = soc_CGM_VOQ_VSQS_PRMSm_field32_get(unit, &data2_above_64, CONNECTION_CLASSf);
            break;
        case 1:
            IQM_static_tbl_data->rate_class = soc_CGM_VOQ_PROFILESm_field32_get(unit, &data_above_64, RATE_CLASS_1f);
            IQM_static_tbl_data->credit_class = soc_CGM_VOQ_PROFILESm_field32_get(unit, &data_above_64, CREDIT_CLASS_1f);
            IQM_static_tbl_data->traffic_class = soc_CGM_VOQ_VSQS_PRMSm_field32_get(unit, &data2_above_64, TRAFFIC_CLASS_1f);
            IQM_static_tbl_data->connection_class = soc_CGM_VOQ_VSQS_PRMSm_field32_get(unit, &data2_above_64, CONNECTION_CLASS_1f);
            break;
        case 2:
            IQM_static_tbl_data->rate_class = soc_CGM_VOQ_PROFILESm_field32_get(unit, &data_above_64, RATE_CLASS_2f);
            IQM_static_tbl_data->credit_class = soc_CGM_VOQ_PROFILESm_field32_get(unit, &data_above_64, CREDIT_CLASS_2f);
            IQM_static_tbl_data->traffic_class  = soc_CGM_VOQ_VSQS_PRMSm_field32_get(unit, &data2_above_64, TRAFFIC_CLASS_2f);
            IQM_static_tbl_data->connection_class = soc_CGM_VOQ_VSQS_PRMSm_field32_get(unit, &data2_above_64, CONNECTION_CLASS_2f);
            break;
        case 3:
            IQM_static_tbl_data->rate_class = soc_CGM_VOQ_PROFILESm_field32_get(unit, &data_above_64, RATE_CLASS_3f);
            IQM_static_tbl_data->credit_class = soc_CGM_VOQ_PROFILESm_field32_get(unit, &data_above_64, CREDIT_CLASS_3f);
            IQM_static_tbl_data->traffic_class = soc_CGM_VOQ_VSQS_PRMSm_field32_get(unit, &data2_above_64, TRAFFIC_CLASS_3f);
            IQM_static_tbl_data->connection_class = soc_CGM_VOQ_VSQS_PRMSm_field32_get(unit, &data2_above_64, CONNECTION_CLASS_3f);
            break;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
  qax_itm_in_pp_port_scheduler_compensation_profile_set(
    SOC_SAND_IN   int             unit,
    SOC_SAND_IN   int             core,
    SOC_SAND_IN   uint32          in_pp_port,
    SOC_SAND_IN   int  scheduler_profile
  )
{
    uint32 data;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(READ_CGM_IPP_HCPm(unit, CGM_BLOCK(unit, core), in_pp_port, &data));
    soc_CGM_IPP_HCPm_field32_set(unit, &data, SCH_PROFILEf,scheduler_profile);
    SOCDNX_IF_ERR_EXIT(WRITE_CGM_IPP_HCPm(unit, CGM_BLOCK(unit, core), in_pp_port, &data));

exit:
    SOCDNX_FUNC_RETURN;
}

int
  qax_itm_in_pp_port_scheduler_compensation_profile_get(
    SOC_SAND_IN   int             unit,
    SOC_SAND_IN   int             core,
    SOC_SAND_IN   uint32          in_pp_port,
    SOC_SAND_OUT  int*  scheduler_profile
  )
{
    uint32 data;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(READ_CGM_IPP_HCPm(unit, CGM_BLOCK(unit, core), in_pp_port, &data));
    *scheduler_profile = soc_CGM_IPP_HCPm_field32_get(unit, &data, SCH_PROFILEf);

exit:
    SOCDNX_FUNC_RETURN;
}

int
  qax_itm_profile_ocb_only_set(
    SOC_SAND_IN   int unit,
    SOC_SAND_IN   int rate_class,
    SOC_SAND_IN   int is_ocb_only
  )
{
    uint32 data;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(READ_CGM_VOQ_SRAM_DRAM_ONLY_MODEm(unit, MEM_BLOCK_ALL, rate_class, &data));
    /* 0 - regular mode (mix), 1 - ocb only mode */
    soc_mem_field32_set(unit, CGM_VOQ_SRAM_DRAM_ONLY_MODEm, &data, SRAM_DRAM_ONLY_MODEf, is_ocb_only);
     
    SOCDNX_IF_ERR_EXIT(WRITE_CGM_VOQ_SRAM_DRAM_ONLY_MODEm(unit, MEM_BLOCK_ALL, rate_class, &data));         
        
exit:
    SOCDNX_FUNC_RETURN;
}

int
  qax_itm_profile_ocb_only_get(
    SOC_SAND_IN   int unit,
    SOC_SAND_IN   int rate_class,
    SOC_SAND_OUT  int *is_ocb_only
  )
{
    uint32 data, field;


    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(READ_CGM_VOQ_SRAM_DRAM_ONLY_MODEm(unit, MEM_BLOCK_ALL, rate_class, &data));
    /* 0 - regular mode (mix), 1 - ocb only mode */
    field = soc_mem_field32_get(unit, CGM_VOQ_SRAM_DRAM_ONLY_MODEm, &data, SRAM_DRAM_ONLY_MODEf);
    
    *is_ocb_only = (int)field;        
        
exit:
    SOCDNX_FUNC_RETURN;
}

/*
Get QAX ingress congestion statistics.
*/
int qax_itm_congestion_statistics_get(
  SOC_SAND_IN int unit,
  SOC_SAND_IN int core,
  SOC_SAND_OUT ARAD_ITM_CGM_CONGENSTION_STATS *stats /* place current statistics output here */
  )
{
    uint32 val;

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_NULL_CHECK(stats);

    if (!SOC_UNIT_NUM_VALID(unit)) {
        SOCDNX_IF_ERR_EXIT(SOC_E_UNIT);
    } 
    if (((core < 0) || (core > SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores)) && (core != SOC_CORE_ALL)) { 
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM); 
    }

    /* collect current value statistics */  

    /* Instantaneous SRAM-Buffers free counter */
    SOCDNX_IF_ERR_EXIT(READ_CGM_SRAM_BUFFERS_FREE_STATUSr(unit, &val));
    stats->sram_buf_free = val; /* place the value into the 32 bits integer */	

    /* Minimal SRAM-Buffers free counter */
    SOCDNX_IF_ERR_EXIT(READ_CGM_SRAM_BUFFERS_FREE_MIN_STATUSr(unit, &val));
    stats->sram_buf_min_free = val; /* place the value into the 32 bits integer */		

    /* Instantaneous SRAM-PDBs free counter */
    SOCDNX_IF_ERR_EXIT(READ_CGM_SRAM_PDBS_FREE_STATUSr(unit, &val));
    stats->sram_pdbs_free = val; /* place the value into the 32 bits integer */	

    /* Minimal SRAM-PDBs free counter */
    SOCDNX_IF_ERR_EXIT(READ_CGM_SRAM_PDBS_FREE_MIN_STATUSr(unit, &val));
    stats->sram_pdbs_min_free = val; /* place the value into the 32 bits integer */		

    /* Instantaneous DRAM-BDBs free counter */
    SOCDNX_IF_ERR_EXIT(READ_CGM_DRAM_BDBS_FREE_STATUSr(unit, &val));
    stats->bdb_free = val; /* place the value into the 32 bits integer */			

    /* Minimal DRAM-BDBs free counter */
    SOCDNX_IF_ERR_EXIT(READ_CGM_DRAM_BDBS_FREE_MIN_STATUSr(unit, &val));
    stats->min_bdb_free = val; /* place the value into the 32 bits integer */

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Get QAX ingress minimal free resources.
 * The counters clear on read.
 */
int qax_itm_min_free_resources_stat_get(
  SOC_SAND_IN int unit,
  SOC_SAND_IN int core,
  SOC_SAND_IN SOC_TMC_ITM_CGM_RSRC_STAT_TYPE type,
  SOC_SAND_OUT uint64 *value
  )
{
    uint32 val32;

    SOCDNX_INIT_FUNC_DEFS;

    switch (type) {
        case SOC_TMC_ITM_CGM_RSRC_STAT_MIN_FREE_BDB:
            /* Minimal DRAM-BDBs free counter */
            SOCDNX_IF_ERR_EXIT(READ_CGM_DRAM_BDBS_FREE_MIN_STATUSr(unit, &val32));
            break;
        
        case SOC_TMC_ITM_CGM_RSRC_STAT_MIN_FREE_SRAM_BUFFERS:
            /* Minimal SRAM-Buffers free counter */
            SOCDNX_IF_ERR_EXIT(READ_CGM_SRAM_BUFFERS_FREE_MIN_STATUSr(unit, &val32));
            break;

        case SOC_TMC_ITM_CGM_RSRC_STAT_MIN_FREE_SRAM_PDB:
            /* Minimal SRAM-PDBs free counter */
            SOCDNX_IF_ERR_EXIT(READ_CGM_SRAM_PDBS_FREE_MIN_STATUSr(unit, &val32));
            break;

        default:
            SOCDNX_IF_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_SOC_MSG("Resource statistic type not supported for this device.")));
    }

    COMPILER_64_SET(*value, 0, val32);

exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     Set the drop precedence value above which 
*     all packets will always be discarded.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  qax_itm_dp_discard_set(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32                  discard_dp
  )
{     
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_SAND_IF_ERR_EXIT(arad_itm_dp_discard_set_verify(unit, discard_dp));

    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, CGM_GLBL_RJCT_PRMSr, SOC_CORE_ALL, 0, DP_LEVEL_RJCT_THf, discard_dp));

exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     Get the drop precedence value above which 
*     all packets will always be discarded.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  qax_itm_dp_discard_get(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_OUT uint32                  *discard_dp
  )
{     
    uint32 fld_val;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(discard_dp);

    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, CGM_GLBL_RJCT_PRMSr, SOC_CORE_ALL, 0, DP_LEVEL_RJCT_THf, &fld_val));

    *discard_dp = fld_val;

exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     Set the size of committed queue size (i.e., the
 *     guaranteed memory) for each VOQ, even in the case that a
 *     set of queues consume most of the memory resources.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
int
  qax_itm_committed_q_size_set(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32                  rt_cls_ndx,
    SOC_SAND_IN  SOC_TMC_ITM_GUARANTEED_INFO *info, 
    SOC_SAND_OUT SOC_TMC_ITM_GUARANTEED_INFO *exact_info 
  )
{
    uint64    data;
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E thresh_type;
    int32 max_guaranteed_limit;
   
    SOCDNX_INIT_FUNC_DEFS;
    
    SOCDNX_NULL_CHECK(info);
    SOCDNX_NULL_CHECK(exact_info);
    

    max_guaranteed_limit = QAX_ITM_GRNT_BYTES_MAX;

    if (rt_cls_ndx >  ARAD_ITM_QT_RT_CLS_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }
    if (info->guaranteed_size[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES] > max_guaranteed_limit) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("guaranteed size %d is out of range\n"), info->guaranteed_size));
    }
    if (info->guaranteed_size[SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES] > QAX_ITM_GRNT_SRAM_BYTES_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("guaranteed sram size %d is out of range\n"), 
                                           info->guaranteed_size[SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES]));
    }
    if (info->guaranteed_size[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS] > QAX_ITM_GRNT_SRAM_PDS_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("guaranteed sram pds %d is out of range\n"), 
                                           info->guaranteed_size[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS]));
    }

    
    SOCDNX_IF_ERR_EXIT(READ_CGM_VOQ_GRNTD_PRMSm(unit, MEM_BLOCK_ANY, rt_cls_ndx, &data));
    
    for (thresh_type = 0; thresh_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; thresh_type++) {
        SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &voq_guaranteed_th_mant_exp[thresh_type],1,
                                                       &data,
                                                       info->guaranteed_size[thresh_type], 
                                                       &exact_info->guaranteed_size[thresh_type]));
    }

    
    SOCDNX_IF_ERR_EXIT( WRITE_CGM_VOQ_GRNTD_PRMSm(unit, MEM_BLOCK_ANY, rt_cls_ndx, &data));


exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     get the size of committed queue size (i.e., the
 *     guaranteed memory) for each VOQ, even in the case that a
 *     set of queues consume most of the memory resources.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
int
  qax_itm_committed_q_size_get(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32                  rt_cls_ndx,
    SOC_SAND_OUT SOC_TMC_ITM_GUARANTEED_INFO *exact_info
  )
{
    uint64 data;
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E thresh_type;

    SOCDNX_INIT_FUNC_DEFS;
    
    SOCDNX_NULL_CHECK(exact_info);

    if (rt_cls_ndx >  ARAD_ITM_QT_RT_CLS_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }
    
    /*
     *    Exact the exponent and the mantissa to get the exact value
     */
    SOCDNX_IF_ERR_EXIT(READ_CGM_VOQ_GRNTD_PRMSm(unit, MEM_BLOCK_ANY, rt_cls_ndx, &data));

    for (thresh_type = 0; thresh_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; thresh_type++) {

        _qax_itm_mantissa_exp_field_get(unit, &voq_guaranteed_th_mant_exp[thresh_type],&data, &exact_info->guaranteed_size[thresh_type]);

    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     Sets FADT drop parameters - per rate-class
*     and drop precedence. The FADT drop mechanism drops
*     packets that are mapped to queues that exceed thresholds
*     of this structure.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  qax_itm_fadt_tail_drop_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  SOC_TMC_ITM_FADT_DROP_INFO  *info,
    SOC_SAND_OUT SOC_TMC_ITM_FADT_DROP_INFO  *exact_info
  )
{

    soc_reg_above_64_val_t data;
    int mem_id;
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E thresh_type;
    uint32 max_size_limit = QAX_ITM_QUEUE_SIZE_BYTES_MAX;
    uint32 alpha_field;
    
    SOCDNX_INIT_FUNC_DEFS;
    
    SOCDNX_NULL_CHECK(info);
    SOCDNX_NULL_CHECK(exact_info);
    
    if (rt_cls_ndx >  ARAD_ITM_QT_RT_CLS_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    if (drop_precedence_ndx >= SOC_TMC_NOF_DROP_PRECEDENCE ) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("drop precedence index %d is out of range\n"),drop_precedence_ndx ));
    }

    if (info->max_threshold[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES] >  max_size_limit) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("max threshold %d is out of range\n"),
                                           info->max_threshold[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES]));
    }

    if (info->max_threshold[SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES] >  QAX_ITM_QUEUE_SIZE_SRAM_BYTES_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("max threshold %d is out of range\n"),
                                           info->max_threshold[SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES]));
    }

    if (info->max_threshold[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS] >  QAX_ITM_QUEUE_SIZE_SRAM_PDS_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("max threshold %d is out of range\n"),
                                           info->max_threshold[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS]));
    }

    for (thresh_type = SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES; thresh_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; thresh_type++) {
        switch (thresh_type) {
            case SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES: {
                mem_id = CGM_VOQ_WORDS_RJCT_PRMSm;
                break;
            }
            case SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES: {
                mem_id = CGM_VOQ_SRAM_WORDS_RJCT_PRMSm;
                break;
            }
            case SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS: {
                mem_id = CGM_VOQ_SRAM_PDS_RJCT_PRMSm;
                break;
            }
            /* Must default. Without default we get compilation error */
            /* coverity[dead_error_begin:FALSE]*/
            default:  {
                SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unexpected threshold type %d\n"), thresh_type));
            }
        }
        
        
        SOCDNX_IF_ERR_EXIT(soc_mem_read(unit, mem_id, MEM_BLOCK_ANY, 
                                        rt_cls_ndx * SOC_TMC_NOF_DROP_PRECEDENCE + drop_precedence_ndx, data));

        SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit,&voq_fadt_max_th_mant_exp[thresh_type], 1, data,
                                                           info->max_threshold[thresh_type], 
                                                           &(exact_info->max_threshold[thresh_type])));
        
        SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit,&voq_fadt_min_th_mant_exp[thresh_type], 1, data,
                                                           info->min_threshold[thresh_type], 
                                                           &(exact_info->min_threshold[thresh_type])));
        
        alpha_field =  _qax_itm_alpha_to_field(unit, info->adjust_factor[thresh_type]);
        soc_mem_field32_set(unit, mem_id, &data, VOQ_FADT_ADJUST_FACTORf,
                            alpha_field);
        exact_info->adjust_factor[thresh_type] = info->adjust_factor[thresh_type];
    
        SOCDNX_IF_ERR_EXIT(soc_mem_write(unit, mem_id, MEM_BLOCK_ANY, 
                                         rt_cls_ndx * SOC_TMC_NOF_DROP_PRECEDENCE + drop_precedence_ndx, data));
    }
    
exit:
    SOCDNX_FUNC_RETURN;

}



/*********************************************************************
*     Sets tail drop parameter - max-queue-size per rate-class
*     and drop precedence. The tail drop mechanism drops
*     packets that are mapped to queues that exceed thresholds
*     of this structure.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  qax_itm_fadt_tail_drop_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_OUT SOC_TMC_ITM_FADT_DROP_INFO  *info
  )

{

    soc_reg_above_64_val_t data;
    int mem_id;
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E thresh_type;
    uint32 alpha_field;

    SOCDNX_INIT_FUNC_DEFS;


    SOCDNX_NULL_CHECK(info);

    if (rt_cls_ndx >  ARAD_ITM_QT_RT_CLS_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    if (drop_precedence_ndx >=  SOC_TMC_NOF_DROP_PRECEDENCE) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("drop precedence index %d is out of range\n"), drop_precedence_ndx));
    }


    for (thresh_type = SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES; thresh_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; thresh_type++) {
        switch (thresh_type) {
            case SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES: {
                mem_id = CGM_VOQ_WORDS_RJCT_PRMSm;
                break;
            }
            case SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES: {
                mem_id = CGM_VOQ_SRAM_WORDS_RJCT_PRMSm;
                break;
            }
            case SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS: {
                mem_id = CGM_VOQ_SRAM_PDS_RJCT_PRMSm;
                break;
            }
            /* Must default. Without default we get compilation error */
            /* coverity[dead_error_begin:FALSE]*/ 
            default:  {
                SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unexpected threshold type %d\n"), thresh_type));
            }
        }
    
        
        SOCDNX_IF_ERR_EXIT(soc_mem_read(unit, mem_id, MEM_BLOCK_ANY, 
                                        rt_cls_ndx * SOC_TMC_NOF_DROP_PRECEDENCE + drop_precedence_ndx, data));

        _qax_itm_mantissa_exp_field_get(unit, &voq_fadt_max_th_mant_exp[thresh_type], data,&(info->max_threshold[thresh_type]));
        _qax_itm_mantissa_exp_field_get(unit, &voq_fadt_min_th_mant_exp[thresh_type], data,&(info->min_threshold[thresh_type]));

        alpha_field = soc_mem_field32_get(unit, mem_id, &data, VOQ_FADT_ADJUST_FACTORf);
        info->adjust_factor[thresh_type] = _qax_itm_field_to_alpha(unit, alpha_field);
    }

exit:
    SOCDNX_FUNC_RETURN;

}

/*********************************************************************
*     Sets tail drop parameter for ECN per rate-class.
*     The tail drop mechanism drops
*     packets that are mapped to queues that exceed thresholds
*     of this structure.
*     Details: in the H file. (search for prototype)
*********************************************************************/
/* 
   NOTE!
   This function has legacy API used for Jericho and older.
   In QAX, it is used for ECN drop ONLY
   and must be called with drop_precedence_ndx == SOC_TMC_NOF_DROP_PRECEDENCE.

   For regular tail drop, 
   qax_itm_fadt_tail_drop_set/qax_itm_fadt_tail_drop_get are used
   
*/
int
  qax_itm_tail_drop_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  ARAD_ITM_TAIL_DROP_INFO  *info,
    SOC_SAND_OUT ARAD_ITM_TAIL_DROP_INFO  *exact_info
  )
{
    soc_reg_above_64_val_t data;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);
    SOCDNX_NULL_CHECK(exact_info);

    if (rt_cls_ndx >  ARAD_ITM_QT_RT_CLS_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    /* in QAX, used for ECN drop only */
    if (drop_precedence_ndx != SOC_TMC_NOF_DROP_PRECEDENCE ) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("This function should be called for ECN drop only\n")));
    }

    /* max_inst_q_size */
    SOCDNX_IF_ERR_EXIT(soc_mem_read(unit, CGM_CNI_PRMSm, MEM_BLOCK_ANY, 
                                    rt_cls_ndx, data));
    
    SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit,&voq_wred_ecn_max_size_th_mant_exp, 1, data,
                                                       info->max_inst_q_size,
                                                       &(exact_info->max_inst_q_size)));

    SOCDNX_IF_ERR_EXIT(soc_mem_write(unit, CGM_CNI_PRMSm, MEM_BLOCK_ANY, 
                                    rt_cls_ndx, data));

exit:
    SOCDNX_FUNC_RETURN;

}

/*********************************************************************
*     Gets tail drop parameter max-queue-size for ECN  per rate-class.
*     The tail drop mechanism drops
*     packets that are mapped to queues that exceed thresholds
*     of this structure.
*     Details: in the H file. (search for prototype)
*********************************************************************/
/* 
   NOTE!
   This function has legacy API used for Jericho and older.
   In QAX, it is used for ECN drop ONLY
   and must be called with drop_precedence_ndx == SOC_TMC_NOF_DROP_PRECEDENCE.

   For regular tail drop, 
   qax_itm_fadt_tail_drop_set/qax_itm_fadt_tail_drop_get are used
   
*/
int
  qax_itm_tail_drop_get(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_OUT ARAD_ITM_TAIL_DROP_INFO  *info
  )
{
    soc_reg_above_64_val_t data;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    if (rt_cls_ndx >  ARAD_ITM_QT_RT_CLS_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    /* in QAX, used for ECN drop only */
    if (drop_precedence_ndx != SOC_TMC_NOF_DROP_PRECEDENCE ) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("This function should be called for ECN drop only\n")));
    }

    /* max_inst_q_size */
    SOCDNX_IF_ERR_EXIT(soc_mem_read(unit, CGM_CNI_PRMSm, MEM_BLOCK_ANY, 
                                    rt_cls_ndx, data));
    
    _qax_itm_mantissa_exp_field_get(unit,&voq_wred_ecn_max_size_th_mant_exp, data,
                                    &info->max_inst_q_size);


exit:
    SOCDNX_FUNC_RETURN;
}


/*********************************************************************
*    Gets the WRED parameters from the data in HW
*    By converting them appropriately.
*********************************************************************/
STATIC void
_qax_itm_hw_data_to_wred_info (
        SOC_SAND_IN  int                                                  unit,
        SOC_SAND_IN  uint32                                               min_avrg_th,
        SOC_SAND_IN  uint32                                               max_avrg_th,
        SOC_SAND_IN  uint32                                               c1,
        SOC_SAND_IN  uint32                                               c2,
        SOC_SAND_IN  uint32                                               c3,
        SOC_SAND_OUT SOC_TMC_ITM_WRED_QT_DP_INFO                          *wred_param
  )
{
    SOC_SAND_U64
        u64_1,
        u64_2,
        u64_3; 
    uint32 remainder;
    
    /* Min average threshold */
    wred_param->min_avrg_th = min_avrg_th;

    /* Max average threshold */
    wred_param->max_avrg_th = max_avrg_th;
    
    /*
     * max_packet_size
     */
    wred_param->max_packet_size = ((c3 == 0) && (c2 == 1)) ? 0 : (0x1 << (c3)) * SOC_TMC_ITM_WRED_GRANULARITY;
    
    /*
     * C2 = MaxProb * MinTh / (MaxTh - MinTh) 
     * ==>
     * MaxProb = ( C2 * (MaxTh - MinTh) ) / MinTh
    */
    soc_sand_u64_multiply_longs(c2, (max_avrg_th - min_avrg_th), &u64_1);
    soc_sand_u64_devide_u64_long(&u64_1, min_avrg_th, &u64_2);
    remainder = soc_sand_u64_devide_u64_long(&u64_2, SOC_TMC_WRED_NORMALIZE_FACTOR, &u64_3);
    soc_sand_u64_to_long(&u64_3, &wred_param->max_probability);
    if (remainder > (SOC_TMC_WRED_NORMALIZE_FACTOR/2))
    {
          wred_param->max_probability++;
    }
    
    if(wred_param->max_probability > 100)
    {
        wred_param->max_probability = 100;
    }
}


STATIC void
  _qax_itm_wred_info_to_hw_data (
     SOC_SAND_IN   int unit,
     SOC_SAND_IN   SOC_TMC_ITM_WRED_QT_DP_INFO                          *wred_param,
     SOC_SAND_IN   uint32                                               min_avrg_th_exact,
     SOC_SAND_IN   uint32                                               max_avrg_th_exact,
     SOC_SAND_OUT  uint32*                                              c1,
     SOC_SAND_OUT  uint32*                                              c2,
     SOC_SAND_OUT  uint32*                                              c3
  )
{
    uint32
        max_prob,
        calc,
        max_val_c1;
    int32
        avrg_th_diff_wred_granular = 0;
    int32
        min_avrg_th_exact_wred_granular,
        max_avrg_th_exact_wred_granular;
    uint32
        trunced;
    SOC_SAND_U64
        u64_2,
        u64_c2 = {{0}};
    
    
    trunced = FALSE;
    
    /*
     * min_avrg_th
     */
    min_avrg_th_exact_wred_granular = min_avrg_th_exact / SOC_TMC_ITM_WRED_GRANULARITY;
    
    /*
     * max_avrg_th
     */
    max_avrg_th_exact_wred_granular = max_avrg_th_exact / SOC_TMC_ITM_WRED_GRANULARITY;
    
    /*
     * max_packet_size
     */
    calc = wred_param->max_packet_size;
    if (calc > SOC_TMC_ITM_WRED_MAX_PACKET_SIZE_FOR_CALC)
    {
        calc = SOC_TMC_ITM_WRED_MAX_PACKET_SIZE_FOR_CALC;
    }
    calc = SOC_SAND_DIV_ROUND_UP(calc, SOC_TMC_ITM_WRED_GRANULARITY );
    *c3 = (max_avrg_th_exact == 0 ? 0 : soc_sand_log2_round_up(calc));
    
    /*
     * max_probability
     */
    max_prob = (wred_param->max_probability);
    
    /*
     * max_probability
     * C1 = ((2^32)/100)*max-prob / (max-th - min-th) in powers of 2
     */
    if(max_prob>=100)
    {
        max_prob = 99;
    }
    calc = SOC_TMC_WRED_NORMALIZE_FACTOR * max_prob; 
    
    /*
     * We do not use 'SOC_SAND_DIV_ROUND' or 'SOC_SAND_DIV_ROUND_UP'
     * because at this point we might have in calc '((2^32)/100)*max-prob'
     * which can be very large number and the other dividers do ADD before
     * the division.
     */
    max_val_c1 = 31; /* soc_sand_log2_round_down(0xFFFFFFFF) */
    
    avrg_th_diff_wred_granular =
        (max_avrg_th_exact_wred_granular - min_avrg_th_exact_wred_granular);
    
    if(avrg_th_diff_wred_granular == 0)
    {
        *c1 = max_val_c1;
    }
    else
    {
        calc = SOC_SAND_DIV_ROUND_DOWN(calc, avrg_th_diff_wred_granular);
    *c1 = soc_sand_log2_round_down(calc);
    }
    
    if(*c1 < max_val_c1)
    {
        /*
         * Check if a bigger C1 gives closer result of the value we add.
         */
        uint32
            now     = 1 <<(*c1),
            changed = 1 <<(*c1+1),
            diff_with_now,
            diff_with_change;
        
        diff_with_change = changed-calc;
        diff_with_now    = calc-now;
        if( diff_with_change < diff_with_now)
        {
            *c1 += 1;
        }
    }
    
    SOC_SAND_LIMIT_FROM_ABOVE(*c1, max_val_c1);
    
    if (min_avrg_th_exact_wred_granular > 0)
    { /* This limit from above is HW restriction */
        max_val_c1 = SOC_SAND_DIV_ROUND_DOWN(0xFFFFFFFF, min_avrg_th_exact_wred_granular);
        max_val_c1 = soc_sand_log2_round_down(max_val_c1);
        SOC_SAND_LIMIT_FROM_ABOVE(*c1, max_val_c1);
    }
    
    /*
     * max_probability
     * C2 = FACTOR * max-prob * min-th / (max-th - min-th)
     */
    soc_sand_u64_multiply_longs(
        SOC_TMC_WRED_NORMALIZE_FACTOR,
        max_prob * min_avrg_th_exact_wred_granular,
        &u64_2
        );
    soc_sand_u64_devide_u64_long(&u64_2, avrg_th_diff_wred_granular, &u64_c2);
    
    trunced = soc_sand_u64_to_long(&u64_c2, c2);
    
    if (trunced)
    {
        *c2 = 0xFFFFFFFF;
    }
    *c2 = (max_avrg_th_exact == 0 ? 1 : *c2);
    
}

void _qax_itm_wred_mem_fields_get( 
        int unit,
        int rt_cls_ndx,
        int drop_precedence_ndx, 
        int* is_ecn,
        int* mem,
        int* wred_en_field,
        int* c1_field, 
        int* c2_field, 
        int* c3_field,
        int* entry_offset
        )
{
    *is_ecn = (drop_precedence_ndx == SOC_TMC_NOF_DROP_PRECEDENCE);
    
    if (*is_ecn) {
        *mem = CGM_CNI_PRMSm;

        *wred_en_field = WRED_ENf;
        *c1_field = WRED_C_1f;
        *c2_field = WRED_C_2f;
        *c3_field = WRED_C_3f;

        
        *entry_offset = rt_cls_ndx;
    } else {
        *mem = CGM_VOQ_WORDS_RJCT_PRMSm;

        *wred_en_field = VOQ_WRED_ENf;
        *c1_field = VOQ_WRED_C_1f;
        *c2_field = VOQ_WRED_C_2f;
        *c3_field = VOQ_WRED_C_3f;

        *entry_offset = (rt_cls_ndx * SOC_TMC_NOF_DROP_PRECEDENCE) + drop_precedence_ndx;
    }

}

/*********************************************************************
*     Sets WRED parameters per rate-class and drop precedence,
*     including wred-enable and the admit test logic. Also, as
*     part of the WRED parameters; max-queue,
*     WRED-thresholds/probability. To configure WRED
*     Configuration that is per queue-type only (exponential
*     weight queue), use the function
*     arad_itm_wred_exp_wq_set.
*     Details: in the H file. (search for prototype)
*********************************************************************/
STATIC int
  _qax_itm_wred_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  ARAD_ITM_WRED_QT_DP_INFO *info
  )
{
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);


    if (rt_cls_ndx > ARAD_ITM_QT_RT_CLS_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    if (drop_precedence_ndx > SOC_TMC_NOF_DROP_PRECEDENCE ) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("drop precedence index %d is out of range\n"),
                                           drop_precedence_ndx ));
    }

    if (info->min_avrg_th > info->max_avrg_th)
    {
       SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("min threshold %d is higher than max threshold\n"),
                                          info->min_avrg_th, info->max_avrg_th));
    }
    
    if (info->min_avrg_th > QAX_ITM_Q_WRED_INFO_MIN_AVRG_TH_MAX)
    {
       SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("min threshold %d is out of range\n"),
                                          info->min_avrg_th));
    }

    if (info->max_avrg_th > QAX_ITM_Q_WRED_INFO_MAX_AVRG_TH_MAX)
    {
       SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("max threshold %d is out of range\n"),
                                          info->max_avrg_th));
    }

    if (info->max_probability > QAX_ITM_WRED_QT_DP_INFO_MAX_PROBABILITY_MAX)
    {
       SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("max probability %d is out of range\n"),
                                          info->max_probability));
    }

    if (info->max_packet_size > SOC_TMC_ITM_WRED_MAX_PACKET_SIZE)
    {
       SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("max packet size %d is out of range\n"),
                                          info->max_packet_size));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     Sets WRED parameters per rate-class and drop precedence,
*     including wred-enable and the admit test logic. Also, as
*     part of the WRED parameters; max-queue,
*     WRED-thresholds/probability. To configure WRED
*     Configuration that is per queue-type only (exponential
*     weight queue), use the function
*     arad_itm_wred_exp_wq_set.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  qax_itm_wred_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  ARAD_ITM_WRED_QT_DP_INFO *info,
    SOC_SAND_OUT ARAD_ITM_WRED_QT_DP_INFO *exact_info
  )
{
    soc_reg_above_64_val_t data;
    int mem;
    int wred_en_field, c1_field, c2_field, c3_field;
    int entry_offset;
    uint32 exact_min_avrg_th = 0, exact_max_avrg_th = 0;
    uint32 c1, c2, c3;
    int is_ecn;
    
    SOCDNX_INIT_FUNC_DEFS;
    
    SOCDNX_NULL_CHECK(info);
    SOCDNX_NULL_CHECK(exact_info);

    SOCDNX_IF_ERR_EXIT(_qax_itm_wred_verify(unit, rt_cls_ndx, drop_precedence_ndx, info));

    _qax_itm_wred_mem_fields_get(unit, rt_cls_ndx, drop_precedence_ndx, 
                                  &is_ecn, &mem,
                                  &wred_en_field, &c1_field, &c2_field, &c3_field,
                                  &entry_offset
        );
    

    SOCDNX_IF_ERR_EXIT(soc_mem_read(unit, mem, MEM_BLOCK_ANY, 
                                    entry_offset , data));
    
    SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit,&voq_wred_ecn_min_th_mant_exp[is_ecn], 1, data,
                                                       info->min_avrg_th,
                                                       &exact_min_avrg_th));

    SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &voq_wred_ecn_max_th_mant_exp[is_ecn], 1, data,
                                                       info->max_avrg_th,
                                                       &exact_max_avrg_th));

    _qax_itm_wred_info_to_hw_data(unit, info, exact_min_avrg_th, exact_max_avrg_th, &c1, &c2, &c3);

    soc_mem_field32_set(unit, mem, data, wred_en_field, info->wred_en);
    soc_mem_field32_set(unit, mem, data, c2_field, c2);
    soc_mem_field32_set(unit, mem, data, c3_field, c3);
    soc_mem_field32_set(unit, mem, data, c1_field, c1);
    if (!is_ecn) {
        soc_mem_field32_set(unit, mem, data, VOQ_WRED_IGNR_PKT_SIZEf, SOC_SAND_BOOL2NUM(info->ignore_packet_size));
    }
 
    SOCDNX_IF_ERR_EXIT(soc_mem_write(unit, mem, MEM_BLOCK_ANY, 
                                     entry_offset , data));
    
    _qax_itm_hw_data_to_wred_info(unit, exact_min_avrg_th, exact_max_avrg_th, c1, c2, c3, exact_info);
                       
    exact_info->wred_en = info->wred_en;

    exact_info->ignore_packet_size = (!is_ecn ? SOC_SAND_BOOL2NUM(info->ignore_packet_size) : 0);

exit:
    SOCDNX_FUNC_RETURN;
}


/*********************************************************************
*     Sets WRED parameters per rate-class and drop precedence,
*     including wred-enable and the admit test logic. Also, as
*     part of the WRED parameters; max-queue,
*     WRED-thresholds/probability. To configure WRED
*     Configuration that is per queue-type only (exponential
*     weight queue), use the function
*     arad_itm_wred_exp_wq_set.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  qax_itm_wred_get(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_OUT ARAD_ITM_WRED_QT_DP_INFO *info
  )
{
    soc_reg_above_64_val_t data;
    int mem;
    int wred_en_field, c1_field, c2_field, c3_field;
    int entry_offset;
    uint32 min_avrg_th = 0, max_avrg_th = 0;
    uint32 c1, c2, c3, wred_pckt_sz_ignr = 0;
    int is_ecn;
    uint32 wred_en;
 

    SOCDNX_INIT_FUNC_DEFS;
    
    SOCDNX_NULL_CHECK(info);
 
    if (rt_cls_ndx > ARAD_ITM_QT_RT_CLS_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    if (drop_precedence_ndx > SOC_TMC_NOF_DROP_PRECEDENCE ) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("drop precedence index %d is out of range\n"),drop_precedence_ndx ));
    }
    
    _qax_itm_wred_mem_fields_get( unit,
                                  rt_cls_ndx, drop_precedence_ndx, 
                                  &is_ecn, &mem, &wred_en_field,
                                  &c1_field, &c2_field, &c3_field,
                                  &entry_offset
            );


    SOCDNX_IF_ERR_EXIT(soc_mem_read(unit, mem, MEM_BLOCK_ANY, 
                                    entry_offset , data));


    /*
     * min_avrg_th
     */
    _qax_itm_mantissa_exp_field_get(unit, &voq_wred_ecn_min_th_mant_exp[is_ecn], &data, &min_avrg_th);
    
    /*
     * max_avrg_th
     */
    _qax_itm_mantissa_exp_field_get(unit, &voq_wred_ecn_max_th_mant_exp[is_ecn], &data, &max_avrg_th);

    wred_en = soc_mem_field32_get(unit, mem, data, wred_en_field);
    c2 = soc_mem_field32_get(unit, mem, data, c2_field );
    c3 = soc_mem_field32_get(unit, mem, data, c3_field);
    c1 = soc_mem_field32_get(unit, mem, data, c1_field);

    if (!is_ecn) {
        wred_pckt_sz_ignr = soc_mem_field32_get(unit, mem, data, VOQ_WRED_IGNR_PKT_SIZEf);
    }

    _qax_itm_hw_data_to_wred_info (unit, min_avrg_th, max_avrg_th, c1, c2, c3, info);
                       
    info->wred_en = SOC_SAND_NUM2BOOL(wred_en);

    info->ignore_packet_size =
        SOC_SAND_NUM2BOOL(wred_pckt_sz_ignr);
    
exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     Sets exponential weighted queue per rate-class. The
*     calculation the device does: Average_constant =
*     2^(-RED_exp_weight)if (Instantaneous-Queue-size <
*     Average-queue-size) Average-queue-size =
*     Instantaneous-Queue-size else Average-queue-size =
*     (1-Average_constant)*Average-queue-size +
*     Average_constant*Instantaneous-Queue-size To configure
*     WRED configuration which are per queue-type and dp, use
*     the function arad_itm_wred_info_set.
*     Details: in the H file. (search for prototype)
*********************************************************************/
STATIC int
  _qax_itm_wred_exp_wq_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                  exp_wq
  )
{
    SOCDNX_INIT_FUNC_DEFS;

    if (rt_cls_ndx > ARAD_ITM_QT_RT_CLS_MAX ) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    if (exp_wq > ARAD_ITM_WQ_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("exp. weight %d is out of range\n"), exp_wq));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     Sets exponential weighted queue per rate-class. The
*     calculation the device does: Average_constant =
*     2^(-RED_exp_weight)if (Instantaneous-Queue-size <
*     Average-queue-size) Average-queue-size =
*     Instantaneous-Queue-size else Average-queue-size =
*     (1-Average_constant)*Average-queue-size +
*     Average_constant*Instantaneous-Queue-size To configure
*     WRED configuration which are per queue-type and dp, use
*     the functionarad_itm_wred_info_set.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  qax_itm_wred_exp_wq_set(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 exp_wq,
    SOC_SAND_IN  uint8                  enable
  )
{
    uint32 data;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT (_qax_itm_wred_exp_wq_verify(
                            unit,
                            rt_cls_ndx,
                            exp_wq
                            ));

    SOCDNX_IF_ERR_EXIT(soc_mem_read(
                           unit,
                           CGM_VOQ_AVRG_PRMSm,
                           MEM_BLOCK_ANY,
                           rt_cls_ndx,
                           &data
                           ));
    
    soc_mem_field32_set(unit, CGM_VOQ_AVRG_PRMSm, &data, AVRG_WEIGHTf, exp_wq);
    soc_mem_field32_set(unit, CGM_VOQ_AVRG_PRMSm, &data, AVRG_ENf, enable);

    SOCDNX_IF_ERR_EXIT(soc_mem_write(
                           unit,
                           CGM_VOQ_AVRG_PRMSm,
                           MEM_BLOCK_ANY,
                           rt_cls_ndx,
                           &data
                           ));
exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     Sets exponential weighted queue per rate-class. The
*     calculation the device does: Average_constant =
*     2^(-RED_exp_weight)if (Instantaneous-Queue-size <
*     Average-queue-size) Average-queue-size =
*     Instantaneous-Queue-sizeelse Average-queue-size =
*     (1-Average_constant)*Average-queue-size +
*     Average_constant*Instantaneous-Queue-size To configure
*     WRED configuration which are per queue-type and dp, use
*     the functionarad_itm_wred_info_set.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  qax_itm_wred_exp_wq_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32              rt_cls_ndx,
    SOC_SAND_OUT  uint32             *exp_wq
  )
{
    uint32 data;

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_NULL_CHECK(exp_wq);

    if (rt_cls_ndx > ARAD_ITM_QT_RT_CLS_MAX ) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }
    
    SOCDNX_IF_ERR_EXIT(soc_mem_read(
                           unit,
                           CGM_VOQ_AVRG_PRMSm,
                           MEM_BLOCK_ANY,
                           rt_cls_ndx,
                           &data
                           ));
    
   *exp_wq = soc_mem_field32_get(unit, CGM_VOQ_AVRG_PRMSm, &data, AVRG_WEIGHTf);

exit:
    SOCDNX_FUNC_RETURN;
}

int
  qax_itm_dyn_total_thresh_set(
    SOC_SAND_IN int    unit,
    SOC_SAND_IN int    core,
    SOC_SAND_IN uint8  is_ocb_only,
                int32  reservation_increase_array[SOC_DPP_DEFS_MAX(NOF_CORES)][SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] /* the (signed) amount in which the thresholds should decrease (according to 100% as will be set for DP 0) */
  )
{
    uint32
        res;
    SOC_TMC_ITM_GUARANTEED_RESOURCE guaranteed_resources;
    soc_dpp_guaranteed_pair_t *guaranteed_pair;
    int32 reservation_increase;
    int core_id;
    int dp;
    soc_reg_above_64_val_t data;

    itm_mantissa_exp_threshold_info voq_shared_oc_set_th_mant_exp[] = {
        {/* words */
            CGM_VOQ_SHRD_OC_RJCT_THm,
            WORDS_SET_THf,
            8, /* mantissa_bits */
            5, /* exp_bits */
            1  /* factor */ /* factor is always 1 since the API is with other thresholds and not with user */
        },
        {/* sram_words */
            CGM_VOQ_SHRD_OC_RJCT_THm,
            SRAM_WORDS_SET_THf,
            8, /* mantissa_bits */
            4, /* exp_bits */
            1  /* factor */
        },
        {/* sram_pds */
            CGM_VOQ_SHRD_OC_RJCT_THm,
            SRAM_PDS_SET_THf,
            8, /* mantissa_bits */
            4, /* exp_bits */
            1 /* factor */
        }
    };    

    itm_mantissa_exp_threshold_info voq_shared_oc_clr_th_mant_exp[] = {
        {/* words */
            CGM_VOQ_SHRD_OC_RJCT_THm,
            WORDS_CLR_THf,
            8, /* mantissa_bits */
            5, /* exp_bits */
            1  /* factor */ /* factor is always 1 since the API is with other thresholds and not with user */
        },
        {/* sram_words */
            CGM_VOQ_SHRD_OC_RJCT_THm,
            SRAM_WORDS_CLR_THf,
            8, /* mantissa_bits */
            4, /* exp_bits */
            1  /* factor */
        },
        {/* sram_pds */
            CGM_VOQ_SHRD_OC_RJCT_THm,
            SRAM_PDS_CLR_THf,
            8, /* mantissa_bits */
            4, /* exp_bits */
            1 /* factor */
        }
    };    

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_NULL_CHECK(reservation_increase_array);

    if (!is_ocb_only) {
        int thresh_type;
        /* ocb only is not relevant for QAX */
        SOC_DPP_CORES_ITER(core, core_id){
            res = sw_state_access[unit].dpp.soc.qax.tm.guaranteed_info.get(unit, core_id, &guaranteed_resources);
            SOCDNX_IF_ERR_EXIT(res);
            for (thresh_type = 0; thresh_type < DPP_COSQ_INGRESS_THRESHOLD_NOF_TYPES; thresh_type++) {
                guaranteed_pair =  &(guaranteed_resources.guaranteed[thresh_type]);
                reservation_increase = reservation_increase_array[core_id][thresh_type];

                if (reservation_increase < 0 &&  ((uint32)(-reservation_increase)) > guaranteed_pair->used) {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_SOCDNX_MSG("requested change in reserved resource %d is out of range\n"), 
                                                          reservation_increase));
                }
                /* update the amount of the resource that is left */
                if (reservation_increase) {
                    int32 resource_left_calc =  ((int32)(guaranteed_pair->total -
                                                         (guaranteed_pair->used))) - reservation_increase;
                    uint32 resource_left =  resource_left_calc;
                    uint32 dummy;
                    int numerator[] = { 20, /* 20/20 = 100% for GREEN  */ 
                                        17, /* 17/20 =  85% for YELLOW */
                                        15, /* 15/20 =  75% for RED    */
                                        0   /*  0/20 =   0% for BLACK  */
                    };
                    int denominator = 20;
                    
                    if (resource_left_calc < 0) { 
                        /* check if we are out of the resource */
                        SOCDNX_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_SOCDNX_MSG("out of resources. Left amount %d\n"), 
                                                          resource_left_calc));
                    }
                    
                    for (dp = 0 ; dp < SOC_TMC_NOF_DROP_PRECEDENCE; dp++) {
                        /* note that set and clr fields are in the same register */
                        SOCDNX_IF_ERR_EXIT(READ_CGM_VOQ_SHRD_OC_RJCT_THm(unit, MEM_BLOCK_ANY, dp, &data));
                        
                        if (resource_left) { /* configure drop thresholds according to new amount of resource left */
                            SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &voq_shared_oc_set_th_mant_exp[thresh_type], 0, data,
                                                                               resource_left/denominator*numerator[dp],
                                                                               &dummy));
                            SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit, &voq_shared_oc_clr_th_mant_exp[thresh_type], 0, data,
                                                                               resource_left/denominator*numerator[dp],
                                                                               &dummy));
                        }
                        else {
                            soc_mem_field32_set(unit, CGM_VOQ_SHRD_OC_RJCT_THm, data, voq_shared_oc_set_th_mant_exp[thresh_type].field_id, 
                                                ARAD_ITM_GRNT_BDS_OR_DBS_DISABLED);
                            soc_mem_field32_set(unit, CGM_VOQ_SHRD_OC_RJCT_THm, data, voq_shared_oc_clr_th_mant_exp[thresh_type].field_id, 
                                                ARAD_ITM_GRNT_BDS_OR_DBS_DISABLED);
                        }
                        SOCDNX_IF_ERR_EXIT(WRITE_CGM_VOQ_SHRD_OC_RJCT_THm(unit, MEM_BLOCK_ANY, dp, data));
                    }
                    guaranteed_pair->used += reservation_increase;
                }
                res = sw_state_access[unit].dpp.soc.qax.tm.guaranteed_info.set(unit, core_id, &guaranteed_resources);
                SOCDNX_SAND_IF_ERR_EXIT(res);
            }
        }
    }

exit:
  SOCDNX_FUNC_RETURN;
}



/*********************************************************************
*     Gets the queue size of a queue
*********************************************************************/
int
  qax_itm_queue_dyn_info_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  uint32                  queue_ndx,
    SOC_SAND_OUT SOC_TMC_ITM_QUEUE_DYN_INFO *info
  )  
{
    soc_reg_above_64_val_t data;

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_NULL_CHECK(info);

    if (queue_ndx > ARAD_MAX_QUEUE_ID(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("queue index %d is out of range\n"), queue_ndx));
    }

    SOCDNX_IF_ERR_EXIT(READ_CGM_VOQ_SIZEm(unit, CGM_BLOCK(unit, core), queue_ndx, data));

    info->pq_inst_que_size[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES] = soc_mem_field32_get(unit, CGM_VOQ_SIZEm, data, WORDS_SIZEf) * 16;
    info->pq_inst_que_size[SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES] = soc_mem_field32_get(unit, CGM_VOQ_SIZEm, data, SRAM_WORDS_SIZEf) * 16;
    info->pq_inst_que_size[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS] = soc_mem_field32_get(unit, CGM_VOQ_SIZEm, data, SRAM_PDS_SIZEf);

exit:
  SOCDNX_FUNC_RETURN;
}

/*----------------------------------------------------------------------------------------------------------*/
/*      DRAM bound     */

int qax_itm_dram_bound_alpha_field_get( SOC_SAND_IN int                 unit,
                                        SOC_SAND_IN SOC_TMC_INGRESS_DRAM_BOUND_TYPE_E dram_thresh,
                                        SOC_SAND_IN SOC_TMC_INGRESS_THRESHOLD_TYPE_E thresh_type)
{
    int field_id = INVALIDf;

    switch (dram_thresh) {
        case SOC_TMC_INGRESS_DRAM_BOUND:
            switch (thresh_type) {
                case SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                    field_id = SRAM_WORDS_BOUND_ADJUST_FACTORf;
                    break;
                case SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                    field_id = SRAM_PDS_BOUND_ADJUST_FACTORf;
                    break;
                default:
                    break;
            }
            break;
        case SOC_TMC_INGRESS_DRAM_BOUND_RECOVERY_FAILURE:
            switch (thresh_type) {
                case SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                    field_id = SRAM_WORDS_RECOVERY_ADJUST_FACTORf;
                    break;
                case SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                    field_id = SRAM_PDS_RECOVERY_ADJUST_FACTORf;
                    break;
                default:
                    break;
                }
        default:
            break;
    }

    return field_id;
}

typedef enum {
    QAX_DRAM_BOUND_THRESH_MIN = 0,
    QAX_DRAM_BOUND_THRESH_MAX,
    QAX_DRAM_BOUND_THRESH_FREE_MIN,
    QAX_DRAM_BOUND_THRESH_FREE_MAX
} qax_dram_bound_thresh_type_t;

int _qax_itm_dram_bound_mant_exp_info_get(
    int unit,
    SOC_TMC_INGRESS_DRAM_BOUND_TYPE_E dram_thresh,
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E resource_type, 
    qax_dram_bound_thresh_type_t thresh_type, 
    itm_mantissa_exp_threshold_info* info
    )
{
    SOCDNX_INIT_FUNC_DEFS;
    
    SOCDNX_NULL_CHECK(info);

    info->mem_id = CGM_VOQ_DRAM_BOUND_PRMSm;
    info->factor = (resource_type == SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES ? 16 : 1);

    switch (thresh_type) {
        case QAX_DRAM_BOUND_THRESH_MIN:
            switch (dram_thresh) {
                case SOC_TMC_INGRESS_DRAM_BOUND:
                    switch (resource_type) {
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_BOUND_MIN_THf;
                            break;
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_BOUND_MIN_THf;
                            break;
                        default:
                            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                case SOC_TMC_INGRESS_DRAM_BOUND_RECOVERY_FAILURE:
                    switch (resource_type) {
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_RECOVERY_MIN_THf;
                            break;
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_RECOVERY_MIN_THf;
                            break;
                        default:
                            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                default:
                    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unexpected dram threshold %d\n"), dram_thresh));
            }
            break;
        case QAX_DRAM_BOUND_THRESH_MAX:
            switch (dram_thresh) {
                case SOC_TMC_INGRESS_DRAM_BOUND:
                    switch (resource_type) {
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_BOUND_MAX_THf;
                            break;
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_BOUND_MAX_THf;
                            break;
                        default:
                            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                case SOC_TMC_INGRESS_DRAM_BOUND_RECOVERY_FAILURE:
                    switch (resource_type) {
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_RECOVERY_MAX_THf;
                            break;
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_RECOVERY_MAX_THf;
                            break;
                        default:
                            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                default:
                    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unexpected dram threshold %d\n"), dram_thresh));
            }
            break;
        case QAX_DRAM_BOUND_THRESH_FREE_MIN:
            switch (dram_thresh) {
                case SOC_TMC_INGRESS_DRAM_BOUND:
                    switch (resource_type) {
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_BOUND_FREE_MIN_THf;
                            break;
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_BOUND_FREE_MIN_THf;
                            break;
                        default:
                            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                case SOC_TMC_INGRESS_DRAM_BOUND_RECOVERY_FAILURE:
                    switch (resource_type) {
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_RECOVERY_FREE_MIN_THf;
                            break;
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_RECOVERY_FREE_MIN_THf;
                            break;
                        default:
                            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                default:
                    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unexpected dram threshold %d\n"), dram_thresh));
            }
            break;
        case QAX_DRAM_BOUND_THRESH_FREE_MAX:
            switch (dram_thresh) {
                case SOC_TMC_INGRESS_DRAM_BOUND:
                    switch (resource_type) {
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_BOUND_FREE_MAX_THf;
                            break;
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_BOUND_FREE_MAX_THf;
                            break;
                        default:
                            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                case SOC_TMC_INGRESS_DRAM_BOUND_RECOVERY_FAILURE:
                    switch (resource_type) {
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_RECOVERY_FREE_MAX_THf;
                            break;
                        case SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_RECOVERY_FREE_MAX_THf;
                            break;
                        default:
                            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                default:
                    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unexpected dram threshold %d\n"), dram_thresh));
            }
            break;
    }

    info->mantissa_bits = 8;
    info->exp_bits = soc_mem_field_length(unit, info->mem_id, info->field_id) - info->mantissa_bits;

exit:
  SOCDNX_FUNC_RETURN;

}

int
  qax_itm_dram_bound_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
                 SOC_TMC_ITM_DRAM_BOUND_INFO  *info,
    SOC_SAND_OUT SOC_TMC_ITM_DRAM_BOUND_INFO  *exact_info
  )
{
    soc_reg_above_64_val_t data;
    SOC_TMC_INGRESS_DRAM_BOUND_TYPE_E dram_thresh;
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E resource_type;
    uint32 alpha_field_value;
    int alpha_field_id;
 
    SOCDNX_INIT_FUNC_DEFS;
    
    SOCDNX_NULL_CHECK(info);
    SOCDNX_NULL_CHECK(exact_info);

    if (rt_cls_ndx >  ARAD_ITM_QT_RT_CLS_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    SOCDNX_IF_ERR_EXIT(soc_mem_read(unit, CGM_VOQ_DRAM_BOUND_PRMSm, MEM_BLOCK_ANY, 
                                    rt_cls_ndx, data));
    for (resource_type = SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES; resource_type <= SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS; resource_type++) {
        for (dram_thresh = SOC_TMC_INGRESS_DRAM_BOUND; dram_thresh < SOC_TMC_INGRESS_DRAM_BOUND_NOF_TYPES; dram_thresh++) {
            itm_mantissa_exp_threshold_info mantissa_exp_info;
            SOC_TMC_ITM_DRAM_BOUND_THRESHOLD* dram_threshold = SOC_TMC_ITM_DRAM_BOUND_INFO_thresh_get(unit, info, dram_thresh, resource_type);
            SOC_TMC_ITM_DRAM_BOUND_THRESHOLD* exact_dram_threshold = SOC_TMC_ITM_DRAM_BOUND_INFO_thresh_get(unit, exact_info, dram_thresh, resource_type);
            

            SOCDNX_IF_ERR_EXIT(_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, QAX_DRAM_BOUND_THRESH_MAX, &mantissa_exp_info));
            SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit,&mantissa_exp_info, 1, data,
                                                               dram_threshold->max_threshold, 
                                                               &(exact_dram_threshold->max_threshold)));
        
            SOCDNX_IF_ERR_EXIT(_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, QAX_DRAM_BOUND_THRESH_MIN, &mantissa_exp_info));
            SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit,&mantissa_exp_info, 1, data,
                                                               dram_threshold->min_threshold, 
                                                               &(exact_dram_threshold->min_threshold)));

            SOCDNX_IF_ERR_EXIT(_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, QAX_DRAM_BOUND_THRESH_FREE_MAX, &mantissa_exp_info));
            SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit,&mantissa_exp_info, 1, data,
                                                               dram_threshold->free_max_threshold, 
                                                               &(exact_dram_threshold->free_max_threshold)));
        
            SOCDNX_IF_ERR_EXIT(_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, QAX_DRAM_BOUND_THRESH_FREE_MIN, &mantissa_exp_info));
            SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit,&mantissa_exp_info, 1, data,
                                                               dram_threshold->free_min_threshold, 
                                                               &(exact_dram_threshold->free_min_threshold)));


            alpha_field_id = qax_itm_dram_bound_alpha_field_get(unit,dram_thresh, resource_type);
            if (alpha_field_id == INVALIDf) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("can't find alpha field name\n")));
            }
            alpha_field_value =  _qax_itm_alpha_to_field(unit, dram_threshold->alpha);
            soc_mem_field32_set(unit, CGM_VOQ_DRAM_BOUND_PRMSm, &data, alpha_field_id, alpha_field_value);
            exact_dram_threshold->alpha = dram_threshold->alpha;
    
        }
    }

    SOCDNX_IF_ERR_EXIT(_qax_itm_mantissa_exp_field_set(unit,&voq_dram_bound_qsize_recovery_th_mant_exp, 1, data,
                                                       info->qsize_recovery_th,
                                                       &(exact_info->qsize_recovery_th)));

    SOCDNX_IF_ERR_EXIT(soc_mem_write(unit, CGM_VOQ_DRAM_BOUND_PRMSm, MEM_BLOCK_ANY, 
                                     rt_cls_ndx , data));

exit:
  SOCDNX_FUNC_RETURN;
}

int
  qax_itm_dram_bound_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_OUT SOC_TMC_ITM_DRAM_BOUND_INFO  *info
  )
{
    soc_reg_above_64_val_t data;
    SOC_TMC_INGRESS_DRAM_BOUND_TYPE_E dram_thresh;
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E resource_type;
    uint32 alpha_field_value;
    int alpha_field_id;
 
    SOCDNX_INIT_FUNC_DEFS;
    
    SOCDNX_NULL_CHECK(info);

    if (rt_cls_ndx >  ARAD_ITM_QT_RT_CLS_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    SOCDNX_IF_ERR_EXIT(soc_mem_read(unit, CGM_VOQ_DRAM_BOUND_PRMSm, MEM_BLOCK_ANY, 
                                    rt_cls_ndx, data));
    for (resource_type = SOC_TMC_INGRESS_THRESHOLD_SRAM_BYTES; resource_type <= SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS; resource_type++) {
 
        for (dram_thresh = SOC_TMC_INGRESS_DRAM_BOUND; dram_thresh < SOC_TMC_INGRESS_DRAM_BOUND_NOF_TYPES; dram_thresh++) {
            SOC_TMC_ITM_DRAM_BOUND_THRESHOLD* dram_threshold = SOC_TMC_ITM_DRAM_BOUND_INFO_thresh_get(unit, info, dram_thresh, resource_type);
            itm_mantissa_exp_threshold_info mantissa_exp_info;

            SOCDNX_IF_ERR_EXIT(_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, QAX_DRAM_BOUND_THRESH_FREE_MAX, &mantissa_exp_info));
            _qax_itm_mantissa_exp_field_get(unit,&mantissa_exp_info, data,
                                                               &(dram_threshold->free_max_threshold));
        
            SOCDNX_IF_ERR_EXIT(_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, QAX_DRAM_BOUND_THRESH_FREE_MIN, &mantissa_exp_info));
            _qax_itm_mantissa_exp_field_get(unit,&mantissa_exp_info, data,
                                                               &(dram_threshold->free_min_threshold));

            SOCDNX_IF_ERR_EXIT(_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, QAX_DRAM_BOUND_THRESH_MAX, &mantissa_exp_info));
            _qax_itm_mantissa_exp_field_get(unit,&mantissa_exp_info, data,
                                                               &(dram_threshold->max_threshold));
        
            SOCDNX_IF_ERR_EXIT(_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, QAX_DRAM_BOUND_THRESH_MIN, &mantissa_exp_info));
            _qax_itm_mantissa_exp_field_get(unit,&mantissa_exp_info, data,
                                                               &(dram_threshold->min_threshold));


            alpha_field_id = qax_itm_dram_bound_alpha_field_get(unit, dram_thresh, resource_type);
            if (alpha_field_id == INVALIDf) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("can't find alpha field name\n")));
            }
            alpha_field_value = soc_mem_field32_get(unit, CGM_VOQ_DRAM_BOUND_PRMSm, &data, alpha_field_id);
            dram_threshold->alpha = _qax_itm_field_to_alpha(unit,alpha_field_value);
    
        }
    }

    _qax_itm_mantissa_exp_field_get(unit,&voq_dram_bound_qsize_recovery_th_mant_exp, data,
                                                       &info->qsize_recovery_th);

exit:
  SOCDNX_FUNC_RETURN;
}


/*****************/
/* VSQ functions */
/* {             */
/*****************/

/*!
 * \brief
 *  Map PP-port to a PG profile.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit - 
 *      The unit id.
 *  \param [in] core_id - 
 *      The core id.
 *  \param [in] src_pp_port -
 *      The In-PP-Port.
 *  \param [in] pg_tc_profile -
 *      The index of the PG profile.
 */
int
  qax_itm_vsq_pg_tc_profile_mapping_set(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int core_id,
    SOC_SAND_IN uint32 src_pp_port,
    SOC_SAND_IN int pg_tc_profile
  )
{
    int rv = SOC_E_NONE;
    uint32 data = 0;
    SOCDNX_INIT_FUNC_DEFS;

    if (core_id < 0 || core_id >= SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    rv = READ_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), src_pp_port, &data);
    SOCDNX_IF_ERR_EXIT(rv);

    soc_mem_field32_set(unit, CGM_IPPPMm, &data, TC_BITMAP_INDEXf, pg_tc_profile);

    rv = WRITE_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), src_pp_port, &data);
    SOCDNX_IF_ERR_EXIT(rv);

exit: 
    SOCDNX_FUNC_RETURN;
}

/*!
 * \brief
 *  Get the PG profile index of a port.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] core_id -
 *      The core id.
 *  \param [in] src_pp_port -
 *      The In_PP_Port.
 *  \param [out] pg_tc_profile -
 *      The resulting index of the PG profile.
 */
int
  qax_itm_vsq_pg_tc_profile_mapping_get(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int core_id,
    SOC_SAND_IN uint32 src_pp_port,
    SOC_SAND_OUT int *pg_tc_profile
  )
{
    int rv = SOC_E_NONE;
    uint32 data = 0;
    SOCDNX_INIT_FUNC_DEFS;

    if (core_id < 0 || core_id >= SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    rv = READ_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), src_pp_port, &data);
    SOCDNX_IF_ERR_EXIT(rv);

    *pg_tc_profile = soc_mem_field32_get(unit, CGM_IPPPMm, &data, TC_BITMAP_INDEXf);

exit: 
    SOCDNX_FUNC_RETURN;
}

/*!
 * \brief
 *  Map {In-PP-Port.TcBitmapIndex,TC} to PG-Offset. \n
 *  \n
 *  The table has 8 TC indexes, each holds 16 mapping profiles. \n
 *  \n
 *  pg_tc_bitmap holds mapping between TC and PG. \n
 *  Each TC have 3 bits in the bitmap. \n
 *  The value in each TC(3 bits) is the corresponding PG. \n
 *  \n
 *  Psaudo-code for what is done: \n
 *  PG-Offset[pg_tc_profile_id] = pg_tc_bitmap[TC]
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] core_id -
 *      The core id.
 *  \param [in] pg_tc_profile_id -
 *      The index to the table's bitmap.
 *  \param [in] pg_tc_bitmap - 
 *      the mapping bitmap to set.
 */
int
  qax_itm_vsq_pg_tc_profile_set(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int core_id,
    SOC_SAND_IN int pg_tc_profile_id,
    SOC_SAND_IN uint32 pg_tc_bitmap
  )
{
    int rv = SOC_E_NONE;
    SHR_BITDCLNAME(pg_tc_bitmap_data, QAX_ITM_VSQ_PG_OFFSET_FIELD_SIZE);
    int tc = 0;
    SOCDNX_INIT_FUNC_DEFS;

    /* bitmap validation */
    if (pg_tc_bitmap & ~((1 << SOC_TMC_NOF_TRAFFIC_CLASSES * 3) - 1)) {
        LOG_ERROR(BSL_LS_SOC_COSQ, 
                  (BSL_META_U(unit, "PG TC mapping bitmap is invalid %d, maximum bit nust be %d\n"), 
                   pg_tc_bitmap, SOC_TMC_NOF_TRAFFIC_CLASSES * 3));
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    for (tc = 0; tc < SOC_TMC_NOF_TRAFFIC_CLASSES; ++tc) {
         /*
          * Get PG-Offset at TC index in pg_tc_bitmap
          * and set the PG-Offset at pg_tc_profile_id index
          */
        SHR_BITCOPY_RANGE(pg_tc_bitmap_data, pg_tc_profile_id * 3, &pg_tc_bitmap, tc * 3, 3);
        rv = WRITE_CGM_VSQ_PG_TC_BITMAPm(unit, CGM_BLOCK(unit, SOC_CORE_ALL), tc, pg_tc_bitmap_data);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit: 
    SOCDNX_FUNC_RETURN;
}

/*!
 * \brief
 *  Get PG-offsets of a profile.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] core_id -
 *      The core id.
 *  \param [in] pg_tc_profile_id -
 *      The profile index.
 *  \param [out] pg_tc_bitmap -
 *      The result map of PG-offsets.
 */
int
  qax_itm_vsq_pg_tc_profile_get(
    SOC_SAND_IN int         unit,
    SOC_SAND_IN int         core_id,
    SOC_SAND_IN int         pg_tc_profile_id,
    SOC_SAND_OUT uint32     *pg_tc_bitmap
  )
{
    uint32 rv = SOC_E_NONE;
    SHR_BITDCLNAME(pg_tc_bitmap_data, QAX_ITM_VSQ_PG_OFFSET_FIELD_SIZE);
    uint32 pg_offset_bitmap = 0;
    int tc = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(pg_tc_bitmap);

    for (tc = 0; tc < SOC_TMC_NOF_TRAFFIC_CLASSES; ++tc) {
        rv = READ_CGM_VSQ_PG_TC_BITMAPm(unit, CGM_BLOCK(unit, core_id), tc, pg_tc_bitmap_data);
        SOCDNX_IF_ERR_EXIT(rv);

        SHR_BITCOPY_RANGE(&pg_offset_bitmap, tc * 3, pg_tc_bitmap_data, pg_tc_profile_id * 3, 3);
    }

    *pg_tc_bitmap = pg_offset_bitmap;

exit: 
    SOCDNX_FUNC_RETURN;
}

/*!
 * \brief
 *  Set Port-Base VSQ parameters.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] pg_ndx -
 *      Index of PG (VSQ-F).
 *  \param [in] pg_prm -
 *      PG parameters: lossless, use port guaranteed, pool id.
 */
int
  qax_itm_vsq_pb_prm_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32              pg_ndx,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_PG_PRM *pg_prm
  )
{
    int rv = SOC_E_NONE;
    uint32 data = 0;   
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(pg_prm);

    if (pg_ndx >= SOC_DPP_DEFS_GET(unit, nof_vsq_f)) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
   
    rv = READ_CGM_PB_VSQ_PRMSm(unit, CGM_BLOCK(unit, SOC_CORE_ALL), pg_ndx, &data);
    SOCDNX_IF_ERR_EXIT(rv);
    
    soc_mem_field32_set(unit, CGM_PB_VSQ_PRMSm, &data, IS_LOSSLESSf, pg_prm->is_lossles);
    soc_mem_field32_set(unit, CGM_PB_VSQ_PRMSm, &data, POOL_IDf, pg_prm->pool_id);
    soc_mem_field32_set(unit, CGM_PB_VSQ_PRMSm, &data, USE_PORT_GRNTDf, pg_prm->use_min_port);
    soc_mem_field32_set(unit, CGM_PB_VSQ_PRMSm, &data, ADMT_PROFILEf, pg_prm->admit_profile);

    rv = WRITE_CGM_PB_VSQ_PRMSm(unit, CGM_BLOCK(unit, SOC_CORE_ALL), pg_ndx, &data);
    SOCDNX_IF_ERR_EXIT(rv);

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Get Port-Base VSQ parameters.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] pg_ndx -
 *      Index of PG (VSQ-F).
 *  \param [out] pg_prm -
 *      PG parameters: lossless, use port guaranteed, pool id.
 */
int
  qax_itm_vsq_pb_prm_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32              pg_ndx,
    SOC_SAND_OUT SOC_TMC_ITM_VSQ_PG_PRM *pg_prm
  )
{
    int rv = SOC_E_NONE;
    uint32 data = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(pg_prm);

    if (pg_ndx >= SOC_DPP_DEFS_GET(unit, nof_vsq_f)) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
   
    rv = READ_CGM_PB_VSQ_PRMSm(unit, CGM_BLOCK(unit, SOC_CORE_ALL), pg_ndx, &data);
    SOCDNX_IF_ERR_EXIT(rv);
    
    pg_prm->is_lossles = soc_mem_field32_get(unit, CGM_PB_VSQ_PRMSm, &data, IS_LOSSLESSf);
    pg_prm->pool_id = soc_mem_field32_get(unit, CGM_PB_VSQ_PRMSm, &data, POOL_IDf);
    pg_prm->use_min_port = soc_mem_field32_get(unit, CGM_PB_VSQ_PRMSm, &data, USE_PORT_GRNTDf);
    pg_prm->admit_profile = soc_mem_field32_get(unit, CGM_PB_VSQ_PRMSm, &data, ADMT_PROFILEf);

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Map PP-port to VSQ-E index and PG base.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] core_id -
 *      The core id.
 *  \param [in] src_pp_port -
 *      PP-port.
 *  \param [in] src_port_vsq_index -
 *      VSQ-E index.
 *  \param [in] pg_base -
 *      PG-Base index.
 *  \param [in] enable -
 *      1/0 to enable/disable the mapping.
 */
int
  qax_itm_src_vsqs_mapping_set(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int core_id,
    SOC_SAND_IN int src_pp_port,
    SOC_SAND_IN int src_port_vsq_index,
    SOC_SAND_IN int pg_base,
    SOC_SAND_IN uint8 enable
  )
{
    int rv = SOC_E_NONE, data = 0;
    SOCDNX_INIT_FUNC_DEFS;

    if (core_id < 0 || core_id >= SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (src_pp_port < -1 || src_pp_port > SOC_MAX_NUM_PORTS) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (src_port_vsq_index < 0 || src_port_vsq_index >= SOC_DPP_DEFS_GET(unit, nof_vsq_e)) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (pg_base < 0 || pg_base >= SOC_DPP_DEFS_GET(unit, nof_vsq_f)) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    rv = READ_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), src_pp_port, &data);
    SOCDNX_IF_ERR_EXIT(rv);

    /*VSQE mapping*/
    /*In-PP-Port => VSQ-E index */
    soc_mem_field32_set(unit, CGM_IPPPMm, &data, NIF_PORTf, enable ? src_port_vsq_index : 0);

    /*VSQF mapping*/
    /*In-PP-Port(8) => { PG-Base(9), PG-MAP-Profile-Bitmap-Index(4)}*/
    soc_mem_field32_set(unit, CGM_IPPPMm, &data, PG_BASEf, enable ? pg_base : 0);

    rv = WRITE_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), src_pp_port, &data);
    SOCDNX_IF_ERR_EXIT(rv);

exit: 
    SOCDNX_FUNC_RETURN;
}

/*!
 * \brief
 *  Get VSQ-E index and PG-Base corresponding to PP-port.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] core_id -
 *      The core id.
 *  \param [in] src_pp_port -
 *      PP-port.
 *  \param [out] src_port_vsq_index -
 *      VSQ-E index.
 *  \param [out] pg_base -
 *      PG-Base index.
 *  \param [out] enable -
 *      Whether the mapping is enabled/disabled.
 */
int
  qax_itm_src_vsqs_mapping_get(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  int core_id,
    SOC_SAND_IN  int src_pp_port,
    SOC_SAND_OUT int *src_port_vsq_index,
    SOC_SAND_OUT int *pg_base,
    SOC_SAND_OUT uint8 *enable
  )
{
    int rv = SOC_E_NONE, data = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(src_port_vsq_index);
    SOCDNX_NULL_CHECK(pg_base);
    SOCDNX_NULL_CHECK(enable);

    if (core_id < 0 || core_id >= SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (src_pp_port < 0 || src_pp_port >= SOC_MAX_NUM_PORTS) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    rv = READ_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), src_pp_port, &data);
    SOCDNX_IF_ERR_EXIT(rv);

    /*VSQE mapping*/
    *src_port_vsq_index = soc_mem_field32_get(unit, CGM_IPPPMm, &data, NIF_PORTf);
    /*VSQF mapping*/
    *pg_base = soc_mem_field32_get(unit, CGM_IPPPMm, &data, PG_BASEf);

    *enable = 0;
    if (!(*src_port_vsq_index == 0 && *pg_base == 0)) {
        *enable = TRUE;
    }
exit: 
    SOCDNX_FUNC_RETURN;
}

/*!
 * \brief
 *  Get VSQ's WRED information.
 *
 *  \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] vsq_group_ndx -
 *      The VSQ type.
 *  \param [in] vsq_rt_cls_ndx -
 *      The VSQ rate class.
 *  \param [in] drop_precedence_ndx -
 *      The DP (drop precedence).
 *  \param [in] pool_id -
 *      The pool id.
 *  \param [out] info -
 *      WRED information.
 */
int
    qax_itm_vsq_wred_get(
        SOC_SAND_IN  int                    unit,
        SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
        SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
        SOC_SAND_IN  uint32                 drop_precedence_ndx,
        SOC_SAND_IN  int                    pool_id,
        SOC_SAND_OUT SOC_TMC_ITM_WRED_QT_DP_INFO *info) 
{
    int res = SOC_E_NONE;
    uint32 entry_offset = 0;
    soc_reg_above_64_val_t data;
    const soc_mem_t mem_arr_CGM_VSQ_WRED_RJCT_PRMS[SOC_TMC_NOF_VSQ_GROUPS] = {CGM_VSQA_WRED_RJCT_PRMSm, CGM_VSQB_WRED_RJCT_PRMSm, CGM_VSQC_WRED_RJCT_PRMSm, CGM_VSQD_WRED_RJCT_PRMSm, CGM_VSQE_WRED_RJCT_PRMSm, CGM_VSQF_WRED_RJCT_PRMSm};
    itm_mantissa_exp_threshold_info wred_man_exp_info;
    uint32 min_avrg_th = 0, max_avrg_th = 0;
    uint32 c1 = 0, c2 = 0, c3 = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);
    sal_memset(&wred_man_exp_info, 0x0, sizeof(itm_mantissa_exp_threshold_info));

    if (vsq_group_ndx < 0 || vsq_group_ndx >= SOC_TMC_NOF_VSQ_GROUPS) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    } 
    if (vsq_group_ndx == SOC_TMC_ITM_VSQ_GROUP_CTGRY) {
        if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_A_QT_RT_CLS_MAX(unit)) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
    } else {
        if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }
    if (drop_precedence_ndx >= SOC_TMC_NOF_DROP_PRECEDENCE) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (pool_id < 0 || pool_id >= SOC_TMC_ITM_NOF_RSRC_POOLS) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    entry_offset = (vsq_rt_cls_ndx * SOC_TMC_NOF_DROP_PRECEDENCE) + drop_precedence_ndx;
    if (vsq_group_ndx == SOC_TMC_ITM_VSQ_GROUP_SRC_PORT) {
        /* VSQ-E table is doubled. The first half is for pool 0 and the second half is for pool 1 */
        entry_offset += pool_id * (SOC_TMC_ITM_VSQ_NOF_RATE_CLASSES(unit) * SOC_TMC_NOF_DROP_PRECEDENCE);
    }

    res = soc_mem_read(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    SOCDNX_IF_ERR_EXIT(res); 

    wred_man_exp_info.mem_id = mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx];
    wred_man_exp_info.mantissa_bits = CGM_ITM_NOF_MANTISSA_BITS;
    wred_man_exp_info.factor = SOC_TMC_ITM_WRED_GRANULARITY;

    /* Min average threshold */
    wred_man_exp_info.field_id = WRED_MIN_THf;
    wred_man_exp_info.exp_bits = soc_mem_field_length(unit, wred_man_exp_info.mem_id, wred_man_exp_info.field_id) - wred_man_exp_info.mantissa_bits;
    _qax_itm_mantissa_exp_field_get(unit, &wred_man_exp_info, data, &min_avrg_th);

    /* Max average threshold */
    wred_man_exp_info.field_id = WRED_MAX_THf;
    wred_man_exp_info.exp_bits = soc_mem_field_length(unit, wred_man_exp_info.mem_id, wred_man_exp_info.field_id) - wred_man_exp_info.mantissa_bits;
    _qax_itm_mantissa_exp_field_get(unit, &wred_man_exp_info, data, &max_avrg_th);

    info->wred_en = SOC_SAND_NUM2BOOL(soc_mem_field32_get(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_ENf));
    info->ignore_packet_size = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_IGNR_PKT_SIZEf);   
    c1 = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_C_1f);
    c2 = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_C_2f);
    c3 = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_C_3f);
    info->min_avrg_th = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_MIN_THf);
    info->max_avrg_th = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_MAX_THf);

    _qax_itm_hw_data_to_wred_info (unit, min_avrg_th, max_avrg_th, c1, c2, c3, info);

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Set VSQ's WRED information.
 *  WRED's drop-probability is computed the following way:
 *  Drop-prob = Max-p/(MaxTh-MinTh)*(AvgSize-MinTh)*PktSize/MaxPktSize
 *            = ((2^C1)*AvgSize - C2) * (PktSize / (2^C3))
 *
 *  \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] vsq_group_ndx -
 *      The VSQ type.
 *  \param [in] vsq_rt_cls_ndx -
 *      The VSQ rate class.
 *  \param [in] drop_precedence_ndx -
 *      The DP (drop precedence).
 *  \param [in] pool_id -
 *      The pool id.
 *  \param [in] info -
 *      WRED information.
 *  \param [in] exact_info -
 *      The exact WRED information that was written.
 */
int
    qax_itm_vsq_wred_set(
        SOC_SAND_IN  int                    unit,
        SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
        SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
        SOC_SAND_IN  uint32                 drop_precedence_ndx,
        SOC_SAND_IN  int                    pool_id,
        SOC_SAND_IN  SOC_TMC_ITM_WRED_QT_DP_INFO *info,
        SOC_SAND_OUT SOC_TMC_ITM_WRED_QT_DP_INFO *exact_info)
{
    int res = SOC_E_NONE;
    uint32 entry_offset = 0;
    soc_reg_above_64_val_t data;
    const soc_mem_t mem_arr_CGM_VSQ_WRED_RJCT_PRMS[SOC_TMC_NOF_VSQ_GROUPS] = {CGM_VSQA_WRED_RJCT_PRMSm, CGM_VSQB_WRED_RJCT_PRMSm, CGM_VSQC_WRED_RJCT_PRMSm, CGM_VSQD_WRED_RJCT_PRMSm, CGM_VSQE_WRED_RJCT_PRMSm, CGM_VSQF_WRED_RJCT_PRMSm};
    itm_mantissa_exp_threshold_info wred_man_exp_info;
    uint32 exact_min_avrg_th = 0, exact_max_avrg_th = 0;
    uint32 c1 = 0, c2 = 0, c3 = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);
    sal_memset(&wred_man_exp_info, 0x0, sizeof(itm_mantissa_exp_threshold_info));

    if (vsq_group_ndx < 0 || vsq_group_ndx >= SOC_TMC_NOF_VSQ_GROUPS) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    } 
    if (vsq_group_ndx == SOC_TMC_ITM_VSQ_GROUP_CTGRY) {
        if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_A_QT_RT_CLS_MAX(unit)) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
    } else {
        if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }
    if (drop_precedence_ndx >= SOC_TMC_NOF_DROP_PRECEDENCE) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (pool_id < 0 || pool_id >= SOC_TMC_ITM_NOF_RSRC_POOLS) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    entry_offset = (vsq_rt_cls_ndx * SOC_TMC_NOF_DROP_PRECEDENCE) + drop_precedence_ndx;
    if (vsq_group_ndx == SOC_TMC_ITM_VSQ_GROUP_SRC_PORT) {
        /* VSQ-E table is doubled. The first half is for pool 0 and the second half is for pool 1 */
        entry_offset += pool_id * (SOC_TMC_ITM_VSQ_NOF_RATE_CLASSES(unit) * SOC_TMC_NOF_DROP_PRECEDENCE);
    }

    res = soc_mem_read(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    SOCDNX_IF_ERR_EXIT(res); 

    wred_man_exp_info.mem_id = mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx];
    wred_man_exp_info.mantissa_bits = CGM_ITM_NOF_MANTISSA_BITS;
    wred_man_exp_info.factor = SOC_TMC_ITM_WRED_GRANULARITY;

    /* Min average threshold */
    wred_man_exp_info.field_id = WRED_MIN_THf;
    wred_man_exp_info.exp_bits = soc_mem_field_length(unit, wred_man_exp_info.mem_id, wred_man_exp_info.field_id) - wred_man_exp_info.mantissa_bits;
    res = _qax_itm_mantissa_exp_field_set(unit, &wred_man_exp_info, 1, data, info->min_avrg_th, &exact_min_avrg_th);
    SOCDNX_IF_ERR_EXIT(res);

    /* Max average threshold */
    wred_man_exp_info.field_id = WRED_MAX_THf;
    wred_man_exp_info.exp_bits = soc_mem_field_length(unit, wred_man_exp_info.mem_id, wred_man_exp_info.field_id) - wred_man_exp_info.mantissa_bits;
    res = _qax_itm_mantissa_exp_field_set(unit, &wred_man_exp_info, 1, data, info->max_avrg_th, &exact_max_avrg_th);
    SOCDNX_IF_ERR_EXIT(res);

    _qax_itm_wred_info_to_hw_data(unit, info, exact_min_avrg_th, exact_max_avrg_th, &c1, &c2, &c3);

    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_ENf, SOC_SAND_NUM2BOOL(info->wred_en));
    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_IGNR_PKT_SIZEf, SOC_SAND_BOOL2NUM(info->ignore_packet_size));   
    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_C_1f, c1);
    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_C_2f, c2);
    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_C_3f, c3);

    res = soc_mem_write(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    SOCDNX_IF_ERR_EXIT(res); 

    _qax_itm_hw_data_to_wred_info(unit, exact_min_avrg_th, exact_max_avrg_th, c1, c2, c3, exact_info);
    exact_info->wred_en = SOC_SAND_BOOL2NUM(info->wred_en);
    exact_info->ignore_packet_size = SOC_SAND_BOOL2NUM(info->ignore_packet_size);

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Set Average parameters for VSQ WRED.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] vsq_group_ndx -
 *      VSQ type (A-F).
 *  \param [in] vsq_rt_cls_ndx -
 *      The VSQ rate-class (profile).
 *  \param [in] pool_id -
 *      Not used (legacy).
 *  \param [in] info -
 *      The average parameters.
 */
int
  qax_itm_vsq_wred_gen_set(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP          vsq_group_ndx,
    SOC_SAND_IN  uint32                         vsq_rt_cls_ndx,
    SOC_SAND_IN  int                            pool_id,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_WRED_GEN_INFO  *info
  )
{
    int res = SOC_E_NONE;
    uint32 data;
    const soc_mem_t mem_arr_CGM_VSQ_AVRG_PRMS[SOC_TMC_NOF_VSQ_GROUPS] = 
        {CGM_VSQA_AVRG_PRMSm, CGM_VSQB_AVRG_PRMSm, CGM_VSQC_AVRG_PRMSm, CGM_VSQD_AVRG_PRMSm, CGM_VSQE_AVRG_PRMSm, CGM_VSQF_AVRG_PRMSm};
    SOCDNX_INIT_FUNC_DEFS;

    if (vsq_group_ndx >= SOC_TMC_NOF_VSQ_GROUPS) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (vsq_group_ndx == SOC_TMC_ITM_VSQ_GROUP_CTGRY) {
        if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_A_QT_RT_CLS_MAX(unit)) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
    } else {
        if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }   

    if (info->exp_wq >= (0x1 << soc_mem_field_length(unit, CGM_VSQA_AVRG_PRMSm, AVRG_WEIGHTf))) {
      SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    res = soc_mem_read(unit, mem_arr_CGM_VSQ_AVRG_PRMS[vsq_group_ndx], CGM_BLOCK(unit, SOC_CORE_ALL), vsq_rt_cls_ndx, &data);
    SOCDNX_IF_ERR_EXIT(res);
    
    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_AVRG_PRMS[vsq_group_ndx], &data, AVRG_UPDT_ENf, info->wred_en);
    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_AVRG_PRMS[vsq_group_ndx], &data, AVRG_WEIGHTf, info->exp_wq);

    res = soc_mem_write(unit, mem_arr_CGM_VSQ_AVRG_PRMS[vsq_group_ndx], CGM_BLOCK(unit, SOC_CORE_ALL), vsq_rt_cls_ndx, &data);
    SOCDNX_IF_ERR_EXIT(res);

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Get Average parameters for VSQ WRED.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] vsq_group_ndx -
 *      VSQ type (A-F).
 *  \param [in] vsq_rt_cls_ndx -
 *      The VSQ rate-class (profile).
 *  \param [in] pool_id -
 *      Not used (legacy).
 *  \param [out] info -
 *      The average parameters.
 */
int
  qax_itm_vsq_wred_gen_get(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP          vsq_group_ndx,
    SOC_SAND_IN  uint32                         vsq_rt_cls_ndx,
    SOC_SAND_IN  int                            pool_id,
    SOC_SAND_OUT SOC_TMC_ITM_VSQ_WRED_GEN_INFO  *info
  )
{
    int res = SOC_E_NONE;
    uint32 data;
    const soc_mem_t mem_arr_CGM_VSQ_AVRG_PRMS[SOC_TMC_NOF_VSQ_GROUPS] = 
        {CGM_VSQA_AVRG_PRMSm, CGM_VSQB_AVRG_PRMSm, CGM_VSQC_AVRG_PRMSm, CGM_VSQD_AVRG_PRMSm, CGM_VSQE_AVRG_PRMSm, CGM_VSQF_AVRG_PRMSm};
    SOCDNX_INIT_FUNC_DEFS;

    if (vsq_group_ndx >= SOC_TMC_NOF_VSQ_GROUPS) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (vsq_group_ndx == SOC_TMC_ITM_VSQ_GROUP_CTGRY) {
        if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_A_QT_RT_CLS_MAX(unit)) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
    } else {
        if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }   

    res = soc_mem_read(unit, mem_arr_CGM_VSQ_AVRG_PRMS[vsq_group_ndx], CGM_BLOCK(unit, SOC_CORE_ALL), vsq_rt_cls_ndx, &data);
    SOCDNX_IF_ERR_EXIT(res);

    info->wred_en = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_AVRG_PRMS[vsq_group_ndx], &data, AVRG_UPDT_ENf);
    info->exp_wq = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_AVRG_PRMS[vsq_group_ndx], &data, AVRG_WEIGHTf);

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Set taildrop for Destination-based-VSQs (VSQ-A/B/C/D). \n
 *  \n
 *  The taildrop is set per resource type (Words, SRAM-Buffers, SRAM-PDs) per DP.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] vsq_group_ndx -
 *      The VSQ type.
 *  \param [in] vsq_rt_cls_ndx -
 *      VSQ's rate class. Used as index in the table.
 *  \param [in] drop_precedence_ndx -
 *      The DP (drop-precedence) to set.
 *  \param [in] pool_id -
 *      Not used (src-based VSQs configured in another function).
 *  \param [in] is_headroom -
 *      Not used (src-based VSQs configured in another function).
 *  \param [in] info -
 *      Tail drop thresholds per resource type.
 *  \param [out] exact_info -
 *      Return exact tail drop thresholds.
 */
int
  qax_itm_vsq_tail_drop_set(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  int                    pool_id,
    SOC_SAND_IN  int                    is_headroom,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_TAIL_DROP_INFO  *info,
    SOC_SAND_OUT SOC_TMC_ITM_VSQ_TAIL_DROP_INFO  *exact_info
  )
{
    int rv = SOC_E_NONE;
    uint32 entry_offset;
    const soc_mem_t mem_arr_CGM_VSQ_RJCT_PRMS[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES][SOC_TMC_NOF_NON_SRC_BASED_VSQ_GROUPS] = 
       {{CGM_VSQA_WORDS_RJCT_PRMSm, CGM_VSQB_WORDS_RJCT_PRMSm, CGM_VSQC_WORDS_RJCT_PRMSm, CGM_VSQD_WORDS_RJCT_PRMSm, },
        {CGM_VSQA_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQB_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQC_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQD_SRAM_BUFFERS_RJCT_PRMSm,},
        {CGM_VSQA_SRAM_PDS_RJCT_PRMSm, CGM_VSQB_SRAM_PDS_RJCT_PRMSm, CGM_VSQC_SRAM_PDS_RJCT_PRMSm, CGM_VSQD_SRAM_PDS_RJCT_PRMSm}};
    const soc_field_t field_arr_MAX_SIZE_TH_DP[SOC_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_SIZE_TH_DP_0f, MAX_SIZE_TH_DP_1f, MAX_SIZE_TH_DP_2f, MAX_SIZE_TH_DP_3f};
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = SOC_TMC_INGRESS_THRESHOLD_INVALID;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);
    SOCDNX_NULL_CHECK(exact_info);

    rv = qax_itm_vsq_tail_drop_default_get(unit, exact_info);
    SOCDNX_SAND_IF_ERR_EXIT(rv);

    if (vsq_group_ndx < 0 || vsq_group_ndx >= SOC_TMC_NOF_NON_SRC_BASED_VSQ_GROUPS) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    } 
    if (vsq_group_ndx == SOC_TMC_ITM_VSQ_GROUP_CTGRY) {
        if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_A_QT_RT_CLS_MAX(unit)) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
    } else {
        if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }   
    if (drop_precedence_ndx >= SOC_TMC_NOF_DROP_PRECEDENCE) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    entry_offset = vsq_rt_cls_ndx;

    for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type][vsq_group_ndx], 
                field_arr_MAX_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                info->max_inst_q_size_th[rsrc_type], 
                &exact_info->max_inst_q_size_th[rsrc_type]);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*
 * Set maximal thresholds to VSQ Flow Control
 */
int
  qax_itm_vsq_fc_default_get(
      SOC_SAND_IN  int                 unit,
      SOC_SAND_OUT SOC_TMC_ITM_VSQ_FC_INFO  *info
  )
{
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    /* Disable VSQ A-E Flow Control (set to max) */
    info->size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set = CGM_ITM_VSQ_WORDS_SIZE_MAX(unit) * CGM_ITM_VSQ_WORDS_RESOLUTION;
    info->size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].clear = info->size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set;

    info->size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set = CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX(unit);
    info->size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].clear = info->size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set;

    info->size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set = CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX;
    info->size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].clear = info->size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set;

    /* Disable VSQ-F Flow Control */
    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set.max_threshold = CGM_ITM_VSQ_WORDS_SIZE_MAX(unit) * CGM_ITM_VSQ_WORDS_RESOLUTION;
    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set.min_threshold = info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set.max_threshold;
    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set.alpha = 0;
    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].clear_offset = 0;

    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set.max_threshold = CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX(unit);
    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set.min_threshold = info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set.max_threshold;
    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set.alpha = 0;
    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].clear_offset = 0;

    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set.max_threshold = CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX;
    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set.min_threshold = info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set.max_threshold;
    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set.alpha = 0;
    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].clear_offset = 0;

exit:
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Return default tail drop information. \n
 *  \n
 *   Set taildrop thresholds to maximum size. \n
 *   Zero SOC_TMC_ITM_VSQ_TAIL_DROP_INFO params that are not used.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [out] info -
 *      Default tail drop information.
 */
int
  qax_itm_vsq_tail_drop_default_get(
      SOC_SAND_IN  int                 unit,
      SOC_SAND_OUT SOC_TMC_ITM_VSQ_TAIL_DROP_INFO  *info
  )
{
    uint32 max_size_exp = 0, max_size_mnt = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    /* Zero non-relevant params */
    /* Jericho2 TBD: Remove this line */
    info->max_inst_q_size = info->max_inst_q_size_bds = info->alpha = 0;

    /* Set default taildrop (max size) */
    arad_iqm_mantissa_exponent_get(unit, (0x1 << soc_mem_field_length(unit, CGM_VSQA_WORDS_RJCT_PRMSm, MAX_SIZE_TH_DP_0f)) - 1, CGM_ITM_NOF_MANTISSA_BITS, &max_size_mnt, &max_size_exp);
    info->max_inst_q_size_th[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES] = (max_size_mnt * (0x1 << max_size_exp));
    arad_iqm_mantissa_exponent_get(unit, (0x1 << soc_mem_field_length(unit, CGM_VSQA_SRAM_BUFFERS_RJCT_PRMSm, MAX_SIZE_TH_DP_0f)) - 1, CGM_ITM_NOF_MANTISSA_BITS, &max_size_mnt, &max_size_exp);
    info->max_inst_q_size_th[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS] = (max_size_mnt * (0x1 << max_size_exp));
    arad_iqm_mantissa_exponent_get(unit, (0x1 << soc_mem_field_length(unit, CGM_VSQA_SRAM_PDS_RJCT_PRMSm, MAX_SIZE_TH_DP_0f)) - 1, CGM_ITM_NOF_MANTISSA_BITS, &max_size_mnt, &max_size_exp);
    info->max_inst_q_size_th[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS] = (max_size_mnt * (0x1 << max_size_exp));

exit:
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Get taildrop for Destination-based-VSQs (VSQ-A/B/C/D). \n
 *  \n
 *  The taildrop is per resource type (Words, SRAM-Buffers, SRAM-PDs) per DP.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] vsq_group_ndx -
 *      The VSQ type.
 *  \param [in] vsq_rt_cls_ndx -
 *      VSQ's rate class. Used as index in the table.
 *  \param [in] drop_precedence_ndx -
 *      The DP (drop-precedence) to set.
 *  \param [in] pool_id -
 *      Not used (src-based VSQs configured in another function).
 *  \param [in] is_headroom -
 *      Not used (src-based VSQs configured in another function).
 *  \param [out] info -
 *      The returned Tail drop thresholds per resource type.
 */
int
  qax_itm_vsq_tail_drop_get(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  int                    pool_id,
    SOC_SAND_IN  int                    is_headroom,
    SOC_SAND_OUT SOC_TMC_ITM_VSQ_TAIL_DROP_INFO  *info
  )
{
    int rv = SOC_E_NONE;
    uint32 entry_offset;
    const soc_mem_t mem_arr_CGM_VSQ_RJCT_PRMS[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES][SOC_TMC_NOF_NON_SRC_BASED_VSQ_GROUPS] = 
       {{CGM_VSQA_WORDS_RJCT_PRMSm, CGM_VSQB_WORDS_RJCT_PRMSm, CGM_VSQC_WORDS_RJCT_PRMSm, CGM_VSQD_WORDS_RJCT_PRMSm, },
        {CGM_VSQA_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQB_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQC_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQD_SRAM_BUFFERS_RJCT_PRMSm,},
        {CGM_VSQA_SRAM_PDS_RJCT_PRMSm, CGM_VSQB_SRAM_PDS_RJCT_PRMSm, CGM_VSQC_SRAM_PDS_RJCT_PRMSm, CGM_VSQD_SRAM_PDS_RJCT_PRMSm}};
    const soc_field_t field_arr_MAX_SIZE_TH_DP[SOC_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_SIZE_TH_DP_0f, MAX_SIZE_TH_DP_1f, MAX_SIZE_TH_DP_2f, MAX_SIZE_TH_DP_3f};
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = SOC_TMC_INGRESS_THRESHOLD_INVALID;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    rv = qax_itm_vsq_tail_drop_default_get(unit, info);
    SOCDNX_SAND_IF_ERR_EXIT(rv);

    if (vsq_group_ndx < 0 || vsq_group_ndx >= SOC_TMC_NOF_NON_SRC_BASED_VSQ_GROUPS) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    } 
    if (vsq_group_ndx == SOC_TMC_ITM_VSQ_GROUP_CTGRY) {
        if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_A_QT_RT_CLS_MAX(unit)) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
    } else {
        if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }   
    if (drop_precedence_ndx >= SOC_TMC_NOF_DROP_PRECEDENCE) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    entry_offset = vsq_rt_cls_ndx;

    for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type][vsq_group_ndx], 
                field_arr_MAX_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                &info->max_inst_q_size_th[rsrc_type]);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Set Src-Port-VSQ (VSQ-E) resource allocation params \n
 *  (Guaranteed, Shared and Headroom). \n
 *  \n
 *  The function update all resource types (Words, SRAM-Buffers, SRAM-PDs) for \n
 *  specific rate class with specific DP.
 * 
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] vsq_rt_cls_ndx -
 *      The VSQ's rate class to update.
 *  \param [in] drop_precedence_ndx -
 *      The DP (drop-precedence) to update.
 *  \param [in] pool_id -
 *      Pool to update (0/1).
 *  \param [in] info -
 *      The resource allocation information to set.
 *  \param [out] exact_info -
 *      The exact resource allocation params that were written.
 */
int
  qax_itm_vsq_src_port_rjct_set(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  int                    pool_id,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_SRC_PORT_INFO       *info,
    SOC_SAND_OUT SOC_TMC_ITM_VSQ_SRC_PORT_INFO       *exact_info
  )
{
    uint32 rv = SOC_E_NONE;
    uint32 entry_offset = vsq_rt_cls_ndx;
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = SOC_TMC_INGRESS_THRESHOLD_INVALID;
    const soc_mem_t mem_arr_CGM_VSQ_RJCT_PRMS[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        {CGM_VSQE_WORDS_RJCT_PRMSm, CGM_VSQE_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQE_SRAM_PDS_RJCT_PRMSm};
    const soc_field_t field_arr_MAX_GRNTD_SIZE_TH_DP[SOC_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_GRNTD_SIZE_TH_DP_0f, MAX_GRNTD_SIZE_TH_DP_1f, MAX_GRNTD_SIZE_TH_DP_2f, MAX_GRNTD_SIZE_TH_DP_3f};
    const soc_field_t field_arr_MAX_SHRD_SIZE_TH_DP[SOC_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_SHRD_SIZE_TH_DP_0f, MAX_SHRD_SIZE_TH_DP_1f, MAX_SHRD_SIZE_TH_DP_2f, MAX_SHRD_SIZE_TH_DP_3f};
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);
    SOCDNX_NULL_CHECK(exact_info);

    if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (drop_precedence_ndx >= SOC_TMC_NOF_DROP_PRECEDENCE) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (pool_id < 0 || pool_id >= SOC_TMC_ITM_NOF_RSRC_POOLS) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    /* VSQ-E table is doubled. The first half is for pool 0 and the second half is for pool 1 */
    entry_offset += pool_id * SOC_TMC_ITM_VSQ_NOF_RATE_CLASSES(unit);

    for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        /* Set Guaranteed */
        rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                field_arr_MAX_GRNTD_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                info->max_guaranteed[rsrc_type], 
                &exact_info->max_guaranteed[rsrc_type]);
        SOCDNX_IF_ERR_EXIT(rv);

        /* Set Shared Pool */
        rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                field_arr_MAX_SHRD_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                info->max_shared[rsrc_type], 
                &exact_info->max_shared[rsrc_type]);
        SOCDNX_IF_ERR_EXIT(rv);

        /* Set Headroom */
        rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                MAX_HDRM_SIZE_THf, 
                entry_offset,
                info->max_headroom[rsrc_type], 
                &exact_info->max_headroom[rsrc_type]);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Get Src-Port-VSQ (VSQ-E) resource allocation params \n
 *  (Guaranteed, Shared and Headroom). \n
 *  \n
 *  The function read all resource types (Words, SRAM-Buffers, SRAM-PDs) for \n
 *  specific rate class with specific DP.
 * 
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] vsq_rt_cls_ndx -
 *      The VSQ's rate class to update.
 *  \param [in] drop_precedence_ndx -
 *      The DP (drop-precedence) to update.
 *  \param [in] pool_id -
 *      Pool to update (0/1).
 *  \param [out] info -
 *      The returned resource allocation params.
 */
int
  qax_itm_vsq_src_port_rjct_get(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  int                    pool_id,
    SOC_SAND_OUT SOC_TMC_ITM_VSQ_SRC_PORT_INFO       *info
  )
{
    uint32 rv = SOC_E_NONE;
    uint32 entry_offset = vsq_rt_cls_ndx;
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = SOC_TMC_INGRESS_THRESHOLD_INVALID;
    const soc_mem_t mem_arr_CGM_VSQ_RJCT_PRMS[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        {CGM_VSQE_WORDS_RJCT_PRMSm, CGM_VSQE_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQE_SRAM_PDS_RJCT_PRMSm};
    const soc_field_t field_arr_MAX_GRNTD_SIZE_TH_DP[SOC_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_GRNTD_SIZE_TH_DP_0f, MAX_GRNTD_SIZE_TH_DP_1f, MAX_GRNTD_SIZE_TH_DP_2f, MAX_GRNTD_SIZE_TH_DP_3f};
    const soc_field_t field_arr_MAX_SHRD_SIZE_TH_DP[SOC_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_SHRD_SIZE_TH_DP_0f, MAX_SHRD_SIZE_TH_DP_1f, MAX_SHRD_SIZE_TH_DP_2f, MAX_SHRD_SIZE_TH_DP_3f};
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (drop_precedence_ndx >= SOC_TMC_NOF_DROP_PRECEDENCE) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (pool_id < 0 || pool_id >= SOC_TMC_ITM_NOF_RSRC_POOLS) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    /* VSQ-E table is doubled. The first half is for pool 0 and the second half is for pool 1 */
    entry_offset += pool_id * SOC_TMC_ITM_VSQ_NOF_RATE_CLASSES(unit);

    for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        /* Set Guaranteed */
        rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                field_arr_MAX_GRNTD_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                &info->max_guaranteed[rsrc_type]);
        SOCDNX_IF_ERR_EXIT(rv);

        /* Set Shared Pool */
        rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                field_arr_MAX_SHRD_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                &info->max_shared[rsrc_type]);
        SOCDNX_IF_ERR_EXIT(rv);

        /* Set Headroom */
        rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                MAX_HDRM_SIZE_THf, 
                entry_offset,
                &info->max_headroom[rsrc_type]);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Set PG-VSQ (VSQ-F) resource allocation params \n
 *  (Guaranteed, Shared and Headroom). \n
 *  \n
 *  The function update all resource types (Words, SRAM-Buffers, SRAM-PDs) for \n
 *  specific rate class with specific DP.
 * 
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] vsq_rt_cls_ndx -
 *      The VSQ's rate class to update.
 *  \param [in] drop_precedence_ndx -
 *      The DP (drop-precedence) to update.
 *  \param [in] info -
 *      The resource allocation information to set.
 *  \param [out] exact_info -
 *      The exact resource allocation information that were written.
 *
 *  \remark
 *      1) PG-VSQ shared size is defined by FADT. \n
 *      2) PG-VSQ SRAM-Headroom have also max-headroom-nominal \n
 *         and max_headroom-extension.
 */
int
  qax_itm_vsq_pg_rjct_set(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                     vsq_rt_cls_ndx,
    SOC_SAND_IN  uint32                     drop_precedence_ndx,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_PG_INFO    *info,
    SOC_SAND_OUT SOC_TMC_ITM_VSQ_PG_INFO    *exact_info
  )
{
    uint32 rv = SOC_E_NONE;
    uint32 entry_offset = vsq_rt_cls_ndx;
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = SOC_TMC_INGRESS_THRESHOLD_INVALID;
    const soc_mem_t mem_arr_CGM_VSQ_RJCT_PRMS[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        {CGM_VSQF_WORDS_RJCT_PRMSm, CGM_VSQF_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQF_SRAM_PDS_RJCT_PRMSm};
    const soc_field_t field_arr_MAX_GRNTD_SIZE_TH_DP[SOC_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_GRNTD_SIZE_TH_DP_0f, MAX_GRNTD_SIZE_TH_DP_1f, MAX_GRNTD_SIZE_TH_DP_2f, MAX_GRNTD_SIZE_TH_DP_3f};
    const soc_field_t field_arr_SHRD_SIZE_FADT_MAX_TH_DP[SOC_TMC_NOF_DROP_PRECEDENCE] = 
        {SHRD_SIZE_FADT_MAX_TH_DP_0f, SHRD_SIZE_FADT_MAX_TH_DP_1f, SHRD_SIZE_FADT_MAX_TH_DP_2f, SHRD_SIZE_FADT_MAX_TH_DP_3f};
    const soc_field_t field_arr_SHRD_SIZE_FADT_MIN_TH_DP[SOC_TMC_NOF_DROP_PRECEDENCE] = 
        {SHRD_SIZE_FADT_MIN_TH_DP_0f, SHRD_SIZE_FADT_MIN_TH_DP_1f, SHRD_SIZE_FADT_MIN_TH_DP_2f, SHRD_SIZE_FADT_MIN_TH_DP_3f};
    const soc_field_t field_arr_SHRD_SIZE_FADT_ADJUST_FACTOR_DP[SOC_TMC_NOF_DROP_PRECEDENCE] = 
        {SHRD_SIZE_FADT_ADJUST_FACTOR_DP_0f, SHRD_SIZE_FADT_ADJUST_FACTOR_DP_1f, SHRD_SIZE_FADT_ADJUST_FACTOR_DP_2f, SHRD_SIZE_FADT_ADJUST_FACTOR_DP_3f};
    SOC_TMC_ITM_VSQ_FADT_FIELDS_INFO pg_shared_fadt_fields = {0};
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);
    SOCDNX_NULL_CHECK(exact_info);

    if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (drop_precedence_ndx >= SOC_TMC_NOF_DROP_PRECEDENCE) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    pg_shared_fadt_fields.max_field = field_arr_SHRD_SIZE_FADT_MAX_TH_DP[drop_precedence_ndx];
    pg_shared_fadt_fields.min_field = field_arr_SHRD_SIZE_FADT_MIN_TH_DP[drop_precedence_ndx];
    pg_shared_fadt_fields.alpha_field = field_arr_SHRD_SIZE_FADT_ADJUST_FACTOR_DP[drop_precedence_ndx];

    for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        /* Set Guaranteed */
        rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                field_arr_MAX_GRNTD_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                info->max_guaranteed[rsrc_type], 
                &exact_info->max_guaranteed[rsrc_type]);
        SOCDNX_IF_ERR_EXIT(rv);

        /* Set Shared Pool - for PG-VSQ it's in FADT form */
        rv = qax_itm_vsq_rjct_fadt_set(unit,
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                pg_shared_fadt_fields,
                entry_offset,
                info->max_shared[rsrc_type],
                &exact_info->max_shared[rsrc_type]);
        SOCDNX_IF_ERR_EXIT(rv);

        /* Set Headroom */
        rv = qax_itm_vsq_pg_headroom_rjct_set(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                entry_offset,
                info->max_headroom[rsrc_type], 
                &exact_info->max_headroom[rsrc_type]);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Set PG-VSQ (VSQ-F) resource allocation params \n
 *  (Guaranteed, Shared and Headroom). \n
 *  \n
 *  The function reads all resource types (Words, SRAM-Buffers, SRAM-PDs) for \n
 *  specific rate class with specific DP.
 * 
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] vsq_rt_cls_ndx -
 *      The VSQ's rate class to update.
 *  \param [in] drop_precedence_ndx -
 *      The DP (drop-precedence) to update.
 *  \param [out] info -
 *      The returned resource allocation information.
 *
 *  \remark
 *      1) PG-VSQ shared size is defined by FADT. \n
 *      2) PG-VSQ SRAM-Headroom have also max-headroom-nominal \n
 *         and max_headroom-extension.
 */
int
  qax_itm_vsq_pg_rjct_get(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                     vsq_rt_cls_ndx,
    SOC_SAND_IN  uint32                     drop_precedence_ndx,
    SOC_SAND_OUT SOC_TMC_ITM_VSQ_PG_INFO    *info
  )
{
    int rv = SOC_E_NONE;
    uint32 entry_offset = vsq_rt_cls_ndx;
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = SOC_TMC_INGRESS_THRESHOLD_INVALID;
    const soc_mem_t mem_arr_CGM_VSQ_RJCT_PRMS[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        {CGM_VSQF_WORDS_RJCT_PRMSm, CGM_VSQF_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQF_SRAM_PDS_RJCT_PRMSm};
    const soc_field_t field_arr_MAX_GRNTD_SIZE_TH_DP[SOC_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_GRNTD_SIZE_TH_DP_0f, MAX_GRNTD_SIZE_TH_DP_1f, MAX_GRNTD_SIZE_TH_DP_2f, MAX_GRNTD_SIZE_TH_DP_3f};
    const soc_field_t field_arr_SHRD_SIZE_FADT_MAX_TH_DP[SOC_TMC_NOF_DROP_PRECEDENCE] = 
        {SHRD_SIZE_FADT_MAX_TH_DP_0f, SHRD_SIZE_FADT_MAX_TH_DP_1f, SHRD_SIZE_FADT_MAX_TH_DP_2f, SHRD_SIZE_FADT_MAX_TH_DP_3f};
    const soc_field_t field_arr_SHRD_SIZE_FADT_MIN_TH_DP[SOC_TMC_NOF_DROP_PRECEDENCE] = 
        {SHRD_SIZE_FADT_MIN_TH_DP_0f, SHRD_SIZE_FADT_MIN_TH_DP_1f, SHRD_SIZE_FADT_MIN_TH_DP_2f, SHRD_SIZE_FADT_MIN_TH_DP_3f};
    const soc_field_t field_arr_SHRD_SIZE_FADT_ADJUST_FACTOR_DP[SOC_TMC_NOF_DROP_PRECEDENCE] = 
        {SHRD_SIZE_FADT_ADJUST_FACTOR_DP_0f, SHRD_SIZE_FADT_ADJUST_FACTOR_DP_1f, SHRD_SIZE_FADT_ADJUST_FACTOR_DP_2f, SHRD_SIZE_FADT_ADJUST_FACTOR_DP_3f};
    SOC_TMC_ITM_VSQ_FADT_FIELDS_INFO pg_shared_fadt_fields = {0};
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (drop_precedence_ndx >= SOC_TMC_NOF_DROP_PRECEDENCE) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    pg_shared_fadt_fields.max_field = field_arr_SHRD_SIZE_FADT_MAX_TH_DP[drop_precedence_ndx];
    pg_shared_fadt_fields.min_field = field_arr_SHRD_SIZE_FADT_MIN_TH_DP[drop_precedence_ndx];
    pg_shared_fadt_fields.alpha_field = field_arr_SHRD_SIZE_FADT_ADJUST_FACTOR_DP[drop_precedence_ndx];

    for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        /* Set Guaranteed */
        rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                field_arr_MAX_GRNTD_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                &info->max_guaranteed[rsrc_type]);
        SOCDNX_IF_ERR_EXIT(rv);

        /* Set Shared Pool - for PG-VSQ it's in FADT form */
        rv = qax_itm_vsq_rjct_fadt_get(unit,
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                pg_shared_fadt_fields,
                entry_offset,
                &info->max_shared[rsrc_type]);
        SOCDNX_IF_ERR_EXIT(rv);

        /* Set Headroom */
        rv = qax_itm_vsq_pg_headroom_rjct_get(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                entry_offset,
                &info->max_headroom[rsrc_type]);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Set VSQ reject threshold. \n
 *  \n
 *  Convert VSQ reject threshold to mantissa-exponent form, and write it \n
 *  into the table. \n
 *  \n
 *  The function converts a value to mant-exp, write it to the table, \n
 *  converts the mant-exp form back, and return the exact value to \n
 *  the user.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit - 
 *      The unit id.
 *  \param [in] rsrc_type -
 *      The resource type (Words, SRAM-Buffers, SRAM-PDs).
 *  \param [in] mem -
 *      The table to write into.
 *  \param [in] field -
 *      The field to set.
 *  \param [in] entry_offset -
 *      The index in the table.
 *  \param [in] threshold -
 *      The value to set.
 *  \param [out] result_threshold -
 *      The exact value that was set.
 */
STATIC int
  qax_itm_vsq_rjct_man_exp_set(
    int                                 unit,
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
    soc_mem_t                           mem,
    soc_field_t                         field,
    uint32                              entry_offset,
    uint32                              threshold,
    uint32                              *result_threshold
  )
{
    int rv = SOC_E_NONE;
    soc_reg_above_64_val_t data;
    itm_mantissa_exp_threshold_info rjct_man_exp_info = {0};
    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);

    rjct_man_exp_info.mem_id = mem;

    rv = soc_mem_read(unit, rjct_man_exp_info.mem_id, CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    SOCDNX_IF_ERR_EXIT(rv);

    rjct_man_exp_info.field_id = field;
    rjct_man_exp_info.mantissa_bits = CGM_ITM_NOF_MANTISSA_BITS;
    rjct_man_exp_info.exp_bits = soc_mem_field_length(unit, rjct_man_exp_info.mem_id, rjct_man_exp_info.field_id) - rjct_man_exp_info.mantissa_bits;
    rjct_man_exp_info.factor = (rsrc_type == SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES) ? (CGM_ITM_VSQ_WORDS_RESOLUTION) : (1);

    rv = _qax_itm_mantissa_exp_field_set(unit, &rjct_man_exp_info, 1, data, threshold, result_threshold);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = soc_mem_write(unit, rjct_man_exp_info.mem_id, CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    SOCDNX_IF_ERR_EXIT(rv);

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Get VSQ reject threshold. \n
 *  \n
 *  Read threshold from table in mantissa-exponenet form, and convert it to decimal form. \n
 *
 * \par DIRECT INPUT:
 *  \param [in] rsrc_type -
 *      The resource type (Words, SRAM-Buffers, SRAM-PDs).
 *  \param [in] mem -
 *      The table to write into.
 *  \param [in] field -
 *      The field to set.
 *  \param [in] entry_offset -
 *      The index in the table.
 *  \param [out] result_threshold -
 *      The resulted value.
 */
STATIC int
  qax_itm_vsq_rjct_man_exp_get(
    int                                 unit,
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
    soc_mem_t                           mem,
    soc_field_t                         field,
    uint32                              entry_offset,
    uint32                              *result_threshold
  )
{
    int rv = SOC_E_NONE;
    soc_reg_above_64_val_t data;
    itm_mantissa_exp_threshold_info rjct_man_exp_info = {0};
    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);

    rjct_man_exp_info.mem_id = mem;

    rv = soc_mem_read(unit, rjct_man_exp_info.mem_id, CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    SOCDNX_IF_ERR_EXIT(rv);

    rjct_man_exp_info.field_id = field;
    rjct_man_exp_info.mantissa_bits = CGM_ITM_NOF_MANTISSA_BITS;
    rjct_man_exp_info.exp_bits = soc_mem_field_length(unit, rjct_man_exp_info.mem_id, rjct_man_exp_info.field_id) - rjct_man_exp_info.mantissa_bits;
    rjct_man_exp_info.factor = (rsrc_type == SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES) ? (CGM_ITM_VSQ_WORDS_RESOLUTION) : (1);

    _qax_itm_mantissa_exp_field_get(unit, &rjct_man_exp_info, data, result_threshold);

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Set VSQ FADT params. \n
 *  \n
 *  Write FADT params to the given table. \n
 *  Return the exact FADT params that was written.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] rsrc_type -
 *      The resource type (Words, SRAM-Buffers, SRAM-PDs).
 *  \param [in] mem -
 *      The table to write into.
 *  \param [in] fadt_fields -
 *      The FADT fields to set.
 *  \param [in] entry_offset -
 *      The index in the table.
 *  \param [in] fadt_info -
 *      The FADT values to set.
 *  \param [out] exact_fadt_info -
 *      The exact FADT values that was set.
 */
STATIC int
  qax_itm_vsq_rjct_fadt_set(
    int                              unit,
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type,
    soc_mem_t                        mem,
    SOC_TMC_ITM_VSQ_FADT_FIELDS_INFO fadt_fields,
    uint32                           entry_offset,
    SOC_TMC_FADT_INFO                fadt_info,
    SOC_TMC_FADT_INFO                *exact_fadt_info
  )
{
    int rv = SOC_E_NONE;
    soc_reg_above_64_val_t data;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);

    if ((fadt_info.alpha > ITM_FADT_MAX_ALPHA) || (fadt_info.alpha < ITM_FADT_MIN_ALPHA)) {
        LOG_ERROR(BSL_LS_SOC_COSQ, (BSL_META_U(unit, "FADT's alpha must be between -7 and 7\n")));
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    /* Set FADT max threshold */
    rv = qax_itm_vsq_rjct_man_exp_set(unit, 
            rsrc_type,
            mem, 
            fadt_fields.max_field,
            entry_offset,
            fadt_info.max_threshold,
            &exact_fadt_info->max_threshold);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Set FADT min threshold */
    rv = qax_itm_vsq_rjct_man_exp_set(unit, 
            rsrc_type,
            mem, 
            fadt_fields.min_field,
            entry_offset,
            fadt_info.min_threshold,
            &exact_fadt_info->min_threshold);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Set FADT alpha */
    rv = soc_mem_read(unit, mem, CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    SOCDNX_IF_ERR_EXIT(rv);

    soc_mem_field32_set(unit, mem, data, fadt_fields.alpha_field, _qax_itm_alpha_to_field(unit, fadt_info.alpha));

    rv = soc_mem_write(unit, mem, CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    SOCDNX_IF_ERR_EXIT(rv);

    exact_fadt_info->alpha = fadt_info.alpha;

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Get VSQ FADT params. \n
 *  \n
 *  Read FADT params of the given table. \n
 *  Return the decimal FADT params.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] rsrc_type -
 *      The resource type (Words, SRAM-Buffers, SRAM-PDs).
 *  \param [in] mem -
 *      The table to write into.
 *  \param [in] fadt_fields -
 *      The FADT fields to set.
 *  \param [in] entry_offset -
 *      The index in the table.
 *  \param [out] fadt_info -
 *      The returned FADT values.
 */
STATIC int
  qax_itm_vsq_rjct_fadt_get(
    int                              unit,
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type,
    soc_mem_t                        mem,
    SOC_TMC_ITM_VSQ_FADT_FIELDS_INFO fadt_fields,
    uint32                           entry_offset,
    SOC_TMC_FADT_INFO                *fadt_info
  )
{
    int rv = SOC_E_NONE;
    soc_reg_above_64_val_t data;
    uint32 alpha = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);

    /* Set FADT max threshold */
    rv = qax_itm_vsq_rjct_man_exp_get(unit, 
            rsrc_type,
            mem, 
            fadt_fields.max_field,
            entry_offset,
            &fadt_info->max_threshold);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Set FADT min threshold */
    rv = qax_itm_vsq_rjct_man_exp_get(unit, 
            rsrc_type,
            mem, 
            fadt_fields.min_field,
            entry_offset,
            &fadt_info->min_threshold);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Set FADT alpha */
    rv = soc_mem_read(unit, mem, CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    SOCDNX_IF_ERR_EXIT(rv);

    alpha = soc_mem_field32_get(unit, mem, data, fadt_fields.alpha_field);    
    fadt_info->alpha = _qax_itm_field_to_alpha(unit, alpha);

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Set PG-VSQ Headroom params. \n
 *  \n
 *  Write Headroom params to the given table. \n
 *  Return the exact Headroom params that was written. \n
 *  \n
 *  PG-VSQ headroom is divided to: \n
 *   Words           - headroom. \n
 *   SRAM-Buffers    - headroom, headroom nominal, headroom extension. \n
 *   SRAM-PDs        - headroom, headroom nominal, headroom extension. \n
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] rsrc_type -
 *      The resource type (Words, SRAM-Buffers, SRAM-PDs).
 *  \param [in] mem -
 *      The table to write into.
 *  \param [in] entry_offset -
 *      The index in the table.
 *  \param [in] headroom_info -
 *      The headroom values to set.
 *  \param [out] exact_headroom_info -
 *      The exact headroom values that was set.
 */
STATIC int
  qax_itm_vsq_pg_headroom_rjct_set(
    int                                 unit,
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
    soc_mem_t                           mem,
    uint32                              entry_offset,
    SOC_TMC_ITM_VSQ_PG_HDRM_INFO        headroom_info,
    SOC_TMC_ITM_VSQ_PG_HDRM_INFO        *exact_headroom_info
  )
{
    int rv = SOC_E_NONE;
    SOCDNX_INIT_FUNC_DEFS;

    /* Set max headroom threshold */
    rv = qax_itm_vsq_rjct_man_exp_set(unit, 
            rsrc_type,
            mem, 
            MAX_HDRM_SIZE_THf,
            entry_offset,
            headroom_info.max_headroom,
            &exact_headroom_info->max_headroom);
    SOCDNX_IF_ERR_EXIT(rv);

    if (rsrc_type != SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES) {
        /* Set max headroom nominal threshold */
        rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                mem, 
                MAX_HDRM_NOMINAL_SIZE_THf,
                entry_offset,
                headroom_info.max_headroom_nominal,
                &exact_headroom_info->max_headroom_nominal);
        SOCDNX_IF_ERR_EXIT(rv);

        /* Set max headroom extension threshold */
        rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                mem, 
                MAX_HDRM_EXT_SIZE_THf,
                entry_offset,
                headroom_info.max_headroom_extension,
                &exact_headroom_info->max_headroom_extension);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Get PG-VSQ Headroom params. \n
 *  \n
 *  Read Headroom params to the given table. \n
 *  Return the decimal Headroom params. \n
 *  \n
 *  PG-VSQ headroom is divided to: \n
 *   Words           - headroom. \n
 *   SRAM-Buffers    - headroom, headroom nominal, headroom extension. \n
 *   SRAM-PDs        - headroom, headroom nominal, headroom extension. \n
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] rsrc_type -
 *      The resource type (Words, SRAM-Buffers, SRAM-PDs).
 *  \param [in] mem -
 *      The table to write into.
 *  \param [in] entry_offset -
 *      The index in the table.
 *  \param [out] headroom_info -
 *      The headroom values to set.
 */
STATIC int
  qax_itm_vsq_pg_headroom_rjct_get(
    int                                 unit,
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
    soc_mem_t                           mem,
    uint32                              entry_offset,
    SOC_TMC_ITM_VSQ_PG_HDRM_INFO        *headroom_info
  )
{
    int rv = SOC_E_NONE;
    SOCDNX_INIT_FUNC_DEFS;

    /* Set max headroom threshold */
    rv = qax_itm_vsq_rjct_man_exp_get(unit, 
            rsrc_type,
            mem, 
            MAX_HDRM_SIZE_THf,
            entry_offset,
            &headroom_info->max_headroom);
    SOCDNX_IF_ERR_EXIT(rv);

    if (rsrc_type != SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES) {
        /* Set max headroom nominal threshold */
        rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                mem, 
                MAX_HDRM_NOMINAL_SIZE_THf,
                entry_offset,
                &headroom_info->max_headroom_nominal);
        SOCDNX_IF_ERR_EXIT(rv);

        /* Set max headroom extension threshold */
        rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                mem, 
                MAX_HDRM_EXT_SIZE_THf,
                entry_offset,
                &headroom_info->max_headroom_extension);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Each Virtual Statistics Queue has a VSQ-Rate-Class (profile). \n
 *  This function assigns a VSQ with its Rate Class.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] core_id -
 *      Not used.
 *  \param [in] is_ocb_only -
 *      Not used.
 *  \param [in] vsq_group_ndx -
 *      VSQ type (VSQ-A/B/C/D/E/F).
 *  \param [in] vsq_in_group_ndx -
 *      The VSQ index.
 *  \param [in] vsq_rt_cls -
 *      The rate class to assign with the VSQ.
 */
int
  qax_itm_vsq_qt_rt_cls_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  uint8               is_ocb_only,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_NDX    vsq_in_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls
  )
{
    int rv = SOC_E_NONE;
    uint32 data = 0;
    soc_mem_t mem = 0;
    uint32 entry_offset = 0;
    SOCDNX_INIT_FUNC_DEFS;

    rv = qax_itm_vsq_qt_rt_cls_verify(unit, vsq_group_ndx, vsq_in_group_ndx);
    SOCDNX_IF_ERR_EXIT(rv);

    if (vsq_group_ndx == SOC_TMC_ITM_VSQ_GROUP_CTGRY) {
        if (vsq_rt_cls > SOC_TMC_ITM_VSQ_A_QT_RT_CLS_MAX(unit)) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
    } else {
        if (vsq_rt_cls > SOC_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }

    switch(vsq_group_ndx) { 
        case SOC_TMC_ITM_VSQ_GROUP_CTGRY:
            mem = CGM_VSQA_PRMSm;
            break;
        case SOC_TMC_ITM_VSQ_GROUP_CTGRY_TRAFFIC_CLS:
            mem = CGM_VSQB_PRMSm;
            break;
        case SOC_TMC_ITM_VSQ_GROUP_CTGRY_2_3_CNCTN_CLS:
            mem = CGM_VSQC_PRMSm;
            break;
        case SOC_TMC_ITM_VSQ_GROUP_STTSTCS_TAG:
            mem = CGM_VSQD_PRMSm;
            break;
        case SOC_TMC_ITM_VSQ_GROUP_SRC_PORT:
            mem = CGM_VSQE_PRMSm;
            break;
        case SOC_TMC_ITM_VSQ_GROUP_PG:
            mem = CGM_VSQF_PRMSm;
            break;
        default:
            break;
    }

    entry_offset = vsq_in_group_ndx;

    rv = soc_mem_read(unit, mem, CGM_BLOCK(unit, core_id), entry_offset, &data);
    SOCDNX_IF_ERR_EXIT(rv);

    soc_mem_field32_set(unit, mem, &data, RATE_CLASSf, vsq_rt_cls);

    rv = soc_mem_write(unit, mem, CGM_BLOCK(unit, core_id), entry_offset, &data);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Validate params for vsq_rt_cls functions.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] vsq_group_ndx -
 *      VSQ type (VSQ-A/B/C/D/E/F).
 *  \param [in] vsq_in_group_ndx -
 *      The VSQ index.
 */
STATIC int
  qax_itm_vsq_qt_rt_cls_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_NDX    vsq_in_group_ndx
  )
{
    uint32 rv = SOC_E_NONE;
    uint32 vsq_in_group_size;
    SOCDNX_INIT_FUNC_DEFS;

    if (vsq_group_ndx >= SOC_TMC_NOF_VSQ_GROUPS) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    rv = arad_itm_vsq_in_group_size_get(unit, vsq_group_ndx, &vsq_in_group_size);
    SOCDNX_IF_ERR_EXIT(rv);

    if (vsq_in_group_ndx >= vsq_in_group_size) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

exit:
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Each Virtual Statistics Queue has a VSQ-Rate-Class (profile). \n
 *  This function returns the Rate Class that is assigned with the VSQ.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] core_id -
 *      Not used.
 *  \param [in] is_ocb_only -
 *      Not used.
 *  \param [in] vsq_group_ndx -
 *      VSQ type (VSQ-A/B/C/D/E/F).
 *  \param [in] vsq_in_group_ndx -
 *      The VSQ index.
 *  \param [out] vsq_rt_cls -
 *      The rate class that is assign with the VSQ.
 */
int
  qax_itm_vsq_qt_rt_cls_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  uint8               is_ocb_only,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_NDX    vsq_in_group_ndx,
    SOC_SAND_OUT uint32                 *vsq_rt_cls
  )
{
    int rv = SOC_E_NONE;
    uint32 data = 0;
    soc_mem_t mem = 0;
    uint32 entry_offset = 0;
    SOCDNX_INIT_FUNC_DEFS;

    rv = qax_itm_vsq_qt_rt_cls_verify(unit, vsq_group_ndx, vsq_in_group_ndx);
    SOCDNX_IF_ERR_EXIT(rv);

    switch(vsq_group_ndx) { 
        case SOC_TMC_ITM_VSQ_GROUP_CTGRY:
            mem = CGM_VSQA_PRMSm;
            break;
        case SOC_TMC_ITM_VSQ_GROUP_CTGRY_TRAFFIC_CLS:
            mem = CGM_VSQB_PRMSm;
            break;
        case SOC_TMC_ITM_VSQ_GROUP_CTGRY_2_3_CNCTN_CLS:
            mem = CGM_VSQC_PRMSm;
            break;
        case SOC_TMC_ITM_VSQ_GROUP_STTSTCS_TAG:
            mem = CGM_VSQD_PRMSm;
            break;
        case SOC_TMC_ITM_VSQ_GROUP_SRC_PORT:
            mem = CGM_VSQE_PRMSm;
            break;
        case SOC_TMC_ITM_VSQ_GROUP_PG:
            mem = CGM_VSQF_PRMSm;
            break;
        default:
            break;
    }

    entry_offset = vsq_in_group_ndx;

    rv = soc_mem_read(unit, mem, CGM_BLOCK(unit, core_id), entry_offset, &data);
    SOCDNX_IF_ERR_EXIT(rv);

    *vsq_rt_cls = soc_mem_field32_get(unit, mem, &data, RATE_CLASSf);

exit:
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Get PP-port corresponds to a VSQ-E index.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] core_id -
 *      The core id.
 *  \param [in] src_port_vsq_index -
 *      VSQ-E index.
 *  \param [out] src_pp_port -
 *      The associated PP-port.
 *  \param [out] enable -
 *      1 if found PP-port.
 */
int
  qax_itm_vsq_src_port_get(
    SOC_SAND_IN int    unit,
    SOC_SAND_IN int    core_id,
    SOC_SAND_IN int    src_port_vsq_index,
    SOC_SAND_OUT uint32 *src_pp_port,
    SOC_SAND_OUT uint8  *enable
  )
{
    int rv;
    uint32 tmp_src_pp_port, data;
    uint8 found = 0;
    int tmp_src_port_vsq_index = 0;
    uint32 phy_port = -1;
    soc_port_t logical_port;
    int core_get;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(enable);
    SOCDNX_NULL_CHECK(src_pp_port);

    if (core_id < 0 || core_id >= SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (src_port_vsq_index < 0 || src_port_vsq_index >= SOC_DPP_DEFS_GET(unit, nof_vsq_e)) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    /* 
     * Since VSQ-E index matches NIF_PORT, we can get the pp_port directly
     * from VSQ-E index. Then we need to validate that the data in the table
     * match:
     * CGM_IPPPMm[pp_port].NIF_PORTf = VSQ-E_index.
     */
    /* Get PP port that matches the phy_port */
    phy_port = src_port_vsq_index + 1; /* phy_port is 1 based */
    logical_port = SOC_INFO(unit).port_p2l_mapping[phy_port];
    rv = soc_port_sw_db_local_to_pp_port_get(unit, logical_port, &tmp_src_pp_port, &core_get);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Get VSQ-E index of the PP-port entry */
    rv = READ_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), tmp_src_pp_port, &data);
    SOCDNX_IF_ERR_EXIT(rv);
    tmp_src_port_vsq_index = soc_mem_field32_get(unit, CGM_IPPPMm, &data, NIF_PORTf);

    /* Validate VSQ-E index of the PP-port matches the phy port */
    if (tmp_src_port_vsq_index == src_port_vsq_index) {
        found = 1;
    }

    if (found) {
        *src_pp_port = tmp_src_pp_port;
        *enable = 1;
    } else {
        *src_pp_port = -1;
        *enable = 0;
    }

exit: 
    SOCDNX_FUNC_RETURN;
}

/*!
 * \brief
 *  Get PG-VSQ mapping corresponds to a PG-BASE-VSQ index.
 *
 */
/*!
 * \brief
 *  Get PP-port corresponds to a VSQ-F index.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] core_id -
 *      The core id.
 *  \param [in] pg_vsq_base -
 *      VSQ-F pg_base index.
 *  \param [in] cosq -
 *      PG-VSQ offset.
 *  \param [out] src_pp_port -
 *      The associated PP-port.
 *  \param [out] enable -
 *      1 if found PP-port.
 */
int
  qax_itm_vsq_pg_mapping_get(
    SOC_SAND_IN int     unit,
    SOC_SAND_IN int     core_id,
    SOC_SAND_IN uint32  pg_vsq_base,
    SOC_SAND_IN int     cosq,
    SOC_SAND_OUT uint32 *src_pp_port,
    SOC_SAND_OUT uint8  *enable
  )
{
    int rv;
    uint32 pg_vsq_id;
    uint32 tmp_pg_vsq_base = -1, tmp_src_pp_port = 0, data = 0;
    int found = 0;
    soc_port_t logical_port;
    uint32 src_link = -1, phy_port = -1;
    int core_get;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(enable);
    SOCDNX_NULL_CHECK(src_pp_port);

    if (core_id < 0 || core_id >= SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (cosq < 0 || cosq >= SOC_TMC_NOF_TRAFFIC_CLASSES) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    pg_vsq_id = pg_vsq_base + cosq;
    if (pg_vsq_id >= SOC_DPP_DEFS_GET(unit, nof_vsq_f)) {
        LOG_ERROR(BSL_LS_SOC_COSQ, (BSL_META_U(unit, "Invalid PG VSQ ID %d\n"), pg_vsq_id));
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    /* 
     * Since Base-PG-VSQ index matches (serdes * nof_traffic_classes), we can 
     * get the pp_port directly from it. Then we need to validate that the data 
     * in the table match:
     * CGM_IPPPMm[pp_port].PG_BASEf = Base-PG-VSQ.
     */
    /* Get PP port that matches the phy_port */
    src_link = pg_vsq_id / SOC_TMC_NOF_TRAFFIC_CLASSES;
    phy_port = src_link + 1; /* phy_port is 1 based */
    rv = MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_add, (unit, phy_port, &phy_port));
    SOCDNX_IF_ERR_EXIT(rv);
    logical_port = SOC_INFO(unit).port_p2l_mapping[phy_port];
    rv = soc_port_sw_db_local_to_pp_port_get(unit, logical_port, &tmp_src_pp_port, &core_get);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Get Base-PG-VSQ index of the PP-port entry */
    rv = READ_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), tmp_src_pp_port, &data);
    SOCDNX_IF_ERR_EXIT(rv);
    tmp_pg_vsq_base = soc_mem_field32_get(unit, CGM_IPPPMm, &data, PG_BASEf);

    /* Validate Base-PG-VSQ index of the PP-port matches */
    if (tmp_pg_vsq_base == pg_vsq_base) {
        found = 1;
    }

    if (found) {
        *src_pp_port = tmp_src_pp_port;
        *enable = 1;
    } else {
        *src_pp_port = -1;
        *enable = 0;
    }

exit: 
    SOCDNX_FUNC_RETURN;
}

/*!
 * \brief
 *  Verify that category ranges are in range. \n
 *
 *  VOQs are devided to 4 categories in contiguous blocks. \n
 *  This mapped to VSQ-A.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] info -
 *      The category ranges to validate.
 *
 * \remark
 *  Category-4: from 'category-end-3' till the last queue.
 */
STATIC int 
  qax_itm_category_rngs_verify( 
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  SOC_TMC_ITM_CATEGORY_RNGS *info 
  ) 
{ 
    SOCDNX_INIT_FUNC_DEFS; 
    SOCDNX_NULL_CHECK(info);    

    if ((info->vsq_ctgry0_end > info->vsq_ctgry1_end) || (info->vsq_ctgry1_end > info->vsq_ctgry2_end) || (info->vsq_ctgry0_end > info->vsq_ctgry2_end)) { 
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

    if (info->vsq_ctgry0_end > SOC_DPP_DEFS_GET(unit, nof_queues)-1) { 
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

    if (info->vsq_ctgry1_end > (SOC_DPP_DEFS_GET(unit, nof_queues)-1)) { 
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

    if (info->vsq_ctgry2_end > (SOC_DPP_DEFS_GET(unit, nof_queues)-1)) { 
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

exit: 
    SOCDNX_FUNC_RETURN; 
} 
 
/*!
 * \brief
 *  Set VSQ-A category ranges. \n
 *
 *  VOQs are divided to 4 categories in contiguous blocks.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] core_id -
 *      The core id.
 *  \param [in] info -
 *      The category ranges.
 *
 * \remark
 *  Category-4: from 'category-end-3' till the last queue.
 */
int 
  qax_itm_category_rngs_set( 
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 core_id, 
    SOC_SAND_IN  SOC_TMC_ITM_CATEGORY_RNGS *info 
  ) 
{ 
    int rv; 
    uint64 reg_val; 
    SOCDNX_INIT_FUNC_DEFS; 

    SOCDNX_NULL_CHECK(info); 

    COMPILER_64_ZERO(reg_val);

    if (((core_id < 0) || (core_id > SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores)) && core_id != SOC_CORE_ALL) { 
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

    rv = qax_itm_category_rngs_verify(unit, info); 
    SOCDNX_IF_ERR_EXIT(rv); 

    rv = READ_CGM_VSQ_CATEGORY_RANGESr(unit, &reg_val); 
    SOCDNX_IF_ERR_EXIT(rv); 

    soc_reg64_field32_set(unit, CGM_VSQ_CATEGORY_RANGESr, &reg_val, VSQ_CATEGORY_RANGE_0f, info->vsq_ctgry0_end); 
    soc_reg64_field32_set(unit, CGM_VSQ_CATEGORY_RANGESr, &reg_val, VSQ_CATEGORY_RANGE_1f, info->vsq_ctgry1_end); 
    soc_reg64_field32_set(unit, CGM_VSQ_CATEGORY_RANGESr, &reg_val, VSQ_CATEGORY_RANGE_2f, info->vsq_ctgry2_end); 

    rv = WRITE_CGM_VSQ_CATEGORY_RANGESr(unit, reg_val); 
    SOCDNX_IF_ERR_EXIT(rv); 

exit: 
    SOCDNX_FUNC_RETURN; 
} 
 
/*!
 * \brief
 *  Get VSQ-A category ranges. \n
 *
 *  VOQs are divided to 4 categories in contiguous blocks.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] core_id -
 *      The core id.
 *  \param [out] info -
 *      The category ranges.
 *
 * \remark
 *  Category-4: from 'category-end-3' till the last queue.
 */
int 
  qax_itm_category_rngs_get( 
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 core_id, 
    SOC_SAND_OUT SOC_TMC_ITM_CATEGORY_RNGS *info 
  ) 
{ 
    int rv; 
    uint64 reg_val; 
    SOCDNX_INIT_FUNC_DEFS; 

    SOCDNX_NULL_CHECK(info); 

    COMPILER_64_ZERO(reg_val);

    if (((core_id < 0) || (core_id > SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores)) && core_id != SOC_CORE_ALL) { 
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

    rv = READ_CGM_VSQ_CATEGORY_RANGESr(unit, &reg_val); 
    SOCDNX_IF_ERR_EXIT(rv); 

    info->vsq_ctgry0_end = soc_reg64_field32_get(unit, CGM_VSQ_CATEGORY_RANGESr, reg_val, VSQ_CATEGORY_RANGE_0f); 
    info->vsq_ctgry1_end = soc_reg64_field32_get(unit, CGM_VSQ_CATEGORY_RANGESr, reg_val, VSQ_CATEGORY_RANGE_1f); 
    info->vsq_ctgry2_end = soc_reg64_field32_get(unit, CGM_VSQ_CATEGORY_RANGESr, reg_val, VSQ_CATEGORY_RANGE_2f); 

exit: 
    SOCDNX_FUNC_RETURN; 
} 

/*!
 * \brief
 *  Verify that the given global reject thresholds are in valid ranges.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] info -
 *      Reject thresholds to validate.
 */
STATIC
int
  qax_itm_glob_rcs_drop_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  SOC_TMC_ITM_GLOB_RCS_DROP_TH *info
  )
{
    int color = 0, sram_type = 0;
    uint32 max_threshold = 0;
    SOCDNX_INIT_FUNC_DEFS; 

    SOCDNX_NULL_CHECK(info); 

    /* Validate Global Free SRAM resources reject thresholds. */
    /* Resources can be SRAM-PDBs or SRAM-Buffers. */
    for (sram_type = 0; sram_type < SOC_TMC_INGRESS_THRESHOLD_NOF_SRAM_TYPES; ++sram_type) {
        max_threshold = (sram_type == 0) ? (CGM_ITM_VSQ_SRAM_PDBS_SIZE_MAX) : (CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX(unit));
        for (color = 0; color < SOC_TMC_NOF_DROP_PRECEDENCE; ++color) {
            if (info->global_free_sram[sram_type][color].set > max_threshold) {
                SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
            }
            if (info->global_free_sram[sram_type][color].clear > max_threshold) {
                SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
            }

            /* Validate SRAM-Only thresholds */
            if (info->global_free_sram_only[sram_type][color].set > max_threshold) {
                SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
            }
            if (info->global_free_sram_only[sram_type][color].clear > max_threshold) {
                SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
            }
        }
    }

    /* Validate Global Free DRAM-BDBs resources reject thresholds */
    for (color = 0; color < SOC_TMC_NOF_DROP_PRECEDENCE; ++color) {
        if (info->global_free_dram[color].set > CGM_ITM_VSQ_DRAM_BDBS_SIZE_MAX) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
        if (info->global_free_dram[color].clear > CGM_ITM_VSQ_DRAM_BDBS_SIZE_MAX) {
            SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Set Global reject thresholds.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] core_id -
 *      The core id.
 *  \param [in] info -
 *      The global reject thresholds to set.
 *  \param [out] exact_info -
 *      The exact global reject thresholds that was written.
 */
int
  qax_itm_glob_rcs_drop_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  SOC_TMC_ITM_GLOB_RCS_DROP_TH *info,
    SOC_SAND_OUT SOC_TMC_ITM_GLOB_RCS_DROP_TH *exact_info
  )
{
    int rv = SOC_E_NONE; 
    int color = 0, sram_type = 0;
    uint32 entry_offset = 0;

    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = SOC_TMC_INGRESS_THRESHOLD_INVALID;

    soc_field_t field_arr_GLBL_FR_SRAM_SET_TH[SOC_TMC_INGRESS_THRESHOLD_NOF_SRAM_TYPES] = 
        {SRAM_PDBS_SET_THf, SRAM_BUFFERS_SET_THf};
    soc_field_t field_arr_GLBL_FR_SRAM_CLR_TH[SOC_TMC_INGRESS_THRESHOLD_NOF_SRAM_TYPES] = 
        {SRAM_PDBS_CLR_THf, SRAM_BUFFERS_CLR_THf};

    SOCDNX_INIT_FUNC_DEFS; 

    SOCDNX_NULL_CHECK(info); 
    SOCDNX_NULL_CHECK(exact_info); 

    if (((core_id < 0) || (core_id > SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores)) && core_id != SOC_CORE_ALL) { 
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

    /* Validate inputs are in range */
    rv = qax_itm_glob_rcs_drop_verify(unit, info);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Set Global free SRAM resources reject thresholds */
    for (sram_type = 0; sram_type < SOC_TMC_INGRESS_THRESHOLD_NOF_SRAM_TYPES; ++sram_type) {
        for (color = 0; color < SOC_TMC_NOF_DROP_PRECEDENCE; ++color) {
            /* Set thresholds for regular VOQs */
            entry_offset = color;

            /* set SET threshold */
            rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_SET_TH[sram_type], 
                    entry_offset,
                    info->global_free_sram[sram_type][color].set, 
                    &exact_info->global_free_sram[sram_type][color].set);
            SOCDNX_IF_ERR_EXIT(rv);

            /* set CLEAR threshold */
            rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_CLR_TH[sram_type], 
                    entry_offset,
                    info->global_free_sram[sram_type][color].clear, 
                    &exact_info->global_free_sram[sram_type][color].clear);
            SOCDNX_IF_ERR_EXIT(rv);

            /* Set thresholds for SRAM-only VOQs */
            entry_offset = color + SOC_TMC_NOF_DROP_PRECEDENCE;

            /* set SET threshold */
            rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_SET_TH[sram_type], 
                    entry_offset,
                    info->global_free_sram_only[sram_type][color].set, 
                    &exact_info->global_free_sram_only[sram_type][color].set);
            SOCDNX_IF_ERR_EXIT(rv);

            /* set CLEAR threshold */
            rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_CLR_TH[sram_type], 
                    entry_offset,
                    info->global_free_sram_only[sram_type][color].clear, 
                    &exact_info->global_free_sram_only[sram_type][color].clear);
            SOCDNX_IF_ERR_EXIT(rv);
        }
    }

    /* Set Global free DRAM resources reject thresholds */
    for (color = 0; color < SOC_TMC_NOF_DROP_PRECEDENCE; ++color) {
        entry_offset = color;

        /* set SET threshold */
        rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                CGM_GLBL_FR_DRAM_RJCT_THm, 
                DRAM_BDBS_SET_THf, 
                entry_offset,
                info->global_free_dram[color].set, 
                &exact_info->global_free_dram[color].set);
        SOCDNX_IF_ERR_EXIT(rv);

        /* set CLEAR threshold */
        rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                CGM_GLBL_FR_DRAM_RJCT_THm, 
                DRAM_BDBS_CLR_THf, 
                entry_offset,
                info->global_free_dram[color].clear, 
                &exact_info->global_free_dram[color].clear);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Get Global reject thresholds.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] core_id -
 *      The core id.
 *  \param [out] info -
 *      The returned global reject thresholds.
 */
int
  qax_itm_glob_rcs_drop_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_OUT  SOC_TMC_ITM_GLOB_RCS_DROP_TH *info
  )
{
    int rv = SOC_E_NONE; 
    int color = 0, sram_type = 0;
    uint32 entry_offset = 0;

    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = SOC_TMC_INGRESS_THRESHOLD_INVALID;

    soc_field_t field_arr_GLBL_FR_SRAM_SET_TH[SOC_TMC_INGRESS_THRESHOLD_NOF_SRAM_TYPES] = 
        {SRAM_PDBS_SET_THf, SRAM_BUFFERS_SET_THf};
    soc_field_t field_arr_GLBL_FR_SRAM_CLR_TH[SOC_TMC_INGRESS_THRESHOLD_NOF_SRAM_TYPES] = 
        {SRAM_PDBS_CLR_THf, SRAM_BUFFERS_CLR_THf};

    SOCDNX_INIT_FUNC_DEFS; 

    SOCDNX_NULL_CHECK(info); 

    if (((core_id < 0) || (core_id > SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores)) && core_id != SOC_CORE_ALL) { 
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

    /* Set Global free SRAM resources reject thresholds */
    for (sram_type = 0; sram_type < SOC_TMC_INGRESS_THRESHOLD_NOF_SRAM_TYPES; ++sram_type) {
        for (color = 0; color < SOC_TMC_NOF_DROP_PRECEDENCE; ++color) {
            /* Set thresholds for regular VOQs */
            entry_offset = color;

            /* set SET threshold */
            rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_SET_TH[sram_type], 
                    entry_offset,
                    &info->global_free_sram[sram_type][color].set);
            SOCDNX_IF_ERR_EXIT(rv);

            /* set CLEAR threshold */
            rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_CLR_TH[sram_type], 
                    entry_offset,
                    &info->global_free_sram[sram_type][color].clear);
            SOCDNX_IF_ERR_EXIT(rv);

            /* Set thresholds for SRAM-only VOQs */
            entry_offset = color + SOC_TMC_NOF_DROP_PRECEDENCE;

            /* set SET threshold */
            rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_SET_TH[sram_type], 
                    entry_offset,
                    &info->global_free_sram_only[sram_type][color].set);
            SOCDNX_IF_ERR_EXIT(rv);

            /* set CLEAR threshold */
            rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_CLR_TH[sram_type], 
                    entry_offset,
                    &info->global_free_sram_only[sram_type][color].clear);
            SOCDNX_IF_ERR_EXIT(rv);
        }
    }

    /* Set Global free DRAM resources reject thresholds */
    for (color = 0; color < SOC_TMC_NOF_DROP_PRECEDENCE; ++color) {
        entry_offset = color;

        /* set SET threshold */
        rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                CGM_GLBL_FR_DRAM_RJCT_THm, 
                DRAM_BDBS_SET_THf, 
                entry_offset,
                &info->global_free_dram[color].set);

        /* set CLEAR threshold */
        rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                CGM_GLBL_FR_DRAM_RJCT_THm, 
                DRAM_BDBS_CLR_THf, 
                entry_offset,
                &info->global_free_dram[color].clear);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*
 * Validity check for qax_itm_vsq_fc_set() input paramters
 */
STATIC int
  qax_itm_vsq_fc_verify(
    SOC_SAND_IN int                        unit,
    SOC_SAND_IN SOC_TMC_ITM_VSQ_GROUP      vsq_group_ndx,
    SOC_SAND_IN uint32                     vsq_rt_cls_ndx,
    SOC_SAND_IN int                        pool_id,
    SOC_SAND_IN SOC_TMC_ITM_VSQ_FC_INFO    *info
  )
{
    SOCDNX_INIT_FUNC_DEFS;

    if (vsq_group_ndx < 0 || vsq_group_ndx >= SOC_TMC_NOF_VSQ_GROUPS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Invalid VSQ group (%d)"), vsq_group_ndx));
    }
    if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Invalid rate class (%d)"), vsq_rt_cls_ndx));
    }
    if (pool_id < 0 || pool_id >= SOC_TMC_ITM_NOF_RSRC_POOLS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Invalid pool id (%d)"), pool_id));
    }

    /* Verify VSQs A-E flow control sizes */
    if (info->size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].clear > ((CGM_ITM_VSQ_WORDS_SIZE_MAX(unit) + 1) * CGM_ITM_VSQ_WORDS_RESOLUTION)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Total bytes flow control CLEAR threshold is too high (%u)"), 
                    info->size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].clear));
    }
    if (info->size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set > ((CGM_ITM_VSQ_WORDS_SIZE_MAX(unit) + 1) * CGM_ITM_VSQ_WORDS_RESOLUTION)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Total bytes flow control SET threshold is too high (%u)"), 
                    info->size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set));
    }
    if (info->size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].clear > CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX(unit) + 1) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("SRAM-Buffers flow control CLEAR threshold is too high (%u)"), 
                    info->size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].clear));
    }
    if (info->size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set > CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX(unit) + 1) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("SRAM-Buffers flow control SET threshold is too high (%u)"), 
                    info->size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set));
    }
    if (info->size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].clear > CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX + 1) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("SRAM-PDs flow control CLEAR threshold is too high (%u)"), 
                    info->size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].clear));
    }
    if (info->size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set > CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX + 1) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("SRAM-PDs flow control SET threshold is too high (%u)"), 
                    info->size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set));
    }

    /* Verify VSQ-F flow control sizes */
    if (info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set.max_threshold > ((CGM_ITM_VSQ_WORDS_SIZE_MAX(unit) + 1) * CGM_ITM_VSQ_WORDS_RESOLUTION)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Total bytes flow control SET fadt max threshold is too high (%u)"), 
                    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set.max_threshold));
    }
    if (info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set.min_threshold > info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set.max_threshold) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Total bytes flow control SET fadt min threshold (%u) can't be above fadt max threshold (%u)"), 
                    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set.min_threshold,
                    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set.max_threshold));
    }
    if (info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set.alpha > ITM_FADT_MAX_ALPHA || 
            info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set.alpha < ITM_FADT_MIN_ALPHA) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Total bytes flow control SET fadt alpha must be between (%d <= alpha <= %d)"), 
                    ITM_FADT_MIN_ALPHA, ITM_FADT_MAX_ALPHA));
    }
    if (info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].clear_offset > info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set.min_threshold) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Total bytes flow control CLEAR fadt offset (%u) can't be above fadt min threshold (%u)"), 
                    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].clear_offset,
                    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set.min_threshold));
    }

    if (info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set.max_threshold > CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX(unit) + 1) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("SRAM-Buffers flow control SET fadt max threshold is too high (%u)"), 
                    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set.max_threshold));
    }
    if (info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set.min_threshold > info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set.max_threshold) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("SRAM-Buffers flow control SET fadt min threshold (%u) can't be above fadt max threshold (%u)"), 
                    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set.min_threshold,
                    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set.max_threshold));
    }
    if (info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set.alpha > ITM_FADT_MAX_ALPHA || 
            info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set.alpha < ITM_FADT_MIN_ALPHA) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("SRAM-Buffers flow control SET fadt alpha must be between (%d <= alpha <= %d)"), 
                    ITM_FADT_MIN_ALPHA, ITM_FADT_MAX_ALPHA));
    }
    if (info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].clear_offset > info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set.min_threshold) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("SRAM-Buffers flow control CLEAR fadt offset (%u) can't be above fadt min threshold (%u)"), 
                    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].clear_offset,
                    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set.min_threshold));
    }

    if (info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set.max_threshold > CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX + 1) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("SRAM-PDs flow control SET fadt max threshold is too high (%u)"), 
                    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set.max_threshold));
    }
    if (info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set.min_threshold > info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set.max_threshold) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("SRAM-PDs flow control SET fadt min threshold (%u) can't be above fadt max threshold (%u)"), 
                    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set.min_threshold,
                    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set.max_threshold));
    }
    if (info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set.alpha > ITM_FADT_MAX_ALPHA || 
            info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set.alpha < ITM_FADT_MIN_ALPHA) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("SRAM-PDs flow control SET fadt alpha must be between (%d <= alpha <= %d)"), 
                    ITM_FADT_MIN_ALPHA, ITM_FADT_MAX_ALPHA));
    }
    if (info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].clear_offset > info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set.min_threshold) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("SRAM-PDs flow control CLEAR fadt offset (%u) can't be above fadt min threshold (%u)"), 
                    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].clear_offset,
                    info->fadt_size_fc[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].set.min_threshold));
    }

exit: 
    SOCDNX_FUNC_RETURN; 
}

/*!
 * \brief
 *  Set the VSQ flow-control info according to vsq-group-id
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] vsq_group_ndx -
 *      The VSQ type.
 *  \param [in] vsq_rt_cls_ndx -
 *      The VSQ rate class.
 *  \param [in] pool_id -
 *      The pool id. Relevant for VSQ-E.
 *  \param [in] info -
 *      FC information.
 *  \param [in] exact_info -
 *      The exact FC information that was written.
 */
int
  qax_itm_vsq_fc_set(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP      vsq_group_ndx,
    SOC_SAND_IN  uint32                     vsq_rt_cls_ndx,
    SOC_SAND_IN  int                        pool_id,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_FC_INFO    *info,
    SOC_SAND_OUT SOC_TMC_ITM_VSQ_FC_INFO    *exact_info
  )
{
    int rv = SOC_E_NONE;
    uint32 entry_offset = 0;
    const soc_mem_t mem_arr_CGM_VSQ_FC_PRMS[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES][SOC_TMC_NOF_VSQ_GROUPS] = 
        { {CGM_VSQA_WORDS_FC_PRMSm, CGM_VSQB_WORDS_FC_PRMSm, CGM_VSQC_WORDS_FC_PRMSm, CGM_VSQD_WORDS_FC_PRMSm, CGM_VSQE_WORDS_FC_PRMSm, CGM_VSQF_WORDS_FC_PRMSm},
          {CGM_VSQA_SRAM_BUFFERS_FC_PRMSm, CGM_VSQB_SRAM_BUFFERS_FC_PRMSm, CGM_VSQC_SRAM_BUFFERS_FC_PRMSm, CGM_VSQD_SRAM_BUFFERS_FC_PRMSm, CGM_VSQE_SRAM_BUFFERS_FC_PRMSm, CGM_VSQF_SRAM_BUFFERS_FC_PRMSm},
          {CGM_VSQA_SRAM_PDS_FC_PRMSm, CGM_VSQB_SRAM_PDS_FC_PRMSm, CGM_VSQC_SRAM_PDS_FC_PRMSm, CGM_VSQD_SRAM_PDS_FC_PRMSm, CGM_VSQE_SRAM_PDS_FC_PRMSm, CGM_VSQF_SRAM_PDS_FC_PRMSm} };
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = SOC_TMC_INGRESS_THRESHOLD_INVALID;
    SOC_TMC_ITM_VSQ_FADT_FIELDS_INFO pg_fc_fadt_fields;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);
    SOCDNX_NULL_CHECK(exact_info);

    /* Verify input params */
    rv = qax_itm_vsq_fc_verify(unit, vsq_group_ndx, vsq_rt_cls_ndx, pool_id, info);
    SOCDNX_IF_ERR_EXIT(rv);

    entry_offset = vsq_rt_cls_ndx;
    if (vsq_group_ndx == SOC_TMC_ITM_VSQ_GROUP_SRC_PORT) {
        /* VSQ-E table is doubled. The first half is for pool 0 and the second half is for pool 1 */
        entry_offset += pool_id * SOC_TMC_ITM_VSQ_NOF_RATE_CLASSES(unit);
    }

    if (vsq_group_ndx == SOC_TMC_ITM_VSQ_GROUP_PG) {
        pg_fc_fadt_fields.max_field = FADT_MAX_THf;
        pg_fc_fadt_fields.min_field = FADT_MIN_THf;
        pg_fc_fadt_fields.alpha_field = FADT_ADJUST_FACTORf;

        for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
            /* Set FADT SET threshold */
            rv = qax_itm_vsq_rjct_fadt_set(unit,
                    rsrc_type,
                    mem_arr_CGM_VSQ_FC_PRMS[rsrc_type][vsq_group_ndx], 
                    pg_fc_fadt_fields,
                    entry_offset,
                    info->fadt_size_fc[rsrc_type].set,
                    &exact_info->fadt_size_fc[rsrc_type].set);
            SOCDNX_IF_ERR_EXIT(rv);

            /* Set CLEAR threshold */
            rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                    rsrc_type,
                    mem_arr_CGM_VSQ_FC_PRMS[rsrc_type][vsq_group_ndx], 
                    FADT_OFFSETf, 
                    entry_offset,
                    info->fadt_size_fc[rsrc_type].clear_offset,
                    &exact_info->fadt_size_fc[rsrc_type].clear_offset);
            SOCDNX_IF_ERR_EXIT(rv);
        }
    } else {
        for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
            /* Set SET threshold */
            rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                    rsrc_type,
                    mem_arr_CGM_VSQ_FC_PRMS[rsrc_type][vsq_group_ndx], 
                    SET_THf, 
                    entry_offset,
                    info->size_fc[rsrc_type].set, 
                    &exact_info->size_fc[rsrc_type].set);
            SOCDNX_IF_ERR_EXIT(rv);

            /* Set CLEAR threshold */
            rv = qax_itm_vsq_rjct_man_exp_set(unit, 
                    rsrc_type,
                    mem_arr_CGM_VSQ_FC_PRMS[rsrc_type][vsq_group_ndx], 
                    CLR_THf, 
                    entry_offset,
                    info->size_fc[rsrc_type].clear, 
                    &exact_info->size_fc[rsrc_type].clear);
            SOCDNX_IF_ERR_EXIT(rv);
        }
    }

exit: 
    SOCDNX_FUNC_RETURN;
}

/*!
 * \brief
 *  Get the VSQ flow-control info according to vsq-group-id
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] vsq_group_ndx -
 *      The VSQ type.
 *  \param [in] vsq_rt_cls_ndx -
 *      The VSQ rate class.
 *  \param [in] pool_id -
 *      The pool id. Relevant for VSQ-E.
 *  \param [in] info -
 *      FC information.
 */
int
  qax_itm_vsq_fc_get(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP      vsq_group_ndx,
    SOC_SAND_IN  uint32                     vsq_rt_cls_ndx,
    SOC_SAND_IN  int                        pool_id,
    SOC_SAND_OUT SOC_TMC_ITM_VSQ_FC_INFO    *info
  )
{
    int rv = SOC_E_NONE;
    uint32 entry_offset = 0;
    const soc_mem_t mem_arr_CGM_VSQ_FC_PRMS[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES][SOC_TMC_NOF_VSQ_GROUPS] = 
        { {CGM_VSQA_WORDS_FC_PRMSm, CGM_VSQB_WORDS_FC_PRMSm, CGM_VSQC_WORDS_FC_PRMSm, CGM_VSQD_WORDS_FC_PRMSm, CGM_VSQE_WORDS_FC_PRMSm, CGM_VSQF_WORDS_FC_PRMSm},
          {CGM_VSQA_SRAM_BUFFERS_FC_PRMSm, CGM_VSQB_SRAM_BUFFERS_FC_PRMSm, CGM_VSQC_SRAM_BUFFERS_FC_PRMSm, CGM_VSQD_SRAM_BUFFERS_FC_PRMSm, CGM_VSQE_SRAM_BUFFERS_FC_PRMSm, CGM_VSQF_SRAM_BUFFERS_FC_PRMSm},
          {CGM_VSQA_SRAM_PDS_FC_PRMSm, CGM_VSQB_SRAM_PDS_FC_PRMSm, CGM_VSQC_SRAM_PDS_FC_PRMSm, CGM_VSQD_SRAM_PDS_FC_PRMSm, CGM_VSQE_SRAM_PDS_FC_PRMSm, CGM_VSQF_SRAM_PDS_FC_PRMSm} };
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = SOC_TMC_INGRESS_THRESHOLD_INVALID;
    SOC_TMC_ITM_VSQ_FADT_FIELDS_INFO pg_fc_fadt_fields;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    if (vsq_group_ndx < 0 || vsq_group_ndx >= SOC_TMC_NOF_VSQ_GROUPS) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (pool_id < 0 || pool_id >= SOC_TMC_ITM_NOF_RSRC_POOLS) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (vsq_rt_cls_ndx > SOC_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
        SOCDNX_IF_ERR_EXIT(SOC_E_PARAM);
    }

    entry_offset = vsq_rt_cls_ndx;
    if (vsq_group_ndx == SOC_TMC_ITM_VSQ_GROUP_SRC_PORT) {
        /* VSQ-E table is doubled. The first half is for pool 0 and the second half is for pool 1 */
        entry_offset += pool_id * SOC_TMC_ITM_VSQ_NOF_RATE_CLASSES(unit);
    }

    if (vsq_group_ndx == SOC_TMC_ITM_VSQ_GROUP_PG) {
        pg_fc_fadt_fields.max_field = FADT_MAX_THf;
        pg_fc_fadt_fields.min_field = FADT_MIN_THf;
        pg_fc_fadt_fields.alpha_field = FADT_ADJUST_FACTORf;

        for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
            /* Set FADT SET threshold */
            rv = qax_itm_vsq_rjct_fadt_get(unit,
                    rsrc_type,
                    mem_arr_CGM_VSQ_FC_PRMS[rsrc_type][vsq_group_ndx], 
                    pg_fc_fadt_fields,
                    entry_offset,
                    &info->fadt_size_fc[rsrc_type].set);
            SOCDNX_IF_ERR_EXIT(rv);

            /* Set CLEAR threshold */
            rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                    rsrc_type,
                    mem_arr_CGM_VSQ_FC_PRMS[rsrc_type][vsq_group_ndx], 
                    FADT_OFFSETf, 
                    entry_offset,
                    &info->fadt_size_fc[rsrc_type].clear_offset);
            SOCDNX_IF_ERR_EXIT(rv);
        }
    } else {
        for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
            /* Set SET threshold */
            rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                    rsrc_type,
                    mem_arr_CGM_VSQ_FC_PRMS[rsrc_type][vsq_group_ndx], 
                    SET_THf, 
                    entry_offset,
                    &info->size_fc[rsrc_type].set);
            SOCDNX_IF_ERR_EXIT(rv);

            /* Set CLEAR threshold */
            rv = qax_itm_vsq_rjct_man_exp_get(unit, 
                    rsrc_type,
                    mem_arr_CGM_VSQ_FC_PRMS[rsrc_type][vsq_group_ndx], 
                    CLR_THf, 
                    entry_offset,
                    &info->size_fc[rsrc_type].clear);
            SOCDNX_IF_ERR_EXIT(rv);
        }
    }

exit: 
    SOCDNX_FUNC_RETURN;
}

STATIC int
  soc_qax_itm_reserved_resource_init(
    int unit
  )
{
    soc_error_t rv;
    SOC_TMC_ITM_INGRESS_RESERVED_RESOURCE ingress_reserved_resource;
    int core_id;
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = SOC_TMC_INGRESS_THRESHOLD_INVALID;
    SOCDNX_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dpp.soc.jericho.tm.ingress_reserved_resource.alloc(unit, SOC_DPP_DEFS_MAX(NOF_CORES));
    SOCDNX_IF_ERR_EXIT(rv);

    SOC_DPP_CORES_ITER(SOC_CORE_ALL, core_id) {
        ingress_reserved_resource.dram_reserved = 0;
        ingress_reserved_resource.ocb_reserved = 0;
        for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
            ingress_reserved_resource.reserved[rsrc_type] = 0;
        }
        rv = sw_state_access[unit].dpp.soc.jericho.tm.ingress_reserved_resource.set(unit, core_id, &ingress_reserved_resource);
        SOCDNX_IF_ERR_EXIT(rv);
    }


exit:
    SOCDNX_FUNC_RETURN;
}

/* Get global resource allocation sizes */
int
  qax_itm_global_resource_allocation_get(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN int core, 
    SOC_SAND_OUT SOC_TMC_ITM_INGRESS_CONGESTION_MGMT *ingress_congestion_mgmt
  )
{
    uint32 res = SOC_E_NONE;
    SOC_TMC_ITM_INGRESS_RESERVED_RESOURCE ingress_reserved_resource;
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = SOC_TMC_INGRESS_THRESHOLD_INVALID;
    SOCDNX_INIT_FUNC_DEFS;

    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].total = CGM_ITM_WORDS_SIZE_MAX(unit, core);
    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].total = CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX(unit);
    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].total = CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX;

    res = sw_state_access[unit].dpp.soc.jericho.tm.ingress_reserved_resource.get(unit, core, &ingress_reserved_resource);
    SOCDNX_IF_ERR_EXIT(res);

    for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        ingress_congestion_mgmt->global[rsrc_type].reserved = ingress_reserved_resource.reserved[rsrc_type];
    }

    /* Get HW information */
    res = qax_itm_resource_allocation_get(unit, core, ingress_congestion_mgmt);
    SOCDNX_IF_ERR_EXIT(res);

exit: 
    SOCDNX_FUNC_RETURN; 
}

/* Set global resource allocation sizes */
/* The total memory will not be set */
int
  qax_itm_global_resource_allocation_set(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN int core, 
    SOC_SAND_IN SOC_TMC_ITM_INGRESS_CONGESTION_MGMT *ingress_congestion_mgmt
  )
{
    uint32 res = SOC_E_NONE;
    SOC_TMC_ITM_INGRESS_RESERVED_RESOURCE ingress_reserved_resource;
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = SOC_TMC_INGRESS_THRESHOLD_INVALID;
    SOCDNX_INIT_FUNC_DEFS;

    res = sw_state_access[unit].dpp.soc.jericho.tm.ingress_reserved_resource.get(unit, core, &ingress_reserved_resource);
    SOCDNX_IF_ERR_EXIT(res);

    for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        ingress_reserved_resource.reserved[rsrc_type] = ingress_congestion_mgmt->global[rsrc_type].reserved;
    }

    res = sw_state_access[unit].dpp.soc.jericho.tm.ingress_reserved_resource.set(unit, core, &ingress_reserved_resource);
    SOCDNX_IF_ERR_EXIT(res);

    /* Set HW information */
    res = qax_itm_resource_allocation_set(unit, core, ingress_congestion_mgmt);
    SOCDNX_IF_ERR_EXIT(res);

exit: 
    SOCDNX_FUNC_RETURN; 
}

STATIC int
  qax_itm_resource_allocation_verify(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int core,
    SOC_SAND_IN SOC_TMC_ITM_INGRESS_CONGESTION_MGMT *ingress_congestion_mgmt
  )
{
    SOCDNX_INIT_FUNC_DEFS;

    /* validate core */
    if (core != SOC_CORE_ALL && 
       (core < 0 || core >= SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Invalid core id (%d)"), core));
    }

    /* validate Words size */
    if (ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].pool_0 > CGM_ITM_WORDS_SIZE_MAX(unit, core)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Pool 0 Words size (%u) is too high"), 
                    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].pool_0));
    }
    if (ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].pool_1 > CGM_ITM_WORDS_SIZE_MAX(unit, core)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Pool 1 Words size (%u) is too high"), 
                    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].pool_1));
    }
    if (ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].headroom > CGM_ITM_WORDS_SIZE_MAX(unit, core)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Headroom Words size (%u) is too high"), 
                    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].headroom));
    }

    /* validate SRAM-Buffers size */
    if (ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].pool_0 > CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Pool 0 SRAM-Buffers size (%u) is too high"), 
                    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].pool_0));
    }
    if (ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].pool_1 > CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Pool 1 SRAM-Buffers size (%u) is too high"), 
                    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].pool_1));
    }
    if (ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].headroom > CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Headroom SRAM-Buffers size (%u) is too high"), 
                    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].headroom));
    }
    if (ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].nominal_headroom > CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Nominal Headroom SRAM-Buffers size (%u) is too high"), 
                    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].nominal_headroom));
    }
    if (ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].nominal_headroom > 
            ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].headroom) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Nominal Headroom SRAM-Buffers size (%u) can't be greater than Headroom size (%u)"), 
                    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].nominal_headroom, 
                    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].headroom));
    }

    /* validate SRAM-PDs size */
    if (ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].pool_0 > CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Pool 0 SRAM-PDs size (%u) is too high"), 
                    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].pool_0));
    }
    if (ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].pool_1 > CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Pool 1 SRAM-PDs size (%u) is too high"), 
                    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].pool_1));
    }
    if (ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].headroom > CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Headroom SRAM-PDs size (%u) is too high"), 
                    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].headroom));
    }
    if (ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].nominal_headroom > CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Nominal Headroom SRAM-PDs size (%u) is too high"), 
                    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].nominal_headroom));
    }
    if (ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].nominal_headroom > 
            ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].headroom) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Nominal Headroom SRAM-PDs size (%u) can't be greater than Headroom size (%u)"), 
                    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].nominal_headroom, 
                    ingress_congestion_mgmt->global[SOC_TMC_INGRESS_THRESHOLD_SRAM_PDS].headroom));
    }

exit: 
    SOCDNX_FUNC_RETURN; 
}

STATIC int
  qax_itm_resource_allocation_set(
    SOC_SAND_IN int                     unit,
    SOC_SAND_IN int                     core_id,
    SOC_SAND_IN SOC_TMC_ITM_INGRESS_CONGESTION_MGMT *ingress_congestion_mgmt
  ) 
{
    int rv = SOC_E_NONE;
    const soc_reg_t reg_arr_CGM_PB_VSQ_OC_TH[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        { CGM_PB_VSQ_WORDS_OC_THr, CGM_PB_VSQ_SRAM_BUFFERS_OC_THr, CGM_PB_VSQ_SRAM_PDS_OC_THr };
    const soc_field_t field_arr_PB_VSQ_SHRD_OC_TH_POOL_0[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        { PB_VSQ_WORDS_SHRD_OC_TH_POOL_0f, PB_VSQ_SRAM_BUFFERS_SHRD_OC_TH_POOL_0f, PB_VSQ_SRAM_PDS_SHRD_OC_TH_POOL_0f };
    const soc_field_t field_arr_PB_VSQ_SHRD_OC_TH_POOL_1[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        { PB_VSQ_WORDS_SHRD_OC_TH_POOL_1f, PB_VSQ_SRAM_BUFFERS_SHRD_OC_TH_POOL_1f, PB_VSQ_SRAM_PDS_SHRD_OC_TH_POOL_1f };
    const soc_field_t field_arr_PB_VSQ_HDRM_OC_TH_0[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        { -1, PB_VSQ_SRAM_BUFFERS_HDRM_OC_TH_0f, PB_VSQ_SRAM_PDS_HDRM_OC_TH_0f };
    const soc_field_t field_arr_PB_VSQ_HDRM_OC_TH_1_POOL_0[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        { -1, PB_VSQ_SRAM_BUFFERS_HDRM_OC_TH_1_POOL_0f, PB_VSQ_SRAM_PDS_HDRM_OC_TH_1_POOL_0f };
    const soc_field_t field_arr_PB_VSQ_HDRM_OC_TH_1_POOL_1[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        { -1, PB_VSQ_SRAM_BUFFERS_HDRM_OC_TH_1_POOL_1f, PB_VSQ_SRAM_PDS_HDRM_OC_TH_1_POOL_1f };
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = SOC_TMC_INGRESS_THRESHOLD_INVALID;
    soc_reg_above_64_val_t data;
    int lossless_pool = 0;
    uint32 hdrm_ext_size = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(ingress_congestion_mgmt);

    rv = qax_itm_resource_allocation_verify(unit, core_id, ingress_congestion_mgmt);
    SOCDNX_IF_ERR_EXIT(rv);

    for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        SOC_REG_ABOVE_64_CLEAR(data);

        rv = soc_reg_above_64_get(unit, reg_arr_CGM_PB_VSQ_OC_TH[rsrc_type], REG_PORT_ANY, 0, data);
        SOCDNX_IF_ERR_EXIT(rv);

        /* Set total pool 0 size */
        soc_reg_above_64_field32_set(unit, 
                                     reg_arr_CGM_PB_VSQ_OC_TH[rsrc_type], 
                                     data, 
                                     field_arr_PB_VSQ_SHRD_OC_TH_POOL_0[rsrc_type], 
                                     ingress_congestion_mgmt->global[rsrc_type].pool_0);

        /* Set total pool 1 size */
        soc_reg_above_64_field32_set(unit, 
                                     reg_arr_CGM_PB_VSQ_OC_TH[rsrc_type], 
                                     data, 
                                     field_arr_PB_VSQ_SHRD_OC_TH_POOL_1[rsrc_type], 
                                     ingress_congestion_mgmt->global[rsrc_type].pool_1);

        /* Set headroom size */
        if (rsrc_type == SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES) {
            /* Set total headroom Words size */
            soc_reg_above_64_field32_set(unit, 
                                         reg_arr_CGM_PB_VSQ_OC_TH[rsrc_type], 
                                         data, 
                                         PB_VSQ_WORDS_HDRM_OC_THf, 
                                         ingress_congestion_mgmt->global[rsrc_type].headroom);
        } else {
            /* Set nominal headroom sram size */
            soc_reg_above_64_field32_set(unit, 
                                         reg_arr_CGM_PB_VSQ_OC_TH[rsrc_type], 
                                         data, 
                                         field_arr_PB_VSQ_HDRM_OC_TH_0[rsrc_type], 
                                         ingress_congestion_mgmt->global[rsrc_type].nominal_headroom);

            /* 
             * All lossless PGs should be assign to the same pool,
             * so the headroom will be associates with only 1 pool,
             * i.e. pool_0 OR pool_1. The other pool headroom is 0.
             */
            rv = sw_state_access[unit].dpp.soc.qax.tm.lossless_pool_id.get(unit, core_id, &lossless_pool);
            SOCDNX_IF_ERR_EXIT(rv);

            hdrm_ext_size = ingress_congestion_mgmt->global[rsrc_type].headroom - ingress_congestion_mgmt->global[rsrc_type].nominal_headroom;

            /* Set headroom extension sram size for pool 0 */
            soc_reg_above_64_field32_set(unit, 
                                         reg_arr_CGM_PB_VSQ_OC_TH[rsrc_type], 
                                         data, 
                                         field_arr_PB_VSQ_HDRM_OC_TH_1_POOL_0[rsrc_type], 
                                         (lossless_pool == 0) ? (hdrm_ext_size) : 0);

            /* Set headroom extension sram size for pool 1 */
            soc_reg_above_64_field32_set(unit, 
                                         reg_arr_CGM_PB_VSQ_OC_TH[rsrc_type], 
                                         data, 
                                         field_arr_PB_VSQ_HDRM_OC_TH_1_POOL_1[rsrc_type], 
                                         (lossless_pool == 1) ? (hdrm_ext_size) : 0);
        }

        rv = soc_reg_above_64_set(unit, reg_arr_CGM_PB_VSQ_OC_TH[rsrc_type], core_id, 0, data);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit: 
    SOCDNX_FUNC_RETURN;
}

STATIC int
  qax_itm_resource_allocation_get(
    SOC_SAND_IN int                     unit,
    SOC_SAND_IN int                     core_id,
    SOC_SAND_OUT SOC_TMC_ITM_INGRESS_CONGESTION_MGMT *ingress_congestion_mgmt
  ) 
{
    int rv = SOC_E_NONE;
    const soc_reg_t reg_arr_CGM_PB_VSQ_OC_TH[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        { CGM_PB_VSQ_WORDS_OC_THr, CGM_PB_VSQ_SRAM_BUFFERS_OC_THr, CGM_PB_VSQ_SRAM_PDS_OC_THr };
    const soc_field_t field_arr_PB_VSQ_SHRD_OC_TH_POOL_0[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        { PB_VSQ_WORDS_SHRD_OC_TH_POOL_0f, PB_VSQ_SRAM_BUFFERS_SHRD_OC_TH_POOL_0f, PB_VSQ_SRAM_PDS_SHRD_OC_TH_POOL_0f };
    const soc_field_t field_arr_PB_VSQ_SHRD_OC_TH_POOL_1[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        { PB_VSQ_WORDS_SHRD_OC_TH_POOL_1f, PB_VSQ_SRAM_BUFFERS_SHRD_OC_TH_POOL_1f, PB_VSQ_SRAM_PDS_SHRD_OC_TH_POOL_1f };
    const soc_field_t field_arr_PB_VSQ_HDRM_OC_TH_0[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        { -1, PB_VSQ_SRAM_BUFFERS_HDRM_OC_TH_0f, PB_VSQ_SRAM_PDS_HDRM_OC_TH_0f };
    const soc_field_t field_arr_PB_VSQ_HDRM_OC_TH_1_POOL_0[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        { -1, PB_VSQ_SRAM_BUFFERS_HDRM_OC_TH_1_POOL_0f, PB_VSQ_SRAM_PDS_HDRM_OC_TH_1_POOL_0f };
    const soc_field_t field_arr_PB_VSQ_HDRM_OC_TH_1_POOL_1[SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        { -1, PB_VSQ_SRAM_BUFFERS_HDRM_OC_TH_1_POOL_1f, PB_VSQ_SRAM_PDS_HDRM_OC_TH_1_POOL_1f };
    soc_field_t lossless_pool_hdrm_ext_field;
    SOC_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = SOC_TMC_INGRESS_THRESHOLD_INVALID;
    soc_reg_above_64_val_t data;
    int lossless_pool = 0;
    uint32 hdrm_ext_size = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(ingress_congestion_mgmt);

    if (core_id != SOC_CORE_ALL && 
       (core_id < 0 || core_id >= SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Invalid core id (%d)"), core_id));
    }

    for (rsrc_type = 0; rsrc_type < SOC_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        SOC_REG_ABOVE_64_CLEAR(data);

        rv = soc_reg_above_64_get(unit, reg_arr_CGM_PB_VSQ_OC_TH[rsrc_type], REG_PORT_ANY, 0, data);
        SOCDNX_IF_ERR_EXIT(rv);

        /* Get total pool 0 size */
        ingress_congestion_mgmt->global[rsrc_type].pool_0 = 
            soc_reg_above_64_field32_get(unit, 
                                         reg_arr_CGM_PB_VSQ_OC_TH[rsrc_type], 
                                         data, 
                                         field_arr_PB_VSQ_SHRD_OC_TH_POOL_0[rsrc_type]
                                        );

        /* Get total pool 1 size */
        ingress_congestion_mgmt->global[rsrc_type].pool_1 = 
            soc_reg_above_64_field32_get(unit, 
                                         reg_arr_CGM_PB_VSQ_OC_TH[rsrc_type], 
                                         data, 
                                         field_arr_PB_VSQ_SHRD_OC_TH_POOL_1[rsrc_type]
                                        );

        /* Get headroom size */
        if (rsrc_type == SOC_TMC_INGRESS_THRESHOLD_TOTAL_BYTES) {
            /* Set total headroom Words size */
            ingress_congestion_mgmt->global[rsrc_type].headroom = 
                soc_reg_above_64_field32_get(unit, 
                                             reg_arr_CGM_PB_VSQ_OC_TH[rsrc_type], 
                                             data, 
                                             PB_VSQ_WORDS_HDRM_OC_THf
                                            );

            /* For Words, nominal_headroom == max_headroom */
            ingress_congestion_mgmt->global[rsrc_type].nominal_headroom = 
                ingress_congestion_mgmt->global[rsrc_type].headroom;
        } else {
            /* Get nominal headroom sram size */
            ingress_congestion_mgmt->global[rsrc_type].nominal_headroom = 
                soc_reg_above_64_field32_get(unit, 
                                             reg_arr_CGM_PB_VSQ_OC_TH[rsrc_type], 
                                             data, 
                                             field_arr_PB_VSQ_HDRM_OC_TH_0[rsrc_type]
                                            );

            /* 
             * All lossless PGs should be assign to the same pool,
             * so the headroom will be associates with only 1 pool,
             * i.e. pool_0 OR pool_1. The other pool headroom is 0.
             */
            rv = sw_state_access[unit].dpp.soc.qax.tm.lossless_pool_id.get(unit, core_id, &lossless_pool);
            SOCDNX_IF_ERR_EXIT(rv);

            lossless_pool_hdrm_ext_field = (lossless_pool == 0) ? 
                (field_arr_PB_VSQ_HDRM_OC_TH_1_POOL_0[rsrc_type]) : 
                (field_arr_PB_VSQ_HDRM_OC_TH_1_POOL_1[rsrc_type]);

            hdrm_ext_size = 
                soc_reg_above_64_field32_get(unit, 
                                             reg_arr_CGM_PB_VSQ_OC_TH[rsrc_type], 
                                             data, 
                                             lossless_pool_hdrm_ext_field
                                            );

            ingress_congestion_mgmt->global[rsrc_type].headroom = 
                ingress_congestion_mgmt->global[rsrc_type].nominal_headroom + hdrm_ext_size;
        }
    }

exit: 
    SOCDNX_FUNC_RETURN;
}

/*******************/
/* } VSQ Functions */
/*******************/


/*
 * Scheduler (credits) compensation configuration. 
 * compensation is configured dynamically with a value 
 * Per queue, Per in pp port  or Per OutlIF Profile 
 *
 * When called for per queue or per in pp port compensation, 
 * additional info contains compensation profile and compensation value for the other type 
 * (in pp port for queue and queue for in pp port),
 * Since HW is configured with a single value (sum of the two).
 */
int qax_itm_credits_adjust_size_set (
    SOC_SAND_IN int   unit,
    SOC_SAND_IN int   core,
    SOC_SAND_IN int   index, /* entity index, the entity depends on adjust_type */
    SOC_SAND_IN SOC_TMC_ITM_CGM_PKT_SIZE_ADJUST_TYPE adjust_type,
    SOC_SAND_IN int   delta,
    SOC_SAND_IN SOC_TMC_ITM_PKT_SIZE_ADJUST_INFO* additional_info
  ) 
{
    int core_id = 0;
    int delta_internal;
    int offset, delta_total;
    SOCDNX_INIT_FUNC_DEFS;
  
    /* verify delta is in range */
    if ((delta > QAX_ITM_SCHEDULER_DELTA_MAX) || (delta < QAX_ITM_SCHEDULER_DELTA_MIN)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Delta %d is out of range"), delta));
    }

    SOC_DPP_CORES_ITER(core, core_id) {

        if (adjust_type == SOC_TMC_ITM_PKT_SIZE_ADJUST_QUEUE || adjust_type == SOC_TMC_ITM_PKT_SIZE_ADJUST_PORT) { /* per queue (credit class) * ipp port (credit class) compensation */
            uint32 data;
            /* 
               the key to the table is [credit_class_profile(3bits) concat IPP-profile(3bits)]
            */
            data = 0;
            SOCDNX_NULL_CHECK(additional_info);
            
            if (adjust_type == SOC_TMC_ITM_PKT_SIZE_ADJUST_QUEUE) {
            
                SOCDNX_IF_ERR_EXIT(jer_itm_sched_compensation_offset_and_delta_get(unit, 
                                                                                   index, /* queue profile*/
                                                                                   delta,  /* queue delta */
                                                                                   additional_info->index, /* in pp_port profile */
                                                                                   additional_info->value, /* in pp_port delta */
                                                                                   &offset,
                                                                                   &delta_total));
            }  else {
                /* adjust_type == SOC_TMC_ITM_PKT_SIZE_ADJUST_PORT */
                SOCDNX_IF_ERR_EXIT(jer_itm_sched_compensation_offset_and_delta_get(unit, 
                                                                                   additional_info->index, /* queue profile*/
                                                                                   additional_info->value,  /* queue delta */
                                                                                   index, /* in pp_port profile */
                                                                                   delta, /* in pp_port delta */
                                                                                   &offset,
                                                                                   &delta_total));

            }

            /* convert value to 2's complement */
            delta_internal = CONVERT_SIGNED_NUM_TO_TWO_COMPLEMENT_METHOD(delta_total, EIGHT_BITS);

            soc_mem_field32_set(unit, CGM_SCH_CPMm, &data, HDR_DELTAf, delta_internal); /* this is the only field in the table -- no need to read the data */         
            SOCDNX_IF_ERR_EXIT(WRITE_CGM_SCH_CPMm(unit, CGM_BLOCK(unit, core_id), index, &data));

        } else if (adjust_type == SOC_TMC_ITM_PKT_SIZE_ADJUST_APPEND_SIZE_PTR) { /* per OutLif profile (header append pointer) compensation */
            soc_reg_above_64_val_t above64;
            /* 
               The key to the table is 8 bits.
               5 lower bits is outlif, provided by user. 
               The 3 upper bits should be zero

               -   Nwk-Header-Append(8) =   (Cfg-Append-Mode == 0) ? 0 :
               (Cfg-Append-Mode == 1) ? User-Header-1[15:8] :  {3'b0, Out-LIF[17:13]}

               we work with mode = 2
            */
            
            if (index > CGM_HAP_NOF_ENTRIES) {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Invalid append pointer id %d"), index));
            }

            /* convert value to 2's complement */
            delta_internal = CONVERT_SIGNED_NUM_TO_TWO_COMPLEMENT_METHOD(delta, QAX_ITM_SCHEDULER_DELTA_NOF_BITS);

            SOCDNX_IF_ERR_EXIT(READ_CGM_HAPMm(unit, CGM_BLOCK(unit, core_id), index, above64));
            soc_mem_field32_set(unit, CGM_HAPMm, above64, SCH_DELTAf, delta_internal);          
            SOCDNX_IF_ERR_EXIT(WRITE_CGM_HAPMm(unit, CGM_BLOCK(unit, core_id), index, above64));

        } else {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Invalid adjust_type %d"), adjust_type));
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Scheduler (credits) compensation configuration. 
 * 2 delta(compensations) are configured dynamically with a value 
 * Per queue and Per OutlIF Profile 
 */
int qax_itm_credits_adjust_size_get (
    SOC_SAND_IN int   unit,
    SOC_SAND_IN int   core,
    SOC_SAND_IN int   index, /* entity index, the entity depends on adjust_type */
    SOC_SAND_IN SOC_TMC_ITM_CGM_PKT_SIZE_ADJUST_TYPE adjust_type,
    SOC_SAND_OUT int   *delta
  ) 
{
    int core_id = 0;
    int delta_internal;

    SOCDNX_INIT_FUNC_DEFS;
  
    SOCDNX_NULL_CHECK(delta);

    core_id = ((core == SOC_CORE_ALL) ? 0 : core);

    /* per queue and per port compensation values are obtained from SW (template manager) */
    
    if (adjust_type == SOC_TMC_ITM_PKT_SIZE_ADJUST_APPEND_SIZE_PTR) { /* per OutLif profile (header append pointer) compensation */

        soc_reg_above_64_val_t above64;
        /* 
           The key to the table is 8 bits.
           5 lower bits is outlif, provided by user. 
           The 3 upper bits should be zero
        */
        if (index >= CGM_HAP_NOF_ENTRIES) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Invalid append pointer id %d"), index));
        }

        SOCDNX_IF_ERR_EXIT(READ_CGM_HAPMm(unit, CGM_BLOCK(unit, core_id), index, above64));
        delta_internal = soc_mem_field32_get(unit, CGM_HAPMm, above64, SCH_DELTAf);          

    } else {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Invalid adjust_type %d"), adjust_type));
    }


    /* convert from 2's complement to value */
    *delta = CONVERT_TWO_COMPLEMENT_INTO_SIGNED_NUM(delta_internal, QAX_ITM_SCHEDULER_DELTA_NOF_BITS);

exit:
    SOCDNX_FUNC_RETURN;
}

int
  qax_ingress_drop_status(
    SOC_SAND_IN int   unit,
    SOC_SAND_OUT uint32 *is_max_size 
  )
{
    uint64 val;
    uint32 is_words_max_size = 0, is_sram_words_max_size = 0, is_sram_pds_max_size = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(READ_CGM_REJECT_STATUS_BITMAPr(unit, &val)); 
    is_words_max_size = soc_reg64_field32_get(unit, CGM_REJECT_STATUS_BITMAPr, val, VOQ_WORDS_MAX_SIZE_RJCTf);
    is_sram_words_max_size = soc_reg64_field32_get(unit, CGM_REJECT_STATUS_BITMAPr, val, VOQ_SRAM_WORDS_MAX_SIZE_RJCTf);
    is_sram_pds_max_size = soc_reg64_field32_get(unit, CGM_REJECT_STATUS_BITMAPr, val, VOQ_SRAM_PDS_MAX_SIZE_RJCTf);
    if (is_words_max_size != 0 || is_sram_words_max_size != 0 || is_sram_pds_max_size != 0) {
        *is_max_size = 1;
    } else {
        *is_max_size = 0;
    }
    
exit:
  SOCDNX_FUNC_RETURN;
}


#undef _ERR_MSG_MODULE_NAME

