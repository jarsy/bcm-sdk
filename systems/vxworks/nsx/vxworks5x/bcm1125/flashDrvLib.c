#include "vxWorks.h"
#include "taskLib.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "config.h"
#include "flashDrvLib.h"

#define TOTAL_LOADED_SECS  3

int             flashVerbose = 0; /* DEBUG */
unsigned int    flashBaseAddress = 0;
int             flashSize = 0x200000;
int             flashDevSectorSize = 0x10000;
int             flashSectorCount = 0;
LOCAL struct flash_drv_funcs_s *flashDrvFuncs = &flash28f128;

LOCAL struct flashLoadedSectorInfo {
    SEM_ID fsSemID;
    int   sector;
    int   dirty;
    char *buffer;
} flashLoadedSectors[TOTAL_LOADED_SECS];

#define FS_CACHE_LOCK(_x_) \
    semTake(flashLoadedSectors[(_x_)].fsSemID, WAIT_FOREVER)
#define FS_CACHE_UNLOCK(_x_) \
    semGive(flashLoadedSectors[(_x_)].fsSemID)

LOCAL int
flashFlushLoadedSector(int number, int reason)
{
    if (flashLoadedSectors[number].sector < 0 ||
        !flashLoadedSectors[number].dirty) {
        if (flashVerbose)
            printf("flashFlushLoadedSector(%d): not dirty\n", number);
        return (OK);
    }

    if (flashVerbose) {
        printf("flashFlushLoadedSector(%d): Flushing %d %s\n", number,
                flashLoadedSectors[number].sector,
                (reason == 0 ? "<- flashSync" : "<- flashBlkWrite"));
    }

    if (flashDrvFuncs->flashEraseSector(flashLoadedSectors[number].sector)==ERROR) {
        return (ERROR);
    }

    if (flashDrvFuncs->flashWrite(flashLoadedSectors[number].sector,
            flashLoadedSectors[number].buffer, 0, FLASH_SECTOR_SIZE) == ERROR) {
        return (ERROR);
    }

    if (flashVerbose) {
        printf("                           Flushing %d done\n",
                flashLoadedSectors[number].sector);
    }

    flashLoadedSectors[number].sector = -1;
    flashLoadedSectors[number].dirty = 0;

    return (OK);
}

int
flashDrvLibInit(void)
{
    FLASH_TYPES     dev;
    FLASH_VENDORS   vendor;
    int i;

    flashBaseAddress = FLASH_BASE_ADDRESS_FLASH_BOOT;

    flashDrvFuncs = &flash28f128;
    flashDrvFuncs->flashAutoSelect(&dev, &vendor);

    switch (vendor) {
        case AMD:
        case ALLIANCE:
        switch (dev) {
            case FLASH_2F040:
                flashSectorCount = 8;
                flashDevSectorSize = 0x10000;
                if (flashVerbose)
                    printf("flashInit(): 2F040 Found\n");
                break;
            case FLASH_2F080:
                flashSectorCount = 16;
                flashDevSectorSize = 0x10000;
                if (flashVerbose)
                    printf("flashInit(): 2F080 Found\n");
                break;

            case FLASH_2L081:
                flashSectorCount = 16;
                flashDevSectorSize = 0x10000;
                if (flashVerbose)
                    printf("flashInit(): 29LV081B Found\n");
                break;

            case FLASH_2L160:
            case FLASH_2L017:
                flashSectorCount = 32;
                flashDevSectorSize = 0x10000;
                if (flashVerbose)
                    printf("flashInit(): 29LV160D Found\n");
                break;

            default:
                printf("flashInit(): Unrecognized Device (0x%02X)\n", dev);
                return (ERROR);
        }
        break;

        case INTEL:
        switch (dev) {
            case FLASH_2F128:
                flashSectorCount = 128;
                flashDevSectorSize = 0x20000;
                if (flashVerbose)
                    printf("flashInit(): 28F128 Found\n");
                break;
            case FLASH_2F320:
                flashSectorCount = 32;
                flashDevSectorSize = 0x20000;
                if (flashVerbose)
                    printf("flashInit(): 28F320 Found\n");
                break;

            default:
                printf("flashInit(): Unrecognized Device (0x%02X)\n", dev);
                return (ERROR);
        }
        break;
        default:
            printf("flashInit(): Unrecognized Vendor (0x%02X)\n", vendor);
            return (ERROR);
    }
    flashSize = flashDevSectorSize * flashSectorCount;

    for (i = 0; i < TOTAL_LOADED_SECS; i++) {
        flashLoadedSectors[i].buffer = malloc(FLASH_SECTOR_SIZE);
        if (flashLoadedSectors[i].buffer == NULL) {
            printf("flashInit(): malloc() failed\n");
            for (; i > 0; i--) {
                free(flashLoadedSectors[i-1].buffer);
            }
            return (ERROR);
        }
        flashLoadedSectors[i].sector = -1;
        flashLoadedSectors[i].dirty = 0;
        flashLoadedSectors[i].fsSemID =
            semMCreate (SEM_Q_PRIORITY | SEM_DELETE_SAFE);
    }

    return (OK);
}

