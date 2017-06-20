/* $Id: jer2_qax_fabric.h,v 1.30 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER2_QAX_FABRIC_INCLUDED__
/* { */
#define __JER2_QAX_FABRIC_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/cosq.h>
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/error.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */


/*Index to TXQ_PRIORITY_BITS_MAPPING_2_FDT is built: hp:1, mc:1, dp:2, trfcls:3, i.e. bhmddttt*/
#define SOC_JER2_QAX_FABRIC_PRIORITY_NDX_NOF             (128)
#define SOC_JER2_QAX_FABRIC_PRIORITY_NDX_TC_MASK         (0x7)
#define SOC_JER2_QAX_FABRIC_PRIORITY_NDX_TC_OFFSET       (0)
#define SOC_JER2_QAX_FABRIC_PRIORITY_NDX_DP_MASK         (0x18)
#define SOC_JER2_QAX_FABRIC_PRIORITY_NDX_DP_OFFSET       (3)
#define SOC_JER2_QAX_FABRIC_PRIORITY_NDX_IS_MC_MASK      (0x20)
#define SOC_JER2_QAX_FABRIC_PRIORITY_NDX_IS_MC_OFFSET    (5)
#define SOC_JER2_QAX_FABRIC_PRIORITY_NDX_IS_HP_MASK      (0x40)
#define SOC_JER2_QAX_FABRIC_PRIORITY_NDX_IS_HP_OFFSET    (6)


/* } */

/*************
 * MACROS    *
 *************/
/* { */

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
*     soc_jer2_qax_fabric_multicast_set
* FUNCTION:
*     Setting destination for a specific multicast id in kalia
* INPUT:
*       int             unit            - Identifier of the device to access.
*       soc_multicast_t mc_id           - multicast id
*       uint32          destid_count    - number of destination for this mc_id
*       soc_module_t    *destid_array   - specific destination for replication for this specific mc_id
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Used in mbcm dispatcher.
*   For Kalia only (not relevant for JER2_QAX)
*********************************************************************/

soc_error_t
soc_jer2_qax_fabric_multicast_set(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  soc_multicast_t                mc_id,
    DNX_SAND_IN  uint32                         destid_count,
    DNX_SAND_IN  soc_module_t                   *destid_array
  );

/*********************************************************************
* NAME:
*     soc_jer2_qax_fabric_force_set
* FUNCTION:
*     DIAG function. Force fabric interface for local / fabric or restore back to operational mode
* INPUT:
*       int   unit - Identifier of the device to access.
*       soc_dnx_fabric_force_t force - enum for requested force mode (local/fabric/restore)
* RETURNS:
*       OK or ERROR indication.
* REMARKS:
*       Relevant for Kalia only. Not supported in jer2_qax.
*       Used in mbcm dispatcher.
*********************************************************************/
soc_error_t
  soc_jer2_qax_fabric_force_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN soc_dnx_fabric_force_t        force
  );

/*********************************************************************
* NAME:
*     soc_jer2_qax_fabric_link_config_ovrd
* FUNCTION:
*     Overwriting jer2_qax default fabric configuration in case of kalia
* INPUT:
*       int   unit - Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
* REMARKS:
*   Used in mbcm dispatcher.
*   For Kalia only (not relevant for JER2_QAX)
*********************************************************************/

soc_error_t
soc_jer2_qax_fabric_link_config_ovrd(
  int                unit
);




/*********************************************************************
* NAME:
*     soc_jer2_qax_fabric_cosq_control_backward_flow_control_set / get
* TYPE:
*   PROC
* DATE:
*   Dec 03 2015
* FUNCTION:
*     Enable / disable backwards flow control on supported fifos
* INPUT:
*  DNX_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN soc_gport_t                          port -
*     gport number.
*  DNX_SAND_IN int                                   enable -
*     Whether to enable / disable the feature.
*  DNX_SAND_IN int                                  fifo_type -
*     Type of fifo to configure
*
* REMARKS:
*   Used in mbcm dispatcher.
*   For Kalia only (not relevant for JER2_QAX)
*********************************************************************/
soc_error_t
  soc_jer2_qax_fabric_cosq_control_backward_flow_control_set(
      DNX_SAND_IN int                                   unit,
      DNX_SAND_IN soc_gport_t                           port,
      DNX_SAND_IN int                                   enable,
      DNX_SAND_IN soc_dnx_cosq_gport_egress_core_fifo_t fifo_type
  );

soc_error_t
  soc_jer2_qax_fabric_cosq_control_backward_flow_control_get(
      DNX_SAND_IN int                                   unit,
      DNX_SAND_IN soc_gport_t                           port,
      DNX_SAND_OUT int                                  *enable,
      DNX_SAND_IN soc_dnx_cosq_gport_egress_core_fifo_t fifo_type
  );


