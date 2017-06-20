
/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_FLOW_CONTROL_INCLUDED__
/* { */
#define __ARAD_FLOW_CONTROL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/arad_api_flow_control.h>
#include <soc/dpp/ARAD/arad_api_nif.h>
#include <soc/dpp/ARAD/arad_api_mgmt.h>
#include <soc/dpp/TMC/tmc_api_ingress_traffic_mgmt.h>

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

/*********************************************************************
* NAME:
*     arad_flow_control_init
* FUNCTION:
*     Initialization of the Arad blocks configured in this module.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
soc_error_t
  arad_flow_control_init(
    SOC_SAND_IN  int                 unit
  );

soc_error_t
  arad_flow_control_init_oob_tx(
    SOC_SAND_IN  int                 unit
  );

soc_error_t
  arad_flow_control_init_oob_rx(
    SOC_SAND_IN  int                 unit
  );

soc_error_t
   arad_fc_enables_set(
      SOC_SAND_IN int                           unit,
      SOC_SAND_IN SOC_TMC_FC_ENABLE_BITMAP      *cfc_enables,
      SOC_SAND_IN SOC_TMC_FC_ENABLE_BITMAP      *ena_info
    );

soc_error_t
   arad_fc_enables_get(
      SOC_SAND_IN int                           unit,
      SOC_SAND_IN SOC_TMC_FC_ENABLE_BITMAP      *cfc_enables,
      SOC_SAND_OUT SOC_TMC_FC_ENABLE_BITMAP     *ena_info
    );

soc_error_t
  arad_fc_init_pfc_mapping(
      SOC_SAND_IN int  unit
    );

uint32
  arad_fc_inbnd_max_nof_priorities_get(
    SOC_SAND_IN  ARAD_INTERFACE_ID             nif_ndx
  );

/*********************************************************************
* NAME:
 *   arad_fc_gen_inbnd_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Inband Flow Control Generation Configuration, based on
 *   Arad Ingress state indications.
 * INPUT:
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_INTERFACE_ID             nif_ndx -
 *     Network Interface index. Range: 0 - 63.
 *   SOC_SAND_IN  SOC_TMC_FC_GEN_INBND_INFO           *info -
 *     Inbound FC Generation mode and configuration.
 * REMARKS:
 *   1. This API is not applicable for Interlaken NIF 2. FC
 *   generation can be triggered by VSQ (statistics tag)
 *   and/or CNM. VSQ to NIF/Class mapping is predefined,
 *   according to the NIF type and FC mode (LL/CB). For CNM -
 *   use arad_fc_gen_inbnd_cnm_map_set API to complete the
 *   configuration
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_fc_gen_inbnd_set(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  soc_port_t                         port,
    SOC_SAND_IN  SOC_TMC_FC_GEN_INBND_INFO          *info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_fc_gen_inbnd_set_unsafe" API.
 *     Refer to "arad_fc_gen_inbnd_set_unsafe" API for details.
*********************************************************************/
soc_error_t
  arad_fc_gen_inbnd_get(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  soc_port_t                 port,
    SOC_SAND_OUT SOC_TMC_FC_GEN_INBND_INFO  *info
  );

soc_error_t
  arad_fc_clear_calendar_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_MODE                 cal_mode_ndx,
    SOC_SAND_IN  ARAD_FC_CAL_TYPE                 cal_type,
    SOC_SAND_IN  SOC_TMC_FC_OOB_ID                   if_ndx
  );

