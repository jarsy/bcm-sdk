/* $Id: jer_fabric.h,v 1.30 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER_FABRIC_INCLUDED__
/* { */
#define __JER_FABRIC_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/cosq.h>
#include <soc/dpp/SAND/Utils/sand_framework.h>
#include <soc/error.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define SOC_JER_FABRIC_PCP_LENGTH                   (2)
#define SOC_JER_FABRIC_PRIORITY_LENGTH              (2)
#define SOC_JER_FABRIC_PRIORITY_NOF                 (4)

#define SOC_JER_FABRIC_PRIORITY_NDX_MASK            (0xff)
#define SOC_JER_FABRIC_PRIORITY_NDX_NOF             (256)
#define SOC_JER_FABRIC_TDM_NDX_NOF                  (256)

#define SOC_JER_FABRIC_PRIORITY_NDX_IS_TDM_MASK     (0x1)
#define SOC_JER_FABRIC_PRIORITY_NDX_IS_TDM_OFFSET   (0)
#define SOC_JER_FABRIC_PRIORITY_NDX_TC_MASK         (0xE)
#define SOC_JER_FABRIC_PRIORITY_NDX_TC_OFFSET       (1)
#define SOC_JER_FABRIC_PRIORITY_NDX_DP_MASK         (0x30)
#define SOC_JER_FABRIC_PRIORITY_NDX_DP_OFFSET       (4)
#define SOC_JER_FABRIC_PRIORITY_NDX_IS_MC_MASK      (0x40)
#define SOC_JER_FABRIC_PRIORITY_NDX_IS_MC_OFFSET    (6)
#define SOC_JER_FABRIC_PRIORITY_NDX_IS_HP_MASK      (0x80)
#define SOC_JER_FABRIC_PRIORITY_NDX_IS_HP_OFFSET    (7)
#define SOC_JER_FABRIC_STK_FAP_GROUP_SIZE           (32)
#define SOC_JER_FABRIC_STK_MAX_IGNORED_FAP_IDS      (16)
#define SOC_JER_FABRIC_MESH_MC_FAP_GROUP_SIZE       (32)
#define SOC_JER_FABRIC_MESH_MC_REPLICATION_LENGTH   (4)
#define SOC_JER_FABRIC_MESH_MC_REPLICATION_MASK     (0xf)
#define SOC_JER_FABRIC_GROUP_CTX_LENGTH             (4)

#define SOC_JER_FABRIC_MESH_MC_REPLICATION_DEST_0_BIT    (0x1)
#define SOC_JER_FABRIC_MESH_MC_REPLICATION_DEST_1_BIT    (0x2)
#define SOC_JER_FABRIC_MESH_MC_REPLICATION_LOCAL_0_BIT   (0x4)
#define SOC_JER_FABRIC_MESH_MC_REPLICATION_LOCAL_1_BIT   (0x8)

#define SOC_JER_FABRIC_DATA_CELL_RECEIVED_SIZE_IN_BYTES  (74)

#define SOC_JER_FABRIC_DQCF_NOF_CTX_PER_IPT_CORE         (20)
#define SOC_JER_FABRIC_DTQ_NOF_CTX_PER_IPT_CORE          (6)

#define SOC_JER_FABRIC_SYNC_E_MIN_DIVIDER              (2)
#define SOC_JER_FABRIC_SYNC_E_MAX_DIVIDER              (16)

/* } */

/*************
 * MACROS    *
 *************/
/* { */

/*************
 * TYPE DEFS *
 *************/
/* { */

typedef struct {
    int dest_0[SOC_DPP_DEFS_MAX(MAX_NOF_FAP_ID_MAPPED_TO_DEST_LIMIT)];
    int dest_0_count;
    int dest_1[SOC_DPP_DEFS_MAX(MAX_NOF_FAP_ID_MAPPED_TO_DEST_LIMIT)];
    int dest_1_count;
    int dest_2[SOC_DPP_DEFS_MAX(MAX_NOF_FAP_ID_MAPPED_TO_DEST_LIMIT)];
    int dest_2_count;
} JER_MODID_GROUP_MAP;

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
*    soc_jer_fabric_init
* FUNCTION:
*   Initialization of the Jericho blocks configured in fabric module.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  soc_jer_fabric_init(
    SOC_SAND_IN  int unit  
  );


