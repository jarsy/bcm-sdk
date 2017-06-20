#include "vxWorks.h"
#include "taskLib.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "config.h"
#include "flashDrvLib.h"

/* $Id: flash28f128DrvLib.c,v 1.3 2011/07/21 16:14:43 yshtil Exp $
 * INTEL Flash commands
 */
#define INTEL_FLASH_READ_MODE     0xFF
#define INTEL_FLASH_ERASE_BLOCK   0x20
#define INTEL_FLASH_ERASE_CONFIRM 0xD0
#define INTEL_FLASH_PROGRAM       0x40
#define INTEL_FLASH_RD_SR         0x70
#define INTEL_FLASH_CLR_SR        0x50
#define INTEL_FLASH_WB            0xE8
#define INTEL_FLASH_WB_CONFIRM    0xD0
#define INTEL_FLASH_WBSIZE        32 

#define FLASH_ADDR(dev, addr) \
    ((volatile UINT8 *) ((int)(dev) + (int)(addr)))

#define FLASH_WRITE(dev, addr, value) \
    (*FLASH_ADDR(dev, addr) = (value))

#define FLASH_READ(dev, addr) \
    (*FLASH_ADDR(dev, addr))

LOCAL void
flashReadReset(void)
{
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x000, INTEL_FLASH_READ_MODE);
}

LOCAL void
flashAutoSelect(FLASH_TYPES *dev, FLASH_VENDORS *vendor)
{
    flashReadReset();
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x000, 0x90);

    *vendor = FLASH_READ(FLASH_BASE_ADDRESS, 0);
    *dev = FLASH_READ(FLASH_BASE_ADDRESS, 2);

    if (flashVerbose) {
        printf("\nflashAutoSelect(): dev = 0x%x, vendor = 0x%x\n",
               (int)*dev, (int)*vendor);
    }
    flashReadReset();
    if ((*dev != FLASH_2F128) || (*vendor != INTEL)) {
        *vendor = *dev = 0xFF;
    }
}

LOCAL int
flashEraseDevices(volatile UINT8 *sectorBasePtr)
{
    int             i;
    unsigned int    status = 0;

    if (flashVerbose) {
        printf("Erasing Sector @ 0x%08x\n",(UINT32)sectorBasePtr);
    }
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x0, INTEL_FLASH_READ_MODE);
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x0, INTEL_FLASH_CLR_SR);
    FLASH_WRITE(sectorBasePtr, 0x000, INTEL_FLASH_ERASE_BLOCK);
    FLASH_WRITE(sectorBasePtr, 0x000, INTEL_FLASH_ERASE_CONFIRM);
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x0, INTEL_FLASH_RD_SR);

    for (i = 0; i < FLASH_ERASE_TIMEOUT_COUNT; i++) {
        taskDelay(FLASH_ERASE_TIMEOUT_TICKS);

        status = FLASH_READ(FLASH_BASE_ADDRESS, 0x0);

        if ((status & 0x80) == 0x80) {
            break;
        }
    }

    flashReadReset();
    if ((status & 0xa0) != 0x80) {
        printf("flashEraseDevices(): addr 0x%08x erase failed\n",
           (int)sectorBasePtr);
        return (ERROR);
    }
    return (OK);
}

LOCAL int
flashEraseSector(int sectorNum)
{
    UINT8 *sectorBasePtr = (UINT8 *)FLASH_SECTOR_ADDRESS(sectorNum);

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
flashRead(int sectorNum, char *buff,
    unsigned int offset, unsigned int count)
{
    flashReadReset();
    if (sectorNum < 0 || sectorNum >= flashSectorCount) {
        printf("flashRead(): Illegal sector %d\n", sectorNum);
        return (ERROR);
    }

    bcopy((char *)(FLASH_SECTOR_ADDRESS(sectorNum) + offset), buff, count);

    return (0);
}

LOCAL int
flashProgramBlock(volatile UINT8 *addr, UINT8 *val)
{
    int                 polls;
    int                 i;
    unsigned char       status;

    FLASH_WRITE(addr, 0x000, INTEL_FLASH_WB);
    /*
     * XSR7 transitions to a 1, the buffer is ready for loading.
     */
    while (((status = FLASH_READ(FLASH_BASE_ADDRESS, 0x0)) & 0x80) == 0) {
        taskDelay(FLASH_PROGRAM_TIMEOUT_TICKS);
    }
    FLASH_WRITE(addr, 0x000, INTEL_FLASH_WBSIZE -  1);
    for(i = 0; i < INTEL_FLASH_WBSIZE; i++) {
        FLASH_WRITE(addr, i, val[i]);
    }
    FLASH_WRITE(addr, 0x000, INTEL_FLASH_WB_CONFIRM);
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x000, INTEL_FLASH_RD_SR);

    for (polls = 0; polls < FLASH_PROGRAM_TIMEOUT_POLLS; polls++) {
        status = FLASH_READ(FLASH_BASE_ADDRESS, 0x0);

        if ((status & 0x80) == 0x80) {
            break;
        }
        taskDelay(FLASH_PROGRAM_TIMEOUT_TICKS);
    }

    if ((status & 0x90) != 0x80) {
        printf("flashProgramDevices(): Address 0x%08x program failed\n",
               (int)addr);
        return (ERROR);
    } else {
        return (OK);
    }

}

