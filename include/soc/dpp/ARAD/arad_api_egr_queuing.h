/* $Id: arad_api_egr_queuing.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __ARAD_API_EGR_QUEUING_INCLUDED__
/* { */
#define __ARAD_API_EGR_QUEUING_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/ARAD/arad_api_general.h>
#include <soc/dpp/TMC/tmc_api_egr_queuing.h>
#include <soc/dpp/dpp_config_imp_defs.h>
#include <soc/dpp/cosq.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define ARAD_EGR_OFP_SCH_WFQ_WEIGHT_MAX 255

#define ARAD_EGR_OFP_INTERFACE_PRIO_NONE SOC_TMC_EGR_OFP_INTERFACE_PRIO_NONE

#define ARAD_NOF_THRESH_TYPES                                       SOC_TMC_NOF_THRESH_TYPES

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

#define ARAD_EGR_OFP_INTERFACE_PRIO_HIGH                  SOC_TMC_EGR_OFP_INTERFACE_PRIO_HIGH
#define ARAD_EGR_OFP_INTERFACE_PRIO_MID                   SOC_TMC_EGR_OFP_INTERFACE_PRIO_MID
#define ARAD_EGR_OFP_INTERFACE_PRIO_LOW                   SOC_TMC_EGR_OFP_INTERFACE_PRIO_LOW
#define ARAD_EGR_OFP_INTERFACE_PRIO_PFC_LOWEST            SOC_TMC_EGR_OFP_INTERFACE_PRIO_PFC_LOWEST
#define ARAD_EGR_OFP_CHNIF_NOF_PRIORITIES                 SOC_TMC_EGR_OFP_CHNIF_NOF_PRIORITIES
typedef SOC_TMC_EGR_OFP_INTERFACE_PRIO                        ARAD_EGR_OFP_INTERFACE_PRIO;

#define ARAD_EGR_QUEUING_PARTITION_SCHEME_DISCRETE        SOC_TMC_EGR_QUEUING_PARTITION_SCHEME_DISCRETE
#define ARAD_EGR_QUEUING_PARTITION_SCHEME_STRICT          SOC_TMC_EGR_QUEUING_PARTITION_SCHEME_STRICT
#define ARAD_NOF_EGR_QUEUING_PARTITION_SCHEMES            SOC_TMC_NOF_EGR_QUEUING_PARTITION_SCHEMES
typedef SOC_TMC_EGR_QUEUING_PARTITION_SCHEME                  ARAD_EGR_QUEUING_PARTITION_SCHEME;

typedef SOC_TMC_EGR_QUEUING_TH_DB_GLOBAL                      ARAD_EGR_QUEUING_TH_DB_GLOBAL;
typedef SOC_TMC_EGR_QUEUING_TH_DB_POOL                        ARAD_EGR_QUEUING_TH_DB_POOL;
typedef SOC_TMC_EGR_QUEUING_TH_DB_PORT                        ARAD_EGR_QUEUING_TH_DB_PORT;
typedef SOC_TMC_EGR_QUEUING_DEV_TH                            ARAD_EGR_QUEUING_DEV_TH;
typedef SOC_TMC_EGR_QUEUING_MC_COS_MAP                        ARAD_EGR_QUEUING_MC_COS_MAP;
typedef SOC_TMC_EGR_QUEUING_IF_FC                             ARAD_EGR_QUEUING_IF_FC;
typedef SOC_TMC_EGR_QUEUING_IF_UC_FC                          ARAD_EGR_QUEUING_IF_UC_FC;

#define ARAD_EGR_PORT_THRESH_TYPE_0                       SOC_TMC_EGR_PORT_THRESH_TYPE_0
#define ARAD_EGR_PORT_THRESH_TYPE_1                       SOC_TMC_EGR_PORT_THRESH_TYPE_1
#define ARAD_EGR_PORT_THRESH_TYPE_2                       SOC_TMC_EGR_PORT_THRESH_TYPE_2
#define ARAD_EGR_PORT_THRESH_TYPE_3                       SOC_TMC_EGR_PORT_THRESH_TYPE_3
#define ARAD_EGR_PORT_THRESH_TYPE_4                       SOC_TMC_EGR_PORT_THRESH_TYPE_4
#define ARAD_EGR_PORT_THRESH_TYPE_5                       SOC_TMC_EGR_PORT_THRESH_TYPE_5
#define ARAD_EGR_PORT_THRESH_TYPE_6                       SOC_TMC_EGR_PORT_THRESH_TYPE_6
#define ARAD_EGR_PORT_THRESH_TYPE_7                       SOC_TMC_EGR_PORT_THRESH_TYPE_7
#define ARAD_EGR_PORT_THRESH_TYPE_8                       SOC_TMC_EGR_PORT_THRESH_TYPE_8
#define ARAD_EGR_PORT_THRESH_TYPE_9                       SOC_TMC_EGR_PORT_THRESH_TYPE_9
#define ARAD_EGR_PORT_THRESH_TYPE_10                      SOC_TMC_EGR_PORT_THRESH_TYPE_10
#define ARAD_EGR_PORT_THRESH_TYPE_11                      SOC_TMC_EGR_PORT_THRESH_TYPE_11
#define ARAD_EGR_PORT_THRESH_TYPE_12                      SOC_TMC_EGR_PORT_THRESH_TYPE_12
#define ARAD_EGR_PORT_THRESH_TYPE_13                      SOC_TMC_EGR_PORT_THRESH_TYPE_13
#define ARAD_EGR_PORT_THRESH_TYPE_14                      SOC_TMC_EGR_PORT_THRESH_TYPE_14
#define ARAD_EGR_PORT_THRESH_TYPE_15                      SOC_TMC_EGR_PORT_THRESH_TYPE_15
#define ARAD_EGR_PORT_NOF_THRESH_TYPES                    SOC_TMC_EGR_PORT_NOF_THRESH_TYPES_ARAD
typedef SOC_TMC_EGR_PORT_THRESH_TYPE                          ARAD_EGR_PORT_THRESH_TYPE;

