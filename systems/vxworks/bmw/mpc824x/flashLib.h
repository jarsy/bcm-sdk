#ifndef	FLASH_LIB_H
#define	FLASH_LIB_H

extern int sysClkRateGet(void);

#define	FLASH_PROGRAM_POLLS		100000
#define	FLASH_ERASE_SECTOR_TIMEOUT	(10 /*SEC*/  *  sysClkRateGet())

typedef struct flash_dev_s {
    int		bank;			/* Bank 0 or 1 */
    UINT32	base;			/* Base address */
    int		sectors;		/* Sector count */
    int		lgSectorSize;		/* Log2(usable bytes/sector) */
    int		vendorID;		/* Expected vendor ID */
    int		deviceID;		/* Expected device ID */
    int		found;			/* Set if found by flashLibInit */
    int		swap;			/* Set for bank 1 if byte swap req'd */
} flash_dev_t;

#define FLASH_MAX_POS(dev) \
	((dev)->sectors << (dev)->lgSectorSize)

#define FLASH_SECTOR_POS(dev, sector) \
	((sector) << (dev)->lgSectorSize)

#define FLASH0_BANK			0
#define FLASH0_VENDOR_ID		0x01
#define FLASH0_DEVICE_ID		0x49
#define FLASH0_32MB_DEVICE_ID           0x7E

extern	flash_dev_t			flashDev[];
extern	int				flashDevCount;

/* $Id: flashLib.h,v 1.3 2011/07/21 16:14:08 yshtil Exp $
 * Device pointers
 *
 * These must be kept in sync with the table in flashLib.c.
 */
#define	FLASH_DEV_BANK0_ENTRY		0

#define FLASH_DEV_BANK0_SA0		(&flashDev[FLASH_DEV_BANK0_ENTRY+0])
#define FLASH_DEV_BANK0_SA1		(&flashDev[FLASH_DEV_BANK0_ENTRY+1])
#define FLASH_DEV_BANK0_SA2		(&flashDev[FLASH_DEV_BANK0_ENTRY+2])
#define FLASH_DEV_BANK0_SA3		(&flashDev[FLASH_DEV_BANK0_ENTRY+3])
#define FLASH_DEV_BANK0_LOW		(&flashDev[FLASH_DEV_BANK0_ENTRY+4])
#define FLASH_DEV_BANK0_BOOT		(&flashDev[FLASH_DEV_BANK0_ENTRY+5])
#define FLASH_DEV_BANK0_HIGH		(&flashDev[FLASH_DEV_BANK0_ENTRY+6])

STATUS flashLibInit(void);
STATUS flashEraseSector(flash_dev_t *dev, int sector);
STATUS flashErase(flash_dev_t *dev);
STATUS flashRead(flash_dev_t *dev, int pos, char *buf, int len);
STATUS flashWrite(flash_dev_t *dev, int pos, char *buf, int len);
STATUS flashWritable(flash_dev_t *dev, int pos, int len);
STATUS flashDiag(flash_dev_t *dev);
STATUS flashDiagAll(void);

#endif	/* !FLASH_LIB_H */
