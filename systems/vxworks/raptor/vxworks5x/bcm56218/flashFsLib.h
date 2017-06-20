#ifndef	FLASH_FS_LIB_H
#define	FLASH_FS_LIB_H

#include "mbz.h"

/* $Id: flashFsLib.h,v 1.3 2011/07/21 16:14:55 yshtil Exp $
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


#define FLASH_BOOT_START	   (flash_boot_start)
#define FLASH_BOOT_START_SECTOR	   ((FLASH_BOOT_START) / (FLASH_SECTOR_SIZE))
#define FLASH_BOOT_SIZE		   flashBootSize
#define FLASH_BOOT_SIZE_SECTORS	   \
    (((FLASH_BOOT_SIZE) + ((FLASH_SECTOR_SIZE) - 1))/ (FLASH_SECTOR_SIZE))

#define FLASH_NVRAM_SIZE	   (NV_RAM_SIZE)
#define FLASH_NVRAM_START	   (flash_nvram_start)
#define FLASH_NVRAM_SECTOR_OFFSET   \
            ((FLASH_NVRAM_START) & (FLASH_SECTOR_SIZE-1))
#define FLASH_NVRAM_START_SECTOR    \
            ((FLASH_NVRAM_START) / (FLASH_SECTOR_SIZE))
#define FLASH_NVRAM_SIZE_SECTORS   \
      (((FLASH_NVRAM_START) + ((FLASH_SECTOR_SIZE) - 1))/ (FLASH_SECTOR_SIZE))

#define FLASH_FS_BLOCK_SIZE	   512

#define FLASH_FS_SIZE		   (flashFsSize)
#define FLASH_FS_SIZE_BLOCKS	   ((FLASH_FS_SIZE) / (FLASH_FS_BLOCK_SIZE))
#define FLASH_FS_BLOCK_PER_SECTOR  ((FLASH_SECTOR_SIZE) / (FLASH_FS_BLOCK_SIZE))

#define	FLASH_FS_NAME	"flash:"

typedef enum {

    FLASH_FS_SYNC = 0x10000

} FLASH_FS_CUSTOM_IOCTL_DEFINITIONS;

#endif