/*********************************************************************
* NAME:
 *   arad_fc_gen_inbnd_glb_hp_set_unsafe
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
  arad_fc_gen_inbnd_glb_hp_set_unsafe(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_INGR_GEN_GLB_HP_MODE     fc_mode
  );

uint32
  arad_fc_gen_inbnd_glb_hp_set_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_INGR_GEN_GLB_HP_MODE     fc_mode
  );

uint32
  arad_fc_gen_inbnd_glb_hp_get_verify(
    SOC_SAND_IN  int                      unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_fc_gen_inbnd_glb_hp_set_unsafe" API.
 *     Refer to "arad_fc_gen_inbnd_glb_hp_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_fc_gen_inbnd_glb_hp_get_unsafe(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_OUT SOC_TMC_FC_INGR_GEN_GLB_HP_MODE     *fc_mode
  );

/*********************************************************************
* NAME:
 *   arad_fc_gen_inbnd_cnm_map_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map the 3-LSB of the CMN CPID (represent TC), to the FC
 *   indication to generate when using Class Based Flow
 *   Control
 * INPUT:
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                      cpid_tc_ndx -
 *     The 3-LSB of the CPID index. Expected to represent TC.
 *   SOC_SAND_IN  uint32                      fc_class -
 *     The FC Class of the generated CBFC. Note: according to
 *     the inheritance configuration of the relevant NIF, the
 *     specified class may affect also lower classes. Range: 0 -
 *     7.
 * REMARKS:
 *   Inband FC generation, including CNM, is configured per
 *   NIF by arad_fc_gen_inbnd_set API. This API completes the
 *   CNM configuration when Class Based FC is used.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_fc_gen_inbnd_cnm_map_set_unsafe(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                      cpid_tc_ndx,
    SOC_SAND_IN  uint8                      enable_ll,
    SOC_SAND_IN  uint32                      fc_class
  );

uint32
  arad_fc_gen_inbnd_cnm_map_set_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                      cpid_tc_ndx,
    SOC_SAND_IN  uint8                      enable_ll,
    SOC_SAND_IN  uint32                      fc_class
  );

uint32
  arad_fc_gen_inbnd_cnm_map_get_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                      cpid_tc_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_fc_gen_inbnd_cnm_map_set_unsafe" API.
 *     Refer to "arad_fc_gen_inbnd_cnm_map_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_fc_gen_inbnd_cnm_map_get_unsafe(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                      cpid_tc_ndx,
    SOC_SAND_OUT uint8                      *enable_ll,
    SOC_SAND_OUT uint32                      *fc_class
  );
/*********************************************************************
* NAME:
 *   arad_fc_rec_inbnd_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Inband Flow Control Reception Configuration
 * INPUT:
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_INTERFACE_ID             nif_ndx -
 *     Network Interface index. Range: 0 - 63.
 *   SOC_SAND_IN  SOC_TMC_FC_REC_INBND_INFO           *info -
 *     Inbound FC Reception mode and configuration.
 * REMARKS:
 *   1. This API is not applicable for Interlaken NIF 2. For
 *   CB - use arad_fc_rec_inbnd_ofp_map_set API to complete the
 *   configuration
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

soc_error_t
  arad_fc_rec_inbnd_get_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  ARAD_INTERFACE_ID             nif_ndx
  );

/*********************************************************************
* NAME:
 *   arad_fc_gen_cal_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configures Calendar-based interface calendar for Flow
 *   Control Generation (OOB/ILKN-Inband TX).
 * INPUT:
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_TMC_FC_CAL_MODE                 cal_mode_ndx -
 *     The interface and mode used for FC calendar-based
 *     generation: SPI-OOB, ILKN-OOB or ILKN-Inband
 *   SOC_SAND_IN  SOC_TMC_FC_OOB_ID                   if_ndx -
 *     The Interface Index of the interface on which the FC is
 *     generated. For OOB interface (both SPI and ILKN), only
 *     SOC_TMC_FC_OOB_ID_B can be used. Additionally, ILKN-A can be
 *     used.
 *   SOC_SAND_IN  SOC_TMC_FC_CAL_IF_INFO              *cal_conf -
 *     Calendar-based (OOB) interface configuration -
 *     enable/disable, calendar configuration - length and
 *     repetitions.
 *   SOC_SAND_IN  SOC_TMC_FC_GEN_CALENDAR             *cal_buff -
 *     A buffer with cal_conf.cal_len entries. An allocation
 *     with the appropriate size should be made. Each entry
 *     configures a calendar channel, defining the source of
 *     the Flow Control that controls this channel.
 * REMARKS:
 *   1. For global resources - use indexes defined in
 *   ARAD_FC_CAL_GLB_RCS_ID.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_fc_gen_cal_set(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_MODE                 cal_mode_ndx,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID                if_ndx,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_INFO              *cal_conf,
    SOC_SAND_IN  SOC_TMC_FC_GEN_CALENDAR             *cal_buff
  );

soc_error_t
  arad_fc_gen_cal_set_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_MODE                 cal_mode_ndx,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID                if_ndx,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_INFO              *cal_conf,
    SOC_SAND_IN  SOC_TMC_FC_GEN_CALENDAR             *cal_buff
  );

soc_error_t
  arad_fc_gen_cal_get_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_MODE                 cal_mode_ndx,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID                if_ndx
  );


/*********************************************************************
*     Gets the configuration set by the
 *     "arad_fc_gen_cal_set_unsafe" API.
 *     Refer to "arad_fc_gen_cal_set_unsafe" API for details.
*********************************************************************/
soc_error_t
  arad_fc_gen_cal_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_MODE                 cal_mode_ndx,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID                if_ndx,
    SOC_SAND_OUT SOC_TMC_FC_CAL_IF_INFO              *cal_conf,
    SOC_SAND_OUT SOC_TMC_FC_GEN_CALENDAR             *cal_buff
  );

