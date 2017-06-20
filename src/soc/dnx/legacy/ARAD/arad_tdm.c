#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_tdm.c,v 1.36 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_TDM
/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dnx/swstate/access/lag_access.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnxc/legacy/dnxc_dev_feature_manager.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

#include <soc/dnx/legacy/ARAD/arad_api_framework.h>
#include <soc/dnx/legacy/ARAD/arad_framework.h>
#include <soc/dnx/legacy/ARAD/arad_tdm.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/mem.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_api_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_api_fabric.h>

#include <soc/dnx/legacy/port_sw_db.h>

#include <soc/dnx/legacy/drv.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define JER2_ARAD_EGQ_TDM_MAP_QUEUE_TO_TDM_REG_MULT_NOF_REGS          (8)
#define JER2_ARAD_TDM_PORT_NDX_MAX                                    (255)
#define JER2_ARAD_TDM_PB_CELL_SIZE_MAX                                (128)
#define JER2_ARAD_TDM_IS_TDM_MAX                                      (2)
#define JER2_ARAD_TDM_FTMH_OPT_UC_DEST_IF_MAX                         (255)
#define JER2_ARAD_TDM_FTMH_OPT_UC_DEST_FAP_ID_MAX                     (1*1024-1)
#define JER2_ARAD_TDM_FTMH_OPT_MC_MC_ID_MAX                           (64*1024-1)
#define JER2_ARAD_TDM_FTMH_STANDARD_UC_DEST_FAP_PORT_MAX              (255)
#define JER2_ARAD_TDM_FTMH_STANDARD_UC_DEST_FAP_ID_MAX                (2*1024-1)
#define JER2_ARAD_TDM_FTMH_STANDARD_UC_USER_DEFINE_2_MAX              (128*1024-1)
#define JER2_ARAD_TDM_FTMH_STANDARD_MC_MC_ID_MAX                      (64*1024-1)
#define JER2_ARAD_TDM_FTMH_STANDARD_MC_USER_DEF_MAX                   (8192*1024 - 1)
#define JER2_ARAD_TDM_FTMH_INFO_ACTION_ING_MAX                        (JER2_ARAD_TDM_NOF_ING_ACTIONS-1)
#define JER2_ARAD_TDM_FTMH_INFO_ACTION_EG_MAX                         (JER2_ARAD_TDM_NOF_EG_ACTIONS-1)
#define JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_NDX_MAX                  (35)
#define JER2_ARAD_TDM_FRAGMENT_NUM_VSC_128                            (0x180)
#define JER2_ARAD_TDM_FRAGMENT_NUM_VSC_256                            (0x1)
#define JER2_ARAD_TDM_FTMH_OPT_TYPE_UC                                (0)
#define JER2_ARAD_TDM_FTMH_OPT_TYPE_MC                                (1)

#define JER2_ARAD_TDM_FTMH_PB_OPT_MODE_VAL_FLD                        (0x2)
#define JER2_ARAD_TDM_FTMH_PB_STA_MODE_VAL_FLD                        (0x1)
#define JER2_ARAD_TDM_FTMH_UNCHANGED_MODE_VAL_FLD                     (0x0)
#define JER2_ARAD_TDM_FTMH_EXTERNAL_MODE_VAL_FLD                      (0x3)
#define JER2_ARAD_TDM_FTMH_JER2_ARAD_OPT_MODE_VAL_FLD                      (SOC_IS_JERICHO(unit)?(0x2):(0x4))
#define JER2_ARAD_TDM_FTMH_JER2_ARAD_STA_MODE_VAL_FLD                      (SOC_IS_JERICHO(unit)?(0x1):(0x5))
/*
 * Define of fields inside the header
 */
/* PB FTMH Optimize Unicast */
#define JER2_ARAD_TDM_PB_FTMH_OPT_TYPE_START_BIT                         (71)
#define JER2_ARAD_TDM_PB_FTMH_OPT_UC_DEST_IF_LSB                         (66)
#define JER2_ARAD_TDM_PB_FTMH_OPT_UC_DEST_IF_MSB                         (70)
#define JER2_ARAD_TDM_PB_FTMH_OPT_UC_DEST_IF_NOF_BITS                    (JER2_ARAD_TDM_PB_FTMH_OPT_UC_DEST_IF_MSB - JER2_ARAD_TDM_PB_FTMH_OPT_UC_DEST_IF_LSB + 1)
#define JER2_ARAD_TDM_PB_FTMH_OPT_UC_DEST_FAP_ID_LSB                     (56)
#define JER2_ARAD_TDM_PB_FTMH_OPT_UC_DEST_FAP_ID_MSB                     (65)
#define JER2_ARAD_TDM_PB_FTMH_OPT_UC_DEST_FAP_ID_NOF_BITS                (JER2_ARAD_TDM_PB_FTMH_OPT_UC_DEST_FAP_ID_MSB - JER2_ARAD_TDM_PB_FTMH_OPT_UC_DEST_FAP_ID_LSB + 1)

/* JER2_ARAD FTMH Optimize Unicast */
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_TYPE_START_BIT                       (63)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_UC_DEST_IF_LSB                       (40)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_UC_DEST_IF_MSB                       (47)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_UC_DEST_IF_NOF_BITS                  (JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_UC_DEST_IF_MSB - JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_UC_DEST_IF_LSB + 1)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_UC_DEST_FAP_ID_LSB                   (48)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_UC_DEST_FAP_ID_MSB                   (58)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_UC_DEST_FAP_ID_NOF_BITS              (JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_UC_DEST_FAP_ID_MSB - JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_UC_DEST_FAP_ID_LSB + 1)

/* Common FTMH Optimize Unicast */
#define JER2_ARAD_TDM_FTMH_OPT_TYPE_START_BIT \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_OPT_TYPE_START_BIT:JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_TYPE_START_BIT)
#define JER2_ARAD_TDM_FTMH_OPT_UC_DEST_IF_LSB \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_OPT_UC_DEST_IF_LSB:JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_UC_DEST_IF_LSB)
#define JER2_ARAD_TDM_FTMH_OPT_UC_DEST_IF_NOF_BITS \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_OPT_UC_DEST_IF_NOF_BITS:JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_UC_DEST_IF_NOF_BITS)
#define JER2_ARAD_TDM_FTMH_OPT_UC_DEST_FAP_ID_LSB \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_OPT_UC_DEST_FAP_ID_LSB:JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_UC_DEST_FAP_ID_LSB)
#define JER2_ARAD_TDM_FTMH_OPT_UC_DEST_FAP_ID_NOF_BITS \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_OPT_UC_DEST_FAP_ID_NOF_BITS:JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_UC_DEST_FAP_ID_NOF_BITS)

/* PB FTMH Optimize Multicast */
#define JER2_ARAD_TDM_PB_FTMH_OPT_MC_MC_ID_LSB                           (56)
#define JER2_ARAD_TDM_PB_FTMH_OPT_MC_MC_ID_MSB                           (69)
#define JER2_ARAD_TDM_PB_FTMH_OPT_MC_MC_ID_NOF_BITS                      (JER2_ARAD_TDM_PB_FTMH_OPT_MC_MC_ID_MSB - JER2_ARAD_TDM_PB_FTMH_OPT_MC_MC_ID_LSB + 1)

/* JER2_ARAD FTMH Optimize Multicast */
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_MC_MC_ID_LSB                         (40)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_MC_MC_ID_MSB                         (58)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_MC_MC_ID_NOF_BITS                    (JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_MC_MC_ID_MSB - JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_MC_MC_ID_LSB + 1)

/* Common FTMH Optimize Multicast */
#define JER2_ARAD_TDM_FTMH_OPT_MC_MC_ID_LSB \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_OPT_MC_MC_ID_LSB:JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_MC_MC_ID_LSB)
#define JER2_ARAD_TDM_FTMH_OPT_MC_MC_ID_NOF_BITS \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_OPT_MC_MC_ID_NOF_BITS:JER2_ARAD_TDM_JER2_ARAD_FTMH_OPT_MC_MC_ID_NOF_BITS)

/* PB FTMH Standard Global */
#define JER2_ARAD_TDM_PB_FTMH_HEADER_START                               (8)
#define JER2_ARAD_TDM_PB_FTMH_STA_TYPE_START_BIT                         (16 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_VERSION_LSB                            (62 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_VERSION_MSB                            (63 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_VERSION_NOF_BITS                       (JER2_ARAD_TDM_PB_FTMH_STA_VERSION_MSB - JER2_ARAD_TDM_PB_FTMH_STA_VERSION_LSB + 1)

/* PB FTMH Standard UC */
#define JER2_ARAD_TDM_FTMH_STA_TYPE_UC                                   (0)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_1_LSB                  (0 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_1_MSB                  (15 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_1_NOF_BITS             (JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_1_MSB - JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_1_LSB + 1)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_1_GET_START_BIT        (0)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_2_LSB                  (19 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_2_MSB                  (23 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_2_NOF_BITS             (JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_2_MSB - JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_2_LSB + 1)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_2_GET_START_BIT        (JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_1_NOF_BITS)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_3_LSB                  (32 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_3_MSB                  (42 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_3_NOF_BITS             (JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_3_MSB - JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_3_LSB + 1)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_3_GET_START_BIT        (JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_2_GET_START_BIT + JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_2_NOF_BITS)
/* nof bits part 1-2 */
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_NOF_BITS_PART_1_2      (JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_1_NOF_BITS + JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_2_NOF_BITS)

#define JER2_ARAD_TDM_PB_FTMH_STA_UC_FAP_PORT_ID_LSB                     (24 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_FAP_PORT_ID_MSB                     (31 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_FAP_PORT_ID_NOF_BITS                (JER2_ARAD_TDM_PB_FTMH_STA_UC_FAP_PORT_ID_MSB - JER2_ARAD_TDM_PB_FTMH_STA_UC_FAP_PORT_ID_LSB + 1)

#define JER2_ARAD_TDM_PB_FTMH_STA_UC_FAP_ID_LSB                          (48 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_FAP_ID_MSB                          (58 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_UC_FAP_ID_NOF_BITS                     (JER2_ARAD_TDM_PB_FTMH_STA_UC_FAP_ID_MSB - JER2_ARAD_TDM_PB_FTMH_STA_UC_FAP_ID_LSB + 1)

/* JER2_ARAD FTMH Standard UC */
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_TYPE_START_BIT                       (23)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_1_LSB                (0)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_1_MSB                (22)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_1_NOF_BITS           (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_1_MSB - JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_1_LSB + 1)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_1_GET_START_BIT      (0)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_2_LSB                (24)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_2_MSB                (30)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_2_NOF_BITS           (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_2_MSB - JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_2_LSB + 1)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_2_GET_START_BIT      (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_1_NOF_BITS)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_3_LSB                (39)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_3_MSB                (40)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_3_NOF_BITS           (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_3_MSB - JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_3_LSB + 1)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_3_GET_START_BIT      (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_2_GET_START_BIT + JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_2_NOF_BITS)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_4_LSB                (41)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_4_MSB                (57)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_4_NOF_BITS           (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_4_MSB - JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_4_LSB + 1)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_4_GET_START_BIT      (0)
/* nof bits part 1-2 */
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_NOF_BITS_PART_1_2    (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_1_NOF_BITS + JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_2_NOF_BITS)
/* nof bits parts 1-3 */


#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_FAP_PORT_ID_LSB                   (31)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_FAP_PORT_ID_MSB                   (38)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_FAP_PORT_ID_NOF_BITS              (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_FAP_PORT_ID_MSB - JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_FAP_PORT_ID_LSB + 1)

#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_FAP_ID_LSB                        (58)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_FAP_ID_MSB                        (68)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_FAP_ID_NOF_BITS                   (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_FAP_ID_MSB - JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_FAP_ID_LSB + 1)

/* Common FTMH Standard Unicast */
#define JER2_ARAD_TDM_FTMH_STA_TYPE_START_BIT \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_TYPE_START_BIT:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_TYPE_START_BIT)
#define JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_1_LSB \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_1_LSB:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_1_LSB)
#define JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_1_NOF_BITS \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_1_NOF_BITS:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_1_NOF_BITS)
#define JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_1_GET_START_BIT \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_1_GET_START_BIT:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_1_GET_START_BIT)
#define JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_2_LSB \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_2_LSB:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_2_LSB)
#define JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_2_NOF_BITS \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_2_NOF_BITS:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_2_NOF_BITS)
#define JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_2_GET_START_BIT \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_2_GET_START_BIT:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_2_GET_START_BIT)
#define JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_3_LSB \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_3_LSB:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_3_LSB)
#define JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_3_NOF_BITS \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_3_NOF_BITS:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_3_NOF_BITS)
#define JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_3_GET_START_BIT \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_3_GET_START_BIT:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_3_GET_START_BIT)
#define JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_NOF_BITS_PART_1_2 \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_UC_USER_DEFINED_NOF_BITS_PART_1_2:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_NOF_BITS_PART_1_2)
#define JER2_ARAD_TDM_FTMH_STA_UC_FAP_PORT_ID_LSB \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_UC_FAP_PORT_ID_LSB:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_FAP_PORT_ID_LSB)
#define JER2_ARAD_TDM_FTMH_STA_UC_FAP_PORT_ID_NOF_BITS \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_UC_FAP_PORT_ID_NOF_BITS:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_FAP_PORT_ID_NOF_BITS)
#define JER2_ARAD_TDM_FTMH_STA_UC_FAP_ID_LSB \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_UC_FAP_ID_LSB:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_FAP_ID_LSB)
#define JER2_ARAD_TDM_FTMH_STA_UC_FAP_ID_NOF_BITS \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_UC_FAP_ID_NOF_BITS:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_FAP_ID_NOF_BITS)

