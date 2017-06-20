
/* $Id: arad_api_flow_control.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_API_FLOW_CONTROL_INCLUDED__
/* { */
#define __ARAD_API_FLOW_CONTROL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Utils/sand_pp_mac.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/arad_api_general.h>
#include <soc/dpp/TMC/tmc_api_flow_control.h>
#include <soc/dpp/ARAD/arad_api_nif.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */


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

typedef enum
{
  /*
   *  OFP FC priority - No FC
   */
  ARAD_FC_OFP_PRIORITY_NONE = 0x0,
  /*
   *  OFP FC priority - Low
   */
  ARAD_FC_OFP_PRIORITY_LP = 0x1,
  /*
   *  OFP FC priority - High. HP-FC triggers also LP-FC
   */
  ARAD_FC_OFP_PRIORITY_HP = 0x3,
  /*
   *  Number of types in ARAD_FC_OFP_PRIORITY
   */
  ARAD_FC_NOF_OFP_PRIORITYS = 3
}ARAD_FC_OFP_PRIORITY;

typedef enum
{
  /* Arad ILKN Interface ID A */
  ARAD_FC_ILKN_ID_A = 0,

  /* Arad ILKN Interface ID B */
  ARAD_FC_ILKN_ID_B = 1,

  /* Arad number of ILKN Interfaces */
  ARAD_FC_ILKN_IDS = 2
}ARAD_FC_ILKN_ID;

typedef enum
{
  /* Reception Calendar Type */
  ARAD_FC_CAL_TYPE_RX = 0,

  /* Generation Calendar Type */
  ARAD_FC_CAL_TYPE_TX = 1,

  /* Calendar Types */
  ARAD_FC_CAL_TYPES = 2
}ARAD_FC_CAL_TYPE;

typedef SOC_TMC_FC_ILKN_CAL_LLFC ARAD_FC_ILKN_CAL_LLFC;

typedef SOC_TMC_FC_ILKN_LLFC_INFO ARAD_FC_ILKN_LLFC_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /* Enable the ILKN Retransmit option */
  uint32 enable;

  /* RX Polarity of the retransmit indication
   * 0 - Normal
   * 1 - Inverted */
  uint32 rx_polarity;
  /* TX Polarity of the retransmit indication
   * 0 - Normal
   * 1 - Inverted */
  uint32 tx_polarity;

  /* Raise en error indication upon CRC errors */
  uint32 error_indication;

}ARAD_FC_ILKN_RETRANSMIT_INFO;

#if defined(BCM_88650_B0)
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /* Enable the ILKN Retransmit calendar */
  uint32 enable;

  /* Calendar length (0-2) */
  uint32 length;

}ARAD_FC_ILKN_RETRANSMIT_CAL_CFG;
#endif

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /* Enable the Scheduler OOB Watchdog */
  uint32 enable;

  /* When no good message was received during the last configured period,
   * an error indication is raised  */
  uint32 period;

  /* Enabling raising an error when no good message 
   * was received during the last period */
  uint32 error_indication;

  /* Raise an error on the first CRC error */
  uint32 crc_error;

}ARAD_FC_SCH_OOB_WD_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /* Enable the Scheduler OOB Interface 
   * (otherwise the FC indication won't pass to the EGQ) */
  uint32 enable;

  /* The CL range of the FC indication vector
   * The actual range is from 0 to the given value
   * This should be equal to the calendar length of the SCH OOB Interface
   * Max value - 4K-1 */
  uint32 sch_range;

  /* Hold the WatchDog info for the SCH OOB Interface */
  ARAD_FC_SCH_OOB_WD_INFO wd_info;

}ARAD_FC_SCH_OOB_INFO;

typedef struct  
{
  SOC_SAND_MAGIC_NUM_VAR

  /* The queue pair for which to set the PFC Bitmap
   * Range: 0-255 */
  uint32 queue_pair;

  /* The PFC Bitmap: 8 bits, one for each TC */
  uint8 pfc_bm;

}ARAD_FC_PFC_BM_INFO;

