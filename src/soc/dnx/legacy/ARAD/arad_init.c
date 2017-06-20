/* $Id: jer2_arad_init.c,v 1.309 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#include <shared/bsl.h>

#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnxc/legacy/dnxc_mem.h>
#include <soc/dnxc/legacy/dnxc_mbist.h>
#include <soc/mem.h>

#include <soc/dnx/legacy/TMC/tmc_api_ports.h>
#include <soc/dnx/legacy/dnx_config_defs.h>
#include <soc/dnx/legacy/dnx_config_imp_defs.h>

#include <soc/dnx/legacy/drv.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Utils/sand_workload_status.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>

#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>

#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>

#include <soc/dnx/legacy/ARAD/arad_defs.h>
#include <soc/dnx/legacy/ARAD/arad_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_chip_defines.h>
#include <soc/dnx/legacy/ARAD/arad_init.h>
#include <soc/dnx/legacy/ARAD/arad_egr_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_flow_control.h>
#include <soc/dnx/legacy/ARAD/arad_ingress_scheduler.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_end2end.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_ports.h>
#include <soc/dnx/legacy/ARAD/arad_fabric.h>
#include <soc/dnx/legacy/ARAD/arad_ingress_packet_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_ingress_traffic_mgmt.h>
#include <soc/dnx/legacy/multicast.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_diagnostics.h>
#include <soc/dnx/legacy/ARAD/arad_cell.h>
#include <soc/dnx/legacy/ARAD/arad_tdm.h>
#include <soc/dnx/legacy/ARAD/arad_chip_regs.h>

#include <soc/dnx/legacy/port_sw_db.h>

#include <soc/dnx/legacy/JER/jer_regs.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */


#define JER2_ARAD_MGMT_INIT_EGQ_MAX_FRG_VAR            127

/*
 *  EGQ wait for init
 */

/*
 *  DRAM wait for init
 */

/*
 *  DRAM buffers
 */
#define JER2_ARAD_INIT_DRAM_BYTES_FOR_FBC               32
#define JER2_ARAD_INIT_DRAM_FBC_SEQUENCE_SIZE           11
#define JER2_ARAD_INIT_DRAM_BUFF_TO_FBC_DELTA_MIN       256


/*
 *  DRAM Configuration
 */


/*
 *  If set, allows
 *  viewing table initialization percentage
 */

/*
 *  If set, allows initialization without
 *  setting all the required interfaces.
 *  This mode is for bring-up/debug only
 */

/*
 *  Controls Duty Cycle (DCF-fix). Normally disabled
 */

/*
 *    OFP rates
 */

/*
 *  Minimum packet size in Interlaken mode
 */


#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_NDIV_INT_START (3)
#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_CH0_MDIV_START (13)
#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_PDIV_START (43)
#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_KP_START (27)
#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_KI_START (31)
#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_KA_START (34)
#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_PLL_CTRL_START (46)
#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_NDIV_INT_LENGTH (10)
#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_CH0_MDIV_LENGTH (8)
#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_PDIV_LENGTH (3)
#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_KP_LENGTH (4)
#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_KI_LENGTH (3)
#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_KA_LENGTH (3)

/* The bit should be set if Fvco > 3200)*/
#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_BIT_VCO_DIV2 (JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_PLL_CTRL_START + 26)
/* For fast lock always assert this BIT */
#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_BIT_IF_FAST (JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_PLL_CTRL_START + 27)
#define JER2_ARAD_MGMT_INIT_DDR_PLL_FREF_EFF (25)

#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_NDIV_MIN (2800/JER2_ARAD_MGMT_INIT_DDR_PLL_FREF_EFF)
#define JER2_ARAD_MGMT_INIT_DDR_PLL_CFG_I_NDIV_MAX (4000/JER2_ARAD_MGMT_INIT_DDR_PLL_FREF_EFF)

/* definitions for the calculation of the size in the external fbc
 * 64 bytes has place to 22 lines */


#define JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_NDIV_INT_START    (3)
#define JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_CH0_MDIV_START    (13)
#define JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_KP_START          (27)
#define JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_KI_START          (31)
#define JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_KA_START         (34)
#define JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_PDIV_START        (43)
#define JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_PLL_CTRL_START    (46)
#define JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_NDIV_INT_LENGTH   (8)
#define JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_CH0_MDIV_LENGTH   (8)
#define JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_KP_LENGTH         (4)
#define JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_KI_LENGTH         (3)
#define JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_KA_LENGTH        (3)
#define JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_PDIV_LENGTH       (3)
#define JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_PLL_CTRL_LENGTH   (30)

/* OCB buffer type table configuration */
#define JER2_ARAD_INIT_OCB_BUFFER_TYPE_NOF_ENTRIES               (320)

#define JER2_ARAD_INIT_OCB_BUF_TYPE_NO_OCB_BUF 0x0
#define JER2_ARAD_INIT_OCB_BUF_TYPE_UC_OCB_BUF 0x1
#define JER2_ARAD_INIT_OCB_BUF_TYPE_FLMC_OCB_BUF 0x2

#define JER2_ARAD_INIT_OCB_BUF_TYPE_FWD_UC 0x1
#define JER2_ARAD_INIT_OCB_BUF_TYPE_FWD_FMC 0x2
#define JER2_ARAD_INIT_OCB_BUF_TYPE_FWD_ING_MC_1_COPY 0x3
#define JER2_ARAD_INIT_OCB_BUF_TYPE_FWD_ING_MC_N_COPY 0x4

#define JER2_ARAD_INIT_OCB_BUF_TYPE_SNOOP_NO_SNOOP 0x0
#define JER2_ARAD_INIT_OCB_BUF_TYPE_MIRROR_NO_MIRROR 0x0
#define JER2_ARAD_INIT_OCB_BUF_TYPE_MIRROR_UC_FMC_MIRROR 0x1
#define JER2_ARAD_INIT_OCB_BUF_TYPE_MIRROR_ING_MC_MIRROR 0x2

#define JER2_ARAD_INIT_OCB_BUF_TYPE_QUEUE_ELIGIBILE 0x1

#define JER2_ARAD_INIT_OCB_BUF_TYPE_MCID_IN_RANGE 0x1

#define JER2_ARAD_INIT_DDR_FREQUENCY_25_MHZ    (25)
#define JER2_ARAD_INIT_DDR_FREQUENCY_125_MHZ   (125)

/*
 * link level flow control default threshold
 */
#define JER2_ARAD_INIT_LLFC_DUAL_PIPE_TH (39)
#define JER2_ARAD_INIT_LLFC_SINGLE_PIPE_TH (103)

/*
 * PetraB mode for FTMH
 * {
 */
/*
 * Header map nof entries
 */
#define JER2_ARAD_INIT_HM_NOF_ADDRS                      1024

#define JER2_ARAD_INIT_HM_TAKEN_FROM_EEI_EXT             0x1
#define JER2_ARAD_INIT_HM_TAKEN_FROM_EEP_EXT             0x2
#define JER2_ARAD_INIT_HM_TAKEN_FROM_LEARN_EXT           0x3
#define JER2_ARAD_INIT_HM_TAKEN_FROM_2b00_FTMH_EXT_13_0  0x4
#define JER2_ARAD_INIT_HM_TAKEN_FROM_2b01_FTMH_EXT_13_0  0x5
#define JER2_ARAD_INIT_HM_TAKEN_FROM_2b10_FTMH_EXT_13_0  0x6
#define JER2_ARAD_INIT_HM_TAKEN_FROM_PPH_SYSTEM_VSI      0x7

/*
 * Each addr index is a representation of the fields presented next.
 * The header map initialization is made based on this addr index.
 *
 * HMAI - Header Map Index
 *
 * Addr index fields:
 *
 * Fields: [ |-PphLearnExtAsdType-| |-EeiExtExists-| |-EepExtExists-| |-FtmhExtMsbs-| |-FwdCode-| ]
 * Bits:   [ |--------0:1---------| |------2-------| |------3-------| |-----4:5-----| |---6:9---| ]
 * LSB,Sz: [ |--------0,2---------| |-----2,1------| |-----3,1------| |-----4,2-----| |---6,4---| ]
 */
#define JER2_ARAD_INIT_HMI_EEI_EXT_EXISTS_LSB                    2
#define JER2_ARAD_INIT_HMI_EEP_EXT_EXISTS_LSB                    3
#define JER2_ARAD_INIT_HMI_FTMH_EXT_MSBS_LSB                     4
#define JER2_ARAD_INIT_HMI_FWD_CODE_LSB                          6

#define JER2_ARAD_INIT_HMI_EEI_EXT_EXISTS_NOF_BITS               1
#define JER2_ARAD_INIT_HMI_EEP_EXT_EXISTS_NOF_BITS               1
#define JER2_ARAD_INIT_HMI_FTMH_EXT_MSBS_NOF_BITS                2
#define JER2_ARAD_INIT_HMI_FWD_CODE_NOF_BITS                     4

/*
 * } PetraB mode for FTMH
 */

/*
 * defines for ext voltage mode settings
 */
#define JER2_ARAD_INIT_SYNCE_PADS_SEL_15                         2
#define JER2_ARAD_INIT_FC_PADS_SEL_33                            0
#define JER2_ARAD_INIT_FC_PADS_SEL_15                            2
#define JER2_ARAD_INIT_SYNCE_PADS_REF_INT_DS                     0
#define JER2_ARAD_INIT_SYNCE_PADS_AMP_EN                         1
#define JER2_ARAD_INIT_FC_PADS_REF_INT_EN                        1
#define JER2_ARAD_INIT_FC_PADS_REF_INT_DS                        0
#define JER2_ARAD_INIT_FC_PADS_AMP_EN                            1
#define JER2_ARAD_INIT_FC_PADS_AMP_DS                            0

/* } */

/*************
 *  MACROS   *
 *************/
/* { */



/*
 *  max value of the count
 *   it's equal to the maximum value count field may get - 1
*/


/*
 *  Prints initialization advance.
 *  Assumes the following variables are defined:
 *   - uint8 silent
 *   - uint32 stage_internal_id
 */
#define JER2_ARAD_INIT_PRINT_ADVANCE(str)                                           \
{                                                                              \
  if (!silent)                                                                 \
  {                                                                            \
   LOG_VERBOSE(BSL_LS_SOC_INIT, \
               (BSL_META_U(unit, \
                           "    + %.2u: %s\n\r"), ++stage_id, str));      \
  }                                                                            \
}

#define JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE(str)                                                \
{                                                                                                    \
  if (!silent)                                                                                       \
  {                                                                                                  \
   LOG_VERBOSE(BSL_LS_SOC_INIT, \
               (BSL_META_U(unit, \
                           "        ++ %.2u: %s\n\r"), ++stage_internal_id, str));             \
  }                                                                                                  \
}

#define JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_1_ADVANCE(str)                                                \
{                                                                                                    \
  if (!silent)                                                                                       \
  {                                                                                                  \
   LOG_VERBOSE(BSL_LS_SOC_INIT, \
               (BSL_META_U(unit, \
                           "            ++ %.2u: %s\n\r"), ++stage_internal_id, str));         \
  }                                                                                                  \
}

#define JER2_ARAD_INIT_CLAC_M_N_AND_DIFF(dram_freq, freq_diff, m, n, freq_diff_best, m_best, n_best)\
  /* The exact frequency is JER2_ARAD_MGMT_INIT_DDR_PLL_FREF_EFF*n/m */\
  freq_diff = ((JER2_ARAD_MGMT_INIT_DDR_PLL_FREF_EFF)*(n) - (dram_freq)*(m))/(m);\
  if(freq_diff < 0)\
  {\
    freq_diff = -freq_diff;\
  }\
\
  if(freq_diff < freq_diff_best)\
  {\
    m_best = (m);\
    n_best = n;\
    freq_diff_best = freq_diff;\
  }
/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/*
 *  DRAM buffers boundaries configuration.
 *  This configuration is per-buffer type: unicast, mini-multicast and full-multicast
 */
typedef struct {
  uint32 start;
  uint32 end;
}JER2_ARAD_MGMT_DBUFF_BOUNDARIES;

typedef struct {
  JER2_ARAD_MGMT_DBUFF_BOUNDARIES  ocb_fmc;
  JER2_ARAD_MGMT_DBUFF_BOUNDARIES  fmc;
  JER2_ARAD_MGMT_DBUFF_BOUNDARIES  mmc;
  JER2_ARAD_MGMT_DBUFF_BOUNDARIES  ocb_uc;
  JER2_ARAD_MGMT_DBUFF_BOUNDARIES  uc;
  JER2_ARAD_MGMT_DBUFF_BOUNDARIES  fbc_ocb_uc;
  JER2_ARAD_MGMT_DBUFF_BOUNDARIES  fbc_uc;
  JER2_ARAD_MGMT_DBUFF_BOUNDARIES  fbc_mmc;
  JER2_ARAD_MGMT_DBUFF_BOUNDARIES  fbc_ocb_fmc;
  JER2_ARAD_MGMT_DBUFF_BOUNDARIES  fbc_fmc;
} JER2_ARAD_INIT_DBUFFS_BDRY;

typedef enum
{
  /*
   * fabric
   */
  JER2_ARAD_INIT_SERDES_TYPE_FABRIC = 0,
  /*
   * nif
   */
  JER2_ARAD_INIT_SERDES_TYPE_NIF = 1,
  /*
   * number of jer2_arad init serdes types
   */
  JER2_ARAD_INIT_NOF_SREDES_TYPES = 2
}JER2_ARAD_INIT_SERDES_TYPE;

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */


static
soc_reg_t jer2_arad_interrupts_mask_registers[] = {
    CFC_INTERRUPT_MASK_REGISTERr,
    CRPS_INTERRUPT_MASK_REGISTERr,
    CRPS_PAR_ERR_INTERRUPT_MASK_REGISTERr,
    CRPS_SRC_INVLID_ACCESS_INTERRUPT_REGISTER_MASKr,
    EGQ_ECC_INTERRUPT_REGISTER_MASKr,
    EGQ_ERPP_DISCARD_INT_REG_MASKr,
    EGQ_ERPP_DISCARD_INT_REG_MASK_2r,
    EGQ_INTERRUPT_MASK_REGISTERr,
    EGQ_PKT_REAS_INT_REG_MASKr,
    EPNI_ECC_INTERRUPT_REGISTER_MASKr,
    EPNI_ESEM_INTERRUPT_MASK_REGISTER_ONEr,
    EPNI_INTERRUPT_MASK_REGISTERr,
    EPNI_PP_INT_REG_MASKr,
    FCR_INTERRUPT_MASK_REGISTERr,
    FCT_INTERRUPT_MASK_REGISTERr,
    FDR_INTERRUPT_MASK_REGISTERr,
    FDR_INTERRUPT_MASK_REGISTER_1r,
    FDR_INTERRUPT_MASK_REGISTER_2r,
    FDR_INTERRUPT_MASK_REGISTER_3r,
    FDT_INTERRUPT_MASK_REGISTERr,
    FMAC_INTERRUPT_MASK_REGISTERr,
    FMAC_INTERRUPT_MASK_REGISTER_1r,
    FMAC_INTERRUPT_MASK_REGISTER_2r,
    FMAC_INTERRUPT_MASK_REGISTER_3r,
    FMAC_INTERRUPT_MASK_REGISTER_4r,
    FMAC_INTERRUPT_MASK_REGISTER_5r,
    FMAC_INTERRUPT_MASK_REGISTER_6r,
    FMAC_INTERRUPT_MASK_REGISTER_7r,
    FMAC_INTERRUPT_MASK_REGISTER_8r,
    FMAC_INTERRUPT_MASK_REGISTER_9r,
    FSRD_INTERRUPT_MASK_REGISTERr,
    FSRD_QUAD_INTERRUPT_MASK_REGISTERr,
    FSRD_QUAD_INTERRUPT_MASK_REGISTERr,
    FSRD_QUAD_INTERRUPT_MASK_REGISTERr,
    IDR_ECC_INTERRUPT_REGISTER_MASKr,
    IDR_INTERRUPT_MASK_REGISTERr,
    IDR_REASSEMBLY_INTERRUPT_REGISTER_MASKr,
    IHB_INTERRUPT_MASK_REGISTERr,
    IHB_OEMA_INTERRUPT_MASK_REGISTER_ONEr,
    IHB_OEMB_INTERRUPT_MASK_REGISTER_ONEr,
    IHP_INTERRUPT_MASK_REGISTERr,
    IHP_ISA_INTERRUPT_MASK_REGISTER_ONEr,
    IHP_ISB_INTERRUPT_MASK_REGISTER_ONEr,
    IHP_MACT_INTERRUPT_MASK_REGISTER_TWOr,
    PPDB_B_LARGE_EM_INTERRUPT_REGISTER_ONEr,
    IPS_INTERRUPT_MASK_REGISTERr,
    IPT_ECC_ERR_INTERRUPT_MASK_REGISTERr,
    IPT_INTERRUPT_MASK_REGISTERr,
    IQM_ECC_INTERRUPT_REGISTER_MASKr,
    IQM_INTERRUPT_MASK_REGISTERr,
    IRE_ECC_INTERRUPT_REGISTER_MASKr,
    IRE_INTERRUPT_MASK_REGISTERr,
    IRR_ECC_INTERRUPT_REGISTER_MASKr,
    IRR_INTERRUPT_MASK_REGISTERr,
    MMU_ECC_INTERRUPT_REGISTER_MASKr,
    MMU_INTERRUPT_MASK_REGISTERr,
    NBI_ECC_INTERRUPT_MASK_REGISTERr,
    NBI_ILKN_INTERRUPT_MASK_REGISTERr,
    NBI_INTERRUPT_MASK_REGISTERr,
    NBI_LINK_STATUS_CHANGE_INTERRUPT_MASK_REGISTERr,
    NBI_NBI_THROWN_BURSTS_COUNTERS_75P_INTERRUPT_MASK_REGISTERr,
    NBI_STAT_CNT_75P_INTERRUPT_MASK_REGISTERr,
    NBI_STAT_INTERRUPT_MASK_REGISTERr,
    OAMP_ECC_INTERRUPT_REGISTER_MASKr,
    OAMP_INTERRUPT_MASK_REGISTERr,
    OAMP_RMAPEM_INTERRUPT_MASK_REGISTER_ONEr,
    OLP_ECC_INTERRUPT_REGISTER_MASKr,
    OLP_INTERRUPT_MASK_REGISTERr,
    RTP_INTERRUPT_MASK_REGISTERr,
    SCH_ECC_1B_ERR_INTERRUPT_MASK_REGISTERr,
    SCH_ECC_2B_ERR_INTERRUPT_MASK_REGISTERr,
    SCH_INTERRUPT_MASK_REGISTERr,
    SCH_PAR_ERR_INTERRUPT_MASK_REGISTERr,
    NUM_SOC_REG
};

static
soc_reg_t ardon_interrupts_mask_registers[] = {
    CFC_INTERRUPT_MASK_REGISTERr,
    CRPS_INTERRUPT_MASK_REGISTERr,
    CRPS_SRC_INVLID_ACCESS_INTERRUPT_REGISTER_MASKr,
    CRPS_SRC_CMD_WAS_FILTERED_INTERRUPT_REGISTER_MASKr,
    CRPS_PAR_ERR_INTERRUPT_MASK_REGISTERr,
    DRCC_INTERRUPT_MASK_REGISTERr,
    /* DRCE_INTERRUPT_MASK_REGISTERr, Internal error */
    /* DRCE_ECC_INTERRUPT_REGISTER_MASKr, internal error */
    /* DRCF_INTERRUPT_MASK_REGISTERr, internal error */
    /* DRCF_ECC_INTERRUPT_REGISTER_MASKr, internal error */
    /* DRCG_INTERRUPT_MASK_REGISTERr, internal error */
    /* DRCG_ECC_INTERRUPT_REGISTER_MASKr, internal error */
    /* DRCH_INTERRUPT_MASK_REGISTERr, internal error */
    /* DRCH_ECC_INTERRUPT_REGISTER_MASKr, internal error */
    EGQ_INTERRUPT_MASK_REGISTERr,
    EGQ_ERPP_DISCARD_INT_REG_MASKr,
    EGQ_ERPP_DISCARD_INT_REG_MASK_2r,
    EGQ_PKT_REAS_INT_REG_MASKr,
    EGQ_ECC_INTERRUPT_REGISTER_MASKr,
    EPNI_INTERRUPT_MASK_REGISTERr,
    EPNI_PP_INT_REG_MASKr,
    /* EPNI_ESEM_INTERRUPT_MASK_REGISTER_ONEr, error: op failed */
    EPNI_ECC_INTERRUPT_REGISTER_MASKr,
    FCR_INTERRUPT_MASK_REGISTERr,
    FCT_INTERRUPT_MASK_REGISTERr,
    FDR_INTERRUPT_MASK_REGISTERr,
    FDR_INTERRUPT_MASK_REGISTER_1r,
    FDR_INTERRUPT_MASK_REGISTER_2r,
    FDR_INTERRUPT_MASK_REGISTER_3r,
    FDT_INTERRUPT_MASK_REGISTERr,
    FMAC_INTERRUPT_MASK_REGISTERr,
    FMAC_INTERRUPT_MASK_REGISTER_1r,
    FMAC_INTERRUPT_MASK_REGISTER_2r,
    FMAC_INTERRUPT_MASK_REGISTER_3r,
    FMAC_INTERRUPT_MASK_REGISTER_4r,
    FMAC_INTERRUPT_MASK_REGISTER_5r,
    FMAC_INTERRUPT_MASK_REGISTER_6r,
    FMAC_INTERRUPT_MASK_REGISTER_7r,
    FMAC_INTERRUPT_MASK_REGISTER_8r,
    FMAC_INTERRUPT_MASK_REGISTER_9r,
    FSRD_INTERRUPT_MASK_REGISTERr,
    FSRD_QUAD_INTERRUPT_MASK_REGISTERr,
    IDR_INTERRUPT_MASK_REGISTERr,
    IDR_REASSEMBLY_INTERRUPT_REGISTER_MASKr,
    IDR_ECC_INTERRUPT_REGISTER_MASKr,
    IHB_INTERRUPT_MASK_REGISTERr,
    IHP_INTERRUPT_MASK_REGISTERr,
    IPS_INTERRUPT_MASK_REGISTERr,
    IPT_INTERRUPT_MASK_REGISTERr,
    IPT_ECC_ERR_INTERRUPT_MASK_REGISTERr,
    IQM_INTERRUPT_MASK_REGISTERr,
    IQM_ECC_INTERRUPT_REGISTER_MASKr,
    IRE_INTERRUPT_MASK_REGISTERr,
    IRE_ECC_INTERRUPT_REGISTER_MASKr,
    IRR_INTERRUPT_MASK_REGISTERr,
    IRR_ECC_INTERRUPT_REGISTER_MASKr,
    MMU_INTERRUPT_MASK_REGISTERr,
    MMU_ECC_INTERRUPT_REGISTER_MASKr,
    OCB_INTERRUPT_MASK_REGISTERr,
    OCB_ECC_INTERRUPT_REGISTER_MASKr,
    RTP_INTERRUPT_MASK_REGISTERr,
    SCH_INTERRUPT_MASK_REGISTERr,
    SCH_ECC_1B_ERR_INTERRUPT_MASK_REGISTERr,
    SCH_ECC_2B_ERR_INTERRUPT_MASK_REGISTERr,
    SCH_PAR_ERR_INTERRUPT_MASK_REGISTERr,
    NUM_SOC_REG
};

static
soc_reg_t jer2_arad_interrupt_monitor_mem_reg[] = {
    CFC_PARITY_ERR_MONITOR_MEM_MASKr,
    CRPS_PAR_ERR_MEM_MASKr,
    EGQ_ECC_ERR_1B_MONITOR_MEM_MASKr,
    EGQ_ECC_ERR_2B_MONITOR_MEM_MASKr,
    EPNI_ECC_ERR_1B_MONITOR_MEM_MASKr,
    EPNI_ECC_ERR_2B_MONITOR_MEM_MASKr,
    FCR_ECC_1B_ERR_MONITOR_MEM_MASKr,
    FCR_ECC_2B_ERR_MONITOR_MEM_MASKr,
    FDR_ECC_1B_ERR_MONITOR_MEM_MASK_PRIMARYr,
    FDR_ECC_1B_ERR_MONITOR_MEM_MASK_SECONDARYr,
    FDR_ECC_2B_ERR_MONITOR_MEM_MASK_PRIMARYr,
    FDR_ECC_2B_ERR_MONITOR_MEM_MASK_SECONDARYr,
    FDT_ECC_1B_ERR_MONITOR_MEM_MASKr,
    FDT_ECC_2B_ERR_MONITOR_MEM_MASKr,
    FDT_PARITY_ERR_MONITOR_MEM_MASKr,
    FMAC_ECC_1B_ERR_MONITOR_MEM_MASKr,
    FMAC_ECC_2B_ERR_MONITOR_MEM_MASKr,
    IDR_ECC_ERR_1B_MONITOR_MEM_MASKr,
    IDR_ECC_ERR_2B_MONITOR_MEM_MASKr,
    IDR_PAR_ERR_MEM_MASKr,
    IHP_ECC_ERR_1B_MONITOR_MEM_MASKr,
    IHP_ECC_ERR_2B_MONITOR_MEM_MASKr,
    IPS_ECC_ERR_1B_MONITOR_MEM_MASKr,
    IPS_ECC_ERR_2B_MONITOR_MEM_MASKr,
    IPS_PAR_ERR_MEM_MASKr,
    IPT_ECC_ERR_1B_MONITOR_MEM_MASKr,
    IPT_ECC_ERR_2B_MONITOR_MEM_MASKr,
    IPT_PAR_ERR_MEM_MASKr,
    IQM_ECC_ERR_1B_MONITOR_MEM_MASKr,
    IQM_ECC_ERR_2B_MONITOR_MEM_MASKr,
    IQM_PAR_ERR_MEM_MASKr,
    IRE_ECC_ERR_1B_MONITOR_MEM_MASKr,
    IRE_ECC_ERR_2B_MONITOR_MEM_MASKr,
    IRE_PAR_ERR_MEM_MASKr,
    IRR_ECC_ERR_1B_MONITOR_MEM_MASKr,
    IRR_ECC_ERR_2B_MONITOR_MEM_MASKr,
    IRR_PAR_ERR_MEM_MASKr,
    MMU_ECC_ERR_1B_MONITOR_MEM_MASK_1r,
    MMU_ECC_ERR_1B_MONITOR_MEM_MASK_2r,
    MMU_ECC_ERR_2B_MONITOR_MEM_MASK_1r,
    MMU_ECC_ERR_2B_MONITOR_MEM_MASK_2r,
    OAMP_ECC_ERR_MONITOR_MEM_MASKr,
    OLP_PAR_ERR_MEM_MASKr,
    IHP_PAR_0_ERR_MEM_MASKr,
    IHP_PAR_1_ERR_MEM_MASKr,
    IHP_PAR_3_ERR_MEM_MASKr,
    IHP_PAR_ERR_MONITOR_MEM_MASKr,
    NBI_ECC_ERR_1B_MONITOR_MEM_MASKr,
    NBI_ECC_ERR_2B_MONITOR_MEM_MASKr,
    OAMP_PAR_0_ERR_MONITOR_MEM_MASKr,
    EGQ_PAR_ERR_MEM_MASKr,
    NBI_PARITY_ERR_MONITOR_MEM_MASKr,
    IHB_PAR_ERR_MEM_MASKr,
    EPNI_PAR_ERR_MEM_MASKr,
    INVALIDr
};