/* PB FTMH Standard MC */
#define JER2_ARAD_TDM_FTMH_STA_TYPE_MC                                   (1)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_1_LSB                  (14 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_1_MSB                  (15 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_1_NOF_BITS             (JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_1_MSB - JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_1_LSB + 1)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_1_GET_START_BIT        (0)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_2_LSB                  (19 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_2_MSB                  (20 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_2_NOF_BITS             (JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_2_MSB - JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_2_LSB + 1)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_2_GET_START_BIT        (JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_1_NOF_BITS)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_3_LSB                  (21 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_3_MSB                  (23 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_3_NOF_BITS             (JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_3_MSB - JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_3_LSB + 1)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_3_GET_START_BIT        (JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_2_GET_START_BIT + JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_2_NOF_BITS)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_4_LSB                  (32 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_4_MSB                  (47 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_4_NOF_BITS             (JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_4_MSB - JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_4_LSB + 1)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_4_GET_START_BIT        (JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_3_GET_START_BIT + JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_3_NOF_BITS)
/* nof bits parts 1-2 */
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_NOF_BITS_PART_1_2      (JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_1_NOF_BITS + JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_2_NOF_BITS)
/* nof bits parts 1-3 */
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_NOF_BITS_PART_1_3      (JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_NOF_BITS_PART_1_2 + JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_3_NOF_BITS)

#define JER2_ARAD_TDM_PB_FTMH_STA_MC_MC_ID_LSB                           (0 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_MC_ID_MSB                           (13 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_MC_ID_NOF_BITS                      (JER2_ARAD_TDM_PB_FTMH_STA_MC_MC_ID_MSB - JER2_ARAD_TDM_PB_FTMH_STA_MC_MC_ID_LSB + 1)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_INTERNAL_USE_LSB                    (24 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_INTERNAL_USE_MSB                    (31 + JER2_ARAD_TDM_PB_FTMH_HEADER_START)
#define JER2_ARAD_TDM_PB_FTMH_STA_MC_INTERNAL_USE_NOF_BITS               (JER2_ARAD_TDM_PB_FTMH_STA_MC_INTERNAL_USE_MSB - JER2_ARAD_TDM_PB_FTMH_STA_MC_INTERNAL_USE_LSB + 1)

/* JER2_ARAD FTMH Standard MC */
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_1_LSB                (0)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_1_MSB                (3)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_1_NOF_BITS           (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_1_MSB - JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_1_LSB + 1)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_1_GET_START_BIT      (0)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_2_LSB                (24)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_2_MSB                (30)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_2_NOF_BITS           (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_2_MSB - JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_2_LSB + 1)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_2_GET_START_BIT      (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_1_NOF_BITS)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_3_LSB                (39)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_3_MSB                (56)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_3_NOF_BITS           (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_3_MSB - JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_3_LSB + 1)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_3_GET_START_BIT      (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_2_GET_START_BIT + JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_2_NOF_BITS)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_4_LSB                (57)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_4_MSB                (59)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_4_NOF_BITS           (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_4_MSB - JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_4_LSB + 1)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_4_GET_START_BIT      (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_3_GET_START_BIT + JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_3_NOF_BITS)
/* nof bits parts 1-2 */
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_NOF_BITS_PART_1_2    (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_1_NOF_BITS + JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_2_NOF_BITS)
/* nof bits parts 1-3 */
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_NOF_BITS_PART_1_3    (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_NOF_BITS_PART_1_2 + JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_3_NOF_BITS)

#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_MC_ID_LSB                           (4)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_MC_ID_MSB                           (22)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_MC_ID_NOF_BITS                      (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_MC_ID_MSB - JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_MC_ID_LSB + 1)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_INTERNAL_USE_LSB                    (31)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_INTERNAL_USE_MSB                    (38)
#define JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_INTERNAL_USE_NOF_BITS               (JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_INTERNAL_USE_MSB - JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_INTERNAL_USE_LSB + 1)

/* Common FTMH Standard Multicast */
#define JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_1_LSB \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_1_LSB:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_1_LSB)
#define JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_1_NOF_BITS \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_1_NOF_BITS:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_1_NOF_BITS)
#define JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_1_GET_START_BIT \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_1_GET_START_BIT:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_1_GET_START_BIT)
#define JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_2_LSB \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_2_LSB:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_2_LSB)
#define JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_2_NOF_BITS \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_2_NOF_BITS:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_2_NOF_BITS)
#define JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_2_GET_START_BIT \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_2_GET_START_BIT:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_2_GET_START_BIT)
#define JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_3_LSB \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_3_LSB:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_3_LSB)
#define JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_3_NOF_BITS \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_3_NOF_BITS:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_3_NOF_BITS)
#define JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_3_GET_START_BIT \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_3_GET_START_BIT:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_3_GET_START_BIT)
#define JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_4_LSB \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_4_LSB:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_4_LSB)
#define JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_4_NOF_BITS \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_4_NOF_BITS:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_4_NOF_BITS)
#define JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_4_GET_START_BIT \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_4_GET_START_BIT:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_4_GET_START_BIT)
#define JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_NOF_BITS_PART_1_2 \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_NOF_BITS_PART_1_2:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_NOF_BITS_PART_1_2)
#define JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_NOF_BITS_PART_1_3 \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_USER_DEFINED_NOF_BITS_PART_1_3:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_USER_DEFINED_NOF_BITS_PART_1_3)
#define JER2_ARAD_TDM_FTMH_STA_MC_MC_ID_LSB \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_MC_ID_LSB:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_MC_ID_LSB)
#define JER2_ARAD_TDM_FTMH_STA_MC_MC_ID_NOF_BITS \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_MC_ID_NOF_BITS:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_MC_ID_NOF_BITS)
#define JER2_ARAD_TDM_FTMH_STA_MC_INTERNAL_USE_LSB \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_INTERNAL_USE_LSB:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_INTERNAL_USE_LSB)
#define JER2_ARAD_TDM_FTMH_STA_MC_INTERNAL_USE_NOF_BITS \
    ((is_petrab_in_system) ? JER2_ARAD_TDM_PB_FTMH_STA_MC_INTERNAL_USE_NOF_BITS:JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_MC_INTERNAL_USE_NOF_BITS)

/* CONTEXT MAP ID */
#define JER2_ARAD_TDM_CONTEXT_MAP_DEFAULT_VAL                         (0x0)

#define JER2_ARAD_TDM_FDT_IRE_TDM_MASKS_SIZE (0x2)

/*JER2_ARAD TDM DIRECT ROUTING*/


/*mode 2 RTP reachable bitmap and mask with IreTdmMask*/
#define JER2_ARAD_TDM_DIRECT_ROUNTING_RPT_EACHABLE_WITH_IRE_TDM_MASK_MODE 0x2
/*mode 3 RTP link-integrity bitmap and mask with IreTdmMask*/
#define JER2_ARAD_TDM_DIRECT_ROUNTING_RPT_LINK_INTEGRITY_WITH_IRE_TDM_MASK_MODE 0x3

#define JER2_ARAD_TDM_DIRECT_ROUNTING_RPT_REACHABLE_MODE(enable_rpt_reachable) \
    (enable_rpt_reachable ? JER2_ARAD_TDM_DIRECT_ROUNTING_RPT_LINK_INTEGRITY_WITH_IRE_TDM_MASK_MODE : JER2_ARAD_TDM_DIRECT_ROUNTING_RPT_EACHABLE_WITH_IRE_TDM_MASK_MODE)
#define JER2_ARAD_TDM_DIRECT_ROUNTING_RPT_REACHABLE_IS_ENABLED(enable_rpt_mode) \
    ((enable_rpt_mode == JER2_ARAD_TDM_DIRECT_ROUNTING_RPT_LINK_INTEGRITY_WITH_IRE_TDM_MASK_MODE) ? TRUE: FALSE)

#define JER2_ARAD_TDM_PUSH_QUEUE_TYPE                            (DNX_TMC_ITM_QT_NDX_15)
/* } */
/*************
 * MACROS    *
 *************/
/* { */

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

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

static int
jer2_arad_tdm_local_to_reassembly_context_get(int unit, uint32 port, uint32 *reassembly_context)
{
    soc_error_t rv;

    DNXC_INIT_FUNC_DEFS;
    
    if (SOC_IS_JERICHO(unit)) {
        *reassembly_context = port;
    } else {
        rv = lag_state.local_to_reassembly_context.get(unit, port, reassembly_context);
        DNXC_IF_ERR_EXIT(rv);
    }

exit:
    DNXC_FUNC_RETURN;
}

static uint8
jer2_arad_tdm_is_petrab_in_system_get(DNX_SAND_IN int unit)
{
  if (SOC_IS_JERICHO(unit)) {
    return FALSE;
  } else {
    return jer2_arad_sw_db_is_petrab_in_system_get(unit);
  }
}