typedef enum  
{
  ARAD_FC_OOB_TX_SPEED_CORE_2 = SOC_TMC_FC_OOB_TX_SPEED_CORE_2,
  ARAD_FC_OOB_TX_SPEED_CORE_4 = SOC_TMC_FC_OOB_TX_SPEED_CORE_4,
  ARAD_FC_OOB_TX_SPEED_CORE_8 = SOC_TMC_FC_OOB_TX_SPEED_CORE_8
}ARAD_FC_OOB_TX_SPEED;


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

void 
  ARAD_FC_ILKN_RETRANSMIT_INFO_clear(
    SOC_SAND_OUT ARAD_FC_ILKN_RETRANSMIT_INFO *info
  );

#if defined(BCM_88650_B0)
void 
  ARAD_FC_ILKN_RETRANSMIT_CAL_CFG_clear(
    SOC_SAND_OUT ARAD_FC_ILKN_RETRANSMIT_CAL_CFG *info
  );
#endif


int arad_fc_shr_mapping(int unit, int fcv_bit, int cl_index, int select, int valid);
int arad_fc_init_shr_mapping(int unit);


/*********************************************************************
* NAME:
 *   arad_fc_gen_inbnd_glb_hp_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable/disable Flow Control generation, based on Ingress
 *   Global Resources - high priority, via NIF. Flow Control
 *   generation may be either Link Level or Class Based. For
 *   Link Level - Flow Control will be generated on all
 *   links. For Class Based - Flow Control will be generated
 *   on all Flow Control Classes.
 * INPUT:
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_TMC_FC_INGR_GEN_GLB_HP_MODE     fc_mode -
 *     Flow Control mode. If enabled, when Ingress Global
 *     Resources high priority Flow Control is indicated.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_fc_gen_inbnd_glb_hp_set(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_INGR_GEN_GLB_HP_MODE     fc_mode
  );


/*********************************************************************
*     Gets the configuration set by the
 *     "arad_fc_gen_inbnd_glb_hp_set" API.
 *     Refer to "arad_fc_gen_inbnd_glb_hp_set" API for details.
*********************************************************************/
uint32
  arad_fc_gen_inbnd_glb_hp_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_OUT SOC_TMC_FC_INGR_GEN_GLB_HP_MODE     *fc_mode
  );


/*********************************************************************
* NAME:
 *   arad_fc_nif_pause_frame_src_addr_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Function description
 * INPUT:
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                      mal_ndx -
 *     MAC Lane index. Range: 0 - 15.
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS            *mac_addr -
 *     Source MAC address, part of 802.3 pause frame.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_fc_nif_pause_frame_src_addr_set(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                      mal_ndx,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS            *mac_addr
  );


#if defined(BCM_88650_B0)

/*********************************************************************
* NAME:
 *   arad_fc_ilkn_retransmit_cal_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Function description
 * INPUT:
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_NIF_ILKN_ID                   ilkn_ndx -
 *     ILKN index (ARAD_NIF_ID_ILKN_0, ARAD_NIF_ID_ILKN_1)
 *   SOC_SAND_IN  ARAD_FC_DIRECTION                  direction_ndx -
 *     FC Direction (RX or TX)
 *   SOC_SAND_IN  ARAD_FC_ILKN_RETRANSMIT_CAL_CFG    *cal_cfg -
 *      The enable and length of the calendar
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

uint32
  arad_fc_ilkn_retransmit_cal_set(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  ARAD_NIF_ILKN_ID                   ilkn_ndx,
    SOC_SAND_IN  ARAD_FC_DIRECTION                  direction_ndx,
    SOC_SAND_IN  ARAD_FC_ILKN_RETRANSMIT_CAL_CFG    *cal_cfg
  );

uint32
  arad_fc_ilkn_retransmit_cal_get(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  ARAD_NIF_ILKN_ID                   ilkn_ndx,
    SOC_SAND_IN  ARAD_FC_DIRECTION                  direction_ndx,
    SOC_SAND_OUT ARAD_FC_ILKN_RETRANSMIT_CAL_CFG    *cal_cfg
  );

#endif




/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_API_FLOW_CONTROL_INCLUDED__*/
#endif

