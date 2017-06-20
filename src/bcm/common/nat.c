
/*
 * $Id: nat.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    nat.c
 * Purpose: Common NAT API
 */

#include <soc/defs.h>

#include <sal/core/libc.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>

#include <bcm/nat.h>
#include <bcm/error.h>
#include <bcm/debug.h>
#include <bcm/types.h>


#ifdef INCLUDE_L3
/*
 * Function:
 *  bcm_l3_nat_egress_t_init
 * Description:
 *  Initialize an egress NAT structure
 * Parameters:
 *  nat_info -    [IN/OUT] pointer to NAT structure
 * Returns:
 *	NONE
 */
void 
bcm_l3_nat_egress_t_init(bcm_l3_nat_egress_t *nat_info) 
{
    if (NULL != nat_info) {
        sal_memset(nat_info, 0, sizeof (*nat_info));
    }
    return;
}

/*
 * Function:
 *  bcm_l3_nat_ingress_t_init
 * Description:
 *  Initialize an ingress NAT structure
 * Parameters:
 *  nat_info -    [IN/OUT] pointer to ingress NAT structure
 * Returns:
 *	NONE
 */
void 
bcm_l3_nat_ingress_t_init(bcm_l3_nat_ingress_t *nat_info) 
{
    if (NULL != nat_info) {
        sal_memset(nat_info, 0, sizeof (*nat_info));
    }
    return;
}
#endif /* INCLUDE_L3 */