/*********************************************************************
* NAME:
*     jer2_arad_tdm_unit_has_tdm
* FUNCTION:
*     check if unit has at least one port work in tdm mode(bypass or pkt).
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_OUT uint32             *tm_port_found -
*     output 1 if device has tdm port 0 if no
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/

uint32 
 jer2_arad_tdm_unit_has_tdm(
        DNX_SAND_IN int unit,
        DNX_SAND_OUT uint32 *tdm_found)
{
    soc_dnx_config_tdm_t *tdm = &(SOC_DNX_CONFIG(unit)->tdm);

    DNX_SAND_INIT_ERROR_DEFINITIONS(0);
    DNX_SAND_CHECK_NULL_INPUT(tdm_found);
    *tdm_found=0;

    if(tdm->is_bypass || tdm->is_packet) {
        *tdm_found = 1;
    }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_tdm_unit_has_tdm()",0,0);

}


/*********************************************************************
* NAME:
*     jer2_arad_tdm_init
* FUNCTION:
*     Initialization of the TDM configuration depends on the tdm mode.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  jer2_arad_tdm_init(
    DNX_SAND_IN  int                 unit
  )
{
  uint32
    fld_val,
    vsc_mode,
    res,
    tdm_egress_priority,
    tdm_egress_dp;
/*  uint32
    nof_dram;*/
  JER2_ARAD_MGMT_TDM_MODE
    tdm_mode;
  JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE
    ilkn_tdm_dedicated_queuing;
  uint8
    is_local;
  uint8
    /*dram_enable,*/
    is_petrab_in_system;
  soc_reg_above_64_val_t
    data;
  char *propval;


  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_INIT);

  SOC_REG_ABOVE_64_CLEAR(data);

  tdm_mode = jer2_arad_sw_db_tdm_mode_get(unit);
  is_petrab_in_system = jer2_arad_sw_db_is_petrab_in_system_get(unit);
  ilkn_tdm_dedicated_queuing = jer2_arad_sw_db_ilkn_tdm_dedicated_queuing_get(unit);  
  tdm_egress_priority = SOC_DNX_CONFIG(unit)->jer2_arad->init.tdm_egress_priority;
  tdm_egress_dp = SOC_DNX_CONFIG(unit)->jer2_arad->init.tdm_egress_dp;  

  if (!SOC_UNIT_NUM_VALID(unit)) {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_ILLEGAL_DEVICE_ID, 4, exit);
  }
  /*  we must check if device support tdm*/
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 9, exit,dnxc_property_get_str(unit,spn_FAP_TDM_BYPASS,&propval));

  /*TDM SP MODE CONFIGURATION*/
  if (ilkn_tdm_dedicated_queuing == JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON) {
      /*Enable ilkn_tdm_dedicated_queueing*/
      fld_val = 0x1;
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_ILAKEN_0_INTERLEAVE_ENf,  fld_val));
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  12,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_TDM_EPE_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_ILAKEN_0_INTERLEAVE_ENf,  fld_val));
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  15,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_ILAKEN_1_INTERLEAVE_ENf,  fld_val));
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  18,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_TDM_EPE_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_ILAKEN_1_INTERLEAVE_ENf,  fld_val));
  }
  /*nof_dram = soc_property_get((unit), spn_EXT_RAM_PRESENT, -1);
  dram_enable = (nof_dram == 0) ? 0:1;*/
  
  /* General configuration - tdm mode */
  
  /* TDM general configuration */  
  if (!(tdm_mode == JER2_ARAD_MGMT_TDM_MODE_PACKET)) /* TDM bypass is enabled at least in one port */
  {
    /* Enable tdm cell mode only */
    fld_val = 0x1;
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ECI_GLOBAL_10r, REG_PORT_ANY, 0, EGRESS_TDM_MODEf,  fld_val));
   /*  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  22,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_GLOBAL_10r, REG_PORT_ANY, 0, EGRESS_TDM_MODEf,  dram_enable)); */
        
    /* Enable 2 bytes ftmh only in optimize ftmh mode */
    fld_val = (tdm_mode == JER2_ARAD_MGMT_TDM_MODE_TDM_OPT)?0x1:0x0;
    /*JER2_ARAD_FLD_SET(ECI_TDM_CONFIGURATIONr, TDM_2BYTES_FTMHf, fld_val, 30, exit);*/
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  33,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_2_BYTES_FTMHf,  fld_val));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  35,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_TDM_EPE_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_2_BYTES_FTMHf,  fld_val));        
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  37,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDT_TDM_CONFIGURATIONr, REG_PORT_ANY, 0, TDM_FTMH_OPTIMIZEDf,  fld_val));
  }
  else
  {
    fld_val = 0x0;
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  45,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ECI_GLOBAL_10r, REG_PORT_ANY, 0, EGRESS_TDM_MODEf,  fld_val));
    /*DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  45,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_TDM_CONFIGURATIONr, REG_PORT_ANY, 0, EGRESS_TDM_MODEf,  fld_val));*/
    /* DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  47,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_GLOBAL_10r, REG_PORT_ANY, 0, EGRESS_TDM_MODEf,  fld_val)); */
  }

  /* Fragment number */
  /* Enable EGQ frag-num */
  fld_val = 0x1;
  /*DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  50,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_TDM_, SOC_CORE_ALL, 0, TDM_FDR_FRAG_NUM1_ENf,  fld_val));    */
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  64,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDT_TDM_CONFIGURATIONr, REG_PORT_ANY, 0, TDM_PKT_MODE_ENf,  fld_val));


  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  66,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, ECI_GLOBALEr, REG_PORT_ANY, 0, GLBL_VSC_128_MODEf, &vsc_mode));

  fld_val = (vsc_mode) ? JER2_ARAD_TDM_FRAGMENT_NUM_VSC_128:JER2_ARAD_TDM_FRAGMENT_NUM_VSC_256;
  
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  68,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ECI_GLOBAL_10r, REG_PORT_ANY, 0, TDM_FRG_NUMf,  fld_val));   
/*    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  61,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_GLOBAL_10r, REG_PORT_ANY, 0, TDM_FRG_NUMf,  fld_val)); */ 

  /* PetraB FTMH */
  fld_val = DNX_SAND_BOOL2NUM(is_petrab_in_system);
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  62,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDT_TDM_CONFIGURATIONr, REG_PORT_ANY, 0, TDM_PETRAB_FTMHf,  fld_val));
  /* PetraB Strip Fabric CRC, when enabled means it is NOT strip fabric CRC */
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  80,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_STRIP_FABRIC_CRC_ENf,  fld_val));

  fld_val = DNX_SAND_BOOL2NUM_INVERSE(is_petrab_in_system);
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  80,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, SOC_CORE_ALL, 0, VSC_256_UC_TDM_CRC_ENf,  fld_val));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  80,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, SOC_CORE_ALL, 0, VSC_256_MC_TDM_CRC_ENf,  fld_val));

  DNX_SAND_CHECK_FUNC_RESULT(jer2_arad_tdm_unit_has_tdm(unit,&fld_val), 82,  exit);
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  83,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_CONTEXT_MODEf,  fld_val));
  
  /* TDM packet size limit range 65-255 */
  fld_val = JER2_ARAD_TDM_CELL_SIZE_MIN;
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  63, exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IRE_TDM_SIZEr, REG_PORT_ANY, 0, TDM_MIN_SIZEf,  fld_val));
  
  fld_val = (vsc_mode /* 128 */) ? JER2_ARAD_TDM_PB_CELL_SIZE_MAX:JER2_ARAD_TDM_CELL_SIZE_MAX;
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  65, exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IRE_TDM_SIZEr, REG_PORT_ANY, 0, TDM_MAX_SIZEf,  fld_val));    

  /* TDM MC use only VLAN membership table (i.e. no need for TDM special format) */
  fld_val = 0;
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  63, exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_EGRESS_REPLICATION_GENERAL_CONFIGr, SOC_CORE_ALL, 0, TDM_REP_FORMAT_ENf,  fld_val));

  /* CRC removal */
  
  { /* Always enable as Packet mode can be set parallel to any mode */
    /* IPT recognize TDM packets */

    /* IRE FTMH version for TDM packet to identify the packets as TDM flows. */
    fld_val = JER2_ARAD_TDM_VERSION_ID;
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  120,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IRE_STATIC_CONFIGURATIONr, REG_PORT_ANY, 0, FTMH_VERSIONf, fld_val));
    
    /* IPT TDM enable */
    fld_val = 0x1;
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  125,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_IPT_ENABLESr, REG_PORT_ANY, 0, TDM_ENf,  fld_val));
    
    /* Strip crc for TDM packets, always disable as in JER2_ARAD we always add Fabric CRC at the ingress */
    fld_val = 0x0;
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  130, exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_IPT_ENABLESr, REG_PORT_ANY, 0, TDM_STRIP_FABRIC_CRC_ENf,  fld_val));

    /* Enable push queue for TDM packets */
    fld_val = JER2_ARAD_TDM_PUSH_QUEUE_TYPE;
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  135,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPS_PUSH_QUEUE_TYPES_CONFIGr, SOC_CORE_ALL, 0, PUSH_QUEUE_TYPEf,  fld_val));
    
    fld_val = 0x1;
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  140,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPS_PUSH_QUEUE_TYPES_CONFIGr, SOC_CORE_ALL, 0, PUSH_QUEUE_TYPE_ENf,  fld_val));
  }

  /*
   * In NON Fabric mode, enable traffic tdm local only.
   * Note that fabric module must be initialize before TDM module
   */
  if (tdm_mode == JER2_ARAD_MGMT_TDM_MODE_TDM_STA)
  {
    is_local =
      (SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.connect_mode == JER2_ARAD_FABRIC_CONNECT_MODE_SINGLE_FAP)? TRUE:FALSE;

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  160,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDT_FDT_ENABLER_REGISTERr, REG_PORT_ANY, 0, FORCE_ALL_LOCALf,  is_local));
  }
  
  /*
   * If TDM can arrive over primary pipe enable TDM priority 2 or 3  
   * Otherwise just 3
   */
  if (SOC_DNX_CONFIG(unit)->tdm.is_tdm_over_primary_pipe) {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  170,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDR_FDR_ENABLERS_REGISTER_2r, REG_PORT_ANY, 0, TDM_HEADER_PRIORITYf,  0x2));
  } else {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  171,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDR_FDR_ENABLERS_REGISTER_2r, REG_PORT_ANY, 0, TDM_HEADER_PRIORITYf,  0x3));
  }
  /* 
   * MODE #2 does not read from RTP - only looks at the link status. 
   * Used for TDM static routing.
   */
  fld_val = 0x2;
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  180,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDT_LINK_BITMAP_CONFIGURATIONr, REG_PORT_ANY, 0, IRE_TDM_MASK_MODEf,  fld_val));


  /* TDM egress priority configuration */
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  181,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, REG_PORT_ANY, 0, TDM_PKT_TCf,  tdm_egress_priority));
  /* TDM egress dp configuration */
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  181,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, REG_PORT_ANY, 0, TDM_PKT_DPf,  tdm_egress_dp));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_tdm_init()",0,0);
}


uint32
  jer2_arad_tdm_ing_ftmh_fill_header(
    DNX_SAND_IN     int                   unit,
    DNX_SAND_IN     DNX_TMC_TDM_FTMH                   *ftmh,
    DNX_SAND_IN     JER2_ARAD_TDM_FTMH_INFO_MODE       ftmh_mode,
    DNX_SAND_OUT    JER2_ARAD_IRE_TDM_CONFIG_TBL_DATA  *tbl_data
  )
{
  uint32
    tmp = 0,
    res = DNX_SAND_OK;
  uint8
    is_petrab_in_system;
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_ING_FTMH_FILL_HEADER);

  DNX_SAND_CHECK_NULL_INPUT(ftmh);
  DNX_SAND_CHECK_NULL_INPUT(tbl_data);

  is_petrab_in_system = jer2_arad_tdm_is_petrab_in_system_get(unit);
  
  switch(ftmh_mode)
  {
    case JER2_ARAD_TDM_FTMH_INFO_MODE_OPT_UC:
      /* Optimized & UC */
      tmp = JER2_ARAD_TDM_FTMH_OPT_TYPE_UC;
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_OPT_TYPE_START_BIT,
        1,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 15, exit);

      tmp = ftmh->opt_uc.dest_if;
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_OPT_UC_DEST_IF_LSB,
        JER2_ARAD_TDM_FTMH_OPT_UC_DEST_IF_NOF_BITS,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      tmp = ftmh->opt_uc.dest_fap_id;
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_OPT_UC_DEST_FAP_ID_LSB,
        JER2_ARAD_TDM_FTMH_OPT_UC_DEST_FAP_ID_NOF_BITS,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 25, exit);
      break;
    case JER2_ARAD_TDM_FTMH_INFO_MODE_OPT_MC:
      /* Optimized & MC */
      tmp = JER2_ARAD_TDM_FTMH_OPT_TYPE_MC;
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_OPT_TYPE_START_BIT,
        1,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

      tmp = ftmh->opt_mc.mc_id;
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_OPT_MC_MC_ID_LSB,
        JER2_ARAD_TDM_FTMH_OPT_MC_MC_ID_NOF_BITS,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 35, exit);
      break;
    case JER2_ARAD_TDM_FTMH_INFO_MODE_STANDARD_UC:
      /* Standard & UC */
      tmp = JER2_ARAD_TDM_FTMH_STA_TYPE_UC;
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_STA_TYPE_START_BIT,
        1,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
      if (is_petrab_in_system)
      {
        tmp = 0;
        res = dnx_sand_bitstream_set_any_field(
          &tmp,
          JER2_ARAD_TDM_PB_FTMH_STA_VERSION_LSB,
          JER2_ARAD_TDM_PB_FTMH_STA_VERSION_NOF_BITS,
          &(tbl_data->header[0]));
        DNX_SAND_CHECK_FUNC_RESULT(res, 45, exit);
      }

      tmp = ftmh->standard_uc.dest_fap_id;
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_STA_UC_FAP_ID_LSB,
        JER2_ARAD_TDM_FTMH_STA_UC_FAP_ID_NOF_BITS,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 49, exit);

      tmp = ftmh->standard_uc.dest_fap_port;
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_STA_UC_FAP_PORT_ID_LSB,
        JER2_ARAD_TDM_FTMH_STA_UC_FAP_PORT_ID_NOF_BITS,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      /* User defined */
      tmp = 0;
      res = dnx_sand_bitstream_get_any_field(
        &ftmh->standard_uc.user_def,
        JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_1_GET_START_BIT,
        JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_1_NOF_BITS,
        &(tmp)
        );
      DNX_SAND_CHECK_FUNC_RESULT(res, 52, exit);
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_1_LSB,
        JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_1_NOF_BITS,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 54, exit);
      
      tmp = 0;
      res = dnx_sand_bitstream_get_any_field(
        &ftmh->standard_uc.user_def,
        JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_2_GET_START_BIT,
        JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_2_NOF_BITS,
        &(tmp)
        );
      DNX_SAND_CHECK_FUNC_RESULT(res, 56, exit);
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_2_LSB,
        JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_2_NOF_BITS,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 58, exit);

      tmp = 0;
      res = dnx_sand_bitstream_get_any_field(
        &ftmh->standard_uc.user_def,
        JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_3_GET_START_BIT,
        JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_3_NOF_BITS,
        &(tmp)
        );
      DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_3_LSB,
        JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_3_NOF_BITS,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 62, exit);

      /* User define more than 32bits only in JER2_ARAD FTMH */
      if (!is_petrab_in_system)
      {
        tmp = 0;
        res = dnx_sand_bitstream_get_any_field(
          &ftmh->standard_uc.user_def_2,
          JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_4_GET_START_BIT,
          JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_4_NOF_BITS,
          &(tmp)
          );
        DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);
        res = dnx_sand_bitstream_set_any_field(
          &tmp,
          JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_4_LSB,
          JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_4_NOF_BITS,
          &(tbl_data->header[0]));
        DNX_SAND_CHECK_FUNC_RESULT(res, 62, exit);
      }
      break;
    case JER2_ARAD_TDM_FTMH_INFO_MODE_STANDARD_MC:
      /* Standard & MC */
      tmp = JER2_ARAD_TDM_FTMH_STA_TYPE_MC;
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_STA_TYPE_START_BIT,
        1,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 68, exit);

      if (is_petrab_in_system)
      {
        tmp = 0;
        res = dnx_sand_bitstream_set_any_field(
          &tmp,
          JER2_ARAD_TDM_PB_FTMH_STA_VERSION_LSB,
          JER2_ARAD_TDM_PB_FTMH_STA_VERSION_NOF_BITS,
          &(tbl_data->header[0]));
        DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);
      }

      tmp = ftmh->standard_mc.mc_id;
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_STA_MC_MC_ID_LSB,
        JER2_ARAD_TDM_FTMH_STA_MC_MC_ID_NOF_BITS,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 72, exit);

      tmp = 0xff;
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_STA_MC_INTERNAL_USE_LSB,
        JER2_ARAD_TDM_FTMH_STA_MC_INTERNAL_USE_NOF_BITS,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 74, exit);

      /* User defined */
      res = dnx_sand_bitstream_get_any_field(
        &ftmh->standard_mc.user_def,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_1_GET_START_BIT,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_1_NOF_BITS,
        &(tmp)
        );
      DNX_SAND_CHECK_FUNC_RESULT(res, 76, exit);
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_1_LSB,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_1_NOF_BITS,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 78, exit);

      res = dnx_sand_bitstream_get_any_field(
        &ftmh->standard_mc.user_def,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_2_GET_START_BIT,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_2_NOF_BITS,
        &(tmp)
        );
      DNX_SAND_CHECK_FUNC_RESULT(res, 80, exit);
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_2_LSB,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_2_NOF_BITS,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 82, exit);

      res = dnx_sand_bitstream_get_any_field(
        &ftmh->standard_mc.user_def,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_3_GET_START_BIT,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_3_NOF_BITS,
        &(tmp)
        );
      DNX_SAND_CHECK_FUNC_RESULT(res, 84, exit);
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_3_LSB,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_3_NOF_BITS,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 86, exit);

      res = dnx_sand_bitstream_get_any_field(
        &ftmh->standard_mc.user_def,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_4_GET_START_BIT,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_4_NOF_BITS,
        &(tmp)
        );
      DNX_SAND_CHECK_FUNC_RESULT(res, 88, exit);
      res = dnx_sand_bitstream_set_any_field(
        &tmp,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_4_LSB,
        JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_4_NOF_BITS,
        &(tbl_data->header[0]));
      DNX_SAND_CHECK_FUNC_RESULT(res, 90, exit);
      break;
    default:
      break;
  }  

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_ing_ftmh_fill_header()", 0, 0);
}

