/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: jer_trunk.h
 */

#ifndef __JER_TRUNK_INCLUDED__

#define __JER_TRUNK_INCLUDED__

/* 
 *  INCLUDES
 */


/* 
 *  FUNCTIONS
 */

/* 
 * soc_jer_port_direct_lb_key_set - set min & max LB-Key 
 *  if set_min == FALSE  - dosn't set min value.
 *  if set_max == FALSE  - dosn't set max value.
 */
uint32 
  soc_jer_trunk_direct_lb_key_set( 
    SOC_SAND_IN int unit, 
    SOC_SAND_IN int core_id, 
    SOC_SAND_IN uint32 local_port,
    SOC_SAND_IN uint32 min_lb_key,
    SOC_SAND_IN uint32 set_min,
    SOC_SAND_IN uint32 max_lb_key,
    SOC_SAND_IN uint32 set_max
   );

/* 
 * soc_jer_port_direct_lb_key_get - get min & max LB-Key 
 *  if set_min == NULL  - doesn't get min value.
 *  if set_max == NULL  - doesn't get max value.
 */
uint32 
    soc_jer_trunk_direct_lb_key_get(
      SOC_SAND_IN int unit, 
      SOC_SAND_IN int core_id, 
      SOC_SAND_IN uint32  local_port,
      SOC_SAND_OUT uint32 *min_lb_key,
      SOC_SAND_OUT uint32 *max_lb_key
   );

int soc_jer_trunk_init (int unit);

#endif /*__JER_TRUNK_INCLUDED__*/