#define ARAD_EGR_Q_PRIO_ALL                               SOC_TMC_EGR_Q_PRIO_ALL
#define ARAD_EGR_Q_PRIO_0                                 SOC_TMC_EGR_Q_PRIO_0
#define ARAD_EGR_Q_PRIO_1                                 SOC_TMC_EGR_Q_PRIO_1
#define ARAD_EGR_Q_PRIO_2                                 SOC_TMC_EGR_Q_PRIO_2
#define ARAD_EGR_Q_PRIO_3                                 SOC_TMC_EGR_Q_PRIO_3
#define ARAD_EGR_Q_PRIO_4                                 SOC_TMC_EGR_Q_PRIO_4
#define ARAD_EGR_Q_PRIO_5                                 SOC_TMC_EGR_Q_PRIO_5
#define ARAD_EGR_Q_PRIO_6                                 SOC_TMC_EGR_Q_PRIO_6
#define ARAD_EGR_Q_PRIO_7                                 SOC_TMC_EGR_Q_PRIO_7
#define ARAD_EGR_NOF_Q_PRIO                               SOC_TMC_EGR_NOF_Q_PRIO_ARAD
typedef SOC_TMC_EGR_Q_PRIO                                    ARAD_EGR_Q_PRIO;

typedef SOC_TMC_EGR_OFP_SCH_INFO                              ARAD_EGR_OFP_SCH_INFO;
typedef SOC_TMC_EGR_DROP_THRESH                               ARAD_EGR_DROP_THRESH;
typedef SOC_TMC_EGR_FC_DEVICE_THRESH                          ARAD_EGR_FC_DEVICE_THRESH;
typedef SOC_TMC_EGR_FC_CHNIF_THRESH                           ARAD_EGR_FC_CHNIF_THRESH;
typedef SOC_TMC_EGR_FC_OFP_THRESH                             ARAD_EGR_FC_OFP_THRESH;

#define ARAD_EGR_UCAST_TO_SCHED                           SOC_TMC_EGR_UCAST_TO_SCHED
#define ARAD_EGR_MCAST_TO_UNSCHED                         SOC_TMC_EGR_MCAST_TO_UNSCHED
#define ARAD_EGR_NOF_Q_PRIO_MAPPING_TYPES                 SOC_TMC_EGR_NOF_Q_PRIO_MAPPING_TYPES
typedef SOC_TMC_EGR_Q_PRIO_MAPPING_TYPE                       ARAD_EGR_Q_PRIO_MAPPING_TYPE;

#define ARAD_EGR_PORT_SHAPER_DATA_MODE                    SOC_TMC_EGR_PORT_SHAPER_DATA_MODE
#define ARAD_EGR_PORT_SHAPER_PACKET_MODE                  SOC_TMC_EGR_PORT_SHAPER_PACKET_MODE
#define ARAD_EGR_NOF_PORT_SHAPER_MODES                    SOC_TMC_EGR_NOF_PORT_SHAPER_MODES
typedef SOC_TMC_EGR_PORT_SHAPER_MODE                          ARAD_EGR_PORT_SHAPER_MODE;

#define ARAD_EGR_PORT_ONE_PRIORITY                        SOC_TMC_EGR_PORT_ONE_PRIORITY                          
#define ARAD_EGR_PORT_TWO_PRIORITIES                      SOC_TMC_EGR_PORT_TWO_PRIORITIES
#define ARAD_EGR_PORT_EIGHT_PRIORITIES                    SOC_TMC_EGR_PORT_EIGHT_PRIORITIES
#define ARAD_EGR_NOF_PORT_PRIORITY_MODES                  SOC_TMC_EGR_NOF_PORT_PRIORITY_MODES
typedef SOC_TMC_EGR_PORT_PRIORITY_MODE                        ARAD_EGR_PORT_PRIORITY_MODE;

typedef SOC_TMC_EGR_Q_PRIORITY                                ARAD_EGR_Q_PRIORITY;

typedef SOC_TMC_EGR_FC_DEV_THRESH_INNER                       ARAD_EGR_FC_DEV_THRESH_INNER;
typedef SOC_TMC_EGR_OFP_SCH_WFQ                               ARAD_EGR_OFP_SCH_WFQ;
typedef SOC_TMC_EGR_THRESH_INFO                               ARAD_EGR_THRESH_INFO;

typedef SOC_TMC_EGR_QUEUING_TCG_INFO                          ARAD_EGR_QUEUING_TCG_INFO;
typedef SOC_TMC_EGR_TCG_SCH_WFQ                               ARAD_EGR_TCG_SCH_WFQ;