/*********************************************************************
*     Configure the FTMH header operation
 *     (added/unchanged/removed) at the ingress,
 *     with the FTMH fields if added.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_ing_ftmh_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core_id,
    DNX_SAND_IN  uint32                   port_ndx,
    DNX_SAND_IN  JER2_ARAD_TDM_ING_ACTION           action_ing,
    DNX_SAND_IN  JER2_ARAD_TDM_FTMH                 *ftmh,
    DNX_SAND_IN  uint8                   is_mc
  )
{
  uint32
    res = DNX_SAND_OK;
  JER2_ARAD_IRE_TDM_CONFIG_TBL_DATA
    jer2_arad_ire_tdm_config_tbl_data;
  JER2_ARAD_MGMT_TDM_MODE
    tdm_mode;
  DNX_TMC_PORT_HEADER_TYPE
    incoming_header_type = 0;
  JER2_ARAD_TDM_FTMH_INFO_MODE
    ftmh_mode = JER2_ARAD_TDM_FTMH_INFO_MODE_OPT_UC;
  uint8
    is_petrab_in_system;
  uint32
    reassembly_context;
  int local_port;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_ING_FTMH_SET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(ftmh);

  if(SOC_IS_JERICHO(unit) && (action_ing == JER2_ARAD_TDM_ING_ACTION_CUSTOMER_EMBED)) {
    DNX_SAND_CHECK_FUNC_RESULT(DNX_SAND_ERR, 9, exit);
  }

  res = dnx_port_sw_db_tm_to_local_port_get(unit, core_id, port_ndx, &local_port);
  DNX_SAND_CHECK_FUNC_RESULT(res, 4, exit);
  res = jer2_arad_tdm_local_to_reassembly_context_get(unit,local_port, &reassembly_context);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);

  res = dnx_sand_os_memset(
    &jer2_arad_ire_tdm_config_tbl_data,
    0,
    sizeof(JER2_ARAD_IRE_TDM_CONFIG_TBL_DATA)
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  
  res = jer2_arad_ire_tdm_config_tbl_get_unsafe(
    unit,
    reassembly_context,
    &jer2_arad_ire_tdm_config_tbl_data
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 11, exit);
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  /* CPU port, no limited packet size */
  res = jer2_arad_port_header_type_get_unsafe(
          unit,
          core_id,
          port_ndx,
          &incoming_header_type,
          &outgoing_header_type
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
#endif 
      
  if (incoming_header_type == DNX_TMC_PORT_HEADER_TYPE_CPU)
  {
    jer2_arad_ire_tdm_config_tbl_data.cpu = 0x1;
  }

  tdm_mode = jer2_arad_sw_db_tdm_mode_get(unit);
  is_petrab_in_system = jer2_arad_tdm_is_petrab_in_system_get(unit);

  switch(action_ing)
  {
    case JER2_ARAD_TDM_ING_ACTION_ADD:
      if (tdm_mode == JER2_ARAD_MGMT_TDM_MODE_TDM_OPT)
      {
        /* Action add: TDM optimize */
        jer2_arad_ire_tdm_config_tbl_data.mode = (is_petrab_in_system) ? JER2_ARAD_TDM_FTMH_PB_OPT_MODE_VAL_FLD : JER2_ARAD_TDM_FTMH_JER2_ARAD_OPT_MODE_VAL_FLD;
        ftmh_mode = (is_mc == FALSE) ? JER2_ARAD_TDM_FTMH_INFO_MODE_OPT_UC:JER2_ARAD_TDM_FTMH_INFO_MODE_OPT_MC;
      }
      else
      {
        jer2_arad_ire_tdm_config_tbl_data.mode = (is_petrab_in_system) ? JER2_ARAD_TDM_FTMH_PB_STA_MODE_VAL_FLD : JER2_ARAD_TDM_FTMH_JER2_ARAD_STA_MODE_VAL_FLD;
        ftmh_mode = (is_mc == FALSE) ? JER2_ARAD_TDM_FTMH_INFO_MODE_STANDARD_UC:JER2_ARAD_TDM_FTMH_INFO_MODE_STANDARD_MC;
      }
      break;
    case JER2_ARAD_TDM_ING_ACTION_CUSTOMER_EMBED:
      /* Same as ADD, but different mode */
      jer2_arad_ire_tdm_config_tbl_data.mode = JER2_ARAD_TDM_FTMH_EXTERNAL_MODE_VAL_FLD;
      if (tdm_mode == JER2_ARAD_MGMT_TDM_MODE_TDM_OPT)
      {
        ftmh_mode = (is_mc == FALSE) ? JER2_ARAD_TDM_FTMH_INFO_MODE_OPT_UC:JER2_ARAD_TDM_FTMH_INFO_MODE_OPT_MC;
      }
      else
      {
        ftmh_mode = (is_mc == FALSE) ? JER2_ARAD_TDM_FTMH_INFO_MODE_STANDARD_UC:JER2_ARAD_TDM_FTMH_INFO_MODE_STANDARD_MC;
      }
      break;
    case JER2_ARAD_TDM_ING_ACTION_NO_CHANGE:
      jer2_arad_ire_tdm_config_tbl_data.mode = JER2_ARAD_TDM_FTMH_UNCHANGED_MODE_VAL_FLD;
      break;
    default:
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_TDM_ACTION_ING_OUT_OF_RANGE_ERR, 15, exit);
  }

  if (action_ing != JER2_ARAD_TDM_ING_ACTION_NO_CHANGE)
  {
    res = jer2_arad_tdm_ing_ftmh_fill_header(
      unit,
      ftmh,
      ftmh_mode,
      &jer2_arad_ire_tdm_config_tbl_data
      );
    DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }  

  res = jer2_arad_ire_tdm_config_tbl_set_unsafe(
          unit,
          reassembly_context,
          &jer2_arad_ire_tdm_config_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_ing_ftmh_set_unsafe()", port_ndx, 0);
}

/*********************************************************************
*     Configure the FTMH header operation
 *     (added/unchanged/removed) at the egress,
 *     with the FTMH fields if added.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_eg_ftmh_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core_id,
    DNX_SAND_IN  uint32                port_ndx,
    DNX_SAND_IN  JER2_ARAD_TDM_EG_ACTION    action_eg
  )
{
    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY  
  uint32
    res;
  DNX_TMC_PORT_HEADER_TYPE
    header_type = 0;    

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_EG_FTMH_SET_UNSAFE);

  if(SOC_IS_JERICHO(unit) && (action_eg == JER2_ARAD_TDM_EG_ACTION_CUSTOMER_EXTRACT)
     && (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "tdm_eg_editing_is_cud_stamping", 0) != 1)
     && (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "tdm_eg_editing_is_cud_stamping_type1", 0) != 1)) {
      DNX_SAND_CHECK_FUNC_RESULT(DNX_SAND_ERR, 9, exit);
  }

  switch(action_eg)
  {
    case JER2_ARAD_TDM_EG_ACTION_REMOVE:
      header_type = DNX_TMC_PORT_HEADER_TYPE_TDM; /* No matter it is TDM or TDM_RAW */
      break;
    case JER2_ARAD_TDM_EG_ACTION_CUSTOMER_EXTRACT: /* PMM */
      header_type = DNX_TMC_PORT_HEADER_TYPE_TDM_PMM;
      break;
    case JER2_ARAD_TDM_EG_ACTION_NO_CHANGE:  /* Don't touch */
      header_type = DNX_TMC_PORT_HEADER_TYPE_CPU;
      break;
    default:
      break;
  }
  res = jer2_arad_port_header_type_set_unsafe(
          unit,
          core_id,
          port_ndx,
          DNX_TMC_PORT_DIRECTION_OUTGOING,
          header_type
        );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
    

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_eg_ftmh_set_unsafe()", port_ndx, 0);
#endif 
    return -1;
}

/*********************************************************************
*     Configure the FTMH header operation
 *     (added/unchanged/removed) at the ingress and egress,
 *     with the FTMH fields if added.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_ftmh_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core_id,
    DNX_SAND_IN  uint32                   port_ndx,
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_INFO            *info
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_FTMH_SET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(info);
  
  res = jer2_arad_tdm_ing_ftmh_set_unsafe(
          unit,
          core_id,
          port_ndx,
          info->action_ing,
          &info->ftmh,
          info->is_mc
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = jer2_arad_tdm_eg_ftmh_set_unsafe(
          unit,
          core_id,
          port_ndx,
          info->action_eg
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_ftmh_set_unsafe()", port_ndx, 0);
}

uint32
  jer2_arad_tdm_ftmh_set_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx,
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_INFO            *info
  )
{
  uint32
    res = DNX_SAND_OK;
  JER2_ARAD_MGMT_TDM_MODE
    tdm_mode;
  uint8
    is_petrab_in_system;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_FTMH_SET_VERIFY);

  if(SOC_IS_JERICHO(unit) && (info->action_ing == JER2_ARAD_TDM_ING_ACTION_CUSTOMER_EMBED)) {
    DNX_SAND_CHECK_FUNC_RESULT(DNX_SAND_ERR, 9, exit);
  }

  tdm_mode = jer2_arad_sw_db_tdm_mode_get(unit);
  is_petrab_in_system = jer2_arad_tdm_is_petrab_in_system_get(unit);

  DNX_SAND_ERR_IF_ABOVE_MAX(port_ndx, JER2_ARAD_TDM_PORT_NDX_MAX, JER2_ARAD_TDM_PORT_NDX_OUT_OF_RANGE_ERR, 10, exit);
  /* PMM only in soc_petrab standard mode */
  if (info->action_ing == JER2_ARAD_TDM_ING_ACTION_CUSTOMER_EMBED)    
  {
    if (!(is_petrab_in_system) ||  (tdm_mode != JER2_ARAD_MGMT_TDM_MODE_TDM_STA))
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_TDM_CUSTOMER_EMBED_IN_OPTIMIZED_MODE_ERR, 7, exit);
    }    
  }
  JER2_ARAD_STRUCT_VERIFY(JER2_ARAD_TDM_FTMH_INFO, info, 20, exit);
  /* API relevant only in tdm traffic mode. */
  DNX_SAND_ERR_IF_BELOW_MIN(tdm_mode,JER2_ARAD_MGMT_TDM_MODE_TDM_OPT, JER2_ARAD_TDM_INVALID_TDM_MODE_ERR, 25, exit);

  /* Verify struct JER2_ARAD_TDM_FTMH */
  DNX_SAND_CHECK_NULL_INPUT(&(info->ftmh));

  switch (tdm_mode)
  {
  case JER2_ARAD_MGMT_TDM_MODE_TDM_OPT:
    if (!info->is_mc)
    {
      JER2_ARAD_STRUCT_VERIFY(JER2_ARAD_TDM_FTMH_OPT_UC, &(info->ftmh.opt_uc), 10, exit);
    }
    else
    {
      JER2_ARAD_STRUCT_VERIFY(JER2_ARAD_TDM_FTMH_OPT_MC, &(info->ftmh.opt_mc), 11, exit);
    }
    break;
  case JER2_ARAD_MGMT_TDM_MODE_TDM_STA:
    if (!info->is_mc)
    {
      JER2_ARAD_STRUCT_VERIFY(JER2_ARAD_TDM_FTMH_STANDARD_UC, &(info->ftmh.standard_uc), 12, exit);
    }
    else
    {
      JER2_ARAD_STRUCT_VERIFY(JER2_ARAD_TDM_FTMH_STANDARD_MC, &(info->ftmh.standard_mc), 13, exit);
    }
    break;
  default:
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_TDM_INVALID_TDM_MODE_ERR, 20, exit);
    break;
  }
  

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_ftmh_set_verify()", port_ndx, 0);
}

uint32
  jer2_arad_tdm_ftmh_get_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx
  )
{
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_FTMH_GET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(port_ndx, JER2_ARAD_TDM_PORT_NDX_MAX, JER2_ARAD_TDM_PORT_NDX_OUT_OF_RANGE_ERR, 10, exit);
  
  /* API relevant only in tdm traffic mode. */
  DNX_SAND_ERR_IF_BELOW_MIN(jer2_arad_sw_db_tdm_mode_get(unit),JER2_ARAD_MGMT_TDM_MODE_TDM_OPT, JER2_ARAD_TDM_INVALID_TDM_MODE_ERR, 25, exit);
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_ftmh_get_verify()", port_ndx, 0);
}

