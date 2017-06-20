/*
* $Id: cosq.h,v 1.7 Broadcom SDK $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*
* This file contains structures and functions declarations for 
* In-band cell configuration and Source Routed Cell.
* 
*/
#ifndef _SOC_DNX_COSQ_H
#define _SOC_DNX_COSQ_H

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/error.h>
#include <soc/dnx/legacy/TMC/tmc_api_end2end_scheduler.h>
#include <soc/dnx/legacy/TMC/tmc_api_ingress_traffic_mgmt.h>
#include <soc/dnx/legacy/TMC/tmc_api_flow_control.h>

#define SOC_DNX_COSQ_PORT_PRIORITY_MIN   0
#define SOC_DNX_COSQ_PORT_PRIORITY_MAX   3

typedef struct dnx_soc_ips_queue_info_s {
    uint32 queue_id;
    uint32 queue_byte_size;
    uint32 target_flow_id;
    uint32 got_flow_info;
    uint32 target_fap_id;
    uint32 target_data_port_id;
} dnx_soc_ips_queue_info_t;

typedef struct dnx_soc_hr2ps_info_s {
    uint32              mode;
    uint32              weight;
    uint32              kbits_sec_max;
    uint32              max_burst;
    DNX_TMC_SCH_SE_INFO se_info;
    DNX_TMC_SCH_FLOW    flow_info;
} dnx_soc_hr2ps_info_t;

/* flow control / addmission settings */
typedef struct dnx_soc_cosq_threshold_s {
    int dp;                         /* drop precedence. relevant for
                                       BCM_COSQ_THRESHOLD_PER_DP flag value */
    int value;                      /* threshold value for drop/flow control */
    int tc;                         /* traffic class for which the thresholds
                                       are to be configured */
} dnx_soc_cosq_threshold_t;

typedef enum soc_dnx_cosq_gport_fabric_pipe_e {
    soc_dnx_cosq_gport_fabric_pipe_all = 0,
    soc_dnx_cosq_gport_fabric_pipe_ingress = 1,
    soc_dnx_cosq_gport_fabric_pipe_egress = 2
} soc_dnx_cosq_gport_fabric_pipe_t;

typedef enum soc_dnx_cosq_gport_egress_core_fifo_e {
    soc_dnx_cosq_gport_egress_core_fifo_fabric_ucast = 0,
    soc_dnx_cosq_gport_egress_core_fifo_fabric_mcast = 1,
    soc_dnx_cosq_gport_egress_core_fifo_fabric_tdm = 2,
    soc_dnx_cosq_gport_egress_core_fifo_local_ucast = 3,
    soc_dnx_cosq_gport_egress_core_fifo_local_mcast = 4,
    soc_dnx_cosq_gport_egress_core_fifo_local_tdm = 5,
    soc_dnx_cosq_gport_egress_core_fifo_fabric_generic_pipe = 6
} soc_dnx_cosq_gport_egress_core_fifo_t;

typedef enum soc_dnx_cosq_threshold_type_e {
    soc_dnx_cosq_threshold_bytes = _SHR_COSQ_THRESHOLD_BYTES,
    soc_dnx_cosq_threshold_packet_descriptors = _SHR_COSQ_THRESHOLD_PACKET_DESCRIPTORS,
    soc_dnx_cosq_threshold_packets = _SHR_COSQ_THRESHOLD_PACKETS,
    soc_dnx_cosq_threshold_data_buffers = _SHR_COSQ_THRESHOLD_DATA_BUFFERS,
    soc_dnx_cosq_threshold_available_packet_descriptors = _SHR_COSQ_THRESHOLD_AVAILABLE_PACKET_DESCRIPTORS,
    soc_dnx_cosq_threshold_available_data_buffers = _SHR_COSQ_THRESHOLD_AVAILABLE_DATA_BUFFERS,
    soc_dnx_cosq_threshold_buffer_descriptor_buffers = _SHR_COSQ_THRESHOLD_BUFFER_DESCRIPTOR_BUFFERS,
    soc_dnx_cosq_threshold_buffer_descriptors = _SHR_COSQ_THRESHOLD_BUFFER_DESCRIPTORS,
    soc_dnx_cosq_threshold_dbuffs = _SHR_COSQ_THRESHOLD_DBUFFS,
    soc_dnx_cosq_threshold_full_dbuffs = _SHR_COSQ_THRESHOLD_FULL_DBUFFS,
    soc_dnx_cosq_threshold_mini_dbuffs = _SHR_COSQ_THRESHOLD_MINI_DBUFFS,
    soc_dnx_cosq_threshold_dynamic_weight = _SHR_COSQ_THRESHOLD_DYNAMIC_WEIGHT,
    soc_dnx_cosq_threshold_packet_descriptors_min = _SHR_COSQ_THRESHOLD_PACKET_DESCRIPTORS_MIN,
    soc_dnx_cosq_threshold_packet_descriptors_max = _SHR_COSQ_THRESHOLD_PACKET_DESCRIPTORS_MAX,
    soc_dnx_cosq_threshold_packet_descriptors_alpha = _SHR_COSQ_THRESHOLD_PACKET_DESCRIPTORS_ALPHA,
    soc_dnx_cosq_threshold_data_buffers_min = _SHR_COSQ_THRESHOLD_DATA_BUFFERS_MIN,
    soc_dnx_cosq_threshold_data_buffers_max = _SHR_COSQ_THRESHOLD_DATA_BUFFERS_MAX,
    soc_dnx_cosq_thereshold_data_buffers_alpha = _SHR_COSQ_THERESHOLD_DATA_BUFFERS_ALPHA,
    soc_dnx_cosq_threshold_num = _SHR_COSQ_THRESHOLD_NUM
} soc_dnx_cosq_threshold_type_t;

