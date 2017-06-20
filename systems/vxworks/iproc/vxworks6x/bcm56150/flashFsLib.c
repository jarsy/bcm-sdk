#include "types.h"
#include "vxWorks.h"
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"

#include "ioLib.h"
#include "dosFsLib.h"
#include "fioLib.h"
#include "blkIo.h"
#include "string.h"
#include "flashDrvLib.h"
#include "flashFsLib.h"
#include "errnoLib.h"
#include "config.h"

BLK_DEV         flashBlkDev;
device_t        flashFsLibXbdDev = 0;
extern device_t        xbdBlkDevCreate (BLK_DEV *bd, const char *name);
DOS_VOLUME_DESC_ID     flashDosVolDesc = 0;

int             flashFsVerbose = 0;

STATUS
flashFsGetPhys(int blkNum, int *sectorNum, int *offset)
{
    int     sect;

    if (blkNum >= FLASH_FS_SIZE_BLOCKS) {
        return (ERROR);
    }

    sect = blkNum / FLASH_FS_BLOCK_PER_SECTOR;

    /* Skip boot sector */
    sect += FLASH_BOOT_SIZE_SECTORS;
    /* Skip nvram and env sector */
    sect += (FLASH_NVRAM_SIZE / FLASH_SECTOR_SIZE);

    *sectorNum = sect;

    *offset = (blkNum % FLASH_FS_BLOCK_PER_SECTOR) * FLASH_FS_BLOCK_SIZE;

    if (flashFsVerbose > 2) {
        printf("flashFsGetPhys(): blkNum = %d, "
               "*sectorNum = %d, *offset = 0x%x\n",
               blkNum, *sectorNum, *offset);
    }

    return (OK);
}

STATUS
flashFsBlkRead(BLK_DEV *pDev, int startBlk, int numBlks, char *pBuf)
{
    int             blkIndx, phySectorNum, offset;

    if (flashFsVerbose > 1) {
        printf("flashFsBlkRead(): startBlk = %d, numBlks = %d\n",
               startBlk, numBlks);
    }

    for (blkIndx = 0; blkIndx < numBlks; blkIndx++) {
        if (flashFsGetPhys((startBlk + blkIndx), &phySectorNum, &offset) ==
            ERROR) {
            if (flashFsVerbose)
            printf("flashFsBlkRead(): flashFsGetPhys() failed\n");

            return (ERROR);
        }

        if (flashBlkRead(phySectorNum, pBuf,
                 offset, FLASH_FS_BLOCK_SIZE) == ERROR) {
            if (flashFsVerbose)
            printf("flashFsBlkRead(): flashBlkRead() failed\n");

            return (ERROR);
        }

        pBuf += FLASH_FS_BLOCK_SIZE;
    }

    return (OK);
}

STATUS
flashFsBlkWrite(BLK_DEV *pDev, int startBlk, int numBlks, char *pBuf)
{
    int             blkIndx, phySectorNum, offset;

    if (flashFsVerbose > 1) {
        printf("flashFsBlkWrite(): startBlk = %d, numBlks = %d\n",
               startBlk, numBlks);
    }

    for (blkIndx = 0; blkIndx < numBlks; blkIndx++) {
        if (flashFsGetPhys((startBlk + blkIndx), &phySectorNum, &offset) ==
            ERROR) {

            printf("flashFsBlkWrite(): flashFsGetPhys() failed\n");
            return (ERROR);
        }

        if (flashBlkWrite(phySectorNum, pBuf,
                  offset, FLASH_FS_BLOCK_SIZE) == ERROR) {

            printf("flashFsBlkWrite(): flashBlkWrite() failed\n");
            return (ERROR);
        }

        pBuf += FLASH_FS_BLOCK_SIZE;
    }

    return (OK);
}

STATUS
flashSync(void)
{
    int             flashFd = 0;
    STATUS          rc;

    if ((flashFd = open(FLASH_FS_NAME, O_RDWR, 0666)) == ERROR) {
    printf("flashSync(): open() failed\n");
    }
    rc = ioctl(flashFd, FLASH_FS_SYNC, 0);
    close(flashFd);
    return rc;
}

