/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: cpudevs.c,v 1.9 Broadcom SDK $
 * 
 * CPU based I2C management
 *
 */

#include <shared/bsl.h>

#include <sal/types.h>
#include <soc/drv.h>
#include <soc/i2c.h>
#include <shared/bsl.h>
#ifdef BCM_CALADAN3_SVK

/* LM75 driver - lm75.c */
extern i2c_driver_t _soc_i2c_lm75_driver;
/* MAX127 Driver - max127.c */
extern i2c_driver_t _soc_i2c_max127_driver;
/* W229B Driver - w229b.c */
extern i2c_driver_t _soc_i2c_w229b_driver;
/* AT24C64 EEPROM driver - 24c64.c */
extern i2c_driver_t _soc_i2c_eep24c64_driver;
/* XFP driver - xfp.c */
extern i2c_driver_t _soc_i2c_xfp_driver;
/* PCIE config space register access driver - pcie.c */
extern i2c_driver_t _soc_i2c_pcie_driver;
/* Phillips PCF8574 Parallel Port - pcf8574.c */
/* Phillips PCF8574 Parallel Port - pcf8574.c */
extern i2c_driver_t _soc_i2c_pcf8574_driver;
/* LTC 1427 DAC */
extern i2c_driver_t _soc_i2c_ltc1427_driver ;
/* Matrix Orbital LCD Display */
extern i2c_driver_t _soc_i2c_lcd_driver;
/* Cypress 22393 clock chip driver */
extern i2c_driver_t _soc_i2c_cy2239x_driver;
/* Cypress 22150 clock chip driver */
extern i2c_driver_t _soc_i2c_cy22150_driver;
/* LTC4258 Powered Ethernet chip driver */
extern i2c_driver_t _soc_i2c_ltc4258_driver;
/* PD63000 Powered Ethernet chip driver */
extern i2c_driver_t _soc_i2c_pd63000_driver;
/* MAX5478 Digital Potentiometer chip driver */
extern i2c_driver_t _soc_i2c_max5478_driver;
/* BCM59101 Power Over Ethernet chip driver */
extern i2c_driver_t _soc_i2c_bcm59101_driver;
/* PCA9548 8-channel i2c mux chip driver */
extern i2c_driver_t _soc_i2c_pca9548_driver;
/* ADP4000 an integrated power control IC */
extern i2c_driver_t _soc_i2c_adp4000_driver;

/* PCA9505 40-bit i2c bus IO port */
extern i2c_driver_t _soc_i2c_pca9505_driver;

/* Summit SMM665C Active DC control */
extern i2c_driver_t _soc_i2c_smm665c_driver;

#define	I2CDEV(_name, _num, _drv) {	\
	I2C_##_name##_##_num,		\
	I2C_##_name##_SADDR##_num,	\
	&_soc_i2c_##_drv##_driver,	\
	}

STATIC i2c_device_t cpui2c_devices[] = {

    I2CDEV(IOP, 0, pca9505),
    I2CDEV(IOP, 1, pca9505),
    I2CDEV(IOP, 2, pca9505),
    I2CDEV(IOP, 3, pca9505),
    I2CDEV(IOP, 4, pca9505),
    I2CDEV(IOP, 5, pca9505),
    I2CDEV(IOP, 6, pca9505),
    I2CDEV(ADC, 0, max127),
    I2CDEV(ADOC, 0,smm665c)   

};

/*
    I2CDEV(MUX, 0, pca9548),
    I2CDEV(MUX, 1, pca9548),
    I2CDEV(MUX, 2, pca9548),
    I2CDEV(MUX, 3, pca9548),
    I2CDEV(MUX, 4, pca9548),
    I2CDEV(MUX, 5, pca9548),
    I2CDEV(MUX, 6, pca9548),

*/

#define NUM_I2C_DEVICES COUNTOF(cpui2c_devices)
#define KHZ_TO_HZ(h) (1000 * (h))

typedef struct _soc_i2c_probe_info_s {
    int			devid;
    int			devid_found;
    int			devices_found;
    soc_i2c_bus_t	*i2cbus;
    int                 i2c_nvram_skip;
    int                 i2c_hclk_skip;
    int                 i2c_poe_power;
    int                 i2c_muxed_devid_count[NUM_I2C_DEVICES];
    int                 i2c_mux_stack[NUM_I2C_DEVICES];
    int                 i2c_mux_stack_depth;
} _soc_i2c_probe_info_t;


#define MAX_I2C_BUS 2
#define CPUI2CBUS(b) &cpubus[(b)]

soc_i2c_bus_t cpubus[MAX_I2C_BUS];