#define ARAD_EGR_CGM_OTM_PORTS_NUM 256
#define ARAD_EGR_CGM_QUEUES_NUM 256
#define ARAD_EGR_CGM_IF_NUM SOC_DPP_IMP_DEFS_MAX(NOF_CORE_INTERFACES)
/*
Arad congestion management statics.
Both the current and the maximum values of these statistics can be retrieved.
The maximum values retrieved are since their previous retrieve.
*/
typedef struct
{
    /* total number of allocated packet descriptors */
    uint16 pd;
    /* total number of allocated Data Buffers */
    uint16 db;
    /* number of packet descriptors allocated to unicast packets */
    uint16 uc_pd;
    /* number of packet descriptors allocated to multicast replication packets */
    uint16 mc_pd;
    /* number of Data Buffers allocated to unicast packets */
    uint16 uc_db;
    /* number of Data Buffers allocated to multicast packets, regardless of number of replications */
    uint16 mc_db;
    /* Current number of packet descriptors allocated to unicast packets destined to this interface. */
    uint32 uc_pd_if[ARAD_EGR_CGM_IF_NUM];
    /* Current number of packet descriptors allocated to Multicast replication packets destined to this interface */
    uint32 mc_pd_if[ARAD_EGR_CGM_IF_NUM];
    /* The size of this interface contributed by unicast packets only, where the size is measured in 256B units. */
    uint32 uc_size_256_if[ARAD_EGR_CGM_IF_NUM];
    /* The size of this interface contributed by Multicast replication packets only, where the size is measured in 256B units. */
    uint32 mc_size_256_if[ARAD_EGR_CGM_IF_NUM];

    /* UC/MC number of PDs per OTM port */
    uint16 uc_pd_port[ARAD_EGR_CGM_OTM_PORTS_NUM];
    uint16 mc_pd_port[ARAD_EGR_CGM_OTM_PORTS_NUM];
    /* UC/MC number of PDs per queue */
    uint16 uc_pd_queue[ARAD_EGR_CGM_QUEUES_NUM];
    uint16 mc_pd_queue[ARAD_EGR_CGM_QUEUES_NUM];
    /* UC number of DBs per OTM port */
    uint32 uc_db_port[ARAD_EGR_CGM_OTM_PORTS_NUM];
    /* MC size in 256 bytes per OTM port (each DB contributes according to the number of replications) */
    uint32 mc_db_port[ARAD_EGR_CGM_OTM_PORTS_NUM];
    /* UC number of DBs per queue */
    uint32 uc_db_queue[ARAD_EGR_CGM_QUEUES_NUM];
    /* MC size in 256 bytes per queue (each DB contributes according to the number of replications) */
    uint32 mc_db_queue[ARAD_EGR_CGM_QUEUES_NUM];

    /* number of packet descriptors allocated to multicast replication packets bound to Service Pools */
    uint16 mc_pd_sp[2];
    /* number of Data Buffers allocated to multicast packets bound to Service Pool0 */
    uint16 mc_db_sp[2];
    /* Number of MC-PD'S - Per SP per TC. Indicates the value of Multicast Packet Descriptors Counter ( Per SP per TC). Low 8 counters are for service-pool-0 and high 8 counters are for service-pool-1. */
    uint16 mc_pd_sp_tc[16];
    /* Number of MC-DB'S - Per SP per TC. Indicates the value of Multicast Data buffers Counter ( Per SP per TC). Low 8 counters are for service-pool-0 and high 8 counters are for service-pool-1. */
    uint16 mc_db_sp_tc[16];

    /* Current number of available reserved packet descriptors in Service Pools. */
    uint16 mc_rsvd_pd_sp[2]; /* supported only for current values and not for max values */
    /* Current number of available reserved Data Buffers in Service Pools. */
    uint16 mc_rsvd_db_sp[2]; /* supported only for current values and not for max values */

} ARAD_EGR_CGM_CONGENSTION_STATS;



/* Arad congestion management counters */
typedef struct
{
    /* counts the number of packets that were dropped due to lack of unicast packet descriptors */
    uint32 uc_pd_dropped;
    /* counts the number of packets that were dropped due to lack of multicast packet descriptors */
    uint32 mc_rep_pd_dropped;
    /* counts the number of packets that were dropped by the RQP due to lack of unicast data buffers */
    uint32 uc_db_dropped_by_rqp;
    /* counts the number of packets that were dropped by the PQP due to lack of unicast data buffers */
    uint32 uc_db_dropped_by_pqp;
    /* Counts the number of packets that were dropped due to lack of multicast data buffers. Note that this counter does not count each replication drop but rather the packet drops before replication, i.e. when a packet with n replication is dropped due to lack of multicast data buffers, the counter is increased by one regardless of number of replications. */
    uint32 mc_db_dropped;

    /* Counts the number of multicast replications that were dropped due to reaching to the maximum port or queue length, where length is measured in units of 256 bytes. */
    uint32 mc_rep_db_dropped;
} ARAD_EGR_CGM_CONGENSTION_COUNTERS;


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
*     arad_egr_ofp_thresh_type_set
* TYPE:
*   PROC
* DATE:
*   Dec 20 2007
* FUNCTION:
*     Sets Outgoing FAP Port (OFP) threshold type, per port.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID         ofp_ndx -
*     OFP index. Range: 0 - ARAD_FAP_PORT_ID_MAX
*  SOC_SAND_IN  ARAD_EGR_PORT_THRESH_TYPE ofp_thresh_type -
*     The threshold type to set. Port-level Drop thresholds
*     and Flow control thresholds will be set per threshold
*     type.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_egr_ofp_thresh_type_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  ARAD_FAP_PORT_ID    tm_port,
    SOC_SAND_IN  ARAD_EGR_PORT_THRESH_TYPE ofp_thresh_type
  );

