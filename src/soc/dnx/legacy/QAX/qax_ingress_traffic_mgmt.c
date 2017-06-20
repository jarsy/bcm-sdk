/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: jer2_qax_ingress_traffic_mgmt.c
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

/*************
 * INCLUDES  *
 *************/
#include <shared/bsl.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/QAX/qax_ingress_traffic_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_ingress_traffic_mgmt.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dnx/legacy/mbcm.h>

/***********
 * DEFINES *
 * {       *
 ***********/

/* VSQ man-exp field's number of mantissa bits */
#define CGM_ITM_VSQ_MANTISSA_BITS  8
/* VSQ Words 16B resolution */
#define CGM_ITM_VSQ_WORDS_RESOLUTION    16

#define JER2_QAX_ITM_GRNT_BYTES_MAX (0xFFFFFFFF) /*(16 * 256 * 1024 * 1024 -1)*/ /* 2^28 words -- in bytes */
#define JER2_QAX_ITM_GRNT_SRAM_BYTES_MAX (16 * 256 * 1024) /* 2^18 words -- in bytes */
#define JER2_QAX_ITM_GRNT_SRAM_PDS_MAX (32 * 1024) /* 2^15 */

#define JER2_QAX_ITM_QUEUE_SIZE_BYTES_MAX (0xFFFFFFFF) /*(16 * 256 * 1024 * 1024 - 1)*/ /* 2^28 words -- in bytes */
#define JER2_QAX_ITM_QUEUE_SIZE_SRAM_BYTES_MAX (16 * 256 * 1024) /* 2^18 words -- in bytes */
#define JER2_QAX_ITM_QUEUE_SIZE_SRAM_PDS_MAX (32 * 1024) /* 2^15 */

#define JER2_QAX_ITM_WRED_QT_DP_INFO_MAX_PROBABILITY_MAX 100
#define JER2_QAX_ITM_Q_WRED_INFO_MIN_AVRG_TH_MAX (3 * 0x80000000)
#define JER2_QAX_ITM_Q_WRED_INFO_MAX_AVRG_TH_MAX (3 * 0x80000000)


/* Up to 192M Words (Word=16B) */
/* Number of buffers * buffer size in bytes / word size */

#define CGM_ITM_VSQ_WORDS_SIZE_MAX(unit)         (-1 /*SOC_DNX_CONFIG(unit)->jer2_jer->dbuffs.dbuffs_bdries.fbc_mnmc_0.size \
                                                    * (SOC_DNX_CONFIG(unit)->jer2_arad->init.dram.dbuff_size / CGM_ITM_VSQ_WORDS_RESOLUTION)*/)
/* 16K SRAM-Buffers (Buffer=256B) */
#define CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX  0x3FFF
/* 32K SRAM-PDs */
#define CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX      0x7FFF 
/* 16K SRAM-PDBs (PDB=2*PD) */
#define CGM_ITM_VSQ_SRAM_PDBS_SIZE_MAX     0x3FFF 
/* 96K DRAM-BDBs (BDB=8BD) */
#define CGM_ITM_VSQ_DRAM_BDBS_SIZE_MAX     0x17FFF

#define ITM_FADT_MAX_ALPHA  (7)
#define ITM_FADT_MIN_ALPHA  (-7)

#define JER2_QAX_ITM_VSQ_PG_OFFSET_FIELD_SIZE  48


/* scheduler delta range */
#define JER2_QAX_ITM_SCHEDULER_DELTA_MAX                 (127)
#define JER2_QAX_ITM_SCHEDULER_DELTA_MIN                 (-128)
#define JER2_QAX_ITM_SCHEDULER_DELTA_NOF_BITS            (8)
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
#define JER2_QAX_SRAM_PDS_IN_GUARANTEED       0x1
#define JER2_QAX_SRAM_WORDS_IN_GUARANTEED     0x2
#define JER2_QAX_WORDS_IN_GUARANTEED          0x4

/*
 * This enum is used to define each bit in the CGM_<..>_RJCT_MASKm and is currently internal, 
 * if in the future an API is needed to control those Reject masks the enum needs to move from 
 * C file to relevant header. 
 */
typedef enum {
    JER2_QAX_VOQ_SRAM_PDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT          = 0,
    JER2_QAX_VOQ_SRAM_PDS_MAX_SIZE_REJECT_BIT                        = 1,
    JER2_QAX_VOQ_SRAM_WORDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT        = 2,
    JER2_QAX_VOQ_SRAM_WORDS_MAX_SIZE_REJECT_BIT                      = 3,
    JER2_QAX_VOQ_WORDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT             = 4,
    JER2_QAX_VOQ_WORDS_MAX_SIZE_REJECT_BIT                           = 5,
    JER2_QAX_VOQ_SYSTEM_RED_REJECT_BIT                               = 6,
    JER2_QAX_VOQ_WRED_REJECT_BIT                                     = 7,
    JER2_QAX_VOQ_DRAM_BLOCK_REJECT_BIT                               = 8,
    JER2_QAX_PB_VSQ_SRAM_PDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT       = 9,
    JER2_QAX_PB_VSQ_SRAM_PDS_MAX_SIZE_REJECT_BIT                     = 10,
    JER2_QAX_VSQ_D_SRAM_PDS_MAX_SIZE_REJECT_BIT                      = 11,
    JER2_QAX_VSQ_C_SRAM_PDS_MAX_SIZE_REJECT_BIT                      = 12,
    JER2_QAX_VSQ_B_SRAM_PDS_MAX_SIZE_REJECT_BIT                      = 13,
    JER2_QAX_VSQ_A_SRAM_PDS_MAX_SIZE_REJECT_BIT                      = 14,
    JER2_QAX_PB_VSQ_SRAM_BUFFERS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT   = 15,
    JER2_QAX_PB_VSQ_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT                 = 16,
    JER2_QAX_VSQ_D_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT                  = 17,
    JER2_QAX_VSQ_C_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT                  = 18,
    JER2_QAX_VSQ_B_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT                  = 19,
    JER2_QAX_VSQ_A_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT                  = 20,
    JER2_QAX_PB_VSQ_WORDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT          = 21,
    JER2_QAX_PB_VSQ_WORDS_MAX_SIZE_REJECT_BIT                        = 22,
    JER2_QAX_VSQ_D_WORDS_MAX_SIZE_REJECT_BIT                         = 23,
    JER2_QAX_VSQ_C_WORDS_MAX_SIZE_REJECT_BIT                         = 24,
    JER2_QAX_VSQ_B_WORDS_MAX_SIZE_REJECT_BIT                         = 25,
    JER2_QAX_VSQ_A_WORDS_MAX_SIZE_REJECT_BIT                         = 26,
    JER2_QAX_VSQ_F_WORDS_WRED_REJECT_BIT                             = 27,
    JER2_QAX_VSQ_E_WORDS_WRED_REJECT_BIT                             = 28,
    JER2_QAX_VSQ_D_WORDS_WRED_REJECT_BIT                             = 29,
    JER2_QAX_VSQ_C_WORDS_WRED_REJECT_BIT                             = 30,
    JER2_QAX_VSQ_B_WORDS_WRED_REJECT_BIT                             = 31,
    JER2_QAX_VSQ_A_WORDS_WRED_REJECT_BIT                             = 32,
    JER2_QAX_DRAM_BDBS_OCCUPANCY_REJECT_BIT                          = 33,
    JER2_QAX_SRAM_BUFFERS_OCCUPANCY_REJECT_BIT                       = 34,
    JER2_QAX_SRAM_PDBS_OCCUPANCY_REJECT_BIT                          = 35,
    JER2_QAX_CFG_MAX_DP_LEVEL_REJECT_BIT                             = 36,
    JER2_QAX_CGM_NOF_ADMISSION_TESTS                                 = 37
} cgm_reject_admission_tests_e;

/***********
 * DEFINES *
 * }       *
 ***********/

/********************
 * static FUNCTIONS *
 * {                *
 ********************/

static  uint32 
  _jer2_qax_itm_alpha_to_field(
     int     unit,
     int     alpha
  );

static int 
  _jer2_qax_itm_field_to_alpha(
     int     unit,
     uint32 alpha_field
  );

static int
jer2_qax_itm_drop_global_thresholds_init(int unit);

static uint32
  jer2_qax_itm_vsq_WRED_QT_DP_INFO_to_WRED_TBL_DATA(
     DNX_SAND_IN int                                                     unit,
     DNX_SAND_IN DNX_TMC_ITM_WRED_QT_DP_INFO                            *wred_param,
     DNX_SAND_INOUT DNX_TMC_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA *tbl_data
  );

static uint32
  jer2_qax_itm_vsq_WRED_TBL_DATA_to_WRED_QT_DP_INFO(
     DNX_SAND_IN int                                                     unit,
     DNX_SAND_IN  DNX_TMC_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA  *tbl_data,
     DNX_SAND_OUT DNX_TMC_ITM_WRED_QT_DP_INFO                            *wred_param
  );

static uint32 
  _jer2_qax_itm_mantissa_exp_field_set(
    int                  unit,
    itm_mantissa_exp_threshold_info* info,
    int round_up,
    void *data,
    uint32 threshold, 
    uint32* result_threshold
    );

static int
  jer2_qax_itm_vsq_rjct_man_exp_set(
    int                                 unit,
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
    soc_mem_t                           mem,
    soc_field_t                         field,
    uint32                              entry_offset,
    uint32                              threshold,
    uint32                              *result_threshold
  );

static int
  jer2_qax_itm_vsq_rjct_man_exp_get(
    int                                 unit,
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
    soc_mem_t                           mem,
    soc_field_t                         field,
    uint32                              entry_offset,
    uint32                              *result_threshold
  );

static int
  jer2_qax_itm_vsq_rjct_fadt_set(
    int                              unit,
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type,
    soc_mem_t                        mem,
    DNX_TMC_ITM_VSQ_FADT_FIELDS_INFO fadt_fields,
    uint32                           entry_offset,
    DNX_TMC_ITM_VSQ_FADT_INFO        fadt_info,
    DNX_TMC_ITM_VSQ_FADT_INFO        *exact_fadt_info
  );

static int
  jer2_qax_itm_vsq_rjct_fadt_get(
    int                              unit,
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type,
    soc_mem_t                        mem,
    DNX_TMC_ITM_VSQ_FADT_FIELDS_INFO fadt_fields,
    uint32                           entry_offset,
    DNX_TMC_ITM_VSQ_FADT_INFO        *exact_fadt_info
  );

static int
  jer2_qax_itm_vsq_pg_headroom_rjct_set(
    int                                 unit,
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
    soc_mem_t                           mem,
    uint32                              entry_offset,
    DNX_TMC_ITM_VSQ_PG_HDRM_INFO        headroom_info,
    DNX_TMC_ITM_VSQ_PG_HDRM_INFO        *exact_headroom_info
  );

static int
  jer2_qax_itm_vsq_pg_headroom_rjct_get(
    int                                 unit,
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
    soc_mem_t                           mem,
    uint32                              entry_offset,
    DNX_TMC_ITM_VSQ_PG_HDRM_INFO        *exact_headroom_info
  );

static int
  jer2_qax_itm_vsq_qt_rt_cls_verify(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_NDX    vsq_in_group_ndx
  );

static int 
  jer2_qax_itm_category_rngs_verify( 
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  DNX_TMC_ITM_CATEGORY_RNGS *info 
  ); 

static int
  jer2_qax_itm_glob_rcs_drop_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  DNX_TMC_ITM_GLOB_RCS_DROP_TH *info
  );

/********************
 * static FUNCTIONS *
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
    5, /* exp_bits */
    16 /* factor */
};




static  uint32 _jer2_qax_itm_alpha_to_field(
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

static int _jer2_qax_itm_field_to_alpha(
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


static uint32  _jer2_qax_itm_mantissa_exp_field_set(
    int                  unit,
    itm_mantissa_exp_threshold_info* info,
    int round_up,
    void *data,
    uint32 threshold, 
    uint32* result_threshold
    )
{
    uint32    exp_man, mnt = 0, exp = 0;
    
    DNXC_INIT_FUNC_DEFS;

    if (round_up) {
        DNXC_SAND_IF_ERR_EXIT(dnx_sand_break_to_mnt_exp_round_up(
                                    DNX_SAND_DIV_ROUND_UP(threshold, info->factor),
                                    info->mantissa_bits,
                                    info->exp_bits,
                                    0, &mnt, &exp
                                    ));
    } else {
        DNXC_SAND_IF_ERR_EXIT(dnx_sand_break_to_mnt_exp_round_down(
                                    threshold/info->factor,
                                    info->mantissa_bits,
                                    info->exp_bits,
                                    0, &mnt, &exp
                                    ));
    }

    /* Write them according to the rate class entry */
    jer2_arad_iqm_mantissa_exponent_set(unit, mnt, exp, info->mantissa_bits, &exp_man);
    soc_mem_field32_set(unit, info->mem_id, data, info->field_id, exp_man);
    *result_threshold = mnt * (info->factor << exp);

exit:
    DNXC_FUNC_RETURN;
}

static void  _jer2_qax_itm_mantissa_exp_field_get(
    int                  unit,
    itm_mantissa_exp_threshold_info* info,
    void *data,
    uint32* result_threshold
    )
{
    uint32    exp_man, mnt = 0, exp = 0;
    
    exp_man = soc_mem_field32_get(unit, info->mem_id, data, info->field_id);
    jer2_arad_iqm_mantissa_exponent_get(unit, exp_man, info->mantissa_bits, &mnt, &exp);
    *result_threshold = mnt * (info->factor << exp);
}



/* ****************************************************
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
int jer2_qax_itm_cgm_guaranteed_reject_mask_create(int unit, int is_voq, int is_over_voq, int is_over_vsq, int words_in_guaranteed, int sram_words_in_guaranteed, int sram_pds_in_guaranteed, SHR_BITDCL* mem_mask)
{
    int in_guaranteed = words_in_guaranteed || sram_words_in_guaranteed || sram_pds_in_guaranteed;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(mem_mask);

    SHR_BITCLR_RANGE(mem_mask, 0, JER2_QAX_CGM_NOF_ADMISSION_TESTS);
    SHR_BITWRITE(mem_mask, JER2_QAX_VOQ_SRAM_PDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT,          (!((is_over_voq && in_guaranteed) || sram_pds_in_guaranteed)));
    SHR_BITWRITE(mem_mask, JER2_QAX_VOQ_SRAM_PDS_MAX_SIZE_REJECT_BIT,                        (!((is_over_voq && in_guaranteed) || sram_pds_in_guaranteed)));
    SHR_BITWRITE(mem_mask, JER2_QAX_VOQ_SRAM_WORDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT,        (!((is_over_voq && in_guaranteed) || sram_words_in_guaranteed)));
    SHR_BITWRITE(mem_mask, JER2_QAX_VOQ_SRAM_WORDS_MAX_SIZE_REJECT_BIT,                      (!((is_over_voq && in_guaranteed) || sram_words_in_guaranteed)));
    SHR_BITWRITE(mem_mask, JER2_QAX_VOQ_WORDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT,             (!((is_over_voq && in_guaranteed) || words_in_guaranteed)));
    SHR_BITWRITE(mem_mask, JER2_QAX_VOQ_WORDS_MAX_SIZE_REJECT_BIT,                           (!((is_over_voq && in_guaranteed) || words_in_guaranteed)));
    SHR_BITWRITE(mem_mask, JER2_QAX_VOQ_SYSTEM_RED_REJECT_BIT,                               1);
    SHR_BITWRITE(mem_mask, JER2_QAX_VOQ_WRED_REJECT_BIT,                                     (!(is_voq && in_guaranteed)));
    SHR_BITWRITE(mem_mask, JER2_QAX_VOQ_DRAM_BLOCK_REJECT_BIT,                               1);
    SHR_BITWRITE(mem_mask, JER2_QAX_PB_VSQ_SRAM_PDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT,       (!(is_voq ? (is_over_vsq && sram_pds_in_guaranteed) : ((is_over_vsq && in_guaranteed) || sram_pds_in_guaranteed))));
    SHR_BITWRITE(mem_mask, JER2_QAX_PB_VSQ_SRAM_PDS_MAX_SIZE_REJECT_BIT,                     (!(is_voq ? (is_over_vsq && sram_pds_in_guaranteed) : ((is_over_vsq && in_guaranteed) || sram_pds_in_guaranteed))));
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_D_SRAM_PDS_MAX_SIZE_REJECT_BIT,                      !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_C_SRAM_PDS_MAX_SIZE_REJECT_BIT,                      !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_B_SRAM_PDS_MAX_SIZE_REJECT_BIT,                      !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_A_SRAM_PDS_MAX_SIZE_REJECT_BIT,                      !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_PB_VSQ_SRAM_BUFFERS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT,   (!(is_voq ? (is_over_vsq && sram_words_in_guaranteed) : ((is_over_vsq && in_guaranteed) || sram_words_in_guaranteed))));
    SHR_BITWRITE(mem_mask, JER2_QAX_PB_VSQ_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT,                 (!(is_voq ? (is_over_vsq && sram_words_in_guaranteed) : ((is_over_vsq && in_guaranteed) || sram_words_in_guaranteed))));
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_D_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT,                  !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_C_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT,                  !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_B_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT,                  !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_A_SRAM_BUFFERS_MAX_SIZE_REJECT_BIT,                  !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_PB_VSQ_WORDS_TOTAL_SHARED_OCCUPANCY_REJECT_BIT,          (!(is_voq ? (is_over_vsq && words_in_guaranteed) : ((is_over_vsq && in_guaranteed) || words_in_guaranteed))));
    SHR_BITWRITE(mem_mask, JER2_QAX_PB_VSQ_WORDS_MAX_SIZE_REJECT_BIT,                        (!(is_voq ? (is_over_vsq && words_in_guaranteed) : ((is_over_vsq && in_guaranteed) || words_in_guaranteed))));
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_D_WORDS_MAX_SIZE_REJECT_BIT,                         !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_C_WORDS_MAX_SIZE_REJECT_BIT,                         !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_B_WORDS_MAX_SIZE_REJECT_BIT,                         !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_A_WORDS_MAX_SIZE_REJECT_BIT,                         !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_F_WORDS_WRED_REJECT_BIT,                             !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_E_WORDS_WRED_REJECT_BIT,                             !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_D_WORDS_WRED_REJECT_BIT,                             !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_C_WORDS_WRED_REJECT_BIT,                             !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_B_WORDS_WRED_REJECT_BIT,                             !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_VSQ_A_WORDS_WRED_REJECT_BIT,                             !in_guaranteed);
    SHR_BITWRITE(mem_mask, JER2_QAX_DRAM_BDBS_OCCUPANCY_REJECT_BIT,                          1);
    SHR_BITWRITE(mem_mask, JER2_QAX_SRAM_BUFFERS_OCCUPANCY_REJECT_BIT,                       1);
    SHR_BITWRITE(mem_mask, JER2_QAX_SRAM_PDBS_OCCUPANCY_REJECT_BIT,                          1);
    SHR_BITWRITE(mem_mask, JER2_QAX_CFG_MAX_DP_LEVEL_REJECT_BIT,                             1);
        
exit:
    DNXC_FUNC_RETURN;
}

/* ****************************************************
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
int jer2_qax_itm_admission_tests_defaults_set (int unit)
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
    SHR_BITDCLNAME(mem_mask, JER2_QAX_CGM_NOF_ADMISSION_TESTS);

    DNXC_INIT_FUNC_DEFS;

    enforce_admission_test_loosly = (SOC_DNX_CONFIG(unit)->jer2_jer->tm.cgm_mgmt_guarantee_mode == DNX_TMC_ITM_CGM_MGMT_GUARANTEE_LOOSE);
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
        words_in_guaranteed = mem_index & JER2_QAX_WORDS_IN_GUARANTEED;
        sram_words_in_guaranteed = mem_index & JER2_QAX_SRAM_WORDS_IN_GUARANTEED;
        sram_pds_in_guaranteed = mem_index & JER2_QAX_SRAM_PDS_IN_GUARANTEED;
        DNXC_IF_ERR_EXIT( jer2_qax_itm_cgm_guaranteed_reject_mask_create(unit, 1, voq_over_voq, voq_over_vsq, words_in_guaranteed, sram_words_in_guaranteed, sram_pds_in_guaranteed, mem_mask));
        soc_mem_write(unit, mem, MEM_BLOCK_ALL, mem_index, mem_mask);
    }

    /* Set reject mask for CGM_VSQ_GRNTD_RJCT_MASK */
    max_mem_index = soc_mem_index_max(unit, CGM_VSQ_GRNTD_RJCT_MASKm);
    mem = CGM_VSQ_GRNTD_RJCT_MASKm;
    for (mem_index = 0; mem_index <= max_mem_index; ++mem_index) 
    {
        words_in_guaranteed = mem_index & JER2_QAX_WORDS_IN_GUARANTEED;
        sram_words_in_guaranteed = mem_index & JER2_QAX_SRAM_WORDS_IN_GUARANTEED;
        sram_pds_in_guaranteed = mem_index & JER2_QAX_SRAM_PDS_IN_GUARANTEED;
        DNXC_IF_ERR_EXIT( jer2_qax_itm_cgm_guaranteed_reject_mask_create(unit, 0, vsq_over_voq, vsq_over_vsq, words_in_guaranteed, sram_words_in_guaranteed, sram_pds_in_guaranteed, mem_mask));
        soc_mem_write(unit, mem, MEM_BLOCK_ALL, mem_index, mem_mask);
    }

    /* Set Lossless (8) and default (0) */
    SHR_BITSET_RANGE(mem_mask, 0, JER2_QAX_CGM_NOF_ADMISSION_TESTS);
    WRITE_CGM_PB_VSQ_RJCT_MASKm(unit, MEM_BLOCK_ALL, 0, mem_mask);
    SHR_BITCLR_RANGE(mem_mask, 0, JER2_QAX_DRAM_BDBS_OCCUPANCY_REJECT_BIT);
    WRITE_CGM_PB_VSQ_RJCT_MASKm(unit, MEM_BLOCK_ALL, 8, mem_mask);

    /* Set PP Admission profiles ECN/ECI (1) and default (0) */
    SHR_BITSET_RANGE(mem_mask, 0, JER2_QAX_CGM_NOF_ADMISSION_TESTS);
    WRITE_CGM_PP_RJCT_MASKm(unit, MEM_BLOCK_ALL, 0, mem_mask);
    SHR_BITCLR_RANGE(mem_mask, 0, JER2_QAX_CGM_NOF_ADMISSION_TESTS);
    SHR_BITSET_RANGE(mem_mask, 0, JER2_QAX_CGM_NOF_ADMISSION_TESTS);
    SHR_BITCLR(mem_mask, JER2_QAX_VOQ_WRED_REJECT_BIT);
    SHR_BITCLR(mem_mask, JER2_QAX_VSQ_A_WORDS_MAX_SIZE_REJECT_BIT);
    SHR_BITCLR(mem_mask, JER2_QAX_VSQ_B_WORDS_MAX_SIZE_REJECT_BIT);
    SHR_BITCLR(mem_mask, JER2_QAX_VSQ_C_WORDS_MAX_SIZE_REJECT_BIT);
    SHR_BITCLR(mem_mask, JER2_QAX_VSQ_D_WORDS_MAX_SIZE_REJECT_BIT);
    SHR_BITCLR(mem_mask, JER2_QAX_VSQ_A_WORDS_WRED_REJECT_BIT);
    SHR_BITCLR(mem_mask, JER2_QAX_VSQ_B_WORDS_WRED_REJECT_BIT);
    SHR_BITCLR(mem_mask, JER2_QAX_VSQ_C_WORDS_WRED_REJECT_BIT);
    SHR_BITCLR(mem_mask, JER2_QAX_VSQ_D_WORDS_WRED_REJECT_BIT);
    SHR_BITCLR(mem_mask, JER2_QAX_VSQ_E_WORDS_WRED_REJECT_BIT);
    SHR_BITCLR(mem_mask, JER2_QAX_VSQ_F_WORDS_WRED_REJECT_BIT);
    WRITE_CGM_PP_RJCT_MASKm(unit, MEM_BLOCK_ALL, 1, mem_mask);

exit:
    DNXC_FUNC_RETURN;
}


static int jer2_qax_itm_scheduler_compensation_init(
    DNX_SAND_IN  int unit
    )
{
    int index;
    uint32 table_data[10];
    uint32 data;
    uint64 data64;
    int cgm_delta_signs_reg;

    DNXC_INIT_FUNC_DEFS;

    /* Scheduler compensation init
     * Only 8 first profiles are used so credit class is mapped 1-1 to credit class profile
     */
    sal_memset(table_data, 0x0, sizeof(table_data));
    for (index = 0; index < 32; index++) { /* enable all types for all profiles, including those we don't plan to use */
        /* when compensation tag exists(new mechanism) we enable all compensations types */
        if (DNX_TMC_ITM_COMPENSATION_LEGACY_MODE(unit)) { 
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
        DNXC_IF_ERR_EXIT(WRITE_CGM_SCH_HCMm(unit, MEM_BLOCK_ALL, index, table_data));
    }

    for (index = 0; index < 8; index++) { /* we are going to use 8 first profiles only to allow 1-1 mapping on the second mapping level */

        /* map VOQ credit class to compensation profile  (1-1 mapping)*/
        
        DNXC_IF_ERR_EXIT(READ_CGM_VOQ_HCPm(unit, MEM_BLOCK_ANY , index, table_data));
        soc_mem_field32_set(unit, CGM_VOQ_HCPm, table_data, SCH_PROFILEf, index); 
        DNXC_IF_ERR_EXIT(WRITE_CGM_VOQ_HCPm(unit, MEM_BLOCK_ALL, index, table_data));
    }

    /* negate delta - packet size compensation feature */
    if (SOC_IS_QUX(unit)) {
        cgm_delta_signs_reg = CGM_REG_04F2r; /* CGM_DELTA_SIGNS */
    } else {
        cgm_delta_signs_reg = CGM_REG_04F5r; /* CGM_DELTA_SIGNS */
    }

    DNXC_IF_ERR_EXIT(soc_reg32_get(unit, cgm_delta_signs_reg, REG_PORT_ANY, 0, &data)); /* CGM_DELTA_SIGNS */
    soc_reg_field_set(unit, cgm_delta_signs_reg, &data, FIELD_7_7f, TRUE); /* CreditHdrTruncateNeg */
    soc_reg_field_set(unit, cgm_delta_signs_reg, &data, FIELD_8_8f, TRUE); /* CreditIrppDeltaNeg */
    DNXC_IF_ERR_EXIT(soc_reg32_set(unit, cgm_delta_signs_reg, REG_PORT_ANY, 0, data));

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
    DNXC_IF_ERR_EXIT(READ_IHB_LBP_GENERAL_CONFIG_0r(unit, SOC_CORE_ALL, &data64)); 
    soc_reg64_field32_set(unit, IHB_LBP_GENERAL_CONFIG_0r, &data64, NWK_HDR_TRUNCATE_MODEf, 0x2);
    soc_reg64_field32_set(unit, IHB_LBP_GENERAL_CONFIG_0r, &data64, NWK_HDR_APPEND_MODEf, 0x2);
    DNXC_IF_ERR_EXIT(WRITE_IHB_LBP_GENERAL_CONFIG_0r(unit, SOC_CORE_ALL, data64));

exit:
    DNXC_FUNC_RETURN;

}

int
  jer2_qax_itm_init(
    DNX_SAND_IN  int  unit
  )
{
    uint32                 data = 0;
    soc_reg_above_64_val_t above64;
    uint64                 val64;
    int                    res;
    int                    core_index; 

    DNXC_INIT_FUNC_DEFS;

    data = 0x0;
    if (!SOC_IS_QUX(unit)) {
    soc_mem_field32_set(unit, IPS_CRVSm, &data, CR_VAL_BMPf, 0x0); 
    res = jer2_arad_fill_table_with_entry(unit, IPS_CRVSm, MEM_BLOCK_ANY, &data);
    DNXC_IF_ERR_EXIT(res);

    data = 0;
    res = READ_IPS_CREDIT_CONFIGr(unit, &data); 
    DNXC_IF_ERR_EXIT(res);
    soc_reg_field_set(unit, IPS_CREDIT_CONFIGr, &data, CR_VAL_SEL_ENABLEf, TRUE);
    res = WRITE_IPS_CREDIT_CONFIGr(unit, data); 
    DNXC_IF_ERR_EXIT(res);
    }

    SOC_REG_ABOVE_64_CLEAR(above64); /* set IPS_STORED_CREDITS_USAGE_CONFIGURATIONr */
    soc_reg_above_64_field32_set(unit, IPS_STORED_CREDITS_USAGE_CONFIGURATIONr, above64, MUL_PKT_DEQf, 0xf0f0);
    soc_reg_above_64_field32_set(unit, IPS_STORED_CREDITS_USAGE_CONFIGURATIONr, above64, MUL_PKT_DEQ_BYTESf, 0x10);
    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, IPS_STORED_CREDITS_USAGE_CONFIGURATIONr, REG_PORT_ANY, 0, above64));

    /* init CGM counters */
    COMPILER_64_SET(val64, 0xFFFFF, 0xFFFFFFFF);
    DNXC_IF_ERR_EXIT(WRITE_CGM_PRG_CTR_SETTINGSr(unit, val64));

    /* Enable stamping Fabric Header */
    DNXC_IF_ERR_EXIT(READ_ITE_STAMPING_FABRIC_HEADER_ENABLEr(unit, &val64));
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, STAMP_FAP_PORTf, 1);
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, STAMP_FWD_ACTIONf, 1);
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, STAMP_LB_KEY_EXT_ENf, 1);
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, STAMP_FWDACTION_TDMf, 1);
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, STAMP_DP_ENf, 0xff);
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, SNP_STAMP_TRAP_CODEf, 1);
    soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, STAMP_CNI_BITf, 1);
    if (SOC_DNX_CONFIG(unit)->jer2_arad->init.mirror_stamp_sys_dsp_ext) 
    {
        /* Enable DSP-Ext stamping for mirrored/snooped packets */
        soc_reg64_field32_set(unit, ITE_STAMPING_FABRIC_HEADER_ENABLEr, &val64, STAMP_DSP_EXT_ENf, 0);
    }
    DNXC_IF_ERR_EXIT(WRITE_ITE_STAMPING_FABRIC_HEADER_ENABLEr(unit, val64));

    /* 
     * Read CGM minimum occupancy registers (watermark) in order for 
     * future reads to be correct (the registers set to maximum on read, 
     * but first reads are always 0).
     */
    DNXC_IF_ERR_EXIT(READ_CGM_SRAM_BUFFERS_FREE_MIN_STATUSr(unit, &data));
    DNXC_IF_ERR_EXIT(READ_CGM_SRAM_PDBS_FREE_MIN_STATUSr(unit, &data));
    DNXC_IF_ERR_EXIT(READ_CGM_DRAM_BDBS_FREE_MIN_STATUSr(unit, &data));

    /*
     * FIELD_0_0 should be 0 in both registers (ASIC feedback)
     */
    DNXC_IF_ERR_EXIT(READ_IPS_AVOID_DRAM_CAM_FULLr(unit, &data));
    soc_reg_field_set(unit, IPS_AVOID_DRAM_CAM_FULLr, &data, FIELD_0_0f, 0);
    DNXC_IF_ERR_EXIT(WRITE_IPS_AVOID_DRAM_CAM_FULLr(unit, data));

    DNXC_IF_ERR_EXIT(READ_IPS_AVOID_FABRIC_CAM_FULLr(unit, &data));
    soc_reg_field_set(unit, IPS_AVOID_FABRIC_CAM_FULLr, &data, FIELD_0_0f, 0);
    DNXC_IF_ERR_EXIT(WRITE_IPS_AVOID_FABRIC_CAM_FULLr(unit, data));

    /* set the total of the guarenteed VOQ resource, by default the dynamic shared space should be equal to the total memory */
    SOC_DNX_CORES_ITER(BCM_CORE_ALL, core_index) {
        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        res = sw_state_access[unit].dnx.soc.jer2_qax.tm.guaranteed_info.get(unit, core_index, &guaranteed_resources);
        DNXC_IF_ERR_EXIT(res);

        /* Words */
        guaranteed_resources.guaranteed[DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].total = 
            (core_index == 0 ? 
             SOC_DNX_CONFIG(unit)->jer2_jer->dbuffs.dbuffs_bdries.fbc_mnmc_0.size /* Number of buffers */
             : SOC_DNX_CONFIG(unit)->jer2_jer->dbuffs.dbuffs_bdries.fbc_mnmc_1.size
            * (SOC_DNX_CONFIG(unit)->jer2_arad->init.dram.dbuff_size/* buffer size in bytes -- 4K for JER2_QAX */
               / 16);/* word size */

        /* SRAM words */
        guaranteed_resources.guaranteed[DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES].total = SOC_DNX_DEFS_GET(unit, ocb_memory_size)
            * (1024 * 1024 / 8 /* MBits -> Bytes */
              / 16);      /* word size */

        /* SRAM PDs*/
        guaranteed_resources.guaranteed[DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS].total = SOC_DNX_DEFS_GET(unit, pdm_size);

        /* update warm boot data */
        res = sw_state_access[unit].dnx.soc.jer2_qax.tm.guaranteed_info.set(unit, core_index, &guaranteed_resources);
        DNXC_IF_ERR_EXIT(res);
#endif 
    }
	
    /* Init default global reject thresholds */
    DNXC_IF_ERR_EXIT(jer2_qax_itm_drop_global_thresholds_init(unit));
	
    /* Configure Admission tests defaults */
    DNXC_IF_ERR_EXIT(jer2_qax_itm_admission_tests_defaults_set(unit));


    DNXC_IF_ERR_EXIT(jer2_qax_itm_scheduler_compensation_init(unit));
    


exit:
    DNXC_FUNC_RETURN;
}

