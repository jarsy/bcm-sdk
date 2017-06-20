/*
 * $Id: robo_srab.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BCM5301X SRAB utility functions
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
#include "robo_srab.h"
#include "nssrabreg.h"
#include <shared/et/bcmdevs.h>

#ifdef	BCMDBG
#define	SRAB_ERR(args)	printk args
#else
#define	SRAB_ERR(args)
#endif

#define SI_CC_IDX   0

/* Private state per RoboSwitch */
typedef struct {
    void *sbh;        /* SiliconBackplane handle */

    nssrabregs_t *regs;      /* pointer to chip registers */

#ifdef _KERNEL_
    spinlock_t lock;        /* per-device perimeter lock */    
#endif
} robo_info_t;

/* Forward declarations */

void *robo_attach(void *sbh);
void robo_detach(void *robo);
void robo_wreg(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);
void robo_rreg(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);

static void *robo_sbh = NULL;

#define ROBO_POLL_TIMEOUT 1000
#define ROBO_IF_TIMEOUT 10000

#define CCB_SRAB_BASE 0x18007000
#define NSP_CCB_SRAB_BASE 0x18036000
#define CORE_SIZE 0x1000

/* Get access to the RoboSwitch */
void *
robo_attach(void *sih)
{
    robo_info_t *robo;
    si_info_t *sii;
    nssrabregs_t *regs;      /* pointer to chip registers */
	struct si_pub *sii_pub = NULL;
	uint32	base_addr;

    sii = SI_INFO((si_t*)sih);

    /* Allocate private state */
    if (!(robo = MALLOC(sii->osh, sizeof(robo_info_t)))) {
        SRAB_ERR(("robo_attach: out of memory"));
        return NULL;
    } else {
        robo_sbh = (void *)robo;
    }
    bzero((char *) robo, sizeof(robo_info_t));

    robo->sbh = sih;
	sii_pub = &sii->pub;

	if (sii_pub->chip == BCM53020_CHIP_ID) {
		base_addr = NSP_CCB_SRAB_BASE;
	} else {
		base_addr = CCB_SRAB_BASE;
	}
    if ((regs = (void*)REG_MAP(base_addr, CORE_SIZE)) == NULL) {
        SRAB_ERR(("robo_attach: can't get base address"));
        return NULL;
    }
/*    printk("%d: robo_attach: regs=0x%x\n",__LINE__,(uint32)regs);*/
    robo->regs = regs;

#ifdef _KERNEL_
    /* Initialize lock */
    spin_lock_init(&robo->lock);
#endif
    return robo;
}

/* Release access to the RoboSwitch */
void
robo_detach(void *rinfo)
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
void _switch_interface_reset(void *rinfo)
{
    uint32 timeout;
    robo_info_t *robo = (robo_info_t *)rinfo;
    si_info_t *sii;
    osl_t       *osh;
    nssrabregs_t *regs;      /* pointer to chip registers */

    sii = SI_INFO((si_t*)robo->sbh);
    osh = sii->osh;
    regs = robo->regs;

    COMPILER_REFERENCE(osh);

    /* Wait for switch initialization complete */
    timeout = ROBO_IF_TIMEOUT;
    while (!(R_REG(osh, &regs->chipcommonb_srab_sw_if) & CHIPCOMMONB_SRAB_SW_IF_SW_INIT_DONE_MASK)) {
        if (!timeout--) {
            SRAB_ERR(("srab reset switch interface: timeout"));
            break;
        }
    }

    /* Set the SRAU reset bit */
    W_REG(osh, &regs->chipcommonb_srab_cmdstat, CHIPCOMMONB_SRAB_CMDSTAT_SRA_RST_MASK);
    /* Wait for it to auto-clear */
    timeout = ROBO_IF_TIMEOUT;
    while (R_REG(osh, &regs->chipcommonb_srab_cmdstat) & CHIPCOMMONB_SRAB_CMDSTAT_SRA_RST_MASK) {
        if (!timeout--) {
            SRAB_ERR(("srab reset switch interface: timeout sra_rst"));
            return;
        }
    }
}

