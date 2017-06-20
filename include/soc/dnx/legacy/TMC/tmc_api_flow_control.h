/*
 * $Id: jer2_jer2_jer2_tmc_api_flow_control.h,v 1.19 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef __DNX_TMC_API_FLOW_CONTROL_INCLUDED__
/* { */
#define __DNX_TMC_API_FLOW_CONTROL_INCLUDED__


/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>

#include <soc/dnx/legacy/TMC/tmc_api_general.h>
#include <soc/dnx/legacy/TMC/tmc_api_ingress_traffic_mgmt.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*
 * Calendar Interface ID
 *   Arad: Interlaken(0-1) OOB(0-1)
 *   Jer:  Interlaken(0-6) OOB(0-1)
 *
 */
typedef uint32 DNX_TMC_FC_CAL_IF_ID;

#define DNX_TMC_FC_NOF_PP_INTF     2

#define DNX_TMC_FC_OOB_CAL_LEN_MAX 512

#define DNX_TMC_FC_PFC_GENERIC_BITMAP_SIZE 256

#define DNX_TMC_FC_HCFC_BITMAP_SIZE 512

#define DNX_TMC_FC_ILKN_MUB_GEN_CAL_LEN 8

#define  DNX_TMC_FC_OOB_CAL_REP_MIN 1
#define  DNX_TMC_FC_OOB_CAL_REP_MAX 15

#define  DNX_TMC_FC_HCFC_CHANNEL_NUM 5

/* 2^13 */
#define  DNX_TMC_FC_CAL_PAUSE_RESOLUTION_CLK_MAX    0x2000

typedef enum  
{
  DNX_TMC_FC_OOB_TX_SPEED_CORE_2 = 0,
    
  DNX_TMC_FC_OOB_TX_SPEED_CORE_4 = 1,
  
  DNX_TMC_FC_OOB_TX_SPEED_CORE_8 = 2,
  
  DNX_TMC_FC_NOF_OOB_TX_SPEED = 3
}DNX_TMC_FC_OOB_TX_SPEED;

typedef enum
{
    DNX_TMC_FC_EN = 0,

    DNX_TMC_FC_EGQ_TO_NIF_CNM_LLFC_EN = 1, 

    DNX_TMC_FC_EGQ_TO_NIF_CNM_PFC_EN = 2, 

    DNX_TMC_FC_EGQ_TO_SCH_DEVICE_EN = 3,

    DNX_TMC_FC_EGQ_TO_SCH_ERP_EN = 4, 
    
    DNX_TMC_FC_EGQ_TO_SCH_ERP_TC_EN = 5, 

    DNX_TMC_FC_EGQ_TO_SCH_IF_EN = 6,
    
    DNX_TMC_FC_EGQ_TO_SCH_PFC_EN = 7,
    
    DNX_TMC_FC_GLB_RSC_TO_EGQ_RCL_PFC_EN = 8, 

    DNX_TMC_FC_GLB_RSC_TO_HCFC_HP_CFG = 9, 

    DNX_TMC_FC_GLB_RSC_TO_HCFC_LP_CFG = 10, 

    DNX_TMC_FC_GLB_RSC_TO_NIF_LLFC_EN = 11, 
    
    DNX_TMC_FC_GLB_RSC_TO_NIF_PFC_EN = 12, 

    DNX_TMC_FC_GLB_RSC_TO_RCL_PFC_HP_CFG = 13, 

    DNX_TMC_FC_GLB_RSC_TO_RCL_PFC_LP_CFG = 14, 

    DNX_TMC_FC_ILKN_RX_TO_EGQ_PFC_EN = 15, 

    DNX_TMC_FC_ILKN_RX_TO_EGQ_PORT_EN = 16, 
    
    DNX_TMC_FC_ILKN_RX_TO_GEN_PFC_EN = 17, 

    DNX_TMC_FC_ILKN_RX_TO_NIF_FAST_LLFC_EN = 18, 

    DNX_TMC_FC_ILKN_RX_TO_RET_REQ_EN = 19, 

    DNX_TMC_FC_LLFC_VSQ_TO_NIF_EN = 20, 
    
    DNX_TMC_FC_NIF_TO_GEN_PFC_EN = 21, 

    DNX_TMC_FC_PFC_VSQ_TO_NIF_EN = 22, 

    DNX_TMC_FC_SCH_OOB_RX_EN = 23, 

    DNX_TMC_FC_SPI_OOB_RX_TO_EGQ_PFC_EN = 24, 

    DNX_TMC_FC_SPI_OOB_RX_TO_EGQ_PORT_EN = 25, 

    DNX_TMC_FC_SPI_OOB_RX_TO_GEN_PFC_EN = 26, 

    DNX_TMC_FC_SPI_OOB_RX_TO_NIF_FAST_LLFC_EN = 27, 

    DNX_TMC_FC_SPI_OOB_RX_TO_RET_REQ_EN = 28, 

    DNX_TMC_FC_STAT_VSQ_TO_HCFC_EN = 29,

    DNX_TMC_FC_EGQ_TO_SCH_LAG_EN = 30,
    DNX_TMC_FC_NOF_ENABLEs = 31
}DNX_TMC_FC_ENABLE;

typedef enum
{
  /*
   *  Out Of Band Flow Control Interface A. Used for Flow
   *  Control reception or generation.
   */
  DNX_TMC_FC_OOB_ID_A = 0,
  /*
   *  Out Of Band Flow Control Interface B. Used for Flow
   *  Control reception or generation.
   */
  DNX_TMC_FC_OOB_ID_B = 1,
  /*
   *  Number of types in DNX_TMC_FC_OOB_ID
   */
  DNX_TMC_FC_NOF_OOB_IDS = 2
}DNX_TMC_FC_OOB_ID;

typedef enum
{
  /*
   *  Calendar-based Flow Control Mode: SPI-like Out-Of-Band
   */
  DNX_TMC_FC_CAL_MODE_SPI_OOB = 0,
  /*
   *  Calendar-based Flow Control Mode: Interlaken Inband
   */
  DNX_TMC_FC_CAL_MODE_ILKN_INBND = 1,
  /*
   *  Calendar-based Flow Control Mode: Interlaken Out-Of-Band
   */
  DNX_TMC_FC_CAL_MODE_ILKN_OOB = 2,
  /*
   *  Number of types in DNX_TMC_FC_CAL_MODE
   */
  DNX_TMC_FC_NOF_CAL_MODES = 3
}DNX_TMC_FC_CAL_MODE;