static char *cpu_i2c_saddr_to_string_table[0x80] = {
    /* 0X00 */ "Unknown",
    /* 0X01 */ "Unknown",
    /* 0X02 */ "Unknown",
    /* 0X03 */ "Unknown",
    /* 0X04 */ "Unknown",
    /* 0X05 */ "Unknown",
    /* 0X06 */ "Unknown",
    /* 0X07 */ "Unknown",
    /* 0X08 */ "Unknown",
    /* 0X09 */ "Unknown",
    /* 0X0A */ "Unknown",
    /* 0X0B */ "Unknown",
    /* 0X0C */ "Unknown",
    /* 0X0D */ "Unknown",
    /* 0X0E */ "Unknown",
    /* 0X0F */ "Unknown",
    /* 0X10 */ "Unknown",
    /* 0X11 */ "Unknown",
    /* 0X12 */ "Unknown",
    /* 0X13 */ "Unknown",
    /* 0X14 */ "Unknown",
    /* 0X15 */ "Unknown",
    /* 0X16 */ "Unknown",
    /* 0X17 */ "Unknown",
    /* 0X18 */ "Unknown",
    /* 0X19 */ "Unknown",
    /* 0X1A */ "Unknown",
    /* 0X1B */ "Unknown",
    /* 0X1C */ "Unknown",
    /* 0X1D */ "Unknown",
    /* 0X1E */ "Unknown",
    /* 0X1F */ "Unknown",
    /* 0X20 */ "iop0: PCA9505 40bit IO port",
    /* 0X21 */ "iop1: PCA9505 40bit IO port",
    /* 0X22 */ "iop2: PCA9505 40bit IO port",
    /* 0X23 */ "iop3: PCA9505 40bit IO port",
    /* 0X24 */ "iop4: PCA9505 40bit IO port",
    /* 0X25 */ "iop5: PCA9505 40bit IO port",
    /* 0X26 */ "iop6: PCA9505 40bit IO port",
    /* 0X27 */ "lpt1: PCF8574 XMC Board Id",
    /* 0X28 */ "adc0: MAX127 DAS",
    /* 0X29 */ "Unknown",
    /* 0X2A */ "Unknown",
    /* 0X2B */ "Unknown",
    /* 0X2C */ "Unknown",
    /* 0X2D */ "Unknown",
    /* 0X2E */ "Unknown",
    /* 0X2F */ "Unknown",
    /* 0X30 */ "Unknown",
    /* 0X31 */ "Unknown",
    /* 0X32 */ "Unknown",
    /* 0X33 */ "Unknown",
    /* 0X34 */ "Unknown",
    /* 0X35 */ "Unknown",
    /* 0X36 */ "Unknown",
    /* 0X37 */ "Unknown",
    /* 0X38 */ "Unknown",
    /* 0X39 */ "Unknown",
    /* 0X3A */ "Unknown",
    /* 0X3B */ "Unknown",
    /* 0X3C */ "Unknown",
    /* 0X3D */ "Unknown",
    /* 0X3E */ "Unknown",
    /* 0X3F */ "Unknown",
    /* 0X40 */ "Unknown",
    /* 0X41 */ "Unknown",
    /* 0X42 */ "Unknown",
    /* 0X43 */ "Unknown",
    /* 0X44 */ "Unknown",
    /* 0X45 */ "Unknown",
    /* 0X46 */ "Unknown",
    /* 0X47 */ "Unknown",
    /* 0X48 */ "Unknown",
    /* 0X49 */ "Unknown",
    /* 0X4A */ "Unknown",
    /* 0X4B */ "Unknown",
    /* 0X4C */ "Unknown",
    /* 0X4D */ "Unknown",
    /* 0X4E */ "adoc0: SMM665C Active DC control",
    /* 0X4F */ "Unknown",
    /* 0X50 */ "nvram0: XMC 24LC128 eeprom",
    /* 0X51 */ "Unknown",
    /* 0X52 */ "Unknown",
    /* 0X53 */ "Unknown",
    /* 0X54 */ "XMC DDR",
    /* 0X55 */ "Unknown",
    /* 0X56 */ "Unknown",
    /* 0X57 */ "Unknown",
    /* 0X58 */ "QSFP eeprom?",
    /* 0X59 */ "Unknown",
    /* 0X5A */ "Unknown",
    /* 0X5B */ "Unknown",
    /* 0X5C */ "adoc0: SMM665C Active DC control int mem",
    /* 0X5D */ "adoc0: SMM665C Active DC control int mem",
    /* 0X5E */ "Unknown",
    /* 0X5F */ "adoc0: SMM665C Active DC control config",
    /* 0X60 */ "Unknown",
    /* 0X61 */ "Unknown",
    /* 0X62 */ "Unknown",
    /* 0X63 */ "Unknown",
    /* 0X64 */ "Unknown",
    /* 0X65 */ "ADP4000-0",
    /* 0X66 */ "Unknown",
    /* 0X67 */ "Unknown",
    /* 0X68 */ "Unknown",
    /* 0X69 */ "pll0: Cypress W229B/W311 Clock Chip",
    /* 0X6A */ "clk0: Cypress W2239x clock Chip",
    /* 0X6B */ "clk0: Cypress W22150 clock Chip",
    /* 0X6C */ "Unknown",
    /* 0X6D */ "Unknown",
    /* 0X6E */ "Unknown",
    /* 0X6F */ "Unknown",
    /* 0X70 */ "Unknown", 
    /* 0X71 */ "mux0: PCA9548 i2c Switch",
    /* 0X72 */ "mux1: PCA9548 i2c Switch",
    /* 0X73 */ "mux2: PCA9548 i2c Switch",
    /* 0X74 */ "mux3: PCA9548 i2c Switch",
    /* 0X75 */ "mux4: PCA9548 i2c Switch",
    /* 0X76 */ "mux5: PCA9548 i2c Switch",
    /* 0X77 */ "mux6: PCA9548 i2c Switch",
    /* 0X78 */ "Unknown",
    /* 0X79 */ "Unknown",
    /* 0X7A */ "Unknown",
    /* 0X7B */ "Unknown",
    /* 0X7C */ "Unknown",
    /* 0X7D */ "Unknown",
    /* 0X7E */ "Unknown",
    /* 0X7F */ "Unknown"
};