static
soc_reg_t ardon_interrupt_monitor_mem_reg[] = {
    CFC_PARITY_ERR_MONITOR_MEM_MASKr,
    CRPS_PAR_ERR_MEM_MASKr,
    EGQ_ECC_ERR_1B_MONITOR_MEM_MASKr,
    EGQ_ECC_ERR_2B_MONITOR_MEM_MASKr,
    EGQ_PAR_ERR_MEM_MASKr,
    EPNI_ECC_ERR_1B_MONITOR_MEM_MASKr,
    EPNI_ECC_ERR_2B_MONITOR_MEM_MASKr,
    EPNI_PAR_ERR_MEM_MASKr,
    FCR_ECC_1B_ERR_MONITOR_MEM_MASKr,
    FCR_ECC_2B_ERR_MONITOR_MEM_MASKr,
    FDR_ECC_1B_ERR_MONITOR_MEM_MASK_PRIMARYr,
    FDR_ECC_2B_ERR_MONITOR_MEM_MASK_PRIMARYr,
    FDR_ECC_1B_ERR_MONITOR_MEM_MASK_SECONDARYr,
    FDR_ECC_2B_ERR_MONITOR_MEM_MASK_SECONDARYr,
    FDT_ECC_1B_ERR_MONITOR_MEM_MASKr,
    FDT_ECC_2B_ERR_MONITOR_MEM_MASKr,
    FDT_PARITY_ERR_MONITOR_MEM_MASKr,
    FMAC_ECC_1B_ERR_MONITOR_MEM_MASKr,
    FMAC_ECC_2B_ERR_MONITOR_MEM_MASKr,
    IDR_ECC_ERR_1B_MONITOR_MEM_MASKr,
    IDR_ECC_ERR_2B_MONITOR_MEM_MASKr,
    IDR_PAR_ERR_MEM_MASKr,
    IHB_PAR_ERR_MEM_MASKr,
    IHP_PAR_ERR_MONITOR_MEM_MASKr,
    IPS_ECC_ERR_2B_MONITOR_MEM_MASKr,
    IPS_ECC_ERR_1B_MONITOR_MEM_MASKr,
    IPS_PAR_ERR_MEM_MASKr,
    IPT_ECC_ERR_2B_MONITOR_MEM_MASKr,
    IPT_ECC_ERR_1B_MONITOR_MEM_MASKr,
    IPT_PAR_ERR_MEM_MASKr,
    IQM_ECC_ERR_1B_MONITOR_MEM_MASKr,
    IQM_ECC_ERR_2B_MONITOR_MEM_MASKr,
    IQM_PAR_ERR_MEM_MASKr,
    IRE_ECC_ERR_1B_MONITOR_MEM_MASKr,
    IRE_ECC_ERR_2B_MONITOR_MEM_MASKr,
    IRE_PAR_ERR_MEM_MASKr,
    IRR_ECC_ERR_1B_MONITOR_MEM_MASKr,
    IRR_ECC_ERR_2B_MONITOR_MEM_MASKr,
    IRR_PAR_ERR_MEM_MASKr,
    MMU_ECC_ERR_2B_MONITOR_MEM_MASK_1r,
    MMU_ECC_ERR_2B_MONITOR_MEM_MASK_2r,
    MMU_ECC_ERR_1B_MONITOR_MEM_MASK_1r,
    MMU_ECC_ERR_1B_MONITOR_MEM_MASK_2r,
    NBI_ECC_ERR_1B_MONITOR_MEM_MASKr,
    NBI_ECC_ERR_2B_MONITOR_MEM_MASKr,
    NBI_PARITY_ERR_MONITOR_MEM_MASKr,
    OCB_ECC_ERR_2B_MONITOR_MEM_MASKr,
    OCB_ECC_ERR_1B_MONITOR_MEM_MASKr,
    INVALIDr
};


/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */


uint32
jer2_arad_init_operation_mode_set(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  JER2_ARAD_MGMT_INIT             *init
    )
{
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  /*
   * Update SW DB with operation mode
   */
  jer2_arad_sw_db_tdm_mode_set(
          unit,
          init->tdm_mode
        );

  jer2_arad_sw_db_ilkn_tdm_dedicated_queuing_set(
          unit,
          init->ilkn_tdm_dedicated_queuing
        );

  jer2_arad_sw_db_is_petrab_in_system_set(
          unit,
          init->is_petrab_in_system
        );


  JER2_ARAD_DO_NOTHING_AND_EXIT;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_init_operation_mode_set()", 0, 0);
}

uint32
jer2_arad_init_pdm_nof_entries_calc(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  JER2_ARAD_INIT_PDM_MODE         pdm_mode,
    DNX_SAND_OUT uint32                     *pdm_nof_entries)
{
    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INIT_DRAM_NOF_BUFFS_CALC);

    if(pdm_mode == JER2_ARAD_INIT_PDM_MODE_SIMPLE) {
        *pdm_nof_entries = SOC_DNX_DEFS_GET(unit, pdm_size); /* For Arad: 1.5M */
    } else { /* PDM mode reduced */
        *pdm_nof_entries = SOC_DNX_DEFS_GET(unit, pdm_size)*3/4; /* For Arad: 0.75*1.5M */
    }

    JER2_ARAD_DO_NOTHING_AND_EXIT;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_init_pdm_nof_entries_calc()", 0, 0);
}
/*
 * Get the number of fbc (cache) buffers needed for given amount of
 * DRAM buffers.
 */
uint32
  jer2_arad_init_dram_fbc_buffs_get(
    DNX_SAND_IN  uint32  buffs_without_fbc,
    DNX_SAND_IN  uint32  buff_size_bytes,
    DNX_SAND_OUT uint32 *fbc_nof_bufs
  )
{
  uint32
    fbcs_for_buff,
    nof_fbc_buffs;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(JER2_ARAD_INIT_DRAM_FBC_BUFFS_GET);

  DNX_SAND_ERR_IF_OUT_OF_RANGE(
    buff_size_bytes, JER2_ARAD_ITM_DBUFF_SIZE_BYTES_MIN, JER2_ARAD_ITM_DBUFF_SIZE_BYTES_MAX,
    JER2_ARAD_ITM_DRAM_BUF_SIZE_OUT_OF_RANGE_ERR, 10, exit
  );

  fbcs_for_buff = DNX_SAND_DIV_ROUND_UP(buff_size_bytes, JER2_ARAD_INIT_DRAM_BYTES_FOR_FBC);
  nof_fbc_buffs = DNX_SAND_DIV_ROUND_UP(buffs_without_fbc, fbcs_for_buff * JER2_ARAD_INIT_DRAM_FBC_SEQUENCE_SIZE);

  if (nof_fbc_buffs == 0) {
      nof_fbc_buffs = 1;
  }

  *fbc_nof_bufs = nof_fbc_buffs;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_init_dram_fbc_buffs_get()", 0, 0);
}

uint32
  jer2_arad_init_dram_max_without_fbc_get(
    DNX_SAND_IN  uint32  buffs_with_fbc,
    DNX_SAND_IN  uint32  buff_size_bytes,
    DNX_SAND_OUT uint32 *buffs_without_fbc
  )
{
  DNX_SAND_U64
    dividend,
    buffs_no_fbc;
  uint32
    divisor;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(JER2_ARAD_INIT_DRAM_MAX_WITHOUT_FBC_GET);

  DNX_SAND_ERR_IF_OUT_OF_RANGE(
    buff_size_bytes, JER2_ARAD_ITM_DBUFF_SIZE_BYTES_MIN, JER2_ARAD_ITM_DBUFF_SIZE_BYTES_MAX,
    JER2_ARAD_ITM_DRAM_BUF_SIZE_OUT_OF_RANGE_ERR, 10, exit
  );

  dnx_sand_u64_multiply_longs(
    buffs_with_fbc,
    (JER2_ARAD_INIT_DRAM_FBC_SEQUENCE_SIZE * buff_size_bytes),
    &dividend
  );

  divisor = JER2_ARAD_INIT_DRAM_FBC_SEQUENCE_SIZE * buff_size_bytes + JER2_ARAD_INIT_DRAM_BYTES_FOR_FBC;

  dnx_sand_u64_devide_u64_long(
    &dividend,
    divisor,
    &buffs_no_fbc
  );

  dnx_sand_u64_to_long(&buffs_no_fbc, buffs_without_fbc);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_init_dram_max_without_fbc_get()", 0, 0);
}

/*
 * Pll Configurations
 */
static uint32 jer2_arad_init_serdes_pll_set(
    DNX_SAND_IN int                         unit,
    DNX_SAND_IN JER2_ARAD_INIT_SERDES_TYPE             serdes_type,
    DNX_SAND_IN soc_dnxc_init_serdes_ref_clock_t  ref_clock)
{

    uint32
        val,
        res;
    soc_reg_above_64_val_t
        reg_above_64,
        field_above_64;
    int32
        srd_ndx;
    const static soc_reg_t
        eci_srd_pll_config[] = {ECI_SRD_0_PLL_CONFIGr, ECI_SRD_1_PLL_CONFIGr};
    const static soc_reg_t
        eci_srd_pll_status[] = {ECI_SRD_0_PLL_STATUSr, ECI_SRD_1_PLL_STATUSr};
    const static soc_field_t
        eci_srd_pll_locked[] = {SRD_0_PLL_LOCKEDf, SRD_1_PLL_LOCKEDf};

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_INIT_SERDES_PLL_SET);

     if (serdes_type == JER2_ARAD_INIT_SERDES_TYPE_FABRIC) {
        srd_ndx = 0;
    } else {
        srd_ndx = 1;
    }

    /* Clear SRD PLL Config register */
    SOC_REG_ABOVE_64_CLEAR(reg_above_64);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, soc_reg_above_64_set(unit, eci_srd_pll_config[srd_ndx], REG_PORT_ANY, 0, reg_above_64));

    /* Configure SRD PLL config register */
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    val = 25;
    SHR_BITCOPY_RANGE(field_above_64, JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_NDIV_INT_START, &val, 0, JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_NDIV_INT_LENGTH);

    if (ref_clock == soc_dnxc_init_serdes_ref_clock_125) {
        val = 25;
    } else if (ref_clock == soc_dnxc_init_serdes_ref_clock_156_25) {
        val = 20;
    }
    SHR_BITCOPY_RANGE(field_above_64, JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_CH0_MDIV_START, &val, 0, JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_CH0_MDIV_LENGTH);

    val = 8;
    SHR_BITCOPY_RANGE(field_above_64, JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_KP_START, &val, 0, JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_KP_LENGTH);

    val = 1;
    SHR_BITCOPY_RANGE(field_above_64, JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_KI_START, &val, 0, JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_KI_LENGTH);

    val = 4;
    SHR_BITCOPY_RANGE(field_above_64, JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_KA_START, &val, 0, JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_KA_LENGTH);

    val = 1;
    SHR_BITCOPY_RANGE(field_above_64, JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_PDIV_START, &val, 0, JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_PDIV_LENGTH);

    val = 0x15440000;
    SHR_BITCOPY_RANGE(field_above_64, JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_PLL_CTRL_START, &val, 0, JER2_ARAD_MGMT_INIT_DDR_LCPLL_CFG_I_PLL_CTRL_LENGTH);

    soc_reg_above_64_field_set(unit, ECI_SRD_0_PLL_CONFIGr, reg_above_64, SRD_PLL_CTRLf, field_above_64);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, soc_reg_above_64_set(unit, eci_srd_pll_config[srd_ndx], REG_PORT_ANY, 0, reg_above_64));

    sal_usleep(10);

    /* asserting the reset bit */
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, soc_reg_above_64_get(unit, eci_srd_pll_config[srd_ndx], REG_PORT_ANY, 0, reg_above_64));
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
    soc_reg_above_64_field_set(unit, ECI_SRD_0_PLL_CONFIGr, reg_above_64, RESETBf, field_above_64);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, soc_reg_above_64_set(unit, eci_srd_pll_config[srd_ndx], REG_PORT_ANY, 0, reg_above_64));

    /* polling on pll_locked */
    res = jer2_arad_polling(
        unit,
        JER2_ARAD_TIMEOUT,
        JER2_ARAD_MIN_POLLS,
        eci_srd_pll_status[srd_ndx],
        REG_PORT_ANY,
        0,
        eci_srd_pll_locked[srd_ndx],
        1);
    DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

    /*post reset */
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 70, exit, soc_reg_above_64_get(unit, eci_srd_pll_config[srd_ndx], REG_PORT_ANY, 0, reg_above_64));
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
    soc_reg_above_64_field_set(unit, eci_srd_pll_config[srd_ndx], reg_above_64, POST_RESETBf, field_above_64);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 80, exit, soc_reg_above_64_set(unit, eci_srd_pll_config[srd_ndx], REG_PORT_ANY, 0, reg_above_64));

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_init_serdes_pll_set()", serdes_type, ref_clock);
}

uint32 jer2_arad_mgmt_init_ddr_configure(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  int32           ddr_id,
    DNX_SAND_IN  uint32           dram_freq,
    DNX_SAND_IN  uint32           synt_dram_freq)
{
   
    DNXC_LEGACY_FIXME_ASSERT;
    return -1;
}

static uint32 jer2_arad_init_ddr_pll_set(
    DNX_SAND_IN int                         unit,
    DNX_SAND_IN  JER2_ARAD_MGMT_INIT           *init)
{

   
    DNXC_LEGACY_FIXME_ASSERT;
    return -1;
}

static uint32 jer2_arad_init_ts_pll_set(
    DNX_SAND_IN int                         unit)
{

    uint32
        res,
        val;
    soc_reg_above_64_val_t
        reg_above_64,
        field_above_64;

    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

     /* Clear TS PLL Config/status register */
    SOC_REG_ABOVE_64_CLEAR(reg_above_64);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, soc_reg_above_64_set(unit, ECI_TS_PLL_CONFIGr, REG_PORT_ANY, 0, reg_above_64));

    /* Configure TS PLL config register - 0x1e000000000000000020000000809b80183c0 */
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    val = 0xb80183c0;
    SHR_BITCOPY_RANGE(field_above_64, 0, &val, 0, 32);
    val = 0x00000809;
    SHR_BITCOPY_RANGE(field_above_64, 32, &val, 0, 32);
    val = 0x00000200;
    SHR_BITCOPY_RANGE(field_above_64, 64, &val, 0, 32);
    val = 0x1e000;
    SHR_BITCOPY_RANGE(field_above_64, 128, &val, 0, 32);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CTRLf, field_above_64);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, soc_reg_above_64_set(unit, ECI_TS_PLL_CONFIGr, REG_PORT_ANY, 0, reg_above_64));

    sal_usleep(10);

    /* asserting the reset bit */
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, soc_reg_above_64_get(unit, ECI_TS_PLL_CONFIGr, REG_PORT_ANY, 0, reg_above_64));
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, RESETBf, field_above_64);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, soc_reg_above_64_set(unit, ECI_TS_PLL_CONFIGr, REG_PORT_ANY, 0, reg_above_64));

    /* polling on pll_locked */
    res = jer2_arad_polling(
        unit,
        JER2_ARAD_TIMEOUT,
        JER2_ARAD_MIN_POLLS,
        ECI_TS_PLL_STATUSr,
        REG_PORT_ANY,
        0,
        TS_PLL_LOCKEDf,
        1);
    DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

    /*post reset */
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 70, exit, soc_reg_above_64_get(unit, ECI_TS_PLL_CONFIGr, REG_PORT_ANY, 0, reg_above_64));
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, POST_RESETBf, field_above_64);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 80, exit, soc_reg_above_64_set(unit, ECI_TS_PLL_CONFIGr, REG_PORT_ANY, 0, reg_above_64));

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_init_ts_pll_set()", 0, 0);
}


static uint32 jer2_arad_init_bs_pll_set(
    DNX_SAND_IN int                         unit)
{

    uint32
        res;
    soc_reg_above_64_val_t
        reg_above_64,
        field_above_64;
    uint32 reg32_val;

    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

     /* Clear BS PLL Config/status register */
    SOC_REG_ABOVE_64_CLEAR(reg_above_64);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, soc_reg_above_64_set(unit, ECI_BS_PLL_CONFIGr, REG_PORT_ANY, 0, reg_above_64));



    /* Configure BS PLL config register to output (Out) 20Mhz to bs module. */
    /* BS PLL refferece clock (Ref) oscilator is 25Mhz                      */
    /* Out = (Ref / pdiv) * ndiv_int / mdiv                                 */
    /* Out = (25 / 1) * 120 / 150 = 20Mhz                                   */
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_WORD_SET(field_above_64,   1, 0);
    soc_reg_above_64_field_set(unit, ECI_BS_PLL_CONFIGr, reg_above_64, I_PDIVf,      field_above_64);
    SOC_REG_ABOVE_64_WORD_SET(field_above_64, 120, 0);
    soc_reg_above_64_field_set(unit, ECI_BS_PLL_CONFIGr, reg_above_64, I_NDIV_INTf,  field_above_64);
    SOC_REG_ABOVE_64_WORD_SET(field_above_64, 150, 0);
    soc_reg_above_64_field_set(unit, ECI_BS_PLL_CONFIGr, reg_above_64, I_CH_1_MDIVf, field_above_64);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, soc_reg_above_64_set(unit, ECI_BS_PLL_CONFIGr, REG_PORT_ANY, 0, reg_above_64));

    sal_usleep(10);

    /* asserting the reset bit */
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, soc_reg_above_64_get(unit, ECI_BS_PLL_CONFIGr, REG_PORT_ANY, 0, reg_above_64));
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
    soc_reg_above_64_field_set(unit, ECI_BS_PLL_CONFIGr, reg_above_64, RESETBf, field_above_64);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, soc_reg_above_64_set(unit, ECI_BS_PLL_CONFIGr, REG_PORT_ANY, 0, reg_above_64));

    /* polling on pll_locked */
    res = jer2_arad_polling(
        unit,
        JER2_ARAD_TIMEOUT,
        JER2_ARAD_MIN_POLLS,
        ECI_BS_PLL_STATUSr,
        REG_PORT_ANY,
        0,
        BS_PLL_LOCKEDf,
        1);
    DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

    /* post reset */
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 70, exit, soc_reg_above_64_get(unit, ECI_BS_PLL_CONFIGr, REG_PORT_ANY, 0, reg_above_64));
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
    soc_reg_above_64_field_set(unit, ECI_BS_PLL_CONFIGr, reg_above_64, POST_RESETBf, field_above_64);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 80, exit, soc_reg_above_64_set(unit, ECI_BS_PLL_CONFIGr, REG_PORT_ANY, 0, reg_above_64));


    /* set bs pll as input to the bs module */
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 90, exit, soc_reg_above_64_get(unit, ECI_BS_PLL_CONFIGr, REG_PORT_ANY, 0, reg_above_64));
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
    soc_reg_above_64_field_set(unit, ECI_BS_PLL_CONFIGr, reg_above_64, BS_CLK_SELECTf, field_above_64);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 100, exit, soc_reg_above_64_set(unit, ECI_BS_PLL_CONFIGr, REG_PORT_ANY, 0, reg_above_64));

    /* Set GPIO 0-1 to be used for Timesync events (1pps signal) */
    reg32_val = 0x0;
    soc_reg_field_set(unit, ECI_GPIO_TS_SELr, &reg32_val, GPIO_0_SELf, 0x1);
    soc_reg_field_set(unit, ECI_GPIO_TS_SELr, &reg32_val, GPIO_1_SELf, 0x1);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 22, exit, soc_reg32_set(unit, ECI_GPIO_TS_SELr, REG_PORT_ANY, 0, reg32_val));

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_init_bs_pll_set()", 0, 0);
}


uint32 jer2_arad_mgmt_init_pll_reset(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  JER2_ARAD_MGMT_INIT           *init)
{
    uint32
        res;

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_INIT_PLL_RESET);

    if (!SOC_IS_ARDON(unit)) {

        res = jer2_arad_init_ddr_pll_set(unit, init);
        DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);

        res = jer2_arad_init_serdes_pll_set(unit, JER2_ARAD_INIT_SERDES_TYPE_NIF, init->pll.nif_clk_freq);
        DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);

        res = jer2_arad_init_serdes_pll_set(unit, JER2_ARAD_INIT_SERDES_TYPE_FABRIC, init->pll.fabric_clk_freq);
        DNX_SAND_CHECK_FUNC_RESULT(res, 90, exit);
    }

    /* configure the TS_PLL that in order to use TimeSync 1588 (and BroadSync features) */
    if (init->pll.ts_clk_mode == 0x1) {
        res = jer2_arad_init_ts_pll_set(unit);
        DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit);
    }

    if (SOC_IS_ARADPLUS(unit) && (!SOC_IS_ARDON(unit))) {
        if (soc_property_get(unit, spn_PHY_1588_DPLL_FREQUENCY_LOCK, 0)) {
            res = jer2_arad_init_bs_pll_set(unit);
            DNX_SAND_CHECK_FUNC_RESULT(res, 110, exit);
        }
    }

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_mgmt_init_pll_reset()", 0, 0);
}

/*
 *  DRAM buffers initialization }
 */

/*
 *  All the configurations in this function must occur before setting
 *  Arad internal blocks Out Of Reset.
 *  All these configurations affect ECC registers.
 *  This includes the following configurations:
 *  - DRAM Buffers
 *  - QDR protection type
 *  - Fabric CRC enable/disable
 *  - DRAM CRC enable/disable
 *  This function must be called at least once.
 */
static uint32
  jer2_arad_mgmt_init_before_blocks_oor(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  JER2_ARAD_INIT_DBUFFS_BDRY        *dbuffs_bdries,
    DNX_SAND_IN  JER2_ARAD_MGMT_INIT               *init
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    fld_val,
    reg_val;
/*  uint8
    is_stag,
    is_fap2x_coexist;*/
  JER2_ARAD_MGMT_FTMH_LB_EXT_MODE
    ftmh_lb_ext_mode;
  uint32
    ftmh_lb_key_enable;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_INIT_BEFORE_BLOCKS_OOR);

  DNX_SAND_CHECK_NULL_INPUT(init);
  /************************************************************************/
  /*  Configure DRAM PLL                                              */
  /************************************************************************/

    res = jer2_arad_mgmt_init_pll_reset(
          unit,
          init
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  /************************************************************************/
  /* Fabric                                                               */
  /************************************************************************/

  /*
   *  Mesh Mode
   */
  if (init->fabric.connect_mode == JER2_ARAD_FABRIC_CONNECT_MODE_MESH && !(SOC_DNX_CONFIG(unit)->tdm.is_bypass))
  {
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  100,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ECI_GLOBAL_1r, REG_PORT_ANY, 0, MESH_MODEf,  0x1));
  }
  else
  {
    /* Not enabled, also for BACK2BACK and single-context devices) */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  110,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ECI_GLOBAL_1r, REG_PORT_ANY, 0, MESH_MODEf,  0x0));
  }

  /*
   *  CRC enable and send
   */
  fld_val = 0x1;
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  120,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ECI_GLOBAL_1r, REG_PORT_ANY, 0, ADD_DRAM_CRCf,  fld_val));

  /*
   *  Disable fabric CRC only if fap20/fap21 in system
   */
  fld_val = 0;
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  130,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ECI_GLOBAL_1r, REG_PORT_ANY, 0, NO_FAB_CRCf,  fld_val));

  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  res = jer2_arad_ports_ftmh_extension_set_unsafe(
          unit,
          init->fabric.ftmh_extension
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 150, exit);
#endif 

  /*
   * FTMH LB mode
   */
  ftmh_lb_ext_mode = init->fabric.ftmh_lb_ext_mode;
  ftmh_lb_key_enable = 0;
  if (ftmh_lb_ext_mode != JER2_ARAD_MGMT_FTMH_LB_EXT_MODE_DISABLED) {
    ftmh_lb_key_enable = 0x1;
  }

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  35,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_ECI_GLOBALFr(unit, &reg_val));
  JER2_ARAD_FLD_TO_REG(ECI_GLOBALFr, FTMH_LB_KEY_EXT_ENf, ftmh_lb_key_enable, reg_val, 40, exit);
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  55,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_ECI_GLOBALFr(unit,  reg_val));

  /*
   * FTMH Stacking mode
   */
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  160,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_ECI_GLOBALFr(unit, &reg_val));
  JER2_ARAD_FLD_TO_REG(ECI_GLOBALFr, FTMH_STACKING_EXT_ENABLEf, init->fabric.ftmh_stacking_ext_mode, reg_val, 170, exit);
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  180,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_ECI_GLOBALFr(unit,  reg_val));

  /*
   * Pad Configuration
   */
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  190,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_ECI_PAD_CONFIGURATION_REGISTERr(unit, &reg_val));
  JER2_ARAD_FLD_TO_REG(ECI_PAD_CONFIGURATION_REGISTERr, SYNCE_PADS_SELf, JER2_ARAD_INIT_SYNCE_PADS_SEL_15, reg_val, 210, exit);
  JER2_ARAD_FLD_TO_REG(ECI_PAD_CONFIGURATION_REGISTERr, SYNCE_PADS_REF_INT_ENf, JER2_ARAD_INIT_SYNCE_PADS_REF_INT_DS , reg_val, 220, exit);
  JER2_ARAD_FLD_TO_REG(ECI_PAD_CONFIGURATION_REGISTERr, SYNCE_PADS_AMP_ENf, JER2_ARAD_INIT_SYNCE_PADS_AMP_EN , reg_val, 230, exit);

  if (init->ex_vol_mod == JER2_ARAD_MGMT_EXT_VOL_MOD_HSTL_1p5V) {
      JER2_ARAD_FLD_TO_REG(ECI_PAD_CONFIGURATION_REGISTERr, FC_PADS_SELf, JER2_ARAD_INIT_FC_PADS_SEL_15, reg_val, 240, exit);
      JER2_ARAD_FLD_TO_REG(ECI_PAD_CONFIGURATION_REGISTERr, FC_PADS_REF_INT_ENf, JER2_ARAD_INIT_FC_PADS_REF_INT_DS, reg_val, 250, exit);
      JER2_ARAD_FLD_TO_REG(ECI_PAD_CONFIGURATION_REGISTERr, FC_PADS_AMP_ENf, JER2_ARAD_INIT_FC_PADS_AMP_EN, reg_val, 260, exit);
  } else if (init->ex_vol_mod == JER2_ARAD_MGMT_EXT_VOL_MOD_3p3V) {
      JER2_ARAD_FLD_TO_REG(ECI_PAD_CONFIGURATION_REGISTERr, FC_PADS_SELf, JER2_ARAD_INIT_FC_PADS_SEL_33, reg_val, 270, exit);
      JER2_ARAD_FLD_TO_REG(ECI_PAD_CONFIGURATION_REGISTERr, FC_PADS_REF_INT_ENf, JER2_ARAD_INIT_FC_PADS_REF_INT_DS, reg_val, 280, exit);
      JER2_ARAD_FLD_TO_REG(ECI_PAD_CONFIGURATION_REGISTERr, FC_PADS_AMP_ENf, JER2_ARAD_INIT_FC_PADS_AMP_DS, reg_val, 290, exit)
  } else if (init->ex_vol_mod == JER2_ARAD_MGMT_EXT_VOL_MOD_HSTL_1p5V_VDDO) {
      JER2_ARAD_FLD_TO_REG(ECI_PAD_CONFIGURATION_REGISTERr, FC_PADS_SELf, JER2_ARAD_INIT_FC_PADS_SEL_15, reg_val, 300, exit);
      JER2_ARAD_FLD_TO_REG(ECI_PAD_CONFIGURATION_REGISTERr, FC_PADS_REF_INT_ENf, JER2_ARAD_INIT_FC_PADS_REF_INT_EN, reg_val, 310, exit);
      JER2_ARAD_FLD_TO_REG(ECI_PAD_CONFIGURATION_REGISTERr, FC_PADS_AMP_ENf, JER2_ARAD_INIT_FC_PADS_AMP_EN, reg_val, 320, exit);
  }

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  330,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_ECI_PAD_CONFIGURATION_REGISTERr(unit,  reg_val));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_init_before_blocks_oor()", 0, 0);
}

/*
 *  Complete the initialization based on pre-Out-Of-Reset configuration
 */
static uint32
  jer2_arad_mgmt_init_after_blocks_oor(
    DNX_SAND_IN int unit,
    DNX_SAND_IN  JER2_ARAD_MGMT_INIT        *hw_adj,
    DNX_SAND_IN  JER2_ARAD_INIT_DBUFFS_BDRY      *dbuffs_bdries
  )
{
  uint32
    index,
    res;
  uint64
      val64;
  
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_INIT_AFTER_BLOCKS_OOR);

  if (SOC_IS_QAX(unit)) {
      DNX_SAND_SET_ERROR_MSG((_BSL_DNX_SAND_MSG("access to registers should be fixed for JER2_QAX at places we used _REG(32|64) access routines")));
  }


  /* Init broadcast 60 - FSRD */
  for (index=0 ; index < SOC_DNX_DEFS_GET(unit, nof_instances_fsrd) ; index++) {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  18,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_FSRD_SBUS_BROADCAST_IDr(unit,  index,  60));
  }

  /* Init broadcast 61 - FMAC */
  for (index=0 ; index < SOC_DNX_DEFS_GET(unit, nof_instances_fmac) ; index++) {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  19,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_FMAC_SBUS_BROADCAST_IDr(unit,  index,  61));
  }


  /************************************************************************/
  /* DRAM Buffers configuration                                                             */
  /************************************************************************/
  
  DNXC_LEGACY_FIXME_ASSERT;

    /*
     * RTP
     */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  440,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, RTP_MC_TRAVERSE_RATEr, REG_PORT_ANY, 0, MC_TRAVERSE_RATEf,  0x1E00));

    /*
     * PVT
     */
    if (SOC_IS_ARDON(unit)) {
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 480, exit, ardon_mgmt_drv_pvt_monitor_enable(unit));

    } else {

        COMPILER_64_SET(val64, 0, 0x01D0003);
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 490, exit, WRITE_ECI_PVT_MON_A_CONTROL_REGr(unit, val64));
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 500, exit, WRITE_ECI_PVT_MON_B_CONTROL_REGr(unit, val64));
        sal_usleep(20000);
        COMPILER_64_SET(val64, 0, 0x41D0003);
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 510, exit, WRITE_ECI_PVT_MON_A_CONTROL_REGr(unit, val64));
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 520, exit, WRITE_ECI_PVT_MON_B_CONTROL_REGr(unit, val64));
    }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_init_after_blocks_oor()", 0, 0);
}


