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
int             flashDosFormat = 0;
int             flashBootSize = 512*1024;
int             flashFsSize = 0;

int             flashFsSectorStart = 0; /* 5 for A1 */
int             flash_boot_start  = 0x1c00000; /* 0x0 for A1 */
int             flash_nvram_start = 0x1c90000; /* 0x90000 for A1 */

DOS_VOL_DESC   *flashDosVolDesc = NULL;

/* $Id: flashFsLib.c,v 1.8 2011/07/21 16:14:55 yshtil Exp $
 * The boot ROM resides at the begining of the onboard flash, from 0 MB
 * through 0 MB plus 512 KB if VxBoot, or from 0 MB through 1 MB if CFE.
 *
 * CFE may request a new partitioning scheme by embedding a partition
 * table in the flash.
 *
 * flashFsGetPhys remaps logical into physical sectors in such as way as
 * to skip over the two boot sectors.
 */

int             flashFsVerbose = 0;

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
flashFsCheckPtable(ptable_t *ptbl, const char *pname,
                   int *psize, int *poffset)
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
            if (strcmp(ptbl->part[i].name, pname) == 0) {
                *psize = ptbl->part[i].size;
                *poffset = ptbl->part[i].offset;
                return OK;
            }
        }
        return ERROR;
}

STATUS
flashFsFindBootPartition(int *bootSize, int *bootOffset)
{
    int         offset, sect, sectSize, numSectors;
    char        *buff;
    ptable_t    *ptbl;

    sectSize = flashGetSectorSize();
    /* scan for 1MB around boot sector for ptable. */
    numSectors = 0x100000/sectSize;

    if ((buff = malloc(sectSize)) == NULL) {
        printf("flashFindBootPartition(): malloc() failed\n");
        return ERROR;
    }

    for (sect = 0; sect < numSectors; sect++) {
        if (flashBlkRead(sect+FLASH_BOOT_START_SECTOR, 
                         buff, 0, sectSize) == ERROR) {
            printf("flashFindBootPartition(): flashBlkRead() failed\n");
            return ERROR;
        }
        for (offset = 0; offset < sectSize; offset += 0x8000) {
            ptbl = (ptable_t *)&buff[offset];
            if (flashFsCheckPtable(ptbl, "boot", bootSize, bootOffset) 
                                                                 == OK) {
                printf("Found flash boot partition table "
                       "at offset 0x%08x Size 0x%08x flash offset 0x%08x\n", 
                       (sect + FLASH_BOOT_START_SECTOR) * sectSize + offset,
                       *bootSize, *bootOffset);
                free(buff);
                return OK;
            }
        }
    }
    free(buff);
    return ERROR;
}

STATUS
flashFsFindVxworksPartition(int *pSize, int *pOffset)
{
    int         offset, sect, sectSize, numSectors;
    char        *buff;
    ptable_t    *ptbl;

    sectSize = flashGetSectorSize();
    numSectors = 0x100000/sectSize;

    if ((buff = malloc(sectSize)) == NULL) {
        printf("flashFindBootPartition(): malloc() failed\n");
        return ERROR;
    }

    for (sect = 0; sect < numSectors; sect++) {
        if (flashBlkRead(sect+FLASH_BOOT_START_SECTOR, 
                         buff, 0, sectSize) == ERROR) {
            printf("flashFindBootPartition(): flashBlkRead() failed\n");
            return ERROR;
        }
        for (offset = 0; offset < sectSize; offset += 0x8000) {
            ptbl = (ptable_t *)&buff[offset];
            if ((flashFsCheckPtable(ptbl, "vxworks", pSize, pOffset) == OK) ||
                (flashFsCheckPtable(ptbl, "fatfs", pSize, pOffset) == OK) ||
                (flashFsCheckPtable(ptbl, "dosfs", pSize, pOffset) == OK)) {
                goto found;
            }
        }
    }
    free(buff);
    return ERROR;

found:
    printf("Found flash vxworks filesystem partition at offset 0x%08x "
           " Partiion size : 0x%08x Partition offset : 0x%08x\n", 
           (sect + FLASH_BOOT_START_SECTOR) * sectSize + offset,
           *pSize, *pOffset);
    free(buff);
    return OK;
}

