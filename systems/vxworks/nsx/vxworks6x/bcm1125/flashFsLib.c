#include "types.h"
#include "vxWorks.h"
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "taskLib.h"

#include "ioLib.h"
#include "dosFsLib.h"
#include "dcacheCbio.h"
#include "fioLib.h"
#include "blkIo.h"
#include "string.h"
#include "flashDrvLib.h"
#include "flashFsLib.h"
#include "errnoLib.h"

#include "config.h"

BLK_DEV         flashBlkDev;
int             flashDosFormat = 0;
int             flashBootSize = (1024 * 1024);
device_t        flashXbdDev = 0;
extern device_t xbdBlkDevCreate      (BLK_DEV *bd, const char *name);
DOS_VOLUME_DESC_ID   flashDosVolDesc = 0;

extern STATUS dosFsDevInitOptionsSet(int options);
extern STATUS dosFsMkfsOptionsSet(int options);

/* $Id: flashFsLib.c,v 1.7 2011/07/21 16:14:49 yshtil Exp $
 * The boot ROM resides at the begining of the 16 MB flash, from 0 MB
 * through 1 MB.
 *
 * CFE may request a new partitioning scheme by embedding a partition
 * table in the flash.
 *
 * flashFsGetPhys remaps logical into physical sectors in such as way as
 * to skip over the two boot sectors.
 */

int flashFsVerbose = 0;  /* DEBUG */

/*
 * CFE partition table
 */

#define PTABLE_MAGIC           0x5054424C /* 'PTBL' (big-endian) */
#define PTABLE_MAGIC_LE        0x4C425450 /* 'PTBL' (little-endian) */
#define PTABLE_PNAME_MAX       16
#define PTABLE_MAX_PARTITIONS  8

typedef struct partition_s {
    char            name[PTABLE_PNAME_MAX];
    unsigned int    offset;
    unsigned int    size;
    unsigned int    type;
    unsigned int    flags;
} partition_t;

typedef struct ptable_s {
    unsigned int    magic;
    unsigned int    version;
    unsigned int    chksum;
    unsigned int    reserved;
    partition_t     part[PTABLE_MAX_PARTITIONS];
} ptable_t;

STATUS
flashFsCheckPtable(ptable_t *ptbl, int *bootSize)
{
        unsigned int    chksum, u32, *p32;
        int             i, swapped = 0;

        if (ptbl->magic == PTABLE_MAGIC_LE) {
                swapped = 1;
        }
        else if (ptbl->magic != PTABLE_MAGIC) {
                return ERROR;
        }
        chksum = 0;
        p32 = (unsigned int*)ptbl;
        for (i = 0; i < sizeof(ptable_t)/4; i++) {
                chksum ^= p32[i];
                if (swapped) {
                        swab((char*)&p32[i], (char*)&u32, 4);
                        p32[i] = u32;
                }
        }
        if (chksum != 0) {
                return ERROR;
        }
        for (i = 0; i < PTABLE_MAX_PARTITIONS && ptbl->part[i].size; i++) {
            /*
             * We consider the boot partition to span the all partitions
             * up to the first VxWorks compatible filesystem.
             */
            if (strcmp(ptbl->part[i].name, "vxworks") == 0 ||
                strcmp(ptbl->part[i].name, "dosfs") == 0 ||
                strcmp(ptbl->part[i].name, "fatfs") == 0) {

                *bootSize = ptbl->part[i].offset;
                return OK;
            }
        }
        return ERROR;
}

STATUS
flashFsFindBootPartition(int *bootSize)
{
    int         offset, sect, sectCnt, sectSize;
    char        *buff;
    ptable_t    *ptbl;

    sectCnt = flashGetSectorCount();
    sectSize = flashSize / sectCnt;

    if ((buff = malloc(sectSize)) == NULL) {
        printf("flashFindBootPartition(): malloc() failed\n");
        return ERROR;
    }

    for (sect = 0; sect < sectCnt; sect++) {
        if (flashBlkRead(sect, buff, 0, sectSize) == ERROR) {
            printf("flashFindBootPartition(): flashBlkRead() failed\n");
            return ERROR;
        }
        for (offset = 0; offset < sectSize; offset += 0x8000) {
            ptbl = (ptable_t *)&buff[offset];
            if (flashFsCheckPtable(ptbl, bootSize) == OK) {
                printf("Found flash partition table "
                       "at offset 0x%08x\n", sect * sectSize + offset);
                return OK;
            }
        }
    }
    return ERROR;
}

STATUS
flashFsGetPhys(int blkNum, int *sectorNum, int *offset)
{
    int		sect;

    if (blkNum >= FLASH_FS_SIZE_BLOCKS)
	return (ERROR);

    sect = blkNum / FLASH_FS_BLOCK_PER_SECTOR;

    if (sect >= FLASH_BOOT_START_SECTOR) {
	/* Skip boot sector */
	sect += FLASH_BOOT_SIZE_SECTORS;
    }

    *sectorNum = sect;

    *offset = (blkNum % FLASH_FS_BLOCK_PER_SECTOR) * FLASH_FS_BLOCK_SIZE;

    if (flashFsVerbose > 2)
	printf("flashFsGetPhys(): blkNum = %d, "
	       "*sectorNum = %d, *offset = 0x%x\n",
	       blkNum, *sectorNum, *offset);

    return (OK);
}