/*
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
int jer2_qax_itm_drop_global_thresholds_init(int unit)
{
    DNX_TMC_ITM_GLOB_RCS_DROP_TH glbl_drop, glbl_drop_exact;
    int core_index = 0;
    /*uint32 voq_threshold = 0, vsq_threshold = 0;*/
    int rv = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS;

    SOC_DNX_CORES_ITER(SOC_DNX_CORE_ALL(unit), core_index) {

        /* clear structs */
        DNX_TMC_ITM_GLOB_RCS_DROP_TH_clear(&glbl_drop);
        DNX_TMC_ITM_GLOB_RCS_DROP_TH_clear(&glbl_drop_exact);

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

        rv = jer2_qax_itm_glob_rcs_drop_set(unit, core_index, &glbl_drop, &glbl_drop_exact);
        DNXC_IF_ERR_EXIT(rv);
    }

exit: 
    DNXC_FUNC_RETURN; 
}

int
  jer2_qax_itm_per_queue_info_set(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32          queue,
    DNX_SAND_IN   JER2_ARAD_IQM_STATIC_TBL_DATA* IQM_static_tbl_data
  )
{
    soc_reg_above_64_val_t data_above_64, data2_above_64;
    uint32 offset;
    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data_above_64);
    SOC_REG_ABOVE_64_CLEAR(data2_above_64);

    offset = queue / 4; /* each entry handles 4 queues */
    /* read */
    DNXC_IF_ERR_EXIT(READ_CGM_VOQ_PROFILESm(unit, CGM_BLOCK(unit, core), offset, &data_above_64));
    DNXC_IF_ERR_EXIT(READ_CGM_VOQ_VSQS_PRMSm(unit, CGM_BLOCK(unit, core), offset, &data2_above_64));

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
    DNXC_IF_ERR_EXIT(WRITE_CGM_VOQ_PROFILESm(unit, CGM_BLOCK(unit, core), offset, &data_above_64));
    DNXC_IF_ERR_EXIT(WRITE_CGM_VOQ_VSQS_PRMSm(unit, CGM_BLOCK(unit, core), offset, &data2_above_64));

exit:
    DNXC_FUNC_RETURN;
}

int
  jer2_qax_itm_per_queue_info_get(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32          queue,
    DNX_SAND_OUT   JER2_ARAD_IQM_STATIC_TBL_DATA* IQM_static_tbl_data
  )
{
    soc_reg_above_64_val_t data_above_64, data2_above_64;
    uint32 offset;
    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data_above_64);
    SOC_REG_ABOVE_64_CLEAR(data2_above_64);
    IQM_static_tbl_data->que_signature = 0; /* no signature in JER2_QAX */

    offset = queue / 4; /* each entry handles 4 queues */
    /* read */
    DNXC_IF_ERR_EXIT(READ_CGM_VOQ_PROFILESm(unit, CGM_BLOCK(unit, core), offset, &data_above_64));
    DNXC_IF_ERR_EXIT(READ_CGM_VOQ_VSQS_PRMSm(unit, CGM_BLOCK(unit, core), offset, &data2_above_64));

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
    DNXC_FUNC_RETURN;
}

int
  jer2_qax_itm_profile_ocb_only_set(
    DNX_SAND_IN   int unit,
    DNX_SAND_IN   int rate_class,
    DNX_SAND_IN   int is_ocb_only
  )
{
    uint32 data;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(READ_CGM_VOQ_SRAM_DRAM_ONLY_MODEm(unit, MEM_BLOCK_ALL, rate_class, &data));
    /* 0 - regular mode (mix), 1 - ocb only mode */
    soc_mem_field32_set(unit, CGM_VOQ_SRAM_DRAM_ONLY_MODEm, &data, SRAM_DRAM_ONLY_MODEf, is_ocb_only);
     
    DNXC_IF_ERR_EXIT(WRITE_CGM_VOQ_SRAM_DRAM_ONLY_MODEm(unit, MEM_BLOCK_ALL, rate_class, &data));         
        
exit:
    DNXC_FUNC_RETURN;
}

int
  jer2_qax_itm_profile_ocb_only_get(
    DNX_SAND_IN   int unit,
    DNX_SAND_IN   int rate_class,
    DNX_SAND_OUT  int *is_ocb_only
  )
{
    uint32 data, field;


    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(READ_CGM_VOQ_SRAM_DRAM_ONLY_MODEm(unit, MEM_BLOCK_ALL, rate_class, &data));
    /* 0 - regular mode (mix), 1 - ocb only mode */
    field = soc_mem_field32_get(unit, CGM_VOQ_SRAM_DRAM_ONLY_MODEm, &data, SRAM_DRAM_ONLY_MODEf);
    
    *is_ocb_only = (int)field;        
        
exit:
    DNXC_FUNC_RETURN;
}

/*
Get JER2_QAX ingress congestion statistics.
*/
int jer2_qax_itm_congestion_statistics_get(
  DNX_SAND_IN int unit,
  DNX_SAND_IN int core,
  DNX_SAND_OUT JER2_ARAD_ITM_CGM_CONGENSTION_STATS *stats /* place current statistics output here */
  )
{
    uint32 val;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(stats);

    if (!SOC_UNIT_NUM_VALID(unit)) {
        DNXC_IF_ERR_EXIT(SOC_E_UNIT);
    } 
    if (((core < 0) || (core > SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores)) && (core != SOC_CORE_ALL)) { 
        DNXC_IF_ERR_EXIT(SOC_E_PARAM); 
    }

    /* collect current value statistics */  

    /* Instantaneous SRAM-Buffers free counter */
    DNXC_IF_ERR_EXIT(READ_CGM_SRAM_BUFFERS_FREE_STATUSr(unit, &val));
    stats->sram_buf_free = val; /* place the value into the 32 bits integer */	

    /* Minimal SRAM-Buffers free counter */
    DNXC_IF_ERR_EXIT(READ_CGM_SRAM_BUFFERS_FREE_MIN_STATUSr(unit, &val));
    stats->sram_buf_min_free = val; /* place the value into the 32 bits integer */		

    /* Instantaneous SRAM-PDBs free counter */
    DNXC_IF_ERR_EXIT(READ_CGM_SRAM_PDBS_FREE_STATUSr(unit, &val));
    stats->sram_pdbs_free = val; /* place the value into the 32 bits integer */	

    /* Minimal SRAM-PDBs free counter */
    DNXC_IF_ERR_EXIT(READ_CGM_SRAM_PDBS_FREE_MIN_STATUSr(unit, &val));
    stats->sram_pdbs_min_free = val; /* place the value into the 32 bits integer */		

    /* Instantaneous DRAM-BDBs free counter */
    DNXC_IF_ERR_EXIT(READ_CGM_DRAM_BDBS_FREE_STATUSr(unit, &val));
    stats->bdb_free = val; /* place the value into the 32 bits integer */			

    /* Minimal DRAM-BDBs free counter */
    DNXC_IF_ERR_EXIT(READ_CGM_DRAM_BDBS_FREE_MIN_STATUSr(unit, &val));
    stats->min_bdb_free = val; /* place the value into the 32 bits integer */

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Get JER2_QAX ingress minimal free resources.
 * The counters clear on read.
 */
int jer2_qax_itm_min_free_resources_stat_get(
  DNX_SAND_IN int unit,
  DNX_SAND_IN int core,
  DNX_SAND_IN DNX_TMC_ITM_CGM_RSRC_STAT_TYPE type,
  DNX_SAND_OUT uint64 *value
  )
{
    uint32 val32;

    DNXC_INIT_FUNC_DEFS;

    switch (type) {
        case DNX_TMC_ITM_CGM_RSRC_STAT_MIN_FREE_BDB:
            /* Minimal DRAM-BDBs free counter */
            DNXC_IF_ERR_EXIT(READ_CGM_DRAM_BDBS_FREE_MIN_STATUSr(unit, &val32));
            break;
        
        case DNX_TMC_ITM_CGM_RSRC_STAT_MIN_FREE_SRAM_BUFFERS:
            /* Minimal SRAM-Buffers free counter */
            DNXC_IF_ERR_EXIT(READ_CGM_SRAM_BUFFERS_FREE_MIN_STATUSr(unit, &val32));
            break;

        case DNX_TMC_ITM_CGM_RSRC_STAT_MIN_FREE_SRAM_PDB:
            /* Minimal SRAM-PDBs free counter */
            DNXC_IF_ERR_EXIT(READ_CGM_SRAM_PDBS_FREE_MIN_STATUSr(unit, &val32));
            break;

        default:
            DNXC_IF_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_SOC_MSG("Resource statistic type not supported for this device.")));
    }

    COMPILER_64_SET(*value, 0, val32);

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Set the drop precedence value above which 
*     all packets will always be discarded.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_qax_itm_dp_discard_set(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  uint32                  discard_dp
  )
{     
    DNXC_INIT_FUNC_DEFS;

    DNXC_SAND_IF_ERR_EXIT(jer2_arad_itm_dp_discard_set_verify(unit, discard_dp));

    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, CGM_GLBL_RJCT_PRMSr, SOC_CORE_ALL, 0, DP_LEVEL_RJCT_THf, discard_dp));

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Get the drop precedence value above which 
*     all packets will always be discarded.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_qax_itm_dp_discard_get(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_OUT uint32                  *discard_dp
  )
{     
    uint32 fld_val;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(discard_dp);

    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, CGM_GLBL_RJCT_PRMSr, SOC_CORE_ALL, 0, DP_LEVEL_RJCT_THf, &fld_val));

    *discard_dp = fld_val;

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Set the size of committed queue size (i.e., the
 *     guaranteed memory) for each VOQ, even in the case that a
 *     set of queues consume most of the memory resources.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_qax_itm_committed_q_size_set(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  uint32                  rt_cls_ndx,
    DNX_SAND_IN  DNX_TMC_ITM_GUARANTEED_INFO *info, 
    DNX_SAND_OUT DNX_TMC_ITM_GUARANTEED_INFO *exact_info 
  )
{
    uint64    data;
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E thresh_type;
    int32 max_guaranteed_limit;
   
    DNXC_INIT_FUNC_DEFS;
    
    DNXC_NULL_CHECK(info);
    DNXC_NULL_CHECK(exact_info);
    

    max_guaranteed_limit = JER2_QAX_ITM_GRNT_BYTES_MAX;

    if (rt_cls_ndx >  JER2_ARAD_ITM_QT_RT_CLS_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }
    if (info->guaranteed_size[DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES] > max_guaranteed_limit) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("guaranteed size %d is out of range\n"), info->guaranteed_size));
    }
    if (info->guaranteed_size[DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES] > JER2_QAX_ITM_GRNT_SRAM_BYTES_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("guaranteed sram size %d is out of range\n"), 
                                           info->guaranteed_size[DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES]));
    }
    if (info->guaranteed_size[DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS] > JER2_QAX_ITM_GRNT_SRAM_PDS_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("guaranteed sram pds %d is out of range\n"), 
                                           info->guaranteed_size[DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS]));
    }

    
    DNXC_IF_ERR_EXIT(READ_CGM_VOQ_GRNTD_PRMSm(unit, MEM_BLOCK_ANY, rt_cls_ndx, &data));
    
    for (thresh_type = 0; thresh_type < DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES; thresh_type++) {
        DNXC_IF_ERR_EXIT(_jer2_qax_itm_mantissa_exp_field_set(unit, &voq_guaranteed_th_mant_exp[thresh_type],1,
                                                       &data,
                                                       info->guaranteed_size[thresh_type], 
                                                       &exact_info->guaranteed_size[thresh_type]));
    }

    
    DNXC_IF_ERR_EXIT( WRITE_CGM_VOQ_GRNTD_PRMSm(unit, MEM_BLOCK_ANY, rt_cls_ndx, &data));


exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     get the size of committed queue size (i.e., the
 *     guaranteed memory) for each VOQ, even in the case that a
 *     set of queues consume most of the memory resources.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_qax_itm_committed_q_size_get(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  uint32                  rt_cls_ndx,
    DNX_SAND_OUT DNX_TMC_ITM_GUARANTEED_INFO *exact_info
  )
{
    uint64 data;
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E thresh_type;

    DNXC_INIT_FUNC_DEFS;
    
    DNXC_NULL_CHECK(exact_info);

    if (rt_cls_ndx >  JER2_ARAD_ITM_QT_RT_CLS_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }
    
    /*
     *    Exact the exponent and the mantissa to get the exact value
     */
    DNXC_IF_ERR_EXIT(READ_CGM_VOQ_GRNTD_PRMSm(unit, MEM_BLOCK_ANY, rt_cls_ndx, &data));

    for (thresh_type = 0; thresh_type < DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES; thresh_type++) {

        _jer2_qax_itm_mantissa_exp_field_get(unit, &voq_guaranteed_th_mant_exp[thresh_type],&data, &exact_info->guaranteed_size[thresh_type]);

    }

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Sets FADT drop parameters - per rate-class
*     and drop precedence. The FADT drop mechanism drops
*     packets that are mapped to queues that exceed thresholds
*     of this structure.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_qax_itm_fadt_tail_drop_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  DNX_TMC_ITM_FADT_DROP_INFO  *info,
    DNX_SAND_OUT DNX_TMC_ITM_FADT_DROP_INFO  *exact_info
  )
{

    soc_reg_above_64_val_t data;
    int mem_id;
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E thresh_type;
    uint32 max_size_limit = JER2_QAX_ITM_QUEUE_SIZE_BYTES_MAX;
    uint32 alpha_field;
    
    DNXC_INIT_FUNC_DEFS;
    
    DNXC_NULL_CHECK(info);
    DNXC_NULL_CHECK(exact_info);
    
    if (rt_cls_ndx >  JER2_ARAD_ITM_QT_RT_CLS_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    if (drop_precedence_ndx >= DNX_TMC_NOF_DROP_PRECEDENCE ) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("drop precedence index %d is out of range\n"),drop_precedence_ndx ));
    }

    if (info->max_threshold[DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES] >  max_size_limit) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("max threshold %d is out of range\n"),
                                           info->max_threshold[DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES]));
    }

    if (info->max_threshold[DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES] >  JER2_QAX_ITM_QUEUE_SIZE_SRAM_BYTES_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("max threshold %d is out of range\n"),
                                           info->max_threshold[DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES]));
    }

    if (info->max_threshold[DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS] >  JER2_QAX_ITM_QUEUE_SIZE_SRAM_PDS_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("max threshold %d is out of range\n"),
                                           info->max_threshold[DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS]));
    }

    for (thresh_type = DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES; thresh_type < DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES; thresh_type++) {
        switch (thresh_type) {
            case DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES: {
                mem_id = CGM_VOQ_WORDS_RJCT_PRMSm;
                break;
            }
            case DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES: {
                mem_id = CGM_VOQ_SRAM_WORDS_RJCT_PRMSm;
                break;
            }
            case DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS: {
                mem_id = CGM_VOQ_SRAM_PDS_RJCT_PRMSm;
                break;
            }
            default:  {
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unexpected threshold type %d\n"), thresh_type));
            }
        }
        
        
        DNXC_IF_ERR_EXIT(soc_mem_read(unit, mem_id, MEM_BLOCK_ANY, 
                                        rt_cls_ndx * DNX_TMC_NOF_DROP_PRECEDENCE + drop_precedence_ndx, data));

        DNXC_IF_ERR_EXIT(_jer2_qax_itm_mantissa_exp_field_set(unit,&voq_fadt_max_th_mant_exp[thresh_type], 1, data,
                                                           info->max_threshold[thresh_type], 
                                                           &(exact_info->max_threshold[thresh_type])));
        
        DNXC_IF_ERR_EXIT(_jer2_qax_itm_mantissa_exp_field_set(unit,&voq_fadt_min_th_mant_exp[thresh_type], 1, data,
                                                           info->min_threshold[thresh_type], 
                                                           &(exact_info->min_threshold[thresh_type])));
        
        alpha_field =  _jer2_qax_itm_alpha_to_field(unit, info->adjust_factor[thresh_type]);
        soc_mem_field32_set(unit, mem_id, &data, VOQ_FADT_ADJUST_FACTORf,
                            alpha_field);
        exact_info->adjust_factor[thresh_type] = info->adjust_factor[thresh_type];
    
        DNXC_IF_ERR_EXIT(soc_mem_write(unit, mem_id, MEM_BLOCK_ANY, 
                                         rt_cls_ndx * DNX_TMC_NOF_DROP_PRECEDENCE + drop_precedence_ndx, data));
    }
    
exit:
    DNXC_FUNC_RETURN;

}



/*********************************************************************
*     Sets tail drop parameter - max-queue-size per rate-class
*     and drop precedence. The tail drop mechanism drops
*     packets that are mapped to queues that exceed thresholds
*     of this structure.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_qax_itm_fadt_tail_drop_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_OUT DNX_TMC_ITM_FADT_DROP_INFO  *info
  )

{

    soc_reg_above_64_val_t data;
    int mem_id;
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E thresh_type;
    uint32 alpha_field;

    DNXC_INIT_FUNC_DEFS;


    DNXC_NULL_CHECK(info);

    if (rt_cls_ndx >  JER2_ARAD_ITM_QT_RT_CLS_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    if (drop_precedence_ndx >=  DNX_TMC_NOF_DROP_PRECEDENCE) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("drop precedence index %d is out of range\n"), drop_precedence_ndx));
    }


    for (thresh_type = DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES; thresh_type < DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES; thresh_type++) {
        switch (thresh_type) {
            case DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES: {
                mem_id = CGM_VOQ_WORDS_RJCT_PRMSm;
                break;
            }
            case DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES: {
                mem_id = CGM_VOQ_SRAM_WORDS_RJCT_PRMSm;
                break;
            }
            case DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS: {
                mem_id = CGM_VOQ_SRAM_PDS_RJCT_PRMSm;
                break;
            }
            default:  {
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unexpected threshold type %d\n"), thresh_type));
            }
        }
    
        
        DNXC_IF_ERR_EXIT(soc_mem_read(unit, mem_id, MEM_BLOCK_ANY, 
                                        rt_cls_ndx * DNX_TMC_NOF_DROP_PRECEDENCE + drop_precedence_ndx, data));

        _jer2_qax_itm_mantissa_exp_field_get(unit, &voq_fadt_max_th_mant_exp[thresh_type], data,&(info->max_threshold[thresh_type]));
        _jer2_qax_itm_mantissa_exp_field_get(unit, &voq_fadt_min_th_mant_exp[thresh_type], data,&(info->min_threshold[thresh_type]));

        alpha_field = soc_mem_field32_get(unit, mem_id, &data, VOQ_FADT_ADJUST_FACTORf);
        info->adjust_factor[thresh_type] = _jer2_qax_itm_field_to_alpha(unit, alpha_field);
    }

exit:
    DNXC_FUNC_RETURN;

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
   In JER2_QAX, it is used for ECN drop ONLY
   and must be called with drop_precedence_ndx == DNX_TMC_NOF_DROP_PRECEDENCE.

   For regular tail drop, 
   jer2_qax_itm_fadt_tail_drop_set/jer2_qax_itm_fadt_tail_drop_get are used
   
*/
int
  jer2_qax_itm_tail_drop_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  JER2_ARAD_ITM_TAIL_DROP_INFO  *info,
    DNX_SAND_OUT JER2_ARAD_ITM_TAIL_DROP_INFO  *exact_info
  )
{
    soc_reg_above_64_val_t data;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);
    DNXC_NULL_CHECK(exact_info);

    if (rt_cls_ndx >  JER2_ARAD_ITM_QT_RT_CLS_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    /* in JER2_QAX, used for ECN drop only */
    if (drop_precedence_ndx != DNX_TMC_NOF_DROP_PRECEDENCE ) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("This function should be called for ECN drop only\n")));
    }

    /* max_inst_q_size */
    DNXC_IF_ERR_EXIT(soc_mem_read(unit, CGM_CNI_PRMSm, MEM_BLOCK_ANY, 
                                    rt_cls_ndx, data));
    
    DNXC_IF_ERR_EXIT(_jer2_qax_itm_mantissa_exp_field_set(unit,&voq_wred_ecn_max_size_th_mant_exp, 1, data,
                                                       info->max_inst_q_size,
                                                       &(exact_info->max_inst_q_size)));

    DNXC_IF_ERR_EXIT(soc_mem_write(unit, CGM_CNI_PRMSm, MEM_BLOCK_ANY, 
                                    rt_cls_ndx, data));

exit:
    DNXC_FUNC_RETURN;

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
   In JER2_QAX, it is used for ECN drop ONLY
   and must be called with drop_precedence_ndx == DNX_TMC_NOF_DROP_PRECEDENCE.

   For regular tail drop, 
   jer2_qax_itm_fadt_tail_drop_set/jer2_qax_itm_fadt_tail_drop_get are used
   
*/
int
  jer2_qax_itm_tail_drop_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_OUT JER2_ARAD_ITM_TAIL_DROP_INFO  *info
  )
{
    soc_reg_above_64_val_t data;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);

    if (rt_cls_ndx >  JER2_ARAD_ITM_QT_RT_CLS_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    /* in JER2_QAX, used for ECN drop only */
    if (drop_precedence_ndx != DNX_TMC_NOF_DROP_PRECEDENCE ) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("This function should be called for ECN drop only\n")));
    }

    /* max_inst_q_size */
    DNXC_IF_ERR_EXIT(soc_mem_read(unit, CGM_CNI_PRMSm, MEM_BLOCK_ANY, 
                                    rt_cls_ndx, data));
    
    _jer2_qax_itm_mantissa_exp_field_get(unit,&voq_wred_ecn_max_size_th_mant_exp, data,
                                    &info->max_inst_q_size);


exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
*    Gets the WRED parameters from the data in HW
*    By converting them appropriately.
*********************************************************************/
static void
_jer2_qax_itm_hw_data_to_wred_info (
        DNX_SAND_IN  int                                                  unit,
        DNX_SAND_IN  uint32                                               c1,
        DNX_SAND_IN  uint32                                               c2,
        DNX_SAND_IN  uint32                                               c3,
        DNX_SAND_INOUT JER2_ARAD_ITM_WRED_QT_DP_INFO                           *wred_param
  )
{
    uint32
        avrg_th_diff_wred_granular,
        two_power_c1,
        remainder;
    
    DNX_SAND_U64
        u64_1,
        u64_2;
    
    
    /* min threshold and max threshold are set in advance on wred_param */
    
    /*
     * max_packet_size
     */
    wred_param->max_packet_size = ((c3 == 0) && (c2 == 1)) ? 0 : (0x1 << (c3)) * DNX_TMC_ITM_WRED_GRANULARITY;
    
    avrg_th_diff_wred_granular =
        (wred_param->max_avrg_th - wred_param->min_avrg_th) / DNX_TMC_ITM_WRED_GRANULARITY;
    
    two_power_c1 = 1 << c1;
    /*
     * C1 = ((2^32)/100)*max-prob / (max-th - min-th) in powers of 2
     * ==>
     * max-prob =  ( 2^C1 * (max-th - min-th) ) / ((2^32)/100)
     */
    dnx_sand_u64_multiply_longs(two_power_c1, avrg_th_diff_wred_granular, &u64_1);
    remainder = dnx_sand_u64_devide_u64_long(&u64_1, DNX_TMC_WRED_NORMALIZE_FACTOR, &u64_2);
    dnx_sand_u64_to_long(&u64_2, &wred_param->max_probability);
    
    if(remainder > (DNX_TMC_WRED_NORMALIZE_FACTOR/2))
    {
        wred_param->max_probability++;
    }
    
    if(wred_param->max_probability > 100)
    {
        wred_param->max_probability = 100;
    }
    
}


static void
  _jer2_qax_itm_wred_info_to_hw_data (
     DNX_SAND_IN   int unit,
     DNX_SAND_IN   DNX_TMC_ITM_WRED_QT_DP_INFO                          *wred_param,
     DNX_SAND_IN   uint32                                               min_avrg_th_exact,
     DNX_SAND_IN   uint32                                               max_avrg_th_exact,
     DNX_SAND_OUT  uint32*                                              c1,
     DNX_SAND_OUT  uint32*                                              c2,
     DNX_SAND_OUT  uint32*                                              c3
  )
{
    uint32
        max_prob,
        calc,
        max_val_c1,
        min_avrg_th_16_byte;
    int32
        avrg_th_diff_wred_granular = 0;
    int32
        min_avrg_th_exact_wred_granular,
        max_avrg_th_exact_wred_granular;
    uint32
        trunced;
    DNX_SAND_U64
        u64_1,
        u64_2,
        u64_c2 = {{0}};
    
    
    trunced = FALSE;
    
    /*
     * min_avrg_th
     */
    min_avrg_th_16_byte = DNX_SAND_DIV_ROUND_UP(wred_param->min_avrg_th,DNX_TMC_ITM_WRED_GRANULARITY);
    min_avrg_th_exact_wred_granular = min_avrg_th_exact / DNX_TMC_ITM_WRED_GRANULARITY;
    
    /*
     * max_avrg_th
     */
    max_avrg_th_exact_wred_granular = max_avrg_th_exact / DNX_TMC_ITM_WRED_GRANULARITY;
    
    /*
     * max_packet_size
     */
    calc = wred_param->max_packet_size;
    if (calc > DNX_TMC_ITM_WRED_MAX_PACKET_SIZE_FOR_CALC)
    {
        calc = DNX_TMC_ITM_WRED_MAX_PACKET_SIZE_FOR_CALC;
    }
    calc = DNX_SAND_DIV_ROUND_UP(calc, DNX_TMC_ITM_WRED_GRANULARITY );
    *c3 = (wred_param->max_avrg_th == 0 ? 0 : dnx_sand_log2_round_up(calc));
    
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
    calc = DNX_TMC_WRED_NORMALIZE_FACTOR * max_prob; 
    
    /*
     * We do not use 'DNX_SAND_DIV_ROUND' or 'DNX_SAND_DIV_ROUND_UP'
     * because at this point we might have in calc '((2^32)/100)*max-prob'
     * which can be very large number and the other dividers do ADD before
     * the division.
     */
    max_val_c1 = 31; /* dnx_sand_log2_round_down(0xFFFFFFFF) */
    
    avrg_th_diff_wred_granular =
        (max_avrg_th_exact_wred_granular - min_avrg_th_exact_wred_granular);
    
    if(avrg_th_diff_wred_granular == 0)
    {
        *c1 = max_val_c1;
    }
    else
    {
        calc = DNX_SAND_DIV_ROUND_DOWN(calc, avrg_th_diff_wred_granular);
    *c1 = dnx_sand_log2_round_down(calc);
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
    
    DNX_SAND_LIMIT_FROM_ABOVE(*c1, max_val_c1);
    
    if (min_avrg_th_16_byte > 0)
    { /* This limit from above is HW restriction */
        max_val_c1 = DNX_SAND_DIV_ROUND_DOWN(0xFFFFFFFF, min_avrg_th_16_byte);
        max_val_c1 = dnx_sand_log2_round_down(max_val_c1);
        DNX_SAND_LIMIT_FROM_ABOVE(*c1, max_val_c1);
    }
    
    /*
     * max_probability
     * C2 = FACTOR * max-prob * min-th / (max-th - min-th)
     */
    dnx_sand_u64_multiply_longs(
        DNX_TMC_WRED_NORMALIZE_FACTOR,
        max_prob * min_avrg_th_exact_wred_granular,
        &u64_2
        );
    dnx_sand_u64_devide_u64_long(&u64_2, avrg_th_diff_wred_granular, &u64_c2);
    
    /*
     * P =
     */
    dnx_sand_u64_multiply_longs(
        min_avrg_th_exact_wred_granular,
        (1 << *c1),
        &u64_1
        );
    
    if(dnx_sand_u64_is_bigger(&u64_c2, &u64_1))
    {
        dnx_sand_os_memcpy(&u64_c2, &u64_1, sizeof(DNX_SAND_U64));
    }
    
    trunced = dnx_sand_u64_to_long(&u64_c2, c2);
    
    if (trunced)
    {
        *c2 = 0xFFFFFFFF;
    }
    *c2 = (wred_param->max_avrg_th == 0 ? 1 : *c2);
    
}

void _jer2_qax_itm_wred_mem_fields_get( 
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
    *is_ecn = (drop_precedence_ndx == DNX_TMC_NOF_DROP_PRECEDENCE);
    
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

        *entry_offset = (rt_cls_ndx * DNX_TMC_NOF_DROP_PRECEDENCE) + drop_precedence_ndx;
    }

}

/*********************************************************************
*     Sets WRED parameters per rate-class and drop precedence,
*     including wred-enable and the admit test logic. Also, as
*     part of the WRED parameters; max-queue,
*     WRED-thresholds/probability. To configure WRED
*     Configuration that is per queue-type only (exponential
*     weight queue), use the function
*     jer2_arad_itm_wred_exp_wq_set.
*     Details: in the H file. (search for prototype)
*********************************************************************/
static int
  _jer2_qax_itm_wred_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  JER2_ARAD_ITM_WRED_QT_DP_INFO *info
  )
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);


    if (rt_cls_ndx > JER2_ARAD_ITM_QT_RT_CLS_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    if (drop_precedence_ndx > DNX_TMC_NOF_DROP_PRECEDENCE ) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("drop precedence index %d is out of range\n"),
                                           drop_precedence_ndx ));
    }

    if (info->min_avrg_th > info->max_avrg_th)
    {
       DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("min threshold %d is higher than max threshold\n"),
                                          info->min_avrg_th, info->max_avrg_th));
    }
    
    if (info->min_avrg_th > JER2_QAX_ITM_Q_WRED_INFO_MIN_AVRG_TH_MAX)
    {
       DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("min threshold %d is out of range\n"),
                                          info->min_avrg_th));
    }

    if (info->max_avrg_th > JER2_QAX_ITM_Q_WRED_INFO_MAX_AVRG_TH_MAX)
    {
       DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("max threshold %d is out of range\n"),
                                          info->max_avrg_th));
    }

    if (info->max_probability > JER2_QAX_ITM_WRED_QT_DP_INFO_MAX_PROBABILITY_MAX)
    {
       DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("max probability %d is out of range\n"),
                                          info->max_probability));
    }

    if (info->max_packet_size > DNX_TMC_ITM_WRED_MAX_PACKET_SIZE)
    {
       DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("max packet size %d is out of range\n"),
                                          info->max_packet_size));
    }

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Sets WRED parameters per rate-class and drop precedence,
*     including wred-enable and the admit test logic. Also, as
*     part of the WRED parameters; max-queue,
*     WRED-thresholds/probability. To configure WRED
*     Configuration that is per queue-type only (exponential
*     weight queue), use the function
*     jer2_arad_itm_wred_exp_wq_set.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_qax_itm_wred_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  JER2_ARAD_ITM_WRED_QT_DP_INFO *info,
    DNX_SAND_OUT JER2_ARAD_ITM_WRED_QT_DP_INFO *exact_info
  )
{
    soc_reg_above_64_val_t data;
    int mem;
    int wred_en_field, c1_field, c2_field, c3_field;
    int entry_offset;
    uint32 c1, c2, c3;
    int is_ecn;
    
    DNXC_INIT_FUNC_DEFS;
    
    DNXC_NULL_CHECK(info);
    DNXC_NULL_CHECK(exact_info);

    DNXC_IF_ERR_EXIT(_jer2_qax_itm_wred_verify(unit, rt_cls_ndx, drop_precedence_ndx, info));

    _jer2_qax_itm_wred_mem_fields_get(unit, rt_cls_ndx, drop_precedence_ndx, 
                                  &is_ecn, &mem,
                                  &wred_en_field, &c1_field, &c2_field, &c3_field,
                                  &entry_offset
        );
    

    DNXC_IF_ERR_EXIT(soc_mem_read(unit, mem, MEM_BLOCK_ANY, 
                                    entry_offset , data));
    
    DNXC_IF_ERR_EXIT(_jer2_qax_itm_mantissa_exp_field_set(unit,&voq_wred_ecn_min_th_mant_exp[is_ecn], 1, data,
                                                       info->min_avrg_th,
                                                       &exact_info->min_avrg_th));

    DNXC_IF_ERR_EXIT(_jer2_qax_itm_mantissa_exp_field_set(unit, &voq_wred_ecn_max_th_mant_exp[is_ecn], 1, data,
                                                       info->max_avrg_th,
                                                       &exact_info->max_avrg_th));

    _jer2_qax_itm_wred_info_to_hw_data(unit, info, exact_info->min_avrg_th, exact_info->max_avrg_th, &c1, &c2, &c3);

    soc_mem_field32_set(unit, mem, data, wred_en_field, info->wred_en);
    soc_mem_field32_set(unit, mem, data, c2_field, c2);
    soc_mem_field32_set(unit, mem, data, c3_field, c3);
    soc_mem_field32_set(unit, mem, data, c1_field, c1);
    if (!is_ecn) {
        soc_mem_field32_set(unit, mem, data, VOQ_WRED_IGNR_PKT_SIZEf, DNX_SAND_BOOL2NUM(info->ignore_packet_size));
    }
 
    DNXC_IF_ERR_EXIT(soc_mem_write(unit, mem, MEM_BLOCK_ANY, 
                                     entry_offset , data));
    
    _jer2_qax_itm_hw_data_to_wred_info (unit, c1, c2, c3, exact_info);
                       
    exact_info->wred_en = info->wred_en;

    exact_info->ignore_packet_size = (!is_ecn ? DNX_SAND_BOOL2NUM(info->ignore_packet_size) : 0);

exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
*     Sets WRED parameters per rate-class and drop precedence,
*     including wred-enable and the admit test logic. Also, as
*     part of the WRED parameters; max-queue,
*     WRED-thresholds/probability. To configure WRED
*     Configuration that is per queue-type only (exponential
*     weight queue), use the function
*     jer2_arad_itm_wred_exp_wq_set.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_qax_itm_wred_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_OUT JER2_ARAD_ITM_WRED_QT_DP_INFO *info
  )
{
    soc_reg_above_64_val_t data;
    int mem;
    int wred_en_field, c1_field, c2_field, c3_field;
    int entry_offset;
    uint32 c1, c2, c3, wred_pckt_sz_ignr = 0;
    int is_ecn;
    uint32 wred_en;
 

    DNXC_INIT_FUNC_DEFS;
    
    DNXC_NULL_CHECK(info);
 
    if (rt_cls_ndx > JER2_ARAD_ITM_QT_RT_CLS_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    if (drop_precedence_ndx > DNX_TMC_NOF_DROP_PRECEDENCE ) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("drop precedence index %d is out of range\n"),drop_precedence_ndx ));
    }
    
    _jer2_qax_itm_wred_mem_fields_get( unit,
                                  rt_cls_ndx, drop_precedence_ndx, 
                                  &is_ecn, &mem, &wred_en_field,
                                  &c1_field, &c2_field, &c3_field,
                                  &entry_offset
            );


    DNXC_IF_ERR_EXIT(soc_mem_read(unit, mem, MEM_BLOCK_ANY, 
                                    entry_offset , data));


    /*
     * min_avrg_th
     */
    _jer2_qax_itm_mantissa_exp_field_get(unit, &voq_wred_ecn_min_th_mant_exp[is_ecn], &data, &info->min_avrg_th);
    
    /*
     * max_avrg_th
     */
    _jer2_qax_itm_mantissa_exp_field_get(unit, &voq_wred_ecn_max_th_mant_exp[is_ecn], &data, &info->max_avrg_th);

    wred_en = soc_mem_field32_get(unit, mem, data, wred_en_field);
    c2 = soc_mem_field32_get(unit, mem, data, c2_field );
    c3 = soc_mem_field32_get(unit, mem, data, c3_field);
    c1 = soc_mem_field32_get(unit, mem, data, c1_field);

    if (!is_ecn) {
        wred_pckt_sz_ignr = soc_mem_field32_get(unit, mem, data, VOQ_WRED_IGNR_PKT_SIZEf);
    }

    _jer2_qax_itm_hw_data_to_wred_info (unit, c1, c2, c3, info);
                       
    info->wred_en = DNX_SAND_NUM2BOOL(wred_en);

    info->ignore_packet_size =
        DNX_SAND_NUM2BOOL(wred_pckt_sz_ignr);
    
