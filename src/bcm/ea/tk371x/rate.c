/*
 * $Id: rate.c,$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    rate.c
 * Rate - Broadcom EA tk371x Rate Limiting API.
 *
 */
#include <sal/types.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <bcm/rate.h>
#include <bcm/port.h>
#include <bcm/types.h>
#include <bcm/error.h>

#include <bcm_int/tk371x_dispatch.h>
/*
 * Function:
 *  bcm_tk371x_rate_bcast_get
 * Description:
 *  Get rate limit for BCAST packets
 * Parameters:
 *  unit - EA tk371x device unit number
 *  pps - (OUT) Rate limit value in packets/second
 *  flags - (OUT) Bitmask with one or more of BCM_RATE_*
 *  port - Port number for which BCAST limit is requested
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  - The rate_limit value in API is packet per second, but Drv layer
 *      will use this value by Kb per second.
 */
int
bcm_tk371x_rate_bcast_get(
		int unit,
		int *pps,
		int *flags,
		int port){
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *  bcm_tk371x_rate_bcast_set
 * Description:
 *  Configure rate limit for BCAST packets
 * Parameters:
 *  unit - EA tk371x device unit number
 *  pps - Rate limit value in packets/second
 *  flags - Bitmask with one or more of BCM_RATE_*
 *      port - Port number for which BCAST limit needs to be set
 * Returns:
 *  BCM_E_NONE - Success.
 *  BCM_E_UNAVAIL - Not supported.
 * Notes:
 *  - EA chip on rate/storm control use the same rate_limit value.
 *    This limit value is system basis but port basis.
 *  - The rate_limit value in API is packet per second, but Drv layer
 *      will view this value as Kb per second.
 */
int
bcm_tk371x_rate_bcast_set(
		int unit,
		int pps,
		int flags,
		int port){
	return BCM_E_UNAVAIL;
}

