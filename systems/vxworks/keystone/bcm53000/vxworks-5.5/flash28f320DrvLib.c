/* $Id: flash28f320DrvLib.c,v 1.3 2011/07/21 16:14:24 yshtil Exp $ */
#include "vxWorks.h"
#include "taskLib.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "config.h"
#include "flashDrvLib.h"

#ifdef  MIPSEL
#define ADDR_XOR_3 0
#else
#define ADDR_XOR_3 3
#endif
#ifdef MBZ
#define xor_val ADDR_XOR_3
#else
#define xor_val 0x0
#endif

#define FLASH_ADDR(dev, addr) \
    ((volatile UINT8 *) (((int)(dev) + (int)(addr)) ^ xor_val))

#define FLASH_WRITE(dev, addr, value) \
    (*FLASH_ADDR(dev, addr) = (value))

#define FLASH_READ(dev, addr) \
    (*FLASH_ADDR(dev, addr))

LOCAL void
flashReadReset(void)
{
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x000, 0xff);
}

LOCAL void
flashAutoSelect(FLASH_TYPES *dev, FLASH_VENDORS *vendor)
{
    flashReadReset();
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x000, 0x90);

    *vendor = FLASH_READ(FLASH_BASE_ADDRESS, 0);
    *dev = FLASH_READ(FLASH_BASE_ADDRESS, 2);

    if (flashVerbose) {
        printf("flashAutoSelect(): dev = 0x%x, vendor = 0x%x\n",
               (int)*dev, (int)*vendor);
    }

    flashReadReset();
    if ((*dev != FLASH_2F320) || (*vendor != INTEL)) {
        *vendor = *dev = 0xFF;
    }
}

LOCAL int
flashEraseDevices(volatile UINT8 *sectorBasePtr)
{
    int             i;
    unsigned int    tmp = 0;

    if (flashVerbose) {
        printf("Erasing Sector @ 0x%08x\n",(UINT32)sectorBasePtr);
    }
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x0, 0xFF);
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x0, 0x50);
    FLASH_WRITE(sectorBasePtr, 0x000, 0x20);
    FLASH_WRITE(sectorBasePtr, 0x000, 0xd0);
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x0, 0x70);

    for (i = 0; i < FLASH_ERASE_TIMEOUT_COUNT; i++) {
        taskDelay(FLASH_ERASE_TIMEOUT_TICKS);

        tmp = FLASH_READ(FLASH_BASE_ADDRESS, 0x0);

        if ((tmp & 0x80) == 0x80) {
            break;
        }
    }

    flashReadReset();
    if ((tmp & 0xa0) != 0x80) {
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
flashProgramDevices(volatile UINT8 *addr, UINT8 val)
{
    int             polls;
    unsigned char    tmp;

    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x000, 0x50);
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x000, 0xFF);
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x000, 0x40);
    /* FLASH_WRITE(addr, 0x0, val);*/
    *addr = val;
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x000, 0x70);

    for (polls = 0; polls < FLASH_PROGRAM_TIMEOUT_POLLS; polls++) {
        tmp = FLASH_READ(FLASH_BASE_ADDRESS, 0x0);

        if ((tmp & 0x80) == 0x80) {
            break;
        }
        taskDelay(0);
    }

    flashReadReset();
    if ((tmp & 0x90) != 0x80) {
        printf("flashProgramDevices(): Address 0x%08x program failed\n",
               (int)addr);
        return (ERROR);
    } else {
        return (OK);
    }

}

LOCAL int
flashWrite(int sectorNum, char *buff,
            unsigned int offset, unsigned int count)
{
    UINT8   *curBuffPtr, *flashBuffPtr;
    int             i;

    curBuffPtr = (UINT8 *)buff;
    flashBuffPtr = (UINT8 *)(FLASH_SECTOR_ADDRESS(sectorNum) + offset);

    for (i = 0; i < count; i++) {
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

struct flash_drv_funcs_s flash28f320 = {
    FLASH_2F320, INTEL,
    flashAutoSelect,
    flashEraseSector,
    flashRead,
    flashWrite
};

