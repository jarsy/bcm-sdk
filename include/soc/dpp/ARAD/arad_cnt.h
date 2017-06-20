/* $Id: arad_cnt.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_CNT_INCLUDED__
/* { */
#define __ARAD_CNT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/ARAD/arad_api_ports.h>
#include <soc/dpp/ARAD/arad_api_cnt.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_CNT_NOF_MODE_EG_TYPES   SOC_TMC_CNT_NOF_MODE_EG_TYPES_ARAD

#define ARAD_COUNTER_NDX_MAX(unit, proc_id) SOC_TMC_COUNTER_NDX_MAX(unit, proc_id)

#define ARAD_CNT_USE_DMA                                           (1) /* If set to 0, change also _FIFO_DEPTH_MAX to 64 */

#define ARAD_CNT_PREFETCH_TMR_CFG_MS                                   (0x30d40) /* 0.33 ms in clocks for arad (arad_clk: 600Mhz)*/
#define ARAD_CNT_PREFETCH_TMR_CFG_ALGO                                 (0x1e) /* 10 ms - 30 for ARAD */
#define ARAD_CNT_PREFETCH_TMR_CFG_SEQ                                  (0x12c) /* 50 ms - 300 */
#define ARAD_CNT_READ_DIRECT_READ                                      (2)

typedef struct {
    soc_reg_t cfg_1_r;
    soc_reg_t cfg_2_r;
    soc_reg_t src_cfg_r;
    soc_field_t src_type_f;
    soc_field_t queue_shift_f;
    soc_field_t base_q_f;
    soc_field_t top_q_f;
} ARAD_CNT_CRPS_IQM_CMD;

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

uint32
  arad_cnt_init(
    SOC_SAND_IN  int                 unit
  );

/* 
 * Purpose:
 *  Mark DMA channel as used for CRPS if there is a hardware-fifo mapped to it. 
 *  Reset the DMA channel is there is no hardware-fifo mapped to it.
 *  Arguments
 *    unit
 *    channel - DMA-channel to map
 *    crps_fifo - HW-counter-processor-FIFO mapped from
 */
uint32
  arad_cnt_channel_to_fifo_mapping_set(
     SOC_SAND_IN  int                             unit,
     SOC_SAND_IN  uint8                           channel,
     SOC_SAND_IN  uint8                           crps_fifo);
/* 
 * Purpose:
 *  Get DMA-channel to CRPS-FIFO mapping or any other application reserving it.
 *  Arguments
 *     unit
 *     channel - DMA-channel to map
 *Out: crps_fifo - HW-counter-processor-FIFO mapped from
 */
uint32
  arad_cnt_channel_to_fifo_mapping_get(
     SOC_SAND_IN  int                             unit,
     SOC_SAND_IN  uint8                           channel,
     SOC_SAND_OUT uint8*                          crps_fifo);
/*********************************************************************
* NAME:
 *   arad_cnt_dma_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the DMA for Counter Processor
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx -
 *     Counter processor index
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnt_dma_set_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  int                   dma_ndx,
	SOC_SAND_IN  uint8                 channel,
    SOC_SAND_IN  int                   cache_length
  );
uint32
  arad_cnt_dma_set_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  int                   dma_ndx,
	SOC_SAND_IN  uint8                 channel,
    SOC_SAND_IN  int                   cache_length
  );
uint32
  arad_cnt_dma_unset_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  int                   dma_ndx,
	SOC_SAND_IN  uint8                 channel
  );
uint32
  arad_cnt_dma_unset_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  int                   dma_ndx,
	SOC_SAND_IN  uint8                 channel
  );
/*********************************************************************
* NAME:
 *   arad_cnt_counters_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the Counter Processor: its counting mode and
 *   its counting source.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx -
 *     Counter processor index
 *   SOC_SAND_IN  ARAD_CNT_COUNTERS_INFO        *info -
 *     Counter info of the counter processor
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnt_counters_set_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx,
    SOC_SAND_IN  ARAD_CNT_COUNTERS_INFO        *info
  );

uint32
  arad_cnt_counters_set_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx,
    SOC_SAND_IN  ARAD_CNT_COUNTERS_INFO        *info
  );

uint32
  arad_cnt_counters_get_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_cnt_counters_set_unsafe" API.
 *     Refer to "arad_cnt_counters_set_unsafe" API for details.
*********************************************************************/
uint32
  arad_cnt_counters_get_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx,
    SOC_SAND_OUT ARAD_CNT_COUNTERS_INFO        *info
  );

