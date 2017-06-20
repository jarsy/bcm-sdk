/*
 * $Id: hal.c,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <vxWorks.h>
#include <version.h>
#include <kernelLib.h>
#include <sysLib.h>
#include <stdlib.h>
#include <dosFsLib.h>

#include <sal/appl/sal.h>
#include <sal/appl/config.h>
#include <sal/appl/vxworks/hal.h>
#include "keystone.h"
#include "flashDrvLib.h"
#include "flashFsLib.h"
#include "srecLoad.h"

#include "i2cM41T81Clock.h"

static int bcm53000_upgrade_image(char *fname, unsigned int flags, 
                 int (*f_loader)(char *fname, char *fbuf, 
                                 int bufSize, int *entry_pt))
{
    char        *buf = 0;
    int         rv = -1;
    int         i = 0;
    int         entry;
#if !defined(NSSIOLE)
    char        tmpc;
#endif /* !NSSIOLE || BCM_ICS */

    if (flashDrvLibInit() == ERROR) {
        printf("flashBoot: Boot flash not found (jumpered right?)\n");
        goto done;
    }

    if ((buf = malloc(FLASH_BOOT_SIZE)) == 0) {
    printf("flashBoot: out of memory\n");
    goto done;
    }

    if (f_loader(fname, buf, FLASH_BOOT_SIZE, &entry) < 0) {
    printf("flashBoot: Failed to read image.\n");
    goto done;
    }

#if !defined(NSSIOLE)
    for(i = 0; i < (512 * 1024); i += 4) {
        tmpc = buf[i];
        buf[i] = buf[i + 3];
        buf[i + 3] = tmpc;

        tmpc = buf[i + 1];
        buf[i + 1] = buf[i + 2];
        buf[i + 2] = tmpc;
    }
#endif

    printf("%d\nErasing boot area ...", i);

    for (i = FLASH_BOOT_START_SECTOR;
         i < FLASH_BOOT_START_SECTOR + FLASH_BOOT_SIZE_SECTORS; i++) {
        printf("FLASH_BOOT_START = 0x%08x\n", FLASH_BOOT_START);
        printf("FLASH_SECTOR_SIZE = %d\n", FLASH_SECTOR_SIZE);
        printf("FLASH_BOOT_START_SECTOR = %d\n", FLASH_BOOT_START_SECTOR);
        printf("FLASH_BOOT_SIZE_SECTORS = %d\n", FLASH_BOOT_SIZE_SECTORS);
    

        if (flashEraseBank(i, 1) != OK) {
        printf("\nflashBoot: failed erasing -- PROM DESTROYED\n");
            goto done;
        }

        printf(".");
    }

    printf("done\nWriting boot data ...");

    if (flashBlkWrite(FLASH_BOOT_START_SECTOR, buf,
              0, FLASH_BOOT_SIZE) != OK) {
        printf("\nflashBoot: failed writing -- PROM DESTROYED\n");
        goto done;
    }

    printf("\nDone\n");

    rv = 0;

 done:

    if (buf) {
    free(buf);
    }

    return rv;
}

static int bcm53000_print_info(void)
{
    int core = 0, mem = 0, sb = 0;

    sysGetClocks(&core, &mem, &sb);

    if (core) {
        printf("CPU: %d MHz, ", core/1000000);
    } else {
        printf("CPU: Unknown Mhz, ");
    }

    if (mem) {
        printf("MEM: %d MHz, ", mem/1000000);
    } else {
        printf("MEM: Unknown Mhz, ");
    }


    if (sb) {
        printf("SB: %d MHz, ", sb/1000000);
    } else {
        printf("SB: Unknown MHz, ");
    }

    return 0;
}


static char * bcm53000_flash_dev_name(void)
{
    return FLASH_FS_NAME;
}

static int bcm53000_format_flash(int format, unsigned int flags)
{
    int rv = 0;
    if (format) {
# ifdef DOS_OPT_DEFAULT
    if (dosFsVolFormat(FLASH_FS_NAME, DOS_OPT_DEFAULT, 0)) {
        rv |= -1;
    }
# else
    if (diskInit(FLASH_FS_NAME)) {
        rv |= -1;
    }
# endif
        flashFsSync();
    }
    return(rv);
}

extern void sys_timestamp_get(uint32 *up, uint32 *low, uint32 *freq);
static int key_timestamp_get (uint32 *up, uint32 *low, uint32 *freq)
{
    sys_timestamp_get(up, low, freq);
    return OK;
}

static platform_hal_t bcm53000_hal_info = {
    "BCM53000",                                    /* name */
    0,                                             /* flags */
    (PLATFORM_CAP_DMA_MEM_UNCACHABLE |
     PLATFORM_CAP_PCI | PLATFORM_CAP_FLASH_FS),    /* caps */
    0,                                             /* bus_caps */
    bcm53000_print_info,                          /* f_dump_info */
    bcm53000_upgrade_image,                       /* f_upgrade_image */
    bcm53000_flash_dev_name,                      /* f_flash_device_name */
    bcm53000_format_flash,                        /* f_format_fs */
    flashFsSync,                                   /* f_fs_sync */
    NULL,                                          /* f_fs_check */
    m41t81_tod_set,                                /* f_tod_set */
    m41t81_tod_get,                                /* f_tod_get */
    NULL,                                          /* f_start_wdog */
    NULL,                                          /* f_reboot */
    NULL,                                          /* f_led_write_string */
    key_timestamp_get,                             /* f_timestamp_get */
    NULL                                           /* f_i2c_op */
};

int platform_attach(platform_hal_t **platform_info)
{
    if (!platform_info) {
        return -1;
    }

    *platform_info = &bcm53000_hal_info;
    return 0;
}

