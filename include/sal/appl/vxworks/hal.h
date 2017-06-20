/*
 * $Id: hal.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef __BCM_VXWORKS_HAL_H
#define __BCM_VXWORKS_HAL_H

typedef struct platform_hal_s {
    char                *name;

    unsigned int        flags;

    /* Platform Capablities */
#define PLATFORM_CAP_MULTICORE        0x000001
#define PLATFORM_CAP_ENDIAN_FUNNY     0x000002
#define PLATFORM_CAP_DMA_MEM_UNCACHABLE   0x000004
#define PLATFORM_CAP_FLASH_FS         0x000100 /* Flash filesystem implemented */
#define PLATFORM_CAP_WATCHDOG         0x000200
#define PLATFORM_CAP_PCI              0x010000
    unsigned int        caps;

    /* Platform BUS flags TBD */
    unsigned int        bus_caps;
    
    /* Dump Platform Info */
    int     (*f_dump_info)(void);

    /* Upgrade Images */
#define IMAGE_F_BOOT      0x01 /* Only boot images for now */
    int     (*f_upgrade_image)(char *imageName, unsigned int flags, 
                               int (*f_loader)(char *fname, char *fbuf, 
                                               int bufSize, int *entry_pt));

    /* Flash filesystem device name */
    char *  (*f_flash_device_name)(void);

#define FLASH_FORMAT_DOS        0x01
    int     (*f_format_fs)(int format, unsigned int flags);
    int     (*f_fs_sync)(void);     /* Sync Filesystem */
    int     (*f_fs_check)(void);    /* Check Filesystem */

    /* TOD (time of Day) Set interface */
    int     (*f_tod_set)(int year, int month, int day, 
                         int hour, int min, int sec);

    /* TOD (time of Day) Get interface */
    int     (*f_tod_get)(int *year, int *month, int *day,
                         int *hour, int *min, int *sec);

    /* WatchDog */
    int     (*f_start_wdog)(unsigned int usec);

    /* Reboot */
#define REBOOT_COLD         0
#define REBOOT_WARM         1
#define REBOOT_SHUT         2
    int     (*f_reboot)(int opt);

    
    
    
    /* Led functions */
    int     (*f_led_write_string)(const char *s);

    /* get timestamp value(number of ticks since boot) and frequency */
    int (*f_timestamp_get)(uint32 *up,     /* upper 32bit timestamp value */
                           uint32 *low,    /* lower 32bit timestamp value*/
                           uint32 *freq);  /* number of ticks per second */

    /* i2c operation using cpu i2c controller */
    int (*f_i2c_op)(int unit,       /* I2C controller number */
                    uint32 flags,   /* rd/wr options and other options */
                    uint16 slave,   /* slave address on the I2C bus */
                    uint32 addr,    /* internal address on slave */
                    uint8 addr_len, /* length of address in bytes */
                    uint8 *buf,     /* buffer to hold the read data */
                    uint8 buf_len); /* buffer length */
/* i2c operation flags */
#define HAL_I2C_WR              0x0 /* This is a bit flag and is the default. 
                                    * RD bit not set implies write 
                                    */
#define HAL_I2C_RD              0x1  
#define HAL_I2C_FAST_ACCESS     0x2
} platform_hal_t;

extern platform_hal_t *platform_info;

#define SAL_IS_PLATFORM_INFO_VALID     (platform_info != NULL)

int platform_attach(platform_hal_t **platform_info);

#endif /* __BCM_VXWORKS_HAL_H */