/*********************************************************************
*     Configure the FTMH header operation
 *     (added/unchanged/removed) at the ingress,
 *     with the FTMH fields if added.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_ing_ftmh_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx,
    DNX_SAND_OUT JER2_ARAD_TDM_ING_ACTION           *action_ing,
    DNX_SAND_OUT JER2_ARAD_TDM_FTMH                 *ftmh,
    DNX_SAND_OUT uint8                   *is_mc
  )
{
  uint32
    tmp = 0,
    res = DNX_SAND_OK;
  JER2_ARAD_IRE_TDM_CONFIG_TBL_DATA
    jer2_arad_ire_tdm_config_tbl_data;
  JER2_ARAD_MGMT_TDM_MODE
    tdm_mode;
  uint8
    is_petrab_in_system;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_ING_FTMH_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(action_ing);
  DNX_SAND_CHECK_NULL_INPUT(ftmh);
  DNX_SAND_CHECK_NULL_INPUT(is_mc);

  res = dnx_sand_os_memset(
          &jer2_arad_ire_tdm_config_tbl_data,
          0,
          sizeof(JER2_ARAD_IRE_TDM_CONFIG_TBL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  
  res = jer2_arad_ire_tdm_config_tbl_get_unsafe(
          unit,
          port_ndx,
          &jer2_arad_ire_tdm_config_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (jer2_arad_ire_tdm_config_tbl_data.mode == JER2_ARAD_TDM_FTMH_EXTERNAL_MODE_VAL_FLD) {
      *action_ing = JER2_ARAD_TDM_ING_ACTION_CUSTOMER_EMBED;
  } else if (jer2_arad_ire_tdm_config_tbl_data.mode == JER2_ARAD_TDM_FTMH_UNCHANGED_MODE_VAL_FLD) {
      *action_ing = JER2_ARAD_TDM_ING_ACTION_NO_CHANGE;
  } else {
    *action_ing = JER2_ARAD_TDM_ING_ACTION_ADD;
  }
  
  if (*action_ing != JER2_ARAD_TDM_ING_ACTION_NO_CHANGE)
  {
    tdm_mode = jer2_arad_sw_db_tdm_mode_get(unit);
    is_petrab_in_system = jer2_arad_tdm_is_petrab_in_system_get(unit);

   
    if (jer2_arad_ire_tdm_config_tbl_data.mode == (is_petrab_in_system ? JER2_ARAD_TDM_FTMH_PB_OPT_MODE_VAL_FLD:JER2_ARAD_TDM_FTMH_JER2_ARAD_OPT_MODE_VAL_FLD))
    {
      /* Verify tdm mode is right */
      if (tdm_mode != JER2_ARAD_MGMT_TDM_MODE_TDM_OPT)
      {
        DNX_SAND_SET_ERROR_CODE(JER2_ARAD_TDM_INVALID_TDM_MODE_ERR, 23, exit);
      }     
      res = dnx_sand_bitstream_get_any_field(
        &(jer2_arad_ire_tdm_config_tbl_data.header[0]),
        JER2_ARAD_TDM_FTMH_OPT_TYPE_START_BIT,
        1,
        &(tmp)
      );
      DNX_SAND_CHECK_FUNC_RESULT(res, 25, exit);

      if (tmp == JER2_ARAD_TDM_FTMH_OPT_TYPE_UC)
      {
        /* Optimized & UC */
        *is_mc = FALSE;
        res = dnx_sand_bitstream_get_any_field(
          &jer2_arad_ire_tdm_config_tbl_data.header[0],
          JER2_ARAD_TDM_FTMH_OPT_UC_DEST_FAP_ID_LSB,
          JER2_ARAD_TDM_FTMH_OPT_UC_DEST_FAP_ID_NOF_BITS,
          &(ftmh->opt_uc.dest_fap_id)
        );
        DNX_SAND_CHECK_FUNC_RESULT(res, 27, exit);

        res = dnx_sand_bitstream_get_any_field(
          &jer2_arad_ire_tdm_config_tbl_data.header[0],
          JER2_ARAD_TDM_FTMH_OPT_UC_DEST_IF_LSB,
          JER2_ARAD_TDM_FTMH_OPT_UC_DEST_IF_NOF_BITS,
          &(ftmh->opt_uc.dest_if)
          );
        DNX_SAND_CHECK_FUNC_RESULT(res, 29, exit);
      }
      else
      {
        /* Optimized & MC */
        *is_mc = TRUE;
        res = dnx_sand_bitstream_get_any_field(
          &jer2_arad_ire_tdm_config_tbl_data.header[0],
          JER2_ARAD_TDM_FTMH_OPT_MC_MC_ID_LSB,
          JER2_ARAD_TDM_FTMH_OPT_MC_MC_ID_NOF_BITS,
          &(ftmh->opt_mc.mc_id)
        );
        DNX_SAND_CHECK_FUNC_RESULT(res, 32, exit);
      }
      
    }
    else
    {
      /* Verify tdm mode is right */
      if (tdm_mode != JER2_ARAD_MGMT_TDM_MODE_TDM_STA)
      {
        DNX_SAND_SET_ERROR_CODE(JER2_ARAD_TDM_INVALID_TDM_MODE_ERR, 33, exit);
      }

      res = dnx_sand_bitstream_get_any_field(
        &jer2_arad_ire_tdm_config_tbl_data.header[0],
        JER2_ARAD_TDM_FTMH_STA_TYPE_START_BIT,
        1,
        &(tmp)
      );
      DNX_SAND_CHECK_FUNC_RESULT(res, 34, exit);

      if (tmp == JER2_ARAD_TDM_FTMH_STA_TYPE_UC)
      {
        /* Standard & UC */
        *is_mc = FALSE;

        tmp = 0;
        res = dnx_sand_bitstream_get_any_field(
          &jer2_arad_ire_tdm_config_tbl_data.header[0],
          JER2_ARAD_TDM_FTMH_STA_UC_FAP_ID_LSB,
          JER2_ARAD_TDM_FTMH_STA_UC_FAP_ID_NOF_BITS,
          &tmp
        );
        DNX_SAND_CHECK_FUNC_RESULT(res, 36, exit);

        ftmh->standard_uc.dest_fap_id = tmp;

        tmp = 0;
        res = dnx_sand_bitstream_get_any_field(
          &jer2_arad_ire_tdm_config_tbl_data.header[0],
          JER2_ARAD_TDM_FTMH_STA_UC_FAP_PORT_ID_LSB,
          JER2_ARAD_TDM_FTMH_STA_UC_FAP_PORT_ID_NOF_BITS,
          &tmp
          );
        DNX_SAND_CHECK_FUNC_RESULT(res, 36, exit);
        ftmh->standard_uc.dest_fap_port = tmp;

        /* User defined */
        res = dnx_sand_bitstream_get_any_field(
          &jer2_arad_ire_tdm_config_tbl_data.header[0],
          JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_1_LSB,
          JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_1_NOF_BITS,
          &(ftmh->standard_uc.user_def)
        );
        DNX_SAND_CHECK_FUNC_RESULT(res, 38, exit);
        
        tmp = 0;
        res = dnx_sand_bitstream_get_any_field(
          &jer2_arad_ire_tdm_config_tbl_data.header[0],
          JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_2_LSB,
          JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_2_NOF_BITS,
          &(tmp)
          );
        DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
        res = dnx_sand_bitstream_set_any_field(
          &tmp,
          JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_1_NOF_BITS,
          JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_2_NOF_BITS,
          &(ftmh->standard_uc.user_def));
        DNX_SAND_CHECK_FUNC_RESULT(res, 41, exit);

        tmp = 0;
        res = dnx_sand_bitstream_get_any_field(
          &jer2_arad_ire_tdm_config_tbl_data.header[0],
          JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_3_LSB,
          JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_3_NOF_BITS,
          &(tmp)
          );
        DNX_SAND_CHECK_FUNC_RESULT(res, 42, exit);
        res = dnx_sand_bitstream_set_any_field(
          &tmp,
          JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_NOF_BITS_PART_1_2,
          JER2_ARAD_TDM_FTMH_STA_UC_USER_DEFINED_3_NOF_BITS,
          &(ftmh->standard_uc.user_def));
        DNX_SAND_CHECK_FUNC_RESULT(res, 43, exit);

        /* User define fields for more than 32bits. Valid only when working in JER2_ARAD FTMH */
        if (!is_petrab_in_system)
        {
          tmp = 0;
          res = dnx_sand_bitstream_get_any_field(
            &jer2_arad_ire_tdm_config_tbl_data.header[0],
            JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_4_LSB,
            JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_4_NOF_BITS,
            &(tmp)
            );
          DNX_SAND_CHECK_FUNC_RESULT(res, 42, exit);
          res = dnx_sand_bitstream_set_any_field(
            &tmp,
            0,
            JER2_ARAD_TDM_JER2_ARAD_FTMH_STA_UC_USER_DEFINED_4_NOF_BITS,
            &(ftmh->standard_uc.user_def_2));
          DNX_SAND_CHECK_FUNC_RESULT(res, 43, exit);
        }
      }
      else
      {
        /* Standard & MC */
        *is_mc = TRUE;
        res = dnx_sand_bitstream_get_any_field(
          &jer2_arad_ire_tdm_config_tbl_data.header[0],
          JER2_ARAD_TDM_FTMH_STA_MC_MC_ID_LSB,
          JER2_ARAD_TDM_FTMH_STA_MC_MC_ID_NOF_BITS,
          &(ftmh->standard_mc.mc_id)
        );
        DNX_SAND_CHECK_FUNC_RESULT(res, 46, exit);

        /* User defined */
        res = dnx_sand_bitstream_get_any_field(
          &jer2_arad_ire_tdm_config_tbl_data.header[0],
          JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_1_LSB,
          JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_1_NOF_BITS,
          &(ftmh->standard_mc.user_def)
        );
        DNX_SAND_CHECK_FUNC_RESULT(res, 48, exit);

        tmp = 0;
        res = dnx_sand_bitstream_get_any_field(
          &jer2_arad_ire_tdm_config_tbl_data.header[0],
          JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_2_LSB,
          JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_2_NOF_BITS,
          &(tmp)
          );
        DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
        res = dnx_sand_bitstream_set_any_field(
          &tmp,
          JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_1_NOF_BITS,
          JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_2_NOF_BITS,
          &(ftmh->standard_mc.user_def));
        DNX_SAND_CHECK_FUNC_RESULT(res, 51, exit);

        tmp = 0;
        res = dnx_sand_bitstream_get_any_field(
          &jer2_arad_ire_tdm_config_tbl_data.header[0],
          JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_3_LSB,
          JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_3_NOF_BITS,
          &(tmp)
          );
        DNX_SAND_CHECK_FUNC_RESULT(res, 52, exit);
        res = dnx_sand_bitstream_set_any_field(
          &tmp,
          JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_NOF_BITS_PART_1_2,
          JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_3_NOF_BITS,
          &(ftmh->standard_mc.user_def));
        DNX_SAND_CHECK_FUNC_RESULT(res, 53, exit);

        tmp = 0;
        res = dnx_sand_bitstream_get_any_field(
          &jer2_arad_ire_tdm_config_tbl_data.header[0],
          JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_4_LSB,
          JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_4_NOF_BITS,
          &(tmp)
          );
        DNX_SAND_CHECK_FUNC_RESULT(res, 54, exit);
        res = dnx_sand_bitstream_set_any_field(
          &tmp,
          JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_NOF_BITS_PART_1_3,
          JER2_ARAD_TDM_FTMH_STA_MC_USER_DEFINED_4_NOF_BITS,
          &(ftmh->standard_mc.user_def));
        DNX_SAND_CHECK_FUNC_RESULT(res, 56, exit);
      }

    }       
    
  }
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_ing_ftmh_get_unsafe()", port_ndx, 0);
}

