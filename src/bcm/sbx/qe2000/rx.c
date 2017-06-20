/*
 * $Id: rx.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        rx.c
 */

#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>

#include <bcm/error.h>
#include <bcm/rx.h>

#include <bcm_int/sbx/rx.h>

#include <soc/sbx/hal_ka_auto.h>

int
sbx_qe2000_rx_config(int unit, bcm_rx_cfg_t *cfg)
{
    int value;

    if (cfg->pkt_size > RX_PKT_SIZE_DFLT) {
        return BCM_E_PARAM;
    }

    /* mapping 
       0x0 256 bytes
       0x1 512 bytes
       0x2 1024 bytes
       0x3 2048 bytes
       0x4 4086 bytes
       0x5 8192 bytes
       0x6 16384 bytes
       0x7 256 bytes
    */
    for (value=0; value<7; value++) {
        if (cfg->pkt_size <= (0x100 << value))
            break;
    }

    SAND_HAL_RMW_FIELD (unit, KA, PC_RXBUF_SIZE, RXBUF_SIZE, value);

    return BCM_E_NONE;
}
