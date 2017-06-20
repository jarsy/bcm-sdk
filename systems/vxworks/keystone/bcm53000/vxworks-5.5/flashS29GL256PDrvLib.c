/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * $Id: flashS29GL256PDrvLib.c,v 1.3 Broadcom SDK $
 */
 
#include "vxWorks.h"
#include "taskLib.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "config.h"
#include "flashDrvLib.h"

#define FLASH_16BIT 1   /* Actually effective only for programming */
#define DELAY() do { int i; for(i=0;i<32768; i++); } while(0)
#define RETRY  100 /* retry count for probing flash device */

#define FLASH_BUFFER_WRITE 1   /* Use write buffer programming operation */

#define FLASH_ADDR(dev, addr) \
    ((volatile UINT8 *) ((int)(dev) + (int)(addr)))

#define FLASH_WRITE(dev, addr, value) \
    (*FLASH_ADDR(dev, addr) = (value))

#define FLASH_READ(dev, addr) \
    (*FLASH_ADDR(dev, addr))

#define FLASH_ADDR16(dev, addr) \
    ((volatile UINT16 *) ((int)(dev) + (int)(addr*2)))

#ifdef BE_HOST
#define FLASH_WRITE16(dev, addr, value) \
    (*FLASH_ADDR16(dev, addr) = (value << 8))
#else
#define FLASH_WRITE16(dev, addr, value) \
    (*FLASH_ADDR16(dev, addr) = (value))
#endif

#define AMD_FLASH_WBSIZE  64

#if FLASH_16BIT
#define AMD_FLASH_WBWORDCNT \
    ((AMD_FLASH_WBSIZE/2)-1)
#else
#define AMD_FLASH_WBWORDCNT \
    (AMD_FLASH_WBSIZE -1)
#endif

extern struct flash_drv_funcs_s flashs29gl256p;

LOCAL void
flashReadReset(void)
{
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_1, AMD_FLASH_RESET);
}

LOCAL void
flashAutoSelect(FLASH_TYPES *dev, FLASH_VENDORS *vendor)
{
    int i = 0;
    unsigned int mask = 0xff; /* 8 bit mode */

    flashReadReset();   

    /* We use 8bit mode when autoselecting for better compatibility */
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_1, AMD_FLASH_MAGIC_1);
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_2, AMD_FLASH_MAGIC_2);
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_1, AMD_FLASH_AUTOSEL);

    /* Since BCM5300x runs too fast, we need some delay in between */
    DELAY();

    *vendor = (FLASH_VENDORS)FLASH_READ(FLASH_BASE_ADDRESS, 0);
    if (*vendor != 1) {
        for (i = 1 ; i <= RETRY ; i++) {
    *vendor = (FLASH_VENDORS)FLASH_READ(FLASH_BASE_ADDRESS, 0);
            if (*vendor == 1) {
                i = RETRY;
            }
        }
    }
    *dev = (FLASH_TYPES)FLASH_READ(FLASH_BASE_ADDRESS, 2);

    /*
      * If device ID is 0x227e(16 bits) or 0x7e(8 bits), 
      * this is a device with 3-bytes device ID.
      * Read the other 2 bytes and set those 2-bytes as device ID
      */
    if (((int)*vendor == 1) && (((int)*dev & mask) == (FLASH_29GL128 & mask))) {
        *dev = ((((FLASH_TYPES)FLASH_READ(FLASH_BASE_ADDRESS, 0x1c) & mask) << 8) |
            ((FLASH_TYPES)FLASH_READ(FLASH_BASE_ADDRESS, 0x1e) & mask));
    }
    if (flashVerbose)
        printf("flashAutoSelect(): dev = 0x%x, vendor = 0x%x\n",
               (int)*dev, (int)*vendor);
    flashReadReset();   

    if ((*dev != FLASH_S29GL128P && *dev != FLASH_S29GL256P && 
         *dev != FLASH_S29GL512P && *dev != FLASH_S29GL01GP) ||
        ((*vendor != AMD) && (*vendor != ALLIANCE) && *vendor != MXIC)) {
        *vendor = *dev = 0xFF;
    }

}