LOCAL int
flashProgramDevices(volatile UINT8 *addr, UINT8 val)
{
    unsigned int value;

    /* Send a program command */
    FLASH_WRITE(addr, 0x0, INTEL_FLASH_PROGRAM);

    /* Write a byte */
    FLASH_WRITE(addr, 0x0, val);

    while ((FLASH_READ(addr, 0) & 0x80) != 0x80) {
        taskDelay(FLASH_PROGRAM_TIMEOUT_TICKS);
    }

    value = FLASH_READ(addr, 0) & 0xFF;
    if (value & (0x01|0x08|0x10))
        return ERROR;
    return (OK);
}

#if 0
LOCAL int
flashProgramDevices(volatile UINT8 *addr, UINT8 val)
{
    int             polls;
    unsigned char    status;

    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x000, INTEL_FLASH_CLR_SR);
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x000, INTEL_FLASH_READ_MODE);
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x000, INTEL_FLASH_PROGRAM);
    *addr = val; /* FLASH_WRITE(addr, 0x0, val);*/
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x000, INTEL_FLASH_RD_SR);

    for (polls = 0; polls < FLASH_PROGRAM_TIMEOUT_POLLS; polls++) {
        status = FLASH_READ(FLASH_BASE_ADDRESS, 0x0);

        if ((status & 0x80) == 0x80) {
            break;
        }
        taskDelay(FLASH_PROGRAM_TIMEOUT_TICKS);
    }

    flashReadReset();
    if ((status & 0x90) != 0x80) {
        printf("flashProgramDevices(): Address 0x%08x program failed\n",
               (int)addr);
        return (ERROR);
    } else {
        return (OK);
    }
}
#endif

LOCAL int
flashWrite(int sectorNum, char *buff,
            unsigned int offset, unsigned int count)
{
    UINT8   *curBuffPtr, *flashBuffPtr;
    int             i;

    curBuffPtr = (UINT8 *)buff;
    flashBuffPtr = (UINT8 *)(FLASH_SECTOR_ADDRESS(sectorNum) + offset);

    for (i = 0; i < count; i++) {
        if (((offset + i) % INTEL_FLASH_WBSIZE) == 0) {
            break;
        }
        if (flashProgramDevices(flashBuffPtr, *curBuffPtr) == ERROR) {
            printf("flashWrite(): Failed: Sector %d, address 0x%x\n",
               sectorNum, (int)flashBuffPtr);
            return (ERROR);
        }

        flashBuffPtr++;
        curBuffPtr++;
    }
    for (; i < count; i += INTEL_FLASH_WBSIZE) {
        if ((i + INTEL_FLASH_WBSIZE) >= count) {
            break;
        }
        if (flashProgramBlock(flashBuffPtr, curBuffPtr) != OK) {
            printf("flashWrite(): Failed: Sector %d, address 0x%x\n",
               sectorNum, (int)flashBuffPtr);
            return (ERROR);
        }
        flashBuffPtr += INTEL_FLASH_WBSIZE;
        curBuffPtr += INTEL_FLASH_WBSIZE;
    }
    for (; i < count; i++) {
        if (flashProgramDevices(flashBuffPtr, *curBuffPtr) == ERROR) {
            printf("flashWrite(): Failed: Sector %d, address 0x%x\n",
               sectorNum, (int)flashBuffPtr);
            return (ERROR);
        }

        flashBuffPtr++;
        curBuffPtr++;
    }
    return (0);
}

struct flash_drv_funcs_s flash28f128 = {
    FLASH_2F128, INTEL,
    flashAutoSelect,
    flashEraseSector,
    flashRead,
    flashWrite
};

