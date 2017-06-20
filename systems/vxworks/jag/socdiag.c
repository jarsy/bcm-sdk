/*
 * $Id: socdiag.c,v 1.11 Broadcom SDK $
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
static void
led_beat(void *p)
{
    uint32 period = (1000000 / 3);

    if ((SYS_REVID_GET() == BOARD_ID_LM_1) ||
       (SYS_REVID_GET() == BOARD_ID_LM_P48))
        return;

    for (;;) {
	sal_led_string("BCM\\");
	sal_usleep(period / 4);
	sal_led_string("BCM|");
	sal_usleep(period / 4);
	sal_led_string("BCM/");
	sal_usleep(period / 4);
	sal_led_string("BCM-");
	sal_usleep(period / 4);
    }
}

/* 
 * Create the BCM 47XX MIPS bde
 */

#include "bde/vxworks/vxbde.h"

ibde_t *bde;

int
bde_create(void)
{	
    vxbde_bus_t bus; 
    bus.base_addr_start = 0xa8100000;
    bus.int_line = 4;
#ifdef MIPSEL
    bus.be_pio = 0;
    bus.be_packet = 0;
    bus.be_other = 0;
#else
    bus.be_pio = 0;
    bus.be_packet = 1;
    bus.be_other = 0;
#endif
    if (SYS_REVID_GET() == BOARD_ID_JAG_BCM56218_EB) {
        return vxbde_eb_create(&bus, &bde, 0xBA800000);
    } else {
        return vxbde_create(&bus, &bde);
    }
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
    sal_thread_create("bcmLED", 8192, 99, led_beat, 0);
}
