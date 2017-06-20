/*
 * $Id: i2cPCF8547Ioport.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef __PCF_8547_H
#define __PCF_8547_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The IO pad is connected to SMBus channel 2
 */
#define PCF8574_SMBUS_CHAN       1
#define PCF8574_CCR_ADDRESS      0x27

STATUS pcf8574_boardid_get(UINT8 *board_id);

#ifdef __cplusplus
}
#endif

#endif /* __PCF_8547_H */

