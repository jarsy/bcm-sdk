/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * $Id: flashDrvLib.h,v 1.4 Broadcom SDK $
 */
 
#ifndef	FLASH_DRV_LIB_H
#define	FLASH_DRV_LIB_H

#define	FLASH_BASE_ADDRESS_PLCC_BOOT    0xBFC00000
#define FLASH_BASE_ADDRESS_FLASH_BOOT   0xBFC00000
#define FLASH_BASE_ADDRESS_ALIAS        0xBC000000

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

/*
 * AMD Flash commands and magic offsets
 */

#define AMD_FLASH_MAGIC_ADDR16_1  0x555 /* 555 for 16-bit devices in 16-bit mode */
#define AMD_FLASH_MAGIC_ADDR16_2  0x2AA /* 2AA for 16-bit devices in 16-bit mode */
#define AMD_FLASH_MAGIC_ADDR8_1  0xAAA /* AAA for 16-bit devices in 8-bit mode */
#define AMD_FLASH_MAGIC_ADDR8_2  0x555 /* 555 for 16-bit devices in 8-bit mode */

#define AMD_FLASH_RESET  0xF0
#define AMD_FLASH_MAGIC_1	 0xAA
#define AMD_FLASH_MAGIC_2	 0x55
#define AMD_FLASH_AUTOSEL  0x90
#define AMD_FLASH_DEVCODE8  0x1
#define AMD_FLASH_DEVCODE16  0x1
#define AMD_FLASH_DEVCODE16B  0x2
#define AMD_FLASH_MANID  0x0
#define AMD_FLASH_PROGRAM	 0xA0
#define AMD_FLASH_UNLOCK_BYPASS  0x20
#define AMD_FLASH_ERASE_3	 0x80
#define AMD_FLASH_ERASE_4	 0xAA
#define AMD_FLASH_ERASE_5	 0x55
#define AMD_FLASH_ERASE_ALL_6  0x10
#define AMD_FLASH_ERASE_SEC_6  0x30
#define AMD_FLASH_WRITE_BUFFER  0x25
#define AMD_FLASH_WRITE_CONFIRM  0x29

typedef enum {
    AMD = 0x01,
    ALLIANCE = 0x52,
    INTEL = 0x89,
    MXIC = 0xc2
} FLASH_VENDORS;

typedef enum {
    FLASH_2F320 = 0x16,
    FLASH_2F040 = 0xA4,
    FLASH_2F080 = 0xD5,
    FLASH_2L081 = 0x38,
    FLASH_2L160 = 0x49,
    FLASH_2L017 = 0xC8,
    FLASH_2L640 = 0x7E,
    FLASH_2L320 = 0xF9,
    FLASH_MX2L640 = 0xcb,
    FLASH_29GL128 = 0x227E,
    FLASH_S29GL128P = 0x2101,
    FLASH_S29GL256P = 0x2201,
    FLASH_S29GL512P = 0x2301,
    FLASH_S29GL01GP = 0x2801
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
extern struct flash_drv_funcs_s flash29l640;
extern struct flash_drv_funcs_s flash29l320;
extern struct flash_drv_funcs_s flash29gl128;
extern struct flash_drv_funcs_s flashs29gl256p;
extern struct flash_drv_funcs_s flashsflash;

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