/*********************************************************************
* NAME:
*    soc_jer_fabric_regs_init
* FUNCTION:
*   Initialization of Jericho regs configured in fabric module.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  soc_jer_fabric_regs_init(
    SOC_SAND_IN  int         unit
  );

/*********************************************************************  
* NAME:
*     jer_fabric_gci_backoff_masks_init
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure init fabric flow-control on fabric
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     init gci backoff masks
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer_fabric_gci_backoff_masks_init(
    SOC_SAND_IN  int                 unit
  );


/*********************************************************************  
* NAME:
*     jer_fabric_pcp_dest_mode_config_set / get
* TYPE:
*   PROC
* DATE:
*   Jun 18 2013
* FUNCTION:
*     Enables set / get operations on fabric-pcp (packet cell packing)
*     per destination device.
*     there are three supported pcp modes:
*       - 0- No Packing
*       - 1- Simple Packing
*       - 2- Continous Packing
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32              flags-
*  SOC_SAND_IN  uint32              modid-
*     Id of destination device
*  SOC_SAND_IN/OUT uint32/uint32*  pcp_mode-
*     mode of pcp to set/get.
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer_fabric_pcp_dest_mode_config_set(
    SOC_SAND_IN int              unit,
    SOC_SAND_IN uint32           flags,
    SOC_SAND_IN uint32           modid,
    SOC_SAND_IN uint32           pcp_mode
  );

soc_error_t
  jer_fabric_pcp_dest_mode_config_get(
    SOC_SAND_IN int              unit,
    SOC_SAND_IN uint32           flags,
    SOC_SAND_IN uint32           modid,
    SOC_SAND_OUT uint32          *pcp_mode
  );

/*********************************************************************
* NAME:
*     soc_jer_cosq_gport_sched_set/get
* FUNCTION:
*     Configuration of weight for WFQs in fabric pipes:
*     all, ingress, egress.
* INPUT:
*  SOC_SAND_IN  int                                unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  int                                pipe -
*     Which pipe's weight to configure (0,1,2)
*  SOC_SAND_IN/SOC_SAND_OUT  int/int*              weight -
*     value to configure/retrieve pipe's weight
*  SOC_SAND_IN  soc_dpp_cosq_gport_fabric_pipe_t   fabric_pipe_type -
*     type of fabric pipe to configure (all, ingress, egress)
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/
soc_error_t
  soc_jer_cosq_gport_sched_set(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  int                                pipe,
    SOC_SAND_IN  int                                weight,
    SOC_SAND_IN  soc_dpp_cosq_gport_fabric_pipe_t   fabric_pipe_type  
  );

soc_error_t
  soc_jer_cosq_gport_sched_get(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  int                                pipe,
    SOC_SAND_OUT  int*                               weight,
    SOC_SAND_IN  soc_dpp_cosq_gport_fabric_pipe_t   fabric_pipe_type  
  );

/*********************************************************************  
* NAME:
*     soc_jer_fabric_link_thresholds_pipe_set / get
* TYPE:
*   PROC
* DATE:
*   Jun 18 2013
* FUNCTION:
*     Enables set / get threshold values of several types
*     of fabric links (RCI, LLFC)
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN soc_fabric_pipe_t                  pipe -
*       Fabric pipe number
*  SOC_SAND_IN soc_fabric_link_threshold_type_t   *type -
*      type of thresholds to configure
*  SOC_SAND_IN / OUT int                          *value -
*       Value of threshold to configure / retrieve.
* REMARKS:
*     None.
*********************************************************************/
uint32
  soc_jer_fabric_link_thresholds_pipe_set(
    SOC_SAND_IN int                                  unit,
    SOC_SAND_IN soc_dpp_fabric_pipe_t                pipe,  
    SOC_SAND_IN soc_dpp_fabric_link_threshold_type_t type, 
    SOC_SAND_IN int                                  value
  );

uint32
  soc_jer_fabric_link_thresholds_pipe_get(
    SOC_SAND_IN int                                  unit,
    SOC_SAND_IN soc_dpp_fabric_pipe_t                pipe,  
    SOC_SAND_IN soc_dpp_fabric_link_threshold_type_t type, 
    SOC_SAND_OUT int                                 *value
  );


