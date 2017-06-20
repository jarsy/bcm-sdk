/*
 * $Id: brcm_osl.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Linux OS Independent Layer
 */

#define	BRCM_OSL

#include <shared/bsl.h>

#include <shared/et/typedefs.h>
#include <shared/et/osl.h>
#include <shared/et/bcmenetrxh.h>
#include <soc/ethdma.h>
#if defined(KEYSTONE) || defined(ROBO_4704) || defined(IPROC_CMICD)

void*
et_pktget(void *dev, int chan, uint len, bool send)
{
    int unit = (int)dev;
    eth_dv_t *dv;

    if (send) {
        return ((void *)NULL);
    }

    dv = et_soc_rx_chain_get(unit, chan, 0);

    return ((void*) dv);
}

void
et_pktfree(void* dev, void *p)
{
    int unit = (int)dev;
    eth_dv_t *dv = (eth_dv_t *)p;

    switch (dv->dv_op) {
        case DV_TX:
            if (dv->dv_done_chain) {
                dv->dv_done_chain(dv->dv_unit, dv);
            }
            break;

        case DV_RX:
            if (!dv->dv_length) {
                dv->dv_flags |= RXF_RXER;
            }

            /* free allocated packet buffer */
            if (dv->dv_done_packet) {
                dv->dv_flags |= RXF_RXER;
                dv->dv_done_packet(dv->dv_unit, dv, dv->dv_dcb);
            } else {
                soc_cm_sfree(unit, (void *)dv->dv_dcb->dcb_vaddr);
                soc_eth_dma_dv_free(unit, (eth_dv_t *)dv);
            }
            break;

        default:
            LOG_CLI((BSL_META_U(unit,
                                "ERROR: unit %d unknown dma op %d\n"), unit, dv->dv_op));
            assert(0);
            return;
    }
}

#endif /* defined(KEYSTONE) || defined(ROBO_4704) || defined(IPROC_CMICD) */
