/* $Id: jer2_arad_mgmt.h,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER2_ARAD_MGMT_INCLUDED__
/* { */
#define __JER2_ARAD_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_api_mgmt.h>
#include <soc/dnx/legacy/drv.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*
 *  Packet size limitations
 */
/* Minimal packet size, variable size cells */
#define JER2_ARAD_MGMT_PCKT_SIZE_BYTES_VSC_MIN      64
/* Maximal packet size, variable size cells */
#define JER2_ARAD_MGMT_PCKT_SIZE_BYTES_VSC_MAX      (16*1024 - 128)
/* Minimal packet size, fixed size cells */
#define JER2_ARAD_MGMT_PCKT_SIZE_BYTES_FSC_MIN      33
/* Maximal packet size, fixed size cells */
#define JER2_ARAD_MGMT_PCKT_SIZE_BYTES_FSC_MAX      (16*1024 - 128)

/* Maximal packet size of the packet on the line (before ingress editing) */
#define JER2_ARAD_MGMT_PCKT_SIZE_BYTES_EXTERN_MAX      (JER2_ARAD_MGMT_PCKT_SIZE_BYTES_FSC_MAX    \
                                                   + (JER2_ARAD_MGMT_PCKT_RNG_NIF_CRC_BYTES  \
                                                   - JER2_ARAD_MGMT_PCKT_RNG_DRAM_CRC_BYTES))

#define JER2_ARAD_MGMT_IDR_NUM_OF_CONTEXT            (192)

#define JER2_ARAD_MGMT_MAX_BUFFERS_PER_PACKET        (32)

/*
 * jer2_arad_mgmt_all_ctrl_cells_enable_set flags
 */
#define JER2_ARAD_MGMT_ALL_CTRL_CELLS_FLAGS_NONE           0
#define JER2_ARAD_MGMT_ALL_CTRL_CELLS_FLAGS_SOFT_RESET     0x1

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

typedef struct soc_dnx_config_jer2_arad_plus_s {
    uint16 nof_remote_faps_with_remote_credit_value; /* number of remote faps for which the reomte credit value  was assigned */
    uint32 per_module_credit_value[JER2_ARAD_PLUS_CREDIT_VALUE_MODE_WORDS];
} soc_dnx_config_jer2_arad_plus_t;

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
*     jer2_arad_register_device_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     This procedure registers a new device to be taken care
*     of by this device driver. Physical device must be
*     accessible by CPU when this call is made..
* INPUT:
*  DNX_SAND_IN  uint32                  *base_address -
*     Base address of direct access memory assigned for
*     device's registers. This parameter needs to be specified
*     even if physical access to device is not by direct
*     access memory since all logic, within driver, up to
*     actual physical access, assumes 'virtual' direct access
*     memory. Memory block assigned by this pointer must not
*     overlap other memory blocks in user's system and
*     certainly not memory blocks assigned to other JER2_ARAD
*     devices using this procedure.
*  DNX_SAND_IN  DNX_SAND_RESET_DEVICE_FUNC_PTR reset_device_ptr -
*     BSP-function for device reset. Refer to
*     'DNX_SAND_RESET_DEVICE_FUNC_PTR' definition.
*  DNX_SAND_OUT uint32                 *unit_ptr -
*     This procedure loads pointed memory with identifier of
*     newly added device. This identifier is to be used by the
*     caller for further accesses to this device..
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_register_device_unsafe(
             uint32                  *base_address,
    DNX_SAND_IN  DNX_SAND_RESET_DEVICE_FUNC_PTR reset_device_ptr,
    DNX_SAND_INOUT int                 *unit_ptr
  );

