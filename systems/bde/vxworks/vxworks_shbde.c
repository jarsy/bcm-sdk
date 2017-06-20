/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#include <vxWorks.h>
#include <string.h>

#include <sal/appl/pci.h>

#include <shbde.h>
#include <shbde_iproc.h>
#include "vxworks_shbde.h"

/* Hardware abstractions for shared BDE functions */

static unsigned short
vxworks_pcic16_read(void *pci_dev, unsigned int addr)
{
    pci_dev_t *dev = (pci_dev_t *)pci_dev;
    UINT16 data;

    pciConfigInWord(dev->busNo, dev->devNo, dev->funcNo, addr, &data);
    return data;
}

static void
vxworks_pcic16_write(void *pci_dev, unsigned int addr, unsigned short data)
{
    pci_dev_t *dev = (pci_dev_t *)pci_dev;

    pciConfigOutWord(dev->busNo, dev->devNo, dev->funcNo, addr, data);
}

static unsigned int
vxworks_pcic32_read(void *pci_dev, unsigned int addr)
{
    pci_dev_t *dev = (pci_dev_t *)pci_dev;
    UINT32 data;

    pciConfigInLong(dev->busNo, dev->devNo, dev->funcNo, addr, &data);
    return data;
}

static void
vxworks_pcic32_write(void *pci_dev, unsigned int addr, unsigned int data)
{
    pci_dev_t *dev = (pci_dev_t *)pci_dev;

    pciConfigOutLong(dev->busNo, dev->devNo, dev->funcNo, addr, data);
}

static unsigned int
vxworks_io32_read(void *addr)
{
    return *((volatile UINT32 *)addr);
}

static void
vxworks_io32_write(void *addr, unsigned int data)
{
    *((volatile UINT32 *)addr) = data;
}

/*
 * Function:
 *      vxworks_shbde_hal_init
 * Purpose:
 *      Initialize hardware abstraction module for Vxworks kernel.
 * Parameters:
 *      shbde - pointer to uninitialized hardware abstraction module
 *      log_func - optional log output function
 * Returns:
 *      Always 0
 */
int
vxworks_shbde_hal_init(shbde_hal_t *shbde, shbde_log_func_t log_func)
{
    memset(shbde, 0, sizeof(*shbde));

    shbde->log_func = log_func;

    shbde->pcic16_read = vxworks_pcic16_read;
    shbde->pcic16_write = vxworks_pcic16_write;
    shbde->pcic32_read = vxworks_pcic32_read;
    shbde->pcic32_write = vxworks_pcic32_write;

    shbde->io32_read = vxworks_io32_read;
    shbde->io32_write = vxworks_io32_write;

    return 0;
}