/*********************************************************************  
* NAME:
*     soc_jer_fabric_cosq_control_backward_flow_control_set / get
* TYPE:
*   PROC
* DATE:
*   Jun 18 2013
* FUNCTION:
*     Enable / disable backwards flow control on supported fifos 
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN soc_gport_t                          port -
*     gport number.
*  SOC_SAND_IN int                                   enable -
*     Whether to enable / disable the feature.
*  SOC_SAND_IN int                                  fifo_type -
*     Type of fifo to configure 
*    
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  soc_jer_fabric_cosq_control_backward_flow_control_set(
      SOC_SAND_IN int                                   unit,
      SOC_SAND_IN soc_gport_t                           port,
      SOC_SAND_IN int                                   enable,
      SOC_SAND_IN soc_dpp_cosq_gport_egress_core_fifo_t fifo_type
  );

soc_error_t
  soc_jer_fabric_cosq_control_backward_flow_control_get(
      SOC_SAND_IN int                                   unit,
      SOC_SAND_IN soc_gport_t                           port,
      SOC_SAND_OUT int                                  *enable,
      SOC_SAND_IN soc_dpp_cosq_gport_egress_core_fifo_t fifo_type
  );


/*********************************************************************  
* NAME:
*     soc_jer_fabric_egress_core_cosq_gport_sched_set / get
* TYPE:
*   PROC
* DATE:
*   Jun 18 2013
* FUNCTION:
*     Set WFQ weight on supported fifos.
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN soc_gport_t                          pipe -
*     Which pipe to access.
*  SOC_SAND_IN int                                  weight -
*     Weight value to configure.
*  SOC_SAND_IN int                                  fifo_type -
*     Type of fifo to configure 
*    
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  soc_jer_fabric_egress_core_cosq_gport_sched_set(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  int                                pipe,
    SOC_SAND_IN  int                                weight,
    SOC_SAND_IN  soc_dpp_cosq_gport_egress_core_fifo_t   fifo_type 
  );

soc_error_t
  soc_jer_fabric_egress_core_cosq_gport_sched_get(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  int                                pipe,
    SOC_SAND_OUT int                                *weight,
    SOC_SAND_IN  soc_dpp_cosq_gport_egress_core_fifo_t   fifo_type 
  );


/*********************************************************************  
* NAME:
*     soc_jer_fabric_cosq_gport_rci_threshold_set / get
* TYPE:
*   PROC
* DATE:
*   Jun 18 2013
* FUNCTION:
*     Set rci threshold on supported fifos.
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN soc_gport_t                          gport -
*     gport number.
*  SOC_SAND_IN  int                                 threshold_val -
*     Threshold value to configure.
*  SOC_SAND_IN int                                  fifo_type -
*     Type of fifo to configure 
*    
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  soc_jer_fabric_cosq_gport_rci_threshold_set(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  soc_gport_t            gport,
    SOC_SAND_IN  int                    threshold_val,
    SOC_SAND_IN  soc_dpp_cosq_gport_egress_core_fifo_t  fifo_type
  );

soc_error_t
  soc_jer_fabric_cosq_gport_rci_threshold_get(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  soc_gport_t            gport,
    SOC_SAND_OUT  int                    *threshold_val,
    SOC_SAND_IN  soc_dpp_cosq_gport_egress_core_fifo_t  fifo_type
  );


/*********************************************************************  
* NAME:
*     soc_jer_fabric_cosq_gport_priority_drop_threshold_set / get
* TYPE:
*   PROC
* DATE:
*   Jun 18 2013
* FUNCTION:
*     Set priority drop threshold on supported fifos.
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN soc_gport_t                          gport -
*     gport number.
*  SOC_SAND_IN  soc_cosq_threshold_t                *threshold_val -
*     sturuct which contains the threshold value
*     to configure / retreive.
*  SOC_SAND_IN int                                  fifo_type -
*     Type of fifo to configure 
*    
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  soc_jer_fabric_cosq_gport_priority_drop_threshold_set(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  soc_gport_t                            gport,
    SOC_SAND_IN  soc_cosq_threshold_t                   *threshold,
    SOC_SAND_IN  soc_dpp_cosq_gport_egress_core_fifo_t  fifo_type
  );

soc_error_t
  soc_jer_fabric_cosq_gport_priority_drop_threshold_get(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  soc_gport_t                            gport,
    SOC_SAND_INOUT  soc_cosq_threshold_t                *threshold,
    SOC_SAND_IN  soc_dpp_cosq_gport_egress_core_fifo_t  fifo_type
  );


/*********************************************************************  
* NAME:
*     soc_jer_fabric_stack_module_all_reachable_ignore_id_set / get
* TYPE:
*   PROC
* DATE:
*   Jun 18 2013
* FUNCTION:
*     Configure a device to be excluded from all-reachable vector calculation
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN soc_module_t                         module -
*     FAP-id.
*  SOC_SAND_IN / OUT  int / int*                    enable -
*     enable / disable the exclusion 
*    
* REMARKS:
*     None.
*********************************************************************/
uint32
  soc_jer_fabric_stack_module_all_reachable_ignore_id_set(
    SOC_SAND_IN  int                unit, 
    SOC_SAND_IN  soc_module_t       module, 
    SOC_SAND_IN  int                enable
  );

