/*
 * $Id: linkscan.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#include "vxWorks.h"
#include "intLib.h"
#if (CPU_FAMILY == MIPS)
#include "mbz.h"
#endif
#include "config.h"
#include "stdio.h"
#include "assert.h"
#include "time.h"
#include "taskLib.h"
#include "sysLib.h"
#include "string.h"
#include "drv/pci/pciConfigLib.h"
#include "drv/pci/pciIntLib.h"
#include "systemInit.h"
#include "dmaOps.h"

static pbmp_t myLinkMask[MAX_DEVICES];

#define soc_link_mask_get(u,m) (*(m)=myLinkMask[u])
#define soc_link_mask_set(u,m) (myLinkMask[u]=(m),0)

int bcm_port_enable_set(int unit, bcm_port_t port, int enable);
int phy_fe_ge_link_get(int unit, soc_port_t port, int *link);

static void ledproc_linkscan_cb(int unit, soc_port_t port, int linkstatus);
int PCI_DEBUG_ON = 0;

static int
_bcm_linkscan_update_port(int unit, int port)
{
    pbmp_t lc_pbm_link;
    int    cur_link, new_link;

    soc_link_mask_get(unit, &lc_pbm_link);
    cur_link = (lc_pbm_link >> port) & 1;

    SOC_IF_ERROR_RETURN(phy_fe_ge_link_get(unit, port, &new_link));

    if (cur_link == new_link) { /* No change */
        return BCM_E_NONE;
    }

    PRINTF_DEBUG2(("MII_STAT : 0x%08x 0x%08x\n", new_link, cur_link));

    /*
     * If disabling, stop ingresses from sending any more traffic to
     * this port.
     */
    if (!new_link) {
        SOC_PBMP_PORT_REMOVE(lc_pbm_link, port);
    } else {
        SOC_PBMP_PORT_ADD(lc_pbm_link, port);
    }

    PRINTF_DEBUG(("SETTING 0x%08x\n", lc_pbm_link));
    PCI_DEBUG_ON = 0;
    soc_link_mask_set(unit, lc_pbm_link);

    ledproc_linkscan_cb(unit, port, new_link);

    /* Program MACs (only if port is not forced) */
    SOC_IF_ERROR_RETURN(bcm_port_enable_set(unit, port, new_link));

    PCI_DEBUG_ON = 0;

    return BCM_E_NONE;
}


static int
_bcm_linkscan_update(int unit)
{
    pbmp_t     save_link_change, new_link;
    bcm_port_t port;

    soc_link_mask_get(unit, &save_link_change);

    HE_PBMP_GE_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(_bcm_linkscan_update_port(unit, port));
    }

    soc_link_mask_get(unit, &new_link);

    if (save_link_change != new_link) {
        WRITE_EPC_LINK_BMAPr(unit, new_link | 0x00000001);
        {
            pbmp_t foo;
            READ_EPC_LINK_BMAPr(unit,&foo);
            PRINTF_DEBUG(("EPC LINK :: %d 0x%08x\n", unit, foo));
        }
    }
    return BCM_E_NONE;
}


#define CMIC_LED_CTRL               0x00001000
#define CMIC_LED_CTRL               0x00001000
#define CMIC_LED_STATUS             0x00001004
#define CMIC_LED_PROGRAM_RAM_BASE   0x00001800
#define CMIC_LED_DATA_RAM_BASE      0x00001c00
#define CMIC_LED_PROGRAM_RAM(_a)    (CMIC_LED_PROGRAM_RAM_BASE + 4 * (_a))
#define CMIC_LED_PROGRAM_RAM_SIZE   0x100
#define CMIC_LED_DATA_RAM(_a)       (CMIC_LED_DATA_RAM_BASE + 4 * (_a))
#define CMIC_LED_DATA_RAM_SIZE      0x100

#define LS_LED_DATA_OFFSET          0x80

static int
ledproc_load(int unit, uint8 *program, int bytes)
{
    int offset;

    SOC_IF_ERROR_RETURN(soc_pci_write(unit, CMIC_LED_CTRL, 0x0));

    for (offset = 0; offset < CMIC_LED_PROGRAM_RAM_SIZE; offset++) {
        SOC_IF_ERROR_RETURN(soc_pci_write(unit,
            CMIC_LED_PROGRAM_RAM(offset),
            (offset < bytes) ? (uint32) program[offset] : 0));
    }

    for (offset = 0x80; offset < CMIC_LED_DATA_RAM_SIZE; offset++) {
        SOC_IF_ERROR_RETURN(soc_pci_write(unit,CMIC_LED_DATA_RAM(offset),0));
    }

    SOC_IF_ERROR_RETURN(soc_pci_write(unit, CMIC_LED_CTRL, 0x1));

    return SOC_E_NONE;
}


static void
ledproc_linkscan_cb(int unit, soc_port_t port, int linkstatus)
{
}


void
linkScanThread(void *opt)
{
#if 0
    int unit = (int)opt;

    while (1) {
        if (_bcm_linkscan_update(unit) != 0) {
            PRINTF_ERROR(("LINKSCAN TASK FAIL........ terminating"));
            break;
        }
        sal_usleep(100 * MILLISECOND_USEC);
    }
#endif
    return;
}


int
myLinkScan(int unit)
{
    if (sal_thread_create("LinkScan", SAL_THREAD_STKSZ,
                           8, linkScanThread, (void*)unit) == NULL) {
        PRINTF_ERROR(("FAIL TO INITIALIZE LINKSCANTHREAD\n"));
        return -1;
    }

    return 0;
}
