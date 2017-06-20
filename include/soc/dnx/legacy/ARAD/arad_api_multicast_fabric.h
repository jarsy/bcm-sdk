/* $Id: jer2_arad_api_multicast_fabric.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER2_ARAD_API_MULTICAST_FABRIC_INCLUDED__
/* { */
#define __JER2_ARAD_API_MULTICAST_FABRIC_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_api_general.h>
#include <soc/dnx/legacy/TMC/tmc_api_multicast_fabric.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define JER2_ARAD_MULT_FABRIC_NOF_BE_CLASSES      3

#define JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_DONT_MAP DNX_TMC_MULT_FABRIC_FLOW_CONTROL_DONT_MAP

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

#define JER2_ARAD_MULT_FABRIC_CLS_MIN                          DNX_TMC_MULT_FABRIC_CLS_MIN
#define JER2_ARAD_MULT_FABRIC_CLS_MAX                          DNX_TMC_MULT_FABRIC_CLS_MAX
typedef DNX_TMC_MULT_FABRIC_CLS_RNG                            JER2_ARAD_MULT_FABRIC_CLS_RNG;

/* Arad Egress Multicast Fabric Class: 0-3.    */
typedef DNX_TMC_MULT_FABRIC_CLS JER2_ARAD_MULT_FABRIC_CLS;

typedef DNX_TMC_MULT_FABRIC_PORT_INFO                          JER2_ARAD_MULT_FABRIC_PORT_INFO;
typedef DNX_TMC_MULT_FABRIC_SHAPER_INFO                        JER2_ARAD_MULT_FABRIC_SHAPER_INFO;
typedef DNX_TMC_MULT_FABRIC_BE_CLASS_INFO                      JER2_ARAD_MULT_FABRIC_BE_CLASS_INFO;
typedef DNX_TMC_MULT_FABRIC_BE_INFO                            JER2_ARAD_MULT_FABRIC_BE_INFO;
typedef DNX_TMC_MULT_FABRIC_GR_INFO                            JER2_ARAD_MULT_FABRIC_GR_INFO;
typedef DNX_TMC_MULT_FABRIC_INFO                               JER2_ARAD_MULT_FABRIC_INFO;
typedef DNX_TMC_MULT_FABRIC_ACTIVE_LINKS                       JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS;
typedef DNX_TMC_MULT_FABRIC_FLOW_CONTROL_MAP                   JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP;

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
*     jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_set
* TYPE:
*   PROC
* FUNCTION:
*   Maps the embedded traffic class in the packet header to
*   a multicast class (0..3). This multicast class will be
*   further used for egress/fabric replication.
* INPUT:
*   DNX_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  JER2_ARAD_TR_CLS                              tr_cls_ndx -
*     the traffic class index, for which to map a new class.
*     Range: 0 - 7.
*   DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_CLS                     new_mult_cls -
*     The new multicast class that is mapped to the
*     tr_cls_ndx. Range: 0 - 3.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_TR_CLS              tr_cls_ndx,
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_CLS     new_mult_cls
  );

