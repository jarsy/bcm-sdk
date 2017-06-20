/*
 * $Id: bcmi2c.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:	bcmi2c.c
 * Purpose:	BCM I2C API
 *
 * Note : Not for RoboSwitch currently.
 */
#ifdef INCLUDE_I2C

#include <bcm/error.h>
#include <bcm/types.h>

int 
bcm_robo_i2c_open(int unit, char *devname, uint32 flags, int speed)
{
	return BCM_E_UNAVAIL;
}

int 
bcm_robo_i2c_write(int unit, int fd, uint32 addr,
		  uint8 *data, uint32 nbytes)
{
	return BCM_E_UNAVAIL;
}

int 
bcm_robo_i2c_read(int unit, int fd, uint32 addr,
		 uint8 *data, uint32 *nbytes)
{
	return BCM_E_UNAVAIL;
}

int 
bcm_robo_i2c_ioctl(int unit, int fd, int opcode, void *data, int len)
{
	return BCM_E_UNAVAIL;
}
#endif
