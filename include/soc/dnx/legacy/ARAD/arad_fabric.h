/* $Id: jer2_arad_fabric.h,v 1.30 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER2_ARAD_FABRIC_INCLUDED__
/* { */
#define __JER2_ARAD_FABRIC_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnxc/legacy/dnxc_defs.h>
#include <soc/dnx/legacy/ARAD/arad_api_fabric.h>
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/ARAD/arad_chip_defines.h>
#include <soc/dnx/legacy/ARAD/arad_api_general.h>
#include <soc/dnx/legacy/ARAD/arad_api_mgmt.h>
#include <soc/dnxc/legacy/dnxc_port.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define JER2_ARAD_FBC_PRIORITY_NDX_MASK          (0xff)
#define JER2_ARAD_FBC_PRIORITY_NDX_NOF           (256)

#define JER2_ARAD_FBC_PRIORITY_NDX_IS_TDM_MASK       (0x1)
#define JER2_ARAD_FBC_PRIORITY_NDX_IS_TDM_OFFSET     (0)
#define JER2_ARAD_FBC_PRIORITY_NDX_TC_MASK           (0xE)
#define JER2_ARAD_FBC_PRIORITY_NDX_TC_OFFSET         (1)
#define JER2_ARAD_FBC_PRIORITY_NDX_DP_MASK           (0x30)
#define JER2_ARAD_FBC_PRIORITY_NDX_DP_OFFSET         (4)
#define JER2_ARAD_FBC_PRIORITY_NDX_IS_HP_MASK        (0x80)
#define JER2_ARAD_FBC_PRIORITY_NDX_IS_HP_OFFSET      (7)

#define JER2_ARAD_FBC_PRIORITY_NOF                   (0x4)
#define JER2_ARAD_FBC_PRIORITY_LENGTH                (2)


#define JER2_ARAD_FABRIC_ALDWP_MIN                   (0x2)
#define JER2_ARAD_FABRIC_ALDWP_MAX                   (0xf)
#define JER2_ARAD_FABRIC_VSC128_MAX_CELL_SIZE        (128 /*data*/ + 8 /*header*/ + 1 /*CRC*/)
#define JER2_ARAD_FABRIC_VSC256_MAX_CELL_SIZE        (256 /*data*/ + 11 /*header*/ + 1/*CRC*/)


/*GCI backoff random mask configuration*/
#define JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_1_LEVEL_0       (0x1)
#define JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_1_LEVEL_1       (0xf)
#define JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_1_LEVEL_2       (0x7f)
#define JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_1_LEVEL_3       (0x3ff)

#define JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_2_LEVEL_0       (0x3)
#define JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_2_LEVEL_1       (0x1f)
#define JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_2_LEVEL_2       (0xff)
#define JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_2_LEVEL_3       (0x7ff)

#define JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_3_LEVEL_0       (0x7)
#define JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_3_LEVEL_1       (0x3f)
#define JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_3_LEVEL_2       (0x1ff)
#define JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_3_LEVEL_3       (0xfff)
/* } */

/*************
 * MACROS    *
 *************/
/* { */
#define JER2_ARAD_FBC_LINK_IN_MAC(unit, link_id) ((link_id) % SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac))
#define JER2_ARAD_FBC_MAC_OF_LINK(unit, link_id) ((link_id) / SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac))

#define JER2_ARAD_FBC_IS_MAC_COMBO_NIF_NOT_FBR(mac_id, is_combo_nif_not_fabric) \
  ( (mac_id == SOC_DNX_DEFS_GET(unit, nof_instances_fmac)-1) && (is_combo_nif_not_fabric) )
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
*     jer2_arad_fabric_init
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
uint32
  jer2_arad_fabric_init(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_INIT_FABRIC*      fabric
  );

/*********************************************************************
* NAME:
*    jer2_arad_fabric_regs_init
* FUNCTION:
*   Initialization of the Arad blocks configured in this module.
*   This function directly accesses registers/tables for
*   initializations that are not covered by API-s
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  jer2_arad_fabric_regs_init(
    DNX_SAND_IN  int                 unit
  );

/*********************************************************************
* NAME:
*     jer2_arad_fabric_common_regs_init
* FUNCTION:
*   Initialization of the common blocks from JER2_ARAD, JER2_JERICHO configured in this module.
*   This function directly accesses registers/tables for
*   initializations that are not covered by API-s
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  jer2_arad_fabric_common_regs_init(
    DNX_SAND_IN  int                 unit
  );


/*********************************************************************
* NAME:
*     jer2_arad_fabric_fc_enable_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Feb 12 2008
* FUNCTION:
*     This procedure enables/disables flow-control on fabric
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  JER2_ARAD_CONNECTION_DIRECTION    direction_ndx -
*     Direction index - RX, TX or Both.
*  DNX_SAND_IN  JER2_ARAD_FABRIC_FC           *info -
*     Struct holds the enable/disable link level flow control
*     information.
* REMARKS:
*     None.
* RETURNS:
*     The get function is not entirely symmetric to the set function
*     (where only rx, tx or both directions can be defined). The get
*     function returns the both directions.
*********************************************************************/
uint32
  jer2_arad_fabric_fc_enable_set_unsafe(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  JER2_ARAD_CONNECTION_DIRECTION direction_ndx,
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC            *info
  );

