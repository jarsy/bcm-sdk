/* $Id: jer2_arad_tbl_access.h,v 1.100 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
 */


#ifndef __JER2_ARAD_TBL_ACCESS_H_INCLUDED__
/* { */
#define __JER2_ARAD_TBL_ACCESS_H_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Utils/sand_bitstream.h>
#include <soc/dnx/legacy/ARAD/arad_api_framework.h>
#include <soc/dnx/legacy/ARAD/arad_framework.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_mem_access.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_indirect_access.h>
#include <soc/dnx/legacy/ARAD/arad_chip_tbls.h>
#include <soc/dnx/legacy/ARAD/arad_api_flow_control.h>
#include <soc/dnx/legacy/ARAD/arad_ofp_rates.h>
#include <soc/dnx/legacy/dnx_config_defs.h>
#include <soc/dnx/legacy/dnx_config_imp_defs.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */
#define JER2_ARAD_OLP_PGE_MEM_TBL_ENTRY_SIZE                                                 1
#define JER2_ARAD_IRE_NIF_CTXT_MAP_TBL_ENTRY_SIZE                                            1
#define JER2_ARAD_IRE_NIF_PORT2CTXT_BIT_MAP_TBL_ENTRY_SIZE                                   4
#define JER2_ARAD_IRE_RCY_CTXT_MAP_TBL_ENTRY_SIZE                                            1
#define JER2_ARAD_IRE_PACKET_COUNTERS_TBL_ENTRY_SIZE                                         2
#define JER2_ARAD_IDR_COMPLETE_PC_TBL_ENTRY_SIZE                                             1
#define JER2_ARAD_IRR_SNOOP_TABLE_TBL_ENTRY_SIZE                                             1
#define JER2_ARAD_IRR_GLAG_TO_LAG_RANGE_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_ENTRY_SIZE                                         1
#define JER2_ARAD_IRR_GLAG_MAPPING_TBL_ENTRY_SIZE                                            1
#define JER2_ARAD_IRR_DESTINATION_TABLE_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_IRR_GLAG_NEXT_MEMBER_TBL_ENTRY_SIZE                                        1
#define JER2_ARAD_IRR_RLAG_NEXT_MEMBER_TBL_ENTRY_SIZE                                        1

/* 
 * Arad and Jericho share code, 
 * But they have different values in the mantissa exponent, for taildrop configuration
 * Moreover the CNRED/PQRED configuration are different in Jericho.
 */
#define JER2_ARAD_IQM_CNRED_PQ_MAX_QUE_SIZE_MANTISSA                                         6
#define JER2_ARAD_IQM_CNRED_PQ_MAX_QUE_BUFF_SIZE_MANTISSA                                    7
#define JER2_ARAD_IQM_PQRED_PQ_MAX_QUE_SIZE_MANTISSA                                         6
#define JER2_ARAD_IQM_PQRED_PQ_MAX_QUE_BUFF_SIZE_MANTISSA                                    7
#define JER2_JERICHO_IQM_CNRED_PQ_MAX_QUE_SIZE_MANTISSA                                      8
#define JER2_JERICHO_IQM_CNRED_PQ_MAX_QUE_BUFF_SIZE_MANTISSA                                 8
#define JER2_JERICHO_IQM_PQRED_PQ_MAX_QUE_SIZE_MANTISSA                                      8
#define JER2_JERICHO_IQM_PQRED_PQ_MAX_QUE_BUFF_SIZE_MANTISSA                                 8

#define JER2_ARAD_IQM_BDB_LINK_LIST_TBL_ENTRY_SIZE                                           1
#define JER2_ARAD_IQM_DYNAMIC_TBL_ENTRY_SIZE                                                 3
#define JER2_ARAD_IQM_STATIC_TBL_ENTRY_SIZE                                                  1
#define JER2_ARAD_IQM_PACKET_QUEUE_TAIL_POINTER_TBL_ENTRY_SIZE                               1
#define JER2_ARAD_IQM_PACKET_QUEUE_RED_WEIGHT_TABLE_TBL_ENTRY_SIZE                           1
#define JER2_ARAD_IQM_CREDIT_DISCOUNT_TABLE_TBL_ENTRY_SIZE                                   1
#define JER2_ARAD_IQM_FULL_USER_COUNT_MEMORY_TBL_ENTRY_SIZE                                  1
#define JER2_ARAD_IQM_MINI_MULTICAST_USER_COUNT_MEMORY_TBL_ENTRY_SIZE                        2
#define JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_ENTRY_SIZE                       4
#define JER2_ARAD_IQM_PACKET_DESCRIPTOR_FIFOS_MEMORY_TBL_ENTRY_SIZE                          1
#define JER2_ARAD_IQM_TX_DESCRIPTOR_FIFOS_MEMORY_TBL_ENTRY_SIZE                              3
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_A_TBL_ENTRY_SIZE                       1
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_B_TBL_ENTRY_SIZE                       1
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_C_TBL_ENTRY_SIZE                       1
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_D_TBL_ENTRY_SIZE                       1
#define JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_A_TBL_ENTRY_SIZE                                2
#define JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_B_TBL_ENTRY_SIZE                                2
#define JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_C_TBL_ENTRY_SIZE                                2
#define JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_D_TBL_ENTRY_SIZE                                2
#define JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_A_TBL_ENTRY_SIZE                        1
#define JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_B_TBL_ENTRY_SIZE                        1
#define JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_C_TBL_ENTRY_SIZE                        1
#define JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_D_TBL_ENTRY_SIZE                        1
#define JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_ENTRY_SIZE                 2
#define JER2_ARAD_IQM_SYSTEM_RED_PARAMETERS_TABLE_TBL_ENTRY_SIZE                             1
#define JER2_ARAD_IQM_SYSTEM_RED_DROP_PROBABILITY_VALUES_TBL_ENTRY_SIZE                      1
#define JER2_ARAD_IQM_SYSTEM_RED_TBL_ENTRY_SIZE                                              6
#define JER2_ARAD_QDR_MEM_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_IPS_SYSTEM_PHYSICAL_PORT_LOOKUP_TABLE_TBL_ENTRY_SIZE                       2
#define JER2_ARAD_IPS_DESTINATION_DEVICE_AND_PORT_LOOKUP_TABLE_TBL_ENTRY_SIZE                2
#define JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_ENTRY_SIZE                                    1
#define JER2_ARAD_IPS_QUEUE_TYPE_LOOKUP_TABLE_TBL_ENTRY_SIZE                                 1
#define JER2_ARAD_IPS_QUEUE_PRIORITY_MAP_SELECT_TBL_ENTRY_SIZE                               1
#define JER2_ARAD_IPS_QUEUE_PRIORITY_MAPS_TABLE_TBL_ENTRY_SIZE                               2
#define JER2_ARAD_IPS_QUEUE_SIZE_BASED_THRESHOLDS_TABLE_TBL_ENTRY_SIZE                       2
#define JER2_ARAD_IPS_SLOW_FACTOR_THRESHOLDS_TABLE_TBL_ENTRY_SIZE                            6
#define JER2_ARAD_IPS_CREDIT_BALANCE_BASED_THRESHOLDS_TABLE_TBL_ENTRY_SIZE                   2
#define JER2_ARAD_IPS_EMPTY_QUEUE_CREDIT_BALANCE_TABLE_TBL_ENTRY_SIZE                        2
#define JER2_ARAD_IPS_CREDIT_WATCHDOG_THRESHOLDS_TABLE_TBL_ENTRY_SIZE                        1
#define JER2_ARAD_IPS_QUEUE_DESCRIPTOR_TABLE_TBL_ENTRY_SIZE                                  1
#define JER2_ARAD_IPS_QUEUE_SIZE_TABLE_TBL_ENTRY_SIZE                                        1
#define JER2_ARAD_IPS_SYSTEM_RED_MAX_QUEUE_SIZE_TABLE_TBL_ENTRY_SIZE                         1
#define JER2_ARAD_IPT_BDQ_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_IPT_SELECT_SOURCE_SUM_TBL_ENTRY_SIZE                                       2
#define JER2_ARAD_IPT_PCQ_TBL_ENTRY_SIZE                                                     2
#define JER2_ARAD_IPT_SOP_MMU_TBL_ENTRY_SIZE                                                 2
#define JER2_ARAD_IPT_MOP_MMU_TBL_ENTRY_SIZE                                                 1
#define JER2_ARAD_IPT_FDTCTL_TBL_ENTRY_SIZE                                                  1
#define JER2_ARAD_IPT_FDTDATA_TBL_ENTRY_SIZE                                                 16
#define JER2_ARAD_IPT_EGQCTL_TBL_ENTRY_SIZE                                                  1
#define JER2_ARAD_IPT_EGQDATA_TBL_ENTRY_SIZE                                                 16
#define JER2_ARAD_DPI_DLL_RAM_TBL_ENTRY_SIZE                                                 2
#define JER2_ARAD_RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_DATA_CELLS_TBL_ENTRY_SIZE              3
#define JER2_ARAD_RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_CONTROL_CELLS_TBL_ENTRY_SIZE           3
#define JER2_ARAD_EGQ_NIF_SCM_TBL_ENTRY_SIZE                                                 2
#define JER2_ARAD_EGQ_NIFA_CH0_SCM_TBL_ENTRY_SIZE                                            1
#define JER2_ARAD_EGQ_NIFA_CH1_SCM_TBL_ENTRY_SIZE                                            1
#define JER2_ARAD_EGQ_NIFA_CH2_SCM_TBL_ENTRY_SIZE                                            1
#define JER2_ARAD_EGQ_NIFA_CH3_SCM_TBL_ENTRY_SIZE                                            1
#define JER2_ARAD_EGQ_NIFB_CH0_SCM_TBL_ENTRY_SIZE                                            1
#define JER2_ARAD_EGQ_NIFB_CH1_SCM_TBL_ENTRY_SIZE                                            1
#define JER2_ARAD_EGQ_NIFB_CH2_SCM_TBL_ENTRY_SIZE                                            1
#define JER2_ARAD_EGQ_NIFB_CH3_SCM_TBL_ENTRY_SIZE                                            1
#define JER2_ARAD_EGQ_NIFAB_NCH_SCM_TBL_ENTRY_SIZE                                           1
#define JER2_ARAD_EGQ_RCY_SCM_TBL_ENTRY_SIZE                                                 1
#define JER2_ARAD_EGQ_CPU_SCM_TBL_ENTRY_SIZE                                                 1
#define JER2_ARAD_EGQ_CCM_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_EGQ_PMC_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_EGQ_CBM_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_EGQ_FBM_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_EGQ_FDM_TBL_ENTRY_SIZE                                                     2
#define JER2_ARAD_EGQ_DWM_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_EGQ_RRDM_TBL_ENTRY_SIZE                                                    2
#define JER2_ARAD_EGQ_RPDM_TBL_ENTRY_SIZE                                                    2
#define JER2_ARAD_EGQ_PCT_TBL_ENTRY_SIZE                                                     3
#define JER2_ARAD_EGQ_PER_PORT_LB_RANGE_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_EGQ_CFC_FLOW_CONTROL_TBL_ENTRY_SIZE                                        3
#define JER2_ARAD_EGQ_NIFA_FLOW_CONTROL_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_EGQ_NIFB_FLOW_CONTROL_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_EGQ_CPU_LAST_HEADER_TBL_ENTRY_SIZE                                         2
#define JER2_ARAD_EGQ_IPT_LAST_HEADER_TBL_ENTRY_SIZE                                         2
#define JER2_ARAD_EGQ_FDR_LAST_HEADER_TBL_ENTRY_SIZE                                         2
#define JER2_ARAD_EGQ_CPU_PACKET_COUNTER_TBL_ENTRY_SIZE                                      2
#define JER2_ARAD_EGQ_IPT_PACKET_COUNTER_TBL_ENTRY_SIZE                                      2
#define JER2_ARAD_EGQ_FDR_PACKET_COUNTER_TBL_ENTRY_SIZE                                      2
#define JER2_ARAD_EGQ_RQP_PACKET_COUNTER_TBL_ENTRY_SIZE                                      2
#define JER2_ARAD_EGQ_RQP_DISCARD_PACKET_COUNTER_TBL_ENTRY_SIZE                              2
#define JER2_ARAD_EGQ_EHP_UNICAST_PACKET_COUNTER_TBL_ENTRY_SIZE                              2
#define JER2_ARAD_EGQ_EHP_MULTICAST_HIGH_PACKET_COUNTER_TBL_ENTRY_SIZE                       2
#define JER2_ARAD_EGQ_EHP_MULTICAST_LOW_PACKET_COUNTER_TBL_ENTRY_SIZE                        2
#define JER2_ARAD_EGQ_EHP_DISCARD_PACKET_COUNTER_TBL_ENTRY_SIZE                              2
#define JER2_ARAD_EGQ_PQP_UNICAST_HIGH_PACKET_COUNTER_TBL_ENTRY_SIZE                         2
#define JER2_ARAD_EGQ_PQP_UNICAST_LOW_PACKET_COUNTER_TBL_ENTRY_SIZE                          2
#define JER2_ARAD_EGQ_PQP_MULTICAST_HIGH_PACKET_COUNTER_TBL_ENTRY_SIZE                       2
#define JER2_ARAD_EGQ_PQP_MULTICAST_LOW_PACKET_COUNTER_TBL_ENTRY_SIZE                        2
#define JER2_ARAD_EGQ_PQP_UNICAST_HIGH_BYTES_COUNTER_TBL_ENTRY_SIZE                          2
#define JER2_ARAD_EGQ_PQP_UNICAST_LOW_BYTES_COUNTER_TBL_ENTRY_SIZE                           2
#define JER2_ARAD_EGQ_PQP_MULTICAST_HIGH_BYTES_COUNTER_TBL_ENTRY_SIZE                        2
#define JER2_ARAD_EGQ_PQP_MULTICAST_LOW_BYTES_COUNTER_TBL_ENTRY_SIZE                         2
#define JER2_ARAD_EGQ_PQP_DISCARD_UNICAST_PACKET_COUNTER_TBL_ENTRY_SIZE                      2
#define JER2_ARAD_EGQ_PQP_DISCARD_MULTICAST_PACKET_COUNTER_TBL_ENTRY_SIZE                    2
#define JER2_ARAD_EGQ_FQP_PACKET_COUNTER_TBL_ENTRY_SIZE                                      2
#define JER2_ARAD_EGQ_QDCT_TBL_ENTRY_SIZE                                                    5 /* to support jer2_jer*/
#define JER2_ARAD_EGQ_QQST_TBL_ENTRY_SIZE                                                    5 /* to support jer2_jer*/
#define JER2_ARAD_CFC_RECYCLE_TO_OUT_GOING_FAP_PORT_MAPPING_TBL_ENTRY_SIZE                   1
#define JER2_ARAD_CFC_NIF_A_CLASS_BASED_TO_OFP_MAPPING_TBL_ENTRY_SIZE                        1
#define JER2_ARAD_CFC_NIF_B_CLASS_BASED_TO_OFP_MAPPING_TBL_ENTRY_SIZE                        1
#define JER2_ARAD_CFC_A_SCHEDULERS_BASED_FLOW_CONTROL_TO_OFP_MAPPING_TBL_ENTRY_SIZE          1
#define JER2_ARAD_CFC_B_SCHEDULERS_BASED_FLOW_CONTROL_TO_OFP_MAPPING_TBL_ENTRY_SIZE          1
#define JER2_ARAD_CFC_OUT_OF_BAND_RX_A_CALENDAR_MAPPING_TBL_ENTRY_SIZE                       1
#define JER2_ARAD_CFC_OUT_OF_BAND_RX_B_CALENDAR_MAPPING_TBL_ENTRY_SIZE                       1
#define JER2_ARAD_CFC_OUT_OF_BAND_TX_CALENDAR_MAPPING_TBL_ENTRY_SIZE                         1
#define JER2_ARAD_CFC_HCFC_BITMAP_TBL_SIZE                                                   8
#define JER2_ARAD_SCH_CAL_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_SCH_PORT_NOF_PORTS_PER_ENPORT_TBL_LINE                                    (8)
#define JER2_ARAD_SCH_PORT_NOF_PORTS_PER_FORCE_FC_REG_LINE                                  (32)
#define JER2_ARAD_SCH_DRM_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_SCH_DSM_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_SCH_FDMS_TBL_ENTRY_SIZE                                                    2
#define JER2_ARAD_SCH_SHDS_TBL_ENTRY_SIZE                                                    2
#define JER2_JERICHO_SCH_TOKEN_MEMORY_CONTROLLER_TMC_TBL_ENTRY_SIZE                          1
#define JER2_ARAD_SCH_SEM_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_SCH_FSF_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_SCH_FGM_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_SCH_SHC_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_SCH_SCC_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_SCH_SCT_TBL_ENTRY_SIZE                                                     2
#define JER2_ARAD_SCH_FQM_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_SCH_FFM_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_SCH_TMC_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_SCH_PQS_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_SCH_SCHEDULER_INIT_TBL_ENTRY_SIZE                                          1
#define JER2_ARAD_SCH_FORCE_STATUS_MESSAGE_TBL_ENTRY_SIZE                                    1
#define JER2_ARAD_MMU_DRAM_ADDRESS_SPACE_TBL_ENTRY_SIZE                                      16
#define JER2_ARAD_MMU_IDF_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_MMU_FDF_TBL_ENTRY_SIZE                                                     1
#define JER2_ARAD_MMU_RDFA_WADDR_STATUS_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_MMU_RDFB_WADDR_STATUS_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_MMU_RDFC_WADDR_STATUS_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_MMU_RDFD_WADDR_STATUS_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_MMU_RDFE_WADDR_STATUS_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_MMU_RDFF_WADDR_STATUS_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_MMU_RDF_RADDR_TBL_ENTRY_SIZE                                               1
#define JER2_ARAD_MMU_WAF_HALFA_WADDR_TBL_ENTRY_SIZE                                         1
#define JER2_ARAD_MMU_WAF_HALFB_WADDR_TBL_ENTRY_SIZE                                         1
#define JER2_ARAD_MMU_WAFA_HALFA_RADDR_STATUS_TBL_ENTRY_SIZE                                 1
#define JER2_ARAD_MMU_WAFB_HALFA_RADDR_STATUS_TBL_ENTRY_SIZE                                 1
#define JER2_ARAD_MMU_WAFC_HALFA_RADDR_STATUS_TBL_ENTRY_SIZE                                 1
#define JER2_ARAD_MMU_WAFD_HALFA_RADDR_STATUS_TBL_ENTRY_SIZE                                 1
#define JER2_ARAD_MMU_WAFE_HALFA_RADDR_STATUS_TBL_ENTRY_SIZE                                 1
#define JER2_ARAD_MMU_WAFF_HALFA_RADDR_STATUS_TBL_ENTRY_SIZE                                 1
#define JER2_ARAD_MMU_WAFA_HALFB_RADDR_STATUS_TBL_ENTRY_SIZE                                 1
#define JER2_ARAD_MMU_WAFB_HALFB_RADDR_STATUS_TBL_ENTRY_SIZE                                 1
#define JER2_ARAD_MMU_WAFC_HALFB_RADDR_STATUS_TBL_ENTRY_SIZE                                 1
#define JER2_ARAD_MMU_WAFD_HALFB_RADDR_STATUS_TBL_ENTRY_SIZE                                 1
#define JER2_ARAD_MMU_WAFE_HALFB_RADDR_STATUS_TBL_ENTRY_SIZE                                 1
#define JER2_ARAD_MMU_WAFF_HALFB_RADDR_STATUS_TBL_ENTRY_SIZE                                 1
#define JER2_ARAD_MMU_RAF_WADDR_TBL_ENTRY_SIZE                                               1
#define JER2_ARAD_MMU_RAFA_RADDR_STATUS_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_MMU_RAFB_RADDR_STATUS_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_MMU_RAFC_RADDR_STATUS_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_MMU_RAFD_RADDR_STATUS_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_MMU_RAFE_RADDR_STATUS_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_MMU_RAFF_RADDR_STATUS_TBL_ENTRY_SIZE                                       1
#define JER2_ARAD_NUM_OF_INDIRECT_MODULES  21
/*
 *  Indication for invalid block ID
 */
