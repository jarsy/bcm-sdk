/*
 * $Id: i2cPCF8547Ioport.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <vxWorks.h>            /* vxWorks generics */
#include "config.h"
#include "sysMotI2c.h"
#include "i2cPCF8547Ioport.h"

STATUS
pcf8574_boardid_get(UINT8 *board_id)
{
    
    return (i2cRead(PCF8574_SMBUS_CHAN,
                    PCF8574_CCR_ADDRESS,
                    I2C_DEVICE_TYPE_IOPORT_PCF8574,
                    0,
                    1,
                    board_id));
}

#ifdef INCLUDE_I2C_DEBUG
void pcf8574_boardid_show()
{
    UINT8 board_id;
    
    board_id = 0;

    if (pcf8574_boardid_get(&board_id) == 0) {
        printf("Board ID : %x\n", board_id);
    } else {
        printf("pcf8574_boardid_get() failed\n");
    }
}
#endif /* INCLUDE_I2C_DEBUG */