/*********************************************************************
*     Configure the FTMH header operation
 *     (added/unchanged/removed) at the egress,
 *     with the FTMH fields if added.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_eg_ftmh_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core_id,
    DNX_SAND_IN  uint32                   port_ndx,
    DNX_SAND_OUT JER2_ARAD_TDM_EG_ACTION            *action_eg
  )
{
    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  uint32
    res;
  uint32
    prge_prog_select,
    base_q_pair;
  JER2_ARAD_EGQ_PCT_TBL_DATA
    egq_pct_data;
  uint32 eg_tm_profile;
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_EG_FTMH_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(action_eg);
  

  res = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit,core_id, port_ndx, &base_q_pair);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 130, exit);

  /* EGQ-PCT */
  res = jer2_arad_egq_pct_tbl_get_unsafe(
                unit,
                core_id,
                base_q_pair,
                &(egq_pct_data)
              );
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 140, exit);

  if(soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "user_header_always_remove", 0) == 1) { 
    res = jer2_arad_egr_prog_editor_tm_profile_enum_get(unit, egq_pct_data.prog_editor_profile, &eg_tm_profile);
    DNX_SAND_CHECK_FUNC_RESULT(res, 150, exit);
  }
  else {
    eg_tm_profile = egq_pct_data.prog_editor_profile;
  }

  res = jer2_arad_egr_prog_editor_profile_get(unit, JER2_ARAD_PRGE_TM_SELECT_REMOVE_SYSTEM_HEADER, &prge_prog_select);
  if ((res == SOC_E_NONE) && (eg_tm_profile == prge_prog_select)) {
      *action_eg = JER2_ARAD_TDM_EG_ACTION_REMOVE;
  } else {
      res = jer2_arad_egr_prog_editor_profile_get(unit, JER2_ARAD_PRGE_TM_SELECT_REMOVE_TDM_OPT_FTMH, &prge_prog_select);
      if ((res == SOC_E_NONE) && (eg_tm_profile == prge_prog_select)) {
          *action_eg = JER2_ARAD_TDM_EG_ACTION_REMOVE;
      } else {
          res = jer2_arad_egr_prog_editor_profile_get(unit, JER2_ARAD_PRGE_TM_SELECT_TDM_PMM_HEADER, &prge_prog_select);
          if ((res == SOC_E_NONE) && (eg_tm_profile == prge_prog_select)) {
              *action_eg = JER2_ARAD_TDM_EG_ACTION_CUSTOMER_EXTRACT;
          } else {
              res = jer2_arad_egr_prog_editor_profile_get(unit, JER2_ARAD_PRGE_TM_SELECT_NONE, &prge_prog_select);
              if ((res == SOC_E_NONE) && (eg_tm_profile == prge_prog_select)) {
                  *action_eg = JER2_ARAD_TDM_EG_ACTION_NO_CHANGE;
              } else {
                      *action_eg = JER2_ARAD_TDM_NOF_EG_ACTIONS;
              }
          }
      }
  }
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_ing_ftmh_get_unsafe()", port_ndx, 0);
#endif 
    return -1;
}
/*********************************************************************
*     Configure the FTMH header operation
 *     (added/unchanged/removed) at the ingress and egress,
 *     with the FTMH fields if added.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_ftmh_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core_id,
    DNX_SAND_IN  uint32                   port_ndx,
    DNX_SAND_OUT DNX_TMC_TDM_FTMH_INFO            *info
  )
{
  int local_port;
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_FTMH_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(info);


  DNX_TMC_TDM_FTMH_INFO_clear(info);

  res = dnx_port_sw_db_tm_to_local_port_get(unit, core_id, port_ndx, &local_port);
  DNX_SAND_CHECK_FUNC_RESULT(res, 4, exit);

  res = jer2_arad_tdm_ing_ftmh_get_unsafe(
          unit,
          local_port,
          &info->action_ing,
          &info->ftmh,
          &info->is_mc
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = jer2_arad_tdm_eg_ftmh_get_unsafe(
          unit,
          core_id,
          port_ndx,
          &info->action_eg
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_ftmh_get_unsafe()", port_ndx, 0);
}
/*********************************************************************
*     Configure the size limitations for the TDM cells in the
 *     Optimized FTMH TDM traffic mode.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_opt_size_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                    cell_size
  )
{
  uint32
    res,
    fld_val;
    
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_OPT_SIZE_SET_UNSAFE);

  /* Set optimized TDM packets */
  fld_val = cell_size;
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_2_BYTES_PKT_SIZEf,  fld_val));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_TDM_EPE_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_2_BYTES_PKT_SIZEf,  fld_val));  

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_opt_size_set_unsafe()", cell_size, 0);
}

uint32
  jer2_arad_tdm_opt_size_set_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                    cell_size
  )
{
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_OPT_SIZE_SET_VERIFY);

  DNX_SAND_ERR_IF_OUT_OF_RANGE(cell_size, JER2_ARAD_TDM_CELL_SIZE_MIN, JER2_ARAD_TDM_CELL_SIZE_MAX, JER2_ARAD_TDM_CELL_SIZE_OUT_OF_RANGE_ERR, 10, exit);
  
  /* This API is relevent only in optimized mode. */
  if (jer2_arad_sw_db_tdm_mode_get(unit) != JER2_ARAD_MGMT_TDM_MODE_TDM_OPT)
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_TDM_INVALID_TDM_MODE_ERR, 20, exit);
  }
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_opt_size_set_verify()", cell_size, 0);
}

uint32
  jer2_arad_tdm_opt_size_get_verify(
    DNX_SAND_IN  int                   unit
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_OPT_SIZE_GET_VERIFY);
  
  /* This API is relevent only in optimized mode. */
  if (jer2_arad_sw_db_tdm_mode_get(unit) != JER2_ARAD_MGMT_TDM_MODE_TDM_OPT)
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_TDM_INVALID_TDM_MODE_ERR, 20, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_opt_size_get_verify()", 0, 0);
}

/*********************************************************************
*     Configure the size limitations for the TDM cells in the
 *     Optimized FTMH TDM traffic mode.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_opt_size_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_OUT uint32                    *cell_size
  )
{
  uint32
    res,
    fld_val;     

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_OPT_SIZE_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(cell_size);
  
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_2_BYTES_PKT_SIZEf, &fld_val));
  
  *cell_size = fld_val;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_opt_size_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Configure the size limitations for the TDM cells in the
 *     Standard FTMH TDM traffic mode.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_stand_size_range_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  DNX_SAND_U32_RANGE              *size_range
  )
{
  uint32
    res,
    fld_val;
  int reg;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_STAND_SIZE_RANGE_SET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(size_range);  

  reg = (SOC_IS_QAX(unit)) ? IRE_TDM_CONFIGURATIONSr : IRE_TDM_SIZEr; 

  fld_val = size_range->start;
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20, exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, reg, REG_PORT_ANY, 0, TDM_MIN_SIZEf,  fld_val));

  fld_val = size_range->end;
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30, exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, reg, REG_PORT_ANY, 0, TDM_MAX_SIZEf,  fld_val));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_stand_size_range_set_unsafe()", 0, 0);
}

uint32
  jer2_arad_tdm_stand_size_range_set_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  DNX_SAND_U32_RANGE              *size_range
  )
{
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_STAND_SIZE_RANGE_SET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(size_range->end, JER2_ARAD_TDM_CELL_SIZE_MAX, JER2_ARAD_TDM_CELL_SIZE_OUT_OF_RANGE_ERR, 10, exit);
  DNX_SAND_ERR_IF_BELOW_MIN(size_range->start, JER2_ARAD_TDM_CELL_SIZE_MIN, JER2_ARAD_TDM_CELL_SIZE_OUT_OF_RANGE_ERR, 15, exit);

  /* This API is relevant only in standard mode. */
  if (jer2_arad_sw_db_tdm_mode_get(unit) != JER2_ARAD_MGMT_TDM_MODE_TDM_STA && jer2_arad_sw_db_tdm_mode_get(unit) != JER2_ARAD_MGMT_TDM_MODE_TDM_OPT)
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_TDM_INVALID_TDM_MODE_ERR, 20, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_stand_size_range_set_verify()", 0, 0);
}

uint32
  jer2_arad_tdm_stand_size_range_get_verify(
    DNX_SAND_IN  int                   unit
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_STAND_SIZE_RANGE_GET_VERIFY);
  
  /* This API is relevant only in standard mode. */
  if (jer2_arad_sw_db_tdm_mode_get(unit) != JER2_ARAD_MGMT_TDM_MODE_TDM_STA && jer2_arad_sw_db_tdm_mode_get(unit) != JER2_ARAD_MGMT_TDM_MODE_TDM_OPT)
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_TDM_INVALID_TDM_MODE_ERR, 20, exit);
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_stand_size_range_get_verify()", 0, 0);
}

/*********************************************************************
*     Configure the size limitations for the TDM cells in the
 *     Standard FTMH TDM traffic mode.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_stand_size_range_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_OUT DNX_SAND_U32_RANGE              *size_range
  )
{
  uint32
    res,
    fld_val;
  int reg;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_STAND_SIZE_RANGE_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(size_range);
     
  reg = (SOC_IS_QAX(unit)) ? IRE_TDM_CONFIGURATIONSr : IRE_TDM_SIZEr; 

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20, exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, reg, REG_PORT_ANY, 0, TDM_MIN_SIZEf, &fld_val));
  size_range->start = fld_val;

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30, exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, reg, REG_PORT_ANY, 0, TDM_MAX_SIZEf, &fld_val));
  size_range->end = fld_val;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_stand_size_range_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Set the OFP ports configured as TDM destination.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_arad_tdm_ofp_set_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  soc_port_t            port,
    DNX_SAND_IN  uint8                 is_tdm
  )
{
    uint32
        fld_val,
        is_tdm_val;
    uint8
        is_exists_tdm_port = FALSE;
    JER2_ARAD_MGMT_TDM_MODE
        tdm_mode;   
    soc_reg_above_64_val_t
        data,
        field_val;
    uint32
        i,
        base_q_pair,
        nof_q_pairs,
        curr_q_pair;
    int
        core;
    soc_error_t
        rv;
    DNXC_INIT_FUNC_DEFS;
  
    SOC_REG_ABOVE_64_CLEAR(data);
    SOC_REG_ABOVE_64_CLEAR(field_val);
    if(soc_feature(unit, soc_feature_no_tdm)) {
        SOC_EXIT;
    }    

    tdm_mode = jer2_arad_sw_db_tdm_mode_get(unit);
    is_tdm_val = DNX_SAND_BOOL2NUM(is_tdm);

    /* Retreive base_q_pair & nof_queues for the following DSP_PP port */
    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.base_q_pair.get(unit, port, &base_q_pair);
    DNXC_IF_ERR_EXIT(rv);
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_out_port_priority_get(unit, port, &nof_q_pairs));
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_core_get(unit, port, &core));
  
    DNXC_IF_ERR_EXIT(READ_EGQ_TDM_MAP_QUEUE_TO_TDMr(unit, core, data));

    soc_reg_above_64_field_get(unit, EGQ_TDM_MAP_QUEUE_TO_TDMr, data, TDM_MAP_QUEUE_TO_TDMf,field_val);
   
    /* Add the following settings */
    for (curr_q_pair = base_q_pair; curr_q_pair < base_q_pair+nof_q_pairs; curr_q_pair++)
    {
        SHR_BITCOPY_RANGE(field_val,curr_q_pair,&is_tdm_val,0,1);
    }
    
    /* Set */
    soc_reg_above_64_field_set(unit, EGQ_TDM_MAP_QUEUE_TO_TDMr, data, TDM_MAP_QUEUE_TO_TDMf,field_val);
    DNXC_IF_ERR_EXIT(WRITE_EGQ_TDM_MAP_QUEUE_TO_TDMr(unit, core, data)); 
  
    /* In packet mode, Change TDM enable in EGQ if exists port type tdm */
    if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
        if ((tdm_mode == JER2_ARAD_MGMT_TDM_MODE_PACKET))
        {
            is_exists_tdm_port = is_tdm;
            if (is_exists_tdm_port == FALSE)
            {
                /* Run over all ports and check if port is in tdm type. if not, disable EGQ TDM mode. */
                DNXC_IF_ERR_EXIT(READ_EGQ_TDM_MAP_QUEUE_TO_TDMr(unit, core, data));

                soc_reg_above_64_field_get(unit, EGQ_TDM_MAP_QUEUE_TO_TDMr, data, TDM_MAP_QUEUE_TO_TDMf,field_val);

                for (i = 0; i < JER2_ARAD_EGQ_TDM_MAP_QUEUE_TO_TDM_REG_MULT_NOF_REGS; i++)
                {        
                    fld_val = 0;
                    SHR_BITCOPY_RANGE(&fld_val,0,field_val,i*DNX_SAND_REG_SIZE_BITS,DNX_SAND_REG_SIZE_BITS);
                    if (fld_val != 0)
                    {
                        is_exists_tdm_port = TRUE;
                    }
                }
            }
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GLOBAL_10r, core, 0, EGRESS_TDM_MODEf, is_exists_tdm_port));
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

uint32
  jer2_arad_tdm_ofp_set_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx,
    DNX_SAND_IN  uint8                   is_tdm
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_OFP_SET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(port_ndx, JER2_ARAD_TDM_PORT_NDX_MAX, JER2_ARAD_TDM_PORT_NDX_OUT_OF_RANGE_ERR, 10, exit);
  DNX_SAND_ERR_IF_ABOVE_MAX(is_tdm, JER2_ARAD_TDM_IS_TDM_MAX, JER2_ARAD_TDM_IS_TDM_OUT_OF_RANGE_ERR, 20, exit);
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_ofp_set_verify()", port_ndx, 0);
}

uint32
  jer2_arad_tdm_ofp_get_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_OFP_GET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(port_ndx, JER2_ARAD_TDM_PORT_NDX_MAX, JER2_ARAD_TDM_PORT_NDX_OUT_OF_RANGE_ERR, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_ofp_get_verify()", port_ndx, 0);
}