int cpu_i2c_defaultbus() {
    return CPU_I2C_DEV_NUM_DEFAULT;
}

char *cpu_i2c_saddr_to_string(int bus, i2c_saddr_t saddr)
{
    /*expand to handle bus */
    return (cpu_i2c_saddr_to_string_table[saddr]);
}



/*
 * Function: cpu_i2c_device_count
 *
 * Purpose: Report the number of devices registered
 *          in the system. For now, this is the total number of devices
 *          we have added to the statically defined device descriptor
 *          array above.
 *
 * Parameters:
 *    bus - I2C bus number
 *
 * Returns:
 *        number of devices registered in the system device table.
 *
 * Notes: Currently, we do not support dynamic device loading.
 *        Later, one will be able add a driver to the device table,
 *        without the STATIC attribute.
 */
int
cpu_i2c_device_count(int bus)
{
    COMPILER_REFERENCE(bus);
    return NUM_I2C_DEVICES;
}

/*
 * Function: cpu_i2c_show
 *
 * Parameters:
 *    bus - I2C bus number
 *
 * Purpose: Show all valid devices and their attributes and
 *          statistics.
 * Returns:
 *    none
 * Notes:
 *    none
 */
void
cpu_i2c_show(int bus)
{
    int dev;
    soc_i2c_bus_t	*i2cbus;
    i2c_device_t	*i2cdev;

    /* Make sure that we're already attached, or go get attached */
    if ( !cpu_i2c_is_attached(bus) &&
	 (cpu_i2c_attach(bus, 0, 0) < 0) ) {
        LOG_CLI((BSL_META("cpu bus %d soc_i2c_show: error attaching to I2C bus\n"), 
                 bus));
        return;
    }

    i2cbus = CPUI2CBUS(bus);

    i2cbus->txBytes = i2cbus->rxBytes = 0;

    LOG_CLI((BSL_META("CPU bus %d i2c  bus: mode=%s speed=%dkbps "
                      "SOC_address=0x%02X\nDevices: \n"),
             bus,
             i2cbus->flags&SOC_I2C_MODE_PIO?"PIO":"INTR",
             i2cbus->frequency/1000,
             i2cbus->master_addr));

    for (dev = 0; dev < NUM_I2C_DEVICES; dev++) {
	i2cdev = i2cbus->devs[dev];
	if (i2cdev == NULL) {
	    continue;
	}
	LOG_CLI((BSL_META("%15s: %s%s saddr=0x%02x \n"),
                 i2cdev->devname,
                 i2cdev->driver ? "" : " (detached)",
                 i2cdev->desc,
                 i2cdev->saddr));

	if (i2cdev->driver) {
	    LOG_CLI((BSL_META("                 received %d bytes, "
                              "transmitted %d bytes\n"),
                     i2cdev->rbyte, i2cdev->tbyte));
	    i2cbus->txBytes += i2cdev->tbyte;
	    i2cbus->rxBytes += i2cdev->rbyte;
	}
    }
    LOG_CLI((BSL_META("CPU bus %d i2c  bus: received %d bytes, "
                      "transmitted %d bytes\n"),
             bus, i2cbus->rxBytes, i2cbus->txBytes));
}

/*
 * Function: cpu_i2c_addr
 *
 * Purpose: Return slave address of specified device.
 *
 * Parameters:
 *    bus - I2C bus number
 *    devid - I2C device id returned from cpu_i2c_dev_open
 *
 * Returns:
 *    7-bit or 10-bit I2C slave address of device, in "datasheet-speak"
 *
 * Notes:
 *    none
 */
i2c_saddr_t
cpu_i2c_addr(int bus, int devid)
{
    soc_i2c_bus_t	*i2cbus;

    i2cbus = CPUI2CBUS(bus);
    if (i2cbus && i2cbus->devs[devid]) {
	return i2cbus->devs[devid]->saddr;
    }
    return 0;
}

