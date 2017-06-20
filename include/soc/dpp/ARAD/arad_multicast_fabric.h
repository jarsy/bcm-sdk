/* $Id: arad_multicast_fabric.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __ARAD_MULTICAST_FABRIC_INCLUDED__
/* { */
#define __ARAD_MULTICAST_FABRIC_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>


#include <soc/dpp/ARAD/arad_api_multicast_fabric.h>
#include <soc/dpp/ARAD/arad_chip_regs.h>

#include <soc/dpp/SAND/Utils/sand_framework.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

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

/*********************************************************************
* NAME:
*     arad_multicast_fabric_init
* FUNCTION:
*     Initialization of the Arad blocks configured in this module.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  arad_multicast_fabric_init(
    SOC_SAND_IN  int                 unit
  );

/*********************************************************************
* NAME:
*     arad_mult_fabric_traffic_class_to_multicast_cls_map_verify
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Maps the embedded traffic class in the packet header to
*     a multicast class (0..3). This multicast class will be
*     further used for egress/fabric replication.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  ARAD_TR_CLS         tr_cls_ndx -
*     tr_cls_ndx-the traffic class index, for which to map a
*     new class. SOC_SAND_IN
*  SOC_SAND_IN  ARAD_MULT_FABRIC_CLS     new_mult_cls -
*     new_mult_cls - The new multicast class that is mapped to
*     the tr_cls_ndx.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_mult_fabric_traffic_class_to_multicast_cls_map_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_TR_CLS              tr_cls_ndx,
    SOC_SAND_IN  ARAD_MULT_FABRIC_CLS     new_mult_cls
  );

/*********************************************************************
* NAME:
*     arad_mult_fabric_traffic_class_to_multicast_cls_map_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Maps the embedded traffic class in the packet header to
*   a multicast class (0..3). This multicast class will be
*   further used for egress/fabric replication.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  ARAD_TR_CLS                              tr_cls_ndx -
*     the traffic class index, for which to map a new class.
*     Range: 0 - 7.
*   SOC_SAND_IN  ARAD_MULT_FABRIC_CLS                     new_mult_cls -
*     The new multicast class that is mapped to the
*     tr_cls_ndx. Range: 0 - 3.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_mult_fabric_traffic_class_to_multicast_cls_map_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_TR_CLS         tr_cls_ndx,
    SOC_SAND_IN  ARAD_MULT_FABRIC_CLS     new_mult_cls
  );

/*********************************************************************
* NAME:
*     arad_mult_fabric_traffic_class_to_multicast_cls_map_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Maps the embedded traffic class in the packet header to
*   a multicast class (0..3). This multicast class will be
*   further used for egress/fabric replication.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  ARAD_TR_CLS                              tr_cls_ndx -
*     the traffic class index, for which to map a new class.
*     Range: 0 - 7.
*   SOC_SAND_OUT ARAD_MULT_FABRIC_CLS                     *new_mult_cls -
*     The new multicast class that is mapped to the
*     tr_cls_ndx. Range: 0 - 3.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_mult_fabric_traffic_class_to_multicast_cls_map_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_TR_CLS         tr_cls_ndx,
    SOC_SAND_OUT ARAD_MULT_FABRIC_CLS     *new_mult_cls
  );

/*********************************************************************
* NAME:
*     arad_mult_fabric_base_queue_verify
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configures the base queue of the
*     multicast egress/fabric.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  uint32                  queue_id -
*     the base queue of fabric or egress multicast packets.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_mult_fabric_base_queue_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                  queue_id
  );

/*********************************************************************
* NAME:
*     arad_mult_fabric_base_queue_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   This procedure configures the base queue of the
*   multicast egress/fabric.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                                  queue_id -
*     the base queue of fabric or egress multicast packets.
*     Range: 0 - 32K-1.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_mult_fabric_base_queue_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                  queue_id
  );

/*********************************************************************
* NAME:
*     arad_mult_fabric_base_queue_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   This procedure configures the base queue of the
*   multicast egress/fabric.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_OUT uint32                                  *queue_id -
*     the base queue of fabric or egress multicast packets.
*     Range: 0 - 32K-1.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_mult_fabric_base_queue_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT uint32                  *queue_id
  );

/*********************************************************************
* NAME:
*     arad_mult_fabric_credit_source_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Set the Fabric Multicast credit generator configuration
*   for the Default Fabric Multicast Queue configuration.
*   The fabric multicast queues are 0 - 3, and the credits
*   comes either directly to these queues or according to a
*   scheduler scheme.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  ARAD_MULT_FABRIC_INFO                    *info -
*     Credit source configuration.
*   SOC_SAND_OUT ARAD_MULT_FABRIC_INFO                    *exact_info -
*     Exact credit source configuration as written to the
*     device.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
int
  arad_mult_fabric_credit_source_set(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  int                        core,
    SOC_SAND_IN  ARAD_MULT_FABRIC_INFO      *info
  );

/*********************************************************************
* NAME:
*     arad_mult_fabric_credit_source_get
* TYPE:
*   PROC
* FUNCTION:
*   Set the Fabric Multicast credit generator configuration
*   for the Default Fabric Multicast Queue configuration.
*   The fabric multicast queues are 0 - 3, and the credits
*   comes either directly to these queues or according to a
*   scheduler scheme.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_OUT ARAD_MULT_FABRIC_INFO                    *info -
*     Credit source configuration.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
int
  arad_mult_fabric_credit_source_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  int                      core,
    SOC_SAND_OUT ARAD_MULT_FABRIC_INFO    *info
  );

/*********************************************************************
* NAME:
*   arad_mult_fabric_enhanced_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Configure the Enhanced Fabric Multicast Queue
*   configuration: the fabric multicast queues are defined
*   in a configured range, and the credits are coming to
*   these queues according to a scheduler scheme.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_U32_RANGE                            *queue_range -
*     Range of the FMQs where the fabric multicast packets are
*     sent to. Range: 0 - 32K-1.
* REMARKS:
*   If the Enhanced Fabric Multicast Queue configuration is
*   set, the credits must come to the FMQs via the
*   scheduler. Besides, scheduler schemes must be set for
*   each of the virtual OFP ports which will receive credits
*   for the multicast packets with the corresponding fabric
*   multicast class. The Enhanced Fabric Multicast Queue
*   configuration can be disabled by configuring the
*   'queue_range' back to 0 - 3.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_mult_fabric_enhanced_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_U32_RANGE                            *queue_range
  );

/*********************************************************************
* NAME:
*   arad_mult_fabric_enhanced_set_verify
* TYPE:
*   PROC
* FUNCTION:
*   Configure the Enhanced Fabric Multicast Queue
*   configuration: the fabric multicast queues are defined
*   in a configured range, and the credits are coming to
*   these queues according to a scheduler scheme.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_U32_RANGE                            *queue_range -
*     Range of the FMQs where the fabric multicast packets are
*     sent to. Range: 0 - 32K-1.
* REMARKS:
*   If the Enhanced Fabric Multicast Queue configuration is
*   set, the credits must come to the FMQs via the
*   scheduler. Besides, scheduler schemes must be set for
*   each of the virtual OFP ports which will receive credits
*   for the multicast packets with the corresponding fabric
*   multicast class. The Enhanced Fabric Multicast Queue
*   configuration can be disabled by configuring the
*   'queue_range' back to 0 - 3.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_mult_fabric_enhanced_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_U32_RANGE                            *queue_range
  );

/*********************************************************************
* NAME:
*   arad_mult_fabric_enhanced_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Configure the Enhanced Fabric Multicast Queue
*   configuration: the fabric multicast queues are defined
*   in a configured range, and the credits are coming to
*   these queues according to a scheduler scheme.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_OUT SOC_SAND_U32_RANGE                            *queue_range -
*     Range of the FMQs where the fabric multicast packets are
*     sent to. Range: 0 - 32K-1.
* REMARKS:
*   If the Enhanced Fabric Multicast Queue configuration is
*   set, the credits must come to the FMQs via the
*   scheduler. Besides, scheduler schemes must be set for
*   each of the virtual OFP ports which will receive credits
*   for the multicast packets with the corresponding fabric
*   multicast class. The Enhanced Fabric Multicast Queue
*   configuration can be disabled by configuring the
*   'queue_range' back to 0 - 3.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_mult_fabric_enhanced_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_SAND_U32_RANGE                            *queue_range
  );

/*********************************************************************
* NAME:
*     arad_mult_fabric_active_links_verify
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure sets the FAP links that are eligible for
*     multicast fabric traffic.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  ARAD_MULT_FABRIC_ACTIVE_LINKS *links -
*     A bitmap of the device links. if bit (0 -
*     ARAD_FBR_NOF_LINKS) is up, then its corresponding link (0 -
*     ARAD_FBR_NOF_LINKS) is eligible for spatial multicast
*     distribution. SOC_SAND_IN
*  SOC_SAND_IN  uint8                 tbl_refresh_enable -
*     tbl_refresh_enable - Since multicast distribution table
*     does not exist, the data is taken from link map the user
*     supplies (active_mc_links), and may be combined with
*     data from unicast distribution table. This parameter
*     sets the refresh rate in which, is case of the combined
*     data as explained above, the data is taken from the
*     unicast distribution table. TRUE - Combine calculated
*     data from UC distribution table with active_mc_links.
*     FALSE - Do not combine calculated data from UC
*     distribution table with active_mc_links.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_mult_fabric_active_links_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_MULT_FABRIC_ACTIVE_LINKS *links,
    SOC_SAND_IN  uint8                 tbl_refresh_enable
  );

/*********************************************************************
* NAME:
*   arad_mult_fabric_active_links_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   This procedure sets the FAP links that are eligible for
*   multicast fabric traffic.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  ARAD_MULT_FABRIC_ACTIVE_LINKS            *links -
*     A bitmap of the device links. if bit (0 -
*     ARAD_NOF_LINKS) is up, then its corresponding link (0 -
*     ARAD_NOF_LINKS) is eligible for spatial multicast
*     distribution.
*   SOC_SAND_IN  uint8                                 tbl_refresh_enable -
*     Since multicast distribution table does not exist, the
*     data is taken from link map the user supplies (links),
*     and may be combined with data from unicast distribution
*     table. This parameter sets the refresh rate in which, is
*     case of the combined data as explained above, the data
*     is taken from the Unicast distribution table. TRUE -
*     Combine calculated data from UC distribution table with
*     active_mc_links. FALSE - Do not combine calculated data
*     from UC distribution table with active_mc_links.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   This is a manual per-link configuration.
*   Per-link configuration is also possible.
*********************************************************************/
uint32
  arad_mult_fabric_active_links_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_MULT_FABRIC_ACTIVE_LINKS *links,
    SOC_SAND_IN  uint8                 tbl_refresh_enable
  );

