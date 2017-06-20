/* $Id: jer2_jer2_jer2_tmc_api_fabric.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_jer2_jer2_tmc/include/soc_jer2_jer2_jer2_tmcapi_fabric.h
*
* MODULE PREFIX:  soc_jer2_jer2_jer2_tmcfabric
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

#ifndef __DNX_TMC_API_FABRIC_INCLUDED__
/* { */
#define __DNX_TMC_API_FABRIC_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

#include <soc/dnx/legacy/TMC/tmc_api_general.h>
#include <soc/dnx/legacy/TMC/tmc_api_ports.h>

#include <soc/dnx/legacy/dnx_config_defs.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Maximal number of devices in coexist mode               */
#define  DNX_TMC_FABRIC_NOF_COEXIST_DEVICES 256

/*     Each entry identifies 2 consecutive devices             */
#define  DNX_TMC_FABRIC_NOF_COEXIST_DEV_ENTRIES (DNX_TMC_FABRIC_NOF_COEXIST_DEVICES/2)

/*     Maximal number of fap20v devices in the system          */
#define  DNX_TMC_FABRIC_NOF_FAP20_DEVICES 128

/*     Maximal number of devices in mesh mode                  */
#define  DNX_TMC_FABRIC_MESH_DEVICES_MAX (8)

/*      Maximal value of BYTES between consecutive comma bursts (when Cm_Tx_Byte_Mode is enabled).       */
#define DNX_TMC_FABRIC_SHAPER_BYTES_MAX (65536)

/*      Maximal value of CELLS between consecutive comma bursts (when Cm_Tx_Byte_Mode is disabled).       */
#define DNX_TMC_FABRIC_SHAPER_CELLS_MAX (32768)

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
   *  crc_error Flag: Indication of whether a cell with CRC
   *  error has been detected.
   */
  DNX_TMC_LINK_ERR_TYPE_CRC=0x1,
  /*
   *  Wrong cell size is detected on MAC receive
   */
  DNX_TMC_LINK_ERR_TYPE_SIZE=0x2,
  /*
   *  misalignment_error Flag: Indication of whether a cell
   *  with misalignment error (comma) has been detected.
   */
  DNX_TMC_LINK_ERR_TYPE_MISALIGN=0x4,
  /*
   *  code_group_error Flag: Indication of whether a cell with
   *  code-group combination error (comma) has been detected.
   */
  DNX_TMC_LINK_ERR_TYPE_CODE_GROUP=0x8,
  /*
   *  Number of types in DNX_TMC_LINK_ERR_TYPE
   */
  DNX_TMC_NOF_LINK_ERR_TYPES = 4
}DNX_TMC_LINK_ERR_TYPE;

typedef enum
{
  /*
   *  sig_lock Flag: Indication of whether valid signal has
   *  been locked.
   */
  DNX_TMC_LINK_INDICATE_TYPE_SIG_LOCK=0x1,
  /*
   *  accepting_cells Flag: Indication of whether this link is
   *  'down' or not.
   */
  DNX_TMC_LINK_INDICATE_TYPE_ACCEPT_CELL=0x2,
  /*
   *  Internally fixed error: Error was detected, but fixed by
   *  the FEC mechanism (Always FALSE when FEC is disabled)
   */
  DNX_TMC_LINK_INDICATE_INTRNL_FIXED=0x4,
  /*
   *  Number of types in DNX_TMC_LINK_INDICATE_TYPE
   */
  DNX_TMC_NOF_LINK_INDICATE_TYPES = 3
}DNX_TMC_LINK_INDICATE_TYPE;

typedef enum
{
  /*
   *  Indicate FAP Fabric interface is connected to FE device.
   */
  DNX_TMC_FABRIC_CONNECT_MODE_FE=0,
  /*
   *  Indicate FAP Fabric interface is connected to another
   *  FAP device. Total are 2 FAP devices in the system. No FE
   *  devices.
   */
  DNX_TMC_FABRIC_CONNECT_MODE_BACK2BACK=1,
  /*
   *  Indicate FAP Fabric interface is connected to another 2
   *  FAP. Total are 3, 4, ... FAP devices in the system. No FE
   *  devices.
   */
  DNX_TMC_FABRIC_CONNECT_MODE_MESH=2,
  /*
   *  Indicate FAP Fabric interface is connected to FE device,
   *  and that the system is multistage system.
   */
  DNX_TMC_FABRIC_CONNECT_MODE_MULT_STAGE_FE=3,
  /*
  *  Indicate single FAP in the system, without other FAPs
  *   or FEs
  */
  DNX_TMC_FABRIC_CONNECT_MODE_SINGLE_FAP=4,
  /*
  *  Total number of fabric links modes.
  */
  DNX_TMC_FABRIC_NOF_CONNECT_MODES=5

}DNX_TMC_FABRIC_CONNECT_MODE;