/*********************************************************************
* NAME:
 *   arad_fc_rec_cal_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configures Calendar-based interface calendar for Flow
 *   Control Reception.
 * INPUT:
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_TMC_FC_CAL_MODE                 cal_mode_ndx -
 *     The interface and mode used for FC calendar-based
 *     reception: SPI-OOB, ILKN-OOB or ILKN-Inband
 *   SOC_SAND_IN  SOC_TMC_FC_OOB_ID                   if_ndx -
 *     The Interface Index of the interface on which the FC
 *     reception is handled.
 *   SOC_SAND_IN  SOC_TMC_FC_CAL_IF_INFO              *cal_conf -
 *     Calendar-based (OOB) interface configuration -
 *     enable/disable, calendar configuration - length and
 *     repetitions.
 *   SOC_SAND_IN  SOC_TMC_FC_REC_CALENDAR             *cal_buff -
 *     A buffer with cal_conf.cal_len entries. Each entry
 *     configures a calendar channel, defining the destination
 *     that handles the Flow Control from this channel.
 * REMARKS:
 *   The get function is not symmetric; if the same SCH-based
 *   OFP HR appears both in low and high priority in the
 *   calendar in different entries. In this case, the
 *   returned calendar indicates for the first entry of this
 *   HR a low priority, and high priority for the next
 *   entries of this HR.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_fc_rec_cal_set(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_MODE                 cal_mode_ndx,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID                if_ndx,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_INFO              *cal_conf,
    SOC_SAND_IN  SOC_TMC_FC_REC_CALENDAR             *cal_buff
  );

soc_error_t
  arad_fc_rec_cal_set_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_MODE                 cal_mode_ndx,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID                if_ndx,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_INFO              *cal_conf,
    SOC_SAND_IN  SOC_TMC_FC_REC_CALENDAR             *cal_buff
  );

soc_error_t
  arad_fc_rec_cal_get_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_MODE                 cal_mode_ndx,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID                if_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_fc_rec_cal_set_unsafe" API.
 *     Refer to "arad_fc_rec_cal_set_unsafe" API for details.
*********************************************************************/
soc_error_t
  arad_fc_rec_cal_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_MODE                 cal_mode_ndx,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID                if_ndx,
    SOC_SAND_OUT SOC_TMC_FC_CAL_IF_INFO              *cal_conf,
    SOC_SAND_OUT SOC_TMC_FC_REC_CALENDAR             *cal_buff
  );
/*********************************************************************
* NAME:
 *   arad_fc_ilkn_llfc_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Defines if and how LLFC can be received/generated using
 *   Interlaken NIF.
 * INPUT:
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_NIF_ILKN_ID                 ilkn_ndx -
 *     Interlaken Index. Range: 5000 - 5001. (ILKN-A/ILKN-B).
 *     Can also use ARAD_NIF_ID(ILKN, 0 - 1).
 *   SOC_SAND_IN  ARAD_FC_ILKN_LLFC_INFO           *info -
 *     Link Level Flow Control configuration for the Interlaken
 *     NIF
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_fc_ilkn_llfc_set(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID          ilkn_ndx,
    SOC_SAND_IN  ARAD_FC_DIRECTION             direction_ndx,
    SOC_SAND_IN  ARAD_FC_ILKN_LLFC_INFO           *info
  );

soc_error_t
  arad_fc_ilkn_llfc_set_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID          ilkn_ndx,
    SOC_SAND_IN  ARAD_FC_DIRECTION             direction_ndx,
    SOC_SAND_IN  ARAD_FC_ILKN_LLFC_INFO           *info
  );

soc_error_t
  arad_fc_ilkn_llfc_get_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID             ilkn_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_fc_ilkn_llfc_set_unsafe" API.
 *     Refer to "arad_fc_ilkn_llfc_set_unsafe" API for details.
*********************************************************************/
soc_error_t
  arad_fc_ilkn_llfc_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID             ilkn_ndx,
    SOC_SAND_OUT ARAD_FC_ILKN_LLFC_INFO           *rec_info,
    SOC_SAND_OUT ARAD_FC_ILKN_LLFC_INFO           *gen_info
  );

soc_error_t
  arad_fc_pfc_generic_bitmap_verify(
    SOC_SAND_IN   int   unit,
    SOC_SAND_IN   int       priority,
    SOC_SAND_IN   uint32    pfc_bitmap_index
  );
soc_error_t
  arad_fc_pfc_generic_bitmap_set(
    SOC_SAND_IN   int   unit,
    SOC_SAND_IN   int       priority,
    SOC_SAND_IN   uint32    pfc_bitmap_index,
    SOC_SAND_IN   SOC_TMC_FC_PFC_GENERIC_BITMAP    *pfc_bitmap
  );
soc_error_t
  arad_fc_pfc_generic_bitmap_get(
    SOC_SAND_IN   int   unit,
    SOC_SAND_IN   int       priority,
    SOC_SAND_IN   uint32    pfc_bitmap_index,
    SOC_SAND_OUT  SOC_TMC_FC_PFC_GENERIC_BITMAP    *pfc_bitmap
  );

soc_error_t
arad_fc_pfc_generic_bitmap_valid_update(
    SOC_SAND_IN   int                               unit,
    SOC_SAND_IN   SOC_TMC_FC_PFC_GEN_BMP_SRC_TYPE   src_type,
    SOC_SAND_IN   int                               priority,
    SOC_SAND_OUT  uint32                           *is_valid
    );

soc_error_t
arad_fc_pfc_generic_bitmap_used_update(
    SOC_SAND_IN   int                               unit,
    SOC_SAND_IN   SOC_TMC_FC_PFC_GEN_BMP_SRC_TYPE   src_type,
    SOC_SAND_IN   int                               priority,
    SOC_SAND_IN   int                               pfc_bitmap_index,
    SOC_SAND_OUT  uint32                            is_set
    );

soc_error_t
  arad_fc_cat_2_tc_hcfc_bitmap_set(
    SOC_SAND_IN   int                           unit,
    SOC_SAND_IN   int                           core,
    SOC_SAND_IN   int                           tc,
    SOC_SAND_IN   int                           is_ocb_only,
    SOC_SAND_OUT  SOC_TMC_FC_HCFC_BITMAP        *hcfc_bitmap
  );

soc_error_t
  arad_fc_cat_2_tc_hcfc_bitmap_get(
    SOC_SAND_IN   int                           unit,
    SOC_SAND_IN   int                           core,
    SOC_SAND_IN   int                           tc,
    SOC_SAND_IN   int                           is_ocb_only,
    SOC_SAND_OUT  SOC_TMC_FC_HCFC_BITMAP        *hcfc_bitmap
  );

soc_error_t
  arad_fc_glb_hcfc_bitmap_set(
    SOC_SAND_IN   int                           unit,
    SOC_SAND_IN   int                           core,
    SOC_SAND_IN   int                           is_high_prio,
    SOC_SAND_IN   int                           is_ocb_only,
    SOC_SAND_IN   int                           pool_id,
    SOC_SAND_OUT  SOC_TMC_FC_HCFC_BITMAP        *hcfc_bitmap
  );

soc_error_t
  arad_fc_glb_hcfc_bitmap_get(
    SOC_SAND_IN   int                           unit,
    SOC_SAND_IN   int                           core,
    SOC_SAND_IN   int                           is_high_prio,
    SOC_SAND_IN   int                           is_ocb_only,
    SOC_SAND_IN   int                           pool_id,
    SOC_SAND_OUT  SOC_TMC_FC_HCFC_BITMAP        *hcfc_bitmap
  );

soc_error_t
  arad_fc_hcfc_watchdog_set(
    SOC_SAND_IN   int                           unit,
    SOC_SAND_IN   SOC_TMC_FC_CAL_IF_ID          if_ndx,
    SOC_SAND_IN   SOC_TMC_FC_HCFC_WATCHDOG      *hcfc_watchdog
  );

soc_error_t
  arad_fc_hcfc_watchdog_get(
    SOC_SAND_IN   int                           unit,
    SOC_SAND_IN   SOC_TMC_FC_CAL_IF_ID          if_ndx,
    SOC_SAND_OUT  SOC_TMC_FC_HCFC_WATCHDOG      *hcfc_watchdog
  );

uint32
  arad_fc_ilkn_retransmit_set_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  ARAD_NIF_ILKN_ID                 ilkn_ndx,
    SOC_SAND_IN  ARAD_FC_ILKN_RETRANSMIT_INFO     *ret_info
  );

uint32
  arad_fc_ilkn_retransmit_get_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  ARAD_NIF_ILKN_ID                 ilkn_ndx,
    SOC_SAND_OUT ARAD_FC_ILKN_RETRANSMIT_INFO     *ret_info
  );


uint32
  arad_fc_ilkn_mub_rec_set_unsafe(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  ARAD_NIF_ILKN_ID                 ilkn_ndx,
    SOC_SAND_IN  uint8                            bitmap
  );

uint32
  arad_fc_ilkn_mub_rec_get_unsafe(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  ARAD_NIF_ILKN_ID                 ilkn_ndx,
    SOC_SAND_OUT uint8                            *bitmap
  );

soc_error_t
  arad_fc_ilkn_mub_channel_set(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID           ilkn_ndx,
    SOC_SAND_IN  SOC_TMC_FC_DIRECTION           direction_ndx,
    SOC_SAND_OUT uint8                          bitmap
  );

soc_error_t
  arad_fc_ilkn_mub_channel_get(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID           ilkn_ndx,
    SOC_SAND_IN  SOC_TMC_FC_DIRECTION           direction_ndx,
    SOC_SAND_OUT uint8                          *bitmap
  );

soc_error_t
  arad_fc_ilkn_mub_gen_cal_set(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID             ilkn_ndx,
    SOC_SAND_IN  SOC_TMC_FC_ILKN_MUB_GEN_CAL      *cal_info
  );

soc_error_t
  arad_fc_ilkn_mub_gen_cal_get(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_TMC_FC_CAL_IF_ID             ilkn_ndx,
    SOC_SAND_OUT SOC_TMC_FC_ILKN_MUB_GEN_CAL      *cal_info
  );

uint32
arad_fc_rec_cal_dest_type_to_val_internal(
    SOC_SAND_IN SOC_TMC_FC_REC_CAL_DEST dest
  );

uint32
arad_fc_gen_cal_src_type_to_val_internal(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN SOC_TMC_FC_GEN_CAL_SRC src_type
  );


/*********************************************************************
*     Gets the configuration set by the
 *     "arad_fc_nif_pause_frame_src_addr_set_unsafe" API.
 *     Refer to "arad_fc_nif_pause_frame_src_addr_set_unsafe" API
 *     for details.
*********************************************************************/

uint32
  ARAD_FC_ILKN_LLFC_INFO_verify(
    SOC_SAND_IN  ARAD_FC_ILKN_LLFC_INFO *info
  );

soc_error_t
  arad_fc_pfc_mapping_set(
    SOC_SAND_IN int  unit,
    SOC_SAND_IN uint32                        nif_id,
    SOC_SAND_IN uint32                        src_pfc_ndx,
    SOC_SAND_IN SOC_TMC_FC_PFC_MAP            *pfc_map
  );

