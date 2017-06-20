/*
 * $Id: pca9505.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BCM56xx I2C Device Driver for Phillips PCA9505 i2c switch
 *
 *
 */

#include <shared/bsl.h>

#include <sal/types.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/error.h>
#include <soc/i2c.h>
#include <shared/bsl.h>
#if  defined(SHADOW_SVK) || defined(BCM_CALADAN3_SVK)

#ifdef SHADOW_SVK
#define PCA9505_DEV_MAX         3
#define PCA9505_PORT_MAX        5
#elif BCM_CALADAN3_SVK
#define PCA9505_DEV_MAX         7
#define PCA9505_PORT_MAX        5
#endif

#define PCA9505_IOP_BASE        0x08
#define PCA9505_POL_BASE        0x10
#define PCA9505_IOC_BASE        0x18
#define PCA9505_MSK_BASE        0x20


int
pio_devname(int dev, char **name)
{
    switch (dev) {
    case 0:
        *name = I2C_IOP_0;
        break;
    case 1:
        *name = I2C_IOP_1;
        break;
#ifdef BCM_CALADAN3_SVK
    case 2:
        *name = I2C_IOP_2;
        break;
    case 3:
        *name = I2C_IOP_3;
        break;
    case 4:
        *name = I2C_IOP_4;
        break;
    case 5:
        *name = I2C_IOP_5;
        break;
    case 6:
        *name = I2C_IOP_6;
        break;
#endif

    default:
        *name = I2C_IOP_0;
        break;
    }
    return SOC_E_NONE;
}


/* Port configuration */
static struct pca9505_port_config {
    uint8 io_config;         /* port pin: 0=ouput, 1==input */
    uint8 mask_intr;         /* mask : 0=interrupt, 1==no interrupt*/
    uint8 defaults;          /* default value in case of output*/
    uint8 op_config;         /* used only if output, 1=toggle output, 0=directly set the default*/
} pca9505_port_config[][PCA9505_PORT_MAX] = {

#ifdef SHADOW_SVK
    {   /* device @ SADDR0 */
        { 0xff, 0xff, 0x00, 0x00 },  /* port 0: SFP_ABS[0..7]    */
        { 0x00, 0xff, 0x00, 0x00 },  /* port 1: SFP_TXDIS[0..7]  */
        { 0xff, 0xff, 0x00, 0x00 },  /* port 2: SFP_TXFLT[0..7]  */
        { 0xff, 0xff, 0x00, 0x00 },  /* port 3: QSFP_PRS[0..3], QSFP_INT[4..7] */
        { 0xff, 0xff, 0x00, 0x00 },  /* port 4: UNUSED */
    },
    {   /* device @ SADDR1 */
        { 0xff, 0xff, 0x00, 0x00 },  /* port 0: XON_XOFF[0..7]  */ /* changed this
        to input as per Doug's request as these are overloaded for EB3 address
        */
        { 0xff, 0xff, 0x00, 0x00 },  /* port 1: BRD_ID[0..7]    */
        { 0x00, 0xff, 0x00, 0x00 },  /* port 2: SHAD_GPIO[0..7] */
        { 0xff, 0xff, 0x00, 0x00 },  /* port 3: UNUSED          */
        { 0xff, 0xff, 0x00, 0x00 },  /* port 4: JTAG_TCE[0]     */
    },
    {   /* device @ SADDR0 */
        { 0xe0, 0xff, 0x0e, 0x00 },  /* port 0: MOD_ABS, LO-PWR, PRG[3:1], TXDIS */
        { 0xe0, 0xff, 0x0e, 0x00 },  /* port 1: MOD_ABS, LO-PWR, PRG[3:1], TXDIS */
        { 0xff, 0xff, 0x00, 0x00 },  /* port 2: SFP_TXFLT[0..7]  */
        { 0xff, 0xff, 0x00, 0x00 },  /* port 3: QSFP_PRS[0..3], QSFP_INT[4..7] */
        { 0xff, 0xff, 0x00, 0x00 },  /* port 4: UNUSED */
    }
#elif BCM_CALADAN3_SVK
    {   /* device @ SADDR0 */
        { 0x40, 0xff, 0x80, 0x00 },  /* port 0: */ 
        { 0xff, 0xff, 0x00, 0x00 },  /* port 1: */
        { 0x00, 0xff, 0xff, 0x00 },  /* port 2: */
        { 0x00, 0xff, 0xff, 0x00 },  /* port 3: */
        { 0x00, 0xff, 0xff, 0x00 },  /* port 4: */
    },
    {   /* device @ SADDR1 */
        { 0xd5, 0xff, 0x00, 0x00 },  /* port 0: */ 
        { 0x55, 0xff, 0x00, 0x00 },  /* port 1: */
        { 0x55, 0xff, 0x00, 0x00 },  /* port 2: */
        { 0x55, 0xff, 0x00, 0x00 },  /* port 3: */
        { 0x55, 0xff, 0x00, 0x00 },  /* port 4: */
    },
    {   /* device @ SADDR2 */
        { 0x55, 0xff, 0x00, 0x00 },  /* port 0: */ 
        { 0x55, 0xff, 0x00, 0x00 },  /* port 1: */
        { 0x55, 0xff, 0x00, 0x00 },  /* port 2: */
        { 0x55, 0xff, 0x00, 0x00 },  /* port 3: */
        { 0x55, 0xff, 0x00, 0x00 },  /* port 4: */
    },
    {   /* device @ SADDR3 */
        { 0xdd, 0xff, 0x00, 0x00 },  /* port 0: */ 
        { 0xdd, 0xff, 0x00, 0x00 },  /* port 1: */
        { 0xd5, 0xff, 0x00, 0x00 },  /* port 2: */
        { 0xdd, 0xff, 0x00, 0x00 },  /* port 3: */
        { 0xdd, 0xff, 0x00, 0x00 },  /* port 4: */
    },
    {   /* device @ SADDR4 */
        { 0xdd, 0xff, 0x00, 0x00 },  /* port 0: */ 
        { 0xd5, 0xff, 0x00, 0x00 },  /* port 1: */
        { 0xdd, 0xff, 0x00, 0x00 },  /* port 2: */
        { 0xdd, 0xff, 0x00, 0x00 },  /* port 3: */
        { 0xdd, 0xff, 0x00, 0x00 },  /* port 4: */
    },
    {   /* device @ SADDR5 */
        { 0xd5, 0xff, 0x00, 0x00 },  /* port 0: */ 
        { 0xdd, 0xff, 0x00, 0x00 },  /* port 1: */
        { 0xdd, 0xff, 0x00, 0x00 },  /* port 2: */
        { 0xdd, 0xff, 0x00, 0x00 },  /* port 3: */
        { 0x00, 0xff, 0x00, 0x00 },  /* port 4: */
    },
    {   /* device @ SADDR6 */
        { 0xff, 0xff, 0x00, 0x00 },  /* port 0: */ 
        { 0x00, 0xff, 0x00, 0x00 },  /* port 1: */
        { 0x00, 0xff, 0x00, 0x00 },  /* port 2: */
        { 0x00, 0xff, 0x00, 0x00 },  /* port 3: */
        { 0xff, 0xff, 0x00, 0x00 },  /* port 4: */
    },

#endif
};