/*********************************************************************
*     Set the IFP ports configured as TDM destination.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_ofp_get_unsafe(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx,
    DNX_SAND_OUT uint8                   *is_tdm
  )
{
  uint32
    is_tdm_val = 0;
  uint32
    base_q_pair;
  soc_reg_above_64_val_t
    data,
    field_val;
  uint32 res, tm_port;
  int core;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_OFP_GET_UNSAFE);

  SOC_REG_ABOVE_64_CLEAR(data);
  SOC_REG_ABOVE_64_CLEAR(field_val);

  DNX_SAND_CHECK_NULL_INPUT(is_tdm);

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1220, exit, dnx_port_sw_db_local_to_tm_port_get(unit, port_ndx, &tm_port, &core));
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1221, exit, dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_q_pair));
  
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1230, exit, READ_EGQ_TDM_MAP_QUEUE_TO_TDMr(unit, REG_PORT_ANY, data));

  soc_reg_above_64_field_get(unit, EGQ_TDM_MAP_QUEUE_TO_TDMr, data, TDM_MAP_QUEUE_TO_TDMf,field_val);
   
  SHR_BITCOPY_RANGE(&is_tdm_val,0,field_val,base_q_pair,1);
  
  *is_tdm = DNX_SAND_NUM2BOOL(is_tdm_val);
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_ofp_get_unsafe()", port_ndx, 0);
}

/*********************************************************************
*     Set the IFP ports configured as TDM destination.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_arad_tdm_ifp_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  soc_port_t            port,
    DNX_SAND_IN  uint8                 is_tdm
  )
{
    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    uint32
        is_tdm_val = 0,
        find_context_map_val,
        is_vsc_mode_128,
        context_map_val;
    soc_reg_above_64_val_t
        data,
        field_val; 
    uint32
        reassembly_context;
    JER2_ARAD_PORT2IF_MAPPING_INFO
        info_incoming;
    JER2_ARAD_INTERFACE_ID
        used_if_id;

    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);
    SOC_REG_ABOVE_64_CLEAR(field_val);


    /* Tables retreive by reassembly context */
    DNXC_IF_ERR_EXIT(
        lag_state.local_to_reassembly_context.get(unit, port, &reassembly_context));
    
  
    is_tdm_val = DNX_SAND_BOOL2NUM(is_tdm);

    /* 
     * IFP is TDM when:
     * 1. Stamps ftmh version other than 0
     * 2. TDM Mode is set.
     */

    /* 1. Stamps FTMH version other than 0 */
    DNXC_IF_ERR_EXIT(READ_IRE_SET_FTMH_VERSIONr(unit,data));

    soc_reg_above_64_field_get(unit, IRE_SET_FTMH_VERSIONr, data, SET_FTMH_VERSIONf,field_val);
   
    /* Add the following settings */
    SHR_BITCOPY_RANGE(field_val,reassembly_context,&is_tdm_val,0,1);
   
    /* Set */
    soc_reg_above_64_field_set(unit, IRE_SET_FTMH_VERSIONr, data, SET_FTMH_VERSIONf,field_val);
    DNXC_IF_ERR_EXIT(WRITE_IRE_SET_FTMH_VERSIONr(unit,data));

    /* 2. TDM Mode is set */
    DNXC_IF_ERR_EXIT(READ_IRE_TDM_MODE_MAPr(unit,data));

    soc_reg_above_64_field_get(unit, IRE_TDM_MODE_MAPr, data, TDM_MODE_MAPf,field_val);

    /* Add the following settings */
    SHR_BITCOPY_RANGE(field_val,reassembly_context,&is_tdm_val,0,1);

    /* Set */
    soc_reg_above_64_field_set(unit, IRE_TDM_MODE_MAPr, data, TDM_MODE_MAPf,field_val);
    DNXC_IF_ERR_EXIT(WRITE_IRE_TDM_MODE_MAPr(unit,data));

    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, ECI_GLOBALEr, REG_PORT_ANY, 0, GLBL_VSC_128_MODEf, &is_vsc_mode_128));

    if (!is_vsc_mode_128) 
    {  
        /* 
         * TDM context MAP: 
         * In case of VSC 256, Set Different Context for each interface. 
         * interfaces look up a free place. 
         * In TDM there can be up to 2 TDM interfaces (in VSC 256) 
         */
        DNX_SAND_IF_ERR_EXIT(jer2_arad_port_to_interface_map_get(
            unit,
            0,
            port,
            &info_incoming.if_id,
            &info_incoming.channel_id
          ));  

        DNXC_IF_ERR_EXIT(READ_IRE_TDM_CONTEXT_MAPr(unit,data));

        soc_reg_above_64_field_get(unit, IRE_TDM_CONTEXT_MAPr, data, TDM_CONTEXT_MAPf,field_val);

        /* Find context map val { */
        if (is_tdm) 
        {
            context_map_val = -1;
            for (find_context_map_val = 0; find_context_map_val < JER2_ARAD_NOF_TDM_CONTEXT_MAP; find_context_map_val++) {
                DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.tdm.context_map.get(unit,find_context_map_val,&used_if_id));

                if (used_if_id == JER2_ARAD_IF_ID_NONE || used_if_id == info_incoming.if_id) {
                    /* We found a match */
                    context_map_val = find_context_map_val;

                    /* Update SW DB if neccesary */
                    if (used_if_id == JER2_ARAD_IF_ID_NONE) {
                        DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.tdm.context_map.set(unit,find_context_map_val,info_incoming.if_id));
                    }
                    break;
                }
            }

            /* No match */
            if (context_map_val == -1) {
                DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("JER2_ARAD_TDM_OUT_OF_AVAIABLE_TDM_CONTEXT_ERR")));    
            }
        }
        else
        {
            
            context_map_val = JER2_ARAD_TDM_CONTEXT_MAP_DEFAULT_VAL;
        }
        /* Find context map val } */

        /* Add the following settings */
        SHR_BITCOPY_RANGE(field_val,reassembly_context,&context_map_val,0,1);

        /* Set */
        soc_reg_above_64_field_set(unit, IRE_TDM_CONTEXT_MAPr, data, TDM_CONTEXT_MAPf,field_val);
        DNXC_IF_ERR_EXIT(WRITE_IRE_TDM_CONTEXT_MAPr(unit,data));
    }
    
exit:
    DNXC_FUNC_RETURN;
#endif 
    return -1;
}

uint32
  jer2_arad_tdm_ifp_set_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx,
    DNX_SAND_IN  uint8                   is_tdm
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_IFP_SET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(port_ndx, JER2_ARAD_TDM_PORT_NDX_MAX, JER2_ARAD_TDM_PORT_NDX_OUT_OF_RANGE_ERR, 10, exit);
  DNX_SAND_ERR_IF_ABOVE_MAX(is_tdm, JER2_ARAD_TDM_IS_TDM_MAX, JER2_ARAD_TDM_IS_TDM_OUT_OF_RANGE_ERR, 20, exit);
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_ifp_set_verify()", port_ndx, 0);
}

uint32
  jer2_arad_tdm_ifp_get_verify(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  uint32                   port_ndx
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_IFP_GET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(port_ndx, JER2_ARAD_TDM_PORT_NDX_MAX, JER2_ARAD_TDM_PORT_NDX_OUT_OF_RANGE_ERR, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_ifp_get_verify()", port_ndx, 0);
}

/*********************************************************************
*     Set the OFP ports configured as TDM destination.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_arad_tdm_ifp_get(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  soc_port_t      port,
    DNX_SAND_OUT uint8           *is_tdm
  )
{
    uint32 is_tdm_val = 0;
    soc_reg_above_64_val_t data, field_val;    
    uint32 reassembly_context;

    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);
    SOC_REG_ABOVE_64_CLEAR(field_val);

    /* Tables retreive by reassembly context */
    DNXC_IF_ERR_EXIT(jer2_arad_tdm_local_to_reassembly_context_get(unit, port, &reassembly_context));

    DNXC_IF_ERR_EXIT(READ_IRE_TDM_MODE_MAPr(unit, data));

    soc_reg_above_64_field_get(unit, IRE_TDM_MODE_MAPr, data, TDM_MODE_MAPf,field_val);
   
    is_tdm_val = SHR_BITGET(field_val, reassembly_context);
    *is_tdm = DNX_SAND_NUM2BOOL(is_tdm_val);    
  
exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Set the TDM direct routing configuration. Up to
 *     36 routing profiles can be defined. 
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_direct_routing_profile_map_set_unsafe(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  uint32                     port_ndx,
    DNX_SAND_IN  uint32                     direct_routing_profile    
  )
{
  uint32
    res;
  uint32
    reassembly_context;
  JER2_ARAD_IRE_TDM_CONFIG_TBL_DATA
    jer2_arad_ire_tdm_config_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_MAP_SET_UNSAFE);

  res = jer2_arad_tdm_local_to_reassembly_context_get(unit,port_ndx, &reassembly_context);
DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  res = jer2_arad_ire_tdm_config_tbl_get_unsafe(
          unit,
          reassembly_context,
          &jer2_arad_ire_tdm_config_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  jer2_arad_ire_tdm_config_tbl_data.link_mask_ptr = direct_routing_profile;

  res = jer2_arad_ire_tdm_config_tbl_set_unsafe(
          unit,
          reassembly_context,
          &jer2_arad_ire_tdm_config_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_direct_routing_set_unsafe()", port_ndx, 0);
}

uint32
  jer2_arad_tdm_direct_routing_profile_map_set_verify(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  uint32                     port_ndx,
    DNX_SAND_IN  uint32                     direct_routing_profile  
  )
{
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_MAP_SET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(port_ndx, JER2_ARAD_TDM_PORT_NDX_MAX, JER2_ARAD_TDM_PORT_NDX_OUT_OF_RANGE_ERR, 10, exit);
  DNX_SAND_ERR_IF_ABOVE_MAX(direct_routing_profile, JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_NDX_MAX, JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_NDX_OUT_OF_RANGE_ERR, 10, exit);  
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_direct_routing_profile_map_set_verify()", port_ndx, 0);
}

uint32
  jer2_arad_tdm_direct_routing_profile_map_get_verify(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  uint32                     port_ndx
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_MAP_GET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(port_ndx, JER2_ARAD_TDM_PORT_NDX_MAX, JER2_ARAD_TDM_PORT_NDX_OUT_OF_RANGE_ERR, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_direct_routing_profile_map_get_verify()", port_ndx, 0);
}

/*********************************************************************
*   Sets Incoming FAP Port (IFP) routing profile type, per port.
 *   36 routing profiles can be defined. 
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_direct_routing_profile_map_get_unsafe(
    DNX_SAND_IN   int                     unit,
    DNX_SAND_IN   uint32                     port_ndx,
    DNX_SAND_OUT  uint32                    *direct_routing_profile    
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    reassembly_context;
  JER2_ARAD_IRE_TDM_CONFIG_TBL_DATA
    jer2_arad_ire_tdm_config_tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_MAP_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(direct_routing_profile);

  res = jer2_arad_tdm_local_to_reassembly_context_get(unit, port_ndx, &reassembly_context);
  DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  res = jer2_arad_ire_tdm_config_tbl_get_unsafe(
          unit,
          reassembly_context,
          &jer2_arad_ire_tdm_config_tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  *direct_routing_profile = jer2_arad_ire_tdm_config_tbl_data.link_mask_ptr;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_direct_routing_profile_map_get_unsafe()", port_ndx, 0);
}

/*********************************************************************
*     Set the TDM direct routing configuration. Up to
 *     36 routing profiles can be defined. 
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_direct_routing_set_unsafe(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_IN     uint32                     direct_routing_profile,
    DNX_SAND_IN     JER2_ARAD_TDM_DIRECT_ROUTING_INFO *direct_routing_info,
    DNX_SAND_IN     uint8 enable_rpt_reachable
  )
{
  uint32    
    link_bitmap[2] = {0},
    entry_offset,
    data[JER2_ARAD_TDM_FDT_IRE_TDM_MASKS_SIZE],
    fld_val[JER2_ARAD_TDM_FDT_IRE_TDM_MASKS_SIZE],
    fld_val32,
    res = DNX_SAND_OK;
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_DIRECT_ROUTING_SET_UNSAFE);

  sal_memset(data,0x0,sizeof(uint32)*JER2_ARAD_TDM_FDT_IRE_TDM_MASKS_SIZE);
  sal_memset(fld_val,0x0,sizeof(uint32)*JER2_ARAD_TDM_FDT_IRE_TDM_MASKS_SIZE);

  DNX_SAND_CHECK_NULL_INPUT(direct_routing_info);

  /*
   * Get the entry table
   */
  entry_offset = direct_routing_profile;
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1580, exit, READ_FDT_IRE_TDM_MASKSm(unit, MEM_BLOCK_ANY, entry_offset, data));
  
  /*
   * Modify the good part of the line
   */
  link_bitmap[0] = direct_routing_info->link_bitmap.arr[0];
  link_bitmap[1] = direct_routing_info->link_bitmap.arr[1];
  
  res = dnx_sand_bitstream_set_any_field(
          link_bitmap,
          0,
          SOC_DNX_DEFS_GET(unit, nof_fabric_links),
          fld_val
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  /*
   * Set it back
   */
  soc_FDT_IRE_TDM_MASKSm_field_set(unit, data, LINKS_MASKf , fld_val);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1590, exit, WRITE_FDT_IRE_TDM_MASKSm(unit, MEM_BLOCK_ANY, entry_offset, data));
  /*
   * When we work in direct routing,
   * int FDT_LINK_BITMAP_CONFIGURATION register, field IRE_TDM_MASK_MODE has to be set to 3.
   * since we should ignore RTP-All reachable mask.
   *
   * default value of is IRE_TDM_MASK_MODE=2, which means we use the RTP-all reachable. 
   * MODE #2 does not read from RTP - only looks at the link status. 
   * Used for TDM static routing.
   * 
   * Because the user configures at least some of the fabric routing manually, MODE #3 is configured.
   * There is no way to go back from it.
   */
  fld_val32 = JER2_ARAD_TDM_DIRECT_ROUNTING_RPT_REACHABLE_MODE(enable_rpt_reachable);
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  180,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, FDT_LINK_BITMAP_CONFIGURATIONr, REG_PORT_ANY, 0, IRE_TDM_MASK_MODEf,  fld_val32));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_direct_routing_set_unsafe()", direct_routing_profile, 0);
}

uint32
  jer2_arad_tdm_direct_routing_set_verify(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_IN     uint32                     direct_routing_profile,
    DNX_SAND_IN     JER2_ARAD_TDM_DIRECT_ROUTING_INFO  *direct_routing_info    
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_DIRECT_ROUTING_SET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(direct_routing_profile, JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_NDX_MAX, JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_NDX_OUT_OF_RANGE_ERR, 10, exit);
  
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, JER2_ARAD_TDM_DIRECT_ROUTING_INFO_verify(unit, direct_routing_info));
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_direct_routing_set_verify()", direct_routing_profile, 0);
}

