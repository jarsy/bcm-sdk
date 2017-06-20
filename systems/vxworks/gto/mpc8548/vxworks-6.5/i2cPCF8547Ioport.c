/*
 * $Id: i2cPCF8547Ioport.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <vxWorks.h>            /* vxWorks generics */
#include "config.h"
#include "sysMotI2c.h"
#include "i2cPCF8547Ioport.h"
#include <stdio.h>

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

STATUS
pcf8574_readv(int slave_addr, UINT8 *value)
{
    
    return (i2cRead(PCF8574_SMBUS_CHAN,
                    slave_addr,
                    I2C_DEVICE_TYPE_IOPORT_PCF8574,
                    0,
                    1,
                    value));
}

STATUS
pcf8574_writev(int slave_addr, UINT8 *value)
{
    
    return (i2cWrite(PCF8574_SMBUS_CHAN,
                    slave_addr,
                    I2C_DEVICE_TYPE_IOPORT_PCF8574,
                    0,
                    1,
                    value));
}

STATUS
pcf8574_write_val(int slave_addr, int value)
{
    UINT8 c = value & 0xFF;

    return pcf8574_writev(slave_addr, &c);
}

UINT8
pcf8574_read_val(int slave_addr)
{
    UINT8 c = 0xFF;

    pcf8574_readv(slave_addr, &c);
    return c;
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