/*
 * cpu_i2c_access
 * 
 * Purpose:
 *     This routine searches the list of devices to find a device is
 *     accessible on the CPU bus rather than the BCM Switch
 */
int 
cpu_i2c_access(int devid) {

    int bus;
    bus = cpu_i2c_defaultbus();
    return (cpu_i2c_addr(bus, devid) > 0);
}


/*
 * Function: cpu_i2c_devname
 *
 * Purpose: Return device name of specified device.
 *
 * Parameters:
 *    bus - I2C bus number
 *    devid - I2C device id returned from cpu_i2c_dev_open
 *
 * Returns:
 *    character string name of device
 *
 * Notes:
 *    none
 */
const char *
cpu_i2c_devname(int bus, int devid)
{
    soc_i2c_bus_t	*i2cbus;

    i2cbus = CPUI2CBUS(bus);
    if (i2cbus && i2cbus->devs[devid]) {
	return i2cbus->devs[devid]->devname;
    }
    return NULL;
}

int
cpu_i2c_devdesc_set(int bus, int devid, char *desc)
{
    soc_i2c_bus_t	*i2cbus;

    i2cbus = CPUI2CBUS(bus);
    if (i2cbus && i2cbus->devs[devid]) {
	i2cbus->devs[devid]->desc = desc;
    }
    return SOC_E_NONE;
}

int
cpu_i2c_devdesc_get(int bus, int devid, char **desc)
{
    soc_i2c_bus_t	*i2cbus;

    i2cbus = CPUI2CBUS(bus);
    if (i2cbus && i2cbus->devs[devid]) {
	*desc = i2cbus->devs[devid]->desc;
    } else {
	*desc = "";
    }
    return SOC_E_NONE;
}

/*
 * Function: cpu_i2c_device
 *
 * Purpose: Returns the device structure associated with the bus and
 *          device identifier.
 *
 * Parameters:
 *    bus - I2C bus number
 *    devid - I2C device id returned from cpu_i2c_dev_open
 *
 * Returns:
 *    I2C device descriptor
 *
 * Notes:
 *    none
 */
i2c_device_t *
cpu_i2c_device(int bus, int devid)
{
    soc_i2c_bus_t	*i2cbus;

    i2cbus = CPUI2CBUS(bus);
    if (i2cbus && i2cbus->devs[devid]) {
	return i2cbus->devs[devid];
    }
    return NULL;
}

/*
 * Function: cpu_i2c_devtype
 *
 * Purpose: Return the device driver type, this is an integer value
 *          associated with the driver to determine ownership of the device.
 *
 * Parameters:
 *    bus - I2C bus number
 *    devid - I2C device id returned from cpu_i2c_dev_open
 *
 * Returns:
 *    I2C device type code (from driver)
 *
 * Notes:
 *    Every device should have a unique Device type indentifier.
 *
 */
int
cpu_i2c_devtype(int bus, int devid)
{
    soc_i2c_bus_t	*i2cbus;

    i2cbus = CPUI2CBUS(bus);
    if (i2cbus && i2cbus->devs[devid] && i2cbus->devs[devid]->driver) {
	return i2cbus->devs[devid]->driver->id;
    }
    return 0;
}

/*
 *
 * Function: cpu_i2c_is_attached
 *
 * Purpose:  Return true if I2C bus driver is attached.
 *
 * Parameters:
 *    bus - I2C bus number
 *
 * Returns:
 *    True if the bus driver for I2C is attached.
 *
 * Notes:
 *    None
 */