uint32
  jer2_arad_tdm_direct_routing_get_verify(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_IN     uint32                     direct_routing_profile
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_DIRECT_ROUTING_GET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(direct_routing_profile, JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_NDX_MAX, JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_NDX_OUT_OF_RANGE_ERR, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_direct_routing_get_verify()", direct_routing_profile, 0);
}

/*********************************************************************
*     Set the TDM direct routing configuration. Up to
 *     36 routing profiles can be defined. 
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_direct_routing_get_unsafe(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_IN     uint32                     direct_routing_profile,
    DNX_SAND_OUT    JER2_ARAD_TDM_DIRECT_ROUTING_INFO *direct_routing_info,
    DNX_SAND_OUT    uint8 *enable_rpt_reachable
  )
{
  uint32
    start_bit,
    link_bitmap[2] = {0},
    entry_offset,
    data[JER2_ARAD_TDM_FDT_IRE_TDM_MASKS_SIZE],
    fld_val[JER2_ARAD_TDM_FDT_IRE_TDM_MASKS_SIZE],
    fld_val32,
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_DIRECT_ROUTING_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(direct_routing_info);
  DNX_TMC_TDM_DIRECT_ROUTING_INFO_clear(direct_routing_info);

  DNX_SAND_CHECK_NULL_INPUT(enable_rpt_reachable);

  /*
   * Get the entry table
   */  
  entry_offset = direct_routing_profile;
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1690, exit, READ_FDT_IRE_TDM_MASKSm(unit, MEM_BLOCK_ANY, entry_offset, data));
  soc_FDT_IRE_TDM_MASKSm_field_get(unit, data, LINKS_MASKf , fld_val);


  /*
   * Modify the good part of the line
   */
  start_bit = 0;
  res = dnx_sand_bitstream_get_any_field(
          fld_val,
          start_bit,
          SOC_DNX_DEFS_GET(unit, nof_fabric_links),
          link_bitmap
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  direct_routing_info->link_bitmap.arr[0] = link_bitmap[0];
  direct_routing_info->link_bitmap.arr[1] = link_bitmap[1];

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  180,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_read(unit, FDT_LINK_BITMAP_CONFIGURATIONr, REG_PORT_ANY, 0, IRE_TDM_MASK_MODEf,  &fld_val32));  
  *enable_rpt_reachable = JER2_ARAD_TDM_DIRECT_ROUNTING_RPT_REACHABLE_IS_ENABLED(fld_val32);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_direct_routing_get_unsafe()", direct_routing_profile, 0);
}

/*********************************************************************
*     Enable generating and removing fabric CRC Per FAP Port. 
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_port_packet_crc_set_unsafe(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  soc_port_t                 port_ndx,
    DNX_SAND_IN  uint8                      is_enable,         /* value to configure (is CRC added in fabric) */
    DNX_SAND_IN  uint8                      configure_ingress, /* should ingress be configured */
    DNX_SAND_IN  uint8                      configure_egress   /* should egress be configured */
  )
{
  uint32
    res = DNX_SAND_OK,
    is_crc_enable_val = DNX_SAND_BOOL2NUM(is_enable);
    int core=0;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_PORT_PACKET_CRC_SET_UNSAFE);

  if (configure_ingress) {
      /* Ingress: IRE add packet crc */
      uint32 reassembly_context;
      JER2_ARAD_IRE_TDM_CONFIG_TBL_DATA jer2_arad_ire_tdm_config_tbl_data;

      res = jer2_arad_tdm_local_to_reassembly_context_get(unit, port_ndx, &reassembly_context);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

      res = jer2_arad_ire_tdm_config_tbl_get_unsafe(unit, reassembly_context, &jer2_arad_ire_tdm_config_tbl_data);
      DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      jer2_arad_ire_tdm_config_tbl_data.add_packet_crc = is_crc_enable_val;

      res = jer2_arad_ire_tdm_config_tbl_set_unsafe(unit, reassembly_context, &jer2_arad_ire_tdm_config_tbl_data);
      DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  }

    if(!SOC_IS_JERICHO(unit) && configure_egress) {
      /* Egress: EGQ remove packet crc*/

      uint32 base_q_pair;
      soc_reg_above_64_val_t data, field_val; 

      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1780, exit, dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, port_ndx,  &base_q_pair));
      
      SOC_REG_ABOVE_64_CLEAR(data);
      SOC_REG_ABOVE_64_CLEAR(field_val);

      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1790, exit, READ_EPNI_TDM_EN_CRC_PER_PORTr(unit, data));
      soc_reg_above_64_field_get(unit, EPNI_TDM_EN_CRC_PER_PORTr, data, TDM_EN_CRC_PER_PORTf, field_val);
   
      /* Add the following settings */
      SHR_BITCOPY_RANGE(field_val, base_q_pair, &is_crc_enable_val, 0, 1);
   
      /* Set */
      soc_reg_above_64_field_set(unit, EPNI_TDM_EN_CRC_PER_PORTr, data, TDM_EN_CRC_PER_PORTf,field_val);
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1800, exit, WRITE_EPNI_TDM_EN_CRC_PER_PORTr(unit,data));
    }
  
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_port_packet_crc_set_unsafe()", port_ndx, 0);
}

uint32
  jer2_arad_tdm_port_packet_crc_set_verify(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  uint32                     port_ndx,
    DNX_SAND_IN  uint8                      is_enable  
  )
{
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_PORT_PACKET_CRC_SET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(port_ndx, JER2_ARAD_TDM_PORT_NDX_MAX, JER2_ARAD_TDM_PORT_NDX_OUT_OF_RANGE_ERR, 10, exit);
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_port_packet_crc_set_verify()", port_ndx, 0);
}

uint32
  jer2_arad_tdm_port_packet_crc_get_verify(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  uint32                     port_ndx
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_PORT_PACKET_CRC_GET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(port_ndx, JER2_ARAD_TDM_PORT_NDX_MAX, JER2_ARAD_TDM_PORT_NDX_OUT_OF_RANGE_ERR, 10, exit); 

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_port_packet_crc_get_verify()", port_ndx, 0);
}

/*********************************************************************
*   Sets Incoming FAP Port (IFP) routing profile type, per port.
 *   36 routing profiles can be defined. 
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_tdm_port_packet_crc_get_unsafe(
    DNX_SAND_IN   int                     unit,
    DNX_SAND_IN   soc_port_t                 port_ndx,
    DNX_SAND_OUT  uint8                      *is_ingress_enabled,
    DNX_SAND_OUT  uint8                      *is_egress_enabled
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    reassembly_context;
  JER2_ARAD_IRE_TDM_CONFIG_TBL_DATA
    jer2_arad_ire_tdm_config_tbl_data;
  int core=0;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TDM_PORT_PACKET_CRC_GET_UNSAFE);

  if (is_ingress_enabled) { /* get ingress configuration */
      res = jer2_arad_tdm_local_to_reassembly_context_get(unit, port_ndx, &reassembly_context);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
      res = jer2_arad_ire_tdm_config_tbl_get_unsafe(unit, reassembly_context, &jer2_arad_ire_tdm_config_tbl_data);
      DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
      *is_ingress_enabled = DNX_SAND_NUM2BOOL(jer2_arad_ire_tdm_config_tbl_data.add_packet_crc);
  }

    if(!SOC_IS_JERICHO(unit) && is_egress_enabled) {
      soc_reg_above_64_val_t data, field_val;

      uint32 base_q_pair;
      int results;
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 100, exit, dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, port_ndx,  &base_q_pair));

      SOC_REG_ABOVE_64_CLEAR(data);
      SOC_REG_ABOVE_64_CLEAR(field_val);
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_EPNI_TDM_EN_CRC_PER_PORTr(unit, data));
      soc_reg_above_64_field_get(unit, EPNI_TDM_EN_CRC_PER_PORTr, data, TDM_EN_CRC_PER_PORTf, field_val);      
      SHR_BITTEST_RANGE(field_val, base_q_pair, 1, results);
      *is_egress_enabled = results ? TRUE : FALSE;
    } else if (SOC_IS_JERICHO(unit)) {
        *is_egress_enabled = TRUE;
    }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_tdm_port_packet_crc_get_unsafe()", port_ndx, 0);
}

#if JER2_ARAD_DEBUG_IS_LVL1

uint32
  JER2_ARAD_TDM_FTMH_OPT_UC_verify(
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_OPT_UC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_SAND_ERR_IF_ABOVE_MAX(info->dest_if, JER2_ARAD_TDM_FTMH_OPT_UC_DEST_IF_MAX, JER2_ARAD_TDM_DEST_IF_OUT_OF_RANGE_ERR, 10, exit);
  DNX_SAND_ERR_IF_ABOVE_MAX(info->dest_fap_id, JER2_ARAD_TDM_FTMH_OPT_UC_DEST_FAP_ID_MAX, JER2_ARAD_TDM_DEST_FAP_ID_OUT_OF_RANGE_ERR, 11, exit);

  DNX_SAND_MAGIC_NUM_VERIFY(info);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in JER2_ARAD_TDM_FTMH_OPT_UC_verify()",0,0);
}

uint32
  JER2_ARAD_TDM_FTMH_OPT_MC_verify(
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_OPT_MC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_SAND_ERR_IF_ABOVE_MAX(info->mc_id, JER2_ARAD_TDM_FTMH_OPT_MC_MC_ID_MAX, JER2_ARAD_TDM_MC_ID_OUT_OF_RANGE_ERR, 10, exit);

  DNX_SAND_MAGIC_NUM_VERIFY(info);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in JER2_ARAD_TDM_FTMH_OPT_MC_verify()",0,0);
}

uint32
  JER2_ARAD_TDM_FTMH_STANDARD_UC_verify(
    DNX_SAND_IN  JER2_ARAD_TDM_FTMH_STANDARD_UC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_SAND_ERR_IF_ABOVE_MAX(info->dest_fap_id, JER2_ARAD_TDM_FTMH_STANDARD_UC_DEST_FAP_ID_MAX, JER2_ARAD_TDM_DEST_FAP_ID_OUT_OF_RANGE_ERR, 11, exit);
  DNX_SAND_ERR_IF_ABOVE_MAX(info->dest_fap_port, JER2_ARAD_TDM_FTMH_STANDARD_UC_DEST_FAP_PORT_MAX, JER2_ARAD_TDM_DEST_FAP_PORT_OUT_OF_RANGE_ERR, 13, exit);
  DNX_SAND_ERR_IF_ABOVE_MAX(info->user_def_2, JER2_ARAD_TDM_FTMH_STANDARD_UC_USER_DEFINE_2_MAX, JER2_ARAD_TDM_USER_DEFINE_2_OUT_OF_RANGE_ERR, 15, exit);

  DNX_SAND_MAGIC_NUM_VERIFY(info);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in JER2_ARAD_TDM_FTMH_STANDARD_UC_verify()",0,0);
}

uint32
  JER2_ARAD_TDM_FTMH_STANDARD_MC_verify(
    DNX_SAND_IN  JER2_ARAD_TDM_FTMH_STANDARD_MC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_SAND_ERR_IF_ABOVE_MAX(info->mc_id, JER2_ARAD_TDM_FTMH_STANDARD_MC_MC_ID_MAX, JER2_ARAD_TDM_MC_ID_OUT_OF_RANGE_ERR, 11, exit);
  DNX_SAND_ERR_IF_ABOVE_MAX(info->user_def, JER2_ARAD_TDM_FTMH_STANDARD_MC_USER_DEF_MAX, JER2_ARAD_TDM_MC_USER_DEF_OUT_OF_RANGE_ERR, 15, exit);

  DNX_SAND_MAGIC_NUM_VERIFY(info);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in JER2_ARAD_TDM_FTMH_STANDARD_MC_verify()",0,0);
}

uint32
  JER2_ARAD_TDM_FTMH_INFO_verify(
    DNX_SAND_IN  DNX_TMC_TDM_FTMH_INFO *info
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_SAND_ERR_IF_ABOVE_MAX(info->action_ing, JER2_ARAD_TDM_FTMH_INFO_ACTION_ING_MAX, JER2_ARAD_TDM_ACTION_ING_OUT_OF_RANGE_ERR, 10, exit);
  DNX_SAND_ERR_IF_ABOVE_MAX(info->action_eg, JER2_ARAD_TDM_FTMH_INFO_ACTION_EG_MAX, JER2_ARAD_TDM_ACTION_EG_OUT_OF_RANGE_ERR, 12, exit);

  DNX_SAND_MAGIC_NUM_VERIFY(info);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in JER2_ARAD_TDM_FTMH_INFO_verify()",0,0);
}
uint32
  JER2_ARAD_TDM_DIRECT_ROUTING_INFO_verify(
    DNX_SAND_IN  uint32 unit,
    DNX_SAND_IN  JER2_ARAD_TDM_DIRECT_ROUTING_INFO *info
  )
{
  int port;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  SHR_BIT_ITER(info->link_bitmap.arr , SOC_DNX_DEFS_GET(unit, nof_fabric_links)  , port){
      if (!IS_SFI_PORT(unit, port + FABRIC_LOGICAL_PORT_BASE(unit))){
          DNX_SAND_SET_ERROR_CODE(JER2_ARAD_TDM_LINK_BITMAP_OUT_OF_RANGE_ERR, 10, exit);
      }
  }

  DNX_SAND_MAGIC_NUM_VERIFY(info);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in JER2_ARAD_TDM_DIRECT_ROUTING_INFO_verify()",0,0);
}
#endif /* JER2_ARAD_DEBUG_IS_LVL1 */
/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88690_A0) */