/*********************************************************************
* NAME:
*     arad_mult_fabric_active_links_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   This procedure sets the FAP links that are eligible for
*   multicast fabric traffic.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_OUT ARAD_MULT_FABRIC_ACTIVE_LINKS            *links -
*     A bitmap of the device links. if bit (0 -
*     ARAD_NOF_LINKS) is up, then its corresponding link (0 -
*     ARAD_NOF_LINKS) is eligible for spatial multicast
*     distribution.
*   SOC_SAND_OUT uint8                                 *tbl_refresh_enable -
*     Since multicast distribution table does not exist, the
*     data is taken from link map the user supplies (links),
*     and may be combined with data from unicast distribution
*     table. This parameter sets the refresh rate in which, is
*     case of the combined data as explained above, the data
*     is taken from the Unicast distribution table. TRUE -
*     Combine calculated data from UC distribution table with
*     active_mc_links. FALSE - Do not combine calculated data
*     from UC distribution table with active_mc_links.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_mult_fabric_active_links_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_MULT_FABRIC_ACTIVE_LINKS *links,
    SOC_SAND_OUT uint8                 *tbl_refresh_enable
  );


/*********************************************************************
* NAME:
*     arad_mult_fabric_active_links_set\get unsfae\verify
* TYPE:
*   PROC
* FUNCTION:
*   This procedure sets the FMQ with GCI LB level
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN ARAD_MULT_FABRIC_FLOW_CONTROL_MAP       fc_map (set) - 
*   SOC_SAND_OUT ARAD_MULT_FABRIC_FLOW_CONTROL_MAP      fc_map (get) - 
*     See in struct description.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_mult_fabric_flow_control_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  ARAD_MULT_FABRIC_FLOW_CONTROL_MAP      *fc_map
  );

uint32
  arad_mult_fabric_flow_control_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  ARAD_MULT_FABRIC_FLOW_CONTROL_MAP      *fc_map
  );

uint32
  arad_mult_fabric_flow_control_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT ARAD_MULT_FABRIC_FLOW_CONTROL_MAP      *fc_map
  );

uint32
  arad_mult_fabric_flow_control_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT ARAD_MULT_FABRIC_FLOW_CONTROL_MAP     *fc_map
  );

/* } */

soc_error_t arad_multicast_table_size_get(int unit, uint32* mc_table_size);

#include <soc/dpp/SAND/Utils/sand_footer.h>


/* } __ARAD_MULTICAST_FABRIC_INCLUDED__*/
#endif