typedef enum soc_dnx_cosq_threshold_global_type_e {
    soc_dnx_cosq_threshold_global_type_unicast,
    soc_dnx_cosq_threshold_global_type_multicast,
    soc_dnx_cosq_threshold_global_type_total,
    soc_dnx_cosq_threshold_global_type_service_pool_0,
    soc_dnx_cosq_threshold_global_type_service_pool_1 /* must be after pool_0 */
} soc_dnx_cosq_threshold_global_type_t ;

/* cosq ingress port drop priority maps */
typedef enum soc_dnx_prd_map_e {
    socDnxPrdTmTcDpPriorityTable = 0,
    socDnxPrdIpDscpToPriorityTable = 1,
    socDnxPrdEthPcpDeiToPriorityTable = 2,
    socDnxPrdMplsExpToPriorityTable = 3
} soc_dnx_prd_map_t;


soc_error_t
  soc_dnx_cosq_flow_and_up_info_get(
    DNX_SAND_IN     int                          unit,
    DNX_SAND_IN     int                          core,
    DNX_SAND_IN     uint8                           is_flow,
    DNX_SAND_IN     uint32                          dest_id, /* Destination port or flow ndx */
    DNX_SAND_IN     uint32                          reterive_status,
    DNX_SAND_INOUT  DNX_TMC_SCH_FLOW_AND_UP_INFO    *flow_and_up_info
  );

soc_error_t
  soc_dnx_cosq_hr2ps_info_get(
    DNX_SAND_IN   int                                unit,
    DNX_SAND_IN   int                                core,
    DNX_SAND_IN   uint32                             se_id,
    DNX_SAND_OUT  dnx_soc_hr2ps_info_t                   *hr2ps_info
  );

soc_error_t
  soc_dnx_cosq_non_empty_queues_info_get(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  int                   core_id,
    DNX_SAND_IN  uint32                queue_to_read_from,
    DNX_SAND_IN  uint32                max_array_size,
    DNX_SAND_OUT dnx_soc_ips_queue_info_t* queues,
    DNX_SAND_OUT uint32*               nof_queues_filled,
    DNX_SAND_OUT uint32*               next_queue,
    DNX_SAND_OUT uint32*               reached_end
  );

soc_error_t
  soc_dnx_cosq_ingress_queue_info_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  queue_ndx,
    DNX_SAND_OUT DNX_TMC_ITM_QUEUE_INFO      *info    
  );

soc_error_t
  soc_dnx_cosq_ingress_queue_category_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  queue_ndx,
    DNX_SAND_OUT int                  *voq_category
  );

soc_error_t
  soc_dnx_cosq_ingress_queue_to_flow_mapping_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  queue_ndx,
    DNX_SAND_OUT DNX_TMC_IPQ_QUARTET_MAP_INFO          *queue_map_info
  );

soc_error_t
  soc_dnx_cosq_ingress_test_tmplt_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_OUT  DNX_TMC_ITM_ADMIT_TSTS     *test_tmplt
  );

soc_error_t
  soc_dnx_cosq_vsq_index_global_to_group_get(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  uint32                     vsq_id,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_GROUP      *soc_vsq_group_type,
    DNX_SAND_OUT uint32                        *vsq_index,
    DNX_SAND_OUT uint8                         *is_ocb_only
  );

int soc_dnx_voq_max_size_drop(int unit, uint32 *is_max_size);

/*
 * This function receives a FAP ID and returnes wethrer the FAP ID is local
 * (of a core of the local device) and if so, returns the core ID.
 * For single core devices or devices in single core mode, the core ID is always 0.
 */
soc_error_t soc_dnx_is_fap_id_local_and_get_core_id(
    DNX_SAND_IN   int      unit,
    DNX_SAND_IN   uint32   fap_id,    /* input FAP ID */
    DNX_SAND_OUT  uint8    *is_local, /* returns TRUE/FASLE based on if fap_id is of a local core */
    DNX_SAND_OUT  int      *core_id   /* if is_local returns TRUE, will contain the core ID of the FAP ID */
);

soc_error_t
  soc_dnx_fc_status_info_get(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN DNX_TMC_FC_STATUS_KEY   *fc_status_key,
    DNX_SAND_OUT DNX_TMC_FC_STATUS_INFO *fc_status_info 
  );

/*
 * Convert gport describing a destination to to TM dest information in DNX_TMC_DEST_INFO.
 * Identifies destination type and ID.
 */
soc_error_t 
dnx_gport_to_tm_dest_info( /* This code was originally in _bcm_dnx_gport_to_tm_dest_info() */
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  soc_gport_t       dest,      /* input destination gport */
    DNX_SAND_OUT DNX_TMC_DEST_INFO *dest_info /* output destination structure */
);

#endif