int
flashGetSectorCount(void)
{
    return (flashSectorCount);
}

int
flashEraseBank(int firstSector, int nSectors)
{
    int             sectorNum, errCnt = 0;

    if (firstSector < 0 || firstSector + nSectors > flashSectorCount) {
        printf("flashEraseBank(): Illegal parms %d, %d\n",
           firstSector, nSectors);
        return ERROR;
    }

    for (sectorNum = firstSector;
         sectorNum < firstSector + nSectors; sectorNum++) {
         printf(".");

        if (flashDrvFuncs->flashEraseSector(sectorNum))
            errCnt++;
    }

    printf("\n");

    if (errCnt)
        return (ERROR);
    else
        return (OK);
}

int
flashBlkRead(int sectorNum, char *buff,
         unsigned int offset, unsigned int count)
{
    int i;

    if (sectorNum < 0 || sectorNum >= flashSectorCount) {
        printf("flashBlkRead(): Sector %d invalid\n", sectorNum);
        return (ERROR);
    }

    if (offset < 0 || offset >= FLASH_SECTOR_SIZE) {
        printf("flashBlkRead(): Offset 0x%x invalid\n", offset);
        return (ERROR);
    }

    if (count < 0 || count > FLASH_SECTOR_SIZE - offset) {
        printf("flashBlkRead(): Count 0x%x invalid\n", count);
        return (ERROR);
    }

    /*
     * If the sector is loaded, read from it.  Else, read from the
     * flash itself (slower).
     */
    for (i = 0; i < TOTAL_LOADED_SECS; i++) {
        if (flashLoadedSectors[i].sector == sectorNum) {
            if (flashVerbose)
                printf("flashBlkRead(): from loaded sector %d\n", sectorNum);
            bcopy(&flashLoadedSectors[i].buffer[offset], buff, count);
            return (OK);
        }
    }

    flashDrvFuncs->flashRead(sectorNum, buff, offset, count);

    return (OK);
}

/* $Id: flashDrvLib.c,v 1.3 2011/07/21 16:14:43 yshtil Exp $
 * Check if we can program this part of the flash.  All
 * the data has to be all "1" to be programmed.  Because
 * the flash has to be init to all 1's and change from 1 to 0
 */
LOCAL int
flashCheckCanProgram(int sectorNum, unsigned int offset, unsigned int count)
{
    unsigned char   *flashBuffPtr;
    int             i;

    flashBuffPtr = (unsigned char *)(FLASH_SECTOR_ADDRESS(sectorNum) + offset);

    for (i = 0; i < count; i++) {
        if (flashBuffPtr[i] != 0xff)
            return (ERROR);
    }

    return (OK);
}

int
flashBlkWrite(int sectorNum, char *buff,
          unsigned int offset, unsigned int count)
{
    char *save;
    int i;

    if (sectorNum < 0 || sectorNum >= flashSectorCount) {
        printf("flashBlkWrite(): Sector %d invalid\n", sectorNum);
        return (ERROR);
    }

    if (offset < 0 || offset >= FLASH_SECTOR_SIZE) {
        printf("flashBlkWrite(): Offset 0x%x invalid\n", offset);
        return (ERROR);
    }

    /* 
     * Count must be within range and must be a long word multiple, as
     * we always program long words.
     */

    if ((count < 0) || 
        (count > ((flashSectorCount - sectorNum) * FLASH_SECTOR_SIZE - offset))) {
        printf("flashBlkWrite(): Count 0x%x invalid\n", count);
        return (ERROR);
    }

    /*
     * If the Sector is loaded, write it to buffer.  Else check to see
     * if we can program the sector; if so, program it.  Else, flush the
     * first loaded sector (if loaded and dirty), push loaded sectors
     * up by one, load the new one and copy the data into the last one.
     */

    for (i = 0; i < TOTAL_LOADED_SECS; i++) {
        FS_CACHE_LOCK(i);
        if (flashLoadedSectors[i].sector == sectorNum) {
            if (flashVerbose)
                printf("%d ", sectorNum);
            bcopy(buff, &flashLoadedSectors[i].buffer[offset], count);
            flashLoadedSectors[i].dirty = 1;
            FS_CACHE_UNLOCK(i);
            return (OK);
        }
        FS_CACHE_UNLOCK(i);
    }

    if (flashCheckCanProgram(sectorNum, offset, count) != ERROR) {
        return (flashDrvFuncs->flashWrite(sectorNum, buff, offset, count));
    }

    FS_CACHE_LOCK(0);
    if (flashFlushLoadedSector(0, 1) == ERROR) {
        FS_CACHE_UNLOCK(0);
        return (ERROR);
    }
    FS_CACHE_UNLOCK(0);

    save = flashLoadedSectors[0].buffer;
    for (i = 1; i < TOTAL_LOADED_SECS; i++) {
        FS_CACHE_LOCK(i-1);
        flashLoadedSectors[i-1].sector = flashLoadedSectors[i].sector;
        flashLoadedSectors[i-1].dirty  = flashLoadedSectors[i].dirty;
        flashLoadedSectors[i-1].buffer = flashLoadedSectors[i].buffer;
        FS_CACHE_UNLOCK(i-1);
    }
    i--;
    flashLoadedSectors[i].buffer = save;

    FS_CACHE_LOCK(i);
    if (flashDrvFuncs->flashRead(sectorNum, flashLoadedSectors[i].buffer,
                                 0, FLASH_SECTOR_SIZE) == ERROR) {
        flashLoadedSectors[i].sector = -1;
        FS_CACHE_UNLOCK(i);
        return (ERROR);
    }

    flashLoadedSectors[i].sector = sectorNum;
    bcopy(buff, &flashLoadedSectors[i].buffer[offset], count);
    flashLoadedSectors[i].dirty = 1;
    FS_CACHE_UNLOCK(i);

    if (flashVerbose) {
        printf("flashBlkWrite(): load %d (and write to cache only)\n", sectorNum);
        printf("%d ", sectorNum);
    }

    return (OK);
}