/*********************************************************************
* NAME:
*     jer2_arad_unregister_device_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Undo jer2_arad_register_device()
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     The device ID to be unregistered.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_unregister_device_unsafe(
    DNX_SAND_IN  int                 unit
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_credit_worth_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Bytes-worth of a single credit. It should be configured
*     the same in all the FAPs in the systems, and should be
*     set before programming the scheduler.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint32                  credit_worth -
*     Credit worth value. Range: 1 - 8K-1. Unit: bytes.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_credit_worth_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  credit_worth
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_credit_worth_verify
* TYPE:
*   PROC
* FUNCTION:
*     Bytes-worth of a single credit. It should be configured
*     the same in all the FAPs in the systems, and should be
*     set before programming the scheduler.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint32                  credit_worth -
*     Credit worth value. Range: 1 - 8K-1. Unit: bytes.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_credit_worth_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              credit_worth
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_credit_worth_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Bytes-worth of a single credit. It should be configured
*     the same in all the FAPs in the systems, and should be
*     set before programming the scheduler.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_OUT uint32                  *credit_worth -
*     Credit worth value. Range: 1 - 8K-1. Unit: bytes.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
int
  jer2_arad_mgmt_credit_worth_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint32              *credit_worth
  );

/*
 * Arad+ only: map the module (fap_id) to the given credit value (local, remote or non mapped).
 * Has no special handling of the local device (should not be used for the local device).
 */
uint32
  jer2_arad_plus_mgmt_module_to_credit_worth_map_set_unsafe(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32    fap_id,
    DNX_SAND_IN  uint32    credit_value_type /* should be one of JER2_ARAD_PLUS_FAP_CREDIT_VALUE_* */
  );
/*
 * Arad+ only: Get the mapping the module (fap_id) to the given credit value (local, remote or non mapped).
 * Has no special handling of the local device (should not be used for the local device).
 */
uint32
  jer2_arad_plus_mgmt_module_to_credit_worth_map_get_unsafe(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32    fap_id,
    DNX_SAND_OUT uint32    *credit_value_type /* will be one of JER2_ARAD_PLUS_FAP_CREDIT_VALUE_* */
  );


/*********************************************************************
* NAME:
*     jer2_arad_mgmt_system_fap_id_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Set the fabric system ID of the device. Must be unique
*     in the system.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint32                 sys_fap_id -
*     The system ID of the device (Unique in the system).
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_system_fap_id_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 sys_fap_id
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_system_fap_id_verify
* TYPE:
*   PROC
* FUNCTION:
*     Set the fabric system ID of the device. Must be unique
*     in the system.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint32                 sys_fap_id -
*     The system ID of the device (Unique in the system).
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_system_fap_id_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 sys_fap_id
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_system_fap_id_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Get the fabric system ID of the device. Must be unique
*     in the system.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_OUT uint32                 *sys_fap_id -
*     The system ID of the device (Unique in the system).
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_system_fap_id_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint32                 *sys_fap_id
  );

uint32
  jer2_arad_mgmt_tm_domain_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 tm_domain
  );

uint32
  jer2_arad_mgmt_tm_domain_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 tm_domain
  );

uint32
  jer2_arad_mgmt_tm_domain_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint32                 *tm_domain
  );

uint32
  jer2_arad_mgmt_all_ctrl_cells_enable_get_unsafe(
    DNX_SAND_IN   int  unit,
    DNX_SAND_OUT  uint8  *enable
  );


/*********************************************************************
* NAME:
*     jer2_arad_mgmt_all_ctrl_cells_enable_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Enable / Disable the device from receiving and
*     transmitting control cells.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN uint8  enable -
*     Enable/disable indication
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_all_ctrl_cells_enable_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 enable,
    DNX_SAND_IN  uint32                 flags
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_all_ctrl_cells_enable_verify
* TYPE:
*   PROC
* FUNCTION:
*     Enable / Disable the device from receiving and
*     transmitting control cells.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN uint8  enable -
*     Enable/disable indication
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_all_ctrl_cells_enable_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 enable
  );

/*********************************************************************
*     Enable / Disable the forcing of (bypass) TDM cells to fabric
*********************************************************************/
uint32
  jer2_arad_force_tdm_bypass_traffic_to_fabric_set_unsafe(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  int     enable
  );