/*********************************************************************
* NAME:
*     jer2_arad_fabric_fc_enable_verify
* TYPE:
*   PROC
* DATE:
*   Feb 12 2008
* FUNCTION:
*     This procedure enables/disables flow-control on fabric
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  JER2_ARAD_CONNECTION_DIRECTION    direction_ndx -
*     Direction index - RX, TX or Both.
*  DNX_SAND_IN  JER2_ARAD_FABRIC_FC           *info -
*     Struct holds the enable/disable flow control
*     information.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_fabric_fc_enable_verify(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  JER2_ARAD_CONNECTION_DIRECTION direction_ndx,
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC            *info
  );

/*********************************************************************
* NAME:
*     jer2_arad_fabric_fc_enable_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Feb 12 2008
* FUNCTION:
*     This procedure enables/disables flow-control on fabric
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_OUT JER2_ARAD_FABRIC_FC           *info_rx -
*     Struct holds the enable/disable flow control
*     information for rx.
*  DNX_SAND_OUT JER2_ARAD_FABRIC_FC           *info_tx -
*     Struct holds the enable/disable flow control
*     information for tx.
* RETURNS:
*     OK or ERROR indication.
* REMARKS:
*     Not entirely symmetric to the set function (where only rx,
*     tx or both directions can be defined). The get function returns
*     the both directions.
*********************************************************************/
uint32
  jer2_arad_fabric_fc_enable_get_unsafe(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC            *info_rx,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC            *info_tx
  );

/*********************************************************************  
* NAME:
*     jer2_arad_fabric_flow_control_init
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure init fabric flow-control on fabric
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  JER2_ARAD_CONNECTION_DIRECTION    direction_ndx -
*     Direction index - RX, TX or Both.
*  DNX_SAND_IN  JER2_ARAD_FABRIC_FC           *info -
*     Struct holds the enable/disable link level flow control
*     information.
* REMARKS:
*     None.
*********************************************************************/
uint32
  jer2_arad_fabric_flow_control_init(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  JER2_ARAD_CONNECTION_DIRECTION direction_ndx,
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC            *info
  );


/*********************************************************************  
* NAME:
*     jer2_arad_fabric_gci_backoff_masks_init
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure init fabric flow-control on fabric
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     init gci backoff masks
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer2_arad_fabric_gci_backoff_masks_init(
    DNX_SAND_IN  int                 unit
  );
/*********************************************************************  
* NAME:
*     jer2_arad_fabric_scheduler_adaptation_init
* DATE:
*   December 18 2013
* FUNCTION:
*     This procedure sets the scheduler adaptation to links' states
* INPUT:
*   DNX_SAND_IN  int                unit
* REMARKS:
*     None.
*********************************************************************/
uint32
    jer2_arad_fabric_scheduler_adaptation_init(
       DNX_SAND_IN int    unit
  );

/*********************************************************************
* NAME:
*     jer2_arad_fabric_fc_shaper_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Feb 12 2008
* FUNCTION:
*     This procedure sets parameters of both fabric link 
*     data shapers and flow control shapers. 
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32                 link_ndx -
*     The fabric link index. Range: 0 - 35.
*  DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  shaper_mode -
*    The mode of the shapers - shape transmitted bytes
*    or transmitted cells.
*  DNX_SAND_IN JER2_ARAD_FABRIC_FC_SHAPER    *info -
*     Struct holds the data shaper and
*     flow control shapers information.
*  DNX_SAND_OUT JER2_ARAD_FABRIC_FC_SHAPER    *exact_info -
*     Struct holds the exact data shaper and
*     flow control shapers information that was set.
*  
* RETURNS:
*     OK or ERROR indication.
* REMARKS:
*     
*********************************************************************/
uint32
  jer2_arad_fabric_fc_shaper_set_unsafe(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  uint32                        link_ndx,
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  *shaper_mode,  
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER            *info,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC_SHAPER            *exact_info
  );