/*********************************************************************
* NAME:
*     arad_egr_ofp_thresh_type_get
* TYPE:
*   PROC
* DATE:
*   Dec 20 2007
* FUNCTION:
*     Sets Outgoing FAP Port (OFP) threshold type, per port.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID         ofp_ndx -
*     OFP index. Range: 0 - ARAD_FAP_PORT_ID_MAX
*  SOC_SAND_OUT ARAD_EGR_PORT_THRESH_TYPE *ofp_thresh_type -
*     The threshold type to set. Port-level Drop thresholds
*     and Flow control thresholds will be set per threshold
*     type.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_egr_ofp_thresh_type_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  ARAD_FAP_PORT_ID         ofp_ndx,
    SOC_SAND_OUT ARAD_EGR_PORT_THRESH_TYPE *ofp_thresh_type
  );

/*********************************************************************
* NAME:
*     arad_egr_sched_drop_set
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Set scheduled drop thresholds for egress queues per
*     queue-priority.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_EGR_Q_PRIO          prio_ndx -
*     Queue priority index. Range: ARAD_EGR_Q_PRIO_LOW -
*     ARAD_EGR_Q_PRIO_HIGH.
*  SOC_SAND_IN  ARAD_EGR_DROP_THRESH     *thresh -
*     drop thresholds to set.
*  SOC_SAND_OUT ARAD_EGR_DROP_THRESH     *exact_thresh -
*     will be filled with exact values.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_egr_sched_drop_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  int                 profile,
    SOC_SAND_IN  ARAD_EGR_Q_PRIO          prio_ndx,
    SOC_SAND_IN  ARAD_EGR_DROP_THRESH     *thresh,
    SOC_SAND_OUT ARAD_EGR_DROP_THRESH     *exact_thresh
  );

/*********************************************************************
* NAME:
*     arad_egr_sched_drop_get
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Set scheduled drop thresholds for egress queues per
*     queue-priority.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_EGR_Q_PRIO          prio_ndx -
*     Queue priority index. Range: ARAD_EGR_Q_PRIO_LOW -
*     ARAD_EGR_Q_PRIO_HIGH.
*  SOC_SAND_OUT ARAD_EGR_DROP_THRESH     *thresh -
*     drop thresholds to set.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_egr_sched_drop_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_EGR_Q_PRIO          prio_ndx,
    SOC_SAND_OUT ARAD_EGR_DROP_THRESH     *thresh
  );

/*********************************************************************
* NAME:
*     arad_egr_unsched_drop_set
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Set unscheduled drop thresholds for egress queues, per
*     queue-priority and drop precedence.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_EGR_Q_PRIO          prio_ndx -
*     Queue priority index. Range: ARAD_EGR_Q_PRIO_LOW -
*     ARAD_EGR_Q_PRIO_HIGH.
*  SOC_SAND_IN  uint32                 dp_ndx -
*     Drop precedence index. Range: 0 - ARAD_NOF_DP-1.
*  SOC_SAND_IN  ARAD_EGR_DROP_THRESH     *thresh -
*     drop thresholds to set.
*  SOC_SAND_OUT ARAD_EGR_DROP_THRESH     *exact_thresh -
*     Will be filled with exact values.
* REMARKS:
*     The unscheduled traffic is assigned to the matching threshold
*     not according to the packet Drop Precedence, rather then
*     according to the Unscheduled Drop Priority value, as set
*     by the 'arad_egr_unsched_drop_prio_set' API, per TC and DP
*     The "dp_ndx" in this API refers to this Drop Priority, and not
*     directly to the packet Drop Precedence field
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_egr_unsched_drop_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  int                 profile,
    SOC_SAND_IN  ARAD_EGR_Q_PRIO          prio_ndx,
    SOC_SAND_IN  uint32                 dp_ndx,
    SOC_SAND_IN  ARAD_EGR_DROP_THRESH     *thresh,
    SOC_SAND_OUT ARAD_EGR_DROP_THRESH     *exact_thresh
  );

uint32
  arad_egr_sched_port_fc_thresh_set(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  int                core,
    SOC_SAND_IN  int                threshold_type,
    SOC_SAND_IN  SOC_TMC_EGR_FC_OFP_THRESH *thresh
  );
uint32
  arad_egr_sched_q_fc_thresh_set(
    SOC_SAND_IN  int       unit,
    SOC_SAND_IN  int       core,
    SOC_SAND_IN  int       prio,
    SOC_SAND_IN  int threshold_type,
    SOC_SAND_IN  SOC_TMC_EGR_FC_OFP_THRESH  *thresh
  );

/*********************************************************************
* NAME:
*     arad_egr_unsched_drop_get
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Set unscheduled drop thresholds for egress queues, per
*     queue-priority and drop precedence.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_EGR_Q_PRIO          prio_ndx -
*     Queue priority index. Range: ARAD_EGR_Q_PRIO_LOW -
*     ARAD_EGR_Q_PRIO_HIGH.
*  SOC_SAND_IN  uint32                 dp_ndx -
*     Drop precedence index. Range: 0 - ARAD_NOF_DP-1.
*  SOC_SAND_OUT ARAD_EGR_DROP_THRESH     *thresh -
*     drop thresholds to set.
* REMARKS:
*     The unscheduled traffic is assigned to the matching threshold
*     not according to the packet Drop Precedence, rather then
*     according to the Unscheduled Drop Priority value, as set
*     by the 'arad_egr_unsched_drop_prio_set' API, per TC and DP
*     The "dp_ndx" in this API refers to this Drop Priority, and not
*     directly to the packet Drop Precedence field
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_egr_unsched_drop_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_EGR_Q_PRIO          prio_ndx,
    SOC_SAND_IN  uint32                 dp_ndx,
    SOC_SAND_OUT ARAD_EGR_DROP_THRESH     *thresh
  );

/*********************************************************************
* NAME:
*     arad_egr_dev_fc_set
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Set Flow Control thresholds for egress queues, based on
*     device-level resources. Threshold are set for overall
*     resources, and scheduled resources.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_EGR_FC_DEVICE_THRESH *thresh -
*     Flow Control thresholds to set - Device-level resources.
*  SOC_SAND_OUT ARAD_EGR_FC_DEVICE_THRESH *exact_thresh -
*     Will be filled with exact values.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
int
  arad_egr_dev_fc_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_EGR_FC_DEVICE_THRESH *thresh,
    SOC_SAND_OUT ARAD_EGR_FC_DEVICE_THRESH *exact_thresh
  );

/*********************************************************************
* NAME:
*     arad_egr_dev_fc_get
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     Set Flow Control thresholds for egress queues, based on
*     device-level resources. Threshold are set for overall
*     resources, and scheduled resources.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_EGR_FC_DEVICE_THRESH *thresh -
*     Flow Control thresholds to set - Device-level resources.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
int
  arad_egr_dev_fc_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_OUT ARAD_EGR_FC_DEVICE_THRESH *thresh
  );

/*********************************************************************
* NAME:
*     arad_egr_ofp_fc_set
* TYPE:
*   PROC
* DATE:
*   Dec 20 2007
* FUNCTION:
*     Set Flow Control thresholds for egress queues, per port
*     queue priority and threshold type, based on Outgoing FAP
*     Port (OFP) resources.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_EGR_Q_PRIO          prio_ndx -
*     Queue priority index. Range: ARAD_EGR_Q_PRIO_LOW -
*     ARAD_EGR_Q_PRIO_HIGH.
*  SOC_SAND_IN  ARAD_EGR_PORT_THRESH_TYPE ofp_type_ndx -
*     Per-port threshold type index. Range: 0 -
*     ARAD_EGR_PORT_NOF_THRESH_TYPES-1.
*  SOC_SAND_IN  ARAD_EGR_FC_OFP_THRESH   *thresh -
*     Flow Control thresholds to set - FAP Port resources.
*  SOC_SAND_OUT ARAD_EGR_FC_OFP_THRESH   *exact_thresh -
*     Will be filled with exact values.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
int
  arad_egr_ofp_fc_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_EGR_Q_PRIO          prio_ndx,
    SOC_SAND_IN  ARAD_EGR_PORT_THRESH_TYPE ofp_type_ndx,
    SOC_SAND_IN  ARAD_EGR_FC_OFP_THRESH   *thresh,
    SOC_SAND_OUT ARAD_EGR_FC_OFP_THRESH   *exact_thresh
  );

/*********************************************************************
* NAME:
*     arad_egr_ofp_fc_get
* TYPE:
*   PROC
* DATE:
*   Dec 20 2007
* FUNCTION:
*     Set Flow Control thresholds for egress queues, per port
*     queue priority and threshold type, based on Outgoing FAP
*     Port (OFP) resources.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_EGR_Q_PRIO          prio_ndx -
*     Queue priority index. Range: ARAD_EGR_Q_PRIO_LOW -
*     ARAD_EGR_Q_PRIO_HIGH.
*  SOC_SAND_IN  ARAD_EGR_PORT_THRESH_TYPE ofp_type_ndx -
*     Per-port threshold type index. Range: 0 -
*     ARAD_EGR_PORT_NOF_THRESH_TYPES-1.
*  SOC_SAND_OUT ARAD_EGR_FC_OFP_THRESH   *thresh -
*     Flow Control thresholds to set - FAP Port resources.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
int
  arad_egr_ofp_fc_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_EGR_Q_PRIO          prio_ndx,
    SOC_SAND_IN  ARAD_EGR_PORT_THRESH_TYPE ofp_type_ndx,
    SOC_SAND_OUT ARAD_EGR_FC_OFP_THRESH   *thresh
  );

/*********************************************************************
* NAME:
*     arad_egr_ofp_scheduling_set
* TYPE:
*   PROC
* DATE:
*   Dec 20 2007
* FUNCTION:
*     Set per-port egress scheduling information.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID         ofp_ndx -
*     Outgoing Fap Port index to configure.
*  SOC_SAND_IN  ARAD_EGR_OFP_SCH_INFO    *info -
*     Per-port egress scheduling info
* REMARKS:
*   Some of the scheduling info may be irrelevant -
*   i.e. channelized interface priority is only relevant if the port
*   is mapped to a channelized NIF.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_egr_ofp_scheduling_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  uint32              tm_port,
    SOC_SAND_IN  ARAD_EGR_OFP_SCH_INFO    *info
  );

/*********************************************************************
* NAME:
*     arad_egr_ofp_scheduling_get
* TYPE:
*   PROC
* DATE:
*   Dec 20 2007
* FUNCTION:
*     Set per-port egress scheduling information.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID         ofp_ndx -
*     Outgoing Fap Port index to configure.
*  SOC_SAND_OUT ARAD_EGR_OFP_SCH_INFO    *info -
*     Per-port egress scheduling info
* REMARKS:
*   Some of the scheduling info may be irrelevant -
*   i.e. channelized interface priority is only relevant if the port
*   is mapped to a channelized NIF.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_egr_ofp_scheduling_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  uint32              tm_port,
    SOC_SAND_OUT ARAD_EGR_OFP_SCH_INFO    *info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_egr_dsp_pp_to_base_q_pair_set" API.
 *     Refer to "arad_egr_dsp_pp_to_base_q_pair_set" API for details.
*********************************************************************/
uint32
  arad_egr_dsp_pp_to_base_q_pair_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_FAP_PORT_ID              ofp_ndx,
    SOC_SAND_OUT uint32                      *base_q_pair
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_egr_dsp_pp_priorities_mode_set" API.
 *     Refer to "arad_egr_dsp_pp_priorities_mode_set" API for details.