typedef enum
{

  DNX_TMC_FC_CAL_TYPE_NONE = 0,
  DNX_TMC_FC_CAL_TYPE_SPI  = 1,
  DNX_TMC_FC_CAL_TYPE_ILKN = 2,
  DNX_TMC_FC_CAL_TYPE_HCFC = 3,
  DNX_TMC_FC_CAL_TYPE_COE  = 4,
  DNX_TMC_FC_CAL_TYPE_E2E  = 5,
  DNX_TMC_FC_NOF_TYPES
}DNX_TMC_FC_OOB_TYPE;

typedef enum
{
  DNX_TMC_FC_CAL_INB_TYPE_NONE = 0,
  DNX_TMC_FC_CAL_INB_TYPE_ILKN = 1,
  DNX_TMC_FC_CAL_INB_TYPE_COE  = 2,
  DNX_TMC_FC_CAL_INB_TYPE_E2E  = 3,
  DNX_TMC_FC_CAL_INB_NOF_TYPES
}DNX_TMC_FC_CAL_INB_TYPE;

typedef enum
{
  DNX_TMC_FC_COE_MODE_PAUSE  = 0,
  DNX_TMC_FC_COE_MODE_PFC = 1,
  DNX_TMC_FC_NOF_COE_MODES = 2
}DNX_TMC_FC_COE_MODE;

typedef enum
{
  DNX_TMC_FC_E2E_STATUS_SIZE_8B  = 0,
  DNX_TMC_FC_E2E_STATUS_SIZE_16B = 1,
  DNX_TMC_FC_E2E_STATUS_SIZE_24B = 2,
  DNX_TMC_FC_E2E_STATUS_SIZE_32B = 3,
  DNX_TMC_FC_E2E_STATUS_SIZE_48B = 4,
  DNX_TMC_FC_E2E_STATUS_SIZE_64B = 5,
  DNX_TMC_FC_NOF_E2E_STATUS_SIZE
}DNX_TMC_FC_E2E_STATUS_TYPE;

typedef enum
{
  /*
   *  Disable Ingress Flow Control generation via NIF, upon
   *  Global Resources High Priority FC indication.
   */
  DNX_TMC_FC_INGR_GEN_GLB_HP_MODE_NONE = 0,
  /*
   *  Enable Ingress Flow Control generation via NIF, upon
   *  Global Resources High Priority FC indication, Link
   *  Level.
   */
  DNX_TMC_FC_INGR_GEN_GLB_HP_MODE_LL = 1,
  /*
   *  Enable Ingress Flow Control generation via NIF, upon
   *  Global Resources High Priority FC indication, Class
   *  Based.
   */
  DNX_TMC_FC_INGR_GEN_GLB_HP_MODE_CB = 2,

  DNX_TMC_FC_INGR_GEN_GLB_HP_MODE_PFC = 3,

  /* All the GLB HP options: LL+CB, LL+PFC */
  DNX_TMC_FC_INGR_GEN_GLB_HP_MODE_ALL = 4,
  /*
   *  Number of types in DNX_TMC_FC_INGR_GEN_GLB_HP_MODE
   */
  DNX_TMC_FC_NOF_INGR_GEN_GLB_HP_MODES
}DNX_TMC_FC_INGR_GEN_GLB_HP_MODE;

typedef enum
{
  /*
   *  CBFC Inheritance is disabled. Upon receiving
   *  (generating) FC of a certain class, only this class is
   *  handled
   */
  DNX_TMC_FC_INBND_PFC_INHERIT_DISABLED = 0,
  /*
   *  If TRUE, CBFC classes will affect lower priority
   *  classes, where the highest priority is class 0
   */
  DNX_TMC_FC_INBND_PFC_INHERIT_UP = 1,
  /*
   *  If TRUE, CBFC classes will affect lower priority
   *  classes, where the highest priority is class 7
   */
  DNX_TMC_FC_INBND_PFC_INHERIT_DOWN = 2,
  /*
   *  Number of types in DNX_TMC_FC_INBND_CB_INHERIT
   */
  DNX_TMC_FC_NOF_INBND_PFC_INHERITS = 3
}DNX_TMC_FC_INBND_PFC_INHERIT;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Selects whether, and in which way, the CBFC affects
   *  lower priority classes
   */
  DNX_TMC_FC_INBND_PFC_INHERIT inherit;
  /*
   *  If bit 'b' is set, the appropriate HR in range 128-255
   *  will stop credit generation. For XAUI/SPAUI - 8 HRs can
   *  be set as a reaction point for 8 FC classes,
   *  accordingly. For GMII - 2 HRs (2 FC classes)The index of
   *  the HR to stop, for FC-Class K, is:For XAUI/SPAUI
   *  (NIF-ID = MAL-ID*4), 128 + 8*MAL-ID + K. For GMII, 128 +
   *  2 * NIF-ID + K
   */
  uint8 sch_hr_bitmap;

} DNX_TMC_FC_REC_INBND_PFC;

typedef enum
{
  /*
   *  Flow Control Reception Reaction Point on OFP level -
   *  Scheduler Port. Note 1: this option is not recommended
   *  to stop the OFP! Typically, the OFP is stopped in the
   *  EGQ, which then backpressures the scheduler according to
   *  the EGQ FC threshold configuration. Note 2: this option
   *  can be used to stop HR 0 - 79, High or Low priority FC,
   *  also if the matching OFP is not used, and the HR
   *  scheduler is used inside the scheduling hierarchy.
   */
  DNX_TMC_FC_REC_OFP_RP_SCH = 0x1,
  /*
   *  Flow Control Reception Reaction Point on OFP level -
   *  Egress Queues Manager (EGQ)
   */
  DNX_TMC_FC_REC_OFP_RP_EGQ = 0x2,
  /*
   *  Number of types in DNX_TMC_FC_REC_OFP_RP
   */
  DNX_TMC_FC_NOF_REC_OFP_RPS = 3
}DNX_TMC_FC_REC_OFP_RP;