uint32
  soc_jer_fabric_stack_module_all_reachable_ignore_id_get(
    SOC_SAND_IN  int                unit, 
    SOC_SAND_IN  soc_module_t       module, 
    SOC_SAND_OUT int                *enable
  );


/*********************************************************************  
* NAME:
*     soc_jer_fabric_stack_module_max_all_reachable_set / get
* TYPE:
*   PROC
* DATE:
*   Jun 18 2013
* FUNCTION:
*     Configure a maximum FAP-id to be included in the
*     all-reachable vector calculation 
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN / OUT soc_module_t / soc_module_t*   module -
*     max FAP-id. should be devided by 32
*    
* REMARKS:
*     None.
*********************************************************************/
uint32
  soc_jer_fabric_stack_module_max_all_reachable_set(
    SOC_SAND_IN  int                unit, 
    SOC_SAND_IN  soc_module_t       max_module
  );

uint32
  soc_jer_fabric_stack_module_max_all_reachable_get(
    SOC_SAND_IN  int                unit, 
    SOC_SAND_OUT soc_module_t       *max_module
  );


/*********************************************************************  
* NAME:
*     soc_jer_fabric_stack_module_max_set / get
* TYPE:
*   PROC
* DATE:
*   Jun 18 2013
* FUNCTION:
*     Configure a maximum FAP-id in the system
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN /OUT soc_module_t /soc_module_t*     module -
*     max FAP-id. should be devided by 32
*    
* REMARKS:
*     None.
*********************************************************************/
uint32
  soc_jer_fabric_stack_module_max_set(
    SOC_SAND_IN  int                unit, 
    SOC_SAND_IN  soc_module_t       max_module
  );

uint32
  soc_jer_fabric_stack_module_max_get(
    SOC_SAND_IN  int                unit, 
    SOC_SAND_OUT soc_module_t       *max_module
  );

uint32
  soc_jer_fabric_stack_module_devide_by_32_verify(
    SOC_SAND_IN  int                unit, 
    SOC_SAND_IN soc_module_t        max_module
  );

/*********************************************************************  
* NAME:
*     soc_jer_fabric_force_set / set
* TYPE:
*   PROC
* DATE:
*   Jun 18 2013
* FUNCTION:
*     Configure a maximum FAP-id in the system
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN /OUT soc_module_t /soc_module_t*     module -
*     max FAP-id. should be devided by 32
*    
* REMARKS:
*     None.
*********************************************************************/

soc_error_t
  soc_jer_fabric_force_set(
    SOC_SAND_IN  int                unit, 
    SOC_SAND_IN soc_dpp_fabric_force_t        force
  );
  
