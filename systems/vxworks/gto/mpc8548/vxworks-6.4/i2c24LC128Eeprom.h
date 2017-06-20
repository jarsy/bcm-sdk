/*
 * $Id: i2c24LC128Eeprom.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef	__INCh
#define	__INCeepromh

#ifdef __cplusplus
extern "C" {
#endif

/* includes */

#include <vxWorks.h>
#include <memLib.h>

/* EEPROM is connected to SMBus channel 2  */
#define EEPROM_24LC128_SMBUS_CHAN  1
#define EEPROM_24LC128_CCR_ADDRESS 0x50

/* defines */

#define NV_WR_CYCLE_TIME    1 /* EEPROM write cycle time (ms) */
#define RETRY_NVWRITE_COUNT 5 /* max wait iterations before write abort */

#undef NV_RAM_WR_ENBL       /* write enable procedure */
#define NV_RAM_WR_ENBL      eepromUnlock()

#undef NV_RAM_WR_DSBL       /* write disable procedure */
#define NV_RAM_WR_DSBL      eepromLock()

#undef  NV_RAM_WRITE        /* write procedure */
#define NV_RAM_WRITE        eepromWriteByte

#undef  NV_RAM_READ         /* read procedure */
#define NV_RAM_READ         eepromReadByte

/* function prototypes */

UINT8  eepromReadByte (int offset);
STATUS eepromWriteByte (int offset, UINT8 data);
void   eepromLock (void);
void   eepromUnlock (void);


#ifdef __cplusplus
}
#endif

#endif	/* __INCeepromh */