int
cpu_i2c_is_attached(int bus)
{
    
    soc_i2c_bus_t	*i2cbus;

    i2cbus = CPUI2CBUS(bus);
    if (i2cbus && (i2cbus->flags & SOC_I2C_ATTACHED)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/*
 * Function: cpu_i2c_dev_open
 *
 * Purpose:  Open device, return valid file descriptor or -1 on error.
 *
 * Parameters:
 *    bus - I2C bus number
 *    devname - I2C device name string
 *    flags - arguments to pass to attach, default value should be zero
 *    speed - I2C bus speed, if non-zero, this speed is configured, normally
 *            this argument should be zero unless a speed is desired.
 * Returns:
 *      device identifier for all I2C operations
 *
 * Notes:
 *      This routine should be called before attempting to communicate
 *      with an I2C device which has a registered driver.
 */
int
cpu_i2c_dev_open(int bus, char* devname, uint32 flags, int speed)
{
    int devid, rv;
    soc_i2c_bus_t	*i2cbus;

    /* Make sure that we're already attached, or go get attached */
    if ( !cpu_i2c_is_attached(bus) &&
	 ((rv = cpu_i2c_attach(bus, flags, speed)) < 0) ) {
        return rv;
    }

    i2cbus = CPUI2CBUS(bus);
    for (devid = 0; devid < NUM_I2C_DEVICES; devid++) {
	if (i2cbus->devs[devid]) {
	    if (sal_strcmp(i2cbus->devs[devid]->devname, devname) == 0) {
		return devid;
	    }
	}
    }
    return SOC_E_NOT_FOUND;
}

/*
 * Function: cpu_i2c_dev_write
 *
 * Parameters:
 *    bus - I2C bus number
 *
 * Purpose: Wrapper for device WRITE interface
 * Returns:
 *    none
 * Notes:
 *    none
 */
int
cpu_i2c_dev_write(int bus, int devno, uint16 addr, uint8 *data, uint32 len)
{
    int rv;
    i2c_device_t *devptr = NULL;

    devptr = soc_i2c_device(bus, devno);
    if ((devptr == NULL) || (devptr->driver == NULL) ||
        (devptr->driver->write == NULL)) {
        return SOC_E_PARAM;
    }
    rv = devptr->driver->write(FORCE_CPU_I2C_ACCESS, devno, addr, data, len);
    if (SOC_FAILURE(rv)) {
        LOG_CLI((BSL_META("cpu_i2c_dev_write failed for %d:%d"), bus, devno));
    }

    return rv;;
}



/*
 * Function: cpu_i2c_dev_read
 *
 * Parameters:
 *    bus - I2C bus number
 *
 * Purpose: Wrapper for device READ interface
 * Returns:
 *    none
 * Notes:
 *    none
 */
int
cpu_i2c_dev_read(int bus, int devno, uint16 addr, uint8 *data, uint32 *len)
{
    int rv;
    i2c_device_t *devptr = NULL;

    devptr = soc_i2c_device(bus, devno);
    if ((devptr == NULL) || (devptr->driver == NULL) ||
        (devptr->driver->read == NULL)) {
        return SOC_E_PARAM;
    }
    rv = devptr->driver->read(FORCE_CPU_I2C_ACCESS, devno, addr, data, len);
    if (SOC_FAILURE(rv)) {
        LOG_CLI((BSL_META("cpu_i2c_dev_read failed for %d:%d"), bus, devno));
    }

    return rv;;
}


/*
 * Function: cpu_i2c_dev_ioctl
 *
 * Parameters:
 *    bus - I2C bus number
 *
 * Purpose: Wrapper for device IOCTL interface
 * Returns:
 *    none
 * Notes:
 *    none
 */
int
cpu_i2c_dev_ioctl(int bus, int devno, int cmd, void *data, int len)
{
    int rv;
    i2c_device_t *devptr = NULL;

    devptr = soc_i2c_device(bus, devno);
    if ((devptr == NULL) || (devptr->driver == NULL) ||
        (devptr->driver->ioctl == NULL)) {
        return SOC_E_PARAM;
    }
    rv = devptr->driver->ioctl(FORCE_CPU_I2C_ACCESS, devno, cmd, data, len);
    if (SOC_FAILURE(rv)) {
        LOG_CLI((BSL_META("cpu_i2c_dev_ioctl failed for %d:%d"), bus, devno));
    }

    return rv;;
}


/*
 * Function: cpu_i2c_get_devices
 *
 * Purpose: Return an array of I2C devices; fill in the count.
 *
 * Parameters:
 *    bus - I2C bus number
 *    count - address where the count of devices should be stored.
 *
 * Returns:
 *      device descriptor array
 *
 * Notes:
 *      NYI
 */
i2c_device_t *
cpu_i2c_get_devices(int bus, int* count)
{
    return NULL;
}

/*
 * Function: cpu_i2c_register
 * Purpose : Register a driver with the systems internal table.
 *
 * Parameters:
 *    bus - I2C bus number
 *    devname - device name
 *    device - I2C device descriptor
 *
 * Returns:
 *      SOC_E_NONE - device is registered
 *
 * Notes:
 *      NYI
 */
int
cpu_i2c_register(int bus, char* devname, i2c_device_t* device)
{
    return SOC_E_UNAVAIL;
}


int
cpu_i2c_attach(int sys_bus, uint32 flags, int speed_khz)
{
    soc_i2c_bus_t	*i2cbus;
    int bus = cpu_i2c_defaultbus();
    i2cbus = CPUI2CBUS(bus);

    if (bsl_check(bslLayerSoc, bslSourceI2c, bslSeverityNormal, BSL_UNIT_UNKNOWN)) {
	soc_i2c_decode_flags(FORCE_CPU_I2C_ACCESS, "current flags", i2cbus->flags);
	soc_i2c_decode_flags(FORCE_CPU_I2C_ACCESS, "new flags", flags);
    }


    /* If not yet done, create synchronization semaphores/mutex */
    if (i2cbus->i2cMutex == NULL) {
        i2cbus->i2cMutex = sal_mutex_create("I2C Mutex");
	if (i2cbus->i2cMutex == NULL) {
	    return SOC_E_MEMORY;
	}
    }

    if (i2cbus->i2cIntr == NULL) {
        i2cbus->i2cIntr = sal_sem_create("I2C interrupt", sal_sem_BINARY, 0);
	if (i2cbus->i2cIntr == NULL) {
	    return SOC_E_MEMORY;
	}
    }

    /* Set semaphore timeout values */
    if (SAL_BOOT_QUICKTURN) {
	i2cbus->i2cTimeout = I2C_TIMEOUT_QT;
    } else if (SAL_BOOT_PLISIM) {
	i2cbus->i2cTimeout = I2C_TIMEOUT_PLI;
    } else {
	i2cbus->i2cTimeout = I2C_TIMEOUT;
    }

    i2cbus->flags = SOC_I2C_MODE_PIO;

    /* Number of PIO's (IFLG/ACK) */
    i2cbus->pio_retries = 1000000;
    LOG_INFO(BSL_LS_SOC_I2C,
             (BSL_META("soc_i2c_attach: oldspeed=%d newspeed=%d\n"),
              i2cbus->frequency, KHZ_TO_HZ(speed_khz)));

    /* cpu_set_freq(speed_khz); */

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META("CPU Bus %d i2c 0x%03x bus: mode %s, speed %dKbps\n"),
                 bus , i2cbus->master_addr,
                 (i2cbus->flags & SOC_I2C_MODE_PIO) ? "PIO" : "INTR",
                 (i2cbus->frequency+500) / 1000 ));


    i2cbus->flags |= SOC_I2C_ATTACHED;

    /*
     * Probe for I2C devices, update device list for detected devices ...
     */
    if (flags & SOC_I2C_NO_PROBE) {
        return SOC_E_NONE;
    } else {
        return cpu_i2c_probe(bus);
    }

}