exit:
    DNXC_FUNC_RETURN;
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
*     the function jer2_arad_itm_wred_info_set.
*     Details: in the H file. (search for prototype)
*********************************************************************/
static int
  _jer2_qax_itm_wred_exp_wq_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                  exp_wq
  )
{
    DNXC_INIT_FUNC_DEFS;

    if (rt_cls_ndx > JER2_ARAD_ITM_QT_RT_CLS_MAX ) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    if (exp_wq > JER2_ARAD_ITM_WQ_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("exp. weight %d is out of range\n"), exp_wq));
    }

exit:
    DNXC_FUNC_RETURN;
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
*     the functionjer2_arad_itm_wred_info_set.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_qax_itm_wred_exp_wq_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 exp_wq,
    DNX_SAND_IN  uint8                  enable
  )
{
    uint32 data;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT (_jer2_qax_itm_wred_exp_wq_verify(
                            unit,
                            rt_cls_ndx,
                            exp_wq
                            ));

    DNXC_IF_ERR_EXIT(soc_mem_read(
                           unit,
                           CGM_VOQ_AVRG_PRMSm,
                           MEM_BLOCK_ANY,
                           rt_cls_ndx,
                           &data
                           ));
    
    soc_mem_field32_set(unit, CGM_VOQ_AVRG_PRMSm, &data, AVRG_WEIGHTf, exp_wq);
    soc_mem_field32_set(unit, CGM_VOQ_AVRG_PRMSm, &data, AVRG_ENf, enable);

    DNXC_IF_ERR_EXIT(soc_mem_write(
                           unit,
                           CGM_VOQ_AVRG_PRMSm,
                           MEM_BLOCK_ANY,
                           rt_cls_ndx,
                           &data
                           ));
exit:
    DNXC_FUNC_RETURN;
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
*     the functionjer2_arad_itm_wred_info_set.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_qax_itm_wred_exp_wq_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              rt_cls_ndx,
    DNX_SAND_OUT  uint32             *exp_wq
  )
{
    uint32 data;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(exp_wq);

    if (rt_cls_ndx > JER2_ARAD_ITM_QT_RT_CLS_MAX ) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }
    
    DNXC_IF_ERR_EXIT(soc_mem_read(
                           unit,
                           CGM_VOQ_AVRG_PRMSm,
                           MEM_BLOCK_ANY,
                           rt_cls_ndx,
                           &data
                           ));
    
   *exp_wq = soc_mem_field32_get(unit, CGM_VOQ_AVRG_PRMSm, &data, AVRG_WEIGHTf);

exit:
    DNXC_FUNC_RETURN;
}

int
  jer2_qax_itm_dyn_total_thresh_set(
    DNX_SAND_IN int    unit,
    DNX_SAND_IN int    core,
    DNX_SAND_IN uint8  is_ocb_only,
                int32  reservation_increase_array[SOC_DNX_DEFS_MAX(NOF_CORES)][DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES] /* the (signed) amount in which the thresholds should decrease (according to 100% as will be set for DP 0) */
  )
{
    uint32
        res;
    DNX_TMC_ITM_GUARANTEED_RESOURCE guaranteed_resources;
    soc_dnx_guaranteed_pair_t *guaranteed_pair;
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

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(reservation_increase_array);

    if (!is_ocb_only) {
        int thresh_type;
        /* ocb only is not relevant for JER2_QAX */
        SOC_DNX_CORES_ITER(core, core_id){
            res = sw_state_access[unit].dnx.soc.jer2_qax.tm.guaranteed_info.get(unit, core_id, &guaranteed_resources);
            DNXC_IF_ERR_EXIT(res);
            
            DNXC_LEGACY_FIXME_ASSERT;
            for (thresh_type = 0; thresh_type < -1 /*DNX_COSQ_INGRESS_THRESHOLD_NOF_TYPES*/; thresh_type++) {
                guaranteed_pair =  &(guaranteed_resources.guaranteed[thresh_type]);
                reservation_increase = reservation_increase_array[core_id][thresh_type];

                if (reservation_increase < 0 &&  ((uint32)(-reservation_increase)) > guaranteed_pair->used) {
                    DNXC_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_DNXC_MSG("requested change in reserved resource %d is out of range\n"), 
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
                        DNXC_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_DNXC_MSG("out of resources. Left amount %d\n"), 
                                                          resource_left_calc));
                    }
                    
                    for (dp = 0 ; dp < DNX_TMC_NOF_DROP_PRECEDENCE; dp++) {
                        /* note that set and clr fields are in the same register */
                        DNXC_IF_ERR_EXIT(READ_CGM_VOQ_SHRD_OC_RJCT_THm(unit, MEM_BLOCK_ANY, dp, &data));
                        
                        if (resource_left) { /* configure drop thresholds according to new amount of resource left */
                            DNXC_IF_ERR_EXIT(_jer2_qax_itm_mantissa_exp_field_set(unit, &voq_shared_oc_set_th_mant_exp[thresh_type], 0, data,
                                                                               resource_left/denominator*numerator[dp],
                                                                               &dummy));
                            DNXC_IF_ERR_EXIT(_jer2_qax_itm_mantissa_exp_field_set(unit, &voq_shared_oc_clr_th_mant_exp[thresh_type], 0, data,
                                                                               resource_left/denominator*numerator[dp],
                                                                               &dummy));
                        }
                        else {
                            soc_mem_field32_set(unit, CGM_VOQ_SHRD_OC_RJCT_THm, data, voq_shared_oc_set_th_mant_exp[thresh_type].field_id, 
                                                JER2_ARAD_ITM_GRNT_BDS_OR_DBS_DISABLED);
                            soc_mem_field32_set(unit, CGM_VOQ_SHRD_OC_RJCT_THm, data, voq_shared_oc_clr_th_mant_exp[thresh_type].field_id, 
                                                JER2_ARAD_ITM_GRNT_BDS_OR_DBS_DISABLED);
                        }
                        DNXC_IF_ERR_EXIT(WRITE_CGM_VOQ_SHRD_OC_RJCT_THm(unit, MEM_BLOCK_ANY, dp, data));
                    }
                    guaranteed_pair->used += reservation_increase;
                }

                
                DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
                res = sw_state_access[unit].dnx.soc.jer2_qax.tm.guaranteed_info.set(unit, core_id, &guaranteed_resources);
                DNXC_SAND_IF_ERR_EXIT(res);
#endif 
            }
        }
    }

exit:
  DNXC_FUNC_RETURN;
}



/*********************************************************************
*     Gets the queue size of a queue
*********************************************************************/
int
  jer2_qax_itm_queue_dyn_info_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                  queue_ndx,
    DNX_SAND_OUT DNX_TMC_ITM_QUEUE_DYN_INFO *info
  )  
{
    soc_reg_above_64_val_t data;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(info);

    if (queue_ndx > JER2_ARAD_MAX_QUEUE_ID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("queue index %d is out of range\n"), queue_ndx));
    }

    DNXC_IF_ERR_EXIT(READ_CGM_VOQ_SIZEm(unit, CGM_BLOCK(unit, core), queue_ndx, data));

    info->pq_inst_que_size[DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES] = soc_mem_field32_get(unit, CGM_VOQ_SIZEm, data, WORDS_SIZEf) * 16;
    info->pq_inst_que_size[DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES] = soc_mem_field32_get(unit, CGM_VOQ_SIZEm, data, SRAM_WORDS_SIZEf) * 16;
    info->pq_inst_que_size[DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS] = soc_mem_field32_get(unit, CGM_VOQ_SIZEm, data, SRAM_PDS_SIZEf);

exit:
  DNXC_FUNC_RETURN;
}

/*----------------------------------------------------------------------------------------------------------*/
/*      DRAM bound     */

int jer2_qax_itm_dram_bound_alpha_field_get( DNX_SAND_IN int                 unit,
                                        DNX_SAND_IN DNX_TMC_INGRESS_DRAM_BOUND_TYPE_E dram_thresh,
                                        DNX_SAND_IN DNX_TMC_INGRESS_THRESHOLD_TYPE_E thresh_type)
{
    int field_id = INVALIDf;

    switch (dram_thresh) {
        case DNX_TMC_INGRESS_DRAM_BOUND:
            switch (thresh_type) {
                case DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                    field_id = SRAM_WORDS_BOUND_ADJUST_FACTORf;
                    break;
                case DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                    field_id = SRAM_PDS_BOUND_ADJUST_FACTORf;
                    break;
                default:
                    break;
            }

        case DNX_TMC_INGRESS_DRAM_BOUND_RECOVERY_FAILURE:
            switch (thresh_type) {
                case DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                    field_id = SRAM_WORDS_RECOVERY_ADJUST_FACTORf;
                    break;
                case DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS:
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
    JER2_QAX_DRAM_BOUND_THRESH_MIN = 0,
    JER2_QAX_DRAM_BOUND_THRESH_MAX,
    JER2_QAX_DRAM_BOUND_THRESH_FREE_MIN,
    JER2_QAX_DRAM_BOUND_THRESH_FREE_MAX
} jer2_qax_dram_bound_thresh_type_t;

int _jer2_qax_itm_dram_bound_mant_exp_info_get(
    int unit,
    DNX_TMC_INGRESS_DRAM_BOUND_TYPE_E dram_thresh,
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E resource_type, 
    jer2_qax_dram_bound_thresh_type_t thresh_type, 
    itm_mantissa_exp_threshold_info* info
    )
{
    DNXC_INIT_FUNC_DEFS;
    
    DNXC_NULL_CHECK(info);

    info->mem_id = CGM_VOQ_DRAM_BOUND_PRMSm;
    info->mantissa_bits = 8;
    info->exp_bits = 5;
    info->factor = (resource_type == DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES ? 16 : 1);

    switch (thresh_type) {
        case JER2_QAX_DRAM_BOUND_THRESH_MIN:
            switch (dram_thresh) {
                case DNX_TMC_INGRESS_DRAM_BOUND:
                    switch (resource_type) {
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_BOUND_MIN_THf;
                            break;
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_BOUND_MIN_THf;
                            break;
                        default:
                            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                case DNX_TMC_INGRESS_DRAM_BOUND_RECOVERY_FAILURE:
                    switch (resource_type) {
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_RECOVERY_MIN_THf;
                            break;
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_RECOVERY_MIN_THf;
                            break;
                        default:
                            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                default:
                    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unexpected dram threshold %d\n"), dram_thresh));
            }
            break;
        case JER2_QAX_DRAM_BOUND_THRESH_MAX:
            switch (dram_thresh) {
                case DNX_TMC_INGRESS_DRAM_BOUND:
                    switch (resource_type) {
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_BOUND_MAX_THf;
                            break;
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_BOUND_MAX_THf;
                            break;
                        default:
                            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                case DNX_TMC_INGRESS_DRAM_BOUND_RECOVERY_FAILURE:
                    switch (resource_type) {
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_RECOVERY_MAX_THf;
                            break;
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_RECOVERY_MAX_THf;
                            break;
                        default:
                            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                default:
                    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unexpected dram threshold %d\n"), dram_thresh));
            }
            break;
        case JER2_QAX_DRAM_BOUND_THRESH_FREE_MIN:
            switch (dram_thresh) {
                case DNX_TMC_INGRESS_DRAM_BOUND:
                    switch (resource_type) {
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_BOUND_FREE_MIN_THf;
                            break;
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_BOUND_FREE_MIN_THf;
                            break;
                        default:
                            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                case DNX_TMC_INGRESS_DRAM_BOUND_RECOVERY_FAILURE:
                    switch (resource_type) {
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_RECOVERY_FREE_MIN_THf;
                            break;
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_RECOVERY_FREE_MIN_THf;
                            break;
                        default:
                            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                default:
                    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unexpected dram threshold %d\n"), dram_thresh));
            }
            break;
        case JER2_QAX_DRAM_BOUND_THRESH_FREE_MAX:
            switch (dram_thresh) {
                case DNX_TMC_INGRESS_DRAM_BOUND:
                    switch (resource_type) {
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_BOUND_FREE_MAX_THf;
                            break;
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_BOUND_FREE_MAX_THf;
                            break;
                        default:
                            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                case DNX_TMC_INGRESS_DRAM_BOUND_RECOVERY_FAILURE:
                    switch (resource_type) {
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES:
                            info->field_id = SRAM_WORDS_RECOVERY_FREE_MAX_THf;
                            break;
                        case DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS:
                            info->field_id = SRAM_PDS_RECOVERY_FREE_MAX_THf;
                            break;
                        default:
                            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unexpected resource type %d\n"), resource_type));
                    }
                    break;
                default:
                    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unexpected dram threshold %d\n"), dram_thresh));
            }
            break;
    }

exit:
  DNXC_FUNC_RETURN;

}

int
  jer2_qax_itm_dram_bound_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
                 DNX_TMC_ITM_DRAM_BOUND_INFO  *info,
    DNX_SAND_OUT DNX_TMC_ITM_DRAM_BOUND_INFO  *exact_info
  )
{
    soc_reg_above_64_val_t data;
    DNX_TMC_INGRESS_DRAM_BOUND_TYPE_E dram_thresh;
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E resource_type;
    uint32 alpha_field_value;
    int alpha_field_id;
 
    DNXC_INIT_FUNC_DEFS;
    
    DNXC_NULL_CHECK(info);
    DNXC_NULL_CHECK(exact_info);

    if (rt_cls_ndx >  JER2_ARAD_ITM_QT_RT_CLS_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    DNXC_IF_ERR_EXIT(soc_mem_read(unit, CGM_VOQ_DRAM_BOUND_PRMSm, MEM_BLOCK_ANY, 
                                    rt_cls_ndx, data));
    for (resource_type = DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES; resource_type <= DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS; resource_type++) {
        for (dram_thresh = DNX_TMC_INGRESS_DRAM_BOUND; dram_thresh < DNX_TMC_INGRESS_DRAM_BOUND_NOF_TYPES; dram_thresh++) {
            itm_mantissa_exp_threshold_info mantissa_exp_info;
            DNX_TMC_ITM_DRAM_BOUND_THRESHOLD* dram_threshold = DNX_TMC_ITM_DRAM_BOUND_INFO_thresh_get(unit, info, dram_thresh, resource_type);
            DNX_TMC_ITM_DRAM_BOUND_THRESHOLD* exact_dram_threshold = DNX_TMC_ITM_DRAM_BOUND_INFO_thresh_get(unit, exact_info, dram_thresh, resource_type);
            

            DNXC_IF_ERR_EXIT(_jer2_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, JER2_QAX_DRAM_BOUND_THRESH_MAX, &mantissa_exp_info));
            DNXC_IF_ERR_EXIT(_jer2_qax_itm_mantissa_exp_field_set(unit,&mantissa_exp_info, 1, data,
                                                               dram_threshold->free_max_threshold, 
                                                               &(exact_dram_threshold->free_max_threshold)));
        
            DNXC_IF_ERR_EXIT(_jer2_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, JER2_QAX_DRAM_BOUND_THRESH_MIN, &mantissa_exp_info));
            DNXC_IF_ERR_EXIT(_jer2_qax_itm_mantissa_exp_field_set(unit,&mantissa_exp_info, 1, data,
                                                               dram_threshold->free_min_threshold, 
                                                               &(exact_dram_threshold->free_min_threshold)));

            DNXC_IF_ERR_EXIT(_jer2_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, JER2_QAX_DRAM_BOUND_THRESH_FREE_MAX, &mantissa_exp_info));
            DNXC_IF_ERR_EXIT(_jer2_qax_itm_mantissa_exp_field_set(unit,&mantissa_exp_info, 1, data,
                                                               dram_threshold->max_threshold, 
                                                               &(exact_dram_threshold->max_threshold)));
        
            DNXC_IF_ERR_EXIT(_jer2_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, JER2_QAX_DRAM_BOUND_THRESH_FREE_MIN, &mantissa_exp_info));
            DNXC_IF_ERR_EXIT(_jer2_qax_itm_mantissa_exp_field_set(unit,&mantissa_exp_info, 1, data,
                                                               dram_threshold->min_threshold, 
                                                               &(exact_dram_threshold->min_threshold)));


            alpha_field_id = jer2_qax_itm_dram_bound_alpha_field_get(unit,dram_thresh, resource_type);
            if (alpha_field_id == INVALIDf) {
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("can't find alpha field name\n")));
            }
            alpha_field_value =  _jer2_qax_itm_alpha_to_field(unit, dram_threshold->alpha);
            soc_mem_field32_set(unit, CGM_VOQ_DRAM_BOUND_PRMSm, &data, alpha_field_id, alpha_field_value);
            exact_dram_threshold->alpha = dram_threshold->alpha;
    
        }
    }

    DNXC_IF_ERR_EXIT(_jer2_qax_itm_mantissa_exp_field_set(unit,&voq_dram_bound_qsize_recovery_th_mant_exp, 1, data,
                                                       info->qsize_recovery_th,
                                                       &(exact_info->qsize_recovery_th)));

    DNXC_IF_ERR_EXIT(soc_mem_write(unit, CGM_VOQ_DRAM_BOUND_PRMSm, MEM_BLOCK_ANY, 
                                     rt_cls_ndx , data));

exit:
  DNXC_FUNC_RETURN;
}

int
  jer2_qax_itm_dram_bound_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_OUT DNX_TMC_ITM_DRAM_BOUND_INFO  *info
  )
{
    soc_reg_above_64_val_t data;
    DNX_TMC_INGRESS_DRAM_BOUND_TYPE_E dram_thresh;
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E resource_type;
    uint32 alpha_field_value;
    int alpha_field_id;
 
    DNXC_INIT_FUNC_DEFS;
    
    DNXC_NULL_CHECK(info);

    if (rt_cls_ndx >  JER2_ARAD_ITM_QT_RT_CLS_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("rate class index %d is out of range\n"), rt_cls_ndx));
    }

    DNXC_IF_ERR_EXIT(soc_mem_read(unit, CGM_VOQ_DRAM_BOUND_PRMSm, MEM_BLOCK_ANY, 
                                    rt_cls_ndx, data));
    for (resource_type = DNX_TMC_INGRESS_THRESHOLD_SRAM_BYTES; resource_type <= DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS; resource_type++) {
 
        for (dram_thresh = DNX_TMC_INGRESS_DRAM_BOUND; dram_thresh < DNX_TMC_INGRESS_DRAM_BOUND_NOF_TYPES; dram_thresh++) {
            DNX_TMC_ITM_DRAM_BOUND_THRESHOLD* dram_threshold = DNX_TMC_ITM_DRAM_BOUND_INFO_thresh_get(unit, info, dram_thresh, resource_type);
            itm_mantissa_exp_threshold_info mantissa_exp_info;

            DNXC_IF_ERR_EXIT(_jer2_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, JER2_QAX_DRAM_BOUND_THRESH_FREE_MAX, &mantissa_exp_info));
            _jer2_qax_itm_mantissa_exp_field_get(unit,&mantissa_exp_info, data,
                                                               &(dram_threshold->free_max_threshold));
        
            DNXC_IF_ERR_EXIT(_jer2_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, JER2_QAX_DRAM_BOUND_THRESH_FREE_MIN, &mantissa_exp_info));
            _jer2_qax_itm_mantissa_exp_field_get(unit,&mantissa_exp_info, data,
                                                               &(dram_threshold->free_min_threshold));

            DNXC_IF_ERR_EXIT(_jer2_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, JER2_QAX_DRAM_BOUND_THRESH_MAX, &mantissa_exp_info));
            _jer2_qax_itm_mantissa_exp_field_get(unit,&mantissa_exp_info, data,
                                                               &(dram_threshold->max_threshold));
        
            DNXC_IF_ERR_EXIT(_jer2_qax_itm_dram_bound_mant_exp_info_get(unit,dram_thresh,resource_type, JER2_QAX_DRAM_BOUND_THRESH_MIN, &mantissa_exp_info));
            _jer2_qax_itm_mantissa_exp_field_get(unit,&mantissa_exp_info, data,
                                                               &(dram_threshold->min_threshold));


            alpha_field_id = jer2_qax_itm_dram_bound_alpha_field_get(unit, dram_thresh, resource_type);
            if (alpha_field_id == INVALIDf) {
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("can't find alpha field name\n")));
            }
            alpha_field_value = soc_mem_field32_get(unit, CGM_VOQ_DRAM_BOUND_PRMSm, &data, alpha_field_id);
            dram_threshold->alpha = _jer2_qax_itm_field_to_alpha(unit,alpha_field_value);
    
        }
    }

    _jer2_qax_itm_mantissa_exp_field_get(unit,&voq_dram_bound_qsize_recovery_th_mant_exp, data,
                                                       &info->qsize_recovery_th);

exit:
  DNXC_FUNC_RETURN;
}


/*****************/
/* VSQ functions */
/* {             */
/*****************/