STATIC int
pca9505_read(int unit, int devno, uint16 addr, uint8 *data, uint32 *len)
{

    int rv;
    uint8 saddr;

    saddr = soc_i2c_addr(unit, devno);

    rv = soc_i2c_read_byte_data(unit, saddr, addr, data);
    *len = 1; /* Byte device */
    LOG_INFO(BSL_LS_SOC_I2C,
             (BSL_META_U(unit,
                         "iop@0x%x: read 0x%x from addr 0x%x\n"), saddr, *data, addr));
    soc_i2c_device(unit, devno)->rbyte++;   
    return rv;
}



STATIC int
pca9505_write(int unit, int devno, uint16 addr, uint8 *data, uint32 len)
{
    int rv;
    uint8 saddr;

    saddr = soc_i2c_addr(unit, devno);

    rv = soc_i2c_write_byte_data(unit, saddr, addr, *data);
    LOG_INFO(BSL_LS_SOC_I2C,
             (BSL_META_U(unit,
                         "iop@0x%x: wrote 0x%x to addr 0x%x\n"), saddr, *data, addr));
    soc_i2c_device(unit, devno)->tbyte++;   
    return rv;
}



STATIC int
pca9505_ioctl(int unit, int devno, int opcode, void* data, int len)
{
    return SOC_E_NONE;
}

STATIC int
pca9505_saddr_to_devid(int unit, uint8 saddr) {

    int dev_idx = -1;

#ifdef SHADOW_SVK
    if (saddr == I2C_IOP_SADDR0) {
        dev_idx = 0;
    } else if (saddr == I2C_IOP_SADDR1) {
        dev_idx = 1;
    } else {
        dev_idx = -1;
    }

    if ((saddr == I2C_IOP_SADDR0) &&
        (soc_property_get(unit, "cfp_enable", 0))) {
        dev_idx = 2;
    }

#elif BCM_CALADAN3_SVK
    if (saddr == I2C_IOP_SADDR0) {
        dev_idx = 0;
    } else if (saddr == I2C_IOP_SADDR1) {
        dev_idx = 1;
    } else if (saddr == I2C_IOP_SADDR2) {
        dev_idx = 2;
    } else if (saddr == I2C_IOP_SADDR3) {
        dev_idx = 3;
    } else if (saddr == I2C_IOP_SADDR4) {
        dev_idx = 4;
    } else if (saddr == I2C_IOP_SADDR5) {
        dev_idx = 5;
    } else if (saddr == I2C_IOP_SADDR6) {
        dev_idx = 6;
    } else {
        dev_idx = -1;
    }
#endif
    return dev_idx;

}

