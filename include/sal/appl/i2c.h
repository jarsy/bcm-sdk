/*
 * $Id: $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:	i2c.h
 * Purpose: Implementation of I2C bus commands
 */

#ifndef	_SAL_I2C_H
#define	_SAL_I2C_H

#include <sal/types.h>

/* 
 * unit     - I2C controller 
 * slave    - slave address on the I2C bus
 * addr     - internal address on the slave
 * addr_len - length of internal address
 * buf      - buffer to hold the read data
 * buf_len  - length of data to read
 */
extern int
sal_i2c_read(int unit, uint16 slave, uint32 addr, uint8 addr_len, uint8 *buf, 
             uint8 buf_len);

/* 
 * unit     - I2C controller 
 * slave    - slave address on the I2C bus
 * addr     - internal address on the slave
 * addr_len - length of internal address
 * buf      - buffer to hold the read data
 * buf_len  - length of data to write
 */
extern int
sal_i2c_write(int unit, uint16 slave, uint32 addr, uint8 addr_len, uint8 *buf, 
              uint8 buf_len);

#define SAL_I2C_FAST_ACCESS     (0x1)
/* 
 * unit     - I2C controller 
 * flags    - SAL_I2C_* flags 
 */
extern int
sal_i2c_config_set(int unit, uint32 flags);

/* 
 * unit     - I2C controller 
 * flags    - SAL_I2C_* flags 
 */
extern int
sal_i2c_config_get(int unit, uint32 *flags);
#endif /* _SAL_I2C_H */