/*********************************************************************
* NAME:
 *   arad_cnt_status_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the current status of the counter processor.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx -
 *     Counter processor index
 *   SOC_SAND_OUT ARAD_CNT_STATUS               *proc_status -
 *     Counter processor status info
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnt_status_get_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx,
    SOC_SAND_OUT ARAD_CNT_STATUS               *proc_status
  );

uint32
  arad_cnt_status_get_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx
  );

/*********************************************************************
* NAME:
 *   arad_cnt_algorithmic_read_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Read the counter values saved in a cache according to
 *   the polling algorithm (algorithmic method).
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx -
 *     Counter processor index
 *   SOC_SAND_OUT ARAD_CNT_RESULT_ARR           *result_arr -
 *     Counter values from the first 'nof_counters' counters in
 *     the cache which contains the counters chosen by the
 *     polling algorithm.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnt_algorithmic_read_unsafe(
    SOC_SAND_IN  int                                          unit,
    SOC_SAND_IN  int                                          dma_ndx,
	SOC_SAND_IN  uint8                       				  channel,
    SOC_SAND_IN  SOC_TMC_CNT_COUNTERS_INFO                   *counter_info_array,
    SOC_SAND_OUT ARAD_CNT_RESULT_ARR                         *result_arr
  );

uint32
  arad_cnt_algorithmic_read_verify(
    SOC_SAND_IN  int                                        unit,
    SOC_SAND_IN  int                                        dma_ndx,
	SOC_SAND_IN  uint8                                      channel
  );

/*********************************************************************
* NAME:
 *   arad_cnt_direct_read_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Read the counter value according to the counter.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx -
 *     Counter processor index
 *   SOC_SAND_IN  uint32                    counter_ndx -
 *     Counter index. Range: 0 - 8K-1.
 *   SOC_SAND_OUT ARAD_CNT_RESULT               *read_rslt -
 *     Counter processor read result (in particular the counter
 *     values).
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnt_direct_read_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID          processor_ndx,
    SOC_SAND_IN  ARAD_CNT_COUNTERS_INFO         *counter_info,
    SOC_SAND_IN  uint32                         counter_ndx,
    SOC_SAND_OUT ARAD_CNT_RESULT                *read_rslt
  );

uint32
  arad_cnt_direct_read_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx,
    SOC_SAND_IN  uint32                    counter_ndx
  );

/*********************************************************************
* NAME:
 *   arad_cnt_q2cnt_id_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Convert a queue index to the counter index.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   queue_ndx -
 *     Queue index. Range: 0 - 32K-1.
 *   SOC_SAND_OUT uint32                   *counter_ndx -
 *     Counter index. Range: 0 - 8K-1.
 * REMARKS:
 *   The result depends directly on the parameters set with
 *   the function arad_cnt_src_type_set. Relevant only if the
 *   counting source is the VOQs.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnt_q2cnt_id_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx,
    SOC_SAND_IN  uint32                   queue_ndx,
    SOC_SAND_OUT uint32                   *counter_ndx
  );

uint32
  arad_cnt_q2cnt_id_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx,
    SOC_SAND_IN  uint32                   queue_ndx
  );

/*********************************************************************
* NAME:
 *   arad_cnt_cnt2q_id_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Convert a counter index to the respective queue index.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   counter_ndx -
 *     Counter index. Range: 0 - 8K-1.
 *   SOC_SAND_OUT uint32                   *queue_ndx -
 *     Queue index. Range: 0 - 32K-1.
 * REMARKS:
 *   The result depends directly on the parameters set with
 *   the function arad_cnt_src_type_set. Relevant only if the
 *   counting source is the VOQs.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnt_cnt2q_id_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx,
    SOC_SAND_IN  uint32                   counter_ndx,
    SOC_SAND_OUT uint32                   *queue_ndx
  );

uint32
  arad_cnt_cnt2q_id_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID         processor_ndx,
    SOC_SAND_IN  uint32                   counter_ndx
  );


/*********************************************************************
* NAME:
 *   arad_cnt_meter_hdr_compensation_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the header compensation factor for the counter
 *   and meter modules to enable a meter and counter
 *   processing of this packet according to its original
 *   packet size.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   port_ndx -
 *     Incoming PP port id. Range: 0 - 63.
 *   SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx -
 *     ingress/egress.
 *   SOC_SAND_IN  int32                    hdr_compensation -
 *     Header compensation value to remove from the packet
 *     size. Units: Bytes. Range: -15 - 15.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnt_meter_hdr_compensation_set_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  int                   core_id,
    SOC_SAND_IN  uint32                   port_ndx,
    SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx,
    SOC_SAND_IN  int32                    hdr_compensation
  );

uint32
  arad_cnt_meter_hdr_compensation_set_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                   port_ndx,
    SOC_SAND_IN  ARAD_PORT_DIRECTION      direction,
    SOC_SAND_IN  int32                    hdr_compensation
  );

uint32
  arad_cnt_meter_hdr_compensation_get_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                   port_ndx,
    SOC_SAND_IN  ARAD_PORT_DIRECTION      direction

  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_cnt_meter_hdr_compensation_set_unsafe" API.
 *     Refer to "arad_cnt_meter_hdr_compensation_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_cnt_meter_hdr_compensation_get_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  int                   core_id,
    SOC_SAND_IN  uint32                   port_ndx,
    SOC_SAND_IN  ARAD_PORT_DIRECTION      direction_ndx,
    SOC_SAND_OUT int32                    *hdr_compensation
  );
uint32 
    arad_cnt_lif_counting_set(
       SOC_SAND_IN int                  unit,
       SOC_SAND_IN SOC_TMC_CNT_SRC_TYPE source,
       SOC_SAND_IN int                  command_id,
       SOC_SAND_IN uint32               lif_counting_mask,
       SOC_SAND_IN int                  lif_stack_to_count
       );
uint32 
   arad_cnt_lif_counting_get(
      SOC_SAND_IN int                  unit,
      SOC_SAND_IN SOC_TMC_CNT_SRC_TYPE source,
      SOC_SAND_IN int                  command_id,
      SOC_SAND_IN uint32               lif_counting_mask,
      SOC_SAND_OUT int*                 lif_stack_to_count
   );
uint32 
   arad_cnt_lif_counting_range_set(
      SOC_SAND_IN int                  unit,
      SOC_SAND_IN SOC_TMC_CNT_SRC_TYPE source,
      SOC_SAND_IN int                  range_id,
      SOC_SAND_IN SOC_SAND_U32_RANGE*  range,
      SOC_SAND_IN int stif_counter_id,    
      SOC_SAND_IN int is_double_entry         
   );
int 
   arad_cnt_lif_counting_range_get(
      SOC_SAND_IN int                       unit,
      SOC_SAND_IN SOC_TMC_CNT_SRC_TYPE      source,
      SOC_SAND_IN int                       range_id,
      SOC_SAND_INOUT SOC_SAND_U32_RANGE*    range
      );
uint32 
   arad_cnt_base_val_set(
      SOC_SAND_IN int                       unit,
      SOC_SAND_IN SOC_TMC_CNT_PROCESSOR_ID  processor_ndx,
      SOC_SAND_IN SOC_TMC_CNT_COUNTERS_INFO *info
   );
uint32 
   arad_cnt_epni_regs_set_unsafe(
      SOC_SAND_IN  int                 unit,
      SOC_SAND_IN  ARAD_CNT_SRC_TYPE      src_type,
      SOC_SAND_IN  int                    src_core,
      SOC_SAND_IN ARAD_CNT_MODE_EG_TYPES  eg_mode_type,
      SOC_SAND_IN int                     command_id,
      SOC_SAND_IN int                     crps_config      
      );
#if ARAD_DEBUG_IS_LVL1

uint32
  ARAD_CNT_MODE_EG_verify(
    SOC_SAND_IN  ARAD_CNT_MODE_EG *info
  );

uint32
  ARAD_CNT_COUNTERS_INFO_verify(
     SOC_SAND_IN  int                    unit,
     SOC_SAND_IN  ARAD_CNT_COUNTERS_INFO *info
  );

uint32
  ARAD_CNT_OVERFLOW_verify(
    SOC_SAND_IN  ARAD_CNT_OVERFLOW *info
  );

uint32
  ARAD_CNT_STATUS_verify(
    SOC_SAND_IN  ARAD_CNT_STATUS *info
  );

uint32
  ARAD_CNT_RESULT_verify(
    SOC_SAND_IN  ARAD_CNT_RESULT *info
  );

uint32
  ARAD_CNT_RESULT_ARR_verify(
     SOC_SAND_IN  int                  unit,
     SOC_SAND_IN  ARAD_CNT_RESULT_ARR *info
  );

/*
 * Get the processor id e.g decide wether the counting is
 * done on processor A or on processor B
 */
  uint32
    arad_cnt_get_processor_id(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  ARAD_CNT_PROCESSOR_ID processor_ndx,
      SOC_SAND_OUT uint32               *proc_id
    );