*********************************************************************/
uint32
  arad_egr_dsp_pp_priorities_mode_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  ARAD_FAP_PORT_ID               ofp_ndx,
    SOC_SAND_OUT ARAD_EGR_PORT_PRIORITY_MODE    *priority_mode
  );

/*********************************************************************
* NAME:
 *   arad_egr_queuing_dev_set/get
 * TYPE:
 *   PROC
 *	Set the thresholds of the Multicast / Unicast service pools at device-level.
 * INPUT
 *	SOC_SAND_IN(SOC_SAND_OUT in get)
 *    ARAD_EGR_QUEUING_DEV_TH	*info -
 *		Service pool parameters
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
int
  arad_egr_queuing_dev_set(
	SOC_SAND_IN	 int	unit,
    SOC_SAND_IN  int    core,
	SOC_SAND_IN  ARAD_EGR_QUEUING_DEV_TH	*info
  );

int
  arad_egr_queuing_dev_get(
	SOC_SAND_IN	 int	unit,
    SOC_SAND_IN  int    core,
	SOC_SAND_OUT ARAD_EGR_QUEUING_DEV_TH	*info
  );

int
  arad_egr_queuing_global_drop_set(
      SOC_SAND_IN  int   unit,
      SOC_SAND_IN  int   core,
      SOC_SAND_IN  soc_dpp_cosq_threshold_type_t threshold_type,
      SOC_SAND_IN  int    threshold_value,
      SOC_SAND_IN  soc_dpp_cosq_threshold_global_type_t drop_type
  );

int
  arad_egr_queuing_global_drop_get(
      SOC_SAND_IN  int   unit,
      SOC_SAND_IN  int   core,
      SOC_SAND_IN  soc_dpp_cosq_threshold_type_t threshold_type,
      SOC_SAND_OUT int*    threshold_value,
      SOC_SAND_IN  soc_dpp_cosq_threshold_global_type_t drop_type
  );

int
  arad_egr_queuing_sp_tc_drop_set(
    SOC_SAND_IN    int    unit,
    SOC_SAND_IN    int    core,
    SOC_SAND_IN    int    tc,
    SOC_SAND_IN    soc_dpp_cosq_threshold_type_t threshold_type,
    SOC_SAND_IN    int    threshold_value,
    SOC_SAND_IN    soc_dpp_cosq_threshold_global_type_t drop_type
  );

int
  arad_egr_queuing_sp_tc_drop_get(
    SOC_SAND_IN    int    unit,
    SOC_SAND_IN    int    core,
    SOC_SAND_IN    int    tc,
    SOC_SAND_IN    soc_dpp_cosq_threshold_type_t threshold_type,
    SOC_SAND_OUT   int*   threshold_value,
    SOC_SAND_IN    soc_dpp_cosq_threshold_global_type_t drop_type
  );
int
  arad_egr_queuing_sp_reserved_set(
    SOC_SAND_IN    int    unit,
    SOC_SAND_IN    int    core,
    SOC_SAND_IN    int    tc,
    SOC_SAND_IN    soc_dpp_cosq_threshold_type_t threshold_type,
    SOC_SAND_IN    int    threshold_value,
    SOC_SAND_IN    soc_dpp_cosq_threshold_global_type_t drop_type
  );
int
  arad_egr_queuing_sp_reserved_get(
    SOC_SAND_IN    int    unit,
    SOC_SAND_IN    int    core,
    SOC_SAND_IN    int    tc,
    SOC_SAND_IN    soc_dpp_cosq_threshold_type_t threshold_type,
    SOC_SAND_OUT   int*    threshold_value,
    SOC_SAND_IN    soc_dpp_cosq_threshold_global_type_t drop_type
  );
uint32 
  arad_egr_queuing_sch_unsch_drop_set(
    SOC_SAND_IN    int    unit,
    SOC_SAND_IN    int    core,
    SOC_SAND_IN    int    threshold_type,
    SOC_SAND_IN    SOC_TMC_EGR_QUEUING_DEV_TH *dev_thresh
  );

int
  arad_egr_queuing_global_fc_set(
      SOC_SAND_IN  int   unit,
      SOC_SAND_IN  int   core, 
      SOC_SAND_IN  soc_dpp_cosq_threshold_type_t threshold_type,
      SOC_SAND_IN  int    threshold_value, 
      SOC_SAND_IN  soc_dpp_cosq_threshold_global_type_t drop_type
  );

int
  arad_egr_queuing_global_fc_get(
      SOC_SAND_IN  int   unit,
      SOC_SAND_IN  int   core, 
      SOC_SAND_IN  soc_dpp_cosq_threshold_type_t threshold_type,
      SOC_SAND_OUT int*    threshold_value, 
      SOC_SAND_IN  soc_dpp_cosq_threshold_global_type_t drop_type
  );

int
  arad_egr_queuing_mc_tc_fc_set(
    SOC_SAND_IN    int    unit,
    SOC_SAND_IN    int    core,
    SOC_SAND_IN    int    tc,
    SOC_SAND_IN    soc_dpp_cosq_threshold_type_t threshold_type,
    SOC_SAND_IN    int    threshold_value
  );

int
  arad_egr_queuing_mc_tc_fc_get(
    SOC_SAND_IN    int    unit,
    SOC_SAND_IN    int    core,
    SOC_SAND_IN    int    tc,
    SOC_SAND_IN    soc_dpp_cosq_threshold_type_t threshold_type,
    SOC_SAND_OUT   int*   threshold_value
  );

/*********************************************************************
* NAME:
 *   arad_egr_queuing_mc_cos_map_set/get
 * TYPE:
 *   PROC
 *	Set the COS mapping for egress multicast packets: TC-Group, Service Pool id and Service Pool eligibility.
 * INPUT
 *	SOC_SAND_IN	uint32	tc_ndx -
 *		Traffic Class. Range: 0 - 7.
 *	SOC_SAND_IN	uint32	dp_ndx -
 *		Drop Precedence. Range: 0 - 3.
 *	SOC_SAND_IN	(SOC_SAND_OUT in get) ARAD_EGR_QUEUING_MC_COS_MAP	*info -
 *		COS mapping parameters
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
int
  arad_egr_queuing_mc_cos_map_set(
	SOC_SAND_IN	int	unit,
    SOC_SAND_IN int core,
	SOC_SAND_IN	uint32	tc_ndx,
	SOC_SAND_IN	uint32	dp_ndx,
	SOC_SAND_IN ARAD_EGR_QUEUING_MC_COS_MAP	*info
  );

uint32	
  arad_egr_queuing_mc_cos_map_get(
	SOC_SAND_IN	 int	unit,
    SOC_SAND_IN  int    core,
	SOC_SAND_IN	 uint32	tc_ndx,
	SOC_SAND_IN	 uint32	dp_ndx,
	SOC_SAND_OUT ARAD_EGR_QUEUING_MC_COS_MAP	*info
  );

/*********************************************************************
* NAME:
 *   arad_egr_queuing_if_fc_set/get
 * TYPE:
 *   PROC
 *	Set the Interface Flow Control profiles for UC and MC.
 * INPUT
 *	SOC_SAND_IN	ARAD_INTERFACE_ID	if_ndx -
 *		Interface ID.
 *	SOC_SAND_IN	(SOC_SAND_OUT in get) ARAD_EGR_QUEUING_IF_FC	*info -
 *		Interface Flow Control profiles
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
int
  arad_egr_queuing_if_fc_set(
	SOC_SAND_IN	 int	unit,
	SOC_SAND_IN	 ARAD_INTERFACE_ID	if_ndx,
	SOC_SAND_IN  ARAD_EGR_QUEUING_IF_FC	*info
  );

uint32
  arad_egr_queuing_if_fc_get(
	SOC_SAND_IN	 int	unit,
	SOC_SAND_IN	 ARAD_INTERFACE_ID	if_ndx,
	SOC_SAND_OUT ARAD_EGR_QUEUING_IF_FC	*info
  );

/*********************************************************************
* NAME:
 *   arad_egr_queuing_if_fc_uc_set/get
 * TYPE:
 *   PROC
 *	Set the Interface Flow Control profile attributes for UC traffic.
 * INPUT
 *	SOC_SAND_IN	uint32	uc_if_profile_ndx-
 *		Unicast interface threshold profile.
 *	SOC_SAND_IN	(SOC_SAND_OUT for get) ARAD_EGR_QUEUING_IF_UC_FC	*info -
 *		Interface Flow Control profile attributes
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
int
  arad_egr_queuing_if_fc_uc_max_set(
	SOC_SAND_IN	int	unit,
    SOC_SAND_IN int core,
	SOC_SAND_IN	uint32	uc_if_profile_ndx,
	SOC_SAND_IN ARAD_EGR_QUEUING_IF_UC_FC	*info
  );

int
  arad_egr_queuing_if_fc_uc_set(
	SOC_SAND_IN	int	unit,
    SOC_SAND_IN int core,
	SOC_SAND_IN	uint32	uc_if_profile_ndx,
	SOC_SAND_IN ARAD_EGR_QUEUING_IF_UC_FC	*info
  );

int
  arad_egr_queuing_if_fc_uc_get(
	SOC_SAND_IN	 int	unit,
    SOC_SAND_IN  int    core,
	SOC_SAND_IN	 uint32	uc_if_profile_ndx,
	SOC_SAND_OUT ARAD_EGR_QUEUING_IF_UC_FC	*info
  );

/*********************************************************************
* NAME:
 *   arad_egr_queuing_if_fc_mc_set/get
 * TYPE:
 *   PROC
 *	Set the Interface Flow Control profile attributes for MC traffic.
 * INPUT
 *	SOC_SAND_IN	uint32	mc_if_profile_ndx-
 *		Unicast interface threshold profile.
 *	SOC_SAND_IN	(SOC_SAND_OUT for get) uint32	pd_th -
 * Total consumed Multicast PD per interface threshold.
 * Range: 0 - 0x7FFF.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

int
  arad_egr_queuing_if_fc_mc_set(
	SOC_SAND_IN	int	unit,
    SOC_SAND_IN int core,
	SOC_SAND_IN	uint32	mc_if_profile_ndx,
	SOC_SAND_IN uint32    pd_th
  );

int
  arad_egr_queuing_if_fc_mc_get(
	SOC_SAND_IN	 int	unit,
	SOC_SAND_IN	 uint32	mc_if_profile_ndx,
	SOC_SAND_OUT uint32   *pd_th
  );

uint32 
  arad_egr_queuing_if_uc_map_set(
    SOC_SAND_IN  int        unit,
    SOC_SAND_IN  int        core,
    SOC_SAND_IN  soc_port_if_t interface_type,
    SOC_SAND_IN  uint32     internal_if_id,
    SOC_SAND_IN  int        profile
  );

uint32 
  arad_egr_queuing_if_mc_map_set(
    SOC_SAND_IN  int        unit,
    SOC_SAND_IN  int        core,
    SOC_SAND_IN  soc_port_if_t interface_type,
    SOC_SAND_IN  uint32     internal_if_id,
    SOC_SAND_IN  int        profile
  );

/*********************************************************************
* NAME:
 *   arad_egr_queuing_ofp_tcg_set/get
 * TYPE:
 * PROC
 *	Associate the queue-pair (Port,Priority) to traffic class
 *    groups (TCG) attributes.
 * Input
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_FAP_PORT_ID              ofp_ndx -
 *     Outgoing Fap Port index. Range: 0 - 255.
 *   SOC_SAND_IN  uint32	                    tcg_info -
 *     TCG attributes to be filled
 * REMARKS:
 *   The last four TCG are single-member groups. Scheduling within
 *   a TCG is done in a strict priority manner according to the
 *   priority level. (e.g. If P1,P2,P7 within the same TCG1.
 *   Then SP select is descending priority P7,P2,P1).
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_egr_queuing_ofp_tcg_set(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  int                            core,
    SOC_SAND_IN  uint32                         tm_port,    
    SOC_SAND_IN  ARAD_EGR_QUEUING_TCG_INFO      *tcg_info
  );

uint32
  arad_egr_queuing_ofp_tcg_get(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  int                            core,
    SOC_SAND_IN  ARAD_FAP_PORT_ID               tm_port,    
    SOC_SAND_OUT ARAD_EGR_QUEUING_TCG_INFO      *tcg_info
  );

/*********************************************************************
* NAME:
*     arad_sch_port_tcg_weight_set
* TYPE:
*   PROC
* DATE:
*  
* FUNCTION:
*     Sets, for a specified TCG within a certain OFP
*     its excess rate. Excess traffic is scheduled between other TCGs
*     according to a weighted fair queueing or strict priority policy.
*     Set invalid, in case TCG not take part of this policy.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  ARAD_FAP_PORT_ID          ofp_ndx -
*     Port id, 0 - 255. 
*  SOC_SAND_IN  ARAD_TCG_NDX              tcg_ndx -
*     TCG index. 0-7.
*  SOC_SAND_IN  ARAD_EGR_TCG_SCH_WFQ      *tcg_weight -
*     TCG weight information.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   This function must only be called for eight priorities port.
*********************************************************************/
uint32
  arad_egr_queuing_tcg_weight_set(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  int                       core,
    SOC_SAND_IN  ARAD_FAP_PORT_ID          tm_port,
    SOC_SAND_IN  ARAD_TCG_NDX              tcg_ndx,
    SOC_SAND_IN  ARAD_EGR_TCG_SCH_WFQ      *tcg_weight
  );