uint32
  jer2_arad_mgmt_init_finalize(
    DNX_SAND_IN int unit
  )
{
    int rv;
    soc_port_t port, link;
    uint32 reg_val,res = DNX_SAND_OK;

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_INIT_FINALIZE_ERR);

    /* clear all FMAC interrupts */
    PBMP_SFI_ITER(unit, port)
    {
        link = SOC_DNX_FABRIC_PORT_TO_LINK(unit, port);
        rv = jer2_arad_fabric_link_status_clear(unit, link);
        if (rv != SOC_E_NONE) {
            DNX_SAND_SET_ERROR_CODE(JER2_ARAD_MGMT_INIT_FINALIZE_ERR, 10, exit);
        }
    }

    /* Set trigger of EPNI credits to OAM/OLP interfaces (Cmicm interface is init at rx module) */
    reg_val = 0x0;
    soc_reg_field_set(unit, EPNI_INIT_TXI_CONFIGr, &reg_val, INIT_TXI_OLPf, 0x1);
    soc_reg_field_set(unit, EPNI_INIT_TXI_CONFIGr, &reg_val, INIT_TXI_OAMf, 0x1);
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_EPNI_INIT_TXI_CONFIGr(unit, SOC_CORE_ALL,  reg_val));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_init_finalize()", 0, 0);
}

int soc_dnx_jer2_arad_dma_init(int unit)
{
    uint32 reg_val;
    soc_reg_above_64_val_t above_64;

    DNXC_INIT_FUNC_DEFS;
    reg_val = 0x0;

    if (SOC_IS_JERICHO(unit)) {
        /* reset pir credits */
        DNXC_IF_ERR_EXIT(READ_ECI_GP_CONTROL_9r(unit, above_64));
        soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, above_64, PIR_TXI_CREDITS_INIT_VALUEf, 32);
        soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, above_64, PIR_TXI_CREDITS_INITf, 1);
        SOC_DNX_ALLOW_WARMBOOT_WRITE(WRITE_ECI_GP_CONTROL_9r(unit, above_64), _rv);
        soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, above_64, PIR_TXI_CREDITS_INITf, 0);
        SOC_DNX_ALLOW_WARMBOOT_WRITE(WRITE_ECI_GP_CONTROL_9r(unit, above_64), _rv);
    } else {
        /* Set trigger of EPNI credits to CMICM */
        soc_reg_field_set(unit, EPNI_INIT_TXI_CONFIGr, &reg_val, INIT_TXI_CMICMf, 0x1);
        SOC_DNX_ALLOW_WARMBOOT_WRITE(soc_reg32_set(unit, EPNI_INIT_TXI_CONFIGr, REG_PORT_ANY, 0, reg_val), _rv);
    }

exit:
    DNXC_FUNC_RETURN;
}

/*
 *      IPT block default values
 */
static uint32 jer2_arad_mgmt_ipt_init(DNX_SAND_IN int unit);

/*
 *  Init sequence -
 *  per-block initialization, hardware adjustments etc. {
 */
static uint32
  jer2_arad_mgmt_functional_init(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_MGMT_INIT           *init,
    DNX_SAND_IN  uint8                 silent
  )
{
  uint32
    res = DNX_SAND_OK;
  soc_error_t
    rc = SOC_E_NONE;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_FUNCTIONAL_INIT);


  res = jer2_arad_egr_queuing_init(
        unit,
        init->eg_cgm_scheme
      );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

   res = jer2_arad_mgmt_ipt_init(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 15, exit);


   res = jer2_arad_init_mesh_topology(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 17, exit);

  res = jer2_arad_fabric_init(
          unit,
          &(init->fabric)
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = jer2_arad_ofp_rates_init(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

  res = jer2_arad_scheduler_end2end_init(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);

  res = jer2_arad_ipq_init(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 95, exit);

  res = jer2_arad_itm_init(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit);

  /* FC should be enabled after ING TM FC thresholds are configures (in jer2_arad_itm_init) */
  rc = jer2_arad_flow_control_init(
          unit
        );
  if (SOC_FAILURE(rc)) {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 120, exit);
  }

  res = dnx_mc_init(unit);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 130, exit);
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY  
  res = jer2_arad_ports_init(unit);
  DNX_SAND_CHECK_FUNC_RESULT(res, 200, exit);
#endif 

  res = jer2_arad_tdm_init(unit);
  DNX_SAND_CHECK_FUNC_RESULT(res, 230, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_functional_init()", 0, 0);
}


uint32
jer2_arad_mgmt_nbi_ecc_mask_get(
      DNX_SAND_IN int                unit,
      DNX_SAND_OUT uint64            *mask
      )
{
    soc_port_t port = 0, master_port = 0;
    uint32 res, offset;
    uint64 lanes_mask;
    uint32 lanes_count = 0;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    /*Bit [0:22] use for MLF and ILKN TX ADAP are allways monitored.
      Bits[23:46] ILKN memories monitored by ILKN lanes that in use*/
    COMPILER_64_SET(*mask, 0, 0x07fffff);
    PBMP_IL_ITER(unit, port){
       DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, dnx_port_sw_db_master_channel_get(unit, port, &master_port));
       if(master_port != port){
           continue;
       }
       DNX_SAND_SOC_IF_ERROR_RETURN(res, 11, exit, dnx_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
       DNX_SAND_SOC_IF_ERROR_RETURN(res, 12, exit, dnx_port_sw_db_num_lanes_get(unit, port, &lanes_count));
       if(offset == 1){
           COMPILER_64_MASK_CREATE(lanes_mask, lanes_count, 47 - lanes_count); /*from last ILKN memory and back*/
       } else{
           COMPILER_64_MASK_CREATE(lanes_mask, lanes_count, 23); /*from first ILKN memory and forward*/
       }

       COMPILER_64_OR(*mask, lanes_mask);
    }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_nbi_ecc_mask_get()", 0, 0);
}

uint32
jer2_arad_ser_init(int unit)
{
    uint32 inst_idx, reg32_val, res;
    uint64                 field64;
    soc_reg_above_64_val_t above_64;
    soc_block_types_t  block;
    int blk;
    int instance;

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_HW_SET_DEFAULTS);

    if (!SOC_UNIT_NUM_VALID(unit)) {
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_ILLEGAL_DEVICE_ID, 4, exit);
    }
    if (!SOC_WARM_BOOT(unit)) {
        if (!SOC_IS_ARDON(unit)) {
            /* unmask SER monitor registers*/
            SOC_REG_ABOVE_64_ALLONES(above_64);
            for(inst_idx=0; jer2_arad_interrupt_monitor_mem_reg[inst_idx] != INVALIDr; inst_idx++) {
                block = SOC_REG_INFO(unit, jer2_arad_interrupt_monitor_mem_reg[inst_idx]).block;
                SOC_BLOCKS_ITER(unit, blk, block) {
                    instance = (SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_CLP || SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_XLP) ? SOC_BLOCK_PORT(unit, blk) : SOC_BLOCK_NUMBER(unit, blk);
                    DNX_SAND_SOC_IF_ERROR_RETURN(res, 24, exit, soc_reg_above_64_set(unit, jer2_arad_interrupt_monitor_mem_reg[inst_idx], instance, 0, above_64));
                }
            }
            reg32_val = 0xFFFFFFFF;
            if (SOC_IS_ARAD_B1_AND_BELOW(unit)) {
                DNX_SAND_SOC_IF_ERROR_RETURN(res, 22, exit, soc_reg32_set(unit, IHB_ECC_ERR_1B_MONITOR_MEM_MASKr, REG_PORT_ANY, 0, reg32_val));
                DNX_SAND_SOC_IF_ERROR_RETURN(res, 22, exit, soc_reg32_set(unit, IHB_ECC_ERR_2B_MONITOR_MEM_MASKr, REG_PORT_ANY, 0, reg32_val));
            }
            if (SOC_IS_ARADPLUS_A0(unit)) {
                DNX_SAND_SOC_IF_ERROR_RETURN(res, 22, exit, soc_reg32_set(unit, IHB_REG_009A_1r, REG_PORT_ANY, 0, reg32_val));
                DNX_SAND_SOC_IF_ERROR_RETURN(res, 22, exit, soc_reg32_set(unit, IHB_REG_009A_2r, REG_PORT_ANY, 0, reg32_val));
            }
            DNX_SAND_SOC_IF_ERROR_RETURN(res, 37, exit, soc_reg_above_64_field32_modify(unit, NBI_PARITY_ERR_MONITOR_MEM_MASKr, REG_PORT_ANY, 0, RLENG_MEM_PARITY_ERR_MASKf, 0));
            DNX_SAND_SOC_IF_ERROR_RETURN(res, 38, exit, soc_reg_above_64_field32_modify(unit, NBI_PARITY_ERR_MONITOR_MEM_MASKr, REG_PORT_ANY, 0, TLENG_MEM_PARITY_ERR_MASKf, 0));
            DNX_SAND_SOC_IF_ERROR_RETURN(res, 39, exit, soc_reg_above_64_field32_modify(unit, NBI_PARITY_ERR_MONITOR_MEM_MASKr, REG_PORT_ANY, 0, RTYPE_MEM_PARITY_ERR_MASKf, 0));
            DNX_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, soc_reg_above_64_field32_modify(unit, NBI_PARITY_ERR_MONITOR_MEM_MASKr, REG_PORT_ANY, 0, TTYPE_MEM_PARITY_ERR_MASKf, 0));
            DNX_SAND_SOC_IF_ERROR_RETURN(res, 41, exit, soc_reg_above_64_field32_modify(unit, NBI_PARITY_ERR_MONITOR_MEM_MASKr, REG_PORT_ANY, 0, RPKTS_MEM_PARITY_ERR_MASKf, 0));
            DNX_SAND_SOC_IF_ERROR_RETURN(res, 42, exit, soc_reg_above_64_field32_modify(unit, NBI_PARITY_ERR_MONITOR_MEM_MASKr, REG_PORT_ANY, 0, TPKTS_MEM_PARITY_ERR_MASKf, 0));
            DNX_SAND_SOC_IF_ERROR_RETURN(res, 43, exit, soc_reg_above_64_field32_modify(unit, NBI_PARITY_ERR_MONITOR_MEM_MASKr, REG_PORT_ANY, 0, RMON_MEM_PARITY_ERR_MASKf, 0));
            DNX_SAND_SOC_IF_ERROR_RETURN(res, 44, exit, soc_reg_above_64_field32_modify(unit, NBI_PARITY_ERR_MONITOR_MEM_MASKr, REG_PORT_ANY, 0, TMON_MEM_PARITY_ERR_MASKf, 0));
        } else {
            /* unmask SER monitor registers*/
            SOC_REG_ABOVE_64_ALLONES(above_64);
            for(inst_idx=0; ardon_interrupt_monitor_mem_reg[inst_idx] != INVALIDr; inst_idx++) {
                block = SOC_REG_INFO(unit, ardon_interrupt_monitor_mem_reg[inst_idx]).block;
                SOC_BLOCKS_ITER(unit, blk, block) {
                    instance = (SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_CLP || SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_XLP) ? SOC_BLOCK_PORT(unit, blk) : SOC_BLOCK_NUMBER(unit, blk);
                    DNX_SAND_SOC_IF_ERROR_RETURN(res, 24, exit, soc_reg_above_64_set(unit, ardon_interrupt_monitor_mem_reg[inst_idx], instance, 0, above_64));
                }
            }
        }

        DNX_SAND_SOC_IF_ERROR_RETURN(res, 24, exit, jer2_arad_mgmt_nbi_ecc_mask_get(unit, &field64));
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 25, exit, soc_reg_set(unit, NBI_ECC_ERR_1B_MONITOR_MEM_MASKr, REG_PORT_ANY, 0, field64));
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 26, exit, soc_reg_set(unit, NBI_ECC_ERR_2B_MONITOR_MEM_MASKr, REG_PORT_ANY, 0, field64));

        /* exclude list - these are memories which cause false alarm inidication */
        if(!SOC_IS_JERICHO_PLUS_A0(unit)){
            DNX_SAND_SOC_IF_ERROR_RETURN(res, 28, exit, soc_reg_field32_modify(unit, EGQ_ECC_ERR_1B_MONITOR_MEM_MASKr, REG_PORT_ANY, BUF_LINK_ECC_1B_ERR_MASKf, 0));
            DNX_SAND_SOC_IF_ERROR_RETURN(res, 29, exit, soc_reg_field32_modify(unit, EGQ_ECC_ERR_2B_MONITOR_MEM_MASKr, REG_PORT_ANY, BUF_LINK_ECC_2B_ERR_MASKf, 0));
        }
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, soc_reg_field32_modify(unit, IDR_ECC_ERR_1B_MONITOR_MEM_MASKr, REG_PORT_ANY, CHUNK_STATUS_ECC__NB_ERR_MASKf, 0));
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 31, exit, soc_reg_field32_modify(unit, IDR_ECC_ERR_2B_MONITOR_MEM_MASKr, REG_PORT_ANY, CHUNK_STATUS_ECC__NB_ERR_MASKf, 0));
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 32, exit, soc_reg_field32_modify(unit, IDR_PAR_ERR_MEM_MASKr, REG_PORT_ANY, CONTEXT_STATUS_DATA_PARITY_ERR_MASKf, 0));
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 33, exit, soc_reg_field32_modify(unit, IRR_PAR_ERR_MEM_MASKr, REG_PORT_ANY, SNOOP_MIRROR_TABLE_0_PARITY_ERR_MASKf, 0));
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 34, exit, soc_reg_field32_modify(unit, IRR_PAR_ERR_MEM_MASKr, REG_PORT_ANY, SNOOP_MIRROR_TABLE_1_PARITY_ERR_MASKf, 0));
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 35, exit, soc_reg_field32_modify(unit, IRE_PAR_ERR_MEM_MASKr, REG_PORT_ANY, NIF_PORT_TO_CTXT_BIT_MAP_PARITY_ERR_MASKf, 0));
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 33, exit, soc_reg_field32_modify(unit, IRE_ECC_ERR_2B_MONITOR_MEM_MASKr, REG_PORT_ANY, CTXT_MEM_CONTROL_ECC__NB_ERR_MASKf, 0));
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 34, exit, soc_reg_field32_modify(unit, IRE_ECC_ERR_1B_MONITOR_MEM_MASKr, REG_PORT_ANY, CTXT_MEM_CONTROL_ECC__NB_ERR_MASKf, 0));
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 35, exit, soc_reg_field32_modify(unit, IRE_PAR_ERR_MEM_MASKr, REG_PORT_ANY, CTXT_MEM_DATA_PARITY_ERR_MASKf, 0));
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 36, exit, soc_reg_above_64_field32_modify(unit, EPNI_PAR_ERR_MEM_MASKr, REG_PORT_ANY, 0, CNTX_PARITY_ERR_MASKf, 0));


    }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_hw_set_defaults()", 0, 0);
}

static uint32
  jer2_arad_mgmt_hw_set_defaults(
    DNX_SAND_IN  int                 unit
  )
{
   uint32
    inst_idx = 0,
    res;
   soc_reg_above_64_val_t above_64;
   soc_block_types_t  block;
   int blk;
   int instance;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_HW_SET_DEFAULTS);



    if (!SOC_UNIT_NUM_VALID(unit)) {
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_ILLEGAL_DEVICE_ID, 4, exit);
    }
    if (!SOC_WARM_BOOT(unit)) {
        SOC_REG_ABOVE_64_CLEAR(above_64);
        if (SOC_IS_ARDON(unit)) {
            for(inst_idx=0; ardon_interrupts_mask_registers[inst_idx] != NUM_SOC_REG; ++inst_idx) {
                block = SOC_REG_INFO(unit, ardon_interrupts_mask_registers[inst_idx]).block;
                SOC_BLOCKS_ITER(unit, blk, block) {
                    instance = (SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_CLP || SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_XLP) ? SOC_BLOCK_PORT(unit, blk) : SOC_BLOCK_NUMBER(unit, blk);
                    DNX_SAND_SOC_IF_ERROR_RETURN(res, 19, exit, soc_reg_above_64_set(unit, ardon_interrupts_mask_registers[inst_idx], instance, 0, above_64));
                }
            }
        } else {
            for(inst_idx=0; jer2_arad_interrupts_mask_registers[inst_idx] != NUM_SOC_REG; ++inst_idx) {
                block = SOC_REG_INFO(unit, jer2_arad_interrupts_mask_registers[inst_idx]).block;
                SOC_BLOCKS_ITER(unit, blk, block) {
                    instance = (SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_CLP || SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_XLP) ? SOC_BLOCK_PORT(unit, blk) : SOC_BLOCK_NUMBER(unit, blk);
                    DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, soc_reg_above_64_set(unit, jer2_arad_interrupts_mask_registers[inst_idx], instance, 0, above_64));
                }
            }
        }
        /* mask dram interrupts */
         
        DNXC_LEGACY_FIXME_ASSERT;
    }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_hw_set_defaults()", 0, 0);
}




/*
 *      MESH_TOPOLOGY block default values
 */
uint32
  jer2_arad_init_mesh_topology(
    DNX_SAND_IN int             unit
    )
{
    uint32 field_val;
    DNXC_INIT_FUNC_DEFS;

    field_val = SOC_DNX_IMP_DEFS_GET(unit, mesh_topology_register_config_4_val);

    if (!SOC_UNIT_NUM_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("Unit num %d is invalid\n"), unit));
    }

    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, REG_PORT_ANY, 0, CONFIG_4f, field_val));

exit:
    DNXC_FUNC_RETURN;
}

/*
 *      IPT block default values
 */
static uint32
    jer2_arad_mgmt_ipt_init(
      DNX_SAND_IN int             unit
      )
{
    uint32
        fabric_connect_mode,
        is_dual_mode,
        dtq_size0, dtq_size1, dtq_size2_6,
        dtq_th0, dtq_th1, dtq_th2_6,
        dpq_size0_5, dpq_size6_15,
        dpq_dqcq_th0_5, dpq_dqcq_th6_15,
        dpq_eir_th0_5, dpq_eir_th6_15,
        dpq_mc_th,
        reg32_val,
        fld;
    uint32 res;
    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_IPT_INIT);



    if (!SOC_UNIT_NUM_VALID(unit)) {
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_ILLEGAL_DEVICE_ID, 4, exit);
    }

    /*Relevant properites get*/
    fabric_connect_mode = SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.connect_mode;
    is_dual_mode = SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.dual_pipe_tdm_packet;

    /*
     *Transmmite Data Queue
     */

    /*size configuration*/
    if (fabric_connect_mode == DNX_TMC_FABRIC_CONNECT_MODE_FE || fabric_connect_mode == DNX_TMC_FABRIC_CONNECT_MODE_MULT_STAGE_FE) {
        /*clos cfg - just the first two queues might be used*/
        if (is_dual_mode) {
            dtq_size0 = 0x1ea;
            dtq_size1 = 0x1ea;
            /*must be cfg - even though not used */
            dtq_size2_6=0x2;
        } else {
            dtq_size0 = 0x3e8;
            /*must be cfg - even though not used */
            dtq_size1 = 0x2;
            dtq_size2_6 = 0x2;
        }
    } else {/*MESH cfg*/
        dtq_size0 = 0x91;
        dtq_size1 = 0x91;
        dtq_size2_6 = 0x91;
    }
    reg32_val = 0;
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_SIZE_0_1r, &reg32_val, DTQ_SIZE_0f, dtq_size0);
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_SIZE_0_1r, &reg32_val, DTQ_SIZE_1f, dtq_size1);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 11, exit, WRITE_IPT_TRANSMIT_DATA_QUEUE_SIZE_0_1r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_SIZE_2_3r, &reg32_val, DTQ_SIZE_2f, dtq_size2_6);
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_SIZE_2_3r, &reg32_val, DTQ_SIZE_3f, dtq_size2_6);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 13, exit, WRITE_IPT_TRANSMIT_DATA_QUEUE_SIZE_2_3r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_SIZE_4_5r, &reg32_val, DTQ_SIZE_4f, dtq_size2_6);
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_SIZE_4_5r, &reg32_val, DTQ_SIZE_5f, dtq_size2_6);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 15, exit, WRITE_IPT_TRANSMIT_DATA_QUEUE_SIZE_4_5r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_SIZE_6r, &reg32_val, DTQ_SIZE_6f, dtq_size2_6);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 17, exit, WRITE_IPT_TRANSMIT_DATA_QUEUE_SIZE_6r(unit, reg32_val));

    /*Queue start address*/
    reg32_val = 0;
    fld = 0;
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_START_ADRESS_0_1r, &reg32_val, DTQ_START_0f, fld);
    fld += dtq_size0 + 1;
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_START_ADRESS_0_1r, &reg32_val, DTQ_START_1f, fld);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 21, exit, WRITE_IPT_TRANSMIT_DATA_QUEUE_START_ADRESS_0_1r(unit, reg32_val));

    reg32_val = 0;
    fld += dtq_size1 + 1;
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_START_ADRESS_2_3r, &reg32_val, DTQ_START_2f, fld);
    fld += dtq_size2_6 + 1;
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_START_ADRESS_2_3r, &reg32_val, DTQ_START_3f, fld);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 23, exit, WRITE_IPT_TRANSMIT_DATA_QUEUE_START_ADRESS_2_3r(unit, reg32_val));

    reg32_val = 0;
    fld += dtq_size2_6 + 1;
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_START_ADRESS_4_5r, &reg32_val, DTQ_START_4f, fld);
    fld += dtq_size2_6 + 1;
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_START_ADRESS_4_5r, &reg32_val, DTQ_START_5f, fld);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 25, exit, WRITE_IPT_TRANSMIT_DATA_QUEUE_START_ADRESS_4_5r(unit, reg32_val));

    reg32_val = 0;
    fld += dtq_size2_6 + 1;
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_START_ADRESS_6r, &reg32_val, DTQ_START_6f, fld);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 27, exit, WRITE_IPT_TRANSMIT_DATA_QUEUE_START_ADRESS_6r(unit, reg32_val));

    /*threholds configuration*/
    dtq_th0 = dtq_size0/2;
    dtq_th1 = dtq_size1/2;
    dtq_th2_6 = dtq_size2_6/2;

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_THRESHOLD_0_1r, &reg32_val, DTQ_TH_0f, dtq_th0);
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_THRESHOLD_0_1r, &reg32_val, DTQ_TH_1f, dtq_th1);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 31, exit, WRITE_IPT_TRANSMIT_DATA_QUEUE_THRESHOLD_0_1r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_THRESHOLD_2_3r, &reg32_val, DTQ_TH_2f, dtq_th2_6);
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_THRESHOLD_2_3r, &reg32_val, DTQ_TH_3f, dtq_th2_6);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 33, exit, WRITE_IPT_TRANSMIT_DATA_QUEUE_THRESHOLD_2_3r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_THRESHOLD_4_5r, &reg32_val, DTQ_TH_4f, dtq_th2_6);
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_THRESHOLD_4_5r, &reg32_val, DTQ_TH_5f, dtq_th2_6);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 35, exit, WRITE_IPT_TRANSMIT_DATA_QUEUE_THRESHOLD_4_5r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_TRANSMIT_DATA_QUEUE_THRESHOLD_6r, &reg32_val, DTQ_TH_6f, dtq_th2_6);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 37, exit, WRITE_IPT_TRANSMIT_DATA_QUEUE_THRESHOLD_6r(unit, reg32_val));


    /*
     *Dram Buffer Pointer Queue
     */

    /*size configuration*/
    if (fabric_connect_mode == DNX_TMC_FABRIC_CONNECT_MODE_FE || fabric_connect_mode == DNX_TMC_FABRIC_CONNECT_MODE_MULT_STAGE_FE) {
        /*clos cfg - just the first six queues might be used*/
        /* queue0/1 - local high/low*/
        /* queue2/3 - fabric high/low*/
        /* queue4/5 - GFMC/BFMC */
        dpq_size0_5 = 0x2a3;

        /*must be cfg - even though not used */
        dpq_size6_15 = 0x2;

    } else {/*MESH cfg*/
        /*queue- high/low per destination id*/
        dpq_size0_5 = 0xff;
        dpq_size6_15 = 0xff;
    }

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_0_1r, &reg32_val, DPQ_SIZE_0f, dpq_size0_5);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_0_1r, &reg32_val, DPQ_SIZE_1f, dpq_size0_5);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 101, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_0_1r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_2_3r, &reg32_val, DPQ_SIZE_2f, dpq_size0_5);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_2_3r, &reg32_val, DPQ_SIZE_3f, dpq_size0_5);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 103, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_2_3r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_4_5r, &reg32_val, DPQ_SIZE_4f, dpq_size0_5);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_4_5r, &reg32_val, DPQ_SIZE_5f, dpq_size0_5);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 105, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_4_5r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_6_7r, &reg32_val, DPQ_SIZE_6f, dpq_size6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_6_7r, &reg32_val, DPQ_SIZE_7f, dpq_size6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 107, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_6_7r(unit, reg32_val));


    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_6_7r, &reg32_val, DPQ_SIZE_6f, dpq_size6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_6_7r, &reg32_val, DPQ_SIZE_7f, dpq_size6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 109, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_6_7r(unit, reg32_val));


    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_8_9r, &reg32_val, DPQ_SIZE_8f, dpq_size6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_8_9r, &reg32_val, DPQ_SIZE_9f, dpq_size6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 111, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_8_9r(unit, reg32_val));


    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_10_11r, &reg32_val, DPQ_SIZE_10f, dpq_size6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_10_11r, &reg32_val, DPQ_SIZE_11f, dpq_size6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 113, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_10_11r(unit, reg32_val));


    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_12_13r, &reg32_val, DPQ_SIZE_12f, dpq_size6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_12_13r, &reg32_val, DPQ_SIZE_13f, dpq_size6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_12_13r(unit, reg32_val));


    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_14_15r, &reg32_val, DPQ_SIZE_14f, dpq_size6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_14_15r, &reg32_val, DPQ_SIZE_15f, dpq_size6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 117, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_SIZE_14_15r(unit, reg32_val));

    /*start address cfg*/
    reg32_val = 0;
    fld = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_0_1r, &reg32_val, DPQ_START_0f, fld);
    fld += dpq_size0_5 + 1;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_0_1r, &reg32_val, DPQ_START_1f, fld);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 121, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_0_1r(unit, reg32_val));

    reg32_val = 0;
    fld += dpq_size0_5 + 1;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_2_3r, &reg32_val, DPQ_START_2f, fld);
    fld += dpq_size0_5 + 1;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_2_3r, &reg32_val, DPQ_START_3f, fld);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 123, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_2_3r(unit, reg32_val));

    reg32_val = 0;
    fld += dpq_size0_5 + 1;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_4_5r, &reg32_val, DPQ_START_4f, fld);
    fld += dpq_size0_5 + 1;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_4_5r, &reg32_val, DPQ_START_5f, fld);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 125, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_4_5r(unit, reg32_val));

    reg32_val = 0;
    fld += dpq_size6_15 + 1;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_6_7r, &reg32_val, DPQ_START_6f, fld);
    fld += dpq_size6_15 + 1;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_6_7r, &reg32_val, DPQ_START_7f, fld);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 127, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_6_7r(unit, reg32_val));

    reg32_val = 0;
    fld += dpq_size6_15 + 1;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_8_9r, &reg32_val, DPQ_START_8f, fld);
    fld += dpq_size6_15 + 1;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_8_9r, &reg32_val, DPQ_START_9f, fld);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 129, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_8_9r(unit, reg32_val));

    reg32_val = 0;
    fld += dpq_size6_15 + 1;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_10_11r, &reg32_val, DPQ_START_10f, fld);
    fld += dpq_size6_15 + 1;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_10_11r, &reg32_val, DPQ_START_11f, fld);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 131, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_10_11r(unit, reg32_val));

    reg32_val = 0;
    fld += dpq_size6_15 + 1;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_12_13r, &reg32_val, DPQ_START_12f, fld);
    fld += dpq_size6_15 + 1;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_12_13r, &reg32_val, DPQ_START_13f, fld);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 133, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_12_13r(unit, reg32_val));

    reg32_val = 0;
    fld += dpq_size6_15 + 1;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_14_15r, &reg32_val, DPQ_START_14f, fld);
    fld += dpq_size6_15 + 1;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_14_15r, &reg32_val, DPQ_START_15f, fld);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 135, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_START_ADDRESS_14_15r(unit, reg32_val));

    /*DQCQ thresholds configuration*/
    dpq_dqcq_th0_5 = dpq_size0_5/2;
    dpq_dqcq_th6_15 = dpq_size6_15/2;

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_01r, &reg32_val, DPQ_DQCQ_TH_0f, dpq_dqcq_th0_5);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_01r, &reg32_val, DPQ_DQCQ_TH_1f, dpq_dqcq_th0_5);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 141, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_01r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_23r, &reg32_val, DPQ_DQCQ_TH_2f, dpq_dqcq_th0_5);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_23r, &reg32_val, DPQ_DQCQ_TH_3f, dpq_dqcq_th0_5);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 143, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_23r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_4_5r, &reg32_val, DPQ_DQCQ_TH_4f, dpq_dqcq_th0_5);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_4_5r, &reg32_val, DPQ_DQCQ_TH_5f, dpq_dqcq_th0_5);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 145, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_4_5r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_6_7r, &reg32_val, DPQ_DQCQ_TH_6f, dpq_dqcq_th6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_6_7r, &reg32_val, DPQ_DQCQ_TH_7f, dpq_dqcq_th6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 147, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_6_7r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_8_9r, &reg32_val, DPQ_DQCQ_TH_8f, dpq_dqcq_th6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_8_9r, &reg32_val, DPQ_DQCQ_TH_9f, dpq_dqcq_th6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 149, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_8_9r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_10_11r, &reg32_val, DPQ_DQCQ_TH_10f, dpq_dqcq_th6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_10_11r, &reg32_val, DPQ_DQCQ_TH_11f, dpq_dqcq_th6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 151, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_10_11r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_12_13r, &reg32_val, DPQ_DQCQ_TH_12f, dpq_dqcq_th6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_12_13r, &reg32_val, DPQ_DQCQ_TH_13f, dpq_dqcq_th6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 153, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_12_13r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_14_15r, &reg32_val, DPQ_DQCQ_TH_14f, dpq_dqcq_th6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_14_15r, &reg32_val, DPQ_DQCQ_TH_15f, dpq_dqcq_th6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 155, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_DQCQ_THRESHOLD_14_15r(unit, reg32_val));

    /*EIR credits thresholds configuration*/
    dpq_eir_th0_5 = 2*dpq_size0_5/3;
    dpq_eir_th6_15 = 2*dpq_size6_15/3;

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_0_1r, &reg32_val, DPQ_EIR_CRDT_TH_0f, dpq_eir_th0_5);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_0_1r, &reg32_val, DPQ_EIR_CRDT_TH_1f, dpq_eir_th0_5);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 161, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_0_1r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_2_3r, &reg32_val, DPQ_EIR_CRDT_TH_2f, dpq_eir_th0_5);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_2_3r, &reg32_val, DPQ_EIR_CRDT_TH_3f, dpq_eir_th0_5);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 163, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_2_3r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_4_5r, &reg32_val, DPQ_EIR_CRDT_TH_4f, dpq_eir_th0_5);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_4_5r, &reg32_val, DPQ_EIR_CRDT_TH_5f, dpq_eir_th0_5);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 165, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_4_5r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_6_7r, &reg32_val, DPQ_EIR_CRDT_TH_6f, dpq_eir_th6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_6_7r, &reg32_val, DPQ_EIR_CRDT_TH_7f, dpq_eir_th6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 167, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_6_7r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_8_9r, &reg32_val, DPQ_EIR_CRDT_TH_8f, dpq_eir_th6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_8_9r, &reg32_val, DPQ_EIR_CRDT_TH_9f, dpq_eir_th6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 169, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_8_9r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_10_11r, &reg32_val, DPQ_EIR_CRDT_TH_10f, dpq_eir_th6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_10_11r, &reg32_val, DPQ_EIR_CRDT_TH_11f, dpq_eir_th6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 171, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_10_11r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_12_13r, &reg32_val, DPQ_EIR_CRDT_TH_12f, dpq_eir_th6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_12_13r, &reg32_val, DPQ_EIR_CRDT_TH_13f, dpq_eir_th6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 173, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_12_13r(unit, reg32_val));

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_14_15r, &reg32_val, DPQ_EIR_CRDT_TH_14f, dpq_eir_th6_15);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_14_15r, &reg32_val, DPQ_EIR_CRDT_TH_15f, dpq_eir_th6_15);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 175, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_EIR_CRDT_THRESHOLD_14_15r(unit, reg32_val));

    /*MC thresholds configuration*/
    dpq_mc_th =  2*dpq_size0_5/3;

    reg32_val = 0;
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_MULTICAST_THRESHOLDr, &reg32_val, DPQ_MC_TH_4f, dpq_mc_th);
    soc_reg_field_set(unit, IPT_DRAM_BUFFER_POINTER_QUEUE_MULTICAST_THRESHOLDr, &reg32_val, DPQ_MC_TH_5f, dpq_mc_th);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 200, exit, WRITE_IPT_DRAM_BUFFER_POINTER_QUEUE_MULTICAST_THRESHOLDr(unit, reg32_val));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_ipt_init()", 0, 0);
}

