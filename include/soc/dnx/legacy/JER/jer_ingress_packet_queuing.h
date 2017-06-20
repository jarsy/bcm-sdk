#include <soc/mcm/memregs.h>
/* $Id: jer2_jer_ingress_packet_queuing.c,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER2_JER_INGRESS_SCHEDULER_QUEUEING_H__
#define __JER2_JER_INGRESS_SCHEDULER_QUEUEING_H__

#include <soc/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/TMC/tmc_api_ingress_packet_queuing.h>


/*********************************************************************
*     Sets the Explicit Flow Unicast packets mapping to queue.
*     Doesn't affect packets that arrive with destination_id
*     in the header.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_jer_ipq_explicit_mapping_mode_info_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  DNX_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
    );

uint32
jer2_jer_ipq_explicit_mapping_mode_info_get(
   DNX_SAND_IN  int                                 unit,
   DNX_SAND_OUT DNX_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
   );

/*********************************************************************
* NAME:
*     jer2_jer_ipq_init
* FUNCTION:
*     Initialization of the Arad blocks configured in this module.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
jer2_jer_ipq_init(
   DNX_SAND_IN  int                 unit
   );


int
jer2_jer_ipq_default_invalid_queue_set(
   DNX_SAND_IN  int            unit,
   DNX_SAND_IN  int            core,
   DNX_SAND_IN  uint32         queue_id,
   DNX_SAND_IN  int            enable);


int
jer2_jer_ipq_default_invalid_queue_get(
   DNX_SAND_IN  int            unit,
   DNX_SAND_IN  int            core,
   DNX_SAND_OUT uint32         *queue_id,
   DNX_SAND_OUT int            *enable);

#endif /* __JER2_JER_INGRESS_SCHEDULER_QUEUEING_H__ */

