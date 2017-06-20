/* $Id: flashLib.c,v 1.3 2011/07/21 16:14:08 yshtil Exp $
 * Modified Vooha Mousse Board flash driver for BMW
 * by Curt McDowell, 08-06-99, Broadcom Corp.
 */

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vxWorks.h"
#include "taskLib.h"

#include "config.h"
#include "flashLib.h"

#include "bmw.h"

int flashLibDebug = 0;
int flashLibInited = 0;

#define PRINTF			if (flashLibDebug) printf

#define PRIVATE			static

/***********************************************************************
 *
 * Virtual Flash Devices on BMW board
 *
 * These must be kept in sync with the definitions in flashLib.h.
 *
 ***********************************************************************/

flash_dev_t flashDev[] = {
    /* Bank 0 sector SA0 (16 kB) */
    {	FLASH0_BANK, FLASH0_SEG0_START, 1, 14,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
    /* Bank 0 sector SA1 (8 kB) */
    {	FLASH0_BANK, FLASH0_SEG0_START + 0x4000, 1, 13,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
    /* Bank 0 sector SA2 (8 kB) */
    {	FLASH0_BANK, FLASH0_SEG0_START + 0x6000, 1, 13,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
    /* Bank 0 sector SA3 (32 kB) */
    {	FLASH0_BANK, FLASH0_SEG0_START + 0x8000, 1, 15,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
    /* Bank 0 sector SA4 - SA18 1M  - 64KB*/
    {	FLASH0_BANK, FLASH0_SEG1_START, 15, 16,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
    /* Bank 0 sectors SA19-SA26, jumper can occlude this by PLCC (512 kB) */
    /* This is where the Kahlua boot vector and boot ROM code resides. */
    {	FLASH0_BANK, FLASH0_SEG2_START, 8, 16,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
    /* Bank 0 sectors SA27-SA34 (512 kB) */
    {	FLASH0_BANK, FLASH0_SEG3_START, 8, 16,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
};

flash_dev_t flashDev32MB[] = {
    /* Not Used */
    {	FLASH0_BANK, FLASH0_SEG0_START, 1, 14,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
    /* Not used */
    {	FLASH0_BANK, FLASH0_SEG0_START + 0x4000, 1, 13,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
    /* Not used */
    {	FLASH0_BANK, FLASH0_SEG0_START + 0x6000, 1, 13,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
    /* Not used */
    {	FLASH0_BANK, FLASH0_SEG0_START + 0x8000, 1, 15,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
    /* 248 sectors X 128KB = 31MB */
    {	FLASH0_BANK, FLASH2_START, 248, 17,
	FLASH0_VENDOR_ID, FLASH0_32MB_DEVICE_ID
    },
    /* This is where the Kahlua boot vector and boot ROM code resides. */
    /* 128 Kbyte sectors * 4 sectors = 512KB */
    {	FLASH0_BANK, FLASH0_SEG2_START, 4, 17,
	FLASH0_VENDOR_ID, FLASH0_32MB_DEVICE_ID
    },
    /* 128 Kbyte sectors * 4 sectors 512 KB */
    {	FLASH0_BANK, FLASH0_SEG3_START, 4, 17,
	FLASH0_VENDOR_ID, FLASH0_32MB_DEVICE_ID
    },
};

int flashDevCount = (sizeof (flashDev) / sizeof (flashDev[0]));

#define DEV(no)			(&flashDev[no])
#define DEV_NO(dev)		((dev) - flashDev)

/***********************************************************************
 *
 * Private Flash Routines
 *
 ***********************************************************************/

static UINT16 swap16(UINT16 val)
{
    /* Can't be a macro because side effects would load twice */
    return (val << 8 | val >> 8);
}

/*
 * The convention is:
 *
 * "addr" is always the PROM raw address, which is the address of an
 * 8-bit quantity for flash 0 and 16-bit quantity for flash 1.
 *
 * "pos" is always a logical byte position from the PROM beginning.
 */

#define FLASH0_ADDR(dev, addr) \
	((UINT8 *) ((dev)->base + (addr)))

#define FLASH0_WRITE(dev, addr, value) \
	(*FLASH0_ADDR(dev, addr) = (value))

#define FLASH0_READ(dev, addr) \
	(*FLASH0_ADDR(dev, addr))

#define FLASH1_ADDR(dev, addr) \
	((UINT16 *) ((dev)->base + (addr) * 8))

#define FLASH1_WRITE(dev, addr, value) \
	(*FLASH1_ADDR(dev, addr) = dev->swap ? swap16(value) : (value))

#define FLASH1_READ(dev, addr) \
	(dev->swap ? swap16(*FLASH1_ADDR(dev, addr)) : *FLASH1_ADDR(dev, addr))

PRIVATE int flashCheck(flash_dev_t *dev)
{
    if (! flashLibInited) {
	printf("flashCheck: flashLib not initialized\n");
	return ERROR;
    }

    if (dev < &flashDev[0] || dev >= &flashDev[flashDevCount]) {
	printf("flashCheck: Bad dev parameter\n");
	return ERROR;
    }

    if (! dev->found) {
	printf("flashCheck: Device %d not available\n", DEV_NO(dev));
	return ERROR;
    }

    return OK;
}

PRIVATE void flashReset(flash_dev_t *dev)
{
    PRINTF("flashReset: dev=%d\n", DEV_NO(dev));

    if (dev->bank == FLASH0_BANK) {
	FLASH0_WRITE(dev, 0x555, 0xaa);
	FLASH0_WRITE(dev, 0xaaa, 0x55);
	FLASH0_WRITE(dev, 0x555, 0xf0);
    } else {
	FLASH1_WRITE(dev, 0xaaa, 0xaa);
	FLASH1_WRITE(dev, 0x2aa, 0x55);
	FLASH1_WRITE(dev, 0xaaa, 0xf0);
    }

    taskDelay(1);

    PRINTF("flashReset: done\n");
}

PRIVATE int flashProbe(flash_dev_t *dev)
{
    int			rv, deviceID, vendorID;

    PRINTF("flashProbe: dev=%d\n", DEV_NO(dev));

    if (dev->bank == FLASH0_BANK) {
	FLASH0_WRITE(dev, 0xaaa, 0xaa);
	FLASH0_WRITE(dev, 0x555, 0x55);
	FLASH0_WRITE(dev, 0xaaa, 0x90);

	taskDelay(1);

	vendorID = FLASH0_READ(dev, 0);
	deviceID = FLASH0_READ(dev, 2);

	FLASH0_WRITE(dev, 0, 0xf0);
    } else {
	FLASH1_WRITE(dev, 0x555, 0xaa);
	FLASH1_WRITE(dev, 0x2aa, 0x55);
	FLASH1_WRITE(dev, 0x555, 0x90);

	taskDelay(1);

	vendorID = FLASH1_READ(dev, 0);
	deviceID = FLASH1_READ(dev, 1);

	FLASH1_WRITE(dev, 0, 0xf0);
    }

    PRINTF("flashProbe: vendor=0x%x device=0x%x\n", vendorID, deviceID);

    if (vendorID == dev->vendorID && deviceID == dev->deviceID)
	rv = OK;
    else
	rv = ERROR;

    PRINTF("flashProbe: rv=%d\n", rv);

    return rv;
}

PRIVATE int flashWait(flash_dev_t *dev, int addr, int expect, int erase)
{
    int			rv = ERROR;
    int			i, data;
    int			polls;

#if 0
    PRINTF("flashWait: dev=%d addr=0x%x expect=0x%x erase=%d\n",
	   DEV_NO(dev), addr, expect, erase);
#endif

    if (erase)
	polls = FLASH_ERASE_SECTOR_TIMEOUT;	/* Ticks */
    else
	polls = FLASH_PROGRAM_POLLS;		/* Loops */

    for (i = 0; i < polls; i++) {
	if (erase)
	    taskDelay(1);

	if (dev->bank == FLASH0_BANK)
	    data = FLASH0_READ(dev, addr);
	else
	    data = FLASH1_READ(dev, addr);

	if (((data ^ expect) & 0x80) == 0) {
	    rv = OK;
	    goto done;
	}

	if (data & 0x20) {
	    /*
	     * If the 0x20 bit has come on, it could actually be because
	     * the operation succeeded, so check the done bit again.
	     */

	    if (dev->bank == FLASH0_BANK)
		data = FLASH0_READ(dev, addr);
	    else
		data = FLASH1_READ(dev, addr);

	    if (((data ^ expect) & 0x80) == 0) {
		rv = OK;
		goto done;
	    }

	    printf("flashWait: Program error (dev: %d, addr: 0x%x)\n",
		   DEV_NO(dev), addr);

	    flashReset(dev);
	    goto done;
	}
    }

    printf("flashWait: Timeout %s (dev: %d, addr: 0x%x)\n",
	   erase ? "erasing sector" : "programming byte",
	   DEV_NO(dev), addr);

 done:

#if 0
    PRINTF("flashWait: rv=%d\n", rv);
#endif

    return rv;
}

/***********************************************************************
 *
 * Public Flash Routines
 *
 ***********************************************************************/

STATUS flashLibInit(void)
{
    int			i;

    PRINTF("flashLibInit: devices=%d\n", flashDevCount);

    for (i = 0; i < flashDevCount; i++) {
	flash_dev_t	*dev = &flashDev[i];

	flashReset(dev);

	if (flashProbe(dev) != ERROR) {
	    dev->found = 1;
        } else {
            /* Probe if 32MB flash exists */
            flashDev[i] = flashDev32MB[i];
            if (flashProbe(dev) != ERROR) {
                dev->found = 1;
            }
        }
        if (dev->found) {
            PRINTF("flashLibInit: dev=%d base = 0x%08X sectors = %d SectorSize= %d\n", i, dev->base, dev->sectors, dev->lgSectorSize);
        }
    }

    flashLibInited = 1;

    PRINTF("flashLibInit: done\n");

    return OK;
}

STATUS flashEraseSector(flash_dev_t *dev, int sector)
{
    int			pos, addr;

    PRINTF("flashErasesector: dev=%d sector=%d\n", DEV_NO(dev), sector);

    if (flashCheck(dev) == ERROR)
	return ERROR;

    if (sector < 0 || sector >= dev->sectors) {
	printf("flashEraseSector: Sector out of range (dev: %d, sector: %d)\n",
	       DEV_NO(dev), sector);
	return ERROR;
    }

    pos = FLASH_SECTOR_POS(dev, sector);

    if (dev->bank == FLASH0_BANK) {
	addr = pos;

	FLASH0_WRITE(dev, 0xaaa, 0xaa);
	FLASH0_WRITE(dev, 0x555, 0x55);
	FLASH0_WRITE(dev, 0xaaa, 0x80);
	FLASH0_WRITE(dev, 0xaaa, 0xaa);
	FLASH0_WRITE(dev, 0x555, 0x55);
	FLASH0_WRITE(dev, addr, 0x30);
    } else {
	addr = pos / 2;

	FLASH1_WRITE(dev, 0x555, 0xaa);
	FLASH1_WRITE(dev, 0x2aa, 0x55);
	FLASH1_WRITE(dev, 0x555, 0x80);
	FLASH1_WRITE(dev, 0x555, 0xaa);
	FLASH1_WRITE(dev, 0x2aa, 0x55);
	FLASH1_WRITE(dev, addr, 0x30);
    }

    return flashWait(dev, addr, 0xff, 1);
}

/*
 * Note: it takes about as long to flash all sectors together with Chip
 * Erase as it does to flash them one at a time (about 30 seconds for 2
 * MB).  Also since we want to be able to treat subsets of sectors as if
 * they were complete devices, we don't use Chip Erase.
 */

STATUS flashErase(flash_dev_t *dev)
{
    int			sector;

    PRINTF("flashErase: dev=%d sectors=%d\n", DEV_NO(dev), dev->sectors);

    if (flashCheck(dev) == ERROR)
	return ERROR;

    for (sector = 0; sector < dev->sectors; sector++)
	if (flashEraseSector(dev, sector) == ERROR)
	    return ERROR;

    return OK;
}

/*
 * Read and write bytes
 */

STATUS flashRead(flash_dev_t *dev, int pos, char *buf, int len)
{
    int			addr, words;

    PRINTF("flashRead: dev=%d pos=0x%x buf=0x%x len=0x%x\n",
	   DEV_NO(dev), pos, (int) buf, len);

    if (flashCheck(dev) == ERROR)
	return ERROR;

    if (pos < 0 || len < 0 || pos + len > FLASH_MAX_POS(dev)) {
	printf("flashRead: Position out of range "
	       "(dev: %d, pos: 0x%x, len: 0x%x)\n",
	       DEV_NO(dev), pos, len);
	return ERROR;
    }

    if (len == 0)
	return OK;

    if (dev->bank == FLASH0_BANK) {
	addr = pos;
	words = len;

	PRINTF("flashRead: memcpy(0x%x, 0x%x, 0x%x)\n",
	       (int) buf, (int) FLASH0_ADDR(dev, pos), len);

	memcpy(buf, FLASH0_ADDR(dev, addr), words);

	return OK;
    }

    if ((pos | (UINT32) buf | len) & 1) {
	printf("flashRead: Unaligned parameter "
	       "(dev: %d, pos: 0x%x, buf: 0x%x, len: 0x%x)\n",
	       DEV_NO(dev), pos, (int) buf, len);
	return ERROR;
    }

    addr = pos / 2;
    words = len / 2;

    while (words--) {
	*(UINT16 *) buf = FLASH1_READ(dev, addr);
	addr++;
	buf += 2;
    }

    PRINTF("flashRead: rv=OK\n");

    return OK;
}

STATUS flashWrite(flash_dev_t *dev, int pos, char *buf, int len)
{
    int 		addr, words;

    PRINTF("flashWrite: dev=%d pos=0x%x buf=0x%x len=0x%x\n",
	   DEV_NO(dev), pos, (int) buf, len);

    if (flashCheck(dev) == ERROR)
	return ERROR;

    if (pos < 0 || len < 0 || pos + len > FLASH_MAX_POS(dev)) {
	printf("flashWrite: Position out of range "
	       "(dev: %d, pos: 0x%x, len: 0x%x)\n",
	       DEV_NO(dev), pos, len);
	return ERROR;
    }

    if (len == 0)
	return OK;

    if (dev->bank == FLASH0_BANK) {
	UINT8 tmp;

	addr = pos;
	words = len;

	while (words--) {
	    tmp = *buf;
	    if (~FLASH0_READ(dev, addr) & tmp) {
		printf("flashWrite: Attempt to program 0 to 1 "
		       "(dev: %d, addr: 0x%x, data: 0x%x)\n",
		       DEV_NO(dev), addr, tmp);
		return ERROR;
	    }
	    FLASH0_WRITE(dev, 0xaaa, 0xaa);
	    FLASH0_WRITE(dev, 0x555, 0x55);
	    FLASH0_WRITE(dev, 0xaaa, 0xa0);
	    FLASH0_WRITE(dev, addr, tmp);
	    if (flashWait(dev, addr, tmp, 0) < 0)
		return ERROR;
	    buf++;
	    addr++;
	}
    } else {
	UINT16 tmp;

	if ((pos | (UINT32) buf | len) & 1) {
	    printf("flashWrite: Unaligned parameter "
		   "(dev: %d, pos: 0x%x, buf: 0x%x, len: 0x%x)\n",
		   DEV_NO(dev), pos, (int) buf, len);
	    return ERROR;
	}

	addr = pos / 2;
	words = len / 2;

	while (words--) {
	    tmp = *(UINT16 *) buf;
	    if (~FLASH1_READ(dev, addr) & tmp) {
		printf("flashWrite: Attempt to program 0 to 1 "
		       "(dev: %d, addr: 0x%x, data: 0x%x)\n",
		       DEV_NO(dev), addr, tmp);
		return ERROR;
	    }
	    FLASH1_WRITE(dev, 0x555, 0xaa);
	    FLASH1_WRITE(dev, 0x2aa, 0x55);
	    FLASH1_WRITE(dev, 0x555, 0xa0);
	    FLASH1_WRITE(dev, addr, tmp);
	    if (flashWait(dev, addr, tmp, 0) < 0)
		return ERROR;
	    buf += 2;
	    addr++;
	}
    }

    PRINTF("flashWrite: rv=OK\n");

    return OK;
}

/*
 * flashWritable returns TRUE if a range contains all F's.
 */

STATUS flashWritable(flash_dev_t *dev, int pos, int len)
{
    int 		addr, words;
    int			rv = ERROR;

    PRINTF("flashWritable: dev=%d pos=0x%x len=0x%x\n",
	   DEV_NO(dev), pos, len);

    if (flashCheck(dev) == ERROR)
	goto done;

    if (pos < 0 || len < 0 || pos + len > FLASH_MAX_POS(dev)) {
	printf("flashWritable: Position out of range "
	       "(dev: %d, pos: 0x%x, len: 0x%x)\n",
	       DEV_NO(dev), pos, len);
	goto done;
    }

    if (len == 0) {
	rv = 1;
	goto done;
    }

    if (dev->bank == FLASH0_BANK) {
	addr = pos;
	words = len;

	while (words--) {
	    if (FLASH0_READ(dev, addr) != 0xff) {
		rv = 0;
		goto done;
	    }
	    addr++;
	}
    } else {
	if ((pos | len) & 1) {
	    printf("flashWritable: Unaligned parameter "
		   "(dev: %d, pos: 0x%x, len: 0x%x)\n",
		   DEV_NO(dev), pos, len);
	    return ERROR;
	}

	addr = pos / 2;
	words = len / 2;

	while (words--) {
	    if (FLASH1_READ(dev, addr) != 0xffff) {
		rv = 0;
		goto done;
	    }
	    addr++;
	}
    }

    rv = 1;

 done:
    PRINTF("flashWrite: rv=%d\n", rv);
    return rv;
}

/***********************************************************************
 *
 * Flash Diagnostics
 *
 ***********************************************************************/

STATUS flashDiag(flash_dev_t *dev)
{
    UINT32		*buf = 0;
    int			i, len, sector;
    int			rv = ERROR;

    if (flashCheck(dev) == ERROR)
	return ERROR;

    printf("flashDiag: Testing device %d, "
	   "base: 0x%x, %d sectors @ %d kB = %d kB\n",
	   DEV_NO(dev), dev->base,
	   dev->sectors,
	   1 << (dev->lgSectorSize - 10),
	   dev->sectors << (dev->lgSectorSize - 10));

    len = 1 << dev->lgSectorSize;

    printf("flashDiag: Erasing\n");

    if (flashErase(dev) == ERROR) {
	printf("flashDiag: Erase failed\n");
	goto done;
    }

    buf = malloc(len);

    if (buf == 0) {
	printf("flashDiag: Out of memory\n");
	goto done;
    }

    /*
     * Write unique counting pattern to each sector
     */

    for (sector = 0; sector < dev->sectors; sector++) {
	printf("flashDiag: Write sector %d\n", sector);

	for (i = 0; i < len / 4; i++)
	    buf[i] = sector << 24 | i;

	if (flashWrite(dev,
		       sector << dev->lgSectorSize,
		       (char *) buf,
		       len) == ERROR) {
	    printf("flashDiag: Write failed (dev: %d, sector: %d)\n",
		   DEV_NO(dev), sector);
	    goto done;
	}
    }

    /*
     * Verify
     */

    for (sector = 0; sector < dev->sectors; sector++) {
	printf("flashDiag: Verify sector %d\n", sector);

	if (flashRead(dev,
		      sector << dev->lgSectorSize,
		      (char *) buf,
		      len) == ERROR) {
	    printf("flashDiag: Read failed (dev: %d, sector: %d)\n",
		   DEV_NO(dev), sector);
	    goto done;
	}

	for (i = 0; i < len / 4; i++) {
	    if (buf[i] != (sector << 24 | i)) {
		printf("flashDiag: Verify error "
		       "(dev: %d, sector: %d, offset: 0x%x)\n",
		       DEV_NO(dev), sector, i);
		printf("flashDiag: Expected 0x%08x, got 0x%08x\n",
		       sector << 24 | i, buf[i]);

		goto done;
	    }
	}
    }

    printf("flashDiag: Erasing\n");

    if (flashErase(dev) == ERROR) {
	printf("flashDiag: Final erase failed\n");
	goto done;
    }

    rv = OK;

 done:
    if (buf)
	free(buf);

    if (rv == OK)
	printf("flashDiag: Device %d passed\n", DEV_NO(dev));
    else
	printf("flashDiag: Device %d failed\n", DEV_NO(dev));

    return rv;
}

STATUS flashDiagAll(void)
{
    int			i;
    int			rv = OK;

    PRINTF("flashDiagAll: devices=%d\n", flashDevCount);

    for (i = 0; i < flashDevCount; i++) {
	flash_dev_t	*dev = &flashDev[i];

	if (dev->found && flashDiag(dev) == ERROR)
	    rv = ERROR;
    }

    if (rv == OK)
	printf("flashDiagAll: Passed\n");
    else
	printf("flashDiagAll: Failed because of earlier errors\n");

    return OK;
}

#if 0
#define FLASH_BASE_ADDRESS 0xfff00000
#define FLASH_ADDR(dev, addr) \
    ((volatile UINT8 *) (((int)(dev) + (int)(addr)) ^ xor_val))

#define FLASH_WRITE(dev, addr, value) \
    (*FLASH_ADDR(dev, addr) = (value))

#define FLASH_READ(dev, addr) \
    (*FLASH_ADDR(dev, addr))

LOCAL void
flashReadReset(void)
{
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0xaaa, 0xf0);
}

LOCAL void
flashAutoSelect(FLASH_TYPES *dev, FLASH_VENDORS *vendor)
{
    flashReadReset();   
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0xaaa, 0xaa);
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0x555, 0x55);
    FLASH_WRITE(FLASH_BASE_ADDRESS, 0xaaa, 0x90);

    *vendor = FLASH_READ(FLASH_BASE_ADDRESS, 0);
    *dev = FLASH_READ(FLASH_BASE_ADDRESS, 2);

    if (flashVerbose)
        printf("flashAutoSelect(): dev = 0x%x, vendor = 0x%x\n",
               (int)*dev, (int)*vendor);
    flashReadReset();   

    if ((*dev != FLASH_2L160) || ((*vendor != AMD) && (*vendor != ALLIANCE))) {
        *vendor = *dev = 0xFF;
    }
}
#endif