static uint32
  jer2_arad_mgmt_fsrd_init(
      DNX_SAND_IN int             unit
      )
{
    uint32 reg_val,hv_disable, res;
    int blk_ins, quad, global_quad;
    int quad_index, quad_disabled[SOC_JER2_ARAD_NOF_QUADS_IN_FSRD*SOC_DNX_DEFS_MAX(NOF_INSTANCES_FSRD)];

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_HW_ADJUST_FABRIC);



    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_FSRD_SRD_QUAD_CTRLr(unit, 0,  0, &reg_val));

    /* find and indicate disabled quads:
     * run through all FSRD instances, and for each instance run through each quad
     * (nof_quads_per_instance*nof_instances) */
    for (quad=0; quad <(SOC_JER2_ARAD_NOF_QUADS_IN_FSRD*SOC_DNX_DEFS_GET(unit, nof_instances_fsrd)); quad++) {
        /* assume quad is disabled */
        quad_disabled[quad] = TRUE;
        /* check if all ports of quad are disabled. mark quad as enabled otherwise */
        for (quad_index = 0; quad_index < 4; quad_index++) {
            /* if there exists at least one enabled port belonging to quad, quad_disabled=FALSE */
            if (!(SOC_PBMP_MEMBER(SOC_CONTROL(unit)->info.sfi.disabled_bitmap,
                            (FABRIC_LOGICAL_PORT_BASE(unit)+quad*4)+quad_index))) {
                quad_disabled[quad] = FALSE;
                break;
            }
        }
    }

    for(blk_ins=0 ; blk_ins<SOC_DNX_DEFS_GET(unit, nof_instances_fsrd) ; blk_ins++) {
        for(quad=0 ; quad<SOC_JER2_ARAD_NOF_QUADS_IN_FSRD ; quad++) {
            if (quad_disabled[blk_ins*SOC_DNX_DEFS_GET(unit, nof_instances_fsrd)+quad]) {
                continue;
            }
            if (!SOC_IS_ARDON(unit)) {
                soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_POWER_DOWNf, 0);  /*GGG*/
            }
            soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_IDDQf, 0);
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  12,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_FSRD_SRD_QUAD_CTRLr(unit,  blk_ins,  quad,  reg_val));
        }
    }
    sal_usleep(20);

    for(blk_ins=0 ; blk_ins<SOC_DNX_DEFS_GET(unit, nof_instances_fsrd) ; blk_ins++) {
        for(quad=0 ; quad<SOC_JER2_ARAD_NOF_QUADS_IN_FSRD ; quad++) {
            if (quad_disabled[blk_ins*SOC_DNX_DEFS_GET(unit, nof_instances_fsrd)+quad]) {
                continue;
            }
            soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_RSTB_HWf, 1);
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  14,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_FSRD_SRD_QUAD_CTRLr(unit,  blk_ins,  quad,  reg_val));
        }
    }
    sal_usleep(20);

    for(blk_ins=0 ; blk_ins<SOC_DNX_DEFS_GET(unit, nof_instances_fsrd) ; blk_ins++) {
        for(quad=0 ; quad<SOC_JER2_ARAD_NOF_QUADS_IN_FSRD ; quad++) {
            if (quad_disabled[blk_ins*SOC_DNX_DEFS_GET(unit, nof_instances_fsrd)+quad]) {
                continue;
            }
            soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_MDIO_REGSf, 1);
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  16,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_FSRD_SRD_QUAD_CTRLr(unit,  blk_ins,  quad,  reg_val));
        }
    }
    sal_usleep(20);

    for(blk_ins=0 ; blk_ins<SOC_DNX_DEFS_GET(unit, nof_instances_fsrd) ; blk_ins++) {
        for(quad=0 ; quad<SOC_JER2_ARAD_NOF_QUADS_IN_FSRD ; quad++) {
            if (quad_disabled[blk_ins*SOC_DNX_DEFS_GET(unit, nof_instances_fsrd)+quad]) {
                continue;
            }
            soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_RSTB_PLLf, 1);
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  18,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_FSRD_SRD_QUAD_CTRLr(unit,  blk_ins,  quad,  reg_val));
        }
    }
    sal_usleep(20);

    for(blk_ins=0 ; blk_ins<SOC_DNX_DEFS_GET(unit, nof_instances_fsrd) ; blk_ins++) {
        for(quad=0 ; quad<SOC_JER2_ARAD_NOF_QUADS_IN_FSRD ; quad++) {
            if (quad_disabled[blk_ins*SOC_DNX_DEFS_GET(unit, nof_instances_fsrd)+quad]) {
                continue;
            }
            soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_RSTB_FIFOf, 1);
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_FSRD_SRD_QUAD_CTRLr(unit,  blk_ins,  quad,  reg_val));
        }
    }
    sal_usleep(20);

    for(blk_ins=0 ; blk_ins<SOC_DNX_DEFS_GET(unit, nof_instances_fsrd) ; blk_ins++) {
        for(quad=0 ; quad<SOC_JER2_ARAD_NOF_QUADS_IN_FSRD ; quad++) {
            if (quad_disabled[blk_ins*SOC_DNX_DEFS_GET(unit, nof_instances_fsrd)+quad]) {
                continue;
            }
            global_quad = blk_ins*SOC_JER2_ARAD_NOF_QUADS_IN_FSRD + quad;
            hv_disable = soc_property_suffix_num_get(unit, global_quad, spn_SRD_TX_DRV_HV_DISABLE, "quad", 0);
            soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_TX_DRV_HV_DISABLEf, hv_disable);
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  22,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_FSRD_SRD_QUAD_CTRLr(unit,  blk_ins,  quad,  reg_val));
        }
    }


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_fsrd_init()", 0, 0);
}

static uint32
jer2_arad_mgmt_init_header_map(
   DNX_SAND_IN int unit
   )
{
  uint32
    line,
    res,
  /* Header map addr */
    addr_idx,
  /* Header map index fields {*/
     /* hmif_pph_learn_ext_asd_type, */ /* Unused */
    hmif_eei_ext_exists,
    hmif_eep_ext_exists,
    hmif_ftmh_ext_msbs,
    hmif_fwd_code,
  /* } Header map index fields */
  /* Header map fields { */
    hmf_out_lif_src,
    hmf_vsi_or_vrf_src,
    hmf_in_lif_src,
    hmf_eei_valid,
    hmf_eei_src;
  /* hmf_pph_learn_ext_asd_type; */ /* Unused */
  /* } Header map fields */

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_IPT_INIT);

  /* FIMXE: Leave unuseds? */
  for (addr_idx = 0; addr_idx < JER2_ARAD_INIT_HM_NOF_ADDRS; addr_idx++) {
    /* hmif_pph_learn_ext_asd_type_bits = 0; */ /* Unused */
    hmif_eei_ext_exists = 0;
    hmif_eep_ext_exists = 0;
    hmif_ftmh_ext_msbs = 0;
    hmif_fwd_code = 0;

    /* Get the index fields from the index */
    /* SHR_BITCOPY_RANGE(hmif_pph_learn_ext_asd_type_bits,
                        0, addr_idx, JER2_ARAD_INIT_HMI_LEARN_EXT_VALID_LSB, JER2_ARAD_INIT_HMI_LEARN_EXT_VALID_NOF_BITS); */ /* Unused */
    SHR_BITCOPY_RANGE(&hmif_eei_ext_exists,
                      0, &addr_idx, JER2_ARAD_INIT_HMI_EEI_EXT_EXISTS_LSB, JER2_ARAD_INIT_HMI_EEI_EXT_EXISTS_NOF_BITS);
    SHR_BITCOPY_RANGE(&hmif_eep_ext_exists,
                      0, &addr_idx, JER2_ARAD_INIT_HMI_EEP_EXT_EXISTS_LSB, JER2_ARAD_INIT_HMI_EEP_EXT_EXISTS_NOF_BITS);
    SHR_BITCOPY_RANGE(&hmif_ftmh_ext_msbs,
                      0, &addr_idx, JER2_ARAD_INIT_HMI_FTMH_EXT_MSBS_LSB, JER2_ARAD_INIT_HMI_FTMH_EXT_MSBS_NOF_BITS);
    SHR_BITCOPY_RANGE(&hmif_fwd_code,
                      0, &addr_idx, JER2_ARAD_INIT_HMI_FWD_CODE_LSB, JER2_ARAD_INIT_HMI_FWD_CODE_NOF_BITS);

      hmf_out_lif_src      = 0;
      hmf_vsi_or_vrf_src   = 0;
      hmf_in_lif_src       = 0;
      hmf_eei_valid        = 0;
      hmf_eei_src          = 0;

    if (hmif_fwd_code == DNX_TMC_PKT_FRWRD_TYPE_BRIDGE) {
      if (hmif_eep_ext_exists) {
        hmf_out_lif_src       = JER2_ARAD_INIT_HM_TAKEN_FROM_EEP_EXT;
      } else {
        hmf_out_lif_src       = (hmif_ftmh_ext_msbs == 0x01) ?
           JER2_ARAD_INIT_HM_TAKEN_FROM_2b00_FTMH_EXT_13_0 : 0;
      }
      hmf_vsi_or_vrf_src      = JER2_ARAD_INIT_HM_TAKEN_FROM_PPH_SYSTEM_VSI;
      hmf_in_lif_src          = JER2_ARAD_INIT_HM_TAKEN_FROM_LEARN_EXT;
      hmf_eei_valid           = 0;
      hmf_eei_src             = 0;
    } else if (hmif_fwd_code == DNX_TMC_PKT_FRWRD_TYPE_IPV4_UC) {
      hmf_out_lif_src         = JER2_ARAD_INIT_HM_TAKEN_FROM_PPH_SYSTEM_VSI;
      hmf_vsi_or_vrf_src      = 0;
      hmf_in_lif_src          = 0;
      hmf_eei_valid           = hmif_eep_ext_exists;
      hmf_eei_src             = JER2_ARAD_INIT_HM_TAKEN_FROM_EEP_EXT;
    } else if (hmif_fwd_code == DNX_TMC_PKT_FRWRD_TYPE_IPV4_MC) {
      hmf_out_lif_src         = JER2_ARAD_INIT_HM_TAKEN_FROM_EEP_EXT;
      hmf_vsi_or_vrf_src      = JER2_ARAD_INIT_HM_TAKEN_FROM_EEP_EXT;
      hmf_in_lif_src          = JER2_ARAD_INIT_HM_TAKEN_FROM_PPH_SYSTEM_VSI;
      hmf_eei_valid           = 0;
      hmf_eei_src             = 0;
    } else if (hmif_fwd_code == DNX_TMC_PKT_FRWRD_TYPE_MPLS) {
      hmf_out_lif_src         = JER2_ARAD_INIT_HM_TAKEN_FROM_EEP_EXT;
      hmf_vsi_or_vrf_src      = JER2_ARAD_INIT_HM_TAKEN_FROM_PPH_SYSTEM_VSI;
      hmf_in_lif_src          = 0;
      hmf_eei_valid           = hmif_eei_ext_exists;
      hmf_eei_src             = hmif_eei_ext_exists ? JER2_ARAD_INIT_HM_TAKEN_FROM_EEI_EXT : 0;
    } else if (hmif_fwd_code == DNX_TMC_PKT_FRWRD_TYPE_BRIDGE_AFTER_TERM) {
      if (hmif_eep_ext_exists) {
        hmf_out_lif_src   = JER2_ARAD_INIT_HM_TAKEN_FROM_EEP_EXT;
      } else {
        switch (hmif_ftmh_ext_msbs) {
        case 0x01:
          hmf_out_lif_src  = JER2_ARAD_INIT_HM_TAKEN_FROM_2b01_FTMH_EXT_13_0;
          break;
        case 0x02:
          hmf_out_lif_src  = JER2_ARAD_INIT_HM_TAKEN_FROM_2b10_FTMH_EXT_13_0;
          break;
        default:
          hmf_out_lif_src  = JER2_ARAD_INIT_HM_TAKEN_FROM_2b00_FTMH_EXT_13_0;
          break;
        }
      }

      hmf_vsi_or_vrf_src       = JER2_ARAD_INIT_HM_TAKEN_FROM_PPH_SYSTEM_VSI;
      hmf_in_lif_src           = JER2_ARAD_INIT_HM_TAKEN_FROM_LEARN_EXT;
      hmf_eei_valid            = hmif_eei_ext_exists;
      hmf_eei_src              = hmif_eei_ext_exists ? JER2_ARAD_INIT_HM_TAKEN_FROM_EEI_EXT : 0;
    } else if (hmif_fwd_code == DNX_TMC_PKT_FRWRD_TYPE_TM) {
      switch (hmif_ftmh_ext_msbs) {
        case 0x01:
          hmf_out_lif_src  = JER2_ARAD_INIT_HM_TAKEN_FROM_2b01_FTMH_EXT_13_0;
          break;
        case 0x02:
          hmf_out_lif_src  = JER2_ARAD_INIT_HM_TAKEN_FROM_2b10_FTMH_EXT_13_0;
          break;
        default:
          hmf_out_lif_src  = JER2_ARAD_INIT_HM_TAKEN_FROM_2b00_FTMH_EXT_13_0;
          break;
        }
      hmf_vsi_or_vrf_src   = 0;
      hmf_in_lif_src       = 0;
      hmf_eei_valid        = 0;
      hmf_eei_src          = 0;
    } else {
      hmf_out_lif_src      = 0;
      hmf_vsi_or_vrf_src   = 0;
      hmf_in_lif_src       = 0;
      hmf_eei_valid        = 0;
      hmf_eei_src          = 0;
    }

    line = 0;
    res = READ_EPNI_HEADER_MAPm(unit, MEM_BLOCK_ANY, addr_idx, &line);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 400000, exit);

    soc_mem_field32_set(unit, EPNI_HEADER_MAPm, &line, OUT_LIF_SRCf,    hmf_out_lif_src);
    soc_mem_field32_set(unit, EPNI_HEADER_MAPm, &line, VSI_OR_VRF_SRCf, hmf_vsi_or_vrf_src);
    soc_mem_field32_set(unit, EPNI_HEADER_MAPm, &line, IN_LIF_SRCf,     hmf_in_lif_src);
    soc_mem_field32_set(unit, EPNI_HEADER_MAPm, &line, EEI_VALIDf,      hmf_eei_valid);
    soc_mem_field32_set(unit, EPNI_HEADER_MAPm, &line, EEI_SRCf,        hmf_eei_src);
    res = WRITE_EPNI_HEADER_MAPm(unit, MEM_BLOCK_ANY, addr_idx, &line);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 500000, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_init_header_map()", 0, 0);
}

static uint32
  jer2_arad_mgmt_init_fabric(
    DNX_SAND_IN int             unit,
    DNX_SAND_IN JER2_ARAD_INIT_FABRIC*  init_fabric
  )
{
  uint32
    fld_val = 0,
    res = 0;

  JER2_ARAD_FABRIC_CELL_FORMAT cell_format;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_HW_ADJUST_FABRIC);

  DNX_SAND_CHECK_NULL_INPUT(init_fabric);

  /*
   * Fabric Serdes - get out of reset
   */
  if (!SOC_IS_ARDON(unit)) {
      res = jer2_arad_mgmt_fsrd_init(unit);
      DNX_SAND_CHECK_FUNC_RESULT(res, 13, exit);
  }

  /*
   *  EGQ - maximum fragment number
   *  Depends on fixed/variable cell size configuration
   */
  fld_val = JER2_ARAD_MGMT_INIT_EGQ_MAX_FRG_VAR;
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  15,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_MAXIMUM_AND_MINIMUM_PACKET_SIZEr, SOC_CORE_ALL, 0, MAX_FRG_NUMf,  fld_val));

  res = jer2_arad_fabric_connect_mode_set_unsafe(unit, init_fabric->connect_mode);
  DNX_SAND_CHECK_FUNC_RESULT(res, 17, exit);

  /*
   *  Cell Format
   */
  jer2_arad_JER2_ARAD_FABRIC_CELL_FORMAT_clear(&cell_format);
  cell_format.segmentation_enable = init_fabric->segmentation_enable;
  res = jer2_arad_fabric_cell_format_set_unsafe(
          unit,
          &cell_format
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 19, exit);
  /*
   * 128 Bytes configurations
   */

  fld_val = DNX_SAND_BOOL2NUM(init_fabric->is_fe600);
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  31,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ECI_GLOBALEr, REG_PORT_ANY, 0, GLBL_VSC_128_MODEf,  fld_val));
  /*if vsc256 enable fabric interleaving*/
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  36,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_BRDC_FMAC_CNTRL_INTRLVD_MODE_REGr(unit, init_fabric->is_fe600 ? 0 : 0xf));

  if(init_fabric->is_128_in_system) {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  33,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ECI_GLOBALEr, REG_PORT_ANY, 0, SYS_CONFIG_1f,  0x1));
  } else if (init_fabric->is_dual_mode_in_system) {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  34,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ECI_GLOBALEr, REG_PORT_ANY, 0, SYS_CONFIG_1f,  0x2));
  } else {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  35,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ECI_GLOBALEr, REG_PORT_ANY, 0, SYS_CONFIG_1f,  0x0));
  }

  if(init_fabric->is_128_in_system) {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  50,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDR_REG_016Ar, REG_PORT_ANY, 0, FIELD_0_0f,  0x0));
  } else {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  51,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDR_REG_016Ar, REG_PORT_ANY, 0, FIELD_0_0f,  0x1));
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  52,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDR_REG_016Ar, REG_PORT_ANY, 0, FIELD_4_22f,  0x1800));
  }

  /*soc_petrab mode - ftmh*/
  if (SOC_DNX_CONFIG(unit)->tm.is_petrab_in_system) {
      
      DNXC_LEGACY_FIXME_ASSERT;

      #define JER2_ARAD_PP_SYSTEM_HEADERS_MODE_PETRAB -1
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  37,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ECI_GLOBALFr, REG_PORT_ANY, 0, SYSTEM_HEADERS_MODEf,  JER2_ARAD_PP_SYSTEM_HEADERS_MODE_PETRAB));
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  38,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDT_MULTICAST_ID_MASKr, REG_PORT_ANY, 0, MULTICAST_ID_MASKf,  0x3fff));

      /* Configure the header map */
      res = jer2_arad_mgmt_init_header_map(unit);
      DNX_SAND_CHECK_FUNC_RESULT(res, 54264, exit);
  }

  /*
   * configure llfc rx threshold according to pipe mode
   */
  fld_val = init_fabric->dual_pipe_tdm_packet ? JER2_ARAD_INIT_LLFC_DUAL_PIPE_TH : JER2_ARAD_INIT_LLFC_SINGLE_PIPE_TH;
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  37,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDR_LINK_LEVEL_FLOW_CONTROLr, REG_PORT_ANY, 0, LNK_LVL_FC_THf,  fld_val));

  /*
   * Set the Dual mode
   */
  fld_val = init_fabric->dual_pipe_tdm_packet;
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  32,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ECI_GLOBALEr, REG_PORT_ANY, 0, PARALLEL_DATA_PATHf,  fld_val));

  {
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  241,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_REG_0101r, REG_PORT_ANY, 0, FIELD_0_0f,  0));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  242,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDR_FDR_ENABLERS_REGISTER_2r, REG_PORT_ANY, 0, FIELD_1_1f,  1));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  244,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDT_FDT_ENABLER_REGISTERr, REG_PORT_ANY, 0, FIELD_12_12f,  1));
  }



exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_init_fabric()", res, 0);
}

/*
 *  Prepare for NIF configuration;
 *  some settings must be configured at early stages,
 *  e.g. the clock configuration should be prior
 *  to SerDes trimming
 */
uint32
  jer2_arad_mgmt_init_nif_prep(
    DNX_SAND_IN int     unit,
    DNX_SAND_IN  uint8    silent
  )
{
  
  DNXC_LEGACY_FIXME_ASSERT;
  return -1;
}


/*********************************************************************
*     Initialize a sub-set of the HW interfaces of the device.
*     The function might be called more than once, each time
*     with different fields, indicated to be written to the
*     device
*     Details: in the H file. (search for prototype)
*********************************************************************/
static
  uint32
    jer2_arad_mgmt_hw_interfaces_set_unsafe(
      DNX_SAND_IN  int                unit,
      DNX_SAND_IN  JER2_ARAD_MGMT_INIT*          init,
      DNX_SAND_IN  uint8                silent
    )
{
  uint32
    res,
    stage_id = 0,
    stage_internal_id = 0;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_HW_INTERFACES_SET_UNSAFE);
  DNX_SAND_PCID_LITE_SKIP(unit);
  (void)stage_internal_id;
  (void)stage_id;
  DNX_SAND_CHECK_NULL_INPUT(init);



  if (init->fabric.enable) {
    JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_1_ADVANCE("Fabric");
    res = jer2_arad_mgmt_init_fabric(
            unit,
            &(init->fabric)
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_hw_interfaces_set_unsafe()", 0, 0);
}

/*********************************************************************
*     Set bubble configuration for IHP & EHP
*********************************************************************/
static
  uint32
    jer2_arad_bubble_configuration_set_unsafe(
      DNX_SAND_IN  int                unit
    )
{
  uint32
    res, conf_reg_val;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);



  if(soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "bubble_egr", 1) == 1) {

   /*
    * Set EHP bubble configuration
    */

    res = READ_EGQ_EHP_BUBBLE_CONFIGURATIONr(unit, REG_PORT_ANY, &conf_reg_val);
    DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    /* Enable constant bubble generation every 32*BubbleDelay clocks.*/
    soc_reg_field_set(unit, EGQ_EHP_BUBBLE_CONFIGURATIONr, &conf_reg_val, EHP_CONST_BUBBLE_ENf, 1);

    /* If set then the bubble is generated only instead of serving the CPU interface. */
    soc_reg_field_set(unit, EGQ_EHP_BUBBLE_CONFIGURATIONr, &conf_reg_val, EHP_RQP_BUBBLE_REQ_ENf, 1);

    /* The minimum delay between the bubble request to the bubble */
    soc_reg_field_set(unit, EGQ_EHP_BUBBLE_CONFIGURATIONr, &conf_reg_val, EHP_BUBBLE_DELAYf, 31);

    res = WRITE_EGQ_EHP_BUBBLE_CONFIGURATIONr(unit, REG_PORT_ANY, conf_reg_val);
    DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);


  /*
   * Set FQP bubble configuration
   */

    res = READ_EGQ_FQP_BUBBLE_CONFIGURATIONr(unit, REG_PORT_ANY, &conf_reg_val);
    DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    /* Enable constant bubble generation every 32*BubbleDelay clocks.*/
    soc_reg_field_set(unit, EGQ_FQP_BUBBLE_CONFIGURATIONr, &conf_reg_val, FQP_CONST_BUBBLE_ENf, 1);

    /* The minimum delay between the bubble request to the bubble */
    soc_reg_field_set(unit, EGQ_FQP_BUBBLE_CONFIGURATIONr, &conf_reg_val, FQP_BUBBLE_DELAYf, 31);

    res = WRITE_EGQ_FQP_BUBBLE_CONFIGURATIONr(unit, REG_PORT_ANY, conf_reg_val);
    DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  }

    if(soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "bubble_ing", 1) == 1) {

  /*
   * Set IHP bubble configuration
   */

    /* limit packet rate by bubbles injected every SyncCounter clock cycles. */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_IHP_ENABLERSr, SOC_CORE_ALL, 0, FORCE_BUBBLESf,  1));

    /* every SyncCounter number of clocks a bubble will be inserted to the IHP pipe */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_SYNC_COUNTERr, SOC_CORE_ALL, 0, SYNC_COUNTERf,  0x3ff/* should be examined*/));
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "jer2_arad_bubble_configuration_set_unsafe()", 0, 0);
}

