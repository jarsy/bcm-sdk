/* 
 * $Id: led.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        led.c
 * Purpose:     LED driver support
 *
 */

#include <bcm/error.h>
#include <board_int/support.h>

/*
 * Function:
 *     board_led_init
 * Purpose:
 *     Initialize LED subsystem
 * Parameters:
 *     unit - (IN) BCM device number
 * Returns:
 *     BCM_E_NONE - success
 *     BCM_E_XXX - failed
 */
int
board_led_init(int unit, int offset, int len, uint8 *pgm)
{
    /* clear program, data */
    /* copy program */
    /* register with linkscan */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     board_led_deinit
 * Purpose:
 *     Deinitialize LED subsystem
 * Parameters:
 *     unit - (IN) BCM device number
 * Returns:
 *     BCM_E_NONE - success
 *     BCM_E_XXX - failed
 */
int
board_led_deinit(int unit)
{
    return BCM_E_UNAVAIL;
}