/*
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
  jer2_qax_itm_vsq_pg_tc_profile_mapping_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core_id,
    DNX_SAND_IN uint32 src_pp_port,
    DNX_SAND_IN int pg_tc_profile
  )
{
    int rv = SOC_E_NONE;
    uint32 data = 0;
    DNXC_INIT_FUNC_DEFS;

    if (core_id < 0 || core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    rv = READ_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), src_pp_port, &data);
    DNXC_IF_ERR_EXIT(rv);

    soc_mem_field32_set(unit, CGM_IPPPMm, &data, TC_BITMAP_INDEXf, pg_tc_profile);

    rv = WRITE_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), src_pp_port, &data);
    DNXC_IF_ERR_EXIT(rv);

exit: 
    DNXC_FUNC_RETURN;
}

/*
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
  jer2_qax_itm_vsq_pg_tc_profile_mapping_get(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core_id,
    DNX_SAND_IN uint32 src_pp_port,
    DNX_SAND_OUT int *pg_tc_profile
  )
{
    int rv = SOC_E_NONE;
    uint32 data = 0;
    DNXC_INIT_FUNC_DEFS;

    if (core_id < 0 || core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    rv = READ_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), src_pp_port, &data);
    DNXC_IF_ERR_EXIT(rv);

    *pg_tc_profile = soc_mem_field32_get(unit, CGM_IPPPMm, &data, TC_BITMAP_INDEXf);

exit: 
    DNXC_FUNC_RETURN;
}

/*
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
  jer2_qax_itm_vsq_pg_tc_profile_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core_id,
    DNX_SAND_IN int pg_tc_profile_id,
    DNX_SAND_IN uint32 pg_tc_bitmap
  )
{
    int rv = SOC_E_NONE;
    SHR_BITDCLNAME(pg_tc_bitmap_data, JER2_QAX_ITM_VSQ_PG_OFFSET_FIELD_SIZE);
    int tc = 0;
    DNXC_INIT_FUNC_DEFS;

    /* bitmap validation */
    if (pg_tc_bitmap & ~((1 << DNX_TMC_NOF_TRAFFIC_CLASSES * 3) - 1)) {
        LOG_ERROR(BSL_LS_SOC_COSQ, 
                  (BSL_META_U(unit, "PG TC mapping bitmap is invalid %d, maximum bit nust be %d\n"), 
                   pg_tc_bitmap, DNX_TMC_NOF_TRAFFIC_CLASSES * 3));
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    for (tc = 0; tc < DNX_TMC_NOF_TRAFFIC_CLASSES; ++tc) {
         /*
          * Get PG-Offset at TC index in pg_tc_bitmap
          * and set the PG-Offset at pg_tc_profile_id index
          */
        SHR_BITCOPY_RANGE(pg_tc_bitmap_data, pg_tc_profile_id * 3, &pg_tc_bitmap, tc * 3, 3);
        rv = WRITE_CGM_VSQ_PG_TC_BITMAPm(unit, CGM_BLOCK(unit, SOC_CORE_ALL), tc, pg_tc_bitmap_data);
        DNXC_IF_ERR_EXIT(rv);
    }

exit: 
    DNXC_FUNC_RETURN;
}

/*
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
  jer2_qax_itm_vsq_pg_tc_profile_get(
    DNX_SAND_IN int         unit,
    DNX_SAND_IN int         core_id,
    DNX_SAND_IN int         pg_tc_profile_id,
    DNX_SAND_OUT uint32     *pg_tc_bitmap
  )
{
    uint32 rv = SOC_E_NONE;
    SHR_BITDCLNAME(pg_tc_bitmap_data, JER2_QAX_ITM_VSQ_PG_OFFSET_FIELD_SIZE);
    uint32 pg_offset_bitmap = 0;
    int tc = 0;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(pg_tc_bitmap);

    for (tc = 0; tc < DNX_TMC_NOF_TRAFFIC_CLASSES; ++tc) {
        rv = READ_CGM_VSQ_PG_TC_BITMAPm(unit, CGM_BLOCK(unit, core_id), tc, pg_tc_bitmap_data);
        DNXC_IF_ERR_EXIT(rv);

        SHR_BITCOPY_RANGE(&pg_offset_bitmap, tc * 3, pg_tc_bitmap_data, pg_tc_profile_id * 3, 3);
    }

    *pg_tc_bitmap = pg_offset_bitmap;

exit: 
    DNXC_FUNC_RETURN;
}

/*
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
  jer2_qax_itm_vsq_pb_prm_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              pg_ndx,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_PG_PRM *pg_prm
  )
{
    int rv = SOC_E_NONE;
    uint32 data = 0;   
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(pg_prm);

    if (pg_ndx >= DNX_TMC_ITM_VSQ_GROUPF_SZE(unit)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
   
    rv = READ_CGM_PB_VSQ_PRMSm(unit, CGM_BLOCK(unit, SOC_CORE_ALL), pg_ndx, &data);
    DNXC_IF_ERR_EXIT(rv);
    
    soc_mem_field32_set(unit, CGM_PB_VSQ_PRMSm, &data, IS_LOSSLESSf, pg_prm->is_lossles);
    soc_mem_field32_set(unit, CGM_PB_VSQ_PRMSm, &data, POOL_IDf, pg_prm->pool_id);
    soc_mem_field32_set(unit, CGM_PB_VSQ_PRMSm, &data, USE_PORT_GRNTDf, pg_prm->use_min_port);
    soc_mem_field32_set(unit, CGM_PB_VSQ_PRMSm, &data, ADMT_PROFILEf, pg_prm->admit_profile);

    rv = WRITE_CGM_PB_VSQ_PRMSm(unit, CGM_BLOCK(unit, SOC_CORE_ALL), pg_ndx, &data);
    DNXC_IF_ERR_EXIT(rv);

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_vsq_pb_prm_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              pg_ndx,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_PG_PRM *pg_prm
  )
{
    int rv = SOC_E_NONE;
    uint32 data = 0;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(pg_prm);

    if (pg_ndx >= DNX_TMC_ITM_VSQ_GROUPF_SZE(unit)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
   
    rv = READ_CGM_PB_VSQ_PRMSm(unit, CGM_BLOCK(unit, SOC_CORE_ALL), pg_ndx, &data);
    DNXC_IF_ERR_EXIT(rv);
    
    pg_prm->is_lossles = soc_mem_field32_get(unit, CGM_PB_VSQ_PRMSm, &data, IS_LOSSLESSf);
    pg_prm->pool_id = soc_mem_field32_get(unit, CGM_PB_VSQ_PRMSm, &data, POOL_IDf);
    pg_prm->use_min_port = soc_mem_field32_get(unit, CGM_PB_VSQ_PRMSm, &data, USE_PORT_GRNTDf);
    pg_prm->admit_profile = soc_mem_field32_get(unit, CGM_PB_VSQ_PRMSm, &data, ADMT_PROFILEf);

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_src_vsqs_mapping_set(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core_id,
    DNX_SAND_IN int src_pp_port,
    DNX_SAND_IN int src_port_vsq_index,
    DNX_SAND_IN int pg_base,
    DNX_SAND_IN uint8 enable
  )
{
    int rv = SOC_E_NONE, data = 0;
    DNXC_INIT_FUNC_DEFS;

    if (core_id < 0 || core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (src_pp_port < -1 || src_pp_port > SOC_MAX_NUM_PORTS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (src_port_vsq_index < 0 || src_port_vsq_index >= DNX_TMC_ITM_VSQ_GROUPE_SZE(unit)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (pg_base < 0 || pg_base >= DNX_TMC_ITM_VSQ_GROUPF_SZE(unit)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    rv = READ_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), src_pp_port, &data);
    DNXC_IF_ERR_EXIT(rv);

    /*VSQE mapping*/
    /*In-PP-Port => VSQ-E index */
    soc_mem_field32_set(unit, CGM_IPPPMm, &data, NIF_PORTf, enable ? src_port_vsq_index : 0);

    /*VSQF mapping*/
    /*In-PP-Port(8) => { PG-Base(9), PG-MAP-Profile-Bitmap-Index(4)}*/
    soc_mem_field32_set(unit, CGM_IPPPMm, &data, PG_BASEf, enable ? pg_base : 0);

    rv = WRITE_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), src_pp_port, &data);
    DNXC_IF_ERR_EXIT(rv);

exit: 
    DNXC_FUNC_RETURN;
}

/*
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
  jer2_qax_itm_src_vsqs_mapping_get(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  int core_id,
    DNX_SAND_IN  int src_pp_port,
    DNX_SAND_OUT int *src_port_vsq_index,
    DNX_SAND_OUT int *pg_base,
    DNX_SAND_OUT uint8 *enable
  )
{
    int rv = SOC_E_NONE, data = 0;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(src_port_vsq_index);
    DNXC_NULL_CHECK(pg_base);
    DNXC_NULL_CHECK(enable);

    if (core_id < 0 || core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (src_pp_port < 0 || src_pp_port >= SOC_MAX_NUM_PORTS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    rv = READ_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), src_pp_port, &data);
    DNXC_IF_ERR_EXIT(rv);

    /*VSQE mapping*/
    *src_port_vsq_index = soc_mem_field32_get(unit, CGM_IPPPMm, &data, NIF_PORTf);
    /*VSQF mapping*/
    *pg_base = soc_mem_field32_get(unit, CGM_IPPPMm, &data, PG_BASEf);

    *enable = 0;
    if (!(*src_port_vsq_index == 0 && *pg_base == 0)) {
        *enable = TRUE;
    }
exit: 
    DNXC_FUNC_RETURN;
}

/*
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
    jer2_qax_itm_vsq_wred_get(
        DNX_SAND_IN  int                    unit,
        DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
        DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
        DNX_SAND_IN  uint32                 drop_precedence_ndx,
        DNX_SAND_IN  int                    pool_id,
        DNX_SAND_OUT DNX_TMC_ITM_WRED_QT_DP_INFO *info) 
{
    int res = SOC_E_NONE;
    uint32 entry_offset = 0;
    soc_reg_above_64_val_t data;
    const soc_mem_t mem_arr_CGM_VSQ_WRED_RJCT_PRMS[DNX_TMC_NOF_VSQ_GROUPS] = {CGM_VSQA_WRED_RJCT_PRMSm, CGM_VSQB_WRED_RJCT_PRMSm, CGM_VSQC_WRED_RJCT_PRMSm, CGM_VSQD_WRED_RJCT_PRMSm, CGM_VSQE_WRED_RJCT_PRMSm, CGM_VSQF_WRED_RJCT_PRMSm};
    DNX_TMC_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA  tbl_data;
    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);

    if (vsq_group_ndx < 0 || vsq_group_ndx >= DNX_TMC_NOF_VSQ_GROUPS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    } 
    if (vsq_group_ndx == DNX_TMC_ITM_VSQ_GROUP_CTGRY) {
        if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_A_QT_RT_CLS_MAX(unit)) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
    } else {
        if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }
    if (drop_precedence_ndx >= DNX_TMC_NOF_DROP_PRECEDENCE) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (pool_id < 0 || pool_id >= DNX_TMC_ITM_NOF_RSRC_POOLS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    entry_offset = (vsq_rt_cls_ndx * DNX_TMC_NOF_DROP_PRECEDENCE) + drop_precedence_ndx;
    if (vsq_group_ndx == DNX_TMC_ITM_VSQ_GROUP_SRC_PORT) {
        /* VSQ-E table is doubled. The first half is for pool 0 and the second half is for pool 1 */
        entry_offset += pool_id * (DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit) * DNX_TMC_NOF_DROP_PRECEDENCE);
    }

    res = soc_mem_read(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    DNXC_IF_ERR_EXIT(res); 

    tbl_data.c3 = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_C_3f);
    tbl_data.c2 = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_C_2f);
    tbl_data.c1 = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_C_1f);
    tbl_data.vq_wred_pckt_sz_ignr = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_IGNR_PKT_SIZEf);   
    tbl_data.max_avrg_th = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_MAX_THf);
    tbl_data.min_avrg_th = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_MIN_THf);
    info->wred_en = DNX_SAND_NUM2BOOL(soc_mem_field32_get(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_ENf));

    res = jer2_qax_itm_vsq_WRED_TBL_DATA_to_WRED_QT_DP_INFO(unit, &tbl_data, info);
    DNXC_IF_ERR_EXIT(res);

exit: 
    DNXC_FUNC_RETURN; 
}

/*
 * \brief
 *  Set VSQ's WRED information.
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
    jer2_qax_itm_vsq_wred_set(
        DNX_SAND_IN  int                    unit,
        DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
        DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
        DNX_SAND_IN  uint32                 drop_precedence_ndx,
        DNX_SAND_IN  int                    pool_id,
        DNX_SAND_IN  DNX_TMC_ITM_WRED_QT_DP_INFO *info,
        DNX_SAND_OUT DNX_TMC_ITM_WRED_QT_DP_INFO *exact_info)
{
    int res = SOC_E_NONE;
    uint32 entry_offset = 0;
    soc_reg_above_64_val_t data;
    const soc_mem_t mem_arr_CGM_VSQ_WRED_RJCT_PRMS[DNX_TMC_NOF_VSQ_GROUPS] = {CGM_VSQA_WRED_RJCT_PRMSm, CGM_VSQB_WRED_RJCT_PRMSm, CGM_VSQC_WRED_RJCT_PRMSm, CGM_VSQD_WRED_RJCT_PRMSm, CGM_VSQE_WRED_RJCT_PRMSm, CGM_VSQF_WRED_RJCT_PRMSm};
    DNX_TMC_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA  tbl_data;
    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);

    if (vsq_group_ndx < 0 || vsq_group_ndx >= DNX_TMC_NOF_VSQ_GROUPS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    } 
    if (vsq_group_ndx == DNX_TMC_ITM_VSQ_GROUP_CTGRY) {
        if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_A_QT_RT_CLS_MAX(unit)) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
    } else {
        if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }
    if (drop_precedence_ndx >= DNX_TMC_NOF_DROP_PRECEDENCE) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (pool_id < 0 || pool_id >= DNX_TMC_ITM_NOF_RSRC_POOLS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    entry_offset = (vsq_rt_cls_ndx * DNX_TMC_NOF_DROP_PRECEDENCE) + drop_precedence_ndx;
    if (vsq_group_ndx == DNX_TMC_ITM_VSQ_GROUP_SRC_PORT) {
        /* VSQ-E table is doubled. The first half is for pool 0 and the second half is for pool 1 */
        entry_offset += pool_id * (DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit) * DNX_TMC_NOF_DROP_PRECEDENCE);
    }

    res = soc_mem_read(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    DNXC_IF_ERR_EXIT(res); 

    res = jer2_qax_itm_vsq_WRED_QT_DP_INFO_to_WRED_TBL_DATA(unit, info, &tbl_data);
    DNXC_IF_ERR_EXIT(res);

    res = jer2_qax_itm_vsq_WRED_TBL_DATA_to_WRED_QT_DP_INFO(unit, &tbl_data, exact_info);
    DNXC_IF_ERR_EXIT(res);
    exact_info->wred_en = DNX_SAND_BOOL2NUM(info->wred_en);

    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_C_3f, tbl_data.c3);
    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_C_2f, tbl_data.c2);
    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_C_1f, tbl_data.c1);
    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_IGNR_PKT_SIZEf, tbl_data.vq_wred_pckt_sz_ignr);   
    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_MAX_THf, tbl_data.max_avrg_th);
    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_MIN_THf, tbl_data.min_avrg_th);
    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], data, WRED_ENf, DNX_SAND_NUM2BOOL(info->wred_en));

    res = soc_mem_write(unit, mem_arr_CGM_VSQ_WRED_RJCT_PRMS[vsq_group_ndx], CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    DNXC_IF_ERR_EXIT(res); 
exit: 
    DNXC_FUNC_RETURN; 
}

/*
 * \brief
 *  Convert WRED_QT_DP_INFO to WRED_TBL_DATA.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] wred_param -
 *      WRED paramters.
 *  \param [inout] tbl_data -
 *      WRED table.
 */
static uint32
  jer2_qax_itm_vsq_WRED_QT_DP_INFO_to_WRED_TBL_DATA(
    DNX_SAND_IN int                                                     unit,
    DNX_SAND_IN DNX_TMC_ITM_WRED_QT_DP_INFO                            *wred_param,
    DNX_SAND_INOUT DNX_TMC_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA *tbl_data
  )
{
  uint32
    res,
    max_prob,
    calc,
    max_val_c1,
    max_avrg_th_16_byte;
  int32
    avrg_th_diff_wred_granular = 0;
  int32
    min_avrg_th_exact_wred_granular,
    max_avrg_th_exact_wred_granular;
  uint32
    trunced;
  DNX_SAND_U64
    u64_1,
    u64_2,
    u64_c2 = {{0}};
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(wred_param); 
    DNXC_NULL_CHECK(tbl_data); 
    trunced = FALSE;

    /*
    * min_avrg_th
    */
    tbl_data->min_avrg_th = 0;
    max_avrg_th_16_byte = DNX_SAND_DIV_ROUND_UP(wred_param->min_avrg_th,DNX_TMC_ITM_WRED_GRANULARITY);
    res = jer2_arad_itm_man_exp_buffer_set(
          max_avrg_th_16_byte,
          DNX_TMC_ITM_VSQ_WRED_AVRG_TH_MNT_LSB,
          DNX_TMC_ITM_VSQ_WRED_AVRG_TH_MNT_NOF_BITS,
          DNX_TMC_ITM_VSQ_WRED_AVRG_TH_EXP_LSB,
          DNX_TMC_ITM_VSQ_WRED_AVRG_TH_EXP_NOF_BITS,
          FALSE,
          &(tbl_data->min_avrg_th),
          &min_avrg_th_exact_wred_granular
        );
    DNXC_SAND_IF_ERR_EXIT(res);
    /* min_avrg_th_exact *= JER2_ARAD_ITM_WRED_GRANULARITY; */
    /*
    * max_avrg_th
    */
    tbl_data->max_avrg_th = 0;
    res = jer2_arad_itm_man_exp_buffer_set(
            DNX_SAND_DIV_ROUND_UP(wred_param->max_avrg_th,DNX_TMC_ITM_WRED_GRANULARITY),
            DNX_TMC_ITM_VSQ_WRED_AVRG_TH_MNT_LSB,
            DNX_TMC_ITM_VSQ_WRED_AVRG_TH_MNT_NOF_BITS,
            DNX_TMC_ITM_VSQ_WRED_AVRG_TH_EXP_LSB,
            DNX_TMC_ITM_VSQ_WRED_AVRG_TH_EXP_NOF_BITS,
            FALSE,
            &(tbl_data->max_avrg_th),
            &max_avrg_th_exact_wred_granular
            );
    DNXC_SAND_IF_ERR_EXIT(res);

    /* max_avrg_th_exact *= DNX_TMC_ITM_WRED_GRANULARITY; */
    /*
    * max_packet_size
    */
    calc = wred_param->max_packet_size;
    if (calc > DNX_TMC_ITM_WRED_MAX_PACKET_SIZE_FOR_CALC) {
        calc = DNX_TMC_ITM_WRED_MAX_PACKET_SIZE_FOR_CALC;
    } 
    calc = DNX_SAND_DIV_ROUND_UP(calc, DNX_TMC_ITM_WRED_GRANULARITY);
    tbl_data->c3 = dnx_sand_log2_round_up(calc);

    /*
    *  Packet size ignore
    */
    tbl_data->vq_wred_pckt_sz_ignr = wred_param->ignore_packet_size;
    /*
     * max_probability
     */
    max_prob = (wred_param->max_probability);
    if(max_prob>=100) {
        max_prob = 99;
    }
    /*
    * max_probability
    * C1 = ((2^32)/100)*max-prob / (max-th - min-th) in powers of 2
    */
    calc = DNX_TMC_WRED_NORMALIZE_FACTOR * max_prob;
    /*
     * We do not use 'DNX_SAND_DIV_ROUND' or 'DNX_SAND_DIV_ROUND_UP'
     * because at this point we might have in calc '((2^32)/100)*max-prob'
     * which can be very large number and the other dividers do ADD before
     * the division.
     */
    max_val_c1 = 31; /* dnx_sand_log2_round_down(0xFFFFFFFF) */
    avrg_th_diff_wred_granular = (max_avrg_th_exact_wred_granular - min_avrg_th_exact_wred_granular);
    if(avrg_th_diff_wred_granular == 0) {
        tbl_data->c1 = max_val_c1;
    } else {
        calc = DNX_SAND_DIV_ROUND_DOWN(calc, avrg_th_diff_wred_granular);
        tbl_data->c1 = dnx_sand_log2_round_down(calc);
    } 
    if(tbl_data->c1 < max_val_c1) {
        /*
         * Check if a bigger C1 gives closer result of the value we add.
         */
        uint32
            now     = 1 <<(tbl_data->c1),
            changed = 1 <<(tbl_data->c1+1),
            diff_with_now,
            diff_with_change;
        diff_with_change = changed-calc;

        diff_with_now    = calc-now;
        if( diff_with_change < diff_with_now) {
            tbl_data->c1 += 1;
        }
    }
    DNX_SAND_LIMIT_FROM_ABOVE(tbl_data->c1, max_val_c1);
    if (max_avrg_th_16_byte > 0) {
        max_val_c1 = DNX_SAND_DIV_ROUND_DOWN(0xFFFFFFFF, max_avrg_th_16_byte);
        max_val_c1 = dnx_sand_log2_round_down(max_val_c1);
        DNX_SAND_LIMIT_FROM_ABOVE(tbl_data->c1, max_val_c1);
    }
    /*
    * max_probability
    * C2 = FACTOR * max-prob * min-th / (max-th - min-th)
    */
    dnx_sand_u64_multiply_longs(DNX_TMC_WRED_NORMALIZE_FACTOR, max_prob * min_avrg_th_exact_wred_granular, &u64_2);
    dnx_sand_u64_devide_u64_long(&u64_2, avrg_th_diff_wred_granular, &u64_c2);
    /* 
    * Validate C2 value. 
    * P-Drop(red) = (c1*AvgSize-c2)*PcktSize*c3 
    * So validate that C2 > C1*MinAvgSize, otherwise C2 = C1*MinAvgSize
    */
    dnx_sand_u64_multiply_longs(min_avrg_th_exact_wred_granular, (1 << tbl_data->c1),&u64_1);
    if(dnx_sand_u64_is_bigger(&u64_c2, &u64_1)) {
        sal_memcpy(&u64_c2, &u64_1, sizeof(DNX_SAND_U64));
    }

    trunced = dnx_sand_u64_to_long(&u64_c2, &tbl_data->c2);
    if (trunced) {
        tbl_data->c2 = 0xFFFFFFFF;
    }
exit: 
    DNXC_FUNC_RETURN; 
}

/*
 * \brief
 *  Convert WRED_TBL_DATA to WRED_QT_DP_INFO.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] tbl_data -
 *      WRED table.
 *  \param [out] wred_param -
 *      WRED paramters.
 */
