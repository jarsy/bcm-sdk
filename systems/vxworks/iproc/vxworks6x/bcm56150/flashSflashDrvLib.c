/*
 * $Id: flashSflashDrvLib.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    flashSflashDrvLib.c
 */
#include "vxWorks.h"
#include "taskLib.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "config.h"
#include "flashDrvLib.h"
#include "sflash.h"

extern struct flash_drv_funcs_s flashsflash;

LOCAL void
flashAutoSelect(FLASH_TYPES *dev, FLASH_VENDORS *vendor)
{
    *vendor = *dev = 0xFF;
}

LOCAL int
flashEraseDevices(volatile unsigned char *sectorBasePtr)
{
    int ret = 0;

    if (flashVerbose) {
        printf("Erasing Sector @ 0x%08x\n",(unsigned int)sectorBasePtr);
    }

    /* Erase block */
    if ((ret = sflash_erase((unsigned int)sectorBasePtr)) < 0) {
        return (ERROR);
    }

    if (flashVerbose > 1)
        printf("flashEraseDevices(): all devices erased\n");

    return (OK);
}

LOCAL int
flashEraseSector(int sectorNum)
{
    unsigned char *sectorBasePtr = (unsigned char *)(sectorNum * FLASH_SECTOR_SIZE);

    if (sectorNum < 0 || sectorNum >= flashSectorCount) {
        printf("flashEraseSector(): Sector %d invalid\n", sectorNum);
        return (ERROR);
    }

    if (flashEraseDevices(sectorBasePtr) == ERROR) {
        printf("flashEraseSector(): erase devices failed sector=%d\n",
            sectorNum);
        return (ERROR);
    }

    if (flashVerbose)
        printf("flashEraseSector(): Sector %d erased\n", sectorNum);

    return (OK);
}

LOCAL int
flashRead(int sectorNum, char *buff, unsigned int offset, unsigned int count)
{
    unsigned char *curBuffPtr, *sectorOffsetPtr;
    unsigned int len = count;
    int bytes;

    if (sectorNum < 0 || sectorNum >= flashSectorCount) {
        printf("flashRead(): Illegal sector %d\n", sectorNum);
        return (ERROR);
    }

    curBuffPtr = (unsigned char *)buff;
    sectorOffsetPtr = (unsigned char *)((sectorNum * FLASH_SECTOR_SIZE) + offset);

    /* Read holding block */
    while (len) {
        if ((bytes = sflash_read((unsigned int)sectorOffsetPtr, len, curBuffPtr)) < 0) {
            printf("flashRead(): Failed: Sector %d, address 0x%x\n",
                sectorNum, (int)sectorOffsetPtr);
            return (ERROR);
        }
        sectorOffsetPtr += bytes;
        len -= bytes;
        curBuffPtr += bytes;
    }

    return (OK);
}

LOCAL int
flashWrite(int sectorNum, char *buff, unsigned int offset, unsigned int count)
{
    unsigned char *curBuffPtr, *sectorOffsetPtr;
    unsigned int len = count;
    int bytes;

    curBuffPtr = (unsigned char *)buff;
    sectorOffsetPtr = (unsigned char *)((sectorNum * FLASH_SECTOR_SIZE) + offset);

    /* Write holding block */
    while (len) {
        if ((bytes = sflash_write((unsigned int)sectorOffsetPtr, len, curBuffPtr)) < 0) {
            printf("flashWrite(): Failed: Sector %d, address 0x%x\n",
                sectorNum, (int)sectorOffsetPtr);
            return (ERROR);
        }

        sectorOffsetPtr += bytes;
        len -= bytes;
        curBuffPtr += bytes;
    }

    return (OK);
}

struct flash_drv_funcs_s flashsflash = {
    (FLASH_TYPES)0, (FLASH_VENDORS)0,
    flashAutoSelect,
    flashEraseSector,
    flashRead,
    flashWrite
};
