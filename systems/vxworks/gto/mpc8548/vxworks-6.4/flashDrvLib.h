/*
 * $Id: flashDrvLib.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef	FLASH_DRV_LIB_H
#define	FLASH_DRV_LIB_H

#define FLASH_DEV_COUNT            1

#define	FLASH_BASE_ADDRESS_BOOT    0xFFF00000

#define	FLASH_DEVICE_COUNT              1
#define FLASH_DEVICE_SECTOR_SIZE        (128 * 1024) 

#define FLASH_SECTOR_SIZE (FLASH_DEVICE_COUNT * FLASH_DEVICE_SECTOR_SIZE)

#define FLASH_SIZE                  (128 * 1024 * 1024) 
#define FLASH_SIZE_SECTORS          (1024)

#define FLASH_BASE_ADDRESS          0xf8000000	
#define FLASH_SECTOR_ADDRESS(sector) \
    (FLASH_BASE_ADDRESS + (sector) * FLASH_SECTOR_SIZE)

#define	FLASH_ERASE_TIMEOUT_COUNT	750
#define	FLASH_ERASE_TIMEOUT_TICKS	2

#define	FLASH_PROGRAM_TIMEOUT_POLLS	100000
#define	FLASH_PROGRAM_TIMEOUT_TICKS	0

typedef enum {
    AMD = 0x01,
} FLASH_VENDORS;

typedef enum {
    FLASH_29GL1G = 0x227E
} FLASH_TYPES;

struct flash_drv_funcs_s {
    FLASH_TYPES dev;
    FLASH_VENDORS vendor;
    void (*flashAutoSelect)(FLASH_TYPES *dev, FLASH_VENDORS *vendor);
    int (*flashEraseSector)(int sectorNum);
    int (*flashRead)(int sectorNum, char *buff,
          unsigned int offset, unsigned int count);
    int (*flashWrite)(int sectorNum, char *buff,
                unsigned int offset, unsigned int count);
    int (*flashFlushLoadedSector)(void);
};

extern int             flashVerbose; /* DEBUG */
extern int             flashSectorCount;
extern int             flashDevSectorSize;
extern int             flashSize;
extern unsigned int    flashBaseAddress;
extern struct flash_drv_funcs_s flash29GL1G;

int             flashDrvLibInit(void);
int             flashGetSectorCount(void);
int             flashGetSectorSize(void);
int             flashEraseBank(int firstSector, int nSectors);
int             flashBlkRead(int sectorNum, char *buff,
                unsigned int offset, unsigned int count);
int             flashBlkWrite(int sectorNum, char *buff,
                unsigned int offset, unsigned int count);
int             flashSyncFilesystem(void);
int             flashDiagnostic(void);

#endif /* FLASH_DRV_LIB_H */