/*********************************************************************
*     Initialize a sub-set of the HW interfaces of the device.
*     The function might be called more than once, each time
*     with different fields, indicated to be written to the
*     device
*     Details: in the H file. (search for prototype)
*********************************************************************/
static
  uint32
    jer2_arad_mgmt_hw_interfaces_verify(
      DNX_SAND_IN  int                 unit,
      DNX_SAND_IN  JER2_ARAD_MGMT_INIT           *init
    )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_HW_INTERFACES_VERIFY);

  DNX_SAND_CHECK_NULL_INPUT(init);
  DNX_SAND_MAGIC_NUM_VERIFY(init);

  if(init->fabric.enable)
  {
    res = jer2_arad_fabric_connect_mode_verify(
            unit,
            init->fabric.connect_mode
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 88, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_hw_interfaces_verify()", 0, 0);
}


/*********************************************************************
*     Initialize of Arad internal blocks.
*     Details: in the H file. (search for prototype)
*********************************************************************/

static uint32
  jer2_arad_mgmt_blocks_init_unsafe(
    DNX_SAND_IN  int                 unit
  )
{
    uint32
        reg32_val,
        res,
        fld_val;
    soc_reg_above_64_val_t
        data_above_64;
    int
        rv = 0;
    soc_reg_t
        idr_mishandling_reg;
    soc_field_t
        idr_mishandling_field;


    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_BLOCKS_INIT_UNSAFE);

    rv = soc_dnx_device_reset(unit, SOC_DNX_RESET_MODE_BLOCKS_RESET, SOC_DNX_RESET_ACTION_OUT_RESET);
    if (rv != SOC_E_NONE) {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_MGMT_EGQ_INIT_FAILS_ERR, 35, exit);
    }

    /*
     * Enable the CDC Bond option in CLP Port: default disabled
     * The setting must be done once the NBI is out-of-reset but not the CLP Ports
     */

        DNX_SAND_SOC_IF_ERROR_RETURN(res, 15, exit, READ_ECI_BLOCKS_SOFT_RESETr(unit, data_above_64));
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_RESETr, data_above_64, CLP_0_RESETf, 1);
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 16, exit, WRITE_ECI_BLOCKS_SOFT_RESETr(unit, data_above_64));

        DNX_SAND_SOC_IF_ERROR_RETURN(res, 17, exit, READ_ECI_BLOCKS_SOFT_RESETr(unit, data_above_64));
        soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_RESETr, data_above_64, CLP_1_RESETf, 1);
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 18, exit, WRITE_ECI_BLOCKS_SOFT_RESETr(unit, data_above_64));
    if (!SOC_IS_ARDON(unit)) {
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  24,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, NBI_NIF_PORTS_CFGr, REG_PORT_ANY,  0, CLP_N_OTP_PORT_BOND_OPTIONf,  0x3));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  25,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, NBI_NIF_PORTS_CFGr, REG_PORT_ANY,  1, CLP_N_OTP_PORT_BOND_OPTIONf,  0x3));
    }
        SOC_REG_ABOVE_64_CLEAR(data_above_64);
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_ECI_BLOCKS_SOFT_RESETr(unit, data_above_64));

    if (!SOC_IS_ARDON(unit)) {
        /* HW reset of Port MLD */
        /* PORT_MLD_CTRL_REG.CLP0 */
        reg32_val = 0;
        soc_reg_field_set(unit, PORT_MLD_CTRL_REGr, &reg32_val, RST_B_HWf, 1);
        soc_reg_field_set(unit, PORT_MLD_CTRL_REGr, &reg32_val, RST_B_MDIOREGSf, 1);
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 22, exit, WRITE_PORT_MLD_CTRL_REGr(unit, SOC_BLOCK_PORT(unit, CLP_BLOCK(unit, 0)) /*###CLP0*/, reg32_val));

        /* PORT_MLD_CTRL_REG.CLP1 */
        reg32_val = 0;
        soc_reg_field_set(unit, PORT_MLD_CTRL_REGr, &reg32_val, RST_B_HWf, 1);
        soc_reg_field_set(unit, PORT_MLD_CTRL_REGr, &reg32_val, RST_B_MDIOREGSf, 1);
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 24, exit, WRITE_PORT_MLD_CTRL_REGr(unit, SOC_BLOCK_PORT(unit, CLP_BLOCK(unit, 1))  /*###CLP1*/, reg32_val));

        /* initalizing gaps between ports info for led processor use */
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  26,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_REG_2029700r(unit,  SOC_BLOCK_PORT(unit, CLP_BLOCK(unit, 0))/*###CLP0*/,  0x6));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  27,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_REG_2029700r(unit,  SOC_BLOCK_PORT(unit, CLP_BLOCK(unit, 1))/*###CLP1*/,  0x6));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  28,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_REG_2029700r(unit,  SOC_BLOCK_PORT(unit, XLP_BLOCK(unit, 0))/*###XLP0*/,  0x6));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  29,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_REG_2029700r(unit,  SOC_BLOCK_PORT(unit, XLP_BLOCK(unit, 1))/*###XLP1*/,  0x6));

       /* Enable the ECC in the memories below */
       /* port_port_ecc_control.cdc_rxfifo_mem_en = 1'h1 ; */
       fld_val = 0x1;
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  34,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, CLP_BLOCK(unit, 0)),  0, CDC_RXFIFO_MEM_ENf,  fld_val));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  35,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, CLP_BLOCK(unit, 1)),  0, CDC_RXFIFO_MEM_ENf,  fld_val));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  36,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, XLP_BLOCK(unit, 0)),  0, CDC_RXFIFO_MEM_ENf,  fld_val));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  37,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, XLP_BLOCK(unit, 1)),  0, CDC_RXFIFO_MEM_ENf,  fld_val));
       /* port_port_ecc_control.cdc_txfifo_mem_en = 1'h1 ; */
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  38,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, CLP_BLOCK(unit, 0)),  0, CDC_TXFIFO_MEM_ENf,  fld_val));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  39,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, CLP_BLOCK(unit, 1)),  0, CDC_TXFIFO_MEM_ENf,  fld_val));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, XLP_BLOCK(unit, 0)),  0, CDC_TXFIFO_MEM_ENf,  fld_val));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  41,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, XLP_BLOCK(unit, 1)),  0, CDC_TXFIFO_MEM_ENf,  fld_val));
       /* port_port_ecc_control.mib_rsc_mem_en    = 1'h1 ; */
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  42,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, CLP_BLOCK(unit, 0)),  0, MIB_RSC_MEM_ENf,  fld_val));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  43,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, CLP_BLOCK(unit, 1)),  0, MIB_RSC_MEM_ENf,  fld_val));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  44,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, XLP_BLOCK(unit, 0)),  0, MIB_RSC_MEM_ENf,  fld_val));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  45,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, XLP_BLOCK(unit, 1)),  0, MIB_RSC_MEM_ENf,  fld_val));
       /*port_port_ecc_control.mib_tsc_mem_en    = 1'h1 ;*/
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  46,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, CLP_BLOCK(unit, 0)),  0, MIB_TSC_MEM_ENf,  fld_val));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  47,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, CLP_BLOCK(unit, 1)),  0, MIB_TSC_MEM_ENf,  fld_val));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  48,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, XLP_BLOCK(unit, 0)),  0, MIB_TSC_MEM_ENf,  fld_val));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  49,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, XLP_BLOCK(unit, 1)),  0, MIB_TSC_MEM_ENf,  fld_val));
       /*port_port_ecc_control.txfifo_mem_en     = 1'h1 ;*/
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  50,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, CLP_BLOCK(unit, 0)),  0, TXFIFO_MEM_ENf,  fld_val));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  51,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, CLP_BLOCK(unit, 1)),  0, TXFIFO_MEM_ENf,  fld_val));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  52,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, XLP_BLOCK(unit, 0)),  0, TXFIFO_MEM_ENf,  fld_val));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  53,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PORT_ECC_CONTROLr,  SOC_BLOCK_PORT(unit, XLP_BLOCK(unit, 1)),  0, TXFIFO_MEM_ENf,  fld_val));
    }

   if (SOC_IS_ARAD_B0_AND_ABOVE(unit)) {
       /* IDR-Mis-handling of flow control from Chunk Fifo to RCT*/
       idr_mishandling_reg = SOC_IS_ARADPLUS(unit)? IDR_COMPATIBILITY_REGISTERr: IDR_SPARE_REGISTER_2r; /* Different register name in Arad+ */
       idr_mishandling_field = SOC_IS_ARADPLUS(unit)? FIELD_6_6f: SYS_CONFIG_1f; /* Different field name in Arad+ */
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  60,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg32_get(unit, idr_mishandling_reg, REG_PORT_ANY,  0, &reg32_val));
       fld_val = 0x1;
       soc_reg_field_set(unit, idr_mishandling_reg, &reg32_val, idr_mishandling_field, fld_val);
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  70,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg32_set(unit, idr_mishandling_reg, REG_PORT_ANY,  0,  reg32_val));
   }

  if (SOC_IS_ARDON(unit)) {
      /* BDB size at ATMF changed from 1.5M to 1M */
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 80, exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_field32_modify(unit, IQM_BDB_CONFIGURATIONr, REG_PORT_ANY, BDB_LIST_SIZEf, 7));
  }


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_blocks_init_unsafe()", 0, 0);
}

/*********************************************************************
*     Initialize Stacking application.
*********************************************************************/

uint32 jer2_arad_mgmt_stk_init(
    DNX_SAND_IN  int                                unit,
    DNX_SAND_IN  JER2_ARAD_MGMT_INIT               *init)
{
    uint32
        res,
        reg_val,
        i,
        port_i,
        base_q_pair,
        flags;
    uint64
        reg_val64;
    soc_pbmp_t
        pbmp;
    JER2_ARAD_EGQ_PPCT_TBL_DATA
        ppct_tbl_data;
    int     core;
    uint32  tm_port;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_IHB_LBP_GENERAL_CONFIG_0r(unit, SOC_CORE_ALL, &reg_val64));
    soc_reg64_field32_set(unit, IHB_LBP_GENERAL_CONFIG_0r, &reg_val64, TM_DOMAINf, init->ports.tm_domain);
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_IHB_LBP_GENERAL_CONFIG_0r(unit, SOC_CORE_ALL,  reg_val64));

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_IRR_STATIC_CONFIGURATIONr(unit, &reg_val));
    JER2_ARAD_FLD_TO_REG(IRR_STATIC_CONFIGURATIONr, USE_STACK_RESOLVEf, 0x1, reg_val, 50, exit);
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  60,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_IRR_STATIC_CONFIGURATIONr(unit,  reg_val));

    DNX_SAND_SOC_IF_ERROR_RETURN(res, 70, exit, dnx_port_sw_db_valid_ports_get(unit, 0, &pbmp));

    SOC_PBMP_ITER(pbmp,port_i) {

        res = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.peer_tm_domain.get(unit, port_i, &ppct_tbl_data.peer_tm_domain_id);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 72, exit);

        if (ppct_tbl_data.peer_tm_domain_id == 0xffffffff) {
            continue;
        }

        res = dnx_port_sw_db_flags_get(unit, port_i, &flags);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 73, exit);

        if (DNX_PORT_IS_ELK_INTERFACE(flags) || DNX_PORT_IS_STAT_INTERFACE(flags)) {
            continue;
        }

        res = dnx_port_sw_db_local_to_tm_port_get(unit, port_i, &tm_port, &core);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 140, exit);

        /* Retreive base_q_pair */
        res = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.base_q_pair.get(unit, port_i, &base_q_pair);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 74, exit);

        res = jer2_arad_egq_ppct_tbl_get_unsafe(unit, core, base_q_pair, &ppct_tbl_data);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 75, exit);

        ppct_tbl_data.is_stacking_port =  0x1;

        res = jer2_arad_egq_ppct_tbl_set_unsafe(unit, core, base_q_pair, &ppct_tbl_data);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 80, exit);
    }

    /* Set Stack Fec Resolve table with default mapping */
    for (i=0; i < 64 ; i++) {

        res = jer2_arad_ipq_stack_fec_map_stack_lag_set_unsafe(unit, i /*TM domain*/, JER2_ARAD_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_ALL, i /* LAG stack */);
        DNX_SAND_CHECK_FUNC_RESULT(res, 90, exit);
    }

    /* Enable stamping of unicast destination in FTMH DSP extention */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  100,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_IRE_STATIC_CONFIGURATIONr(unit, &reg_val));
    soc_reg_field_set(unit, IRE_STATIC_CONFIGURATIONr, &reg_val, STAMP_UC_DESTINATION_IN_FTMHf, 0x1);
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  110,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_IRE_STATIC_CONFIGURATIONr(unit, reg_val));

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_stk_init()", 0, 0);
}

/*********************************************************************
*     Initialize Stacking application.
*********************************************************************/

static uint32 jer2_arad_mgmt_system_red_init(
    DNX_SAND_IN  int                                unit,
    DNX_SAND_IN  JER2_ARAD_MGMT_INIT               *init)
{
    uint32
        res,
        val32 = 0;

    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    if (SOC_IS_QAX(unit)) {
        DNX_SAND_SET_ERROR_MSG((_BSL_DNX_SAND_MSG("access to registers should be fixed for JER2_QAX at places we used _REG(32|64) access routines")));
    }


    DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IPS_IPS_GENERAL_CONFIGURATIONSr_REG32(unit, REG_PORT_ANY, &val32));
    soc_reg_field_set(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, &val32, ENABLE_SYSTEM_REDf, 0x1);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IPS_IPS_GENERAL_CONFIGURATIONSr_REG32(unit, REG_PORT_ANY, val32));

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_system_red_init()", 0, 0);
}


#define BASE_QUEUE 0x17ff0 /* first queue to use for workaround */
#define NOF_QUEUES_TO_BLOCK 13 /* number of queues to use for workaround */
#define NOF_PACKETS_PER_QUEUE 32
#define NUM_OF_REP (NOF_QUEUES_TO_BLOCK * NOF_PACKETS_PER_QUEUE)
#define TEMP_MC_ID 9000 /* multicast ID used temporarily in the workaround */
#define TEMP_FWD_ACTION_PROFILE 0x2a

uint32
  jer2_arad_iqm_workaround(
    DNX_SAND_IN  int                 unit
  )
{
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  uint32 res;
  uint32 i;
  uint32 val32 = 0, intr_val, intr_mask;
  uint64 val64;
  DNX_TMC_MULT_ING_ENTRY mc_group;
  DNX_TMC_ERROR mc_err;
  DNX_TMC_MULT_ING_ENTRY *mc_members = NULL;
  uint32 entry[5] = {0}; /* buffer for memory access */
  soc_reg_above_64_val_t reg_above_64_val = {0};
  uint32 nof_dram_buffs = 0;
  int should_disable_dynamic_mem_changes = 0;
  JER2_ARAD_INIT_DBUFFS_BDRY dbuffs_bdries = {{0}};
  uint32 bu_IHP_PINFO_LLR[3];
  uint32 bu_IHB_FWD_ACT_PROFILE[5];
  uint32 bu_IPS_CRWDTH[1];
  JER2_ARAD_MGMT_INIT *init = &(SOC_DNX_CONFIG(unit)->jer2_arad->init);
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  if (SOC_IS_ARAD_B0_AND_ABOVE(unit)) {
    goto exit;
  }

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IQM_INTERRUPT_REGISTERr(unit, REG_PORT_ANY, &intr_val));
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, READ_IQM_INTERRUPT_MASK_REGISTERr(unit, REG_PORT_ANY, &intr_mask));
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, WRITE_IQM_INTERRUPT_MASK_REGISTERr(unit, REG_PORT_ANY, 0));

  /* open the ingress MC group */
  DNX_TMC_MULT_ING_ENTRY_clear(&mc_group);
  res = dnx_mult_ing_group_open(unit, TEMP_MC_ID, &mc_group, 0, &mc_err);
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  if (mc_err) {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 50, exit);
  }

  JER2_ARAD_ALLOC(mc_members, DNX_TMC_MULT_ING_ENTRY, NUM_OF_REP, "jer2_arad_iqm_workaround.mc_members");
  /* set the ingress MC group */
  for (i = 0; i < NUM_OF_REP; ++i) {
      DNX_TMC_MULT_ING_ENTRY_clear(&(mc_members[i]));
      mc_members[i].destination.type = DNX_TMC_DEST_TYPE_QUEUE;
      mc_members[i].destination.id = (i / 32) + BASE_QUEUE;
      mc_members[i].cud = i + 1;
  }
  res = dnx_mult_ing_group_update(unit, TEMP_MC_ID, mc_members, NUM_OF_REP, &mc_err);
  DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);
  if (mc_err) {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 70, exit);
  }


  /* set port information for port 1, default CPU trap code and forwarding action strength */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 80, exit, READ_IHP_PINFO_LLRm(unit, MEM_BLOCK_ANY, 1, entry));
  sal_memcpy(bu_IHP_PINFO_LLR, entry, sizeof(bu_IHP_PINFO_LLR));
  soc_mem_field32_set(unit, IHP_PINFO_LLRm, entry, DEFAULT_ACTION_PROFILE_FWDf, 7);
  soc_mem_field32_set(unit, IHP_PINFO_LLRm, entry, DEFAULT_CPU_TRAP_CODEf, TEMP_FWD_ACTION_PROFILE);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 90, exit, WRITE_IHP_PINFO_LLRm(unit, MEM_BLOCK_ANY, 1, entry));

  /* make the action profile referred to previously replace the destination with our MC group */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 100, exit, READ_IHB_FWD_ACT_PROFILEm(unit, MEM_BLOCK_ANY, TEMP_FWD_ACTION_PROFILE, entry));
  sal_memcpy(bu_IHB_FWD_ACT_PROFILE, entry, sizeof(bu_IHB_FWD_ACT_PROFILE));
  soc_mem_field32_set(unit, IHB_FWD_ACT_PROFILEm, entry, FWD_ACT_DESTINATIONf, 0x50000 + TEMP_MC_ID);
  soc_mem_field32_set(unit, IHB_FWD_ACT_PROFILEm, entry, FWD_ACT_BYPASS_FILTERINGf, 1);
  soc_mem_field32_set(unit, IHB_FWD_ACT_PROFILEm, entry, FWD_ACT_DESTINATION_OVERWRITEf, 1);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, WRITE_IHB_FWD_ACT_PROFILEm(unit, MEM_BLOCK_ANY, TEMP_FWD_ACTION_PROFILE, entry));
  /* Disable credit watchdog threshold so that the packets will not get erased */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 120, exit, READ_IPS_CRWDTHm(unit, MEM_BLOCK_ANY, 0, entry));
  sal_memcpy(bu_IPS_CRWDTH, entry, sizeof(bu_IPS_CRWDTH));
  soc_mem_field32_set(unit, IPS_CRWDTHm, entry, WD_DELETE_Q_THf, 0);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 130, exit, WRITE_IPS_CRWDTHm(unit, MEM_BLOCK_ANY, 0, entry));


  /* enable data paths */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 140, exit, READ_IRE_STATIC_CONFIGURATIONr(unit, &val32));
  soc_reg_field_set(unit, IRE_STATIC_CONFIGURATIONr, &val32, ENABLE_DATA_PATHf, 1);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 150, exit, WRITE_IRE_STATIC_CONFIGURATIONr(unit, val32));

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 160, exit, READ_IDR_DYNAMIC_CONFIGURATIONr(unit, &val32));
  soc_reg_field_set(unit, IDR_DYNAMIC_CONFIGURATIONr, &val32, ENABLE_DATA_PATHf, 1);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 170, exit, WRITE_IDR_DYNAMIC_CONFIGURATIONr(unit, val32));

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 180, exit, READ_IRR_DYNAMIC_CONFIGURATIONr(unit, &val32));
  soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &val32, ENABLE_DATA_PATH_IQMf, 1);
  soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &val32, ENABLE_DATA_PATH_IDRf, 1);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 190, exit, WRITE_IRR_DYNAMIC_CONFIGURATIONr(unit, val32));

  /* enable traffic */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 200, exit, READ_IQM_IQM_ENABLERSr(unit, REG_PORT_ANY, &val32));
  soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &val32, DSCRD_ALL_PKTf, 0);
  if (init->dram.pdm_mode != JER2_ARAD_INIT_PDM_MODE_SIMPLE /*soc_property_get(unit,spn_BCM886XX_PDM_MODE, 0)*/) {
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &val32, VSQ_CD_ENf, 0);
  }
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 210, exit, WRITE_IQM_IQM_ENABLERSr(unit, REG_PORT_ANY, val32));

  /* Discard credits and do not send delete commands */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 220, exit, READ_IPS_IPS_GENERAL_CONFIGURATIONSr_REG32(unit, REG_PORT_ANY, &val32));
  soc_reg_field_set(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, &val32, DISCARD_ALL_CRDTf, 0);
  soc_reg_field_set(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, &val32, DIS_DEQ_CMDSf, 0);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 230, exit, WRITE_IPS_IPS_GENERAL_CONFIGURATIONSr_REG32(unit, REG_PORT_ANY, val32));


  val32 = 0;
  soc_reg_field_set(unit, IRE_REG_FAP_PORT_CONFIGURATIONr, &val32, REG_REASSEMBLY_CONTEXTf, 1);
  soc_reg_field_set(unit, IRE_REG_FAP_PORT_CONFIGURATIONr, &val32, REG_PORT_TERMINATION_CONTEXTf, 1);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 240, exit, WRITE_IRE_REG_FAP_PORT_CONFIGURATIONr(unit, val32));

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 250, exit, WRITE_IRE_REGI_PKT_DATAr(unit, reg_above_64_val));

  /* send a packet asynchronously from the CPU */
  COMPILER_64_ZERO(val64);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_SEND_DATAf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_ERRf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_SOPf, 1);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_EOPf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_BEf, 31);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 260, exit, WRITE_IRE_REGISTER_INTERFACE_PACKET_CONTROLr(unit, val64));
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_SEND_DATAf, 1);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_ERRf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_SOPf, 1);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_EOPf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_BEf, 31);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 270, exit, WRITE_IRE_REGISTER_INTERFACE_PACKET_CONTROLr(unit, val64));
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_SEND_DATAf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_ERRf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_SOPf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_EOPf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_BEf, 31);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 280, exit, WRITE_IRE_REGISTER_INTERFACE_PACKET_CONTROLr(unit, val64));
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_SEND_DATAf, 1);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_ERRf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_SOPf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_EOPf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_BEf, 31);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 290, exit, WRITE_IRE_REGISTER_INTERFACE_PACKET_CONTROLr(unit, val64));
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_SEND_DATAf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_ERRf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_SOPf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_EOPf, 1);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_BEf, 31);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 300, exit, WRITE_IRE_REGISTER_INTERFACE_PACKET_CONTROLr(unit, val64));
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_SEND_DATAf, 1);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_ERRf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_SOPf, 0);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_EOPf, 1);
  soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &val64, REGI_PKT_BEf, 31);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 310, exit, WRITE_IRE_REGISTER_INTERFACE_PACKET_CONTROLr(unit, val64));


  /* PDM initialization */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 320, exit, READ_IQM_IQM_INITr(unit, REG_PORT_ANY, &val32));
  soc_reg_field_set(unit, IQM_IQM_INITr, &val32, PDM_INITf, 1);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 330, exit, WRITE_IQM_IQM_INITr(unit, REG_PORT_ANY, val32));

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 340, exit, READ_IQM_IQM_INITr(unit, REG_PORT_ANY, &val32));
  soc_reg_field_set(unit, IQM_IQM_INITr, &val32, PDM_INITf, 0);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 350, exit, WRITE_IQM_IQM_INITr(unit, REG_PORT_ANY, val32));


  /* Grant one credit to each queue */
  for (i = 0; i < NOF_QUEUES_TO_BLOCK; ++i) {
    val32 = 0;
    soc_reg_field_set(unit, IPS_MANUAL_QUEUE_OPERATION_QUEUE_IDr, &val32, MAN_QUEUE_IDf, i + BASE_QUEUE);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 360, exit, WRITE_IPS_MANUAL_QUEUE_OPERATION_QUEUE_IDr(unit, REG_PORT_ANY, val32));

    DNX_SAND_SOC_IF_ERROR_RETURN(res, 370, exit, READ_IPS_MANUAL_QUEUE_OPERATIONr(unit, REG_PORT_ANY, &val32));
    soc_reg_field_set(unit, IPS_MANUAL_QUEUE_OPERATIONr, &val32, GRANT_CREDITf, 1);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 380, exit, WRITE_IPS_MANUAL_QUEUE_OPERATIONr(unit, REG_PORT_ANY, val32));
  }

  /* Return tables to their previous values */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 390, exit, WRITE_IPS_CRWDTHm(unit, MEM_BLOCK_ANY, 0, bu_IPS_CRWDTH));
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 400, exit, WRITE_IHP_PINFO_LLRm(unit, MEM_BLOCK_ANY, 1, bu_IHP_PINFO_LLR));
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 410, exit, WRITE_IHB_FWD_ACT_PROFILEm(unit, MEM_BLOCK_ANY, 0x2a, bu_IHB_FWD_ACT_PROFILE));

  /* disable data paths */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 420, exit, READ_IRE_STATIC_CONFIGURATIONr(unit, &val32));
  soc_reg_field_set(unit, IRE_STATIC_CONFIGURATIONr, &val32, ENABLE_DATA_PATHf, 0);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 430, exit, WRITE_IRE_STATIC_CONFIGURATIONr(unit, val32));

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 440, exit, READ_IDR_DYNAMIC_CONFIGURATIONr(unit, &val32));
  soc_reg_field_set(unit, IDR_DYNAMIC_CONFIGURATIONr, &val32, ENABLE_DATA_PATHf, 0);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 450, exit, WRITE_IDR_DYNAMIC_CONFIGURATIONr(unit, val32));

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 460, exit, READ_IRR_DYNAMIC_CONFIGURATIONr(unit, &val32));
  soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &val32, ENABLE_DATA_PATH_IQMf, 0);
  soc_reg_field_set(unit, IRR_DYNAMIC_CONFIGURATIONr, &val32, ENABLE_DATA_PATH_IDRf, 0);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 470, exit, WRITE_IRR_DYNAMIC_CONFIGURATIONr(unit, val32));

  /* disable traffic */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 480, exit, READ_IQM_IQM_ENABLERSr(unit, REG_PORT_ANY, &val32));
  soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &val32, DSCRD_ALL_PKTf, 1);
  if (init->dram.pdm_mode != JER2_ARAD_INIT_PDM_MODE_SIMPLE /*soc_property_get(unit,spn_BCM886XX_PDM_MODE, 0)*/) {
    soc_reg_field_set(unit, IQM_IQM_ENABLERSr, &val32, VSQ_CD_ENf, 1);
  }
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 490, exit, WRITE_IQM_IQM_ENABLERSr(unit, REG_PORT_ANY, val32));

  /* Do not discard credits and do send delete commands */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 500, exit, READ_IPS_IPS_GENERAL_CONFIGURATIONSr_REG32(unit, REG_PORT_ANY, &val32));
  soc_reg_field_set(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, &val32, DISCARD_ALL_CRDTf, 1);
  soc_reg_field_set(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, &val32, DIS_DEQ_CMDSf, 1);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 510, exit, WRITE_IPS_IPS_GENERAL_CONFIGURATIONSr_REG32(unit, REG_PORT_ANY, val32));


  /* Soft init the IPS block */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 520, exit, READ_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));
  soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_INITr, reg_above_64_val, IPS_INITf, 1);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 530, exit, WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 540, exit, READ_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));
  soc_reg_above_64_field32_set(unit, ECI_BLOCKS_SOFT_INITr, reg_above_64_val, IPS_INITf, 0);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 550, exit, WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 560, exit, READ_IQM_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, REG_PORT_ANY, &val32));
  if ((should_disable_dynamic_mem_changes = (val32 & 1) == 0)) {
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 570, exit, WRITE_IQM_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, REG_PORT_ANY, 1));
  } /* enabled changes in IQM dynamic memory */
  /* Manually mark each queue as empty */
  for (i = 0; i < NOF_QUEUES_TO_BLOCK; ++i) {
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 580, exit, READ_IQM_PQDMDm(unit, MEM_BLOCK_ANY, i + BASE_QUEUE, entry));
    soc_mem_field32_set(unit, IQM_PQDMDm, entry, QUE_NOT_EMPTYf, 0);
    soc_mem_field32_set(unit, IQM_PQDMDm, entry, PQ_INST_QUE_SIZEf, 0);
    soc_mem_field32_set(unit, IQM_PQDMDm, entry, PQ_INST_QUE_BUFF_SIZEf, 0);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 590, exit, WRITE_IQM_PQDMDm(unit, MEM_BLOCK_ANY, i + BASE_QUEUE, entry));
  }

  res = dnx_mult_ing_group_close(unit, TEMP_MC_ID);
  DNX_SAND_CHECK_FUNC_RESULT(res, 600, exit);

  if ((init->dram.enable == TRUE) || (init->drc_info.enable)) {
    /* get buffer number */
    nof_dram_buffs = init->dram.nof_dram_buffers;
    DNX_SAND_CHECK_FUNC_RESULT(res, 610, exit);
    if ((init->ocb.ocb_enable == OCB_DISABLED) || (init->ocb.ocb_enable == OCB_ENABLED) || (init->ocb.ocb_enable == OCB_DRAM_SEPARATE)){
        res = jer2_arad_init_dram_buff_boundaries_calc(unit, nof_dram_buffs, init->dram.dbuff_size, init->ocb, &dbuffs_bdries);
        DNX_SAND_CHECK_FUNC_RESULT(res, 620, exit);
    }
    else if (init->ocb.ocb_enable == OCB_ONLY){ /* ocb_enable == OCB_ONLY || ocb_enable == OCB_ONLY_1_DRAM */
        res = jer2_arad_init_dram_buff_boundaries_calc_ocb_only(unit, nof_dram_buffs, init->dram.dbuff_size, init->ocb.repartition_mode, &dbuffs_bdries);
        DNX_SAND_CHECK_FUNC_RESULT(res, 630, exit);
        dbuffs_bdries.fmc.start = dbuffs_bdries.ocb_fmc.start;
    }
    else if (init->ocb.ocb_enable == OCB_ONLY_1_DRAM){
        res = jer2_arad_init_dram_buff_boundaries_calc_ocb_only_1_dram(unit, nof_dram_buffs, init->dram.dbuff_size, init->ocb.repartition_mode, &dbuffs_bdries);
        DNX_SAND_CHECK_FUNC_RESULT(res, 640, exit);
        dbuffs_bdries.fmc.start = dbuffs_bdries.ocb_fmc.start;
    }
  } else{ /*no drams are present*/
      if (init->ocb.ocb_enable != OCB_DISABLED) {
            /* We are on OCB_ENABLE or OCB_ONLY*/
            res = jer2_arad_init_dram_buff_boundaries_calc_ocb_only(unit, nof_dram_buffs, init->dram.dbuff_size, init->ocb.repartition_mode, &dbuffs_bdries);
            DNX_SAND_CHECK_FUNC_RESULT(res, 650, exit);
            dbuffs_bdries.fmc.start = dbuffs_bdries.ocb_fmc.start;
      }
  }

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 660, exit, READ_NBI_STATISTICS_RX_BURSTS_ERR_CNTr(unit, &val32));
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 670, exit, READ_IRE_REGI_PACKET_COUNTERr(unit, &val64));
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 680, exit, READ_IQM_ENQUEUE_PACKET_COUNTERr(unit, REG_PORT_ANY, &val32));