typedef enum
{
  /*
   *  OFP FC priority - No FC
   */
  DNX_TMC_FC_OFP_PRIORITY_NONE = 0x0,
  /*
   *  OFP FC priority - Low
   */
  DNX_TMC_FC_OFP_PRIORITY_LP = 0x1,
  /*
   *  OFP FC priority - High. HP-FC triggers also LP-FC
   */
  DNX_TMC_FC_OFP_PRIORITY_HP = 0x3,
  /*
   *  Number of types in DNX_TMC_FC_OFP_PRIORITY
   */
  DNX_TMC_FC_NOF_OFP_PRIORITYS = 3
}DNX_TMC_FC_OFP_PRIORITY;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Flow Control Reception Reaction Point on OFP level -
   *  Scheduler/EGQ
   */
  DNX_TMC_FC_REC_OFP_RP react_point;
  /*
   *  OFP index. Range: 0 - 79.
   */
  uint32 ofp_ndx;
  /*
   *  FC Priority: high/low
   */
  DNX_TMC_FC_OFP_PRIORITY priority;

} DNX_TMC_FC_REC_OFP_MAP_INFO;

typedef enum
{
  /*
   *  Inband Flow Control disabled in the specified direction
   *  (generation/reception)
   */
  DNX_TMC_FC_INBND_MODE_DISABLED = 0,
  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_INBND_MODE_LL = 1,
  /*
   *  Class-based Flow Control. The number of Flow Control
   *  Classes depends on the NIF type: 2 classes for 1Gbps
   *  interface (SGMII), 8 classes for 10Gbps interfaces
   *  (XAUI/SPAUI)
   */
  DNX_TMC_FC_INBND_MODE_CB = 2,
  /*
   *  Number of types in DNX_TMC_FC_INBND_MODE - petra B
   */
  DNX_TMC_FC_NOF_INBND_MODES_PB = 3,
   /* JER2_ARAD ONLY */
   DNX_TMC_FC_INBND_MODE_PFC = DNX_TMC_FC_NOF_INBND_MODES_PB,
   
   DNX_TMC_FC_INBND_MODE_SAFC = 4,

   DNX_TMC_FC_INBND_MODE_DEVICE_DISABLED = 5,

   DNX_TMC_FC_NOF_INBND_MODES = 6
}DNX_TMC_FC_INBND_MODE;

typedef enum
{
  /*
   *  Inband Flow Control disabled in the specified direction
   *  (generation/reception)
   */
  DNX_TMC_FC_GEN_SRC_NONE = 0,
  /*
   *  FC Generation Source: VSQ LLFC
   */
  DNX_TMC_FC_GEN_SRC_VSQ_LLFC = 1,
  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_GEN_SRC_VSQ_PFC = 2,
  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_GEN_SRC_GLB_HIGH = 3,
  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_GEN_SRC_GLB_LOW = 4,
  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_GEN_SRC_VSQ_CAT2TC = 5,
  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_GEN_SRC_NIF = 6,
  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_GEN_SRC_CNM = 7,

  DNX_TMC_FC_NOF_GEN_SRC = 18
}DNX_TMC_FC_GEN_SRC;


typedef enum
{
  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_GLB_RES_TYPE_BDB = 0,
  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_GLB_RES_TYPE_MINI_MC_DB = 1,
  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_GLB_RES_TYPE_MC_DB = 2,
  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_GLB_RES_TYPE_OCB_DB = 3,
  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_GLB_RES_TYPE_MIX_P0 = 4,
  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_GLB_RES_TYPE_MIX_P1 = 5,
  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_GLB_RES_TYPE_OCB_P0 = 6,
  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_GLB_RES_TYPE_OCB_P1 = 7,  

  /*
   *  Link-level Flow Control
   */
  DNX_TMC_FC_GLB_RES_TYPE_OCB = 8,

  DNX_TMC_FC_GLB_RES_TYPE_DRAM = 9,

  DNX_TMC_FC_GLB_RES_TYPE_POOL0 = 10,

  DNX_TMC_FC_GLB_RES_TYPE_POOL1 = 11,

  DNX_TMC_FC_GLB_RES_TYPE_OCB_HEADROOM = 12,

  DNX_TMC_FC_NOF_GLB_RES_TYPE = 13
}DNX_TMC_FC_GLB_RES_TYPE;

typedef enum
{
  /*
   *  CBFC Inheritance is disabled. Upon receiving
   *  (generating) FC of a certain class, only this class is
   *  handled
   */
  DNX_TMC_FC_INBND_CB_INHERIT_DISABLED = 0,
  /*
   *  If TRUE, CBFC classes will affect lower priority
   *  classes, where the highest priority is class 0
   */
  DNX_TMC_FC_INBND_CB_INHERIT_UP = 1,
  /*
   *  If TRUE, CBFC classes will affect lower priority
   *  classes, where the highest priority is class 7
   */
  DNX_TMC_FC_INBND_CB_INHERIT_DOWN = 2,
  /*
   *  Number of types in DNX_TMC_FC_INBND_CB_INHERIT
   */
  DNX_TMC_FC_NOF_INBND_CB_INHERITS = 3
}DNX_TMC_FC_INBND_CB_INHERIT;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

  DNX_TMC_FC_GEN_SRC gen_src;
  
  /*
   *  Selects whether, and in which way, the CBFC affects
   *  lower priority classes
   */
  DNX_TMC_FC_INBND_PFC_INHERIT inherit;

  /*
   *  If TRUE, PFC is generated based on VSQ PFC. 
   */
  uint8 vsq_pfc_enable;

  /*
   *  Jericho Only.
   */
  uint32 is_ocb_only;


  /*
   *   
   */
  uint32 glbl_rcs_pool;
  
  /*
   *  A bitmap that specifies the classes for CBFC generation.
   *  Controls FC generation upon Global Resources consumption
   *  Low-Level FC. 
   */
  uint32 glbl_rcs_low;

  /*
   *  A bitmap that specifies the classes for CBFC generation.
   *  Controls FC generation upon Global Resources consumption
   *  Low-Level FC. 
   */
  uint32 glbl_rcs_high;

  
  /*
   *  If TRUE, CBFC is generated based on CNM messages. The
   *  relevant NIF will generate FC, based on the 3-LSB of the
   *  CPID. These 3-LSB are expected to represent TC. Note 1: if
   *  port-to-interface mapping for the relevant NIF is
   *  modified, this configuration must be reset after the
   *  modification. Note 2: refer to CNM module APIs for CNM
   *  messages handling configuration
   */
  uint8 cnm_intercept_enable;

  uint32 cnm_pfc_channel;
  /*
   *  8-bits bitmap. If bit 'b' is set, the NIF will generate
   *  FC on class 'b' when lower internal NIF-FC threshold is
   *  crossed. Note: for GMII interface, only bits 0, 1 are
   *  relevant (since only two FC-classes are supported).
   *  Note: when higher internal NIF-FC threshold is crossed,
   *  FC is generated on all classes.
   */
  uint8 nif_cls_bitmap;

  uint8 cat2_tc;

  uint32 cat2_tc_bitmap;  

} DNX_TMC_FC_GEN_INBND_PFC;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Enable/Disable the Calendar-based interface for FC
   *  generation (TX) or reception (RX). Can be enabled as
   *  SPI-OOB, ILKN-OOB or ILKN-Inbnd. OOB-TX enable is only
   *  valid for OOB interface B.
   */
  uint8 enable;
  /*
   *  Number of channels in calendar. SPI Range: 0 - 511. ILKN
   *  Range: 0 - 255.
   */
  uint32 cal_len;
  /*
   *  The number of calendar repetitions within a status
   *  frame. The actual Calendar is composed of adjacent
   *  sections of 'cal_len' length.'cal_len' * 'cal_reps' must
   *  not exceed the total calendar length (512/256). Range: 1
   *  - 15.
   */
  uint32 cal_reps;

} DNX_TMC_FC_CAL_IF_INFO;

