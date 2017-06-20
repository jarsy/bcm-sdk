
/* $Id: jer2_arad_api_flow_control.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER2_ARAD_API_FLOW_CONTROL_INCLUDED__
/* { */
#define __JER2_ARAD_API_FLOW_CONTROL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

#include <soc/dnx/legacy/ARAD/arad_api_general.h>
#include <soc/dnx/legacy/TMC/tmc_api_flow_control.h>


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
  JER2_ARAD_FC_OFP_PRIORITY_NONE = 0x0,
  /*
   *  OFP FC priority - Low
   */
  JER2_ARAD_FC_OFP_PRIORITY_LP = 0x1,
  /*
   *  OFP FC priority - High. HP-FC triggers also LP-FC
   */
  JER2_ARAD_FC_OFP_PRIORITY_HP = 0x3,
  /*
   *  Number of types in JER2_ARAD_FC_OFP_PRIORITY
   */
  JER2_ARAD_FC_NOF_OFP_PRIORITYS = 3
}JER2_ARAD_FC_OFP_PRIORITY;

typedef enum
{
  /* Arad ILKN Interface ID A */
  JER2_ARAD_FC_ILKN_ID_A = 0,

  /* Arad ILKN Interface ID B */
  JER2_ARAD_FC_ILKN_ID_B = 1,

  /* Arad number of ILKN Interfaces */
  JER2_ARAD_FC_ILKN_IDS = 2
}JER2_ARAD_FC_ILKN_ID;

typedef enum
{
  /* Reception Calendar Type */
  JER2_ARAD_FC_CAL_TYPE_RX = 0,

  /* Generation Calendar Type */
  JER2_ARAD_FC_CAL_TYPE_TX = 1,

  /* Calendar Types */
  JER2_ARAD_FC_CAL_TYPES = 2
}JER2_ARAD_FC_CAL_TYPE;

typedef DNX_TMC_FC_ILKN_CAL_LLFC JER2_ARAD_FC_ILKN_CAL_LLFC;

typedef DNX_TMC_FC_ILKN_LLFC_INFO JER2_ARAD_FC_ILKN_LLFC_INFO;


typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

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

}JER2_ARAD_FC_ILKN_RETRANSMIT_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

  /* Enable the ILKN Retransmit calendar */
  uint32 enable;

  /* Calendar length (0-2) */
  uint32 length;

}JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_CFG;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

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

}JER2_ARAD_FC_SCH_OOB_WD_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

  /* Enable the Scheduler OOB Interface 
   * (otherwise the FC indication won't pass to the EGQ) */
  uint32 enable;

  /* The CL range of the FC indication vector
   * The actual range is from 0 to the given value
   * This should be equal to the calendar length of the SCH OOB Interface
   * Max value - 4K-1 */
  uint32 sch_range;

  /* Hold the WatchDog info for the SCH OOB Interface */
  JER2_ARAD_FC_SCH_OOB_WD_INFO wd_info;

}JER2_ARAD_FC_SCH_OOB_INFO;

typedef struct  
{
  DNX_SAND_MAGIC_NUM_VAR

  /* The queue pair for which to set the PFC Bitmap
   * Range: 0-255 */
  uint32 queue_pair;

  /* The PFC Bitmap: 8 bits, one for each TC */
  uint8 pfc_bm;

}JER2_ARAD_FC_PFC_BM_INFO;

typedef enum  
{
  JER2_ARAD_FC_OOB_TX_SPEED_CORE_2 = DNX_TMC_FC_OOB_TX_SPEED_CORE_2,
  JER2_ARAD_FC_OOB_TX_SPEED_CORE_4 = DNX_TMC_FC_OOB_TX_SPEED_CORE_4,
  JER2_ARAD_FC_OOB_TX_SPEED_CORE_8 = DNX_TMC_FC_OOB_TX_SPEED_CORE_8
}JER2_ARAD_FC_OOB_TX_SPEED;


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
  JER2_ARAD_FC_ILKN_RETRANSMIT_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_FC_ILKN_RETRANSMIT_INFO *info
  );

void 
  JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_CFG_clear(
    DNX_SAND_OUT JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_CFG *info
  );


int jer2_arad_fc_shr_mapping(int unit, int fcv_bit, int cl_index, int select, int valid);
int jer2_arad_fc_init_shr_mapping(int unit);


/*********************************************************************
* NAME:
 *   jer2_arad_fc_gen_inbnd_glb_hp_set
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
 *   DNX_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  DNX_TMC_FC_INGR_GEN_GLB_HP_MODE     fc_mode -
 *     Flow Control mode. If enabled, when Ingress Global
 *     Resources high priority Flow Control is indicated.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_fc_gen_inbnd_glb_hp_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_INGR_GEN_GLB_HP_MODE     fc_mode
  );


/*********************************************************************
*     Gets the configuration set by the
 *     "jer2_arad_fc_gen_inbnd_glb_hp_set" API.
 *     Refer to "jer2_arad_fc_gen_inbnd_glb_hp_set" API for details.
*********************************************************************/
uint32
  jer2_arad_fc_gen_inbnd_glb_hp_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_OUT DNX_TMC_FC_INGR_GEN_GLB_HP_MODE     *fc_mode
  );

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __JER2_ARAD_API_FLOW_CONTROL_INCLUDED__*/
#endif