/*********************************************************************
* NAME:
*     jer2_arad_fabric_fc_shaper_verify
* TYPE:
*   PROC
* DATE:
*   Feb 12 2008
* FUNCTION:
*     This procedure sets parameters of both fabric link 
*     data shapers and flow control shapers. 
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32                 link_ndx -
*     The fabric link index. Range: 0 - 35.
*  DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  shaper_mode -
*    The mode of the shapers - shape transmitted bytes
*    or transmitted cells.
*  DNX_SAND_IN JER2_ARAD_FABRIC_FC_SHAPER    *info -
*     Struct holds the data shaper and
*     flow control shapers information.
*  DNX_SAND_OUT JER2_ARAD_FABRIC_FC_SHAPER    *exact_info -
*     Struct holds the exact data shaper and
*     flow control shapers information that was set.
*  
* RETURNS:
*     OK or ERROR indication.
* REMARKS:
*     
*********************************************************************/
uint32
  jer2_arad_fabric_fc_shaper_verify(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  uint32                        link_ndx,
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  *shaper_mode,  
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER            *info,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC_SHAPER            *exact_info
  );


/*********************************************************************
* NAME:
*     jer2_arad_fabric_fc_shaper_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Feb 12 2008
* FUNCTION:
*     This procedure sets parameters of both fabric link 
*     data shapers and flow control shapers. 
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32                 link_ndx -
*     The fabric link index. Range: 0 - 35.
*  DNX_SAND_OUT JER2_ARAD_FABRIC_FC_SHAPER    *info -
*     Struct holds the data shaper and
*     flow control shaper information of the link.
*  DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  shaper_mode -
*    The mode of the shapers - shape transmitted bytes
*    or transmitted cells.
*  
* RETURNS:
*     OK or ERROR indication.
* REMARKS:
*     
*********************************************************************/
uint32
  jer2_arad_fabric_fc_shaper_get_unsafe(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  uint32                         link_ndx,
    DNX_SAND_OUT  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  *shaper_mode,  
    DNX_SAND_OUT  JER2_ARAD_FABRIC_FC_SHAPER            *info
  );


/*********************************************************************
* NAME: 
*     jer2_arad_fabric_mesh_topology_values_config
* DATE:
*     Mar 11 2013
* FUNCTION:
*     This procedure set the mesh topology registers for mesh mode
* INPUT: 
*      DNX_SAND_IN  int                 unit
*      DNX_SAND_IN  uint32                 speed- the sfi ports speed
*      DNX_SAND_IN  JER2_ARAD_PORT_PCS          pcs- the sfi ports encoding
* RETURNS:
*     OK or ERROR indication.
* REMARKS: 
*     We assume all fabric links are configured to the same speed and encoding
*********************************************************************/
int
  jer2_arad_fabric_mesh_topology_values_config(
    DNX_SAND_IN int unit,
    DNX_SAND_IN int speed,
    DNX_SAND_IN soc_dnxc_port_pcs_t pcs
  );
