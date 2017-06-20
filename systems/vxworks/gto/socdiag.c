/*
 * $Id: socdiag.c,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <vxWorks.h>
#include <version.h>
#include <kernelLib.h>
#include <sysLib.h>
#include <gto.h>
#include <stdlib.h>

#include <sal/appl/sal.h>
#include <soc/debug.h>
#include <sal/appl/io.h>
#include <sal/core/boot.h>
#include <sal/core/thread.h>
#include <shared/shr_bprof.h>

/*
 * NOTE: The LED blink process in usrConfig.c blinks the LED at a
 * rate dependent on system load. This task simply shows BCM with
 * a swirl on the LCD display.
 */
extern void sysLedOn();
extern void sysLedOff();
/*ARGSUSED*/
static void
led_beat(void *p)
{
    uint32 period = (1000000 / 3);

    for (;;) {
        sysLedOn(); 
        sal_usleep(period);
        sysLedOff();
        sal_usleep(period);
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
    bus.base_addr_start = 0x00000000;
    bus.int_line = 2;
    bus.be_pio = 1;
    bus.be_packet = 0;
    bus.be_other = 1;

    return vxbde_create(&bus, &bde);
}

int 
bde_mem_test(void)
{
    unsigned int *mem;
    size_t mem_size = 0x20000000; 
    size_t element; 
    size_t count; 

#define MEM_SIZE_64MB  0x4000000

   
    mem = NULL;
    do {
        mem_size = mem_size - MEM_SIZE_64MB;

        mem = malloc(mem_size);
    } while ((mem == NULL) && (mem_size != 0));

    if (mem != NULL) {
        count = mem_size / sizeof(unsigned int);

        printf ("Tesing 0x%08x bytes of memory.\n", mem_size);
        printf ("Start at 0x%08x, end at 0x%08x\n", &mem[0], &mem[count]);
 
        for (element = 0; element < count; element++) {
            mem[element] = (unsigned int)&mem[element];
        }
    
        for (element = 0; element < count; element++) {
            if (mem[element] != (unsigned int)&mem[element]) {
                printf("mem comparison failed at 0x%08x = 0x%08x\n",
                       &mem[element], mem[element]);
            }
        }
        free(mem);
    }
    return 0;
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

#ifdef BCM_BPROF_STATS
    shr_bprof_stats_time_init();
#endif

    printf("SOC BIOS (VxWorks) %s v%s.\n", sysModel(), vxWorksVersion);
    printf("Kernel: %s.\n", kernelVersion());
    printf("Made on %s.\n", creationDate);
    sal_thread_create("bcmCLI", 128*1024,100, diag_shell, 0);
    sal_thread_create("bcmLED", 8192, 99, led_beat, 0);
}

