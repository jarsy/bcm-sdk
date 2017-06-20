/* $Id: jer2_arad_api_fabric.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER2_ARAD_API_FABRIC_INCLUDED__
/* { */
#define __JER2_ARAD_API_FABRIC_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_api_general.h>
#include <soc/dnx/legacy/TMC/tmc_api_fabric.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Maximal number of devices in coexist mode               */
#define  JER2_ARAD_FABRIC_NOF_COEXIST_DEVICES (DNX_TMC_FABRIC_NOF_COEXIST_DEVICES)

/*     Coexist mode devices - each entry identifies 2
 *     consecutive devices                                     */
#define  JER2_ARAD_FABRIC_NOF_COEXIST_DEV_ENTRIES (DNX_TMC_FABRIC_NOF_COEXIST_DEV_ENTRIES)

/*     Maximal number of fap20v devices in the system          */
#define  JER2_ARAD_FABRIC_NOF_FAP20_DEVICES (DNX_TMC_FABRIC_NOF_FAP20_DEVICES)

/*     Maximal number of devices in mesh mode                  */
#define  JER2_ARAD_FABRIC_MESH_DEVICES_MAX (DNX_TMC_FABRIC_MESH_DEVICES_MAX)

/*      Maximal value of BYTES between consecutive comma bursts (when Cm_Tx_Byte_Mode is enabled).       */
#define JER2_ARAD_FABRIC_SHAPER_BYTES_MAX  (DNX_TMC_FABRIC_SHAPER_BYTES_MAX)

/*      Maximal value of CELLS between consecutive comma bursts (when Cm_Tx_Byte_Mode is disabled).       */
#define JER2_ARAD_FABRIC_SHAPER_CELLS_MAX  (DNX_TMC_FABRIC_SHAPER_CELLS_MAX)

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

#define JER2_ARAD_LINK_ERR_TYPE_CRC                            DNX_TMC_LINK_ERR_TYPE_CRC
#define JER2_ARAD_LINK_ERR_TYPE_SIZE                           DNX_TMC_LINK_ERR_TYPE_SIZE
#define JER2_ARAD_LINK_ERR_TYPE_MISALIGN                       DNX_TMC_LINK_ERR_TYPE_MISALIGN
#define JER2_ARAD_LINK_ERR_TYPE_CODE_GROUP                     DNX_TMC_LINK_ERR_TYPE_CODE_GROUP
typedef DNX_TMC_LINK_ERR_TYPE                                  JER2_ARAD_LINK_ERR_TYPE;

#define JER2_ARAD_LINK_INDICATE_TYPE_SIG_LOCK                  DNX_TMC_LINK_INDICATE_TYPE_SIG_LOCK
#define JER2_ARAD_LINK_INDICATE_TYPE_ACCEPT_CELL               DNX_TMC_LINK_INDICATE_TYPE_ACCEPT_CELL
#define JER2_ARAD_LINK_INDICATE_INTRNL_FIXED                   DNX_TMC_LINK_INDICATE_INTRNL_FIXED
typedef DNX_TMC_LINK_INDICATE_TYPE                             JER2_ARAD_LINK_INDICATE_TYPE;

#define JER2_ARAD_FABRIC_CONNECT_MODE_FE                       DNX_TMC_FABRIC_CONNECT_MODE_FE
#define JER2_ARAD_FABRIC_CONNECT_MODE_BACK2BACK                DNX_TMC_FABRIC_CONNECT_MODE_BACK2BACK
#define JER2_ARAD_FABRIC_CONNECT_MODE_MESH                     DNX_TMC_FABRIC_CONNECT_MODE_MESH
#define JER2_ARAD_FABRIC_CONNECT_MODE_MULT_STAGE_FE            DNX_TMC_FABRIC_CONNECT_MODE_MULT_STAGE_FE
#define JER2_ARAD_FABRIC_CONNECT_MODE_SINGLE_FAP               DNX_TMC_FABRIC_CONNECT_MODE_SINGLE_FAP
#define JER2_ARAD_FABRIC_NOF_CONNECT_MODES                     DNX_TMC_FABRIC_NOF_CONNECT_MODES
typedef DNX_TMC_FABRIC_CONNECT_MODE                            JER2_ARAD_FABRIC_CONNECT_MODE;

#define JER2_ARAD_LINK_STATE_ON                                DNX_TMC_LINK_STATE_ON
#define JER2_ARAD_LINK_STATE_OFF                               DNX_TMC_LINK_STATE_OFF
#define JER2_ARAD_LINK_NOF_STATES                              DNX_TMC_LINK_NOF_STATES
typedef DNX_TMC_LINK_STATE                                     JER2_ARAD_LINK_STATE;