int
flashDiagnostic(void)
{
    unsigned int   *flashSectorBuff;
    int             sectorNum, i;

    /*
     * Probe flash; allocate flashLoadedSector Buffer
     */

    flashDrvLibInit();        /* Probe; clear loaded sector */

    flashSectorBuff = (unsigned int *) flashLoadedSectors[0].buffer;

    if (flashVerbose)
        printf("flashDiagnostic(): Executing. Erasing %d Sectors\n",
                flashSectorCount);

    if (flashEraseBank(0, flashSectorCount) == ERROR) {
    if (flashVerbose)
        printf("flashDiagnostic(): flashEraseBank() #1 failed\n");

    return (ERROR);
    }

    /* Write unique counting pattern to each sector. */
    for (sectorNum = 0; sectorNum < flashSectorCount; sectorNum++) {
    if (flashVerbose)
        printf("flashDiagnostic(): writing sector %d\n", sectorNum);

    for (i = 0; i < FLASH_SECTOR_SIZE / sizeof (unsigned int); i++)
        flashSectorBuff[i] = (i + sectorNum);

    if (flashDrvFuncs->flashWrite(sectorNum, (char *)flashSectorBuff,
               0, FLASH_SECTOR_SIZE) == ERROR) {
        if (flashVerbose)
        printf("flashDiagnostic(): flashWrite() failed on %d\n",
               sectorNum);

        return (ERROR);
    }
    }

    /* Verify each sector. */
    for (sectorNum = 0; sectorNum < flashSectorCount; sectorNum++) {
    if (flashVerbose)
        printf("flashDiagnostic(): verifying sector %d\n", sectorNum);

    if (flashDrvFuncs->flashRead(sectorNum, (char *)flashSectorBuff,
              0, FLASH_SECTOR_SIZE) == ERROR) {
        if (flashVerbose)
        printf("flashDiagnostic(): flashRead() failed on %d\n",
               sectorNum);

        return (ERROR);
    }

    for (i = 0; i < FLASH_SECTOR_SIZE / sizeof (unsigned int); i++) {
        if (flashSectorBuff[i] != (i + sectorNum)) {
        if (flashVerbose) {
            printf("flashDiagnostic(): verification failed\n");
            printf("flashDiagnostic(): sector %d, offset 0x%x\n",
               sectorNum, (i * sizeof(unsigned int)));
            printf("flashDiagnostic(): expected 0x%x, got 0x%x\n",
               (i + sectorNum), (int)flashSectorBuff[i]);
        }

        return (ERROR);
        }
    }
    }

    if (flashEraseBank(0, flashSectorCount) == ERROR) {
    if (flashVerbose)
        printf("flashDiagnostic(): flashEraseBank() #2 failed\n");

        return (ERROR);
    }

    if (flashVerbose)
        printf("flashDiagnostic(): Completed without error\n");

    return (OK);
}

int
flashSyncFilesystem(void)
{
    int i;

#if 0
    /* Make sure the disk cache (dcacheUpdTaskId) has finished writing */
    taskDelay(3 * sysClkRateGet());
#endif

    for (i = 0; i < TOTAL_LOADED_SECS; i++) {
        FS_CACHE_LOCK(i);
        if (flashFlushLoadedSector(i, 0) != OK) {
            FS_CACHE_UNLOCK(i);
            return (ERROR);
        }
        FS_CACHE_UNLOCK(i);
    }

    return (OK);
}

