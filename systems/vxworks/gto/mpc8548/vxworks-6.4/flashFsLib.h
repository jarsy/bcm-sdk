/*
 * $Id: flashFsLib.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef	FLASH_FS_LIB_H
#define	FLASH_FS_LIB_H

#include "gto.h"

/*
 * Support for Flash Filesystem and Flash Boot Area
 * Built on top of flashDrvLib.
 */

STATUS flashFsLibInit(void);
STATUS flashFsSync(void);

extern int             flashBootSize;
extern int             flashFsSize;

extern int             flashFsSectorStart;
extern int             flash_boot_start;
extern int             flash_nvram_start;


#define FLASH_BOOT_START	   0x7f00000
#define FLASH_BOOT_START_SECTOR	   ((FLASH_BOOT_START) / (128 * 1024))
#define FLASH_BOOT_SIZE		   (1024 * 1024) 
#define FLASH_BOOT_SIZE_SECTORS	   8 

#define FLASH_NVRAM_SIZE	   0 
#define FLASH_NVRAM_START	   0x7f00000
#define FLASH_NVRAM_SECTOR_OFFSET  0x7f00000 
#define FLASH_NVRAM_START_SECTOR   ((FLASH_NVRAM_START) / (128 * 1024)) 
#define FLASH_NVRAM_SIZE_SECTORS   0 

#define FLASH_FS_BLOCK_SIZE	   512

#define FLASH_FS_SIZE		   (flashFsSize)
#define FLASH_FS_SIZE_BLOCKS	   ((FLASH_FS_SIZE) / (FLASH_FS_BLOCK_SIZE))
#define FLASH_FS_BLOCK_PER_SECTOR  ((128 * 1024) / (FLASH_FS_BLOCK_SIZE))

#define	FLASH_FS_NAME	"flash:"

typedef enum {

    FLASH_FS_SYNC = 0x10000

} FLASH_FS_CUSTOM_IOCTL_DEFINITIONS;

extern int sysIsFlashProm(void);
extern STATUS fsmNameInstall(char *driver, char *volume);
extern STATUS flashFsSync(void);
#endif /* FLASH_FS_LIB_H */