#ifdef DEBUG_WORKAROUND
  LOG_INFO(BSL_LS_SOC_INIT,
           (BSL_META_U(unit,
                       "buffers to clear at 0x%lx\n"),(unsigned long)dbuffs_bdries.fmc.start));

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 690, exit, READ_IQM_FREE_FULL_MULTICAST_DBUFFS_COUNTERr(unit, REG_PORT_ANY, &val32));
  DNX_SAND_CHECK_FUNC_RESULT(res, 700, exit);
  LOG_INFO(BSL_LS_SOC_INIT,
           (BSL_META_U(unit,
                       "IQM_FREE_FULL_MULTICAST_DBUFFS_COUNTER is 0x%lx\n"),(unsigned long)val32));
#endif

  /* Manually release buffers */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 710, exit, READ_IQM_FLUSCNTm(unit, MEM_BLOCK_ANY, dbuffs_bdries.fmc.start, entry));
  soc_mem_field32_set(unit, IQM_FLUSCNTm, entry, FLUS_CNTf, 0);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 720, exit, WRITE_IQM_FLUSCNTm(unit, MEM_BLOCK_ANY, dbuffs_bdries.fmc.start, entry));

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 730, exit, READ_IPT_CPU_D_BUFF_RELEASE_CONTROLr(unit, &val32));
  soc_reg_field_set(unit, IPT_CPU_D_BUFF_RELEASE_CONTROLr, &val32, CPU_RELEASE_BUFFER_VALIDf, 1);
  soc_reg_field_set(unit, IPT_CPU_D_BUFF_RELEASE_CONTROLr, &val32, BUFFER_2_RELEASEf, dbuffs_bdries.fmc.start);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 740, exit, WRITE_IPT_CPU_D_BUFF_RELEASE_CONTROLr(unit, val32));

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 750, exit, WRITE_IQM_INTERRUPT_REGISTERr(unit, REG_PORT_ANY, 0x7fffffff & ~intr_val)); /* 31 bits */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 760, exit, WRITE_IQM_INTERRUPT_MASK_REGISTERr(unit, REG_PORT_ANY, intr_mask));

#ifdef DEBUG_WORKAROUND
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 770, exit, READ_IQM_FREE_FULL_MULTICAST_DBUFFS_COUNTERr(unit, REG_PORT_ANY, &val32));
  DNX_SAND_CHECK_FUNC_RESULT(res, 780, exit);
  LOG_INFO(BSL_LS_SOC_INIT,
           (BSL_META_U(unit,
                       "IQM_FREE_FULL_MULTICAST_DBUFFS_COUNTER is 0x%lx\n"),(unsigned long)val32));
#endif

exit:
  JER2_ARAD_FREE(mc_members);
  if (should_disable_dynamic_mem_changes) { /* disable changes in IQM dynamic memory */
    if(WRITE_IQM_ENABLE_DYNAMIC_MEMORY_ACCESSr(unit, REG_PORT_ANY, 0) != SOC_E_NONE) {
     LOG_ERROR(BSL_LS_SOC_INIT,
               (BSL_META_U(unit,
                           SOC_DNX_MSG("Failed to write to the register IQM_ENABLE_DYNAMIC_MEMORY_ACCESS\n"))));
    }
  }
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_iqm_workaround()", unit, 0);
#endif 
  return -1;
}

#if ! (defined __KERNEL__) && ! (defined _STDLIB_H)
extern char *getenv(const char*);
#endif


#ifdef BCM_88660_A0
/* configure the statistics interface TC source for Arad+ */
uint32
  jer2_arad_set_stat_if_tc_source_unsafe(
    DNX_SAND_IN  int unit
  )
{
    uint32 res;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);
    if (SOC_IS_ARADPLUS(unit)) {
        char *propkey = spn_STAT_IF_TC_SOURCE;
        char *propval = soc_property_get_str(unit, propkey);
        uint32 configuration = 0;

        if (propval) {
            if (sal_strcmp(propval, "ORIGINAL") == 0) { /* TC form IRR */
                configuration = 1;
            } else if (sal_strcmp(propval, "MAPPED") != 0) { /* TC from IQM (default) */
                LOG_ERROR(BSL_LS_SOC_INIT,
                          (BSL_META_U(unit,
                                      "Unexpected property value (\"%s\") for %s\n\r"), propval, propkey));
                DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 50, exit);
            }
        }
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  60,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IQM_STATISTICS_REPORT_CONFIGURATIONS_7r, REG_PORT_ANY, 0, ST_BILL_ING_USE_IRR_TCf,  configuration));
    }
exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_set_stat_if_tc_source_unsafe()", unit, 0);
}
#endif /* BCM_88660_A0 */


/*********************************************************************
*     Initialize the device, including:1. Prevent all the
*     control cells. 2. Initialize the device tables and
*     registers to default values. 3. Initialize
*     board-specific hardware interfaces according to
*     configurable information, as passed in init. 4.
*     Perform basic device initialization. The configuration
*     can be enabled/disabled as passed in enable_info.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_init_sequence_phase1_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_MGMT_INIT         *init,
    DNX_SAND_IN  uint8                  silent
  )
{
  uint32
    res;
  int
    core, rv;
  uint32
     stage_internal_id = 0,
     stage_id = 0,
     port_i,
     iqm_init_timeout;
  uint32
     tm_port, priority_mode,
     shaper_mode, base_q_pair, flags;
  JER2_ARAD_INIT_DBUFFS_BDRY
    dbuffs_bdries;
  soc_reg_above_64_val_t
      reg_above_64_val;
  soc_pbmp_t
      pbmp;
  const char *propval =
#ifdef __KERNEL__
    0;
#else
    getenv("DO_NOT_INITIALIZE_ALL_TABLES");
#endif
  int init_tables;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_INIT_SEQUENCE_PHASE1_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(init);
  (void)stage_internal_id;
  (void)stage_id;
  sal_memset(&dbuffs_bdries, 0, sizeof(dbuffs_bdries));

  /************************************************************************/
  /* Prepare internal data                                                */
  /************************************************************************/

  res = jer2_arad_init_operation_mode_set(unit, init);
  DNX_SAND_CHECK_FUNC_RESULT(res, 3, exit);

  
  DNXC_LEGACY_FIXME_ASSERT;


  /************************************************************************/
  /* Initialize Mgmt*/
  /************************************************************************/
  JER2_ARAD_INIT_PRINT_ADVANCE("Mgmt");

  /************************************************************************/
  /* Initialize basic configuration (must be before per-block Out-Of-Reset*/
  /************************************************************************/
  JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Initialize internal blocks, in-reset");
  res = jer2_arad_mgmt_init_before_blocks_oor(
          unit,
          &dbuffs_bdries,
          init
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 15, exit);


  /************************************************************************/
  /* Out-of-reset Arad internal blocks                                   */
  /************************************************************************/
  JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Take internal blocks out-of-reset");
  res = jer2_arad_mgmt_blocks_init_unsafe(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  
  DNXC_LEGACY_FIXME_ASSERT;


  /************************************************************************/
  /* Initialize all tables                                                */
  /*                                                                      */
  /* Most tables are zeroed. Some - initialized to non-zero default       */
  /************************************************************************/
  JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Initialize tables to zero");
  if (propval) {
    init_tables = !sal_strcmp(propval, "0");
  } else {
#ifdef PLISIM
    if (SAL_BOOT_PLISIM) {
        init_tables = 0;
    }
    else
#endif
    {
        init_tables = 1;
    }
  }
   /* if clearing all tables was not disabled */
  if (init_tables) { /* if clearing all tables was not disabled */
    res = jer2_arad_mgmt_all_tbls_init(unit, silent);
    DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  }

  /* Do soft init after all the tables are reset */
  reg_above_64_val[3] = reg_above_64_val[2] =
  reg_above_64_val[1] = reg_above_64_val[0] = 0xFFFFFFFF;
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 42, exit, WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));
  sal_usleep(10);

  reg_above_64_val[3] = reg_above_64_val[2] =
  reg_above_64_val[1] = reg_above_64_val[0] = 0;
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 45, exit, WRITE_ECI_BLOCKS_SOFT_INITr(unit, reg_above_64_val));




  /************************************************************************/
  /* Validate/Poll for out-of-reset/init-done indications                 */
  /************************************************************************/
  sal_usleep(1000);

  res = jer2_arad_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, IPS_IPS_GENERAL_CONFIGURATIONSr, REG_PORT_ANY, 0, IPS_INIT_TRIGGERf, 0x0);
  DNX_SAND_CHECK_FUNC_RESULT(res, 46, exit);

  iqm_init_timeout = (SOC_DNX_CONFIG(unit)->emulation_system == 1) ? JER2_ARAD_TIMEOUT * 1000 : JER2_ARAD_TIMEOUT;
  res = jer2_arad_polling(unit, iqm_init_timeout, JER2_ARAD_MIN_POLLS, IQM_IQM_INITr, REG_PORT_ANY, 0, PDM_INITf, 0x0);
  DNX_SAND_CHECK_FUNC_RESULT(res, 47, exit);

  res = jer2_arad_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, EGQ_EGQ_BLOCK_INIT_STATUSr, REG_PORT_ANY, 0, EGQ_BLOCK_INITf, 0x0);
  DNX_SAND_CHECK_FUNC_RESULT(res, 48, exit);


  /************************************************************************/
  /* Set Core clock frequency                                             */
  /************************************************************************/
  if (init->core_freq.enable)
  {
      JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Set core clock frequency");
      res = jer2_arad_mgmt_init_set_core_clock_frequency(unit, init);
      DNX_SAND_CHECK_FUNC_RESULT(res, 28, exit);
  }

  /************************************************************************/
  /* Stop all traffic                                                     */
  /************************************************************************/
  JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Stop traffic");
  res = jer2_arad_mgmt_enable_traffic_set(
          unit,
          FALSE
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  /************************************************************************/
  /* Stop Control Cells                                                   */
  /*                                                                      */
  /* Disable the device from sending control cells                        */
  /* prior to FAP-ID setting).                                            */
  /************************************************************************/
  JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Stop control cells")
  res = jer2_arad_mgmt_all_ctrl_cells_enable_set(
          unit,
          FALSE
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 35, exit);

  /************************************************************************/
  /* Initialize basic configuration (based on pre-OOR)                    */
  /************************************************************************/
  JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Finalize internal blocks initialization");
  res = jer2_arad_mgmt_init_after_blocks_oor(
          unit,
          init,
          &dbuffs_bdries
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 37, exit);
  /************************************************************************/
  /* Initialize all tables                                                */
  /*                                                                      */
  /* Most tables are zeroed. Some - initialized to non-zero default       */
  /************************************************************************/
  JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Configure tables defaults");

  res = jer2_arad_mgmt_tbls_init(
          unit,
          silent
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 42, exit);

  /************************************************************************/
  /* Set registers not covered in any functional module, */
  /* with default values different from hardware defaults                 */
  /************************************************************************/
  JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Initialize registers defaults");
  res = jer2_arad_mgmt_hw_set_defaults(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  /************************************************************************/
  /* Set board-related configuration (hardware adjustments)               */
  /************************************************************************/
  if (init != NULL)
  {
    JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Configure HW interfaces");
    res = jer2_arad_mgmt_hw_interfaces_set_unsafe(
            unit,
            init,
            silent
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);
  }
  else
  {
    JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("SKIPPING jer2_arad_mgmt_hw_interfaces_set");
  }
  /************************************************************************/
  /* Set bubble configuration                                             */
  /************************************************************************/

  res = jer2_arad_bubble_configuration_set_unsafe(unit);
  DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

  /************************************************************************/
  /* Set basic configuration                                              */
  /************************************************************************/
    if (init->credit.credit_worth_enable)
    {
      JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Set credit worth");
      rv = jer2_arad_mgmt_credit_worth_set(
              unit,
              init->credit.credit_worth
            );
      DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 70, exit);
    }



  /************************************************************************/
  /* Per functional module, perform initializations                       */
  /* covered by module's functionality.                                   */
  /************************************************************************/
  JER2_ARAD_INIT_PRINT_ADVANCE("Set default configuration");
  res = jer2_arad_mgmt_functional_init(
          unit,
          init,
          silent
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 90, exit);


  /************************************************************************/
  /* Set port configuration.                                              */
  /************************************************************************/
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  JER2_ARAD_INIT_PRINT_ADVANCE("Port");
  /*
   * Before the port configuration: OTMH extensions configuration
   * must know the egress editor program attributes
   */
  JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Set the Egress editor");
  res = jer2_arad_egr_prog_editor_config_dut_by_queue_database(unit);
  DNX_SAND_CHECK_FUNC_RESULT(res, 164, exit);

#endif 
  /************************************************************************/
  /* Set FAP port configuration.                                       */
  /************************************************************************/
  JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Set OFP to Egress port information");

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 165, exit, dnx_port_sw_db_valid_ports_get(unit, 0, &pbmp));

  SOC_PBMP_ITER(pbmp,port_i) {
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 166, exit, dnx_port_sw_db_flags_get(unit, port_i, &flags));
      if (!(DNX_PORT_IS_ELK_INTERFACE(flags) || DNX_PORT_IS_STAT_INTERFACE(flags))) {
          res = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.base_q_pair.get(unit, port_i, &base_q_pair);
          DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 120, exit);

          res = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.shaper_mode.get(unit, port_i, &shaper_mode);
          DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 125, exit);

          res = dnx_port_sw_db_local_to_out_port_priority_get(unit, port_i, &priority_mode);
          DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 135, exit);

          res = dnx_port_sw_db_local_to_tm_port_get(unit, port_i, &tm_port, &core);
          DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 140, exit);

          /* Map local port to base_q_pair */
          res = jer2_arad_egr_dsp_pp_to_base_q_pair_set(unit, core, tm_port, base_q_pair);
          DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 145, exit);

          /* Set port priorities */
          res = jer2_arad_egr_dsp_pp_priorities_mode_set(unit, core, tm_port, priority_mode);
          DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 145, exit);

          /* Set egress port shaper mode */
          res = jer2_arad_egr_dsp_pp_shaper_mode_set_unsafe(unit, core, tm_port, shaper_mode);
          DNX_SAND_CHECK_FUNC_RESULT(res, 160, exit);
      }
  }

  JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Set port to interface mapping");

  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  /*
   *  Set for each NIF interface nof entries to be allocated for base context mapping.
   *  In case dynamic port is enabled the mapping is static (250 channels are assumed for ILKN),                                                                               .
   *  Otherwise base mapping is defined by the maximal channel number.
   */
  if (SOC_DNX_CONFIG(unit)->jer2_arad->init.dynamic_port_enable)
  {
      res = jer2_arad_ports_init_interfaces_dynamic_context_map(unit);
      DNX_SAND_CHECK_FUNC_RESULT(res, 164, exit);
  }
  else
  {
      res = jer2_arad_ports_init_interfaces_context_map(
           unit,
           &(init->ports)
           );
      DNX_SAND_CHECK_FUNC_RESULT(res, 165, exit);
  }

  /*
   *  In case ERP port is enabled, search for unoccupied NIF interface
   */
  res = jer2_arad_ports_init_interfaces_erp_setting(
       unit,
       &(init->ports)
       );
  DNX_SAND_CHECK_FUNC_RESULT(res, 167, exit);

  SOC_PBMP_ITER(pbmp, port_i) {
      jer2_arad_JER2_ARAD_PORT2IF_MAPPING_INFO_clear(&mapping_info);
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 169, exit, dnx_port_sw_db_interface_type_get(unit, port_i, &interface_type));

      res = dnx_port_sw_db_local_to_tm_port_get(unit, port_i, &tm_port, &core);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 175, exit);

      switch (interface_type) {
      case SOC_PORT_IF_XFI:
      case SOC_PORT_IF_RXAUI:
      case SOC_PORT_IF_DNX_XAUI:
      case SOC_PORT_IF_SGMII:
      case SOC_PORT_IF_XLAUI:
      case SOC_PORT_IF_CAUI:
      case SOC_PORT_IF_ILKN:
      case SOC_PORT_IF_TM_INTERNAL_PKT:
        jer2_arad_port_to_nif_type(unit, tm_port, &nif_type);
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 1050, exit, dnx_port_sw_db_first_phy_port_get(unit, port_i, &phy_port));
        mapping_info.if_id = jer2_arad_nif_intern2nif_id(unit, nif_type, phy_port - 1);
        break;
      case SOC_PORT_IF_CPU:
        mapping_info.if_id = JER2_ARAD_IF_ID_CPU;
        break;
      case SOC_PORT_IF_RCY:
        mapping_info.if_id = JER2_ARAD_IF_ID_RCY;
        break;
      case SOC_PORT_IF_ERP:
        mapping_info.if_id = JER2_ARAD_IF_ID_ERP;
        break;
      case SOC_PORT_IF_OLP:
        mapping_info.if_id = JER2_ARAD_IF_ID_OLP;
        break;
      case SOC_PORT_IF_OAMP:
        mapping_info.if_id = JER2_ARAD_IF_ID_OAMP;
        break;
      default:
        DNX_SAND_SET_ERROR_CODE(JER2_ARAD_INCOMPATABLE_NIF_ID_ERR, 1060, exit);
        break;
      }

      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1070, exit, dnx_port_sw_db_channel_get(unit, port_i, &(mapping_info.channel_id)));

      if (mapping_info.if_id == JER2_ARAD_IF_ID_ERP) {
        res = jer2_arad_port_to_interface_map_set_unsafe(
           unit,
           core,
           tm_port,
           JER2_ARAD_PORT_DIRECTION_OUTGOING,
           &mapping_info,
           TRUE
           );
        DNX_SAND_CHECK_FUNC_RESULT(res, 125, exit);
      } else {
        res = jer2_arad_port_to_interface_map_set_unsafe(
           unit,
           core,
           tm_port,
           JER2_ARAD_PORT_DIRECTION_BOTH,
           &mapping_info,
           TRUE
           );
        DNX_SAND_CHECK_FUNC_RESULT(res, 130, exit);
      }
  }
  JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Set port header type");

  SOC_PBMP_ITER(pbmp, port_i) {
    res = jer2_arad_ports_header_type_update(unit, port_i);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 140, exit);
  }

  /* LAG configuration */
  res = jer2_arad_ports_lag_mode_set_unsafe(unit, init->ports.lag_mode);
  DNX_SAND_CHECK_FUNC_RESULT(res, 152, exit);
  res = soc_jer2_arad_trunk_resolve_ingress_mc_destination_method(unit, init->ports.use_trunk_as_ingress_mc_dest);
  DNX_SAND_CHECK_FUNC_RESULT(res, 153, exit);

  DNXC_LEGACY_FIXME_ASSERT;
#endif 


  /* Stacking Configuration */
  if (init->ports.is_stacking_system == 0x1) {
      JER2_ARAD_INIT_PRINT_ADVANCE("Stacking");
      res = jer2_arad_mgmt_stk_init(unit, init);
      DNX_SAND_CHECK_FUNC_RESULT(res, 170, exit);
  }

  /* Systam RED configuration */
  if (init->ports.is_system_red == 0x1) {
      JER2_ARAD_INIT_PRINT_ADVANCE("System RED");
      res = jer2_arad_mgmt_system_red_init(unit, init);
      DNX_SAND_CHECK_FUNC_RESULT(res, 175, exit);
  }

  /************************************************************************/
  /* Set statistic configuration                                          */
  /************************************************************************/
  
  DNXC_LEGACY_FIXME_ASSERT;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_init_sequence_phase1_unsafe()", 0, 0);
}

uint32
  jer2_arad_mgmt_init_sequence_phase1_verify(
    DNX_SAND_IN     int              unit,
    DNX_SAND_IN     JER2_ARAD_MGMT_INIT           *init
  )
{
  uint32
    res = 0;
/*  uint32
    idx;*/

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_INIT_SEQUENCE_PHASE1_VERIFY);

  DNX_SAND_CHECK_NULL_INPUT(init);
  DNX_SAND_MAGIC_NUM_VERIFY(init);

  res = jer2_arad_mgmt_hw_interfaces_verify(
          unit,
          init
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  /*
   * DNX_SAND_FE600 verifications
   */
  if (init->fabric.is_fe600 == TRUE) {
    

    
  }


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_init_sequence_phase1_verify()", 0, 0);
}


/* number of NIF blocks, and number of sub blocks in a CLP block */
#define JER2_ARAD_NOF_NIF_BLOCKS (SOC_JER2_ARAD_NOF_INSTANCES_CLP + SOC_JER2_ARAD_NOF_INSTANCES_XLP)
#define JER2_ARAD_NOF_XMACS_IN_CLP SOC_JER2_ARAD_NOF_QUADS_IN_CLP
#define JER2_ARAD_NOF_CLP_SUB_BLOCKS (JER2_ARAD_NOF_XMACS_IN_CLP + 1)

#define JER2_ARAD_NOF_JER2_ARAD_FABRIC_QUADS(unit)   (SOC_JER2_ARAD_NOF_QUADS_IN_FSRD * SOC_DNX_DEFS_GET(unit, nof_instances_fsrd))
#define JER2_ARAD_MAX_NOF_JER2_ARAD_FABRIC_QUADS     (SOC_JER2_ARAD_NOF_QUADS_IN_FSRD * SOC_DNX_DEFS_MAX(NOF_INSTANCES_FSRD))
#define JER2_ARAD_NOF_JER2_ARAD_FABRIC_3QUAD_FMACS   (SOC_DNX_DEFS_MAX(NOF_INSTANCES_FSRD))

/*********************************************************************
*     Out-of-reset sequence. Enable/Disable the device from
*     receiving and transmitting control cells.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_init_sequence_phase2_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 silent
  )
{
  uint32
    res;
  uint32
/*    idx = 0, */
    stage_id = 0,
    stage_internal_id = 0;


  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_INIT_SEQUENCE_PHASE2_UNSAFE);

(void)stage_internal_id;
(void)stage_id;

  if (!SOC_UNIT_NUM_VALID(unit)) {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_ILLEGAL_DEVICE_ID, 10, exit);
  }

  if (!silent)
  {
   LOG_VERBOSE(BSL_LS_SOC_INIT,
               (BSL_META_U(unit,
                           "\n\r"
                           "   initialization: device %u"
                           "\n\r"),
                unit
                ));
  }

  JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("Enable control cells");
  res = jer2_arad_mgmt_all_ctrl_cells_enable_set(
          unit,
          TRUE
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);



  JER2_ARAD_INIT_PRINT_INTERNAL_LEVEL_0_ADVANCE("initialization done");

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_init_sequence_phase2_unsafe()", 0, 0);
}

/*
 *  Init sequence -
 *  per-block initialization, hardware adjustments etc. }
 */


#if JER2_ARAD_DEBUG_IS_LVL1
#endif /* JER2_ARAD_DEBUG_IS_LVL1 */

static uint32
  jer2_arad_mgmt_ire_tbls_init(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 silent
  )
{
  uint32 res = DNX_SAND_OK;
  uint32 table_entry[3] = {0};

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_IRE_TBLS_INIT);
  DNX_SAND_PCID_LITE_SKIP(unit);

  table_entry[2] = 0x40000; /* Add CRC */
  res = jer2_arad_fill_table_with_entry(unit, IRE_TDM_CONFIGm, MEM_BLOCK_ANY, table_entry);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_ire_tbls_init()",0,0);
}

int _jer2_arad_mgmt_irr_tbls_init_dma_callback(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int copyno,
    DNX_SAND_IN int array_index,
    DNX_SAND_IN int index,
    DNX_SAND_OUT uint32 *value,
    DNX_SAND_IN int entry_sz,
    DNX_SAND_IN void *opaque)
{
  /* Buffer the following loop: */
  /*
  for (lag_size = 0; lag_size < 16; ++lag_size)
  {
    for (hash_indx = 0; hash_indx < 256 ; ++hash_indx)
    {
        Member of lag should be calculated mainly from the msb bits,
        because in stacking systems stacking trunk port is decided by the lsb bits
      irr_smooth_division_tbl_data.member = ((hash_indx >> 4) | (hash_indx << 4)) % (lag_size+1);
      res = jer2_arad_irr_smooth_division_tbl_set_unsafe(
              unit,
              lag_size,
              hash_indx,
              &irr_smooth_division_tbl_data
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 112, exit);
    }
  }
  */
  uint32 lag_size = 0;
  uint32 hash_index = 0;
  uint32 member;
  uint32 uindex = (uint32)index;
  soc_mem_t mem;

  *value = 0;
  SHR_BITCOPY_RANGE(&hash_index, 1, &uindex, 0, JER2_ARAD_IRR_GLAG_DEVISION_HASH_NOF_BITS - 1);
  SHR_BITCOPY_RANGE(&lag_size, 0, &uindex, JER2_ARAD_IRR_GLAG_DEVISION_HASH_NOF_BITS - 1, 4);

  mem = (SOC_IS_QAX(unit)) ?  TAR_SMOOTH_DIVISIONm : IRR_SMOOTH_DIVISIONm;

  member = ((hash_index >> 4) | (hash_index << 4)) % (lag_size+1);
  soc_mem_field32_set(unit, mem, value, MEMBER_0f, member);

  hash_index++;
  member = ((hash_index >> 4) | (hash_index << 4)) % (lag_size+1);
  soc_mem_field32_set(unit, mem, value, MEMBER_1f, member);

  return 0;
}

static uint32
  jer2_arad_mgmt_irr_tbls_init(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 silent
  )
{
  uint32 res = DNX_SAND_OK;
  uint32 table_entry[2];
  soc_error_t new_rv;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_IRR_TBLS_INIT);

  /*IRDB*/
  table_entry[0] = table_entry[1] = 0; /* all 32 relevant bits initialized to 0 */
  res = jer2_arad_fill_table_with_entry(unit, IDR_IRDBm, MEM_BLOCK_ANY, table_entry);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /*MCDB*/
  DNX_SAND_SOC_IF_ERROR_RETURN(res,  20, exit, dnx_mult_rplct_tbl_entry_unoccupied_set_all(unit));
  DNX_SAND_SOC_IF_ERROR_RETURN(res,  30, exit, dnx_mcds_multicast_init2(unit));


  /*Destination Table*/
  table_entry[0] = 0; /* we want the entry to be disabled (all 1s queue) and have a traffic class profile of 0 */
  soc_mem_field32_set(unit, IRR_DESTINATION_TABLEm, table_entry, QUEUE_NUMBERf, 0x1ffff); /* mark a disabled entry */
  soc_mem_field32_set(unit, IRR_DESTINATION_TABLEm, table_entry, TC_PROFILEf, 0); /* JER2_ARAD_IPQ_TC_PROFILE_DFLT is 0 */
  res = jer2_arad_fill_table_with_entry(unit, IRR_DESTINATION_TABLEm, MEM_BLOCK_ANY, table_entry); /* fill table with the entry */
  DNX_SAND_CHECK_FUNC_RESULT(res, 110, exit);

  /*Smooth Division*/
  /* Write the smooth division table using DMA. */
  new_rv = jer2_arad_fill_table_with_variable_values_by_caching(unit, IRR_SMOOTH_DIVISIONm, 0, MEM_BLOCK_ANY, -1, -1,
                                                        _jer2_arad_mgmt_irr_tbls_init_dma_callback, NULL);
  if (new_rv != SOC_E_NONE) {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 112, exit);
  }

  /*snoop mirror table 0*/
  table_entry[0] = 0; /* we want the destination to be drop, which marks the profile as not used */
  soc_mem_field32_set(unit, IRR_SNOOP_MIRROR_DEST_TABLEm, table_entry, DESTINATIONf, 0x3ffff);
  res = jer2_arad_fill_table_with_entry(unit, IRR_SNOOP_MIRROR_DEST_TABLEm, MEM_BLOCK_ANY, table_entry); /* fill table with the entry */
  DNX_SAND_CHECK_FUNC_RESULT(res, 121, exit);

  /*snoop mirror table 1*/
  table_entry[0] = table_entry[1] = 0;
  res = jer2_arad_fill_table_with_entry(unit, IRR_SNOOP_MIRROR_TABLE_1m, MEM_BLOCK_ANY, table_entry); /* fill table with the entry */
  DNX_SAND_CHECK_FUNC_RESULT(res, 122, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_irr_tbls_init()",0,0);
}