#define JER2_ARAD_BLK_INVALID_ID JER2_ARAD_NUM_OF_INDIRECT_MODULES

#define JER2_ARAD_IRR_GLAG_DEVISION_HASH_NOF_BITS  8
/*
 * In the system red mechanism, we divide the queues to 16 ranges,
 * according to the thresholds. and each ranges receives a value of
 * 4 bit (0-15), this represents the queue in the system red test.
 */
#define JER2_ARAD_SYS_RED_NOF_Q_RNGS                                                         16
#define JER2_ARAD_SYS_RED_NOF_Q_RNGS_THS                                                     (JER2_ARAD_SYS_RED_NOF_Q_RNGS-1)

#define JER2_ARAD_IPS_NOF_QUEUE_PRIORITY_MAPS_TABLES 2

#define JER2_ARAD_MULT_ING_NOF_MC_IDS_PER_ENTRY_ING_REP_TBL 8
#define JER2_ARAD_NOF_CFC_NIF_CLS2OFP_MAP_FLDS        2
#define JER2_ARAD_TBL_FGM_NOF_GROUPS_ONE_LINE         8



#define JER2_ARAD_IRR_GLAG_DEVISION_HASH_NOF_BITS  8
#define JER2_ARAD_CNM_CP_SAMPLE_BASE_SIZE                    (8)

#define JER2_ARAD_EPNI_COPY_ENGINE_PROGRAM_NOF_INSTS         (2)

#define JER2_ARAD_EPNI_LFEM_FIELD_SELECT_MAP_NOF_FLDS        (3)

#define JER2_ARAD_CHIP_REGS_NOF_CNT_PROCESSOR_IDS            (2)

#define JER2_ARAD_EGQ_ID                                                         (JER2_ARAD_EGQ_ID)
#define JER2_ARAD_EPNI_ID                                                        (JER2_ARAD_EPNI_ID)


#define JER2_ARAD_MAX_NOF_REPS                                                   (0xfffffff) /* 7F's */
#define JER2_ARAD_FIRST_TBL_ENTRY                                                (0)
#define JER2_ARAD_SINGLE_OPERATION                                               (0)


#define JER2_ARAD_PORTS_FEM_PARSER_PROGRAM_POINTER_SIZE             (4)
#define JER2_ARAD_PORTS_FEM_PFQ_0_SIZE                              (3)
#define JER2_ARAD_PORTS_FEM_SYSTEM_PORT_SIZE                        (16)

/* Number of banks and lines */

/*
 * In the system red mechanism, we divide the queues to 16 ranges,
 * according to the thresholds. and each ranges receives a value of
 * 4 bit (0-15), this represents the queue in the system red test.
 */
#define JER2_ARAD_SYS_RED_NOF_Q_RNGS          16


#define JER2_QAX_SCH_FSF_OFFSET (2*1024)
#define JER2_QAX_SCH_SHDS_OFFSET (32*1024)
#define JER2_QAX_SCH_SCC_OFFSET (8*1024)
#define JER2_QAX_SCH_FQM_OFFSET (16*1024)
#define JER2_QAX_SCH_FFM_OFFSET (8*1024)
#define JER2_QAX_SCH_FDMS_OFFSET (64*1024)

#define QUX_SCH_FSF_OFFSET (3*1024)
#define QUX_SCH_SHDS_OFFSET (48*1024)
#define QUX_SCH_SCC_OFFSET (12*1024)
#define QUX_SCH_FQM_OFFSET (24*1024)
#define QUX_SCH_FDMS_OFFSET (96*1024)


/* definitions for direct base queue to modport mapping tables */
#define BASE_QUEUES_PER_QPM_2_ENTRY 4
#define BASE_QUEUES_PER_QPM_FAP_PORT_MSB_OFFSET 7
#define BASE_QUEUES_PER_QPM_FAP_PORT_NO_MSB_MASK 127

/* } */

/*************
 *  MACROS   *
 *************/
/* { */
/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */
typedef struct
{
  uint32 pge_mem;
} JER2_ARAD_OLP_PGE_MEM_TBL_DATA;

typedef struct
{
  uint32 contexts_bit_mapping[3];
} JER2_ARAD_IRE_NIF_PORT2CTXT_BIT_MAP_TBL_DATA;

typedef struct
{
  uint32 cpu_packet_counter[2];
} JER2_ARAD_IRE_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 pcb_pointer;
  uint32 count;
  uint32 ecc;
} JER2_ARAD_IDR_COMPLETE_PC_TBL_DATA;

typedef struct
{
  uint32 mirror_destination;
  uint32 is_queue_number;
  uint32 is_multicast;
  uint32 is_outbound_mirror;
  uint32 mirror_traffic_class;
  uint32 traffic_class_over_write;
  uint32 mirror_drop_precedence;
  uint32 drop_precedence_over_write;
} JER2_ARAD_IRR_MIRROR_TABLE_TBL_DATA;

typedef struct
{
  uint32 snoop_destination;
  uint32 is_queue_number;
  uint32 is_multicast;
  uint32 snoop_traffic_class;
  uint32 traffic_class_over_write;
  uint32 snoop_drop_precedence;
  uint32 drop_precedence_over_write;
} JER2_ARAD_IRR_SNOOP_TABLE_TBL_DATA;

typedef struct
{
  uint32 glag_to_lag_range;
} JER2_ARAD_IRR_GLAG_TO_LAG_RANGE_TBL_DATA;

typedef struct
{
  uint32 glag_mapping;
} JER2_ARAD_IRR_GLAG_MAPPING_TBL_DATA;

typedef struct
{
  uint32 offset;
  uint32 rr_lb_mode;
} JER2_ARAD_IRR_GLAG_NEXT_MEMBER_TBL_DATA;

typedef struct
{
  uint32 two_lsb;
  uint32 two_msb;
} JER2_ARAD_IRR_RLAG_NEXT_MEMBER_TBL_DATA;


typedef struct
{
  uint32 bdb_link_list;
} JER2_ARAD_IQM_BDB_LINK_LIST_TBL_DATA;

typedef struct
{
  uint32 pq_head_ptr;
  uint32 que_not_empty;
  uint32 pq_inst_que_size;
  uint32 pq_avrg_szie;
  uint32 pq_inst_que_buff_size;
} JER2_ARAD_IQM_DYNAMIC_TBL_DATA;

typedef struct
{
  uint32 credit_class;
  uint32 rate_class;
  uint32 connection_class;
  uint32 traffic_class;
  uint32 que_signature;
} JER2_ARAD_IQM_STATIC_TBL_DATA;

typedef struct
{
  uint32 tail_ptr;
} JER2_ARAD_IQM_PACKET_QUEUE_TAIL_POINTER_TBL_DATA;

typedef struct
{
  uint32 pq_weight;
  uint32 avrg_en;
} JER2_ARAD_IQM_PACKET_QUEUE_RED_WEIGHT_TABLE_TBL_DATA;

typedef struct
{
  uint32 crdt_disc_val;
  uint32 discnt_sign;
} JER2_ARAD_IQM_CREDIT_DISCOUNT_TABLE_TBL_DATA;

typedef struct
{
  uint32 flus_cnt;
} JER2_ARAD_IQM_FULL_USER_COUNT_MEMORY_TBL_DATA;

typedef struct
{
  uint32 mn_us_cnt[2];
} JER2_ARAD_IQM_MINI_MULTICAST_USER_COUNT_MEMORY_TBL_DATA;

typedef struct
{
  uint32 pq_max_que_size_mnt;
  uint32 pq_max_que_size_exp;
  uint32 pq_max_que_size_bds_mnt;
  uint32 pq_max_que_size_bds_exp;
  uint32 pq_wred_en;
  uint32 pq_c2;
  uint32 pq_c3;
  uint32 pq_c1;
  uint32 pq_avrg_max_th;
  uint32 pq_avrg_min_th;
  uint32 pq_wred_pckt_sz_ignr;
  uint32 addmit_logic;
} JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_DATA;

typedef struct
{
  uint32 tx_pd;
} JER2_ARAD_IQM_PACKET_DESCRIPTOR_FIFOS_MEMORY_TBL_DATA;

typedef struct
{
  uint32 tx_dscr[3];
} JER2_ARAD_IQM_TX_DESCRIPTOR_FIFOS_MEMORY_TBL_DATA;

typedef struct
{
  uint32 vsq_rc_a;
} JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_A_TBL_DATA;

typedef struct
{
  uint32 vsq_rc_b;
} JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_B_TBL_DATA;

typedef struct
{
  uint32 vsq_rc_c;
} JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_C_TBL_DATA;

typedef struct
{
  uint32 vsq_rc_d;
} JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_D_TBL_DATA;

typedef struct
{
  uint32 vsq_size_wrds;
  uint32 vsq_size_bds;
} JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_A_TBL_DATA;

typedef struct
{
  uint32 vsq_size_wrds;
  uint32 vsq_size_bds;
} JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_B_TBL_DATA;

typedef struct
{
  uint32 vsq_size_wrds;
  uint32 vsq_size_bds;
} JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_C_TBL_DATA;

typedef struct
{
  uint32 vsq_size_wrds;
  uint32 vsq_size_bds;
} JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_D_TBL_DATA;

typedef struct
{
  uint32 vsq_avrg_size;
} JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_A_TBL_DATA;

typedef struct
{
  uint32 vsq_avrg_size;
} JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_B_TBL_DATA;

typedef struct
{
  uint32 vsq_avrg_size;
} JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_C_TBL_DATA;

typedef struct
{
  uint32 vsq_avrg_size;
} JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_D_TBL_DATA;

typedef struct
{
  uint32 wred_en;
  uint32 avrg_size_en;
  uint32 red_weight_q;
  uint32 set_threshold_words_mnt;
  uint32 set_threshold_words_exp;
  uint32 clear_threshold_words_mnt;
  uint32 clear_threshold_words_exp;
  uint32 set_threshold_bd_mnt;
  uint32 set_threshold_bd_exp;
  uint32 clear_threshold_bd_mnt;
  uint32 clear_threshold_bd_exp;
} JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_DATA;

#define JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA DNX_TMC_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA

typedef struct
{
  uint32 sys_red_en;
  uint32 adm_th;
  uint32 prob_th;
  uint32 drp_th;
  uint32 drp_prob_indx1;
  uint32 drp_prob_indx2;
} JER2_ARAD_IQM_SYSTEM_RED_PARAMETERS_TABLE_TBL_DATA;

typedef struct
{
  uint32 drp_prob;
} JER2_ARAD_IQM_SYSTEM_RED_DROP_PROBABILITY_VALUES_TBL_DATA;

typedef struct
{
  uint32 qsz_rng_th[JER2_ARAD_SYS_RED_NOF_Q_RNGS_THS];
} JER2_ARAD_IQM_SYSTEM_RED_TBL_DATA;

typedef struct
{
  uint32 data;
} JER2_ARAD_QDR_MEM_TBL_DATA;

typedef struct
{
  uint32 sys_phy_port;
} JER2_ARAD_BASE_Q_TO_SYS_PORT_TABLE_TBL_DATA;

typedef struct
{
  uint16 fap_id;
  uint16 fap_port_id;
} JER2_ARAD_BASE_Q_TO_MOD_PORT_TABLE_TBL_DATA;

typedef struct
{
  uint32 dest_port;
  uint32 dest_dev;
} JER2_ARAD_MOD_PORT_TBL_DATA;

typedef struct
{
  uint32 base_flow;
  uint32 sub_flow_mode;
} JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_DATA;

typedef struct
{
  uint32 queue_type_lookup_table;
} JER2_ARAD_IPS_QUEUE_TYPE_LOOKUP_TABLE_TBL_DATA;

typedef struct
{
  uint32 queue_priority_map_select;
} JER2_ARAD_IPS_QUEUE_PRIORITY_MAP_SELECT_TBL_DATA;

typedef struct
{
  uint32 queue_priority_maps_table[JER2_ARAD_IPS_NOF_QUEUE_PRIORITY_MAPS_TABLES];
} JER2_ARAD_IPS_QUEUE_PRIORITY_MAPS_TABLE_TBL_DATA;

typedef struct
{
  uint32 off_to_slow_msg_th;
  uint32 off_to_norm_msg_th;
  uint32 slow_to_norm_msg_th;
  uint32 norm_to_slow_msg_th;
  uint32 fsm_th_mul;
} JER2_ARAD_IPS_QUEUE_SIZE_BASED_THRESHOLDS_TABLE_TBL_DATA;

typedef struct
{
  uint32 backoff_enter_qcr_bal_th;
  uint32 backoff_exit_qcr_bal_th;
  uint32 backlog_enter_qcr_bal_th;
  uint32 backlog_exit_qcr_bal_th;
  uint32 backslow_enter_qcr_bal_th;
  uint32 backslow_exit_qcr_bal_th;
} JER2_ARAD_IPS_CREDIT_BALANCE_BASED_THRESHOLDS_TABLE_TBL_DATA;

typedef struct
{
  uint32 empty_qsatisfied_cr_bal;
  uint32 max_empty_qcr_bal;
  uint32 exceed_max_empty_qcr_bal;
} JER2_ARAD_IPS_EMPTY_QUEUE_CREDIT_BALANCE_TABLE_TBL_DATA;

typedef struct
{
  uint32 wd_status_msg_gen_period;
  uint32 wd_delete_qth;
} JER2_ARAD_IPS_CREDIT_WATCHDOG_THRESHOLDS_TABLE_TBL_DATA;

typedef struct
{
  uint32 cr_bal;
  uint32 crs;
  uint32 one_pkt_deq;
  uint32 wd_last_cr_time;
  uint32 in_dqcq;
  uint32 wd_delete;
  uint32 fsmrq_ctrl;
} JER2_ARAD_IPS_QUEUE_DESCRIPTOR_TABLE_TBL_DATA;

typedef struct
{
  uint32 exponent;
  uint32 mantissa;
  uint32 qsize_4b;
} JER2_ARAD_IPS_QUEUE_SIZE_TABLE_TBL_DATA;

typedef struct
{
  uint32 maxqsz;
  uint32 maxqsz_age;
} JER2_ARAD_IPS_SYSTEM_RED_MAX_QUEUE_SIZE_TABLE_TBL_DATA;
typedef struct
{
  uint32 flow_id;
  uint32 dest_pipe;
  uint32 flow_status;
  uint32 dest_port;
  uint32 queue_size_4b;
  uint32 queue_number;
  uint32 orig_fs;
  uint32 disable_timer;
  uint32 valid;
} JER2_ARAD_IPS_FMS_MSG_MEM_TABLE_TBL_DATA;
typedef struct
{
  uint32 bdq;
} JER2_ARAD_IPT_BDQ_TBL_DATA;

typedef struct
{
  uint32 pcq[2];
} JER2_ARAD_IPT_PCQ_TBL_DATA;

typedef struct
{
  uint32 mop_mmu;
} JER2_ARAD_IPT_MOP_MMU_TBL_DATA;

typedef struct
{
  uint32 fdtctl;
} JER2_ARAD_IPT_FDTCTL_TBL_DATA;

typedef struct
{
  uint32 fdtdata[16];
} JER2_ARAD_IPT_FDTDATA_TBL_DATA;

typedef struct
{
  uint32 egqctl;
} JER2_ARAD_IPT_EGQCTL_TBL_DATA;

typedef struct
{
  uint32 egqdata[16];
} JER2_ARAD_IPT_EGQDATA_TBL_DATA;

typedef struct
{
  uint32 dll_ram[2];
} JER2_ARAD_DPI_DLL_RAM_TBL_DATA;

typedef struct
{
  uint32 unicast_distribution_memory_for_data_cells[3];
} JER2_ARAD_RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_DATA_CELLS_TBL_DATA;

typedef struct
{
  uint32 unicast_distribution_memory_for_control_cells[3];
} JER2_ARAD_RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_CONTROL_CELLS_TBL_DATA;

typedef struct
{
  uint32 port_cr_to_add;
  uint32 ofp_index;
} JER2_ARAD_EGQ_SCM_TBL_DATA;

typedef struct
{
  uint32 port_cr_to_add;
  uint32 ofp_index;
} JER2_ARAD_EGQ_NIFA_CH0_SCM_TBL_DATA;

typedef struct
{
  uint32 port_cr_to_add;
  uint32 ofp_index;
} JER2_ARAD_EGQ_NIFA_CH1_SCM_TBL_DATA;

typedef struct
{
  uint32 port_cr_to_add;
  uint32 ofp_index;
} JER2_ARAD_EGQ_NIFA_CH2_SCM_TBL_DATA;

typedef struct
{
  uint32 port_cr_to_add;
  uint32 ofp_index;
} JER2_ARAD_EGQ_NIFA_CH3_SCM_TBL_DATA;

typedef struct
{
  uint32 port_cr_to_add;
  uint32 ofp_index;
} JER2_ARAD_EGQ_NIFB_CH0_SCM_TBL_DATA;

