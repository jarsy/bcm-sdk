/*
 * $Id: socdiag.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <vxWorks.h>
#include <version.h>
#include <kernelLib.h>
#include <sysLib.h>

#include <sal/appl/sal.h>
#include <soc/debug.h>
#include <sal/appl/io.h>
#include <sal/core/boot.h>
#include <sal/core/thread.h>


/* 
 * Create the iProc(KT2/HR2) bde
 */

#include "bde/vxworks/vxbde.h"

#define IPROC_CMICD_BASE        0x48000000
#define IPROC_CMICD_INT         194

ibde_t *bde;

int
bde_create(void)
{	
    vxbde_bus_t bus; 

    bus.base_addr_start = IPROC_CMICD_BASE;
    bus.int_line = IPROC_CMICD_INT;

    /* Little Endian */
    bus.be_pio = 0;
    bus.be_packet = 0;
    bus.be_other = 0;

    return vxbde_create(&bus, &bde);
}

/*
 * Main: start the diagnostics and CLI shell under vxWorks.
 */
void vxSpawn(void)
{
    extern void diag_shell(void *);

    sal_core_init();
    sal_appl_init();

    printf("SOC BIOS (VxWorks) %s v%s.\n", sysModel(), vxWorksVersion);
    printf("Kernel: %s.\n", kernelVersion());
    printf("Made on %s.\n", creationDate);
    sal_thread_create("bcmCLI", 128*1024,100, diag_shell, 0);
}
