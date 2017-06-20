/* $Id: flashDrvLib.h,v 1.4 2011/07/21 16:14:21 yshtil Exp $ */
#ifndef	FLASH_DRV_LIB_H
#define	FLASH_DRV_LIB_H

#define	FLASH_BASE_ADDRESS_PLCC_BOOT    0xBFC00000
#define FLASH_BASE_ADDRESS_FLASH_BOOT   0xBb000000
#define FLASH_BASE_ADDRESS_ALIAS        0xBc000000

#define	FLASH_DEVICE_COUNT              1
#define FLASH_DEVICE_SECTOR_SIZE        flashDevSectorSize

#define FLASH_SECTOR_SIZE (FLASH_DEVICE_COUNT * FLASH_DEVICE_SECTOR_SIZE)

#define FLASH_SIZE                  flashSize
#define FLASH_SIZE_SECTORS          (FLASH_SIZE / FLASH_SECTOR_SIZE)

#define FLASH_BASE_ADDRESS	flashBaseAddress
#define FLASH_SECTOR_ADDRESS(sector) \
    (FLASH_BASE_ADDRESS + (sector) * FLASH_SECTOR_SIZE)

#define	FLASH_ERASE_TIMEOUT_COUNT	750
#define	FLASH_ERASE_TIMEOUT_TICKS	2

#define	FLASH_PROGRAM_TIMEOUT_POLLS	100000
#define	FLASH_PROGRAM_TIMEOUT_TICKS	0

typedef enum {
    AMD = 0x01,
    ALLIANCE = 0x52,
    INTEL = 0x89
} FLASH_VENDORS;

typedef enum {
    FLASH_2F128 = 0x18,
    FLASH_2F320 = 0x16,
    FLASH_2F040 = 0xA4,
    FLASH_2F080 = 0xD5,
    FLASH_2L081 = 0x38,
    FLASH_2L160 = 0x49,
    FLASH_2L017 = 0xC8
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
extern struct flash_drv_funcs_s flash28f320;
extern struct flash_drv_funcs_s flash29l160;

int             flashDrvLibInit(void);
int             flashGetSectorCount(void);
int             flashEraseBank(int firstSector, int nSectors);
int             flashBlkRead(int sectorNum, char *buff,
                unsigned int offset, unsigned int count);
int             flashBlkWrite(int sectorNum, char *buff,
                unsigned int offset, unsigned int count);
int             flashSyncFilesystem(void);
int             flashDiagnostic(void);

#endif /* FLASH_DRV_LIB_H */