static uint32
  jer2_qax_itm_vsq_WRED_TBL_DATA_to_WRED_QT_DP_INFO(
     DNX_SAND_IN int                                                     unit,
     DNX_SAND_IN  DNX_TMC_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA  *tbl_data,
     DNX_SAND_OUT DNX_TMC_ITM_WRED_QT_DP_INFO                            *wred_param
  )
{
    uint32
      res;
    uint32
      avrg_th_diff_wred_granular,
      two_power_c1,
      remainder;
    DNX_SAND_U64
      u64_1,
      u64_2;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(wred_param); 
    DNXC_NULL_CHECK(tbl_data); 

    res = jer2_arad_itm_man_exp_buffer_get(
            tbl_data->min_avrg_th,
            DNX_TMC_ITM_VSQ_WRED_AVRG_TH_MNT_LSB,
            DNX_TMC_ITM_VSQ_WRED_AVRG_TH_MNT_NOF_BITS,
            DNX_TMC_ITM_VSQ_WRED_AVRG_TH_EXP_LSB,
            DNX_TMC_ITM_VSQ_WRED_AVRG_TH_EXP_NOF_BITS,
            FALSE,
            (int32*)&(wred_param->min_avrg_th)
          );
    DNXC_SAND_IF_ERR_EXIT(res);
    wred_param->min_avrg_th *= DNX_TMC_ITM_WRED_GRANULARITY;
    /*
     * max_avrg_th
     */
    res = jer2_arad_itm_man_exp_buffer_get(
            tbl_data->max_avrg_th,
            DNX_TMC_ITM_VSQ_WRED_AVRG_TH_MNT_LSB,
            DNX_TMC_ITM_VSQ_WRED_AVRG_TH_MNT_NOF_BITS,
            DNX_TMC_ITM_VSQ_WRED_AVRG_TH_EXP_LSB,
            DNX_TMC_ITM_VSQ_WRED_AVRG_TH_EXP_NOF_BITS,
            FALSE,
            (int32*)&(wred_param->max_avrg_th)
          );
    DNXC_SAND_IF_ERR_EXIT(res);
    wred_param->max_avrg_th *= DNX_TMC_ITM_WRED_GRANULARITY;
    /*
     * max_packet_size
     */
    wred_param->max_packet_size = (0x1<<(tbl_data->c3))*DNX_TMC_ITM_WRED_GRANULARITY;
    /*
     *  Packet size ignore
     */
    wred_param->ignore_packet_size = DNX_SAND_NUM2BOOL(tbl_data->vq_wred_pckt_sz_ignr);
    avrg_th_diff_wred_granular = (wred_param->max_avrg_th - wred_param->min_avrg_th) / DNX_TMC_ITM_WRED_GRANULARITY;
    two_power_c1 = 1<<tbl_data->c1;
    /*
     * C1 = ((2^32)/100)*max-prob / (max-th - min-th) in powers of 2
     * ==>
     * max-prob =  ( 2^C1 * (max-th - min-th) ) / ((2^32)/100)
     */
    dnx_sand_u64_multiply_longs(two_power_c1, avrg_th_diff_wred_granular, &u64_1);
    remainder = dnx_sand_u64_devide_u64_long(&u64_1, DNX_TMC_WRED_NORMALIZE_FACTOR, &u64_2);
    dnx_sand_u64_to_long(&u64_2, &wred_param->max_probability);

    if(remainder > (DNX_TMC_WRED_NORMALIZE_FACTOR/2)) {
      wred_param->max_probability++;
    }

    if(wred_param->max_probability > 100) {
      wred_param->max_probability = 100;
    }

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_vsq_wred_gen_set(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP          vsq_group_ndx,
    DNX_SAND_IN  uint32                         vsq_rt_cls_ndx,
    DNX_SAND_IN  int                            pool_id,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_WRED_GEN_INFO  *info
  )
{
    int res = SOC_E_NONE;
    uint32 data;
    const soc_mem_t mem_arr_CGM_VSQ_AVRG_PRMS[DNX_TMC_NOF_VSQ_GROUPS] = 
        {CGM_VSQA_AVRG_PRMSm, CGM_VSQB_AVRG_PRMSm, CGM_VSQC_AVRG_PRMSm, CGM_VSQD_AVRG_PRMSm, CGM_VSQE_AVRG_PRMSm, CGM_VSQF_AVRG_PRMSm};
    DNXC_INIT_FUNC_DEFS;

    if (vsq_group_ndx >= DNX_TMC_NOF_VSQ_GROUPS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (vsq_group_ndx == DNX_TMC_ITM_VSQ_GROUP_CTGRY) {
        if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_A_QT_RT_CLS_MAX(unit)) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
    } else {
        if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }   

    if (info->exp_wq >= (0x1 << soc_mem_field_length(unit, CGM_VSQA_AVRG_PRMSm, AVRG_WEIGHTf))) {
      DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    res = soc_mem_read(unit, mem_arr_CGM_VSQ_AVRG_PRMS[vsq_group_ndx], CGM_BLOCK(unit, SOC_CORE_ALL), vsq_rt_cls_ndx, &data);
    DNXC_IF_ERR_EXIT(res);
    
    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_AVRG_PRMS[vsq_group_ndx], &data, AVRG_UPDT_ENf, info->wred_en);
    soc_mem_field32_set(unit, mem_arr_CGM_VSQ_AVRG_PRMS[vsq_group_ndx], &data, AVRG_WEIGHTf, info->exp_wq);

    res = soc_mem_write(unit, mem_arr_CGM_VSQ_AVRG_PRMS[vsq_group_ndx], CGM_BLOCK(unit, SOC_CORE_ALL), vsq_rt_cls_ndx, &data);
    DNXC_IF_ERR_EXIT(res);

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_vsq_wred_gen_get(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP          vsq_group_ndx,
    DNX_SAND_IN  uint32                         vsq_rt_cls_ndx,
    DNX_SAND_IN  int                            pool_id,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_WRED_GEN_INFO  *info
  )
{
    int res = SOC_E_NONE;
    uint32 data;
    const soc_mem_t mem_arr_CGM_VSQ_AVRG_PRMS[DNX_TMC_NOF_VSQ_GROUPS] = 
        {CGM_VSQA_AVRG_PRMSm, CGM_VSQB_AVRG_PRMSm, CGM_VSQC_AVRG_PRMSm, CGM_VSQD_AVRG_PRMSm, CGM_VSQE_AVRG_PRMSm, CGM_VSQF_AVRG_PRMSm};
    DNXC_INIT_FUNC_DEFS;

    if (vsq_group_ndx >= DNX_TMC_NOF_VSQ_GROUPS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (vsq_group_ndx == DNX_TMC_ITM_VSQ_GROUP_CTGRY) {
        if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_A_QT_RT_CLS_MAX(unit)) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
    } else {
        if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }   

    res = soc_mem_read(unit, mem_arr_CGM_VSQ_AVRG_PRMS[vsq_group_ndx], CGM_BLOCK(unit, SOC_CORE_ALL), vsq_rt_cls_ndx, &data);
    DNXC_IF_ERR_EXIT(res);

    info->wred_en = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_AVRG_PRMS[vsq_group_ndx], &data, AVRG_UPDT_ENf);
    info->exp_wq = soc_mem_field32_get(unit, mem_arr_CGM_VSQ_AVRG_PRMS[vsq_group_ndx], &data, AVRG_WEIGHTf);

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_vsq_tail_drop_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  int                    pool_id,
    DNX_SAND_IN  int                    is_headroom,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_TAIL_DROP_INFO  *info,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_TAIL_DROP_INFO  *exact_info
  )
{
    int rv = SOC_E_NONE;
    uint32 entry_offset;
    const soc_mem_t mem_arr_CGM_VSQ_RJCT_PRMS[DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES][DNX_TMC_NOF_NON_SRC_BASED_VSQ_GROUPS] = 
       {{CGM_VSQA_WORDS_RJCT_PRMSm, CGM_VSQB_WORDS_RJCT_PRMSm, CGM_VSQC_WORDS_RJCT_PRMSm, CGM_VSQD_WORDS_RJCT_PRMSm, },
        {CGM_VSQA_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQB_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQC_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQD_SRAM_BUFFERS_RJCT_PRMSm,},
        {CGM_VSQA_SRAM_PDS_RJCT_PRMSm, CGM_VSQB_SRAM_PDS_RJCT_PRMSm, CGM_VSQC_SRAM_PDS_RJCT_PRMSm, CGM_VSQD_SRAM_PDS_RJCT_PRMSm}};
    const soc_field_t field_arr_MAX_SIZE_TH_DP[DNX_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_SIZE_TH_DP_0f, MAX_SIZE_TH_DP_1f, MAX_SIZE_TH_DP_2f, MAX_SIZE_TH_DP_3f};
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = DNX_TMC_INGRESS_THRESHOLD_INVALID;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);
    DNXC_NULL_CHECK(exact_info);

    rv = jer2_qax_itm_vsq_tail_drop_default_get(unit, exact_info);
    DNXC_SAND_IF_ERR_EXIT(rv);

    if (vsq_group_ndx < 0 || vsq_group_ndx >= DNX_TMC_NOF_NON_SRC_BASED_VSQ_GROUPS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    } 
    if (vsq_group_ndx == DNX_TMC_ITM_VSQ_GROUP_CTGRY) {
        if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_A_QT_RT_CLS_MAX(unit)) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
    } else {
        if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }   
    if (drop_precedence_ndx >= DNX_TMC_NOF_DROP_PRECEDENCE) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    entry_offset = vsq_rt_cls_ndx;

    for (rsrc_type = 0; rsrc_type < DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type][vsq_group_ndx], 
                field_arr_MAX_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                info->max_inst_q_size_th[rsrc_type], 
                &exact_info->max_inst_q_size_th[rsrc_type]);
        DNXC_IF_ERR_EXIT(rv);
    }

exit: 
    DNXC_FUNC_RETURN; 
}

/*
 * \brief
 *  Return default tail drop information. \n
 *  \n
 *   Set taildrop thresholds to maximum size. \n
 *   Zero DNX_TMC_ITM_VSQ_TAIL_DROP_INFO params that are not used.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [out] info -
 *      Default tail drop information.
 */
int
  jer2_qax_itm_vsq_tail_drop_default_get(
      DNX_SAND_IN  int                 unit,
      DNX_SAND_OUT DNX_TMC_ITM_VSQ_TAIL_DROP_INFO  *info
  )
{
    uint32 max_size_exp = 0, max_size_mnt = 0;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);

    /* Zero non-relevant params */
    /* Jericho2 TBD: Remove this line */
    info->max_inst_q_size = info->max_inst_q_size_bds = info->alpha = 0;

    /* Set default taildrop (max size) */
    jer2_arad_iqm_mantissa_exponent_get(unit, (0x1 << soc_mem_field_length(unit, CGM_VSQA_WORDS_RJCT_PRMSm, MAX_SIZE_TH_DP_0f)) - 1, CGM_ITM_VSQ_MANTISSA_BITS, &max_size_mnt, &max_size_exp);
    info->max_inst_q_size_th[DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES] = (max_size_mnt * (0x1 << max_size_exp));
    jer2_arad_iqm_mantissa_exponent_get(unit, (0x1 << soc_mem_field_length(unit, CGM_VSQA_SRAM_BUFFERS_RJCT_PRMSm, MAX_SIZE_TH_DP_0f)) - 1, CGM_ITM_VSQ_MANTISSA_BITS, &max_size_mnt, &max_size_exp);
    info->max_inst_q_size_th[DNX_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS] = (max_size_mnt * (0x1 << max_size_exp));
    jer2_arad_iqm_mantissa_exponent_get(unit, (0x1 << soc_mem_field_length(unit, CGM_VSQA_SRAM_PDS_RJCT_PRMSm, MAX_SIZE_TH_DP_0f)) - 1, CGM_ITM_VSQ_MANTISSA_BITS, &max_size_mnt, &max_size_exp);
    info->max_inst_q_size_th[DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS] = (max_size_mnt * (0x1 << max_size_exp));