typedef enum
{
  /*
   *  Link is turned on (normal operation).
   */
  DNX_TMC_LINK_STATE_ON=0,
  /*
   *  Link is turned off.
   */
  DNX_TMC_LINK_STATE_OFF=1,
  /*
   *  Total Number of fabric link states.
   */
  DNX_TMC_LINK_NOF_STATES=2
}DNX_TMC_LINK_STATE;


typedef enum 
{
  /* 
   * Link Level shaper is calculated according to transmitted cells. 
   */ 
  DNX_TMC_FABRIC_SHAPER_CELLS_MODE = 0,
  /* 
   * Link Level shaper is calculated according to transmitted bytes. 
   */ 
  DNX_TMC_FABRIC_SHAPER_BYTES_MODE = 1,
  /*
   *  Total Number of shaper control modes. 
   */
  DNX_TMC_FABRIC_SHAPER_NOF_MODES = 2

}DNX_TMC_FABRIC_SHAPER_MODE;

/*
 * RCI CONFIG: 
 * Relevat configurable RCI parameters. 
 */
typedef enum 
{

  /*LOCAL RCI threshold at the fabric RX*/
  DNX_TMC_FABRIC_RCI_CONFIG_TYPE_LOCAL_RX_TH= 0,

  /* The value that will be added to RCI bucket whenever a cell carrying RCI indication arrives */
  DNX_TMC_FABRIC_RCI_CONFIG_TYPE_INCREMENT_VALUE = 1,

  /*NOF params*/
  DNX_TMC_FABRIC_RCI_CONFIG_TYPE_NOF = 2 

} DNX_TMC_FABRIC_RCI_CONFIG_TYPE;

/* 
 * GCI Reaction mechanism
 */

typedef enum 
{
  /*
   * GCI Leaky Bucket mechanism
   */
  DNX_TMC_FABRIC_GCI_TYPE_LEAKY_BUCKET = 0,

  /*
   * GCI Random Mechanism
   */
  DNX_TMC_FABRIC_GCI_TYPE_RANDOM_BACKOFF= 1,

  /*NOF GCI TYPEs*/
  DNX_TMC_FABRIC_GCI_TYPE_NOF = 3 

} DNX_TMC_FABRIC_GCI_TYPE;



/* 
 * GCI configuration
 */

typedef enum 
{
  /*
   * GCI random back off thresholds: 
   * The GCI backoff mechanism respond according to to the congestion level
   * The congestion level configurtion includes the following thresholds.  
   */
  DNX_TMC_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_0 = 0,

  DNX_TMC_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_1 = 1,

  DNX_TMC_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_2 = 2,

  /* 
   *GCI leaky bucket config
   */

  /*Congestion threshold - when leaky baucket value is higher, it consider congested */
  DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_1_CONGESTION_TH = 3,

  DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_2_CONGESTION_TH = 4,

  DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_3_CONGESTION_TH = 5,

  DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_4_CONGESTION_TH = 6,

  /*Leaky bucket - the highest value of the leaky bucket*/
  DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_1_FULL = 7,

  DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_2_FULL = 8,

  DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_3_FULL = 9,

  DNX_TMC_FABRIC_GCI_CONFIG_TYPE_LB_4_FULL = 10,

  /*NOF thresholds*/
  DNX_TMC_FABRIC_GCI_CONFIG_TYPE_NOF = 11 

} DNX_TMC_FABRIC_GCI_CONFIG_TYPE;


typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /* 
   * Number of cell between consecutive comma sequences (in case of CELLS_MODE). 
   */ 
  uint32 cells;
  /* 
   * Number of bytes between consecutive comma sequences (in case of BYTES_MODE). 
   */ 
  uint32 bytes;
}DNX_TMC_SHAPER_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /* 
   * The link's data shaper values. 
   */
  DNX_TMC_SHAPER_INFO data_shaper;
  /*
   * The link's shaper values when under flow control. 
   * Only applies when the link fc mode is enabled.  
   */
  DNX_TMC_SHAPER_INFO fc_shaper;
}DNX_TMC_FABRIC_FC_SHAPER;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /* 
   * Link Level FC shaper mode - bytes_mode/cells_mode.
   */ 
  DNX_TMC_FABRIC_SHAPER_MODE shaper_mode; 

}DNX_TMC_FABRIC_FC_SHAPER_MODE_INFO;

