/*
 * $Id: robo_spi.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BCM5301X SPI utility functions
 */

/* #define EXPORT_SYMTAB */
#include <linux-bde.h>
#include <typedefs.h>
#include <shared/et/linux_osl.h>
#include <aiutils.h>
#include <aiutils_priv.h>
#include <sbchipc.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <soc/devids.h>
#include "robo_spi.h"
#include "nspspireg.h"
#include <shared/et/bcmdevs.h>

#ifdef  BCMDBG
#define SPI_ERR(args)   printk args
#else
#define SPI_ERR(args)
#endif

#define SI_CC_IDX   0

/* Private state per RoboSwitch */
typedef struct {
    void *sbh;        /* SiliconBackplane handle */
    nspspiregs_t *regs;      /* pointer to chip registers */
#ifdef _KERNEL_
    spinlock_t lock;        /* per-device perimeter lock */
#endif
} robo_info_t;

/* Forward declarations */

void *robo_attach_spi(void *sbh);
void robo_detach_spi(void *robo);
void robo_spi_wreg(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);
void robo_spi_rreg(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);

static void *robo_sbh = NULL;

#define ROBO_POLL_TIMEOUT 1000
#define ROBO_IF_TIMEOUT 10000

#define NSP_SF3_SPI_BASE 0x180000E4
#define CORE_SIZE        0x0C
#define SF3_MODEL_ID     0x35

/* Get access to the RoboSwitch */
void *
robo_attach_spi(void *sih)
{
    robo_info_t *robo;
    nspspiregs_t *regs;      /* pointer to chip registers */
    uint32 base_addr;
    /* Allocate private state */
    if (!(robo = MALLOC(sii->osh, sizeof(robo_info_t)))) {
        SPI_ERR(("robo_attach: out of memory"));
        return NULL;
    } else {
        robo_sbh = (void *)robo;
    }
    bzero((char *) robo, sizeof(robo_info_t));
    robo->sbh = sih;
    base_addr = NSP_SF3_SPI_BASE;
    if ((regs = (void*)REG_MAP(base_addr, sizeof(uint32) * 3)) == NULL) {
        SPI_ERR(("robo_attach: can't get base address"));
        return NULL;
    }
    robo->regs = regs;

#ifdef _KERNEL_
    /* Initialize lock */
    spin_lock_init(&robo->lock);
#endif
    return robo;
}

/* Release access to the RoboSwitch */
void
robo_detach_spi(void *rinfo)
{
    robo_info_t *robo = (robo_info_t *)rinfo;
    si_info_t *sii;

    sii = SI_INFO((si_t*)robo->sbh);

    if (robo->regs) {
        REG_UNMAP(robo->regs);
        robo->regs = NULL;
    }

    COMPILER_REFERENCE(sii);

    /* Free private state */
    MFREE(sii->osh, robo, sizeof(robo_info_t));
}

static
uint64 _switch_reg_read(void *rinfo, uint8 page, uint8 offset)
{
    uint64 value = ~(uint64)0;
    uint32 regval;
    uint32 timeout = ROBO_POLL_TIMEOUT;
    robo_info_t *robo = (robo_info_t *)rinfo;
    si_info_t *sii;
    osl_t       *osh;
    nspspiregs_t *regs;      /* pointer to chip registers */

    sii = SI_INFO((si_t*)robo->sbh);
    osh = sii->osh;
    COMPILER_REFERENCE(osh);
    regs = robo->regs;

    regval = (CCA_GSIOCTL_STARTBUSY_MASK |
            (CCA_GSIOCTL_GSIOCODE_2 << CCA_GSIOCTL_GSIOCODE_SHIFT) |
            (CCA_GSIOCTL_GSIOOP_READ & CCA_GSIOCTL_GSIOOP_MASK));
    W_REG(NULL, &regs->cca_gsioaddress, BCM53134_SPI_STS_REG);
    W_REG(NULL, &regs->cca_gsioctl, regval);
    while (R_REG(osh , &regs->cca_gsioctl) & CCA_GSIOCTL_STARTBUSY_MASK){
        OSL_DELAY(1);
    };

    regval = (CCA_GSIOCTL_STARTBUSY_MASK |
            (CCA_GSIOCTL_GSIOCODE_2 << CCA_GSIOCTL_GSIOCODE_SHIFT) |
            (CCA_GSIOCTL_GSIOOP_WRITE & CCA_GSIOCTL_GSIOOP_MASK));
    W_REG(NULL, &regs->cca_gsioaddress, BCM53134_SPI_PAGE_REG);
    W_REG(NULL, &regs->cca_gsiodata, page);
    W_REG(NULL, &regs->cca_gsioctl, regval);
    while (R_REG(osh , &regs->cca_gsioctl) & CCA_GSIOCTL_STARTBUSY_MASK) {
        OSL_DELAY(1);
    };

    regval = (CCA_GSIOCTL_STARTBUSY_MASK |
            (CCA_GSIOCTL_GSIOCODE_2 << CCA_GSIOCTL_GSIOCODE_SHIFT) |
            (CCA_GSIOCTL_GSIOOP_READ & CCA_GSIOCTL_GSIOOP_MASK));
    W_REG(NULL, &regs->cca_gsioaddress, offset);
    W_REG(NULL, &regs->cca_gsioctl, regval);
    while (R_REG(osh , &regs->cca_gsioctl) & CCA_GSIOCTL_STARTBUSY_MASK){
        OSL_DELAY(1);
    };
    do {
        W_REG(NULL, &regs->cca_gsioaddress, BCM53134_SPI_STS_REG);
        W_REG(NULL, &regs->cca_gsioctl, regval);
        while (R_REG(osh , &regs->cca_gsioctl) & CCA_GSIOCTL_STARTBUSY_MASK){
            OSL_DELAY(1);
        };
        regval = R_REG(osh, &regs->cca_gsiodata);
        timeout--;
    } while ((R_REG(osh, &regs->cca_gsiodata) & BCM53134_SPI_RACK_MASK) &&
           (timeout));
    regval = (CCA_GSIOCTL_STARTBUSY_MASK | CCA_GSIOCTL_NUMDATA_MASK |
            (CCA_GSIOCTL_GSIOCODE_2 << CCA_GSIOCTL_GSIOCODE_SHIFT) |
            (CCA_GSIOCTL_GSIOOP_READ & CCA_GSIOCTL_GSIOOP_MASK));
    W_REG(NULL, &regs->cca_gsioaddress, BCM53134_SPI_DATA_REG);
    W_REG(NULL, &regs->cca_gsioctl, regval);
    while (R_REG(osh , &regs->cca_gsioctl) & CCA_GSIOCTL_STARTBUSY_MASK) {
        OSL_DELAY(1);
    };

    regval = R_REG(osh, &regs->cca_gsiodata);

    value = (uint64) regval;
    return value;
}