/*********************************************************************
* NAME:
*     soc_jer_fabric_priority_bits_mapping_to_fdt_index_get
* FUNCTION:
*     configure cell attributes(is_tdm, is_hp, tc, dp, is_mc)
*     to an index in IPT_PRIORITY_BITS_MAPPING_2_FDT table
* INPUT:
*  SOC_SAND_IN  int                                unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                             tc -
*     traffic class
*  SOC_SAND_IN  uint32                             dp -
*     drop precedence
*  SOC_SAND_IN  uint32                             flags -
*     relevant flags for cell (is_mc, is_hp)
*  SOC_SAND_OUT uint32                             *index -
*     retrieved index value.
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/
uint32
soc_jer_fabric_priority_bits_mapping_to_fdt_index_get(
    SOC_SAND_IN  int                        unit, 
    SOC_SAND_IN  uint32                     tc, 
    SOC_SAND_IN  uint32                     dp,
    SOC_SAND_IN  uint32                     flags,
    SOC_SAND_OUT uint32                     *index
  );

/*********************************************************************
* NAME:
*     soc_jer_fabric_priority_set / get
* FUNCTION:
*     Set / Get fabric priority according to:
*     traffic_class, is_tdm(flags), queue_type: hc/lc (flags), dp(color).
* INPUT:
*  SOC_SAND_IN  int                                unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                             tc -
*     traffic class
*  SOC_SAND_IN  uint32                             dp -
*     drop precedence
*  SOC_SAND_IN  uint32                             flags -
*     relevant flags for cell (is_mc, is_hp)
*  SOC_SAND_IN/OUT   int/int*                      fabric_priority -
*     fabric priority to set/ get.
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/
soc_error_t
soc_jer_fabric_priority_set(
    SOC_SAND_IN  int                unit, 
    SOC_SAND_IN  uint32             tc, 
    SOC_SAND_IN  uint32             dp,
    SOC_SAND_IN  uint32             flags,
    SOC_SAND_IN  int                fabric_priority
  );

soc_error_t
soc_jer_fabric_priority_get(
    SOC_SAND_IN  int                unit, 
    SOC_SAND_IN  uint32             tc, 
    SOC_SAND_IN  uint32             dp,
    SOC_SAND_IN  uint32             flags,
    SOC_SAND_OUT  int                *fabric_priority
  );


uint32
soc_jer_fabric_port_to_fmac(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  soc_port_t                     port,
    SOC_SAND_OUT  int                            *fmac_index,
    SOC_SAND_OUT  int                            *fmac_inner_link
    );

uint32
soc_jer_fabric_link_topology_set(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  soc_module_t                   local_dest_id, 
    SOC_SAND_IN  int                            links_count,
    SOC_SAND_IN  soc_port_t                     *links_array
  );

uint32
soc_jer_fabric_link_topology_get(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  soc_module_t                   local_dest_id, 
    SOC_SAND_IN  int                            max_links_count,
    SOC_SAND_OUT int                            *links_count, 
    SOC_SAND_OUT soc_port_t                     *links_array
  );

uint32
soc_jer_fabric_link_topology_unset(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  soc_module_t                   local_dest_id
  );

soc_error_t
soc_jer_fabric_multicast_set(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  soc_multicast_t                mc_id,
    SOC_SAND_IN  uint32                         destid_count,
    SOC_SAND_IN  soc_module_t                   *destid_array
  );

uint32
soc_jer_fabric_multicast_get(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  soc_multicast_t                mc_id,
    SOC_SAND_IN  uint32                         destid_count_max,
    SOC_SAND_OUT int                            *destid_count,
    SOC_SAND_OUT soc_module_t                   *destid_array
  );

uint32 
soc_jer_fabric_local_dest_id_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  soc_module_t                   local_dest_id
  );

uint32
soc_jer_fabric_modid_group_get(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  soc_module_t                   local_dest,
    SOC_SAND_IN  int                            modid_max_count,
    SOC_SAND_OUT soc_module_t                   *modid_array,
    SOC_SAND_OUT int                            *modid_count
  );

uint32
soc_jer_fabric_modid_group_set(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  soc_module_t                   local_dest,
    SOC_SAND_IN  int                            modid_count,
    SOC_SAND_IN  soc_module_t                   *modid_array
  );

int
  soc_jer_fabric_mode_validate(
    SOC_SAND_IN int                           unit, 
    SOC_SAND_OUT SOC_TMC_FABRIC_CONNECT_MODE   *fabric_connect_mode
  );
  
/*********************************************************************
* NAME:
*     soc_jer_fabric_minimal_links_to_dest_set / get
* FUNCTION:
*     Set / Get minimal links per destination device.
*     once the number of active links of a destination device is lower than
*     the configurable value, all of its cells will be dropped.
* INPUT:
*  SOC_SAND_IN  int                                unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  soc_module_t                       module_id -
*     Id of device to configure (SOC_MODID_ALL will configure all devices)
*  SOC_SAND_IN/OUT   int/int*                      num_of_links-
*     num of links to configure
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/
uint32
soc_jer_fabric_minimal_links_to_dest_set(
    SOC_SAND_IN int             unit,
    SOC_SAND_IN soc_module_t    module_id,
    SOC_SAND_IN int             num_of_links
  );

uint32
soc_jer_fabric_minimal_links_to_dest_get(
    SOC_SAND_IN int             unit,
    SOC_SAND_IN soc_module_t    module_id,
    SOC_SAND_OUT int            *num_of_links
  );


/*********************************************************************
* NAME:
*     soc_jer_fabric_minimal_links_all_reachable_set / get
* FUNCTION:
*     Set / Get minimal links for all-reachable vactor.
*     In calculation of multicast distribution links,
*     ignore from FAP's with number of links below this number.
* INPUT:
*  SOC_SAND_IN  int                                unit -
*     Identifier of the device to access.
*  SOC_SAND_IN/OUT   int/int*                      num_of_links-
*     num of links to configure
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/
uint32
soc_jer_fabric_minimal_links_all_reachable_set(
    SOC_SAND_IN int             unit,
    SOC_SAND_IN int             num_of_links
  );

uint32
soc_jer_fabric_minimal_links_all_reachable_get(
    SOC_SAND_IN int          unit,
    SOC_SAND_OUT int            *num_of_links
  );

/*********************************************************************
* NAME:
*     soc_jer_fabric_rci_thresholds_config_set / get
* FUNCTION:
*     Configure RCI related information- severity levels,
*     threaholds for all severity levels, min congested links.
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN/OUT   soc_dpp_fabric_rci_config_t    rci_config -
*     RCI information struct
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/
uint32
  soc_jer_fabric_rci_thresholds_config_set(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  soc_dpp_fabric_rci_config_t    rci_config);

uint32
  soc_jer_fabric_rci_thresholds_config_get(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_OUT soc_dpp_fabric_rci_config_t    *rci_config);

/*********************************************************************
* NAME:
*     soc_jer_fabric_link_repeater_enable_set / get
* FUNCTION:
*     Enable link is connected to a repeater
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  soc_port_t                          port -
*     Port number
*  SOC_SAND_IN/OUT  int/int*                        enable -
*     Enable / disable link is connect to a repeater
*  SOC_SAND_IN/OUT  int/int*                        empty_cell_size -
*     Size of empty cell to configure
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/
uint32
  soc_jer_fabric_link_repeater_enable_set(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  soc_port_t port,
    SOC_SAND_IN  int enable,
    SOC_SAND_IN  int empty_cell_size
  );

uint32
  soc_jer_fabric_link_repeater_enable_get(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  soc_port_t port,
    SOC_SAND_OUT int *enable,
    SOC_SAND_OUT int *empty_cell_size
  );

/*********************************************************************
* NAME:
*     soc_jer_fabric_load_balance_init
* FUNCTION:
*     Initialize load-balancing related configuration
* INPUT:
*  SOC_SAND_IN  int                                 unit -
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/
uint32
  soc_jer_fabric_load_balance_init(
    SOC_SAND_IN  int            unit
  );

/*********************************************************************
* NAME:
*     soc_jer_fabric_cell_cpu_data_get
* FUNCTION:
*     Get data from a received source routed cell
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT uint32                              *cell_buffer -
*     Buffer to which the data is retreived.
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/
uint32
soc_jer_fabric_cell_cpu_data_get(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_OUT uint32                  *cell_buffer
  );
  
/*********************************************************************
* NAME:
*     soc_jer_fabric_queues_info_get
* FUNCTION:
*     Get queues max occupancy statuses.
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT soc_dpp_fabric_queues_info_t        *queues_info -
*     Struct to receive all queue's statuses.
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/  
uint32
  soc_jer_fabric_queues_info_get(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_OUT soc_dpp_fabric_queues_info_t    *queues_info
  );

/*********************************************************************
* NAME:
*     soc_jer_fabric_mesh_topology_get
* FUNCTION:
*     Retreive mesh topology statuses
* INPUT:
*  SOC_SAND_IN  int                                  unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT soc_dcmn_fabric_mesh_topology_diag_t *mesh_topology_diag - 
*     Struct to receive all mesh topology statuses.
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/
uint32 
  soc_jer_fabric_mesh_topology_get(
    SOC_SAND_IN  int                                     unit, 
    SOC_SAND_OUT soc_dcmn_fabric_mesh_topology_diag_t    *mesh_topology_diag
  );

/*********************************************************************
* NAME:
*     soc_jer_fabric_mesh_check
* FUNCTION:
*     Check mesh status
* INPUT:
*  SOC_SAND_IN  int                                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN uint8                                    stand_alone,
*     Is device stand alone.
*  SOC_SAND_OUT uint8 *success - 
*     mesh status check.
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/
uint32 
  soc_jer_fabric_mesh_check(
    SOC_SAND_IN  int                                     unit, 
    SOC_SAND_IN uint8                                    stand_alone,
    SOC_SAND_OUT uint8                                   *success
  );

/*********************************************************************
* NAME:
*     soc_jer_fabric_rx_fifo_get
* FUNCTION:
*     Retreive fabric rx fifo's status
* INPUT:
*  SOC_SAND_IN  int                                  unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT soc_dcmn_fabric_rx_fifo_diag_t       *rx_fifo_diag - 
*     Struct to receive all statuses.
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/
uint32 
  soc_jer_fabric_rx_fifo_status_get(
    SOC_SAND_IN  int                                unit, 
    SOC_SAND_OUT soc_dpp_fabric_rx_fifo_diag_t     *rx_fifo_diag
  );

/*********************************************************************
* NAME:
*     soc_jer_fabric_port_sync_e_link_set /get
* FUNCTION:
*     Configure port for syncE configuration
* INPUT:
*  SOC_SAND_IN  int                                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  int                                  is_master -
*     Configure master /slave port
*  SOC_SAND_IN/OUT  int/int*                         port -
*     Port to configure
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/
uint32
  soc_jer_fabric_port_sync_e_link_set(
    SOC_SAND_IN  int                                unit, 
    SOC_SAND_IN  int                                is_master, 
    SOC_SAND_IN  int                                port
  );

uint32
  soc_jer_fabric_port_sync_e_link_get(
    SOC_SAND_IN  int                                unit, 
    SOC_SAND_IN  int                                is_master, 
    SOC_SAND_OUT int                                *port
  );

/*********************************************************************
* NAME:
*     soc_jer_fabric_port_sync_e_divider_set /get
* FUNCTION:
*     Configure divider for syncE configuration
* INPUT:
*  SOC_SAND_IN  int                                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN/OUT  int/int*                         divider -
*     Value of divider to configure
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/
uint32 
  soc_jer_fabric_port_sync_e_divider_set(
    SOC_SAND_IN  int             unit,
    SOC_SAND_IN  int             divider
  );

uint32 
  soc_jer_fabric_port_sync_e_divider_get(
    SOC_SAND_IN  int             unit,
    SOC_SAND_OUT int             *divider
  );


/*********************************************************************
* NAME:
*     soc_jer_fabric_sync_e_enable_set/get
* FUNCTION:
*     Enable/Disable Fabric SyncE configuration
* INPUT:
*  SOC_SAND_IN  int                                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  int                                  is_master -
*     Configure master port/ slave port
*  SOC_SAND_IN  int                                  to_enable-
*     1- enable synce for master/slave for fabric
*     0- disable synce for master/slave for fabric (means enabled for nif)
*  SOC_SAND_OUT int*                                 is_fabric_synce-
*     indication whether faric syncE is enabled or disabled.
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/
uint32
  soc_jer_fabric_sync_e_enable_set(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  int                                is_master,
    SOC_SAND_IN  int                                to_enable
  );

uint32
  soc_jer_fabric_sync_e_enable_get(
    SOC_SAND_IN  int                                unit, 
    SOC_SAND_IN  int                                is_master, 
    SOC_SAND_OUT int                                *is_fabric_synce
  );



/*********************************************************************
* NAME:
*     soc_jer_fabric_efms_enable_set/get
* FUNCTION:
*     Enable FMS bypass for specifc device in IPS and FCR tables :
*     IPS_FMSBYP and FCR_EFMS_SOURCE_PIPE
* INPUT:
*  SOC_SAND_IN  int                                  unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  bcm_module_t                         modid -
*     device id
*  SOC_SAND_IN/OUT  uint32/uint32*                     val-
*     1 to enable EFMS, 0 to use FMS.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   None.
*********************************************************************/
int
  soc_jer_fabric_efms_enable_set(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  uint32             modid,
    SOC_SAND_IN  int                val
  );

int
  soc_jer_fabric_efms_enable_get(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  uint32             modid,
    SOC_SAND_OUT  int               *val
  );

/*********************************************************************
* NAME:
*     soc_jer_fabric_link_config_ovrd
* FUNCTION:
*     Overwriting jer default fabric configuration in case of qmx
* INPUT:
*       int   unit - Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Used in mbcm dispatcher.
*********************************************************************/

soc_error_t
soc_jer_fabric_link_config_ovrd(
  int                unit
);

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __JER_FABRIC_INCLUDED__*/
#endif