#define JER2_ARAD_FABRIC_LINE_CODING_8_10                      DNX_TMC_FABRIC_LINE_CODING_8_10
#define JER2_ARAD_FABRIC_LINE_CODING_8_9_FEC                   DNX_TMC_FABRIC_LINE_CODING_8_9_FEC
#define JER2_ARAD_FABRIC_NOF_LINE_CODINGS                      DNX_TMC_FABRIC_NOF_LINE_CODINGS
typedef DNX_TMC_FABRIC_LINE_CODING                             JER2_ARAD_FABRIC_LINE_CODING;

typedef DNX_TMC_FABRIC_LINKS_CONNECT_MAP_STAT_INFO             JER2_ARAD_FABRIC_LINKS_CONNECT_MAP_STAT_INFO;
typedef DNX_TMC_FABRIC_LINKS_CON_STAT_INFO_ARR                 JER2_ARAD_FABRIC_LINKS_CON_STAT_INFO_ARR;
typedef DNX_TMC_FABRIC_CELL_FORMAT                             JER2_ARAD_FABRIC_CELL_FORMAT;
typedef DNX_TMC_FABRIC_FC                                      JER2_ARAD_FABRIC_FC;
typedef DNX_TMC_FABRIC_COEXIST_INFO                            JER2_ARAD_FABRIC_COEXIST_INFO;
typedef DNX_TMC_FABRIC_LINKS_STATUS_SINGLE                     JER2_ARAD_FABRIC_LINKS_STATUS_SINGLE;
typedef DNX_TMC_FABRIC_LINKS_STATUS_ALL                        JER2_ARAD_FABRIC_LINKS_STATUS_ALL;
typedef DNX_TMC_LINK_STATE_INFO                                JER2_ARAD_LINK_STATE_INFO;

#define JER2_ARAD_FAR_NOF_DEVICE_TYPES                         DNX_TMC_FAR_NOF_DEVICE_TYPES

#define JER2_ARAD_SRD_POWER_STATE_UP                           DNX_TMC_SRD_POWER_STATE_UP      
#define JER2_ARAD_SRD_POWER_STATE_DOWN                         DNX_TMC_SRD_POWER_STATE_DOWN  
#define JER2_ARAD_SRD_POWER_STATE_UP_AND_RELOCK                DNX_TMC_SRD_POWER_STATE_UP_AND_RELOCK
#define JER2_ARAD_SRD_NOF_POWER_STATES                         DNX_TMC_SRD_NOF_POWER_STATES                           
typedef DNX_TMC_SRD_POWER_STATE                                JER2_ARAD_SRD_POWER_STATE;


#define JER2_ARAD_FABRIC_SHAPER_BYTES_MODE                     DNX_TMC_FABRIC_SHAPER_BYTES_MODE
#define JER2_ARAD_FABRIC_SHAPER_CELLS_MODE                     DNX_TMC_FABRIC_SHAPER_CELLS_MODE
#define JER2_ARAD_FABRIC_SHAPER_NOF_MODES                      DNX_TMC_FABRIC_SHAPER_NOF_MODES
typedef DNX_TMC_FABRIC_SHAPER_MODE                             JER2_ARAD_FABRIC_SHAPER_MODE;

typedef DNX_TMC_FABRIC_FC_SHAPER_MODE_INFO                     JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO;

typedef DNX_TMC_SHAPER_INFO                                    JER2_ARAD_SHAPER_INFO;

typedef DNX_TMC_FABRIC_FC_SHAPER                              JER2_ARAD_FABRIC_FC_SHAPER;

#define JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_0                    DNX_TMC_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_0
#define JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_1                    DNX_TMC_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_1
#define JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_2                    DNX_TMC_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_2
#define JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_1_CONGESTION_TH              DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_1_CONGESTION_TH
#define JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_2_CONGESTION_TH              DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_2_CONGESTION_TH
#define JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_3_CONGESTION_TH              DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_3_CONGESTION_TH
#define JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_4_CONGESTION_TH              DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_4_CONGESTION_TH
#define JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_1_FULL                       DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_1_FULL
#define JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_2_FULL                       DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_2_FULL
#define JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_3_FULL                       DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_3_FULL
#define JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_4_FULL                       DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_4_FULL
#define JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_NOF                             DNX_TMC_FABRIC_GCI_CONFIG_TYPE_NOF
typedef DNX_TMC_FABRIC_GCI_CONFIG_TYPE                              JER2_ARAD_FABRIC_GCI_CONFIG_TYPE;

#define JER2_ARAD_FABRIC_GCI_TYPE_LEAKY_BUCKET                           DNX_TMC_FABRIC_GCI_TYPE_LEAKY_BUCKET
#define JER2_ARAD_FABRIC_GCI_TYPE_RANDOM_BACKOFF                         DNX_TMC_FABRIC_GCI_TYPE_RANDOM_BACKOFF
#define JER2_ARAD_FABRIC_GCI_TYPE_NOF                                    DNX_TMC_FABRIC_GCI_TYPE_NOF
typedef DNX_TMC_FABRIC_GCI_TYPE                                     JER2_ARAD_FABRIC_GCI_TYPE;