STATUS
flashFsBlkRead(BLK_DEV *pDev, int startBlk, int numBlks, char *pBuf)
{
    int             blkIndx, phySectorNum, offset;

    if (flashFsVerbose > 1)
	printf("flashFsBlkRead(): startBlk = %d, numBlks = %d\n",
	       startBlk, numBlks);

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

    if (flashFsVerbose > 1)
	printf("flashFsBlkWrite(): startBlk = %d, numBlks = %d\n",
	       startBlk, numBlks);

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
        return ERROR;
    }
    rc = ioctl(flashFd, FIOFLUSH/*FIOSYNC*/, 0);
    rc = ioctl(flashFd, FLASH_FS_SYNC, 0);
    close(flashFd);
    return rc;
}

STATUS
flashFsIoctl(BLK_DEV *pDev, int funcCode, int arg)
{
    int			sectorAfterBoot;

    if (flashFsVerbose > 1)
	printf("flashFsBlkIoctl(): called\n");

    switch (funcCode) {
    case FIODISKFORMAT:
	/* Clear flashDrvLib's cached sector */
	if (flashDrvLibInit() == ERROR) {
	    printf("flashFsLibInit(): flashDrvLibInit() failed\n");
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

int
tstFlashFile(void)
{
    int             fh;
    char            buff[20];

    printf("creating\n");
    fh = creat("myfile", O_RDWR);
    if (fh < 0) {
	printf("tstFlashFile(): create() failed\n");
	return (ERROR);
    }

    printf("writing\n");
    write(fh, buff, 10);

    printf("closing\n");
    close(fh);

    return (0);

}

int
tstFlashFile_1(void)
{
    int             fh;
    char            buff[40];

    printf("opening\n");
    fh = open("myfile", O_RDWR, 0664);
    if (fh < 0) {
	printf("tstFlashFile_1(): open() failed\n");
	return (ERROR);
    }

    printf("writing\n");
    write(fh, buff, 10);
    printf("writing\n");
    write(fh, buff, 23);

    printf("closing\n");
    close(fh);

    return (0);
}

STATUS
flashFsLibInit(void)
{
    int         bootSize;

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
     * Determine where our file system starts
     */

    if (flashFsFindBootPartition(&bootSize) == OK) {
        flashBootSize = bootSize;
    }
    printf("Flash boot partition size = %d KB\n", flashBootSize / 1024);

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
	printf("flashFsLibInit: Initializing...");
    }


    if (!flashDosFormat) {
        char fsmName[12];
        sprintf (fsmName, "%s:0", FLASH_FS_NAME); /* Create name for mapping. */
        fsmNameInstall (fsmName, FLASH_FS_NAME);  /* Install name mapping. */

        if ((flashXbdDev = 
             xbdBlkDevCreate(&flashBlkDev, FLASH_FS_NAME)) == NULLDEV) {
            printf("flashFsLibInit: failed to created xdb device !!\n");
            return (ERROR);
        }

        if (dosFsDevCreate(FLASH_FS_NAME, flashXbdDev, 
                           DOSFS_DEFAULT_MAX_FILES, DOS_CHK_NONE)) {
            printf("Failed to create DOS file device \n");
            return (ERROR);
        }

        flashDosVolDesc = dosFsVolDescGet(FLASH_FS_NAME, NULL);

        if (flashDosVolDesc == NULL) {
            if (flashFsVerbose) {
                printf("\nflashFsLibInit: first time initialization...\n");
            }

            dosFsVolFormat(FLASH_FS_NAME, DOS_OPT_VXLONGNAMES, NULL);
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
   /* if (flashDosVolDesc) {  */
	if (flashFsVerbose) {
	    printf("flashFsSync(): Syncing...\n");
	}
	flashSyncFilesystem();
	if (flashFsVerbose) {
	    printf("done\n");
	}
   /*  } */
    return OK;
}

/*
 * Disable disk cache
 */
STATUS
flashFsDisableDcache(void)
{
    if (flashFsVerbose)
        printf("flashFsDisableDcache\n");

    /* Disable disk cache - Hack */
    taskDelete(dcacheUpdTaskId);
    dcacheUpdTaskId = 0;

#if 0
    CBIO_DEV_ID dcacheHandle;
    void *devHandle;
    char fullname[20];

    /* Set the disk cache sync delay to 0 */
    devHandle = iosDevFind("flash:", &fullname);
    dcacheHandle = *(CBIO_DEV_ID*)((int)devHandle + 24);   /* FIXME */
    if (flashFsVerbose) {
        printf("dcacheDevTune - devHandle=0x%x, dcacheHandle=0x%x\n",
                (int)devHandle, (int)dcacheHandle);
    }
    if (dcacheDevTune(dcacheHandle, 0, 0, 0, 0) != OK) {
        printf("flashFsDisableDcache: dcacheDevTune failed\n");
        return ERROR;
    }

    dcacheDevDisable(dcacheHandle);   /* only for dosFS 2.2 */
#endif

    /* dcacheShow(dcacheHandle, 0); */

    return OK;
}