/*
 * Get the number of counters and the following registers values from info:
 * CrpsCntSrcGroupSizes
 * CrpsOneEntryModeCntCmd(arad_b0 and above)
 *
 */
uint32
  arad_cnt_ingress_params_get(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  ARAD_CNT_COUNTERS_INFO *info,
    SOC_SAND_OUT uint32           *group_size,
    SOC_SAND_OUT uint32           *nof_counters,
    SOC_SAND_OUT uint32           *one_entry_mode_cnt
  );

uint32
  _arad_cnt_group_size_field_by_src_type_get(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                 proc_id, 
    SOC_SAND_IN  int                    src_core,
    SOC_SAND_IN  ARAD_CNT_SRC_TYPE      src_type,
    SOC_SAND_IN  int                    command_id,
    SOC_SAND_OUT soc_field_t            *field
    );


uint32
    arad_cnt_crps_iqm_cmd_get(int unit, int proc_id, int command_id, SOC_SAND_OUT ARAD_CNT_CRPS_IQM_CMD *crps_iqm_cmd);

uint32
  _jer_cnt_counter_bmap_mem_by_src_type_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                proc_id,
    SOC_SAND_IN  int                   src_core,
    SOC_SAND_IN  ARAD_CNT_SRC_TYPE     src_type,
    SOC_SAND_IN  int                   command_id,
    SOC_SAND_OUT soc_reg_t             *reg, 
    SOC_SAND_OUT soc_field_t           *fields);

uint32
  _arad_cnt_do_not_count_field_by_src_type_get(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                 proc_id,
    SOC_SAND_IN  int                    src_core,
    SOC_SAND_IN  ARAD_CNT_SRC_TYPE      src_type,
    SOC_SAND_IN  int                    command_id,
    SOC_SAND_OUT soc_field_t            *field
    );

int arad_cnt_stif_ingress_pp_source_set(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN int command_id, 
    SOC_SAND_IN SOC_TMC_CNT_MODE_EG_TYPE eg_type, 
    SOC_SAND_IN int counter_base
    );




#endif /* ARAD_DEBUG_IS_LVL1 */
/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_CNT_INCLUDED__*/
#endif