static uint32
  jer2_arad_mgmt_iqm_tbls_init(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 silent
  )
{
  uint32
    res = DNX_SAND_OK,
    table_entry[1],
    cnred_table_entry[4] = {0};
  DNX_SAND_RET
    ret = DNX_SAND_OK;
  JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_DATA
    iqm_vsq_flow_control_parameters_table_group_tbl_data;
  JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_DATA
    iqm_packet_queue_red_parameters_table_tbl_data;
  JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA
    iqm_vsq_queue_parameters_table_group_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_IQM_TBLS_INIT);
  DNX_SAND_PCID_LITE_SKIP(unit);

  ret = dnx_sand_os_memset(&iqm_packet_queue_red_parameters_table_tbl_data, 0x0, sizeof(iqm_packet_queue_red_parameters_table_tbl_data));
  DNX_SAND_CHECK_FUNC_RESULT(ret, 19, exit);

  ret = dnx_sand_os_memset(&iqm_vsq_queue_parameters_table_group_tbl_data, 0x0, sizeof(iqm_vsq_queue_parameters_table_group_tbl_data));
  DNX_SAND_CHECK_FUNC_RESULT(ret, 36, exit);

  ret = dnx_sand_os_memset(&iqm_vsq_flow_control_parameters_table_group_tbl_data, 0x0, sizeof(iqm_vsq_flow_control_parameters_table_group_tbl_data));
  DNX_SAND_CHECK_FUNC_RESULT(ret, 37, exit);

  /*Packet Queue Red Weight table*/
  table_entry[0] = 0;
  soc_mem_field32_set(unit, IQM_PQWQm, table_entry, PQ_WEIGHTf, 2);
  soc_mem_field32_set(unit, IQM_PQWQm, table_entry, PQ_AVRG_ENf, 1); /* Needed to enable WRED for the rate class */
  res = jer2_arad_fill_table_with_entry(unit, IQM_PQWQm, MEM_BLOCK_ANY, table_entry); /* fill table with the entry */
  DNX_SAND_CHECK_FUNC_RESULT(res, 110, exit);

  soc_mem_field32_set(unit, IQM_CNREDm, cnred_table_entry, PQ_MAX_QUE_SIZEf, 0x7FF);
  soc_mem_field32_set(unit, IQM_CNREDm, cnred_table_entry, PQ_MAX_QUE_BUFF_SIZEf, 0x7FF);
  soc_mem_field32_set(unit, IQM_CNREDm, cnred_table_entry, PQ_AVRG_MAX_THf, 0x7FF);
  res = jer2_arad_fill_table_with_entry(unit, IQM_CNREDm, MEM_BLOCK_ANY, cnred_table_entry); /* fill table with the entry */
  DNX_SAND_CHECK_FUNC_RESULT(res, 120, exit);


  /*Packet Queue Red parameters table*/
  res = jer2_arad_iqm_packet_queue_red_parameters_table_tbl_fill_unsafe(unit, &iqm_packet_queue_red_parameters_table_tbl_data);
  DNX_SAND_CHECK_FUNC_RESULT(res, 152, exit);

  {

  res = jer2_arad_iqm_vsq_queue_parameters_table_group_tbl_fill_unsafe(unit, &iqm_vsq_queue_parameters_table_group_tbl_data);
         DNX_SAND_CHECK_FUNC_RESULT(res, 152, exit);

  iqm_vsq_flow_control_parameters_table_group_tbl_data.avrg_size_en = TRUE;
  res = jer2_arad_iqm_vsq_flow_control_parameters_table_group_tbl_fill_unsafe(unit, &iqm_vsq_flow_control_parameters_table_group_tbl_data);
                DNX_SAND_CHECK_FUNC_RESULT(res, 153, exit);
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_iqm_tbls_init()",0,0);
}

uint32
  jer2_arad_mgmt_ips_tbls_init(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 silent
  )
{
  uint32 res = DNX_SAND_OK;
  uint32 table_entry[2] = {0};

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_IPS_TBLS_INIT);
  DNX_SAND_PCID_LITE_SKIP(unit);

  /*Destination Device And Port Lookup Tables*/

  /* Both QPM_{1,2} tables are initialized to all 1s */
  /* in case of direct VOQ mapping mode, create the modport2sysport mapping  data structure */
  if (JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit)) { /* indirect VOQ mapping mode */

    /* Here QPM_1 maps from base queue to system phys port and QPM_2 maps from there to device x port */
    /* we want the entry to be disabled (all 1s queue) and have a traffic class profile of 0 */
    soc_mem_field32_set(unit, IPS_QPM_1_SYS_REDm, table_entry, SYS_PHY_PORTf, DNX_TMC_MAX_SYSTEM_PHYSICAL_PORT_ID); /* mark as disabled entry 0xfff */
    res = jer2_arad_fill_table_with_entry(unit, IPS_QPM_1_SYS_REDm, IPS_BLOCK(unit, SOC_CORE_ALL), table_entry); /* fill table with the entry */
    DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    table_entry[0] = 0; /* we want the entry to be disabled (all 1s queue) and have a traffic class profile of 0 */
    soc_mem_field32_set(unit, IPS_QPM_2_SYS_REDm, table_entry, DEST_PORTf, JER2_ARAD_MAX_FAP_PORT_ID);
    soc_mem_field32_set(unit, IPS_QPM_2_SYS_REDm, table_entry, DEST_DEVf, JER2_ARAD_MAX_FAP_ID);

  } else { /* direct VOQ mapping mode */

    /* Here QPM_1 abd QPM_2 map together from base queue to device x port */
    /* we want the entry to be disabled (all 1s queue) and have a traffic class profile of 0 */
    soc_mem_field32_set(unit, IPS_QPM_1_NO_SYS_REDm, table_entry, DEST_DEVf, JER2_ARAD_MAX_FAP_ID);
    soc_mem_field32_set(unit, IPS_QPM_1_NO_SYS_REDm, table_entry, DEST_PORT_MSBf, 1); /* mark as disabled entry */
    res = jer2_arad_fill_table_with_entry(unit, IPS_QPM_1_NO_SYS_REDm, IPS_BLOCK(unit, SOC_CORE_ALL), table_entry); /* fill table with the entry */
    DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);

    table_entry[0] = 0xfffffff; /* 4 7bit fields each containing 0x7f */

  }
  /* fill all of QPM_2, even entries not used in indirect mapping mode to be sure we won't get parity/ECC errors on a wrong access to it */
  res = jer2_arad_fill_table_with_entry(unit, IPS_QPM_2_NO_SYS_REDm, IPS_BLOCK(unit, SOC_CORE_ALL), table_entry);
  DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);
  /*
   * Mark whole table as 'invalid'. See jer2_arad_interrupt_handles_corrective_action_ips_qdesc().
   */
  table_entry[0] = JER2_ARAD_IPQ_INVALID_FLOW_QUARTET ;
  res = jer2_arad_fill_table_with_entry(unit, IPS_FLWIDm, IPS_BLOCK(unit, SOC_CORE_ALL), table_entry) ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 90, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_ips_tbls_init()",0,0);
}

uint32
  jer2_arad_mgmt_ipt_tbls_init(
      DNX_SAND_IN  int                 unit,
      DNX_SAND_IN  uint8                 silent
    )
{
    int i;
    uint32 res;

    uint32 fabric_priority;
    uint32 table_default[SOC_MAX_MEM_WORDS];

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_IPT_TBLS_INIT);
    DNX_SAND_PCID_LITE_SKIP(unit);



    if (!SOC_UNIT_NUM_VALID(unit)) {
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_ILLEGAL_DEVICE_ID, 4, exit);
    }

    res = dnx_sand_os_memset(&table_default, 0x0, sizeof(table_default));
    DNX_SAND_CHECK_FUNC_RESULT(res, 27, exit);

    /*0 - 3 - fabric priorities*/
    for(i=0;i<JER2_ARAD_FBC_PRIORITY_NDX_NOF ; i++) {
        uint32 is_tdm, tc;
        /*params according to i*/
        /*dp = (i & JER2_ARAD_FBC_PRIORITY_NDX_DP_MASK) >> JER2_ARAD_FBC_PRIORITY_NDX_DP_OFFSET;*/
        is_tdm = (i & JER2_ARAD_FBC_PRIORITY_NDX_IS_TDM_MASK) >> JER2_ARAD_FBC_PRIORITY_NDX_IS_TDM_OFFSET;
        tc = (i & JER2_ARAD_FBC_PRIORITY_NDX_TC_MASK) >> JER2_ARAD_FBC_PRIORITY_NDX_TC_OFFSET;
        /*is_hp = (i & JER2_ARAD_FBC_PRIORITY_NDX_IS_HP_MASK) >> JER2_ARAD_FBC_PRIORITY_NDX_IS_HP_OFFSET;*/

        if(is_tdm) {
            fabric_priority = 3;
        } else {/*according to tc*/
            /*tc=0, 1, 2 ==> prio=0*/
            /*tc=3, 4, 5 ==> prio=1*/
            /*tc=6, 7    ==> prio=2/prio=1 for is_tdm_over_primary_pipe*/
            fabric_priority = tc/3;
            if ((!SOC_IS_JERICHO(unit)) && (SOC_DNX_CONFIG(unit)->tdm.is_tdm_over_primary_pipe) && (fabric_priority==2)) {
                /*Disable priority 2 - saved for TDM over primary pipe*/
                fabric_priority = 1;
            }
        }
        if (SOC_IS_JERICHO(unit)) {
            res = WRITE_IPT_PRIORITY_BITS_MAPPING_2_FDTm(unit, MEM_BLOCK_ALL, i, &fabric_priority);
            DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 25, exit);
            res = WRITE_IPT_TDM_BIT_MAPPING_2_FDTm(unit, MEM_BLOCK_ALL, i, &is_tdm);
            DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit);
        } else {
            SHR_BITCOPY_RANGE(table_default, i*JER2_ARAD_FBC_PRIORITY_LENGTH, &fabric_priority, 0, JER2_ARAD_FBC_PRIORITY_LENGTH);
        }
    }
    if (!(SOC_IS_JERICHO(unit))) {
        res = WRITE_IPT_PRIORITY_BITS_MAPPING_2_FDTm(unit, MEM_BLOCK_ALL, 0, table_default);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
    }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_ipt_tbls_init()",0,0);
}

uint32
  jer2_arad_mgmt_egq_tbls_init(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 silent
  )
{
  uint32 res = DNX_SAND_OK;
  uint32 table_entry[SOC_DNX_IMP_DEFS_MAX(EGQ_PPCT_NOF_LONGS)];
  soc_reg_above_64_val_t data;
  uint32 mtu_table_entry[4] = {0, 0, 0xFE000000, 0x7F}; /*setting the MTU to max value*/


  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_EGQ_TBLS_INIT);

  sal_memset(table_entry, 0x0, sizeof(table_entry));

  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {

      table_entry[0] = 127; /* Invalid */
      res = jer2_arad_fill_table_with_entry(unit, EGQ_FQP_NIF_PORT_MUXm, MEM_BLOCK_ANY, table_entry);
      DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      table_entry[0] = 32; /* Invalid */
      res = jer2_arad_fill_table_with_entry(unit, EGQ_PQP_NIF_PORT_MUXm, MEM_BLOCK_ANY, table_entry);
      DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      res = jer2_arad_fill_table_with_entry(unit, EGQ_PP_PPCTm, MEM_BLOCK_ANY, &mtu_table_entry);
      DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  }

  SOC_REG_ABOVE_64_CLEAR(data);
  soc_mem_field32_set(unit, EGQ_PPCTm, data, LB_KEY_MAXf, 0xff);
  soc_mem_field32_set(unit, EGQ_PPCTm, data, CGM_PORT_PROFILEf, JER2_ARAD_EGR_PORT_THRESH_TYPE_15);
  res = jer2_arad_fill_table_with_entry(unit, EGQ_PPCTm, MEM_BLOCK_ANY, data); /* fill table with the entry */
  DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

  SOC_REG_ABOVE_64_CLEAR(data);
  soc_mem_field32_set(unit, EGQ_PCTm, data, CGM_PORT_PROFILEf, JER2_ARAD_EGR_PORT_THRESH_TYPE_15);
  res = jer2_arad_fill_table_with_entry(unit, EGQ_PCTm, MEM_BLOCK_ANY, data); /* fill table with the entry */
  DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);

  SOC_REG_ABOVE_64_CLEAR(data);
  soc_mem_field32_set(unit, EGQ_DSP_PTR_MAPm, data, OUT_TM_PORTf, JER2_ARAD_EGR_INVALID_BASE_Q_PAIR);
  res  = jer2_arad_fill_table_with_entry(unit, EGQ_DSP_PTR_MAPm, MEM_BLOCK_ANY, data);
  DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_egq_tbls_init()",0,0);
}

