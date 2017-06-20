/* $Id: jer_multicast_fabric.h,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef _SOC_JER_MULTICAST_FABRIC_H
#define _SOC_JER_MULTICAST_FABRIC_H

#include <soc/dpp/ARAD/arad_chip_regs.h>

/*********************************************************************
*     Configure the Enhanced Fabric Multicast Queue
*     configuration: the fabric multicast queues are defined
*     in a configured range, and the credits are coming to
*     these queues according to a scheduler scheme.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer_mult_fabric_enhanced_set(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_SAND_U32_RANGE                  *queue_range
  );
/*********************************************************************
*     Configure the Enhanced Fabric Multicast Queue
*     configuration: the fabric multicast queues are defined
*     in a configured range, and the credits are coming to
*     these queues according to a scheduler scheme.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer_mult_fabric_enhanced_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_INOUT SOC_SAND_U32_RANGE                *queue_range
  );

#endif /*_SOC_JER_MULTICAST_FABRIC_H*/