/*
 * Function: _cpu_i2c_probe_device
 * Purpose: Internal probing of a particular I2C device.
 *
 * Parameters:
 *	muxed - I2C device search beyond one or more MUX's
 *      i2c_probe_info - data structure of I2C probe parameters
 *
 * Return:
 *	SOC_E_***
 */
int
_cpu_i2c_probe_device(int bus, int muxed,
                      _soc_i2c_probe_info_t *i2c_probe_info)
{
    int			devid = i2c_probe_info->devid;
    int                 rv = SOC_E_NOT_FOUND;
    int                 mux_stack_index;
    i2c_dev_init_func_t	load;
    char		*devdesc;

    for (mux_stack_index = 0;
         mux_stack_index < i2c_probe_info->i2c_mux_stack_depth;
         mux_stack_index++) {
        if (i2c_probe_info->i2c_mux_stack[mux_stack_index] == devid) {
            /* This is a previous MUX in our tree, skip */
            return rv;
        }
    }

    if (muxed &&
        (NULL != i2c_probe_info->i2cbus->devs[devid]) &&
        (0 == i2c_probe_info->i2c_muxed_devid_count[devid])) {
        /* Top level device, don't probe again during MUX scan. */
        return rv;
    }


    if (SOC_E_NONE ==
        cpu_i2c_device_present(bus, cpui2c_devices[devid].saddr)) {
	  
        /* An ACK was received, there's a device at this SADDR.      */
        /* We infer the device type solely by its I2C slave address. */

        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META("Cpu bus %d i2c 0x%x: found %s: %s\n"),
                  bus,
                  cpui2c_devices[devid].saddr,
                  cpui2c_devices[devid].devname,
                  cpui2c_devices[devid].desc ?
                  cpui2c_devices[devid].desc : ""));
	    
        /* Load the device driver */
        i2c_probe_info->i2cbus->devs[devid] = &cpui2c_devices[devid];

        /* Else, if the device is beyond one or more MUX's, then we
         * need to check it is present and initialize each time
         * it is found.
         */
	    
        /* set flags, device id, hook up bus, etc  */
        if (cpui2c_devices[devid].driver) {
            cpui2c_devices[devid].driver->flags |= I2C_DEV_OK;
            cpui2c_devices[devid].driver->flags |= I2C_REG_STATIC;
            cpui2c_devices[devid].driver->devno = devid;

            /* Load device if init function exists */
            load = cpui2c_devices[devid].driver->load;
            if (load) {
                if ((rv = load(FORCE_CPU_I2C_ACCESS,
                           devid,
                           cpui2c_devices[devid].testdata,
                           cpui2c_devices[devid].testlen)) < 0) {
                    LOG_INFO(BSL_LS_SOC_I2C,
                             (BSL_META("cpu bus %d i2c 0x%x: init failed %s - %s\n"),
                              bus,
                              cpui2c_devices[devid].saddr,
                              cpui2c_devices[devid].devname,
                              soc_errmsg(rv)));
                } else {
                    LOG_INFO(BSL_LS_SOC_I2C,
                             (BSL_META("cpu bus %d i2c: "
                                       "Loaded driver for 0x%02x - %s\n"),
                              bus, devid, cpui2c_devices[devid].devname));
                }    
            }
        }
        cpu_i2c_devdesc_get(bus, devid, &devdesc);
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META("CPU bus %d i2c 0x%x %s: %s\n"),
                     bus,
                     cpui2c_devices[devid].saddr,
                     cpui2c_devices[devid].devname,
                     devdesc));

        /* Only count a device type once, but each device instance */
        i2c_probe_info->devices_found += 1;
        if (!muxed){
            i2c_probe_info->devid_found += 1;
        } else {
            if (0 == i2c_probe_info->i2c_muxed_devid_count[devid]) {
                i2c_probe_info->devid_found += 1;
            }
            i2c_probe_info->i2c_muxed_devid_count[devid] += 1;
        }
        rv = SOC_E_EXISTS;
    } else if (!muxed) {  /* Don't clear info when scanning beyond MUX's */
        /* I2C device not present */
        i2c_probe_info->i2cbus->devs[devid] = NULL;
    }
    return rv;
}
/*
 * Function: _cpu_i2c_probe_mux
 * Purpose: Internal probing of a given I2C MUX.
 *
 * Parameters:
 *	bus - I2C bus number
 *      mux_devid_range - I2C device ID for MUX to scan, -1 for all
 *      i2c_probe_info - data structre of I2C probe parameters
 *
 * Return:
 *	SOC_E_***
 */
