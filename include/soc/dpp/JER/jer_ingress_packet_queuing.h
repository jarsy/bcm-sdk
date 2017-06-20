#include <soc/mcm/memregs.h>
/* $Id: jer_ingress_packet_queuing.c,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER_INGRESS_SCHEDULER_QUEUEING_H__
#define __JER_INGRESS_SCHEDULER_QUEUEING_H__

#include <soc/error.h>
#include <soc/dpp/SAND/Utils/sand_framework.h>
#include <soc/dpp/TMC/tmc_api_ingress_packet_queuing.h>


/*********************************************************************
*     Sets the Explicit Flow Unicast packets mapping to queue.
*     Doesn't affect packets that arrive with destination_id
*     in the header.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer_ipq_explicit_mapping_mode_info_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  SOC_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
    );

uint32
jer_ipq_explicit_mapping_mode_info_get(
   SOC_SAND_IN  int                                 unit,
   SOC_SAND_OUT SOC_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
   );

/*********************************************************************
* NAME:
*     jer_ipq_init
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
jer_ipq_init(
   SOC_SAND_IN  int                 unit
   );


int
jer_ipq_default_invalid_queue_set(
   SOC_SAND_IN  int            unit,
   SOC_SAND_IN  int            core,
   SOC_SAND_IN  uint32         queue_id,
   SOC_SAND_IN  int            enable);


int
jer_ipq_default_invalid_queue_get(
   SOC_SAND_IN  int            unit,
   SOC_SAND_IN  int            core,
   SOC_SAND_OUT uint32         *queue_id,
   SOC_SAND_OUT int            *enable);

#endif /* __JER_INGRESS_SCHEDULER_QUEUEING_H__ */

