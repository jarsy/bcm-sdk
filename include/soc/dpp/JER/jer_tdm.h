/*
 * $Id: jer_tdm.h, v1 18/11/2014 09:55:39 azarrin $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef __JER_TDM_INCLUDED__

#define __JER_TDM_INCLUDED__


/*************
 * FUNCTIONS *
 *************/

/* 
* Configure all ports that allow TDM traffic to ingress the device. TDM traffic on any other port will be dropped. 
* Modifying the set of TDM-enabled ports can be used for fast protection-switching of TDM traffic. 
*/ 
int jer_tdm_ingress_failover_set(int unit, bcm_pbmp_t tdm_enable_pbmp);

/* 
* Retrieve all ports that allow TDM traffic to ingress the device. TDM traffic on any other port will be dropped. 
* Modifying the set of TDM-enabled ports can be used for fast protection-switching of TDM traffic. 
*/ 
int jer_tdm_ingress_failover_get(int unit, bcm_pbmp_t *tdm_enable_pbmp);

int 
jer_mesh_tdm_multicast_set(
   int                            unit, 
   soc_port_t                     port,
   uint32                         flags, 
   uint32                         destid_count,
   soc_module_t                   *destid_array
   );

/*********************************************************************
* NAME:
*     jer_tdm_init
* FUNCTION:
*     Initialization of the TDM configuration depends on the tdm mode.
* INPUT:
*    int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   1. Called as part of the initialization sequence.
*********************************************************************/
uint32
  jer_tdm_init(
      int  unit
  );

#endif /*__JER_TDM_INCLUDED__*/