/*********************************************************************
* NAME:
*     jer2_arad_fabric_stand_alone_fap_mode_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Feb 12 2008
* FUNCTION:
*  Used internally during initialization
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint8                 is_single_fap_mode -
*     Indicator. TRUE - Device is in STANDALONE mode. FALSE -
*     Device is NOT in STANDALONE mode.
* REMARKS:
*     None. 
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_fabric_stand_alone_fap_mode_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 is_single_fap_mode
  );

/*********************************************************************
* NAME:
*     jer2_arad_fabric_stand_alone_fap_mode_verify
* TYPE:
*   PROC
* DATE:
*   Feb 12 2008
* FUNCTION:
*  Used internally during initialization
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint8                 is_single_fap_mode -
*     Indicator. TRUE - Device is in STANDALONE mode. FALSE -
*     Device is NOT in STANDALONE mode.
* REMARKS:
*     None. 
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_fabric_stand_alone_fap_mode_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 is_single_fap_mode
  );

/*********************************************************************
* NAME:
*     jer2_arad_fabric_stand_alone_fap_mode_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Feb 12 2008
* FUNCTION:
*  Used internally during initialization (or if guided by support for debug)
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_OUT uint8                 *is_single_fap_mode -
*     Indicator. TRUE - Device is in STANDALONE mode. FALSE -
*     Device is NOT in STANDALONE mode.
* REMARKS:
*     None. 
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_fabric_stand_alone_fap_mode_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint8                 *is_single_fap_mode
  );
/*********************************************************************
* NAME:
*     jer2_arad_fabric_connect_mode_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Feb 12 2008
* FUNCTION:
*     Configure the fabric mode to work in one of the
*     following modes: FE, back to back, mesh or multi stage
*     FE.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  JER2_ARAD_FABRIC_CONNECT_MODE fabric_mode -
*     The fabric connection mode to set.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_fabric_connect_mode_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_FABRIC_CONNECT_MODE fabric_mode
  );

/*********************************************************************
* NAME:
*     jer2_arad_fabric_connect_mode_verify
* TYPE:
*   PROC
* DATE:
*   Feb 12 2008
* FUNCTION:
*     Configure the fabric mode to work in one of the
*     following modes: FE, back to back, mesh or multi stage
*     FE.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  JER2_ARAD_FABRIC_CONNECT_MODE fabric_mode -
*     The fabric connection mode to set.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_fabric_connect_mode_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_FABRIC_CONNECT_MODE fabric_mode
  );


/*********************************************************************
* NAME:
*     jer2_arad_fabric_topology_status_connectivity_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Retrieve the connectivity map from the device.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  int                 link_index_min -
*     The first link index which this API reterives the info
*  DNX_SAND_IN  int                 link_index_max -
*     The last link index which this API reterives the info
*  DNX_SAND_OUT JER2_ARAD_FABRIC_LINKS_CON_STAT_INFO_ARR *connectivity_map -
*     The connectivity map
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_fabric_topology_status_connectivity_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                    link_index_min,
    DNX_SAND_IN  int                    link_index_max,
    DNX_SAND_OUT JER2_ARAD_FABRIC_LINKS_CON_STAT_INFO_ARR *connectivity_map
  );
/*********************************************************************
* NAME:
*     jer2_arad_link_on_off_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Set Fabric link, and optionally, the appropriate SerDes,
*     on/off state.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint32                 link_ndx -
*     The fabric link index. Range: 0 - 35.
*  DNX_SAND_IN  JER2_ARAD_LINK_STATE_INFO     *info -
*     Fabric link on/off state.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
int
  jer2_arad_link_on_off_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  soc_port_t               port,
    DNX_SAND_IN  JER2_ARAD_LINK_STATE_INFO     *info
  );

soc_error_t
    jer2_arad_link_power_set(
        int unit, 
        soc_port_t port, 
        uint32 flags, 
        soc_dnxc_port_power_t power
        );

/*********************************************************************
* NAME:
*     jer2_arad_link_on_off_verify
* TYPE:
*   PROC
* FUNCTION:
*     Set Fabric link, and optionally, the appropriate SerDes,
*     on/off state.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint32                 link_ndx -
*     The fabric link index. Range: 0 - 35.
*  DNX_SAND_IN  JER2_ARAD_LINK_STATE_INFO     *info -
*     Fabric link on/off state.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_link_on_off_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 link_ndx,
    DNX_SAND_IN  JER2_ARAD_LINK_STATE_INFO     *info
  );

/*********************************************************************
* NAME:
*     jer2_arad_link_on_off_get
* TYPE:
*   PROC
* FUNCTION:
*     Set Fabric link, and optionally, the appropriate SerDes,
*     on/off state.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint32                 link_ndx -
*     The fabric link index. Range: 0 - 35.
*  DNX_SAND_OUT JER2_ARAD_LINK_STATE_INFO     *info -
*     Fabric link on/off state.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
int
  jer2_arad_link_on_off_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  soc_port_t             port,
    DNX_SAND_OUT JER2_ARAD_LINK_STATE_INFO   *info
  );

soc_error_t
    jer2_arad_link_power_get(
        int unit, 
        soc_port_t port, 
        soc_dnxc_port_power_t* power
        );

/*
 * Function:
 *      jer2_arad_fabric_reachability_status_get
 * Purpose:
 *      Get reachability status
 * Parameters:
 *      unit                - (IN)  Unit number.
 *      moduleid            - (IN)  Module to check reachbility to
 *      links_max           - (IN)  Max size of links_array
 *      links_array         - (OUT) Links which moduleid is erachable through
 *      links_count         - (OUT) Size of links_array
 * Returns:
 *      SOC_E_xxx
 */

soc_error_t
jer2_arad_fabric_reachability_status_get(
  int unit,
  int moduleid,
  int links_max,
  uint32 *links_array,
  int *links_count
);

/*
 * Function:
 *      jer2_arad_fabric_link_status_all_get
 * Purpose:
 *      Get all links status
 * Parameters:
 *      unit                 - (IN)  Unit number.
 *      links_array_max_size - (IN)  max szie of link_status array
 *      link_status          - (OUT) array of link status per link
 *      errored_token_count  - (OUT) array error token count per link
 *      links_array_count    - (OUT) array actual size
 * Returns:
 *      SOC_E_xxx
 */

soc_error_t
jer2_arad_fabric_link_status_all_get(
  int unit,
  int links_array_max_size,
  uint32* link_status,
  uint32* errored_token_count,
  int* links_array_count
);

/*
 * Function:
 *      jer2_arad_fabric_link_status_clear
 * Purpose:
 *      clear link status
 * Parameters:
 *      unit                - (IN)  Unit number
 *      link                - (IN) Link #
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
jer2_arad_fabric_link_status_clear(
    int unit,
    soc_port_t link
);


/*
 * Function:
 *      jer2_arad_dnx_fabric_link_status_get
 * Purpose:
 *      Get link status
 * Parameters:
 *      unit                - (IN)  Unit number.
 *      link_id             - (IN)  Link
 *      link_status         - (OUT) According to link status get
 *      errored_token_count - (OUT) Errored token count
 * Returns:
 *      SOC_E_xxx
 */