typedef enum
{
  DNX_TMC_FC_GEN_CAL_SRC_STE = 0,
  /*
   *  Calendar-based Flow Control source - Index Range
   *  (NIF-ID): 0 - 63.
   */
  DNX_TMC_FC_GEN_CAL_SRC_NIF = 1,
  /*
   *  Calendar-based Flow Control source - Global Resource,
   *  High Priority FC. Index Range (Glbl-Rcs-HP-Id): 0 - 2.
   */
  DNX_TMC_FC_GEN_CAL_SRC_GLB_HP = 2,
  /*
   *  Calendar-based Flow Control source - Index Range
   *  (Glbl-Rcs-LP-Id): 0 - 2.
   */
  DNX_TMC_FC_GEN_CAL_SRC_GLB_LP = 3,
  /*
   *  Invalid/non-existing source. Serves as "empty calendar
   *  entry" indication.
   */
  DNX_TMC_FC_GEN_CAL_SRC_NONE = 4,
  /*
   * Number of types in SOC_PB_FC_GEN_CAL_SRC - Petra B
   */
  DNX_TMC_FC_NOF_GEN_CAL_SRCS_PB = 5,

  /* Arad Only */
  DNX_TMC_FC_GEN_CAL_SRC_LLFC_VSQ = DNX_TMC_FC_NOF_GEN_CAL_SRCS_PB,

  /* Arad Only */
  DNX_TMC_FC_GEN_CAL_SRC_PFC_VSQ,

  /* Arad Only */
  DNX_TMC_FC_GEN_CAL_SRC_GLB_RCS,

  /* Arad Only */
  DNX_TMC_FC_GEN_CAL_SRC_HCFC,
  
  /* Arad Only */
  DNX_TMC_FC_GEN_CAL_SRC_LLFC,

  /* Arad Only */
  DNX_TMC_FC_GEN_CAL_SRC_RETRANSMIT,

  /* Arad Only */
  DNX_TMC_FC_GEN_CAL_SRC_CONST,

  /* JER2_JERICHO Only */
  DNX_TMC_FC_GEN_CAL_SRC_STTSTCS_TAG,

  /*
   *  Number of types in DNX_TMC_FC_GEN_CAL_SRC
   */
  DNX_TMC_FC_NOF_GEN_CAL_SRCS
}DNX_TMC_FC_GEN_CAL_SRC;

typedef enum
{
  /*
   *  Global Resources Index- Buffer Descriptors
   */
  DNX_TMC_FC_CAL_GLB_RCS_ID_BDB = 0,
  /*
   *  Global Resources Index- MIX-Buffers resource for JER2_QAX
   */
  DNX_TMC_FC_CAL_GLB_RCS_ID_MIX = DNX_TMC_FC_CAL_GLB_RCS_ID_BDB,
  /*
   *  Global Resources Index- Unicast Data Buffers
   */
  DNX_TMC_FC_CAL_GLB_RCS_ID_UNI = 1,
  /*
   *  Global Resources Index- OCB-Buffers resource for JER2_QAX
   */
  DNX_TMC_FC_CAL_GLB_RCS_ID_OCB = DNX_TMC_FC_CAL_GLB_RCS_ID_UNI,
  /*
   *  Global Resources Index- Multicast Data Buffers
   */
  DNX_TMC_FC_CAL_GLB_RCS_ID_MUL = 2,
  /*
   *  Global Resources Index- Pool-0 resource for JER2_QAX
   */
  DNX_TMC_FC_CAL_GLB_RCS_ID_P0 = DNX_TMC_FC_CAL_GLB_RCS_ID_MUL,
  /*
   *  Total number of Global Resources Indexes.
   */
  DNX_TMC_FC_NOF_CAL_GLB_RCS_IDS_JER2_ARAD = 3,
  /*
   *  Global Resources Index- OCB Data Buffers
   */
  DNX_TMC_FC_CAL_GLB_RCS_ID_OCB_DB = DNX_TMC_FC_NOF_CAL_GLB_RCS_IDS_JER2_ARAD,
  /*
   *  Global Resources Index- Pool-1 resource for JER2_QAX
   */
  DNX_TMC_FC_CAL_GLB_RCS_ID_P1 = DNX_TMC_FC_NOF_CAL_GLB_RCS_IDS_JER2_ARAD,
  /*
   *  Global Resources Index- Shared Buffers/BDs resource of Pool-0
   */
  DNX_TMC_FC_CAL_GLB_RCS_ID_MIX_P0 = 4,
  /*
   *  Global Resources Index- HEADROOM resource for JER2_QAX
   */
  DNX_TMC_FC_CAL_GLB_RCS_ID_HEADROOM = DNX_TMC_FC_CAL_GLB_RCS_ID_MIX_P0,
  /*
   *  Global Resources Index- Shared Buffers/BDs resource of Pool-1
   */
  DNX_TMC_FC_CAL_GLB_RCS_ID_MIX_P1 = 5,
  /*
   *  Total number of Global Resources Indexes for JER2_QAX.
   */
  DNX_TMC_FC_NOF_CAL_GLB_RCS_IDS_JER2_QAX = DNX_TMC_FC_CAL_GLB_RCS_ID_MIX_P1,
  /*
   *  Global Resources Index- Shared OCB-Buffers resource of Pool-0
   */
  DNX_TMC_FC_CAL_GLB_RCS_ID_OCB_P0 = 6,
  /*
   *  Global Resources Index- Shared OCB-Buffers resource of Pool-1
   */
  DNX_TMC_FC_CAL_GLB_RCS_ID_OCB_P1 = 7,
  /*
   *  Total number of Global Resources Indexes.
   */
  DNX_TMC_FC_NOF_CAL_GLB_RCS_IDS = 8
}DNX_TMC_FC_CAL_GLB_RCS_ID;