typedef struct
{
  uint32 port_cr_to_add;
  uint32 ofp_index;
} JER2_ARAD_EGQ_NIFB_CH1_SCM_TBL_DATA;

typedef struct
{
  uint32 port_cr_to_add;
  uint32 ofp_index;
} JER2_ARAD_EGQ_NIFB_CH2_SCM_TBL_DATA;

typedef struct
{
  uint32 port_cr_to_add;
  uint32 ofp_index;
} JER2_ARAD_EGQ_NIFB_CH3_SCM_TBL_DATA;

typedef struct
{
  uint32 port_cr_to_add;
  uint32 ofp_index;
} JER2_ARAD_EGQ_NIFAB_NCH_SCM_TBL_DATA;

typedef struct
{
  uint32 port_cr_to_add;
  uint32 ofp_index;
} JER2_ARAD_EGQ_CPU_SCM_TBL_DATA;

typedef struct
{
  uint32 port_cr_to_add;
  uint32 ofp_index;
} JER2_ARAD_EGQ_RCY_SCM_TBL_DATA;

typedef struct
{
  uint32 interface_select;
} JER2_ARAD_EGQ_CCM_TBL_DATA;

typedef struct
{
  uint32 port_max_credit;
} JER2_ARAD_EGQ_PMC_TBL_DATA;

typedef struct
{
  uint32 cbm;
} JER2_ARAD_EGQ_CBM_TBL_DATA;

typedef struct
{
  uint32 free_buffer_memory;
} JER2_ARAD_EGQ_FBM_TBL_DATA;

typedef struct
{
  uint32 free_descriptor_memory[2];
} JER2_ARAD_EGQ_FDM_TBL_DATA;

typedef struct
{
  uint32 mc_or_mc_low_queue_weight;
  uint32 uc_or_uc_low_queue_weight;
} JER2_ARAD_EGQ_DWM_TBL_DATA;

typedef struct
{
  uint32 crcremainder;
  uint32 reas_state;
  uint32 eopfrag_num;
  uint32 nxt_frag_number;
  uint32 stored_seg_size;
  uint32 fix129;
} JER2_ARAD_EGQ_RRDM_TBL_DATA;

typedef struct
{
  uint32 packet_start_buffer_pointer;
  uint32 packet_buffer_write_pointer;
  uint32 reas_state;
  uint32 packet_frag_cnt;
} JER2_ARAD_EGQ_RPDM_TBL_DATA;


typedef struct
{
  /*Each bit in this array represents a port of the OFPs(0-255)*/
  uint32 vlan_membership[8];
} JER2_ARAD_EGQ_VLAN_TABLE_TBL_DATA; 

typedef struct
{
  uint32 cfc_flow_control[3];
} JER2_ARAD_EGQ_CFC_FLOW_CONTROL_TBL_DATA;

typedef struct
{
  uint32 nifa_flow_control;
} JER2_ARAD_EGQ_NIFA_FLOW_CONTROL_TBL_DATA;

typedef struct
{
  uint32 nifb_flow_control;
} JER2_ARAD_EGQ_NIFB_FLOW_CONTROL_TBL_DATA;

typedef struct
{
  uint32 cpu_last_header[2];
} JER2_ARAD_EGQ_CPU_LAST_HEADER_TBL_DATA;

typedef struct
{
  uint32 ipt_last_header[2];
} JER2_ARAD_EGQ_IPT_LAST_HEADER_TBL_DATA;

typedef struct
{
  uint32 fdr_last_header[2];
} JER2_ARAD_EGQ_FDR_LAST_HEADER_TBL_DATA;

typedef struct
{
  uint32 cpu_packet_counter[2];
} JER2_ARAD_EGQ_CPU_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 ipt_packet_counter[2];
} JER2_ARAD_EGQ_IPT_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 fdr_packet_counter[2];
} JER2_ARAD_EGQ_FDR_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 rqp_packet_counter[2];
} JER2_ARAD_EGQ_RQP_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 rqp_discard_packet_counter[2];
} JER2_ARAD_EGQ_RQP_DISCARD_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 ehp_unicast_packet_counter[2];
} JER2_ARAD_EGQ_EHP_UNICAST_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 ehp_multicast_high_packet_counter[2];
} JER2_ARAD_EGQ_EHP_MULTICAST_HIGH_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 ehp_multicast_low_packet_counter[2];
} JER2_ARAD_EGQ_EHP_MULTICAST_LOW_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 ehp_discard_packet_counter[2];
} JER2_ARAD_EGQ_EHP_DISCARD_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 pqp_unicast_high_packet_counter[2];
} JER2_ARAD_EGQ_PQP_UNICAST_HIGH_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 pqp_unicast_low_packet_counter[2];
} JER2_ARAD_EGQ_PQP_UNICAST_LOW_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 pqp_multicast_high_packet_counter[2];
} JER2_ARAD_EGQ_PQP_MULTICAST_HIGH_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 pqp_multicast_low_packet_counter[2];
} JER2_ARAD_EGQ_PQP_MULTICAST_LOW_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 pqp_unicast_high_bytes_counter[2];
} JER2_ARAD_EGQ_PQP_UNICAST_HIGH_BYTES_COUNTER_TBL_DATA;

typedef struct
{
  uint32 pqp_unicast_low_bytes_counter[2];
} JER2_ARAD_EGQ_PQP_UNICAST_LOW_BYTES_COUNTER_TBL_DATA;

typedef struct
{
  uint32 pqp_multicast_high_bytes_counter[2];
} JER2_ARAD_EGQ_PQP_MULTICAST_HIGH_BYTES_COUNTER_TBL_DATA;

typedef struct
{
  uint32 pqp_multicast_low_bytes_counter[2];
} JER2_ARAD_EGQ_PQP_MULTICAST_LOW_BYTES_COUNTER_TBL_DATA;

typedef struct
{
  uint32 pqp_discard_unicast_packet_counter[2];
} JER2_ARAD_EGQ_PQP_DISCARD_UNICAST_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 pqp_discard_multicast_packet_counter[2];
} JER2_ARAD_EGQ_PQP_DISCARD_MULTICAST_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 fqp_packet_counter[2];
} JER2_ARAD_EGQ_FQP_PACKET_COUNTER_TBL_DATA;

typedef struct
{
  uint32 egq_ofp_num;
} JER2_ARAD_CFC_RECYCLE_TO_OUT_GOING_FAP_PORT_MAPPING_TBL_DATA;

typedef struct
{
  uint32 egq_ofp_num[JER2_ARAD_NOF_CFC_NIF_CLS2OFP_MAP_FLDS];
} JER2_ARAD_CFC_NIF_CLASS_BASED_TO_OFP_MAPPING_TBL_DATA;

typedef struct
{
  uint32 ofp_hr;
  uint32 lp_ofp_valid;
  uint32 hp_ofp_valid;
} JER2_ARAD_CFC_SCHEDULERS_BASED_FLOW_CONTROL_TO_OFP_MAPPING_TBL_DATA;

typedef struct
{
  uint32 fc_index;
  uint32 fc_dest_sel;
} JER2_ARAD_CFC_OUT_OF_BAND_RX_CALENDAR_MAPPING_TBL_DATA;

typedef struct
{
  uint32 fc_index;
  uint32 fc_source_sel;
} JER2_ARAD_CFC_OUT_OF_BAND_TX_CALENDAR_MAPPING_TBL_DATA;

typedef struct
{
  uint32 device_rate;
} JER2_ARAD_SCH_DRM_TBL_DATA;

typedef struct
{
  uint32 dual_shaper_ena;
} JER2_ARAD_SCH_DSM_TBL_DATA;

typedef struct
{
  uint32 sch_number;
  uint32 cos;
  uint32 hrsel_dual;
} JER2_ARAD_SCH_FDMS_TBL_DATA;

typedef struct
{
  uint32 peak_rate_man_even;
  uint32 peak_rate_exp_even;
  uint32 max_burst_even;
  uint32 slow_rate2_sel_even;
  uint32 peak_rate_man_odd;
  uint32 peak_rate_exp_odd;
  uint32 max_burst_odd;
  uint32 slow_rate2_sel_odd;
  uint32 max_burst_update_even;
  uint32 max_burst_update_odd;
} JER2_ARAD_SCH_SHDS_TBL_DATA;

typedef struct
{
  uint32 sch_enable;
} JER2_ARAD_SCH_SEM_TBL_DATA;

typedef struct
{
  uint32 sfenable;
} JER2_ARAD_SCH_FSF_TBL_DATA;

typedef struct
{
  uint32 flow_group[JER2_ARAD_TBL_FGM_NOF_GROUPS_ONE_LINE];
} JER2_ARAD_SCH_FGM_TBL_DATA;

typedef struct
{
  uint32 hrmode;
  uint32 hrmask_type;
} JER2_ARAD_SCH_SHC_TBL_DATA;

typedef struct
{
  uint32 clsch_type;
} JER2_ARAD_SCH_SCC_TBL_DATA;

typedef struct
{
  uint32 clconfig;
  uint32 af0_inv_weight;
  uint32 af1_inv_weight;
  uint32 af2_inv_weight;
  uint32 af3_inv_weight;
  uint32 wfqmode;
  uint32 enh_clen;
  uint32 enh_clsphigh;
} JER2_ARAD_SCH_SCT_TBL_DATA;

typedef struct
{
  uint32 base_queue_num;
  uint32 sub_flow_mode;
  uint32 flow_slow_enable;
} JER2_ARAD_SCH_FQM_TBL_DATA;

typedef struct
{
  uint32 device_number;
} JER2_ARAD_SCH_FFM_TBL_DATA;

typedef struct
{
  uint32 token_count;
  uint32 slow_status;
} JER2_ARAD_SCH_TMC_TBL_DATA;

typedef struct
{
  uint32 max_qsz;
  uint32 flow_id;
  uint32 aging_bit;
} JER2_ARAD_SCH_PQS_TBL_DATA;

typedef struct
{
  uint32 schinit;
} JER2_ARAD_SCH_SCHEDULER_INIT_TBL_DATA;

typedef struct
{
  uint32 message_flow_id;
  uint32 message_type;
} JER2_ARAD_SCH_FORCE_STATUS_MESSAGE_TBL_DATA;
typedef struct
{
  uint32 data[16];
} JER2_ARAD_MMU_DRAM_ADDRESS_SPACE_TBL_DATA;

typedef struct
{
  uint32 data;
} JER2_ARAD_MMU_IDF_TBL_DATA;

typedef struct
{
  uint32 data;
} JER2_ARAD_MMU_FDF_TBL_DATA;

typedef struct
{
  uint32 waddr;
  uint32 status;
} JER2_ARAD_MMU_RDFA_WADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 waddr;
  uint32 status;
} JER2_ARAD_MMU_RDFB_WADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 waddr;
  uint32 status;
} JER2_ARAD_MMU_RDFC_WADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 waddr;
  uint32 status;
} JER2_ARAD_MMU_RDFD_WADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 waddr;
  uint32 status;
} JER2_ARAD_MMU_RDFE_WADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 waddr;
  uint32 status;
} JER2_ARAD_MMU_RDFF_WADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr;
} JER2_ARAD_MMU_RDF_RADDR_TBL_DATA;

typedef struct
{
  uint32 waddr_half_a;
} JER2_ARAD_MMU_WAF_HALFA_WADDR_TBL_DATA;

typedef struct
{
  uint32 waddr_half_b;
} JER2_ARAD_MMU_WAF_HALFB_WADDR_TBL_DATA;

typedef struct
{
  uint32 raddr_half_a;
  uint32 status_half_a;
} JER2_ARAD_MMU_WAFA_HALFA_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr_half_a;
  uint32 status_half_a;
} JER2_ARAD_MMU_WAFB_HALFA_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr_half_a;
  uint32 status_half_a;
} JER2_ARAD_MMU_WAFC_HALFA_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr_half_a;
  uint32 status_half_a;
} JER2_ARAD_MMU_WAFD_HALFA_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr_half_a;
  uint32 status_half_a;
} JER2_ARAD_MMU_WAFE_HALFA_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr_half_a;
  uint32 status_half_a;
} JER2_ARAD_MMU_WAFF_HALFA_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr_half_b;
  uint32 status_half_b;
} JER2_ARAD_MMU_WAFA_HALFB_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr_half_b;
  uint32 status_half_b;
} JER2_ARAD_MMU_WAFB_HALFB_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr_half_b;
  uint32 status_half_b;
} JER2_ARAD_MMU_WAFC_HALFB_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr_half_b;
  uint32 status_half_b;
} JER2_ARAD_MMU_WAFD_HALFB_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr_half_b;
  uint32 status_half_b;
} JER2_ARAD_MMU_WAFE_HALFB_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr_half_b;
  uint32 status_half_b;
} JER2_ARAD_MMU_WAFF_HALFB_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 waddr;
} JER2_ARAD_MMU_RAF_WADDR_TBL_DATA;

typedef struct
{
  uint32 raddr;
  uint32 status;
} JER2_ARAD_MMU_RAFA_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr;
  uint32 status;
} JER2_ARAD_MMU_RAFB_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr;
  uint32 status;
} JER2_ARAD_MMU_RAFC_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr;
  uint32 status;
} JER2_ARAD_MMU_RAFD_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr;
  uint32 status;
} JER2_ARAD_MMU_RAFE_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 raddr;
  uint32 status;
} JER2_ARAD_MMU_RAFF_RADDR_STATUS_TBL_DATA;

typedef struct
{
  uint32 select_source_sum[JER2_ARAD_IPT_SELECT_SOURCE_SUM_TBL_ENTRY_SIZE];
} JER2_ARAD_IPT_SELECT_SOURCE_SUM_TBL_DATA;