soc_error_t
jer2_arad_fabric_link_status_get(
  int unit,
  bcm_port_t link_id,
  uint32 *link_status,
  uint32 *errored_token_count
);

/*
* Function:
*      jer2_arad_fabric_loopback_set
* Purpose:
*      Set port loopback
* Parameters:
*      unit      - (IN)  Unit number.
*      port      - (IN)  port number 
*      loopback  - (IN)  soc_dnx_loopback_mode_t
* Returns:
*      SOC_E_xxx
* Notes:
*/

soc_error_t
jer2_arad_fabric_loopback_set(
  int unit,
  soc_port_t port,
  soc_dnxc_loopback_mode_t loopback
);

/*
* Function:
*      jer2_arad_fabric_loopback_get
* Purpose:
*      Get port loopback
* Parameters:
*      unit      - (IN)  Unit number.
*      port      - (IN)  port number 
*      loopback  - (OUT) soc_dnx_loopback_mode_t
* Returns:
*      SOC_E_xxx
* Notes:
*/

soc_error_t
jer2_arad_fabric_loopback_get(
  int unit,
  soc_port_t port,
  soc_dnxc_loopback_mode_t* loopback
);

soc_error_t jer2_arad_link_control_strip_crc_set(int unit, soc_port_t port, int strip_crc);
soc_error_t jer2_arad_link_control_strip_crc_get(int unit, soc_port_t port, int* strip_crc);

soc_error_t jer2_arad_link_control_rx_enable_set(int unit, soc_port_t port, uint32 flags, int enable);
soc_error_t jer2_arad_link_control_tx_enable_set(int unit, soc_port_t port, int enable);
soc_error_t jer2_arad_link_control_rx_enable_get(int unit, soc_port_t port, int* enable);
soc_error_t jer2_arad_link_control_tx_enable_get(int unit, soc_port_t port, int* enable);

/*
 * Function:
 *      jer2_arad_link_port_fault_get
 * Purpose:
 *      Get port loopback
 * Parameters:
 *      unit -  (IN)  BCM device number 
 *      port -  (IN)  Device or logical port number .
 *      flags - (OUT) Flags to indicate fault type 
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
jer2_arad_link_port_fault_get(
  int unit,
  bcm_port_t link_id,
  uint32* flags
);

soc_error_t jer2_arad_link_port_bucket_fill_rate_validate(int unit, uint32 bucket_fill_rate);

/*
 * Function:
 *      jer2_arad_fabric_links_cell_interleaving_set
 * Purpose:
 *      Set link interleaving mode
 * Parameters:
 *      unit  - (IN) Unit number.
 *      link  - (IN) Link
 *      val   - (IN) Is link in interleaving mode (0-false, 1-true)
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
jer2_arad_fabric_links_cell_interleaving_set(int unit, soc_port_t link, int val);

/*
 * Function:
 *      jer2_arad_fabric_links_cell_interleaving_get
 * Purpose:
 *      Get link interleaving mode
 * Parameters:
 *      unit  - (IN)  Unit number.
 *      link  - (IN)  Link
 *      val   - (OUT) Is link in interleaving mode (0-false, 1-true)
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
jer2_arad_fabric_links_cell_interleaving_get(int unit, soc_port_t link, int* val);

/*
 * Function:
 *      jer2_arad_fabric_links_llf_control_source_set
 * Purpose:
 *      Set LLFC control source
 * Parameters:
 *      unit     - (IN) Unit number.
 *      link     - (IN) Link
 *      val      - (IN) LLFC control source
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
jer2_arad_fabric_links_llf_control_source_set(int unit, soc_port_t link, soc_dnxc_fabric_control_source_t val);

/*
 * Function:
 *      soc_fe1600_fabric_links_llf_control_source_get
 * Purpose:
 *      Get LLFC control source
 * Parameters:
 *      unit     - (IN)  Unit number.
 *      link     - (IN)  Link
 *      val      - (OUT) LLFC control source
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
jer2_arad_fabric_links_llf_control_source_get(int unit, soc_port_t link, soc_dnxc_fabric_control_source_t* val);

/*
 * Function:
 *      jer2_arad_fabric_links_isolate_set
 * Purpose:
 *      Activate / Isolate link
 * Parameters:
 *      unit  - (IN) Unit number.
 *      link  - (IN) Link to activate / isolate
 *      val   - (IN) Activate / Isolate
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
jer2_arad_fabric_links_isolate_set(int unit, soc_port_t link, soc_dnxc_isolation_status_t val);

/*
 * Function:
 *      jer2_arad_fabric_links_isolate_set
 * Purpose:
 *      Get link state (Activated / Isolated)
 * Parameters:
 *      unit  - (IN)  Unit number.
 *      link  - (IN)  Link
 *      val   - (OUT) Link state (Activated / Isolated)
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
jer2_arad_fabric_links_isolate_get(int unit, soc_port_t link, soc_dnxc_isolation_status_t* val);

/*
 * Function:
 *      jer2_arad_fabric_links_nof_links_get
 * Purpose:
 *      Get number of links
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      nof_links  - (OUT) Number of links.
 * Returns:
 *      Number of links
 */
