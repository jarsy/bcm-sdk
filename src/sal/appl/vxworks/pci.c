/*
 * $Id: pci.c,v 1.27 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * PCI memory and configuration space routines.
 *
 * NOTE: the base address used in this file is opaque. This means that
 * the PCI device must first have its base-address programmed by
 * writing the start offset to PCI configuration offset 0x10.
 * Writes to the memory space afterwards requires the address to be
 * based on the base-address plus the offset to the desire register
 * to be accessed.
 */

#include <shared/bsl.h>

#include <sys/types.h>
#include <assert.h>

#define __PROTOTYPE_5_0		/* Get stdarg prototypes for logMsg */
#include <vxWorks.h>
#include <cacheLib.h>
#include <vmLib.h>
#include <vxLib.h>
#include <memLib.h>
#include <sysLib.h>
#include <intLib.h>
#include "config.h"

#include <sal/appl/pci.h>

#include <soc/debug.h>

#include <sal/appl/io.h>

#if (defined(BCM_ICS) || defined(IPROC_CMICD))
#define pciIntConnect intConnect

int pci_config_putw(pci_dev_t *dev, uint32 addr, uint32 data)
{
    return 0xffffffff;
}
uint32 pci_config_getw(pci_dev_t *dev, uint32 addr)
{
    return 0xffffffff;
}

#else
/*
 * Write a DWORD (32 bits) of data to PCI configuration space
 * at the specified offset.
 */

int pci_config_putw(pci_dev_t *dev, uint32 addr, uint32 data)
{
    STATUS requestStatus;

    LOG_VERBOSE(BSL_LS_SOC_PCI,
                (BSL_META("PCI(%d,%d,%d) configW(0x%x)=0x%x\n"),
                 dev->busNo, dev->devNo, dev->funcNo, addr, data));

    assert(! (addr & 3));

    requestStatus = pciConfigOutLong(dev->busNo,
				     dev->devNo,
				     dev->funcNo,
				     (int) addr,
				     (UINT32) data);
    return requestStatus;
}

/*
 * Read a DWORD (32 bits) of data from PCI configuration space
 * at the specified offset.
 */

uint32 pci_config_getw(pci_dev_t *dev, uint32 addr)
{
    UINT32 inWord = 0;
    STATUS requestStatus;

    assert(! (addr & 3));

    LOG_VERBOSE(BSL_LS_SOC_PCI,
                (BSL_META("PCI(%d,%d,%d) configR(0x%x)="),
                 dev->busNo, dev->devNo, dev->funcNo, addr));

    requestStatus = pciConfigInLong(dev->busNo,
				    dev->devNo,
				    dev->funcNo,
				    addr,
				    (UINT32*) &inWord);

    if (requestStatus != OK) {
	cli_out("ERROR: PCI configuration read bus=%d dev=%d func=%d "
                "(0x%x=0x%x) -READ ERROR(%d)\n",
                dev->busNo, dev->devNo, dev->funcNo, addr, inWord, 
                requestStatus);
	return -1;
    } else {
        LOG_VERBOSE(BSL_LS_SOC_PCI,
                    (BSL_META("0x%x\n"), inWord));
	return inWord;
    }
}
#endif

/*
 * Write a DWORD (32 bits) of data to PCI memory space
 * at the specified offset. Return last value at address.
 * NOTE: Deprecated routine
 */

int pci_memory_putw(pci_dev_t *dev, uint32 addr, uint32 data)
{
    *(uint32 *)INT_TO_PTR(addr) = data;
    return 0;
}

/*
 * Read a DWORD (32 bits) of data from PCI memory space.
 * NOTE: Deprecated routine
 */

uint32 pci_memory_getw(pci_dev_t *dev, uint32 addr)
{
    return *(uint32 *)INT_TO_PTR(addr);
}

/*
 * pci_int_connect
 *
 *   Adds an interrupt service routine to the ISR chain for a
 *   specified interrupt line.
 */

int pci_int_connect(int intLine,
		    pci_isr_t isr,
		    void *isr_data)
{
#if !defined(NEGEV)
    extern int sysVectorIRQ0;
#endif

#if defined(NSX) || defined(GTO) || defined(METROCORE)
    int i;
#endif
  
    LOG_VERBOSE(BSL_LS_SOC_PCI,
                (BSL_META("pci_int_connect: intLine=%d, isr=%p, isr_data=%p\n"),
                 intLine, (void *)isr, (void *)isr_data));

#if defined(NSX) || defined(METROCORE)
    /* all int lines */
    for (i = 0; i < 4; i++) {
        /* cli_out("pciIntConnect int line = %d\n", intLine + i); */
        if (pciIntConnect ((VOIDFUNCPTR *)
                     INUM_TO_IVEC(sysVectorIRQ0 + intLine + i),
                     (VOIDFUNCPTR) isr,
                     PTR_TO_INT(isr_data)) != OK) {
            return -1;
        }
    }
    return 0;
#endif

#if defined(GTO)
    for (i = 0; i < 4; i++) {
        /* PCI interrupts are connected at external interrupt
         * 0, 1, 2, and 3.
         */ 
        if (pciIntConnect ((VOIDFUNCPTR *)
                     INUM_TO_IVEC(sysVectorIRQ0 + i),
                     (VOIDFUNCPTR) isr,
                     PTR_TO_INT(isr_data)) != OK) {
            return -1;
        }

        if (intEnable(sysVectorIRQ0 + i) == ERROR) {
	    return -1;
        }
    }
    return 0;
#endif

#if !defined (NEGEV)
    if (pciIntConnect ((VOIDFUNCPTR *)
		       INUM_TO_IVEC(sysVectorIRQ0 + intLine),
		       (VOIDFUNCPTR) isr,
		       PTR_TO_INT(isr_data)) != OK) {
      return -1;
    }
#ifdef IPROC_CMICD
    if (intEnable(sysVectorIRQ0 + intLine) == ERROR) {
        return -1;
    }
#endif
#endif

#if (CPU_FAMILY != PPC) && !defined(IDTRP334) && \
    !defined(MBZ) && !defined(IDT438) && !defined(NSX) && !defined(ROBO_4704) \
    && !defined(ROBO_4702) && !defined(RAPTOR) && !defined(METROCORE) \
    && !defined(KEYSTONE)
    if (sysIntEnablePIC(intLine) == ERROR) {
	return -1;
    }
#endif
    
#if (CPU_FAMILY == PPC)
    if (intEnable(intLine) != OK) {
	return -1;
    }
#endif /* (CPU_FAMILY == PPC) */

    return 0;
}
