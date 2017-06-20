/* $Id: flashFsLib.c,v 1.5 2011/07/21 16:14:08 yshtil Exp $
 * Vooha Mousse Board flash filesystem driver
 * by Curt McDowell, 08-06-99, Broadcom Corp.
 */

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ioLib.h>

#include "vxWorks.h"
#include "dosFsLib.h"
#include "fioLib.h"
#include "blkIo.h"
#include "flashLib.h"
#include "flashFsLib.h"
#include "errnoLib.h"

#include "config.h"

#if 0
#define PRIVATE			static
#else
#define PRIVATE
#endif

PRIVATE BLK_DEV		 flashBlkDev;
PRIVATE DOS_VOL_CONFIG	 flashVolConfig;
#if VX_VERSION == 62
static device_t        flashFsLibXbdDev = 0;
extern device_t xbdBlkDevCreate      (BLK_DEV *bd, const char *name);
DOS_VOLUME_DESC_ID   flashDosVolDesc = 0;
#else
DOS_VOL_DESC   *flashDosVolDesc = NULL;
#endif


/*
 * Flash Filesystem Layout
 *
 * flashLib combines the following flashLib.c devices into a single
 * logical device (each piece having only 64 kB sectors):
 *
 *   Device			Size		DOS Blocks	Range
 *   -------------------	-------		----		---------
 *   FLASH_DEV_BANK0_LOW	 960 kB		1920		0-1919
 *   FLASH_DEV_BANK0_HIGH	 512 kB		1024		1920-2947
 *
 *   Logical device		1472 kB		2948		0-2947
 *
 * See bmw.h and flashLib.c for description of flash memory layout.
 */

typedef struct flash_vol_s {
    flash_dev_t	       *dev;
    int			blocks;		/* Number of DOS blocks on volume */
    int			lgSectorSize;	/* Copied from flash device */
    void	       *bufData;	/* Buffered write sector, 0 if none */
    int			bufSector;	/* Buffered data sector number */
} flash_vol_t;

flash_vol_t flashVol[] = {
    {	FLASH_DEV_BANK0_LOW	},
    {	FLASH_DEV_BANK0_HIGH	},
};

int flashVolCount = sizeof (flashVol) / sizeof (flashVol[0]);

#define	DOS_SECTOR_SIZE_LG	9
#define	DOS_SECTOR_SIZE		(1 << DOS_SECTOR_SIZE_LG)

#define VERBOSE_BLKFILL		0x0001
#define VERBOSE_BLKFLUSH	0x0002
#define VERBOSE_BLKINVAL	0x0004
#define VERBOSE_FLASHES		0x0008
#define VERBOSE_GETPHYS		0x0010
#define VERBOSE_BLKREAD		0x0020
#define VERBOSE_BLKWRITE	0x0040
#define VERBOSE_IOCTL		0x0080
#define VERBOSE_SYNC		0x0100

int flashFsVerbose = 0;
int flashFsTotalBlocks = 0;
int flashFsInited = 0;

PRIVATE STATUS flashFsBlkInvalAll(void)
{
    int			volnum;

    if (flashFsVerbose & VERBOSE_BLKINVAL)
	printf("flashFsBlkInvalAll: entered\n");

    for (volnum = 0; volnum < flashVolCount; volnum++) {
	flash_vol_t	*vol = &flashVol[volnum];

	if (vol->bufData != 0) {
	    free(vol->bufData);
	    vol->bufData = 0;
	}
    }

    return OK;
}

PRIVATE STATUS flashFsBlkFill(int volnum, int sector)
{
    flash_vol_t		*vol = &flashVol[volnum];

    if (flashFsVerbose & VERBOSE_BLKFILL)
	printf("flashFsBlkFill: vol=%d, sector=%d\n", volnum, sector);

    if (vol->bufData)
	return ERROR;

    vol->bufData = malloc(1 << vol->lgSectorSize);

    if (vol->bufData == 0) {
	printf("flashFsBlkFill: Out of memory\n");
	return ERROR;
    }

    if (flashRead(vol->dev,
		  sector << vol->lgSectorSize,
		  vol->bufData,
		  1 << vol->lgSectorSize) == ERROR)
	return ERROR;

    vol->bufSector = sector;

    if (flashFsVerbose & VERBOSE_BLKFILL)
	printf("flashFsBlkFill: rv=OK\n");

    return OK;
}