exit:
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_vsq_tail_drop_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  int                    pool_id,
    DNX_SAND_IN  int                    is_headroom,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_TAIL_DROP_INFO  *info
  )
{
    int rv = SOC_E_NONE;
    uint32 entry_offset;
    const soc_mem_t mem_arr_CGM_VSQ_RJCT_PRMS[DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES][DNX_TMC_NOF_NON_SRC_BASED_VSQ_GROUPS] = 
       {{CGM_VSQA_WORDS_RJCT_PRMSm, CGM_VSQB_WORDS_RJCT_PRMSm, CGM_VSQC_WORDS_RJCT_PRMSm, CGM_VSQD_WORDS_RJCT_PRMSm, },
        {CGM_VSQA_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQB_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQC_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQD_SRAM_BUFFERS_RJCT_PRMSm,},
        {CGM_VSQA_SRAM_PDS_RJCT_PRMSm, CGM_VSQB_SRAM_PDS_RJCT_PRMSm, CGM_VSQC_SRAM_PDS_RJCT_PRMSm, CGM_VSQD_SRAM_PDS_RJCT_PRMSm}};
    const soc_field_t field_arr_MAX_SIZE_TH_DP[DNX_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_SIZE_TH_DP_0f, MAX_SIZE_TH_DP_1f, MAX_SIZE_TH_DP_2f, MAX_SIZE_TH_DP_3f};
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = DNX_TMC_INGRESS_THRESHOLD_INVALID;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);

    rv = jer2_qax_itm_vsq_tail_drop_default_get(unit, info);
    DNXC_SAND_IF_ERR_EXIT(rv);

    if (vsq_group_ndx < 0 || vsq_group_ndx >= DNX_TMC_NOF_NON_SRC_BASED_VSQ_GROUPS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    } 
    if (vsq_group_ndx == DNX_TMC_ITM_VSQ_GROUP_CTGRY) {
        if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_A_QT_RT_CLS_MAX(unit)) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
    } else {
        if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }   
    if (drop_precedence_ndx >= DNX_TMC_NOF_DROP_PRECEDENCE) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    entry_offset = vsq_rt_cls_ndx;

    for (rsrc_type = 0; rsrc_type < DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type][vsq_group_ndx], 
                field_arr_MAX_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                &info->max_inst_q_size_th[rsrc_type]);
        DNXC_IF_ERR_EXIT(rv);
    }

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_vsq_src_port_rjct_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  int                    pool_id,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_SRC_PORT_INFO       *info,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_SRC_PORT_INFO       *exact_info
  )
{
    uint32 rv = SOC_E_NONE;
    uint32 entry_offset = vsq_rt_cls_ndx;
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = DNX_TMC_INGRESS_THRESHOLD_INVALID;
    const soc_mem_t mem_arr_CGM_VSQ_RJCT_PRMS[DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        {CGM_VSQE_WORDS_RJCT_PRMSm, CGM_VSQE_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQE_SRAM_PDS_RJCT_PRMSm};
    const soc_field_t field_arr_MAX_GRNTD_SIZE_TH_DP[DNX_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_GRNTD_SIZE_TH_DP_0f, MAX_GRNTD_SIZE_TH_DP_1f, MAX_GRNTD_SIZE_TH_DP_2f, MAX_GRNTD_SIZE_TH_DP_3f};
    const soc_field_t field_arr_MAX_SHRD_SIZE_TH_DP[DNX_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_SHRD_SIZE_TH_DP_0f, MAX_SHRD_SIZE_TH_DP_1f, MAX_SHRD_SIZE_TH_DP_2f, MAX_SHRD_SIZE_TH_DP_3f};
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);
    DNXC_NULL_CHECK(exact_info);

    if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (drop_precedence_ndx >= DNX_TMC_NOF_DROP_PRECEDENCE) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (pool_id < 0 || pool_id >= DNX_TMC_ITM_NOF_RSRC_POOLS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    /* VSQ-E table is doubled. The first half is for pool 0 and the second half is for pool 1 */
    entry_offset += pool_id * vsq_rt_cls_ndx;

    for (rsrc_type = 0; rsrc_type < DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        /* Set Guaranteed */
        rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                field_arr_MAX_GRNTD_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                info->max_guaranteed[rsrc_type], 
                &exact_info->max_guaranteed[rsrc_type]);
        DNXC_IF_ERR_EXIT(rv);

        /* Set Shared Pool */
        rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                field_arr_MAX_SHRD_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                info->max_shared[rsrc_type], 
                &exact_info->max_shared[rsrc_type]);
        DNXC_IF_ERR_EXIT(rv);

        /* Set Headroom */
        rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                MAX_HDRM_SIZE_THf, 
                entry_offset,
                info->max_headroom[rsrc_type], 
                &exact_info->max_headroom[rsrc_type]);
        DNXC_IF_ERR_EXIT(rv);
    }

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_vsq_src_port_rjct_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                 vsq_rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_IN  int                    pool_id,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_SRC_PORT_INFO       *info
  )
{
    uint32 rv = SOC_E_NONE;
    uint32 entry_offset = vsq_rt_cls_ndx;
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = DNX_TMC_INGRESS_THRESHOLD_INVALID;
    const soc_mem_t mem_arr_CGM_VSQ_RJCT_PRMS[DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        {CGM_VSQE_WORDS_RJCT_PRMSm, CGM_VSQE_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQE_SRAM_PDS_RJCT_PRMSm};
    const soc_field_t field_arr_MAX_GRNTD_SIZE_TH_DP[DNX_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_GRNTD_SIZE_TH_DP_0f, MAX_GRNTD_SIZE_TH_DP_1f, MAX_GRNTD_SIZE_TH_DP_2f, MAX_GRNTD_SIZE_TH_DP_3f};
    const soc_field_t field_arr_MAX_SHRD_SIZE_TH_DP[DNX_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_SHRD_SIZE_TH_DP_0f, MAX_SHRD_SIZE_TH_DP_1f, MAX_SHRD_SIZE_TH_DP_2f, MAX_SHRD_SIZE_TH_DP_3f};
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);

    if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (drop_precedence_ndx >= DNX_TMC_NOF_DROP_PRECEDENCE) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (pool_id < 0 || pool_id >= DNX_TMC_ITM_NOF_RSRC_POOLS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    /* VSQ-E table is doubled. The first half is for pool 0 and the second half is for pool 1 */
    entry_offset += pool_id * vsq_rt_cls_ndx;

    for (rsrc_type = 0; rsrc_type < DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        /* Set Guaranteed */
        rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                field_arr_MAX_GRNTD_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                &info->max_guaranteed[rsrc_type]);
        DNXC_IF_ERR_EXIT(rv);

        /* Set Shared Pool */
        rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                field_arr_MAX_SHRD_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                &info->max_shared[rsrc_type]);
        DNXC_IF_ERR_EXIT(rv);

        /* Set Headroom */
        rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                MAX_HDRM_SIZE_THf, 
                entry_offset,
                &info->max_headroom[rsrc_type]);
        DNXC_IF_ERR_EXIT(rv);
    }

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_vsq_pg_rjct_set(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  uint32                     vsq_rt_cls_ndx,
    DNX_SAND_IN  uint32                     drop_precedence_ndx,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_PG_INFO    *info,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_PG_INFO    *exact_info
  )
{
    uint32 rv = SOC_E_NONE;
    uint32 entry_offset = vsq_rt_cls_ndx;
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = DNX_TMC_INGRESS_THRESHOLD_INVALID;
    const soc_mem_t mem_arr_CGM_VSQ_RJCT_PRMS[DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        {CGM_VSQF_WORDS_RJCT_PRMSm, CGM_VSQF_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQF_SRAM_PDS_RJCT_PRMSm};
    const soc_field_t field_arr_MAX_GRNTD_SIZE_TH_DP[DNX_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_GRNTD_SIZE_TH_DP_0f, MAX_GRNTD_SIZE_TH_DP_1f, MAX_GRNTD_SIZE_TH_DP_2f, MAX_GRNTD_SIZE_TH_DP_3f};
    const soc_field_t field_arr_SHRD_SIZE_FADT_MAX_TH_DP[DNX_TMC_NOF_DROP_PRECEDENCE] = 
        {SHRD_SIZE_FADT_MAX_TH_DP_0f, SHRD_SIZE_FADT_MAX_TH_DP_1f, SHRD_SIZE_FADT_MAX_TH_DP_2f, SHRD_SIZE_FADT_MAX_TH_DP_3f};
    const soc_field_t field_arr_SHRD_SIZE_FADT_MIN_TH_DP[DNX_TMC_NOF_DROP_PRECEDENCE] = 
        {SHRD_SIZE_FADT_MIN_TH_DP_0f, SHRD_SIZE_FADT_MIN_TH_DP_1f, SHRD_SIZE_FADT_MIN_TH_DP_2f, SHRD_SIZE_FADT_MIN_TH_DP_3f};
    const soc_field_t field_arr_SHRD_SIZE_FADT_ADJUST_FACTOR_DP[DNX_TMC_NOF_DROP_PRECEDENCE] = 
        {SHRD_SIZE_FADT_ADJUST_FACTOR_DP_0f, SHRD_SIZE_FADT_ADJUST_FACTOR_DP_1f, SHRD_SIZE_FADT_ADJUST_FACTOR_DP_2f, SHRD_SIZE_FADT_ADJUST_FACTOR_DP_3f};
    DNX_TMC_ITM_VSQ_FADT_FIELDS_INFO pg_shared_fadt_fields = {0};
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);
    DNXC_NULL_CHECK(exact_info);

    if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (drop_precedence_ndx >= DNX_TMC_NOF_DROP_PRECEDENCE) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    pg_shared_fadt_fields.max_field = field_arr_SHRD_SIZE_FADT_MAX_TH_DP[drop_precedence_ndx];
    pg_shared_fadt_fields.min_field = field_arr_SHRD_SIZE_FADT_MIN_TH_DP[drop_precedence_ndx];
    pg_shared_fadt_fields.alpha_field = field_arr_SHRD_SIZE_FADT_ADJUST_FACTOR_DP[drop_precedence_ndx];

    for (rsrc_type = 0; rsrc_type < DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        /* Set Guaranteed */
        rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                field_arr_MAX_GRNTD_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                info->max_guaranteed[rsrc_type], 
                &exact_info->max_guaranteed[rsrc_type]);
        DNXC_IF_ERR_EXIT(rv);

        /* Set Shared Pool - for PG-VSQ it's in FADT form */
        rv = jer2_qax_itm_vsq_rjct_fadt_set(unit,
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                pg_shared_fadt_fields,
                entry_offset,
                info->max_shared[rsrc_type],
                &exact_info->max_shared[rsrc_type]);
        DNXC_IF_ERR_EXIT(rv);

        /* Set Headroom */
        rv = jer2_qax_itm_vsq_pg_headroom_rjct_set(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                entry_offset,
                info->max_headroom[rsrc_type], 
                &exact_info->max_headroom[rsrc_type]);
        DNXC_IF_ERR_EXIT(rv);
    }

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_vsq_pg_rjct_get(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  uint32                     vsq_rt_cls_ndx,
    DNX_SAND_IN  uint32                     drop_precedence_ndx,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_PG_INFO    *info
  )
{
    int rv = SOC_E_NONE;
    uint32 entry_offset = vsq_rt_cls_ndx;
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = DNX_TMC_INGRESS_THRESHOLD_INVALID;
    const soc_mem_t mem_arr_CGM_VSQ_RJCT_PRMS[DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES] = 
        {CGM_VSQF_WORDS_RJCT_PRMSm, CGM_VSQF_SRAM_BUFFERS_RJCT_PRMSm, CGM_VSQF_SRAM_PDS_RJCT_PRMSm};
    const soc_field_t field_arr_MAX_GRNTD_SIZE_TH_DP[DNX_TMC_NOF_DROP_PRECEDENCE] = 
        {MAX_GRNTD_SIZE_TH_DP_0f, MAX_GRNTD_SIZE_TH_DP_1f, MAX_GRNTD_SIZE_TH_DP_2f, MAX_GRNTD_SIZE_TH_DP_3f};
    const soc_field_t field_arr_SHRD_SIZE_FADT_MAX_TH_DP[DNX_TMC_NOF_DROP_PRECEDENCE] = 
        {SHRD_SIZE_FADT_MAX_TH_DP_0f, SHRD_SIZE_FADT_MAX_TH_DP_1f, SHRD_SIZE_FADT_MAX_TH_DP_2f, SHRD_SIZE_FADT_MAX_TH_DP_3f};
    const soc_field_t field_arr_SHRD_SIZE_FADT_MIN_TH_DP[DNX_TMC_NOF_DROP_PRECEDENCE] = 
        {SHRD_SIZE_FADT_MIN_TH_DP_0f, SHRD_SIZE_FADT_MIN_TH_DP_1f, SHRD_SIZE_FADT_MIN_TH_DP_2f, SHRD_SIZE_FADT_MIN_TH_DP_3f};
    const soc_field_t field_arr_SHRD_SIZE_FADT_ADJUST_FACTOR_DP[DNX_TMC_NOF_DROP_PRECEDENCE] = 
        {SHRD_SIZE_FADT_ADJUST_FACTOR_DP_0f, SHRD_SIZE_FADT_ADJUST_FACTOR_DP_1f, SHRD_SIZE_FADT_ADJUST_FACTOR_DP_2f, SHRD_SIZE_FADT_ADJUST_FACTOR_DP_3f};
    DNX_TMC_ITM_VSQ_FADT_FIELDS_INFO pg_shared_fadt_fields = {0};
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);

    if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (drop_precedence_ndx >= DNX_TMC_NOF_DROP_PRECEDENCE) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    pg_shared_fadt_fields.max_field = field_arr_SHRD_SIZE_FADT_MAX_TH_DP[drop_precedence_ndx];
    pg_shared_fadt_fields.min_field = field_arr_SHRD_SIZE_FADT_MIN_TH_DP[drop_precedence_ndx];
    pg_shared_fadt_fields.alpha_field = field_arr_SHRD_SIZE_FADT_ADJUST_FACTOR_DP[drop_precedence_ndx];

    for (rsrc_type = 0; rsrc_type < DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
        /* Set Guaranteed */
        rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                field_arr_MAX_GRNTD_SIZE_TH_DP[drop_precedence_ndx], 
                entry_offset,
                &info->max_guaranteed[rsrc_type]);
        DNXC_IF_ERR_EXIT(rv);

        /* Set Shared Pool - for PG-VSQ it's in FADT form */
        rv = jer2_qax_itm_vsq_rjct_fadt_get(unit,
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                pg_shared_fadt_fields,
                entry_offset,
                &info->max_shared[rsrc_type]);
        DNXC_IF_ERR_EXIT(rv);

        /* Set Headroom */
        rv = jer2_qax_itm_vsq_pg_headroom_rjct_get(unit, 
                rsrc_type,
                mem_arr_CGM_VSQ_RJCT_PRMS[rsrc_type], 
                entry_offset,
                &info->max_headroom[rsrc_type]);
        DNXC_IF_ERR_EXIT(rv);
    }

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
static int
  jer2_qax_itm_vsq_rjct_man_exp_set(
    int                                 unit,
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
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
    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);

    rjct_man_exp_info.mem_id = mem;

    rv = soc_mem_read(unit, rjct_man_exp_info.mem_id, CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    DNXC_IF_ERR_EXIT(rv);

    rjct_man_exp_info.field_id = field;
    rjct_man_exp_info.mantissa_bits = CGM_ITM_VSQ_MANTISSA_BITS;
    rjct_man_exp_info.exp_bits = soc_mem_field_length(unit, rjct_man_exp_info.mem_id, rjct_man_exp_info.field_id) - rjct_man_exp_info.mantissa_bits;
    rjct_man_exp_info.factor = (rsrc_type == DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES) ? (CGM_ITM_VSQ_WORDS_RESOLUTION) : (1);

    rv = _jer2_qax_itm_mantissa_exp_field_set(unit, &rjct_man_exp_info, 1, data, threshold, result_threshold);
    DNXC_IF_ERR_EXIT(rv);

    rv = soc_mem_write(unit, rjct_man_exp_info.mem_id, CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    DNXC_IF_ERR_EXIT(rv);

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
static int
  jer2_qax_itm_vsq_rjct_man_exp_get(
    int                                 unit,
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
    soc_mem_t                           mem,
    soc_field_t                         field,
    uint32                              entry_offset,
    uint32                              *result_threshold
  )
{
    int rv = SOC_E_NONE;
    soc_reg_above_64_val_t data;
    itm_mantissa_exp_threshold_info rjct_man_exp_info = {0};
    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);

    rjct_man_exp_info.mem_id = mem;

    rv = soc_mem_read(unit, rjct_man_exp_info.mem_id, CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    DNXC_IF_ERR_EXIT(rv);

    rjct_man_exp_info.field_id = field;
    rjct_man_exp_info.mantissa_bits = CGM_ITM_VSQ_MANTISSA_BITS;
    rjct_man_exp_info.exp_bits = soc_mem_field_length(unit, rjct_man_exp_info.mem_id, rjct_man_exp_info.field_id) - rjct_man_exp_info.mantissa_bits;
    rjct_man_exp_info.factor = (rsrc_type == DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES) ? (CGM_ITM_VSQ_WORDS_RESOLUTION) : (1);

    _jer2_qax_itm_mantissa_exp_field_get(unit, &rjct_man_exp_info, data, result_threshold);

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
static int
  jer2_qax_itm_vsq_rjct_fadt_set(
    int                              unit,
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type,
    soc_mem_t                        mem,
    DNX_TMC_ITM_VSQ_FADT_FIELDS_INFO fadt_fields,
    uint32                           entry_offset,
    DNX_TMC_ITM_VSQ_FADT_INFO        fadt_info,
    DNX_TMC_ITM_VSQ_FADT_INFO        *exact_fadt_info
  )
{
    int rv = SOC_E_NONE;
    soc_reg_above_64_val_t data;
    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);

    if ((fadt_info.alpha > ITM_FADT_MAX_ALPHA) || (fadt_info.alpha < ITM_FADT_MIN_ALPHA)) {
        LOG_ERROR(BSL_LS_SOC_COSQ, (BSL_META_U(unit, "FADT's alpha must be between -7 and 7\n")));
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    /* Set FADT max threshold */
    rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
            rsrc_type,
            mem, 
            fadt_fields.max_field,
            entry_offset,
            fadt_info.max_threshold,
            &exact_fadt_info->max_threshold);
    DNXC_IF_ERR_EXIT(rv);

    /* Set FADT min threshold */
    rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
            rsrc_type,
            mem, 
            fadt_fields.min_field,
            entry_offset,
            fadt_info.min_threshold,
            &exact_fadt_info->min_threshold);
    DNXC_IF_ERR_EXIT(rv);

    /* Set FADT alpha */
    rv = soc_mem_read(unit, mem, CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    DNXC_IF_ERR_EXIT(rv);

    soc_mem_field32_set(unit, mem, data, fadt_fields.alpha_field, _jer2_qax_itm_alpha_to_field(unit, fadt_info.alpha));

    rv = soc_mem_write(unit, mem, CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    DNXC_IF_ERR_EXIT(rv);

    exact_fadt_info->alpha = fadt_info.alpha;

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
static int
  jer2_qax_itm_vsq_rjct_fadt_get(
    int                              unit,
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type,
    soc_mem_t                        mem,
    DNX_TMC_ITM_VSQ_FADT_FIELDS_INFO fadt_fields,
    uint32                           entry_offset,
    DNX_TMC_ITM_VSQ_FADT_INFO        *fadt_info
  )
{
    int rv = SOC_E_NONE;
    soc_reg_above_64_val_t data;
    uint32 alpha = 0;
    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);

    /* Set FADT max threshold */
    rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
            rsrc_type,
            mem, 
            fadt_fields.max_field,
            entry_offset,
            &fadt_info->max_threshold);
    DNXC_IF_ERR_EXIT(rv);

    /* Set FADT min threshold */
    rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
            rsrc_type,
            mem, 
            fadt_fields.min_field,
            entry_offset,
            &fadt_info->min_threshold);
    DNXC_IF_ERR_EXIT(rv);

    /* Set FADT alpha */
    rv = soc_mem_read(unit, mem, CGM_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
    DNXC_IF_ERR_EXIT(rv);

    alpha = soc_mem_field32_get(unit, mem, data, fadt_fields.alpha_field);    
    fadt_info->alpha = _jer2_qax_itm_field_to_alpha(unit, alpha);

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
static int
  jer2_qax_itm_vsq_pg_headroom_rjct_set(
    int                                 unit,
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
    soc_mem_t                           mem,
    uint32                              entry_offset,
    DNX_TMC_ITM_VSQ_PG_HDRM_INFO        headroom_info,
    DNX_TMC_ITM_VSQ_PG_HDRM_INFO        *exact_headroom_info
  )
{
    int rv = SOC_E_NONE;
    DNXC_INIT_FUNC_DEFS;

    /* Set max headroom threshold */
    rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
            rsrc_type,
            mem, 
            MAX_HDRM_SIZE_THf,
            entry_offset,
            headroom_info.max_headroom,
            &exact_headroom_info->max_headroom);
    DNXC_IF_ERR_EXIT(rv);

    if (rsrc_type != DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES) {
        /* Set max headroom nominal threshold */
        rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                mem, 
                MAX_HDRM_NOMINAL_SIZE_THf,
                entry_offset,
                headroom_info.max_headroom_nominal,
                &exact_headroom_info->max_headroom_nominal);
        DNXC_IF_ERR_EXIT(rv);

        /* Set max headroom extension threshold */
        rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                mem, 
                MAX_HDRM_EXT_SIZE_THf,
                entry_offset,
                headroom_info.max_headroom_extension,
                &exact_headroom_info->max_headroom_extension);
        DNXC_IF_ERR_EXIT(rv);
    }

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
static int
  jer2_qax_itm_vsq_pg_headroom_rjct_get(
    int                                 unit,
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E    rsrc_type,
    soc_mem_t                           mem,
    uint32                              entry_offset,
    DNX_TMC_ITM_VSQ_PG_HDRM_INFO        *headroom_info
  )
{
    int rv = SOC_E_NONE;
    DNXC_INIT_FUNC_DEFS;

    /* Set max headroom threshold */
    rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
            rsrc_type,
            mem, 
            MAX_HDRM_SIZE_THf,
            entry_offset,
            &headroom_info->max_headroom);
    DNXC_IF_ERR_EXIT(rv);

    if (rsrc_type != DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES) {
        /* Set max headroom nominal threshold */
        rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                mem, 
                MAX_HDRM_NOMINAL_SIZE_THf,
                entry_offset,
                &headroom_info->max_headroom_nominal);
        DNXC_IF_ERR_EXIT(rv);

        /* Set max headroom extension threshold */
        rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                mem, 
                MAX_HDRM_EXT_SIZE_THf,
                entry_offset,
                &headroom_info->max_headroom_extension);
        DNXC_IF_ERR_EXIT(rv);
    }

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_vsq_qt_rt_cls_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core_id,
    DNX_SAND_IN  uint8               is_ocb_only,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_NDX    vsq_in_group_ndx,
    DNX_SAND_IN  uint32                 vsq_rt_cls
  )
{
    int rv = SOC_E_NONE;
    uint32 data = 0;
    soc_mem_t mem = 0;
    uint32 entry_offset = 0;
    DNXC_INIT_FUNC_DEFS;

    rv = jer2_qax_itm_vsq_qt_rt_cls_verify(unit, vsq_group_ndx, vsq_in_group_ndx);
    DNXC_IF_ERR_EXIT(rv);

    if (vsq_group_ndx == DNX_TMC_ITM_VSQ_GROUP_CTGRY) {
        if (vsq_rt_cls > DNX_TMC_ITM_VSQ_A_QT_RT_CLS_MAX(unit)) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
    } else {
        if (vsq_rt_cls > DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }

    switch(vsq_group_ndx) { 
        case DNX_TMC_ITM_VSQ_GROUP_CTGRY:
            mem = CGM_VSQA_PRMSm;
            break;
        case DNX_TMC_ITM_VSQ_GROUP_CTGRY_TRAFFIC_CLS:
            mem = CGM_VSQB_PRMSm;
            break;
        case DNX_TMC_ITM_VSQ_GROUP_CTGRY_2_3_CNCTN_CLS:
            mem = CGM_VSQC_PRMSm;
            break;
        case DNX_TMC_ITM_VSQ_GROUP_STTSTCS_TAG:
            mem = CGM_VSQD_PRMSm;
            break;
        case DNX_TMC_ITM_VSQ_GROUP_SRC_PORT:
            mem = CGM_VSQE_PRMSm;
            break;
        case DNX_TMC_ITM_VSQ_GROUP_PG:
            mem = CGM_VSQF_PRMSm;
            break;
        default:
            break;
    }

    entry_offset = vsq_in_group_ndx;

    rv = soc_mem_read(unit, mem, CGM_BLOCK(unit, core_id), entry_offset, &data);
    DNXC_IF_ERR_EXIT(rv);

    soc_mem_field32_set(unit, mem, &data, RATE_CLASSf, vsq_rt_cls);

    rv = soc_mem_write(unit, mem, CGM_BLOCK(unit, core_id), entry_offset, &data);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN; 
}

/*
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
static int
  jer2_qax_itm_vsq_qt_rt_cls_verify(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_NDX    vsq_in_group_ndx
  )
{
    uint32 rv = SOC_E_NONE;
    uint32 vsq_in_group_size;
    DNXC_INIT_FUNC_DEFS;

    if (vsq_group_ndx >= DNX_TMC_NOF_VSQ_GROUPS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    rv = jer2_arad_itm_vsq_in_group_size_get(unit, vsq_group_ndx, &vsq_in_group_size);
    DNXC_IF_ERR_EXIT(rv);

    if (vsq_in_group_ndx >= vsq_in_group_size) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

exit:
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_vsq_qt_rt_cls_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core_id,
    DNX_SAND_IN  uint8               is_ocb_only,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP  vsq_group_ndx,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_NDX    vsq_in_group_ndx,
    DNX_SAND_OUT uint32                 *vsq_rt_cls
  )
{
    int rv = SOC_E_NONE;
    uint32 data = 0;
    soc_mem_t mem = 0;
    uint32 entry_offset = 0;
    DNXC_INIT_FUNC_DEFS;

    rv = jer2_qax_itm_vsq_qt_rt_cls_verify(unit, vsq_group_ndx, vsq_in_group_ndx);
    DNXC_IF_ERR_EXIT(rv);

    switch(vsq_group_ndx) { 
        case DNX_TMC_ITM_VSQ_GROUP_CTGRY:
            mem = CGM_VSQA_PRMSm;
            break;
        case DNX_TMC_ITM_VSQ_GROUP_CTGRY_TRAFFIC_CLS:
            mem = CGM_VSQB_PRMSm;
            break;
        case DNX_TMC_ITM_VSQ_GROUP_CTGRY_2_3_CNCTN_CLS:
            mem = CGM_VSQC_PRMSm;
            break;
        case DNX_TMC_ITM_VSQ_GROUP_STTSTCS_TAG:
            mem = CGM_VSQD_PRMSm;
            break;
        case DNX_TMC_ITM_VSQ_GROUP_SRC_PORT:
            mem = CGM_VSQE_PRMSm;
            break;
        case DNX_TMC_ITM_VSQ_GROUP_PG:
            mem = CGM_VSQF_PRMSm;
            break;
        default:
            break;
    }

    entry_offset = vsq_in_group_ndx;

    rv = soc_mem_read(unit, mem, CGM_BLOCK(unit, core_id), entry_offset, &data);
    DNXC_IF_ERR_EXIT(rv);

    *vsq_rt_cls = soc_mem_field32_get(unit, mem, &data, RATE_CLASSf);

exit:
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_vsq_src_port_get(
    DNX_SAND_IN int    unit,
    DNX_SAND_IN int    core_id,
    DNX_SAND_IN int    src_port_vsq_index,
    DNX_SAND_OUT uint32 *src_pp_port,
    DNX_SAND_OUT uint8  *enable
  )
{
    int rv;
    uint32 tmp_src_pp_port, data;
    uint8 found = 0;
    int tmp_src_port_vsq_index = 0;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(enable);
    DNXC_NULL_CHECK(src_pp_port);

    if (core_id < 0 || core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (src_port_vsq_index < 0 || src_port_vsq_index >= DNX_TMC_ITM_VSQ_GROUPE_SZE(unit)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    /* Search all PP-ports for src_port_vsq_index (NIF_PORTf) */
    for (tmp_src_pp_port = 0; tmp_src_pp_port < SOC_MAX_NUM_PORTS; tmp_src_pp_port++) {
        rv = READ_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), tmp_src_pp_port, &data);
        DNXC_IF_ERR_EXIT(rv);

        tmp_src_port_vsq_index = soc_mem_field32_get(unit, CGM_IPPPMm, &data, NIF_PORTf);
        if (tmp_src_port_vsq_index == src_port_vsq_index) {
            found = 1;
            break;
        }
    }

    if (found) {
        *src_pp_port = tmp_src_pp_port;
        *enable = 1;
    } else {
        *src_pp_port = -1;
        *enable = 0;
    }

exit: 
    DNXC_FUNC_RETURN;
}

/*
 * \brief
 *  Get PG-VSQ mapping corresponds to a PG-BASE-VSQ index.
 *
 */
/*
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
  jer2_qax_itm_vsq_pg_mapping_get(
    DNX_SAND_IN int     unit,
    DNX_SAND_IN int     core_id,
    DNX_SAND_IN uint32  pg_vsq_base,
    DNX_SAND_IN int     cosq,
    DNX_SAND_OUT uint32 *src_pp_port,
    DNX_SAND_OUT uint8  *enable
  )
{
    int rv;
    uint32 pg_vsq_id;
    uint32 tmp_pg_vsq_base = -1, tmp_src_pp_port = 0, data = 0;
    int found = 0;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(enable);
    DNXC_NULL_CHECK(src_pp_port);

    if (core_id < 0 || core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (cosq < 0 || cosq >= DNX_TMC_NOF_TRAFFIC_CLASSES) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    pg_vsq_id = pg_vsq_base + cosq;
    if (pg_vsq_id >= DNX_TMC_ITM_VSQ_GROUPF_SZE(unit)) {
        LOG_ERROR(BSL_LS_SOC_COSQ, (BSL_META_U(unit, "Invalid PG VSQ ID %d\n"), pg_vsq_id));
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    /* Search all PP-ports for pg_vsq_base (PG_BASEf) */
    for (tmp_src_pp_port = 0; tmp_src_pp_port < SOC_MAX_NUM_PORTS; tmp_src_pp_port++) {
        rv = READ_CGM_IPPPMm(unit, CGM_BLOCK(unit, core_id), tmp_src_pp_port, &data);
        DNXC_IF_ERR_EXIT(rv);

        tmp_pg_vsq_base = soc_mem_field32_get(unit, CGM_IPPPMm, &data, PG_BASEf);
        if (tmp_pg_vsq_base == pg_vsq_base) {
            found = 1;
            break;
        }
    }

    if (found) {
        *src_pp_port = tmp_src_pp_port;
        *enable = 1;
    } else {
        *src_pp_port = -1;
        *enable = 0;
    }

exit: 
    DNXC_FUNC_RETURN;
}

/*
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
static int 
  jer2_qax_itm_category_rngs_verify( 
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  DNX_TMC_ITM_CATEGORY_RNGS *info 
  ) 
{ 
    DNXC_INIT_FUNC_DEFS; 
    DNXC_NULL_CHECK(info);    

    if ((info->vsq_ctgry0_end > info->vsq_ctgry1_end) || (info->vsq_ctgry1_end > info->vsq_ctgry2_end) || (info->vsq_ctgry0_end > info->vsq_ctgry2_end)) { 
        DNXC_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

    if (info->vsq_ctgry0_end > SOC_DNX_DEFS_GET(unit, nof_queues)-1) { 
        DNXC_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

    if (info->vsq_ctgry1_end > (SOC_DNX_DEFS_GET(unit, nof_queues)-1)) { 
        DNXC_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

    if (info->vsq_ctgry2_end > (SOC_DNX_DEFS_GET(unit, nof_queues)-1)) { 
        DNXC_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

exit: 
    DNXC_FUNC_RETURN; 
} 
 
/*
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
  jer2_qax_itm_category_rngs_set( 
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 core_id, 
    DNX_SAND_IN  DNX_TMC_ITM_CATEGORY_RNGS *info 
  ) 
{ 
    int rv; 
    uint64 reg_val; 
    DNXC_INIT_FUNC_DEFS; 

    DNXC_NULL_CHECK(info); 

    COMPILER_64_ZERO(reg_val);

    if (((core_id < 0) || (core_id > SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores)) && core_id != SOC_CORE_ALL) { 
        DNXC_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

    rv = jer2_qax_itm_category_rngs_verify(unit, info); 
    DNXC_IF_ERR_EXIT(rv); 

    rv = READ_CGM_VSQ_CATEGORY_RANGESr(unit, &reg_val); 
    DNXC_IF_ERR_EXIT(rv); 

    soc_reg64_field32_set(unit, CGM_VSQ_CATEGORY_RANGESr, &reg_val, VSQ_CATEGORY_RANGE_0f, info->vsq_ctgry0_end); 
    soc_reg64_field32_set(unit, CGM_VSQ_CATEGORY_RANGESr, &reg_val, VSQ_CATEGORY_RANGE_1f, info->vsq_ctgry1_end); 
    soc_reg64_field32_set(unit, CGM_VSQ_CATEGORY_RANGESr, &reg_val, VSQ_CATEGORY_RANGE_2f, info->vsq_ctgry2_end); 

    rv = WRITE_CGM_VSQ_CATEGORY_RANGESr(unit, reg_val); 
    DNXC_IF_ERR_EXIT(rv); 

exit: 
    DNXC_FUNC_RETURN; 
} 
 
/*
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
  jer2_qax_itm_category_rngs_get( 
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 core_id, 
    DNX_SAND_OUT DNX_TMC_ITM_CATEGORY_RNGS *info 
  ) 
{ 
    int rv; 
    uint64 reg_val; 
    DNXC_INIT_FUNC_DEFS; 

    DNXC_NULL_CHECK(info); 

    COMPILER_64_ZERO(reg_val);

    if (((core_id < 0) || (core_id > SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores)) && core_id != SOC_CORE_ALL) { 
        DNXC_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

    rv = READ_CGM_VSQ_CATEGORY_RANGESr(unit, &reg_val); 
    DNXC_IF_ERR_EXIT(rv); 

    info->vsq_ctgry0_end = soc_reg64_field32_get(unit, CGM_VSQ_CATEGORY_RANGESr, reg_val, VSQ_CATEGORY_RANGE_0f); 
    info->vsq_ctgry1_end = soc_reg64_field32_get(unit, CGM_VSQ_CATEGORY_RANGESr, reg_val, VSQ_CATEGORY_RANGE_1f); 
    info->vsq_ctgry2_end = soc_reg64_field32_get(unit, CGM_VSQ_CATEGORY_RANGESr, reg_val, VSQ_CATEGORY_RANGE_2f); 

exit: 
    DNXC_FUNC_RETURN; 
} 

/*
 * \brief
 *  Verify that the given global reject thresholds are in valid ranges.
 *
 * \par DIRECT INPUT:
 *  \param [in] unit -
 *      The unit id.
 *  \param [in] info -
 *      Reject thresholds to validate.
 */
