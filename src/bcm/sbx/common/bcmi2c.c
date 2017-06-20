/*
 * $Id: bcmi2c.c,v 1.5.270.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:	bcmi2c.c
 * Purpose:	BCM I2C API
 */

#ifdef INCLUDE_I2C

#include <soc/drv.h>
#include <soc/i2c.h>

#include <bcm/error.h>
#include <bcm/bcmi2c.h>
#include <soc/bsc.h>
#include <sal/compiler.h>

int
bcm_sbx_i2c_open(int unit,
                 char *devname,
                 uint32 flags,
                 int speed)
{
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SIRIUS(unit) || SOC_IS_CALADAN3(unit)) {
        return soc_i2c_devopen(unit, devname, flags, speed);
    }
#endif
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_i2c_write(int unit,
                  int fd,
                  uint32 addr,
                  uint8 *data,
                  uint32 nbytes)
{
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SIRIUS(unit) || SOC_IS_CALADAN3(unit)) {
        if (soc_i2c_device(unit, fd)->driver == NULL) {
            return BCM_E_PARAM;
        }
        
        return soc_i2c_device(unit, fd)->driver->write(unit, fd,
                                                       addr, data, nbytes);
    }
#endif
    return BCM_E_UNAVAIL;

}

int
bcm_sbx_i2c_read(int unit,
                 int fd,
                 uint32 addr,
                 uint8 *data,
                 uint32 * nbytes)
{
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SIRIUS(unit) || SOC_IS_CALADAN3(unit)) {
        if (soc_i2c_device(unit, fd)->driver == NULL) {
            return BCM_E_PARAM;
        }
        
        return soc_i2c_device(unit, fd)->driver->read(unit, fd,
                                                      addr, data, nbytes);
    }
#endif
    return BCM_E_UNAVAIL;

}

int
bcm_sbx_i2c_ioctl(int unit,
                  int fd,
                  int opcode,
                  void *data,
                  int len)
{
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SIRIUS(unit) || SOC_IS_CALADAN3(unit)) {

        if (soc_i2c_device(unit, fd)->driver == NULL) {
            return BCM_E_PARAM;
        }
        
        return soc_i2c_device(unit, fd)->driver->ioctl(unit, fd, opcode,
                                                       data, len);
    }
#endif
    return BCM_E_UNAVAIL;

}

#ifdef BCM_FE2000_SUPPORT

int
bcm_fe2000_i2c_open(int unit, char *devname, uint32 flags, int speed)
{
	COMPILER_REFERENCE(flags);
	COMPILER_REFERENCE(speed);

	return soc_bsc_devopen(unit, devname);
}

int
bcm_fe2000_i2c_write(int unit, int fd, uint32 addr, uint8 *data, uint32 nbytes)
{
	/* nbytes is not used.  One byte a time */
	COMPILER_REFERENCE(nbytes);

	if ((soc_bsc_device(unit, fd) == NULL) || (soc_bsc_device(unit, fd)->driver == NULL)) {
		return BCM_E_PARAM;
	}
	return(soc_bsc_device(unit, fd)->driver->write(unit, fd, addr, (uint32)*data));
}

int
bcm_fe2000_i2c_read(int unit, int fd, uint32 addr, uint8 *data, uint32 *nbytes)
{
	uint32 tmp;
	int retv;

	/* nbytes is not used.  One byte a time */
	COMPILER_REFERENCE(nbytes);

	if ((soc_bsc_device(unit, fd) == NULL) || (soc_bsc_device(unit, fd)->driver == NULL)) {
		return BCM_E_PARAM;
	}
	retv = soc_bsc_device(unit, fd)->driver->read(unit, fd, addr, &tmp);
	if (retv < 0) {
		return retv;
	}
	*data = tmp & 0xff;
	return SOC_E_NONE;
}

int
bcm_fe2000_i2c_ioctl(int unit, int fd, int opcode, void *data, int len)
{
	if ((soc_bsc_device(unit, fd) == NULL) || (soc_bsc_device(unit, fd)->driver == NULL)) {
		return BCM_E_PARAM;
	}

	return soc_bsc_device(unit, fd)->driver->ioctl(unit, fd, opcode, data, len);
}
#endif /* BCM_FE2000_SUPPORT */
#endif /* INCLUDE_I2C */