typedef enum
{
  /*
   *  8b/10b line coding
   */
  DNX_TMC_FABRIC_LINE_CODING_8_10 = 0,
  /*
   *  Note applicable for Soc_petra-A. 8b/9b with FEC (fire code)
   *  linke coding
   */
  DNX_TMC_FABRIC_LINE_CODING_8_9_FEC = 1,
  /*
   *  Total number of line coding modes
   */
  DNX_TMC_FABRIC_NOF_LINE_CODINGS = 2
}DNX_TMC_FABRIC_LINE_CODING;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  The device ID on the other side of the link
   */
  uint32 far_unit;
  /*
   *  The device Type on the other side of the link
   */
  DNX_TMC_FAR_DEVICE_TYPE far_dev_type;
  /*
   *  Link number at connected device
   */
  uint32 far_link_id;
  /*
   *  Indication of whether the connected device is of the
   *  expected type (e.g. FE2 connects to FE13 if it is in
   *  multistage configuration and to FAP if it is not).
   */
  uint8 is_connected_expected;
  /*
   *  Indication of whether the link is physically connected
   *  (i.e. signal is detected).
   */
  uint8 is_phy_connect;
  /*
   *  Indication of whether the link serves for cell
   *  transmission and not just physically connected to
   *  another device. A link may be physically connected, but
   *  due to a high error rate, cannot be used for data
   *  transfer.
   */
  uint8 is_logically_connected;
}DNX_TMC_FABRIC_LINKS_CONNECT_MAP_STAT_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Per link information
   */
  DNX_TMC_FABRIC_LINKS_CONNECT_MAP_STAT_INFO link_info[SOC_DNX_DEFS_MAX(NOF_FABRIC_LINKS)];
}DNX_TMC_FABRIC_LINKS_CON_STAT_INFO_ARR;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE,variable cell size will be used. This
   *  configuration is invalid for DNX_SAND_FE200 systems.
   */
  uint8 variable_cell_size_enable;
  /*
   *  If TRUE, fabric segmentation will be performed to
   *  improve the fabric performance in fixed size cell
   *  systems.
   */
  uint8 segmentation_enable;
}DNX_TMC_FABRIC_CELL_FORMAT;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  If ENTRY i is TRUE, enables flow control for link i (0 -
   *  35) in the specified direction.
   */
  uint8 enable[SOC_DNX_DEFS_MAX(NOF_FABRIC_LINKS)];
}DNX_TMC_FABRIC_FC;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE,the chip works in co-exist mode. Coexist mode is
   *  a mode in which the fabric connects a soc_petra device to
   *  fap2x device, and enables to distribute traffic more
   *  evenly over links in this case.
   */
  uint8 enable;
  /*
   *  Define, for the first 256 device ids, if they are
   *  coexist destination (1 entry for two consecutive
   *  destinations).
   */
  uint8 coexist_dest_map[DNX_TMC_FABRIC_NOF_COEXIST_DEV_ENTRIES];
  /*
   *  If TRUE for link, the LSB of the FAP id as transmitted in the
   *  reachability cells on that link is 1. If FALSE - it is 0. This allows
   *  the device in coexist mode to be seen as two devices,
   *  for better bandwidth distribution. Note: the FAP id of a
   *  device in coexist mode must be even, and the next odd id
   *  must not be used.
   */
  uint8 fap_id_odd_link_map[SOC_DNX_DEFS_MAX(NOF_FABRIC_LINKS)];
}DNX_TMC_FABRIC_COEXIST_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Indication of whether this link status entry is
   *  meaningful.
   */
  uint8 valid;
  /*
   *  TRUE if the link is out-of-reset in TX
   *  direction
   */
  uint8 up_tx;
  /*
   *  TRUE if the link is out-of-reset in RX
   *  direction
   */
  uint8 up_rx;
  /*
   *  Bit Map of link error
   */
  uint8 errors_bm;
  /*
   *  Bit Map of the link physical indications:
   */
  uint8 indications_bm;
  /*
   *	Link CRC error counter
   */
  uint32 crc_err_counter;
  /*
   *  Value of leaky bucket counter of corresponding Serdes. A
   *  high value indicates problems with the link.
   */
  uint32 leaky_bucket_counter;
}DNX_TMC_FABRIC_LINKS_STATUS_SINGLE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Flag. Indication of whether any of the link array
   *  elements below is reporting an error condition.
   */
  uint8 error_in_some;
  /*
   *  Array of one status element per Serdes.
   */
  DNX_TMC_FABRIC_LINKS_STATUS_SINGLE single_link_status[SOC_DNX_DEFS_MAX(NOF_FABRIC_LINKS)];
}DNX_TMC_FABRIC_LINKS_STATUS_ALL;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Fabric link state - normal operation or down.
   */
  DNX_TMC_LINK_STATE on_off;
  /*
   *  Set the serdes lane accordingly - power down when fabric
   *  link is turned off, power up when turned on.
   */
  uint8 serdes_also;
}DNX_TMC_LINK_STATE_INFO;

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
  DNX_TMC_FABRIC_LINKS_CONNECT_MAP_STAT_INFO_clear(
    DNX_SAND_OUT DNX_TMC_FABRIC_LINKS_CONNECT_MAP_STAT_INFO *info
  );

