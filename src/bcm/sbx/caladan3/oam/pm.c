/*
 * $Id: pm.c,v 1.35 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * CALADAN3 OAM Performance measurement function
 */

#if defined(INCLUDE_L3)

#include <bcm/error.h>
#include <bcm/oam.h>

int
bcm_caladan3_oam_loss_add(int unit, bcm_oam_loss_t *loss_ptr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_caladan3_oam_loss_get(int unit, bcm_oam_loss_t *loss_ptr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_caladan3_oam_loss_delete(int unit, bcm_oam_loss_t *loss_ptr)
{
    
    return BCM_E_UNAVAIL;
}


/*
 *   Function
 *      bcm_caladan3_oam_delay_add
 *   Purpose
 *      Add delay measurement object to an existing local endpoint
 *   Parameters
 *       unit        = BCM device number
 *       delay_ptr   = delay object to add
 *   Returns
 *       BCM_E_*
 */
int
bcm_caladan3_oam_delay_add(int unit, bcm_oam_delay_t *delay_ptr)
{
    return BCM_E_UNAVAIL;
}


int
bcm_caladan3_oam_delay_get(int unit, bcm_oam_delay_t *delay_ptr)
{
    return BCM_E_UNAVAIL;

}


int
bcm_caladan3_oam_delay_delete(int unit, bcm_oam_delay_t *delay_ptr)
{
    return BCM_E_UNAVAIL;
}

#endif  /* INCLUDE_L3 */