typedef struct
{
  uint32 reassembly_context;
  uint32 port_termination_context;
  uint32 parity;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRE_NIF_CTXT_MAP_TBL_DATA;

typedef struct
{
  uint32 contexts_bit_mapping[6];
  uint32 parity;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRE_NIF_PORT_TO_CTXT_BIT_MAP_TBL_DATA;

typedef struct
{
  uint32 reassembly_context;
  uint32 port_termination_context;
  uint32 parity;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRE_RCY_CTXT_MAP_TBL_DATA;

typedef struct
{
  uint32 reassembly_context;
  uint32 port_termination_context;
  uint32 parity;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRE_CPU_CTXT_MAP_TBL_DATA;

typedef struct
{
  uint32 mode;
  uint32 cpu;
  uint32 header[3];
  uint32 link_mask_ptr;
  uint32 add_packet_crc;
  uint32 parity;
  uint32 mc_replication;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRE_TDM_CONFIG_TBL_DATA;

typedef struct
{
  uint32 org_size;
  uint32 size;
  uint32 parity;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IDR_CONTEXT_MRU_TBL_DATA;


typedef struct
{
  uint32 time_stamp;
  uint32 level;
  uint32 reserved;
  uint32 parity;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IDR_ETHERNET_METER_STATUS_TBL_DATA;

typedef struct
{
  uint32 time_stamp;
  uint32 level;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IDR_GLOBAL_METER_STATUS_TBL_DATA;


typedef struct
{
  uint32 destination;
  uint32 tc;
  uint32 tc_ow;
  uint32 dp;
  uint32 dp_ow;
  uint32 parity;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_SNOOP_MIRROR_TABLE0_TBL_DATA;

typedef struct
{
  uint32 meter_ptr0;
  uint32 meter_ptr0_ow;
  uint32 meter_ptr1;
  uint32 meter_ptr1_ow;
  uint32 counter_ptr0;
  uint32 counter_ptr0_ow;
  uint32 counter_ptr1;
  uint32 counter_ptr1_ow;
  uint32 dp_cmd;
  uint32 dp_cmd_ow;
  uint32 parity;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_SNOOP_MIRROR_TABLE1_TBL_DATA;

typedef struct
{
  uint32 ecc;
  uint32 pcp;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_FREE_PCB_MEMORY_TBL_DATA;

typedef struct
{
  uint32 ecc;
  uint32 pcp;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_PCB_LINK_TABLE_TBL_DATA;

typedef struct
{
  uint32 ecc;
  uint32 is_pcp;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_IS_FREE_PCB_MEMORY_TBL_DATA;

typedef struct
{
  uint32 ecc;
  uint32 is_pcp;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_IS_PCB_LINK_TABLE_TBL_DATA;

typedef struct
{
  uint32 ecc;
  uint32 pointer;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_RPF_MEMORY_TBL_DATA;

typedef struct
{
  uint32 descriptor[3];
  uint32 ecc;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_MCR_MEMORY_TBL_DATA;

typedef struct
{
  uint32 descriptor[2];
  uint32 ecc;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_ISF_MEMORY_TBL_DATA;

typedef struct
{
  uint32 queue_number;
  uint32 queue_valid;
  uint32 tc_profile;
  uint8 valid[SOC_DNX_DEFS_MAX(NOF_CORES)];
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_DESTINATION_TABLE_TBL_DATA;

typedef struct
{
  uint32 range;
  uint32 mode;
  uint32 parity;
#ifdef BCM_88660_A0
  uint8 is_stateful;
#endif /* BCM_88660_A0 */
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_DATA;
/* This struct really affects two tables (one was "copied" from TM to PP): [IRR] Lag To Lag Range and [IHB] IPP Lag To Lag Range*/

typedef struct
{
  uint32 destination;
  uint32 parity;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_LAG_MAPPING_TBL_DATA;

typedef struct
{
  uint32 offset;
  uint32 parity;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_LAG_NEXT_MEMBER_TBL_DATA;

typedef struct
{
  uint32 member;
  uint32 parity;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_DATA;

typedef struct
{
  /*
   *	For each old traffic class, mapping to a new traffic class.
   */
  uint32 traffic_class_mapping[JER2_ARAD_NOF_TRAFFIC_CLASSES];
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_TRAFFIC_CLASS_MAPPING_TBL_DATA;

typedef struct
{
  uint32 stack_lag;
  uint32 parity;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_STACK_FEC_RESOLVE_TBL_DATA;

typedef struct
{
  uint32 base_queue;
  uint32 parity;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IRR_STACK_TRUNK_RESOLVE_TBL_DATA;

typedef struct
{
  uint32 qdr_dll_mem;
} __ATTRIBUTE_PACKED__ JER2_ARAD_QDR_QDR_DLL_MEM_TBL_DATA;


typedef struct
{
  uint32 cp_enable;
  uint32 cp_class;
  uint32 cp_id;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IQM_CNM_DESCRIPTOR_STATIC_TBL_DATA;

typedef struct
{
  uint32 cp_enqued1;
  uint32 cp_qsize_old;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IQM_CNM_DESCRIPTOR_DYNAMIC_TBL_DATA;

typedef struct
{
  uint32 cp_qeq;
  uint32 cp_w;
  uint32 cp_fb_max_val;
  uint32 cp_quant_div;
  uint32 cp_sample_base[JER2_ARAD_CNM_CP_SAMPLE_BASE_SIZE];
  uint32 cp_fixed_sample_base;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IQM_CNM_PARAMETERS_TABLE_TBL_DATA;

typedef struct
{
  uint32 cp_enable_1;
  uint32 cp_profile_1;
  uint32 cp_enable_2;
  uint32 cp_profile_2;
  uint32 cp_id;
}__ATTRIBUTE_PACKED__ JER2_ARAD_IQM_CNM_DS_TBL_DATA;

typedef struct
{
  uint32 iqm_dp;
  uint32 etm_de;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IQM_METER_PROCESSOR_RESULT_RESOLVE_TABLE_STATIC_TBL_DATA;

typedef struct
{
  uint32 time_stamp;
  uint32 cbl;
  uint32 ebl;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IQM_NORMAL_DYNAMICA_TBL_DATA;

typedef struct
{
  uint32 time_stamp;
  uint32 cbl;
  uint32 ebl;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IQM_NORMAL_DYNAMICB_TBL_DATA;

typedef struct
{
  uint32 time_stamp;
  uint32 cbl;
  uint32 ebl;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IQM_HIGH_DYNAMICA_TBL_DATA;

typedef struct
{
  uint32 time_stamp;
  uint32 cbl;
  uint32 ebl;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IQM_HIGH_DYNAMICB_TBL_DATA;



typedef struct
{
  uint32 packets_counter;
  uint32 octets_counter;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IQM_CNTS_MEM_TBL_DATA;

typedef struct
{
  uint32 ovth_counter_bits[2];
} __ATTRIBUTE_PACKED__ JER2_ARAD_IQM_OVTH_MEMA_TBL_DATA;

typedef struct
{
  uint32 ovth_counter_bits[2];
} __ATTRIBUTE_PACKED__ JER2_ARAD_IQM_OVTH_MEMB_TBL_DATA;

typedef struct
{
  uint32 sop_mmu[3];
} __ATTRIBUTE_PACKED__ JER2_ARAD_IPT_SOP_MMU_TBL_DATA;


typedef struct
{
  uint32 cfg_byte_cnt[2];
} __ATTRIBUTE_PACKED__ JER2_ARAD_IPT_CFG_BYTE_CNT_TBL_DATA;

typedef struct
{
  uint32 egq_txq_wr_addr;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IPT_EGQ_TXQ_WR_ADDR_TBL_DATA;

typedef struct
{
  uint32 egq_txq_rd_addr;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IPT_EGQ_TXQ_RD_ADDR_TBL_DATA;

typedef struct
{
  uint32 dtq0_wr_addr;
  uint32 dtq1_wr_addr;
  uint32 dtq2_wr_addr;
  uint32 dtq3_wr_addr;
  uint32 dtq4_wr_addr;
  uint32 dtq5_wr_addr;
  uint32 dtq6_wr_addr;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IPT_FDT_TXQ_WR_ADDR_TBL_DATA;

typedef struct
{
  uint32 dtq0_rd_addr;
  uint32 dtq1_rd_addr;
  uint32 dtq2_rd_addr;
  uint32 dtq3_rd_addr;
  uint32 dtq4_rd_addr;
  uint32 dtq5_rd_addr;
  uint32 dtq6_rd_addr;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IPT_FDT_TXQ_RD_ADDR_TBL_DATA;

typedef struct
{
  uint32 mask0;
  uint32 mask1;
  uint32 mask2;
  uint32 mask3;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IPT_GCI_BACKOFF_MASK_TBL_DATA;


typedef struct
{
  uint32 ipt_contro_l_fifo[2];
} __ATTRIBUTE_PACKED__ JER2_ARAD_FDT_IPT_CONTRO_L_FIFO_TBL_DATA;

typedef struct
{
  uint32 read_pointer;
  uint32 packet_size256to_eop;
  uint32 counter_decreament;
  uint32 copy_data;
  uint32 last_seg_size;
  uint32 before_last_seg_size;
  uint32 pqp_qnum;
  uint32 pqp_oc768_qnum;
} __ATTRIBUTE_PACKED__ JER2_ARAD_EGQ_QM_TBL_DATA;

typedef struct
{
  uint32 qsm;
} __ATTRIBUTE_PACKED__ JER2_ARAD_EGQ_QSM_TBL_DATA;

typedef struct
{
  uint32 dcm;
} __ATTRIBUTE_PACKED__ JER2_ARAD_EGQ_DCM_TBL_DATA;

typedef struct
{
  uint32 mc_high_queue_weight;
  uint32 uc_high_queue_weight;
} __ATTRIBUTE_PACKED__ JER2_ARAD_EGQ_DWM_NEW_TBL_DATA;


typedef struct
{
  uint32 prog_editor_value; /* PrgeVar */
  uint32 prog_editor_profile; /* PrgeProfile */  
  uint32 outbound_mirr;  
  uint32 port_profile; /* CgmPortProfile*/
  uint32 port_ch_num;
  uint32 port_type;
  uint32 cr_adjust_type;
  uint32 cos_map_profile;
  uint32 mirror_enable;
  uint32 mirror_cmd;
  uint32 mirror_channel;
  uint32 ecc;
} __ATTRIBUTE_PACKED__ JER2_ARAD_EGQ_PCT_TBL_DATA;


typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

  uint32 map_profile; /* 3b */
  uint32 is_egr_mc; /* 1b */  
  uint32 tc; /* 3b */
  uint32 dp; /* 2b */

} __ATTRIBUTE_PACKED__ JER2_ARAD_EGQ_TC_DP_MAP_TBL_ENTRY;

typedef struct
{
  uint32 tc;
  uint32 dp;
} JER2_ARAD_EGQ_TC_DP_MAP_TBL_DATA;

typedef struct
{
  uint32 fqp_nif_port_mux;
} __ATTRIBUTE_PACKED__ JER2_ARAD_EGQ_FQP_NIF_PORT_MUX_TBL_DATA;

typedef struct
{
  uint32 pqp_nif_port_mux;
} __ATTRIBUTE_PACKED__ JER2_ARAD_EGQ_PQP_NIF_PORT_MUX_TBL_DATA;

typedef struct
{
  uint32 key_profile_map_index;
} __ATTRIBUTE_PACKED__ JER2_ARAD_EGQ_KEY_PROFILE_MAP_INDEX_TBL_DATA;

typedef struct
{
  uint32 key_select;
  uint32 key_and_value;
  uint32 key_or_value;
  uint32 tcam_profile;
} __ATTRIBUTE_PACKED__ JER2_ARAD_EGQ_TCAM_KEY_RESOLUTION_PROFILE_TBL_DATA;

typedef struct
{
  uint32 parser_last_sys_record[8];
} __ATTRIBUTE_PACKED__ JER2_ARAD_EGQ_PARSER_LAST_SYS_RECORD_TBL_DATA;

typedef struct
{
  uint32 parser_last_nwk_record1[8];
} __ATTRIBUTE_PACKED__ JER2_ARAD_EGQ_PARSER_LAST_NWK_RECORD1_TBL_DATA;

typedef struct
{
  uint32 parser_last_nwk_record2[2];
} __ATTRIBUTE_PACKED__ JER2_ARAD_EGQ_PARSER_LAST_NWK_RECORD2_TBL_DATA;

typedef struct
{
  uint32 erpp_debug[8];
} __ATTRIBUTE_PACKED__ JER2_ARAD_EGQ_ERPP_DEBUG_TBL_DATA;




typedef struct
{
  uint32 ofp_num;
  uint32 sch_hp_valid;
  uint32 sch_lp_valid;
  uint32 egq_hp_valid;
  uint32 egq_lp_valid;
} __ATTRIBUTE_PACKED__ JER2_ARAD_CFC_RCL2_OFP_TBL_DATA;

typedef struct
{
  struct
  {
    uint32 ofp_num;
    struct
    {
      uint32 hp_valid;
      uint32 lp_valid;
    }sch;
    struct
    {
      uint32 hp_valid;
      uint32 lp_valid;
    }egq;
  }data[4];
} __ATTRIBUTE_PACKED__ JER2_ARAD_CFC_NIFCLSB2_OFP_TBL_DATA;

typedef struct
{
  uint32 fc_index;
  uint32 fc_dest_sel;
  uint32 parity;
} __ATTRIBUTE_PACKED__ JER2_ARAD_CFC_CALRX_TBL_DATA;

typedef struct
{
  uint32 bitmap[JER2_ARAD_CFC_HCFC_BITMAP_TBL_SIZE];
} __ATTRIBUTE_PACKED__ JER2_ARAD_CFC_HCFC_BITMAP_TBL_DATA;

typedef struct
{
  uint32 ofp_hr;
  uint32 lp_ofp_valid;
  uint32 hp_ofp_valid;
} __ATTRIBUTE_PACKED__ JER2_ARAD_CFC_OOB_SCH_MAP_TBL_DATA;

typedef struct
{
  uint32 fc_index;
  uint32 fc_source_sel;
} __ATTRIBUTE_PACKED__ JER2_ARAD_CFC_CALTX_TBL_DATA;

typedef struct
{
  uint32 lb_key_max;
  uint32 lb_key_min;
  uint32 second_range_lb_key_max;
  uint32 second_range_lb_key_min;
  uint32 is_stacking_port;
  uint32 peer_tm_domain_id;
  uint32 port_type;
  uint32 cnm_intrcpt_fc_vec_llfc;
  uint32 cnm_intrcpt_fc_vec_pfc;  
  uint32 cnm_intrcpt_fc_en;
  uint32 cnm_intrcpt_drop_en;
  uint32 ad_count_out_port_flag;
  uint32 cgm_port_profile; /* Threshold type */
  uint32 cgm_interface;
  uint32 base_q_pair_num;
  uint32 cos_map_profile;
  uint32 pmf_data;
  uint32 disable_filtering;
} JER2_ARAD_EGQ_PPCT_TBL_DATA;

#ifdef BCM_88660_A0 

typedef struct
{  
  uint32 lb_key_max;
  uint32 lb_key_min;
} JER2_ARAD_PER_PORT_LB_RANGE_TBL_DATA;

#endif /*BCM_88660_A0*/

typedef struct
{
  uint32 system_port_offset1;
  uint32 system_port_value;
  uint32 system_port_profile;
  uint32 system_port_value_to_use;
} __ATTRIBUTE_PACKED__ JER2_ARAD_IHP_TM_PORT_SYS_PORT_CONFIG_TBL_DATA;


typedef struct
{
  uint32 out_tm_port;
  uint32 out_pp_port;
  uint32 dst_system_port;
} JER2_ARAD_PP_EGQ_DSP_PTR_MAP_TBL_DATA;

/* { */
/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/

/*
 * Determining dynamic memories
 */
void jer2_arad_tbl_default_dynamic_set(void);

/*
 * Returns TRUE if the memory is dynamic
 */
int dnx_tbl_is_dynamic(int unit, soc_mem_t mem);

void jer2_arad_tbl_mark_cachable(int unit);

void
  jer2_arad_tbls_nof_repetitions_clear(void);

void
  jer2_arad_iqm_mantissa_exponent_get(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              reg_val,
    DNX_SAND_IN   uint32              mantissa_size,
    DNX_SAND_OUT  uint32              *mantissa,
    DNX_SAND_OUT  uint32              *exponent
  );

void
  jer2_arad_iqm_mantissa_exponent_set(
    DNX_SAND_IN   int             unit,
	DNX_SAND_IN   uint32              mantissa,
	DNX_SAND_IN   uint32              exponent,
    DNX_SAND_IN   uint32              mantissa_size,
	DNX_SAND_OUT  uint32              *reg_val
  );

/*
 *  Set number of repetitions for table write.
 *  Each write command is executed
 *  IndirectCommandCount number of times. The address is
 *  advanced by one for each write command. If set to 0,
 *  only one operation is performed.
 */
uint32
  jer2_arad_olp_set_reps_for_tbl_unsafe(
    DNX_SAND_IN   int  unit,
    DNX_SAND_IN   uint32   nof_reps
  );

/*
 *  Set number of repetitions for table write.
 *  Each write command is executed
 *  IndirectCommandCount number of times. The address is
 *  advanced by one for each write command. If set to 0,
 *  only one operation is performed.
 */
uint32
  jer2_arad_ire_set_reps_for_tbl_unsafe(
    DNX_SAND_IN   int  unit,
    DNX_SAND_IN   uint32   nof_reps
  );

/*
 *  Set number of repetitions for table write.
 *  Each write command is executed
 *  IndidrctCommandCount number of times. The address is
 *  advanced by one for each write command. If set to 0,
 *  only one operation is performed.
 */
uint32
  jer2_arad_idr_set_reps_for_tbl_unsafe(
    DNX_SAND_IN   int  unit,
    DNX_SAND_IN   uint32   nof_reps
  );

/*
 *  Set number of repetitions for table write.
 *  Each write command is executed
 *  IndirectCommandCount number of times. The address is
 *  advanced by one for each write command. If set to 0,
 *  only one operation is performed.
 */
uint32
  jer2_arad_irr_set_reps_for_tbl_unsafe(
    DNX_SAND_IN   int  unit,
    DNX_SAND_IN   uint32   nof_reps
  );


/*
 *  Set number of repetitions for table write for QDR block
 *  Each write command is executed nof_reps times.
 *  The address is advanced by one for each write command.
 *  If set to 0 only one operation is performed.
 */
uint32
  jer2_arad_qdr_set_reps_for_tbl_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              nof_reps
  );

/*
 *  Set number of repetitions for table write.
 *  Each write command is executed
 *  IndirectCommandCount number of times. The address is
 *  advanced by one for each write command. If set to 0,
 *  only one operation is performed.
 */
uint32
  jer2_arad_iqm_set_reps_for_tbl_unsafe(
    DNX_SAND_IN   int  unit,
    DNX_SAND_IN   uint32   nof_reps
  );

/*
 *  Set number of repetitions for table write.
 *  Each write command is executed
 *  IndirectCommandCount number of times. The address is
 *  advanced by one for each write command. If set to 0,
 *  only one operation is performed.
 */
uint32
  jer2_arad_ips_set_reps_for_tbl_unsafe(
    DNX_SAND_IN   int  unit,
    DNX_SAND_IN   uint32   nof_reps
  );

uint32
  jer2_arad_ipt_set_reps_for_tbl_unsafe(
    DNX_SAND_IN   int  unit,
    DNX_SAND_IN   uint32   nof_reps
  );

uint32
  jer2_arad_dpi_set_reps_for_tbl_unsafe(
    DNX_SAND_IN   int  unit,
    DNX_SAND_IN   uint32  block_id,
    DNX_SAND_IN   uint32   nof_reps
  );

/*
 *  Set number of repetitions for table write.
 *  Each write command is executed
 *  IndirectCommandCount number of times. The address is
 *  advanced by one for each write command. If set to 0,
 *  only one operation is performed.
 */
uint32
  jer2_arad_rtp_set_reps_for_tbl_unsafe(
    DNX_SAND_IN   int  unit,
    DNX_SAND_IN   uint32   nof_reps
  );

/*
 *  Set number of repetitions for table write.
 *  Each write command is executed
 *  IndirectCommandCount number of times. The address is
 *  advanced by one for each write command. If set to 0,
 *  only one operation is performed.
 */
uint32
  jer2_arad_egq_set_reps_for_tbl_unsafe(
    DNX_SAND_IN   int  unit,
    DNX_SAND_IN   uint32   nof_reps
  );

/*
 *  Set number of repetitions for table write.
 *  Each write command is executed
 *  IndirectCommandCount number of times. The address is
 *  advanced by one for each write command. If set to 0,
 *  only one operation is performed.
 */
uint32
  jer2_arad_cfc_set_reps_for_tbl_unsafe(
    DNX_SAND_IN   int  unit,
    DNX_SAND_IN   uint32   nof_reps
  );

/*
 *  Set number of repetitions for table write.
 *  Each write command is executed
 *  IndirectCommandCount number of times. The address is
 *  advanced by one for each write command. If set to 0,
 *  only one operation is performed.
 */
uint32
  jer2_arad_sch_set_reps_for_tbl_unsafe(
    DNX_SAND_IN   int  unit,
    DNX_SAND_IN   uint32   nof_reps
  );

/*
 * Read indirect table pge_mem_tbl from block OLP,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_olp_pge_mem_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_OLP_PGE_MEM_TBL_DATA* OLP_pge_mem_tbl_data
  );

/*
 * Write indirect table pge_mem_tbl from block OLP,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_olp_pge_mem_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_OLP_PGE_MEM_TBL_DATA* OLP_pge_mem_tbl_data
  );

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
  );

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
  );

/*
 * Read indirect table nif_port2ctxt_bit_map_tbl from block IRE,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ire_nif_port2ctxt_bit_map_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IRE_NIF_PORT2CTXT_BIT_MAP_TBL_DATA* IRE_nif_port2ctxt_bit_map_tbl_data
  );

/*
 * Write indirect table nif_port2ctxt_bit_map_tbl from block IRE,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ire_nif_port2ctxt_bit_map_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IRE_NIF_PORT2CTXT_BIT_MAP_TBL_DATA* IRE_nif_port2ctxt_bit_map_tbl_data
  );

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
  );

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
  );

/*
 * Read indirect table rcy_ctxt_map_tbl from block IRE,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ire_cpu_ctxt_map_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IRE_CPU_CTXT_MAP_TBL_DATA* IRE_rcy_ctxt_map_tbl_data
  );

/*
 * Write indirect table rcy_ctxt_map_tbl from block IRE,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ire_cpu_ctxt_map_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IRE_CPU_CTXT_MAP_TBL_DATA* IRE_rcy_ctxt_map_tbl_data
  );

/*
 * Read indirect table complete_pc_tbl from block IDR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_idr_complete_pc_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IDR_COMPLETE_PC_TBL_DATA* IDR_complete_pc_tbl_data
  );

/*
 * Write indirect table complete_pc_tbl from block IDR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_idr_complete_pc_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IDR_COMPLETE_PC_TBL_DATA* IDR_complete_pc_tbl_data
  );

/*
 * Read indirect table mirror_table_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_mirror_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IRR_MIRROR_TABLE_TBL_DATA* IRR_mirror_table_tbl_data
  );

/*
 * Write indirect table mirror_table_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_mirror_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IRR_MIRROR_TABLE_TBL_DATA* IRR_mirror_table_tbl_data
  );

/*
 * Read indirect table snoop_table_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_snoop_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IRR_SNOOP_TABLE_TBL_DATA* IRR_snoop_table_tbl_data
  );

/*
 * Write indirect table snoop_table_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_snoop_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IRR_SNOOP_TABLE_TBL_DATA* IRR_snoop_table_tbl_data
  );

/*
 * Read indirect table glag_to_lag_range_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_glag_to_lag_range_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IRR_GLAG_TO_LAG_RANGE_TBL_DATA* IRR_glag_to_lag_range_tbl_data
  );

/*
 * Write indirect table glag_to_lag_range_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_glag_to_lag_range_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IRR_GLAG_TO_LAG_RANGE_TBL_DATA* IRR_glag_to_lag_range_tbl_data
  );

/*
 * Read indirect table smooth_division_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_smooth_division_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              lag_indx,
    DNX_SAND_IN   uint32              hash_val,
    DNX_SAND_OUT  JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_DATA* IRR_smooth_division_tbl_data
  );

/*
 * Write indirect table smooth_division_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_smooth_division_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              lag_size,
    DNX_SAND_IN   uint32              hash_val,
    DNX_SAND_IN   JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_DATA* IRR_smooth_division_tbl_data
  );

/*
 * Read indirect table glag_mapping_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_glag_mapping_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              lag_ndx,
    DNX_SAND_IN   uint32              port_ndx,
    DNX_SAND_OUT  JER2_ARAD_IRR_GLAG_MAPPING_TBL_DATA* IRR_glag_mapping_tbl_data
  );

/*
 * Write indirect table glag_mapping_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_glag_mapping_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              lag_ndx,
    DNX_SAND_IN   uint32              port_ndx,
    DNX_SAND_IN   JER2_ARAD_IRR_GLAG_MAPPING_TBL_DATA* IRR_glag_mapping_tbl_data
  );

/*
 * Read indirect table destination_table_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_destination_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IRR_DESTINATION_TABLE_TBL_DATA* IRR_destination_table_tbl_data
  );

/*
 * Write indirect table destination_table_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_destination_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IRR_DESTINATION_TABLE_TBL_DATA* IRR_destination_table_tbl_data
  );

/*
 * Read indirect table glag_next_member_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_glag_next_member_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IRR_GLAG_NEXT_MEMBER_TBL_DATA* IRR_glag_next_member_tbl_data
  );

/*
 * Write indirect table glag_next_member_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_glag_next_member_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IRR_GLAG_NEXT_MEMBER_TBL_DATA* IRR_glag_next_member_tbl_data
  );

/*
 * Read indirect table rlag_next_member_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_rlag_next_member_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IRR_RLAG_NEXT_MEMBER_TBL_DATA* IRR_rlag_next_member_tbl_data
  );

/*
 * Write indirect table rlag_next_member_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_rlag_next_member_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IRR_RLAG_NEXT_MEMBER_TBL_DATA* IRR_rlag_next_member_tbl_data
  );


/*
 * Read indirect table bdb_link_list_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_bdb_link_list_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_BDB_LINK_LIST_TBL_DATA* IQM_bdb_link_list_tbl_data
  );

/*
 * Write indirect table bdb_link_list_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_bdb_link_list_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_BDB_LINK_LIST_TBL_DATA* IQM_bdb_link_list_tbl_data
  );

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
  );

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
  );

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
  );

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
  );

/*
 * Read indirect table packet_queue_tail_pointer_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_packet_queue_tail_pointer_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_PACKET_QUEUE_TAIL_POINTER_TBL_DATA* IQM_packet_queue_tail_pointer_tbl_data
  );

/*
 * Write indirect table packet_queue_tail_pointer_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_packet_queue_tail_pointer_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_PACKET_QUEUE_TAIL_POINTER_TBL_DATA* IQM_packet_queue_tail_pointer_tbl_data
  );

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
  );

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
  );

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
  );

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
  );

/*
 * Read indirect table full_user_count_memory_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_full_user_count_memory_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_FULL_USER_COUNT_MEMORY_TBL_DATA* IQM_full_user_count_memory_tbl_data
  );

/*
 * Write indirect table full_user_count_memory_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_full_user_count_memory_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_FULL_USER_COUNT_MEMORY_TBL_DATA* IQM_full_user_count_memory_tbl_data
  );

/*
 * Read indirect table mini_multicast_user_count_memory_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_mini_multicast_user_count_memory_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_MINI_MULTICAST_USER_COUNT_MEMORY_TBL_DATA* IQM_mini_multicast_user_count_memory_tbl_data
  );

/*
 * Write indirect table mini_multicast_user_count_memory_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_mini_multicast_user_count_memory_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_MINI_MULTICAST_USER_COUNT_MEMORY_TBL_DATA* IQM_mini_multicast_user_count_memory_tbl_data
  );
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
    );
/*
 * Read indirect table packet_queue_red_parameters_table_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_packet_queue_red_parameters_table_tbl_get_unsafe(
    DNX_SAND_IN   int                                             unit,
    DNX_SAND_IN   uint32                                              rt_cls_ndx,
    DNX_SAND_IN   uint32                                             drop_precedence_ndx,
    DNX_SAND_OUT  JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_DATA* IQM_packet_queue_red_parameters_table_tbl_data
  );

/*
 * Write indirect table packet_queue_red_parameters_table_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_packet_queue_red_parameters_table_tbl_set_unsafe(
    DNX_SAND_IN   int                                             unit,
    DNX_SAND_IN   uint32                                              rt_cls_ndx,
    DNX_SAND_IN   uint32                                             drop_precedence_ndx,
    DNX_SAND_IN   JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_DATA* IQM_packet_queue_red_parameters_table_tbl_data
  );

/*
 * Fill indirect table packet_queue_red_parameters_table_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_packet_queue_red_parameters_table_tbl_fill_unsafe(
    DNX_SAND_IN   int                                             unit,
    DNX_SAND_IN   JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_DATA* IQM_packet_queue_red_parameters_table_tbl_data
  );

uint32
  jer2_arad_IQM_packet_descriptor_fifos_memory_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IQM_PACKET_DESCRIPTOR_FIFOS_MEMORY_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_IQM_packet_descriptor_fifos_memory_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IQM_PACKET_DESCRIPTOR_FIFOS_MEMORY_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_IQM_tx_descriptor_fifos_memory_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IQM_TX_DESCRIPTOR_FIFOS_MEMORY_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_IQM_tx_descriptor_fifos_memory_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IQM_TX_DESCRIPTOR_FIFOS_MEMORY_TBL_DATA  *tbl_data
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/*
 * Read indirect table vsq_qsize_memory_group_a_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_qsize_memory_group_a_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_A_TBL_DATA* IQM_vsq_qsize_memory_group_a_tbl_data
  );

/*
 * Write indirect table vsq_qsize_memory_group_a_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_qsize_memory_group_a_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_A_TBL_DATA* IQM_vsq_qsize_memory_group_a_tbl_data
  );

/*
 * Read indirect table vsq_qsize_memory_group_b_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_qsize_memory_group_b_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_B_TBL_DATA* IQM_vsq_qsize_memory_group_b_tbl_data
  );

/*
 * Write indirect table vsq_qsize_memory_group_b_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_qsize_memory_group_b_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_B_TBL_DATA* IQM_vsq_qsize_memory_group_b_tbl_data
  );

/*
 * Read indirect table vsq_qsize_memory_group_c_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_qsize_memory_group_c_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_C_TBL_DATA* IQM_vsq_qsize_memory_group_c_tbl_data
  );

/*
 * Write indirect table vsq_qsize_memory_group_c_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_qsize_memory_group_c_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_C_TBL_DATA* IQM_vsq_qsize_memory_group_c_tbl_data
  );

/*
 * Read indirect table vsq_qsize_memory_group_d_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_qsize_memory_group_d_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_D_TBL_DATA* IQM_vsq_qsize_memory_group_d_tbl_data
  );

/*
 * Write indirect table vsq_qsize_memory_group_d_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_qsize_memory_group_d_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_D_TBL_DATA* IQM_vsq_qsize_memory_group_d_tbl_data
  );

/*
 * Read indirect table vsq_average_qsize_memory_group_a_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_average_qsize_memory_group_a_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_A_TBL_DATA* IQM_vsq_average_qsize_memory_group_a_tbl_data
  );

/*
 * Write indirect table vsq_average_qsize_memory_group_a_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_average_qsize_memory_group_a_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_A_TBL_DATA* IQM_vsq_average_qsize_memory_group_a_tbl_data
  );

/*
 * Read indirect table vsq_average_qsize_memory_group_b_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_average_qsize_memory_group_b_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_B_TBL_DATA* IQM_vsq_average_qsize_memory_group_b_tbl_data
  );

/*
 * Write indirect table vsq_average_qsize_memory_group_b_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_average_qsize_memory_group_b_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_B_TBL_DATA* IQM_vsq_average_qsize_memory_group_b_tbl_data
  );

/*
 * Read indirect table vsq_average_qsize_memory_group_c_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_average_qsize_memory_group_c_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_C_TBL_DATA* IQM_vsq_average_qsize_memory_group_c_tbl_data
  );

/*
 * Write indirect table vsq_average_qsize_memory_group_c_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_average_qsize_memory_group_c_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_C_TBL_DATA* IQM_vsq_average_qsize_memory_group_c_tbl_data
  );

/*
 * Read indirect table vsq_average_qsize_memory_group_d_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_average_qsize_memory_group_d_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_D_TBL_DATA* IQM_vsq_average_qsize_memory_group_d_tbl_data
  );

/*
 * Write indirect table vsq_average_qsize_memory_group_d_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_average_qsize_memory_group_d_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_D_TBL_DATA* IQM_vsq_average_qsize_memory_group_d_tbl_data
  );

/*
 * Read indirect table vsq_flow_control_parameters_table_group_a_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_flow_control_parameters_table_group_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              group_id,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_DATA* IQM_vsq_flow_control_parameters_table_group_tbl_data
  );

/*
 * Write indirect table vsq_flow_control_parameters_table_group_a_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_flow_control_parameters_table_group_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              group_id,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_DATA* IQM_vsq_flow_control_parameters_table_group_tbl_data
  );

/*
 * Read indirect table vsq_queue_parameters_table_group_a_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_queue_parameters_table_group_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              group_id,
    DNX_SAND_IN   uint32              vsq_rt_cls_ndx,
    DNX_SAND_IN   uint32             drop_precedence_ndx,
    DNX_SAND_OUT  JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA* IQM_vsq_queue_parameters_table_group_tbl_data
  );

/*
 * Write indirect table vsq_queue_parameters_table_group_a_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_queue_parameters_table_group_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              group_id,
    DNX_SAND_IN   uint32              vsq_rt_cls_ndx,
    DNX_SAND_IN   uint32             drop_precedence_ndx,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA* IQM_vsq_queue_parameters_table_group_tbl_data
  );

/*
 * Init indirect table vsq_queue_parameters_table_group_a_tbl from block IQM,
 * doesn't take semaphore!
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_vsq_queue_parameters_table_group_tbl_fill_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_DATA* IQM_vsq_queue_parameters_table_group_tbl_data
  );

uint32
  jer2_arad_iqm_vsq_flow_control_parameters_table_group_tbl_fill_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_DATA* iqm_vsq_flow_control_parameters_table_group_tbl_data
  );

/*
 * Read indirect table system_red_parameters_table_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_system_red_parameters_table_tbl_get_unsafe(
    DNX_SAND_IN   int                                       unit,
    DNX_SAND_IN   uint32                                        rt_cls_ndx,
    DNX_SAND_IN   uint32                                       drop_precedence_ndx,
    DNX_SAND_OUT  JER2_ARAD_IQM_SYSTEM_RED_PARAMETERS_TABLE_TBL_DATA* IQM_system_red_parameters_table_tbl_data
  );

/*
 * Write indirect table system_red_parameters_table_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_system_red_parameters_table_tbl_set_unsafe(
    DNX_SAND_IN   int                                       unit,
    DNX_SAND_IN   uint32                                        rt_cls_ndx,
    DNX_SAND_IN   uint32                                       drop_precedence_ndx,
    DNX_SAND_IN   JER2_ARAD_IQM_SYSTEM_RED_PARAMETERS_TABLE_TBL_DATA* IQM_system_red_parameters_table_tbl_data
  );

/*
 * Read indirect table system_red_drop_probability_values_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_system_red_drop_probability_values_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_SYSTEM_RED_DROP_PROBABILITY_VALUES_TBL_DATA* IQM_system_red_drop_probability_values_tbl_data
  );

/*
 * Write indirect table system_red_drop_probability_values_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_system_red_drop_probability_values_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_SYSTEM_RED_DROP_PROBABILITY_VALUES_TBL_DATA* IQM_system_red_drop_probability_values_tbl_data
  );

/*
 * Read indirect table system_red_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_system_red_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IQM_SYSTEM_RED_TBL_DATA* IQM_system_red_tbl_data
  );

/*
 * Write indirect table system_red_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_system_red_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IQM_SYSTEM_RED_TBL_DATA* IQM_system_red_tbl_data
  );



/*
 * Read indirect table qdr_memory from block QDR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_qdr_memory_tbl_get_unsafe(
    DNX_SAND_IN   int                 unit,
    DNX_SAND_IN   uint32                  entry_offset,
    DNX_SAND_OUT  JER2_ARAD_QDR_MEM_TBL_DATA*   QDR_memory_tbl_data
  );

/*
 * Write indirect table qdr_memory from block QDR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_qdr_memory_tbl_set_unsafe(
    DNX_SAND_IN   int                 unit,
    DNX_SAND_IN   uint32                  entry_offset,
    DNX_SAND_IN  JER2_ARAD_QDR_MEM_TBL_DATA*   QDR_memory_tbl_data
  );

/*
 * Read indirect table qdr_dll_mem_tbl from block QDR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_qdr_qdr_dll_mem_tbl_get_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                       entry_offset,
    DNX_SAND_OUT JER2_ARAD_QDR_QDR_DLL_MEM_TBL_DATA *tbl_data
  );

/*
 * Write indirect table qdr_dll_mem_tbl from block QDR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_qdr_qdr_dll_mem_tbl_set_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                       entry_offset,
    DNX_SAND_IN  JER2_ARAD_QDR_QDR_DLL_MEM_TBL_DATA *tbl_data
  );

/*
 * Read indirect table system_physical_port_lookup_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_indirect_base_queue_to_system_physical_port_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_BASE_Q_TO_SYS_PORT_TABLE_TBL_DATA* system_physical_port_tbl_data
  );

/*
 * Write indirect table system_physical_port_lookup_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_indirect_base_queue_to_system_physical_port_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_BASE_Q_TO_SYS_PORT_TABLE_TBL_DATA* system_physical_port_tbl_data
  );

uint32
  jer2_arad_indirect_base_queue_to_system_physical_port_tbl_region_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   uint32              region_size,
    DNX_SAND_IN   JER2_ARAD_BASE_Q_TO_SYS_PORT_TABLE_TBL_DATA* system_physical_port_tbl_data
  );

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
  );

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
    DNX_SAND_IN uint32 entry_offset,
    DNX_SAND_IN JER2_ARAD_BASE_Q_TO_MOD_PORT_TABLE_TBL_DATA* mod_port_tbl_data
  );


uint32
  jer2_arad_direct_base_queue_to_system_physical_port_tbl_region_set_unsafe(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int core,
    DNX_SAND_IN uint32 entry_offset,
    DNX_SAND_IN uint32 region_size,
    DNX_SAND_IN JER2_ARAD_BASE_Q_TO_MOD_PORT_TABLE_TBL_DATA* mod_port_tbl_data
  );
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
  );

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
  );

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
  );

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
  );


uint32
  jer2_arad_ips_flow_id_lookup_table_tbl_region_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   uint32              region_size,
    DNX_SAND_IN   JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_DATA* IPS_flow_id_lookup_table_tbl_data
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/*
 * Read indirect table queue_descriptor_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ips_queue_descriptor_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPS_QUEUE_DESCRIPTOR_TABLE_TBL_DATA* IPS_queue_descriptor_table_tbl_data
  );

/*
 * Write indirect table queue_descriptor_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ips_queue_descriptor_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPS_QUEUE_DESCRIPTOR_TABLE_TBL_DATA* IPS_queue_descriptor_table_tbl_data
  );

/*
 * Read indirect table queue_size_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ips_queue_size_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPS_QUEUE_SIZE_TABLE_TBL_DATA* IPS_queue_size_table_tbl_data
  );

/*
 * Write indirect table queue_size_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ips_queue_size_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPS_QUEUE_SIZE_TABLE_TBL_DATA* IPS_queue_size_table_tbl_data
  );

/*
 * Read indirect table system_red_max_queue_size_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ips_system_red_max_queue_size_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPS_SYSTEM_RED_MAX_QUEUE_SIZE_TABLE_TBL_DATA* IPS_system_red_max_queue_size_table_tbl_data
  );

/*
 * Write indirect table system_red_max_queue_size_table_tbl from block IPS,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ips_system_red_max_queue_size_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPS_SYSTEM_RED_MAX_QUEUE_SIZE_TABLE_TBL_DATA* IPS_system_red_max_queue_size_table_tbl_data
  );

/*
 * Read indirect table bdq_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_bdq_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPT_BDQ_TBL_DATA* IPT_bdq_tbl_data
  );

/*
 * Write indirect table bdq_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_bdq_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPT_BDQ_TBL_DATA* IPT_bdq_tbl_data
  );

/*
 * Read indirect table pcq_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_pcq_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPT_PCQ_TBL_DATA* IPT_pcq_tbl_data
  );

/*
 * Write indirect table pcq_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_pcq_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPT_PCQ_TBL_DATA* IPT_pcq_tbl_data
  );

/*
 * Read indirect table sop_mmu_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_sop_mmu_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPT_SOP_MMU_TBL_DATA* IPT_sop_mmu_tbl_data
  );

/*
 * Write indirect table sop_mmu_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_sop_mmu_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPT_SOP_MMU_TBL_DATA* IPT_sop_mmu_tbl_data
  );

/*
 * Read indirect table mop_mmu_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_mop_mmu_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPT_MOP_MMU_TBL_DATA* IPT_mop_mmu_tbl_data
  );

/*
 * Write indirect table mop_mmu_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_mop_mmu_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPT_MOP_MMU_TBL_DATA* IPT_mop_mmu_tbl_data
  );

/*
 * Read indirect table fdtctl_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_fdtctl_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPT_FDTCTL_TBL_DATA* IPT_fdtctl_tbl_data
  );

/*
 * Write indirect table fdtctl_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_fdtctl_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPT_FDTCTL_TBL_DATA* IPT_fdtctl_tbl_data
  );

/*
 * Read indirect table fdtdata_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_fdtdata_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPT_FDTDATA_TBL_DATA* IPT_fdtdata_tbl_data
  );

/*
 * Write indirect table fdtdata_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_fdtdata_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPT_FDTDATA_TBL_DATA* IPT_fdtdata_tbl_data
  );

/*
 * Read indirect table egqctl_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_egqctl_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPT_EGQCTL_TBL_DATA* IPT_egqctl_tbl_data
  );

/*
 * Write indirect table egqctl_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_egqctl_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPT_EGQCTL_TBL_DATA* IPT_egqctl_tbl_data
  );

/*
 * Read indirect table egqdata_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_egqdata_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_IPT_EGQDATA_TBL_DATA* IPT_egqdata_tbl_data
  );

/*
 * Write indirect table egqdata_tbl from block IPT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_ipt_egqdata_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_IPT_EGQDATA_TBL_DATA* IPT_egqdata_tbl_data
  );

/*
 * Read indirect table dll_ram_tbl from block DPI,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_dpi_dll_ram_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   uint32             dram_ndx,
    DNX_SAND_OUT  JER2_ARAD_DPI_DLL_RAM_TBL_DATA* DPI_dll_ram_tbl_data
  );

/*
 * Write indirect table dll_ram_tbl from block DPI,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_dpi_dll_ram_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   uint32             dram_ndx,
    DNX_SAND_IN  JER2_ARAD_DPI_DLL_RAM_TBL_DATA* DPI_dll_ram_tbl_data
  );

/*
 * Read indirect table unicast_distribution_memory_for_data_cells_tbl from block RTP,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_rtp_unicast_distribution_memory_for_data_cells_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_DATA_CELLS_TBL_DATA* RTP_unicast_distribution_memory_for_data_cells_tbl_data
  );

/*
 * Write indirect table unicast_distribution_memory_for_data_cells_tbl from block RTP,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_rtp_unicast_distribution_memory_for_data_cells_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_DATA_CELLS_TBL_DATA* RTP_unicast_distribution_memory_for_data_cells_tbl_data
  );

/*
 * Read indirect table unicast_distribution_memory_for_control_cells_tbl from block RTP,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_rtp_unicast_distribution_memory_for_control_cells_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_CONTROL_CELLS_TBL_DATA* RTP_unicast_distribution_memory_for_control_cells_tbl_data
  );

/*
 * Write indirect table unicast_distribution_memory_for_control_cells_tbl from block RTP,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_rtp_unicast_distribution_memory_for_control_cells_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_CONTROL_CELLS_TBL_DATA* RTP_unicast_distribution_memory_for_control_cells_tbl_data
  );

/*
 * Read indirect table scm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_scm_tbl_get_unsafe(
    DNX_SAND_IN   int                     unit,
    DNX_SAND_IN   int                     core,
    DNX_SAND_IN   JER2_ARAD_OFP_RATES_CAL_INFO *cal_info,
    DNX_SAND_IN   uint32                  entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_SCM_TBL_DATA*  EGQ_nif_scm_tbl_data
  );

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
  );

/*
 * Read indirect table nifab_nch_scm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_nifab_nch_scm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_NIFAB_NCH_SCM_TBL_DATA* EGQ_nifab_nch_scm_tbl_data
  );

/*
 * Write indirect table nifab_nch_scm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_nifab_nch_scm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_NIFAB_NCH_SCM_TBL_DATA* EGQ_nifab_nch_scm_tbl_data
  );

/*
 * Read indirect table rcy_scm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_rcy_scm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_SCM_TBL_DATA* EGQ_rcy_scm_tbl_data
  );

/*
 * Write indirect table rcy_scm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_rcy_scm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_SCM_TBL_DATA* EGQ_rcy_scm_tbl_data
  );

/*
 * Read indirect table cpu_scm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_cpu_scm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_SCM_TBL_DATA* EGQ_cpu_scm_tbl_data
  );

/*
 * Write indirect table cpu_scm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_cpu_scm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_SCM_TBL_DATA* EGQ_cpu_scm_tbl_data
  );

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
  );

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
  );

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
  );

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
  );

/*
 * Read indirect table cbm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_cbm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_CBM_TBL_DATA* EGQ_cbm_tbl_data
  );

/*
 * Write indirect table cbm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_cbm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_CBM_TBL_DATA* EGQ_cbm_tbl_data
  );

/*
 * Read indirect table fbm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_fbm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_FBM_TBL_DATA* EGQ_fbm_tbl_data
  );

/*
 * Write indirect table fbm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_fbm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_FBM_TBL_DATA* EGQ_fbm_tbl_data
  );

/*
 * Read indirect table fdm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_fdm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_FDM_TBL_DATA* EGQ_fdm_tbl_data
  );

/*
 * Write indirect table fdm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_fdm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_FDM_TBL_DATA* EGQ_fdm_tbl_data
  );

/*
 * Read indirect table dwm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_dwm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32          entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_DWM_TBL_DATA* EGQ_dwm_tbl_data
  );

/*
 * Write indirect table dwm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_dwm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32          entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_DWM_TBL_DATA* EGQ_dwm_tbl_data
  );

/*
 * Read indirect table rrdm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_rrdm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_RRDM_TBL_DATA* EGQ_rrdm_tbl_data
  );

/*
 * Write indirect table rrdm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_rrdm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_RRDM_TBL_DATA* EGQ_rrdm_tbl_data
  );

/*
 * Read indirect table rpdm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_rpdm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_RPDM_TBL_DATA* EGQ_rpdm_tbl_data
  );

/*
 * Write indirect table rpdm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_rpdm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_RPDM_TBL_DATA* EGQ_rpdm_tbl_data
  );

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
    );

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
  );

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
  );

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
  );

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
  );

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
  );
  
#endif /* BCM_88660_A0 */

/*
 * Read indirect table vlan_table_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_vlan_table_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_VLAN_TABLE_TBL_DATA* EGQ_vlan_table_tbl_data
  );

/*
 * Write indirect table vlan_table_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_vlan_table_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_VLAN_TABLE_TBL_DATA* EGQ_vlan_table_tbl_data
  );


/*
 * Read indirect table cfc_flow_control_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_cfc_flow_control_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_CFC_FLOW_CONTROL_TBL_DATA* EGQ_cfc_flow_control_tbl_data
  );

/*
 * Write indirect table cfc_flow_control_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_cfc_flow_control_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_CFC_FLOW_CONTROL_TBL_DATA* EGQ_cfc_flow_control_tbl_data
  );

/*
 * Read indirect table nifa_flow_control_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_nifa_flow_control_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_NIFA_FLOW_CONTROL_TBL_DATA* EGQ_nifa_flow_control_tbl_data
  );

/*
 * Write indirect table nifa_flow_control_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_nifa_flow_control_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_NIFA_FLOW_CONTROL_TBL_DATA* EGQ_nifa_flow_control_tbl_data
  );

/*
 * Read indirect table nifb_flow_control_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_nifb_flow_control_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_NIFB_FLOW_CONTROL_TBL_DATA* EGQ_nifb_flow_control_tbl_data
  );

/*
 * Write indirect table nifb_flow_control_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_nifb_flow_control_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_NIFB_FLOW_CONTROL_TBL_DATA* EGQ_nifb_flow_control_tbl_data
  );

/*
 * Read indirect table cpu_last_header_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_cpu_last_header_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_CPU_LAST_HEADER_TBL_DATA* EGQ_cpu_last_header_tbl_data
  );

/*
 * Write indirect table cpu_last_header_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_cpu_last_header_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_CPU_LAST_HEADER_TBL_DATA* EGQ_cpu_last_header_tbl_data
  );

/*
 * Read indirect table ipt_last_header_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_ipt_last_header_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_IPT_LAST_HEADER_TBL_DATA* EGQ_ipt_last_header_tbl_data
  );

/*
 * Write indirect table ipt_last_header_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_ipt_last_header_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_IPT_LAST_HEADER_TBL_DATA* EGQ_ipt_last_header_tbl_data
  );

/*
 * Read indirect table fdr_last_header_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_fdr_last_header_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_FDR_LAST_HEADER_TBL_DATA* EGQ_fdr_last_header_tbl_data
  );

/*
 * Write indirect table fdr_last_header_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_fdr_last_header_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_FDR_LAST_HEADER_TBL_DATA* EGQ_fdr_last_header_tbl_data
  );

/*
 * Read indirect table cpu_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_cpu_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_CPU_PACKET_COUNTER_TBL_DATA* EGQ_cpu_packet_counter_tbl_data
  );

/*
 * Write indirect table cpu_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_cpu_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_CPU_PACKET_COUNTER_TBL_DATA* EGQ_cpu_packet_counter_tbl_data
  );

/*
 * Read indirect table ipt_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_ipt_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_IPT_PACKET_COUNTER_TBL_DATA* EGQ_ipt_packet_counter_tbl_data
  );

/*
 * Write indirect table ipt_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_ipt_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_IPT_PACKET_COUNTER_TBL_DATA* EGQ_ipt_packet_counter_tbl_data
  );

/*
 * Read indirect table fdr_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_fdr_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_FDR_PACKET_COUNTER_TBL_DATA* EGQ_fdr_packet_counter_tbl_data
  );

/*
 * Write indirect table fdr_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_fdr_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_FDR_PACKET_COUNTER_TBL_DATA* EGQ_fdr_packet_counter_tbl_data
  );

/*
 * Read indirect table rqp_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_rqp_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_RQP_PACKET_COUNTER_TBL_DATA* EGQ_rqp_packet_counter_tbl_data
  );

/*
 * Write indirect table rqp_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_rqp_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_RQP_PACKET_COUNTER_TBL_DATA* EGQ_rqp_packet_counter_tbl_data
  );

/*
 * Read indirect table rqp_discard_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_rqp_discard_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_RQP_DISCARD_PACKET_COUNTER_TBL_DATA* EGQ_rqp_discard_packet_counter_tbl_data
  );

/*
 * Write indirect table rqp_discard_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_rqp_discard_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_RQP_DISCARD_PACKET_COUNTER_TBL_DATA* EGQ_rqp_discard_packet_counter_tbl_data
  );

/*
 * Read indirect table ehp_unicast_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_ehp_unicast_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_EHP_UNICAST_PACKET_COUNTER_TBL_DATA* EGQ_ehp_unicast_packet_counter_tbl_data
  );

/*
 * Write indirect table ehp_unicast_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_ehp_unicast_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_EHP_UNICAST_PACKET_COUNTER_TBL_DATA* EGQ_ehp_unicast_packet_counter_tbl_data
  );

/*
 * Read indirect table ehp_multicast_high_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_ehp_multicast_high_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_EHP_MULTICAST_HIGH_PACKET_COUNTER_TBL_DATA* EGQ_ehp_multicast_high_packet_counter_tbl_data
  );

/*
 * Write indirect table ehp_multicast_high_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_ehp_multicast_high_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_EHP_MULTICAST_HIGH_PACKET_COUNTER_TBL_DATA* EGQ_ehp_multicast_high_packet_counter_tbl_data
  );

/*
 * Read indirect table ehp_multicast_low_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_ehp_multicast_low_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_EHP_MULTICAST_LOW_PACKET_COUNTER_TBL_DATA* EGQ_ehp_multicast_low_packet_counter_tbl_data
  );

/*
 * Write indirect table ehp_multicast_low_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_ehp_multicast_low_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_EHP_MULTICAST_LOW_PACKET_COUNTER_TBL_DATA* EGQ_ehp_multicast_low_packet_counter_tbl_data
  );

/*
 * Read indirect table ehp_discard_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_ehp_discard_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_EHP_DISCARD_PACKET_COUNTER_TBL_DATA* EGQ_ehp_discard_packet_counter_tbl_data
  );

/*
 * Write indirect table ehp_discard_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_ehp_discard_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_EHP_DISCARD_PACKET_COUNTER_TBL_DATA* EGQ_ehp_discard_packet_counter_tbl_data
  );

/*
 * Read indirect table pqp_unicast_high_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_unicast_high_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_PQP_UNICAST_HIGH_PACKET_COUNTER_TBL_DATA* EGQ_pqp_unicast_high_packet_counter_tbl_data
  );

/*
 * Write indirect table pqp_unicast_high_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_unicast_high_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_PQP_UNICAST_HIGH_PACKET_COUNTER_TBL_DATA* EGQ_pqp_unicast_high_packet_counter_tbl_data
  );

/*
 * Read indirect table pqp_unicast_low_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_unicast_low_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_PQP_UNICAST_LOW_PACKET_COUNTER_TBL_DATA* EGQ_pqp_unicast_low_packet_counter_tbl_data
  );

/*
 * Write indirect table pqp_unicast_low_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_unicast_low_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_PQP_UNICAST_LOW_PACKET_COUNTER_TBL_DATA* EGQ_pqp_unicast_low_packet_counter_tbl_data
  );

/*
 * Read indirect table pqp_multicast_high_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_multicast_high_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_PQP_MULTICAST_HIGH_PACKET_COUNTER_TBL_DATA* EGQ_pqp_multicast_high_packet_counter_tbl_data
  );

/*
 * Write indirect table pqp_multicast_high_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_multicast_high_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_PQP_MULTICAST_HIGH_PACKET_COUNTER_TBL_DATA* EGQ_pqp_multicast_high_packet_counter_tbl_data
  );

/*
 * Read indirect table pqp_multicast_low_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_multicast_low_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_PQP_MULTICAST_LOW_PACKET_COUNTER_TBL_DATA* EGQ_pqp_multicast_low_packet_counter_tbl_data
  );

/*
 * Write indirect table pqp_multicast_low_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_multicast_low_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_PQP_MULTICAST_LOW_PACKET_COUNTER_TBL_DATA* EGQ_pqp_multicast_low_packet_counter_tbl_data
  );

/*
 * Read indirect table pqp_unicast_high_bytes_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_unicast_high_bytes_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_PQP_UNICAST_HIGH_BYTES_COUNTER_TBL_DATA* EGQ_pqp_unicast_high_bytes_counter_tbl_data
  );

/*
 * Write indirect table pqp_unicast_high_bytes_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_unicast_high_bytes_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_PQP_UNICAST_HIGH_BYTES_COUNTER_TBL_DATA* EGQ_pqp_unicast_high_bytes_counter_tbl_data
  );

/*
 * Read indirect table pqp_unicast_low_bytes_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_unicast_low_bytes_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_PQP_UNICAST_LOW_BYTES_COUNTER_TBL_DATA* EGQ_pqp_unicast_low_bytes_counter_tbl_data
  );

/*
 * Write indirect table pqp_unicast_low_bytes_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_unicast_low_bytes_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_PQP_UNICAST_LOW_BYTES_COUNTER_TBL_DATA* EGQ_pqp_unicast_low_bytes_counter_tbl_data
  );

/*
 * Read indirect table pqp_multicast_high_bytes_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_multicast_high_bytes_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_PQP_MULTICAST_HIGH_BYTES_COUNTER_TBL_DATA* EGQ_pqp_multicast_high_bytes_counter_tbl_data
  );

/*
 * Write indirect table pqp_multicast_high_bytes_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_multicast_high_bytes_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_PQP_MULTICAST_HIGH_BYTES_COUNTER_TBL_DATA* EGQ_pqp_multicast_high_bytes_counter_tbl_data
  );

/*
 * Read indirect table pqp_multicast_low_bytes_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_multicast_low_bytes_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_PQP_MULTICAST_LOW_BYTES_COUNTER_TBL_DATA* EGQ_pqp_multicast_low_bytes_counter_tbl_data
  );

/*
 * Write indirect table pqp_multicast_low_bytes_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_multicast_low_bytes_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_PQP_MULTICAST_LOW_BYTES_COUNTER_TBL_DATA* EGQ_pqp_multicast_low_bytes_counter_tbl_data
  );

/*
 * Read indirect table pqp_discard_unicast_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_discard_unicast_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_PQP_DISCARD_UNICAST_PACKET_COUNTER_TBL_DATA* EGQ_pqp_discard_unicast_packet_counter_tbl_data
  );

/*
 * Write indirect table pqp_discard_unicast_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_discard_unicast_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_PQP_DISCARD_UNICAST_PACKET_COUNTER_TBL_DATA* EGQ_pqp_discard_unicast_packet_counter_tbl_data
  );

/*
 * Read indirect table pqp_discard_multicast_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_discard_multicast_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_PQP_DISCARD_MULTICAST_PACKET_COUNTER_TBL_DATA* EGQ_pqp_discard_multicast_packet_counter_tbl_data
  );

/*
 * Write indirect table pqp_discard_multicast_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_discard_multicast_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_PQP_DISCARD_MULTICAST_PACKET_COUNTER_TBL_DATA* EGQ_pqp_discard_multicast_packet_counter_tbl_data
  );

/*
 * Read indirect table fqp_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_fqp_packet_counter_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_EGQ_FQP_PACKET_COUNTER_TBL_DATA* EGQ_fqp_packet_counter_tbl_data
  );

/*
 * Write indirect table fqp_packet_counter_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_fqp_packet_counter_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_EGQ_FQP_PACKET_COUNTER_TBL_DATA* EGQ_fqp_packet_counter_tbl_data
  );


/*
 * Read indirect table recycle_to_out_going_fap_port_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_recycle_to_out_going_fap_port_mapping_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_CFC_RECYCLE_TO_OUT_GOING_FAP_PORT_MAPPING_TBL_DATA* CFC_recycle_to_out_going_fap_port_mapping_tbl_data
  );

/*
 * Write indirect table recycle_to_out_going_fap_port_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_recycle_to_out_going_fap_port_mapping_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_CFC_RECYCLE_TO_OUT_GOING_FAP_PORT_MAPPING_TBL_DATA* CFC_recycle_to_out_going_fap_port_mapping_tbl_data
  );

/*
 * Read indirect table nif_a_class_based_to_ofp_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_nif_a_class_based_to_ofp_mapping_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_CFC_NIF_CLASS_BASED_TO_OFP_MAPPING_TBL_DATA* CFC_nif_a_class_based_to_ofp_mapping_tbl_data
  );

/*
 * Write indirect table nif_a_class_based_to_ofp_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_nif_a_class_based_to_ofp_mapping_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_CFC_NIF_CLASS_BASED_TO_OFP_MAPPING_TBL_DATA* CFC_nif_a_class_based_to_ofp_mapping_tbl_data
  );

/*
 * Read indirect table nif_b_class_based_to_ofp_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_nif_b_class_based_to_ofp_mapping_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_CFC_NIF_CLASS_BASED_TO_OFP_MAPPING_TBL_DATA* CFC_nif_b_class_based_to_ofp_mapping_tbl_data
  );

/*
 * Write indirect table nif_b_class_based_to_ofp_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_nif_b_class_based_to_ofp_mapping_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_CFC_NIF_CLASS_BASED_TO_OFP_MAPPING_TBL_DATA* CFC_nif_b_class_based_to_ofp_mapping_tbl_data
  );

/*
 * Read indirect table a_schedulers_based_flow_control_to_ofp_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_a_schedulers_based_flow_control_to_ofp_mapping_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_CFC_SCHEDULERS_BASED_FLOW_CONTROL_TO_OFP_MAPPING_TBL_DATA* CFC_a_schedulers_based_flow_control_to_ofp_mapping_tbl_data
  );

/*
 * Write indirect table a_schedulers_based_flow_control_to_ofp_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_a_schedulers_based_flow_control_to_ofp_mapping_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_CFC_SCHEDULERS_BASED_FLOW_CONTROL_TO_OFP_MAPPING_TBL_DATA* CFC_a_schedulers_based_flow_control_to_ofp_mapping_tbl_data
  );

/*
 * Read indirect table b_schedulers_based_flow_control_to_ofp_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_b_schedulers_based_flow_control_to_ofp_mapping_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_CFC_SCHEDULERS_BASED_FLOW_CONTROL_TO_OFP_MAPPING_TBL_DATA* CFC_b_schedulers_based_flow_control_to_ofp_mapping_tbl_data
  );

/*
 * Write indirect table b_schedulers_based_flow_control_to_ofp_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_b_schedulers_based_flow_control_to_ofp_mapping_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_CFC_SCHEDULERS_BASED_FLOW_CONTROL_TO_OFP_MAPPING_TBL_DATA* CFC_b_schedulers_based_flow_control_to_ofp_mapping_tbl_data
  );

/*
 * Read indirect table out_of_band_rx_a_calendar_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_out_of_band_rx_a_calendar_mapping_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_CFC_OUT_OF_BAND_RX_CALENDAR_MAPPING_TBL_DATA* CFC_out_of_band_rx_a_calendar_mapping_tbl_data
  );

/*
 * Write indirect table out_of_band_rx_a_calendar_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_out_of_band_rx_a_calendar_mapping_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_CFC_OUT_OF_BAND_RX_CALENDAR_MAPPING_TBL_DATA* CFC_out_of_band_rx_a_calendar_mapping_tbl_data
  );

/*
 * Read indirect table out_of_band_rx_b_calendar_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_out_of_band_rx_b_calendar_mapping_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_CFC_OUT_OF_BAND_RX_CALENDAR_MAPPING_TBL_DATA* CFC_out_of_band_rx_b_calendar_mapping_tbl_data
  );

/*
 * Write indirect table out_of_band_rx_b_calendar_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_out_of_band_rx_b_calendar_mapping_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_CFC_OUT_OF_BAND_RX_CALENDAR_MAPPING_TBL_DATA* CFC_out_of_band_rx_b_calendar_mapping_tbl_data
  );

/*
 * Read indirect table out_of_band_tx_calendar_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_out_of_band_tx_calendar_mapping_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_CFC_OUT_OF_BAND_TX_CALENDAR_MAPPING_TBL_DATA* CFC_out_of_band_tx_calendar_mapping_tbl_data
  );

/*
 * Write indirect table out_of_band_tx_calendar_mapping_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_out_of_band_tx_calendar_mapping_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_CFC_OUT_OF_BAND_TX_CALENDAR_MAPPING_TBL_DATA* CFC_out_of_band_tx_calendar_mapping_tbl_data
  );

/*
 * Read indirect table cal_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
int
  jer2_arad_sch_cal_tbl_get(
    DNX_SAND_IN   int                    unit,
    DNX_SAND_IN   int                    core_id,
    DNX_SAND_IN   uint32                 sch_offset,
    DNX_SAND_IN   uint32                 sch_to_get /*A (0) or B (1)*/,
    DNX_SAND_IN   uint32                 slots_count,
    DNX_SAND_OUT  uint32*                slots
  );

int
  jer2_arad_sch_cal_max_size_get(
    DNX_SAND_IN   int                    unit,
    DNX_SAND_IN   uint32                 sch_offset,
    DNX_SAND_OUT  uint32*                max_cal_size
   );

/*
 * Write indirect table cal_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
int
  jer2_arad_sch_cal_tbl_set(
    DNX_SAND_IN   int                    unit,
    DNX_SAND_IN   int                    core_id,
    DNX_SAND_IN   uint32                 chan_arb_id,
    DNX_SAND_IN   uint32                 sch_to_set /*A (0) or B (1)*/,
    DNX_SAND_IN   uint32                 slots_count,
                  uint32*                slots
  );

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
  );

/*
 * Write indirect table drm_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_drm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_DRM_TBL_DATA* SCH_drm_tbl_data
  );

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
  );

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
  );

/*
 * Read indirect table fdms_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_fdms_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_FDMS_TBL_DATA* SCH_fdms_tbl_data
  );

/*
 * Write indirect table fdms_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_fdms_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_FDMS_TBL_DATA* SCH_fdms_tbl_data
  );

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
  );

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
  );

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
  );

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
  );

/*
 * Read indirect table fsf_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_fsf_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_FSF_TBL_DATA* SCH_fsf_tbl_data
  );

/*
 * Write indirect table fsf_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_fsf_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_FSF_TBL_DATA* SCH_fsf_tbl_data
  );

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
  );

/*
 * Write indirect table fgm_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_fgm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_FGM_TBL_DATA* SCH_fgm_tbl_data
  );

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
  );

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
  );

/*
 * Read indirect table scc_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_scc_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_SCC_TBL_DATA* SCH_scc_tbl_data
  );

/*
 * Write indirect table scc_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_scc_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_SCC_TBL_DATA* SCH_scc_tbl_data
  );

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
  );

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
  );

/*
 * Read indirect table fqm_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_fqm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_FQM_TBL_DATA* SCH_fqm_tbl_data
  );

/*
 * Write indirect table fqm_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_fqm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_FQM_TBL_DATA* SCH_fqm_tbl_data
  );

/*
 * Read indirect table ffm_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_ffm_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_FFM_TBL_DATA* SCH_ffm_tbl_data
  );

/*
 * Write indirect table ffm_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_ffm_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   int             core,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_FFM_TBL_DATA* SCH_ffm_tbl_data
  );

/*
 * Read indirect table soc_jer2_tmctbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_jer2_tmc_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_TMC_TBL_DATA* SCH_jer2_tmc_tbl_data
  );

/*
 * Write indirect table soc_jer2_tmctbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_jer2_tmc_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_TMC_TBL_DATA* SCH_jer2_tmc_tbl_data
  );

/*
 * Write indirect table pqs_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_sch_pqs_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_SCH_PQS_TBL_DATA* SCH_pqs_tbl_data
  );

uint32
  jer2_arad_sch_pqs_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_SCH_PQS_TBL_DATA* SCH_pqs_tbl_data
  );

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
  );

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
  );


/*
 * Get the number of banks and number of Drams
 */
uint32
  jer2_arad_mmu_dram_address_space_info_get_unsafe(
    DNX_SAND_IN    int                               unit,
    DNX_SAND_OUT   uint32                                *nof_drams,
    DNX_SAND_OUT   uint32                                *nof_banks,
    DNX_SAND_OUT   uint32                                *nof_cols
  );

uint32
  jer2_arad_mmu_dram_address_space_tbl_verify(
    DNX_SAND_IN   int                               unit,
    DNX_SAND_IN   uint32                                dram_ndx,
    DNX_SAND_IN   uint32                                bank_ndx,
    DNX_SAND_IN   uint32                                row_ndx,
    DNX_SAND_IN   uint32                                col_ndx
  );

/*
 * col_ndx has to be multiply of 16.
 * proper configuration should be for the
 * DRAM, before access it. (number of drams columns etc.)
 */
uint32
  jer2_arad_mmu_dram_address_space_tbl_get_unsafe(
    DNX_SAND_IN   int                               unit,
    DNX_SAND_IN   uint32                                dram_ndx,
    DNX_SAND_IN   uint32                                bank_ndx,
    DNX_SAND_IN   uint32                                row_ndx,
    DNX_SAND_IN   uint32                                col_ndx,
    DNX_SAND_OUT  JER2_ARAD_MMU_DRAM_ADDRESS_SPACE_TBL_DATA*  MMU_dram_address_space_tbl_data
  );

/*
 * Write indirect table dram_address_space_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_dram_address_space_tbl_set_unsafe(
    DNX_SAND_IN   int                               unit,
    DNX_SAND_IN   uint32                                dram_ndx,
    DNX_SAND_IN   uint32                                bank_ndx,
    DNX_SAND_IN   uint32                                row_ndx,
    DNX_SAND_IN   uint32                                col_ndx,
    DNX_SAND_IN  JER2_ARAD_MMU_DRAM_ADDRESS_SPACE_TBL_DATA*  MMU_dram_address_space_tbl_data
  );

/*
 * Read indirect table idf_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_idf_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_IDF_TBL_DATA* MMU_idf_tbl_data
  );

/*
 * Write indirect table idf_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_idf_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_IDF_TBL_DATA* MMU_idf_tbl_data
  );

/*
 * Read indirect table fdf_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_fdf_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_FDF_TBL_DATA* MMU_fdf_tbl_data
  );

/*
 * Write indirect table fdf_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_fdf_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_FDF_TBL_DATA* MMU_fdf_tbl_data
  );

/*
 * Read indirect table rdfa_waddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rdfa_waddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_RDFA_WADDR_STATUS_TBL_DATA* MMU_rdfa_waddr_status_tbl_data
  );

/*
 * Write indirect table rdfa_waddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rdfa_waddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_RDFA_WADDR_STATUS_TBL_DATA* MMU_rdfa_waddr_status_tbl_data
  );

/*
 * Read indirect table rdfb_waddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rdfb_waddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_RDFB_WADDR_STATUS_TBL_DATA* MMU_rdfb_waddr_status_tbl_data
  );

/*
 * Write indirect table rdfb_waddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rdfb_waddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_RDFB_WADDR_STATUS_TBL_DATA* MMU_rdfb_waddr_status_tbl_data
  );

/*
 * Read indirect table rdfc_waddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rdfc_waddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_RDFC_WADDR_STATUS_TBL_DATA* MMU_rdfc_waddr_status_tbl_data
  );

/*
 * Write indirect table rdfc_waddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rdfc_waddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_RDFC_WADDR_STATUS_TBL_DATA* MMU_rdfc_waddr_status_tbl_data
  );

/*
 * Read indirect table rdfd_waddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rdfd_waddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_RDFD_WADDR_STATUS_TBL_DATA* MMU_rdfd_waddr_status_tbl_data
  );

/*
 * Write indirect table rdfd_waddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rdfd_waddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_RDFD_WADDR_STATUS_TBL_DATA* MMU_rdfd_waddr_status_tbl_data
  );

/*
 * Read indirect table rdfe_waddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rdfe_waddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_RDFE_WADDR_STATUS_TBL_DATA* MMU_rdfe_waddr_status_tbl_data
  );

/*
 * Write indirect table rdfe_waddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rdfe_waddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_RDFE_WADDR_STATUS_TBL_DATA* MMU_rdfe_waddr_status_tbl_data
  );

/*
 * Read indirect table rdff_waddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rdff_waddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_RDFF_WADDR_STATUS_TBL_DATA* MMU_rdff_waddr_status_tbl_data
  );

/*
 * Write indirect table rdff_waddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rdff_waddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_RDFF_WADDR_STATUS_TBL_DATA* MMU_rdff_waddr_status_tbl_data
  );

/*
 * Read indirect table rdf_raddr_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rdf_raddr_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_RDF_RADDR_TBL_DATA* MMU_rdf_raddr_tbl_data
  );

/*
 * Write indirect table rdf_raddr_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rdf_raddr_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_RDF_RADDR_TBL_DATA* MMU_rdf_raddr_tbl_data
  );

/*
 * Read indirect table waf_halfa_waddr_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_waf_halfa_waddr_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_WAF_HALFA_WADDR_TBL_DATA* MMU_waf_halfa_waddr_tbl_data
  );

/*
 * Write indirect table waf_halfa_waddr_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_waf_halfa_waddr_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_WAF_HALFA_WADDR_TBL_DATA* MMU_waf_halfa_waddr_tbl_data
  );

/*
 * Read indirect table waf_halfb_waddr_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_waf_halfb_waddr_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_WAF_HALFB_WADDR_TBL_DATA* MMU_waf_halfb_waddr_tbl_data
  );

/*
 * Write indirect table waf_halfb_waddr_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_waf_halfb_waddr_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_WAF_HALFB_WADDR_TBL_DATA* MMU_waf_halfb_waddr_tbl_data
  );

/*
 * Read indirect table wafa_halfa_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafa_halfa_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_WAFA_HALFA_RADDR_STATUS_TBL_DATA* MMU_wafa_halfa_raddr_status_tbl_data
  );

/*
 * Write indirect table wafa_halfa_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafa_halfa_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_WAFA_HALFA_RADDR_STATUS_TBL_DATA* MMU_wafa_halfa_raddr_status_tbl_data
  );

/*
 * Read indirect table wafb_halfa_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafb_halfa_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_WAFB_HALFA_RADDR_STATUS_TBL_DATA* MMU_wafb_halfa_raddr_status_tbl_data
  );

/*
 * Write indirect table wafb_halfa_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafb_halfa_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_WAFB_HALFA_RADDR_STATUS_TBL_DATA* MMU_wafb_halfa_raddr_status_tbl_data
  );

/*
 * Read indirect table wafc_halfa_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafc_halfa_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_WAFC_HALFA_RADDR_STATUS_TBL_DATA* MMU_wafc_halfa_raddr_status_tbl_data
  );

/*
 * Write indirect table wafc_halfa_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafc_halfa_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_WAFC_HALFA_RADDR_STATUS_TBL_DATA* MMU_wafc_halfa_raddr_status_tbl_data
  );

/*
 * Read indirect table wafd_halfa_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafd_halfa_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_WAFD_HALFA_RADDR_STATUS_TBL_DATA* MMU_wafd_halfa_raddr_status_tbl_data
  );

/*
 * Write indirect table wafd_halfa_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafd_halfa_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_WAFD_HALFA_RADDR_STATUS_TBL_DATA* MMU_wafd_halfa_raddr_status_tbl_data
  );

/*
 * Read indirect table wafe_halfa_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafe_halfa_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_WAFE_HALFA_RADDR_STATUS_TBL_DATA* MMU_wafe_halfa_raddr_status_tbl_data
  );

/*
 * Write indirect table wafe_halfa_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafe_halfa_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_WAFE_HALFA_RADDR_STATUS_TBL_DATA* MMU_wafe_halfa_raddr_status_tbl_data
  );

/*
 * Read indirect table waff_halfa_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_waff_halfa_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_WAFF_HALFA_RADDR_STATUS_TBL_DATA* MMU_waff_halfa_raddr_status_tbl_data
  );

/*
 * Write indirect table waff_halfa_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_waff_halfa_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_WAFF_HALFA_RADDR_STATUS_TBL_DATA* MMU_waff_halfa_raddr_status_tbl_data
  );

/*
 * Read indirect table wafa_halfb_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafa_halfb_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_WAFA_HALFB_RADDR_STATUS_TBL_DATA* MMU_wafa_halfb_raddr_status_tbl_data
  );

/*
 * Write indirect table wafa_halfb_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafa_halfb_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_WAFA_HALFB_RADDR_STATUS_TBL_DATA* MMU_wafa_halfb_raddr_status_tbl_data
  );

/*
 * Read indirect table wafb_halfb_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafb_halfb_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_WAFB_HALFB_RADDR_STATUS_TBL_DATA* MMU_wafb_halfb_raddr_status_tbl_data
  );

/*
 * Write indirect table wafb_halfb_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafb_halfb_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_WAFB_HALFB_RADDR_STATUS_TBL_DATA* MMU_wafb_halfb_raddr_status_tbl_data
  );

/*
 * Read indirect table wafc_halfb_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafc_halfb_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_WAFC_HALFB_RADDR_STATUS_TBL_DATA* MMU_wafc_halfb_raddr_status_tbl_data
  );

/*
 * Write indirect table wafc_halfb_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafc_halfb_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_WAFC_HALFB_RADDR_STATUS_TBL_DATA* MMU_wafc_halfb_raddr_status_tbl_data
  );

/*
 * Read indirect table wafd_halfb_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafd_halfb_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_WAFD_HALFB_RADDR_STATUS_TBL_DATA* MMU_wafd_halfb_raddr_status_tbl_data
  );

/*
 * Write indirect table wafd_halfb_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafd_halfb_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_WAFD_HALFB_RADDR_STATUS_TBL_DATA* MMU_wafd_halfb_raddr_status_tbl_data
  );

/*
 * Read indirect table wafe_halfb_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafe_halfb_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_WAFE_HALFB_RADDR_STATUS_TBL_DATA* MMU_wafe_halfb_raddr_status_tbl_data
  );

/*
 * Write indirect table wafe_halfb_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_wafe_halfb_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_WAFE_HALFB_RADDR_STATUS_TBL_DATA* MMU_wafe_halfb_raddr_status_tbl_data
  );

/*
 * Read indirect table waff_halfb_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_waff_halfb_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_WAFF_HALFB_RADDR_STATUS_TBL_DATA* MMU_waff_halfb_raddr_status_tbl_data
  );

/*
 * Write indirect table waff_halfb_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_waff_halfb_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_WAFF_HALFB_RADDR_STATUS_TBL_DATA* MMU_waff_halfb_raddr_status_tbl_data
  );

/*
 * Read indirect table raf_waddr_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_raf_waddr_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_RAF_WADDR_TBL_DATA* MMU_raf_waddr_tbl_data
  );

/*
 * Write indirect table raf_waddr_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_raf_waddr_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_RAF_WADDR_TBL_DATA* MMU_raf_waddr_tbl_data
  );

/*
 * Read indirect table rafa_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rafa_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_RAFA_RADDR_STATUS_TBL_DATA* MMU_rafa_raddr_status_tbl_data
  );

/*
 * Write indirect table rafa_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rafa_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_RAFA_RADDR_STATUS_TBL_DATA* MMU_rafa_raddr_status_tbl_data
  );

/*
 * Read indirect table rafb_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rafb_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_RAFB_RADDR_STATUS_TBL_DATA* MMU_rafb_raddr_status_tbl_data
  );

/*
 * Write indirect table rafb_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rafb_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_RAFB_RADDR_STATUS_TBL_DATA* MMU_rafb_raddr_status_tbl_data
  );

/*
 * Read indirect table rafc_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rafc_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_RAFC_RADDR_STATUS_TBL_DATA* MMU_rafc_raddr_status_tbl_data
  );

/*
 * Write indirect table rafc_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rafc_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_RAFC_RADDR_STATUS_TBL_DATA* MMU_rafc_raddr_status_tbl_data
  );

/*
 * Read indirect table rafd_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rafd_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_RAFD_RADDR_STATUS_TBL_DATA* MMU_rafd_raddr_status_tbl_data
  );

/*
 * Write indirect table rafd_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rafd_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_RAFD_RADDR_STATUS_TBL_DATA* MMU_rafd_raddr_status_tbl_data
  );

/*
 * Read indirect table rafe_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rafe_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_RAFE_RADDR_STATUS_TBL_DATA* MMU_rafe_raddr_status_tbl_data
  );

/*
 * Write indirect table rafe_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_rafe_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_RAFE_RADDR_STATUS_TBL_DATA* MMU_rafe_raddr_status_tbl_data
  );

/*
 * Read indirect table raff_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_raff_raddr_status_tbl_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_OUT  JER2_ARAD_MMU_RAFF_RADDR_STATUS_TBL_DATA* MMU_raff_raddr_status_tbl_data
  );

/*
 * Write indirect table raff_raddr_status_tbl from block MMU,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_mmu_raff_raddr_status_tbl_set_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_IN   uint32              entry_offset,
    DNX_SAND_IN   JER2_ARAD_MMU_RAFF_RADDR_STATUS_TBL_DATA* MMU_raff_raddr_status_tbl_data
  );


uint32
  jer2_arad_ipt_select_source_sum_get_unsafe(
    DNX_SAND_IN   int             unit,
    DNX_SAND_OUT  JER2_ARAD_IPT_SELECT_SOURCE_SUM_TBL_DATA* IPT_select_source_sum_tbl_data
  );


/*
 *  Set number of repetitions for table write for MMU block
 *  Each write command is executed nof_reps times.
 *  The address is advanced by one for each write command.
 *  If set to 0 only one operation is performed.
 */
uint32
  jer2_arad_mmu_set_reps_for_tbl_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              nof_reps
  );

/*
 *  Set number of repetitions for table write for FDT block
 *  Each write command is executed nof_reps times.
 *  The address is advanced by one for each write command.
 *  If set to 0 only one operation is performed.
 */
uint32
  jer2_arad_fdt_set_reps_for_tbl_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              nof_reps
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );


uint32
  jer2_arad_irr_free_pcb_memory_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRR_FREE_PCB_MEMORY_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_irr_free_pcb_memory_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRR_FREE_PCB_MEMORY_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_irr_pcb_link_table_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRR_PCB_LINK_TABLE_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_irr_pcb_link_table_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRR_PCB_LINK_TABLE_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_irr_is_free_pcb_memory_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRR_IS_FREE_PCB_MEMORY_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_irr_is_free_pcb_memory_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRR_IS_FREE_PCB_MEMORY_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_irr_is_pcb_link_table_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRR_IS_PCB_LINK_TABLE_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_irr_is_pcb_link_table_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRR_IS_PCB_LINK_TABLE_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_irr_rpf_memory_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRR_RPF_MEMORY_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_irr_rpf_memory_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRR_RPF_MEMORY_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_irr_mcr_memory_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRR_MCR_MEMORY_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_irr_mcr_memory_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRR_MCR_MEMORY_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_irr_isf_memory_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRR_ISF_MEMORY_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_irr_isf_memory_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRR_ISF_MEMORY_TBL_DATA  *tbl_data
  );

/*
 * Read indirect table lag_to_lag_range_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_lag_to_lag_range_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table lag_to_lag_range_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_lag_to_lag_range_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_DATA  *tbl_data
  );

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
  );

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
  );

/*
 * Read indirect table lag_next_member_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_lag_next_member_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRR_LAG_NEXT_MEMBER_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table lag_next_member_tbl from block IRR,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_irr_lag_next_member_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRR_LAG_NEXT_MEMBER_TBL_DATA  *tbl_data
  );

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
  );

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
  );

uint32
  jer2_arad_irr_stack_trunk_resolve_table_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32             entry_offset,
    DNX_SAND_OUT JER2_ARAD_IRR_STACK_TRUNK_RESOLVE_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_irr_stack_trunk_resolve_table_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IRR_STACK_TRUNK_RESOLVE_TBL_DATA  *tbl_data
  );

/*
 * Read indirect table ovth_mema_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_ovth_mema_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IQM_OVTH_MEMA_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table ovth_mema_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_ovth_mema_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IQM_OVTH_MEMA_TBL_DATA  *tbl_data
  );

/*
 * Read indirect table ovth_memb_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_ovth_memb_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IQM_OVTH_MEMB_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table ovth_memb_tbl from block IQM,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_iqm_ovth_memb_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IQM_OVTH_MEMB_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_ipt_cfg_byte_cnt_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IPT_CFG_BYTE_CNT_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_ipt_cfg_byte_cnt_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IPT_CFG_BYTE_CNT_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_ipt_egq_txq_wr_addr_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IPT_EGQ_TXQ_WR_ADDR_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_ipt_egq_txq_wr_addr_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IPT_EGQ_TXQ_WR_ADDR_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_ipt_egq_txq_rd_addr_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IPT_EGQ_TXQ_RD_ADDR_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_ipt_egq_txq_rd_addr_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IPT_EGQ_TXQ_RD_ADDR_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_ipt_fdt_txq_wr_addr_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IPT_FDT_TXQ_WR_ADDR_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_ipt_fdt_txq_wr_addr_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IPT_FDT_TXQ_WR_ADDR_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_ipt_fdt_txq_rd_addr_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IPT_FDT_TXQ_RD_ADDR_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_ipt_fdt_txq_rd_addr_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IPT_FDT_TXQ_RD_ADDR_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_ipt_gci_backoff_mask_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_IPT_GCI_BACKOFF_MASK_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_ipt_gci_backoff_mask_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_IPT_GCI_BACKOFF_MASK_TBL_DATA  *tbl_data
  );

/*
 * Read indirect table ipt_contro_l_fifo_tbl from block FDT,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_fdt_ipt_contro_l_fifo_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_FDT_IPT_CONTRO_L_FIFO_TBL_DATA  *tbl_data
  );


/*
 * Read indirect table qm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_qm_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_EGQ_QM_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table qm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_qm_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_EGQ_QM_TBL_DATA  *tbl_data
  );

/*
 * Read indirect table qsm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_qsm_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_EGQ_QSM_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table qsm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_qsm_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_EGQ_QSM_TBL_DATA  *tbl_data
  );

/*
 * Read indirect table dcm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_dcm_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_EGQ_DCM_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table dcm_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_dcm_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_EGQ_DCM_TBL_DATA  *tbl_data
  );

/*
 * Read indirect table dwm_new_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_dwm_new_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_EGQ_DWM_NEW_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table dwm_new_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_dwm_new_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_EGQ_DWM_NEW_TBL_DATA  *tbl_data
  );

void
  JER2_ARAD_EGQ_TC_DP_MAP_TBL_ENTRY_clear(
    DNX_SAND_OUT JER2_ARAD_EGQ_TC_DP_MAP_TBL_ENTRY       *info
  );
/*
 * Read indirect table tc_dp_map_table_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
int
  jer2_arad_egq_tc_dp_map_tbl_get_unsafe(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  int                            core,
    DNX_SAND_IN  JER2_ARAD_EGQ_TC_DP_MAP_TBL_ENTRY   *entry,
    DNX_SAND_OUT JER2_ARAD_EGQ_TC_DP_MAP_TBL_DATA    *tbl_data
  );

/*
 * Write indirect table tc_dp_map_table_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
int
  jer2_arad_egq_tc_dp_map_tbl_set_unsafe(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  int                            core,
    DNX_SAND_IN  JER2_ARAD_EGQ_TC_DP_MAP_TBL_ENTRY   *entry,
    DNX_SAND_IN  JER2_ARAD_EGQ_TC_DP_MAP_TBL_DATA    *tbl_data
  );


/*
 * Read indirect table fqp_nif_port_mux_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_fqp_nif_port_mux_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_EGQ_FQP_NIF_PORT_MUX_TBL_DATA  *tbl_data
  );

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
  );

/*
 * Read indirect table pqp_nif_port_mux_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_pqp_nif_port_mux_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_EGQ_PQP_NIF_PORT_MUX_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table pqp_nif_port_mux_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
int
  jer2_arad_egq_pqp_nif_port_mux_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_EGQ_PQP_NIF_PORT_MUX_TBL_DATA  *tbl_data
  );
/*
 * Read indirect table key_profile_map_index_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_key_profile_map_index_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              fwd_type_ndx,
    DNX_SAND_IN  uint32              acl_profile_ndx,
    DNX_SAND_OUT JER2_ARAD_EGQ_KEY_PROFILE_MAP_INDEX_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table key_profile_map_index_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_key_profile_map_index_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              fwd_type_ndx,
    DNX_SAND_IN  uint32              acl_profile_ndx,
    DNX_SAND_IN  JER2_ARAD_EGQ_KEY_PROFILE_MAP_INDEX_TBL_DATA  *tbl_data
  );

/*
 * Read indirect table tcam_key_resolution_profile_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_tcam_key_resolution_profile_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_EGQ_TCAM_KEY_RESOLUTION_PROFILE_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table tcam_key_resolution_profile_tbl from block EGQ,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_egq_tcam_key_resolution_profile_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_EGQ_TCAM_KEY_RESOLUTION_PROFILE_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_egq_parser_last_sys_record_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_EGQ_PARSER_LAST_SYS_RECORD_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_egq_parser_last_sys_record_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_EGQ_PARSER_LAST_SYS_RECORD_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_egq_parser_last_nwk_record1_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_EGQ_PARSER_LAST_NWK_RECORD1_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_egq_parser_last_nwk_record1_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_EGQ_PARSER_LAST_NWK_RECORD1_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_egq_parser_last_nwk_record2_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_EGQ_PARSER_LAST_NWK_RECORD2_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_egq_parser_last_nwk_record2_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_EGQ_PARSER_LAST_NWK_RECORD2_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_egq_erpp_debug_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_EGQ_ERPP_DEBUG_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_egq_erpp_debug_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_EGQ_ERPP_DEBUG_TBL_DATA  *tbl_data
  );



/*
 * Read indirect table rcl2_ofp_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_rcl2_ofp_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_CFC_RCL2_OFP_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table rcl2_ofp_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_rcl2_ofp_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_CFC_RCL2_OFP_TBL_DATA  *tbl_data
  );

/*
 * Read indirect table nifclsb2_ofp_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_nifclsb2_ofp_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_CFC_NIFCLSB2_OFP_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table nifclsb2_ofp_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_nifclsb2_ofp_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_CFC_NIFCLSB2_OFP_TBL_DATA  *tbl_data
  );

/*
 * Read indirect table calrxb_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_oob_calrx_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID          if_ndx,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_CFC_CALRX_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table calrxb_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_oob_calrx_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID          if_ndx,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_CFC_CALRX_TBL_DATA  *tbl_data
  );

/*
 * Read indirect table oob0_sch_map_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_oob_sch_map_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID          if_ndx,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_CFC_OOB_SCH_MAP_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table oob0_sch_map_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_oob_sch_map_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID          if_ndx,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_CFC_OOB_SCH_MAP_TBL_DATA  *tbl_data
  );

/*
 * Read indirect table caltx_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_oob_caltx_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID          if_ndx,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_CFC_CALTX_TBL_DATA  *tbl_data
  );

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
  );

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
  );

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
  );

uint32
  jer2_arad_cfc_hcfc_bitmap_tbl_set_unsafe(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32  entry_offset,
    DNX_SAND_IN  JER2_ARAD_CFC_HCFC_BITMAP_TBL_DATA  *tbl_data
  );

uint32
  jer2_arad_cfc_hcfc_bitmap_tbl_get_unsafe(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32  entry_offset,
    DNX_SAND_OUT JER2_ARAD_CFC_HCFC_BITMAP_TBL_DATA  *tbl_data
  );

/*
 * Read indirect table ilkn_sch_map_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_ilkn_sch_map_tbl_get_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID          if_ndx,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_OUT JER2_ARAD_CFC_OOB_SCH_MAP_TBL_DATA  *tbl_data
  );

/*
 * Write indirect table ilkn_sch_map_tbl from block CFC,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer2_arad_cfc_ilkn_sch_map_tbl_set_unsafe(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID          if_ndx,
    DNX_SAND_IN  uint32              entry_offset,
    DNX_SAND_IN  JER2_ARAD_CFC_OOB_SCH_MAP_TBL_DATA  *tbl_data
  );

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
  );

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
  );


uint32
  jer2_arad_iqm_cnm_profile_tbl_get_unsafe (
    DNX_SAND_IN  int                                   unit,
    DNX_SAND_IN  uint32                                   profile_ndx,
    DNX_SAND_OUT JER2_ARAD_IQM_CNM_PARAMETERS_TABLE_TBL_DATA   *tbl_data
  );

uint32
  jer2_arad_iqm_cnm_profile_tbl_set_unsafe (
    DNX_SAND_IN  int                                   unit,
    DNX_SAND_IN  uint32                                   profile_ndx,
    DNX_SAND_IN  JER2_ARAD_IQM_CNM_PARAMETERS_TABLE_TBL_DATA   *tbl_data
  );

uint32
  jer2_arad_iqm_cnm_ds_tbl_get_unsafe(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  uint32                     entry_offset,
    DNX_SAND_OUT JER2_ARAD_IQM_CNM_DS_TBL_DATA   *tbl_data
  );

uint32
  jer2_arad_iqm_cnm_ds_tbl_set_unsafe(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  uint32                     entry_offset,
    DNX_SAND_IN  JER2_ARAD_IQM_CNM_DS_TBL_DATA   *tbl_data
  );

/* Fill a table with a given entry fast, using DMA */
uint32
  jer2_arad_fill_table_with_entry(
    DNX_SAND_IN int       unit,
    DNX_SAND_IN soc_mem_t mem,
    DNX_SAND_IN int       copyno,
    DNX_SAND_IN void      *data
  );

int
jer2_arad_fill_memory_with_incremental_field(const int unit, const soc_mem_t mem, const soc_field_t field,
                                unsigned array_index_min, unsigned array_index_max,
                                const int copyno,
                                int index_min, int index_max,
                                const void *initial_entry);

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
  );

soc_mem_t
  jer2_arad_egq_scm_chan_arb_id2scm_id(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint32 chan_arb_id
  );
/**
 * BRIEF 
 *   A callback to define the value of a table entry.
 * DESCRIPTION 
 *   This callback is meant to be used with jer2_arad_fill_table_with_variable_values_by_caching (for more info see
 *   jer2_arad_fill_table_with_variable_values_by_caching).
 *   For each entry written with jer2_arad_fill_table_with_variable_values_by_caching, this callback will be called
 *   to define the contents of the entry before writing.
 *   The contents are buffered and written to DMA in bulk.
 * PARAMS 
 *   unit
 *   copyno - The block to write to (MEM_BLOCK_ANY can be used).
 *   array_index - Should be ignored.
 *   index - The index to which the value will be written.
 *   entry_sz - The size of the entry in 32-bit boundaries.
 *              Examples:
 *                An entry with 1-32 bits would have entry_sz == 1.
 *                An entry with 33-64 bits would have entry_sz == 2.
 *   value - The value to be written at index to the specified memory (to jer2_arad_fill_table_with_variable_values_by_caching).
 *           Mapping to HW:
 *            The least significant bit (bit 0) of the least significant word (word 0) corresponds to bit 0 in HW.
 *            Bit 1 of the least significant word (word 0) corresponds to bit 0 in HW.
 *            The least significant bit (bit 0) of word 1 corresponds to bit 32 in HW.
 *            Etc...
 *   opaque - Opaque user data that was passed to jer2_arad_fill_table_with_variable_values_by_caching.
 */
typedef int (*jer2_arad_fill_table_with_variable_values_by_caching_callback)(
    DNX_SAND_IN int unit, 
    DNX_SAND_IN int copyno, 
    DNX_SAND_IN int array_index, 
    DNX_SAND_IN int index, 
    DNX_SAND_OUT uint32 *value, 
    DNX_SAND_IN int entry_sz,
    DNX_SAND_IN void *opaque);

/** update hw table with cache if available */
int jer2_arad_update_table_with_cache(int unit, soc_mem_t mem);

/**
 * BRIEF 
 *   Use DMA to write a big table using caching.
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
    DNX_SAND_IN soc_mem_t mem,
    DNX_SAND_IN unsigned  array_index,
    DNX_SAND_IN int       copyno,
    DNX_SAND_IN int       index_start,
    DNX_SAND_IN int       index_end,
    DNX_SAND_IN jer2_arad_fill_table_with_variable_values_by_caching_callback callback,
    DNX_SAND_IN void *    opaque
  );

soc_error_t jer2_arad_tbl_access_deinit(DNX_SAND_IN int unit);

soc_error_t jer2_arad_tbl_access_init_unsafe(DNX_SAND_IN int unit);

soc_error_t _jer2_arad_palladium_backdoor_dispatch_full_table_write(
           DNX_SAND_IN int unit,
           DNX_SAND_IN char *sw_mem_name,
           DNX_SAND_IN uint32 entry_data[],
           DNX_SAND_IN uint32 entry_data_len
           );

/* } */

/*********************************************************************
* NAME:
 *   jer2_arad_tbl_access_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   jer2_arad_api_tbl_access module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
DNX_ERROR_DESC_ELEMENT*
  jer2_arad_tbl_access_get_errs_ptr(void);
/* } */
      
int jer2_arad_tbl_mem_cache_mem_set( int unit, soc_mem_t mem, void* en);
int jer2_arad_tbl_mem_cache_enable_parity_tbl(int unit, soc_mem_t mem, void* en);
int jer2_arad_tbl_mem_cache_enable_ecc_tbl(int unit, soc_mem_t mem, void* en);
int jer2_arad_tbl_mem_cache_enable_specific_tbl(int unit, soc_mem_t mem, void* en);
int jer2_arad_tbl_cache_enable_predefined_tbl(int unit);

                                                              
#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


/* } __JER2_ARAD_TBL_ACCESS_H_INCLUDED__*/
#endif