static uint32
  jer2_arad_mgmt_sch_tbls_init(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 silent
  )
{
  uint32
    table_entry[2] = {0},
    res = DNX_SAND_OK;
  JER2_ARAD_SCH_SCHEDULER_INIT_TBL_DATA
  sch_scheduler_init_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_SCH_TBLS_INIT);
  DNX_SAND_PCID_LITE_SKIP(unit);

  res = jer2_arad_fill_table_with_entry(unit, SCH_SCHEDULER_ENABLE_MEMORY_SEMm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (SOC_IS_JERICHO(unit) && !SOC_IS_QUX(unit))
  {
      res = jer2_arad_fill_table_with_entry(unit, SCH_SCHEDULER_ENABLE_MEMORY_SEM_Bm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      DNX_SAND_CHECK_FUNC_RESULT(res, 17, exit);
  }

  /* SHC */
  /* all relevant bits initialized to 0 */
  res = jer2_arad_fill_table_with_entry(unit, SCH_HR_SCHEDULER_CONFIGURATION_SHCm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* SCT */
  /* all relevant bits initialized to 0 */
  res = jer2_arad_fill_table_with_entry(unit, SCH_CL_SCHEDULERS_TYPE__SCTm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  /* FGM */
  /* all relevant bits initialized to 0 */
  res = jer2_arad_fill_table_with_entry(unit, SCH_FLOW_GROUP_MEMORY_FGMm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  /* DSPP */
  soc_mem_field32_set(unit, SCH_DSP_2_PORT_MAP_DSPPm, table_entry, DSP_2_PORT_MAP_DSPPf, JER2_ARAD_EGR_INVALID_BASE_Q_PAIR);
  res = jer2_arad_fill_table_with_entry(unit, SCH_DSP_2_PORT_MAP_DSPPm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
  DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  soc_mem_field32_set(unit, SCH_DSP_2_PORT_MAP_DSPPm, table_entry, DSP_2_PORT_MAP_DSPPf, 0);

  /*Scheduler Init*/
  sch_scheduler_init_tbl_data.schinit = 0x1;

  {
      res = jer2_arad_sch_scheduler_init_tbl_set_unsafe(
            unit,
            0,
            &sch_scheduler_init_tbl_data
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 132, exit);

  }

  /* SCH_FLOW_TO_FIP_MAPPING */
  /* all relevant bits initialized to DNX_TMC_MAX_FAP_ID */
  soc_mem_field32_set(unit, SCH_FLOW_TO_FIP_MAPPING__FFMm, table_entry, DEVICE_NUMBERf, DNX_TMC_MAX_FAP_ID);
  res = jer2_arad_fill_table_with_entry(unit, SCH_FLOW_TO_FIP_MAPPING__FFMm, MEM_BLOCK_ANY, table_entry);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  soc_mem_field32_set(unit, SCH_FLOW_TO_FIP_MAPPING__FFMm, table_entry, DEVICE_NUMBERf, 0);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_sch_tbls_init()",0,0);
}

uint32
  jer2_arad_mgmt_tbls_init(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 silent
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_TBLS_INIT);



  res = jer2_arad_mgmt_sch_tbls_init(unit, silent);
  DNX_SAND_CHECK_FUNC_RESULT(res, 28, exit);

  res = jer2_arad_mgmt_irr_tbls_init(unit, silent);
  DNX_SAND_CHECK_FUNC_RESULT(res, 14, exit);

  res = jer2_arad_mgmt_ire_tbls_init(unit, silent);
  DNX_SAND_CHECK_FUNC_RESULT(res, 12, exit);

  res = jer2_arad_mgmt_iqm_tbls_init(unit, silent);
  DNX_SAND_CHECK_FUNC_RESULT(res, 17, exit);

  res = jer2_arad_mgmt_ips_tbls_init(unit, silent);
  DNX_SAND_CHECK_FUNC_RESULT(res, 19, exit);

  res = jer2_arad_mgmt_ipt_tbls_init(unit, silent);
  DNX_SAND_CHECK_FUNC_RESULT(res, 21, exit);

  res = jer2_arad_mgmt_egq_tbls_init(unit, silent);
  DNX_SAND_CHECK_FUNC_RESULT(res, 25, exit);


  JER2_ARAD_DO_NOTHING_AND_EXIT;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_tbls_init()", 0, 0);
}

static
uint32
  jer2_arad_mgmt_all_tbls_init_enable_dynamic(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 enable
  )
{
  soc_reg_t
    enable_dynamic_memory_access[11] = {
      CRPS_ENABLE_DYNAMIC_MEMORY_ACCESSr, EGQ_ENABLE_DYNAMIC_MEMORY_ACCESSr,
      EPNI_ENABLE_DYNAMIC_MEMORY_ACCESSr, IDR_ENABLE_DYNAMIC_MEMORY_ACCESSr,
      IHB_ENABLE_DYNAMIC_MEMORY_ACCESSr, IPS_ENABLE_DYNAMIC_MEMORY_ACCESSr,
       IPT_ENABLE_DYNAMIC_MEMORY_ACCESSr, IQM_ENABLE_DYNAMIC_MEMORY_ACCESSr,
       IRE_ENABLE_DYNAMIC_MEMORY_ACCESSr, IRR_ENABLE_DYNAMIC_MEMORY_ACCESSr,
       OLP_ENABLE_DYNAMIC_MEMORY_ACCESSr
      };
  uint32
    res,
    memory_access_id = 0;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  for(memory_access_id = 0; memory_access_id < 11; memory_access_id++)
  {
      if (SOC_IS_ARDON(unit) && enable_dynamic_memory_access[memory_access_id] == OLP_ENABLE_DYNAMIC_MEMORY_ACCESSr) {
          /* Skip this register on Ardon */
          continue;
      }
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10+memory_access_id,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg32_set(unit, enable_dynamic_memory_access[memory_access_id], REG_PORT_ANY,  0,  enable));
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_all_tbls_init_enable_dynamic()",0,0);
}


/*
 * Init ALL Arad non read only  non wide tables that are not initialized elsewhere
 * Currently all tables appear here.
 * Tables initialized to non zero elsewhere are ifdefed out.
 * Wide tables (entry > 640 bits) can not be accessed by regular memory access functions.
 * Removed tables who are not read only, but all their fields are read only.
 */

uint32
  jer2_arad_mgmt_all_tbls_init(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 silent
  )
{
  uint32 res, errnum;
  uint32 table_entry[128] = {0};
  uint32 vsi_low_cfg[1] = {0};
  soc_mem_t mem_lcl;
  soc_error_t new_rv;


  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  new_rv = jer2_arad_tbl_access_init_unsafe(unit);
  if (new_rv != SOC_E_NONE) {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 55555, exit);
  }

  res = jer2_arad_mgmt_all_tbls_init_enable_dynamic(unit, 1);
  DNX_SAND_CHECK_FUNC_RESULT(res, 5, exit);


  errnum = 1000; /* BRDC */
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {

      errnum = 2000; /* CFC */
      res = jer2_arad_fill_table_with_entry(unit, CFC_CAT_2_TC_MAP_HCFCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CFC_CAT_2_TC_MAP_NIFm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CFC_ILKN_RX_0_CALm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CFC_ILKN_RX_1_CALm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CFC_ILKN_TX_0_CALm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CFC_ILKN_TX_1_CALm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CFC_NIF_PFC_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CFC_RCL_VSQ_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CFC_SPI_OOB_RX_0_CALm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CFC_SPI_OOB_RX_1_CALm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CFC_SPI_OOB_TX_0_CALm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CFC_SPI_OOB_TX_1_CALm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      if (SOC_IS_ARDON(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, CFC_NIF_SHR_MAPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }

      errnum = 3000; /* CRPS */
      res = jer2_arad_fill_table_with_entry(unit, CRPS_CRPS_0_CNTS_MEMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CRPS_CRPS_0_OVTH_MEMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CRPS_CRPS_1_CNTS_MEMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CRPS_CRPS_1_OVTH_MEMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CRPS_CRPS_2_CNTS_MEMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CRPS_CRPS_2_OVTH_MEMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CRPS_CRPS_3_CNTS_MEMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CRPS_CRPS_3_OVTH_MEMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CRPS_MEM_0080000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CRPS_MEM_0090000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CRPS_MEM_00A0000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, CRPS_MEM_00B0000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

#ifdef BCM_88660_A0
      if (SOC_IS_ARADPLUS(unit) && !SOC_IS_ARDON(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, CRPS_IRPP_OFFSET_BMAP_Am, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_IRPP_OFFSET_BMAP_Bm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_EGQ_OFFSET_BMAPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_EPNI_OFFSET_BMAP_Am, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_EPNI_OFFSET_BMAP_Bm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_IQM_OFFSET_BMAP_Am, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_IQM_OFFSET_BMAP_Bm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_IQM_OFFSET_BMAP_Cm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_IQM_OFFSET_BMAP_Dm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      } else
#endif
      {
          res = jer2_arad_fill_table_with_entry(unit, CRPS_MEM_00C0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_MEM_00D0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_MEM_00E0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_MEM_00F0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_MEM_0100000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_MEM_0110000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_MEM_0120000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_MEM_0130000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, CRPS_MEM_0140000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
    }

      errnum = 5000; /* EGQ */
      res = jer2_arad_fill_table_with_entry(unit, EGQ_ACTION_PROFILE_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_CBMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_CCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_CH_0_SCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_CH_1_SCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_CH_2_SCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_CH_3_SCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_CH_4_SCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_CH_5_SCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_CH_6_SCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_CH_7_SCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_CH_8_SCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_CH_9_SCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_CNM_QUANTA_TO_FC_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_DCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_DSP_PTR_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      if (SOC_IS_JERICHO(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, EGQ_DSP_PTR_MAPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
      res = jer2_arad_fill_table_with_entry(unit, EGQ_DWMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_DWM_8Pm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_EPS_PRIO_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_FBMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_FDMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_HEADER_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_MAP_OUTLIF_TO_DSPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_MAP_PS_TO_IFCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_MC_SP_TC_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_NONCH_SCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_PCTm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_PDCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_PDCMAXm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_PDCT_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_PMCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_PMF_KEY_GEN_LSBm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_PMF_KEY_GEN_MSBm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_PMF_PROGRAM_SELECTION_CAMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_PPCTm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_PP_PPCTm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_PQSMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_PQSMAXm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_PQST_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_QDCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_QDCMAXm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_QDCT_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_QM_0m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_QM_1m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_QM_2m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_QM_3m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_QP_CBMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_QP_PMCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_QP_SCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_QQSMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_QQSMAXm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_QQST_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_RCMMCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_RCMUCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_RPDMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_RRDMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_TCG_CBMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_TCG_PMCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_TCG_SCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_TC_DP_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_TTL_SCOPEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EGQ_VLAN_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      if (!SOC_IS_ARDON(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, EGQ_VSI_MEMBERSHIPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, EGQ_AUX_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
      res = jer2_arad_fill_table_with_entry(unit, EGQ_VSI_PROFILEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }
      res = jer2_arad_fill_table_with_entry(unit, EGQ_IVEC_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
      errnum = 6000; /* EPNI */
      res = jer2_arad_fill_table_with_entry(unit, EPNI_ACE_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_ACE_TO_FHEIm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_ACE_TO_OUT_LIFm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_ACE_TO_OUT_PP_PORTm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_COUNTER_SOURCE_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_DSCP_EXP_TO_PCP_DEIm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      /* PCP_DSCP_EXP_TO_PCP_DEI and NATIVE_PCP_DSCP_EXP_TO_PCP_DEI are duplicated */
      if (SOC_IS_JERICHO_PLUS(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, EPNI_NATIVE_DSCP_EXP_TO_PCP_DEIm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
  if (!SOC_IS_ARDON(unit)) {
      res = jer2_arad_fill_table_with_entry(unit, EDB_ESEM_MANAGEMENT_REQUESTm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EDB_ESEM_STEP_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }
      res = jer2_arad_fill_table_with_entry(unit, EPNI_ETH_OAM_OPCODE_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_EVEC_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      /* EVEC and native EVEC are duplicated */
      if (SOC_IS_JERICHO_PLUS(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, EPNI_NATIVE_EVEC_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
      res = jer2_arad_fill_table_with_entry(unit, EPNI_EXP_REMARKm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_HEADER_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_LFEM_FIELD_SELECT_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_LINK_LAYER_VLAN_PROCESSING_LLVPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }
  errnum += 10;
  res = jer2_arad_fill_table_with_entry(unit, EPNI_IVEC_TABLEm, MEM_BLOCK_ANY, table_entry);
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

  if ((SOC_DNX_CONFIG(unit)->emulation_system == 0) && (!SOC_IS_ARDON(unit))) {

      res = jer2_arad_fill_table_with_entry(unit, EPNI_MEM_520000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_MEM_5A0000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_MEM_5B0000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_MEM_630000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_MEM_640000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_MEM_6C0000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_MEM_6D0000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_MEM_760000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_MEM_770000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
  }
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {

      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_MIRROR_PROFILE_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_MIRROR_PROFILE_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_MPLS_CMD_PROFILEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_MY_CFM_MAC_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PCP_DEI_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      /* PCP_DEI_TABLE and NATIVE_PCP_DEI_TABLE are duplicated */
      if (SOC_IS_JERICHO_PLUS(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, EPNI_NATIVE_PCP_DEI_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PP_COUNTER_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PACKET_PROCESSING_PORT_CONFIGURATION_TABLE_PP_PCTm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PRGE_DATAm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PRGE_INSTRUCTION_0m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PRGE_INSTRUCTION_1m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PRGE_INSTRUCTION_10m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PRGE_INSTRUCTION_11m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PRGE_INSTRUCTION_2m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PRGE_INSTRUCTION_3m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PRGE_INSTRUCTION_4m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PRGE_INSTRUCTION_5m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PRGE_INSTRUCTION_6m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PRGE_INSTRUCTION_7m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PRGE_INSTRUCTION_8m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PRGE_INSTRUCTION_9m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PRGE_PROGRAMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_PRGE_PROGRAM_SELECTION_CAMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_REMARK_MPLS_TO_DSCPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_REMARK_MPLS_TO_EXPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, EPNI_SPANNING_TREE_PROTOCOL_STATE_MEMORY_STPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      if (!SOC_IS_ARDON(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, EPNI_DSCP_REMARKm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, EPNI_EEDB_BANKm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, EPNI_ISID_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, EPNI_REMARK_IPV4_TO_DSCPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, EPNI_REMARK_IPV4_TO_EXPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, EPNI_REMARK_IPV6_TO_DSCPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, EPNI_REMARK_IPV6_TO_EXPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, EPNI_TX_TAG_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
  }

  errnum = 9000; /* FDT */
  res = jer2_arad_fill_table_with_entry(unit, FDT_IN_BAND_MEMm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, FDT_IRE_TDM_MASKSm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {

      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, FDT_MEM_100000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }

  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {

      errnum = 11000; /* FSRD */
      res = jer2_arad_fill_table_with_entry(unit, FSRD_FSRD_WL_EXT_MEMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

      errnum = 12000; /* IDR */
      res = jer2_arad_fill_table_with_entry(unit, IDR_CONTEXT_MRUm, MEM_BLOCK_ANY, table_entry); /* Overwritten later */
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IDR_DROP_PRECEDENCE_MAPPINGm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IDR_ETHERNET_METER_CONFIGm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IDR_ETHERNET_METER_PROFILESm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IDR_GLOBAL_METER_PROFILESm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IDR_GLOBAL_METER_STATUSm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      if (!SOC_IS_ARDON(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, IDR_CONTEXT_COLORm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IDR_CONTEXT_SIZEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IDR_MCDA_DYNAMICm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IDR_MCDA_PRFCFG_0m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IDR_MCDA_PRFSELm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
  }
  if (!SOC_IS_ARDON(unit)) {
      if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
          res = jer2_arad_fill_table_with_entry(unit, IDR_MCDB_DYNAMICm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
      res = jer2_arad_fill_table_with_entry(unit, IDR_MCDB_PRFCFG_0m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IDR_MCDB_PRFSELm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
      res = jer2_arad_fill_table_with_entry(unit, IDR_MEM_00000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }

  res = jer2_arad_fill_table_with_entry(unit, IDR_MEM_10000m, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {

      res = jer2_arad_fill_table_with_entry(unit, IDR_MEM_180000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IDR_MEM_1B0000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IDR_MEM_1F0000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IDR_MEM_30000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IDR_MEM_40000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IDR_MEM_50000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IDR_MEM_60000m, MEM_BLOCK_ANY, table_entry);
  }
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {

      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IDR_OCB_BUFFER_TYPEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

      errnum = 13000; /* IHB */
      res = jer2_arad_fill_table_with_entry(unit, IHB_CPU_TRAP_CODE_CTRm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

      if (!SOC_IS_ARDON(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, IHB_DESTINATION_STATUSm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_DSCP_EXP_MAPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_DSCP_EXP_REMARKm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_ETHERNET_OAM_OPCODE_MAPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_FEC_ECMPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_FEC_ENTRYm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_FEC_ENTRY_ACCESSEDm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, SOC_IS_ARADPLUS_AND_BELOW(unit)?IHB_FEC_SUPER_ENTRYm:PPDB_A_FEC_SUPER_ENTRY_BANKm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
  }
  res = jer2_arad_fill_table_with_entry(unit, IHB_FEM_BIT_SELECTm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
      res = jer2_arad_fill_table_with_entry(unit, IHB_FWD_ACT_PROFILEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_HEADER_PROFILEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_IN_PORT_KEY_GEN_VARm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_LB_PFC_PROFILEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_LB_VECTOR_PROGRAM_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      if (!SOC_IS_ARDON(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, IHB_FLP_KEY_CONSTRUCTIONm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_FLP_LOOKUPSm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_FLP_PROCESSm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_FLP_PROGRAM_KEY_GEN_VARm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
#ifdef BCM_88690_SUPPORT
          if(SOC_IS_JERICHO(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, IHP_FLP_PROGRAM_SELECTION_CAMm, MEM_BLOCK_ANY, table_entry);
          } else
#endif
          {
              res = jer2_arad_fill_table_with_entry(unit, IHB_FLP_PROGRAM_SELECTION_CAMm, MEM_BLOCK_ANY, table_entry);
          }
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_FLP_PTC_PROGRAM_SELECTm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_IEEE_1588_ACTIONm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_IEEE_1588_IDENTIFICATION_CAMm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_LPMm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_LPM_2m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_LPM_3m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_LPM_4m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_LPM_5m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_LPM_6m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_1040000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_1050000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_10E0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_10F0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_14A0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_1520000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_1530000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_15B0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_15C0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_1640000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_1650000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_16E0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_16F0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_250000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          if(SOC_IS_ARADPLUS_AND_BELOW(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_260000m, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_270000m, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          }
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_E00000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_E10000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_EA0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_F20000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_F30000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_FB0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_MEM_FC0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
      }
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_MRR_ACT_PROFILEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_OAM_COUNTER_FIFOm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      
      res = jer2_arad_fill_table_with_entry(unit, IHB_L_4_OPSm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PFC_INFOm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PINFO_COUNTERSm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PINFO_FERm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PINFO_LBPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PINFO_PMFm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PMF_FEM_PROGRAMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PMF_INITIAL_KEY_2ND_PASSm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PMF_PASS_1_KEY_GEN_LSBm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PMF_PASS_1_KEY_GEN_MSBm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PMF_PASS_1_LOOKUPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PMF_PASS_2_KEY_GEN_LSBm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PMF_PASS_2_KEY_GEN_MSBm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PMF_PASS_2_KEY_UPDATEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PMF_PASS_2_LOOKUPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PMF_PROGRAM_COUNTERSm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PMF_PROGRAM_GENERALm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PMF_PROGRAM_SELECTION_CAMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PROGRAM_KEY_GEN_VARm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PTC_INFO_PMFm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_PTC_KEY_GEN_VARm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_SNOOP_ACTIONm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_SNP_ACT_PROFILEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACCESS_PROFILEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTIONm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      if (!SOC_IS_ARDON(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, IHP_MY_BFD_DIPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_OAMAm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_OAMBm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_OAM_CHANNEL_TYPEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_OAM_MY_CFM_MACm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, PPDB_A_OEMA_MANAGEMENT_REQUESTm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, PPDB_A_OEMA_STEP_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, PPDB_A_OEMB_MANAGEMENT_REQUESTm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, PPDB_A_OEMB_STEP_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_OPCODE_MAP_RXm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_OPCODE_MAP_TXm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          if(SOC_IS_JERICHO(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, IHP_PINFO_FLP_0m, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              res = jer2_arad_fill_table_with_entry(unit, IHP_PINFO_FLP_1m, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          } else {
              res = jer2_arad_fill_table_with_entry(unit, IHB_PINFO_FLPm, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          }
          if(SOC_IS_ARADPLUS_AND_BELOW(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, IHB_TCAM_ACTION_24m, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              res = jer2_arad_fill_table_with_entry(unit, IHB_TCAM_ACTION_25m, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              res = jer2_arad_fill_table_with_entry(unit, IHB_TCAM_ACTION_26m, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              res = jer2_arad_fill_table_with_entry(unit, IHB_TCAM_ACTION_27m, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          } else {
#ifdef BCM_88690_SUPPORT
              if(SOC_IS_JERICHO(unit)) {
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_SMALL_24m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_SMALL_25m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_SMALL_26m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_SMALL_27m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_SMALL_28m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_SMALL_29m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_SMALL_30m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_SMALL_31m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              }
#endif
          }
          res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_HIT_INDICATIONm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          if(SOC_IS_ARADPLUS_AND_BELOW(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, IHB_TCAM_ACTION_HIT_INDICATION_24m, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              res = jer2_arad_fill_table_with_entry(unit, IHB_TCAM_ACTION_HIT_INDICATION_25m, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              res = jer2_arad_fill_table_with_entry(unit, IHB_TCAM_ACTION_HIT_INDICATION_26m, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              res = jer2_arad_fill_table_with_entry(unit, IHB_TCAM_ACTION_HIT_INDICATION_27m, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          } else {
#ifdef BCM_88690_SUPPORT
              if(SOC_IS_JERICHO(unit)) {
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_HIT_INDICATION_SMALL_24m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_HIT_INDICATION_SMALL_25m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_HIT_INDICATION_SMALL_26m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_HIT_INDICATION_SMALL_27m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_HIT_INDICATION_SMALL_28m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_HIT_INDICATION_SMALL_29m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_HIT_INDICATION_SMALL_30m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ACTION_HIT_INDICATION_SMALL_31m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              }
#endif
          }
          if(SOC_IS_ARADPLUS_AND_BELOW(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, IHB_TCAM_ENTRY_PARITY_12m, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              res = jer2_arad_fill_table_with_entry(unit, IHB_TCAM_ENTRY_PARITY_13m, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          } else {
#ifdef BCM_88690_SUPPORT
              if(SOC_IS_JERICHO(unit)) {
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ENTRY_PARITY_SMALL_12m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ENTRY_PARITY_SMALL_13m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ENTRY_PARITY_SMALL_14m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
                  res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ENTRY_PARITY_SMALL_15m, MEM_BLOCK_ANY, table_entry);
                  errnum += 10;
                  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              }
#endif
          }
          res = jer2_arad_fill_table_with_entry(unit, IHB_UNKNOWN_DA_MAPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_VRF_CONFIGm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
      if (!SOC_IS_ARDON(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_BANKm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
      res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_ENTRY_PARITYm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, PPDB_A_TCAM_PD_PROFILEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHB_TIME_STAMP_FIFOm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

      errnum = 14000; /* IHP */
      if (!SOC_IS_ARDON(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, IHP_ACTION_PROFILE_MPLS_VALUEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_BVD_CFGm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_BVD_FID_CLASSm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_BVD_TOPOLOGY_IDm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_DEFAULT_COUNTER_SOURCEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_DESIGNATED_VLAN_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_FID_CLASS_2_FIDm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_INGRESS_VLAN_EDIT_COMMAND_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_IN_RIF_CONFIG_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          if (SOC_IS_JERICHO(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, IHB_ISEM_MANAGEMENT_REQUESTm, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              res = jer2_arad_fill_table_with_entry(unit, IHB_ISEM_STEP_TABLEm, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          } else {
              res = jer2_arad_fill_table_with_entry(unit, IHP_ISA_MANAGEMENT_REQUESTm, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              res = jer2_arad_fill_table_with_entry(unit, IHP_ISA_STEP_TABLEm, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              res = jer2_arad_fill_table_with_entry(unit, IHP_ISB_MANAGEMENT_REQUESTm, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
              res = jer2_arad_fill_table_with_entry(unit, IHP_ISB_STEP_TABLEm, MEM_BLOCK_ANY, table_entry);
              errnum += 10;
              DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          }
          res = jer2_arad_fill_table_with_entry(unit, IHP_LIF_ACCESSEDm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_LIF_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_AGING_CONFIGURATION_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_FID_COUNTER_DBm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_FID_COUNTER_PROFILE_DBm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_FID_PROFILE_DBm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_FLUSH_DBm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_FORMAT_0_TYPE_0m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_PORT_MINE_TABLE_LAG_PORTm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_PORT_MINE_TABLE_PHYSICAL_PORTm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_STEP_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
      res = jer2_arad_fill_table_with_entry(unit, IHP_LLR_LLVPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_LL_MIRROR_PROFILEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
      if (!SOC_IS_ARDON(unit)) {
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
#ifdef BCM_88690_A0
          if(SOC_IS_JERICHO(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_AGET_Hm, MEM_BLOCK_ANY, table_entry);
          } else
#endif /* BCM_88690_A0 */
          {
              res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_300000m, MEM_BLOCK_ANY, table_entry);
          }
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
#ifdef BCM_88690_A0
          if(SOC_IS_JERICHO(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_AGET_AUXm, MEM_BLOCK_ANY, table_entry);
          } else
#endif /* BCM_88690_A0 */
          {
              res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_380000m, MEM_BLOCK_ANY, table_entry);
          }
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
  }
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
      if (!SOC_IS_ARDON(unit)) {
#ifdef BCM_88690_A0
          if(SOC_IS_JERICHO(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_KEYT_PLDT_Hm, MEM_BLOCK_ANY, table_entry);
          } else
#endif /* BCM_88690_A0 */
          {
              res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_500000m, MEM_BLOCK_ANY, table_entry);
          }
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
#ifdef BCM_88690_A0
          if(SOC_IS_JERICHO(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_KEYT_AUXm, MEM_BLOCK_ANY, table_entry);
          } else
#endif /* BCM_88690_A0 */
          {
              res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_580000m, MEM_BLOCK_ANY, table_entry);
          }
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
#ifdef BCM_88690_A0
          if(SOC_IS_JERICHO(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_PLDT_AUXm, MEM_BLOCK_ANY, table_entry);
          } else
#endif /* BCM_88690_A0 */
          {
              res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_610000m, MEM_BLOCK_ANY, table_entry);
          }
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_620000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
#ifdef BCM_88690_A0
          if(SOC_IS_JERICHO(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_STEP_TABLEm, MEM_BLOCK_ANY, table_entry);
          } else
#endif /* BCM_88690_A0 */
          {
              res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_6A0000m, MEM_BLOCK_ANY, table_entry);
          }
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
#ifdef BCM_88690_A0
          if(SOC_IS_JERICHO(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_STEP_TABLEm, MEM_BLOCK_ANY, table_entry);
          } else
#endif /* BCM_88690_A0 */
          {
              res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_6B0000m, MEM_BLOCK_ANY, table_entry);
          }
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
#ifdef BCM_88690_A0
          if(SOC_IS_JERICHO(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_MAA_CAMm, MEM_BLOCK_ANY, table_entry);
          } else
#endif /* BCM_88690_A0 */
          {
              res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_740000m, MEM_BLOCK_ANY, table_entry);
          }
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
#ifdef BCM_88690_A0
          if(SOC_IS_JERICHO(unit)) {
              res = jer2_arad_fill_table_with_entry(unit, PPDB_B_LARGE_EM_MAA_CAM_PAYLOADm, MEM_BLOCK_ANY, table_entry);
          } else
#endif /* BCM_88690_A0 */
          {
              res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_750000m, MEM_BLOCK_ANY, table_entry);
          }
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_7A0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_820000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_830000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_8B0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_8C0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_940000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_950000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_9E0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_9F0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_A10000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_A90000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_AA0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_B20000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_B30000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_BB0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_BC0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_C50000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_MEM_C60000m, MEM_BLOCK_ANY, table_entry);
      }
  }
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {

      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PACKET_FORMAT_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PARSER_CUSTOM_MACRO_PARAMETERSm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PARSER_CUSTOM_MACRO_PROTOCOLSm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PARSER_CUSTOM_MACRO_WORD_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PARSER_ETH_PROTOCOLSm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PARSER_IPV4_NEXT_PROTOCOL_SIZEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PARSER_IPV6_NEXT_PROTOCOL_SIZEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PARSER_IP_PROTOCOLSm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PARSER_MPLS_NEXT_PROTOCOL_SPECULATE_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PARSER_PROGRAMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PARSER_PROGRAM_POINTER_FEM_BIT_SELECT_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PARSER_PROGRAM_POINTER_FEM_FIELD_SELECT_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PARSER_PROGRAM_POINTER_FEM_MAP_INDEX_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PFQ_0_FEM_BIT_SELECT_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PFQ_0_FEM_FIELD_SELECT_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PFQ_0_FEM_MAP_INDEX_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PINFO_LLRm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PORT_PROTOCOLm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PP_PORT_INFOm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PTC_INFOm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PTC_PARSER_PROGRAM_POINTER_CONFIGm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PTC_PFQ_0_CONFIGm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PTC_SYS_PORT_CONFIGm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_PTC_VIRTUAL_PORT_CONFIGm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_RECYCLE_COMMANDm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_RESERVED_MCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_SRC_SYSTEM_PORT_FEM_BIT_SELECT_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_SRC_SYSTEM_PORT_FEM_FIELD_SELECT_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_SRC_SYSTEM_PORT_FEM_MAP_INDEX_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_SUBNET_CLASSIFYm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_TOS_2_COSm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_VIRTUAL_PORT_FEM_BIT_SELECT_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_VIRTUAL_PORT_FEM_FIELD_SELECT_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_VIRTUAL_PORT_FEM_MAP_INDEX_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IHP_VIRTUAL_PORT_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      if (!SOC_IS_ARDON(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, IHP_STP_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_TC_DP_MAP_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_TERMINATION_PROFILE_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VLAN_EDIT_PCP_DEI_MAPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VSI_PORT_MEMBERSHIPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VLAN_RANGE_COMPRESSION_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VRID_MY_MAC_MAPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VRID_TO_VRF_MAPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VSI_HIGH_DA_NOT_FOUND_DESTINATIONm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VSI_HIGH_MY_MACm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VSI_HIGH_PROFILEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VSI_LOW_CFG_1m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          /* deafault FID = VSI */
          soc_mem_field32_set(unit, IHP_VSI_LOW_CFG_2m, vsi_low_cfg, FID_CLASSf, 7);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VSI_LOW_CFG_2m, MEM_BLOCK_ANY, vsi_low_cfg);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VTT_1ST_KEY_PROG_SEL_TCAMm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VTT_1ST_LOOKUP_PROGRAM_0m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VTT_1ST_LOOKUP_PROGRAM_1m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VTT_2ND_KEY_PROG_SEL_TCAMm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VTT_2ND_LOOKUP_PROGRAM_0m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VTT_2ND_LOOKUP_PROGRAM_1m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VTT_IN_PP_PORT_CONFIGm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VTT_IN_PP_PORT_VLAN_CONFIGm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VTT_LLVPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VTT_PP_PORT_TT_KEY_VARm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VTT_PP_PORT_VSI_PROFILESm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VTT_PP_PORT_VT_KEY_VARm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VTT_PTC_CONFIGm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }

      errnum = 15000; /* IPS */
      res = jer2_arad_fill_table_with_entry(unit, IPS_CRBALm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, (SOC_IS_JERICHO(unit) ? IPS_CRBALTHm : IPS_CRBALTHm), MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, (SOC_IS_JERICHO(unit) ? IPS_CRWDTHm : IPS_CRWDTHm), MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, (SOC_IS_JERICHO(unit) ? IPS_EMPTYQCRBALm : IPS_EMPTYQCRBALm), MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, (SOC_IS_JERICHO(unit)? IPS_FLWIDm : IPS_FLWIDm), MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IPS_MAXQSZm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
      res = jer2_arad_fill_table_with_entry(unit, IPS_MEM_180000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IPS_MEM_1A0000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IPS_MEM_1E0000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IPS_MEM_200000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IPS_MEM_220000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IPS_MEM_240000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }
  res = jer2_arad_fill_table_with_entry(unit, IPS_QPM_1_SYS_REDm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IPS_QPRISELm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IPS_QSZm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, (SOC_IS_JERICHO(unit) ? IPS_QSZTHm : IPS_QSZTHm), MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, (SOC_IS_JERICHO(unit)? IPS_QTYPEm : IPS_QTYPEm), MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IPS_Q_PRIORITY_BIT_MAPm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

  errnum = 16000; /* IPT */
  res = jer2_arad_fill_table_with_entry(unit, IPT_BDQm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IPT_EGQCTLm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IPT_EGQDATAm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IPT_FDTCTLm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IPT_FDTDATAm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IPT_GCI_BACKOFF_MASKm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IPT_ITM_TO_OTM_MAPm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IPT_MOP_MMUm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IPT_PCQm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IPT_PRIORITY_BITS_MAPPING_2_FDTm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IPT_SNP_MIR_CMD_MAPm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IPT_SOP_MMUm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

  errnum = 17000; /* IQM */
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
      res = jer2_arad_fill_table_with_entry(unit, IQM_BDBLLm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_CPDMDm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_CPDMSm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_CPPRMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_CRDTDISm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_DBFFMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_DELFFMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_FLUSCNTm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_GRSPRMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_ITMPMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_MEM_7E00000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_MNUSCNTm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_NIFTCMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_OCBPRMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
      res = jer2_arad_fill_table_with_entry(unit, IQM_PDMm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }

  res = jer2_arad_fill_table_with_entry(unit, IQM_PQDMDm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IQM_PQDMSm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IQM_PQREDm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
      res = jer2_arad_fill_table_with_entry(unit, IQM_PQWQm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }
  res = jer2_arad_fill_table_with_entry(unit, IQM_SCRBUFFTHm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IQM_SPRDPRMm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IQM_SRCQRNGm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IQM_SRDPRBm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IQM_TAILm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {

      res = jer2_arad_fill_table_with_entry(unit, IQM_VQFCPR_MAm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VQFCPR_MBm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VQFCPR_MCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VQFCPR_MDm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VQFCPR_MEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VQFCPR_MFm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VQPR_MAm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VQPR_MBm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VQPR_MCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VQPR_MDm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VQPR_MEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VQPR_MFm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VSQDRC_Am, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VSQDRC_Bm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VSQDRC_Cm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VSQDRC_Dm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VSQDRC_Em, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VSQDRC_Fm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

      res = jer2_arad_fill_table_with_entry(unit, IQM_VSQ_A_MX_OCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VSQ_B_MX_OCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VSQ_C_MX_OCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VSQ_D_MX_OCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VSQ_E_MX_OCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VSQ_F_MX_OCm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VS_QA_AVGm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VS_QA_QSZm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VS_QB_AVGm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VS_QB_QSZm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VS_QC_AVGm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VS_QC_QSZm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VS_QD_AVGm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VS_QD_QSZm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VS_QE_AVGm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VS_QE_QSZm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VS_QF_AVGm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IQM_VS_QF_QSZm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

      errnum = 18000; /* IRE */
  }
  res = jer2_arad_fill_table_with_entry(unit, IRE_CPU_CTXT_MAPm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IRE_CTXT_MEM_CONTROLm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IRE_NIF_CTXT_MAPm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IRE_NIF_PORT_MAPm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
      res = jer2_arad_fill_table_with_entry(unit, IRE_NIF_PORT_TO_CTXT_BIT_MAPm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

  res = jer2_arad_fill_table_with_entry(unit, IRE_RCY_CTXT_MAPm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

  errnum = 19000; /* IRR */
  res = jer2_arad_fill_table_with_entry(unit, IRR_FLOW_TABLEm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IRR_FREE_PCB_MEMORYm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IDR_IRDBm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IRR_ISF_MEMORYm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IRR_IS_FREE_PCB_MEMORYm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IRR_IS_PCB_LINK_TABLEm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IRR_LAG_MAPPINGm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IRR_LAG_NEXT_MEMBERm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IRR_LAG_TO_LAG_RANGEm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, IRR_MCR_MEMORYm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }

  /* These tables are not accessible in Arad+ */
  if (!SOC_IS_ARADPLUS(unit)) {
      res = jer2_arad_fill_table_with_entry(unit, IRR_MEM_300000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

      res = jer2_arad_fill_table_with_entry(unit, IRR_MEM_340000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IRR_MEM_3C0000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

  }

  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {

      res = jer2_arad_fill_table_with_entry(unit, IRR_PCB_LINK_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IRR_SMOOTH_DIVISIONm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IRR_STACK_FEC_RESOLVEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IRR_STACK_TRUNK_RESOLVEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, IRR_TRAFFIC_CLASS_MAPPINGm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit)

  }

  errnum = 20000; /* MMU */
  res = jer2_arad_fill_table_with_entry(unit, MMU_FDFm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  res = jer2_arad_fill_table_with_entry(unit, MMU_IDFm, MEM_BLOCK_ANY, table_entry);
  errnum += 10;
  DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);


  errnum = 21000; /* NBI */
  if ((SOC_DNX_CONFIG(unit)->emulation_system == 0) && (!SOC_IS_ARDON(unit))) {

      res = jer2_arad_fill_table_with_entry(unit, NBI_MEM_92000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, NBI_MEM_93000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, NBI_MEM_94000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, NBI_MEM_95000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }

  if (!SOC_IS_ARDON(unit)) {
      res = jer2_arad_fill_table_with_entry(unit, NBI_MLF_RX_MEM_A_CTRLm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, NBI_MLF_RX_MEM_B_CTRLm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, NBI_MLF_TX_MEM_CTRLm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

      errnum = 22000; /* OAMP */

      mem_lcl = SOC_IS_ARADPLUS(unit)? OAMP_MEM_20000m: OAMP_LOCAL_PORT_2_SYSTEM_PORTm;
      res = jer2_arad_fill_table_with_entry(unit, mem_lcl, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }

  if ((SOC_DNX_CONFIG(unit)->emulation_system == 0) && (!SOC_IS_ARDON(unit))) {

      res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_100000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_180000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_190000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_210000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_220000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_2A0000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_2B0000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_340000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_350000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_40000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_50000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_60000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      if (SOC_IS_ARAD_B1_AND_BELOW(unit)) {
          /* Arad-A0 and B0 only table */
          res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_70000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
      res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_80000m, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }
  if (!SOC_IS_ARDON(unit)) {
      if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
          res = jer2_arad_fill_table_with_entry(unit, OAMP_MEP_DBm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
      res = jer2_arad_fill_table_with_entry(unit, OAMP_RMAPEM_MANAGEMENT_REQUESTm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, OAMP_REMOTE_MEP_EXACT_MATCH_STEP_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
          res = jer2_arad_fill_table_with_entry(unit, OAMP_RMEP_DBm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
      res = jer2_arad_fill_table_with_entry(unit, OAMP_UMC_TABLEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

      errnum = 24000; /* OLP */
      res = jer2_arad_fill_table_with_entry(unit, OLP_DSP_EVENT_ROUTEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
          res = jer2_arad_fill_table_with_entry(unit, OLP_MEM_00000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
  }
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {

      errnum = 25000; /* RTP */
      res = jer2_arad_fill_table_with_entry(unit, RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_CTRL_CELLSm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_DATA_CELLSm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, RTP_UNICAST_DISTRIBUTION_MEMORYm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

      errnum = 26000; /* SCH */
      res = jer2_arad_fill_table_with_entry(unit, SCH_CH_NIF_CALENDAR_CONFIGURATION__CNCCm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_CH_NIF_RATES_CONFIGURATION__CNRCm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_CIR_SHAPER_CALENDAR__CSCm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_CL_SCHEDULERS_CONFIGURATION_SCCm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_CL_SCHEDULERS_TYPE__SCTm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);

      res = jer2_arad_fill_table_with_entry(unit, SCH_SHARED_DEVICE_RATE_SHARED_DRMm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      if(SOC_IS_JERICHO(unit)){
          /*
           * This memory block, per core, only exists on Jericho.
           * Its counterpart on Arad is SCH_SHARED_DEVICE_RATE_SHARED_DRM
           * which is also denoted as SCH_DEVICE_RATE_MEMORY__DRM
           */
          res = jer2_arad_fill_table_with_entry(unit, SCH_DEVICE_RATE_MEMORY_DRMm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
      res = jer2_arad_fill_table_with_entry(unit, SCH_DSP_2_PORT_MAP__DSPPm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_DUAL_SHAPER_MEMORY_DSMm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_FC_MAP_FCMm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_FLOW_DESCRIPTOR_MEMORY_STATIC_FDMSm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_FLOW_GROUP_MEMORY_FGMm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_FLOW_SUB_FLOW_FSFm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_FLOW_TO_FIP_MAPPING__FFMm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_FLOW_TO_QUEUE_MAPPING_FQMm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_FORCE_STATUS_MESSAGEm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_HR_SCHEDULER_CONFIGURATION_SHCm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }
  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {

      res = jer2_arad_fill_table_with_entry(unit, SCH_MEM_01F00000m, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_MEM_04700000m, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_MEM_04A00000m, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_MEM_04D00000m, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }

  if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {

      res = jer2_arad_fill_table_with_entry(unit, SCH_ONE_PORT_NIF_CONFIGURATION_OPNCm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_PIR_SHAPER_CALENDAR__PSCm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_PORT_ENABLE_PORTENm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_PORT_GROUP_PFGMm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_PORT_QUEUE_SIZE__PQSm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_PORT_SCHEDULER_MAP_PSMm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_PORT_SCHEDULER_WEIGHTS_PSWm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR__CALm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_SCHEDULER_ENABLE_MEMORY_SEMm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      if (SOC_IS_JERICHO(unit) && !SOC_IS_QUX(unit))
      {
          res = jer2_arad_fill_table_with_entry(unit, SCH_SCHEDULER_ENABLE_MEMORY_SEM_Bm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
          errnum +=10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
      res = jer2_arad_fill_table_with_entry(unit, SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      res = jer2_arad_fill_table_with_entry(unit, SCH_TOKEN_MEMORY_CONTROLLER__TMCm, SCH_BLOCK(unit, SOC_CORE_ALL), table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
  }

#ifdef BCM_88660_A0
  if ((SOC_DNX_CONFIG(unit)->emulation_system == 0) && (SOC_IS_ARADPLUS(unit))) {
      /* Arad+ tables */
      res = jer2_arad_fill_table_with_entry(unit, EGQ_PER_PORT_LB_RANGEm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      if (!SOC_IS_ARDON(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, EPNI_IP_TOS_MARKINGm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, EPNI_PP_REMARK_PROFILEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IDR_MCDA_PCUCm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IDR_MCDB_PCUCm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IDR_PCD_MAPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_CONSISTENT_HASHING_PROGRAM_SEL_TCAMm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_CONSISTENT_HASHING_PROGRAM_VARIABLESm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_ELK_PAYLOAD_FORMATm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_FEC_ECMP_IS_STATEFULm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_FLP_CONSISTENT_HASHING_KEY_GENm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHB_IPP_LAG_TO_LAG_RANGEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, IHP_VRID_MY_MAC_CAMm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
      res = jer2_arad_fill_table_with_entry(unit, IQM_FRDMTm, MEM_BLOCK_ANY, table_entry);
      errnum += 10;
      DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      if (!SOC_IS_ARDON(unit)) {
          res = jer2_arad_fill_table_with_entry(unit, OAMP_LMM_DA_NIC_TABLEm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_20000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_B0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, OAMP_MEM_C0000m, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, OAMP_MEP_DB_DM_STATm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, OAMP_MEP_DB_LM_DBm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, OAMP_MEP_DB_LM_STATm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, OAMP_MEP_DB_RFC_6374_ON_MPLSTPm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, OAMP_PE_GEN_MEMm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, OAMP_PE_PROGRAMm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
          res = jer2_arad_fill_table_with_entry(unit, OAMP_PE_PROG_TCAMm, MEM_BLOCK_ANY, table_entry);
          errnum += 10;
          DNX_SAND_CHECK_FUNC_RESULT(res, errnum, exit);
      }
  }
#endif /* BCM_88660_A0 */

  res = jer2_arad_mgmt_all_tbls_init_enable_dynamic(unit, 0);
  DNX_SAND_CHECK_FUNC_RESULT(res, 11116, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_all_tbls_init()",0,0);
}


/*
 *  Tables Initialization }
 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>



#endif /* of #if defined(BCM_88690_A0) */