/*********************************************************************
*     Check if forcing of (bypass) TDM cells to fabric
*********************************************************************/
uint32
  jer2_arad_force_tdm_bypass_traffic_to_fabric_get_unsafe(
    DNX_SAND_IN  int     unit,
    DNX_SAND_OUT int     *enable
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_enable_traffic_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Enable / Disable the device from receiving and
*     transmitting traffic.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint8                 enable -
*     DNX_SAND_IN uint32 enable_indication
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_enable_traffic_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 enable
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_enable_traffic_verify
* TYPE:
*   PROC
* FUNCTION:
*     Enable / Disable the device from receiving and
*     transmitting traffic.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint8                 enable -
*     DNX_SAND_IN uint8 enable
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_enable_traffic_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 enable
  );

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_enable_traffic_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Enable / Disable the device from receiving and
*     transmitting traffic.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_OUT uint8  *enable -
*     DNX_SAND_OUT uint8  enable
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_enable_traffic_get_unsafe(
    DNX_SAND_IN  int  unit,
    DNX_SAND_OUT uint8  *enable
  );

/*********************************************************************
* NAME:
 *   jer2_arad_mgmt_max_pckt_size_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the maximal allowed packet size. The limitation can
 *   be performed based on the packet size before or after
 *   the ingress editing (external and internal configuration
 *   mode, accordingly). Packets above the specified value
 *   are dropped.
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  uint32                      port_ndx -
 *     Incoming port index. Range: 0 - 79.
 *   DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx -
 *     External mode filters the packets according to there
 *     original size. Internal mode filters the packets
 *     according to their size inside the device, after ingress
 *     editing.
 *   DNX_SAND_IN  uint32                       *max_size -
 *     Maximal allowed packet size per incoming port. Packets
 *     above this value will be dropped. Units: bytes.
 * REMARKS:
 *   1. This API gives a better resolution (i.e., per
 *   incoming port) than jer2_arad_mgmt_pckt_size_range_set. 2.
 *   If both APIs are used to configure the maximal packet
 *   size, the value configured is set by the API called at
 *   last.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_max_pckt_size_set_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      port_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    DNX_SAND_IN  uint32                       max_size
  );

uint32
  jer2_arad_mgmt_max_pckt_size_set_verify(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      port_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    DNX_SAND_IN  uint32                       max_size
  );

uint32
  jer2_arad_mgmt_max_pckt_size_get_verify(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      port_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "jer2_arad_mgmt_max_pckt_size_set_unsafe" API.
 *     Refer to "jer2_arad_mgmt_max_pckt_size_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  jer2_arad_mgmt_max_pckt_size_get_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      port_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    DNX_SAND_OUT uint32                       *max_size
  );
uint32
  jer2_arad_mgmt_pckt_size_range_set_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE          *size_range
  );

uint32
  jer2_arad_mgmt_pckt_size_range_get_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    DNX_SAND_OUT JER2_ARAD_MGMT_PCKT_SIZE          *size_range
  );

/*********************************************************************
 * Set the MTU (maximal allowed packet size) for any packet,
 * according to the buffer size.
 *********************************************************************/
uint32
  jer2_arad_mgmt_set_mru_by_dbuff_size(
    DNX_SAND_IN  int     unit
  );

uint8
  jer2_arad_mgmt_is_pp_enabled(
    DNX_SAND_IN int unit
  );