static
void _switch_reg_write(void *rinfo, uint8 page, uint8 offset, uint64 value, uint len) 
{
    uint32 regval = 0;
    uint32 temp;
    robo_info_t *robo = (robo_info_t *)rinfo;
    si_info_t *sii;
    osl_t       *osh;
    uint8 *data8_ptr;
    uint8 *val8 = (uint8 *) &value;
    nspspiregs_t *regs;      /* pointer to chip registers */

    sii = SI_INFO((si_t*)robo->sbh);
    osh = sii->osh;
    COMPILER_REFERENCE(osh);
    regs = robo->regs;

    regval = (CCA_GSIOCTL_STARTBUSY_MASK |
            (CCA_GSIOCTL_GSIOCODE_2 << CCA_GSIOCTL_GSIOCODE_SHIFT) |
            (CCA_GSIOCTL_GSIOOP_READ & CCA_GSIOCTL_GSIOOP_MASK));
    W_REG(NULL, &regs->cca_gsioaddress, BCM53134_SPI_STS_REG);
    W_REG(NULL, &regs->cca_gsioctl, regval);
    while (R_REG(osh , &regs->cca_gsioctl) & CCA_GSIOCTL_STARTBUSY_MASK){
        OSL_DELAY(1);
    };

    regval = (CCA_GSIOCTL_STARTBUSY_MASK |
            (CCA_GSIOCTL_GSIOCODE_2 << CCA_GSIOCTL_GSIOCODE_SHIFT) |
            (CCA_GSIOCTL_GSIOOP_WRITE & CCA_GSIOCTL_GSIOOP_MASK));
    W_REG(NULL, &regs->cca_gsioaddress, BCM53134_SPI_PAGE_REG);
    W_REG(NULL, &regs->cca_gsiodata, page);
    W_REG(NULL, &regs->cca_gsioctl, regval);
    while (R_REG(osh , &regs->cca_gsioctl) & CCA_GSIOCTL_STARTBUSY_MASK) {
        OSL_DELAY(1);
    };

    data8_ptr = (uint8 *)&regval;
    for (temp = 0; temp < len; temp++) {
        data8_ptr[temp] = val8[len - temp -1];
    }
    temp = (((len - 1) << CCA_GSIOCTL_NUMDATA_SHIFT) & CCA_GSIOCTL_NUMDATA_MASK);
    W_REG(NULL, &regs->cca_gsioaddress, offset);
    W_REG(NULL, &regs->cca_gsiodata, regval );
    regval = (CCA_GSIOCTL_STARTBUSY_MASK | temp |
            (CCA_GSIOCTL_GSIOCODE_2 << CCA_GSIOCTL_GSIOCODE_SHIFT) |
            (CCA_GSIOCTL_GSIOOP_WRITE & CCA_GSIOCTL_GSIOOP_MASK));
    W_REG(NULL, &regs->cca_gsioctl, regval);
    while (R_REG(osh , &regs->cca_gsioctl) & CCA_GSIOCTL_STARTBUSY_MASK) {
        OSL_DELAY(1);
    };
}


/* Write chip register */
void
robo_spi_wreg(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len)
{
    uint64 regval;
    uint8  *data8_ptr;
#ifdef _KERNEL_
    unsigned long flags;
    spin_lock_irqsave(&robo->lock, flags);
#endif

    regval = 0;
    data8_ptr = (uint8 *)&regval;
    memcpy(data8_ptr, buf, len);

    if (len <= 4) {
        _switch_reg_write(rinfo, page, addr, (regval & UINT32_MASK), len);
    } else {
        _switch_reg_write(rinfo, page, addr, (regval & UINT32_MASK), (uint)4);
        _switch_reg_write(rinfo, page, addr, ((regval >> 32) & UINT32_MASK), (uint)(len - 4));
    }
#ifdef _KERNEL_
    spin_unlock_irqrestore(&robo->lock, flags);
#endif
}

/* Read chip register */
void
robo_spi_rreg(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len)
{
    uint64 regval;
    uint8  *data8_ptr;
#ifdef _KERNEL_
    unsigned long flags;
    spin_lock_irqsave(&robo->lock, flags);
#endif
    if (len <= 4) {
        regval = _switch_reg_read(rinfo, page, addr);
    } else {
        regval = (((_switch_reg_read(rinfo, page, addr + 4)) << 32) |
                    ( _switch_reg_read(rinfo, page, addr)));
    }

    data8_ptr = (uint8_t *)&regval;
    memcpy(buf, data8_ptr, len);

#ifdef _KERNEL_
    spin_unlock_irqrestore(&robo->lock, flags);
#endif
}

