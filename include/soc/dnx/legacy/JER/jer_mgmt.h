/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: jer2_jer_mgmt.h
 */

#ifndef __JER2_JER_MGMT_INCLUDED__

#define __JER2_JER_MGMT_INCLUDED__

#include <shared/swstate/sw_state.h>

#define NOF_FIFO_DMA_CHANNELS (12)
#define NOF_DMA_FIFO_PER_CMC (4)

typedef enum dnx_dma_fifo_channel_src_e {
    dnx_dma_fifo_channel_src_crps_0_to_3 = 0x0,
    dnx_dma_fifo_channel_src_crps_4_to_7,
    dnx_dma_fifo_channel_src_crps_8_to_11,    
    dnx_dma_fifo_channel_src_crps_12_to_15,        
    dnx_dma_fifo_channel_src_oam_status,
    dnx_dma_fifo_channel_src_oam_event,
    dnx_dma_fifo_channel_src_olp,
    dnx_dma_fifo_channel_src_ppdb_cpu_reply,    
    dnx_dma_fifo_channel_src_max,
    dnx_dma_fifo_channel_src_reserved = 0xF
} dnx_dma_fifo_channel_src_t;

/* each field in the array holds the channel number for the source it represent */
typedef struct jer2_jer_mgmt_dma_fifo_source_channels_s {
    int dma_fifo_source_channels_array[dnx_dma_fifo_channel_src_max]; 
} jer2_jer_mgmt_dma_fifo_source_channels_t; 


/*********************************************************************
*     Bytes-worth of a single credit. It should be configured
*     the same in all the FAPs in the systems, and should be
*     set before programming the scheduler.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_jer_mgmt_credit_worth_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              credit_worth
  ) ;
/*********************************************************************
*     Bytes-worth of a single credit. It should be configured
*     the same in all the FAPs in the systems, and should be
*     set before programming the scheduler.
*********************************************************************/
int
   jer2_jer_mgmt_credit_worth_get(
           DNX_SAND_IN  int                 unit,
           DNX_SAND_OUT uint32              *credit_worth
          ) ;



uint32
  jer2_jer_mgmt_credit_worth_remote_set(
    DNX_SAND_IN  int    unit,
	DNX_SAND_IN  uint32    credit_worth_remote
  ) ;


uint32
  jer2_jer_mgmt_credit_worth_remote_get(
    DNX_SAND_IN  int    unit,
	DNX_SAND_OUT uint32    *credit_worth_remote
  ) ;


uint32
  jer2_jer_mgmt_module_to_credit_worth_map_set(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32    fap_id,
    DNX_SAND_IN  uint32    credit_value_type /* should be one of JER2_JERICHO_FAP_CREDIT_VALUE_* */
  ) ;


uint32
  jer2_jer_mgmt_module_to_credit_worth_map_get(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32    fap_id,
    DNX_SAND_OUT uint32    *credit_value_type /* will be one of JER2_JERICHO_FAP_CREDIT_VALUE_* */
  ) ;



uint32
  jer2_jer_mgmt_change_all_faps_credit_worth_unsafe(
    DNX_SAND_IN  int    unit,
    DNX_SAND_OUT uint8     credit_value_to_use
  ) ;




/*********************************************************************
* Set the fabric system ID of the device. Must be unique in the system.
*********************************************************************/
uint32 jer2_jer_mgmt_system_fap_id_set(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  uint32  sys_fap_id
  );

/* return the FAP ID of (core 0 of) the device */
uint32
  jer2_jer_mgmt_system_fap_id_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint32              *sys_fap_id
  );


/* 
 * PVT
 */
int jer2_jer_mgmt_temp_pvt_get(int unit, int temperature_max, soc_switch_temperature_monitor_t *temperature_array, int *temperature_count);
int jer2_jer_mgmt_drv_pvt_monitor_enable(int unit);

/*
 * Function:
 *      jer2_jer_mgmt_revision_fixes
 * Purpose:
 *      set all the bits controlling the revision fixes (chicken bits) in the device.
 * Parameters:
 *      unit    - Device Number
 * Returns:
 *      SOC_E_XXX
 */
int jer2_jer_mgmt_revision_fixes (int unit);

/*********************************************************************
* NAME:
*     jer2_jer_mgmt_enable_traffic_set
* TYPE:
*   PROC
* DATE:
*   Nov 16 2014
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
  jer2_jer_mgmt_enable_traffic_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 enable
  );

uint32
  jer2_jer_mgmt_enable_traffic_get(
    DNX_SAND_IN  int unit,
    DNX_SAND_OUT uint8 *enable
  );

/*********************************************************************
* NAME:
*     jer2_jer_mgmt_enable_traffic_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Nov 16 2014
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
  jer2_jer_mgmt_enable_traffic_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 enable
  );

uint32
  jer2_jer_mgmt_enable_traffic_get_unsafe(
    DNX_SAND_IN  int unit,
    DNX_SAND_OUT  uint8 *enable
  );


/*********************************************************************
 * Set the MTU (maximal allowed packet size) for any packet,
 * according to the buffer size.
 *********************************************************************/
uint32 jer2_jer_mgmt_set_mru_by_dbuff_size(
    DNX_SAND_IN  int     unit
  );

int jer2_jer_mgmt_dma_fifo_channel_free_find(DNX_SAND_IN int unit, DNX_SAND_IN uint8 skip_pci_cmc, DNX_SAND_OUT int * channel_number);
int jer2_jer_mgmt_dma_fifo_channel_set(DNX_SAND_IN int unit, DNX_SAND_IN int channel, DNX_SAND_IN dnx_dma_fifo_channel_src_t value);
int jer2_jer_mgmt_dma_fifo_channel_get (DNX_SAND_IN int unit, DNX_SAND_IN dnx_dma_fifo_channel_src_t source, DNX_SAND_OUT int* channel);
int jer2_jer_mgmt_dma_fifo_source_channels_db_init (int unit);

int
  jer2_jer_mgmt_voq_is_ocb_eligible_get(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  int         core_id,
    DNX_SAND_IN  uint32      qid,
    DNX_SAND_OUT uint32      *is_ocb_eligible
  );

int jer2_jer_mgmt_avs_value_get(
    DNX_SAND_IN    int      unit,
    DNX_SAND_OUT   uint32*  avs_val);

/*
 * Set if to forward packets whose mirror/snoop copies are dropped in ingress sue to FIFO being full.
 */
int jer2_jer_mgmt_mirror_snoop_forward_original_when_dropped_set(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  uint8       enabled
  );

/*
 * Get if to forward packets whose mirror/snoop copies are dropped in ingress sue to FIFO being full.
 */
int jer2_jer_mgmt_mirror_snoop_forward_original_when_dropped_get(
    DNX_SAND_IN  int         unit,
    DNX_SAND_OUT uint8       *enabled
  );

#endif /*__JER2_JER_MGMT_INCLUDED__*/