int
_cpu_i2c_probe_mux(int bus, int mux_devid_range,
                   _soc_i2c_probe_info_t *i2c_probe_info)
{
    int			devid, mux_devid;
    int			mux_channel;
    int                 rv = SOC_E_NONE;
    uint8               mux_data;
    int                 mux_devid_min, mux_devid_max;

    if (mux_devid_range < 0) {
        mux_devid_min = 0;
        mux_devid_max = NUM_I2C_DEVICES - 1;
    } else {
        mux_devid_min = mux_devid_range;
        mux_devid_max = mux_devid_range;
    }

    /* Now scan down the MUX tree */
    for (mux_devid = mux_devid_min;
         mux_devid <= mux_devid_max;
         mux_devid++) {
        if (cpui2c_devices[mux_devid].driver != &_soc_i2c_pca9548_driver) {
            /* Only the MUX's are interesting here. */
            continue;
        }
        if (NULL == i2c_probe_info->i2cbus->devs[mux_devid]) {
            /* We didn't find a MUX of this type */
            continue;
        }

        if ((0 == i2c_probe_info->i2c_mux_stack_depth) && /* Top level */
            (0 != i2c_probe_info->i2c_muxed_devid_count[mux_devid])) {
            /* We found this MUX type at a lower level of the tree */
            continue;
        }
        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META("CPU bus %d i2c: Detected MUX 0x%02x - %s\n"),
                  bus, mux_devid,
                  cpui2c_devices[mux_devid].devname));
        for (mux_channel = 0; mux_channel < PCA9548_CHANNEL_NUM;
             mux_channel++) {
            /* Set the channel so we can probe beyond it */
            mux_data = (1 << mux_channel);
            if ((rv = cpui2c_devices[mux_devid].driver->write(FORCE_CPU_I2C_ACCESS, mux_devid,
                                         0, &mux_data, 1)) < 0) {
                LOG_INFO(BSL_LS_SOC_I2C,
                         (BSL_META("CPU bus %d i2c: "
                                   "Could not assign channel %d to %s\n"),
                          bus, mux_channel,
                          cpui2c_devices[mux_devid].devname));
                return rv;
            }
            LOG_INFO(BSL_LS_SOC_I2C,
                     (BSL_META("CPU bus %d i2c: Set channel %d of MUX 0x%02x - %s\n"),
                      bus, mux_channel, mux_devid,
                      cpui2c_devices[mux_devid].devname));
            for (devid = 0; devid < NUM_I2C_DEVICES; devid++) {
                i2c_probe_info->devid = devid;
                rv = _cpu_i2c_probe_device(bus, TRUE, i2c_probe_info);
                if (SOC_E_EXISTS == rv) {
                    /* Is it a MUX? Recurse! */
                    if (cpui2c_devices[devid].driver ==
                        &_soc_i2c_pca9548_driver) {
                        /* Record MUX stack to get here */
                        i2c_probe_info->i2c_mux_stack[i2c_probe_info->i2c_mux_stack_depth] = devid;
                        i2c_probe_info->i2c_mux_stack_depth++;
                        rv = _cpu_i2c_probe_mux(bus, devid,
                                                i2c_probe_info);
                        if (SOC_FAILURE(rv)) {
                            /* Access error */
                            return rv;
                        }
                    }
                    rv = SOC_E_NONE;
                } else if ((SOC_E_NOT_FOUND == rv) || (SOC_E_INIT == rv)) {
                    /* No problem */
                    rv = SOC_E_NONE;
                } else if (SOC_FAILURE(rv)) {
                    /* Access error */
                    return rv;
                }
            }
        }

        /* Restore default channel 0 on the MUX now that
         * the probe is complete */
        mux_channel = 0;
        mux_data = (1 << mux_channel);
        if ((rv = cpui2c_devices[mux_devid].driver->write(FORCE_CPU_I2C_ACCESS, mux_devid,
                                                       0, &mux_data, 1)) < 0) {
            LOG_INFO(BSL_LS_SOC_I2C,
                     (BSL_META("CPU bus %d i2c: Could not assign channel %d to %s\n"),
                      bus, mux_channel,
                      cpui2c_devices[mux_devid].devname));
            return rv;
        }
    }

    i2c_probe_info->i2c_mux_stack_depth--;
    i2c_probe_info->i2c_mux_stack[i2c_probe_info->i2c_mux_stack_depth] = -1;
    return rv;
}