typedef enum
{
  /*
   *  Calendar-based Flow Control destination (Reaction Point)
   *  - HR scheduling element. Index Range (HR-SE-ID): 128 -
   *  255.
   */
  DNX_TMC_FC_REC_CAL_DEST_HR = 0,
  /*
   *  Calendar-based Flow Control destination - Outgoing FAP
   *  Port, Egress (EGQ), Low Priority FC. Index Range
   *  (OFP-ID): 0 - 79.
   */
  DNX_TMC_FC_REC_CAL_DEST_OFP_EGQ_LP = 1,
  /*
   *  Calendar-based Flow Control destination - Outgoing FAP
   *  Port, Egress (EGQ), High Priority FC. Index Range
   *  (OFP-ID): 0 - 79.
   */
  DNX_TMC_FC_REC_CAL_DEST_OFP_EGQ_HP = 2,
  /*
   *  Calendar-based Flow Control destination - Outgoing FAP
   *  Port, HR scheduling element, Low Priority FC. Index
   *  Range (OFP-HR-ID): 0 - 79.
   */
  DNX_TMC_FC_REC_CAL_DEST_OFP_SCH_HR_LP = 3,
  /*
   *  Calendar-based Flow Control destination - Outgoing FAP
   *  Port, HR scheduling element, High FC. Index Range
   *  (OFP-HR-ID): 0 - 79.
   */
  DNX_TMC_FC_REC_CAL_DEST_OFP_SCH_HR_HP = 4,
  /*
   *  Calendar-based Flow Control destination - Network
   *  Interface. Index Range (NIF-ID): 0 - 63.
   */
  DNX_TMC_FC_REC_CAL_DEST_NIF = 5,
  /*
   *  Invalid/non-existing destination. Serves as "empty
   *  calendar entry" indication.
   */
  DNX_TMC_FC_REC_CAL_DEST_NONE = 6,
  /*
   *  Number of types in DNX_TMC_FC_REC_CAL_DEST - petra B
   */
  DNX_TMC_FC_NOF_REC_CAL_DESTS_PB = 7,

  /* Arad Only */
  DNX_TMC_FC_REC_CAL_DEST_PFC = DNX_TMC_FC_NOF_REC_CAL_DESTS_PB,

  /* Arad Only */
  DNX_TMC_FC_REC_CAL_DEST_NIF_LL,

  /* Arad Only */
  DNX_TMC_FC_REC_CAL_DEST_PORT_2_PRIORITY,

  /* Arad Only */
  DNX_TMC_FC_REC_CAL_DEST_PORT_8_PRIORITY,

  /* Arad Only */
  DNX_TMC_FC_REC_CAL_DEST_GENERIC_PFC,

  /* Arad Only */
  DNX_TMC_FC_REC_CAL_DEST_RETRANSMIT,

  DNX_TMC_FC_NOF_REC_CAL_DESTS
}DNX_TMC_FC_REC_CAL_DEST;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Calendar-based Flow Control source (trigger).
   */
  DNX_TMC_FC_GEN_CAL_SRC source;

  uint32 id;

} DNX_TMC_FC_GEN_CALENDAR;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Calendar-based Flow Control destination (Reaction
   *  Point).
   */
  DNX_TMC_FC_REC_CAL_DEST destination;

  uint32 id;
  
} DNX_TMC_FC_REC_CALENDAR;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

  DNX_TMC_FC_GEN_SRC gen_src;

  /*
   *  Jericho Only.
   */
  uint32 is_ocb_only;


  /*
   *   
   */
  uint32 glbl_rcs_pool;
  
  /*
   *  If TRUE, LLFC is generated based on CNM messages. Note
   *  1: if port-to-interface mapping for the relevant NIF is
   *  modified, this configuration must be reset after the
   *  modification. Note 2: refer to CNM module APIs for CNM
   *  messages handling configuration
   */
  uint8 cnm_enable;

  /*
   *  If TRUE, LLFC is generated based on global resource. 
   */
  uint8 glbl_rcs_enable;

  /*
   *  If TRUE, LLFC is generated based on VSQ LLFC. 
   */
  uint8 vsq_llfc_enable;

  /*
   *  If TRUE, LLFC is generated based on Almost full NIF indication. 
   */
  uint8 nif_enable;

} DNX_TMC_FC_GEN_INBND_LL;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Selects whether, and in which way, the CBFC affects
   *  lower priority classes
   */
  DNX_TMC_FC_INBND_CB_INHERIT inherit;
  /*
   *  If bit 'b' is set, the appropriate HR in range 128-255
   *  will stop credit generation. For XAUI/SPAUI - 8 HRs can
   *  be set as a reaction point for 8 FC classes,
   *  accordingly. For GMII - 2 HRs (2 FC classes)The index of
   *  the HR to stop, for FC-Class K, is:For XAUI/SPAUI
   *  (NIF-ID = MAL-ID*4), 128 + 8*MAL-ID + K. For GMII, 128 +
   *  2 * NIF-ID + K
   */
  uint8 sch_hr_bitmap;

} DNX_TMC_FC_REC_INBND_CB;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Selects whether, and in which way, the CBFC affects
   *  lower priority classes
   */
  DNX_TMC_FC_INBND_CB_INHERIT inherit;
  /*
   *  A bitmap that specifies the classes for CBFC generation.
   *  Controls FC generation upon Global Resources consumption
   *  Low-Level FC. 
   */
  uint32 glbl_rcs_low;
  /*
   *  If TRUE, CBFC is generated based on CNM messages. The
   *  relevant NIF will generate FC, based on the 3-LSB of the
   *  CPID. These 3-LSB are expected to represent TC. Note 1: if
   *  port-to-interface mapping for the relevant NIF is
   *  modified, this configuration must be reset after the
   *  modification. Note 2: refer to CNM module APIs for CNM
   *  messages handling configuration
   */
  uint8 cnm_intercept_enable;
  /*
   *  8-bits bitmap. If bit 'b' is set, the NIF will generate
   *  FC on class 'b' when lower internal NIF-FC threshold is
   *  crossed. Note: for GMII interface, only bits 0, 1 are
   *  relevant (since only two FC-classes are supported).
   *  Note: when higher internal NIF-FC threshold is crossed,
   *  FC is generated on all classes.
   */
  uint8 nif_cls_bitmap;

} DNX_TMC_FC_GEN_INBND_CB;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Disabled/Link-Level/Class-Based (according to the NIF
   *  type)
   */
  DNX_TMC_FC_INBND_MODE mode;
  /*
   *  Only when CBFC mode is selected (ignored otherwise) -
   *  CBFC configuration
   *  Soc_petra only
   */
  DNX_TMC_FC_GEN_INBND_CB cb;
  /*
   *  Only when LLFC mode is selected (ignored otherwise) -
   *  LLFC configuration
   */
  DNX_TMC_FC_GEN_INBND_LL ll;

  /* Arad only */
  DNX_TMC_FC_GEN_INBND_PFC pfc;

  /* Jericho only */
  int core;

} DNX_TMC_FC_GEN_INBND_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Disabled/Link-Level/Class-Based (according to the NIF
   *  type)
   */
  DNX_TMC_FC_INBND_MODE mode;
  /*
   *  Only when CBFC mode is selected (ignored otherwise) -
   *  CBFC configuration
   */
  DNX_TMC_FC_REC_INBND_CB cb;

  DNX_TMC_FC_REC_INBND_PFC pfc;

} DNX_TMC_FC_REC_INBND_INFO;