LOCAL int
flashEraseDevices(volatile unsigned char *sectorBasePtr)
{
    int             i;
    unsigned int    tmp;

    if (flashVerbose) {
        printf("Erasing Sector @ 0x%08x\n",(unsigned int)sectorBasePtr);
    }

    /* We use 8bit mode when autoselecting for better compatibility */
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_1, AMD_FLASH_MAGIC_1);
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_2, AMD_FLASH_MAGIC_2);
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_1, AMD_FLASH_ERASE_3);
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_1, AMD_FLASH_ERASE_4);
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_2, AMD_FLASH_ERASE_5);
    FLASH_WRITE
        (sectorBasePtr, 0x0, AMD_FLASH_ERASE_SEC_6);

    for (i = 0; i < FLASH_ERASE_TIMEOUT_COUNT; i++) {
        taskDelay(FLASH_ERASE_TIMEOUT_TICKS);

        tmp = FLASH_READ(sectorBasePtr, 0x0);

        if ((tmp & 0x80) == 0x80) {
            if (flashVerbose > 1)
                printf("flashEraseDevices(): all devices erased\n");
            return (OK);
        }
    }

    if ((tmp & 0x20) == 0x20) {
        printf("flashEraseDevices(): addr 0x%08x erase failed\n",
           (int)sectorBasePtr);
    } else {
        printf("flashEraseDevices(): addr 0x%08x erase timed out\n",
           (int)sectorBasePtr);
    }

    flashReadReset();
    return (ERROR);
}

LOCAL int
flashEraseSector(int sectorNum)
{
    unsigned char   *sectorBasePtr =
	(unsigned char *)FLASH_SECTOR_ADDRESS(sectorNum);

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
    if (sectorNum < 0 || sectorNum >= flashSectorCount) {
        printf("flashRead(): Illegal sector %d\n", sectorNum);
        return (ERROR);
    }

    bcopy((char *)(FLASH_SECTOR_ADDRESS(sectorNum) + offset), buff, count);

    return (0);
}


LOCAL int
#if FLASH_16BIT
flashProgramDevices(volatile unsigned short *addr, unsigned short val)
#else
flashProgramDevices(volatile unsigned char *addr, unsigned char val)
#endif
{
    int             polls;
    unsigned int    tmp;

    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_1, AMD_FLASH_MAGIC_1);
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_2, AMD_FLASH_MAGIC_2);
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_1, AMD_FLASH_PROGRAM);

    *addr = val;

    for (polls = 0; polls < FLASH_PROGRAM_TIMEOUT_POLLS; polls++) {
        tmp = *addr;

#if FLASH_16BIT
        if ((tmp & 0x8080) == (val & 0x8080)) {
#else
        if ((tmp & 0x80) == (val & 0x80)) {
#endif
            if (flashVerbose > 2)
                printf("flashProgramDevices(): devices programmed\n");
            return (OK);
        }
    }

#if FLASH_16BIT
    if ((tmp & 0x2020) != 0) {
#else
    if ((tmp & 0x20) != 0) {
#endif
	/* 
	 * We've already waited so long that chances are nil that the
	 * 0x80 bits will change again.  Don't bother re-checking them.
	 */
		printf("flashProgramDevices(): Address 0x%08x program failed\n",
		       (int)addr);
    } else {
        printf("flashProgramDevices(): timed out\n");
    }

    flashReadReset();
    return (ERROR);
}

#if FLASH_BUFFER_WRITE
LOCAL void
flashWriteBufferAbortReset(void)
{
#if FLASH_16BIT
    FLASH_WRITE16
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR16_1, AMD_FLASH_MAGIC_1);
    FLASH_WRITE16
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR16_2, AMD_FLASH_MAGIC_2);
    FLASH_WRITE16
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR16_1, AMD_FLASH_RESET);
#else
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_1, AMD_FLASH_MAGIC_1);
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_2, AMD_FLASH_MAGIC_2);
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_1, AMD_FLASH_RESET);
#endif
}

LOCAL int
#if FLASH_16BIT
flashProgramBlock(volatile unsigned short *addr, unsigned short *val)
#else
flashProgramBlock(volatile unsigned char *addr, unsigned char *val)
#endif
{
    int  wc, polls;
#if FLASH_16BIT
    unsigned short  read_1, read_2, read_3;
    volatile unsigned short *daddr;
#else
    unsigned char  read_1, read_2, read_3;
    volatile unsigned char *daddr;
#endif
    daddr = addr;

#if FLASH_16BIT
    /* Do an "unlock write" sequence  (cycles 1-2) */
    FLASH_WRITE16
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR16_1, AMD_FLASH_MAGIC_1);
    FLASH_WRITE16
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR16_2, AMD_FLASH_MAGIC_2);

    /* Send a write buffer load command to sector address (cycle 3) */
    FLASH_WRITE16(addr, 0x0, AMD_FLASH_WRITE_BUFFER);

    /* Send a write word count (minus 1) to sector address (cycle 4) */
    FLASH_WRITE16(addr, 0x0, AMD_FLASH_WBWORDCNT);

    /* Number of words(N) loaded into the write bufferm N=32 words (64 bytes) */
    for (wc = 0; wc <= AMD_FLASH_WBWORDCNT; wc++) {
        *addr = *val;
        addr++;
        val++;
    }

    /* Send a write confirm command to write buffer to flash (Last cycle) */
    FLASH_WRITE16(daddr, 0x0, AMD_FLASH_WRITE_CONFIRM);