soc_error_t 
jer2_arad_fabric_links_nof_links_get(int unit, int* nof_links);

/*
 * Function:
 *      jer2_arad_fabric_force_signal_detect_set
 * Purpose:
 *      enable force signal detect in a mac
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      mac_instance  - (IN) MAC index.
 * Returns:
 *      Number of links
 */
soc_error_t
jer2_arad_fabric_force_signal_detect_set(int unit, int mac_instance);

/*********************************************************************
* NAME:
*     jer2_arad_fabric_cell_format_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Mar 25 2012
* FUNCTION:
*     This procedure sets fabric links operation mode.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  JER2_ARAD_FABRIC_CELL_FORMAT  *info -
*     Fabric links operation mode.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_fabric_cell_format_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_FABRIC_CELL_FORMAT  *info
  );


/*********************************************************************
* NAME:
*     jer2_arad_fabric_link_up_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Apr 25 2012
* FUNCTION:
*     This procedure get link status up/down
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32                 link_idx -
*  DNX_SAND_OUT uint32                 *up -
*     
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
int
  jer2_arad_fabric_link_up_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  soc_port_t          port,
    DNX_SAND_OUT int                 *up
  );

/*********************************************************************
* NAME:
*     jer2_arad_fabric_nof_links_get
* TYPE:
*   PROC
* DATE:
*   Apr 25 2013
* FUNCTION:
*     Get nof fabric links
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_OUT uint32                 *nof_links -
*     
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  jer2_arad_fabric_nof_links_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT int                    *nof_links
  );

/*
 * Function:
 *      jer2_arad_fabric_port_speed_get
 * Purpose:
 *      Setting the speed for a given port
 * Parameters:
 *      unit - unit #.
 *      port - port #.
 *      speed (OUT)- Value in megabits/sec (10, 100, etc)
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_XXX
 */
int
jer2_arad_fabric_port_speed_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN uint32                   port,
    DNX_SAND_OUT int                    *speed
);

/*
 * Function:
 *      jer2_arad_fabric_port_speed_set
 * Purpose:
 *      Setting the speed for a given port
 * Parameters:
 *      unit - unit #.
 *      port - port #.
 *      speed - Value in megabits/sec (10, 100, etc)
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_XXX
 */
uint32
jer2_arad_fabric_port_speed_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN uint32                  port,
    DNX_SAND_IN int                     speed
);

/*********************************************************************
* NAME:
*     jer2_arad_fabric_priority_set
* TYPE:
*   PROC
* DATE:
*   June 24 2012
* FUNCTION:
*      Setting fabric priority according to: traffic_class, is_tdm(flags), queue_type: hc/lc (flags), color.
* INPUT:
*      unit - unit #.
*      tc - tc #.
*      dp - color #
*      flags - BCM_FABRIC_QUEUE_PRIORITY_HIGH_ONLY, BCM_FABRIC_QUEUE_PRIORITY_LOW_ONLY, BCM_FABRIC_PRIORITY_TDM.
*      fabric_priority - value to set
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

soc_error_t
jer2_arad_fabric_priority_set(
    DNX_SAND_IN  int                              unit, 
    DNX_SAND_IN  uint32                              tc, 
    DNX_SAND_IN  uint32                              dp,
    DNX_SAND_IN  uint32                              flags,
    DNX_SAND_IN  int                               fabric_priority
  );

/*********************************************************************
* NAME:
*     jer2_arad_fabric_priority_set
* TYPE:
*   PROC
* DATE:
*   June 24 2012
* FUNCTION:
*      Getting fabric priority according to: traffic_class, is_tdm(flags), queue_type: hc/lc (flags), color.
* INPUT:
*      unit - unit #.
*      tc - tc #.
*      dp - color #
*      flags - BCM_FABRIC_QUEUE_PRIORITY_HIGH_ONLY, BCM_FABRIC_QUEUE_PRIORITY_LOW_ONLY, BCM_FABRIC_PRIORITY_TDM.
*      fabric_priority (out).
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
soc_error_t
jer2_arad_fabric_priority_get(
    DNX_SAND_IN  int                              unit, 
    DNX_SAND_IN  uint32                              tc, 
    DNX_SAND_IN  uint32                              dp,
    DNX_SAND_IN  uint32                              flags,
    DNX_SAND_OUT int                                 *fabric_priority
  );

/*
 * Function:
 *      jer2_arad_fabric_port_wcmod_ucode_load
 * Purpose:
 *      optimizing loading firmware time
 * Parameters:
 *      unit - Unit #.
 *      port - port #. (check if the firmware is already loaded for the specific port, but load firmware fo all ports if necessary
 *      arr - wcmod table
 *      arr_len - wcmod table length
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_XXX
 */