uint32
  arad_egr_queuing_tcg_weight_get(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  int                       core,
    SOC_SAND_IN  uint32                    tm_port,
    SOC_SAND_IN  ARAD_TCG_NDX              tcg_ndx,
    SOC_SAND_OUT ARAD_EGR_TCG_SCH_WFQ      *tcg_weight
  );


void
  arad_ARAD_EGR_Q_PRIORITY_clear(
    SOC_SAND_OUT ARAD_EGR_Q_PRIORITY *info
  );

void
  arad_ARAD_EGR_DROP_THRESH_clear(
    SOC_SAND_OUT ARAD_EGR_DROP_THRESH *info
  );

void
  arad_ARAD_EGR_THRESH_INFO_clear(
    SOC_SAND_OUT ARAD_EGR_THRESH_INFO *info
  );

void
  arad_ARAD_EGR_QUEUING_TH_DB_GLOBAL_clear(
    SOC_SAND_OUT ARAD_EGR_QUEUING_TH_DB_GLOBAL *info
  );

void
  arad_ARAD_EGR_QUEUING_TH_DB_POOL_clear(
    SOC_SAND_OUT ARAD_EGR_QUEUING_TH_DB_POOL *info
  );

void
  arad_ARAD_EGR_QUEUING_TH_DB_PORT_clear(
    SOC_SAND_OUT ARAD_EGR_QUEUING_TH_DB_PORT *info
  );