static
void _switch_request_grant(void *rinfo)
{
    uint32 regval;
    uint32 timeout = ROBO_IF_TIMEOUT;
    robo_info_t *robo = (robo_info_t *)rinfo;
    si_info_t *sii;
    osl_t       *osh;
    nssrabregs_t *regs;      /* pointer to chip registers */

    sii = SI_INFO((si_t*)robo->sbh);
    osh = sii->osh;
    regs = robo->regs;

    COMPILER_REFERENCE(osh);

	regval = R_REG(osh, &regs->chipcommonb_srab_sw_if);
    regval |= CHIPCOMMONB_SRAB_SW_IF_RCAREQ_MASK;

    W_REG(osh, &regs->chipcommonb_srab_sw_if, regval);
    while (!(R_REG(osh, &regs->chipcommonb_srab_sw_if) & CHIPCOMMONB_SRAB_SW_IF_RCAGNT_MASK)) {
        if (!timeout--) {
            SRAB_ERR(("srab request grant: timeout"));
            return;
        }
    }
}

static
void _switch_release_grant(void *rinfo)
{
    uint32 regval;
    robo_info_t *robo = (robo_info_t *)rinfo;
    si_info_t *sii;
    osl_t       *osh;
    nssrabregs_t *regs;      /* pointer to chip registers */

    sii = SI_INFO((si_t*)robo->sbh);
    osh = sii->osh;
    regs = robo->regs;

    COMPILER_REFERENCE(osh);

	regval = R_REG(osh, &regs->chipcommonb_srab_sw_if);
    regval &= ~CHIPCOMMONB_SRAB_SW_IF_RCAREQ_MASK;
    W_REG(osh, &regs->chipcommonb_srab_sw_if, regval);
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
    nssrabregs_t *regs;      /* pointer to chip registers */

    sii = SI_INFO((si_t*)robo->sbh);
    osh = sii->osh;
    regs = robo->regs;

    COMPILER_REFERENCE(osh);

    /* Assemble read command */
    _switch_request_grant(rinfo);
    regval = ((page << CHIPCOMMONB_SRAB_CMDSTAT_SRA_PAGE_SHIFT)
              | (offset << CHIPCOMMONB_SRAB_CMDSTAT_SRA_OFFSET_SHIFT)
              | CHIPCOMMONB_SRAB_CMDSTAT_SRA_GORDYN_MASK);
    W_REG(osh, &regs->chipcommonb_srab_cmdstat, regval);

    /* Wait for command complete */
    while (R_REG(osh, &regs->chipcommonb_srab_cmdstat) & CHIPCOMMONB_SRAB_CMDSTAT_SRA_GORDYN_MASK) {
        if (!--timeout) {
			SRAB_ERR(("robo_read: timeout"));
            _switch_interface_reset(rinfo);
            break;
        }
    }
    if (timeout) {
        /* Didn't time out, read and return the value */
        value = (((uint64)R_REG(osh, &regs->chipcommonb_srab_rdh)) << 32)
                        | R_REG(osh, &regs->chipcommonb_srab_rdl);
    }

    _switch_release_grant(rinfo);
    return value;
}

static
void _switch_reg_write(void *rinfo, uint8 page, uint8 offset, uint64 value)
{
    uint32 regval;
    uint32 timeout = ROBO_POLL_TIMEOUT;
    robo_info_t *robo = (robo_info_t *)rinfo;
    si_info_t *sii;
    osl_t       *osh;
    nssrabregs_t *regs;      /* pointer to chip registers */

    sii = SI_INFO((si_t*)robo->sbh);
    osh = sii->osh;
    regs = robo->regs;

    COMPILER_REFERENCE(osh);

    _switch_request_grant(rinfo);
    /* Load the value to write */
    W_REG(osh, &regs->chipcommonb_srab_wdh, (uint32)(value >> 32));
    W_REG(osh, &regs->chipcommonb_srab_wdl, (uint32)(value));

    /* Issue the write command */
    regval = ((page << CHIPCOMMONB_SRAB_CMDSTAT_SRA_PAGE_SHIFT)
              | (offset << CHIPCOMMONB_SRAB_CMDSTAT_SRA_OFFSET_SHIFT)
              | CHIPCOMMONB_SRAB_CMDSTAT_SRA_GORDYN_MASK
              | CHIPCOMMONB_SRAB_CMDSTAT_SRA_WRITE_MASK);
    W_REG(osh, &regs->chipcommonb_srab_cmdstat, regval);
    /* Wait for command complete */
    while (R_REG(osh, &regs->chipcommonb_srab_cmdstat) & CHIPCOMMONB_SRAB_CMDSTAT_SRA_GORDYN_MASK) {
        if (!--timeout) {
			SRAB_ERR(("robo_write: timeout"));
            _switch_interface_reset(rinfo);
            break;
        }
    }
    _switch_release_grant(rinfo);

}