STATUS
flashFsIoctl(BLK_DEV *pDev, int funcCode, int arg)
{
    int         sectorAfterBoot;

    if (flashFsVerbose > 1)
    printf("flashFsIoctl(0x%x): called\n", funcCode);

    switch (funcCode) {
    case FIODISKFORMAT:
        /* Clear flashDrvLib's cached sector */
        if (flashDrvLibInit() == ERROR) {
            printf("flashFsInit(): flashDrvLibInit() failed\n");
            return (ERROR);
        }

        /* From beginning of flash to beginning of Boot Area */
        if (flashEraseBank(0, FLASH_BOOT_START_SECTOR) == ERROR) {
            return (ERROR);
        }

        sectorAfterBoot = FLASH_BOOT_START_SECTOR + FLASH_BOOT_SIZE_SECTORS;

        /* From end of Boot Area to end of flash */
        if (flashEraseBank(sectorAfterBoot,
                   FLASH_SIZE_SECTORS - sectorAfterBoot) == ERROR) {
            return (ERROR);
        }
        break;

    case FLASH_FS_SYNC:
        flashSyncFilesystem();
        break;

    default:
        errnoSet(S_ioLib_UNKNOWN_REQUEST);
        return (ERROR);
        break;
    }

    return (OK);
}

STATUS fsmNameInstall(char *driver, char *volume);

STATUS
flashFsLibInit(void)
{
    /* 
     * We are considered initialized once flashDosVolDesc is non-NULL.
     */    
    if (flashDosVolDesc) {
        return OK;
    }

    if (flashDrvLibInit() == ERROR) {
        printf("flashFsLibInit(): flashDrvLibInit() failed\n");
        return (ERROR);
    }

    /* 
     * Set up BLK DEV structure
     */

    flashBlkDev.bd_blkRd = flashFsBlkRead;
    flashBlkDev.bd_blkWrt = flashFsBlkWrite;
    flashBlkDev.bd_ioctl = flashFsIoctl;
    flashBlkDev.bd_reset = NULL;
    flashBlkDev.bd_statusChk = NULL;
    flashBlkDev.bd_removable = FALSE;
    flashBlkDev.bd_nBlocks = FLASH_FS_SIZE_BLOCKS;
    flashBlkDev.bd_bytesPerBlk = FLASH_FS_BLOCK_SIZE;
    flashBlkDev.bd_blksPerTrack = 1;
    flashBlkDev.bd_nHeads = 1;
    flashBlkDev.bd_retry = 1;
    flashBlkDev.bd_mode = O_RDWR;
    flashBlkDev.bd_readyChanged = FALSE;

    if (flashFsVerbose) {
        printf("flashFsLibInit: Initializing\n");
    }

    /* 
     * Create DOS file system
     */
    {
        char fsmName[12];
        sprintf (fsmName, "%s:0", FLASH_FS_NAME); /* Create name for mapping. */
        fsmNameInstall (fsmName, FLASH_FS_NAME);  /* Install name mapping. */
    
        if ((flashFsLibXbdDev = 
             xbdBlkDevCreate(&flashBlkDev, FLASH_FS_NAME)) == NULLDEV) {
            printf("flashFsLibInit: failed to created xdb device !!\n");
            return (ERROR);
        }
    
        if (dosFsDevCreate(FLASH_FS_NAME, flashFsLibXbdDev, 
                           DOSFS_DEFAULT_MAX_FILES, DOS_CHK_NONE)) {
            printf("Failed to create DOS file device \n");
            return (ERROR);
        }
    
        flashDosVolDesc = dosFsVolDescGet(FLASH_FS_NAME, NULL);
    
        if (flashDosVolDesc == NULL) {
            if (flashFsVerbose) {
                printf("\nflashFsLibInit: first time initialization...\n");
            }
    
            dosFsVolFormat(FLASH_FS_NAME, DOS_OPT_DEFAULT, NULL);
            flashDosVolDesc = dosFsVolDescGet(FLASH_FS_NAME, NULL);
            if (flashDosVolDesc == NULL) {
                printf("Failed to Format DOS file device \n");
                return (ERROR);
            }
        }
        
        if (flashFsVerbose) {
            dosFsShow(FLASH_FS_NAME, DOS_CHK_VERB_2);
        } 
    }

   if (flashFsVerbose) {
        printf("done\n");
    }

    flashFsSync();
 
    return OK;
}

STATUS
flashFsSync(void)
{ 
    if (flashFsVerbose) {
       printf("flashFsSync(): Syncing...");
    }
    flashSyncFilesystem();
    if (flashFsVerbose) {
        printf("done\n");
    }

    return OK; 
}