typedef struct 
{
  /* Jericho only */
  int core;
  
  /* 256 Bits - representing the 256 queue-pairs at the EGQ.
   * The CFC contains 16 generic bitmaps */
  uint32 bitmap[DNX_TMC_FC_PFC_GENERIC_BITMAP_SIZE / 32];
}DNX_TMC_FC_PFC_GENERIC_BITMAP;

typedef struct 
{
  /* 512 Bits - representing 512 HCFC FCs for Jericho, 256 HCFC FCs for Arad */
  uint32 bitmap[DNX_TMC_FC_HCFC_BITMAP_SIZE / 32];
}DNX_TMC_FC_HCFC_BITMAP;

typedef struct 
{
  /* Enable bitmap defines the set of enables */
  SHR_BITDCL bmp[_SHR_BITDCLSIZE(DNX_TMC_FC_NOF_ENABLEs)]; 
}DNX_TMC_FC_ENABLE_BITMAP;

typedef enum
{
  /*
   *  Mapping to EGQ PFC
   */
  DNX_TMC_FC_PFC_MAP_EGQ = 0,
  /*
   *  Mapping by using Generic PFC Bitmap
   */
  DNX_TMC_FC_PFC_MAP_GENERIC_BITMAP = 1,
  /*
   *  Number of types in DNX_TMC_FC_PFC_MAP_MODE
   */
  DNX_TMC_FC_NOF_PFC_MAP_MODE = 2
}DNX_TMC_FC_PFC_MAP_MODE;

typedef struct 
{
  /* 
   * In case mode is DNX_TMC_FC_PFC_MAP_EGQ, index is dst PFC index;
   * In case mode is DNX_TMC_FC_PFC_MAP_GENERIC_BITMAP, index is generic PFC bitmaps index;
   */
  uint32 index;

  DNX_TMC_FC_PFC_MAP_MODE mode;

  uint32 valid;
  
}DNX_TMC_FC_PFC_MAP;

typedef enum
{
  /*
   *  ILKN Calendar cannot be used to indicate LLFC (RX/TX)
   */
  DNX_TMC_FC_ILKN_CAL_LLFC_NONE = 0,
  /*
   *  ILKN Calendar Channel 0 indicates LLFC (RX/TX)
   */
  DNX_TMC_FC_ILKN_CAL_LLFC_CH_0 = 1,
  /*
   *  ILKN Calendar Channels 16*n (i.e. 0, 16, 32, .., 240)
   *  indicate LLFC (RX/TX)
   */
  DNX_TMC_FC_ILKN_CAL_LLFC_CH_16N = 2,
  /*
   *  Number of types in JER2_ARAD_FC_ILKN_CAL_LLFC
   */
  DNX_TMC_FC_NOF_ILKN_CAL_LLFCS = 3
}DNX_TMC_FC_ILKN_CAL_LLFC;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  For RX (Flow Control Reception): if a certain bit in the
   *  multiple use bits is received with a value of XOFF, and
   *  its corresponding bit in the mask is set, this is
   *  interpreted as request for link level flow control on
   *  the ILKN. For TX (Flow Control Generation), this mask
   *  will be sent as LLFC indication when requested. The value
   *  '0' disables Inband-LLFC using multiple-use-bits.
   */
  uint8 multi_use_mask;
  /*
   *  Defines whether the ILKN-FC calendar can be used to
   *  receive/generate LLFC. If it can, defines the calendar
   *  channel/channels used for LLFC indication
   */
  DNX_TMC_FC_ILKN_CAL_LLFC cal_channel;

} DNX_TMC_FC_ILKN_LLFC_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

  DNX_TMC_FC_GEN_CALENDAR entries[DNX_TMC_FC_ILKN_MUB_GEN_CAL_LEN];
} DNX_TMC_FC_ILKN_MUB_GEN_CAL;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  This field consists of the thresholds for setting LLFC or 
   *  clearing LLFC.
   */
  DNX_TMC_THRESH_WITH_HYST_INFO llfc;
  /*
   *  This field consists of the thresholds for setting PFC or 
   *  clearing PFC.
   */
  DNX_TMC_THRESH_WITH_HYST_INFO pfc;

} DNX_TMC_FC_PORT_FIFO_TH;