void
  DNX_TMC_FABRIC_LINKS_CON_STAT_INFO_ARR_clear(
    DNX_SAND_OUT DNX_TMC_FABRIC_LINKS_CON_STAT_INFO_ARR *info
  );

void
  DNX_TMC_FABRIC_CELL_FORMAT_clear(
    DNX_SAND_OUT DNX_TMC_FABRIC_CELL_FORMAT *info
  );

void
  DNX_TMC_FABRIC_FC_clear(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_OUT DNX_TMC_FABRIC_FC *info
  );

void
  DNX_TMC_FABRIC_COEXIST_INFO_clear(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_OUT DNX_TMC_FABRIC_COEXIST_INFO *info
  );

void
  DNX_TMC_FABRIC_LINKS_STATUS_SINGLE_clear(
    DNX_SAND_OUT DNX_TMC_FABRIC_LINKS_STATUS_SINGLE *info
  );

void
  DNX_TMC_FABRIC_LINKS_STATUS_ALL_clear(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_OUT DNX_TMC_FABRIC_LINKS_STATUS_ALL *info
  );

void
  DNX_TMC_LINK_STATE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_LINK_STATE_INFO *info
  );

void 
  DNX_TMC_FABRIC_FC_SHAPER_clear(
      DNX_SAND_OUT DNX_TMC_FABRIC_FC_SHAPER *info
  );

void 
  DNX_TMC_FABRIC_FC_SHAPER_MODE_INFO_clear(
      DNX_SAND_OUT DNX_TMC_FABRIC_FC_SHAPER_MODE_INFO *info
  );

#if DNX_TMC_DEBUG_IS_LVL1

/*********************************************************************
* NAME:
*     soc_jer2_jer2_jer2_tmcfabric_topology_status_connectivity_print
* TYPE:
*   PROC
* FUNCTION:
*     Print the connectivity ma.
* INPUT:
*  DNX_SAND_IN  DNX_TMC_FABRIC_LINKS_CON_STAT_INFO_ARR *connectivity_map -
*     The connectivity map
* RETURNS:
*     OK or ERROR indication.
* REMARKS:
*     The far_link_id (link number at the connected device) indication
*     is modulo 64
*********************************************************************/
uint32
  soc_jer2_jer2_jer2_tmcfabric_topology_status_connectivity_print(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_IN  DNX_TMC_FABRIC_LINKS_CON_STAT_INFO_ARR *connectivity_map
  );

const char*
  DNX_TMC_LINK_ERR_TYPE_to_string(
    DNX_SAND_IN DNX_TMC_LINK_ERR_TYPE enum_val
  );

const char*
  DNX_TMC_LINK_INDICATE_TYPE_to_string(
    DNX_SAND_IN DNX_TMC_LINK_INDICATE_TYPE enum_val
  );

const char*
  DNX_TMC_FABRIC_CONNECT_MODE_to_string(
    DNX_SAND_IN DNX_TMC_FABRIC_CONNECT_MODE enum_val
  );

const char*
  DNX_TMC_LINK_STATE_to_string(
    DNX_SAND_IN DNX_TMC_LINK_STATE enum_val
  );

const char*
  DNX_TMC_FABRIC_LINE_CODING_to_string(
    DNX_SAND_IN  DNX_TMC_FABRIC_LINE_CODING enum_val
  );

void
  DNX_TMC_FABRIC_LINKS_CONNECT_MAP_STAT_INFO_print(
    DNX_SAND_IN DNX_TMC_FABRIC_LINKS_CONNECT_MAP_STAT_INFO *info,
    DNX_SAND_IN uint8                         table_format
  );

void
  DNX_TMC_FABRIC_LINKS_CON_STAT_INFO_ARR_print(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_IN DNX_TMC_FABRIC_LINKS_CON_STAT_INFO_ARR *info,
    DNX_SAND_IN uint8                     table_format
  );

void
  DNX_TMC_FABRIC_CELL_FORMAT_print(
    DNX_SAND_IN DNX_TMC_FABRIC_CELL_FORMAT *info
  );

void
  DNX_TMC_FABRIC_FC_print(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_IN DNX_TMC_FABRIC_FC *info
  );

void
  DNX_TMC_FABRIC_COEXIST_INFO_print(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_IN DNX_TMC_FABRIC_COEXIST_INFO *info
  );


void
  DNX_TMC_FABRIC_LINKS_STATUS_ALL_print(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_IN DNX_TMC_FABRIC_LINKS_STATUS_ALL *info,
    DNX_SAND_IN DNX_SAND_PRINT_FLAVORS     print_flavour
  );

void
  DNX_TMC_LINK_STATE_INFO_print(
    DNX_SAND_IN DNX_TMC_LINK_STATE_INFO *info
  );

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_FABRIC_INCLUDED__*/
#endif