STATIC int
pca9505_init(int unit, int devno, void* data, int len)
{
    int port;
    int dev_idx;
    int rv;
    uint8 value = 0;
    uint8 saddr;

    saddr = soc_i2c_addr(unit, devno);        

    soc_i2c_devdesc_set(unit, devno, "PCA9505 IO Port");

    dev_idx = pca9505_saddr_to_devid(unit, saddr);
    if (dev_idx < 0) {
        LOG_CLI((BSL_META_U(unit,
                            "%s: invalid address 0x%x\n"),
                 soc_i2c_devname(unit, devno), saddr));
        return SOC_E_CONFIG;
    }

    /* Configure port IO pin direction and mask */
    for (port = 0; port < PCA9505_PORT_MAX; port++) {

        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META_U(unit,
                             "iop: Initializing port %d \n"), port));
        /* Disable polarity inversion */
        rv = pca9505_write(unit, devno, PCA9505_POL_BASE + port, 
                               &value, 1);

        /* Configure reasonable value for output */
        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META_U(unit,
                             "iop: Setting known output on %d : 0x%x \n"), 
                  port, pca9505_port_config[dev_idx][port].defaults));
        if (pca9505_port_config[dev_idx][port].op_config) {
            /* Support for toggling any resets */
            uint8 idata = 0;
            idata = pca9505_port_config[dev_idx][port].defaults &
                     ~(pca9505_port_config[dev_idx][port].op_config);
            idata |= ~(pca9505_port_config[dev_idx][port].defaults & 
                       pca9505_port_config[dev_idx][port].op_config);
            rv = pca9505_write(unit, devno, PCA9505_IOP_BASE + port, &idata, 1); 
        }
        rv = pca9505_write(unit, devno, PCA9505_IOP_BASE + port, 
                           &pca9505_port_config[dev_idx][port].defaults, 1);
        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META_U(unit,
                             "iop: Setting interrupt mask on %d : 0x%x \n"), 
                  port, pca9505_port_config[dev_idx][port].mask_intr));
        rv = pca9505_write(unit, devno, PCA9505_MSK_BASE + port, 
                           &pca9505_port_config[dev_idx][port].mask_intr, 1);
        if (rv != SOC_E_NONE) {
            LOG_CLI((BSL_META_U(unit,
                                "Failed initializing PCA9505 interrupt mask "
                                "addr=0x%x, port=%d\n"), saddr, port));
        }

        /* Setup IO config only after loading correct values */
        rv = pca9505_write(unit, devno, PCA9505_IOC_BASE + port, 
                           &pca9505_port_config[dev_idx][port].io_config, 1);
        if (rv != SOC_E_NONE) {
            LOG_CLI((BSL_META_U(unit,
                                "Failed initializing PCA9505 pin direction "
                                "addr=0x%x, port=%d\n"), saddr, port));
            return SOC_E_FAIL;
        }
    }        
    return SOC_E_NONE;
}

void
pca9505_dump(int dev, int startp, int endp)
{
    uint8 val_0;
    uint8 val_8;
    uint8 val_10;
    uint8 val_18;
    uint8 val_20;
    int i;
    uint8 saddr;

    switch (dev) {
    case 0:
        saddr = I2C_IOP_SADDR0; break;
    case 1:
        saddr = I2C_IOP_SADDR1; break;
    case 2:
#ifdef SHADOW_SVK
        saddr = I2C_IOP_SADDR0; break;
#elif BCM_CALADAN3_SVK
        saddr = I2C_IOP_SADDR2; break;
    case 3:
        saddr = I2C_IOP_SADDR3; break;
    case 4:
        saddr = I2C_IOP_SADDR4; break;
    case 5:
        saddr = I2C_IOP_SADDR5; break;
    case 6:
        saddr = I2C_IOP_SADDR6; break;
#endif
    default:
        return;
    }
    for(i = startp; i < endp; i++) {
        soc_i2c_read_byte_data(0,saddr, 0x00 + i, &val_0);
        soc_i2c_read_byte_data(0,saddr, 0x08 + i, &val_8);
        soc_i2c_read_byte_data(0,saddr, 0x10 + i, &val_10);
        soc_i2c_read_byte_data(0,saddr, 0x18 + i, &val_18);
        soc_i2c_read_byte_data(0,saddr, 0x20 + i, &val_20);
        LOG_CLI((BSL_META("R[0x%02x] = 0x%02x\n"), 0x00 + i, (uint32)val_0));
        LOG_CLI((BSL_META("R[0x%02x] = 0x%02x\n"), 0x08 + i, (uint32)val_8));
        LOG_CLI((BSL_META("R[0x%02x] = 0x%02x\n"), 0x10 + i, (uint32)val_10));
        LOG_CLI((BSL_META("R[0x%02x] = 0x%02x\n"), 0x18 + i, (uint32)val_18));
        LOG_CLI((BSL_META("R[0x%02x] = 0x%02x\n"), 0x20 + i, (uint32)val_20));
   }
}


/* PCA9505 Clock Chip Driver callout */
i2c_driver_t _soc_i2c_pca9505_driver = {
    0x0, 0x0, /* System assigned bytes */
    PCA9505_DEVICE_TYPE,
    pca9505_read,
    pca9505_write,
    pca9505_ioctl,
    pca9505_init,
    NULL,
};

#endif /* SHADOW_SVK */