PRIVATE STATUS flashFsBlkFlush(int volnum)
{
    flash_vol_t		*vol = &flashVol[volnum];
    int			try;

    if (flashFsVerbose & VERBOSE_BLKFLUSH)
	printf("flashFsBlkFlush: vol=%d\n", volnum);

    if (vol->bufData != 0) {
	if (flashFsVerbose & VERBOSE_FLASHES)
	    printf("flashFsBlkFlush: erase vol=%d sector=%d\n",
		   volnum, vol->bufSector);

	for (try = 1;
	     flashEraseSector(vol->dev, vol->bufSector) == ERROR;
	     try++) {
	    if (try == 3)
		return ERROR;
	    printf("flashFsBlkFlush: retrying erase (sector %d)\n",
		   vol->bufSector);
	}

	if (flashWrite(vol->dev,
		       vol->bufSector << vol->lgSectorSize,
		       vol->bufData,
		       1 << vol->lgSectorSize) == ERROR)
	    return ERROR;

	free(vol->bufData);
	vol->bufData = 0;
    }

    if (flashFsVerbose & VERBOSE_BLKFLUSH)
	printf("flashFsBlkFlush: rv=OK\n");

    return OK;
}

PRIVATE STATUS flashFsBlkFlushAll(void)
{
    int		volnum;

    if (flashFsVerbose & VERBOSE_BLKFLUSH)
	printf("flashFsBlkFlushAll: entered\n");

    for (volnum = 0; volnum < flashVolCount; volnum++)
	if (flashFsBlkFlush(volnum) == ERROR)
	    return ERROR;

    if (flashFsVerbose & VERBOSE_BLKFLUSH)
	printf("flashFsBlkFlushAll: rv=OK\n");

    return OK;
}

PRIVATE STATUS flashFsBlkEraseAll(void)
{
    int		volnum;

    flashFsBlkInvalAll();

    for (volnum = 0; volnum < flashVolCount; volnum++)
	if (flashErase(flashVol[volnum].dev) == ERROR)
	    return ERROR;

    return OK;
}

PRIVATE STATUS flashFsGetPhys(int blkNum, int *volnum_p, int *pos_p)
{
    int		volnum, block = blkNum;

    for (volnum = 0; volnum < flashVolCount; volnum++) {
	flash_vol_t	*vol = &flashVol[volnum];

	if (block < vol->blocks) {
	    *volnum_p = volnum;
	    break;
	}

	block -= vol->blocks;
    }

    if (volnum == flashVolCount) {
	printf("flashFsGetPhys: DOS block %d out of range\n", blkNum);
	return ERROR;
    }

    *pos_p = block * DOS_SECTOR_SIZE;

    if (flashFsVerbose & VERBOSE_GETPHYS)
	printf("flashFsGetPhys: blkNum = %d, volnum = %d, pos = 0x%08x\n",
	       blkNum, volnum, *pos_p);

    return OK;
}

PRIVATE STATUS flashFsBlkRead(BLK_DEV *pDev,
			      int startBlk, int numBlks, char *buf)
{
    int			block, pos, volnum;

    if (flashFsVerbose & VERBOSE_BLKREAD)
	printf("flashFsBlkRead: startBlk = %d, numBlks = %d\n",
	       startBlk, numBlks);

    for (block = 0; block < numBlks; block++) {
	int		sector, offset;
	flash_vol_t	*vol;

	if (flashFsGetPhys(startBlk + block, &volnum, &pos) == ERROR)
	    return ERROR;

	vol = &flashVol[volnum];

	sector = (pos >> vol->lgSectorSize);
	offset = pos - (sector << vol->lgSectorSize);

	if (vol->bufData && vol->bufSector == sector) {
	    /*
	     * Block is in buffered sector.  Read from there.
	     */

	    memcpy(buf, vol->bufData + offset, DOS_SECTOR_SIZE);
	} else {
	    /*
	     * Read block directly from PROM.
	     */

	    if (flashRead(vol->dev, pos, buf, DOS_SECTOR_SIZE) == ERROR)
		return ERROR;
	}

	buf += DOS_SECTOR_SIZE;
    }

    return OK;
}

PRIVATE STATUS flashFsBlkWrite(BLK_DEV *pDev,
			       int startBlk, int numBlks, char *buf)
{
    int			block, pos, volnum;

    if (flashFsVerbose & VERBOSE_BLKWRITE)
	printf("flashFsBlkWrite(): startBlk = %d, numBlks = %d\n",
	       startBlk, numBlks);

    for (block = 0; block < numBlks; block++) {
	int		sector, offset;
	flash_vol_t	*vol;

	if (flashFsGetPhys(startBlk + block, &volnum, &pos) == ERROR)
	    return ERROR;

	vol = &flashVol[volnum];

	sector = (pos >> vol->lgSectorSize);
	offset = pos - (sector << flashVol[volnum].lgSectorSize);

	if (vol->bufData && vol->bufSector == sector) {
	    /*
	     * Block is in dirty buffered sector, so write to there.
	     */

	    memcpy(vol->bufData + offset, buf, DOS_SECTOR_SIZE);
	} else {
	    int		rv;

	    /*
	     * If block is not in buffered sector, but the corresponding
	     * block in the PROM contains all F's, write directly to
	     * PROM without flashing.
	     */

	    rv = flashWritable(vol->dev, pos, DOS_SECTOR_SIZE);

	    if (rv == ERROR)
		return ERROR;

	    if (rv) {
		/*
		 * Safe to write in place
		 */

		if (flashWrite(vol->dev, pos, buf, DOS_SECTOR_SIZE) == ERROR)
		    return ERROR;
	    } else {
		/*
		 * Must flush old dirty buffer (if any),
		 * and create a new dirty buffer.
		 */

		if (flashFsBlkFlush(volnum) == ERROR)
		    return ERROR;

		if (flashFsBlkFill(volnum, sector) == ERROR)
		    return ERROR;

		memcpy(vol->bufData + offset, buf, DOS_SECTOR_SIZE);
	    }
	}

	buf += DOS_SECTOR_SIZE;
    }

    return OK;
}

