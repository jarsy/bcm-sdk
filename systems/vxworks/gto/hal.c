/*
 * $Id: hal.c,v 1.10 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <vxWorks.h>
#include <version.h>
#include <kernelLib.h>
#include <sysLib.h>
#include <gto.h>
#include <stdlib.h>
#include <dosFsLib.h>
#include <intLib.h>
#include <vxLib.h>

#include <sal/appl/sal.h>
#include <sal/appl/config.h>
#include <sal/appl/vxworks/hal.h>
/* #include "../../../include/sal/appl/vxworks/hal.h" */
#include "flashDrvLib.h"
#include "flashFsLib.h"
#include "srecLoad.h"
#include "i2cM41T81Clock.h"
#include "sysMotI2c.h"

static int gto_upgrade_image(char *fname, unsigned int flags, 
                 int (*f_loader)(char *fname, char *fbuf, 
                                 int bufSize, int *entry_pt))
{
    char		*buf = 0;
    int			rv = -1;
    int			i = 0;
    int			entry;

    /* Re-probe in case jumper moved */
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

static char * gto_flash_dev_name(void)
{
    return FLASH_FS_NAME;
}

static int gto_format_flash(int format, unsigned int flags)
{
    int	rv = 0;

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
        printf("GTO Flash Formatted .\n");
        flashFsSync();
    }

    return(rv);
}

static int gto_timestamp_get (uint32 *up, uint32 *low,uint32 *freq)
{
    /* 
     * GTO platform ensures that the CCB operates at 400MHz
     * The CCB frequency is divided by 8 before being fed to the timebase
     * So, the frequency is 50MHz (counter granularity is 20ns)
     */
    *freq = 50000000;
    vxTimeBaseGet(up,low);
    return OK;
}

/*
 * Function     : gto_i2c_op
 * Purpose      : Implement I2C rd/wr functionality
 * Parameters   : unit      : I2C controller number
 *                flags     : rd/wr options and other options
 *                slave     : slave address on the I2C bus
 *                addr      : internal address on slave 
 *                addr_len  : length of address in bytes
 *                buf       : buffer to hold the read data
 *                buf_len   : buffer length
 * Returns      : 0 - on success, -1 - on failure
 */
static int gto_i2c_op(int unit, uint32 flags, uint16 slave, uint32 addr, 
                       uint8 addr_len, uint8 *buf, uint8 buf_len)
{
#if VX_VERSION == 65
    int             rv, command;
    i2cCmdPckt_t    cmd_pkt;    /* command packet */

    if ((buf == NULL) || (buf_len == 0)) {
        return ERROR;
    }

    if (flags & HAL_I2C_FAST_ACCESS) {
        I2C_FAST_ACCESS_SET(1);
    } else {
        I2C_FAST_ACCESS_SET(0);
    }

    if (flags & HAL_I2C_RD) {
        command = I2C_READOP;
    } else {
        /* default is WR */
        command = I2C_WRITOP;
    }

    /* Build command packet. */
    cmd_pkt.command = command;
    cmd_pkt.status = 0;
    cmd_pkt.memoryAddress = (unsigned int)buf;
    cmd_pkt.blockNumber = addr;
    cmd_pkt.blockSize = addr_len;
    cmd_pkt.nBlocks = buf_len;
    cmd_pkt.eCount = buf_len;
    cmd_pkt.aCount = 0;
    cmd_pkt.deviceType = I2C_DEVICE_TYPE_GENERIC;

    rv = i2cDoOp(unit, slave, &cmd_pkt);
    
    /* Return the appropriate status. */
    if (cmd_pkt.status != 0) {
        printf("i2cDoOp failed. status:%d \n", cmd_pkt.status);
        rv = ERROR;
	} else {
        rv = OK;
    }
    return rv;
#else
    printf("gto_i2c_op: not implemented for VxWorks: %d \n", VX_VERSION);
    return ERROR;
#endif 
}

static platform_hal_t gto_hal_info = {
    "GTO",                                         /* name */
    0,                                             /* flags */
    (PLATFORM_CAP_PCI | PLATFORM_CAP_FLASH_FS),    /* caps */
    0,                                             /* bus_caps */
    NULL,                                          /* f_dump_info */
    gto_upgrade_image,                             /* f_upgrade_image */
    gto_flash_dev_name,                            /* f_flash_device_name */
    gto_format_flash,                              /* f_format_fs */
    flashFsSync,                                   /* f_fs_sync */
    NULL,                                          /* f_fs_check */
    m41t81_tod_set,                                /* f_tod_set */
    m41t81_tod_get,                                /* f_tod_get */
    NULL,                                          /* f_start_wdog */
    NULL,                                          /* f_reboot */
    NULL,                                          /* f_led_write_string */
    gto_timestamp_get,                             /* f_timestamp_get */
    gto_i2c_op                                     /* f_i2c_op */
};

int platform_attach(platform_hal_t **platform_info)
{
    if (!platform_info) {
        return -1;
    }

    *platform_info = &gto_hal_info;
    return 0;
}

