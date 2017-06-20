/*
 * $Id: socdiag.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <vxWorks.h>
#include <version.h>
#include <kernelLib.h>
#include <sysLib.h>
#include <stdlib.h>
/* include <negev.h> */
#include <usrConfig.h>

#include <sal/appl/sal.h>
#include <soc/debug.h>
#include <sal/appl/io.h>
#include <sal/appl/config.h>
#include <sal/core/boot.h>
#include <sal/core/thread.h>

/*
 * NOTE: The LED blink process in usrConfig.c blinks the LED at a
 * rate dependent on system load. This task simply shows BCM with
 * a swirl on the LCD display.
 */

#if NEGEV_MISSING_FEATURE
extern void sysLedOn();
extern void sysLedOff();
#else
void sysLedOn(void) { } ;
void sysLedOff(void) { };
#define ROOT_TASK_PRIORITY      198
#define STARTER_TASK_STACK_SIZE 200000
#define SOC_CLI_THREAD_PRI 100
STATUS
   starterProc(void);
#endif

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
 * Create the Negev/MPC860 bde
 */

#include "bde/vxworks/vxbde.h"

ibde_t *bde;

int
bde_create(void)
{
    vxbde_bus_t bus;
    bus.base_addr_start = 0x40000000;
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
        printf ("Start at 0x%08x, end at 0x%08x\n", (unsigned int)&mem[0], (unsigned int)&mem[count]);
 
        for (element = 0; element < count; element++) {
            mem[element] = (unsigned int)&mem[element];
        }
    
        for (element = 0; element < count; element++) {
            if (mem[element] != (unsigned int)&mem[element]) {
                printf("mem comparison failed at 0x%08x = 0x%08x\n",
                       (unsigned int)&mem[element], (unsigned int)mem[element]);
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

    sal_config_set("eb_probe", "1");
    sal_config_set("eb_dev_addr.0", "0x40000000");
    sal_config_set("eb_bus_read_8bit.0", "1");
    sal_config_set("eb_bus_write_8bit.0", "1");

    printf("SOC BIOS (VxWorks) %s v%s.\n", sysModel(), vxWorksVersion);
    printf("Kernel: %s.\n", kernelVersion());
    printf("Made on %s.\n", creationDate);
    sal_thread_create("bcmCLI", 128*1024, SOC_CLI_THREAD_PRI, diag_shell, 0);
    sal_thread_create("bcmLED", 8192, 99, led_beat, 0);
}

STATUS
progStart()
{
    int ix, consoleFd;
    char consoleName [20];

    for (ix = 0; ix < 2; ix++) {
         sprintf (consoleName, "%s%d", "/tyCo/", ix);
         /* (void) tyCoDevCreate (consoleName, ix, 512, 512); */
         consoleFd = open (consoleName, O_RDWR, 0);

         /* set baud rate */

         (void) ioctl (consoleFd, FIOBAUDRATE, 38400);
         (void) ioctl (consoleFd, FIOSETOPTIONS, OPT_TERMINAL);

    }

    printf("progStart: Complete\n") ;

    sal_core_init();
    sal_appl_init();

  /* from ref_design_starter.c */
  taskSpawn(
      "stProcTask", 50, 0, STARTER_TASK_STACK_SIZE,(FUNCPTR)starterProc,
      0,0,0,0,0,0,0,0,0,0);

  /*
   * Leave this task cycling forever, taking very little
   * CPU time. Use to reconnect Ethernet in case cable was not inserted at startup.
   */
  printf("progStart: Reduce priority of progStartTask\n") ;
  taskPrioritySet(taskIdSelf(),ROOT_TASK_PRIORITY) ; 

  taskDelay(sysClkRateGet()*60);
  vxSpawn();

 	while(1) {
	;
	} 

    printf("progStart: Return\n") ;
    return (OK);
}