/*********************************************************************
* NAME:
 *   jer2_arad_mgmt_ocb_mc_range_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the ocb muliticast range.
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   DNX_SAND_IN  uint32                      range_ndx -
 *     Incoming range index. Range: 0 - 1.
 *   DNX_SAND_IN  JER2_ARAD_MGMT_OCB_MC_RANGE *range -
 *     Structure with minimum and maximum.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_ocb_mc_range_set_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      range_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_OCB_MC_RANGE         *range
  );

uint32
  jer2_arad_mgmt_ocb_mc_range_get_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      range_ndx,
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_MC_RANGE         *range
  );

/*********************************************************************
* NAME:
 *   jer2_arad_mgmt_ocb_voq_eligible_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the ocb queue parameters.
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *  DNX_SAND_IN  uint32                    q_category_ndx -
 *     Queue category. Range: 0 - 3.
 *  DNX_SAND_IN  uint32                    rate_class_ndx -
 *     Queue rate class index. Range: 0 - 63
 *  DNX_SAND_IN  uint32                    tc_ndx - 
 *     Traffic class. Range: 0 - 7.
 *  DNX_SAND_IN  JER2_ARAD_MGMT_OCB_VOQ_INFO       *info - 
 *     Structure with the required data:
 *      - enable/diasable the ocb.
 *      - 2 word thresholds
 *      - 2 buffers thresholds
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_ocb_voq_eligible_set_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                    q_category_ndx,
    DNX_SAND_IN  uint32                    rate_class_ndx,
    DNX_SAND_IN  uint32                    tc_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_OCB_VOQ_INFO       *info,
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_VOQ_INFO    *exact_info
  );

uint32
  jer2_arad_mgmt_ocb_voq_eligible_get_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                    q_category_ndx,
    DNX_SAND_IN  uint32                    rate_class_ndx,
    DNX_SAND_IN  uint32                    tc_ndx,
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_VOQ_INFO       *info
  );

/*********************************************************************
* NAME:
 *   jer2_arad_mgmt_ocb_voq_eligible_dynamic_set_unsafe
 * FUNCTION:
 *   Set the  IDR_MEM_1F0000 Stores1 bit per queue number (128k queues) which indicates if the queue can be used.
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *  DNX_SAND_IN  uint32                    qid -
 *     the qid range 0-128K
 *  DNX_SAND_IN  uint32                    enable -
 *     enable q FALSE or TRUE
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

uint32
soc_jer2_arad_cache_table_update_all(
    DNX_SAND_IN int unit
  );

/*********************************************************************
* NAME:
 *   jer2_arad_mgmt_ocb_voq_eligible_dynamic_set_unsafe
 * FUNCTION:
 *   Set the  IDR_MEM_1F0000 Stores1 bit per queue number (128k queues) which indicates if the queue can be used.
 * INPUT:
 *   DNX_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *  DNX_SAND_IN  uint32                    qid -
 *     the qid range 0-128K
 *  DNX_SAND_IN  uint32                    enable -
 *     enable q FALSE or TRUE
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

uint32
  jer2_arad_mgmt_ocb_voq_eligible_dynamic_set_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                    qid,
    DNX_SAND_IN  uint32                    enable
  );

/*********************************************************************
*     Set the Soc_JER2_ARAD B0 revision specific features.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32 soc_jer2_arad_mgmt_rev_b0_set_unsafe(
    DNX_SAND_IN  int       unit);

#ifdef BCM_88660_A0
/*********************************************************************
*     Set the Soc_JER2_ARAD PLUS revision specific features.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32 soc_jer2_arad_mgmt_rev_jer2_arad_plus_set_unsafe(
    DNX_SAND_IN  int       unit);
    
#endif

int 
  jer2_arad_mgmt_nof_block_instances(
    int unit, 
    soc_block_types_t block_types, 
    int *nof_block_instances
  );

int ardon_mgmt_drv_pvt_monitor_enable(int unit);

int 
   jer2_arad_mgmt_temp_pvt_get(
     int unit,
     int temperature_max,
     soc_switch_temperature_monitor_t *temperature_array,
     int *temperature_count
   ) ;

/*********************************************************************
* NAME:
*     jer2_arad_mgmt_init_set_core_clock_frequency
* TYPE:
*   PROC
* DATE:
*   MAY 25 2014
* FUNCTION:
*     set core clock frequency according to given input
*     in JER2_ARAD_MGMT_INIT struct. 
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mgmt_init_set_core_clock_frequency (
    DNX_SAND_IN int unit,
    DNX_SAND_IN JER2_ARAD_MGMT_INIT* init
    );


/*********************************************************************
*     Get the AVS - Adjustable Voltage Scaling value of the Arad
*********************************************************************/
int jer2_arad_mgmt_avs_value_get(
        int       unit,
        uint32*      avs_val);
/* } */
uint32
  jer2_arad_mgmt_sw_ver_set_unsafe(
    DNX_SAND_IN  int                      unit);


    
#ifdef FIXME_DNX_LEGACY
/********************************************************** 
* Set OCB VOQ INFO defaults 
**********************************************************/ 
uint32 jer2_arad_mgmt_ocb_voq_info_defaults_set(
    DNX_SAND_IN     int                         unit,
    DNX_SAND_OUT    JER2_ARAD_MGMT_OCB_VOQ_INFO      *ocb_info
    );
#endif 


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


/* } __JER2_ARAD_MGMT_INCLUDED__*/

#endif