#define JER2_ARAD_FABRIC_RCI_CONFIG_TYPE_LOCAL_RX_TH                     DNX_TMC_FABRIC_RCI_CONFIG_TYPE_LOCAL_RX_TH
#define JER2_ARAD_FABRIC_RCI_CONFIG_TYPE_INCREMENT_VALUE                 DNX_TMC_FABRIC_RCI_CONFIG_TYPE_INCREMENT_VALUE
#define JER2_ARAD_FABRIC_RCI_CONFIG_TYPE_NOF                             DNX_TMC_FABRIC_RCI_CONFIG_TYPE_NOF
typedef DNX_TMC_FABRIC_RCI_CONFIG_TYPE                              JER2_ARAD_FABRIC_RCI_CONFIG_TYPE;

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
*     jer2_arad_fabric_fc_enable_set
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
* RETURNS:
*     OK or ERROR indication.
* REMARKS:
*     The get function is not entirely symmetric to the set function
*     (where only rx, tx or both directions can be defined). The get
*     function returns the both directions.
*********************************************************************/
uint32
  jer2_arad_fabric_fc_enable_set(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  JER2_ARAD_CONNECTION_DIRECTION direction_ndx,
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC            *info
  );

/*********************************************************************
* NAME:
*     jer2_arad_fabric_fc_enable_get
* TYPE:
*   PROC
* DATE:
*   Feb 12 2008
* FUNCTION:
*     This procedure enables/disables flow-control on fabric
*     links.
* INPUT:
*  DNX_SAND_IN  int                     unit -
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
  jer2_arad_fabric_fc_enable_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC           *info_rx,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC           *info_tx
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
*  DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  *shaper_mode -
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
  jer2_arad_fabric_fc_shaper_set(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  uint32                        link_ndx,
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  *shaper_mode,  
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER            *info,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC_SHAPER            *exact_info
  );



/*********************************************************************
* NAME:
*     jer2_arad_fabric_fc_shaper_get
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
  jer2_arad_fabric_fc_shaper_get(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  uint32                         link_ndx,
    DNX_SAND_OUT  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  *shaper_mode,  
    DNX_SAND_OUT  JER2_ARAD_FABRIC_FC_SHAPER            *info
  );




/*********************************************************************
* NAME:
*     jer2_arad_fabric_stand_alone_fap_mode_get
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
  jer2_arad_fabric_stand_alone_fap_mode_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint8                 *is_single_fap_mode
  );
/*********************************************************************
* NAME:
*     jer2_arad_fabric_connect_mode_set
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
  jer2_arad_fabric_connect_mode_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_FABRIC_CONNECT_MODE fabric_mode
  );

/*********************************************************************
* NAME:
*     jer2_arad_fabric_connect_mode_get
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
*  DNX_SAND_OUT JER2_ARAD_FABRIC_CONNECT_MODE *fabric_mode -
*     The fabric connection mode to set.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_fabric_connect_mode_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT JER2_ARAD_FABRIC_CONNECT_MODE *fabric_mode
  );
/*********************************************************************
* NAME:
*     jer2_arad_fabric_topology_status_connectivity_get
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
* RETURNS:
*     OK or ERROR indication.
* REMARKS:
*     The far_link_id (link number at the connected device) indication
*     is modulo 64
*********************************************************************/
uint32
  jer2_arad_fabric_topology_status_connectivity_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                    link_index_min,
    DNX_SAND_IN  int                    link_index_max,
    DNX_SAND_OUT JER2_ARAD_FABRIC_LINKS_CON_STAT_INFO_ARR *connectivity_map
  );

/*********************************************************************
* NAME:
*     jer2_arad_fabric_link_up_get
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

void
  jer2_arad_JER2_ARAD_FABRIC_FC_clear(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC *info
  );

void
  jer2_arad_JER2_ARAD_FABRIC_FC_SHAPER_clear(
      DNX_SAND_OUT JER2_ARAD_FABRIC_FC_SHAPER *shaper
  );

void
  jer2_arad_JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO_clear(
    DNX_SAND_OUT  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  *shaper_mode
  );

void
  jer2_arad_JER2_ARAD_FABRIC_LINKS_CON_STAT_INFO_ARR_clear(
    DNX_SAND_OUT JER2_ARAD_FABRIC_LINKS_CON_STAT_INFO_ARR *info
  );

void
  jer2_arad_JER2_ARAD_FABRIC_CELL_FORMAT_clear(
    DNX_SAND_OUT JER2_ARAD_FABRIC_CELL_FORMAT *info
  );

void
  jer2_arad_JER2_ARAD_LINK_STATE_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_LINK_STATE_INFO *info
  );
const char*
  jer2_arad_JER2_ARAD_FABRIC_CONNECT_MODE_to_string(
    DNX_SAND_IN JER2_ARAD_FABRIC_CONNECT_MODE enum_val
  );


/* } */


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


/* } __JER2_ARAD_API_FABRIC_INCLUDED__*/
#endif

