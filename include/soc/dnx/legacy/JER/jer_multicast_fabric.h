/* $Id: jer2_jer_multicast_fabric.h,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef _SOC_JER2_JER_MULTICAST_FABRIC_H
#define _SOC_JER2_JER_MULTICAST_FABRIC_H

#include <soc/dnx/legacy/ARAD/arad_chip_regs.h>

/*********************************************************************
*     Configure the Enhanced Fabric Multicast Queue
*     configuration: the fabric multicast queues are defined
*     in a configured range, and the credits are coming to
*     these queues according to a scheduler scheme.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_jer_mult_fabric_enhanced_set(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  int                                 core_id,
    DNX_SAND_IN  DNX_SAND_U32_RANGE                  *queue_range
  );
/*********************************************************************
*     Configure the Enhanced Fabric Multicast Queue
*     configuration: the fabric multicast queues are defined
*     in a configured range, and the credits are coming to
*     these queues according to a scheduler scheme.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_jer_mult_fabric_enhanced_get(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  int                                 core_id,
    DNX_SAND_INOUT DNX_SAND_U32_RANGE                *queue_range
  );

#endif /*_SOC_JER2_JER_MULTICAST_FABRIC_H*/