typedef struct 
{
  /* enable watch dog for HCFC OOB RX0/RX1*/
  uint32 enable;

  /*
   * watch dog period
   * Message interface is considered healthy if once in a configured period of time, 
   * a message was received with no error and correct CRC
   * unit: usec.
   */
  uint32 period;

  /*
   * watchdog error status configuration
   * values:
   *   1 - CFC send XOFF when watch-dog error is reported
   *   0 - CFC send XON when watch-dog error is reported
   */
  uint32 error_status;

}DNX_TMC_FC_HCFC_WATCHDOG;

typedef enum
{
  DNX_TMC_FC_PFC_GEN_BMP_SRC_NIF =      0,
  DNX_TMC_FC_PFC_GEN_BMP_SRC_OOB =      1,
  DNX_TMC_FC_PFC_GEN_BMP_SRC_ILKN_INB = 2,
  DNX_TMC_FC_PFC_GEN_BMP_SRC_MUB =      3,
  DNX_TMC_FC_PFC_GEN_BMP_SRC_TYPES
}DNX_TMC_FC_PFC_GEN_BMP_SRC_TYPE;

typedef struct {
  uint32 nif_pfc_gen_bmp_used[8];
  uint32 cal_pfc_gen_bmp_used[8];
  uint32 ilkn_inb_pfc_gen_bmp_used[8];
  uint32 mub_pfc_gen_bmp_used[8];
} DNX_TMC_FC_PFC_GEN_BMP_INFO;

#define DNX_TMC_FC_INTF_TYPE_NIF      0
#define DNX_TMC_FC_INTF_TYPE_ILKN     1
#define DNX_TMC_FC_INTF_TYPE_MUB      2
#define DNX_TMC_FC_INTF_TYPE_SPI      3
#define DNX_TMC_FC_INTF_TYPE_HCFC     4

#define DNX_TMC_FC_NIF_TYPE_LLFC      0
#define DNX_TMC_FC_NIF_TYPE_PFC       1
#define DNX_TMC_FC_NIF_TYPE_SAFC      2

#define DNX_TMC_FC_CALENDAR_TYPE_LLFC 0
#define DNX_TMC_FC_CALENDAR_TYPE_CHFC 1

#define DNX_TMC_FC_SRC_TYPE_GLB       0
#define DNX_TMC_FC_SRC_TYPE_VSQ       1
#define DNX_TMC_FC_SRC_TYPE_NIF       2
#define DNX_TMC_FC_SRC_TYPE_HCFC      3

typedef struct  {
    uint32                          port;

    uint32                          intf_type;            /* DNX_TMC_FC_INTF_TYPE_XXX:     0-NIF, 1-ILKN, 2-MUB, 3-SPI, 4-HCFC */
    uint32                          is_oob;
    DNX_TMC_FC_DIRECTION            direction;

    uint32                          nif_fc_type;          /* DNX_TMC_FC_NIF_TYPE_XXX:      0-llfc, 1-pfc, 2-safc */

    uint32                          calendar_fc_type;     /* DNX_TMC_FC_CALENDAR_TYPE_XXX: 0-llfc, 1-chfc */
    uint32                          calendar_id;

    uint32                          src_type;             /* DNX_TMC_FC_SRC_TYPE_XXX:      0-global, 1-vsq, 2-nif */
    DNX_TMC_ITM_VSQ_GROUP           vsq_type;             /* A/B/C/D/E/F */
    uint32                          vsq_id;
} DNX_TMC_FC_STATUS_KEY;

