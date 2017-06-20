/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 *               [BSC] - Broadcom Serical Control (I2C constroller)
 *                 |
 *       ==========O=======o======================o========== I2C bus
 *                         |                      |
 *                    [pca9548 - 8-port mux]  [at24c64 - serial eeprom]
 *                      | | | | | | | |
 *           [  SFP ]  -  | | | | | | |
 *            [  SFP  ]  -  | | | | | |-- [ SFP ]
 *             [  SFP  ]   -  | | | |
 *               [  SFP  ]   -  | | ---[ SPF ]
 *                 [  SFP  ]   -   - [  SFP  ]
 */


#if defined(INCLUDE_I2C) && defined(BCM_FE2000_SUPPORT)
#include <sal/types.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/error.h>
#include <soc/bsc.h>
#include <soc/sbx/fe2k_common/sbFe2000Common.h>
#include <appl/diag/sbx/brd_sbx.h>

extern unsigned int
sbFe2000UtilIICRead(	int unit,
			unsigned int slave_dev_addr,
			unsigned int reg_index,
			unsigned int *data);

extern unsigned int
sbFe2000UtilIICWrite(	int unit,
			unsigned int slave_dev_addr,
			unsigned int reg_index,
			unsigned int data);

/*
 * Function: sfp_init
 *
 * Parameters:
 *    unit - StrataSwitch device number or BSC bus number
 *    devno - chip device id
 */
static int
sfp_init(int unit, int devno)
{
    return SOC_E_NONE;
}

static int sfp_write(int unit, int devno, uint32 addr, uint32 wdata)
{
    soc_bsc_bus_t *bscbus = BSCBUS(unit);
    int saddr = bscbus->devs[devno]->saddr;
    int chan = bscbus->devs[devno]->chan;
    int mux = bscbus->devs[devno]->mux;
    int retv;

    LOG_INFO(BSL_LS_SOC_I2C,
             (BSL_META_U(unit,
                         "saddr = %02X chan = %02X mux = %02X addr = %02X wdata = %02X\n"),
              saddr, chan, mux, addr, wdata));
    BSC_LOCK(unit);

    /* Select MUX to slave device */
    retv = sbFe2000UtilIICWrite(unit, mux, 0, chan);
    if (retv != 0) {
        BSC_UNLOCK(unit);
        return retv;
    }

    retv = sbFe2000UtilIICWrite(unit, saddr, addr, wdata);
    if (retv != 0) {
        BSC_UNLOCK(unit);
        return retv;
    }

    BSC_UNLOCK(unit);
    return SOC_E_NONE;
}

static int sfp_read(int unit, int devno, uint32 addr, uint32 *rdata)
{
    soc_bsc_bus_t *bscbus = BSCBUS(unit);
    int saddr = bscbus->devs[devno]->saddr;
    int chan = bscbus->devs[devno]->chan;
    int mux = bscbus->devs[devno]->mux;
    int retv;


    LOG_INFO(BSL_LS_SOC_I2C,
             (BSL_META_U(unit,
                         "saddr = %02X chan = %02X mux = %02X addr = %02X\n"),
              saddr, chan, mux, addr));
    BSC_LOCK(unit);

    /* Select MUX to slave device */
    retv = sbFe2000UtilIICWrite(unit, mux, 0, chan);
    if (retv != 0) {
        BSC_UNLOCK(unit);
        return retv;
    }

    if (rdata != NULL) {
        retv = sbFe2000UtilIICRead(unit, saddr, addr, rdata);
        if (retv != 0) {
            BSC_UNLOCK(unit);
            return retv;
        }
    }
    BSC_UNLOCK(unit);

    LOG_INFO(BSL_LS_SOC_I2C,
             (BSL_META_U(unit,
                         "saddr = %02X chan = %02X mux = %02X addr = %02X rdata = %02X\n"),
              saddr, chan, mux, addr, (rdata != NULL) ? *rdata : 0));
    return SOC_E_NONE;
}

/*
 * Function: soc_sfp_read
 *
 * Purpose: Read len bytes of data into buffer, update len with total
 *          amount read.
 *
 * Parameters:
 *    unit - StrataSwitch device number or BSC bus number
 *    devno - chip device id
 *    addr - NVRAM memory address to read from
 *    data - address of data buffer to read into
 *    len - address containing number of bytes read into data buffer (updated
 *          with number of bytes read on completion).
 *
 * Returns: data bufffer filled in with data from address, number of
 *          bytes read is updated in len field. Status code:
 *
 *          SOC_E_NONE -- no error encounter
 *          SOC_E_TIMEOUT - chip timeout or data error
 *
 * Notes:
 *         Currently uses random address byte read to initiate the read; if
 *         more than one byte of data is requested at the current address, a
 *         sequential read operation is performed.
 */
int
soc_sfp_read(int unit, int devno, uint16 addr, uint8* data, uint32 *len)
{
    int retv, i = 0;
    uint32 rdata;

    if (!soc_bsc_is_attached(unit)) {
        soc_bsc_attach(unit);
    }

    if (!len || (*len == 0) || !data ||
                    ((addr + *len) > SFP_DEVICE_SIZE)) {
        return SOC_E_PARAM;
    }
    for (i = 0; i < *len; i++) {
        retv = sfp_read(unit, devno, addr + i, &rdata);
        if (retv != SOC_E_NONE) {
            goto err_exit;
        }
        data[i] = rdata & 0xff;
    }
    *len = i;
    return SOC_E_NONE;

err_exit:
    *len = i;
    return retv;
}

/*
 * Function: sfp_write
 * Purpose: Write len bytes of data, return the number of bytes written.
 *
 * Parameters:
 *    unit - StrataSwitch device number or BSC bus number
 *    devno - chip device id
 *    addr - NVRAM memory address to write to
 *    data - address of data buffer to write from
 *    len - number of bytes to write
 *
 * Returns:
 *          SOC_E_NONE -- no error encountered
 *          SOC_E_TIMEOUT - chip timeout or data error
 *
 *
 */
int
soc_sfp_write(int unit, int devno, uint16 addr, uint8* data, uint32 len)
{
    int retv, i;
    uint32 wdata;

    if (!soc_bsc_is_attached(unit)) {
        soc_bsc_attach(unit);
    }

    if (!len || !data || ((addr + len) > SFP_DEVICE_SIZE))
            return SOC_E_PARAM;

    for (i = 0; i < len; i++) {
        wdata = data[i];
        retv =  sfp_write(unit, devno, addr + i, wdata);
        if (retv != SOC_E_NONE) {
            goto err_exit;
        }
    }
    return SOC_E_NONE;

err_exit:
    return retv;
}

static int sfp_ioctl(int unit, int devno, int opcode, void* data, int len)
{
    uint8 tmp_data;
    uint32 tmp_len = 1;

    switch(opcode) {
    case BSC_IOCTL_DEVICE_PRESENT:
        if (soc_sfp_read(unit, devno, 0, &tmp_data, &tmp_len) == 0) {
            return 1; /* found */
        } 
    default:
            break;
    }
    return 0;
}

bsc_driver_t _soc_bsc_sfp_driver = {
    0x0,			/* flags */
    0x0,			/* devno */
    SFP_DEVICE_TYPE,	/* id */
    sfp_read,		/* read */
    sfp_write,		/* write */
    sfp_ioctl,		/* ioctl */
    sfp_init,		/* init */
};

#else/* BCM_FE2000_SUPPORT && INCLUDE_I2C */
int _soc_i2c_sfp_not_empty;
#endif /* BCM_FE2000_SUPPORT && INCLUDE_I2C */