/*********************************************************************
* NAME:
*     soc_jer2_qax_fabric_egress_core_cosq_gport_sched_set / get
* TYPE:
*   PROC
* DATE:
*   Dec 03 2015
* FUNCTION:
*     Set WFQ weight on supported fifos.
* INPUT:
*  DNX_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN soc_gport_t                          pipe -
*     Which pipe to access.
*  DNX_SAND_IN int                                  weight -
*     Weight value to configure.
*  DNX_SAND_IN int                                  fifo_type -
*     Type of fifo to configure
*
* REMARKS:
*   Used in mbcm dispatcher.
*   For Kalia only (not relevant for JER2_QAX)
*********************************************************************/

soc_error_t
  soc_jer2_qax_fabric_egress_core_cosq_gport_sched_set(
    DNX_SAND_IN  int                                unit,
    DNX_SAND_IN  int                                pipe,
    DNX_SAND_IN  int                                weight,
    DNX_SAND_IN  soc_dnx_cosq_gport_egress_core_fifo_t   fifo_type);

soc_error_t
  soc_jer2_qax_fabric_egress_core_cosq_gport_sched_get(
    DNX_SAND_IN  int                                unit,
    DNX_SAND_IN  int                                pipe,
    DNX_SAND_OUT int                                *weight,
    DNX_SAND_IN  soc_dnx_cosq_gport_egress_core_fifo_t   fifo_type
  );

/*********************************************************************
* NAME:
*     soc_jer2_qax_cosq_gport_sched_set/get
* FUNCTION:
*     Configuration of weight for WFQs in fabric pipes:
*     all, ingress, egress.
* INPUT:
*  DNX_SAND_IN  int                                unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  int                                pipe -
*     Which pipe's weight to configure (0,1,2)
*  DNX_SAND_IN/DNX_SAND_OUT  int/int*              weight -
*     value to configure/retrieve pipe's weight
*  DNX_SAND_IN  soc_dnx_cosq_gport_fabric_pipe_t   fabric_pipe_type -
*     type of fabric pipe to configure (all, ingress, egress)
*     Note: egress is not legal argument for JER2_QAX. "All" argument is actually identical to "ingress" argument
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
* REMARKS:
*   Used in mbcm dispatcher.
*   For Kalia only (not relevant for JER2_QAX)
*********************************************************************/
soc_error_t
  soc_jer2_qax_cosq_gport_sched_set(
    DNX_SAND_IN  int                                unit,
    DNX_SAND_IN  int                                pipe,
    DNX_SAND_IN  int                                weight,
    DNX_SAND_IN  soc_dnx_cosq_gport_fabric_pipe_t   fabric_pipe_type
  );

soc_error_t
  soc_jer2_qax_cosq_gport_sched_get(
    DNX_SAND_IN  int                                unit,
    DNX_SAND_IN  int                                pipe,
    DNX_SAND_OUT  int*                               weight,
    DNX_SAND_IN  soc_dnx_cosq_gport_fabric_pipe_t   fabric_pipe_type
  );

/*********************************************************************
* NAME:
*     soc_jer2_qax_fabric_cosq_gport_priority_drop_threshold_set / get
* TYPE:
*   PROC
* DATE:
*   Dec 10 2015
* FUNCTION:
*     Set priority drop threshold on supported fifos.
* INPUT:
*  DNX_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN soc_gport_t                          gport -
*     gport number.
*  DNX_SAND_IN  dnx_soc_cosq_threshold_t                *threshold_val -
*     sturuct which contains the threshold value
*     to configure / retreive.
*  DNX_SAND_IN int                                  fifo_type -
*     Type of fifo to configure
*     NOTE: Only soc_dnx_cosq_gport_egress_core_fifo_local_mcast fifo_type is supported in JER2_QAX!
*
* REMARKS:
*   Used in mbcm dispatcher.
*   For Kalia only (not relevant for JER2_QAX)
*********************************************************************/
soc_error_t
  soc_jer2_qax_fabric_cosq_gport_priority_drop_threshold_set(
    DNX_SAND_IN  int                                    unit,
    DNX_SAND_IN  soc_gport_t                            gport,
    DNX_SAND_IN  dnx_soc_cosq_threshold_t                   *threshold,
    DNX_SAND_IN  soc_dnx_cosq_gport_egress_core_fifo_t  fifo_type
  );

soc_error_t
  soc_jer2_qax_fabric_cosq_gport_priority_drop_threshold_get(
    DNX_SAND_IN  int                                    unit,
    DNX_SAND_IN  soc_gport_t                            gport,
    DNX_SAND_INOUT  dnx_soc_cosq_threshold_t                *threshold,
    DNX_SAND_IN  soc_dnx_cosq_gport_egress_core_fifo_t  fifo_type
  );