void
  arad_ARAD_EGR_QUEUING_DEV_TH_clear(
    SOC_SAND_OUT ARAD_EGR_QUEUING_DEV_TH *info
  );

void
  arad_ARAD_EGR_QUEUING_MC_COS_MAP_clear(
    SOC_SAND_OUT ARAD_EGR_QUEUING_MC_COS_MAP *info
  );

void
  arad_ARAD_EGR_QUEUING_IF_FC_clear(
    SOC_SAND_OUT ARAD_EGR_QUEUING_IF_FC *info
  );

void
  arad_ARAD_EGR_QUEUING_IF_UC_FC_clear(
    SOC_SAND_OUT ARAD_EGR_QUEUING_IF_UC_FC *info
  );

void
  arad_ARAD_EGR_FC_DEV_THRESH_INNER_clear(
    SOC_SAND_OUT ARAD_EGR_FC_DEV_THRESH_INNER *info
  );

void
  arad_ARAD_EGR_FC_DEVICE_THRESH_clear(
    SOC_SAND_OUT ARAD_EGR_FC_DEVICE_THRESH *info
  );

void
  arad_ARAD_EGR_FC_CHNIF_THRESH_clear(
    SOC_SAND_OUT ARAD_EGR_FC_CHNIF_THRESH *info
  );

