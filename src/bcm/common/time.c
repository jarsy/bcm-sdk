/*
 * $Id: time.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Time - Broadcom Time BroadSync API - shared functionality
 */

#include <soc/drv.h>
#include <bcm/time.h>


/*
 * Function:
 *      bcm_time_spec_t_init
 * Purpose:
 *      Initialize bcm_time_spec_t structure 
 * Parameters:
 *      spec - (OUT) pointer to bcm_time_spec_t structure to initialize
 */
void
bcm_time_spec_t_init(bcm_time_spec_t *spec)
{
    sal_memset(spec, 0, sizeof (bcm_time_spec_t));
}


/*
 * Function:
 *      bcm_time_interface_t_init
 * Purpose:
 *      Initialize bcm_time_interface_t structure 
 * Parameters:
 *      intf - (OUT) pointer to bcm_time_interface_t structure to initialize
 */
void
bcm_time_interface_t_init(bcm_time_interface_t *intf)
{
    sal_memset(intf, 0, sizeof (bcm_time_interface_t));
}


/*
 * Function:
 *      bcm_time_capture_t_init
 * Purpose:
 *      Initialize bcm_time_capture_t structure 
 * Parameters:
 *      capture - (OUT) pointer to bcm_time_capture_t structure to initialize
 */
void
bcm_time_capture_t_init(bcm_time_capture_t *capture)
{
    sal_memset(capture, 0, sizeof (bcm_time_capture_t));
}

/*
 * Function:
 *      bcm_time_synce_divisor_setting_t_init
 * Purpose:
 *      Initialize bcm_esw_time_synce_divisor_setting_t struct
 * Parameters:
        divisor - (OUT) Divisor setting
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
void
bcm_time_synce_divisor_setting_t_init(
    bcm_time_synce_divisor_setting_t *divisor)
{
    sal_memset(divisor, 0, sizeof (bcm_time_synce_divisor_setting_t));
    return;
}