/*********************************************************************
* NAME:
*     soc_jer2_qax_fabric_cosq_gport_rci_threshold_set / get
* TYPE:
*   PROC
* DATE:
*   Dec 10 2015
* FUNCTION:
*     Set rci threshold on supported fifos.
* INPUT:
*  DNX_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN soc_gport_t                          gport -
*     gport number.
*  DNX_SAND_IN  int                                 threshold_val -
*     Threshold value to configure.
*  DNX_SAND_IN int                                  fifo_type -
*     Type of fifo to configure
*     Note: For JER2_QAX, only soc_dnx_cosq_gport_egress_core_fifo_local_ucast is supported.
*
* REMARKS:
*     Used in mbcm dispatcher.
*     For Kalia only (not relevant for JER2_QAX)
*********************************************************************/

soc_error_t
  soc_jer2_qax_fabric_cosq_gport_rci_threshold_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  soc_gport_t            gport,
    DNX_SAND_IN  int                    threshold_val,
    DNX_SAND_IN  soc_dnx_cosq_gport_egress_core_fifo_t  fifo_type
  );

soc_error_t
  soc_jer2_qax_fabric_cosq_gport_rci_threshold_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  soc_gport_t            gport,
    DNX_SAND_OUT  int                    *threshold_val,
    DNX_SAND_IN  soc_dnx_cosq_gport_egress_core_fifo_t  fifo_type
  );


/*********************************************************************
* NAME:
*     soc_jer2_qax_fabric_priority_set / set
* TYPE:
*   PROC
* DATE:
*   Dec 13 2015
* FUNCTION:
*     Set / Get fabric priority according to:
*     traffic_class, queue_type: hc/lc (flags), dp(color).
* INPUT:
*  DNX_SAND_IN  int                                unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint32                             tc -
*     traffic class
*  DNX_SAND_IN  uint32                             dp -
*     drop precedence
*  DNX_SAND_IN  uint32                             flags -
*     relevant flags for cell (is_mc, is_hp)
*  DNX_SAND_IN/OUT   int/int*                      fabric_priority -
*     fabric priority to set/ get in TXQ_PRIORITY_BITS_MAPPING_2_FDT
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Used in mbcm dispatcher.
*   For Kalia only (not relevant for JER2_QAX)
*********************************************************************/

soc_error_t
soc_jer2_qax_fabric_priority_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  uint32             tc,
    DNX_SAND_IN  uint32             dp,
    DNX_SAND_IN  uint32             flags,
    DNX_SAND_IN  int                fabric_priority
  );

soc_error_t
soc_jer2_qax_fabric_priority_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  uint32             tc,
    DNX_SAND_IN  uint32             dp,
    DNX_SAND_IN  uint32             flags,
    DNX_SAND_OUT  int                *fabric_priority
  );

/*********************************************************************
* NAME:
*     soc_jer2_qax_fabric_queues_info_get
* TYPE:
*   PROC
* DATE:
*   Jun 8 2016
* FUNCTION:
*     Get DTQ and PDQ (DQCQ/DBLF) queues status
* INPUT:
*  DNX_SAND_IN  int                         unit -
*     Identifier of the device to access.
*  DNX_SAND_OUT soc_dnx_fabric_queues_info_t*
*                                           queues_info-
*     struct to hold the status.
* REMARKS:
*   Used in mbcm dispatcher.
*********************************************************************/
uint32
  soc_jer2_qax_fabric_queues_info_get(
    DNX_SAND_IN  int                             unit,
    DNX_SAND_OUT soc_dnx_fabric_queues_info_t    *queues_info
  );


/*********************************************************************
* NAME:
*     jer2_qax_fabric_pcp_dest_mode_config_set
* TYPE:
*   PROC
* DATE:
*   Dec 16 2015
* FUNCTION:
*     Enables set / get operations on fabric-pcp (packet cell packing)
*     per destination device.
*     there are three supported pcp modes:
*       - 0- No Packing
*       - 1- Simple Packing
*       - 2- Continuous Packing
* INPUT:
*  DNX_SAND_IN  int                                     unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint32                                  flags-
*  DNX_SAND_IN  uint32                                  modid-
*     Id of destination device
*  DNX_SAND_IN/OUT uint32*     							pcp_mode-
*     mode of pcp to set/get.
* REMARKS:
*   Used in mbcm dispatcher.
*   For Kalia only (not relevant for JER2_QAX)
*********************************************************************/
soc_error_t
  jer2_qax_fabric_pcp_dest_mode_config_set(
    DNX_SAND_IN int              unit,
    DNX_SAND_IN uint32           flags,
    DNX_SAND_IN uint32           modid,
    DNX_SAND_IN uint32           pcp_mode
  );


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __JER2_QAX_FABRIC_INCLUDED__*/
#endif