uint32
jer2_arad_fabric_wcmod_ucode_load(
    int unit,
    int port, 
    uint8 *arr, 
    int arr_len
);
/*********************************************************************
* NAME:
*     jer2_arad_fabric_gci_enable_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*      Set enbale state of the Local GCI mechanism (random backoff or leaky bucket)
* INPUT:
*      unit - unit #.
*      type - GCI mechanism (random backoff or leaky bucket)
*      value - 
* REMARKS:
*     Enable/disable per pipe is not supported.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32 jer2_arad_fabric_gci_enable_set ( 
    DNX_SAND_IN  int                                 unit,  
    DNX_SAND_IN  JER2_ARAD_FABRIC_GCI_TYPE                   type,
    DNX_SAND_OUT int                                    value
);

/*********************************************************************
* NAME:
*     jer2_arad_fabric_gci_enable_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*      Get enbale state of the Local GCI mechanism (random backoff or leaky bucket)
* INPUT:
*      unit - unit #.
*      type - GCI mechanism (random backoff or leaky bucket)
*      value - 
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32 jer2_arad_fabric_gci_enable_get ( 
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  JER2_ARAD_FABRIC_GCI_TYPE                   type,  
    DNX_SAND_OUT int                                    *value
);

/*********************************************************************
* NAME:
*     jer2_arad_fabric_gci_config_set
* TYPE:
*   PROC
* DATE:
*   April 30 2012
* FUNCTION:
*      Set GCI  related parameters.
* INPUT:
*      unit - unit #.
*      type - The relevant parameter
*      value - 
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32 jer2_arad_fabric_gci_config_set ( 
    DNX_SAND_IN  int                                 unit,  
    DNX_SAND_IN JER2_ARAD_FABRIC_GCI_CONFIG_TYPE             type,
    DNX_SAND_OUT int                                    value
);

/*********************************************************************
* NAME:
*     jer2_arad_fabric_gci_config_get
* TYPE:
*   PROC
* DATE:
*   April 30 2012
* FUNCTION:
*      Get GCI related parameters
* INPUT:
*      unit - unit #.
*      type - The relevant parameter
*      value - 
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32 jer2_arad_fabric_gci_config_get ( 
    DNX_SAND_IN  int                                 unit,  
    DNX_SAND_IN JER2_ARAD_FABRIC_GCI_CONFIG_TYPE             type,
    DNX_SAND_OUT int                                    *value
);

/*********************************************************************
* NAME:
*     jer2_arad_fabric_llfc_threshold_set
* TYPE:
*   PROC
* DATE:
*   April 30 2012
* FUNCTION:
*      Set llfc related thresholds.
* INPUT:
*      unit - unit #.
*      value - 
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32 jer2_arad_fabric_llfc_threshold_set ( 
    DNX_SAND_IN  int                                 unit,  
    DNX_SAND_OUT int                                    value
);

/*********************************************************************
* NAME:
*     jer2_arad_fabric_llfc_threshold_get
* TYPE:
*   PROC
* DATE:
*   May 2012
* FUNCTION:
*      Get llfc related thresholds.
* INPUT:
*      unit - unit #.
*      value - 
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32 jer2_arad_fabric_llfc_threshold_get ( 
    DNX_SAND_IN  int                                 unit,  
    DNX_SAND_OUT int                                    *value
);
/*********************************************************************
* NAME:
*     jer2_arad_fabric_rci_enable_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*      Set enbale state of the Local RCI (FDR FIFOs)
* INPUT:
*      unit - unit #.
*      value - 
* REMARKS:
*     Enable/disable per pipe is not supported.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32 jer2_arad_fabric_rci_enable_set ( 
    DNX_SAND_IN  int                                 unit,  
    DNX_SAND_OUT soc_dnxc_fabric_control_source_t        value
);

/*********************************************************************
* NAME:
*     jer2_arad_fabric_rci_enable_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*      Get enbale state of the Local RCI (FDR FIFOs)
* INPUT:
*      unit - unit #.
*      value - 
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32 jer2_arad_fabric_rci_enable_get ( 
    DNX_SAND_IN  int                                 unit,  
    DNX_SAND_OUT soc_dnxc_fabric_control_source_t       *value
);

/*********************************************************************
* NAME:
*     jer2_arad_fabric_rci_config_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*      Set local rci related parameters.
* INPUT:
*      unit - unit #.
*      rci_config_type - 
*      value - 
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32 jer2_arad_fabric_rci_config_set ( 
    DNX_SAND_IN int                                unit,  
    DNX_SAND_IN JER2_ARAD_FABRIC_RCI_CONFIG_TYPE           rci_config_type,
    DNX_SAND_IN int                                   value
);

/*********************************************************************
* NAME:
*     jer2_arad_fabric_rci_config_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*      Get local rci related parameters.
* INPUT:
*      unit - unit #.
*      rci_config_type - 
*      value - 
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32 jer2_arad_fabric_rci_config_get ( 
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  JER2_ARAD_FABRIC_RCI_CONFIG_TYPE            rci_config_type,  
    DNX_SAND_OUT int                                    *value
  );

#ifdef BCM_88660_A0
/*********************************************************************
* NAME:
*     jer2_arad_fabric_empty_cell_size_set
* FUNCTION:
*   Empty cell and LLFC cell size configuration.
*   Available per FMAC instance.
*   Supported by JER2_ARAD_PLUS only.
*   The remote reapeater can process a limited number of cells (according to remote reapeater core clock),
*   Setting larger cell size will reduce the empty cells rate
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint32                 fmac_index -
*     FMAC #
*DNX_SAND_IN  uint32                    cell_size - 
*     The empty cells size in bytes.          
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
jer2_arad_fabric_empty_cell_size_set (
   DNX_SAND_IN  int                                 unit,
   DNX_SAND_IN  uint32                                 fmac_index,  
   DNX_SAND_IN  uint32                                 cell_size
);

#endif /*BCM_88660_A0*/