/*
 * Function: cpu_i2c_probe
 * Purpose: Probe I2C devices on bus, report devices found.
 *          This routine will walk through our internal I2C device driver
 *          tables, attempt to find the device on the I2C bus, and if
 *          successful, register a device driver for that device.
 *
 *          This allows for the device to be used in an API context as
 *          when the devices are found, the device driver table is filled
 *          with the correct entries for that device (r/w function, etc).
 *
 * Parameters:
 *	bus - I2C bus number
 *
 * Return:
 *	count of found devices or SOC_E_***
 */
int
cpu_i2c_probe(int bus)
{
    _soc_i2c_probe_info_t i2c_probe_info;
    int			devid;
    int                 rv;
    int			mux_devid;
    uint8               mux_data;

    /* Make sure that we're already attached, or go get attached */
    if (!cpu_i2c_is_attached(bus)) {
	return cpu_i2c_attach(bus, 0, 0);
    }

    sal_memset(&i2c_probe_info, 0, sizeof(i2c_probe_info));
    i2c_probe_info.i2cbus = CPUI2CBUS(bus);

    LOG_INFO(BSL_LS_SOC_I2C,
             (BSL_META("cpu bus %d i2c: probing %d I2C devices.\n"),
              bus, NUM_I2C_DEVICES));

    i2c_probe_info.devid_found = 0; /* Installed I2C device types */
    i2c_probe_info.devices_found = 0; /* Number of installed I2C devices */

    /* Initialize device table with none found. */
    for (devid = 0; devid < NUM_I2C_DEVICES; devid++) {
        /* I2C device not present */
        i2c_probe_info.i2cbus->devs[devid] = NULL;
        i2c_probe_info.i2c_muxed_devid_count[devid] = 0;
        i2c_probe_info.i2c_mux_stack[devid] = -1;
    }
    i2c_probe_info.i2c_mux_stack_depth = 0;

    /* First, disable all visible MUX's. */
    for (mux_devid = 0; mux_devid < NUM_I2C_DEVICES; mux_devid++) {
        if (cpui2c_devices[mux_devid].driver != &_soc_i2c_pca9548_driver) {
            /* Only the MUX's are interesting here. */
            continue;
        }
        i2c_probe_info.devid = mux_devid;
        rv = _cpu_i2c_probe_device(bus, FALSE, &i2c_probe_info);
        if (SOC_E_EXISTS == rv) {
            mux_data = 0; /* Shut down ALL channels */
            if ((rv = cpui2c_devices[mux_devid].driver->write(FORCE_CPU_I2C_ACCESS, mux_devid,
                                             0, &mux_data, 1)) < 0) {
                LOG_INFO(BSL_LS_SOC_I2C,
                         (BSL_META("CPU bus %d i2c: Could not disable channels on %s\n"),
                          bus, cpui2c_devices[mux_devid].devname));
                return rv;
            } else {
                /* MUX disabled */
                rv = SOC_E_NONE;
            }
        } else if ((SOC_E_NOT_FOUND == rv) || (SOC_E_INIT == rv)) {
            /* No error */
            rv = SOC_E_NONE;
        } else if (SOC_FAILURE(rv)) {
            /* Access error */
            return rv;
        }
    }

    /* Reset after MUX ops */
    i2c_probe_info.devid_found = 0;
    i2c_probe_info.devices_found = 0;

    /* Probe the currently configured (i.e. MUX'd) I2C bus. */
    for (devid = 0; devid < NUM_I2C_DEVICES; devid++) {
        /* Detect device type */
        i2c_probe_info.devid = devid;
        rv = _cpu_i2c_probe_device(bus, FALSE, &i2c_probe_info);
        if ((SOC_E_EXISTS == rv) || (SOC_E_NOT_FOUND == rv)
            || (SOC_E_INIT == rv)) {
            /* No problem */
            rv = SOC_E_NONE;
        } else if (SOC_FAILURE(rv)) {
            /* Access error */
            return rv;
        }
    }

    /* Now we know all of the devices that aren't behind a MUX.
     * Anything new we find now must be behind a MUX.
     * Recursively scan down the MUX tree.
     */
    if ((rv = _cpu_i2c_probe_mux(bus, -1, &i2c_probe_info)) < 0) {
        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META("CPU bus %d i2c: Could not probe MUX's\n"), bus));
        return rv;
    }

    return i2c_probe_info.devid_found;
}
#endif