/* Write chip register */
void
robo_wreg(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len)
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

    _switch_reg_write(rinfo, page, addr, regval);

#ifdef _KERNEL_
    spin_unlock_irqrestore(&robo->lock, flags);
#endif
}

/* Read chip register */
void
robo_rreg(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len)
{
	uint64 regval;
    uint8  *data8_ptr;

#ifdef _KERNEL_
    unsigned long flags;
    spin_lock_irqsave(&robo->lock, flags);
#endif

    regval = _switch_reg_read(rinfo, page, addr);

    data8_ptr = (uint8_t *)&regval;
    memcpy(buf, data8_ptr, len);

#ifdef _KERNEL_
    spin_unlock_irqrestore(&robo->lock, flags);
#endif
}

#if defined(IPROC_CMICD)
static int
_robo_otp_value_get(uint32 dmu_base, 
                    uint32 addr_offset, uint32 *otp_value)
{
    uint32 timeout;
    uint32 addr_base;
#define OTP_CPU_CONTROL0_OFFSET         0x240
#define OTP_CPU_CONTROL1_OFFSET         0x244
#define OTP_CPU_ADDRESS_OFFSET          0x24c
#define OTP_CPU_BITSEL_OFFSET           0x250
#define OTP_CPU_WDATA_OFFSET            0x254
#define OTP_CPU_CONFIG_OFFSET           0x258
#define OTP_CPU_RDATA_OFFSET            0x25c
#define OTP_CPU_STATUS_OFFSET           0x260


    addr_base = (sal_vaddr_t)ioremap_nocache(dmu_base, 0x1000);
    if (!addr_base) {
        return(1);
    }

	/* config OTP to enable CPU IF */
	writel(0x01, (volatile uint32 *) (addr_base + OTP_CPU_CONFIG_OFFSET));


	/* IF clean */
	writel(0, (volatile uint32 *) (addr_base + OTP_CPU_CONTROL0_OFFSET));
	writel(0, (volatile uint32 *) (addr_base + OTP_CPU_CONTROL1_OFFSET));
	writel(0, (volatile uint32 *) (addr_base + OTP_CPU_ADDRESS_OFFSET));
	writel(0, (volatile uint32 *) (addr_base + OTP_CPU_BITSEL_OFFSET));

	timeout = 10000;
	while(timeout) {
		if(readl((volatile uint32 *) (addr_base + 
                            OTP_CPU_STATUS_OFFSET))& 0x1) {
			break;
        }
		timeout--;
	}
	if(!timeout) {
        return(1);
    }

	/* disable cmd */
	writel(0x00a00600, 
	    (volatile uint32 *) (addr_base + OTP_CPU_CONTROL0_OFFSET));
	writel(0x00a81005, 
        (volatile uint32 *) (addr_base + OTP_CPU_CONTROL1_OFFSET));

	timeout = 10000;
	while(timeout) {
		if(readl((volatile uint32 *) 
                (addr_base + OTP_CPU_STATUS_OFFSET))& 0x1) {
			break;
        }
		timeout--;
	}
	if(!timeout) {
        return(1);
    }

	writel(0x00a00600, 
        (volatile uint32 *) (addr_base + OTP_CPU_CONTROL0_OFFSET));
	writel(0x00a81004, 
        (volatile uint32 *) (addr_base + OTP_CPU_CONTROL1_OFFSET));


	/* address */
	writel(addr_offset, (volatile uint32 *) (addr_base + OTP_CPU_ADDRESS_OFFSET));

	/* read cmd */
	writel(0x00a00600, 
	    (volatile uint32 *) (addr_base + OTP_CPU_CONTROL0_OFFSET));
	writel(0x00881001, 
        (volatile uint32 *) (addr_base + OTP_CPU_CONTROL1_OFFSET));

	timeout = 10000;
	while(timeout) {
		if(readl((volatile uint32 *) 
            (addr_base + OTP_CPU_STATUS_OFFSET))& 0x1) {
			break;
        }
		timeout--;
	}
	if(!timeout) {
        return(1);
    }

	/* read reg */
	*otp_value = readl((volatile uint32 *) (addr_base + OTP_CPU_RDATA_OFFSET));
	/* stop cmd */
	writel(0x00a00600, 
	    (volatile uint32 *) (addr_base + OTP_CPU_CONTROL0_OFFSET));
	writel(0x00a81016, 
        (volatile uint32 *) (addr_base + OTP_CPU_CONTROL1_OFFSET));
    iounmap((void *)addr_base);
      
#undef  OTP_CPU_CONTROL0_OFFSET  
#undef  OTP_CPU_CONTROL1_OFFSET 
#undef  OTP_CPU_ADDRESS_OFFSET 
#undef  OTP_CPU_BITSEL_OFFSET 
#undef  OTP_CPU_WDATA_OFFSET      
#undef  OTP_CPU_CONFIG_OFFSET  
#undef  OTP_CPU_RDATA_OFFSET     
#undef  OTP_CPU_STATUS_OFFSET       
    
	return(0);
}