void
  arad_ARAD_EGR_FC_OFP_THRESH_clear(
    SOC_SAND_OUT ARAD_EGR_FC_OFP_THRESH *info
  );

void
  arad_ARAD_EGR_OFP_SCH_WFQ_clear(
    SOC_SAND_OUT ARAD_EGR_OFP_SCH_WFQ *info
  );

void
  arad_ARAD_EGR_OFP_SCH_INFO_clear(
    SOC_SAND_OUT ARAD_EGR_OFP_SCH_INFO *info
  );

void
  ARAD_EGR_QUEUING_TCG_INFO_clear(
    SOC_SAND_OUT ARAD_EGR_QUEUING_TCG_INFO *info
  );

void
  ARAD_EGR_TCG_SCH_WFQ_clear(
    SOC_SAND_OUT ARAD_EGR_TCG_SCH_WFQ *info
  );


#if ARAD_DEBUG_IS_LVL1

void
  ARAD_EGR_Q_PRIORITY_print(
    SOC_SAND_IN  ARAD_EGR_Q_PRIORITY *info
  );

const char*
  arad_ARAD_EGR_Q_PRIO_to_string(
    SOC_SAND_IN ARAD_EGR_Q_PRIO enum_val
  );



const char*
  arad_ARAD_EGR_PORT_THRESH_TYPE_to_string(
    SOC_SAND_IN ARAD_EGR_PORT_THRESH_TYPE enum_val
  );



const char*
  arad_ARAD_EGR_OFP_INTERFACE_PRIO_to_string(
    SOC_SAND_IN ARAD_EGR_OFP_INTERFACE_PRIO enum_val
  );



const char*
  arad_ARAD_EGR_Q_PRIO_MAPPING_TYPE_to_string(
    SOC_SAND_IN ARAD_EGR_Q_PRIO_MAPPING_TYPE enum_val
  );



void
  arad_ARAD_EGR_DROP_THRESH_print(
    SOC_SAND_IN ARAD_EGR_DROP_THRESH *info
  );

void
  arad_ARAD_EGR_THRESH_INFO_print(
    SOC_SAND_IN ARAD_EGR_THRESH_INFO *info
  );

void
  arad_ARAD_EGR_FC_DEV_THRESH_INNER_print(
    SOC_SAND_IN ARAD_EGR_FC_DEV_THRESH_INNER *info
  );



void
  arad_ARAD_EGR_FC_DEVICE_THRESH_print(
    SOC_SAND_IN ARAD_EGR_FC_DEVICE_THRESH *info
  );


void
  arad_ARAD_EGR_FC_CHNIF_THRESH_print(
    SOC_SAND_IN ARAD_EGR_FC_CHNIF_THRESH *info
  );



void
  arad_ARAD_EGR_FC_OFP_THRESH_print(
    SOC_SAND_IN ARAD_EGR_FC_OFP_THRESH *info
  );



void
  arad_ARAD_EGR_OFP_SCH_WFQ_print(
    SOC_SAND_IN ARAD_EGR_OFP_SCH_WFQ *info
  );



void
  arad_ARAD_EGR_OFP_SCH_INFO_print(
    SOC_SAND_IN ARAD_EGR_OFP_SCH_INFO *info
  );

void
  ARAD_EGR_QUEUING_TCG_INFO_print(
    SOC_SAND_IN ARAD_EGR_QUEUING_TCG_INFO *info
  );

void
  ARAD_EGR_TCG_SCH_WFQ_print(
    SOC_SAND_IN ARAD_EGR_TCG_SCH_WFQ *info
  );

#endif /* ARAD_DEBUG_IS_LVL1 */

/* } */


#include <soc/dpp/SAND/Utils/sand_footer.h>


/* } __ARAD_API_EGR_QUEUING_INCLUDED__*/
#endif