STATUS
flashFsGetPhys(int blkNum, int *sectorNum, int *offset)
{
    int		sect;

    if (blkNum >= FLASH_FS_SIZE_BLOCKS)
	return (ERROR);

    sect = blkNum / FLASH_FS_BLOCK_PER_SECTOR;

    sect += flashFsSectorStart;

    if ((flashFsSectorStart < FLASH_BOOT_START_SECTOR) &&
        (sect >= FLASH_BOOT_START_SECTOR)) {
	/* Skip boot sector */
	sect += FLASH_BOOT_SIZE_SECTORS;
    }

    if ((flashFsSectorStart < FLASH_NVRAM_START_SECTOR) &&
        (sect >= FLASH_NVRAM_START_SECTOR)) {
	/* Skip boot sector */
	sect += FLASH_NVRAM_SIZE_SECTORS;
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
    int         bootSize, bootOffset;
    int         fsSize, fsOffset, tmp;

    /*
     * If booting out of boot prom, no need to init FS.
     */
    if (sysIsFlashProm())
    {
        return OK;
    }
    
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

        printf("Rev id is : %d\n", sysGetSocRevId());
    /* Set default boot and NVRAM size based on Raptor A0/A1 */
    if ((sysGetSocRevId() == 0x01) && ((sysGetSocDevId() & 0x00f0) == 0x0010) &&
       ((sysGetSocDevId() & 0x0f00) != 0x0300)) {
        printf("raptor A0 \n");
        flash_boot_start = 0x1c00000;
        flash_nvram_start = 0x1c90000;
        flashFsSectorStart = 0;
    } else {
        flash_boot_start = 0x0;
        flash_nvram_start = 0x90000;
        /* 10 for sector size 64k, 5 for secotr size 128k */
        if (flashDevSectorSize == 0x10000) {
            flashFsSectorStart = 10;
        } else {
            flashFsSectorStart = 5;
        }
    }

    /* Assume boot sector to be 512kb initially. */
    flashBootSize = 512*1024; 
    bootOffset = FLASH_BOOT_START;

    /* Assume using enire flash */
    flashFsSize = flashSize - flashBootSize - FLASH_SECTOR_SIZE /* nvram sector*/;

    /*
     * find boot sector and its size.
     */
    if (flashFsFindBootPartition(&bootSize, &bootOffset) == OK) {
        flashBootSize = bootSize;
        flashFsSize = flashSize - flashBootSize - FLASH_SECTOR_SIZE;
        if (flashFsVerbose) {
            printf("Flash boot partition size = %d KB\n", 
                   flashBootSize / 1024);
        }
    } 

    /*
     * Find vxworks filesystem partition. If found adjust flash driver
     * accordingly.
     */
    if (flashFsFindVxworksPartition(&fsSize, &fsOffset) == OK) {
        tmp = fsSize;
        if ((fsOffset <= flash_boot_start) && 
            ((fsOffset + tmp) >= (flash_boot_start + flashBootSize))) {
            fsSize -= flashBootSize;
        }
        if ((fsOffset < flash_nvram_start) && 
            ((fsOffset + tmp) >= 
             ((FLASH_NVRAM_START_SECTOR+1)*FLASH_SECTOR_SIZE))) {
            fsSize -= FLASH_SECTOR_SIZE;
        }
        flashFsSize = fsSize;
        flashFsSectorStart = fsOffset/flashGetSectorSize();
        if (flashFsVerbose) {
            printf("Flash filesystem partition size = %d KB\n", 
                  fsSize / 1024);
        }
    }

    printf("FileSystem size %d KB\n", flashFsSize / 1024);

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

    if (!flashDosFormat) {
    flashDosVolDesc = dosFsDevInit(FLASH_FS_NAME, &flashBlkDev, NULL);
    }

    if (flashDosVolDesc == NULL) {
	if (flashFsVerbose) {
	    printf("\nflashFsLibInit: first time initialization...\n");
	}

	if (dosFsMkfsOptionsSet(DOS_OPT_LONGNAMES) == ERROR) {
	    printf("flashFsLibInit: dosFsMkfsOptionsSet failed\n");
	}

	flashDosVolDesc = dosFsMkfs(FLASH_FS_NAME, &flashBlkDev);

	if (flashDosVolDesc == NULL) {
	    printf("flashFsLibInit (first init): dosFsMkfs failed!");
	    printf("(0x%x)\n", errno);
	    return ERROR;
	} else {
	    if (flashFsVerbose) {
		printf("done\n");
	    }
	}
    }

    flashFsSync();

    return OK;
}

STATUS
flashFsSync(void)
{
   /* if (flashDosVolDesc) {*/
	if (flashFsVerbose) {
	    printf("flashFsSync(): Syncing...");
	}
	flashSyncFilesystem();
	if (flashFsVerbose) {
	    printf("done\n");
	}
   /*} */
    return OK;
}