#else
    /* Do an "unlock write" sequence  (cycles 1-2) */
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_1, AMD_FLASH_MAGIC_1);
    FLASH_WRITE
        (FLASH_BASE_ADDRESS, AMD_FLASH_MAGIC_ADDR8_2, AMD_FLASH_MAGIC_2);

    /* Send a write buffer load command to sector address (cycle 3) */
    FLASH_WRITE(addr, 0x0, AMD_FLASH_WRITE_BUFFER);

    /* Send a write word count (minus 1) to sector address (cycle 4) */
    FLASH_WRITE(addr, 0x0, AMD_FLASH_WBWORDCNT);

    /* Number of words(N) loaded into the write bufferm N=32 words (64 bytes) */;
    for (wc = 0; wc <= AMD_FLASH_WBWORDCNT; wc++) {
        *addr = *val;
        addr++;
        val++;
    }

    /* Send a write confirm command to write buffer to flash (Last cycle) */
    FLASH_WRITE(daddr, 0x0, AMD_FLASH_WRITE_CONFIRM);
#endif

    daddr = (addr - 1);

    /* Wait for write to complete */
    for (polls = 0; polls < FLASH_PROGRAM_TIMEOUT_POLLS; polls++) {
        read_1 = *daddr;
        read_2 = *daddr;
        read_3 = *daddr;

        if (read_1 == read_2 && read_2 == read_3) {
            return (OK);
        }
    }
    
    /* Write Buffer Abort Reset */
    flashWriteBufferAbortReset();
    return (ERROR);
}
#endif

LOCAL int
flashWrite(int sectorNum, char *buff, unsigned int offset, unsigned int count)
{
#if FLASH_16BIT
    unsigned short   *curBuffPtr, *flashBuffPtr;
#else
    unsigned char   *curBuffPtr, *flashBuffPtr;
#endif
    int             i, index;

#if FLASH_16BIT
    curBuffPtr = (unsigned short *)buff;
    flashBuffPtr = (unsigned short *)(FLASH_SECTOR_ADDRESS(sectorNum) + offset);
    index = 2;
#else
    curBuffPtr = (unsigned char *)buff;
    flashBuffPtr = (unsigned char *)(FLASH_SECTOR_ADDRESS(sectorNum) + offset);
    index = 1;
#endif

#if FLASH_BUFFER_WRITE /* write buffer program */
    for (i = 0; i < count; i+=index) {
        if (((offset + i) % AMD_FLASH_WBSIZE) == 0) {
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
    for (; i < count; i += AMD_FLASH_WBSIZE) {
        if ((i + AMD_FLASH_WBSIZE) > count) {
            break;
        }
        if (flashProgramBlock(flashBuffPtr, curBuffPtr) != OK) {
            printf("flashWrite(): Failed: Sector %d, address 0x%x\n",
               sectorNum, (int)flashBuffPtr);
            return (ERROR);
        }

        flashBuffPtr += (AMD_FLASH_WBSIZE/index);
        curBuffPtr += (AMD_FLASH_WBSIZE/index);
    }
    for (; i < count; i+=index) {
        if (flashProgramDevices(flashBuffPtr, *curBuffPtr) == ERROR) {
            printf("flashWrite(): Failed: Sector %d, address 0x%x\n",
               sectorNum, (int)flashBuffPtr);
            return (ERROR);
        }

        flashBuffPtr++;
        curBuffPtr++;
    }

#else /* write byte program */
#if FLASH_16BIT
    for (i = 0; i < (count+1)/2; i++) {
#else
    for (i = 0; i < count; i++) {
#endif

        if (flashProgramDevices(flashBuffPtr, *curBuffPtr) == ERROR) {
            printf("flashWrite(): Failed: Sector %d, address 0x%x\n",
               sectorNum, (int)flashBuffPtr);
            return (ERROR);
        }

        flashBuffPtr++;
        curBuffPtr++;
    }
#endif

    return (0);
}

struct flash_drv_funcs_s flashs29gl256p = {
    FLASH_S29GL256P, AMD,
    flashAutoSelect,
    flashEraseSector,
    flashRead,
    flashWrite
};