PRIVATE STATUS flashFsIoctl(BLK_DEV *pDev, int funcCode, int arg)
{
    if (flashFsVerbose & VERBOSE_IOCTL)
	printf("flashFsIoctl(): called, func=%d\n", funcCode);

    switch (funcCode) {
    case FIODISKFORMAT:
        if (flashLibInit() == ERROR) {
            printf("flashFsLibInit(): flashLibInit() failed\n");
            return (ERROR);
        }

	if (flashFsBlkEraseAll() == ERROR)
	    return ERROR;
	break;

    case FIOFLASHSYNC:
	if (flashFsBlkFlushAll() == ERROR)
	    return ERROR;
	break;

    case FIOFLASHINVAL:
	if (flashFsBlkInvalAll() == ERROR)
	    return ERROR;
	break;

    default:
	errnoSet(S_ioLib_UNKNOWN_REQUEST);
	return ERROR;
	break;
    }

    return OK;
}

STATUS flashFsLibInit(void)
{
    int		volnum;

    if (flashLibInit() == ERROR) {
        printf("flashFsLibInit(): flashLibInit() failed\n");
        return (ERROR);
    }

    /*
     * We are considered initialized once flashDosVolDesc is non-NULL.
     */

    if (flashDosVolDesc)
	return OK;

    /*
     * Toss buffers if they already exist.  That could happen if
     * flashFsLibInit fails and ends up getting called again.
     */

    flashFsBlkInvalAll();

    flashFsTotalBlocks = 0;

    for (volnum = 0; volnum < flashVolCount; volnum++) {
	flash_vol_t	*vol = &flashVol[volnum];

	if (! vol->dev->found) {
	    printf("flashFsLibInit: missing volume %d\n", volnum);
	    return ERROR;
	}

	vol->blocks = FLASH_MAX_POS(vol->dev) / DOS_SECTOR_SIZE;
	vol->lgSectorSize = vol->dev->lgSectorSize;

	flashFsTotalBlocks += flashVol[volnum].blocks;
    }

    /*
     * Set up BLK DEV structure
     */

    flashBlkDev.bd_blkRd		= flashFsBlkRead;
    flashBlkDev.bd_blkWrt		= flashFsBlkWrite;
    flashBlkDev.bd_ioctl		= flashFsIoctl;
    flashBlkDev.bd_reset		= NULL;
    flashBlkDev.bd_statusChk		= NULL;
    flashBlkDev.bd_removable		= FALSE;
    flashBlkDev.bd_nBlocks		= flashFsTotalBlocks;
    flashBlkDev.bd_bytesPerBlk		= DOS_SECTOR_SIZE;
    flashBlkDev.bd_blksPerTrack		= 1;
    flashBlkDev.bd_nHeads		= 1;
    flashBlkDev.bd_retry		= 1;
    flashBlkDev.bd_mode			= O_RDWR;
    flashBlkDev.bd_readyChanged		= FALSE;

#if VX_VERSION == 55 || VX_VERSION == 542
    if (dosFsConfigInit(&flashVolConfig,
			0xf8,		/* media byte			*/
			1,		/* secPerCLuster		*/
			1,		/* num of reserved sectors	*/
			1,		/* num of FAT copies		*/
			29,		/* Sectors per fat		*/
			64,		/* max root entries		*/
			0,		/* num hidden			*/
			DOS_OPT_DEFAULT) != OK) {
	printf("flashFsLibInit: dosFsConfigInit failed\n");
	return ERROR;
    }

    flashDosVolDesc = dosFsDevInit(FLASH_FS_NAME,
				   &flashBlkDev,
                                   NULL);
				   /* &flashVolConfig);*/

    if (flashDosVolDesc == 0) {
	printf("flashFsLibInit: dosFsDevInit failed\n");
	return ERROR;
    }
#endif /* VX_VERSION == 55 || VX_VERSION == 542 */

#if VX_VERSION == 62
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
#endif


    return OK;
}