typedef struct  {
    /* 1. FC generation */
    /* 1.1 IQM block*/
    uint32 iqm_glb_mnmc_db_hi_fc_state[2];
    uint32 iqm_glb_mnmc_db_lo_fc_state[2];
    uint32 iqm_glb_flmc_db_hi_fc_state[2];
    uint32 iqm_glb_flmc_db_lo_fc_state[2];
    uint32 iqm_glb_bdb_hi_fc_state[2];
    uint32 iqm_glb_bdb_lo_fc_state[2];
    uint32 iqm_glb_ocb_hi_fc_state[2];
    uint32 iqm_glb_ocb_lo_fc_state[2];

    /* 1.2 CFC block*/
    uint32 cfc_iqm_glb_mnmc_db_hi_fc_state[2];
    uint32 cfc_iqm_glb_mnmc_db_lo_fc_state[2];
    uint32 cfc_iqm_glb_flmc_db_hi_fc_state[2];
    uint32 cfc_iqm_glb_flmc_db_lo_fc_state[2];
    uint32 cfc_iqm_glb_bdb_hi_fc_state[2];
    uint32 cfc_iqm_glb_bdb_lo_fc_state[2];
    uint32 cfc_iqm_glb_ocb_hi_fc_state[2];
    uint32 cfc_iqm_glb_ocb_lo_fc_state[2];
    uint32 cfc_cgm_pool_0_hi_fc_state[2];
    uint32 cfc_cgm_pool_0_lo_fc_state[2];
    uint32 cfc_cgm_pool_1_hi_fc_state[2];
    uint32 cfc_cgm_pool_1_lo_fc_state[2];

    uint32 cfc_iqm_vsq_fc_state[2];
    uint32 cfc_iqm_vsq_fc_state_2[2];

    uint32 cfc_nif_af_fc_status;

    /* 1.3 NBI block*/
    uint32 nbi_llfc_status_from_mlf;
    uint32 nbi_pfc_status_from_mlf;
    uint32 nbi_tx_llfc_from_cfc;
    uint32 nbi_tx_llfc_to_mac;
    uint32 nbi_tx_pfc_from_cfc;
    uint32 nbi_tx_pfc_to_mac;
    uint32 nbi_eth_tx_fc_cnt;
    uint32 nbi_ilkn_rx_chfc_from_cfc_raw;
    uint32 nbi_ilkn_tx_chfc_roc;
    uint32 nbi_mub_tx_from_cfc;
    uint32 nib_mub_tx_value;

    /* 1.4 MIB block*/
    uint64 mib_tx_pause_cnt;
    uint64 mib_tx_pfc_cnt;
    uint64 mib_tx_pfc_x_cnt[8];
    uint64 mib_tx_safc_log_cnt;
    /* uint64 mib_tx_hcfc_cnt; */

    /* 2 FC reception */ 
    /* 2.1 MIB block */
    uint64 mib_rx_pause_cnt;
    uint64 mib_rx_pfc_cnt;
    uint64 mib_rx_pfc_x_cnt[8];
    uint64 mib_rx_safc_log_cnt;
    uint64 mib_rx_safc_phy_cnt;
    uint64 mib_rx_safc_crc_cnt;
    /* uint64 mib_rx_hcfc_cnt;
    uint64 mib_rx_hcfc_crc_cnt; */

    /* 2.2 NBI block */
    uint32 nbi_rx_pfc_from_mac;
    uint32 nbi_rx_pfc_to_cfc;
    uint32 nbi_ilkn_rx_chfc_from_port_raw;
    uint32 nbi_ilkn_rx_chfc_from_port_roc;
    uint64 nbi_ilkn_rx_llfc_cnt;
    uint32 nbi_mub_rx_value;
    uint32 nbi_mub_rx_to_cfc;
    uint64 nbi_ilkn_llfc_stop_tx_cnt;
    uint32 nbi_mub_llfc_stop_tx_from_mub;
    uint32 nbi_llfc_stop_tx_from_cfc;
    uint32 nbi_llfc_stop_tx_to_mac;

    /* 2.3 CFC block */
    uint32 cfc_nif_pfc_status;
    uint32 cfc_ilkn_fc_status;
    uint32 cfc_mub_fc_status;
    uint32 cfc_spi_rx_llfc_status;
    uint32 cfc_spi_rx_pfc_status;
    uint32 cfc_spi_rx_gen_pfc_status;
    uint32 cfc_egq_pfc_status[2];                       /* 8 bits */
    soc_reg_above_64_val_t cfc_egq_pfc_status_full[2];  /* 256 bits */
    uint32 cfc_egq_inf_fc;
    uint32 cfc_egq_dev_fc;
    uint32 cfc_ilkn_rx_crc_err_status;
    uint32 cfc_spi_rx_crc_err_status;
    uint32 cfc_hcfc_rx_crc_err_status;
    uint32 cfc_ilkn_rx_crc_err_cnt;
    uint32 cfc_spi_rx_frame_err_cnt;
    uint32 cfc_spi_rx_dip_2_err_cnt;
    uint32 cfc_hcfc_rx_crc_err_cnt[5];
    uint32 cfc_oob_rx_lanes_status;
    uint32 cfc_hcfc_rx_wd_err_status;

    /* 2.4 EGQ block */
    uint32 egq_nif_fc_status[2];       /* 128/32 */
    uint32 egq_cfc_fc_status[2];                       /* 8 bits */
    soc_reg_above_64_val_t egq_cfc_fc_status_full[2];  /* 256 bits */

    /* 2.5 SCH block */
    uint32 sch_fc_port_cnt[8];
    uint32 sch_fc_inf_cnt;
    uint32 sch_fc_dev_cnt;

    /* 3 calendar info */
    uint32 cal_tx_src_type;            /* DNX_TMC_FC_SRC_TYPE_XXX: 0-global, 1-vsq, 2-nif, 3-hcfc, 4-retransmit, 5-constant */
    uint32 cal_tx_src_vsq_type;        /* DNX_TMC_ITM_VSQ_GROUP:   0-A, 1-B, 2-C, 3-D, 4-E, 5-F */
    uint32 cal_tx_src_id;
    
    uint32 cal_rx_dst_type;            /* DNX_TMC_FC_REC_CAL_DEST_XXX: 0-PFC, 1-NIF, 2-2P port, 3-8P port, 4-Generic PFC bitmap */
    uint32 cal_rx_dst_id;

    /* 4 NIF info */
    uint32 nif_tx_src_id;

    DNX_TMC_FC_PFC_MAP_MODE nif_rx_dst_type[8];            /* DNX_TMC_FC_PFC_MAP_XXX: 0-PFC, 1-Generic PFC bitmap */
    uint32 nif_rx_dst_id[8];

    /* 5 Core info */
    uint32 core_id;
} DNX_TMC_FC_STATUS_INFO;

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
void DNX_TMC_FC_PFC_GENERIC_BITMAP_clear(DNX_SAND_OUT DNX_TMC_FC_PFC_GENERIC_BITMAP *generic_bm);

void DNX_TMC_FC_CAL_IF_INFO_clear(DNX_SAND_OUT DNX_TMC_FC_CAL_IF_INFO *cal_info);

void DNX_TMC_FC_GEN_CALENDAR_clear(DNX_SAND_OUT DNX_TMC_FC_GEN_CALENDAR *cal_info);

void DNX_TMC_FC_GEN_INBND_INFO_clear(DNX_SAND_OUT DNX_TMC_FC_GEN_INBND_INFO *info);

void DNX_TMC_FC_GEN_INBND_CB_clear(DNX_SAND_OUT DNX_TMC_FC_GEN_INBND_CB *info);

void DNX_TMC_FC_GEN_INBND_LL_clear(DNX_SAND_OUT DNX_TMC_FC_GEN_INBND_LL *info);

void DNX_TMC_FC_REC_INBND_CB_clear(DNX_SAND_OUT DNX_TMC_FC_REC_INBND_CB *info);

void DNX_TMC_FC_GEN_INBND_PFC_clear(DNX_SAND_OUT DNX_TMC_FC_GEN_INBND_PFC *info);

void DNX_TMC_FC_REC_CALENDAR_clear(DNX_SAND_OUT DNX_TMC_FC_REC_CALENDAR *info);

void
  DNX_TMC_FC_REC_INBND_INFO_clear(
    DNX_SAND_OUT DNX_TMC_FC_REC_INBND_INFO *info
  );

void
  DNX_TMC_FC_ILKN_LLFC_INFO_clear(
    DNX_SAND_OUT DNX_TMC_FC_ILKN_LLFC_INFO *info
  );

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif



