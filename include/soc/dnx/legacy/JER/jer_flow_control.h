
/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER2_JER_FLOW_CONTROL_INCLUDED__
/* { */
#define __JER2_JER_FLOW_CONTROL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

#include <soc/dnx/legacy/ARAD/arad_api_mgmt.h>
#include <soc/dnx/legacy/TMC/tmc_api_ingress_traffic_mgmt.h>

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
*     jer2_arad_flow_control_init
* FUNCTION:
*     Initialization of the Arad blocks configured in this module.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
soc_error_t
  jer2_jer_flow_control_init(
    DNX_SAND_IN  int                 unit
  );

soc_error_t
  jer2_jer_flow_control_init_oob_tx(
    DNX_SAND_IN  int                 unit
  );

soc_error_t
  jer2_jer_flow_control_init_oob_rx(
    DNX_SAND_IN  int                 unit
  );

soc_error_t
   jer2_jer_fc_enables_set(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN DNX_TMC_FC_ENABLE_BITMAP      *cfc_enables,
      DNX_SAND_IN DNX_TMC_FC_ENABLE_BITMAP      *ena_info
    );

soc_error_t
   jer2_jer_fc_enables_get(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN DNX_TMC_FC_ENABLE_BITMAP      *cfc_enables,
      DNX_SAND_OUT DNX_TMC_FC_ENABLE_BITMAP     *ena_info
    );

soc_error_t
   jer2_jer_fc_init_pfc_mapping(
      DNX_SAND_IN int  unit
    );

/*********************************************************************
* NAME:
 *   jer2_arad_fc_gen_inbnd_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Inband Flow Control Generation Configuration, based on
 *   Arad Ingress state indications.
 * INPUT:
 *   DNX_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  DNX_TMC_INTERFACE_ID             nif_ndx -
 *     Network Interface index. Range: 0 - 63.
 *   DNX_SAND_IN  DNX_TMC_FC_GEN_INBND_INFO           *info -
 *     Inbound FC Generation mode and configuration.
 * REMARKS:
 *   1. This API is not applicable for Interlaken NIF 2. FC
 *   generation can be triggered by VSQ (statistics tag)
 *   and/or CNM. VSQ to NIF/Class mapping is predefined,
 *   according to the NIF type and FC mode (LL/CB). For CNM -
 *   use jer2_arad_fc_gen_inbnd_cnm_map_set API to complete the
 *   configuration
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_jer_fc_gen_inbnd_set(
    DNX_SAND_IN  int                                unit,
    DNX_SAND_IN  soc_port_t                         port,
    DNX_SAND_IN  DNX_TMC_FC_GEN_INBND_INFO          *info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "jer2_arad_fc_gen_inbnd_set_unsafe" API.
 *     Refer to "jer2_arad_fc_gen_inbnd_set_unsafe" API for details.
*********************************************************************/
soc_error_t
  jer2_jer_fc_gen_inbnd_get(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  soc_port_t                 port,
    DNX_SAND_OUT DNX_TMC_FC_GEN_INBND_INFO  *info
  );

soc_error_t
  jer2_jer_fc_clear_calendar_unsafe(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_MODE            cal_mode_ndx,
    DNX_SAND_IN  JER2_ARAD_FC_CAL_TYPE               cal_type,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID              if_ndx
  );

/*********************************************************************
* NAME:
 *   jer2_arad_fc_rec_inbnd_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Inband Flow Control Reception Configuration
 * INPUT:
 *   DNX_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  DNX_TMC_INTERFACE_ID             nif_ndx -
 *     Network Interface index. Range: 0 - 63.
 *   DNX_SAND_IN  DNX_TMC_FC_REC_INBND_INFO           *info -
 *     Inbound FC Reception mode and configuration.
 * REMARKS:
 *   1. This API is not applicable for Interlaken NIF 2. For
 *   CB - use jer2_arad_fc_rec_inbnd_ofp_map_set API to complete the
 *   configuration
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_jer_fc_rec_inbnd_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_INTERFACE_ID             nif_ndx,
    DNX_SAND_IN  DNX_TMC_FC_REC_INBND_INFO           *info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "jer2_arad_fc_rec_inbnd_set_unsafe" API.
 *     Refer to "jer2_arad_fc_rec_inbnd_set_unsafe" API for details.
*********************************************************************/
soc_error_t
  jer2_jer_fc_rec_inbnd_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_INTERFACE_ID             nif_ndx,
    DNX_SAND_OUT DNX_TMC_FC_REC_INBND_INFO           *info
  );
/*********************************************************************
* NAME:
 *   jer2_arad_fc_gen_cal_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configures Calendar-based interface calendar for Flow
 *   Control Generation (OOB/ILKN-Inband TX).
 * INPUT:
 *   DNX_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  DNX_TMC_FC_CAL_MODE                 cal_mode_ndx -
 *     The interface and mode used for FC calendar-based
 *     generation: SPI-OOB, ILKN-OOB or ILKN-Inband
 *   DNX_SAND_IN  DNX_TMC_FC_OOB_ID                   if_ndx -
 *     The Interface Index of the interface on which the FC is
 *     generated. For OOB interface (both SPI and ILKN), only
 *     DNX_TMC_FC_OOB_ID_B can be used. Additionally, ILKN-A can be
 *     used.
 *   DNX_SAND_IN  DNX_TMC_FC_CAL_IF_INFO              *cal_conf -
 *     Calendar-based (OOB) interface configuration -
 *     enable/disable, calendar configuration - length and
 *     repetitions.
 *   DNX_SAND_IN  DNX_TMC_FC_GEN_CALENDAR             *cal_buff -
 *     A buffer with cal_conf.cal_len entries. An allocation
 *     with the appropriate size should be made. Each entry
 *     configures a calendar channel, defining the source of
 *     the Flow Control that controls this channel.
 * REMARKS:
 *   1. For global resources - use indexes defined in
 *   JER2_ARAD_FC_CAL_GLB_RCS_ID.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_jer_fc_gen_cal_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_MODE                 cal_mode_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID                if_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_INFO              *cal_conf,
    DNX_SAND_IN  DNX_TMC_FC_GEN_CALENDAR             *cal_buff
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "jer2_arad_fc_gen_cal_set_unsafe" API.
 *     Refer to "jer2_arad_fc_gen_cal_set_unsafe" API for details.
*********************************************************************/
soc_error_t
  jer2_jer_fc_gen_cal_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_MODE                 cal_mode_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID                if_ndx,
    DNX_SAND_OUT DNX_TMC_FC_CAL_IF_INFO              *cal_conf,
    DNX_SAND_OUT DNX_TMC_FC_GEN_CALENDAR             *cal_buff
  );

/*********************************************************************
* NAME:
 *   jer2_arad_fc_rec_cal_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configures Calendar-based interface calendar for Flow
 *   Control Reception.
 * INPUT:
 *   DNX_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  DNX_TMC_FC_CAL_MODE                 cal_mode_ndx -
 *     The interface and mode used for FC calendar-based
 *     reception: SPI-OOB, ILKN-OOB or ILKN-Inband
 *   DNX_SAND_IN  DNX_TMC_FC_OOB_ID                   if_ndx -
 *     The Interface Index of the interface on which the FC
 *     reception is handled.
 *   DNX_SAND_IN  DNX_TMC_FC_CAL_IF_INFO              *cal_conf -
 *     Calendar-based (OOB) interface configuration -
 *     enable/disable, calendar configuration - length and
 *     repetitions.
 *   DNX_SAND_IN  DNX_TMC_FC_REC_CALENDAR             *cal_buff -
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
  jer2_jer_fc_rec_cal_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_MODE                 cal_mode_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID                if_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_INFO              *cal_conf,
    DNX_SAND_IN  DNX_TMC_FC_REC_CALENDAR             *cal_buff
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "jer2_arad_fc_rec_cal_set_unsafe" API.
 *     Refer to "jer2_arad_fc_rec_cal_set_unsafe" API for details.
*********************************************************************/
soc_error_t
  jer2_jer_fc_rec_cal_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_MODE                 cal_mode_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID                if_ndx,
    DNX_SAND_OUT DNX_TMC_FC_CAL_IF_INFO              *cal_conf,
    DNX_SAND_OUT DNX_TMC_FC_REC_CALENDAR             *cal_buff
  );
/*********************************************************************
* NAME:
 *   jer2_arad_fc_ilkn_llfc_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Defines if and how LLFC can be received/generated using
 *   Interlaken NIF.
 * INPUT:
 *   DNX_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  JER2_ARAD_NIF_ILKN_ID                 ilkn_ndx -
 *     Interlaken Index. Range: 5000 - 5001. (ILKN-A/ILKN-B).
 *     Can also use JER2_ARAD_NIF_ID(ILKN, 0 - 1).
 *   DNX_SAND_IN  JER2_ARAD_FC_ILKN_LLFC_INFO           *info -
 *     Link Level Flow Control configuration for the Interlaken
 *     NIF
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_jer_fc_ilkn_llfc_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID          ilkn_ndx,
    DNX_SAND_IN  JER2_ARAD_FC_DIRECTION             direction_ndx,
    DNX_SAND_IN  JER2_ARAD_FC_ILKN_LLFC_INFO           *info
  );

soc_error_t
  jer2_jer_fc_ilkn_llfc_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID             ilkn_ndx,
    DNX_SAND_OUT JER2_ARAD_FC_ILKN_LLFC_INFO           *rec_info,
    DNX_SAND_OUT JER2_ARAD_FC_ILKN_LLFC_INFO           *gen_info
  );

soc_error_t
  jer2_jer_fc_pfc_generic_bitmap_set(
    DNX_SAND_IN   int   unit,
    DNX_SAND_IN   int       priority,
    DNX_SAND_IN   uint32    pfc_bitmap_index,
    DNX_SAND_IN   DNX_TMC_FC_PFC_GENERIC_BITMAP    *pfc_bitmap
  );
soc_error_t
  jer2_jer_fc_pfc_generic_bitmap_get(
    DNX_SAND_IN   int   unit,
    DNX_SAND_IN   int       priority,
    DNX_SAND_IN   uint32    pfc_bitmap_index,
    DNX_SAND_OUT  DNX_TMC_FC_PFC_GENERIC_BITMAP    *pfc_bitmap
  );

soc_error_t
jer2_jer_fc_pfc_generic_bitmap_valid_update(
    DNX_SAND_IN   int                               unit,
    DNX_SAND_IN   DNX_TMC_FC_PFC_GEN_BMP_SRC_TYPE   src_type,
    DNX_SAND_IN   int                               priority,
    DNX_SAND_OUT  uint32                           *is_valid
    );

soc_error_t
jer2_jer_fc_pfc_generic_bitmap_used_update(
    DNX_SAND_IN   int                               unit,
    DNX_SAND_IN   DNX_TMC_FC_PFC_GEN_BMP_SRC_TYPE   src_type,
    DNX_SAND_IN   int                               priority,
    DNX_SAND_IN   int                               pfc_bitmap_index,
    DNX_SAND_OUT  uint32                            is_set
    );

soc_error_t
  jer2_jer_fc_cat_2_tc_hcfc_bitmap_set(
    DNX_SAND_IN   int                           unit,
    DNX_SAND_IN   int                           core,
    DNX_SAND_IN   int                           tc,
    DNX_SAND_IN   int                           is_ocb_only,
    DNX_SAND_OUT  DNX_TMC_FC_HCFC_BITMAP        *hcfc_bitmap
  );

soc_error_t
  jer2_jer_fc_cat_2_tc_hcfc_bitmap_get(
    DNX_SAND_IN   int                           unit,
    DNX_SAND_IN   int                           core,
    DNX_SAND_IN   int                           tc,
    DNX_SAND_IN   int                           is_ocb_only,
    DNX_SAND_OUT  DNX_TMC_FC_HCFC_BITMAP        *hcfc_bitmap
  );

soc_error_t
  jer2_jer_fc_glb_hcfc_bitmap_set(
    DNX_SAND_IN   int                           unit,
    DNX_SAND_IN   int                           core,
    DNX_SAND_IN   int                           is_high_prio,
    DNX_SAND_IN   int                           is_ocb_only,
    DNX_SAND_IN   int                           pool_id,
    DNX_SAND_OUT  DNX_TMC_FC_HCFC_BITMAP        *hcfc_bitmap
  );

soc_error_t
  jer2_jer_fc_glb_hcfc_bitmap_get(
    DNX_SAND_IN   int                           unit,
    DNX_SAND_IN   int                           core,
    DNX_SAND_IN   int                           is_high_prio,
    DNX_SAND_IN   int                           is_ocb_only,
    DNX_SAND_IN   int                           pool_id,
    DNX_SAND_OUT  DNX_TMC_FC_HCFC_BITMAP        *hcfc_bitmap
  );

soc_error_t
  jer2_jer_fc_ilkn_mub_channel_set(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID           ilkn_ndx,
    DNX_SAND_IN  DNX_TMC_FC_DIRECTION           direction_ndx,
    DNX_SAND_OUT uint8                          bitmap
  );

soc_error_t
  jer2_jer_fc_ilkn_mub_channel_get(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID           ilkn_ndx,
    DNX_SAND_IN  DNX_TMC_FC_DIRECTION           direction_ndx,
    DNX_SAND_OUT uint8                          *bitmap
  );

soc_error_t
  jer2_jer_fc_ilkn_mub_gen_cal_set(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID             ilkn_ndx,
    DNX_SAND_IN  DNX_TMC_FC_ILKN_MUB_GEN_CAL      *cal_info
  );

soc_error_t
  jer2_jer_fc_ilkn_mub_gen_cal_get(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID             ilkn_ndx,
    DNX_SAND_OUT DNX_TMC_FC_ILKN_MUB_GEN_CAL      *cal_info
  );

uint32
  jer2_jer_fc_rec_cal_dest_type_to_val_internal(
    DNX_SAND_IN DNX_TMC_FC_REC_CAL_DEST dest
  );

uint32
  jer2_jer_fc_gen_cal_src_type_to_val_internal(
    DNX_SAND_IN DNX_TMC_FC_GEN_CAL_SRC src_type
  );

int
  jer2_jer_fc_pfc_mapping_set(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32                        nif_id,
    DNX_SAND_IN uint32                        src_pfc_ndx,
    DNX_SAND_IN DNX_TMC_FC_PFC_MAP            *pfc_map
  );

int
  jer2_jer_fc_pfc_mapping_get(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32                        nif_id,
    DNX_SAND_IN uint32                        src_pfc_ndx,
    DNX_SAND_OUT DNX_TMC_FC_PFC_MAP           *pfc_map
  );

soc_error_t
  jer2_jer_fc_inbnd_mode_set(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN soc_port_t                    port,
      DNX_SAND_IN int                           is_generate,
      DNX_SAND_IN DNX_TMC_FC_INBND_MODE         mode
  );

soc_error_t
  jer2_jer_fc_inbnd_mode_get(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN soc_port_t                    port,
      DNX_SAND_IN int                           is_generate,
      DNX_SAND_OUT DNX_TMC_FC_INBND_MODE        *mode
  );

soc_error_t
  jer2_jer_fc_glb_rcs_mask_set(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN int                           core,
      DNX_SAND_IN int                           is_high_prio,
      DNX_SAND_IN DNX_TMC_FC_GLB_RES_TYPE       glb_res_dst,
      DNX_SAND_IN uint32                        glb_res_src_bitmap
    );

soc_error_t
  jer2_jer_fc_glb_rcs_mask_get(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN int                           core,
      DNX_SAND_IN int                           is_high_prio,
      DNX_SAND_IN DNX_TMC_FC_GLB_RES_TYPE       glb_res_dst,
      DNX_SAND_OUT uint32                       *glb_res_src_bitmap
    );

int
  jer2_jer_fc_port_fifo_threshold_set(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  soc_port_t                 port,
    DNX_SAND_IN  DNX_TMC_FC_PORT_FIFO_TH    *info
  );

int
  jer2_jer_fc_port_fifo_threshold_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  soc_port_t               port,
    DNX_SAND_OUT DNX_TMC_FC_PORT_FIFO_TH  *info
  );

soc_error_t
  jer2_jer_fc_vsq_index_group2global(
    DNX_SAND_IN int                    unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP vsq_group,
    DNX_SAND_IN  uint32                vsq_in_group_ndx,
    DNX_SAND_IN  int                   cosq,
    DNX_SAND_IN  uint8                 is_ocb_only,
    DNX_SAND_IN  uint32                src_port,
    DNX_SAND_OUT uint32                *vsq_fc_ndx
  );

soc_error_t
  jer2_jer_fc_cmic_rx_set(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN soc_port_t                    port,
      DNX_SAND_IN uint32                        priority_bmp,
      DNX_SAND_IN int                           is_ena
  );

soc_error_t
  jer2_jer_fc_cmic_rx_get(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN soc_port_t                    port,
      DNX_SAND_OUT uint32                       *priority_bmp,
      DNX_SAND_OUT int                          *is_ena
  );

soc_error_t
   jer2_jer_fc_find_oob_inf_for_ilkn_inf (
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN soc_port_t                    ilkn_if,
      DNX_SAND_OUT DNX_TMC_FC_CAL_IF_ID         *oob_if
  );

soc_error_t
   jer2_jer_fc_connect_ilkn_inf_to_oob_inf (
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN soc_port_t                    ilkn_if
  );

soc_error_t
  jer2_jer_fc_status_info_get(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN DNX_TMC_FC_STATUS_KEY   *fc_status_key,
    DNX_SAND_OUT DNX_TMC_FC_STATUS_INFO *fc_status_info
  );

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __JER2_ARAD_FLOW_CONTROL_INCLUDED__*/
#endif


