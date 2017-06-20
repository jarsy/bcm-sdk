/*
 * $Id: qos.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * All Rights Reserved.$
 *
 * QoS initializers
 */

#include <sal/core/libc.h>
 
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/l2u.h>
#include <soc/util.h>
#include <soc/debug.h>

#include <bcm/port.h>
#include <bcm/error.h> 
#include <bcm/qos.h>

/* 
 * Function:
 *      bcm_qos_map_t_init
 * Purpose:
 *      Initialize the QoS map struct
 * Parameters: 
 *      qos_map - Pointer to the struct to be init'ed
 */
void
bcm_qos_map_t_init(bcm_qos_map_t *qos_map)
{
    if (qos_map != NULL) {
        sal_memset(qos_map, 0, sizeof(*qos_map));
    }
    return;
}