STATUS flashFsSync(void)
{
    if (flashFsVerbose & VERBOSE_SYNC)
	printf("flashFsSync: entered, vol=0x%x\n", (int) flashDosVolDesc);

    if (flashDosVolDesc)
	flashFsBlkFlushAll();

    return OK;
}

/***********************************************************************
 *
 * Block Buffer Diagnostics
 *
 ***********************************************************************/

STATUS flashFsBlkDiag(void)
{
    int			block, nBlocks, sBlock, i;
    UINT32		buf[DOS_SECTOR_SIZE / 4];
    int			iter;

    if (flashFsLibInit() == ERROR) {
	printf("flashFsBlkDiag: init failed\n");
	return ERROR;
    }

    /*
     * Because of the Grey code we can only test a power of 2 blocks.
     * Round down to a power of 2.
     */

    nBlocks = flashFsTotalBlocks;

    while (nBlocks & (nBlocks - 1))
	nBlocks &= (nBlocks - 1);

    /*
     * Erase before first iteration.
     */

    printf("flashFsBlkDiag: Erasing\n");

    if (flashFsBlkEraseAll() == ERROR) {
	printf("flashDiag: Erase failed\n");
	return ERROR;
    }

    for (iter = 0; iter < 2; iter++) {
	/*
	 * The first iteration will be fast because it will be writing
	 * on top of all F's and will therefore avoid the buffer cache
	 * and avoid flashing.
	 *
	 * The second iteration will be slower because it will be
	 * periodically flashing sectors.
	 */

	printf("flashFsBlkDiag: Writing %d blocks in scrambled order\n",
	       nBlocks);

	for (block = 0; block < nBlocks; block++) {
	    /*
	     * Write blocks in pseudo-random order (Grey code
	     * scrambled).  Generate data so PROM has linearly
	     * incrementing words when done.  Grey code is a good test
	     * because it's highly localized but jumps around too.
	     */

	    sBlock = block ^ (block >> 1);

	    printf("%x ", sBlock);

	    for (i = 0; i < DOS_SECTOR_SIZE / 4; i++)
		buf[i] = sBlock * (DOS_SECTOR_SIZE / 4) + i;

	    if (flashFsBlkWrite(0, sBlock, 1, (char *) buf) == ERROR) {
		printf("\nflashFsBlkDiag: Write failed\n");
		return ERROR;
	    }
	}

	printf("\nflashFsBlkDiag: Verifying %d blocks in scrambled order\n",
	       nBlocks);

	for (block = 0; block < nBlocks; block++) {
	    /*
	     * Verify blocks in same order.  Last block should still be
	     * dirty/unwritten, this would test that.
	     */

	    sBlock = block ^ (block >> 1);

	    printf("%x ", sBlock);

	    if (flashFsBlkRead(0, sBlock, 1, (char *) buf) == ERROR) {
		printf("\nflashFsBlkDiag: Read failed\n");
		return ERROR;
	    }

	    for (i = 0; i < DOS_SECTOR_SIZE / 4; i++)
		if (buf[i] != sBlock * (DOS_SECTOR_SIZE / 4) + i) {
		    printf("\nflashFsBlkDiag: Verify error "
			   "(block: %d, offset: 0x%x)\n",
			   sBlock, i);
		    printf("flashFsBlkDiag: Expected 0x%08x, got 0x%08x\n",
			   sBlock * (DOS_SECTOR_SIZE / 4) + i, buf[i]);
		    return ERROR;
		}
	}

	printf("\nflashFsBlkDiag: Flushing\n");

	if (flashFsBlkFlushAll() == ERROR) {
	    printf("flashFsBlkDiag: Flush failed\n");
	    return ERROR;
	}

	/*
	 * Verify again in linear order
	 */

	printf("flashFsBlkDiag: Verifying %d blocks in linear order\n",
	       nBlocks);

	for (block = 0; block < nBlocks; block++) {
	    if (flashFsBlkRead(0, block, 1, (char *) buf) == ERROR) {
		printf("flashFsBlkDiag: Read failed\n");
		return ERROR;
	    }

	    for (i = 0; i < DOS_SECTOR_SIZE / 4; i++)
		if (buf[i] != block * (DOS_SECTOR_SIZE / 4) + i) {
		    printf("flashFsBlkDiag: Verify error "
			   "(block: %d, offset: 0x%x)\n",
			   block, i);
		    printf("flashFsBlkDiag: Expected 0x%08x, got 0x%08x\n",
			   block * (DOS_SECTOR_SIZE / 4) + i, buf[i]);
		    return ERROR;
		}
	}
    }

    printf("flashFsBlkDiag: Erasing\n");

    if (flashFsBlkEraseAll() == ERROR) {
	printf("flashDiag: Erase failed\n");
	return ERROR;
    }

    printf("flashFsBlkDiag: Passed\n");

    return OK;
}