/*********************************************************************
* NAME:
*     jer2_arad_fabric_aldwp_config
* FUNCTION:
*   Configure ALDWP value
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
int 
jer2_arad_fabric_aldwp_config(
   DNX_SAND_IN  int                                 unit);

/*********************************************************************
* NAME:
*     jer2_arad_fabric_minimal_links_to_dest_set
* FUNCTION:
*   Configure minimum links to destination
* REMARKS: 
*   JER2_ARAD_PLUS Only 
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN soc_module_t         module_id -
*     FAP id to configure (SOC_MODID_ALL configures all FAPs) 
*  DNX_SAND_IN  uint32                 minimum_links
*     Minimum number of links to configure
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/

uint32 jer2_arad_fabric_minimal_links_to_dest_set(
    DNX_SAND_IN int                                  unit,
    DNX_SAND_IN soc_module_t                         module_id,
    DNX_SAND_IN int                                  minimum_links
   );

/*********************************************************************
* NAME:
*     jer2_arad_fabric_minimal_links_to_dest_get
* FUNCTION:
*     get minimum number of links, as configured
* REMARKS: 
*   JER2_ARAD_PLUS Only 
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN soc_module_t         module_id -
*     FAP id to configure (SOC_MODID_ALL configures all FAPs) 
*  DNX_SAND_IN  uint32                * minimum_links
*     sets minimum links to the number of minimal links as configured(0 if option is disabled)
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/


uint32 jer2_arad_fabric_minimal_links_to_dest_get(
    DNX_SAND_IN int                                 unit,
    DNX_SAND_IN soc_module_t                        module_id,
    DNX_SAND_OUT int                                *minimum_links
    );


/*********************************************************************
* NAME:
*     jer2_arad_fabric_link_tx_traffic_disable_set
* FUNCTION:
*   Allow/do not allow to send traffic on a specific link.
* INPUT:
*      unit - unit #
*      link - link id #
*      disable - 1: do not allow, 0: allow  
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/

soc_error_t jer2_arad_fabric_link_tx_traffic_disable_set(int unit, soc_port_t link, int disable);


/*********************************************************************
* NAME:
*     jer2_arad_fabric_link_tx_traffic_disable_set
* FUNCTION:
*   Get allow/do not allow to send traffic on a specific link.
* INPUT:
*      unit - unit #
*      link - link id #
*      disable (OUT)- 1: do not allow, 0: allow  
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/

soc_error_t jer2_arad_fabric_link_tx_traffic_disable_get(int unit, soc_port_t link, int *disable);


/*********************************************************************
* NAME:
*     jer2_arad_fabric_mesh_check
* FUNCTION:
*     check mesh status
* INPUT:
*  DNX_SAND_IN  int                                  unit -
*     Identifier of the device to access.
*  DNX_SAND_IN uint8                                    stand_alone,
*     Is device stand alone.
*  DNX_SAND_OUT uint8 *success - 
*     mesh status check.
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/

uint32 jer2_arad_fabric_mesh_check(int unit, uint8 stand_alone, uint8 *success);



soc_error_t jer2_arad_fabric_prbs_polynomial_set(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int value);
soc_error_t jer2_arad_fabric_prbs_polynomial_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int* value);

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __JER2_ARAD_FABRIC_INCLUDED__*/
#endif

