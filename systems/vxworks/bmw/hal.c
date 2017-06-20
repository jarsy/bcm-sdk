/*
 * $Id: hal.c,v 1.7 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <vxWorks.h>
#include <version.h>
#include <kernelLib.h>
#include <sysLib.h>
#include <bmw.h>
#include <stdlib.h>
#include <dosFsLib.h>
#include <vxLib.h>

#include <sal/appl/sal.h>
#include <sal/appl/config.h>
#include <sal/appl/vxworks/hal.h>
#include "flashLib.h"
#include "flashFsLib.h"
#include "srecLoad.h"

static int bmw_upgrade_image(char *fname, unsigned int flags, 
                 int (*f_loader)(char *fname, char *fbuf, 
                                 int bufSize, int *entry_pt))
{
    char		*buf = 0;
    int			rv = -1;
    int			i = 0;
    int			entry;
    int                 bootImgSize = 512 * 1024;

    flashLibInit(); /* Re-probe in case jumper moved */

    if (!FLASH_DEV_BANK0_BOOT->found) {
	printf("flashBoot: Boot flash not found (jumpered right?)\n");
	goto done;
    }

    if ((buf = malloc(bootImgSize)) == 0) {
	printf("flashBoot: out of memory\n");
	goto done;
    }

    if (f_loader(fname, buf, bootImgSize, &entry) < 0) {
	printf("flashBoot: Failed to read image.\n");
	goto done;
    }

    printf("%d\nErasing boot area ...", i);

    for (i = 0; i < FLASH_DEV_BANK0_BOOT->sectors; i++) {
	if (flashEraseSector(FLASH_DEV_BANK0_BOOT, i) < 0) {
	    printf("\nflashBoot: failed erasing -- PROM DESTROYED\n");
	    return ERROR;
	}

	printf(".");
    }

    printf("done\nWriting boot data ...");

    for (i = 0; i < FLASH_DEV_BANK0_BOOT->sectors; i++) {
	if (flashWrite(FLASH_DEV_BANK0_BOOT,
		       i * (1 << FLASH_DEV_BANK0_BOOT->lgSectorSize),
		       buf + i * (1 << FLASH_DEV_BANK0_BOOT->lgSectorSize),
		       (1 << FLASH_DEV_BANK0_BOOT->lgSectorSize)) < 0) {
	    printf("\nflashBoot: failed writing -- PROM DESTROYED\n");
	    return ERROR;
	}

	printf(".");
    }

    printf("\nDone\n");

    rv = 0;

 done:

    if (buf) {
	free(buf);
    }

    return rv;
}

static char * bmw_flash_dev_name(void)
{
    return FLASH_FS_NAME;
}

#define	LOCAL_FS_PFX ((sysHasDOC()) ? "flsh:":"flash:")

static int bmw_format_flash(int format, unsigned int flags)
{
    int	rv = 0;

    if (format) {
	extern STATUS tffsPPCFormat(void);
        if (sysHasDOC()) {
            tffsPPCFormat();
        } else {
# ifdef DOS_OPT_DEFAULT
            if (dosFsVolFormat(LOCAL_FS_PFX, DOS_OPT_DEFAULT, 0)) {
                rv |= -1;
            }
# else
            if (diskInit(LOCAL_FS_PFX)) {
                rv |= -1;
            }
# endif
            flashFsSync();
        }
    }

    return(rv);
}

static int bmw_arm_wdog(unsigned int usec)
{
    SYS_WATCHDOG_RESET();               /* Configure to reset system */
    return sysTodWatchdogArm(usec);
}

extern int sysRamToCpuClkGet (void);
extern int sysPciToRamClkGet (void);

static int bmw_print_info(void)
{
    UINT32	tb, pci, cpu = 0;
    int         ramClk;
    int         factor;

    tb = sysMeasureTimebase ();

    /* 
    * The system logic clock is the RAM clock which runs at
    * 4 times the decrementer rate
    */
    ramClk = tb << 2;

    /* 
    * The ramClk is some multiple of the pciClk.  Find the multiple. 
    * Note: the ClkGet() functions return the multiplier*2 in order
    * to avoid floating point arithmetic.  The BSP reads the PLL config
    * register to get the settings.  Can't use HID1 like mousse-- it's 
    * non-deterministic on the 8245.
    */

    factor = sysPciToRamClkGet ();
    pci = 2 * ramClk / factor;

    /* The CPU clock is a multiple of the RAM clock. */

    factor = sysRamToCpuClkGet();
    cpu = ramClk * (factor >> 1);
    if (factor & 0x1)   /* take care of the .5 multipliers */
       cpu += (ramClk >> 1);
    
    printf("Timebase: %d.%06d MHz, MEM: %d.%06d MHz, PCI: %d.%06d MHz, CPU: %d.%06d MHz\n",
	   tb / 1000000, tb % 1000000,
	   ramClk / 1000000, ramClk % 1000000,
	   pci / 1000000, pci % 1000000,
	   cpu / 1000000, cpu % 1000000);

    return 0;
}

static int bmw_led_write_string(const char * s)
{
#define MPC824X_ALPHAN_CHAR0	((volatile char *) 0x7c002003)
#define MPC824X_ALPHAN_CHAR1	((volatile char *) 0x7c002002)
#define MPC824X_ALPHAN_CHAR2	((volatile char *) 0x7c002001)
#define MPC824X_ALPHAN_CHAR3	((volatile char *) 0x7c002000)
    *MPC824X_ALPHAN_CHAR0 = *s ? *s++ : ' ';
    *MPC824X_ALPHAN_CHAR1 = *s ? *s++ : ' ';
    *MPC824X_ALPHAN_CHAR2 = *s ? *s++ : ' ';
    *MPC824X_ALPHAN_CHAR3 = *s ? *s++ : ' ';

    return 0;
}

static int bmw_timestamp_get (uint32 *up, uint32 *low,uint32 *freq)
{
#if (VX_VERSION != 54)
    *freq = sysMeasureTimebase();
    if (!(*freq)) {
        return ERROR;
    }

    vxTimeBaseGet(up,low);
    return OK;
#else
    return ERROR;
#endif
}

static platform_hal_t bmw_hal_info = {
    "BMW",                                         /* name */
    0,                                             /* flags */
    (PLATFORM_CAP_PCI | PLATFORM_CAP_FLASH_FS),    /* caps */
    0,                                             /* bus_caps */
    bmw_print_info,                                /* f_dump_info */
    bmw_upgrade_image,                             /* f_upgrade_image */
    bmw_flash_dev_name,                            /* f_flash_device_name */
    bmw_format_flash,                              /* f_format_fs */
    flashFsSync,                                   /* f_fs_sync */
    NULL,                                          /* f_fs_check */
    sysTodSet,                                     /* f_tod_set */
    sysTodGet,                                     /* f_tod_get */
    bmw_arm_wdog,                                  /* f_start_wdog */
    NULL,                                          /* f_reboot */
    bmw_led_write_string,                          /* f_led_write_string */
    bmw_timestamp_get,                             /* f_timestamp_get */
    NULL                                           /* f_i2c_op */
};

int platform_attach(platform_hal_t **platform_info)
{
    if (!platform_info) {
        return -1;
    }

    *platform_info = &bmw_hal_info;
    return 0;
}

