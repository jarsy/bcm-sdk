/*
 * $Id: socdiag.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <vxWorks.h>
#include <version.h>
#include <kernelLib.h>
#include <sysLib.h>
#include <bmw.h>

#include <sal/appl/sal.h>
#include <soc/debug.h>
#include <sal/appl/io.h>
#include <sal/core/boot.h>
#include <sal/core/thread.h>

/*
 * NOTE: The LED blink process in usrConfig.c blinks the LED at a
 * rate dependent on system load. This task simply shows BCM with
 * a swirl on the LCD display.
 */

/*ARGSUSED*/
static void
led_beat(void *p)
{
    uint32 period = (1000000 / 3);

    if (sysIsCFM()) {
	for (;;) {
            if ((pldRegRead(CFM_REG_CFM_PRESENCE) & CFM_PRESENCE_OTHER) == 0) {
                if ((pldRegRead(CFM_REG_CFM_RESET) & CFM_RESET_SLOT_ID) == 0) {
                    /* Slot-1 Master, Status up, power on*/
                    pldRegWrite(CFM_REG_STATUS_LEDS, CFM_STATUS_ALL_GREEN);
                } else {
                    /* Slot-2 !Master, Status up, power on*/
                    pldRegWrite(CFM_REG_STATUS_LEDS,
				(CFM_STATUS_STATUS_GREEN |
				 CFM_STATUS_POWER_GREEN));
                }
            } else { /* Only one CFM active */
                pldRegWrite(CFM_REG_STATUS_LEDS, CFM_STATUS_ALL_GREEN);
            }
            sal_usleep(period);
	}
    }

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
 * Create the BMW/MPC8245 bde
 */

#include "bde/vxworks/vxbde.h"

ibde_t *bde;

int
bde_create(void)
{
    vxbde_bus_t bus;
    bus.base_addr_start = 0x81100000;
    bus.int_line = 2;
    bus.be_pio = 1;
    bus.be_packet = 0;
    bus.be_other = 1;
    return vxbde_create(&bus, &bde);
}

/*
 * Main: start the diagnostics and CLI shell under vxWorks.
 */
void
vxSpawn(void)
{
    extern void diag_shell(void *);

#ifdef WIND_DEBUG
    IMPORT taskDelay(int ticks);
    printf("Waiting for CrossWind attach ...\n");
    taskDelay(sysClkRateGet()*60);
#endif
    sal_core_init();
    sal_appl_init();

    printf("SOC BIOS (VxWorks) %s v%s.\n", sysModel(), vxWorksVersion);
    printf("Kernel: %s.\n", kernelVersion());
    printf("Made on %s.\n", creationDate);
    sal_thread_create("bcmCLI", 128*1024,100, diag_shell, 0);
    sal_thread_create("bcmLED", 8192, 99, led_beat, 0);
}