static
int
  jer2_qax_itm_glob_rcs_drop_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  DNX_TMC_ITM_GLOB_RCS_DROP_TH *info
  )
{
    int color = 0, sram_type = 0;
    uint32 max_threshold = 0;
    DNXC_INIT_FUNC_DEFS; 

    DNXC_NULL_CHECK(info); 

    /* Validate Global Free SRAM resources reject thresholds. */
    /* Resources can be SRAM-PDBs or SRAM-Buffers. */
    for (sram_type = 0; sram_type < DNX_TMC_INGRESS_THRESHOLD_NOF_SRAM_TYPES; ++sram_type) {
        max_threshold = (sram_type == 0) ? (CGM_ITM_VSQ_SRAM_PDBS_SIZE_MAX) : (CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX);
        for (color = 0; color < DNX_TMC_NOF_DROP_PRECEDENCE; ++color) {
            if (info->global_free_sram[sram_type][color].set > max_threshold) {
                DNXC_IF_ERR_EXIT(SOC_E_PARAM);
            }
            if (info->global_free_sram[sram_type][color].clear > max_threshold) {
                DNXC_IF_ERR_EXIT(SOC_E_PARAM);
            }

            /* Validate SRAM-Only thresholds */
            if (info->global_free_sram_only[sram_type][color].set > max_threshold) {
                DNXC_IF_ERR_EXIT(SOC_E_PARAM);
            }
            if (info->global_free_sram_only[sram_type][color].clear > max_threshold) {
                DNXC_IF_ERR_EXIT(SOC_E_PARAM);
            }
        }
    }

    /* Validate Global Free DRAM-BDBs resources reject thresholds */
    for (color = 0; color < DNX_TMC_NOF_DROP_PRECEDENCE; ++color) {
        if (info->global_free_dram[color].set > CGM_ITM_VSQ_DRAM_BDBS_SIZE_MAX) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
        if (info->global_free_dram[color].clear > CGM_ITM_VSQ_DRAM_BDBS_SIZE_MAX) {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
    }

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_glob_rcs_drop_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core_id,
    DNX_SAND_IN  DNX_TMC_ITM_GLOB_RCS_DROP_TH *info,
    DNX_SAND_OUT DNX_TMC_ITM_GLOB_RCS_DROP_TH *exact_info
  )
{
    int rv = SOC_E_NONE; 
    int color = 0, sram_type = 0;
    uint32 entry_offset = 0;

    DNX_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = DNX_TMC_INGRESS_THRESHOLD_INVALID;

    soc_field_t field_arr_GLBL_FR_SRAM_SET_TH[DNX_TMC_INGRESS_THRESHOLD_NOF_SRAM_TYPES] = 
        {SRAM_PDBS_SET_THf, SRAM_BUFFERS_SET_THf};
    soc_field_t field_arr_GLBL_FR_SRAM_CLR_TH[DNX_TMC_INGRESS_THRESHOLD_NOF_SRAM_TYPES] = 
        {SRAM_PDBS_CLR_THf, SRAM_BUFFERS_CLR_THf};

    DNXC_INIT_FUNC_DEFS; 

    DNXC_NULL_CHECK(info); 
    DNXC_NULL_CHECK(exact_info); 

    if (((core_id < 0) || (core_id > SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores)) && core_id != SOC_CORE_ALL) { 
        DNXC_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

    /* Validate inputs are in range */
    rv = jer2_qax_itm_glob_rcs_drop_verify(unit, info);
    DNXC_IF_ERR_EXIT(rv);

    /* Set Global free SRAM resources reject thresholds */
    for (sram_type = 0; sram_type < DNX_TMC_INGRESS_THRESHOLD_NOF_SRAM_TYPES; ++sram_type) {
        for (color = 0; color < DNX_TMC_NOF_DROP_PRECEDENCE; ++color) {
            /* Set thresholds for regular VOQs */
            entry_offset = color;

            /* set SET threshold */
            rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_SET_TH[sram_type], 
                    entry_offset,
                    info->global_free_sram[sram_type][color].set, 
                    &exact_info->global_free_sram[sram_type][color].set);
            DNXC_IF_ERR_EXIT(rv);

            /* set CLEAR threshold */
            rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_CLR_TH[sram_type], 
                    entry_offset,
                    info->global_free_sram[sram_type][color].clear, 
                    &exact_info->global_free_sram[sram_type][color].clear);
            DNXC_IF_ERR_EXIT(rv);

            /* Set thresholds for SRAM-only VOQs */
            entry_offset = color + DNX_TMC_NOF_DROP_PRECEDENCE;

            /* set SET threshold */
            rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_SET_TH[sram_type], 
                    entry_offset,
                    info->global_free_sram_only[sram_type][color].set, 
                    &exact_info->global_free_sram_only[sram_type][color].set);
            DNXC_IF_ERR_EXIT(rv);

            /* set CLEAR threshold */
            rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_CLR_TH[sram_type], 
                    entry_offset,
                    info->global_free_sram_only[sram_type][color].clear, 
                    &exact_info->global_free_sram_only[sram_type][color].clear);
            DNXC_IF_ERR_EXIT(rv);
        }
    }

    /* Set Global free DRAM resources reject thresholds */
    for (color = 0; color < DNX_TMC_NOF_DROP_PRECEDENCE; ++color) {
        entry_offset = color;

        /* set SET threshold */
        rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                CGM_GLBL_FR_DRAM_RJCT_THm, 
                DRAM_BDBS_SET_THf, 
                entry_offset,
                info->global_free_dram[color].set, 
                &exact_info->global_free_dram[color].set);
        DNXC_IF_ERR_EXIT(rv);

        /* set CLEAR threshold */
        rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
                rsrc_type,
                CGM_GLBL_FR_DRAM_RJCT_THm, 
                DRAM_BDBS_CLR_THf, 
                entry_offset,
                info->global_free_dram[color].clear, 
                &exact_info->global_free_dram[color].clear);
        DNXC_IF_ERR_EXIT(rv);
    }

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_glob_rcs_drop_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core_id,
    DNX_SAND_OUT  DNX_TMC_ITM_GLOB_RCS_DROP_TH *info
  )
{
    int rv = SOC_E_NONE; 
    int color = 0, sram_type = 0;
    uint32 entry_offset = 0;

    DNX_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = DNX_TMC_INGRESS_THRESHOLD_INVALID;

    soc_field_t field_arr_GLBL_FR_SRAM_SET_TH[DNX_TMC_INGRESS_THRESHOLD_NOF_SRAM_TYPES] = 
        {SRAM_PDBS_SET_THf, SRAM_BUFFERS_SET_THf};
    soc_field_t field_arr_GLBL_FR_SRAM_CLR_TH[DNX_TMC_INGRESS_THRESHOLD_NOF_SRAM_TYPES] = 
        {SRAM_PDBS_CLR_THf, SRAM_BUFFERS_CLR_THf};

    DNXC_INIT_FUNC_DEFS; 

    DNXC_NULL_CHECK(info); 

    if (((core_id < 0) || (core_id > SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores)) && core_id != SOC_CORE_ALL) { 
        DNXC_IF_ERR_EXIT(SOC_E_PARAM); 
    } 

    /* Set Global free SRAM resources reject thresholds */
    for (sram_type = 0; sram_type < DNX_TMC_INGRESS_THRESHOLD_NOF_SRAM_TYPES; ++sram_type) {
        for (color = 0; color < DNX_TMC_NOF_DROP_PRECEDENCE; ++color) {
            /* Set thresholds for regular VOQs */
            entry_offset = color;

            /* set SET threshold */
            rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_SET_TH[sram_type], 
                    entry_offset,
                    &info->global_free_sram[sram_type][color].set);
            DNXC_IF_ERR_EXIT(rv);

            /* set CLEAR threshold */
            rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_CLR_TH[sram_type], 
                    entry_offset,
                    &info->global_free_sram[sram_type][color].clear);
            DNXC_IF_ERR_EXIT(rv);

            /* Set thresholds for SRAM-only VOQs */
            entry_offset = color + DNX_TMC_NOF_DROP_PRECEDENCE;

            /* set SET threshold */
            rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_SET_TH[sram_type], 
                    entry_offset,
                    &info->global_free_sram_only[sram_type][color].set);
            DNXC_IF_ERR_EXIT(rv);

            /* set CLEAR threshold */
            rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
                    rsrc_type,
                    CGM_GLBL_FR_SRAM_RJCT_THm, 
                    field_arr_GLBL_FR_SRAM_CLR_TH[sram_type], 
                    entry_offset,
                    &info->global_free_sram_only[sram_type][color].clear);
            DNXC_IF_ERR_EXIT(rv);
        }
    }

    /* Set Global free DRAM resources reject thresholds */
    for (color = 0; color < DNX_TMC_NOF_DROP_PRECEDENCE; ++color) {
        entry_offset = color;

        /* set SET threshold */
        rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                CGM_GLBL_FR_DRAM_RJCT_THm, 
                DRAM_BDBS_SET_THf, 
                entry_offset,
                &info->global_free_dram[color].set);

        /* set CLEAR threshold */
        rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
                rsrc_type,
                CGM_GLBL_FR_DRAM_RJCT_THm, 
                DRAM_BDBS_CLR_THf, 
                entry_offset,
                &info->global_free_dram[color].clear);
        DNXC_IF_ERR_EXIT(rv);
    }

exit: 
    DNXC_FUNC_RETURN; 
}

/*
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
  jer2_qax_itm_vsq_fc_set(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP      vsq_group_ndx,
    DNX_SAND_IN  uint32                     vsq_rt_cls_ndx,
    DNX_SAND_IN  int                        pool_id,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_FC_INFO    *info,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_FC_INFO    *exact_info
  )
{
    int rv = SOC_E_NONE;
    uint32 entry_offset = 0;
    const soc_mem_t mem_arr_CGM_VSQ_FC_PRMS[DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES][DNX_TMC_NOF_VSQ_GROUPS] = 
        { {CGM_VSQA_WORDS_FC_PRMSm, CGM_VSQB_WORDS_FC_PRMSm, CGM_VSQC_WORDS_FC_PRMSm, CGM_VSQD_WORDS_FC_PRMSm, CGM_VSQE_WORDS_FC_PRMSm, CGM_VSQF_WORDS_FC_PRMSm},
          {CGM_VSQA_SRAM_BUFFERS_FC_PRMSm, CGM_VSQB_SRAM_BUFFERS_FC_PRMSm, CGM_VSQC_SRAM_BUFFERS_FC_PRMSm, CGM_VSQD_SRAM_BUFFERS_FC_PRMSm, CGM_VSQE_SRAM_BUFFERS_FC_PRMSm, CGM_VSQF_SRAM_BUFFERS_FC_PRMSm},
          {CGM_VSQA_SRAM_PDS_FC_PRMSm, CGM_VSQB_SRAM_PDS_FC_PRMSm, CGM_VSQC_SRAM_PDS_FC_PRMSm, CGM_VSQD_SRAM_PDS_FC_PRMSm, CGM_VSQE_SRAM_PDS_FC_PRMSm, CGM_VSQF_SRAM_PDS_FC_PRMSm} };
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = DNX_TMC_INGRESS_THRESHOLD_INVALID;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);
    DNXC_NULL_CHECK(exact_info);

    if (vsq_group_ndx < 0 || vsq_group_ndx >= DNX_TMC_NOF_VSQ_GROUPS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (pool_id < 0 || pool_id >= DNX_TMC_ITM_NOF_RSRC_POOLS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (info->size_fc[DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].clear > CGM_ITM_VSQ_WORDS_SIZE_MAX(unit)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (info->size_fc[DNX_TMC_INGRESS_THRESHOLD_TOTAL_BYTES].set > CGM_ITM_VSQ_WORDS_SIZE_MAX(unit)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (info->size_fc[DNX_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].clear > CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (info->size_fc[DNX_TMC_INGRESS_THRESHOLD_SRAM_BUFFERS].set > CGM_ITM_VSQ_SRAM_BUFFERS_SIZE_MAX) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (info->size_fc[DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS].clear > CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (info->size_fc[DNX_TMC_INGRESS_THRESHOLD_SRAM_PDS].set > CGM_ITM_VSQ_SRAM_PDS_SIZE_MAX) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    entry_offset = vsq_rt_cls_ndx;
    if (vsq_group_ndx == DNX_TMC_ITM_VSQ_GROUP_SRC_PORT) {
        /* VSQ-E table is doubled. The first half is for pool 0 and the second half is for pool 1 */
        entry_offset += pool_id * DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit);
    }

    if (vsq_group_ndx == DNX_TMC_ITM_VSQ_GROUP_PG) {
        
    } else {
        for (rsrc_type = 0; rsrc_type < DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
            /* Set SET threshold */
            rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
                    rsrc_type,
                    mem_arr_CGM_VSQ_FC_PRMS[rsrc_type][vsq_group_ndx], 
                    SET_THf, 
                    entry_offset,
                    info->size_fc[rsrc_type].set, 
                    &exact_info->size_fc[rsrc_type].set);
            DNXC_IF_ERR_EXIT(rv);

            /* Set CLEAR threshold */
            rv = jer2_qax_itm_vsq_rjct_man_exp_set(unit, 
                    rsrc_type,
                    mem_arr_CGM_VSQ_FC_PRMS[rsrc_type][vsq_group_ndx], 
                    CLR_THf, 
                    entry_offset,
                    info->size_fc[rsrc_type].clear, 
                    &exact_info->size_fc[rsrc_type].clear);
            DNXC_IF_ERR_EXIT(rv);
        }
    }

exit: 
    DNXC_FUNC_RETURN;
}

/*
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
  jer2_qax_itm_vsq_fc_get(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP      vsq_group_ndx,
    DNX_SAND_IN  uint32                     vsq_rt_cls_ndx,
    DNX_SAND_IN  int                        pool_id,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_FC_INFO    *info
  )
{
    int rv = SOC_E_NONE;
    uint32 entry_offset = 0;
    const soc_mem_t mem_arr_CGM_VSQ_FC_PRMS[DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES][DNX_TMC_NOF_VSQ_GROUPS] = 
        { {CGM_VSQA_WORDS_FC_PRMSm, CGM_VSQB_WORDS_FC_PRMSm, CGM_VSQC_WORDS_FC_PRMSm, CGM_VSQD_WORDS_FC_PRMSm, CGM_VSQE_WORDS_FC_PRMSm, CGM_VSQF_WORDS_FC_PRMSm},
          {CGM_VSQA_SRAM_BUFFERS_FC_PRMSm, CGM_VSQB_SRAM_BUFFERS_FC_PRMSm, CGM_VSQC_SRAM_BUFFERS_FC_PRMSm, CGM_VSQD_SRAM_BUFFERS_FC_PRMSm, CGM_VSQE_SRAM_BUFFERS_FC_PRMSm, CGM_VSQF_SRAM_BUFFERS_FC_PRMSm},
          {CGM_VSQA_SRAM_PDS_FC_PRMSm, CGM_VSQB_SRAM_PDS_FC_PRMSm, CGM_VSQC_SRAM_PDS_FC_PRMSm, CGM_VSQD_SRAM_PDS_FC_PRMSm, CGM_VSQE_SRAM_PDS_FC_PRMSm, CGM_VSQF_SRAM_PDS_FC_PRMSm} };
    DNX_TMC_INGRESS_THRESHOLD_TYPE_E rsrc_type = DNX_TMC_INGRESS_THRESHOLD_INVALID;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);

    if (vsq_group_ndx < 0 || vsq_group_ndx >= DNX_TMC_NOF_VSQ_GROUPS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (pool_id < 0 || pool_id >= DNX_TMC_ITM_NOF_RSRC_POOLS) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (vsq_rt_cls_ndx > DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    entry_offset = vsq_rt_cls_ndx;
    if (vsq_group_ndx == DNX_TMC_ITM_VSQ_GROUP_SRC_PORT) {
        /* VSQ-E table is doubled. The first half is for pool 0 and the second half is for pool 1 */
        entry_offset += pool_id * DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit);
    }

    if (vsq_group_ndx == DNX_TMC_ITM_VSQ_GROUP_PG) {
        
    } else {
        for (rsrc_type = 0; rsrc_type < DNX_TMC_INGRESS_THRESHOLD_NOF_TYPES; ++rsrc_type) {
            /* Set SET threshold */
            rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
                    rsrc_type,
                    mem_arr_CGM_VSQ_FC_PRMS[rsrc_type][vsq_group_ndx], 
                    SET_THf, 
                    entry_offset,
                    &info->size_fc[rsrc_type].set);
            DNXC_IF_ERR_EXIT(rv);

            /* Set CLEAR threshold */
            rv = jer2_qax_itm_vsq_rjct_man_exp_get(unit, 
                    rsrc_type,
                    mem_arr_CGM_VSQ_FC_PRMS[rsrc_type][vsq_group_ndx], 
                    CLR_THf, 
                    entry_offset,
                    &info->size_fc[rsrc_type].clear);
            DNXC_IF_ERR_EXIT(rv);
        }
    }

exit: 
    DNXC_FUNC_RETURN;
}

/*******************/
/* } VSQ Functions */
/*******************/


/*
 * Scheduler (credits) compensation configuration. 
 * 2 delta(compensations) are configured dynamically with a value 
 * Per queue and Per OutlIF Profile 
 */
int jer2_qax_itm_credits_adjust_size_set (
    DNX_SAND_IN int   unit,
    DNX_SAND_IN int   core,
    DNX_SAND_IN int   index, /* entity index, the entity depends on adjust_type */
    DNX_SAND_IN DNX_TMC_ITM_CGM_PKT_SIZE_ADJUST_TYPE adjust_type,
    DNX_SAND_IN int   delta
  ) 
{
    int offset = 0, core_id = 0;
    int delta_internal;
    DNXC_INIT_FUNC_DEFS;
  
    /* verify delta is in range */
    if ((delta > JER2_QAX_ITM_SCHEDULER_DELTA_MAX) || (delta < JER2_QAX_ITM_SCHEDULER_DELTA_MIN)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Delta %d is out of range"), delta));
    }

    /* convert value to 2's complement */
    delta_internal = CONVERT_SIGNED_NUM_TO_TWO_COMPLEMENT_METHOD(delta, JER2_QAX_ITM_SCHEDULER_DELTA_NOF_BITS);

    SOC_DNX_CORES_ITER(core, core_id) {

        if (adjust_type == DNX_TMC_ITM_PKT_SIZE_ADJUST_QUEUE) { /* per queue (credit class) compensation */
            uint32 data;
            /* 
               the key to the table is [credit_class_profile(3bits) concat IPP-profile(3bits)]
               we always use IPP profile 0, thus we need to set 3 lower bits to 0
            */
            data = 0;
            offset = index << 0x3; /* index is the credit class profile */
            soc_mem_field32_set(unit, CGM_SCH_CPMm, &data, HDR_DELTAf, delta_internal); /* this is the only field in the table -- no need to read the data */         
            DNXC_IF_ERR_EXIT(WRITE_CGM_SCH_CPMm(unit, CGM_BLOCK(unit, core_id), offset, &data));

        } else if (adjust_type == DNX_TMC_ITM_PKT_SIZE_ADJUST_APPEND_SIZE_PTR) { /* per OutLif profile (header append pointer) compensation */
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
                    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid append pointer id %d"), index));
            }
            DNXC_IF_ERR_EXIT(READ_CGM_HAPMm(unit, CGM_BLOCK(unit, core_id), index, above64));
            soc_mem_field32_set(unit, CGM_HAPMm, above64, SCH_DELTAf, delta_internal);          
            DNXC_IF_ERR_EXIT(WRITE_CGM_HAPMm(unit, CGM_BLOCK(unit, core_id), index, above64));

        } else {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid adjust_type %d"), adjust_type));
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Scheduler (credits) compensation configuration. 
 * 2 delta(compensations) are configured dynamically with a value 
 * Per queue and Per OutlIF Profile 
 */
int jer2_qax_itm_credits_adjust_size_get (
    DNX_SAND_IN int   unit,
    DNX_SAND_IN int   core,
    DNX_SAND_IN int   index, /* entity index, the entity depends on adjust_type */
    DNX_SAND_IN DNX_TMC_ITM_CGM_PKT_SIZE_ADJUST_TYPE adjust_type,
    DNX_SAND_OUT int   *delta
  ) 
{
    int offset = 0, core_id = 0;
    int delta_internal;

    DNXC_INIT_FUNC_DEFS;
  
    DNXC_NULL_CHECK(delta);

    core_id = ((core == SOC_CORE_ALL) ? 0 : core);

    if (adjust_type == DNX_TMC_ITM_PKT_SIZE_ADJUST_QUEUE) { /* per queue (credit class) compensation */
        uint32 data;
         
        /* 
           the key to the table is [credit_class_profile(3bits) concat IPP-profile(3bits)]
           we always use IPP profile 0, thus we need to set 3 lower bits to 0
        */
        offset = index << 0x3; /* index is the credit class profile */
        DNXC_IF_ERR_EXIT(READ_CGM_SCH_CPMm(unit, CGM_BLOCK(unit, core_id), offset, &data));
        delta_internal = soc_mem_field32_get(unit, CGM_SCH_CPMm, &data, HDR_DELTAf);          
        

    } else if (adjust_type == DNX_TMC_ITM_PKT_SIZE_ADJUST_APPEND_SIZE_PTR) { /* per OutLif profile (header append pointer) compensation */

        soc_reg_above_64_val_t above64;
        /* 
           The key to the table is 8 bits.
           5 upper bits is outlif, provided by user. 
           The 3 lower bits are different properties which we currently do not distinguish between 
        */
        offset = (index << 3); /* 5 msb bits are the key to the table, so configure 8 entries with the same value */
        if (offset >= CGM_HAP_NOF_ENTRIES) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid append pointer id %d"), index));
        }

        DNXC_IF_ERR_EXIT(READ_CGM_HAPMm(unit, CGM_BLOCK(unit, core_id), offset, above64));
        delta_internal = soc_mem_field32_get(unit, CGM_HAPMm, above64, SCH_DELTAf);          

    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid adjust_type %d"), adjust_type));
    }


    /* convert from 2's complement to value */
    *delta = CONVERT_TWO_COMPLEMENT_INTO_SIGNED_NUM(delta_internal, JER2_QAX_ITM_SCHEDULER_DELTA_NOF_BITS);

exit:
    DNXC_FUNC_RETURN;
}


#undef _ERR_MSG_MODULE_NAME

