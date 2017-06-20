/*
 * $Id: socdiag.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <vxWorks.h>
#include <version.h>
#include <kernelLib.h>
#include <sysLib.h>
#include <mbz.h>

#include <sal/appl/sal.h>
#ifdef MODENA_CHIP
#include <soc/modena/debug.h>
#else
#include <soc/debug.h>
#endif
#include <sal/appl/io.h>
#include <sal/core/boot.h>
#include <sal/core/thread.h>


/*ARGSUSED*/

/* 
 * Create the BCM 47XX MIPS bde
 */

#include "bde/vxworks/vxbde.h"

ibde_t *bde;

int
bde_create(void)
{	
    vxbde_bus_t bus; 
    bus.base_addr_start = 0xa8000000;
    bus.int_line = 0;
#ifdef MIPSEL
    bus.be_pio = 1;
    bus.be_packet = 0;
    bus.be_other = 0;
#else
    bus.be_pio = 0;
    bus.be_packet = 1;
    bus.be_other = 0;
#endif
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