void
robo_model_id_adjust_from_otp(void *rinfo, uint16 *model_id)
{

    si_info_t *sii;
    robo_info_t *robo = (robo_info_t *)rinfo;
    struct si_pub *sii_pub = NULL;
    uint32  dmu_base_addr, data_reg;;

#define NSP_DMU_ENUM_BASE                       0x1803f000
#define NSP_OTP_GPHY_EXT_PWRDOWN_OFFSET         (0xf)
#define NSP_OTP_GPHY_EXT_PWRDOWN_MASK           (0x1f)
#define NSP_OTP_MACSEC_ENABLE_OFFSET            (0xd)
#define NSP_OTP_MACSEC_ENABLE_MASK              (0x300000)


    sii = SI_INFO((si_t*)robo->sbh);
    sii_pub = &sii->pub;

    if (sii_pub->chip == BCM53020_CHIP_ID) {
		dmu_base_addr = NSP_DMU_ENUM_BASE;
	} else {
		return;
	}

    if ((*model_id == BCM53022_DEVICE_ID) || 
        (*model_id == BCM53025_DEVICE_ID)) {
        /* Default SKU : 58622/58625 */
        *model_id += 0x600;
        /* Get the gphy_ext_pwrdown in OTP fields */
        if (_robo_otp_value_get(dmu_base_addr, NSP_OTP_GPHY_EXT_PWRDOWN_OFFSET, 
            &data_reg) == 0x0) {
            if (data_reg & NSP_OTP_GPHY_EXT_PWRDOWN_MASK) {
                /* SKU : 58522/58525 */
                *model_id -= 0x100;
            }            
        }        
    } else if (*model_id == BCM53023_DEVICE_ID) {
        /* Get the gphy_ext_pwrdown in OTP fields */
        if (_robo_otp_value_get(dmu_base_addr, NSP_OTP_MACSEC_ENABLE_OFFSET, 
            &data_reg) == 0x0) {
            if (data_reg & NSP_OTP_MACSEC_ENABLE_MASK) {
                /* SKU ID BCM58623 */
                *model_id += 0x600;
            }
            
        }
    }

#undef  NSP_DMU_ENUM_BASE                     
#undef  NSP_OTP_GPHY_EXT_PWRDOWN_OFFSET  
#undef  NSP_OTP_GPHY_EXT_PWRDOWN_MASK     
#undef  NSP_OTP_MACSEC_ENABLE_OFFSET   
#undef  NSP_OTP_MACSEC_ENABLE_MASK  

    return;

}


#endif /* IPROC_CMICD */