/*********************************************************************
* NAME:
*     jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_get
* TYPE:
*   PROC
* FUNCTION:
*   Maps the embedded traffic class in the packet header to
*   a multicast class (0..3). This multicast class will be
*   further used for egress/fabric replication.
* INPUT:
*   DNX_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  JER2_ARAD_TR_CLS                              tr_cls_ndx -
*     the traffic class index, for which to map a new class.
*     Range: 0 - 7.
*   DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_CLS                     *new_mult_cls -
*     The new multicast class that is mapped to the
*     tr_cls_ndx. Range: 0 - 3.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_TR_CLS         tr_cls_ndx,
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_CLS     *new_mult_cls
  );

/*********************************************************************
* NAME:
*     jer2_arad_mult_fabric_base_queue_set
* TYPE:
*   PROC
* FUNCTION:
*   This procedure configures the base queue of the
*   multicast egress/fabric.
* INPUT:
*   DNX_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  uint32                                  queue_id -
*     the base queue of fabric or egress multicast packets.
*     Range: 0 - 32K-1.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mult_fabric_base_queue_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  queue_id
  );

/*********************************************************************
* NAME:
*     jer2_arad_mult_fabric_base_queue_get
* TYPE:
*   PROC
* FUNCTION:
*   This procedure configures the base queue of the
*   multicast egress/fabric.
* INPUT:
*   DNX_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   DNX_SAND_OUT uint32                                  *queue_id -
*     the base queue of fabric or egress multicast packets.
*     Range: 0 - 32K-1.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mult_fabric_base_queue_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint32                  *queue_id
  );

/*********************************************************************
* NAME:
*   jer2_arad_mult_fabric_enhanced_set
* TYPE:
*   PROC
* FUNCTION:
*   Configure the Enhanced Fabric Multicast Queue
*   configuration: the fabric multicast queues are defined
*   in a configured range, and the credits are coming to
*   these queues according to a scheduler scheme.
* INPUT:
*   DNX_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  DNX_SAND_U32_RANGE                            *queue_range -
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
  jer2_arad_mult_fabric_enhanced_set(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  int                                 core_id,
    DNX_SAND_IN  DNX_SAND_U32_RANGE                            *queue_range
  );

/*********************************************************************
* NAME:
*   jer2_arad_mult_fabric_enhanced_get
* TYPE:
*   PROC
* FUNCTION:
*   Configure the Enhanced Fabric Multicast Queue
*   configuration: the fabric multicast queues are defined
*   in a configured range, and the credits are coming to
*   these queues according to a scheduler scheme.
* INPUT:
*   DNX_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   DNX_SAND_OUT DNX_SAND_U32_RANGE                            *queue_range -
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
  jer2_arad_mult_fabric_enhanced_get(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  int                                 core_id,
    DNX_SAND_OUT DNX_SAND_U32_RANGE                            *queue_range
  );

/*********************************************************************
* NAME:
*     jer2_arad_mult_fabric_active_links_set
* TYPE:
*   PROC
* FUNCTION:
*   This procedure sets the FAP links that are eligible for
*   multicast fabric traffic.
* INPUT:
*   DNX_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS            *links -
*     A bitmap of the device links. if bit (0 -
*     JER2_ARAD_NOF_LINKS) is up, then its corresponding link (0 -
*     JER2_ARAD_NOF_LINKS) is eligible for spatial multicast
*     distribution.
*   DNX_SAND_IN  uint8                                 tbl_refresh_enable -
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
  jer2_arad_mult_fabric_active_links_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS *links,
    DNX_SAND_IN  uint8                 tbl_refresh_enable
  );

/*********************************************************************
* NAME:
*     jer2_arad_mult_fabric_active_links_get
* TYPE:
*   PROC
* FUNCTION:
*   This procedure sets the FAP links that are eligible for
*   multicast fabric traffic.
* INPUT:
*   DNX_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS            *links -
*     A bitmap of the device links. if bit (0 -
*     JER2_ARAD_NOF_LINKS) is up, then its corresponding link (0 -
*     JER2_ARAD_NOF_LINKS) is eligible for spatial multicast
*     distribution.
*   DNX_SAND_OUT uint8                                 *tbl_refresh_enable -
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
  jer2_arad_mult_fabric_active_links_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS *links,
    DNX_SAND_OUT uint8                 *tbl_refresh_enable
  );

/*********************************************************************
* NAME:
*     jer2_arad_mult_fabric_active_links_set\get 
* TYPE:
*   PROC
* FUNCTION:
*   This procedure sets the FMQ with GCI LB level
* INPUT:
*   DNX_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   DNX_SAND_IN JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP       fc_map (set) - 
*   DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP      fc_map (get) - 
*     See in struct description.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_mult_fabric_flow_control_set(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP      *fc_map
  );

uint32
  jer2_arad_mult_fabric_flow_control_get(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP     *fc_map
  );

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_PORT_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_PORT_INFO *info
  );

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_SHAPER_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_SHAPER_INFO *info
  );

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_BE_CLASS_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_BE_CLASS_INFO *info
  );

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_BE_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_BE_INFO *info
  );

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_GR_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_GR_INFO *info
  );

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_INFO *info
  );

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS_clear(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS *info
  );

void
  jer2_arad_JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP_clear(
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP *info
  );

#if JER2_ARAD_DEBUG_IS_LVL1


const char*
  jer2_arad_JER2_ARAD_MULT_FABRIC_CLS_RNG_to_string(
    DNX_SAND_IN JER2_ARAD_MULT_FABRIC_CLS_RNG enum_val
  );



void
  jer2_arad_JER2_ARAD_MULT_FABRIC_PORT_INFO_print(
    DNX_SAND_IN JER2_ARAD_MULT_FABRIC_PORT_INFO *info
  );



void
  jer2_arad_JER2_ARAD_MULT_FABRIC_SHAPER_INFO_print(
    DNX_SAND_IN JER2_ARAD_MULT_FABRIC_SHAPER_INFO *info
  );



void
  jer2_arad_JER2_ARAD_MULT_FABRIC_BE_CLASS_INFO_print(
    DNX_SAND_IN JER2_ARAD_MULT_FABRIC_BE_CLASS_INFO *info
  );



void
  jer2_arad_JER2_ARAD_MULT_FABRIC_BE_INFO_print(
    DNX_SAND_IN JER2_ARAD_MULT_FABRIC_BE_INFO *info
  );



void
  jer2_arad_JER2_ARAD_MULT_FABRIC_GR_INFO_print(
    DNX_SAND_IN JER2_ARAD_MULT_FABRIC_GR_INFO *info
  );



void
  jer2_arad_JER2_ARAD_MULT_FABRIC_INFO_print(
    DNX_SAND_IN JER2_ARAD_MULT_FABRIC_INFO *info
  );



void
  jer2_arad_JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS_print(
    DNX_SAND_IN uint32 unit,
    DNX_SAND_IN JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS *info
  );


#endif /* JER2_ARAD_DEBUG_IS_LVL1 */


/* } */


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


/* } __JER2_ARAD_API_MULTICAST_FABRIC_INCLUDED__*/
#endif
