#include <shared/bsl.h>

#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/*
 * $Id: jer2_arad_tbl_access.c,v 1.186 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */


#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MEM

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/mem.h>
#include <soc/error.h>

#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_chip_tbls.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_api_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/drv.h> /* for assert() */
#include <soc/dnx/legacy/mbcm.h> 
#include <soc/dnxc/legacy/dnxc_mem.h> 
#include <soc/dnxc/legacy/utils.h>
#include <soc/hwstate/hw_log.h>
#include <shared/swstate/access/sw_state_access.h>

#if defined(BCM_88690_A0)
#include <soc/dnx/legacy/JER/jer_sbusdma_desc.h>
#endif /* defined(BCM_88690_A0) */

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

#define JER2_ARAD_WRED_NOF_DROP_PRECEDENCE   (JER2_ARAD_NOF_DROP_PRECEDENCE)


#define JER2_ARAD_QDR_QDR_DLL_MEM_TBL_ENTRY_SIZE 1


#define JER2_ARAD_IHB_TCAM_HIT_INDICATION_NOF_TCAM_ENTRIES_PER_LINE  (8)
#define JER2_ARAD_IHB_TCAM_HIT_INDICATION_TCAM_ENTRY_TO_BIT(line)    (line % JER2_ARAD_IHB_TCAM_HIT_INDICATION_NOF_TCAM_ENTRIES_PER_LINE)
#define JER2_ARAD_IHB_TCAM_HIT_INDICATION_TCAM_ENTRY_TO_LINE(line)   (line / JER2_ARAD_IHB_TCAM_HIT_INDICATION_NOF_TCAM_ENTRIES_PER_LINE)

#define JER2_ARAD_IRE_NIF_CTXT_MAP_TBL_ENTRY_SIZE                                               1
#define JER2_ARAD_IRE_NIF_PORT_TO_CTXT_BIT_MAP_TBL_ENTRY_SIZE                                   7
#define JER2_ARAD_IRE_RCY_CTXT_MAP_TBL_ENTRY_SIZE                                               1
#define JER2_ARAD_IRE_CPU_CTXT_MAP_TBL_ENTRY_SIZE                                               1
#define JER2_ARAD_IRE_TDM_CONFIG_TBL_ENTRY_SIZE                                                 5
#define JER2_ARAD_IDR_CONTEXT_MRU_TBL_ENTRY_SIZE                                                2
#define JER2_ARAD_IRR_SNOOP_MIRROR_TABLE0_TBL_ENTRY_SIZE                                        2
#define JER2_ARAD_IRR_SNOOP_MIRROR_TABLE1_TBL_ENTRY_SIZE                                        4
#define JER2_ARAD_IRR_DESTINATION_TABLE_TBL_ENTRY_SIZE                                          1
#define JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_ENTRY_SIZE                                           1
#define JER2_ARAD_IRR_LAG_MAPPING_TBL_ENTRY_SIZE                                                1
#define JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_ENTRY_SIZE                                            1
#define JER2_ARAD_IRR_TRAFFIC_CLASS_MAPPING_TBL_ENTRY_SIZE                                      1
#define JER2_ARAD_IRR_STACK_TRUNK_RESOLVE_TBL_ENTRY_SIZE                                        1
#define JER2_ARAD_IRR_STACK_FEC_RESOLVE_TBL_ENTRY_SIZE                                          1
#define JER2_ARAD_IHP_TM_PORT_SYS_PORT_CONFIG_TBL_ENTRY_SIZE                                    2
#define JER2_ARAD_QDR_QDR_DLL_MEM_TBL_ENTRY_SIZE                                                1
#define JER2_ARAD_IQM_STATIC_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_IQM_PACKET_QUEUE_RED_WEIGHT_TABLE_TBL_ENTRY_SIZE                              1
#define JER2_ARAD_IQM_CREDIT_DISCOUNT_TABLE_TBL_ENTRY_SIZE                                      1
#define JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_ENTRY_SIZE                          4
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_A_TBL_ENTRY_SIZE                          1
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_B_TBL_ENTRY_SIZE                          1
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_C_TBL_ENTRY_SIZE                          1
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_D_TBL_ENTRY_SIZE                          1
#define JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_A_TBL_ENTRY_SIZE                         3
#define JER2_ARAD_IQM_SYSTEM_RED_PARAMETERS_TABLE_TBL_ENTRY_SIZE                                1
#define JER2_ARAD_IQM_SYSTEM_RED_DROP_PROBABILITY_VALUES_TBL_ENTRY_SIZE                         1
#define JER2_ARAD_IQM_SYSTEM_RED_TBL_ENTRY_SIZE                                                 6
#define JER2_ARAD_IQM_CNM_PARAMETERS_TABLE_TBL_ENTRY_SIZE                                       5
#define JER2_ARAD_IPS_SYSTEM_PHYSICAL_PORT_LOOKUP_TABLE_TBL_ENTRY_SIZE                          2
#define JER2_ARAD_IPS_DESTINATION_DEVICE_AND_PORT_LOOKUP_TABLE_TBL_ENTRY_SIZE                   2
#define JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_IPS_QUEUE_TYPE_LOOKUP_TABLE_TBL_ENTRY_SIZE                                    1
#define JER2_ARAD_IPS_QUEUE_PRIORITY_MAP_SELECT_TBL_ENTRY_SIZE                                  1
#define JER2_ARAD_IPS_QUEUE_PRIORITY_MAPS_TABLE_TBL_ENTRY_SIZE                                  2
#define JER2_ARAD_IPS_QUEUE_SIZE_BASED_THRESHOLDS_TABLE_TBL_ENTRY_SIZE                          2
#define JER2_ARAD_IPS_SLOW_FACTOR_THRESHOLDS_TABLE_TBL_ENTRY_SIZE                               6
#define JER2_ARAD_IPS_CREDIT_BALANCE_BASED_THRESHOLDS_TABLE_TBL_ENTRY_SIZE                      2
#define JER2_ARAD_IPS_EMPTY_QUEUE_CREDIT_BALANCE_TABLE_TBL_ENTRY_SIZE                           2
#define JER2_ARAD_IPS_CREDIT_WATCHDOG_THRESHOLDS_TABLE_TBL_ENTRY_SIZE                           1
#define JER2_ARAD_EGQ_CCM_TBL_ENTRY_SIZE                                                        1
#define JER2_ARAD_EGQ_PMC_TBL_ENTRY_SIZE                                                        1
#define JER2_ARAD_EGQ_DWM_TBL_ENTRY_SIZE                                                        1
#define JER2_ARAD_EGQ_FQP_NIF_PORT_MUX_TBL_ENTRY_SIZE                                           1
#define JER2_ARAD_EGQ_PQP_NIF_PORT_MUX_TBL_ENTRY_SIZE                                           1
#define JER2_ARAD_IHB_PINFO_LBP_TBL_ENTRY_SIZE                                                  2
#define JER2_ARAD_IHB_HEADER_PROFILE_TBL_ENTRY_SIZE                                             1
#define JER2_ARAD_IHB_FEM0_4B_1ST_PASS_MAP_INDEX_TABLE_TBL_ENTRY_SIZE                           1
#define JER2_ARAD_IHB_FEM0_4B_1ST_PASS_MAP_TABLE_TBL_ENTRY_SIZE                                 ((139/32) + 1) /* 19b FEM case*/
#define JER2_ARAD_IHB_PINFO_LBP_TBL_ENTRY_SIZE                                                  2
#define JER2_ARAD_IHB_HEADER_PROFILE_TBL_ENTRY_SIZE                                             1
#define JER2_ARAD_CFC_CALRX_TBL_ENTRY_SIZE                                                      1
#define JER2_ARAD_CFC_OOB_SCH_MAP_TBL_ENTRY_SIZE                                                1
#define JER2_ARAD_CFC_CALTX_TBL_ENTRY_SIZE                                                      1
#define JER2_ARAD_CFC_CALRX_TBL_ENTRY_SIZE                                                1
#define JER2_ARAD_CFC_OOB_SCH_MAP_TBL_ENTRY_SIZE                                              1
#define JER2_ARAD_CFC_OOB_SCH_MAP_TBL_ENTRY_SIZE                                              1
#define JER2_ARAD_CFC_CALTX_TBL_ENTRY_SIZE                                                1
#define JER2_ARAD_SCH_DRM_TBL_ENTRY_SIZE                                                        1
#define JER2_ARAD_SCH_DSM_TBL_ENTRY_SIZE                                                        1
#define JER2_ARAD_SCH_FDMS_TBL_ENTRY_SIZE                                                       2
#define JER2_ARAD_SCH_SHDS_TBL_ENTRY_SIZE                                                       2
#define JER2_ARAD_SCH_SEM_TBL_ENTRY_SIZE                                                        1
#define JER2_ARAD_SCH_FSF_TBL_ENTRY_SIZE                                                        1
#define JER2_ARAD_SCH_FGM_TBL_ENTRY_SIZE                                                        1
#define JER2_ARAD_SCH_SHC_TBL_ENTRY_SIZE                                                        1
#define JER2_ARAD_SCH_SCC_TBL_ENTRY_SIZE                                                        1
#define JER2_ARAD_SCH_SCT_TBL_ENTRY_SIZE                                                        2
#define JER2_ARAD_SCH_FQM_TBL_ENTRY_SIZE                                                        1
#define JER2_ARAD_SCH_FFM_TBL_ENTRY_SIZE                                                        1
#define JER2_ARAD_SCH_SCHEDULER_INIT_TBL_ENTRY_SIZE                                             1
#define JER2_ARAD_SCH_FORCE_STATUS_MESSAGE_TBL_ENTRY_SIZE                                       1


#define JER2_ARAD_PP_IHP_MACT_AGING_CONFIGURATION_TABLE_TBL_ENTRY_SIZE                              ( 1)
#define JER2_ARAD_PP_IHP_TM_PORT_PARSER_PROGRAM_POINTER_CONFIG_TBL_ENTRY_SIZE                                   ( 1)
#define JER2_ARAD_PP_IHP_VIRTUAL_PORT_FEM_BIT_SELECT_TABLE_TBL_ENTRY_SIZE                           ( 1)
#define JER2_ARAD_PP_IHP_VIRTUAL_PORT_FEM_MAP_INDEX_TABLE_TBL_ENTRY_SIZE                            ( 1)
#define JER2_ARAD_PP_IHP_VIRTUAL_PORT_FEM_FIELD_SELECT_MAP_TBL_ENTRY_SIZE                           ( 4)
#define JER2_ARAD_PP_IHP_SRC_SYSTEM_PORT_FEM_FIELD_SELECT_MAP_TBL_ENTRY_SIZE                        ( 4)
#define JER2_ARAD_PP_IHP_PARSER_PROGRAM_POINTER_FEM_BIT_SELECT_TABLE_TBL_ENTRY_SIZE                 ( 1)
#define JER2_ARAD_PP_IHP_PARSER_PROGRAM_POINTER_FEM_MAP_INDEX_TABLE_TBL_ENTRY_SIZE                  ( 1)
#define JER2_ARAD_PP_IHP_PARSER_PROGRAM_POINTER_FEM_FIELD_SELECT_MAP_TBL_ENTRY_SIZE                 ( 1)
#define JER2_ARAD_PP_IHP_PARSER_PROGRAM_TBL_ENTRY_SIZE                                              ( 1)
#define JER2_ARAD_PP_IHP_VLAN_PORT_MEMBERSHIP_TBL_ENTRY_SIZE                                        ( 5)
#define JER2_ARAD_PP_IHP_SEM_RESULT_TABLE_TBL_ENTRY_SIZE                                            ( 2)
#define JER2_ARAD_PP_IHP_VTT1ST_KEY_CONSTRUCTION0_TBL_ENTRY_SIZE                                    ( 5)
#define JER2_ARAD_PP_IHP_VTT1ST_KEY_CONSTRUCTION1_TBL_ENTRY_SIZE                                    ( 6)
#define JER2_ARAD_PP_IHP_VTT2ND_KEY_CONSTRUCTION0_TBL_ENTRY_SIZE                                    ( 5)
#define JER2_ARAD_PP_IHP_VTT2ND_KEY_CONSTRUCTION1_TBL_ENTRY_SIZE                                    ( 6)
#define JER2_ARAD_PP_IHP_TC_DP_MAP_TABLE_TBL_ENTRY_SIZE                                                ( 2)
#define JER2_ARAD_PP_IHP_TERMINATION_PROFILE_TABLE_TBL_ENTRY_SIZE                                   ( 1)
#define JER2_ARAD_PP_IHP_ACTION_PROFILE_MPLS_VALUE_TBL_ENTRY_SIZE                                   ( 1)
#define JER2_ARAD_PP_IHB_FEC_ENTRY_ACCESSED_TBL_ENTRY_SIZE                                          ( 1)
#define JER2_ARAD_PP_IHB_PATH_SELECT_TBL_ENTRY_SIZE                                                 ( 1)
#define JER2_ARAD_PP_IHB_TCAM_ACTION_TBL_ENTRY_SIZE                                                 ( 1)
#define JER2_ARAD_PP_EGQ_INGRESS_VLAN_EDIT_COMMAND_MAP_TBL_ENTRY_SIZE                               ( 1)
#define JER2_ARAD_PP_EGQ_VSI_MEMBERSHIP_TBL_ENTRY_SIZE                                              ( 9)
#define JER2_ARAD_PP_EPNI_TX_TAG_TABLE_TBL_ENTRY_SIZE                                               ( 9)
#define JER2_ARAD_PP_EPNI_PCP_DEI_MAP_TBL_ENTRY_SIZE                                                ( 2)
#define JER2_ARAD_PP_EPNI_DP_MAP_TBL_ENTRY_SIZE                                                     ( 1)
#define JER2_ARAD_PP_EPNI_EGRESS_EDIT_CMD_TBL_ENTRY_SIZE                                            ( 2)
#define JER2_ARAD_PP_EPNI_INGRESS_VLAN_EDIT_COMMAND_MAP_TBL_ENTRY_SIZE                              ( 1)
#define JER2_ARAD_PP_EPNI_ISID_TABLE_TBL_ENTRY_SIZE                                                    ( 1)
#define JER2_ARAD_PP_IHP_VLAN_EDIT_PCP_DEI_MAP_TBL_ENTRY_SIZE                                       ( 1)
#define JER2_ARAD_PP_IHP_INGRESS_VLAN_EDIT_COMMAND_TABLE_TBL_ENTRY_SIZE                             ( 1) 
#define JER2_ARAD_PP_IHB_FLP_PROCESS_TBL_ENTRY_SIZE 3
#define JER2_ARAD_PP_IHB_LPM_TBL_ENTRY_SIZE 1
#define JER2_ARAD_PP_IHB_PMF_PASS_2_KEY_UPDATE_TBL_ENTRY_SIZE 1

#define JER2_ARAD_IQM_CNM_DS_TBL_DATA_SAMPLING_EN_2_BIT     4
#define JER2_ARAD_IQM_CNM_DS_TBL_DATA_SAMPLING_PROFILE_LSB  5
#define JER2_ARAD_IQM_CNM_DS_TBL_DATA_SAMPLING_PROFILE_MSB  7

#define JER2_ARAD_IQM_CRDTDIS_DATA_NOF_BITS 0x7


#define JER2_ARAD_PP_VTT_PD_BITMAP_NOF_FIELDS  4
#define JER2_ARAD_PP_FLP_PD_BITMAP_NOF_FIELDS  7

#define JER2_ARAD_PP_IHP_LIF_TABLE_ENTRY_SIZE 4

static const int irr_snoop_mirror_table_0_backup = IRR_SNOOP_MIRROR_TABLE_0m;
static const int irr_snoop_mirror_table_1_backup = IRR_SNOOP_MIRROR_TABLE_1m;
#undef IRR_SNOOP_MIRROR_TABLE_0m
#define IRR_SNOOP_MIRROR_TABLE_0m \
    (SOC_IS_JERICHO(unit) ? IRR_SNOOP_MIRROR_DEST_TABLEm : irr_snoop_mirror_table_0_backup)
#undef IRR_SNOOP_MIRROR_TABLE_1m
#define IRR_SNOOP_MIRROR_TABLE_1m \
    (SOC_IS_JERICHO(unit) ? IRR_SNOOP_MIRROR_TM_TABLEm : irr_snoop_mirror_table_1_backup)

#define JER2_ARAD_PP_LAG_BY_CHIP(_unit, _name)   \
        ((SOC_IS_QAX(unit)) ?  TAR_##_name##m : IRR_##_name##m)

/* } */

/*************
 *  MACROS   *
 *************/
/* { */

#define SOC_DNX_MEM_FIELD32_GET(field_get,table_name,field_name,exit_num) \
{ \
    res=SOC_E_NONE;\
  if(SOC_E_NONE == res) { \
      field_get = soc_mem_field32_get( \
                      unit, \
                      table_name, \
                      data, \
                      field_name); \
   } \
   DNX_SAND_CHECK_FUNC_RESULT(res, exit_num, exit); \
}

#define SOC_DNX_MEM_FIELD32_SET(field_set,table_name,field_name,exit_num) \
{ \
          res=SOC_E_NONE;\
    if(SOC_E_NONE == res) { \
        soc_mem_field32_set( \
          unit, \
          table_name, \
          data, \
          field_name, \
          field_set ); \
        } \
    DNX_SAND_CHECK_FUNC_RESULT(res, exit_num, exit); \
}


/* for odd entry jump to odd table*/

#define JER2_ARAD_PP_TBL_FEC_ARRAY_INDEX(_entry_index) ((_entry_index)%2)
#define JER2_ARAD_PP_TBL_FEC_ENTRY_INDEX(_entry_index) ((_entry_index)/2)

#define JER2_ARAD_PP_SUPER_FEC_ENTRY_INDEX(fec_ndx) ((fec_ndx)/2)

/* Layout is: 8 banks, each has (FEC_NUM / 8 * 2) lines with 2 fields - data0 and data1. */
#define JER2_JERICHO_PP_SUPER_FEC_SZ                                 (2)
#define JER2_JERICHO_PP_SUPER_FEC_BANK_SIZE(unit)                    (SOC_DNX_DEFS_GET(unit, nof_fecs) / (SOC_DNX_DEFS_GET(unit, nof_fec_banks) * JER2_JERICHO_PP_SUPER_FEC_SZ))
#define JER2_JERICHO_PP_SUPER_FEC_ARRAY_INDEX(unit, _entry_index)    (((_entry_index) / JER2_JERICHO_PP_SUPER_FEC_SZ) / JER2_JERICHO_PP_SUPER_FEC_BANK_SIZE(unit))
#define JER2_JERICHO_PP_SUPER_FEC_ENTRY_INDEX(unit, _entry_index)    (((_entry_index) / JER2_JERICHO_PP_SUPER_FEC_SZ) % JER2_JERICHO_PP_SUPER_FEC_BANK_SIZE(unit))
#define JER2_JERICHO_PP_FEC_ENTRY_OFFSET(unit, _entry_index)         ((_entry_index) % JER2_JERICHO_PP_SUPER_FEC_SZ)

/* Simple TCAM print */
#define JER2_ARAD_PP_TBL_ACC_TCAM_DEBUG_PRINT 0
/*#define JER2_ARAD_PP_TBL_ACC_TCAM_DEBUG_PRINT 1*/

/* Low-Level TCAM print for all the transaction */
#define JER2_ARAD_PP_TBL_ACC_TCAM_LOW_LEVEL_DEBUG_PRINT 0
/*#define JER2_ARAD_PP_TBL_ACC_TCAM_LOW_LEVEL_DEBUG_PRINT 1*/

#define JER2_ARAD_PP_TBL_ACC_TCAM_DEBUG_ACTION_PRINT 0
/*#define JER2_ARAD_PP_TBL_ACC_TCAM_DEBUG_ACTION_PRINT 1*/

/* Conversation from entry offset to lif bank id (0-3 in jer2_arad, 0-7 in Jericho).
   In Arad it's simply the MSBs, in Jericho it's 2 MSBs + 1 LSB */

#define JER2_ARAD_MBMP_SET_DYNAMIC(_mem)      _SHR_PBMP_PORT_ADD(jer2_arad_mem_is_dynamic_bmp[_mem/32], _mem%32)
#define JER2_ARAD_MBMP_IS_DYNAMIC(_mem)       _SHR_PBMP_MEMBER(jer2_arad_mem_is_dynamic_bmp[_mem/32], _mem%32)

/* Conversion FLP instruction valid bitmap from JER2_ARAD to Jericho
 * - during Jericho porting, use instruction 0-2 similarly, 
 * and instructions 3-5 as Jericho instructions 4-6. */
#define FLP_INSTR_VALID_BMP_JER2_ARAD_TO_JER2_JER(inst_valid_bitmap)  \
  (((inst_valid_bitmap & 0x38) << 1) | ((inst_valid_bitmap) & 7))


/* Default mask for TCAM entry field */
#define JER2_ARAD_TBL_ACCESS_DEFAULT_TCAM_ENTRY_FIELD_MASK(_unit, _mem, _field) \
           ((1 << soc_mem_field_length((_unit), (_mem), (_field)))-1)

/* Default PRGE Program Selection TCAM entry field mask */
#define JER2_ARAD_EGR_PRGE_PROG_SEL_DEFAULT_ENTRY_FIELD_MASK(_unit, _field) \
           (JER2_ARAD_TBL_ACCESS_DEFAULT_TCAM_ENTRY_FIELD_MASK((_unit), EPNI_PRGE_PROGRAM_SELECTION_CAMm, (_field)))
/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

static _shr_pbmp_t jer2_arad_mem_is_dynamic_bmp[NUM_SOC_MEM/32];
static uint32 *_jer2_arad_fill_table_with_variable_values_by_caching_buffer[BCM_MAX_NUM_UNITS];
static sal_mutex_t _jer2_arad_fill_table_with_variable_values_by_caching_buffer_lock[BCM_MAX_NUM_UNITS];
/* 64KB buffer */
static const uint32 _jer2_arad_fill_table_with_variable_values_by_caching_buffer_len = (1 << 16) / sizeof(_jer2_arad_fill_table_with_variable_values_by_caching_buffer[0][0]);

/* } */
soc_mem_t soc_jer2_arad_predefined_cached_mem_list[] = {
    CFC_CAT_2_TC_MAP_HCFCm,
    CFC_CAT_2_TC_MAP_NIFm,
    CFC_ILKN_RX_0_CALm,
    CFC_ILKN_RX_1_CALm,
    CFC_ILKN_TX_0_CALm,
    CFC_ILKN_TX_1_CALm,
    CFC_NIF_PFC_MAPm,
    CFC_RCL_VSQ_MAPm,
    CFC_SPI_OOB_RX_0_CALm,
    CFC_SPI_OOB_RX_1_CALm,
    CFC_SPI_OOB_TX_0_CALm,
    CFC_SPI_OOB_TX_1_CALm,
    EGQ_CCMm,
    EGQ_CH_0_SCMm,
    EGQ_CH_1_SCMm,
    EGQ_CH_2_SCMm,
    EGQ_CH_3_SCMm,
    EGQ_CH_4_SCMm,
    EGQ_CH_5_SCMm,
    EGQ_CH_6_SCMm,
    EGQ_CH_7_SCMm,
    EGQ_CH_8_SCMm,
    EGQ_CH_9_SCMm,
    EGQ_DSP_PTR_MAPm,
    EGQ_DWM_8Pm,
    EGQ_DWMm,
    EGQ_HEADER_MAPm,
    EGQ_MAP_OUTLIF_TO_DSPm,
    EGQ_NONCH_SCMm,
    EGQ_PDCT_TABLEm,
    EGQ_PMCm,
    EGQ_PPCTm,
    EGQ_PQST_TABLEm,
    EGQ_QDCT_TABLEm,
    EGQ_QP_PMCm,
    EGQ_QP_SCMm,
    EGQ_QQST_TABLEm,
    EGQ_TCG_PMCm,
    EGQ_TCG_SCMm,
    EGQ_VLAN_TABLEm,
    EGQ_VSI_MEMBERSHIPm,
    EPNI_ACE_TABLEm,
    EPNI_ACE_TO_FHEIm,
    EPNI_ACE_TO_OUT_LIFm,
    EPNI_ACE_TO_OUT_PP_PORTm,
    EPNI_DSCP_EXP_TO_PCP_DEIm,
    EPNI_NATIVE_DSCP_EXP_TO_PCP_DEIm,
    EPNI_DSCP_REMARKm,
    EPNI_EVEC_TABLEm,
    EPNI_NATIVE_EVEC_TABLEm,
    EPNI_EXP_REMARKm,
    EPNI_HEADER_MAPm,
    EPNI_ISID_TABLEm,
    EPNI_MIRROR_PROFILE_TABLEm,
    EPNI_MY_CFM_MAC_TABLEm,
    EPNI_PP_COUNTER_TABLEm,
    EPNI_PRGE_DATAm,
    EPNI_REMARK_IPV4_TO_DSCPm,
    EPNI_REMARK_IPV4_TO_EXPm,
    EPNI_REMARK_IPV6_TO_DSCPm,
    EPNI_REMARK_IPV6_TO_EXPm,
    EPNI_TX_TAG_TABLEm,
    FDT_IN_BAND_MEMm,
    IDR_CONTEXT_MRUm,
    IDR_ETHERNET_METER_CONFIGm,
    IHB_DESTINATION_STATUSm,
    IHB_FEC_ECMPm,
    IHB_FEC_SUPER_ENTRYm,
    IHB_FLP_KEY_CONSTRUCTIONm,
    IHB_FLP_PROCESSm,
    IHB_FWD_ACT_PROFILEm,
    IHB_IN_PORT_KEY_GEN_VARm,
    IHB_IPP_LAG_TO_LAG_RANGEm,
    IHB_LB_PFC_PROFILEm,
    IHB_LPM_2m,
    IHB_LPM_3m,
    IHB_LPM_4m,
    IHB_LPM_5m,
    IHB_LPM_6m,
    IHB_LPMm,
    IHB_OAMAm,
    IHB_OAMBm,
    IHB_PATH_SELECTm,
    IHB_PINFO_COUNTERSm,
    IHB_PINFO_FERm,
    IHB_PINFO_FLPm,
    IHB_PINFO_LBPm,
    IHB_PINFO_PMFm,
    IHB_PMF_FEM_PROGRAMm,
    IHB_PMF_FES_PROGRAMm,
    IHB_PMF_INITIAL_KEY_2ND_PASSm,
    IHB_PMF_PASS_1_KEY_GEN_LSBm,
    IHB_PMF_PASS_1_KEY_GEN_MSBm,
    IHB_PMF_PASS_1_LOOKUPm,
    IHB_PMF_PASS_2_KEY_GEN_LSBm,
    IHB_PMF_PASS_2_KEY_GEN_MSBm,
    IHB_PMF_PASS_2_LOOKUPm,
    IHB_PMF_PROGRAM_COUNTERSm,
    IHB_PMF_PROGRAM_GENERALm,
    IHB_PROGRAM_KEY_GEN_VARm,
    IHB_PTC_INFO_PMFm,
    IHB_PTC_KEY_GEN_VARm,
    IHB_SNOOP_ACTIONm,
    IHB_VRF_CONFIGm,
    IHP_BVD_CFGm,
    IHP_BVD_FID_CLASSm,
    IHP_BVD_TOPOLOGY_IDm,
    IHP_DEFAULT_COUNTER_SOURCEm,
    IHP_INGRESS_VLAN_EDIT_COMMAND_TABLEm,
    IHP_IN_RIF_CONFIG_TABLEm,
    IHP_LLR_LLVPm,
    IHP_LL_MIRROR_PROFILEm,
    IHP_PACKET_FORMAT_TABLEm,
    IHP_PINFO_LLRm,
    IHP_PORT_PROTOCOLm,
    IHP_PP_PORT_INFOm,
    IHP_PTC_INFOm,
    IHP_PTC_PARSER_PROGRAM_POINTER_CONFIGm,
    IHP_PTC_PFQ_0_CONFIGm,
    IHP_PTC_SYS_PORT_CONFIGm,
    IHP_PTC_VIRTUAL_PORT_CONFIGm,
    IHP_RECYCLE_COMMANDm,
    IHP_RESERVED_MCm,
    IHP_STP_TABLEm,
    IHP_TC_DP_MAP_TABLEm,
    IHP_TOS_2_COSm,
    IHP_VIRTUAL_PORT_TABLEm,
    IHP_VLAN_EDIT_PCP_DEI_MAPm,
    IHP_VRID_MY_MAC_MAPm,
    IHP_VRID_TO_VRF_MAPm,
    IHP_VSI_HIGH_DA_NOT_FOUND_DESTINATIONm,
    IHP_VSI_HIGH_MY_MACm,
    IHP_VSI_HIGH_PROFILEm,
    IHP_VSI_LOW_CFG_1m,
    IHP_VSI_LOW_CFG_2m,
    IHP_VTT_1ST_LOOKUP_PROGRAM_1m,
    IHP_VTT_2ND_LOOKUP_PROGRAM_1m,
    IHP_VTT_IN_PP_PORT_CONFIGm,
    IHP_VTT_IN_PP_PORT_VLAN_CONFIGm,
    IHP_VTT_LLVPm,
    IHP_VTT_PP_PORT_TT_KEY_VARm,
    IHP_VTT_PP_PORT_VSI_PROFILESm,
    IHP_VTT_PP_PORT_VT_KEY_VARm,
    IHP_VTT_PTC_CONFIGm,
    IPS_CRBALTHm,
    IPS_EMPTYQCRBALm,
    IPS_QPRISELm,
    IPS_QSZTHm,
    IPS_QTYPEm,
    IPT_SNP_MIR_CMD_MAPm,
    IQM_CNREDm,
    IQM_CPDMSm,
    IQM_OCBPRMm,
    IQM_PQDMSm,
    IQM_PQREDm,
    IQM_PQWQm,
    IQM_SCRBUFFTHm,
    IQM_SPRDPRMm,
    IQM_SRCQRNGm,
    IQM_VQFCPR_MAm,
    IQM_VQFCPR_MBm,
    IQM_VQFCPR_MCm,
    IQM_VQFCPR_MDm,
    IQM_VQFCPR_MEm,
    IQM_VQFCPR_MFm,
    IQM_VQPR_MAm,
    IQM_VQPR_MBm,
    IQM_VQPR_MCm,
    IQM_VQPR_MDm,
    IQM_VQPR_MEm,
    IQM_VQPR_MFm,
    IRE_CPU_CTXT_MAPm,
    IRE_NIF_CTXT_MAPm,
    IRE_NIF_PORT_TO_CTXT_BIT_MAPm,
    IRE_RCY_CTXT_MAPm,
    IRE_TDM_CONFIGm,
    IRR_DESTINATION_TABLEm,
    IRR_FLOW_TABLEm,
    IDR_IRDBm,
    IRR_LAG_MAPPINGm,
    IRR_LAG_TO_LAG_RANGEm,
    IRR_SMOOTH_DIVISIONm,
    IRR_STACK_FEC_RESOLVEm,
    IRR_STACK_TRUNK_RESOLVEm,
    OAMP_MEP_DB_BFD_CC_ON_MPLSTPm,
    
    /*OAMP_MEP_DB_BFD_ON_PWEm,*/
    OAMP_PE_PROGRAMm,
    OAMP_UMC_TABLEm,
    SCH_CIR_SHAPERS_STATIC_TABEL_CSSTm,
    SCH_CL_SCHEDULERS_CONFIGURATION_SCCm,
    SCH_CL_SCHEDULERS_TYPE_SCTm,
    SCH_SHARED_DEVICE_RATE_SHARED_DRMm,
    SCH_DUAL_SHAPER_MEMORY_DSMm,
    SCH_FLOW_DESCRIPTOR_MEMORY_STATIC_FDMSm,
    SCH_FLOW_GROUP_MEMORY_FGMm,
    SCH_FLOW_SUB_FLOW_FSFm,
    SCH_FLOW_TO_FIP_MAPPING_FFMm,
    SCH_FLOW_TO_QUEUE_MAPPING_FQMm,
    SCH_HR_SCHEDULER_CONFIGURATION_SHCm,
    SCH_PIR_SHAPERS_STATIC_TABEL_PSSTm,
    SCH_PORT_SCHEDULER_WEIGHTS_PSWm,
    SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR__CALm,
    SCH_SCHEDULER_ENABLE_MEMORY_SEMm,
    /* parity multi element memories. */
    IDR_MCDA_PRFSELm,
    IDR_MCDB_PRFSELm,
    IHB_FEC_ENTRYm,
    IHP_PARSER_PROGRAMm,

    /* parity multi element memories with alias*/
    EPNI_EEDB_BANKm,
    IHB_TCAM_ACTIONm,
    IHP_LIF_TABLEm,
    IHP_VLAN_PORT_MEMBERSHIP_TABLEm,

    NUM_SOC_MEM
};/*************
 * FUNCTIONS *
 *************/
/* { */

/*
 * Determining dynamic memories
 */
void jer2_arad_tbl_default_dynamic_set(void) {

    sal_memset(jer2_arad_mem_is_dynamic_bmp, 0, sizeof(jer2_arad_mem_is_dynamic_bmp));
    JER2_ARAD_MBMP_SET_DYNAMIC(CRPS_CRPS_0_CNTS_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(CRPS_CRPS_0_OVTH_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(CRPS_CRPS_1_CNTS_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(CRPS_CRPS_1_OVTH_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(CRPS_CRPS_2_CNTS_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(CRPS_CRPS_2_OVTH_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(CRPS_CRPS_3_CNTS_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(CRPS_CRPS_3_OVTH_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_CBMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_DCMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_FBMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_FDMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_PDCMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_PDCMAXm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_PQSMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_PQSMAXm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_QDCMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_QDCMAXm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_QM_0m);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_QM_1m);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_QM_2m);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_QM_3m);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_QP_CBMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_QQSMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_QQSMAXm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_RDMMCm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_RDMUCm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_RPDMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_RRDMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EGQ_TCG_CBMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(EDB_ESEM_MANAGEMENT_REQUESTm);
    JER2_ARAD_MBMP_SET_DYNAMIC(FCR_FCR_CRM_Am);
    JER2_ARAD_MBMP_SET_DYNAMIC(FCR_FCR_CRM_Bm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IDR_COMPLETE_PCm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IDR_CONTEXT_COLORm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IDR_CONTEXT_SIZEm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IDR_GLOBAL_METER_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IDR_MCDA_DYNAMICm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IDR_MCDB_DYNAMICm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IDR_MCDA_PCUCm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IDR_MCDB_PCUCm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHB_CPU_TRAP_CODE_CTRm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHB_FEC_ENTRY_ACCESSEDm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHB_OAM_COUNTER_FIFOm);
    JER2_ARAD_MBMP_SET_DYNAMIC(PPDB_A_OEMA_MANAGEMENT_REQUESTm);
    JER2_ARAD_MBMP_SET_DYNAMIC(PPDB_A_OEMB_MANAGEMENT_REQUESTm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHB_TCAM_ACTION_HIT_INDICATIONm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHB_TCAM_ACTION_HIT_INDICATION_24m);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHB_TCAM_ACTION_HIT_INDICATION_25m);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHB_TCAM_ACTION_HIT_INDICATION_26m);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHB_TCAM_ACTION_HIT_INDICATION_27m);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHB_TCAM_BANKm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHB_TCAM_ENTRY_PARITYm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHB_TCAM_ENTRY_PARITY_12m);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHB_TCAM_ENTRY_PARITY_13m);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHB_TIME_STAMP_FIFOm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHP_ISA_MANAGEMENT_REQUESTm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHP_ISB_MANAGEMENT_REQUESTm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IHP_LIF_ACCESSEDm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPS_CRBALm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPS_MAXQSZm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPS_QDESCm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPS_QDESC_TABLEm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPS_QSZm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPT_BDQm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPT_EGQCTLm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPT_EGQDATAm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPT_EGQ_TXQ_RD_ADDRm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPT_EGQ_TXQ_WR_ADDRm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPT_FDTCTLm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPT_FDTDATAm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPT_FDT_TXQ_RD_ADDRm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPT_FDT_TXQ_WR_ADDRm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPT_MOP_MMUm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPT_PCQm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IPT_SOP_MMUm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_BDBLLm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_CNG_QUE_SETm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_CPDMDm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_DBFFMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_DELFFMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_FLUSCNTm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_MNUSCNTm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_PDMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_PQDMDm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_TAILm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VSQ_A_MX_OCm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VSQ_B_MX_OCm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VSQ_C_MX_OCm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VSQ_D_MX_OCm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VSQ_E_MX_OCm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VSQ_F_MX_OCm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VS_QA_AVGm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VS_QA_QSZm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VS_QB_AVGm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VS_QB_QSZm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VS_QC_AVGm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VS_QC_QSZm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VS_QD_AVGm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VS_QD_QSZm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VS_QE_AVGm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VS_QE_QSZm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VS_QF_AVGm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IQM_VS_QF_QSZm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IRE_CTXT_MEM_CONTROLm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IRR_FREE_PCB_MEMORYm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IRR_ISF_MEMORYm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IRR_IS_FREE_PCB_MEMORYm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IRR_IS_PCB_LINK_TABLEm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IRR_LAG_NEXT_MEMBERm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IRR_MCR_MEMORYm);
    JER2_ARAD_MBMP_SET_DYNAMIC(IRR_PCB_LINK_TABLEm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_DRAM_ADDRESS_SPACEm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_FDFm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_IDFm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RAFA_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RAFB_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RAFC_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RAFD_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RAFE_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RAFF_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RAFG_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RAFH_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RAF_WADDRm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RDFA_WADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RDFB_WADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RDFC_WADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RDFD_WADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RDFE_WADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RDFF_WADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RDFG_WADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RDFH_WADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_RDF_RADDRm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFA_HALFA_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFA_HALFB_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFB_HALFA_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFB_HALFB_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFC_HALFA_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFC_HALFB_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFD_HALFA_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFD_HALFB_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFE_HALFA_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFE_HALFB_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFF_HALFA_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFF_HALFB_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFG_HALFA_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFG_HALFB_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFH_HALFA_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAFH_HALFB_RADDR_STATUSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAF_HALFA_WADDRm);
    JER2_ARAD_MBMP_SET_DYNAMIC(MMU_WAF_HALFB_WADDRm);
    JER2_ARAD_MBMP_SET_DYNAMIC(NBI_MLF_RX_MEM_A_CTRLm);
    JER2_ARAD_MBMP_SET_DYNAMIC(NBI_MLF_RX_MEM_B_CTRLm);
    JER2_ARAD_MBMP_SET_DYNAMIC(NBI_MLF_TX_MEM_CTRLm);
    JER2_ARAD_MBMP_SET_DYNAMIC(NBI_RBINS_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(NBI_RLENG_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(NBI_RPKTS_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(NBI_RTYPE_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(NBI_TBINS_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(NBI_TLENG_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(NBI_TPKTS_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(NBI_TTYPE_MEMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(OAMP_MEP_DBm);
    JER2_ARAD_MBMP_SET_DYNAMIC(OAMP_RMEP_DBm);
    JER2_ARAD_MBMP_SET_DYNAMIC(OAMP_RMAPEM_MANAGEMENT_REQUESTm);
    JER2_ARAD_MBMP_SET_DYNAMIC(OCB_OCBM_EVENm);
    JER2_ARAD_MBMP_SET_DYNAMIC(OCB_OCBM_ODDm);
    JER2_ARAD_MBMP_SET_DYNAMIC(RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_CTRL_CELLSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_DATA_CELLSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(RTP_UNICAST_DISTRIBUTION_MEMORYm);
    JER2_ARAD_MBMP_SET_DYNAMIC(SCH_BUCKET_DEFICIT__BDFm);
    JER2_ARAD_MBMP_SET_DYNAMIC(SCH_FLOW_INSTALLED_MEMORY__FIMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(SCH_FLOW_STATUS_MEMORY__FSMm);
    JER2_ARAD_MBMP_SET_DYNAMIC(SCH_PORT_QUEUE_SIZE__PQSm);
    JER2_ARAD_MBMP_SET_DYNAMIC(SCH_TOKEN_MEMORY_CONTROLLER__TMCm);
}

/*
 * Returns TRUE if the memory is dynamic
 */
int dnx_tbl_is_dynamic(int unit, soc_mem_t mem) {
    soc_mem_t origin_mem = mem;
    SOC_MEM_ALIAS_TO_ORIG(unit,mem);

    return ((JER2_ARAD_MBMP_IS_DYNAMIC(origin_mem) || JER2_ARAD_MBMP_IS_DYNAMIC(mem) || SOC_MEM_IS_INTERNAL(unit, mem)) 
    && SOC_IS_ARADPLUS_AND_BELOW(unit))
    || soc_mem_is_readonly(unit, mem) || soc_mem_is_writeonly (unit, mem) || soc_mem_is_signal(unit, mem) ;
}


/* 
 * Mark all cacheable tables
 */
void jer2_arad_tbl_mark_cachable(int unit) {
    soc_mem_t mem;
    for (mem = 0; mem < NUM_SOC_MEM; mem++) {
        if (!SOC_MEM_IS_VALID(unit, mem)) {
            continue;
        };
        /* Skip the Read-only/Write-Only/Signal tables */
        if (dnx_tbl_is_dynamic(unit, mem))
        {
            continue;
        }

        /* uncacheable memories*/
        switch (mem) {
            /* TCAM */
            case IHB_TCAM_BANK_COMMANDm:
            case PPDB_A_TCAM_BANK_COMMANDm:
            case IHB_TCAM_BANK_REPLYm:
            case PPDB_A_TCAM_BANK_REPLYm:
            case IHB_TCAM_BANKm:
            case PPDB_A_TCAM_BANKm:
            case KAPS_TCMm: 
/*
            case IHB_TCAM_ACTIONm:
            case PPDB_A_TCAM_ACTIONm:
*/

           /* IRR (accordint to SDK-74576(Gili)  should not be cached*/
           /**
            * we will used the cached of the multicast 
            * for shadow fix 
            */
            case IRR_MCDBm:


            /* READ ONLY memories */
            case EPNI_MEM_760000m:
            case FSRD_FSRD_WL_EXT_MEMm:
            case IDR_MEM_1B0000m:
            case IDR_MEM_1F0000m:
            case IHB_MEM_10E0000m:
            case IHB_MEM_16E0000m:
            case IHP_MACT_FLUSH_DBm:
            case IHP_MEM_740000m:
            case IHP_MEM_9E0000m:
            case IHP_MEM_C50000m:
            case OAMP_MEM_340000m:
            case PPDB_B_LARGE_EM_FLUSH_DBm:

            /* need to be checked Skip big entry_len tables causes ASSERT - memory len too big (1024) */
            case IHB_MEM_230000m:

            /* JER2_QAX - memory len too big*/
            case EPNI_MEM_600000m:
            case IHP_MEM_7200000m:

            /* QUX - memory len too big */
            case EPNI_PARSERA_FIFOm:
            case EPNI_PRGEA_FIFOm:
            case IHB_FERA_FIFOm:
            case IHP_FLPA_FIFOm:
            case IHP_PRSRA_FIFOm:
            case IPSEC_TUNNEL_DATAm:

            /* Skip Signal tables */
            case CRPS_MEM_0080000m:
            case CRPS_MEM_0090000m:
            case CRPS_MEM_00A0000m:
            case CRPS_MEM_00B0000m:
            case ECI_MBU_MEMm:
            case IPS_MEM_180000m:
            case IPS_MEM_1A0000m:
            case IPS_MEM_200000m:
            case IPS_MEM_220000m:
            case IPS_MEM_240000m:
            case IQM_MEM_7E00000m:
            case PORT_WC_UCMEM_DATAm:
            case SCH_MEM_01F00000m:
            case SCH_MEM_30000000m:

            /* Skip Signal tables - Arad Only */
            case IRR_MEM_300000m:
            case IRR_MEM_340000m:
            case IRR_MEM_3C0000m:

            /*Skip SER tables */
            case SER_ACC_TYPE_MAPm:
            case SER_MEMORYm:
            case SER_RESULT_0m:
            case SER_RESULT_1m:
            case SER_RESULT_DATA_0m:
            case SER_RESULT_DATA_1m:
            case SER_RESULT_EXPECTED_0m:
            case SER_RESULT_EXPECTED_1m:
            continue;

                             
            case IRE_PPLB_CFGm:
            case OAMP_FLOW_STAT_10_SEC_ENTRY_1m:
            case OAMP_FLOW_STAT_10_SEC_ENTRY_2m:
            case OAMP_FLOW_STAT_10_SEC_ENTRY_3m:
            case OAMP_FLOW_STAT_1_SEC_ENTRY_1m:
            case OAMP_FLOW_STAT_1_SEC_ENTRY_2m:
            case OAMP_FLOW_STAT_1_SEC_ENTRY_3m:
            case OAMP_FLOW_STAT_ACCUM_ENTRY_1m:
            case OAMP_FLOW_STAT_ACCUM_ENTRY_2m:
            case OAMP_FLOW_STAT_ACCUM_ENTRY_3m:
            case OAMP_FLOW_STAT_ACCUM_ENTRY_34m:
            case OAMP_SAT_TX_EVC_PARAMS_ENTRY_1m:
            case OAMP_SAT_TX_EVC_PARAMS_ENTRY_2m:
            case OAMP_SAT_TX_GEN_PARAMSm:

            /*This table is internal OAMP table, so it should nt being cached*/
            case OAMP_MEM_20000m:

            /* this memories are not real memories and should not being cached*/
            case ILKN_PMH_PORT_0_CPU_ACCESSm:
            case ILKN_PMH_PORT_1_CPU_ACCESSm:
            case ILKN_PML_PORT_0_CPU_ACCESSm:
            case ILKN_PML_PORT_1_CPU_ACCESSm:
            case CLPORT_WC_UCMEM_DATAm:
            case XLPORT_WC_UCMEM_DATAm:

                if(SOC_IS_JERICHO(unit)) {
                    continue;
                } else {
                    break;
                }

            case NBI_EGR_MEM_DATAm:
                if(SOC_IS_ARDON(unit)) {
                        continue;
                    } else {
                        break;
                    }
            case PPDB_A_TCAM_ACTIONm:

                if(SOC_IS_ARADPLUS_AND_BELOW(unit)) {
                    continue;
                }

            default:
                break;
        }
        SOC_MEM_INFO(unit, mem).flags |= SOC_MEM_FLAG_CACHABLE;
        LOG_DEBUG(BSL_LS_SOC_MEM, (BSL_META_U(unit, "caching %s \n"), SOC_MEM_NAME(unit, mem)));
    }
}

#ifdef CRASH_RECOVERY_SUPPORT
static uint32
_soc_mem_sbusdma_clear_pcid(DNX_SAND_IN int unit,DNX_SAND_IN  soc_mem_t mem, 
                              DNX_SAND_IN int copyno, DNX_SAND_IN void *data)
{
  uint32    err = 0;

  int    index_min, index_max, blk;
  int    array_index_max = 0;
  int entry_indx, array_index;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  index_min = soc_mem_index_min(unit, mem);
  index_max = soc_mem_index_max(unit, mem);

  if (SOC_MEM_IS_ARRAY(unit, mem)) 
  {
    soc_mem_array_info_t *maip = SOC_MEM_ARRAY_INFOP(unit, mem);
    if (maip) 
    {
        array_index_max = maip->numels - 1;
    }
  }

  SOC_MEM_BLOCK_ITER(unit, mem, blk) 
  {
    if (copyno != COPYNO_ALL && copyno != blk) 
    {
        continue;
    }

    for(array_index = 0; array_index <= array_index_max; ++array_index) 
    {
      for(entry_indx = index_min; entry_indx <= index_max; ++entry_indx) 
      {
          err = soc_mem_array_write(
                  unit,
                  mem,
                  array_index,
                  blk,
                  entry_indx,
                  (void*)data
                );
          DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);
      }
    }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in _soc_mem_sbusdma_clear_pcid()",0,0);
}
#endif /*CRASH_RECOVERY_SUPPORT*/

/* Fill the whole table with the given entry, uses fast DMA filling when run on real hardware */
uint32
  jer2_arad_fill_table_with_entry(
    DNX_SAND_IN int       unit,
    DNX_SAND_IN soc_mem_t mem,
    DNX_SAND_IN int       copyno,
    DNX_SAND_IN void      *data
  )
{
  uint32    res = 0;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

#ifdef CRASH_RECOVERY_SUPPORT
  if (SOC_IS_DONE_INIT(unit) && BCM_UNIT_DO_HW_READ_WRITE(unit))
  {
    res = _soc_mem_sbusdma_clear_pcid(unit, mem, copyno, data);
  }
  else
#endif /* CRASH_RECOVERY_SUPPORT */
  {
    res = dnxc_fill_table_with_entry(unit, mem, copyno, data);
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 222, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fill_table_with_entry()",0,0);
}

/* Fill the specified part of the table with the given entry, uses fast DMA filling when run on real hardware */
uint32
  jer2_arad_fill_partial_table_with_entry(
    DNX_SAND_IN int       unit,
    DNX_SAND_IN soc_mem_t mem,
    DNX_SAND_IN unsigned  array_index_start,
    DNX_SAND_IN unsigned  array_index_end,
    DNX_SAND_IN int       copyno,
    DNX_SAND_IN int       index_start,
    DNX_SAND_IN int       index_end,
    DNX_SAND_IN void      *data
  )
{
  uint32 err = 0;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  if ((!soc_mem_slamable(unit, mem, copyno)
#ifdef CRASH_RECOVERY_SUPPORT 
       ||  (SOC_IS_DONE_INIT(unit) && BCM_UNIT_DO_HW_READ_WRITE(unit))
#endif /* CRASH_RECOVERY_SUPPORT */
       )
#ifdef PLISIM
      || (SAL_BOOT_PLISIM)
#endif
   )
  {
    int index, blk;
    unsigned array_index;

    SOC_MEM_BLOCK_ITER(unit, mem, blk) 
    {
      if (copyno != COPYNO_ALL && copyno != blk) {
        continue;
      }
      for (array_index = array_index_start; array_index <= array_index_end; ++array_index) 
      {
        for (index = index_start; index <= index_end; ++index) 
        {
          err = soc_mem_array_write(unit, mem, array_index, blk, index, (void*)data);
          DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 10, exit);
        }
      }
    }
  }
else {
#ifdef BCM_SBUSDMA_SUPPORT
    err = _soc_mem_sbusdma_clear_specific(unit, mem, array_index_start, array_index_end, copyno, index_start, index_end, (void*)data);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 20, exit);
#endif
    }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fill_partial_table_with_entry()",0,0);
}

#ifdef PALLADIUM_BACKDOOR

#include <sal/appl/io.h>
#include <stdlib.h>

#define _PALLADIUM_BACKDOOR_ROOT                        "/projects/ntsw-sw/home/tomerma"
#define _PALLADIUM_BACKDOOR_SCRIPT_DIR                  _PALLADIUM_BACKDOOR_ROOT "/palladium_backdoor/"
#define _PALLADIUM_BACKDOOR_JER2_JERICHO_ALL_MEMS_FILE       _PALLADIUM_BACKDOOR_SCRIPT_DIR "/palladium_backdoor_jer2_jericho_mem_list.txt"
#define _PALLADIUM_BACKDOOR_DUMP_FILE_TEMPLATE          _PALLADIUM_BACKDOOR_ROOT "/tmpdir/tempXXXXXX"
#define _PALLADIUM_BACKDOOR_FULL_TABLE_WRITE_SCRIPT     _PALLADIUM_BACKDOOR_SCRIPT_DIR "/palladium_backdoor_dispatch_full_table_write.py"
#define _PALLADIUM_BACKDOOR_PARTIAL_TABLE_WRITE_SCRIPT  _PALLADIUM_BACKDOOR_SCRIPT_DIR "/palladium_backdoor_dispatch_partial_table_write.py"
#define _PALLADIUM_BACKDOOR_MAX_STRING_LEN              (1024)
#define _PALLADIUM_BACKDOOR_MAX_SW_MEM_NAME             (256)
#define _PALLADIUM_BACKDOOR_MAX_LOGICAL_WRAPPERS        (128)
#define _PALLADIUM_BACKDOOR_NOF_CORE_IDS                (2)

/* Copy at most max_len chars from src. If src has less characters then stop at the end of src. */
static void _palladium_backdoor_safe_strncpy(char *dst, const char *src, int max_len)
{
  /* One always reserved for the terminating 0 */
  for (max_len--; *src && max_len; src++, dst++, max_len--) {
      *dst = *src;
  }

  *dst = 0;
}

static int _palladium_backdoor_safe_strnlen(const char *str, int max_len)
{
  int i;

  for (i = 0; str[i] && i < max_len; i++);

  return i;
}

#define _PALLADIUM_BACKDOOR_SAFE_SPRINTF(str, format, args...)  do { int __rv = snprintf((str), _PALLADIUM_BACKDOOR_MAX_STRING_LEN, (format), args); DNXC_VERIFY(__rv > 0); DNXC_VERIFY(__rv < _PALLADIUM_BACKDOOR_MAX_STRING_LEN); } while (0)
#define _PALLADIUM_BACKDOOR_SAFE_STRCPY(dst, src)           _palladium_backdoor_safe_strncpy((dst), (src), _PALLADIUM_BACKDOOR_MAX_STRING_LEN)
#define _PALLADIUM_BACKDOOR_SAFE_STRLEN(str)                _palladium_backdoor_safe_strnlen((str), _PALLADIUM_BACKDOOR_MAX_STRING_LEN)

/** 
 * Given an instance of a HW name, transform it to a format. 
 *  
 * General structure is: 
 * {pre-sw-name}{sw-name}{post-sw-name} 
 *  
 * if there is a core_id then {pre-sw-name} looks like {str}%u{str}, otherwise like {str}. 
 * if there is a logical_wrapper then {post-sw-name} looks like {str}%u{str}, otherwise like {str}. 
 *  
 * Parameters: 
 * @block_name - Name of the block (e.g. "EGQ" or "IPT"). 
 * @sw_name - The software name for the table. 
 * @hw_name - A matching HW name. 
 * @hw_format - The HW format will be returned here. 
 * @is_logical_wrapper - An indication of whether there is a logical_wrapper or not is returned here. 
 * @is_logical_wrapper - An indication of whether there is a hw_name per core is returned here.  
 */
soc_error_t _jer2_arad_palladium_backdoor_hw_name_to_hw_format(
   int unit,
   DNX_SAND_IN char *block_name,
   DNX_SAND_IN char *sw_name,
   DNX_SAND_IN char hw_name[_PALLADIUM_BACKDOOR_MAX_STRING_LEN],
   DNX_SAND_OUT char hw_format[_PALLADIUM_BACKDOOR_MAX_STRING_LEN],
   DNX_SAND_OUT uint32 *is_logical_wrapper,
   DNX_SAND_OUT uint32 *is_per_core
   )
{
  int nof_scanned_elems;
  int sw_name_len;
  int hw_format_len;
  char *sw_name_begin;
  char *core_digit_begin;
  char *logical_wrapper_begin;
  char *logical_wrapper_post_num;
  char *curr_hw_format = hw_format;
  char *curr_hw_name;
  char hw_name_copy[_PALLADIUM_BACKDOOR_MAX_STRING_LEN];
  char *logical_wrapper_str;

  DNXC_INIT_FUNC_DEFS;

  sw_name_len = _PALLADIUM_BACKDOOR_SAFE_STRLEN(sw_name);
  _PALLADIUM_BACKDOOR_SAFE_STRCPY(hw_name_copy, hw_name);
  curr_hw_name = hw_name_copy;
  sw_name_begin = strstr(hw_name_copy, sw_name);

  assert(sw_name_begin != NULL);

  /* pre sw_name part. */
  sw_name_begin[0] = 0;

  *is_per_core = FALSE;

  /* Exceptions */
  if (strcmp(block_name, "epni") == 0 ||
      strcmp(block_name, "edb") == 0 ||
      (strcmp(block_name, "egq") == 0 && strcmp(sw_name, "tc_dp_map_table") != 0)) {
      core_digit_begin = hw_name_copy + strlen("jer2_jericho_chip.egq_epni_dbf_2x");
  } else {
      core_digit_begin = hw_name_copy;
  }

  /* Try to find a digit before {sw_name} */
  if (core_digit_begin != NULL) {
    core_digit_begin = (char*)core_digit_begin + strcspn(core_digit_begin, "0123456789");
    *is_per_core = core_digit_begin[0] != 0 ? TRUE : FALSE;
  }

  /* {pre_core_id}%u{post_core_id}{sw_name} */
  if (*is_per_core) {
    uint32 core_id;

    nof_scanned_elems = sscanf(core_digit_begin, "%u", &core_id);
    DNXC_VERIFY(nof_scanned_elems == 1);
    if (core_id > 1) {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Illegal core_id %u for HW memory %s.\n"), core_id, sw_name));
    }
    
    core_digit_begin[0] = 0;
    _PALLADIUM_BACKDOOR_SAFE_SPRINTF(curr_hw_format, "%s%%u%s", hw_name_copy, core_digit_begin + 1);

  /* {pre_sw_name}{sw_name}*/
  } else {
    _PALLADIUM_BACKDOOR_SAFE_STRCPY(curr_hw_format, hw_name_copy);
  }

  curr_hw_format += _PALLADIUM_BACKDOOR_SAFE_STRLEN(curr_hw_format);
  _PALLADIUM_BACKDOOR_SAFE_STRCPY(curr_hw_format, sw_name);
  curr_hw_format += sw_name_len;
  curr_hw_name = sw_name_begin + sw_name_len;

  /* post sw_name part */

  /* Check whether there is a logical wrapper. */
  logical_wrapper_str = "logical_wrapper_";
  logical_wrapper_begin = strstr(curr_hw_name, "logical_wrapper_");
  *is_logical_wrapper = logical_wrapper_begin != NULL ? TRUE : FALSE;

  if (logical_wrapper_begin != NULL) {
    logical_wrapper_post_num = logical_wrapper_begin + _PALLADIUM_BACKDOOR_SAFE_STRLEN(logical_wrapper_str);

    /* Find this char:       x     */
    /* ***logical_wrapper_ddd****  */
    if (!('0' <= *logical_wrapper_post_num && *logical_wrapper_post_num <= '9')) {
      DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Illegal character %c after logical_wrapper_ for HW memory %s.\n"), *logical_wrapper_post_num, hw_name));
    }

    for (; '0' <= *logical_wrapper_post_num && *logical_wrapper_post_num <= '9'; logical_wrapper_post_num++); 

    logical_wrapper_begin[0] = 0;

    _PALLADIUM_BACKDOOR_SAFE_SPRINTF(curr_hw_format, "%s%s%%u%s", curr_hw_name, logical_wrapper_str, logical_wrapper_post_num);
  } else {
    _PALLADIUM_BACKDOOR_SAFE_STRCPY(curr_hw_format, curr_hw_name);
  }

  hw_format_len = _PALLADIUM_BACKDOOR_SAFE_STRLEN(hw_format);

  /* Remove \r\n at end of line. */
  if (strcmp(hw_format + hw_format_len - 2, "\r\n") == 0) {
    hw_format[hw_format_len - 2] = '\0';
  }

exit:
  DNXC_FUNC_RETURN;
}

/**
 * Given a software table name, return the emulation hardware name(s) of the table. 
 * In case the SW table corresponds to several HW tables, a format is returned containing one or more occurences of %u. 
 * More specifically: 
 * If the SW memory has an instance per core (at most 2), this will correspond to one %u. 
 * If the SW memory is split among several HW tables (i.e. the entries are split evenly to several HW "logical wrapper"s), this will correspond to one %u. 
 * If both the above are present, then the first %u will correspond to the core, and the second to the "logical wrapper".
 *  
 * Parmeters:
 * @sw_name - The software name to get the HW name for (if array, then the array index must be appended at the end). 
 * @hw_mem_format - The returned HW name (or format). 
 * @nof_logical_wrappers - If the SW table is split to several HW tables, then this will contain the number of logical wrappers (otherwise 0).
 *                         The entries are split evenly between these HW tables, and hw_mem_format will contain an %u for this.
 * @is_per_core - If the SW table has one HW table per core, then this will be 1 (otherwise 0), and hw_mem_format will contain one %u for this.
 */
soc_error_t _jer2_jericho_palladium_backdoor_sw_mem_to_hw_name(
   int unit,
   DNX_SAND_IN char *sw_name,
   DNX_SAND_OUT char hw_mem_format[_PALLADIUM_BACKDOOR_MAX_STRING_LEN],
   DNX_SAND_OUT uint32 *nof_logical_wrappers,
   DNX_SAND_OUT uint32 *is_per_core
   )
{
  int i;
  int nof_scanned_elems;
  uint32 name_matched = FALSE;
  uint32 is_logical_wrapper;
  uint32 scanned_logical_wrapper;
  uint32 scanned_core_id;
  /* logical wrappers (per core). */
  /* used to check there are no core_id/logical_wrapper/(core_id,logical_wrapper) duplicates. */
  uint32 expected_items_seen[_PALLADIUM_BACKDOOR_NOF_CORE_IDS][_PALLADIUM_BACKDOOR_MAX_LOGICAL_WRAPPERS];
  /* Buffer for lines read from the file. */
  char line_buffer[_PALLADIUM_BACKDOOR_MAX_STRING_LEN];
  /* The sw_name in lower case. */
  char lower_case_sw_name[_PALLADIUM_BACKDOOR_MAX_STRING_LEN];
  /* Will hold the first possible match - .i_{fixed_sw_name}.*/
  char match_string_1[_PALLADIUM_BACKDOOR_MAX_STRING_LEN];
  /* Will hold the second possible match - .{fixed_sw_name}.*/
  char match_string_2[_PALLADIUM_BACKDOOR_MAX_STRING_LEN];
  /* The lower case sw_name without the block. */
  char *fixed_sw_name = lower_case_sw_name;
  /* The block name. */
  char *block_name;
  uint32 sw_name_len;
  FILE *all_mems_file;
  soc_error_t rv;
  struct {
    const char *sw_name;
    const char *hw_format;
    uint32 nof_logical_wrappers;
    uint32 is_per_core;
  } exceptions[] = {
    { "EPNI_REMARK_MPLS_TO_EXP", "jer2_jericho_chip.egq_epni_dbf_2x.egq_epni%u.epni_dft.epni.epni_ind_regs.remark_mpls_to_exp.cpu_data_in", 0, TRUE },
    { "EPNI_REMARK_MPLS_TO_DSCP", "jer2_jericho_chip.egq_epni_dbf_2x.egq_epni%u.epni_dft.epni.epni_ind_regs.remark_mpls_to_dscp.cpu_data_in", 0, TRUE }
  };
  int exception_idx;

  DNXC_INIT_FUNC_DEFS;

  DNXC_VERIFY(exceptions == exceptions);

  for (exception_idx = 0; exception_idx < sizeof(exceptions) / sizeof(exceptions[0]); exception_idx++) {
    if (strcmp(sw_name, exceptions[exception_idx].sw_name) == 0) {
      *nof_logical_wrappers = exceptions[exception_idx].nof_logical_wrappers;
      *is_per_core = exceptions[exception_idx].is_per_core;
      _PALLADIUM_BACKDOOR_SAFE_STRCPY(hw_mem_format, exceptions[exception_idx].hw_format);
      SOC_EXIT;
    }
  }

  if (strcmp(sw_name, "EPNI_REMARK_MPLS_TO_EXP") == 0) {
    *nof_logical_wrappers = 0;
  }

  all_mems_file = fopen(_PALLADIUM_BACKDOOR_JER2_JERICHO_ALL_MEMS_FILE, "r");
  if (!all_mems_file) {
    DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Could not open the palladium all memories file (%s).\n"), _PALLADIUM_BACKDOOR_JER2_JERICHO_ALL_MEMS_FILE)); 
  }
  *nof_logical_wrappers = 0;

  memset(expected_items_seen, 0, sizeof(expected_items_seen));

  line_buffer[_PALLADIUM_BACKDOOR_MAX_STRING_LEN - 1] = 0;

  sw_name_len = _PALLADIUM_BACKDOOR_SAFE_STRLEN(sw_name);

  for (i = 0; i < sw_name_len; i++) {
    lower_case_sw_name[i] = tolower(sw_name[i]);
  }

  lower_case_sw_name[i] = 0;

  /* Exception: EGQ_TC_DP_MAP - The name is egq_tc_dp_map_table. */
  if (strcmp(sw_name, "EGQ_TC_DP_MAP") == 0) {
      strcpy(lower_case_sw_name + i, "_table");
  }

  
  /* Remove the block name (first word ending with underscore) */
  fixed_sw_name = strchr(lower_case_sw_name, '_');
  block_name = lower_case_sw_name;
  /* Exception: PPDB_A and PPDB_B - the block name has an underscore in it. */
  if (
      strncmp(sw_name, "PPDB_A", 6) == 0 ||
      strncmp(sw_name, "PPDB_B", 6) == 0
     ) {
      
    /* Correct for the extra _A or _B */
    fixed_sw_name += 2;
  }

  fixed_sw_name[0] = 0;
  fixed_sw_name++; 

  _PALLADIUM_BACKDOOR_SAFE_SPRINTF(match_string_1, ".i_%s.", fixed_sw_name);
  _PALLADIUM_BACKDOOR_SAFE_SPRINTF(match_string_2, ".%s.", fixed_sw_name);

  while (fgets(line_buffer, _PALLADIUM_BACKDOOR_MAX_STRING_LEN, all_mems_file) == line_buffer) {
    char *sw_name_begin; 
    int core_id = 0;
    int logical_wrapper_id = 0;

    /* Make sure the line is shorter than _PALLADIUM_BACKDOOR_MAX_STRING_LEN. */
    assert(line_buffer[_PALLADIUM_BACKDOOR_MAX_STRING_LEN - 1] == 0);

    /* First search for the name. */
    sw_name_begin = strstr(line_buffer, fixed_sw_name);
    
    if (sw_name_begin != NULL) {

      /* Next check that either it is either of the form .i_{name}. or .{name} and has mem.mem in the name. */
      if ((strstr(line_buffer, match_string_1) != NULL || strstr(line_buffer, match_string_1) != NULL) &&
          (strstr(line_buffer, "mem.mem") != NULL)) {
        uint32 match_error = FALSE;

        /* For some memories, there are matching names that are actually signals and not hw memories. */
        /* For these also search for a mem.mem */       
        if (name_matched == FALSE) {
          name_matched = TRUE;
          /* If this is the first match then create the hw name format. */
          rv = _jer2_arad_palladium_backdoor_hw_name_to_hw_format(unit, block_name, fixed_sw_name, line_buffer, hw_mem_format, &is_logical_wrapper, is_per_core);
          DNXC_IF_ERR_EXIT(rv);
        } 

        /* Now check that the current name matches our pattern (and possible scan the logical wrapper number). */
        if (!is_logical_wrapper && !*is_per_core) {
          if (strstr(line_buffer, hw_mem_format) == NULL) {
            match_error = TRUE;
          }
        } else if (is_logical_wrapper && !*is_per_core) {
          nof_scanned_elems = sscanf(line_buffer, hw_mem_format, &scanned_logical_wrapper);
          if (nof_scanned_elems != 1) {
            match_error = TRUE;
          }
        } else if (!is_logical_wrapper && *is_per_core) {
          nof_scanned_elems = sscanf(line_buffer, hw_mem_format, &scanned_core_id);
          if (nof_scanned_elems != 1) {
            match_error = TRUE;
          }
        } else if (is_logical_wrapper && *is_per_core) {
          nof_scanned_elems = sscanf(line_buffer, hw_mem_format, &scanned_core_id, &scanned_logical_wrapper);
          if (nof_scanned_elems != 2) {
            match_error = TRUE;
          }
        }

        if (match_error) {
          DNXC_EXIT_WITH_ERR(SOC_E_FAIL,(_BSL_DNXC_MSG("HW memory %s expected to match format %s, but it does not (memory %s).\n"), 
                                            line_buffer, hw_mem_format, sw_name));
        }

        if (*is_per_core) {
          if (scanned_core_id > _PALLADIUM_BACKDOOR_NOF_CORE_IDS) {
            /* Check the scanned core id is in bounds. */
            DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("HW memory %s has an illegal core_id (%d > %s) (memory %s).\n"), 
                                              line_buffer, scanned_core_id, _PALLADIUM_BACKDOOR_NOF_CORE_IDS, sw_name));
          }
          core_id = scanned_core_id;
        }

        if (is_logical_wrapper) {
          /* Check the scanned logical wrapper is in bounds. */
          assert(scanned_logical_wrapper < _PALLADIUM_BACKDOOR_MAX_LOGICAL_WRAPPERS);

          /* Update the max if needed. */
          if (scanned_logical_wrapper > *nof_logical_wrappers) {
            *nof_logical_wrappers = scanned_logical_wrapper;
          }

          logical_wrapper_id = scanned_logical_wrapper;
        }

        /* Make sure there are no duplicate items (logical_wrapper/core_id) */
        if (expected_items_seen[core_id][logical_wrapper_id]) {
          char details_str[_PALLADIUM_BACKDOOR_MAX_STRING_LEN];
          
          if (*is_per_core && is_logical_wrapper) {
            _PALLADIUM_BACKDOOR_SAFE_SPRINTF(details_str, "logical_wrapper %d, core %d", logical_wrapper_id, core_id);
          } else if (!*is_per_core && is_logical_wrapper) {
            _PALLADIUM_BACKDOOR_SAFE_SPRINTF(details_str, "logical_wrapper %d, no core_id", logical_wrapper_id);
          } else if (*is_per_core && !is_logical_wrapper) {
            _PALLADIUM_BACKDOOR_SAFE_SPRINTF(details_str, "no logical_wrapper, core %d", core_id);
          } else if (!*is_per_core && is_logical_wrapper) {
            _PALLADIUM_BACKDOOR_SAFE_STRCPY(details_str, "no logical_wrapper, no core_id");
          }

          DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("There are duplicate HW memory instances for memory %s (%s).\n"), 
                                            sw_name, details_str));
        }
        expected_items_seen[core_id][logical_wrapper_id] = TRUE;
      }
    }
  }

  if (!name_matched) {
      DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Could not match SW name %s to a HW name.\n"), sw_name));
  }
  if (is_logical_wrapper) {
    if (*nof_logical_wrappers <= 1) {
      DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Expected a logical_wrapper for format %s (memory %s), but only one logical_wrapper instance found.\n"), 
                                                          hw_mem_format, sw_name));
    }

    /* Currently *nof_logical_wrappers is max, not nof. */
    (*nof_logical_wrappers)++;
  }

  sal_fclose(all_mems_file);

exit:
  DNXC_FUNC_RETURN;
}

extern int close(int);
#include <sys/wait.h>
#include <sys/stat.h>

/**
 * Dispatch a write command to palladium, causing it to do a backdoor write of the supplied file to the supplied HW table. 
 * The same entry data is written to all entries of the table. 
 * 
 * @sw_mem_name - The SW memory table to write to (if memory array then append the index at the end).
 * @entry_data - The entry data to write to each entry in the table.
 * @nof_entries - The length (in 32-bit entries) of entry_data.
 */
soc_error_t _jer2_arad_palladium_backdoor_dispatch_full_table_write(
   DNX_SAND_IN int unit, 
   DNX_SAND_IN char *sw_mem_name,
   DNX_SAND_IN uint32 entry_data[],
   DNX_SAND_IN uint32 entry_data_len
   )
{
  char cmd[_PALLADIUM_BACKDOOR_MAX_STRING_LEN]; 
  char hw_mem_format[_PALLADIUM_BACKDOOR_MAX_STRING_LEN];
  char hw_mem_name[_PALLADIUM_BACKDOOR_MAX_STRING_LEN];
  int hw_mem_names_len = 1;
  uint32 nof_logical_wrappers;
  uint32 is_per_core;
  int int_ret;
  int logical_wrapper_idx;
  int core_idx;
  soc_error_t rv;

  DNXC_INIT_FUNC_DEFS;
  
  /* Get the HW memory format. */  
  rv = _jer2_jericho_palladium_backdoor_sw_mem_to_hw_name(unit, sw_mem_name, hw_mem_format, &nof_logical_wrappers, &is_per_core );  
  DNXC_IF_ERR_EXIT(rv);

  if (is_per_core) {
    hw_mem_names_len = 2;
  }

  /* If nof_logical_wrappers then there is exactly 1 instance.*/
  nof_logical_wrappers = nof_logical_wrappers == 0 ? 1 : nof_logical_wrappers;
  
  /* Loop over all logical wrappers and core ids and write the entries. */
  for (logical_wrapper_idx = 0; logical_wrapper_idx < nof_logical_wrappers; logical_wrapper_idx++) {
    for (core_idx = 0; core_idx < hw_mem_names_len; core_idx++) {
      int entry_data_idx;
      char *curr_cmd;
      int child_exit_status = 0;

      /* Get the current HW name from the format and indices. */
      if (!is_per_core && nof_logical_wrappers == 1) {
        _PALLADIUM_BACKDOOR_SAFE_STRCPY(hw_mem_name, hw_mem_format);
      } else if (!is_per_core && nof_logical_wrappers > 1) {
        _PALLADIUM_BACKDOOR_SAFE_SPRINTF(hw_mem_name, hw_mem_format, logical_wrapper_idx);
      } else if (is_per_core && nof_logical_wrappers == 1) {
        _PALLADIUM_BACKDOOR_SAFE_SPRINTF(hw_mem_name, hw_mem_format, core_idx);
      } else {
        _PALLADIUM_BACKDOOR_SAFE_SPRINTF(hw_mem_name, hw_mem_format, core_idx, logical_wrapper_idx);
      }
      
      /* Construct the command. */
      /* Format is: "python {script} 0x{entry_in_hex} {hw_mem_name}" */
      /* entry_in_hex may be longer than 8 chars. */
      curr_cmd = cmd;
      _PALLADIUM_BACKDOOR_SAFE_SPRINTF(curr_cmd, "python %s ", _PALLADIUM_BACKDOOR_FULL_TABLE_WRITE_SCRIPT); 
      curr_cmd += strlen(curr_cmd);

      _PALLADIUM_BACKDOOR_SAFE_STRCPY(curr_cmd, "0x");
      curr_cmd += strlen(curr_cmd);

      for (entry_data_idx = 0; entry_data_idx < entry_data_len; entry_data_idx++) {
        _PALLADIUM_BACKDOOR_SAFE_SPRINTF(curr_cmd, "%08X", entry_data[entry_data_len - 1 - entry_data_idx]); 
        curr_cmd += strlen(curr_cmd);
      }

      _PALLADIUM_BACKDOOR_SAFE_SPRINTF(curr_cmd, " %s", hw_mem_name); 
     
      /* Execute the command. */
      int_ret = system(cmd);

      child_exit_status = WEXITSTATUS(int_ret);

      if (int_ret == -1 || child_exit_status != 0) {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("There was an error executing the command \"%s\".\n"), cmd));
      }
    }
  }

exit:
  DNXC_FUNC_RETURN;
}

static int _jer2_arad_tbl_access_get_entry_bit_len(int unit, soc_mem_t mem)
{
  int field_idx;
  int entry_bit_len = -1;

  /* Find the biggest field length. */
  for (field_idx = 0; field_idx < SOC_MEM_INFO(unit, mem).nFields; field_idx++) {
    uint16 cur_len = SOC_MEM_INFO(unit, mem).fields[field_idx].bp + SOC_MEM_INFO(unit, mem).fields[field_idx].len;
    entry_bit_len = DNX_SAND_MAX(entry_bit_len, cur_len);
  }

  return entry_bit_len;
}

/**
 * Given a memory print the given entries to the supplied file in a format that is recognized by the palladium backdoor commands. 
 *  
 * @mem - The memory instance to dump. 
 * @entry_data - The data to dump. 
 * @nof_entries - The length (in entries (32 bit)) of entry_data. 
 * @file_name - The name of the file to dump the buffer to.
 */ 
static soc_error_t _palladium_backdoor_write_memory_file(
   DNX_SAND_IN int unit, 
   DNX_SAND_IN soc_mem_t mem,
   DNX_SAND_IN uint32 *entry_data,
   DNX_SAND_IN uint32 nof_entries,
   DNX_SAND_IN char *file_name
   )
{
  int entry_idx;
     int entry_word_index;
    int entry_size_in_words;
     int entry_bit_len = -1;
     int entry_hex_digit_len;
     int entry_last_word_index;
  int n;
  FILE *dump_file;

  DNXC_INIT_FUNC_DEFS;
  
     /* Size of entry in words. */
     entry_size_in_words = soc_mem_entry_words(unit, mem);
    /* Length of entry in bits. */
     entry_bit_len = _jer2_arad_tbl_access_get_entry_bit_len(unit, mem);
     /* Length of entry in hex digits, equal to length in bits divided by 4 and rounded up. */
     entry_hex_digit_len = (entry_bit_len + 3) / 4;
    /* Index of the most significant word in the entry. */
     entry_last_word_index = entry_size_in_words - 1;

  dump_file = fopen(file_name, "w");  /* this file will contain the buffer content so we can pass it to the Emulation*/
  if (!dump_file) {
    DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Could not open the dump file (%s).\n"), file_name)); 
  }

  for (entry_idx = 0; entry_idx < nof_entries; entry_idx++) {
    /* 
     * For each entry print words from last to first.
     * In the last word print only hex digits with mask != 0 (i.e. at least a single bit is used in the hex digit boundary).
     * Example: If the entry is 9 bits, then print 3 hex digits.
     *
     * The rest of the words are printed in full.
     */
    for (entry_word_index = entry_last_word_index; entry_word_index >= 0; entry_word_index--) {
      int hex_digit_num;
      int bit_num;
      uint32 word_bit_mask;

      /* How many hex digits to print in this word? */
      hex_digit_num = entry_hex_digit_len - entry_word_index * 8;
      hex_digit_num = DNX_SAND_MIN(hex_digit_num, 8);

      bit_num = entry_bit_len - entry_word_index * 32;
      bit_num = DNX_SAND_MIN(bit_num, 32);

      word_bit_mask = bit_num == 32 ? ~(uint32)0 : (1 << bit_num) - 1;

      n = fprintf(dump_file, "%0*X", hex_digit_num, entry_data[entry_idx * entry_size_in_words + entry_word_index] & word_bit_mask);
      DNXC_VERIFY(n == hex_digit_num);
    }

    n = fprintf(dump_file, "\n");
    DNXC_VERIFY(n == 1);
    
  }

  fclose(dump_file);

exit:
  DNXC_FUNC_RETURN;
}

/**
 * This function uses the backdoor mechanism to write a buffer to part of a table. 
 * The buffer is dumped to a file and the backdoor mechanism is used to issue a load 
 * buffer command for the right memory and the right offset. 
 * 
 * @mem - The memory to dump. 
 * @array_index - The array index of the memory (if not memory array then this is ignored).
 * @index_start - The index to start at.
 * @index_end - The index to stop at.
 * @entry_data - The buffer to dump to file.
 */
static soc_error_t _jer2_arad_palladium_backdoor_dispatch_partial_table_write(
    DNX_SAND_IN int       unit,
    DNX_SAND_IN soc_mem_t mem,
    DNX_SAND_IN unsigned  array_index,
    DNX_SAND_IN int       index_start,
    DNX_SAND_IN int       index_end,
    DNX_SAND_IN uint32 *  entry_data
  )
{
  int int_ret; 
  char sw_mem_name[_PALLADIUM_BACKDOOR_MAX_STRING_LEN];
  char hw_mem_format[_PALLADIUM_BACKDOOR_MAX_STRING_LEN];
  uint32 nof_logical_wrappers;
  uint32 is_per_core;
  uint32 entries_per_logical_wrapper;
  uint32 nof_entries_to_write;
  uint32 curr_entry_index;
  const int max_core_ids = 2;
  soc_error_t rv;
  int fd;
  char fname[sizeof(_PALLADIUM_BACKDOOR_DUMP_FILE_TEMPLATE)];

  DNXC_INIT_FUNC_DEFS;
       
  /* Assert the name length is under the expected maximal length. */
  DNXC_VERIFY(_PALLADIUM_BACKDOOR_SAFE_STRLEN(SOC_MEM_NAME(unit, mem)) < _PALLADIUM_BACKDOOR_MAX_SW_MEM_NAME);

  /* For memory arrays we add the array index at the end of the name. */
  if (SOC_MEM_IS_ARRAY(unit, mem)) {
    _PALLADIUM_BACKDOOR_SAFE_SPRINTF(sw_mem_name, "%s%d", SOC_MEM_NAME(unit, mem), array_index);
  } else {
    _PALLADIUM_BACKDOOR_SAFE_STRCPY(sw_mem_name, SOC_MEM_NAME(unit, mem));
  }

  /* Get the HW name format. */
  rv = _jer2_jericho_palladium_backdoor_sw_mem_to_hw_name(unit, sw_mem_name, hw_mem_format, &nof_logical_wrappers, &is_per_core);
  DNXC_IF_ERR_EXIT(rv);

  entries_per_logical_wrapper = SOC_MEM_INFO(unit, mem).index_max - SOC_MEM_INFO(unit, mem).index_min + 1;
  if (nof_logical_wrappers > 0) {
    entries_per_logical_wrapper /= nof_logical_wrappers; 
  }
   
  nof_entries_to_write = index_end - index_start + 1;
  curr_entry_index = index_start;


  strcpy(fname, _PALLADIUM_BACKDOOR_DUMP_FILE_TEMPLATE);
  fd = mkstemp(fname);
  if (fd == -1) {
    DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("There was an error opening a random file with template \"%s\".\n"), _PALLADIUM_BACKDOOR_DUMP_FILE_TEMPLATE));
  }
  close(fd);
  int_ret = chmod(fname, S_IRGRP | S_IROTH);
  if (int_ret != 0) {
    DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Could not change permissions for file \"%s\".\n"), fname));
  }

  while (nof_entries_to_write > 0) {
    uint32 logical_wrapper_idx = curr_entry_index / entries_per_logical_wrapper;
    uint32 logical_wrapper_offset = curr_entry_index % entries_per_logical_wrapper;
    uint32 curr_nof_entries_to_write = DNX_SAND_MIN(nof_entries_to_write, entries_per_logical_wrapper - logical_wrapper_offset);
    char hw_mem_names[_PALLADIUM_BACKDOOR_NOF_CORE_IDS][_PALLADIUM_BACKDOOR_MAX_STRING_LEN];
    int hw_mem_names_len = 1;
    int core_idx;

    DNXC_VERIFY(logical_wrapper_idx <= nof_logical_wrappers);
    DNXC_VERIFY(curr_nof_entries_to_write > 0);


    /* Dump the current buffer part to a file. */
    rv = _palladium_backdoor_write_memory_file(unit, mem, entry_data + curr_entry_index, curr_nof_entries_to_write, fname);
    DNXC_IF_ERR_EXIT(rv);

    if (is_per_core) {
      hw_mem_names_len = max_core_ids;
    }

    /* Construct the specific HW instance given the indices and format. */
    if (!is_per_core && nof_logical_wrappers == 0) {
      _PALLADIUM_BACKDOOR_SAFE_STRCPY(hw_mem_names[0], hw_mem_format);
    } else if (!is_per_core && nof_logical_wrappers > 0) {
      _PALLADIUM_BACKDOOR_SAFE_SPRINTF(hw_mem_names[0], hw_mem_format, logical_wrapper_idx);
    } else if (is_per_core && nof_logical_wrappers == 0) {
      _PALLADIUM_BACKDOOR_SAFE_SPRINTF(hw_mem_names[0], hw_mem_format, 0);
      _PALLADIUM_BACKDOOR_SAFE_SPRINTF(hw_mem_names[1], hw_mem_format, 1);
    } else {
      _PALLADIUM_BACKDOOR_SAFE_SPRINTF(hw_mem_names[0], hw_mem_format, 0, logical_wrapper_idx);
      _PALLADIUM_BACKDOOR_SAFE_SPRINTF(hw_mem_names[1], hw_mem_format, 1, logical_wrapper_idx);
    }

    for (core_idx = 0; core_idx < hw_mem_names_len; core_idx++) {
      char cmd[_PALLADIUM_BACKDOOR_MAX_STRING_LEN];
      int child_exit_status = 0;

      /* Construct the command */
      /* Format: "python {script} {dump-file} {hw-mem-name} {first-index} {last-index}" */
      _PALLADIUM_BACKDOOR_SAFE_SPRINTF(cmd, "python %s %s %s %d %d", 
                                              _PALLADIUM_BACKDOOR_PARTIAL_TABLE_WRITE_SCRIPT, 
                                              fname, 
                                              hw_mem_names[core_idx], 
                                              logical_wrapper_offset, 
                                              curr_nof_entries_to_write + logical_wrapper_offset - 1); 
      /* Execute the command. */
      int_ret = system(cmd); 

      child_exit_status = WEXITSTATUS(int_ret);

      if (int_ret == -1 || (child_exit_status) != 0) {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("There was an error executing the command \"%s\".\n"), cmd));
      }
    }

    nof_entries_to_write -= curr_nof_entries_to_write;
    curr_entry_index += curr_nof_entries_to_write;
  }

  /* Delete the file. */
  int_ret = sal_remove(fname);
  if (int_ret != 0) {
    DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Could not delete file %s.\n"), fname));
  }
    
exit:
  DNXC_FUNC_RETURN;
}

#else /* #if defined(__linux) && !defined(__KERNEL__) */

soc_error_t _jer2_arad_palladium_backdoor_dispatch_full_table_write(
   DNX_SAND_IN int unit, 
   DNX_SAND_IN char *sw_mem_name,
   DNX_SAND_IN uint32 entry_data[],
   DNX_SAND_IN uint32 entry_data_len
   )
{
      
  DNXC_INIT_FUNC_DEFS;

  DNXC_VERIFY(FALSE);
  SOC_EXIT;

exit:
  DNXC_FUNC_RETURN;
}
  
static soc_error_t _jer2_arad_palladium_backdoor_dispatch_partial_table_write(
    DNX_SAND_IN int       unit,
    DNX_SAND_IN soc_mem_t mem,
    DNX_SAND_IN unsigned  array_index,
    DNX_SAND_IN int       index_start,
    DNX_SAND_IN int       index_end,
    DNX_SAND_IN uint32 *  entry_data
  )
{
      
  DNXC_INIT_FUNC_DEFS;

  DNXC_VERIFY(FALSE);
  SOC_EXIT;

exit:
  DNXC_FUNC_RETURN;
}

#endif /* PALLADIUM_BACKDOOR */

soc_error_t jer2_arad_tbl_access_deinit(DNX_SAND_IN int unit)
{

  DNXC_INIT_FUNC_DEFS;

  /* If it is PCID, the function returns. */
#ifdef PLISIM
  if (SAL_BOOT_PLISIM) {
    DNXC_FUNC_RETURN;
  }
#endif

  if (_jer2_arad_fill_table_with_variable_values_by_caching_buffer[unit]) {
    if (soc_mem_dmaable(unit, IRR_MCDBm, SOC_MEM_BLOCK_ANY(unit, IRR_MCDBm))) { /* check if we can use DMA */
      soc_cm_sfree(unit, _jer2_arad_fill_table_with_variable_values_by_caching_buffer[unit]);
    } else {
      dnx_sand_os_free(_jer2_arad_fill_table_with_variable_values_by_caching_buffer[unit]);
    }
    _jer2_arad_fill_table_with_variable_values_by_caching_buffer[unit] = NULL;
  }

  if (_jer2_arad_fill_table_with_variable_values_by_caching_buffer_lock[unit]) {
    sal_mutex_destroy(_jer2_arad_fill_table_with_variable_values_by_caching_buffer_lock[unit]);
    _jer2_arad_fill_table_with_variable_values_by_caching_buffer_lock[unit] = NULL;
  }

  DNXC_FUNC_RETURN;
}

soc_error_t jer2_arad_tbl_access_init_unsafe(DNX_SAND_IN int unit)
{

  DNXC_INIT_FUNC_DEFS;

  /* If it is PCID, the function returns. */
#ifdef PLISIM
  if (SAL_BOOT_PLISIM) {
    DNXC_FUNC_RETURN;
  }
#endif

  _jer2_arad_fill_table_with_variable_values_by_caching_buffer_lock[unit] = sal_mutex_create("_jer2_arad_fill_table_with_variable_values_by_caching_buffer_lock[unit]");
  if (_jer2_arad_fill_table_with_variable_values_by_caching_buffer_lock[unit] == NULL) {
    DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Could not allocate memory for a mutex.\n")));
  }

  /* Allocate 64KB of DMA memory. */
  _jer2_arad_fill_table_with_variable_values_by_caching_buffer[unit] = soc_mem_dmaable(unit, IRR_MCDBm, SOC_MEM_BLOCK_ANY(unit, IRR_MCDBm)) ? /* check if we can use DMA */
    soc_cm_salloc(unit, _jer2_arad_fill_table_with_variable_values_by_caching_buffer_len * sizeof(*_jer2_arad_fill_table_with_variable_values_by_caching_buffer[0]), 
      "_jer2_arad_fill_table_with_variable_values_by_caching_buffer[unit]") :
    (uint32*)dnx_sand_os_malloc_any_size(_jer2_arad_fill_table_with_variable_values_by_caching_buffer_len * sizeof(*_jer2_arad_fill_table_with_variable_values_by_caching_buffer[0]), 
      "_jer2_arad_fill_table_with_variable_values_by_caching_buffer[unit]");

  if (_jer2_arad_fill_table_with_variable_values_by_caching_buffer[unit] == NULL) {
    DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Could not allocate a DMA buffer of length %d bytes.\n"), 
                                          _jer2_arad_fill_table_with_variable_values_by_caching_buffer_len * 
                                                                      sizeof(*_jer2_arad_fill_table_with_variable_values_by_caching_buffer[0])));
  }

exit:
  DNXC_FUNC_RETURN;
}

int jer2_arad_update_table_with_cache(int unit, soc_mem_t mem) {
    void *buffer;
    void *buf;
    uint8 orig_read_through_flag;
    int index;
    int entry_min = soc_mem_index_min(unit, mem);
    int entry_dw  = soc_mem_entry_words(unit, mem);
    int index_cnt = soc_mem_index_count(unit, mem);
    int entry_max = soc_mem_index_max(unit, mem);
    int copyno = SOC_MEM_BLOCK_MIN(unit, mem);
    soc_error_t rv = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS;

    /* update table only if cache available and buffer allocation succeeded */
    if ((soc_mem_cache_get(unit, mem, copyno)) && (buffer = soc_cm_salloc(unit, index_cnt * entry_dw * 4, "dma")) != NULL) {
        orig_read_through_flag = SOC_MEM_FORCE_READ_THROUGH(unit);
        SOC_MEM_FORCE_READ_THROUGH_SET(unit, 0); /* read from cache */
        for (index = entry_min; index <= entry_max; index++) {
            buf = soc_mem_table_idx_to_pointer(unit, mem, void *, buffer, index);
            rv = soc_mem_array_read(unit, mem, 0, copyno, index, buf);
            if(SOC_FAILURE(rv)) {
                SOC_MEM_FORCE_READ_THROUGH_SET(unit, orig_read_through_flag);
                DNXC_EXIT_WITH_ERR_NO_MSG(rv);
            }
        }
        rv = soc_mem_write_range(unit, mem, COPYNO_ALL, entry_min, entry_max, buffer);
        soc_cm_sfree(unit, buffer);
        SOC_MEM_FORCE_READ_THROUGH_SET(unit, orig_read_through_flag);
        DNXC_IF_ERR_EXIT(rv);
    } else {
        LOG_ERROR(BSL_LS_SOC_INIT,
                    (BSL_META_U(unit,
                                "%s table is not updated with cache\n"), SOC_MEM_NAME(unit, mem)));
        DNXC_EXIT_WITH_ERR_NO_MSG(SOC_E_FAIL);
    }
exit:
    DNXC_FUNC_RETURN;  
}

/*
 * BRIEF 
 *   Use DMA to write consecutive entries to a memory by first filling a
 *   buffer with the entries, and then writing from the buffer using DMA.
 * DESCRIPTION 
 *   The function uses a big DMA buffer to buffer writes to a table, and then write
 *   them to HW using DMA.
 *   The user specifies the values using a callback for each entry.
 *   It is assumed that all entries are 32 bits or less.
 *   Each entry is a uint32 (even when in hardware it is less than 32 bits)
 *   and can be used like an entry read or written to HW.
 * PARAMS 
 *   unit
 *   mem - The memory to write to.
 *   array_index - Index of the memory array to write to.
 *   copyno - The block to write to (MEM_BLOCK_ANY can be used).
 *   index_start - The index to start writing at. If -1 then the minimal index of the memory is used.
 *   index_end - The index to stop writing at (inclusive - this index will be written). If -1 then the maximal index of the memory is used.
 *   callback - This callback should set the value of each entry to be written.
 *              For more info see jer2_arad_fill_table_with_variable_values_by_caching_callback.
 *   opaque - User data to be passed back untouched to the callback.
 * REMARKS 
 *   The entry contents are not read before writing currently.
 *   Therefore the entry passed to the callback is 0, and should be filled completely.
 * POSSIBLE IMPROVEMENTS 
 *   1. The array_index should be used to enable specifying array_index.
 *   2. There should be an option to instruct the mechanism to read the table contents
 *      before writing.
 * EXAMPLE 
 *    
 */
soc_error_t 
  jer2_arad_fill_table_with_variable_values_by_caching(
    DNX_SAND_IN int       unit,
                          soc_mem_t mem,
    DNX_SAND_IN unsigned  array_index,
    DNX_SAND_IN int       copyno,
    DNX_SAND_IN int       index_start,
    DNX_SAND_IN int       index_end,
    DNX_SAND_IN jer2_arad_fill_table_with_variable_values_by_caching_callback callback,
    DNX_SAND_IN void *    opaque
  )
{
  int index;
  /* Index inside buffer */
  unsigned int buf_idx = 0;
  /* Index to first entry of the buffer. */
  unsigned int buf_start_entry_idx;
  uint32 *buf = _jer2_arad_fill_table_with_variable_values_by_caching_buffer[unit];
  uint32 old_rv;
  soc_error_t new_rv;
  uint8 do_emul = SOC_DNX_CONFIG(unit)->emulation_system && (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "no_backdoor", 0) != 1);
  uint8 do_dma = TRUE;
  int real_index_start;
  int real_index_end;
  int entry_size_in_words;
  int buffer_len_in_words = _jer2_arad_fill_table_with_variable_values_by_caching_buffer_len;
  
  DNXC_INIT_FUNC_DEFS;

  if (strcmp(SOC_MEM_NAME(unit, mem), "EPNI_REMARK_MPLS_TO_EXP") == 0 || 
      strcmp(SOC_MEM_NAME(unit, mem), "EPNI_REMARK_MPLS_TO_DSCP") == 0)
  {
    do_emul = FALSE;
  }

  if (!soc_mem_is_valid(unit, mem)) {
    DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("%d: Invalid memory specified for unit %d.\n"),  mem, unit));
  }

  if (callback == NULL) {
    SOC_EXIT;
  }

  SOC_MEM_ALIAS_TO_ORIG(unit,mem);
  entry_size_in_words = soc_mem_entry_words(unit, mem);
  DNXC_VERIFY(entry_size_in_words <= buffer_len_in_words);
  
  real_index_start = index_start == -1 ? SOC_MEM_INFO(unit, mem).index_min : index_start;
  real_index_end = index_end == -1 ? SOC_MEM_INFO(unit, mem).index_max : index_end;
  
  /* If no DMA we use direct write. */
  if (!soc_mem_slamable(unit, mem, copyno)
#ifdef PLISIM
      || (SAL_BOOT_PLISIM)
#endif
      )
  {
    int index, blk;
    uint32 buf[SOC_MAX_MEM_WORDS];

    SOC_MEM_BLOCK_ITER(unit, mem, blk) 
    {
      if (copyno != COPYNO_ALL && copyno != blk) {
        continue;
      }
      for (index = real_index_start; index <= real_index_end; ++index) 
      {
        sal_memset(buf, 0, sizeof(buf[0]) * entry_size_in_words);

        if (callback(unit, copyno, array_index, index, buf, entry_size_in_words, opaque) != 0) {
          SOC_EXIT;
        }

        old_rv = soc_mem_array_write(unit, mem, array_index, blk, index, (void*)&buf);
        DNXC_SAND_IF_ERR_EXIT(old_rv);
      }
    }

    DNXC_FUNC_RETURN;
  }

  DNXC_VERIFY(buf != NULL);

  sal_mutex_take(_jer2_arad_fill_table_with_variable_values_by_caching_buffer_lock[unit], sal_mutex_FOREVER);
    
  for (index = real_index_start, buf_start_entry_idx = real_index_start; index <= real_index_end; ++index) 
  {
    /* First clear the entry and then fill it. */
    sal_memset(buf + buf_idx, 0, sizeof(buf[0]) * entry_size_in_words);
    if (callback(unit, copyno, array_index, index, buf + buf_idx, entry_size_in_words, opaque) != 0) {
      SOC_EXIT;
    }

    buf_idx += entry_size_in_words;
    /* In case entry_sz == 1, last (fillable) entry is len - 1. */
    /* In case entry_sz > 1, last (fillable) entry is len - entry_sz. */
    /* Therefore if (buf_idx > len - entry_sz) we must flush the buffer.*/
    if (buf_idx > buffer_len_in_words - entry_size_in_words) {
      /* Flush the buffer. */
      if (do_emul) {
        do_dma = FALSE;
        new_rv = _jer2_arad_palladium_backdoor_dispatch_partial_table_write(unit, mem, array_index,  buf_start_entry_idx, index, buf);
        DNXC_IF_ERR_EXIT(new_rv);
      } 
      if (do_dma) {
        old_rv = soc_mem_array_write_range(unit, 0, mem, array_index, copyno, buf_start_entry_idx, index, buf);
        DNXC_SAND_IF_ERR_EXIT(old_rv);
      }

      buf_start_entry_idx = index + 1;
      buf_idx = 0;
      
    }
  }

  if (buf_start_entry_idx <= real_index_end) {
    /* Flush the buffer. */
    if (do_emul) {
      do_dma = FALSE;
      new_rv = _jer2_arad_palladium_backdoor_dispatch_partial_table_write(unit, mem,array_index, buf_start_entry_idx, index - 1, buf);
      DNXC_IF_ERR_EXIT(new_rv);
    } 
    if (do_dma) {
      old_rv = soc_mem_array_write_range(unit, 0, mem, array_index, copyno, buf_start_entry_idx, index - 1, buf);
      DNXC_SAND_IF_ERR_EXIT(old_rv);
    }
  }

  sal_mutex_give(_jer2_arad_fill_table_with_variable_values_by_caching_buffer_lock[unit]);

exit:
  DNXC_FUNC_RETURN;
}

int
jer2_arad_fill_memory_with_incremental_field(const int unit, const soc_mem_t mem, const soc_field_t field,
                                unsigned array_index_min, unsigned array_index_max,
                                const int copyno,
                                int index_min, int index_max,
                                const void *initial_entry)
{
    int    rv = 0, mem_words, mem_size, entry_words, indices_num;
    int    index, blk, tmp;
    unsigned array_index;
    uint32 *buf;
    uint32 *buf2;
    const uint32 *input_entry = initial_entry;
    uint32 field_buf[4] = {0}; /* To hold the field, max size 128 bits */
    int dmaable = soc_mem_dmaable(unit, mem, SOC_MEM_BLOCK_ANY(unit, mem)); /* check if we can use DMA */

    if (initial_entry == NULL) {
        return SOC_E_PARAM;
    }

    /* get legal values for indices, if too small/big use the memory's boundaries */
    tmp = soc_mem_index_min(unit, mem);
    if (index_min < soc_mem_index_min(unit, mem)) {
      index_min = tmp;
    }
    if (index_max < index_min) {
        index_max = index_min;
    } else {
         tmp = soc_mem_index_max(unit, mem);
         if (index_max > tmp) {
             index_max = tmp;
         }
    }

    entry_words = soc_mem_entry_words(unit, mem);
    indices_num = index_max - index_min + 1;
    mem_words = indices_num * entry_words;
    mem_size = mem_words * 4;

    /* get the initial field from the input */
    soc_mem_field_get(unit, mem, initial_entry, field, field_buf);

    if (dmaable) {
        buf = soc_cm_salloc(unit, mem_size, "mem_clear_buf"); /* allocate DMA memory buffer */
    } else {
        buf = (uint32*)dnx_sand_os_malloc_any_size(mem_size, "mem_clear_buf");
    }
    if (buf == NULL) {
        return SOC_E_MEMORY;
    }

    /* get legal values for memory array indices */
    if (SOC_MEM_IS_ARRAY(unit, mem)) {
        soc_mem_array_info_t *maip = SOC_MEM_ARRAY_INFOP(unit, mem);
        if (maip) {
            if (array_index_max >= maip->numels) {
                array_index_max = maip->numels - 1;
            }
        } else {
            array_index_max = 0;
        }
        if (array_index_min > array_index_max) {
            array_index_min = array_index_max;
        }
    } else {
        array_index_min = array_index_max = 0;
    }

    /* fill the allocated memory with the input entry */
    for (index = 0; index < mem_words; ++index) {
        buf[index] = input_entry[index % entry_words];
    }

    SOC_MEM_BLOCK_ITER(unit, mem, blk) {
        if (copyno != COPYNO_ALL && copyno != blk) {
            continue;
        }
        for (array_index = array_index_min; array_index <= array_index_min; ++array_index) {
            /* update the field of all the entries in the buffer */
            for (index = 0, buf2 = buf; index < indices_num; ++index, buf2+=entry_words) {
                soc_mem_field_set(unit, mem, buf2, field, field_buf); /* set the index */
                /* increment the field, to be used in next entry */
                if (!++field_buf[0]) {
                    if (!++field_buf[1]) {
                        if (!++field_buf[2]) {
                            ++field_buf[3];
                        }
                    }
                }
            }

#ifdef PLISIM
            if (SAL_BOOT_PLISIM) {
                for (index = index_min, buf2 = buf; index <= index_max; ++index, buf2+=entry_words) {
                    if ((rv = soc_mem_array_write(unit, mem, array_index, blk, index, buf2)) < 0) {
                        LOG_INFO(BSL_LS_SOC_MEM,
                                 (BSL_META_U(unit,
                                             "Write ERROR: table %s.%d[%d]: %s\n"),
                                             SOC_MEM_UFNAME(unit, mem), copyno==COPYNO_ALL ? 0 : copyno, index,
                                  soc_errmsg(rv)));
                    }
                }
            }
            else 
#endif
            {
                if ((rv = soc_mem_array_write_range(unit, 0, mem, array_index, blk, index_min, index_max, buf)) < 0) {
                    LOG_INFO(BSL_LS_SOC_MEM,
                             (BSL_META_U(unit,
                                         "Write ERROR: table %s.%d[%d-%d]: %s\n"),
                                         SOC_MEM_UFNAME(unit, mem), copyno==COPYNO_ALL ? 0 : copyno, index_min, index_max,
                              soc_errmsg(rv)));
                }
            }
        }
    }
    if (dmaable) {
        soc_cm_sfree(unit, buf);
    } else  {
        dnx_sand_os_free(buf);
    }
    return rv;
}

/*
 * Read indirect table nif_ctxt_map_tbl from block IRE,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ire_nif_ctxt_map_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IRE_NIF_CTXT_MAP_TBL_DATA* IRE_nif_ctxt_map_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IRE_NIF_CTXT_MAP_TBL_ENTRY_SIZE];
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRE_NIF_CTXT_MAP_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IRE_nif_ctxt_map_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IRE_NIF_CTXT_MAP_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          IRE_NIF_CTXT_MAPm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

    err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IRE_nif_ctxt_map_tbl_data->reassembly_context = soc_mem_field32_get(
                  unit,
                  IRE_NIF_CTXT_MAPm,
                  data,
                  REASSEMBLY_CONTEXTf);
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 35, exit);

    if(SOC_E_NONE == err) {
          IRE_nif_ctxt_map_tbl_data->port_termination_context = soc_mem_field32_get(
                  unit,
                  IRE_NIF_CTXT_MAPm,
                  data,
                  PORT_TERMINATION_CONTEXTf);
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ire_nif_ctxt_map_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table nif_ctxt_map_tbl from block IRE,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ire_nif_ctxt_map_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IRE_NIF_CTXT_MAP_TBL_DATA* IRE_nif_ctxt_map_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IRE_NIF_CTXT_MAP_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRE_NIF_CTXT_MAP_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  soc_mem_field32_set(
          unit,
          IRE_NIF_CTXT_MAPm,
          data,
          REASSEMBLY_CONTEXTf,
          IRE_nif_ctxt_map_tbl_data->reassembly_context );

  soc_mem_field32_set(
          unit,
          IRE_NIF_CTXT_MAPm,
          data,
          PORT_TERMINATION_CONTEXTf,
          IRE_nif_ctxt_map_tbl_data->port_termination_context );

  err = soc_mem_write(
          unit,
          IRE_NIF_CTXT_MAPm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ire_nif_ctxt_map_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table rcy_ctxt_map_tbl from block IRE,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ire_rcy_ctxt_map_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IRE_RCY_CTXT_MAP_TBL_DATA* IRE_rcy_ctxt_map_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IRE_RCY_CTXT_MAP_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRE_RCY_CTXT_MAP_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IRE_rcy_ctxt_map_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IRE_RCY_CTXT_MAP_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          IRE_RCY_CTXT_MAPm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);


  IRE_rcy_ctxt_map_tbl_data->reassembly_context   = soc_mem_field32_get(
                  unit,
                  IRE_RCY_CTXT_MAPm,
                  data,
                  REASSEMBLY_CONTEXTf            );

  IRE_rcy_ctxt_map_tbl_data->port_termination_context   = soc_mem_field32_get(
                  unit,
                  IRE_RCY_CTXT_MAPm,
                  data,
                  PORT_TERMINATION_CONTEXTf            );

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ire_rcy_ctxt_map_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table rcy_ctxt_map_tbl from block IRE,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ire_rcy_ctxt_map_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IRE_RCY_CTXT_MAP_TBL_DATA* IRE_rcy_ctxt_map_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IRE_RCY_CTXT_MAP_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRE_RCY_CTXT_MAP_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  soc_mem_field32_set(
          unit,
          IRE_RCY_CTXT_MAPm,
          data,
          PORT_TERMINATION_CONTEXTf,
          IRE_rcy_ctxt_map_tbl_data->port_termination_context );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  soc_mem_field32_set(
          unit,
          IRE_RCY_CTXT_MAPm,
          data,
          REASSEMBLY_CONTEXTf,
          IRE_rcy_ctxt_map_tbl_data->reassembly_context );

  err = soc_mem_write(
          unit,
          IRE_RCY_CTXT_MAPm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ire_rcy_ctxt_map_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table rcy_ctxt_map_tbl from block IRE,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ire_cpu_ctxt_map_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IRE_CPU_CTXT_MAP_TBL_DATA* IRE_cpu_ctxt_map_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IRE_CPU_CTXT_MAP_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRE_CPU_CTXT_MAP_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IRE_cpu_ctxt_map_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IRE_CPU_CTXT_MAP_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          IRE_CPU_CTXT_MAPm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);


    err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IRE_cpu_ctxt_map_tbl_data->reassembly_context   = soc_mem_field32_get(
                  unit,
                  IRE_CPU_CTXT_MAPm,
                  data,
                  REASSEMBLY_CONTEXTf            );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 35, exit);

    if(SOC_E_NONE == err) {
          IRE_cpu_ctxt_map_tbl_data->port_termination_context   = soc_mem_field32_get(
                  unit,
                  IRE_CPU_CTXT_MAPm,
                  data,
                  PORT_TERMINATION_CONTEXTf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ire_cpu_ctxt_map_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table rcy_ctxt_map_tbl from block IRE,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ire_cpu_ctxt_map_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IRE_CPU_CTXT_MAP_TBL_DATA* IRE_cpu_ctxt_map_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IRE_CPU_CTXT_MAP_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRE_CPU_CTXT_MAP_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IRE_CPU_CTXT_MAPm,
          data,
          PORT_TERMINATION_CONTEXTf,
          IRE_cpu_ctxt_map_tbl_data->port_termination_context );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IRE_CPU_CTXT_MAPm,
          data,
          REASSEMBLY_CONTEXTf,
          IRE_cpu_ctxt_map_tbl_data->reassembly_context );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  err = soc_mem_write(
          unit,
          IRE_CPU_CTXT_MAPm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ire_cpu_ctxt_map_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table dynamic_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

int
  jer2_arad_iqm_dynamic_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_DYNAMIC_TBL_DATA* IQM_dynamic_tbl_data
  )
{
    uint32 err;
    uint32 data[JER2_ARAD_IQM_DYNAMIC_TBL_ENTRY_SIZE];
    int core_index = (core == SOC_CORE_ALL) ? 0 : core;
 
    DNXC_INIT_FUNC_DEFS;

    err = dnx_sand_os_memset(&(data[0]),0x0,sizeof(data));
    DNXC_SAND_IF_ERR_EXIT(err);

    err = dnx_sand_os_memset(IQM_dynamic_tbl_data, 0x0, sizeof(JER2_ARAD_IQM_DYNAMIC_TBL_DATA));
    DNXC_SAND_IF_ERR_EXIT(err);

    DNXC_IF_ERR_EXIT(soc_mem_read(unit, IQM_PQDMDm, IQM_BLOCK(unit, core_index), entry_offset, data));

    IQM_dynamic_tbl_data->pq_head_ptr   = soc_mem_field32_get(unit, IQM_PQDMDm, data, PQ_HEAD_PTRf);

    IQM_dynamic_tbl_data->que_not_empty   = soc_mem_field32_get(unit, IQM_PQDMDm, data, QUE_NOT_EMPTYf);

    IQM_dynamic_tbl_data->pq_inst_que_size   = soc_mem_field32_get(unit, IQM_PQDMDm, data, PQ_INST_QUE_SIZEf);

    IQM_dynamic_tbl_data->pq_inst_que_buff_size   = soc_mem_field32_get(unit, IQM_PQDMDm, data, PQ_INST_QUE_BUFF_SIZEf);

    IQM_dynamic_tbl_data->pq_avrg_szie   = soc_mem_field32_get(unit, IQM_PQDMDm, data, PQ_AVRG_SIZEf);

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Write indirect table dynamic_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_dynamic_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_DYNAMIC_TBL_DATA* IQM_dynamic_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_DYNAMIC_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_DYNAMIC_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

 
 




err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_MEM_8000000m,
          data,
          ITEM_0_20f /* PQ_HEAD_PTRf */,
          IQM_dynamic_tbl_data->pq_head_ptr );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_MEM_8000000m,
          data,
          ITEM_21_21f /* QUE_NOT_EMPTYf */,
          IQM_dynamic_tbl_data->que_not_empty );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_MEM_8000000m,
          data,
          ITEM_22_48f /* PQ_INST_QUE_SIZEf */,
          IQM_dynamic_tbl_data->pq_inst_que_size );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_MEM_8000000m,
          data,
          ITEM_49_69f /* PQ_INST_QUE_BUFF_SIZE */,
          IQM_dynamic_tbl_data->pq_inst_que_buff_size );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_MEM_8000000m,
          data,
          ITEM_70_81f /* PQ_AVRG_SZIEf */,
          IQM_dynamic_tbl_data->pq_avrg_szie );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 33, exit);

  err = soc_mem_write(
          unit,
          IQM_MEM_8000000m,
          IQM_BLOCK(unit, core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 34, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_dynamic_tbl_set_unsafe()",0,0);
}
/*
 * Read indirect table static_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

int
  jer2_arad_iqm_static_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_STATIC_TBL_DATA* IQM_static_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_STATIC_TBL_ENTRY_SIZE];
  int core_index = (core == SOC_CORE_ALL) ? 0 : core;
 
  DNXC_INIT_FUNC_DEFS;

  err = dnx_sand_os_memset(&(data[0]), 0x0, sizeof(data));
  DNXC_SAND_IF_ERR_EXIT(err);

  err = dnx_sand_os_memset(IQM_static_tbl_data, 0x0, sizeof(JER2_ARAD_IQM_STATIC_TBL_DATA));
  DNXC_SAND_IF_ERR_EXIT(err);
 
  DNXC_IF_ERR_EXIT(soc_mem_read(unit, IQM_PQDMSm, IQM_BLOCK(unit, core_index), entry_offset, data));

  IQM_static_tbl_data->credit_class   = soc_mem_field32_get(unit, IQM_PQDMSm, data, CREDIT_CLASSf);

  IQM_static_tbl_data->rate_class   = soc_mem_field32_get(unit, IQM_PQDMSm, data, RATE_CLASSf);

  IQM_static_tbl_data->connection_class   = soc_mem_field32_get(unit, IQM_PQDMSm, data, CONNECTION_CLASSf);

  IQM_static_tbl_data->traffic_class   = soc_mem_field32_get(unit, IQM_PQDMSm, data, TRAFFIC_CLASSf);

  IQM_static_tbl_data->que_signature   = soc_mem_field32_get(unit, IQM_PQDMSm, data, QUE_SIGNATUREf);

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Write indirect table static_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

int
  jer2_arad_iqm_static_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_STATIC_TBL_DATA* IQM_static_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_STATIC_TBL_ENTRY_SIZE];
 
  DNXC_INIT_FUNC_DEFS;

  err = dnx_sand_os_memset(&(data[0]), 0x0,sizeof(data));
  DNXC_SAND_IF_ERR_EXIT(err);

  soc_mem_field32_set(unit,IQM_PQDMSm, data, CREDIT_CLASSf, IQM_static_tbl_data->credit_class);

  soc_mem_field32_set(unit, IQM_PQDMSm, data, RATE_CLASSf, IQM_static_tbl_data->rate_class);

  soc_mem_field32_set(unit,IQM_PQDMSm, data, CONNECTION_CLASSf, IQM_static_tbl_data->connection_class);

  soc_mem_field32_set(unit, IQM_PQDMSm, data, TRAFFIC_CLASSf, IQM_static_tbl_data->traffic_class);

  soc_mem_field32_set(unit, IQM_PQDMSm, data, QUE_SIGNATUREf, IQM_static_tbl_data->que_signature);

  err = soc_mem_write(unit, IQM_PQDMSm, MEM_BLOCK_ANY, entry_offset,data);
  DNXC_SAND_IF_ERR_EXIT(err);

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Read indirect table packet_queue_red_weight_table_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_packet_queue_red_weight_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_PACKET_QUEUE_RED_WEIGHT_TABLE_TBL_DATA* IQM_packet_queue_red_weight_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_PACKET_QUEUE_RED_WEIGHT_TABLE_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_PACKET_QUEUE_RED_WEIGHT_TABLE_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IQM_packet_queue_red_weight_table_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IQM_PACKET_QUEUE_RED_WEIGHT_TABLE_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

   err = soc_mem_read(
          unit,
          IQM_PQWQm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  err=SOC_E_NONE;
  if(SOC_E_NONE == err) {
          IQM_packet_queue_red_weight_table_tbl_data->pq_weight   = soc_mem_field32_get(
                  unit,
                  IQM_PQWQm,
                  data,
                  PQ_WEIGHTf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

  if(SOC_E_NONE == err) {
          IQM_packet_queue_red_weight_table_tbl_data->avrg_en   = soc_mem_field32_get(
                  unit,
                  IQM_PQWQm,
                  data,
                  PQ_AVRG_ENf);
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 41, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_packet_queue_red_weight_table_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table packet_queue_red_weight_table_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_packet_queue_red_weight_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_PACKET_QUEUE_RED_WEIGHT_TABLE_TBL_DATA* IQM_packet_queue_red_weight_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_PACKET_QUEUE_RED_WEIGHT_TABLE_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_PACKET_QUEUE_RED_WEIGHT_TABLE_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err=SOC_E_NONE;
  if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_PQWQm,
          data,
          PQ_WEIGHTf,
          IQM_packet_queue_red_weight_table_tbl_data->pq_weight );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_PQWQm,
          data,
          PQ_AVRG_ENf,
          IQM_packet_queue_red_weight_table_tbl_data->avrg_en );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

  err = soc_mem_write(
          unit,
          IQM_PQWQm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_packet_queue_red_weight_table_tbl_set_unsafe()",0,0);
}
/*
 * Read indirect table credit_discount_table_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_credit_discount_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_CREDIT_DISCOUNT_TABLE_TBL_DATA* IQM_credit_discount_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_CREDIT_DISCOUNT_TABLE_TBL_ENTRY_SIZE];
  uint32 fld_val;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_CREDIT_DISCOUNT_TABLE_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);
  if (!SOC_IS_QAX(unit)) {
  err = dnx_sand_os_memset(
          IQM_credit_discount_table_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IQM_CREDIT_DISCOUNT_TABLE_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          IQM_CRDTDISm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 20, exit);

  err=SOC_E_NONE;
  if (SOC_IS_JERICHO(unit)) {
      int val, sign;
      /*There is only one field in jer2_jerticho discout value, where the sign value should be the MSB*/
      fld_val = soc_mem_field32_get(
                    unit,
                    IQM_CRDTDISm,
                    data,
                    DATAf);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);
      /* Fld_val is represented in 2's complement */
      sign =  fld_val >> JER2_ARAD_IQM_CRDTDIS_DATA_NOF_BITS;
      val = fld_val;
      if (sign) {
          val = ~(val - 1);
      }
      IQM_credit_discount_table_tbl_data->crdt_disc_val = val;
      IQM_credit_discount_table_tbl_data->discnt_sign = sign;
  } else {
      if(SOC_E_NONE == err) {
            IQM_credit_discount_table_tbl_data->crdt_disc_val = soc_mem_field32_get(
                    unit,
                    IQM_CRDTDISm,
                    data,
                    CRDT_DISC_VALf);
          }
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);
      if(SOC_E_NONE == err) {
            IQM_credit_discount_table_tbl_data->discnt_sign = soc_mem_field32_get(
                    unit,
                    IQM_CRDTDISm,
                    data,
                    DISCNT_SIGNf);
          }
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 41, exit);
  }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_credit_discount_table_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table credit_discount_table_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_credit_discount_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_CREDIT_DISCOUNT_TABLE_TBL_DATA* IQM_credit_discount_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_CREDIT_DISCOUNT_TABLE_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_CREDIT_DISCOUNT_TABLE_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);
 
err=SOC_E_NONE;
if (!SOC_IS_QAX(unit)) {
  if (SOC_IS_JERICHO(unit)) {
      int sign =  (IQM_credit_discount_table_tbl_data->discnt_sign & 1) << JER2_ARAD_IQM_CRDTDIS_DATA_NOF_BITS;
      int val = (IQM_credit_discount_table_tbl_data->crdt_disc_val & 0x7F);
      if (sign) {
          val = ((~val & 0x7F) + 1) | sign;
      }
      /*There is only one field in jer2_jerticho discout value, where the sign value should be the MSB*/
      soc_mem_field32_set(
        unit,
        IQM_CRDTDISm,
        data,
        DATAf,
        val);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 20, exit);
  } else {
      if(SOC_E_NONE == err) {
            soc_mem_field32_set(
              unit,
              IQM_CRDTDISm,
              data,
              CRDT_DISC_VALf,
              IQM_credit_discount_table_tbl_data->crdt_disc_val );
            }
        DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);
      if(SOC_E_NONE == err) {
            soc_mem_field32_set(
              unit,
              IQM_CRDTDISm,
              data,
              DISCNT_SIGNf,
              IQM_credit_discount_table_tbl_data->discnt_sign );
      }
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);
  }
  err = soc_mem_write(
          unit,
          IQM_CRDTDISm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);
}
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_credit_discount_table_tbl_set_unsafe()",0,0);
}

/* 
 * This procedure is meant to determine the number of bits in the mantissa for 
 * max queue size, 
 * and max queue size in BDs. 
 *  
 * Arad and Jericho share this code, 
 * But they have different values in the mantissa exponnent, for taildrop configuration
 * Moreover the CNRED/PQRED configuartion are different in Jericho.
 */
uint32
    jer2_arad_iqm_packet_queue_red_parameters_tail_drop_mantissa_nof_bits (
       DNX_SAND_IN   int             unit,
       DNX_SAND_IN   uint32             drop_precedence_ndx,
       DNX_SAND_OUT  uint32*            max_mantissa_size,
       DNX_SAND_OUT  uint32*            max_bds_mantissa_size
       )
{
    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TAIL_DROP_MANTISSA_NOF_BITS);
    if (drop_precedence_ndx == DNX_TMC_NOF_DROP_PRECEDENCE) {
        /* 
         * If receives drop_precedence_ndx == DNX_TMC_NOF_DROP_PRECEDENCE will read from the ECN (IQM_CNREDm) configuration.
         */
        if (SOC_IS_JERICHO(unit)){
            *max_mantissa_size = JER2_JERICHO_IQM_CNRED_PQ_MAX_QUE_SIZE_MANTISSA;
            *max_bds_mantissa_size = JER2_JERICHO_IQM_CNRED_PQ_MAX_QUE_BUFF_SIZE_MANTISSA;
        } else {
            *max_mantissa_size = JER2_ARAD_IQM_CNRED_PQ_MAX_QUE_SIZE_MANTISSA;
            *max_bds_mantissa_size = JER2_ARAD_IQM_CNRED_PQ_MAX_QUE_BUFF_SIZE_MANTISSA;
        }
    } else {
        /* 
         * If receives drop_precedence_ndx == DNX_TMC_NOF_DROP_PRECEDENCE will read from the IQM_PQRED configuration.
         */
        if (SOC_IS_JERICHO(unit)){
            *max_mantissa_size = JER2_JERICHO_IQM_PQRED_PQ_MAX_QUE_SIZE_MANTISSA;
            *max_bds_mantissa_size = JER2_JERICHO_IQM_PQRED_PQ_MAX_QUE_BUFF_SIZE_MANTISSA;
        } else {
            *max_mantissa_size = JER2_ARAD_IQM_PQRED_PQ_MAX_QUE_SIZE_MANTISSA;
            *max_bds_mantissa_size = JER2_ARAD_IQM_PQRED_PQ_MAX_QUE_BUFF_SIZE_MANTISSA;
         }
    }
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_packet_queue_red_parameters_tail_drop_mantissa_nof_bits()",0, drop_precedence_ndx);
}
/*
 * Read indirect table packet_queue_red_parameters_table_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 *
 * If receives drop_precedence_ndx == DNX_TMC_NOF_DROP_PRECEDENCE will read from the ECN (IQM_CNREDm) configuration.
 */
uint32
  jer2_arad_iqm_packet_queue_red_parameters_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32             rt_cls_ndx,
    DNX_SAND_IN   uint32             drop_precedence_ndx,
    DNX_SAND_OUT  JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_DATA* IQM_packet_queue_red_parameters_table_tbl_data
  )
{
  uint32 exp_man, err;
  uint32 entry_offset, data[JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_ENTRY_SIZE] = {0};
  soc_mem_t mem;
  int32 
      core_id = 0;
  uint32 
      max_mantissa_size,
      max_bds_mantissa_size ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_GET_UNSAFE);

  DNX_SAND_CHECK_FUNC_RESULT(dnx_sand_os_memset(IQM_packet_queue_red_parameters_table_tbl_data, 0x0,
    sizeof(JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_DATA)), 15, exit);
  if (drop_precedence_ndx == DNX_TMC_NOF_DROP_PRECEDENCE) {
    mem = IQM_CNREDm;
    entry_offset = rt_cls_ndx;
  } else { 
    mem = IQM_PQREDm;
    entry_offset = (rt_cls_ndx * JER2_ARAD_WRED_NOF_DROP_PRECEDENCE) + drop_precedence_ndx;
  }
  err = jer2_arad_iqm_packet_queue_red_parameters_tail_drop_mantissa_nof_bits(unit,
                                                                         drop_precedence_ndx,
                                                                         &max_mantissa_size,
                                                                         &max_bds_mantissa_size);
  DNX_SAND_CHECK_FUNC_RESULT(err, 20, exit);
  
  err = soc_mem_read(unit, mem, IQM_BLOCK(unit, core_id), entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);
  exp_man = soc_mem_field32_get(unit, mem, data, PQ_MAX_QUE_SIZEf);
  jer2_arad_iqm_mantissa_exponent_get(unit, exp_man, max_mantissa_size,
    &(IQM_packet_queue_red_parameters_table_tbl_data->pq_max_que_size_mnt),
    &(IQM_packet_queue_red_parameters_table_tbl_data->pq_max_que_size_exp));

  exp_man = soc_mem_field32_get(unit, mem, data, PQ_MAX_QUE_BUFF_SIZEf);
  jer2_arad_iqm_mantissa_exponent_get(unit, exp_man, max_bds_mantissa_size,
    &(IQM_packet_queue_red_parameters_table_tbl_data->pq_max_que_size_bds_mnt),
    &(IQM_packet_queue_red_parameters_table_tbl_data->pq_max_que_size_bds_exp));

  IQM_packet_queue_red_parameters_table_tbl_data->pq_wred_en = soc_mem_field32_get(unit, mem, data, PQ_WRED_ENf);
  IQM_packet_queue_red_parameters_table_tbl_data->pq_c2 = soc_mem_field32_get(unit, mem, data, PQ_C_2f);
  IQM_packet_queue_red_parameters_table_tbl_data->pq_c3 = soc_mem_field32_get(unit, mem, data, PQ_C_3f);
  IQM_packet_queue_red_parameters_table_tbl_data->pq_c1 = soc_mem_field32_get(unit, mem, data, PQ_C_1f);
  IQM_packet_queue_red_parameters_table_tbl_data->pq_avrg_max_th = soc_mem_field32_get(unit, mem, data, PQ_AVRG_MAX_THf);
  IQM_packet_queue_red_parameters_table_tbl_data->pq_avrg_min_th = soc_mem_field32_get(unit, mem, data, PQ_AVRG_MIN_THf);
  IQM_packet_queue_red_parameters_table_tbl_data->pq_wred_pckt_sz_ignr = soc_mem_field32_get(unit, mem, data, PQ_WRED_PCKT_SZ_IGNRf);
  IQM_packet_queue_red_parameters_table_tbl_data->addmit_logic =
    drop_precedence_ndx == DNX_TMC_NOF_DROP_PRECEDENCE ? 0 :
    soc_mem_field32_get(unit, IQM_PQREDm, data, ADDMIT_LOGICf);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_packet_queue_red_parameters_table_tbl_get_unsafe()", rt_cls_ndx, drop_precedence_ndx);
}

/*
 * Write indirect table packet_queue_red_parameters_table_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 *
 * If receives drop_precedence_ndx == DNX_TMC_NOF_DROP_PRECEDENCE will read from the ECN (IQM_CNREDm) configuration.
 */

uint32
  jer2_arad_iqm_packet_queue_red_parameters_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32             rt_cls_ndx,
    DNX_SAND_IN   uint32             drop_precedence_ndx,
    DNX_SAND_IN   JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_DATA* IQM_packet_queue_red_parameters_table_tbl_data
  )
{
  uint32 exp_man, err, entry_offset, data[JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_ENTRY_SIZE] = {0};
  soc_mem_t mem;
  int32 
      core_id = SOC_CORE_ALL;
  uint32 
      max_mantissa_size,
      max_bds_mantissa_size;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_SET_UNSAFE);
  if (drop_precedence_ndx == DNX_TMC_NOF_DROP_PRECEDENCE) {
    mem = IQM_CNREDm;
    entry_offset = rt_cls_ndx;
  } else { 
    mem = IQM_PQREDm;
    entry_offset = (rt_cls_ndx * JER2_ARAD_WRED_NOF_DROP_PRECEDENCE) + drop_precedence_ndx;
  }
  err = jer2_arad_iqm_packet_queue_red_parameters_tail_drop_mantissa_nof_bits(unit,
                                                                   drop_precedence_ndx,
                                                                   &max_mantissa_size,
                                                                   &max_bds_mantissa_size);
  DNX_SAND_CHECK_FUNC_RESULT(err, 20, exit);
  
  jer2_arad_iqm_mantissa_exponent_set(unit,
      IQM_packet_queue_red_parameters_table_tbl_data->pq_max_que_size_mnt,
      IQM_packet_queue_red_parameters_table_tbl_data->pq_max_que_size_exp,
      max_mantissa_size, &exp_man);
  soc_mem_field32_set(unit, mem, data, PQ_MAX_QUE_SIZEf, exp_man);

  jer2_arad_iqm_mantissa_exponent_set( unit,
      IQM_packet_queue_red_parameters_table_tbl_data->pq_max_que_size_bds_mnt,
      IQM_packet_queue_red_parameters_table_tbl_data->pq_max_que_size_bds_exp,
      max_bds_mantissa_size, &exp_man);
  soc_mem_field32_set(unit, mem, data, PQ_MAX_QUE_BUFF_SIZEf, exp_man);

  soc_mem_field32_set(unit, mem, data, PQ_WRED_ENf, IQM_packet_queue_red_parameters_table_tbl_data->pq_wred_en);
  soc_mem_field32_set(unit, mem, data, PQ_C_2f, IQM_packet_queue_red_parameters_table_tbl_data->pq_c2);
  soc_mem_field32_set(unit, mem, data, PQ_C_3f, IQM_packet_queue_red_parameters_table_tbl_data->pq_c3);
  soc_mem_field32_set(unit, mem, data, PQ_C_1f, IQM_packet_queue_red_parameters_table_tbl_data->pq_c1);
  soc_mem_field32_set(unit, mem, data, PQ_AVRG_MAX_THf, IQM_packet_queue_red_parameters_table_tbl_data->pq_avrg_max_th);
  soc_mem_field32_set(unit, mem, data, PQ_AVRG_MIN_THf, IQM_packet_queue_red_parameters_table_tbl_data->pq_avrg_min_th);
  soc_mem_field32_set(unit, mem, data, PQ_WRED_PCKT_SZ_IGNRf, IQM_packet_queue_red_parameters_table_tbl_data->pq_wred_pckt_sz_ignr);
  if (drop_precedence_ndx != DNX_TMC_NOF_DROP_PRECEDENCE) {
    soc_mem_field32_set(unit, IQM_PQREDm, data, ADDMIT_LOGICf, IQM_packet_queue_red_parameters_table_tbl_data->addmit_logic);
  }

  err = soc_mem_write(unit, mem, IQM_BLOCK(unit, core_id), entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_packet_queue_red_parameters_table_tbl_set_unsafe()",0,0);
}

/*
 * Fill indirect table packet_queue_red_parameters_table_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_packet_queue_red_parameters_table_tbl_fill_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_DATA* IQM_packet_queue_red_parameters_table_tbl_data
  )
{
  uint32 exp_man, err;
  uint32 data[JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(&(data[0]), 0x0, sizeof(data));
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 10, exit);

  jer2_arad_iqm_mantissa_exponent_set(unit, IQM_packet_queue_red_parameters_table_tbl_data->pq_max_que_size_mnt,
                                 IQM_packet_queue_red_parameters_table_tbl_data->pq_max_que_size_exp, 6, &exp_man);
  soc_mem_field32_set(unit, IQM_PQREDm, data, PQ_MAX_QUE_SIZEf, exp_man);
  soc_mem_field32_set(unit, IQM_PQREDm, data, PQ_WRED_ENf, IQM_packet_queue_red_parameters_table_tbl_data->pq_wred_en);
  soc_mem_field32_set(unit, IQM_PQREDm, data, PQ_C_2f, IQM_packet_queue_red_parameters_table_tbl_data->pq_c2);
  soc_mem_field32_set(unit, IQM_PQREDm, data, PQ_C_3f, IQM_packet_queue_red_parameters_table_tbl_data->pq_c3);
  soc_mem_field32_set(unit, IQM_PQREDm, data, PQ_C_1f, IQM_packet_queue_red_parameters_table_tbl_data->pq_c1);
  soc_mem_field32_set(unit, IQM_PQREDm, data, PQ_AVRG_MAX_THf, IQM_packet_queue_red_parameters_table_tbl_data->pq_avrg_max_th);
  soc_mem_field32_set(unit, IQM_PQREDm, data, PQ_AVRG_MIN_THf, IQM_packet_queue_red_parameters_table_tbl_data->pq_avrg_min_th);
  soc_mem_field32_set(unit, IQM_PQREDm, data, PQ_WRED_PCKT_SZ_IGNRf, IQM_packet_queue_red_parameters_table_tbl_data->pq_wred_pckt_sz_ignr);
  soc_mem_field32_set(unit, IQM_PQREDm, data, ADDMIT_LOGICf, IQM_packet_queue_red_parameters_table_tbl_data->addmit_logic);

  err = jer2_arad_fill_table_with_entry(unit, IQM_PQREDm, MEM_BLOCK_ANY, data); /* fill memory with the entry in data */
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_packet_queue_red_parameters_table_tbl_fill_unsafe()",0,0);
}

/*
 * Read indirect table vsq_descriptor_rate_class_group_a_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_vsq_descriptor_rate_class_group_a_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_A_TBL_DATA* IQM_vsq_descriptor_rate_class_group_a_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_A_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_A_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IQM_vsq_descriptor_rate_class_group_a_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_A_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

 
 




  err = soc_mem_read(
          unit,
          IQM_VSQDRC_Am,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

    err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IQM_vsq_descriptor_rate_class_group_a_tbl_data->vsq_rc_a   = soc_mem_field32_get(
                  unit,
                  IQM_VSQDRC_Am,
                  data,
                  VSQ_RC_Af            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_vsq_descriptor_rate_class_group_a_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table vsq_descriptor_rate_class_group_a_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_vsq_descriptor_rate_class_group_a_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_A_TBL_DATA* IQM_vsq_descriptor_rate_class_group_a_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_A_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_A_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

 
 




err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_VSQDRC_Am,
          data,
          VSQ_RC_Af,
          IQM_vsq_descriptor_rate_class_group_a_tbl_data->vsq_rc_a );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  err = soc_mem_write(
          unit,
          IQM_VSQDRC_Am,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_vsq_descriptor_rate_class_group_a_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table vsq_descriptor_rate_class_group_b_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_vsq_descriptor_rate_class_group_b_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_B_TBL_DATA* IQM_vsq_descriptor_rate_class_group_b_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_B_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_B_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IQM_vsq_descriptor_rate_class_group_b_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_B_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

 
 




  err = soc_mem_read(
          unit,
          IQM_VSQDRC_Bm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

    err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IQM_vsq_descriptor_rate_class_group_b_tbl_data->vsq_rc_b   = soc_mem_field32_get(
                  unit,
                  IQM_VSQDRC_Bm,
                  data,
                  VSQ_RC_Bf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_vsq_descriptor_rate_class_group_b_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table vsq_descriptor_rate_class_group_b_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_vsq_descriptor_rate_class_group_b_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_B_TBL_DATA* IQM_vsq_descriptor_rate_class_group_b_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_B_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_B_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

 
 




err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_VSQDRC_Bm,
          data,
          VSQ_RC_Bf,
          IQM_vsq_descriptor_rate_class_group_b_tbl_data->vsq_rc_b );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  err = soc_mem_write(
          unit,
          IQM_VSQDRC_Bm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_vsq_descriptor_rate_class_group_b_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table vsq_descriptor_rate_class_group_c_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_vsq_descriptor_rate_class_group_c_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_C_TBL_DATA* IQM_vsq_descriptor_rate_class_group_c_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_C_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_C_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IQM_vsq_descriptor_rate_class_group_c_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_C_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

 
 




  err = soc_mem_read(
          unit,
          IQM_VSQDRC_Cm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

    err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IQM_vsq_descriptor_rate_class_group_c_tbl_data->vsq_rc_c   = soc_mem_field32_get(
                  unit,
                  IQM_VSQDRC_Cm,
                  data,
                  VSQ_RC_Cf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_vsq_descriptor_rate_class_group_c_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table vsq_descriptor_rate_class_group_c_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_vsq_descriptor_rate_class_group_c_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_C_TBL_DATA* IQM_vsq_descriptor_rate_class_group_c_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_C_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_C_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

 
 




err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_VSQDRC_Cm,
          data,
          VSQ_RC_Cf,
          IQM_vsq_descriptor_rate_class_group_c_tbl_data->vsq_rc_c );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  err = soc_mem_write(
          unit,
          IQM_VSQDRC_Cm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_vsq_descriptor_rate_class_group_c_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table vsq_descriptor_rate_class_group_d_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_vsq_descriptor_rate_class_group_d_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_D_TBL_DATA* IQM_vsq_descriptor_rate_class_group_d_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_D_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_D_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IQM_vsq_descriptor_rate_class_group_d_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_D_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

 
 




  err = soc_mem_read(
          unit,
          IQM_VSQDRC_Dm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

    err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IQM_vsq_descriptor_rate_class_group_d_tbl_data->vsq_rc_d   = soc_mem_field32_get(
                  unit,
                  IQM_VSQDRC_Dm,
                  data,
                  VSQ_RC_Df            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_vsq_descriptor_rate_class_group_d_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table vsq_descriptor_rate_class_group_d_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_vsq_descriptor_rate_class_group_d_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_D_TBL_DATA* IQM_vsq_descriptor_rate_class_group_d_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_D_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_D_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

 
 




err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_VSQDRC_Dm,
          data,
          VSQ_RC_Df,
          IQM_vsq_descriptor_rate_class_group_d_tbl_data->vsq_rc_d );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  err = soc_mem_write(
          unit,
          IQM_VSQDRC_Dm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_vsq_descriptor_rate_class_group_d_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table vsq_flow_control_parameters_table_group_a_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */


void
  jer2_arad_iqm_mantissa_exponent_get(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              reg_val,
    DNX_SAND_IN   uint32              mantissa_size,
    DNX_SAND_OUT  uint32              *mantissa,
    DNX_SAND_OUT  uint32              *exponent
  )
{
    *mantissa = DNX_SAND_GET_BITS_RANGE(reg_val, mantissa_size - 1, 0);
    *exponent = reg_val >> mantissa_size;
}


void
  jer2_arad_iqm_mantissa_exponent_set(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              mantissa,
    DNX_SAND_IN   uint32              exponent,
    DNX_SAND_IN   uint32              mantissa_size,
    DNX_SAND_OUT  uint32              *reg_val
  )
{
    *reg_val = mantissa + (exponent << mantissa_size);
}

/* abstraction of the IQM_VQFCPR_M tables */
static const soc_mem_t mem_arr_IQM_VQFCPR_M[JER2_ARAD_NOF_VSQ_GROUPS] = {IQM_VQFCPR_MAm, IQM_VQFCPR_MBm, IQM_VQFCPR_MCm, IQM_VQFCPR_MDm, IQM_VQFCPR_MEm, IQM_VQFCPR_MFm};

uint32
  jer2_arad_iqm_vsq_flow_control_parameters_table_group_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              group_id,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_DATA* IQM_vsq_flow_control_parameters_table_group_tbl_data
  )
{
  uint32
      mnt_exp = 0,
    err;
  uint32
    data[JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_GET_UNSAFE);

  DNX_SAND_ERR_IF_ABOVE_MAX(
    group_id, JER2_ARAD_NOF_VSQ_GROUPS-1,
    JER2_ARAD_ITM_VSQ_GROUP_ID_OUT_OF_RANGE_ERR, 5, exit
  );

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IQM_vsq_flow_control_parameters_table_group_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          mem_arr_IQM_VQFCPR_M[group_id],
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

    err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IQM_vsq_flow_control_parameters_table_group_tbl_data->wred_en   = soc_mem_field32_get(
                  unit,
                  mem_arr_IQM_VQFCPR_M[group_id],
                  data,
                  WRED_ENf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

    if(SOC_E_NONE == err) {
          IQM_vsq_flow_control_parameters_table_group_tbl_data->avrg_size_en   = soc_mem_field32_get(
                  unit,
                  mem_arr_IQM_VQFCPR_M[group_id],
                  data,
                  AVRG_SIZE_ENf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 41, exit);

    if(SOC_E_NONE == err) {
          IQM_vsq_flow_control_parameters_table_group_tbl_data->red_weight_q   = soc_mem_field32_get(
                  unit,
                  mem_arr_IQM_VQFCPR_M[group_id],
                  data,
                  RED_WEIGHT_Qf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 42, exit);

    if(SOC_E_NONE == err) {
        mnt_exp = soc_mem_field32_get(
                  unit,
                  mem_arr_IQM_VQFCPR_M[group_id],
                  data,
                  SET_THRESHOLD_WORDSf            );

    jer2_arad_iqm_mantissa_exponent_get(
        unit,
        mnt_exp,
        5,
        &(IQM_vsq_flow_control_parameters_table_group_tbl_data->set_threshold_words_mnt),
        &(IQM_vsq_flow_control_parameters_table_group_tbl_data->set_threshold_words_exp));
    
            
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 43, exit);

    if(SOC_E_NONE == err) {
        mnt_exp = soc_mem_field32_get(
                  unit,
                  mem_arr_IQM_VQFCPR_M[group_id],
                  data,
                  CLEAR_THRESHOLD_WORDSf            );
        jer2_arad_iqm_mantissa_exponent_get(
            unit,
            mnt_exp,
            5,
            &(IQM_vsq_flow_control_parameters_table_group_tbl_data->clear_threshold_words_mnt),
            &(IQM_vsq_flow_control_parameters_table_group_tbl_data->clear_threshold_words_exp));
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 45, exit);

    if(SOC_E_NONE == err) {
          mnt_exp = soc_mem_field32_get(
                  unit,
                  mem_arr_IQM_VQFCPR_M[group_id],
                  data,
                  SET_THRESHOLD_BDf            );
          jer2_arad_iqm_mantissa_exponent_get(
              unit,
              mnt_exp,
              4,
              &(IQM_vsq_flow_control_parameters_table_group_tbl_data->set_threshold_bd_mnt),
              &(IQM_vsq_flow_control_parameters_table_group_tbl_data->set_threshold_bd_exp));
             
                 
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 47, exit);


    err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
            mnt_exp = soc_mem_field32_get(
                  unit,
                  mem_arr_IQM_VQFCPR_M[group_id],
                  data,
                  CLEAR_THRESHOLD_BDf            );
            jer2_arad_iqm_mantissa_exponent_get(
                unit,
                mnt_exp,
                4,
                &(IQM_vsq_flow_control_parameters_table_group_tbl_data->clear_threshold_bd_mnt),
                &(IQM_vsq_flow_control_parameters_table_group_tbl_data->clear_threshold_bd_exp));
             }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 49, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_vsq_flow_control_parameters_table_group_a_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table vsq_flow_control_parameters_table_group_a_tbl from block IQM,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_vsq_flow_control_parameters_table_group_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              group_id,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_DATA* IQM_vsq_flow_control_parameters_table_group_tbl_data
  )
{
  uint32
    mnt_exp,
    err;
  uint32
    data[JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_SET_UNSAFE);

  DNX_SAND_ERR_IF_ABOVE_MAX(
    group_id, JER2_ARAD_NOF_VSQ_GROUPS-1,
    JER2_ARAD_ITM_VSQ_GROUP_ID_OUT_OF_RANGE_ERR, 5, exit
  );

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

 
 




err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          mem_arr_IQM_VQFCPR_M[group_id],
          data,
          WRED_ENf,
          IQM_vsq_flow_control_parameters_table_group_tbl_data->wred_en );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          mem_arr_IQM_VQFCPR_M[group_id],
          data,
          AVRG_SIZE_ENf,
          IQM_vsq_flow_control_parameters_table_group_tbl_data->avrg_size_en );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          mem_arr_IQM_VQFCPR_M[group_id],
          data,
          RED_WEIGHT_Qf,
          IQM_vsq_flow_control_parameters_table_group_tbl_data->red_weight_q );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);

  jer2_arad_iqm_mantissa_exponent_set(
      unit,
      IQM_vsq_flow_control_parameters_table_group_tbl_data->set_threshold_words_mnt,
      IQM_vsq_flow_control_parameters_table_group_tbl_data->set_threshold_words_exp,
      5,
      &mnt_exp);


if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          mem_arr_IQM_VQFCPR_M[group_id],
          data,
          SET_THRESHOLD_WORDSf,
          mnt_exp );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 33, exit);

  jer2_arad_iqm_mantissa_exponent_set(
      unit,
      IQM_vsq_flow_control_parameters_table_group_tbl_data->clear_threshold_words_mnt,
      IQM_vsq_flow_control_parameters_table_group_tbl_data->clear_threshold_words_exp,
      5,
      &mnt_exp);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          mem_arr_IQM_VQFCPR_M[group_id],
          data,
          CLEAR_THRESHOLD_WORDSf,
          mnt_exp);
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 35, exit);


  jer2_arad_iqm_mantissa_exponent_set(
      unit,
      IQM_vsq_flow_control_parameters_table_group_tbl_data->set_threshold_bd_mnt,
      IQM_vsq_flow_control_parameters_table_group_tbl_data->set_threshold_bd_exp,
      4,
      &mnt_exp);
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          mem_arr_IQM_VQFCPR_M[group_id],
          data,
          SET_THRESHOLD_BDf,
          mnt_exp );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 37, exit);

  jer2_arad_iqm_mantissa_exponent_set(
      unit,
      IQM_vsq_flow_control_parameters_table_group_tbl_data->clear_threshold_bd_mnt,
      IQM_vsq_flow_control_parameters_table_group_tbl_data->clear_threshold_bd_exp,
      4,
      &mnt_exp);
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          mem_arr_IQM_VQFCPR_M[group_id],
          data,
          CLEAR_THRESHOLD_BDf,
          mnt_exp);
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 39, exit);

  err = soc_mem_write(
          unit,
          mem_arr_IQM_VQFCPR_M[group_id],
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 37, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_vsq_flow_control_parameters_table_group_a_tbl_set_unsafe()",0,0);
}

/* abstraaction of the IQM_VQPR_M? tables */
static const soc_mem_t mem_arr_IQM_VQPR_M[JER2_ARAD_NOF_VSQ_GROUPS] = {IQM_VQPR_MAm, IQM_VQPR_MBm,IQM_VQPR_MCm,IQM_VQPR_MDm,IQM_VQPR_MEm,IQM_VQPR_MFm};
/*
 * Read indirect table vsq_queue_parameters_table_group_a_tbl from block IQM,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_vsq_queue_parameters_table_group_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              group_id,
    DNX_SAND_IN   uint32              vsq_rt_cls_ndx,
    DNX_SAND_IN   uint32             drop_precedence_ndx,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA* IQM_vsq_queue_parameters_table_group_tbl_data
  )
{
  uint32
      exp_man,
    err;
  uint32
    entry_offset,
    data[JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_A_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_GET_UNSAFE);

  DNX_SAND_ERR_IF_ABOVE_MAX(
    group_id, JER2_ARAD_NOF_VSQ_GROUPS-1,
    JER2_ARAD_ITM_VSQ_GROUP_ID_OUT_OF_RANGE_ERR, 5, exit
  );

  /* since IQM_VQPR_M[A-F] have the same fields in hardware, but currently in the description
     IQM_VQPR_M[E-F] have different fields, we use mem_arr_IQM_VQPR_M[group_id] for memory
     access and IQM_VQPR_MAm for filed access */
  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IQM_vsq_queue_parameters_table_group_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  entry_offset =
    (vsq_rt_cls_ndx * JER2_ARAD_WRED_NOF_DROP_PRECEDENCE) + drop_precedence_ndx;



  err = soc_mem_read(
          unit,
          mem_arr_IQM_VQPR_M[group_id],
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

    err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IQM_vsq_queue_parameters_table_group_tbl_data->c2   = soc_mem_field32_get(
                  unit,
                  IQM_VQPR_MAm,
                  data,
                  C_2f            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

    if(SOC_E_NONE == err) {
          IQM_vsq_queue_parameters_table_group_tbl_data->c3   = soc_mem_field32_get(
                  unit,
                  IQM_VQPR_MAm,
                  data,
                  C_3f            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 41, exit);

    if(SOC_E_NONE == err) {
          IQM_vsq_queue_parameters_table_group_tbl_data->c1   = soc_mem_field32_get(
                  unit,
                  IQM_VQPR_MAm,
                  data,
                  C_1f            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 42, exit);

    if(SOC_E_NONE == err) {
          IQM_vsq_queue_parameters_table_group_tbl_data->max_avrg_th   = soc_mem_field32_get(
                  unit,
                  IQM_VQPR_MAm,
                  data,
                  MAX_AVRG_THf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 43, exit);

    if(SOC_E_NONE == err) {
          IQM_vsq_queue_parameters_table_group_tbl_data->min_avrg_th   = soc_mem_field32_get(
                  unit,
                  IQM_VQPR_MAm,
                  data,
                  MIN_AVRG_THf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 44, exit);
    if(SOC_E_NONE == err) {
          IQM_vsq_queue_parameters_table_group_tbl_data->vq_wred_pckt_sz_ignr   = soc_mem_field32_get(
                  unit,
                  IQM_VQPR_MAm,
                  data,
                  VQ_WRED_PCKT_SZ_IGNRf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 45, exit);

    if(SOC_E_NONE == err) {
          exp_man = soc_mem_field32_get(
                  unit,
                  IQM_VQPR_MAm,
                  data,
                  VQ_MAX_SIZE_BDSf            );

          jer2_arad_iqm_mantissa_exponent_get(
              unit,
              exp_man,
              7,
              &(IQM_vsq_queue_parameters_table_group_tbl_data->vq_max_szie_bds_mnt),
              &(IQM_vsq_queue_parameters_table_group_tbl_data->vq_max_szie_bds_exp)
              );

        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 46, exit);

    if(SOC_E_NONE == err) {
          exp_man   = soc_mem_field32_get(
                  unit,
                  IQM_VQPR_MAm,
                  data,
                  VQ_MAX_SIZE_WORDSf            );
          jer2_arad_iqm_mantissa_exponent_get(
              unit,
              exp_man,
              7,
              &(IQM_vsq_queue_parameters_table_group_tbl_data->vq_max_size_words_mnt),
              &(IQM_vsq_queue_parameters_table_group_tbl_data->vq_max_size_words_exp)
              );

        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 48, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_vsq_queue_parameters_table_group_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table vsq_queue_parameters_table_group_a_tbl from block IQM,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_vsq_queue_parameters_table_group_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              group_id,
    DNX_SAND_IN   uint32              vsq_rt_cls_ndx,
    DNX_SAND_IN   uint32             drop_precedence_ndx,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA* IQM_vsq_queue_parameters_table_group_tbl_data
  )
{
  uint32
    err;
  uint32
    entry_offset,
      exp_man,
    data[JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_A_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_SET_UNSAFE);

  DNX_SAND_ERR_IF_ABOVE_MAX(
    group_id, JER2_ARAD_NOF_VSQ_GROUPS-1,
    JER2_ARAD_ITM_VSQ_GROUP_ID_OUT_OF_RANGE_ERR, 5, exit
  );

  /* since IQM_VQPR_M[A-F] have the same fields in hardware, but currently in the description
     IQM_VQPR_M[E-F] have different fields, we use mem_arr_IQM_VQPR_M[group_id] for memory
     access and IQM_VQPR_MAm for filed access */
  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  entry_offset =
    (vsq_rt_cls_ndx * JER2_ARAD_WRED_NOF_DROP_PRECEDENCE) + drop_precedence_ndx;



  err=SOC_E_NONE;
  if(SOC_E_NONE == err) {
          soc_mem_field32_set(
          unit,
          IQM_VQPR_MAm,
          data,
          C_2f,
          IQM_vsq_queue_parameters_table_group_tbl_data->c2 );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  if(SOC_E_NONE == err) {
          soc_mem_field32_set(
          unit,
          IQM_VQPR_MAm,
          data,
          C_3f,
          IQM_vsq_queue_parameters_table_group_tbl_data->c3 );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

  if(SOC_E_NONE == err) {
          soc_mem_field32_set(
          unit,
          IQM_VQPR_MAm,
          data,
          C_1f,
          IQM_vsq_queue_parameters_table_group_tbl_data->c1 );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);

  if(SOC_E_NONE == err) {
          soc_mem_field32_set(
          unit,
          IQM_VQPR_MAm,
          data,
          MAX_AVRG_THf,
          IQM_vsq_queue_parameters_table_group_tbl_data->max_avrg_th );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 33, exit);

  if(SOC_E_NONE == err) {
          soc_mem_field32_set(
          unit,
          IQM_VQPR_MAm,
          data,
          MIN_AVRG_THf,
          IQM_vsq_queue_parameters_table_group_tbl_data->min_avrg_th );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 34, exit);
  if(SOC_E_NONE == err) {
          soc_mem_field32_set(
          unit,
          IQM_VQPR_MAm,
          data,
          VQ_WRED_PCKT_SZ_IGNRf,
          IQM_vsq_queue_parameters_table_group_tbl_data->vq_wred_pckt_sz_ignr );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 35, exit);

  jer2_arad_iqm_mantissa_exponent_set(
      unit,
      IQM_vsq_queue_parameters_table_group_tbl_data->vq_max_szie_bds_mnt,
      IQM_vsq_queue_parameters_table_group_tbl_data->vq_max_szie_bds_exp,
      7,
      &exp_man
     );

  if(SOC_E_NONE == err) {
          soc_mem_field32_set(
          unit,
          IQM_VQPR_MAm,
          data,
          VQ_MAX_SIZE_BDSf,
          exp_man);
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 36, exit);

  jer2_arad_iqm_mantissa_exponent_set(
      unit,
      IQM_vsq_queue_parameters_table_group_tbl_data->vq_max_size_words_mnt,
      IQM_vsq_queue_parameters_table_group_tbl_data->vq_max_size_words_exp,
      7,
      &exp_man
     );

  if(SOC_E_NONE == err) {
          soc_mem_field32_set(
          unit,
          IQM_VQPR_MAm,
          data,
          VQ_MAX_SIZE_WORDSf,
          exp_man);
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 38, exit);

  err = soc_mem_write(
          unit,
          mem_arr_IQM_VQPR_M[group_id],
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_vsq_queue_parameters_table_group_a_tbl_set_unsafe()",0,0);
}


/*
 * Init indirect table vsq_queue_parameters_table_group_a_tbl from block IQM,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_vsq_queue_parameters_table_group_tbl_fill_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA* IQM_vsq_queue_parameters_table_group_tbl_data
  )
{
  uint32 err;
  uint32 exp_man, data[JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_A_TBL_ENTRY_SIZE];
  unsigned group_id;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_SET_UNSAFE);
  /* since IQM_VQPR_M[A-F] have the same fields in hardware, but currently in the description
     IQM_VQPR_M[E-F] have different fields, we use mem_arr_IQM_VQPR_M[group_id] for memory
     access and IQM_VQPR_MAm for filed access */

  err = dnx_sand_os_memset( &(data[0]), 0x0, sizeof(data));
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  soc_mem_field32_set(unit, IQM_VQPR_MAm, data, C_2f, IQM_vsq_queue_parameters_table_group_tbl_data->c2);
  soc_mem_field32_set(unit, IQM_VQPR_MAm, data, C_3f, IQM_vsq_queue_parameters_table_group_tbl_data->c3);
  soc_mem_field32_set(unit, IQM_VQPR_MAm, data, C_1f, IQM_vsq_queue_parameters_table_group_tbl_data->c1 );
  soc_mem_field32_set(unit, IQM_VQPR_MAm, data, MAX_AVRG_THf, IQM_vsq_queue_parameters_table_group_tbl_data->max_avrg_th );
  soc_mem_field32_set(unit, IQM_VQPR_MAm, data, MIN_AVRG_THf, IQM_vsq_queue_parameters_table_group_tbl_data->min_avrg_th );
  soc_mem_field32_set(unit, IQM_VQPR_MAm, data, VQ_WRED_PCKT_SZ_IGNRf, IQM_vsq_queue_parameters_table_group_tbl_data->vq_wred_pckt_sz_ignr );

  jer2_arad_iqm_mantissa_exponent_set(unit, IQM_vsq_queue_parameters_table_group_tbl_data->vq_max_szie_bds_mnt,
                                 IQM_vsq_queue_parameters_table_group_tbl_data->vq_max_szie_bds_exp, 7, &exp_man);
  soc_mem_field32_set(unit, IQM_VQPR_MAm, data, VQ_MAX_SIZE_BDSf, exp_man);
  jer2_arad_iqm_mantissa_exponent_set(unit, IQM_vsq_queue_parameters_table_group_tbl_data->vq_max_size_words_mnt,
                                 IQM_vsq_queue_parameters_table_group_tbl_data->vq_max_size_words_exp, 7, &exp_man);
  soc_mem_field32_set(unit, IQM_VQPR_MAm, data, VQ_MAX_SIZE_WORDSf, exp_man);


  for (group_id = 0; group_id < JER2_ARAD_NOF_VSQ_GROUPS; ++group_id) { /* loop over the 6 memories */
      err = jer2_arad_fill_table_with_entry(unit, mem_arr_IQM_VQPR_M[group_id] , MEM_BLOCK_ANY, data); /* fill memory with the entry in data */
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_vsq_queue_parameters_table_group_a_tbl_fill_unsafe()",0,0);
}


uint32
  jer2_arad_iqm_vsq_flow_control_parameters_table_group_tbl_fill_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_DATA* iqm_vsq_flow_control_parameters_table_group_tbl_data
  )
{
  uint32 err;
  uint32 exp_man, data[JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_ENTRY_SIZE];
  unsigned group_id;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_SET_UNSAFE);
  /* since IQM_VQPR_M[A-F] have the same fields in hardware, but currently in the description
     IQM_VQPR_M[E-F] have different fields, we use mem_arr_IQM_VQPR_M[group_id] for memory
     access and IQM_VQPR_MAm for filed access */

  err = dnx_sand_os_memset( &(data[0]), 0x0, sizeof(data));
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);


  soc_mem_field32_set(unit, IQM_VQFCPR_MAm, data, WRED_ENf, iqm_vsq_flow_control_parameters_table_group_tbl_data->wred_en);
  soc_mem_field32_set(unit, IQM_VQFCPR_MAm, data, AVRG_SIZE_ENf, iqm_vsq_flow_control_parameters_table_group_tbl_data->avrg_size_en);
  soc_mem_field32_set(unit, IQM_VQFCPR_MAm, data, RED_WEIGHT_Qf, iqm_vsq_flow_control_parameters_table_group_tbl_data->red_weight_q);

  jer2_arad_iqm_mantissa_exponent_set(unit, iqm_vsq_flow_control_parameters_table_group_tbl_data->set_threshold_words_mnt,
                                 iqm_vsq_flow_control_parameters_table_group_tbl_data->set_threshold_words_exp, 5, &exp_man);
  soc_mem_field32_set(unit, IQM_VQFCPR_MAm, data, SET_THRESHOLD_WORDSf, exp_man);
  jer2_arad_iqm_mantissa_exponent_set(unit, iqm_vsq_flow_control_parameters_table_group_tbl_data->clear_threshold_words_mnt,
                                 iqm_vsq_flow_control_parameters_table_group_tbl_data->clear_threshold_words_exp, 5, &exp_man);
  soc_mem_field32_set(unit, IQM_VQFCPR_MAm, data, CLEAR_THRESHOLD_WORDSf, exp_man);
  jer2_arad_iqm_mantissa_exponent_set(unit, iqm_vsq_flow_control_parameters_table_group_tbl_data->set_threshold_bd_mnt,
                                 iqm_vsq_flow_control_parameters_table_group_tbl_data->set_threshold_bd_exp, 4, &exp_man);
  soc_mem_field32_set(unit, IQM_VQFCPR_MAm, data, SET_THRESHOLD_BDf, exp_man);
  jer2_arad_iqm_mantissa_exponent_set(unit, iqm_vsq_flow_control_parameters_table_group_tbl_data->clear_threshold_bd_mnt,
                                 iqm_vsq_flow_control_parameters_table_group_tbl_data->clear_threshold_bd_exp, 4, &exp_man);
  soc_mem_field32_set(unit, IQM_VQFCPR_MAm, data, CLEAR_THRESHOLD_BDf, exp_man);


  for (group_id = 0; group_id < JER2_ARAD_NOF_VSQ_GROUPS; ++group_id) { /* loop over the 6 memories */
      err = jer2_arad_fill_table_with_entry(unit, mem_arr_IQM_VQFCPR_M[group_id] , MEM_BLOCK_ANY, data); /* fill memory with the entry in data */
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_vsq_flow_control_parameters_table_group_tbl_fill_unsafe()",0,0);
}

/*
 * Read indirect table system_red_parameters_table_tbl from block IQM,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_system_red_parameters_table_tbl_get_unsafe(
    DNX_SAND_IN   int                                       unit,
    DNX_SAND_IN   uint32                                        rt_cls_ndx,
    DNX_SAND_IN   uint32                                       drop_precedence_ndx,
    DNX_SAND_OUT  JER2_ARAD_IQM_SYSTEM_RED_PARAMETERS_TABLE_TBL_DATA* IQM_system_red_parameters_table_tbl_data
  )
{
  uint32
    err;
  uint32
    entry_offset,
    data[JER2_ARAD_IQM_SYSTEM_RED_PARAMETERS_TABLE_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_SYSTEM_RED_PARAMETERS_TABLE_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IQM_system_red_parameters_table_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IQM_SYSTEM_RED_PARAMETERS_TABLE_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

 
 

  if (!SOC_IS_QAX(unit)) {
  entry_offset =
    (rt_cls_ndx * JER2_ARAD_WRED_NOF_DROP_PRECEDENCE) + drop_precedence_ndx;



  err = soc_mem_read(
          unit,
          IQM_SPRDPRMm,
          MEM_BLOCK_ANY,
          entry_offset,
    data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

    err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IQM_system_red_parameters_table_tbl_data->adm_th   = soc_mem_field32_get(
                  unit,
                  IQM_SPRDPRMm,
                  data,
                  ADM_THf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 41, exit);

    if(SOC_E_NONE == err) {
        IQM_system_red_parameters_table_tbl_data->sys_red_en   = soc_mem_field32_get(
                  unit,
                  IQM_SPRDPRMm,
                data,
                  SRED_ENf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

    if(SOC_E_NONE == err) {
          IQM_system_red_parameters_table_tbl_data->prob_th   = soc_mem_field32_get(
                  unit,
                  IQM_SPRDPRMm,
                  data,
                  PROB_THf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 42, exit);

    if(SOC_E_NONE == err) {
          IQM_system_red_parameters_table_tbl_data->drp_th   = soc_mem_field32_get(
                  unit,
                  IQM_SPRDPRMm,
                  data,
                  DRP_THf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 43, exit);

    if(SOC_E_NONE == err) {
          IQM_system_red_parameters_table_tbl_data->drp_prob_indx1   = soc_mem_field32_get(
                  unit,
                  IQM_SPRDPRMm,
                  data,
                  DRP_PROB_INDX_1f            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 44, exit);

    if(SOC_E_NONE == err) {
          IQM_system_red_parameters_table_tbl_data->drp_prob_indx2   = soc_mem_field32_get(
                  unit,
                  IQM_SPRDPRMm,
                  data,
                  DRP_PROB_INDX_2f            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 45, exit);
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_system_red_parameters_table_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table system_red_parameters_table_tbl from block IQM,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_system_red_parameters_table_tbl_set_unsafe(
    DNX_SAND_IN   int                                       unit,
    DNX_SAND_IN   uint32                                        rt_cls_ndx,
    DNX_SAND_IN   uint32                                       drop_precedence_ndx,
    DNX_SAND_IN   JER2_ARAD_IQM_SYSTEM_RED_PARAMETERS_TABLE_TBL_DATA* IQM_system_red_parameters_table_tbl_data
  )
{
  uint32
    err;
  uint32
    entry_offset,
    data[JER2_ARAD_IQM_SYSTEM_RED_PARAMETERS_TABLE_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_SYSTEM_RED_PARAMETERS_TABLE_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

 
 
  if (!SOC_IS_QAX(unit)) {

  entry_offset =
    (rt_cls_ndx * JER2_ARAD_WRED_NOF_DROP_PRECEDENCE) + drop_precedence_ndx;



err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_SPRDPRMm,
        data,
          SRED_ENf,
        IQM_system_red_parameters_table_tbl_data->sys_red_en );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_SPRDPRMm,
          data,
          ADM_THf,
          IQM_system_red_parameters_table_tbl_data->adm_th );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_SPRDPRMm,
          data,
          PROB_THf,
          IQM_system_red_parameters_table_tbl_data->prob_th );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_SPRDPRMm,
          data,
          DRP_THf,
          IQM_system_red_parameters_table_tbl_data->drp_th );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 33, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_SPRDPRMm,
          data,
          DRP_PROB_INDX_1f,
          IQM_system_red_parameters_table_tbl_data->drp_prob_indx1 );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 34, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_SPRDPRMm,
          data,
          DRP_PROB_INDX_2f,
          IQM_system_red_parameters_table_tbl_data->drp_prob_indx2 );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 35, exit);

  err = soc_mem_write(
          unit,
          IQM_SPRDPRMm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 35, exit);
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_system_red_parameters_table_tbl_set_unsafe()",0,0);
}
/*
 * Read indirect table system_red_drop_probability_values_tbl from block IQM,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_system_red_drop_probability_values_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_SYSTEM_RED_DROP_PROBABILITY_VALUES_TBL_DATA* IQM_system_red_drop_probability_values_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_SYSTEM_RED_DROP_PROBABILITY_VALUES_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_SYSTEM_RED_DROP_PROBABILITY_VALUES_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IQM_system_red_drop_probability_values_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IQM_SYSTEM_RED_DROP_PROBABILITY_VALUES_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

 
 


  if (!SOC_IS_QAX(unit)) {

  err = soc_mem_read(
          unit,
          IQM_SRDPRBm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IQM_system_red_drop_probability_values_tbl_data->drp_prob   = soc_mem_field32_get(
                  unit,
                  IQM_SRDPRBm,
                  data,
                  DRP_PROBf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_system_red_drop_probability_values_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table system_red_drop_probability_values_tbl from block IQM,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_system_red_drop_probability_values_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_SYSTEM_RED_DROP_PROBABILITY_VALUES_TBL_DATA* IQM_system_red_drop_probability_values_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_SYSTEM_RED_DROP_PROBABILITY_VALUES_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_SYSTEM_RED_DROP_PROBABILITY_VALUES_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

 
 

  if (!SOC_IS_QAX(unit)) {


err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IQM_SRDPRBm,
          data,
          DRP_PROBf,
          IQM_system_red_drop_probability_values_tbl_data->drp_prob );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  err = soc_mem_write(
          unit,
          IQM_SRDPRBm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_system_red_drop_probability_values_tbl_set_unsafe()",0,0);
}
/*
 * Read indirect table system_red_tbl from block IQM,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_system_red_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_SYSTEM_RED_TBL_DATA* IQM_system_red_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_SYSTEM_RED_TBL_ENTRY_SIZE];
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_SYSTEM_RED_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IQM_system_red_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IQM_SYSTEM_RED_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);
  if (!SOC_IS_QAX(unit)) {
   err = soc_mem_read(
          unit,
          IQM_SRCQRNGm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  IQM_system_red_tbl_data->qsz_rng_th[0] = soc_mem_field32_get(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_0f);
  IQM_system_red_tbl_data->qsz_rng_th[1] = soc_mem_field32_get(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_1f);
  IQM_system_red_tbl_data->qsz_rng_th[2] = soc_mem_field32_get(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_2f);
  IQM_system_red_tbl_data->qsz_rng_th[3] = soc_mem_field32_get(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_3f);
  IQM_system_red_tbl_data->qsz_rng_th[4] = soc_mem_field32_get(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_4f);
  IQM_system_red_tbl_data->qsz_rng_th[5] = soc_mem_field32_get(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_5f);
  IQM_system_red_tbl_data->qsz_rng_th[6] = soc_mem_field32_get(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_6f);
  IQM_system_red_tbl_data->qsz_rng_th[7] = soc_mem_field32_get(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_7f);
  IQM_system_red_tbl_data->qsz_rng_th[8] = soc_mem_field32_get(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_8f);
  IQM_system_red_tbl_data->qsz_rng_th[9] = soc_mem_field32_get(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_9f);
  IQM_system_red_tbl_data->qsz_rng_th[10] = soc_mem_field32_get(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_10f);
  IQM_system_red_tbl_data->qsz_rng_th[11] = soc_mem_field32_get(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_11f);
  IQM_system_red_tbl_data->qsz_rng_th[12] = soc_mem_field32_get(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_12f);
  IQM_system_red_tbl_data->qsz_rng_th[13] = soc_mem_field32_get(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_13f);
  IQM_system_red_tbl_data->qsz_rng_th[14] = soc_mem_field32_get(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_14f);
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_system_red_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table system_red_tbl from block IQM,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_iqm_system_red_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_SYSTEM_RED_TBL_DATA* IQM_system_red_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IQM_SYSTEM_RED_TBL_ENTRY_SIZE];
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IQM_SYSTEM_RED_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);
  if (!SOC_IS_QAX(unit)) {
  soc_mem_field32_set(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_0f, IQM_system_red_tbl_data->qsz_rng_th[0]);
  soc_mem_field32_set(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_1f, IQM_system_red_tbl_data->qsz_rng_th[1]);
  soc_mem_field32_set(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_2f, IQM_system_red_tbl_data->qsz_rng_th[2]);
  soc_mem_field32_set(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_3f, IQM_system_red_tbl_data->qsz_rng_th[3]);
  soc_mem_field32_set(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_4f, IQM_system_red_tbl_data->qsz_rng_th[4]);
  soc_mem_field32_set(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_5f, IQM_system_red_tbl_data->qsz_rng_th[5]);
  soc_mem_field32_set(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_6f, IQM_system_red_tbl_data->qsz_rng_th[6]);
  soc_mem_field32_set(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_7f, IQM_system_red_tbl_data->qsz_rng_th[7]);
  soc_mem_field32_set(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_8f, IQM_system_red_tbl_data->qsz_rng_th[8]);
  soc_mem_field32_set(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_9f, IQM_system_red_tbl_data->qsz_rng_th[9]);
  soc_mem_field32_set(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_10f, IQM_system_red_tbl_data->qsz_rng_th[10]);
  soc_mem_field32_set(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_11f, IQM_system_red_tbl_data->qsz_rng_th[11]);
  soc_mem_field32_set(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_12f, IQM_system_red_tbl_data->qsz_rng_th[12]);
  soc_mem_field32_set(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_13f, IQM_system_red_tbl_data->qsz_rng_th[13]);
  soc_mem_field32_set(unit, IQM_SRCQRNGm, data, QSZ_RNG_TH_14f, IQM_system_red_tbl_data->qsz_rng_th[14]);

  err = soc_mem_write(
          unit,
          IQM_SRCQRNGm,
          MEM_BLOCK_ANY,
          entry_offset,
            data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 45, exit);
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_iqm_system_red_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table system_physical_port_lookup_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_indirect_base_queue_to_system_physical_port_tbl_get_unsafe(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  int core,
    DNX_SAND_IN  uint32 entry_offset,
    DNX_SAND_OUT JER2_ARAD_BASE_Q_TO_SYS_PORT_TABLE_TBL_DATA* system_physical_port_tbl_data
  )
{
  uint32 err;
  int core_index = (core == SOC_CORE_ALL) ? 0 : core;
  uint32 data[JER2_ARAD_IPS_SYSTEM_PHYSICAL_PORT_LOOKUP_TABLE_TBL_ENTRY_SIZE];
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_SYSTEM_PHYSICAL_PORT_LOOKUP_TABLE_TBL_GET_UNSAFE);
  /* assert(JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit)); */

  err = dnx_sand_os_memset(&(data[0]), 0x0, sizeof(data));
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(system_physical_port_tbl_data, 0x0, sizeof(JER2_ARAD_BASE_Q_TO_SYS_PORT_TABLE_TBL_DATA));
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(unit, IPS_QPM_1_SYS_REDm, IPS_BLOCK(unit, core_index), entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  system_physical_port_tbl_data->sys_phy_port = soc_mem_field32_get(unit, IPS_QPM_1_SYS_REDm, data, SYS_PHY_PORTf);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_indirect_base_queue_to_system_physical_port_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table system_physical_port_lookup_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_indirect_base_queue_to_system_physical_port_tbl_set_unsafe(
    DNX_SAND_IN   int           unit,
    DNX_SAND_IN   int           core,
    DNX_SAND_IN   uint32        entry_offset,
    DNX_SAND_IN   JER2_ARAD_BASE_Q_TO_SYS_PORT_TABLE_TBL_DATA* system_physical_port_tbl_data
  )
{
  uint32 err;
  uint32 data[JER2_ARAD_IPS_SYSTEM_PHYSICAL_PORT_LOOKUP_TABLE_TBL_ENTRY_SIZE];
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_SYSTEM_PHYSICAL_PORT_LOOKUP_TABLE_TBL_SET_UNSAFE);
  /* assert(JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit)); */

  err = dnx_sand_os_memset(&(data[0]), 0x0, sizeof(data));
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  soc_mem_field32_set(unit, IPS_QPM_1_SYS_REDm, data, SYS_PHY_PORTf, system_physical_port_tbl_data->sys_phy_port);

  err = soc_mem_write(unit, IPS_QPM_1_SYS_REDm, IPS_BLOCK(unit, core), entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

  if (SOC_IS_QAX(unit)) {
      err = dnx_sand_os_memset(&(data[0]), 0x0, sizeof(data));
      DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

      soc_mem_field32_set(unit, CGM_QSPMm, data, SYSTEM_PORTf, system_physical_port_tbl_data->sys_phy_port);

      err = soc_mem_write(unit, CGM_QSPMm, CGM_BLOCK(unit, core), entry_offset, data);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit); 
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_indirect_base_queue_to_system_physical_port_tbl_set_unsafe()",0,0);
}


/*
 * Write indirect table system_physical_port_lookup_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_indirect_base_queue_to_system_physical_port_tbl_region_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   uint32              region_size,
    DNX_SAND_IN   JER2_ARAD_BASE_Q_TO_SYS_PORT_TABLE_TBL_DATA* system_physical_port_tbl_data
  )
{
  uint32 err;
  uint32 *data = NULL, *cur_data;
  uint32 flags=0;
  int index_max, index_min;
  int i; 
  int dmaable = soc_mem_dmaable(unit, IPS_QPM_1_SYS_REDm, SOC_MEM_BLOCK_ANY(unit, IPS_QPM_1_SYS_REDm)); /* check if we can use DMA */
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_SYSTEM_PHYSICAL_PORT_LOOKUP_TABLE_TBL_SET_UNSAFE);
  index_min=entry_offset;
  index_max=entry_offset+region_size-1;

  /* assert(JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit)); */
  if (dmaable) {
      data = (uint32 *)soc_cm_salloc(unit, region_size*JER2_ARAD_IPS_SYSTEM_PHYSICAL_PORT_LOOKUP_TABLE_TBL_ENTRY_SIZE*sizeof(uint32), "IPS_QPM_1_SYS_REDm"); /* allocate DMA memory buffer */
  } else {
      data = (uint32*)dnx_sand_os_malloc_any_size(region_size*JER2_ARAD_IPS_SYSTEM_PHYSICAL_PORT_LOOKUP_TABLE_TBL_ENTRY_SIZE*sizeof(uint32), "IPS_QPM_1}");
  }
  if (data == NULL) {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 10, exit);
  }
  err = dnx_sand_os_memset(data, 0x0, region_size*JER2_ARAD_IPS_SYSTEM_PHYSICAL_PORT_LOOKUP_TABLE_TBL_ENTRY_SIZE*sizeof(uint32));
  DNX_SAND_CHECK_FUNC_RESULT(err, 20, exit);

  for (i=0, cur_data=data; i<region_size; i++, cur_data++) {
      soc_mem_field32_set(unit, IPS_QPM_1_SYS_REDm, cur_data, SYS_PHY_PORTf, system_physical_port_tbl_data->sys_phy_port);
  }
  err = soc_mem_array_write_range(unit, flags, IPS_QPM_1_SYS_REDm, 0, IPS_BLOCK(unit, core), index_min, index_max, data);
  DNX_SAND_CHECK_FUNC_RESULT(err, 30, exit);

exit:
    if (data != NULL) {
        if (dmaable) {
            soc_cm_sfree(unit, data);
        } else  {
            dnx_sand_os_free(data);
        }
    }
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_indirect_base_queue_to_system_physical_port_tbl_region_set_unsafe()",0,0);
}

static const soc_field_t dnx_qpm2_field_per_base_queue[BASE_QUEUES_PER_QPM_2_ENTRY] = {DEST_PORT_0f, DEST_PORT_1f, DEST_PORT_2f, DEST_PORT_3f};

/*
 * Read direct tables QPM_1,QPM_2 to get a mapping from a base queue to fap module x port
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_direct_base_queue_to_system_physical_port_tbl_get_unsafe(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  int core,
    DNX_SAND_IN  uint32 base_queue,
    DNX_SAND_OUT JER2_ARAD_BASE_Q_TO_MOD_PORT_TABLE_TBL_DATA* mod_port_tbl_data
  )
{
  uint32 err;
  uint32 data[JER2_ARAD_IPS_SYSTEM_PHYSICAL_PORT_LOOKUP_TABLE_TBL_ENTRY_SIZE] = {0};
  int core_index = (core == SOC_CORE_ALL) ? 0 : core;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  /* assert(JER2_ARAD_IS_VOQ_MAPPING_DIRECT(unit)); */

  err = READ_IPS_QPM_1_NO_SYS_REDm(unit, IPS_BLOCK(unit, core_index), base_queue, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 10, exit);

  mod_port_tbl_data->fap_id = soc_mem_field32_get(unit, IPS_QPM_1_NO_SYS_REDm, data, DEST_DEVf);
  mod_port_tbl_data->fap_port_id = soc_mem_field32_get(unit, IPS_QPM_1_NO_SYS_REDm, data, DEST_PORT_MSBf) << BASE_QUEUES_PER_QPM_FAP_PORT_MSB_OFFSET;

  data[0] = 0;
  data[1] = 0;
  err = READ_IPS_QPM_2_NO_SYS_REDm(unit, IPS_BLOCK(unit, core_index), base_queue / BASE_QUEUES_PER_QPM_2_ENTRY, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 20, exit);
  mod_port_tbl_data->fap_port_id |= soc_mem_field32_get(unit, IPS_QPM_2_NO_SYS_REDm, &data, dnx_qpm2_field_per_base_queue[base_queue % BASE_QUEUES_PER_QPM_2_ENTRY]);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_direct_base_queue_to_system_physical_port_tbl_get_unsafe()",base_queue ,0);
}

/*
 * Write direct tables QPM_1,QPM_2 to set a mapping from a base queue to fap module x port
 * Write indirect table system_physical_port_lookup_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_direct_base_queue_to_system_physical_port_tbl_set_unsafe(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core,
    DNX_SAND_IN uint32 base_queue,
    DNX_SAND_IN JER2_ARAD_BASE_Q_TO_MOD_PORT_TABLE_TBL_DATA* mod_port_tbl_data
  )
{
  uint32 err;
  uint32 data[JER2_ARAD_IPS_SYSTEM_PHYSICAL_PORT_LOOKUP_TABLE_TBL_ENTRY_SIZE] = {0};
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  /* assert(JER2_ARAD_IS_VOQ_MAPPING_DIRECT(unit)); */

  soc_mem_field32_set(unit, IPS_QPM_1_NO_SYS_REDm, data, DEST_DEVf, mod_port_tbl_data->fap_id);
  soc_mem_field32_set(unit, IPS_QPM_1_NO_SYS_REDm, data, DEST_PORT_MSBf, mod_port_tbl_data->fap_port_id >> BASE_QUEUES_PER_QPM_FAP_PORT_MSB_OFFSET);
  err = WRITE_IPS_QPM_1_NO_SYS_REDm(unit, IPS_BLOCK(unit, core), base_queue, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 10, exit);

  data[0] = 0;
  data[1] = 0;
  err = READ_IPS_QPM_2_NO_SYS_REDm(unit, IPS_BLOCK(unit, core), base_queue / BASE_QUEUES_PER_QPM_2_ENTRY, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 20, exit);

  soc_mem_field32_set(unit, IPS_QPM_2_NO_SYS_REDm, data, dnx_qpm2_field_per_base_queue[base_queue % BASE_QUEUES_PER_QPM_2_ENTRY], mod_port_tbl_data->fap_port_id & BASE_QUEUES_PER_QPM_FAP_PORT_NO_MSB_MASK );

  err = WRITE_IPS_QPM_2_NO_SYS_REDm(unit, IPS_BLOCK(unit, core), base_queue / BASE_QUEUES_PER_QPM_2_ENTRY, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_direct_base_queue_to_system_physical_port_tbl_set_unsafe()",base_queue ,0);
}

/*
 * Write direct tables QPM_1,QPM_2 to set a mapping from a base queue to fap module x port
 * Write indirect table system_physical_port_lookup_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_direct_base_queue_to_system_physical_port_tbl_region_set_unsafe(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core,
    DNX_SAND_IN uint32 entry_offset,
    DNX_SAND_IN uint32 region_size,
    DNX_SAND_IN JER2_ARAD_BASE_Q_TO_MOD_PORT_TABLE_TBL_DATA* mod_port_tbl_data
  )
{
  uint32 err;
  uint32 *data = NULL, *cur_data;
  uint32 flags=0;
  int index_max, index_min;
  int i; 
  int dmaable = soc_mem_dmaable(unit, IPS_QPM_2_NO_SYS_REDm, SOC_MEM_BLOCK_ANY(unit, IPS_QPM_2_NO_SYS_REDm)); /* check if we can use DMA */
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  index_min=entry_offset;
  index_max=entry_offset+region_size-1;

  if (dmaable) {
      data = (uint32 *)soc_cm_salloc(unit, region_size*sizeof(uint32), "IPS_QPM_2_NO_SYS_REDm"); /* allocate DMA memory buffer */
  } else {
      data = (uint32*)dnx_sand_os_malloc_any_size(region_size*sizeof(uint32), "IPS_QPM_2_NO_SYS_REDm");
  }
  if (data == NULL) {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 10, exit);
  }
  err = dnx_sand_os_memset(data, 0x0, region_size*sizeof(uint32));
  DNX_SAND_CHECK_FUNC_RESULT(err, 20, exit);

  /* assert(JER2_ARAD_IS_VOQ_MAPPING_DIRECT(unit)); */

  for (i=0, cur_data=data; i<region_size; i++, cur_data++) {
    soc_mem_field32_set(unit, IPS_QPM_1_NO_SYS_REDm, cur_data, DEST_DEVf, mod_port_tbl_data->fap_id);
    soc_mem_field32_set(unit, IPS_QPM_1_NO_SYS_REDm, cur_data, DEST_PORT_MSBf, mod_port_tbl_data->fap_port_id >> BASE_QUEUES_PER_QPM_FAP_PORT_MSB_OFFSET);
  }
  err = soc_mem_array_write_range(unit, flags, IPS_QPM_1_NO_SYS_REDm, 0, IPS_BLOCK(unit, core), index_min , index_max, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);


  err = dnx_sand_os_memset(data, 0x0, region_size*sizeof(uint32));
  DNX_SAND_CHECK_FUNC_RESULT(err, 40, exit);


  err = soc_mem_array_read_range(unit, IPS_QPM_2_NO_SYS_REDm, 0, IPS_BLOCK(unit, core), index_min / BASE_QUEUES_PER_QPM_2_ENTRY, index_max / BASE_QUEUES_PER_QPM_2_ENTRY, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 50, exit);

  for (i=0, cur_data=data; i<region_size; i++, cur_data++) {
      soc_mem_field32_set(unit, IPS_QPM_2_NO_SYS_REDm, cur_data, dnx_qpm2_field_per_base_queue[(entry_offset+i)% BASE_QUEUES_PER_QPM_2_ENTRY], mod_port_tbl_data->fap_port_id & BASE_QUEUES_PER_QPM_FAP_PORT_NO_MSB_MASK );
  }
  err = soc_mem_array_write_range(unit, flags, IPS_QPM_2_NO_SYS_REDm, 0, IPS_BLOCK(unit, core), index_min / BASE_QUEUES_PER_QPM_2_ENTRY , index_max / BASE_QUEUES_PER_QPM_2_ENTRY, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 60, exit);

exit:
    if (data != NULL) {
        if (dmaable) {
            soc_cm_sfree(unit, data);
        } else  {
            dnx_sand_os_free(data);
        }
    }
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_direct_base_queue_to_system_physical_port_tbl_region_set_unsafe()",entry_offset ,0);
}


/*
 * Read indirect table destination_device_and_port_lookup_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_indirect_sysport_to_modport_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MOD_PORT_TBL_DATA* IPS_destination_device_and_port_lookup_table_tbl_data
  )
{
  uint32 err;
  uint32 data[SOC_MAX_MEM_WORDS];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_DESTINATION_DEVICE_AND_PORT_LOOKUP_TABLE_TBL_GET_UNSAFE);
  /* assert(JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit)); */

  err = dnx_sand_os_memset(&(data[0]), 0x0, sizeof(data));
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(IPS_destination_device_and_port_lookup_table_tbl_data, 0x0, sizeof(JER2_ARAD_MOD_PORT_TBL_DATA));
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(unit, IPS_QPM_2_SYS_REDm, MEM_BLOCK_ANY, entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  IPS_destination_device_and_port_lookup_table_tbl_data->dest_port = soc_mem_field32_get(unit, IPS_QPM_2_SYS_REDm, data, DEST_PORTf);
  IPS_destination_device_and_port_lookup_table_tbl_data->dest_dev = soc_mem_field32_get(unit, IPS_QPM_2_SYS_REDm, data, DEST_DEVf);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_indirect_sysport_to_modport_tbl_get_unsafe()", entry_offset, 0);
}

/*
 * Write indirect table destination_device_and_port_lookup_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_indirect_sysport_to_modport_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MOD_PORT_TBL_DATA* IPS_destination_device_and_port_lookup_table_tbl_data
  )
{
  uint32 err;
  uint32 data[JER2_ARAD_IPS_DESTINATION_DEVICE_AND_PORT_LOOKUP_TABLE_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_DESTINATION_DEVICE_AND_PORT_LOOKUP_TABLE_TBL_SET_UNSAFE);
  /* assert(JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit)); */

  err = dnx_sand_os_memset(&(data[0]), 0x0, sizeof(data));
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  soc_mem_field32_set(unit, IPS_QPM_2_SYS_REDm, data, DEST_PORTf, IPS_destination_device_and_port_lookup_table_tbl_data->dest_port);
  soc_mem_field32_set(unit, IPS_QPM_2_SYS_REDm, data, DEST_DEVf, IPS_destination_device_and_port_lookup_table_tbl_data->dest_dev);

  err = soc_mem_write(unit, IPS_QPM_2_SYS_REDm, IPS_BLOCK(unit, SOC_CORE_ALL), entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_indirect_sysport_to_modport_tbl_set_unsafe()",0,0);
}
/*
 * Read indirect table flow_id_lookup_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_flow_id_lookup_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_DATA* IPS_flow_id_lookup_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_ENTRY_SIZE];
  int core_index = (core == SOC_CORE_ALL) ? 0 : core;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(&(data[0]), 0x0, sizeof(data));
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);
  err = dnx_sand_os_memset(IPS_flow_id_lookup_table_tbl_data, 0x0, sizeof(JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_DATA));
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(unit, IPS_FLWIDm, IPS_BLOCK(unit, core_index), entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  IPS_flow_id_lookup_table_tbl_data->base_flow = soc_mem_field32_get(unit, IPS_FLWIDm, data, BASE_FLOWf);
  IPS_flow_id_lookup_table_tbl_data->sub_flow_mode = soc_mem_field32_get(unit, IPS_FLWIDm, data, SUB_FLOW_MODEf);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_flow_id_lookup_table_tbl_get_unsafe()", entry_offset, 0);
}

/*
 * Write indirect table flow_id_lookup_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_flow_id_lookup_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_DATA* IPS_flow_id_lookup_table_tbl_data
  )
{
  uint32 err;
  uint32 data[JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_ENTRY_SIZE];
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(&(data[0]), 0x0, sizeof(data));
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  soc_mem_field32_set(unit, IPS_FLWIDm, data, BASE_FLOWf, IPS_flow_id_lookup_table_tbl_data->base_flow);
  soc_mem_field32_set(unit, IPS_FLWIDm, data, SUB_FLOW_MODEf, IPS_flow_id_lookup_table_tbl_data->sub_flow_mode);

  err = soc_mem_write(unit, IPS_FLWIDm, IPS_BLOCK(unit, core), entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_flow_id_lookup_table_tbl_set_unsafe()",0,0);
}
/*
 * Read indirect table queue_type_lookup_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_queue_type_lookup_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPS_QUEUE_TYPE_LOOKUP_TABLE_TBL_DATA* IPS_queue_type_lookup_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IPS_QUEUE_TYPE_LOOKUP_TABLE_TBL_ENTRY_SIZE];
  int core_index = (core == SOC_CORE_ALL) ? 0 : core;
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_QUEUE_TYPE_LOOKUP_TABLE_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IPS_queue_type_lookup_table_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IPS_QUEUE_TYPE_LOOKUP_TABLE_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

 
 




  err = soc_mem_read(
          unit,
          IPS_QTYPEm,
          IPS_BLOCK(unit, core_index),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IPS_queue_type_lookup_table_tbl_data->queue_type_lookup_table   = soc_mem_field32_get(
                  unit,
                  IPS_QTYPEm,
                  data,
                  QTYPEf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_queue_type_lookup_table_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table flow_id_lookup_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_flow_id_lookup_table_tbl_region_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   uint32              region_size,
    DNX_SAND_IN   JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_DATA* IPS_flow_id_lookup_table_tbl_data
  )
{
  uint32 err;
  uint32 *data = NULL, *cur_data;
  uint32 flags=0;
  int index_max, index_min;
  int i; 
  int dmaable = soc_mem_dmaable(
                 unit, 
                 IPS_FLWIDm, 
                 SOC_MEM_BLOCK_ANY(unit, IPS_FLWIDm)); /* check if we can use DMA */
   
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_SET_UNSAFE);

  index_min=entry_offset;
  index_max=entry_offset+region_size-1;
  if (dmaable) {
      data = (uint32 *)soc_cm_salloc(unit, region_size*sizeof(uint32), "IPS_FLWIDm"); /* allocate DMA memory buffer */
  } else {
      data = (uint32*)dnx_sand_os_malloc_any_size(region_size*sizeof(uint32), "IPS_FLWIDm}");
  }
  if (data == NULL) {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, 10, exit);
  }

  err = dnx_sand_os_memset(data, 0x0, region_size*sizeof(uint32));
  DNX_SAND_CHECK_FUNC_RESULT(err, 20, exit);

  for (i=0, cur_data=data; i<region_size; i++, cur_data++) {
      *cur_data = 0;
      soc_mem_field32_set(unit, IPS_FLWIDm, cur_data, BASE_FLOWf, IPS_flow_id_lookup_table_tbl_data->base_flow);
      soc_mem_field32_set(unit, IPS_FLWIDm, cur_data, SUB_FLOW_MODEf, IPS_flow_id_lookup_table_tbl_data->sub_flow_mode);
  }

  err = soc_mem_array_write_range(unit, flags, IPS_FLWIDm, 0, IPS_BLOCK(unit, core), index_min , index_max, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);


exit:
    if (data != NULL) {
        if (dmaable) {
            soc_cm_sfree(unit, data);
        } else  {
            dnx_sand_os_free(data);
        }
    }
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_flow_id_lookup_table_tbl_region_set_unsafe()",0,0);
}

/*
 * Write indirect table queue_type_lookup_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_queue_type_lookup_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPS_QUEUE_TYPE_LOOKUP_TABLE_TBL_DATA* IPS_queue_type_lookup_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IPS_QUEUE_TYPE_LOOKUP_TABLE_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_QUEUE_TYPE_LOOKUP_TABLE_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

 
 




err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_QTYPEm,
          data,
          QTYPEf,
          IPS_queue_type_lookup_table_tbl_data->queue_type_lookup_table );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  err = soc_mem_write(
          unit,
          IPS_QTYPEm,
          IPS_BLOCK(unit, core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_queue_type_lookup_table_tbl_set_unsafe()",0,0);
}
/*
 * Read indirect table queue_priority_map_select_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_queue_priority_map_select_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPS_QUEUE_PRIORITY_MAP_SELECT_TBL_DATA* IPS_queue_priority_map_select_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IPS_QUEUE_PRIORITY_MAP_SELECT_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_QUEUE_PRIORITY_MAP_SELECT_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IPS_queue_priority_map_select_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IPS_QUEUE_PRIORITY_MAP_SELECT_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

 
 




  err = soc_mem_read(
          unit,
          IPS_QPRISELm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IPS_queue_priority_map_select_tbl_data->queue_priority_map_select   = soc_mem_field32_get(
                  unit,
                  IPS_QPRISELm,
                  data,
                  QPRISELf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_queue_priority_map_select_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table queue_priority_map_select_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_queue_priority_map_select_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPS_QUEUE_PRIORITY_MAP_SELECT_TBL_DATA* IPS_queue_priority_map_select_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IPS_QUEUE_PRIORITY_MAP_SELECT_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_QUEUE_PRIORITY_MAP_SELECT_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

 
 




err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_QPRISELm,
          data,
          QPRISELf,
          IPS_queue_priority_map_select_tbl_data->queue_priority_map_select );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  err = soc_mem_write(
          unit,
          IPS_QPRISELm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_queue_priority_map_select_tbl_set_unsafe()",0,0);
}
/*
 * Read indirect table queue_priority_maps_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_queue_priority_maps_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPS_QUEUE_PRIORITY_MAPS_TABLE_TBL_DATA* IPS_queue_priority_maps_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IPS_QUEUE_PRIORITY_MAPS_TABLE_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_QUEUE_PRIORITY_MAPS_TABLE_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IPS_queue_priority_maps_table_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IPS_QUEUE_PRIORITY_MAPS_TABLE_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

 
 




  err = soc_mem_read(
          unit,
          IPS_Q_PRIORITY_BIT_MAPm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
             soc_mem_field_get(
                  unit,
                  IPS_Q_PRIORITY_BIT_MAPm,
                  data,
                  Q_PRIORITY_BIT_MAPf,
                IPS_queue_priority_maps_table_tbl_data->queue_priority_maps_table            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_queue_priority_maps_table_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table queue_priority_maps_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_queue_priority_maps_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPS_QUEUE_PRIORITY_MAPS_TABLE_TBL_DATA* IPS_queue_priority_maps_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IPS_QUEUE_PRIORITY_MAPS_TABLE_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_QUEUE_PRIORITY_MAPS_TABLE_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

 
 




err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field_set(
          unit,
          IPS_Q_PRIORITY_BIT_MAPm,
          data,
          Q_PRIORITY_BIT_MAPf,
          (uint32*)&(IPS_queue_priority_maps_table_tbl_data->queue_priority_maps_table[0]) );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  err = soc_mem_write(
          unit,
          IPS_Q_PRIORITY_BIT_MAPm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_queue_priority_maps_table_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table queue_size_based_thresholds_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ips_queue_size_based_thresholds_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPS_QUEUE_SIZE_BASED_THRESHOLDS_TABLE_TBL_DATA* IPS_queue_size_based_thresholds_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IPS_QUEUE_SIZE_BASED_THRESHOLDS_TABLE_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_QUEUE_SIZE_BASED_THRESHOLDS_TABLE_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IPS_queue_size_based_thresholds_table_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IPS_QUEUE_SIZE_BASED_THRESHOLDS_TABLE_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          IPS_QSZTHm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IPS_queue_size_based_thresholds_table_tbl_data->off_to_slow_msg_th   = soc_mem_field32_get(
                  unit,
                  IPS_QSZTHm,
                  data,
                  OFF_TO_SLOW_MSG_THf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

    if(SOC_E_NONE == err) {
          IPS_queue_size_based_thresholds_table_tbl_data->off_to_norm_msg_th   = soc_mem_field32_get(
                  unit,
                  IPS_QSZTHm,
                  data,
                  OFF_TO_NORM_MSG_THf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 41, exit);

    if(SOC_E_NONE == err) {
          IPS_queue_size_based_thresholds_table_tbl_data->slow_to_norm_msg_th   = soc_mem_field32_get(
                  unit,
                  IPS_QSZTHm,
                  data,
                  SLOW_TO_NORM_MSG_THf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 42, exit);

    if(SOC_E_NONE == err) {
          IPS_queue_size_based_thresholds_table_tbl_data->norm_to_slow_msg_th   = soc_mem_field32_get(
                  unit,
                  IPS_QSZTHm,
                  data,
                  NORM_TO_SLOW_MSG_THf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 43, exit);

    if(SOC_E_NONE == err) {
          IPS_queue_size_based_thresholds_table_tbl_data->fsm_th_mul   = soc_mem_field32_get(
                  unit,
                  IPS_QSZTHm,
                  data,
                  FSM_TH_MULf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 44, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_queue_size_based_thresholds_table_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table queue_size_based_thresholds_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_queue_size_based_thresholds_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPS_QUEUE_SIZE_BASED_THRESHOLDS_TABLE_TBL_DATA* IPS_queue_size_based_thresholds_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IPS_QUEUE_SIZE_BASED_THRESHOLDS_TABLE_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_QUEUE_SIZE_BASED_THRESHOLDS_TABLE_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);


err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_QSZTHm,
          data,
          OFF_TO_SLOW_MSG_THf,
          IPS_queue_size_based_thresholds_table_tbl_data->off_to_slow_msg_th );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_QSZTHm,
          data,
          OFF_TO_NORM_MSG_THf,
          IPS_queue_size_based_thresholds_table_tbl_data->off_to_norm_msg_th );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_QSZTHm,
          data,
          SLOW_TO_NORM_MSG_THf,
          IPS_queue_size_based_thresholds_table_tbl_data->slow_to_norm_msg_th );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_QSZTHm,
          data,
          NORM_TO_SLOW_MSG_THf,
          IPS_queue_size_based_thresholds_table_tbl_data->norm_to_slow_msg_th );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 33, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_QSZTHm,
          data,
          FSM_TH_MULf,
          IPS_queue_size_based_thresholds_table_tbl_data->fsm_th_mul );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 34, exit);

  err = soc_mem_write(
          unit,
          IPS_QSZTHm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 35, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_queue_size_based_thresholds_table_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table credit_balance_based_thresholds_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_credit_balance_based_thresholds_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPS_CREDIT_BALANCE_BASED_THRESHOLDS_TABLE_TBL_DATA* IPS_credit_balance_based_thresholds_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IPS_CREDIT_BALANCE_BASED_THRESHOLDS_TABLE_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_CREDIT_BALANCE_BASED_THRESHOLDS_TABLE_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IPS_credit_balance_based_thresholds_table_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IPS_CREDIT_BALANCE_BASED_THRESHOLDS_TABLE_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          IPS_CRBALTHm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IPS_credit_balance_based_thresholds_table_tbl_data->backoff_enter_qcr_bal_th   = soc_mem_field32_get(
                  unit,
                  IPS_CRBALTHm,
                  data,
                  BACKOFF_ENTER_Q_CR_BAL_THf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

    if(SOC_E_NONE == err) {
          IPS_credit_balance_based_thresholds_table_tbl_data->backoff_exit_qcr_bal_th   = soc_mem_field32_get(
                  unit,
                  IPS_CRBALTHm,
                  data,
                  BACKOFF_EXIT_Q_CR_BAL_THf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 41, exit);

    if(SOC_E_NONE == err) {
          IPS_credit_balance_based_thresholds_table_tbl_data->backlog_enter_qcr_bal_th   = soc_mem_field32_get(
                  unit,
                  IPS_CRBALTHm,
                  data,
                  BACKLOG_ENTER_Q_CR_BAL_THf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 42, exit);

    if(SOC_E_NONE == err) {
          IPS_credit_balance_based_thresholds_table_tbl_data->backlog_exit_qcr_bal_th   = soc_mem_field32_get(
                  unit,
                  IPS_CRBALTHm,
                  data,
                  BACKLOG_EXIT_Q_CR_BAL_THf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 43, exit);

  if (!SOC_IS_ARADPLUS_AND_BELOW(unit)) {
      IPS_credit_balance_based_thresholds_table_tbl_data->backslow_enter_qcr_bal_th = soc_mem_field32_get(
                    unit,
                    IPS_CRBALTHm,
                    data,
                    BACKSLOW_ENTER_Q_CR_BAL_THf);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 43, exit);

      IPS_credit_balance_based_thresholds_table_tbl_data->backslow_exit_qcr_bal_th = soc_mem_field32_get(
                    unit,
                    IPS_CRBALTHm,
                    data,
                    BACKSLOW_EXIT_Q_CR_BAL_THf);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 44, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_credit_balance_based_thresholds_table_tbl_get_unsafe()",0,0);
}
/*
 * Write indirect table credit_balance_based_thresholds_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_credit_balance_based_thresholds_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPS_CREDIT_BALANCE_BASED_THRESHOLDS_TABLE_TBL_DATA* IPS_credit_balance_based_thresholds_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IPS_CREDIT_BALANCE_BASED_THRESHOLDS_TABLE_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_CREDIT_BALANCE_BASED_THRESHOLDS_TABLE_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);


err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_CRBALTHm,
          data,
          BACKOFF_ENTER_Q_CR_BAL_THf,
          IPS_credit_balance_based_thresholds_table_tbl_data->backoff_enter_qcr_bal_th );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_CRBALTHm,
          data,
          BACKOFF_EXIT_Q_CR_BAL_THf,
          IPS_credit_balance_based_thresholds_table_tbl_data->backoff_exit_qcr_bal_th );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_CRBALTHm,
          data,
          BACKLOG_ENTER_Q_CR_BAL_THf,
          IPS_credit_balance_based_thresholds_table_tbl_data->backlog_enter_qcr_bal_th );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_CRBALTHm,
          data,
          BACKLOG_EXIT_Q_CR_BAL_THf,
          IPS_credit_balance_based_thresholds_table_tbl_data->backlog_exit_qcr_bal_th );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 33, exit);

  if (!SOC_IS_ARADPLUS_AND_BELOW(unit)) {
      soc_mem_field32_set(
         unit,
         IPS_CRBALTHm,
         data,
         BACKSLOW_ENTER_Q_CR_BAL_THf, 
         IPS_credit_balance_based_thresholds_table_tbl_data->backslow_enter_qcr_bal_th);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 43, exit);

      soc_mem_field32_set(
         unit,
         IPS_CRBALTHm,
         data,
         BACKSLOW_EXIT_Q_CR_BAL_THf,
         IPS_credit_balance_based_thresholds_table_tbl_data->backslow_exit_qcr_bal_th);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 44, exit);
  }

  err = soc_mem_write(
          unit,
          IPS_CRBALTHm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 34, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_credit_balance_based_thresholds_table_tbl_set_unsafe()",0,0);
}
/*
 * Read indirect table empty_queue_credit_balance_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_empty_queue_credit_balance_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPS_EMPTY_QUEUE_CREDIT_BALANCE_TABLE_TBL_DATA* IPS_empty_queue_credit_balance_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IPS_EMPTY_QUEUE_CREDIT_BALANCE_TABLE_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_EMPTY_QUEUE_CREDIT_BALANCE_TABLE_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IPS_empty_queue_credit_balance_table_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IPS_EMPTY_QUEUE_CREDIT_BALANCE_TABLE_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          IPS_EMPTYQCRBALm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IPS_empty_queue_credit_balance_table_tbl_data->empty_qsatisfied_cr_bal   = soc_mem_field32_get(
                  unit,
                  IPS_EMPTYQCRBALm,
                  data,
                  EMPTY_Q_SATISFIED_CR_BALf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

    if(SOC_E_NONE == err) {
          IPS_empty_queue_credit_balance_table_tbl_data->max_empty_qcr_bal   = soc_mem_field32_get(
                  unit,
                  IPS_EMPTYQCRBALm,
                  data,
                  MAX_EMPTY_Q_CR_BALf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 41, exit);

    if(SOC_E_NONE == err) {
          IPS_empty_queue_credit_balance_table_tbl_data->exceed_max_empty_qcr_bal   = soc_mem_field32_get(
                  unit,
                  IPS_EMPTYQCRBALm,
                  data,
                  EXCEED_MAX_EMPTY_Q_CR_BALf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 42, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_empty_queue_credit_balance_table_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table empty_queue_credit_balance_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_empty_queue_credit_balance_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPS_EMPTY_QUEUE_CREDIT_BALANCE_TABLE_TBL_DATA* IPS_empty_queue_credit_balance_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IPS_EMPTY_QUEUE_CREDIT_BALANCE_TABLE_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_EMPTY_QUEUE_CREDIT_BALANCE_TABLE_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);


err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_EMPTYQCRBALm,
          data,
          EMPTY_Q_SATISFIED_CR_BALf,
          IPS_empty_queue_credit_balance_table_tbl_data->empty_qsatisfied_cr_bal );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_EMPTYQCRBALm,
          data,
          MAX_EMPTY_Q_CR_BALf,
          IPS_empty_queue_credit_balance_table_tbl_data->max_empty_qcr_bal );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_EMPTYQCRBALm,
          data,
          EXCEED_MAX_EMPTY_Q_CR_BALf,
          IPS_empty_queue_credit_balance_table_tbl_data->exceed_max_empty_qcr_bal );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);

  err = soc_mem_write(
          unit,
          IPS_EMPTYQCRBALm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 33, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_empty_queue_credit_balance_table_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table credit_watchdog_thresholds_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_credit_watchdog_thresholds_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPS_CREDIT_WATCHDOG_THRESHOLDS_TABLE_TBL_DATA* IPS_credit_watchdog_thresholds_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IPS_CREDIT_WATCHDOG_THRESHOLDS_TABLE_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_CREDIT_WATCHDOG_THRESHOLDS_TABLE_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          IPS_credit_watchdog_thresholds_table_tbl_data,
          0x0,
          sizeof(JER2_ARAD_IPS_CREDIT_WATCHDOG_THRESHOLDS_TABLE_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          IPS_CRWDTHm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          IPS_credit_watchdog_thresholds_table_tbl_data->wd_status_msg_gen_period   = soc_mem_field32_get(
                  unit,
                  IPS_CRWDTHm,
                  data,
                  WD_STATUS_MSG_GEN_PERIODf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

    if(SOC_E_NONE == err) {
          IPS_credit_watchdog_thresholds_table_tbl_data->wd_delete_qth   = soc_mem_field32_get(
                  unit,
                  IPS_CRWDTHm,
                  data,
                  WD_DELETE_Q_THf            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 41, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_credit_watchdog_thresholds_table_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table credit_watchdog_thresholds_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_ips_credit_watchdog_thresholds_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPS_CREDIT_WATCHDOG_THRESHOLDS_TABLE_TBL_DATA* IPS_credit_watchdog_thresholds_table_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_IPS_CREDIT_WATCHDOG_THRESHOLDS_TABLE_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPS_CREDIT_WATCHDOG_THRESHOLDS_TABLE_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

err=SOC_E_NONE;
if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_CRWDTHm,
          data,
          WD_STATUS_MSG_GEN_PERIODf,
          IPS_credit_watchdog_thresholds_table_tbl_data->wd_status_msg_gen_period );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          IPS_CRWDTHm,
          data,
          WD_DELETE_Q_THf,
          IPS_credit_watchdog_thresholds_table_tbl_data->wd_delete_qth );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

  err = soc_mem_write(
          unit,
          IPS_CRWDTHm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_credit_watchdog_thresholds_table_tbl_set_unsafe()",0,0);
}

soc_mem_t
  jer2_arad_egq_scm_chan_arb_id2scm_id(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint32 chan_arb_id
  )
{  
  soc_mem_t
    egq_scm_name;
  /*
   *  Go to the correct table
   */
  switch (chan_arb_id)
  {
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_00:
    egq_scm_name = EGQ_CH_0_SCMm;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_01:
    egq_scm_name = EGQ_CH_1_SCMm;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_02:
    egq_scm_name = EGQ_CH_2_SCMm;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_03:
    egq_scm_name = EGQ_CH_3_SCMm;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_04:
    egq_scm_name = EGQ_CH_4_SCMm;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_05:
    egq_scm_name = EGQ_CH_5_SCMm;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_06:
    egq_scm_name = EGQ_CH_6_SCMm;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_07:
    egq_scm_name = EGQ_CH_7_SCMm;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_CPU:
    egq_scm_name = EGQ_CH_8_SCMm;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_RCY:
    egq_scm_name = EGQ_CH_9_SCMm;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CHAN_ARB_NON_CHAN:
    egq_scm_name = EGQ_NONCH_SCMm;
    break;
  default:
    egq_scm_name = EGQ_CH_0_SCMm;
  }

  return egq_scm_name;
}

/*
 * Read indirect table scm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_scm_tbl_get_unsafe(
    DNX_SAND_IN   int                       unit,
    DNX_SAND_IN   int                       core,
    DNX_SAND_IN   JER2_ARAD_OFP_RATES_CAL_INFO   *cal_info,
    DNX_SAND_IN   uint32                    entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_SCM_TBL_DATA*    EGQ_nif_scm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_EGQ_NIF_SCM_TBL_ENTRY_SIZE];
  soc_mem_t
    egq_scm_name;
  soc_field_t
    egq_cr_field_name,
    egq_index_field_name;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGQ_SCM_TBL_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(cal_info);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          EGQ_nif_scm_tbl_data,
          0x0,
          sizeof(JER2_ARAD_EGQ_SCM_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  switch (cal_info->cal_type)
  {
  case JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB:
    if (cal_info->chan_arb_id > (SOC_DNX_DEFS_GET(unit, nof_channelized_calendars) - 1))
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_OFP_RATES_CHAN_ARB_INVALID_ERR, 20, exit);
    }
    egq_scm_name = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_ofp_rates_egq_scm_chan_arb_id2scm_id, (unit, cal_info->chan_arb_id));
    egq_cr_field_name = PORT_CR_TO_ADDf;
    egq_index_field_name = OFP_INDEXf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CAL_PORT_PRIORITY:
    egq_scm_name = EGQ_QP_SCMm;
    egq_cr_field_name = QPAIR_CR_TO_ADDf;
    egq_index_field_name = QPAIR_INDEXf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CAL_TCG:
    egq_scm_name = EGQ_TCG_SCMm;
    egq_cr_field_name = TCG_CR_TO_ADDf;
    egq_index_field_name = TCG_INDEXf;
    break;
  default:
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_OFP_RATES_EGQ_CAL_INVALID_ERR, 25, exit);
  }
  
  JER2_ARAD_DEVICE_CHECK(unit, exit);
  err = soc_mem_read(
          unit,
          egq_scm_name,
          EGQ_BLOCK(unit, core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          EGQ_nif_scm_tbl_data->port_cr_to_add = soc_mem_field32_get(
                  unit,
                  egq_scm_name,
                  data,
                  egq_cr_field_name    );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

    if(SOC_E_NONE == err) {
          EGQ_nif_scm_tbl_data->ofp_index = soc_mem_field32_get(
                  unit,
                  egq_scm_name,
                  data,
                egq_index_field_name );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 41, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egq_scm_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table scm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_egq_scm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   JER2_ARAD_OFP_RATES_CAL_INFO *cal_info,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_SCM_TBL_DATA* EGQ_nif_scm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_EGQ_NIF_SCM_TBL_ENTRY_SIZE];
  soc_mem_t
    egq_scm_name;
  soc_field_t
    egq_cr_field_name,
    egq_index_field_name;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGQ_SCM_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  switch (cal_info->cal_type)
  {
  case JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB:
    if (cal_info->chan_arb_id > (SOC_DNX_DEFS_GET(unit, nof_channelized_calendars) - 1))
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_OFP_RATES_CHAN_ARB_INVALID_ERR, 20, exit);
    }
     /*
      *  Go to the correct table, per chan arb id
      */
    egq_scm_name = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_ofp_rates_egq_scm_chan_arb_id2scm_id, (unit, cal_info->chan_arb_id));
    egq_cr_field_name = PORT_CR_TO_ADDf;
    egq_index_field_name = OFP_INDEXf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CAL_PORT_PRIORITY:
    egq_scm_name = EGQ_QP_SCMm;
    egq_cr_field_name = QPAIR_CR_TO_ADDf;
    egq_index_field_name = QPAIR_INDEXf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CAL_TCG:
    egq_scm_name = EGQ_TCG_SCMm;
    egq_cr_field_name = TCG_CR_TO_ADDf;
    egq_index_field_name = TCG_INDEXf;
    break;
  default:
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_OFP_RATES_EGQ_CAL_INVALID_ERR, 25, exit);
  }  

  err=SOC_E_NONE;
  JER2_ARAD_DEVICE_CHECK(unit, exit);
  if(SOC_E_NONE == err) {
    soc_mem_field32_set(
          unit,
          egq_scm_name,
          data,
          egq_cr_field_name,
          EGQ_nif_scm_tbl_data->port_cr_to_add);
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  if(SOC_E_NONE == err) {
    soc_mem_field32_set(
          unit,
          egq_scm_name,
          data,
          egq_index_field_name,
          EGQ_nif_scm_tbl_data->ofp_index );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

  err = soc_mem_write(
          unit,
          egq_scm_name,
          EGQ_BLOCK(unit, core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egq_scm_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table ccm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_egq_ccm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_CCM_TBL_DATA* EGQ_ccm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_EGQ_CCM_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGQ_CCM_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          EGQ_ccm_tbl_data,
          0x0,
          sizeof(JER2_ARAD_EGQ_CCM_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          EGQ_CCMm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          EGQ_ccm_tbl_data->interface_select   = soc_mem_field32_get(
              unit,
                  EGQ_CCMm,
                  data,
                  INTERFACE_SELECTf
          );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egq_ccm_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table ccm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_egq_ccm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32          entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_CCM_TBL_DATA* EGQ_ccm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_EGQ_CCM_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGQ_CCM_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err=SOC_E_NONE;
  if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          EGQ_CCMm,
          data,
          INTERFACE_SELECTf,
          EGQ_ccm_tbl_data->interface_select
      );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  err = soc_mem_write(
          unit,
          EGQ_CCMm,
          EGQ_BLOCK(unit, core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egq_ccm_tbl_set_unsafe()",0,0);
}

static
  uint32
    jer2_arad_egq_pmc_names_get(
      DNX_SAND_IN   int             unit,
      DNX_SAND_IN   JER2_ARAD_OFP_RATES_CAL_INFO *cal_info,
      DNX_SAND_OUT  soc_mem_t             *egq_pmc_name,
      DNX_SAND_OUT  soc_field_t           *egq_cr_field_name
    )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  DNX_SAND_CHECK_NULL_INPUT(cal_info);
  DNX_SAND_CHECK_NULL_INPUT(egq_pmc_name);
  DNX_SAND_CHECK_NULL_INPUT(egq_cr_field_name);

  switch (cal_info->cal_type)
  {
  case JER2_ARAD_OFP_RATES_EGQ_CAL_CHAN_ARB:
    *egq_pmc_name = EGQ_PMCm;
    *egq_cr_field_name = PORT_MAX_CREDITf;    
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CAL_PORT_PRIORITY:
    *egq_pmc_name = EGQ_QP_PMCm;
    *egq_cr_field_name = QPAIR_MAX_CREDITf;
    break;
  case JER2_ARAD_OFP_RATES_EGQ_CAL_TCG:
    *egq_pmc_name = EGQ_TCG_PMCm;
    *egq_cr_field_name = TCG_MAX_CREDITf;
    break;
  default:
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_OFP_RATES_EGQ_CAL_INVALID_ERR, 25, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egq_pmc_names_get()",0,0);
}

/*
 * Read indirect table pmc_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_egq_pmc_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   JER2_ARAD_OFP_RATES_CAL_INFO *cal_info,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_PMC_TBL_DATA* EGQ_pmc_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_EGQ_PMC_TBL_ENTRY_SIZE];
  soc_mem_t
    egq_pmc_name;
  soc_field_t
    egq_max_credit_field_name;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGQ_PMC_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          EGQ_pmc_tbl_data,
          0x0,
          sizeof(JER2_ARAD_EGQ_PMC_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = jer2_arad_egq_pmc_names_get(
          unit,
          cal_info,
          &egq_pmc_name,
          &egq_max_credit_field_name
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          egq_pmc_name,
          EGQ_BLOCK(unit, core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

err=SOC_E_NONE;
    if(SOC_E_NONE == err) {
          EGQ_pmc_tbl_data->port_max_credit   = soc_mem_field32_get(
                  unit,
                  egq_pmc_name,
                  data,
                  egq_max_credit_field_name            );
        }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egq_pmc_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table pmc_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_egq_pmc_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   JER2_ARAD_OFP_RATES_CAL_INFO *cal_info,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_PMC_TBL_DATA* EGQ_pmc_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_EGQ_PMC_TBL_ENTRY_SIZE];
  soc_mem_t
    egq_pmc_name;
  soc_field_t
    egq_max_credit_field_name;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGQ_PMC_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = jer2_arad_egq_pmc_names_get(
          unit,
          cal_info,
          &egq_pmc_name,
          &egq_max_credit_field_name
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

err=SOC_E_NONE;
  if(SOC_E_NONE == err) {
            soc_mem_field32_set(
          unit,
          egq_pmc_name,
          data,
          egq_max_credit_field_name,
          EGQ_pmc_tbl_data->port_max_credit
      );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  err = soc_mem_write(
          unit,
          egq_pmc_name,
          EGQ_BLOCK(unit, core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egq_pmc_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table dwm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_egq_dwm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_DWM_TBL_DATA* EGQ_dwm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_EGQ_DWM_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGQ_DWM_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          EGQ_dwm_tbl_data,
          0x0,
          sizeof(JER2_ARAD_EGQ_DWM_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          EGQ_DWMm,
          EGQ_BLOCK(unit, core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  EGQ_dwm_tbl_data->mc_or_mc_low_queue_weight = soc_mem_field32_get(
                  unit,
                  EGQ_DWMm,
                  data,
                  MC_OR_MC_LOW_QUEUE_WEIGHTf);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);


  EGQ_dwm_tbl_data->uc_or_uc_low_queue_weight   = soc_mem_field32_get(
          unit,
          EGQ_DWMm,
          data,
          UC_OR_UC_LOW_QUEUE_WEIGHTf);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 41, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egq_dwm_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table dwm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_egq_dwm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_DWM_TBL_DATA* EGQ_dwm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_EGQ_DWM_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_EGQ_DWM_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);


  soc_mem_field32_set(
          unit,
          EGQ_DWMm,
          data,
          MC_OR_MC_LOW_QUEUE_WEIGHTf,
          EGQ_dwm_tbl_data->mc_or_mc_low_queue_weight );



  soc_mem_field32_set(
          unit,
          EGQ_DWMm,
          data,
          UC_OR_UC_LOW_QUEUE_WEIGHTf,
          EGQ_dwm_tbl_data->uc_or_uc_low_queue_weight );

  err = soc_mem_write(
          unit,
          EGQ_DWMm,
          EGQ_BLOCK(unit, core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egq_dwm_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table ppct_tbl from block EGQ,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */

int
  jer2_arad_egq_ppct_tbl_get_unsafe(
    DNX_SAND_IN   int                       unit,
    DNX_SAND_IN   int                       core_id,
    DNX_SAND_IN   uint32                    base_q_pair,
    DNX_SAND_OUT  JER2_ARAD_EGQ_PPCT_TBL_DATA*   EGQ_ppct_tbl_data
  )
{
    uint32
        rv, tmp = 0;
    uint32
        data[SOC_DNX_IMP_DEFS_MAX(EGQ_PPCT_NOF_LONGS)];
 
    DNXC_INIT_FUNC_DEFS;

    sal_memset(EGQ_ppct_tbl_data, 0x0, sizeof(JER2_ARAD_EGQ_PPCT_TBL_DATA));

    rv = READ_EGQ_PPCTm(unit, EGQ_BLOCK(unit, core_id), base_q_pair, data);
    DNXC_IF_ERR_EXIT(rv);

    if(SOC_IS_ARADPLUS_AND_BELOW(unit)) {
        EGQ_ppct_tbl_data->base_q_pair_num = soc_mem_field32_get(unit, EGQ_PPCTm, data, BASE_Q_PAIR_NUMf);
    } else {
        EGQ_ppct_tbl_data->base_q_pair_num = base_q_pair;
    }
    EGQ_ppct_tbl_data->cgm_interface = soc_mem_field32_get(unit, EGQ_PPCTm, data, CGM_INTERFACEf);
    EGQ_ppct_tbl_data->cgm_port_profile = soc_mem_field32_get(unit, EGQ_PPCTm, data, CGM_PORT_PROFILEf);
    EGQ_ppct_tbl_data->cnm_intrcpt_drop_en = soc_mem_field32_get(unit, EGQ_PPCTm, data, CNM_INTRCPT_DROP_ENf);
    EGQ_ppct_tbl_data->cnm_intrcpt_fc_en = soc_mem_field32_get(unit, EGQ_PPCTm, data, CNM_INTRCPT_FC_ENf);

    tmp = soc_mem_field32_get(unit, EGQ_PPCTm, data, CNM_INTRCPT_FC_VEC_INDEXf);
    if(SOC_IS_ARADPLUS_AND_BELOW(unit)) {
        SHR_BITCOPY_RANGE(&(EGQ_ppct_tbl_data->cnm_intrcpt_fc_vec_llfc),0,&tmp,0,5);
        SHR_BITCOPY_RANGE(&(EGQ_ppct_tbl_data->cnm_intrcpt_fc_vec_pfc),0,&tmp,5,8);
    } else {
        SHR_BITCOPY_RANGE(&(EGQ_ppct_tbl_data->cnm_intrcpt_fc_vec_llfc),0,&tmp,0,8);
        SHR_BITCOPY_RANGE(&(EGQ_ppct_tbl_data->cnm_intrcpt_fc_vec_pfc),0,&tmp,8,7);
    }

    EGQ_ppct_tbl_data->disable_filtering = soc_mem_field32_get(unit, EGQ_PPCTm, data, DISABLE_FILTERINGf);
    EGQ_ppct_tbl_data->cos_map_profile = soc_mem_field32_get(unit, EGQ_PPCTm, data, COS_MAP_PROFILEf);
    EGQ_ppct_tbl_data->ad_count_out_port_flag = soc_mem_field32_get(unit, EGQ_PPCTm, data, ADVA_COUNT_OUT_PORT_FLAGf);
    EGQ_ppct_tbl_data->is_stacking_port = soc_mem_field32_get(unit, EGQ_PPCTm, data, IS_STACKING_PORTf);
    EGQ_ppct_tbl_data->lb_key_min = soc_mem_field32_get(unit, EGQ_PPCTm, data, LB_KEY_MINf);
    EGQ_ppct_tbl_data->lb_key_max = soc_mem_field32_get(unit, EGQ_PPCTm, data, LB_KEY_MAXf);
    EGQ_ppct_tbl_data->pmf_data = soc_mem_field32_get(unit, EGQ_PPCTm, data, PMF_DATAf);
    EGQ_ppct_tbl_data->peer_tm_domain_id = soc_mem_field32_get(unit, EGQ_PPCTm, data, PEER_TM_DOMAIN_IDf);
    EGQ_ppct_tbl_data->port_type = soc_mem_field32_get(unit, EGQ_PPCTm, data, PORT_TYPEf);

    if (SOC_IS_JERICHO(unit)) {
        EGQ_ppct_tbl_data->second_range_lb_key_min = soc_mem_field32_get(unit, EGQ_PPCTm, data, SECOND_RANGE_LB_KEY_MINf);
        EGQ_ppct_tbl_data->second_range_lb_key_max = soc_mem_field32_get(unit, EGQ_PPCTm, data, SECOND_RANGE_LB_KEY_MAXf);
    }


exit:
    DNXC_FUNC_RETURN;
}

/*
 * Write indirect table ppct_tbl from block EGQ,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */

int
  jer2_arad_egq_ppct_tbl_set_unsafe(
    DNX_SAND_IN   int                       unit,
    DNX_SAND_IN   int                       core_id,
    DNX_SAND_IN   uint32                    base_q_pair,
    DNX_SAND_IN   JER2_ARAD_EGQ_PPCT_TBL_DATA*   EGQ_ppct_tbl_data
  )
{
    uint32 tmp = 0;
    uint32 data[SOC_DNX_IMP_DEFS_MAX(EGQ_PPCT_NOF_LONGS)];
    int rv;
 
    DNXC_INIT_FUNC_DEFS;

    sal_memset(data, 0, sizeof(uint32) * SOC_DNX_IMP_DEFS_MAX(EGQ_PPCT_NOF_LONGS));

    if(SOC_IS_ARADPLUS_AND_BELOW(unit)) {
        soc_mem_field32_set(unit, EGQ_PPCTm, data, BASE_Q_PAIR_NUMf, EGQ_ppct_tbl_data->base_q_pair_num);
    }

    soc_mem_field32_set(unit, EGQ_PPCTm, data, CGM_INTERFACEf, EGQ_ppct_tbl_data->cgm_interface);
    soc_mem_field32_set(unit, EGQ_PPCTm, data, CGM_PORT_PROFILEf, EGQ_ppct_tbl_data->cgm_port_profile);
    soc_mem_field32_set(unit, EGQ_PPCTm, data, CNM_INTRCPT_DROP_ENf, EGQ_ppct_tbl_data->cnm_intrcpt_drop_en);
    soc_mem_field32_set(unit, EGQ_PPCTm, data, CNM_INTRCPT_FC_ENf, EGQ_ppct_tbl_data->cnm_intrcpt_fc_en);

    if(SOC_IS_ARADPLUS_AND_BELOW(unit)) {
        SHR_BITCOPY_RANGE(&tmp,0,&(EGQ_ppct_tbl_data->cnm_intrcpt_fc_vec_llfc),0,5);
        SHR_BITCOPY_RANGE(&tmp,5,&(EGQ_ppct_tbl_data->cnm_intrcpt_fc_vec_pfc),0,8);
    } else {
        SHR_BITCOPY_RANGE(&tmp,0,&(EGQ_ppct_tbl_data->cnm_intrcpt_fc_vec_llfc),0,8);
        SHR_BITCOPY_RANGE(&tmp,8,&(EGQ_ppct_tbl_data->cnm_intrcpt_fc_vec_pfc),0,7);
    }
    soc_mem_field32_set(unit, EGQ_PPCTm, data, CNM_INTRCPT_FC_VEC_INDEXf, tmp);

    soc_mem_field32_set(unit, EGQ_PPCTm, data, DISABLE_FILTERINGf, EGQ_ppct_tbl_data->disable_filtering);
    soc_mem_field32_set(unit, EGQ_PPCTm, data, COS_MAP_PROFILEf, EGQ_ppct_tbl_data->cos_map_profile);
    soc_mem_field32_set(unit, EGQ_PPCTm, data, ADVA_COUNT_OUT_PORT_FLAGf, EGQ_ppct_tbl_data->ad_count_out_port_flag);
    soc_mem_field32_set(unit, EGQ_PPCTm, data, IS_STACKING_PORTf, EGQ_ppct_tbl_data->is_stacking_port);
    soc_mem_field32_set(unit, EGQ_PPCTm, data, LB_KEY_MINf, EGQ_ppct_tbl_data->lb_key_min);
    soc_mem_field32_set(unit, EGQ_PPCTm, data, LB_KEY_MAXf, EGQ_ppct_tbl_data->lb_key_max);
    soc_mem_field32_set(unit, EGQ_PPCTm, data, PMF_DATAf, EGQ_ppct_tbl_data->pmf_data);
    soc_mem_field32_set(unit, EGQ_PPCTm, data, PEER_TM_DOMAIN_IDf, EGQ_ppct_tbl_data->peer_tm_domain_id);
    soc_mem_field32_set(unit, EGQ_PPCTm, data, PORT_TYPEf, EGQ_ppct_tbl_data->port_type);

    if (SOC_IS_JERICHO(unit)) {
        soc_mem_field32_set(unit, EGQ_PPCTm, data, SECOND_RANGE_LB_KEY_MINf, EGQ_ppct_tbl_data->second_range_lb_key_min);
        soc_mem_field32_set(unit, EGQ_PPCTm, data, SECOND_RANGE_LB_KEY_MAXf, EGQ_ppct_tbl_data->second_range_lb_key_max);
    }

    rv = WRITE_EGQ_PPCTm(unit, EGQ_BLOCK(unit, core_id), base_q_pair, data);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}


#ifdef BCM_88660_A0

/*
 * Read indirect table egq_per_port_lb_range from block EGQ,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_egq_per_port_lb_range_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32             entry_offset,
    DNX_SAND_OUT  JER2_ARAD_PER_PORT_LB_RANGE_TBL_DATA* EGQ_per_port_lb_range_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_EGQ_PER_PORT_LB_RANGE_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  DNX_SAND_CHECK_NULL_INPUT(EGQ_per_port_lb_range_tbl_data);

  err = dnx_sand_os_memset(
          EGQ_per_port_lb_range_tbl_data,
          0x0,
          sizeof(JER2_ARAD_PER_PORT_LB_RANGE_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = soc_mem_read(
          unit,
          EGQ_PER_PORT_LB_RANGEm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  SHR_BITCOPY_RANGE(&(EGQ_per_port_lb_range_tbl_data->lb_key_min), 0, data, 0, 8);
  SHR_BITCOPY_RANGE(&(EGQ_per_port_lb_range_tbl_data->lb_key_max), 0, data, 8, 8);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egq_per_port_lb_range_get_unsafe()",0,0);
}

/*
 * Write indirect table egq_per_port_lb_range from block EGQ,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_egq_per_port_lb_range_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_PER_PORT_LB_RANGE_TBL_DATA* EGQ_per_port_lb_range_tbl_data
  )
{
  uint32
    res, err;
  uint32    
    data[JER2_ARAD_EGQ_PER_PORT_LB_RANGE_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  DNX_SAND_CHECK_NULL_INPUT(EGQ_per_port_lb_range_tbl_data);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_EGQ_PER_PORT_LB_RANGE_TBL_ENTRY_SIZE);
  
  SHR_BITCOPY_RANGE(data, 0, &(EGQ_per_port_lb_range_tbl_data->lb_key_min), 0, 8);
  SHR_BITCOPY_RANGE(data, 8, &(EGQ_per_port_lb_range_tbl_data->lb_key_max), 0, 8);          

  err = soc_mem_write(
          unit,
          EGQ_PER_PORT_LB_RANGEm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 35, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egq_per_port_lb_range_tbl_set_unsafe()",0,0);
}

#endif /*BCM_88660_A0*/

/*
 * CAL table read and write: 
 *  
 * All scheduler calendars share the same table.
 *  The order is:
 *  MAL0-CALA
 *  MAL0-CALB
 *  MAL1-CALA
 *  MAL1-CALB
 *  ...
 *  CPU-CALA
 *  CPU-CALB
 *  RCY-CALA
 *  RCY-CALB
 */ 

#define JER2_ARAD_OFP_RATES_CAL_CHAN_ARB_BASE_SCH_OFFSET          0x0800
#define JER2_ARAD_OFP_RATES_CAL_LEN_SCH_OFFSET                    0x1000

int
  jer2_arad_sch_cal_tbl_get(
    DNX_SAND_IN   int                    unit,
    DNX_SAND_IN   int                    core_id,
    DNX_SAND_IN   uint32                 sch_offset,
    DNX_SAND_IN   uint32                 sch_to_get /*A (0) or B (1)*/,
    DNX_SAND_IN   uint32                 slots_count,
    DNX_SAND_OUT  uint32*                slots
  )
{
    uint32
        cal_offset,
        slot,
        entry;

    DNXC_INIT_FUNC_DEFS;

    cal_offset = JER2_ARAD_OFP_RATES_CAL_CHAN_ARB_BASE_SCH_OFFSET * (sch_offset / 8) + ((sch_offset % 8) * JER2_ARAD_OFP_NOF_RATES_CAL_SETS + sch_to_get) * JER2_ARAD_OFP_RATES_CAL_LEN_SCH_OFFSET;

    for(slot=0; slot < slots_count; slot++) {
        entry = 0;
        DNXC_IF_ERR_EXIT(READ_SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR__CALm(unit, MEM_BLOCK_ALL, cal_offset+slot, &entry));
        soc_mem_field_get(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR__CALm, &entry, HR_SELf, &(slots[slot]));
    }

exit:
    DNXC_FUNC_RETURN;
}

int
  jer2_arad_sch_cal_max_size_get(
    DNX_SAND_IN   int                    unit,
    DNX_SAND_IN   uint32                 sch_offset,
    DNX_SAND_OUT  uint32*                max_cal_size
   )
{
    DNXC_INIT_FUNC_DEFS;

    *max_cal_size = JER2_ARAD_OFP_RATES_CAL_LEN_SCH_MAX;

    DNXC_FUNC_RETURN;
}

int
  jer2_arad_sch_cal_tbl_set(
    DNX_SAND_IN   int                    unit,
    DNX_SAND_IN   int                    core_id,
    DNX_SAND_IN   uint32                 sch_offset,
    DNX_SAND_IN   uint32                 sch_to_set /*A (0) or B (1)*/,
    DNX_SAND_IN   uint32                 slots_count,
                  uint32*                slots
  )
{
    uint32
        cal_offset,
        slot,
        entry;

    DNXC_INIT_FUNC_DEFS;

    cal_offset = JER2_ARAD_OFP_RATES_CAL_CHAN_ARB_BASE_SCH_OFFSET * (sch_offset / 8) + ((sch_offset % 8) * JER2_ARAD_OFP_NOF_RATES_CAL_SETS + sch_to_set) * JER2_ARAD_OFP_RATES_CAL_LEN_SCH_OFFSET;

    for (slot=0; slot < slots_count; slot++) {
        entry = 0;
        soc_mem_field_set(unit, SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR__CALm, &entry, HR_SELf, &(slots[slot]));
        DNXC_IF_ERR_EXIT(WRITE_SCH_SCHEDULER_CREDIT_GENERATION_CALENDAR__CALm(unit, MEM_BLOCK_ALL, cal_offset+slot, &entry));
    }

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Read indirect table drm_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_drm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_DRM_TBL_DATA* SCH_drm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_DRM_TBL_ENTRY_SIZE];
  soc_mem_t mem ;
  soc_field_t field ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_DRM_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(&(data[0]),0x0,sizeof(data)) ;
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err =dnx_sand_os_memset(SCH_drm_tbl_data,0x0,sizeof(JER2_ARAD_SCH_DRM_TBL_DATA));
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  /*
   * Note that for Arad:
   * mem = (soc_mem_t)SCH_DEVICE_RATE_MEMORY__DRMm ;
   * field = (soc_field_t)DEVICE_RATEf ;
   * They are aliased in regs_alias.pm
   */
  mem = (soc_mem_t)SCH_SHARED_DEVICE_RATE_SHARED_DRMm ;
  field = (soc_field_t)SHARED_DRMf ;
  /*
   * Use SCH0 although, apparently, any of the two may be used.
   */
  err =soc_mem_read(unit,mem,SCH_BLOCK(unit, 0),entry_offset,data) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  SCH_drm_tbl_data->device_rate = soc_mem_field32_get(unit,mem,data,field);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_drm_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table drm_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_drm_tbl_set_unsafe(
    DNX_SAND_IN   int                   unit,
    DNX_SAND_IN   uint32                entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_DRM_TBL_DATA *SCH_drm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_DRM_TBL_ENTRY_SIZE];
  soc_mem_t mem ;
  soc_field_t field ;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_DRM_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(&(data[0]),0x0,sizeof(data)) ;
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);
  /*
   * Note that for Arad:
   * mem = (soc_mem_t)SCH_DEVICE_RATE_MEMORY__DRMm ;
   * field = (soc_field_t)DEVICE_RATEf ;
   * They are aliased in regs_alias.pm
   */
  mem = (soc_mem_t)SCH_SHARED_DEVICE_RATE_SHARED_DRMm ;
  field = (soc_field_t)SHARED_DRMf ;
  /*
   * Use SCH0 although, apparently, any of the two may be used.
   */
  soc_mem_field32_set(unit,mem,data,field,SCH_drm_tbl_data->device_rate);
  err = soc_mem_write(unit,mem,SCH_BLOCK(unit, 0),entry_offset,data) ;
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_drm_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table dsm_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_dsm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_DSM_TBL_DATA* SCH_dsm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_DSM_TBL_ENTRY_SIZE];

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_DSM_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          SCH_dsm_tbl_data,
          0x0,
          sizeof(JER2_ARAD_SCH_DSM_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          SCH_DUAL_SHAPER_MEMORY_DSMm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  {
    SCH_dsm_tbl_data->dual_shaper_ena   = soc_mem_field32_get(
            unit,
            SCH_DUAL_SHAPER_MEMORY_DSMm,
            data,
            DUAL_SHAPER_ENAf
    );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_dsm_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table dsm_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_dsm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_DSM_TBL_DATA* SCH_dsm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_DSM_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_DSM_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  {
    soc_mem_field32_set(
          unit,
          SCH_DUAL_SHAPER_MEMORY_DSMm,
          data,
          DUAL_SHAPER_ENAf,
          SCH_dsm_tbl_data->dual_shaper_ena
    ) ;
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  err = soc_mem_write(
          unit,
          SCH_DUAL_SHAPER_MEMORY_DSMm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_dsm_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table fdms_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_fdms_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_FDMS_TBL_DATA* SCH_fdms_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_FDMS_TBL_ENTRY_SIZE];
  uint32
    entry_offset = offset;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FDMS_TBL_GET_UNSAFE);

  if (SOC_IS_QUX(unit)) {
      entry_offset -= QUX_SCH_FDMS_OFFSET;
  }
  else if (SOC_IS_QAX(unit)) {
      entry_offset -= JER2_QAX_SCH_FDMS_OFFSET;
  }
  
  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          SCH_fdms_tbl_data,
          0x0,
          sizeof(JER2_ARAD_SCH_FDMS_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);
  err = soc_mem_read(
          unit,
          SCH_FLOW_DESCRIPTOR_MEMORY_STATIC_FDMSm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);
  SCH_fdms_tbl_data->sch_number   = soc_mem_field32_get(
      unit,SCH_FLOW_DESCRIPTOR_MEMORY_STATIC_FDMSm,
      data,SCH_NUMBERf            );
  SCH_fdms_tbl_data->cos   = soc_mem_field32_get(
      unit,SCH_FLOW_DESCRIPTOR_MEMORY_STATIC_FDMSm,
      data,COSf            );
  SCH_fdms_tbl_data->hrsel_dual   = soc_mem_field32_get(
      unit, SCH_FLOW_DESCRIPTOR_MEMORY_STATIC_FDMSm,
      data,HR_SEL_DUALf            );    
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_fdms_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table fdms_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_fdms_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              offset,
    DNX_SAND_IN   JER2_ARAD_SCH_FDMS_TBL_DATA* SCH_fdms_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_FDMS_TBL_ENTRY_SIZE];
  uint32
    entry_offset = offset;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FDMS_TBL_SET_UNSAFE);

  if (SOC_IS_QUX(unit)) {
      entry_offset -= QUX_SCH_FDMS_OFFSET;
  }
  else if (SOC_IS_QAX(unit)) {
      entry_offset -= JER2_QAX_SCH_FDMS_OFFSET;
  }

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);
  soc_mem_field32_set(
      unit,SCH_FLOW_DESCRIPTOR_MEMORY_STATIC_FDMSm,
      data,SCH_NUMBERf,
      SCH_fdms_tbl_data->sch_number );
  soc_mem_field32_set(
      unit,SCH_FLOW_DESCRIPTOR_MEMORY_STATIC_FDMSm,
      data,COSf,
      SCH_fdms_tbl_data->cos );
  soc_mem_field32_set(
      unit, SCH_FLOW_DESCRIPTOR_MEMORY_STATIC_FDMSm,
      data, HR_SEL_DUALf,
      SCH_fdms_tbl_data->hrsel_dual );
  err = soc_mem_write(
          unit,
          SCH_FLOW_DESCRIPTOR_MEMORY_STATIC_FDMSm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 33, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_fdms_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table shds_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_shds_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_SHDS_TBL_DATA* SCH_shds_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_SHDS_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SHDS_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          SCH_shds_tbl_data,
          0x0,
          sizeof(JER2_ARAD_SCH_SHDS_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  {
    SCH_shds_tbl_data->peak_rate_man_even   = soc_mem_field32_get(
                  unit,
                  SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
                  data,
                  PEAK_RATE_MAN_EVENf            );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);
  {
    SCH_shds_tbl_data->peak_rate_exp_even   = soc_mem_field32_get(
                  unit,
                  SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
                  data,
                  PEAK_RATE_EXP_EVENf            );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 41, exit);
  {
    SCH_shds_tbl_data->max_burst_even   = soc_mem_field32_get(
                  unit,
                  SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
                  data,
                  MAX_BURST_EVENf            );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 42, exit);
  {
    SCH_shds_tbl_data->slow_rate2_sel_even   = soc_mem_field32_get(
                  unit,
                  SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
                  data,
                  SLOW_RATE_2_SEL_EVENf            );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 43, exit);
  {
    SCH_shds_tbl_data->peak_rate_man_odd   = soc_mem_field32_get(
                  unit,
                  SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
                  data,
                  PEAK_RATE_MAN_ODDf            );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 44, exit);
  {
    SCH_shds_tbl_data->peak_rate_exp_odd   = soc_mem_field32_get(
                  unit,
                  SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
                  data,
                  PEAK_RATE_EXP_ODDf            );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 45, exit);
  {
    SCH_shds_tbl_data->max_burst_odd   = soc_mem_field32_get(
                  unit,
                  SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
                  data,
                  MAX_BURST_ODDf            );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 46, exit);
  {
    SCH_shds_tbl_data->slow_rate2_sel_odd   = soc_mem_field32_get(
                  unit,
                  SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
                  data,
                  SLOW_RATE_2_SEL_ODDf            );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 47, exit);
  /*
   * The following clause is for Arad only (not for Jericho).
   */
  {
    SCH_shds_tbl_data->max_burst_update_even   = soc_mem_field32_get(
                  unit,
                  SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
                  data,
                  MAX_BURST_UPDATE_EVENf            );
    DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 48, exit);
    SCH_shds_tbl_data->max_burst_update_odd   = soc_mem_field32_get(
                  unit,
                  SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
                  data,
                  MAX_BURST_UPDATE_ODDf            );
    DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 49, exit);
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_shds_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table shds_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_shds_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_SHDS_TBL_DATA* SCH_shds_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_SHDS_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SHDS_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);
  {
    soc_mem_field32_set(
          unit,
          SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
          data,
          PEAK_RATE_MAN_EVENf,
          SCH_shds_tbl_data->peak_rate_man_even );
    soc_mem_field32_set(
          unit,
          SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
          data,
          PEAK_RATE_EXP_EVENf,
          SCH_shds_tbl_data->peak_rate_exp_even );
    soc_mem_field32_set(
          unit,
          SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
          data,
          MAX_BURST_EVENf,
          SCH_shds_tbl_data->max_burst_even );
    soc_mem_field32_set(
          unit,
          SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
          data,
          SLOW_RATE_2_SEL_EVENf,
          SCH_shds_tbl_data->slow_rate2_sel_even );
    soc_mem_field32_set(
          unit,
          SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
          data,
          PEAK_RATE_MAN_ODDf,
          SCH_shds_tbl_data->peak_rate_man_odd );
    soc_mem_field32_set(
          unit,
          SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
          data,
          PEAK_RATE_EXP_ODDf,
          SCH_shds_tbl_data->peak_rate_exp_odd );
    soc_mem_field32_set(
          unit,
          SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
          data,
          MAX_BURST_ODDf,
          SCH_shds_tbl_data->max_burst_odd );
    soc_mem_field32_set(
          unit,
          SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
          data,
          SLOW_RATE_2_SEL_ODDf,
          SCH_shds_tbl_data->slow_rate2_sel_odd );
  }
  /*
   * The following clause is for Arad only (not for Jericho).
   */
  {
    soc_mem_field32_set(
          unit,
          SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
          data,
          MAX_BURST_UPDATE_EVENf,
          SCH_shds_tbl_data->max_burst_update_even );
    DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 38, exit);
    soc_mem_field32_set(
          unit,
          SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
          data,
          MAX_BURST_UPDATE_ODDf,
          SCH_shds_tbl_data->max_burst_update_odd );
    DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 39, exit);
  }
  err = soc_mem_write(
          unit,
          SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_shds_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table sem_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_sem_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_SEM_TBL_DATA* SCH_sem_tbl_data
  )
{
  uint32
    res;
  uint32
    data[JER2_ARAD_SCH_SEM_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SEM_TBL_GET_UNSAFE);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_SCH_SEM_TBL_ENTRY_SIZE);
  JER2_ARAD_CLEAR(SCH_sem_tbl_data, JER2_ARAD_SCH_SEM_TBL_DATA, 1);

  res = soc_mem_read(unit, SCH_SCHEDULER_ENABLE_MEMORY_SEMm, SCH_BLOCK(unit,core), entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

  SCH_sem_tbl_data->sch_enable = soc_mem_field32_get(unit, SCH_SCHEDULER_ENABLE_MEMORY_SEMm, data, SCH_ENABLEf);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_sem_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table sem_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_sem_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_SEM_TBL_DATA* SCH_sem_tbl_data
  )
{
  uint32
    res;
  uint32
    data[JER2_ARAD_SCH_SEM_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SEM_TBL_SET_UNSAFE);
  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_SCH_SEM_TBL_ENTRY_SIZE);

  soc_mem_field32_set(unit, SCH_SCHEDULER_ENABLE_MEMORY_SEMm, data, SCH_ENABLEf, SCH_sem_tbl_data->sch_enable);

  res = soc_mem_write(
          unit,
          SCH_SCHEDULER_ENABLE_MEMORY_SEMm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );


  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 31, exit);

  if (!SOC_IS_ARADPLUS_AND_BELOW(unit)) {
      if (SOC_IS_QUX(unit)) {
        soc_mem_field32_set(unit, SCH_MEM_07100000m, data, ITEM_0_7f ,SCH_sem_tbl_data->sch_enable);
        res = soc_mem_write(unit, SCH_MEM_07100000m, SCH_BLOCK(unit, core), entry_offset, data);
      }
      else {
        soc_mem_field32_set(unit, SCH_SCHEDULER_ENABLE_MEMORY_SEM_Bm, data, SCH_ENABLEf ,SCH_sem_tbl_data->sch_enable);
        res = soc_mem_write(unit, SCH_SCHEDULER_ENABLE_MEMORY_SEM_Bm, SCH_BLOCK(unit, core), entry_offset, data);
      }
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 37, exit);
  }

  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_sem_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table fsf_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_fsf_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_FSF_TBL_DATA* SCH_fsf_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_FSF_TBL_ENTRY_SIZE];
  uint32
    entry_offset = offset;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FSF_TBL_GET_UNSAFE);

  if (SOC_IS_QUX(unit)) {
      entry_offset -= QUX_SCH_FSF_OFFSET;
  }
  else if (SOC_IS_QAX(unit)) {
      entry_offset -= JER2_QAX_SCH_FSF_OFFSET;
  }
  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          SCH_fsf_tbl_data,
          0x0,
          sizeof(JER2_ARAD_SCH_FSF_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

   err = soc_mem_read(
          unit,
          SCH_FLOW_SUB_FLOW_FSFm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  {
      SCH_fsf_tbl_data->sfenable   = soc_mem_field32_get(
                  unit,
                  SCH_FLOW_SUB_FLOW_FSFm,
                  data,
                  SF_ENABLEf            
      );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_fsf_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table fsf_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_fsf_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              offset,
    DNX_SAND_IN   JER2_ARAD_SCH_FSF_TBL_DATA* SCH_fsf_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_FSF_TBL_ENTRY_SIZE];
  uint32
    entry_offset = offset;
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FSF_TBL_SET_UNSAFE);

  if (SOC_IS_QUX(unit)) {
      entry_offset -= QUX_SCH_FSF_OFFSET;
  }
  else if (SOC_IS_QAX(unit)) {
      entry_offset -= JER2_QAX_SCH_FSF_OFFSET;
  }

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  {
    soc_mem_field32_set(
          unit,
          SCH_FLOW_SUB_FLOW_FSFm,
          data,
          SF_ENABLEf,
          SCH_fsf_tbl_data->sfenable
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  err = soc_mem_write(
          unit,
          SCH_FLOW_SUB_FLOW_FSFm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);
  

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_fsf_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table fgm_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_fgm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_FGM_TBL_DATA* SCH_fgm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_FGM_TBL_ENTRY_SIZE];
 
 
 
  uint32
    fld_idx = 0;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FGM_TBL_GET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          SCH_fgm_tbl_data,
          0x0,
          sizeof(JER2_ARAD_SCH_FGM_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          SCH_FLOW_GROUP_MEMORY_FGMm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  for (fld_idx = 0; fld_idx < JER2_ARAD_TBL_FGM_NOF_GROUPS_ONE_LINE; fld_idx++)
  {
    {
          SCH_fgm_tbl_data->flow_group[fld_idx]     = soc_mem_field32_get(
                  unit,
                  SCH_FLOW_GROUP_MEMORY_FGMm,
                  data,
                  FLOW_GROUP_0f + fld_idx            
        );
    }
    DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 20, exit);
  }


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_fgm_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table fgm_tbl from block SCH,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_fgm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_FGM_TBL_DATA* SCH_fgm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_FGM_TBL_ENTRY_SIZE];
  uint32
    fld_idx = 0;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FGM_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  for (fld_idx = 0; fld_idx < JER2_ARAD_TBL_FGM_NOF_GROUPS_ONE_LINE; fld_idx++)
  {
    {
      soc_mem_field32_set(
          unit,
          SCH_FLOW_GROUP_MEMORY_FGMm,
          data,
          FLOW_GROUP_0f + fld_idx,
            SCH_fgm_tbl_data->flow_group[fld_idx]
          );
    }
    DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 20, exit);
  }
  err = soc_mem_write(
          unit,
          SCH_FLOW_GROUP_MEMORY_FGMm,
          SCH_BLOCK(unit,core),
          entry_offset,
            data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 33, exit);
  

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_fgm_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table shc_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_shc_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_SHC_TBL_DATA* SCH_shc_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_SHC_TBL_ENTRY_SIZE];
 
 
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SHC_TBL_GET_UNSAFE);

  DNX_SAND_ERR_IF_ABOVE_MAX(core, SOC_DNX_DEFS_GET(unit, nof_cores) , JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,15,exit);
  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          SCH_shc_tbl_data,
          0x0,
          sizeof(JER2_ARAD_SCH_SHC_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          SCH_HR_SCHEDULER_CONFIGURATION_SHCm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  {
    SCH_shc_tbl_data->hrmode =
      soc_mem_field32_get(
        unit,
        SCH_HR_SCHEDULER_CONFIGURATION_SHCm,
        data,
        HR_MODEf
      );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

  DNX_SAND_CHECK_FUNC_RESULT(err, 41, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_shc_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table shc_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_shc_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_SHC_TBL_DATA* SCH_shc_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_SHC_TBL_ENTRY_SIZE];
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SHC_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

 {
    soc_mem_field32_set(
          unit,
          SCH_HR_SCHEDULER_CONFIGURATION_SHCm,
          data,
          HR_MODEf,
          SCH_shc_tbl_data->hrmode );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);

  err = soc_mem_write(
          unit,
          SCH_HR_SCHEDULER_CONFIGURATION_SHCm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 33, exit);
  

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_shc_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table scc_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_scc_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_SCC_TBL_DATA* SCH_scc_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_SCC_TBL_ENTRY_SIZE];
  uint32
    entry_offset = offset;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SCC_TBL_GET_UNSAFE);

  if (SOC_IS_QUX(unit)) {
      entry_offset -= QUX_SCH_SCC_OFFSET;
  }
  else if (SOC_IS_QAX(unit)) {
      entry_offset -= JER2_QAX_SCH_SCC_OFFSET;
  }
  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          SCH_scc_tbl_data,
          0x0,
          sizeof(JER2_ARAD_SCH_SCC_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          SCH_CL_SCHEDULERS_CONFIGURATION_SCCm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  {
    SCH_scc_tbl_data->clsch_type =
      soc_mem_field32_get(
                  unit,
                  SCH_CL_SCHEDULERS_CONFIGURATION_SCCm,
                  data,
                  CL_SCH_TYPEf            
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_scc_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table scc_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_scc_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              offset,
    DNX_SAND_IN   JER2_ARAD_SCH_SCC_TBL_DATA* SCH_scc_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_SCC_TBL_ENTRY_SIZE];
  uint32
    entry_offset = offset;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SCC_TBL_SET_UNSAFE);

  if (SOC_IS_QUX(unit)) {
      entry_offset -= QUX_SCH_SCC_OFFSET;
  }
  else if (SOC_IS_QAX(unit)) {
      entry_offset -= JER2_QAX_SCH_SCC_OFFSET;
  }

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  {
    soc_mem_field32_set(
          unit,
          SCH_CL_SCHEDULERS_CONFIGURATION_SCCm,
          data,
          CL_SCH_TYPEf,
          SCH_scc_tbl_data->clsch_type
    );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  err = soc_mem_write(
          unit,
          SCH_CL_SCHEDULERS_CONFIGURATION_SCCm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_scc_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table sct_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_sct_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_SCT_TBL_DATA* SCH_sct_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_SCT_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SCT_TBL_GET_UNSAFE);
  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          SCH_sct_tbl_data,
          0x0,
          sizeof(JER2_ARAD_SCH_SCT_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);
  err = soc_mem_read(
          unit,
          SCH_CL_SCHEDULERS_TYPE_SCTm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);
  {
    SCH_sct_tbl_data->clconfig   = soc_mem_field32_get(
          unit,
          SCH_CL_SCHEDULERS_TYPE_SCTm,
          data,
          CL_CONFIGf            );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);
  {
    SCH_sct_tbl_data->af0_inv_weight   = soc_mem_field32_get(
            unit,
            SCH_CL_SCHEDULERS_TYPE_SCTm,
            data,
            AF_0_INV_WEIGHTf            
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 41, exit);
  {
    SCH_sct_tbl_data->af1_inv_weight   = soc_mem_field32_get(
         unit,
         SCH_CL_SCHEDULERS_TYPE_SCTm,
         data,
         AF_1_INV_WEIGHTf            
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 42, exit);
  {
    SCH_sct_tbl_data->af2_inv_weight   = soc_mem_field32_get(
          unit,
          SCH_CL_SCHEDULERS_TYPE_SCTm,
          data,
          AF_2_INV_WEIGHTf            
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 43, exit);
  {
    SCH_sct_tbl_data->af3_inv_weight   = soc_mem_field32_get(
         unit,
         SCH_CL_SCHEDULERS_TYPE_SCTm,
         data,
         AF_3_INV_WEIGHTf            
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 44, exit);
  {
    SCH_sct_tbl_data->wfqmode   = soc_mem_field32_get(
         unit,
         SCH_CL_SCHEDULERS_TYPE_SCTm,
         data,
         WFQ_MODEf            
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 45, exit);
  {
    SCH_sct_tbl_data->enh_clen   = soc_mem_field32_get(
        unit,
        SCH_CL_SCHEDULERS_TYPE_SCTm,
        data,
        ENH_CL_ENf            
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 46, exit);
  {
    SCH_sct_tbl_data->enh_clsphigh   = soc_mem_field32_get(
          unit,
          SCH_CL_SCHEDULERS_TYPE_SCTm,
          data,
          ENH_CL_SP_HIGHf            
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 47, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_sct_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table sct_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_sct_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_SCT_TBL_DATA* SCH_sct_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_SCT_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SCT_TBL_SET_UNSAFE);
  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit) ;
  {
    soc_mem_field32_set(
          unit,
          SCH_CL_SCHEDULERS_TYPE_SCTm,
          data,
          CL_CONFIGf,
          SCH_sct_tbl_data->clconfig );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);
  {
    soc_mem_field32_set(
          unit,
          SCH_CL_SCHEDULERS_TYPE_SCTm,
          data,
          AF_0_INV_WEIGHTf,
          SCH_sct_tbl_data->af0_inv_weight
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);
  {
    soc_mem_field32_set(
          unit,
          SCH_CL_SCHEDULERS_TYPE_SCTm,
          data,
          AF_1_INV_WEIGHTf,
          SCH_sct_tbl_data->af1_inv_weight
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);
  {
    soc_mem_field32_set(
          unit,
          SCH_CL_SCHEDULERS_TYPE_SCTm,
          data,
          AF_2_INV_WEIGHTf,
          SCH_sct_tbl_data->af2_inv_weight
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 33, exit);
  {
    soc_mem_field32_set(
          unit,
          SCH_CL_SCHEDULERS_TYPE_SCTm,
          data,
          AF_3_INV_WEIGHTf,
          SCH_sct_tbl_data->af3_inv_weight
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 34, exit);
  {
    soc_mem_field32_set(
          unit,
          SCH_CL_SCHEDULERS_TYPE_SCTm,
          data,
          WFQ_MODEf,
          SCH_sct_tbl_data->wfqmode
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 35, exit);
  {
    soc_mem_field32_set(
          unit,
          SCH_CL_SCHEDULERS_TYPE_SCTm,
          data,
          ENH_CL_ENf,
          SCH_sct_tbl_data->enh_clen
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 36, exit);
  {
    soc_mem_field32_set(
          unit,
          SCH_CL_SCHEDULERS_TYPE_SCTm,
          data,
          ENH_CL_SP_HIGHf,
          SCH_sct_tbl_data->enh_clsphigh
        );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 37, exit);

  err = soc_mem_write(
          unit,
          SCH_CL_SCHEDULERS_TYPE_SCTm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 38, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_sct_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table fqm_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_fqm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_FQM_TBL_DATA* SCH_fqm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_FQM_TBL_ENTRY_SIZE];
  uint32
    entry_offset = offset;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FQM_TBL_GET_UNSAFE);

  if (SOC_IS_QUX(unit)) {
      entry_offset -= QUX_SCH_FQM_OFFSET;
  }
  else if (SOC_IS_QAX(unit)) {
      entry_offset -= JER2_QAX_SCH_FQM_OFFSET;
  }
  
  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          SCH_fqm_tbl_data,
          0x0,
          sizeof(JER2_ARAD_SCH_FQM_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);

  err = soc_mem_read(
          unit,
          SCH_FLOW_TO_QUEUE_MAPPING_FQMm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  {
    SCH_fqm_tbl_data->base_queue_num   = soc_mem_field32_get(
                  unit,
                  SCH_FLOW_TO_QUEUE_MAPPING_FQMm,
                  data,
                  BASE_QUEUE_NUMf            );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);
  {
    SCH_fqm_tbl_data->sub_flow_mode   = soc_mem_field32_get(
                  unit,
                  SCH_FLOW_TO_QUEUE_MAPPING_FQMm,
                  data,
                  SUB_FLOW_MODEf            );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 41, exit);
  {
    SCH_fqm_tbl_data->flow_slow_enable   = soc_mem_field32_get(
                  unit,
                  SCH_FLOW_TO_QUEUE_MAPPING_FQMm,
                  data,
                  FLOW_SLOW_ENABLEf            );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 42, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_fqm_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table fqm_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_fqm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              offset,
    DNX_SAND_IN   JER2_ARAD_SCH_FQM_TBL_DATA* SCH_fqm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_FQM_TBL_ENTRY_SIZE];
  uint32
    entry_offset = offset;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FQM_TBL_SET_UNSAFE);

  if (SOC_IS_QUX(unit)) {
      entry_offset -= QUX_SCH_FQM_OFFSET;
  }
  else if (SOC_IS_QAX(unit)) {
      entry_offset -= JER2_QAX_SCH_FQM_OFFSET;
  }

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);
  {
    soc_mem_field32_set(
          unit,
          SCH_FLOW_TO_QUEUE_MAPPING_FQMm,
          data,
          BASE_QUEUE_NUMf,
          SCH_fqm_tbl_data->base_queue_num );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);
  {
    soc_mem_field32_set(
          unit,
          SCH_FLOW_TO_QUEUE_MAPPING_FQMm,
          data,
          SUB_FLOW_MODEf,
          SCH_fqm_tbl_data->sub_flow_mode );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);
  {
    soc_mem_field32_set(
          unit,
          SCH_FLOW_TO_QUEUE_MAPPING_FQMm,
          data,
          FLOW_SLOW_ENABLEf,
          SCH_fqm_tbl_data->flow_slow_enable );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);
  err = soc_mem_write(
          unit,
          SCH_FLOW_TO_QUEUE_MAPPING_FQMm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 33, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_fqm_tbl_set_unsafe()",0,0);
}

/*
 * Read indirect table ffm_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_ffm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_FFM_TBL_DATA* SCH_ffm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_FFM_TBL_ENTRY_SIZE];
  uint32
    entry_offset = offset;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FFM_TBL_GET_UNSAFE);
  if (SOC_IS_QUX(unit)) {
    SOC_EXIT;
  }
  if (SOC_IS_QAX(unit)) {
      entry_offset -= JER2_QAX_SCH_FFM_OFFSET;
  }

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  err = dnx_sand_os_memset(
          SCH_ffm_tbl_data,
          0x0,
          sizeof(JER2_ARAD_SCH_FFM_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 15, exit);
  err = soc_mem_read(
          unit,
          SCH_FLOW_TO_FIP_MAPPING_FFMm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);
  {
    SCH_ffm_tbl_data->device_number   = soc_mem_field32_get(
                  unit,
                  SCH_FLOW_TO_FIP_MAPPING_FFMm,
                  data,
                  DEVICE_NUMBERf            );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_ffm_tbl_get_unsafe()",0,0);
}

/*
 * Write indirect table ffm_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_ffm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              offset,
    DNX_SAND_IN   JER2_ARAD_SCH_FFM_TBL_DATA* SCH_ffm_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_FFM_TBL_ENTRY_SIZE];
  uint32
    entry_offset = offset;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FFM_TBL_SET_UNSAFE);
  if (SOC_IS_QUX(unit)) {
      SOC_EXIT;
  }
  if (SOC_IS_QAX(unit)) {
      entry_offset -= JER2_QAX_SCH_FFM_OFFSET;
  }
  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);
  {
    soc_mem_field32_set(
          unit,
          SCH_FLOW_TO_FIP_MAPPING_FFMm,
          data,
          DEVICE_NUMBERf,
          SCH_ffm_tbl_data->device_number );
  }
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);
  err = soc_mem_write(
          unit,
          SCH_FLOW_TO_FIP_MAPPING_FFMm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 31, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_ffm_tbl_set_unsafe()",0,0);
}
/*
 * Write indirect table scheduler_init_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */


uint32
  jer2_arad_sch_scheduler_init_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_SCHEDULER_INIT_TBL_DATA* SCH_scheduler_init_tbl_data
  )
{
  uint32 err, res, reg_val,i;
  uint32 data[JER2_ARAD_SCH_SCHEDULER_INIT_TBL_ENTRY_SIZE] = {0};
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_SCHEDULER_INIT_TBL_SET_UNSAFE);

  /* clear on set to all the sch memory error interrupts */  
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_SCH_PAR_ERR_INTERRUPT_REGISTERr(unit,  0xffffffff));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  31,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_SCH_ECC_1B_ERR_INTERRUPT_REGISTERr(unit,  0xffffffff));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  32,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_SCH_ECC_2B_ERR_INTERRUPT_REGISTERr(unit,  0xffffffff));

  soc_mem_field32_set(unit, SCH_SCHEDULER_INITm, data, SCH_INITf, SCH_scheduler_init_tbl_data->schinit);

  /* keep current timeout and set a new timeout needed for writing this special table */
  DNX_SAND_CHECK_FUNC_RESULT(READ_CMIC_SBUS_TIMEOUTr(unit, &reg_val), 10, exit);
  DNX_SAND_CHECK_FUNC_RESULT(WRITE_CMIC_SBUS_TIMEOUTr(unit, 0xffffffff), 20, exit);

   if (!SOC_IS_ARADPLUS(unit)) {
      /* write the table entry */
      err = WRITE_SCH_SCHEDULER_INITm(unit, MEM_BLOCK_ANY, entry_offset, data);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 40, exit);
   } else {
       
        for (i=0; i<5; i++) {
            err = WRITE_SCH_SCHEDULER_INITm(unit, MEM_BLOCK_ANY, entry_offset, data);
            if (SOC_E_NONE == err) {
                break;
            } else {
                sal_sleep(1);
            }
        }
        DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 41, exit);
   }
  
  /* restore original timeout */
  DNX_SAND_CHECK_FUNC_RESULT(WRITE_CMIC_SBUS_TIMEOUTr(unit, reg_val), 50, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_scheduler_init_tbl_set_unsafe()",0,0);
}

/*
 * Write indirect table force_status_message_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */

uint32
  jer2_arad_sch_force_status_message_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_FORCE_STATUS_MESSAGE_TBL_DATA* SCH_force_status_message_tbl_data
  )
{
  uint32
    err;
  uint32
    data[JER2_ARAD_SCH_FORCE_STATUS_MESSAGE_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_SCH_FORCE_STATUS_MESSAGE_TBL_SET_UNSAFE);

  err = dnx_sand_os_memset(
          &(data[0]),
          0x0,
          sizeof(data)
        );
  DNX_SAND_CHECK_FUNC_RESULT(err, 10, exit);

  soc_mem_field32_set(
          unit,
          SCH_FORCE_STATUS_MESSAGEm,
          data,
          MESSAGE_FLOW_IDf,
          SCH_force_status_message_tbl_data->message_flow_id );
  soc_mem_field32_set(
          unit,
          SCH_FORCE_STATUS_MESSAGEm,
          data,
          MESSAGE_TYPEf,
          SCH_force_status_message_tbl_data->message_type );
  err = soc_mem_write(
          unit,
          SCH_FORCE_STATUS_MESSAGEm,
          SCH_BLOCK(unit,core),
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 32, exit);
  

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_sch_force_status_message_tbl_set_unsafe()",0,0);
}


/*
 * Get the number of banks and number of Drams
 */
uint32
  jer2_arad_mmu_dram_address_space_info_get_unsafe(
    DNX_SAND_IN    int                               unit,
    DNX_SAND_OUT   uint32                                *nof_drams,
    DNX_SAND_OUT   uint32                                *nof_banks,
    DNX_SAND_OUT   uint32                                *nof_cols
  )
{
  uint32
    res;
  uint32
    reg_val,
    tmp_val;
   
    

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MMU_DRAM_ADDRESS_SPACE_INFO_GET_UNSAFE);


  
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_MMU_GENERAL_CONFIGURATION_REGISTER_2r(unit, &reg_val ));

  JER2_ARAD_FLD_FROM_REG(MMU_GENERAL_CONFIGURATION_REGISTER_2r, DRAM_NUMf, tmp_val, reg_val, 40, exit);

 /*
  * check IF DRAM ID in range.
  */
  switch(tmp_val) {
  case 0x3:
    *nof_drams = 2;
    break;
  case 0x2:
    *nof_drams = 3;
    break;
  case 0x1:
    *nof_drams = 4;
    break;
  case 0x0:
      *nof_drams = 6;
    break;
  case 0x4:
  default:
    *nof_drams = 8;
    break;
  }
 /*
  * check If BANK ID in range.
  */
  JER2_ARAD_FLD_FROM_REG(MMU_GENERAL_CONFIGURATION_REGISTERr, DRAM_BANK_NUMf, tmp_val, reg_val, 60, exit);
    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  switch(tmp_val)
  {
    case 0x0:
      *nof_banks = JER2_ARAD_DRAM_NUM_BANKS_4;
      break;
    case 0x1:
    default:
      *nof_banks = JER2_ARAD_DRAM_NUM_BANKS_8;
      break;
  }
#endif 
    

  JER2_ARAD_FLD_FROM_REG(MMU_GENERAL_CONFIGURATION_REGISTERr, DRAM_COL_NUMf, tmp_val, reg_val, 80, exit);

  switch(tmp_val) {
  case 0x0:
    *nof_cols = 256;
    break;
  case 0x1:
    *nof_cols = 512;
    break;
  case 0x2:
    *nof_cols = 1024;
    break;
  case 0x3:
    *nof_cols = 2048;
    break;
  case 0x4:
    *nof_cols = 4096;
    break;
  default:
    *nof_cols = 8192;
    break;
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mmu_dram_address_space_info_get()",0,0);
}

/* 
 *  TM-Tables - B
 */ 

/*
 * Read indirect table nif_port_to_ctxt_bit_map_tbl from block IRE,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ire_nif_port_to_ctxt_bit_map_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRE_NIF_PORT_TO_CTXT_BIT_MAP_TBL_DATA  *IRE_nif_port_to_ctxt_bit_map_tbl_data
  )
{
  uint32
    res, err = DNX_SAND_OK;
  uint32
    data[JER2_ARAD_IRE_NIF_PORT_TO_CTXT_BIT_MAP_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRE_NIF_PORT_TO_CTXT_BIT_MAP_TBL_GET_UNSAFE);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRE_NIF_PORT_TO_CTXT_BIT_MAP_TBL_ENTRY_SIZE);
  JER2_ARAD_CLEAR(IRE_nif_port_to_ctxt_bit_map_tbl_data, JER2_ARAD_IRE_NIF_PORT_TO_CTXT_BIT_MAP_TBL_DATA, 1);

  err = soc_mem_read(
          unit,
          IRE_NIF_PORT_TO_CTXT_BIT_MAPm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(err, 30, exit);

  soc_mem_field_get(
            unit,
            IRE_NIF_PORT_TO_CTXT_BIT_MAPm,
            data,
            CONTEXTS_BIT_MAPPINGf,
            (uint32*)&(IRE_nif_port_to_ctxt_bit_map_tbl_data->contexts_bit_mapping[0]) );  
  
  IRE_nif_port_to_ctxt_bit_map_tbl_data->parity   = soc_mem_field32_get(
                  unit,
                  IRE_NIF_PORT_TO_CTXT_BIT_MAPm,
                  data,
                  PARITYf            );
  

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ire_nif_port_to_ctxt_bit_map_tbl_get_unsafe()", entry_offset, 0);
}

/*
 * Write indirect table nif_port_to_ctxt_bit_map_tbl from block IRE,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ire_nif_port_to_ctxt_bit_map_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRE_NIF_PORT_TO_CTXT_BIT_MAP_TBL_DATA  *IRE_nif_port_to_ctxt_bit_map_tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[JER2_ARAD_IRE_NIF_PORT_TO_CTXT_BIT_MAP_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRE_NIF_PORT_TO_CTXT_BIT_MAP_TBL_SET_UNSAFE);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRE_NIF_PORT_TO_CTXT_BIT_MAP_TBL_ENTRY_SIZE);

 
            soc_mem_field_set(
          unit,
          IRE_NIF_PORT_TO_CTXT_BIT_MAPm,
          data,
          CONTEXTS_BIT_MAPPINGf,
          (uint32*)&(IRE_nif_port_to_ctxt_bit_map_tbl_data->contexts_bit_mapping[0]));
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  res = soc_mem_write(
          unit,
          IRE_NIF_PORT_TO_CTXT_BIT_MAPm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ire_nif_port_to_ctxt_bit_map_tbl_set_unsafe()", entry_offset, 0);
}

/*
 * Read indirect table tdm_config_tbl from block IRE,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ire_tdm_config_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRE_TDM_CONFIG_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[SOC_MAX_MEM_WORDS];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRE_TDM_CONFIG_TBL_GET_UNSAFE);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRE_TDM_CONFIG_TBL_ENTRY_SIZE);
  JER2_ARAD_CLEAR(tbl_data, JER2_ARAD_IRE_TDM_CONFIG_TBL_DATA, 1);

  res = soc_mem_read(
          unit,
          IRE_TDM_CONFIGm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

  tbl_data->mode = soc_mem_field32_get(
          unit,
          IRE_TDM_CONFIGm,
          data,
          MODEf );

  tbl_data->cpu   = soc_mem_field32_get(
          unit,
          IRE_TDM_CONFIGm,
          data,
          CPUf );

  soc_mem_field_get(
                  unit,
                  IRE_TDM_CONFIGm,
                  data,
                  HEADERf,
              tbl_data->header);

  
  if (!SOC_IS_QAX(unit)) {
      tbl_data->add_packet_crc  = soc_mem_field32_get(
                      unit,
                      IRE_TDM_CONFIGm,
                      data,
                      ADD_PACKET_CRCf );
  }

  if (SOC_IS_JERICHO(unit)) {
      tbl_data->mc_replication  = soc_mem_field32_get(
                          unit,
                          IRE_TDM_CONFIGm,
                          data,
                          MC_REPLICATIONf );
  }

  tbl_data->link_mask_ptr = soc_mem_field32_get(
                  unit,
                  IRE_TDM_CONFIGm,
                  data,
                  LINK_MASK_PTRf );
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ire_tdm_config_tbl_get_unsafe()", entry_offset, 0);
}

/*
 * Write indirect table tdm_config_tbl from block IRE,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ire_tdm_config_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRE_TDM_CONFIG_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[SOC_MAX_MEM_WORDS];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRE_TDM_CONFIG_TBL_SET_UNSAFE);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRE_TDM_CONFIG_TBL_ENTRY_SIZE);

      soc_mem_field32_set(
          unit,
          IRE_TDM_CONFIGm,
          data,
          MODEf,
          tbl_data->mode );

            soc_mem_field32_set(
          unit,
          IRE_TDM_CONFIGm,
          data,
          CPUf,
          tbl_data->cpu );

            soc_mem_field_set(
          unit,
          IRE_TDM_CONFIGm,
          data,
          HEADERf,
          (uint32*)&(tbl_data->header[0]));

  soc_mem_field32_set(
          unit,
          IRE_TDM_CONFIGm,
          data,
          LINK_MASK_PTRf,
          tbl_data->link_mask_ptr );

  
  if (!SOC_IS_QAX(unit)) {
       soc_mem_field32_set(
              unit,
              IRE_TDM_CONFIGm,
              data,
              ADD_PACKET_CRCf,
              tbl_data->add_packet_crc );
  }

  if (SOC_IS_JERICHO(unit)) {
      soc_mem_field32_set(
            unit,
            IRE_TDM_CONFIGm,
            data,
            MC_REPLICATIONf,
            tbl_data->mc_replication );
  }

  res = soc_mem_write(
          unit,
          IRE_TDM_CONFIGm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ire_tdm_config_tbl_set_unsafe()", entry_offset, 0);
}
/*
 * Read indirect table context_mru_tbl from block IDR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_idr_context_mru_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IDR_CONTEXT_MRU_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[JER2_ARAD_IDR_CONTEXT_MRU_TBL_ENTRY_SIZE];
 
 
 

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IDR_CONTEXT_MRU_TBL_GET_UNSAFE);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IDR_CONTEXT_MRU_TBL_ENTRY_SIZE);
  JER2_ARAD_CLEAR(tbl_data, JER2_ARAD_IDR_CONTEXT_MRU_TBL_DATA, 1);

 
 



  

  res = soc_mem_read(
          unit,
          IDR_CONTEXT_MRUm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

          tbl_data->org_size   = soc_mem_field32_get(
                  unit,
                  IDR_CONTEXT_MRUm ,
                  data,
                  MAX_ORG_SIZEf);
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

          tbl_data->size   = soc_mem_field32_get(
                  unit,
                  IDR_CONTEXT_MRUm,
                  data,
                  MAX_SIZEf);
  DNX_SAND_CHECK_FUNC_RESULT(res, 41, exit);

          tbl_data->parity   = soc_mem_field32_get(
                  unit,
                  IDR_CONTEXT_MRUm,
                  data,
                  PARITYf);
  DNX_SAND_CHECK_FUNC_RESULT(res, 42, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_idr_context_mru_tbl_get_unsafe()", entry_offset, 0);
}

/*
 * Write indirect table context_mru_tbl from block IDR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_idr_context_mru_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IDR_CONTEXT_MRU_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[JER2_ARAD_IDR_CONTEXT_MRU_TBL_ENTRY_SIZE];
 
 
 

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IDR_CONTEXT_MRU_TBL_SET_UNSAFE);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IDR_CONTEXT_MRU_TBL_ENTRY_SIZE);

 
 



  

            soc_mem_field32_set(
          unit,
          IDR_CONTEXT_MRUm,
          data,
          MAX_ORG_SIZEf,
          tbl_data->org_size );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

            soc_mem_field32_set(
          unit,
          IDR_CONTEXT_MRUm,
          data,
          MAX_SIZEf,
          tbl_data->size );
  DNX_SAND_CHECK_FUNC_RESULT(res, 31, exit);


  res = soc_mem_write(
          unit,
          IDR_CONTEXT_MRUm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 33, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_idr_context_mru_tbl_set_unsafe()", entry_offset, 0);
}

/*
 * Read indirect table destination_table_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_destination_table_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRR_DESTINATION_TABLE_TBL_DATA  *tbl_data
  )
{
  uint32 res = DNX_SAND_OK;
  soc_mem_t mem;
  soc_field_t valid_field;
  uint32 data[JER2_ARAD_IRR_DESTINATION_TABLE_TBL_ENTRY_SIZE];

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRR_DESTINATION_TABLE_TBL_GET_UNSAFE);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_DESTINATION_TABLE_TBL_ENTRY_SIZE);
  JER2_ARAD_CLEAR(tbl_data, JER2_ARAD_IRR_DESTINATION_TABLE_TBL_DATA, 1);

  if (SOC_IS_QAX(unit)) {
      mem = TAR_DESTINATION_TABLEm; 
      valid_field = VALIDf;
  } else {
      mem = IRR_DESTINATION_TABLEm; 
      valid_field = VALID_0f;
  }

  res = soc_mem_read(unit, mem, MEM_BLOCK_ANY, entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

  tbl_data->queue_number = soc_mem_field32_get(unit, mem, data, QUEUE_NUMBERf);
  tbl_data->tc_profile   = soc_mem_field32_get(unit, mem, data, TC_PROFILEf);

  if ((SOC_IS_JERICHO(unit))) {
      tbl_data->valid[0] = soc_mem_field32_get(unit, mem, data, valid_field);

      if (!SOC_DNX_CORE_MODE_IS_SINGLE_CORE(unit)) {
          tbl_data->valid[1] = soc_mem_field32_get(unit, mem, data, VALID_1f);
      } else {
          tbl_data->valid[1] = 0x0;
      }
  } else {
      tbl_data->valid[0] = 0x1;
      tbl_data->valid[1] = 0x0;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_irr_destination_table_tbl_get_unsafe()", entry_offset, 0);
}

/*
 * Write indirect table destination_table_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_destination_table_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRR_DESTINATION_TABLE_TBL_DATA  *tbl_data
  )
{
  uint32 res = DNX_SAND_OK;

  soc_mem_t mem;
  soc_field_t valid_field;
  uint32 data[JER2_ARAD_IRR_DESTINATION_TABLE_TBL_ENTRY_SIZE];

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRR_DESTINATION_TABLE_TBL_SET_UNSAFE);

  if (SOC_IS_QAX(unit)) {
      mem = TAR_DESTINATION_TABLEm; 
      valid_field = VALIDf;
  } else {
      mem = IRR_DESTINATION_TABLEm; 
      valid_field = VALID_0f;
  }

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_DESTINATION_TABLE_TBL_ENTRY_SIZE);

  soc_mem_field32_set(unit, mem, data, QUEUE_NUMBERf, tbl_data->queue_number);
  soc_mem_field32_set(unit, mem, data, TC_PROFILEf, tbl_data->tc_profile);

  if ((SOC_IS_JERICHO(unit))) {
      soc_mem_field32_set(unit, mem, data, valid_field, (uint32)tbl_data->valid[0]);
      if (!SOC_DNX_CORE_MODE_IS_SINGLE_CORE(unit)) {
          soc_mem_field32_set(unit, mem, data, VALID_1f, (uint32)tbl_data->valid[1]);
      }
  }

  res = soc_mem_write(unit, mem, MEM_BLOCK_ALL, entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_irr_destination_table_tbl_set_unsafe()", entry_offset, 0);
}
/*
 * Read indirect table lag_to_lag_range_tbl from block IRR,
 * doesn't take semaphore. 
 *  
 * Arad+: 
 *   Also read the (IHB)IPP_LAG_TO_LAG_RANGE table which is
 *   duplicated from the TM, with the Mode field changed to
 *   IsStateful (if 1 then the LAG is stateful).
 *   For get, we only use the IsStateful bit.
 *   Specifically, the Range and Parity fields are ignored.
 *  
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_lag_to_lag_range_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_DATA  *tbl_data
  )
{
  uint32 res = DNX_SAND_OK;
  uint32 data[JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_ENTRY_SIZE];
  soc_mem_t mem;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_GET_UNSAFE);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_ENTRY_SIZE);
  JER2_ARAD_CLEAR(tbl_data, JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_DATA, 1);

  mem = JER2_ARAD_PP_LAG_BY_CHIP(unit, LAG_TO_LAG_RANGE);

  res = soc_mem_read(unit, mem, MEM_BLOCK_ANY, entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  tbl_data->range   = soc_mem_field32_get(unit, mem, data, RANGEf);
  tbl_data->mode    = soc_mem_field32_get(unit, mem, data, MODEf);

#ifdef BCM_88660_A0
  if (SOC_IS_ARADPLUS(unit)) {
    JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_ENTRY_SIZE);

    res = READ_IHB_IPP_LAG_TO_LAG_RANGEm(unit, MEM_BLOCK_ANY, entry_offset, data);
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    tbl_data->is_stateful = soc_mem_field32_get(unit, IHB_IPP_LAG_TO_LAG_RANGEm, data, IS_STATEFULf);
  }  
#endif /* BCM_88660_A0 */

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_irr_lag_to_lag_range_tbl_get_unsafe()", entry_offset, 0);
}

/*
 * Write indirect table lag_to_lag_range_tbl from block IRR,
 * doesn't take semaphore. 
 * 
 * Arad+: 
 *   Also update the (IHB)IPP_LAG_TO_LAG_RANGE table which is
 *   duplicated from the TM, with the Mode field changed to
 *   IsStateful (if 1 then the LAG is stateful).
 *  
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_lag_to_lag_range_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_DATA  *tbl_data
  )
{
  uint32 res = DNX_SAND_OK;
  uint32 data[JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_ENTRY_SIZE];
  soc_mem_t mem;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_SET_UNSAFE);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_ENTRY_SIZE);

  mem = JER2_ARAD_PP_LAG_BY_CHIP(unit, LAG_TO_LAG_RANGE);

  soc_mem_field32_set(unit, mem, data, RANGEf, tbl_data->range);
  soc_mem_field32_set(unit, mem, data, MODEf, tbl_data->mode);

  res = soc_mem_write(unit, mem, MEM_BLOCK_ANY, entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

#ifdef BCM_88660_A0
  if (SOC_IS_ARADPLUS(unit)) {
    JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_ENTRY_SIZE);

    soc_mem_field32_set(unit, IHB_IPP_LAG_TO_LAG_RANGEm, data, RANGEf, tbl_data->range);
    soc_mem_field32_set(unit, IHB_IPP_LAG_TO_LAG_RANGEm, data, IS_STATEFULf, tbl_data->is_stateful);

    res = soc_mem_write(unit, IHB_IPP_LAG_TO_LAG_RANGEm, MEM_BLOCK_ANY, entry_offset, data);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
  }  
#endif /* BCM_88660_A0 */

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_irr_lag_to_lag_range_tbl_set_unsafe()", entry_offset, 0);
}
/*
 * Read indirect table lag_mapping_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_lag_mapping_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN   uint32              lag_ndx,
    DNX_SAND_IN   uint32              port_ndx,
    DNX_SAND_OUT JER2_ARAD_IRR_LAG_MAPPING_TBL_DATA  *tbl_data
  )
{
  uint32 entry_offset = 0,
  res = DNX_SAND_OK;
  uint32 data[JER2_ARAD_IRR_LAG_MAPPING_TBL_ENTRY_SIZE];
  soc_mem_t mem;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRR_LAG_MAPPING_TBL_GET_UNSAFE);
    
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  entry_offset = (lag_ndx * jer2_arad_ports_lag_nof_lag_entries_get_unsafe(unit)) | port_ndx;
#endif 

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_LAG_MAPPING_TBL_ENTRY_SIZE);
  JER2_ARAD_CLEAR(tbl_data, JER2_ARAD_IRR_LAG_MAPPING_TBL_DATA, 1);

  mem = JER2_ARAD_PP_LAG_BY_CHIP(unit, LAG_MAPPING);

  res = soc_mem_read(
          unit,
          mem,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

          tbl_data->destination   = soc_mem_field32_get(
                  unit,
                  mem,
                  data,
                  DESTINATIONf            );
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_irr_lag_mapping_tbl_get_unsafe()", entry_offset, 0);
}

/*
 * Write indirect table lag_mapping_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_lag_mapping_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN   uint32              lag_ndx,
    DNX_SAND_IN   uint32              port_ndx,
    DNX_SAND_IN  JER2_ARAD_IRR_LAG_MAPPING_TBL_DATA  *tbl_data
  )
{
  uint32 entry_offset = 0,
  res = DNX_SAND_OK;
  uint32 data[JER2_ARAD_IRR_LAG_MAPPING_TBL_ENTRY_SIZE];
  soc_mem_t mem;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRR_LAG_MAPPING_TBL_SET_UNSAFE);

    
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  entry_offset = (lag_ndx * jer2_arad_ports_lag_nof_lag_entries_get_unsafe(unit)) | port_ndx;
#endif 

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_LAG_MAPPING_TBL_ENTRY_SIZE);

  mem = JER2_ARAD_PP_LAG_BY_CHIP(unit, LAG_MAPPING);
 
   soc_mem_field32_set(
          unit,
          mem,
          data,
          DESTINATIONf,
          tbl_data->destination );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  res = soc_mem_write(
          unit,
          mem,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_irr_lag_mapping_tbl_set_unsafe()", entry_offset, 0);
}
/*
 * Read indirect table smooth_division_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_smooth_division_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN   uint32              lag_size,
    DNX_SAND_IN   uint32              hash_val,
    DNX_SAND_OUT JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_DATA  *tbl_data
  )
{
  uint32 res = DNX_SAND_OK;
  uint32 entry_offset,
         data[JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_ENTRY_SIZE];
  soc_mem_t mem;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_GET_UNSAFE);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_ENTRY_SIZE);
  JER2_ARAD_CLEAR(tbl_data, JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_DATA, 1);

  mem = JER2_ARAD_PP_LAG_BY_CHIP(unit, SMOOTH_DIVISION);

  entry_offset = lag_size << JER2_ARAD_IRR_GLAG_DEVISION_HASH_NOF_BITS;
  entry_offset |= hash_val;

  res = soc_mem_read(
          unit,
          mem,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

          tbl_data->member   = soc_mem_field32_get(
                  unit,
                  mem,
                  data,
                  MEMBER_0f            );
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_irr_smooth_division_tbl_get_unsafe()", lag_size, hash_val);
}

/*
 * Write indirect table smooth_division_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_smooth_division_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN   uint32              lag_size,
    DNX_SAND_IN   uint32              hash_val,
    DNX_SAND_IN  JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_DATA  *tbl_data
  )
{
  uint32 res = DNX_SAND_OK;
  uint32 entry_offset,
         data[JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_ENTRY_SIZE];
  soc_mem_t mem;
 
 
 DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_SET_UNSAFE);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_ENTRY_SIZE);

  mem = JER2_ARAD_PP_LAG_BY_CHIP(unit, SMOOTH_DIVISION);

 entry_offset = lag_size << (JER2_ARAD_IRR_GLAG_DEVISION_HASH_NOF_BITS-1);
 entry_offset |= (hash_val/2);

  res = soc_mem_read(
          unit,
          mem,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);

  if(hash_val%2 == 0) {
      soc_mem_field32_set(
              unit,
              mem,
              data,
              MEMBER_0f,
              tbl_data->member );
      DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  }
  else{
      soc_mem_field32_set(
              unit,
              mem,
              data,
              MEMBER_1f,
              tbl_data->member );
      DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  }

  res = soc_mem_write(
          unit,
          mem,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_irr_smooth_division_tbl_set_unsafe()", lag_size, hash_val);
}

/*
 * Read indirect table traffic_class_mapping_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_traffic_class_mapping_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  int             core_id,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRR_TRAFFIC_CLASS_MAPPING_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[JER2_ARAD_IRR_TRAFFIC_CLASS_MAPPING_TBL_ENTRY_SIZE];
  int32
    tc_ndx = 0;
  uint32
    tmp = 0;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRR_TRAFFIC_CLASS_MAPPING_TBL_GET_UNSAFE);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_TRAFFIC_CLASS_MAPPING_TBL_ENTRY_SIZE);
  JER2_ARAD_CLEAR(tbl_data, JER2_ARAD_IRR_TRAFFIC_CLASS_MAPPING_TBL_DATA, 1);
  if (SOC_IS_JERICHO(unit)) {
      if (core_id == 0 || core_id == SOC_CORE_ALL) {
          res = READ_IRR_TRAFFIC_CLASS_MAPPING_0m(unit, MEM_BLOCK_ANY, entry_offset, data);
          DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

          for (tc_ndx = 0; tc_ndx < JER2_ARAD_NOF_TRAFFIC_CLASSES; ++tc_ndx) {
              tmp = soc_mem_field32_get(unit, IRR_TRAFFIC_CLASS_MAPPING_0m, data, TCf);
              SHR_BITCOPY_RANGE(&(tbl_data->traffic_class_mapping[tc_ndx]), 0, &tmp, (tc_ndx*3), 3);
          }
      } else if (core_id == 1) {
          res = READ_IRR_TRAFFIC_CLASS_MAPPING_1m(unit, MEM_BLOCK_ANY, entry_offset, data);
          DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

          for (tc_ndx = 0; tc_ndx < JER2_ARAD_NOF_TRAFFIC_CLASSES; ++tc_ndx) {
              tmp = soc_mem_field32_get(unit, IRR_TRAFFIC_CLASS_MAPPING_1m, data, TCf);
              SHR_BITCOPY_RANGE(&(tbl_data->traffic_class_mapping[tc_ndx]), 0, &tmp, (tc_ndx*3), 3);
          }
      } 
  } else {
      res = READ_IRR_TRAFFIC_CLASS_MAPPINGm(unit, MEM_BLOCK_ANY, entry_offset, data);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

      for (tc_ndx = 0; tc_ndx < JER2_ARAD_NOF_TRAFFIC_CLASSES; ++tc_ndx) {
        tmp = soc_mem_field32_get(unit, IRR_TRAFFIC_CLASS_MAPPINGm, data, TRAFFIC_CLASS_MAPPINGf);
        SHR_BITCOPY_RANGE(&(tbl_data->traffic_class_mapping[tc_ndx]), 0, &tmp, (tc_ndx*3), 3);
      }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_irr_traffic_class_mapping_tbl_get_unsafe()", entry_offset, tc_ndx);
}

/*
 * Write indirect table traffic_class_mapping_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_traffic_class_mapping_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  int             core_id,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRR_TRAFFIC_CLASS_MAPPING_TBL_DATA  *tbl_data
  )
{
  uint32
    res = SOC_E_NONE,
    tmp = 0;
  uint32
    data[JER2_ARAD_IRR_TRAFFIC_CLASS_MAPPING_TBL_ENTRY_SIZE];
 int32
    tc_ndx = 0;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IRR_TRAFFIC_CLASS_MAPPING_TBL_SET_UNSAFE);
  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_TRAFFIC_CLASS_MAPPING_TBL_ENTRY_SIZE);
  for (tc_ndx = 0; tc_ndx < JER2_ARAD_NOF_TRAFFIC_CLASSES; ++tc_ndx) {
      SHR_BITCOPY_RANGE(&tmp,(tc_ndx*3),&(tbl_data->traffic_class_mapping[tc_ndx]),0,3);
  }
  if (SOC_IS_JERICHO(unit)) {
      if (core_id == 0 || core_id == SOC_CORE_ALL) {
          soc_mem_field32_set(unit, IRR_TRAFFIC_CLASS_MAPPING_0m, data, TCf, tmp);
          res = WRITE_IRR_TRAFFIC_CLASS_MAPPING_0m(unit, MEM_BLOCK_ANY, entry_offset, data);
          DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 11, exit);
      }
      if (core_id == 1 || core_id == SOC_CORE_ALL) {
          soc_mem_field32_set(unit, IRR_TRAFFIC_CLASS_MAPPING_1m, data, TCf, tmp);
          res = WRITE_IRR_TRAFFIC_CLASS_MAPPING_1m(unit, MEM_BLOCK_ANY, entry_offset, data);
          DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 21, exit);
      }
  } else {
      soc_mem_field32_set(unit, IRR_TRAFFIC_CLASS_MAPPINGm, data, TRAFFIC_CLASS_MAPPINGf, tmp);
      res = WRITE_IRR_TRAFFIC_CLASS_MAPPINGm(unit, MEM_BLOCK_ANY, entry_offset, data);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 31, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_irr_traffic_class_mapping_tbl_set_unsafe()", entry_offset, tc_ndx);
}

uint32
  jer2_arad_irr_stack_fec_resolve_table_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32             entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRR_STACK_FEC_RESOLVE_TBL_DATA  *tbl_data
  )
{
  uint32 res = DNX_SAND_OK;
  uint32 data[JER2_ARAD_IRR_STACK_FEC_RESOLVE_TBL_ENTRY_SIZE];

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_STACK_FEC_RESOLVE_TBL_ENTRY_SIZE);
  JER2_ARAD_CLEAR(tbl_data, JER2_ARAD_IRR_STACK_FEC_RESOLVE_TBL_DATA, 1);

  res = soc_mem_read(unit, IRR_STACK_FEC_RESOLVEm, MEM_BLOCK_ANY, entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

  tbl_data->stack_lag = soc_mem_field32_get(unit, IRR_STACK_FEC_RESOLVEm, data, STACK_LAGf);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_irr_stack_fec_resolve_table_tbl_get_unsafe()", entry_offset, 0);
}


uint32
  jer2_arad_irr_stack_fec_resolve_table_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRR_STACK_FEC_RESOLVE_TBL_DATA  *tbl_data
  )
{
  uint32 res = DNX_SAND_OK;
  uint32 data[JER2_ARAD_IRR_STACK_FEC_RESOLVE_TBL_ENTRY_SIZE];

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_STACK_FEC_RESOLVE_TBL_ENTRY_SIZE);

  soc_mem_field32_set(unit, IRR_STACK_FEC_RESOLVEm, data, STACK_LAGf, tbl_data->stack_lag);

  res = soc_mem_write(unit, IRR_STACK_FEC_RESOLVEm, MEM_BLOCK_ALL, entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_irr_stack_fec_resolve_table_tbl_set_unsafe()", entry_offset, 0);
}

uint32
  jer2_arad_irr_stack_trunk_resolve_table_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32             entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRR_STACK_TRUNK_RESOLVE_TBL_DATA  *tbl_data
  )
{
  uint32 res = DNX_SAND_OK;
  uint32 data[JER2_ARAD_IRR_STACK_TRUNK_RESOLVE_TBL_ENTRY_SIZE];

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_STACK_TRUNK_RESOLVE_TBL_ENTRY_SIZE);
  JER2_ARAD_CLEAR(tbl_data, JER2_ARAD_IRR_STACK_TRUNK_RESOLVE_TBL_DATA, 1);

  res = soc_mem_read(unit, IRR_STACK_TRUNK_RESOLVEm, MEM_BLOCK_ANY, entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

  tbl_data->base_queue = soc_mem_field32_get(unit, IRR_STACK_TRUNK_RESOLVEm, data, BASE_QUEUEf);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_irr_stack_trunk_resolve_table_tbl_get_unsafe()", entry_offset, 0);
}


uint32
  jer2_arad_irr_stack_trunk_resolve_table_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRR_STACK_TRUNK_RESOLVE_TBL_DATA  *tbl_data
  )
{
  uint32 res = DNX_SAND_OK;
  uint32 data[JER2_ARAD_IRR_STACK_TRUNK_RESOLVE_TBL_ENTRY_SIZE];

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_IRR_STACK_TRUNK_RESOLVE_TBL_ENTRY_SIZE);

  soc_mem_field32_set(unit, IRR_STACK_TRUNK_RESOLVEm, data, BASE_QUEUEf, tbl_data->base_queue);

  res = soc_mem_write(unit, IRR_STACK_TRUNK_RESOLVEm, MEM_BLOCK_ALL, entry_offset, data);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_irr_stack_trunk_resolve_table_tbl_set_unsafe()", entry_offset, 0);
}

/*
 * Read indirect table tm_port_sys_port_config_tbl from block IHP,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
int
  jer2_arad_ihp_tm_port_sys_port_config_tbl_get_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core_id,
    DNX_SAND_IN  uint32                 entry_offset,
    DNX_SAND_OUT JER2_ARAD_IHP_TM_PORT_SYS_PORT_CONFIG_TBL_DATA  *tbl_data
  )
{
    uint32 data[JER2_ARAD_IHP_TM_PORT_SYS_PORT_CONFIG_TBL_ENTRY_SIZE];
 
    DNXC_INIT_FUNC_DEFS;
 
    sal_memset(data, 0, sizeof(uint32)*JER2_ARAD_IHP_TM_PORT_SYS_PORT_CONFIG_TBL_ENTRY_SIZE);
    sal_memset(tbl_data, 0, sizeof(JER2_ARAD_IHP_TM_PORT_SYS_PORT_CONFIG_TBL_DATA));

    DNXC_IF_ERR_EXIT(soc_mem_read(unit, IHP_PTC_SYS_PORT_CONFIGm, IHP_BLOCK(unit, core_id), entry_offset, data));
    tbl_data->system_port_offset1 =         soc_mem_field32_get(unit, IHP_PTC_SYS_PORT_CONFIGm, data, OFFSETf);
    tbl_data->system_port_value =           soc_mem_field32_get(unit, IHP_PTC_SYS_PORT_CONFIGm, data, VALUEf);
    tbl_data->system_port_profile =         soc_mem_field32_get(unit, IHP_PTC_SYS_PORT_CONFIGm, data, PROFILEf);
    tbl_data->system_port_value_to_use =    soc_mem_field32_get(unit, IHP_PTC_SYS_PORT_CONFIGm, data, VALUE_TO_USEf);

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Write indirect table tm_port_sys_port_config_tbl from block IHP,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
int
  jer2_arad_ihp_tm_port_sys_port_config_tbl_set_unsafe(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core_id,
    DNX_SAND_IN  uint32             entry_offset,
    DNX_SAND_IN  JER2_ARAD_IHP_TM_PORT_SYS_PORT_CONFIG_TBL_DATA  *tbl_data
  )
{
    uint32 data[JER2_ARAD_IHP_TM_PORT_SYS_PORT_CONFIG_TBL_ENTRY_SIZE];
    DNXC_INIT_FUNC_DEFS;
 
    sal_memset(data, 0, sizeof(uint32)*JER2_ARAD_IHP_TM_PORT_SYS_PORT_CONFIG_TBL_ENTRY_SIZE);

    soc_mem_field32_set(unit, IHP_PTC_SYS_PORT_CONFIGm, data, OFFSETf, tbl_data->system_port_offset1);
    soc_mem_field32_set(unit, IHP_PTC_SYS_PORT_CONFIGm, data, VALUEf, tbl_data->system_port_value);
    soc_mem_field32_set(unit, IHP_PTC_SYS_PORT_CONFIGm, data, PROFILEf, tbl_data->system_port_profile);
    soc_mem_field32_set(unit, IHP_PTC_SYS_PORT_CONFIGm, data, VALUE_TO_USEf, tbl_data->system_port_value_to_use);

    DNXC_IF_ERR_EXIT(soc_mem_write(unit, IHP_PTC_SYS_PORT_CONFIGm, IHP_BLOCK(unit, core_id), entry_offset, data));

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Read indirect table pct_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
int
  jer2_arad_egq_pct_tbl_get_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core_id,
    DNX_SAND_IN  uint32                 q_pair,
    DNX_SAND_OUT JER2_ARAD_EGQ_PCT_TBL_DATA  *tbl_data
  )
{
    uint32 data[JER2_ARAD_EGQ_PCT_TBL_ENTRY_SIZE];

    DNXC_INIT_FUNC_DEFS;
 
    sal_memset(data, 0, sizeof(uint32)*JER2_ARAD_EGQ_PCT_TBL_ENTRY_SIZE);
    sal_memset(tbl_data, 0, sizeof(JER2_ARAD_EGQ_PCT_TBL_DATA));

    DNXC_IF_ERR_EXIT(soc_mem_read(
            unit,
            EGQ_PCTm,
            EGQ_BLOCK(unit, core_id),
            q_pair,
            data
    ));

    tbl_data->prog_editor_value = soc_mem_field32_get(unit, EGQ_PCTm, data, PRGE_VARf);
    tbl_data->prog_editor_value = soc_mem_field32_get(unit, EGQ_PCTm, data, PRGE_VARf);
    tbl_data->prog_editor_profile = soc_mem_field32_get(unit, EGQ_PCTm, data, PRGE_PROFILEf);
    /* tbl_data->outbound_mirr = soc_mem_field32_get(unit, EGQ_PCTm, data, OUTBOUND_MIRRf); */
    tbl_data->port_profile = soc_mem_field32_get(unit, EGQ_PCTm, data, CGM_PORT_PROFILEf);
    tbl_data->port_ch_num = soc_mem_field32_get(unit, EGQ_PCTm, data, PORT_CH_NUMf);
    tbl_data->port_type = soc_mem_field32_get(unit, EGQ_PCTm, data, PORT_TYPEf);
    tbl_data->cr_adjust_type = soc_mem_field32_get(unit, EGQ_PCTm, data, CR_ADJUST_TYPEf);
    if (soc_mem_field_valid(unit, EGQ_PCTm, EGRESS_TCf)){
        tbl_data->cos_map_profile = soc_mem_field32_get(unit, EGQ_PCTm, data, EGRESS_TCf);
    } else {
        tbl_data->cos_map_profile = soc_mem_field32_get(unit, EGQ_PCTm, data, COS_MAP_PROFILEf);
    }

    tbl_data->mirror_enable = soc_mem_field32_get(unit, EGQ_PCTm, data, MIRROR_ENABLEf);
    /* tbl_data->mirror_cmd = soc_mem_field32_get(unit, EGQ_PCTm, data, MIRROR_CMDf); */
    tbl_data->mirror_channel = soc_mem_field32_get(unit, EGQ_PCTm, data, MIRROR_CHANNELf);
    tbl_data->ecc = soc_mem_field32_get(unit, EGQ_PCTm, data, ECCf);

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Write indirect table pct_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
int
  jer2_arad_egq_pct_tbl_set_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                    core_id,
    DNX_SAND_IN  uint32                 q_pair,
    DNX_SAND_IN  JER2_ARAD_EGQ_PCT_TBL_DATA  *tbl_data
  )
{
    uint32 data[JER2_ARAD_EGQ_PCT_TBL_ENTRY_SIZE];
 
    DNXC_INIT_FUNC_DEFS;

    sal_memset(data, 0, sizeof(uint32)*JER2_ARAD_EGQ_PCT_TBL_ENTRY_SIZE);
    DNXC_IF_ERR_EXIT(soc_mem_read(
            unit,
            EGQ_PCTm,
            EGQ_BLOCK(unit, core_id),
            q_pair,
            data
    ));

    soc_mem_field32_set(unit, EGQ_PCTm, data, PRGE_VARf, tbl_data->prog_editor_value);
    soc_mem_field32_set(unit, EGQ_PCTm, data, PRGE_PROFILEf, tbl_data->prog_editor_profile);
    /* soc_mem_field32_set(unit, EGQ_PCTm, data, OUTBOUND_MIRRf, tbl_data->outbound_mirr); */
    soc_mem_field32_set(unit, EGQ_PCTm, data, CGM_PORT_PROFILEf, tbl_data->port_profile);
    soc_mem_field32_set(unit, EGQ_PCTm, data, PORT_CH_NUMf, tbl_data->port_ch_num);
    soc_mem_field32_set(unit, EGQ_PCTm, data, PORT_TYPEf, tbl_data->port_type);
    soc_mem_field32_set(unit, EGQ_PCTm, data, CR_ADJUST_TYPEf, tbl_data->cr_adjust_type);
    if (soc_mem_field_valid(unit, EGQ_PCTm, EGRESS_TCf)){
        soc_mem_field32_set(unit, EGQ_PCTm, data, EGRESS_TCf, tbl_data->cos_map_profile);
    } else {
        soc_mem_field32_set(unit, EGQ_PCTm, data, COS_MAP_PROFILEf, tbl_data->cos_map_profile);
    }
    soc_mem_field32_set(unit, EGQ_PCTm, data, MIRROR_ENABLEf, tbl_data->mirror_enable);
    /* soc_mem_field32_set(unit, EGQ_PCTm, data, MIRROR_CMDf, tbl_data->mirror_cmd); */
    soc_mem_field32_set(unit, EGQ_PCTm, data, MIRROR_CHANNELf, tbl_data->mirror_channel);
    soc_mem_field32_set(unit, EGQ_PCTm, data, ECCf, tbl_data->ecc);

    DNXC_IF_ERR_EXIT(soc_mem_write(
            unit,
            EGQ_PCTm,
            EGQ_BLOCK(unit, core_id),
            q_pair,
            data
    ));
exit:
    DNXC_FUNC_RETURN;
}
/*
 * Read indirect table tc_dp_map_table_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
static
  void
    jer2_arad_egq_tc_dp_map_table_entry_translate_unsafe(
      DNX_SAND_IN  int             unit,
      DNX_SAND_IN  JER2_ARAD_EGQ_TC_DP_MAP_TBL_ENTRY       *entry,
      DNX_SAND_OUT uint32                         *entry_offset
    )
{
  uint32
    offset = 0;

  offset |= DNX_SAND_SET_BITS_RANGE(entry->dp, 1, 0);
  offset |= DNX_SAND_SET_BITS_RANGE(entry->tc, 4, 2);  
  offset |= DNX_SAND_SET_BITS_RANGE(entry->is_egr_mc, 5, 5);
  offset |= DNX_SAND_SET_BITS_RANGE(entry->map_profile, 8, 6);

  *entry_offset = offset;
}

void
  JER2_ARAD_EGQ_TC_DP_MAP_TBL_ENTRY_clear(
    DNX_SAND_OUT JER2_ARAD_EGQ_TC_DP_MAP_TBL_ENTRY       *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(JER2_ARAD_EGQ_TC_DP_MAP_TBL_ENTRY));
  info->tc = 0;
  info->dp = 0;  
  info->is_egr_mc = 0;
  info->map_profile = 0;

  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

int
  jer2_arad_egq_tc_dp_map_tbl_get_unsafe(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  int                            core_id,
    DNX_SAND_IN  JER2_ARAD_EGQ_TC_DP_MAP_TBL_ENTRY   *entry,
    DNX_SAND_OUT JER2_ARAD_EGQ_TC_DP_MAP_TBL_DATA    *tbl_data
  )
{
    uint32 entry_offset = 0;
    uint32 data = 0;
    DNXC_INIT_FUNC_DEFS;
 
    sal_memset(tbl_data, 0, sizeof(JER2_ARAD_EGQ_TC_DP_MAP_TBL_DATA));

    jer2_arad_egq_tc_dp_map_table_entry_translate_unsafe(unit, entry, &entry_offset);

    DNXC_IF_ERR_EXIT(soc_mem_read(unit, EGQ_TC_DP_MAPm, EGQ_BLOCK(unit, core_id), entry_offset, &data));

    tbl_data->tc = soc_mem_field32_get(unit, EGQ_TC_DP_MAPm, &data, EGRESS_TCf);
    tbl_data->dp = soc_mem_field32_get(unit, EGQ_TC_DP_MAPm, &data, CGM_MC_DPf);

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Write indirect table tc_dp_map_table_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
int
  jer2_arad_egq_tc_dp_map_tbl_set_unsafe(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  int                            core_id,
    DNX_SAND_IN  JER2_ARAD_EGQ_TC_DP_MAP_TBL_ENTRY   *entry,
    DNX_SAND_IN  JER2_ARAD_EGQ_TC_DP_MAP_TBL_DATA    *tbl_data
  )
{
    uint32 entry_offset = 0;
    uint32 data = 0;

    DNXC_INIT_FUNC_DEFS;

    jer2_arad_egq_tc_dp_map_table_entry_translate_unsafe(unit, entry, &entry_offset);

    soc_mem_field32_set(unit, EGQ_TC_DP_MAPm, &data, EGRESS_TCf, tbl_data->tc);
    soc_mem_field32_set(unit, EGQ_TC_DP_MAPm, &data, CGM_MC_DPf, tbl_data->dp);

    DNXC_IF_ERR_EXIT(soc_mem_write(unit, EGQ_TC_DP_MAPm, EGQ_BLOCK(unit, core_id), entry_offset, &data));

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Write indirect table fqp_nif_port_mux_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
int
  jer2_arad_egq_fqp_nif_port_mux_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_EGQ_FQP_NIF_PORT_MUX_TBL_DATA  *tbl_data
  )
{
    uint32 data[JER2_ARAD_EGQ_FQP_NIF_PORT_MUX_TBL_ENTRY_SIZE];
    int rv;
    DNXC_INIT_FUNC_DEFS;

    sal_memset(data, 0, sizeof(uint32) * JER2_ARAD_EGQ_FQP_NIF_PORT_MUX_TBL_ENTRY_SIZE);
  
    soc_mem_field32_set(
          unit,
          EGQ_FQP_NIF_PORT_MUXm,
          data,
          FQP_NIF_PORT_MUXf,
          tbl_data->fqp_nif_port_mux);

    rv = soc_mem_write(
          unit,
          EGQ_FQP_NIF_PORT_MUXm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

uint32
  jer2_arad_egq_fqp_nif_port_mux_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_FQP_NIF_PORT_MUX_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[JER2_ARAD_EGQ_FQP_NIF_PORT_MUX_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_EGQ_FQP_NIF_PORT_MUX_TBL_ENTRY_SIZE);
  
    res = soc_mem_read(
          unit,
          EGQ_FQP_NIF_PORT_MUXm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

   tbl_data->fqp_nif_port_mux = soc_mem_field32_get(
          unit,
          EGQ_FQP_NIF_PORT_MUXm,
          data,
          FQP_NIF_PORT_MUXf);
  DNX_SAND_CHECK_FUNC_RESULT(res, 31, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egq_fqp_nif_port_mux_tbl_set_unsafe()", entry_offset, 0);
}


/*
 * Write indirect table fqp_nif_port_mux_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
int
  jer2_arad_egq_pqp_nif_port_mux_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_EGQ_PQP_NIF_PORT_MUX_TBL_DATA  *tbl_data
  )
{
    uint32 data[JER2_ARAD_EGQ_PQP_NIF_PORT_MUX_TBL_ENTRY_SIZE];
    int rv;
    DNXC_INIT_FUNC_DEFS;

    sal_memset(data, 0, sizeof(uint32)*JER2_ARAD_EGQ_PQP_NIF_PORT_MUX_TBL_ENTRY_SIZE);
  
    soc_mem_field32_set(
          unit,
          EGQ_PQP_NIF_PORT_MUXm,
          data,
          PQP_NIF_PORT_MUXf,
          tbl_data->pqp_nif_port_mux );

    rv = soc_mem_write(
          unit,
          EGQ_PQP_NIF_PORT_MUXm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

uint32
  jer2_arad_egq_pqp_nif_port_mux_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_PQP_NIF_PORT_MUX_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[JER2_ARAD_EGQ_PQP_NIF_PORT_MUX_TBL_ENTRY_SIZE];
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_EGQ_PQP_NIF_PORT_MUX_TBL_ENTRY_SIZE);
  
    res = soc_mem_read(
          unit,
          EGQ_PQP_NIF_PORT_MUXm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

   tbl_data->pqp_nif_port_mux = soc_mem_field32_get(
          unit,
          EGQ_PQP_NIF_PORT_MUXm,
          data,
          PQP_NIF_PORT_MUXf);
  DNX_SAND_CHECK_FUNC_RESULT(res, 31, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_egq_fqp_nif_port_mux_tbl_set_unsafe()", entry_offset, 0);
}

/*
 * Read indirect table calrx_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_oob_calrx_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID          if_ndx,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_CFC_CALRX_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[JER2_ARAD_CFC_CALRX_TBL_ENTRY_SIZE];
 
 uint32
   tbl;
 

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_CFC_CALRX_TBL_ENTRY_SIZE);
  JER2_ARAD_CLEAR(tbl_data, JER2_ARAD_CFC_CALRX_TBL_DATA, 1);

   tbl = (if_ndx == DNX_TMC_FC_OOB_ID_A ? CFC_SPI_OOB_RX_0_CALm : CFC_SPI_OOB_RX_1_CALm);
 



  

  res = soc_mem_read(
          unit,
          tbl,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

          tbl_data->fc_index   = soc_mem_field32_get(
                  unit,
                  tbl,
                  data,
                  FC_INDEXf            );
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

          tbl_data->fc_dest_sel   = soc_mem_field32_get(
                  unit,
                  tbl,
                  data,
                  FC_DST_SELf            );
  DNX_SAND_CHECK_FUNC_RESULT(res, 41, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_cfc_oob_calrx_tbl_get_unsafe()", entry_offset, 0);
}

/*
 * Write indirect table calrx_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_oob_calrx_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID          if_ndx,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_CFC_CALRX_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[JER2_ARAD_CFC_CALRX_TBL_ENTRY_SIZE];
 uint32
   tbl;
 
 

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_CFC_CALRX_TBL_ENTRY_SIZE);

  tbl = (if_ndx == DNX_TMC_FC_OOB_ID_A ? CFC_SPI_OOB_RX_0_CALm : CFC_SPI_OOB_RX_1_CALm);

 



  

            soc_mem_field32_set(
          unit,
          tbl,
          data,
          FC_INDEXf,
          tbl_data->fc_index );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

            soc_mem_field32_set(
          unit,
          tbl,
          data,
          FC_DST_SELf,
          tbl_data->fc_dest_sel );
  DNX_SAND_CHECK_FUNC_RESULT(res, 31, exit);

  res = soc_mem_write(
          unit,
          tbl,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_cfc_oob_calrx_tbl_set_unsafe()", entry_offset, 0);
}
/*
 * Read indirect table caltx_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_oob_caltx_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID                   if_ndx,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_CFC_CALTX_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[JER2_ARAD_CFC_CALTX_TBL_ENTRY_SIZE];
  uint32
    tbl;
 
 

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_CFC_CALTX_TBL_ENTRY_SIZE);
  JER2_ARAD_CLEAR(tbl_data, JER2_ARAD_CFC_CALTX_TBL_DATA, 1);

 
  tbl = (if_ndx == DNX_TMC_FC_OOB_ID_A ? CFC_SPI_OOB_TX_0_CALm : CFC_SPI_OOB_TX_1_CALm);




  

  res = soc_mem_read(
          unit,
          tbl,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);


          tbl_data->fc_index   = soc_mem_field32_get(
                  unit,
                  tbl,
                  data,
                  FC_INDEXf            );
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

          tbl_data->fc_source_sel   = soc_mem_field32_get(
                  unit,
                  tbl,
                  data,
                  FC_SRC_SELf            );
  DNX_SAND_CHECK_FUNC_RESULT(res, 41, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_cfc_oob_caltx_tbl_get_unsafe()", entry_offset, 0);
}

/*
 * Write indirect table caltx_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_oob_caltx_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID          if_ndx,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_CFC_CALTX_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[JER2_ARAD_CFC_CALTX_TBL_ENTRY_SIZE];
  uint32
    tbl;
 
 

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_CFC_CALTX_TBL_ENTRY_SIZE);

  tbl = (if_ndx == DNX_TMC_FC_OOB_ID_A ? CFC_SPI_OOB_TX_0_CALm : CFC_SPI_OOB_TX_1_CALm);

 



  

            soc_mem_field32_set(
          unit,
          tbl,
          data,
          FC_INDEXf,
          tbl_data->fc_index );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

            soc_mem_field32_set(
          unit,
          tbl,
          data,
          FC_SRC_SELf,
          tbl_data->fc_source_sel );
  DNX_SAND_CHECK_FUNC_RESULT(res, 31, exit);

  res = soc_mem_write(
          unit,
          tbl,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_cfc_oob_caltx_tbl_set_unsafe()", entry_offset, 0);
}

/*
 * Read indirect table ilkn_calrx_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_ilkn_calrx_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID          if_ndx,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_CFC_CALRX_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[JER2_ARAD_CFC_CALRX_TBL_ENTRY_SIZE];
  uint32
    tbl = 0;
 
 

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_CFC_CALRX_TBL_ENTRY_SIZE);
  JER2_ARAD_CLEAR(tbl_data, JER2_ARAD_CFC_CALRX_TBL_DATA, 1);

  tbl = (if_ndx == DNX_TMC_FC_OOB_ID_A ? CFC_ILKN_RX_0_CALm : CFC_ILKN_RX_1_CALm);
 



  

  res = soc_mem_read(
          unit,
          tbl,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

          tbl_data->fc_index   = soc_mem_field32_get(
                  unit,
                  tbl,
                  data,
                  FC_INDEXf            );
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

          tbl_data->fc_dest_sel   = soc_mem_field32_get(
                  unit,
                  tbl,
                  data,
                  FC_DST_SELf            );
  DNX_SAND_CHECK_FUNC_RESULT(res, 41, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_cfc_ilkn_calrx_tbl_get_unsafe()", entry_offset, 0);
}

uint32
  jer2_arad_cfc_hcfc_bitmap_tbl_set_unsafe(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32  entry_offset,
    DNX_SAND_IN  JER2_ARAD_CFC_HCFC_BITMAP_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK,
    i;
  uint32
    data[JER2_ARAD_CFC_HCFC_BITMAP_TBL_SIZE];

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  
  for(i=0; i<JER2_ARAD_CFC_HCFC_BITMAP_TBL_SIZE; i++)
  {
    data[i] = tbl_data->bitmap[i];
  }

  res = soc_mem_write(
          unit,
          CFC_CAT_2_TC_MAP_HCFCm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_cfc_hcfc_bitmap_tbl_set_unsafe()", entry_offset, 0);
}

uint32
  jer2_arad_cfc_hcfc_bitmap_tbl_get_unsafe(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32  entry_offset,
    DNX_SAND_OUT JER2_ARAD_CFC_HCFC_BITMAP_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK,
    i;
  uint32
    data[JER2_ARAD_CFC_HCFC_BITMAP_TBL_SIZE + 1];

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = dnx_sand_os_memset(
        data,
        0x0,
        sizeof(uint32) * JER2_ARAD_CFC_HCFC_BITMAP_TBL_SIZE
      );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
  
  res = soc_mem_read(
          unit,
          CFC_CAT_2_TC_MAP_HCFCm,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  for(i=0; i<JER2_ARAD_CFC_HCFC_BITMAP_TBL_SIZE; i++)
  {
    tbl_data->bitmap[i] = data[i];
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_cfc_hcfc_bitmap_tbl_get_unsafe()", entry_offset, 0);
}


/*
 * Write indirect table ilkn_calrx_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_ilkn_calrx_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID          if_ndx,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_CFC_CALRX_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[JER2_ARAD_CFC_CALRX_TBL_ENTRY_SIZE];
  uint32
    tbl = 0;
 
 

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_CFC_CALRX_TBL_ENTRY_SIZE);

 
  tbl = (if_ndx == DNX_TMC_FC_OOB_ID_A ? CFC_ILKN_RX_0_CALm : CFC_ILKN_RX_1_CALm);




  

            soc_mem_field32_set(
          unit,
          tbl,
          data,
          FC_INDEXf,
          tbl_data->fc_index );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

            soc_mem_field32_set(
          unit,
          tbl,
          data,
          FC_DST_SELf,
          tbl_data->fc_dest_sel );
  DNX_SAND_CHECK_FUNC_RESULT(res, 31, exit);

  /*
  soc_mem_field32_set(
    unit,
    tbl,
    data,
    PARITYf,
    tbl_data->parity );
  DNX_SAND_CHECK_FUNC_RESULT(res, 32, exit);
*/
  res = soc_mem_write(
          unit,
          tbl,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_cfc_ilkn_calrx_tbl_set_unsafe()", entry_offset, 0);
}
/*
 * Read indirect table ilkn_caltx_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_ilkn_caltx_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID          if_ndx,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_CFC_CALTX_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[JER2_ARAD_CFC_CALTX_TBL_ENTRY_SIZE];
 uint32
   tbl;
 
 

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_CFC_CALTX_TBL_ENTRY_SIZE);
  JER2_ARAD_CLEAR(tbl_data, JER2_ARAD_CFC_CALTX_TBL_DATA, 1);

  tbl = (if_ndx == DNX_TMC_FC_OOB_ID_A ? CFC_ILKN_TX_0_CALm : CFC_ILKN_TX_1_CALm); 



  

  res = soc_mem_read(
          unit,
          tbl,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

          tbl_data->fc_index   = soc_mem_field32_get(
                  unit,
                  tbl,
                  data,
                  FC_INDEXf            );
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

          tbl_data->fc_source_sel   = soc_mem_field32_get(
                  unit,
                  tbl,
                  data,
                  FC_SRC_SELf            );
  DNX_SAND_CHECK_FUNC_RESULT(res, 41, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_cfc_ilkn_caltx_tbl_get_unsafe()", entry_offset, 0);
}

/*
 * Write indirect table ilkn_caltx_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_ilkn_caltx_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID          if_ndx,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_CFC_CALTX_TBL_DATA  *tbl_data
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    data[JER2_ARAD_CFC_CALTX_TBL_ENTRY_SIZE];
 uint32
   tbl;
 
 

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  JER2_ARAD_CLEAR(data, uint32, JER2_ARAD_CFC_CALTX_TBL_ENTRY_SIZE);

  tbl = (if_ndx == DNX_TMC_FC_OOB_ID_A ? CFC_ILKN_TX_0_CALm : CFC_ILKN_TX_1_CALm); 

 



  

            soc_mem_field32_set(
          unit,
          tbl,
          data,
          FC_INDEXf,
          tbl_data->fc_index );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

            soc_mem_field32_set(
          unit,
          tbl,
          data,
          FC_SRC_SELf,
          tbl_data->fc_source_sel );
  DNX_SAND_CHECK_FUNC_RESULT(res, 31, exit);

  res = soc_mem_write(
          unit,
          tbl,
          MEM_BLOCK_ANY,
          entry_offset,
          data
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_cfc_ilkn_caltx_tbl_set_unsafe()", entry_offset, 0);
}

int jer2_arad_tbl_mem_cache_enable_parity_tbl(int unit, soc_mem_t mem, void* en)
{
    int rc = SOC_E_NONE;

  SOC_MEM_ALIAS_TO_ORIG(unit,mem);
    /* If table has valid PARITY field - it should be cached
     * Although the table SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC__SHDS has no PARITY field,
     * but it should be cached */
    if (SOC_MEM_FIELD_VALID(unit, mem, PARITYf) || 
        (SOC_IS_ARADPLUS_AND_BELOW(unit) && (mem == SCH_SHAPER_DESCRIPTOR_MEMORY_STATIC_SHDSm))) {
        rc = jer2_arad_tbl_mem_cache_mem_set(unit, mem, en);
    }

    LOG_INFO(BSL_LS_SOC_MEM, (BSL_META_U(unit, "parity memory %s cache\n"),SOC_MEM_NAME(unit, mem)));
    return rc;
}

int jer2_arad_tbl_mem_cache_enable_ecc_tbl(int unit, soc_mem_t mem, void* en)
{
    int rc = SOC_E_NONE;

  SOC_MEM_ALIAS_TO_ORIG(unit,mem);
    /* If table has valid ECC field - it should be cached */
    if (SOC_MEM_FIELD_VALID(unit, mem, ECCf))
        rc = jer2_arad_tbl_mem_cache_mem_set(unit, mem, en);

    LOG_INFO(BSL_LS_SOC_MEM, (BSL_META_U(unit, "ecc memory %s cache\n"),SOC_MEM_NAME(unit, mem)));
    return rc;
}

int jer2_arad_tbl_cache_enable_predefined_tbl(int unit)
{
    int i            = 0;
    int rc             = SOC_E_NONE;
    int enable         = 1;
    soc_mem_t mem     = INVALIDm;
    int size_of_predefined_array = sizeof(soc_jer2_arad_predefined_cached_mem_list)/sizeof(soc_mem_t) - 1 /* (-1) for the last element - NUM_SOC_MEM*/;

    for(i = 0; i < size_of_predefined_array; i++)
    {
        mem = soc_jer2_arad_predefined_cached_mem_list[i];

        if(!SOC_MEM_IS_VALID(unit, mem) || !soc_mem_is_cachable(unit, mem))
        {
            LOG_ERROR(BSL_LS_SOC_MEM, (BSL_META("unit %d cache failed for %d (%s) rv %d\n"), unit, mem, SOC_MEM_NAME(unit, mem), rc));
            return SOC_E_UNAVAIL;
        }
        rc = jer2_arad_tbl_mem_cache_mem_set(unit, mem, &enable);
    }
    return rc;
}

static int soc_mem_is_in_soc_property(int unit, soc_mem_t mem, int en)
{
    /*Check whether given 'mem' is defined in the soc_property configuration.*/
    char mem_name[SOC_PROPERTY_NAME_MAX];
    char *mptr;

    if (en == TRUE)    /* mem_cache_enable_specific_ */
    {
        sal_strncpy(mem_name, spn_MEM_CACHE_ENABLE, SOC_PROPERTY_NAME_MAX);
        mptr = &mem_name[sal_strlen(mem_name)];
        sal_strncpy(mptr, "_specific_", (SOC_PROPERTY_NAME_MAX - sal_strlen(mem_name)));
    }
    else            /* mem_nocache_ */
        sal_strncpy(mem_name, "mem_nocache_",SOC_PROPERTY_NAME_MAX);

    mptr = &mem_name[sal_strlen(mem_name)];
    sal_strncpy(mptr, SOC_MEM_NAME(unit, mem), (SOC_PROPERTY_NAME_MAX - sal_strlen(mem_name)));


    if (soc_property_get(unit, mem_name, 0))
    {
        LOG_ERROR(BSL_LS_APPL_TESTS, (BSL_META("unit %d memory %d (%s), soc_prop %s\n"), unit, mem, SOC_MEM_NAME(unit, mem), mem_name));
        return TRUE;
    }
    return FALSE;
}

int jer2_arad_tbl_mem_cache_enable_specific_tbl(int unit, soc_mem_t mem, void* en)
{
  int rc        = SOC_E_NONE;
    int cache_enable = *(int *)en;

    SOC_MEM_ALIAS_TO_ORIG(unit,mem);
    if (soc_mem_is_in_soc_property(unit, mem, cache_enable))
    {
        if(!SOC_MEM_IS_VALID(unit, mem) || !soc_mem_is_cachable(unit, mem))
        {
            LOG_ERROR(BSL_LS_SOC_MEM, (BSL_META("unit %d cache %s failed for %d (%s) rv %d\n"), unit, cache_enable ?"enable":"disable", mem, SOC_MEM_NAME(unit, mem), rc));
            return SOC_E_UNAVAIL;
        }
        rc = jer2_arad_tbl_mem_cache_mem_set(unit, mem, en);
    }

    return rc;
}

int jer2_arad_tbl_mem_cache_mem_set(int unit, soc_mem_t mem, void* en)
{
    int rc         = SOC_E_NONE;
    int enable     = *(int *)en;
    int dis        = 0;

  SOC_MEM_ALIAS_TO_ORIG(unit,mem);
    if(!SOC_MEM_IS_VALID(unit, mem) || !soc_mem_is_cachable(unit, mem))
    {
        return SOC_E_NONE;
    }

    /* Skip the "mem_nocache_" memories in soc_property */
    if (enable == TRUE)
    {
        if (soc_mem_is_in_soc_property(unit, mem, dis))
            return SOC_E_NONE;
    }

    /* Turn on cache memory for all tables */
    if (soc_mem_is_valid(unit, mem) &&
        ((SOC_MEM_INFO(unit, mem).blocks | SOC_MEM_INFO(unit, mem).blocks_hi) != 0))
    {
       rc = soc_mem_cache_set(unit, mem, COPYNO_ALL, enable);
    }
    else
    {
        rc = SOC_E_NONE;
    }

    return rc;
}
#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>
#endif /* of #if defined(BCM_88690_A0) */




