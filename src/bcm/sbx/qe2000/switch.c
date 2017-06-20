/* $Id: switch.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Broadcom QE2000 Switch API.
 */

#include <soc/drv.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ca_auto.h>

#include <bcm/error.h>
#include <bcm/switch.h>
#include <bcm/debug.h>

#include <bcm/error.h>
#include <soc/sbx/qe2000_scoreboard.h>

#include <bcm_int/sbx/error.h>


int
bcm_qe2000_switch_control_get(int unit,
                 bcm_switch_control_t type,
                 int *val)
{
    if(type == bcmSwitchPktAge ) {
        soc_qe2000_pkt_age_get (unit, val);
        return BCM_E_NONE;
    }

    return BCM_E_UNAVAIL;
}

int
bcm_qe2000_switch_control_set(int unit,
                 bcm_switch_control_t type,
                 int val)
{
     if(type == bcmSwitchPktAge ) {
        soc_qe2000_pkt_age_set (unit, val);
        return BCM_E_NONE;
    }

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_switch_event_register
 * Description:
 *      Registers a call back function for switch critical events
 * Parameters:
 *      unit        - Device unit number
 *  cb          - The desired call back function to register for critical events.
 *  userdata    - Pointer to any user data to carry on.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 *      
 *      Several call back functions could be registered, they all will be called upon
 *      critical event. If registered callback is called it is adviced to log the 
 *  information and reset the chip. 
 *  Same call back function with different userdata is allowed to be registered. 
 */
int 
bcm_qe2000_switch_event_register(int unit, 
			      bcm_switch_event_cb_t cb, 
                              void *userdata)
{
    return soc_event_register(unit, (soc_event_cb_t)cb, userdata);
}


/*
 * Function:
 *      bcm_switch_event_unregister
 * Description:
 *      Unregisters a call back function for switch critical events
 * Parameters:
 *      unit        - Device unit number
 *  cb          - The desired call back function to unregister for critical events.
 *  userdata    - Pointer to any user data associated with a call back function
 * Returns:
 *      BCM_E_xxx
 * Notes:
 *      
 *  If userdata = NULL then all matched call back functions will be unregistered,
 */
int 
bcm_qe2000_switch_event_unregister(int unit, 
				  bcm_switch_event_cb_t cb, 
				  void *userdata)
{
    return soc_event_unregister(unit, (soc_event_cb_t)cb, userdata);
}