soc_error_t
  arad_fc_pfc_mapping_get(
    SOC_SAND_IN int  unit,
    SOC_SAND_IN uint32                        nif_id,
    SOC_SAND_IN uint32                        src_pfc_ndx,
    SOC_SAND_OUT SOC_TMC_FC_PFC_MAP           *pfc_map
  );

int
  arad_fc_port_fifo_threshold_set(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  soc_port_t                 port,
    SOC_SAND_IN  SOC_TMC_FC_PORT_FIFO_TH    *info
  );

int
  arad_fc_port_fifo_threshold_get(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  soc_port_t                 port,
    SOC_SAND_OUT SOC_TMC_FC_PORT_FIFO_TH    *info
  );

soc_error_t
  arad_fc_inbnd_mode_set(
      SOC_SAND_IN int                           unit,
      SOC_SAND_IN soc_port_t                    port,
      SOC_SAND_IN int                           is_generate,
      SOC_SAND_IN SOC_TMC_FC_INBND_MODE         mode
  );

soc_error_t
  arad_fc_inbnd_mode_get(
      SOC_SAND_IN int                           unit,
      SOC_SAND_IN soc_port_t                    port,
      SOC_SAND_IN int                           is_generate,
      SOC_SAND_OUT SOC_TMC_FC_INBND_MODE        *mode
  );

soc_error_t
  arad_fc_vsq_index_group2global(
    SOC_SAND_IN int                    unit,
    SOC_SAND_IN  SOC_TMC_ITM_VSQ_GROUP vsq_group,
    SOC_SAND_IN  uint32                vsq_in_group_ndx,
    SOC_SAND_IN  int                   cosq,
    SOC_SAND_IN  uint8                 is_ocb_only,
    SOC_SAND_IN  uint32                src_port,
    SOC_SAND_OUT uint32                *vsq_fc_ndx
  );

soc_error_t
  arad_fc_cmic_rx_set(
      SOC_SAND_IN int                           unit,
      SOC_SAND_IN soc_port_t                    port,
      SOC_SAND_IN uint32                        priority_bmp,
      SOC_SAND_IN int                           is_ena
  );

soc_error_t
  arad_fc_cmic_rx_get(
      SOC_SAND_IN int                           unit,
      SOC_SAND_IN soc_port_t                    port,
      SOC_SAND_OUT uint32                       *priority_bmp,
      SOC_SAND_OUT int                          *is_ena
  );

soc_error_t
  arad_fc_status_info_get(
    SOC_SAND_IN int                      unit,
    SOC_SAND_IN SOC_TMC_FC_STATUS_KEY   *fc_status_key,
    SOC_SAND_OUT SOC_TMC_FC_STATUS_INFO *fc_status_info
  );

#if defined(BCM_88650_B0)

uint32
  arad_fc_ilkn_retransmit_cal_set_unsafe(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  ARAD_NIF_ILKN_ID                   ilkn_ndx,
    SOC_SAND_IN  ARAD_FC_DIRECTION                  direction_ndx,
    SOC_SAND_IN  ARAD_FC_ILKN_RETRANSMIT_CAL_CFG    *cal_cfg
  );

uint32
  arad_fc_ilkn_retransmit_cal_get_unsafe(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  ARAD_NIF_ILKN_ID                   ilkn_ndx,
    SOC_SAND_IN  ARAD_FC_DIRECTION                  direction_ndx,
    SOC_SAND_OUT ARAD_FC_ILKN_RETRANSMIT_CAL_CFG    *cal_cfg
  );

uint32
  arad_fc_ilkn_retransmit_cal_set_verify(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  ARAD_NIF_ILKN_ID                   ilkn_ndx,
    SOC_SAND_IN  ARAD_FC_DIRECTION                  direction_ndx,
    SOC_SAND_IN  ARAD_FC_ILKN_RETRANSMIT_CAL_CFG    *cal_cfg
  );

#endif


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_FLOW_CONTROL_INCLUDED__*/
#endif


