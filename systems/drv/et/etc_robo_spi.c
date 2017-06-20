/*
 * $Id: etc_robo_spi.c,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BCM53xx RoboSwitch utility functions
 */

/* #define EXPORT_SYMTAB */
#if defined(linux)
#include <linux-bde.h>
#include <typedefs.h>
#include <shared/et/linux_osl.h>
#include <aiutils.h>
#include <aiutils_priv.h>
#include <sbchipc.h>
#include <nvutils.h>
#include <proto/ethernet.h>
#include <et_dbg.h>
#include <etc53xx.h>
#include <soc/etcgmac.h>
#include <soc/gmac_common.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <soc/devids.h>
#include "etc_robo_spi.h"
#elif defined(VXWORKS)
#include <osl.h>
#include <typedefs.h>
#include <hndsoc.h>
#include <siutils.h>
#include <siutils_priv.h>
#include <sbchipc.h>
#include <sbconfig.h>
#include <ethernet.h>
#include <et_dbg.h>
#include <etc53xx.h>
#include <etcgmac.h>
#include <gmac_common.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <soc/devids.h>
#include "etc_robo_spi.h"
#endif

#ifdef	BCMDBG
#define	SPI_MSG(args)	printf args
#else
#define	SPI_MSG(args)
#endif

#if defined(ROBODVT) && defined(VXWORKS) /* Enable error message reported for DVT test env */
#ifdef ET_ERROR
#undef ET_ERROR
#endif

#define ET_ERROR(args)  printf args
#endif

#define ROBO_DEFAULT_SPI_TIMEOUT  50
#define ROBO_DEFAULT_MDCMDIO_TIMEOUT  500

#define SPI_PAGE            0xFF /* SPI global PAGE */
#define SPI_ROBO_RESET      0xFF /* SPI PAGE register */
#define SPI_ROBO_STRAP_CID  6    /* The chip ID (CID) of slave 8051 on SPI bus */
#define SPI_ROBO_STRAP_PAGE 0    /* The page # of slave 8051 on SPI bus */
#define SPI_ROBO_STRAP_MASK 0xF  /* Bit 0-3 reserved for the offset of strap pin setting */
#define ROBO_HW_RESET_GPIO  0x800/* GPIO pin 0 (bitmask 0x1) used for HW reset in BCM5836/4704 */

#define SI_CC_IDX   0

/* Private state per RoboSwitch */
typedef struct {
    void *sbh;        /* SiliconBackplane handle */
    uint coreidx;        /* Current core index */
    uint id;        /* SPI device ID */
    uint8 buf[32];        /* Software control info */
    uint buf_index;        /* the buffer length */
    uint states;        /* start ready to do read/write transaction */
    int cid, page;        /* Current chip ID and page */
    void *ebbase;
    uint8  bustype;
    uint8 phytype; /* direct phy access types, mdcmdio only */
#define DIRECT_PHY_ACCESS_UNKNOWN 0 /* switch chip is unknown */
#define DIRECT_PHY_ACCESS_FE_FAMILY 1  /* 53242,53262,53280,53600 */
#define DIRECT_PHY_ACCESS_GE_FAMILY 2 /* 53115,53118,53101,53125,89500,5389,5396 */
    uint8 resettype; /* switch reset register types, mdcmdio only */
#define HW_RESET_UNKNOWN_FAMILY 0
#define HW_RESET_FE_FAMILY 1
#define HW_RESET_GE_FAMILY 2

#ifdef _KERNEL_
    spinlock_t lock;        /* per-device perimeter lock */    
#endif
} robo_info_t;

/* Forward declarations */
static int chipc_spi_init(robo_info_t * robo);
static int chipc_spi_enable(robo_info_t * robo, uint8 slave);
static int chipc_spi_disable(robo_info_t * robo, uint8 slave);
static int chipc_spi_read(robo_info_t * robo, uint8 * buf, int len, uint8 data_out);
static int chipc_spi_write(robo_info_t * robo, uint8 * buf, int len);
static int chipc_spi_io_select(robo_info_t * robo, cc_spi_id_t id, int en);

void *robo_attach(void *sbh, uint8 ss);

void robo_detach(void *robo);
void robo_wreg(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);
void robo_rreg(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);
void robo_rvmii(void *rinfo, uint8 cid);
int chipc_spi_set_freq(void* robo, cc_spi_id_t id, uint32 speed_hz);

static void *robo_sbh = NULL;

#define ROBO_POLL_DELAY_US 10
#define EB_BUS  0
#define ROBO_SPI_BUS  1
#define ROBO_MDCMDIO_BUS 2


/* Get access to the RoboSwitch */
void *
robo_attach(void *sih, uint8 ss)

{
    robo_info_t *robo;
    si_info_t *sii;

    sii = SI_INFO((si_t*)sih);

    if (!CC_SPI_ID_IS_VALID(ss)){
        ET_ERROR(("%s: Invalid SPI device ID %d\n", FUNCTION_NAME(), ss));
        return NULL;
    }

    /* Allocate private state */
    if (!(robo = MALLOC(sii->osh, sizeof(robo_info_t)))) {
        ET_ERROR(("robo_attach: out of memory"));
        return NULL;
    } else {
        robo_sbh = (void *)robo;
    }

    bzero((char *) robo, sizeof(robo_info_t));
    robo->sbh = sih;
    robo->cid = robo->page = -1;
    robo->bustype = EB_BUS;/*ROBO_SPI_BUS;*/
    robo->phytype = DIRECT_PHY_ACCESS_UNKNOWN;
    robo->resettype = HW_RESET_UNKNOWN_FAMILY;

    robo->id = ss;
    chipc_spi_init(robo);
#ifndef ROBODVT
    chipc_spi_set_freq(robo, ss, SPI_FREQ_DEFAULT);
#else
#if 0
    /* For 8051 slave mode */
    chipc_spi_set_freq(robo, ss, SPI_FREQ_8051);
#else
    /* For 8051 master mode */
    /* For Thunderbolt and Lotus and probably the chips after */
    chipc_spi_set_freq(robo, ss, SPI_FREQ_20MHZ);
#endif
#endif

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
    /* Disable SPI IO select */
    chipc_spi_io_select(robo, robo->id, 0);

    /* Free private state */
    MFREE(sii->osh, robo, sizeof(robo_info_t));
}

void 
robo_switch_bus(void *rinfo,uint8 bustype)
{
    robo_info_t *robo = (robo_info_t *)rinfo;

    if (bustype == ROBO_SPI_BUS) {
        EB_TRACE(("Switch to SPI mode\n"));  
        setSPI(1);
    } else if (bustype == ROBO_MDCMDIO_BUS) {
        EB_TRACE(("Switch to Pseudo Phy mode\n"));  
        setSPI(0);
    } else {
        EB_TRACE(("Switch to EB Bus mode\n"));  
    }
    robo->bustype = bustype;
}
/* Enable serial access to the chip */
static void
robo_enable(robo_info_t *robo)
{
    void *regs;

    /* Save current core index */
#if defined(linux)    
    robo->coreidx = ai_soc_coreidx(robo->sbh);
    
    /* Switch to SPI core for faster access */
    regs = ai_soc_setcoreidx(robo->sbh, SI_CC_IDX);
#else
    robo->coreidx = si_coreidx(robo->sbh);
    
    /* Switch to SPI core for faster access */
    regs = si_setcoreidx(robo->sbh, SI_CC_IDX);
#endif
    ASSERT(regs);
}

/* Disable serial access to the chip */
static void
robo_disable(robo_info_t *robo)
{
    /* Switch back to original core */
#if defined(linux)      
    ai_soc_setcoreidx(robo->sbh, robo->coreidx);
#else
    si_setcoreidx(robo->sbh, robo->coreidx);
#endif
}

/* Write a byte stream to the chip */
static void
robo_write(robo_info_t *robo, uint8 cid, int reg, uint8 * buf, int len)
{
    uint8 data;

    chipc_spi_enable(robo, 0);

    /* Select normal write command */
    data = 0x61 | ((cid & 0x7) << 1);
    chipc_spi_write(robo, &data, 1);

   /* Select the register from the chip to write */
    data = reg;
    chipc_spi_write(robo, &data, 1);

    chipc_spi_write(robo, buf, len);

    chipc_spi_disable(robo, 0);
}

/* Read a byte stream from the chip */
static void
robo_read(robo_info_t *robo, uint8 cid, int reg, uint8 * buf, int len)
{
    uint8 data;

    chipc_spi_enable(robo, 0);

    /* Select normal read command */
    data = 0x60 | ((cid & 0x7) << 1);
    chipc_spi_write(robo, &data, 1);

    /* Select the register from the chip to read */
    data = reg;
    chipc_spi_write(robo, &data, 1);

    chipc_spi_read(robo, buf, len, 0);

    chipc_spi_disable(robo, 0);
}

#if (defined(VXWORKS))
/* Write a byte stream to the chip */
void
robo_aps_spi_write(void *rinfo, uint8 * buf, int len)
{
    robo_info_t *robo = (robo_info_t *)(rinfo ? rinfo : robo_sbh);

    if (robo->bustype != ROBO_SPI_BUS) {
        SPI_MSG(("robo_spi_write: NOT SPI BUS!!!\n"));
        return;
    }    	
    chipc_spi_enable(robo, 0);

    chipc_spi_write(robo, buf, len);

    chipc_spi_disable(robo, 0);
}

/* Read a byte stream from the chip */
void
robo_aps_spi_read(void *rinfo, uint8 * buf, int len)
{
    robo_info_t *robo = (robo_info_t *)(rinfo ? rinfo : robo_sbh);

    if (robo->bustype != ROBO_SPI_BUS) {
        SPI_MSG(("robo_spi_read: NOT SPI BUS!!!\n"));
        return;
    }
    chipc_spi_enable(robo, 0);

    chipc_spi_read(robo, buf, len, 0);

    chipc_spi_disable(robo, 0);
}
#endif /* (defined(VXWORKS) */

/* Function : chipc_spi_mode_ctrl
 *  - Allow user to set SPI device mode control(CPOL, CPHA, Endian and Frequency of SCK).
 * Return : 
 * Note :
 *     flags = SPI_MODE_CTRL_MODE
 *                SPI_MODE_CTRL_ACKEN
 *                SPI_MODE_CTRL_ENDIAN (for FIFO, spi_fifo_io register)
 *                SPI_MODE_CTRL_CLOCK
 *                SPI_MODE_CTRL_LSBEN (for each byte)
 */
static int
chipc_spi_mode_ctrl(robo_info_t *robo, uint32 flags, uint32 value)
{    
    uint32 mask = 0, val = 0, reg_val = 0;

    ASSERT(robo->sbh);

    switch(flags) {
        case SPI_MODE_CTRL_MODE :
            if (!CC_SPI_MODE_IS_VALID(value)){
                ET_ERROR(("%s: flags = 0x%x, Invalid SPI device MODE = 0x%x\n", 
                    FUNCTION_NAME(), flags, value));
                return SPI_ERR_PARAM;
            }
            mask = CC_SPIMCTRL_MODE_MASK;

            switch(value) {
                case CC_SPI_MODE_CPOL_0_CPHA_0 :
                    val = CC_SPIMCTRL_MODE_0;
                    break;
                case CC_SPI_MODE_CPOL_0_CPHA_1 :
                    val = CC_SPIMCTRL_MODE_1;
                    break;
                case CC_SPI_MODE_CPOL_1_CPHA_0 :
                    val = CC_SPIMCTRL_MODE_2;
                    break;
                case CC_SPI_MODE_CPOL_1_CPHA_1 :
                    val = CC_SPIMCTRL_MODE_3;
                    break;
                default :
                    return SPI_ERR_PARAM;
            }
            
            break;
        case SPI_MODE_CTRL_ACKEN:
            mask = CC_SPIMCTRL_ACKEN_MASK;
            if (value) {
                val = CC_SPIMCTRL_ACKEN;
            } else {
                val = 0;
            }
            break;
        case SPI_MODE_CTRL_ENDIAN:
            mask = CC_SPIMCTRL_ENDIAN_MASK;
            if (value) {
                val = CC_SPIMCTRL_BE;
            } else {
                val = CC_SPIMCTRL_LE;
            }
            break;
        case SPI_MODE_CTRL_CLOCK:
            mask = CC_SPIMCTRL_CLK_MASK;
            val = value;
            break;
        case SPI_MODE_CTRL_LSBEN:
            mask = CC_SPIMCTRL_LSB_MASK;
            if (value) {
                val = CC_SPIMCTRL_LSB_FIRST;
            } else {
                val = CC_SPIMCTRL_MSB_FIRST;
            }
            break;
        default :
            return SPI_ERR_PARAM;
    }

    /* select the spi interface */
#if defined(linux)      
    reg_val = ai_soc_spi_modectrl(robo->sbh, mask, val);
#else
    reg_val = si_spi_modectrl(robo->sbh, mask, val);
#endif

    SPI_MSG(("%s: flags = 0x%x, val = 0x%x, reg_val = 0x%x\n", 
        FUNCTION_NAME(), flags, val, reg_val));

    return SPI_ERR_NONE;
}

/* Function : chipc_spi_io_select 
 *  - Enable Keystone's SPI port.
 * Return :
 * Note :
 */
static int 
chipc_spi_io_select(robo_info_t *robo, cc_spi_id_t id, int en) 
{
    uint32 reg_val;

    ASSERT(robo->sbh);
    if (!CC_SPI_ID_IS_VALID(id)){
        ET_ERROR(("%s: Invalid SPI device ID!\n", FUNCTION_NAME()));
        return SPI_ERR_PARAM;
    }
    
    /* select the spi interface */
#if defined(linux)      
    reg_val = ai_soc_spi_select(robo->sbh, (uint8)id, (en) ? TRUE : FALSE);
#else
    reg_val = si_spi_select(robo->sbh, (uint8)id, (en) ? TRUE : FALSE);
#endif

    SPI_MSG(("%s: %s SPI_%d is DONE, reg_val = 0x%x\n", 
            FUNCTION_NAME(), (en) ? "Enable" : "Disable", id, reg_val));
    return SPI_ERR_NONE;
}

/* Function : chipc_spi_io_intr 
 *  - Enable/disable Keystone's SPI interrupt mode.
 * Return :
 * Note :
 */
static int 
chipc_spi_io_intr(robo_info_t *robo, int en) 
{
    uint32 reg_val;

    ASSERT(robo->sbh);
    
    /* select the spi interface */
#if defined(linux)     
    reg_val = ai_soc_spi_ccint_enable(robo->sbh, (en) ? TRUE : FALSE);
#else
    reg_val = si_spi_ccint_enable(robo->sbh, (en) ? TRUE : FALSE);
#endif

    SPI_MSG(("%s: %s is DONE, reg_val = 0x%x\n", 
             FUNCTION_NAME(), (en) ? "Enable" : "Disable", reg_val));
    return SPI_ERR_NONE;
}

/* Function : chipc_spi_set_freq 
 *  - set Keystone's SPI device frequency
 * Return :
 * Note :
 *  1. SPI clock is Keystone backplane
 *  2. the unit of the speed is hz.
 *  3. spi_freq = sys_freq / (2^(N+1)),  N= clock divider parameter (from 1 ~ 15)
 *  4. System Clock will be retrieved from SI interface and a proper spi 
 *      clock value will be auto-generated and selected to match user's 
 *      request frequency.
 */
int 
chipc_spi_set_freq(void *rinfo, cc_spi_id_t id, uint32 speed_hz) 
{
    robo_info_t *robo = (robo_info_t *)rinfo;
    uint  sys_freq, spi_freq;
    int  n;
    uint32  value;

    ASSERT(robo->sbh);

    if (!CC_SPI_ID_IS_VALID(id)){
        ET_ERROR(("%s: Invalid SPI Device ID!\n", FUNCTION_NAME()));
        return SPI_ERR_PARAM;
    }

    /* -------- set spi clock --------- */
    /* get Fsys */
#if defined(linux)     
    sys_freq = ai_soc_clock(robo->sbh);
#else
    sys_freq = si_clock(robo->sbh);
#endif

    SPI_MSG(("%s: sys_clock = %d, speed_hz = %d\n", FUNCTION_NAME(), sys_freq, speed_hz));

    /* retrive the N value */
    /* spi clock counting formula  : spi_freq = sys_freq / (2^(N+1)) */
    for (n = 0; n < SPI_CCD_MAX; n++){
        spi_freq = sys_freq / (2 << n);

        if (spi_freq <= speed_hz) {
            break;
        }
    }

    SPI_MSG(("%s: spi_freq = %d, clock divider parameters N = 0x%x\n", 
        FUNCTION_NAME(), spi_freq, n));

    /* Set the SPI clock (clock divider parameters n : [7:4] in SPI Moder Control register) */
    value = n << 4;
    if (chipc_spi_mode_ctrl(robo, SPI_MODE_CTRL_CLOCK, value)) {
        ET_ERROR(("%s: Failed to set device clock frequence at SPI_%d!\n", FUNCTION_NAME(), id));
        return SPI_ERR_PARAM;
    }

    return SPI_ERR_NONE;
}

/* Function : chipc_spi_fifo_rw 
 *  - Read/write data via FIFO for Keystone's SPI device.
 * Return :
 * Note :
 *     Read data from FIFO : if mask = val = 0
 *     Write data to FIFO : if (mask | val)  != 0
 */
static uint32
chipc_spi_fifo_rw(robo_info_t *robo, uint32 mask, uint32 val) 
{
    uint32 reg_val;

    ASSERT(robo->sbh);

#if defined(linux) 
    reg_val = ai_soc_spi_fifo(robo->sbh, mask, val);
#else
    reg_val = si_spi_fifo(robo->sbh, mask, val);
#endif

    SPI_MSG(("%s: %s fifo reg_val = 0x%x\n", FUNCTION_NAME(), 
        (!mask && !val) ? "Read" : "Write", reg_val));
            
    return reg_val;
}

/* Function : chipc_spi_write_fifo 
 *  - Write data to Keystone's SPI device via FIFO.
 * Return :
 * Note :
 */
static int
chipc_spi_write_fifo(robo_info_t *robo, uint8 *data, int len) 
{
    int idx, m;
    uint32 value;

    ASSERT(robo->sbh);
    /* len < 32 bytes */
    if (len > SPI_FIFO_MAX_SIZE) {
        ET_ERROR(("%s: Write failed : can not exceed 32 bytes!\n", FUNCTION_NAME()));
        return SPI_ERR_PARAM;
    }

    /* Write command/data to Keystone's SPI FIFO */
#if defined(BE_HOST)    
    value = 0;
    for (idx = 0 ; idx < len ; idx++) {
        m = idx % 4;
        value |= (*data << ((3-m)*8));
        data++;
        if ((m == 3) || (idx == (len - 1))) {
            chipc_spi_fifo_rw(robo, CC_SPIFIFOIO_MASK, value);
            value = 0;
        }
    }
#else
    value = 0;
    for (idx = 0 ; idx < len ; idx++) {
        m = idx % 4;
        value |= (*data << (m*8));
        data++;
        if ((m == 3) || (idx == (len - 1))) {
            chipc_spi_fifo_rw(robo, CC_SPIFIFOIO_MASK, value);
            value = 0;
        }
    }
#endif


    return SPI_ERR_NONE;
}

 /* Function : chipc_spi_status 
 *     - Get spi device status (offset 0x28c in chipcommon).
 * Return :
 *     - Return the current SPI status.
 * Note :
 */
static uint32
chipc_spi_status(robo_info_t *robo) 
{
    ASSERT(robo->sbh);

#if defined(linux) 
    return ai_soc_spi_status(robo->sbh);
#else
    return si_spi_status(robo->sbh);
#endif
}

 /* Function : chipc_spi_intr_clear 
 *     - Clear interrupt flag after transcation done.
 * Return :
 * Note :
 */
static int
chipc_spi_intr_clear(robo_info_t *robo) 
{
    uint32 reg_val;

    ASSERT(robo->sbh);

#if defined(linux) 
    reg_val = ai_soc_spi_intr_clear(robo->sbh);
#else
    reg_val = si_spi_intr_clear(robo->sbh);
#endif

    SPI_MSG(("%s: DONE, reg_val = 0x%x\n", FUNCTION_NAME(), reg_val));
    return SPI_ERR_NONE;

}

/* Function : _chipc_spi_wait_for_iflg_set 
 *  - Wait for the spi interrupt flag is up.
 * Return :
 * -  SPI_ERR_NONE or SPI_ERR_TIMEOUT
 * Note :
 */
static int  
_chipc_spi_wait_for_iflg_set(robo_info_t *robo) 
{
    uint32 spireg_val = 0;
    int retry = SPI_INTFLAG_TIMEOUT;

    ASSERT(robo->sbh);

    /* check if the spi IFLG is set */
    while((retry--) > 0) {

        spireg_val = chipc_spi_status(robo);

        if (spireg_val & CC_SPISTS_INTFLAG) {
            return SPI_ERR_NONE;
        }
        OSL_DELAY(10);
    }
    SPI_MSG(("%s: %d, spireg_val=0x%x, retry=%d\n",
            FUNCTION_NAME(), __LINE__, spireg_val, retry));
    
    return (retry > 0) ? SPI_ERR_NONE : SPI_ERR_TIMEOUT;
}

/* Function : chipc_spi_wait 
 *  - Wait for the spi interrupt flag.
 * Return :
 * Note :
 */
static int  
chipc_spi_wait(robo_info_t *robo) 
{
    int rv = SPI_ERR_NONE;
#ifdef SPIDBG 
    uint32 spireg_val = 0;
#endif

    ASSERT(robo->sbh); 

    rv = _chipc_spi_wait_for_iflg_set(robo);

#ifdef SPIDBG    
    /* Get SPI status for Debug */
    spireg_val = chipc_spi_status(robo);
    
    SPI_MSG(("%s: Current SPI status=0x%x\n", FUNCTION_NAME(), spireg_val));
#endif

    return rv;
    
}

/* Function : chipc_spi_config 
 *  - Start to do Read/write transaction on FIFO to Keystone's SPI device.
 * Return :
 * Note :
 */
static int
chipc_spi_config(robo_info_t *robo, cc_spi_id_t id, uint rdc, uint wdc, uint wcc)
{
    int rv = SPI_ERR_NONE;
    uint32 reg_val = 0;

    if (!CC_SPI_ID_IS_VALID(id)){
        ET_ERROR(("%s: Invalid SPI device ID!\n", FUNCTION_NAME()));
        return SPI_ERR_PARAM;
    }

    /* (RdDatCnt/WrDatCnt/WrCmdCnt) <= 32 bytes */
    if ((rdc > SPI_FIFO_MAX_SIZE) || 
        (wdc > SPI_FIFO_MAX_SIZE) || 
        (wcc > SPI_FIFO_MAX_SIZE)) {
        ET_ERROR(("%s: Config failed : can not exceed 32 bytes!\n", FUNCTION_NAME()));
        return SPI_ERR_PARAM;
    }

    /* (WrCmdCnt + WrDatCnt) <= 32 bytes */
    if ((wcc + wdc) > SPI_FIFO_MAX_SIZE) {
        ET_ERROR(("%s: Config failed : (WrCmdCnt+WrDatCnt) can not exceed 32 bytes!\n",
            FUNCTION_NAME()));
        return SPI_ERR_PARAM;
    }
    
    reg_val = (V_SPICFG_SS(id) |
                 V_SPICFG_RDC(rdc) |
                 V_SPICFG_WDC(wdc) |
                 V_SPICFG_WCC(wcc) |
                 V_SPICFG_START);

    SPI_MSG(("%s: reg_val = 0x%x\n", FUNCTION_NAME(), reg_val));

#if defined(linux) 
    ai_soc_spi_config(robo->sbh, CC_SPICFG_MASK, reg_val);
#else
    si_spi_config(robo->sbh, CC_SPICFG_MASK, reg_val);
#endif

    /* Polling SPI status until SPI Interrupt flag is set after done */
    if (chipc_spi_wait(robo)){
        /* timeout occurred */
        ET_ERROR(("%s: TIMEOUT after START at SPI\n", FUNCTION_NAME()));
        return SPI_ERR_TIMEOUT;
    }

    /* Write (0/1) to clear Interrupt flag after done */
    rv = chipc_spi_intr_clear(robo);

    return rv;
}

/* --------- SPI High level driver --------- 
 * The drivers in this level also know as general SPI driver. 
 *  - Request the SPI read/write on SPI device through SPI driver.
 * Function : chipc_spi_attach 
 * Function : chipc_spi_init 
 * Function : chipc_spi_enable 
 * Function : chipc_spi_disable 
 * Function : chipc_spi_read
 * Function : chipc_spi_write
 */

/* Function : chipc_spi_init
 *  - Init Keystone's SPI device SPI0/SPI1/SPI2.
 * Return :
 * Note : 
 *     Enable the serial IO select register (offset 0x2f4 in chipcommon) for SPI devices.
 *     Set the SPI mode control register (offset 0x280 in chipcommon) for SPI.
 *     
 */
static int
chipc_spi_init(robo_info_t *robo)
{
    robo_info_t *s = robo;
    int rv = SPI_ERR_NONE;
    uint32 value = 0;

    bzero(s->buf, sizeof(s->buf));
    s->buf_index = 0;
    s->states = SPI_STATES_DISABLE;

    /* Disable the SPI serial IO interrupt by default */
    if (chipc_spi_io_intr(robo, FALSE)) {
        ET_ERROR(("%s: Failed to disable SPI IO interrupt!\n", FUNCTION_NAME()));
        return SPI_ERR_INTERNAL;
    }

    /* Select the SPI interface for SPI device id */
    if (chipc_spi_io_select(robo, s->id, TRUE)) {
        ET_ERROR(("%s: Failed on enabling SPI_%d!\n", FUNCTION_NAME(), s->id));
        return SPI_ERR_INTERNAL;
    }

    /* Set the SPI mode */
    value = CC_SPI_MODE_CPOL_1_CPHA_1;
    if (chipc_spi_mode_ctrl(robo, SPI_MODE_CTRL_MODE, value)) {
        ET_ERROR(("%s: Failed to set device mode at SPI_%d!\n", FUNCTION_NAME(), s->id));
        return SPI_ERR_PARAM;
    }
#if defined(BE_HOST)
    value = CC_SPIMCTRL_BE;
#else
    value = CC_SPIMCTRL_LE;
#endif



    /* Set the SPI endian */
    if (chipc_spi_mode_ctrl(robo, SPI_MODE_CTRL_ENDIAN, value)) {
        ET_ERROR(("%s: Failed to set device endian at SPI_%d!\n", FUNCTION_NAME(), s->id));
        return SPI_ERR_PARAM;
    }

    /* Initialize SPI's interrupt flag */
    if (chipc_spi_status(robo) & CC_SPISTS_INTFLAG) {
        /* Write (0/1) to clear Interrupt flag for initialization */
        rv = chipc_spi_intr_clear(robo);
    }

    return rv;
}

/* Function : chipc_spi_enable
 *  - Enable the SPI device :
 *       initialize the sw_info : get ready to do read/write transaction on FIFO.
 * Return :
 * Note : 
 */
static int
chipc_spi_enable(robo_info_t *robo, uint8 slave)
{
    robo_info_t *s = robo;

    bzero(s->buf, sizeof(s->buf));
    s->buf_index = 0;
    s->states = SPI_STATES_ENABLE;

    return SPI_ERR_NONE;
}

/* Function : chipc_spi_disable
 *  - Disable the SPI device : 
 *       finished read/write transaction on FIFO (set states = SPI_STATES_DISABLE).
 * Return :
 * Note : 
 *     For SPI write operation (while s->states == SPI_STATES_WRITE), 
 *     we start to do really write transaction on FIFO 
 *     when we call function chipc_spi_disable. 
 *
 *     For SPI read operation, we do really read transaction on FIFO 
 *     when we call function chipc_spi_read.
 */
static int
chipc_spi_disable(robo_info_t *robo, uint8 slave)
{
    robo_info_t *s = robo;

    /* 
     *  if s->states == SPI_STATES_WRITE, start to do write transaction on FIFO
     *  ReadDataCnt = 0, WriteDataCnt = s->buf_index, WriteCmdCnt = 0
     */
    if (s->states == SPI_STATES_WRITE) {
        if (chipc_spi_write_fifo(robo, s->buf, s->buf_index)) {
            ET_ERROR(("%s: Failed to write data to SPI FIFO at SPI_%d!\n", 
                FUNCTION_NAME(), s->id));
            return SPI_ERR_PARAM;
        }
        if (chipc_spi_config(robo, s->id, 0, s->buf_index, 0)) {
            ET_ERROR(("%s: Failed to start write transaction at SPI_%d!\n", 
                FUNCTION_NAME(), s->id));
            return SPI_ERR_PARAM;
        }
    }

    s->states = SPI_STATES_DISABLE;
    
    return SPI_ERR_NONE;
}

/* Function : chipc_spi_read
 *  - Read operation through Keystone's SPI.
 * Return :
 * Note : 
 *     For SPI read operation, we start to do really read transaction on FIFO 
 *     when we call function chipc_spi_read. 
 */
static int
chipc_spi_read(robo_info_t *robo, uint8 * buf, int len, uint8 data_out)
{
    robo_info_t *s = robo;
    int idx, m;
    uint32_t value;

    /* 
     * (Reading data length) <= 32 bytes
     * - The Maximum size of SPI's FIFO is 32 bytes(once time).
     */
    if (len > SPI_FIFO_MAX_SIZE) {
        ET_ERROR(("%s: Read failed : can not exceed 32 bytes!\n",FUNCTION_NAME()));
        return SPI_ERR_PARAM;
    }

    /* 
     *  if s->states == SPI_STATES_WRITE, start to do Read transaction on FIFO
     *  ReadDataCnt = len, WriteDataCnt = s->buf_index, WriteCmdCnt = 0
     */
    if (s->states == SPI_STATES_WRITE) {
        if (chipc_spi_write_fifo(robo, s->buf, s->buf_index)) {
            ET_ERROR(("%s: Failed to write data to SPI FIFO at SPI_%d!\n", 
                FUNCTION_NAME(), s->id));
            return SPI_ERR_PARAM;
        }
        if (chipc_spi_config(robo, s->id, (uint)len, s->buf_index, 0)) {
            ET_ERROR(("%s: Failed to start read transaction at SPI_%d!\n", 
                FUNCTION_NAME(), s->id));
            return SPI_ERR_PARAM;
        }
        s->states = SPI_STATES_READ;
    } else if (s->states == SPI_STATES_ENABLE) {
        if (chipc_spi_config(robo, s->id, (uint)len, s->buf_index, 0)) {
            ET_ERROR(("%s: Failed to start read transaction at SPI_%d!\n", 
                FUNCTION_NAME(), s->id));
            return SPI_ERR_PARAM;
        }
    	s->states = SPI_STATES_READ;
    } else {
        ET_ERROR(("%s: Failed to do SPI read operation!\n",FUNCTION_NAME()));
        return SPI_ERR_PARAM;
    }

    /* Get spi status */
    value = chipc_spi_status(robo);

    /* Check FIFO is not empty for reading */
    if (value & CC_SPISTS_FIFOE) {
        ET_ERROR(("%s: Read failed at SPI: FIFO is empty for reading!\n", FUNCTION_NAME()));
        return SPI_ERR_INTERNAL;
    }

    /* Read data from FIFO IO register until FIFO is empty(read 4 bytes each time) */
#if defined(BE_HOST)
    value = 0;
    for (idx = 0 ; idx < len ; idx++) {
        m = idx % 4;
        if (m == 0) {
            value = chipc_spi_fifo_rw(robo, 0, 0);
        }
        *buf++ = (uint8)(value >> ((3-m)*8));
    }
#else
    value = 0;
    for (idx = 0 ; idx < len ; idx++) {
        m = idx % 4;
        if (m == 0) {
            value = chipc_spi_fifo_rw(robo, 0, 0);
        }
        *buf++ = (uint8)(value >> (m*8));
    }
#endif

    return SPI_ERR_NONE;
}

/* Function : chipc_spi_write 
 *  - Write operation through Keystone's SPI.
 * Return :
 * Note : 
 *     Write the SPI device command(or data) and keep in software buffer first.
 *     For SPI write operation (while s->states == SPI_STATES_WRITE), 
 *     we start to do really write transaction on FIFO 
 *     when we call function chipc_spi_disable. 
 */
static int
chipc_spi_write(robo_info_t *robo, uint8 *buf, int len)
{
    robo_info_t *s = robo;
    int i = 0;

    if (s->states != SPI_STATES_ENABLE && s->states != SPI_STATES_WRITE) {
        ET_ERROR(("%s: Failed to do SPI read/write operation!\n", FUNCTION_NAME()));
        return SPI_ERR_PARAM;
    }

    /* The Maximum size of SPI's FIFO is 32 bytes */
    if ((s->buf_index + len) > SPI_FIFO_MAX_SIZE) {
        ET_ERROR(("%s: Failed to write data to FIFO : the size of FIFO > 32 bytes!\n", 
            FUNCTION_NAME()));
        return SPI_ERR_PARAM;
    }

    /* Write SPI command or data and keep in software first */
    while (len) {
        s->buf[s->buf_index++] = buf[i++];
        len--;
    }

    s->states = SPI_STATES_WRITE;

    return SPI_ERR_NONE;
}

/* poll for RACK */
static int
robo_poll_for_RACK(robo_info_t *robo, uint8 cid)
{
    uint i, timeout;
    uint8 byte;

    /* Timeout after 100 tries without RACK */
    for (i = 0, timeout = ROBO_DEFAULT_SPI_TIMEOUT; timeout;) {
        robo_read(robo, cid, 0xfe, &byte, 1);
         /*  In normal read mode 
               *  check bit 5 are high 
               */
        if (byte & 0x20)
            break;
        else
        {
            timeout--;
        }
    }

    if (timeout == 0) {
        ET_ERROR(("robo_read: timeout"));
        return -1;
    }
    return 0;

}

/* poll for SPIF low */
static int
robo_poll_for_SPIF(robo_info_t *robo, uint8 cid)
{
    uint i, timeout;
    uint8 byte;
    uint data;

    /* Timeout after 100 tries without SPIF low */
    for (i = 0, timeout = ROBO_DEFAULT_SPI_TIMEOUT; timeout;) {
        robo_read(robo, cid, 0xfe, &byte, 1);
        /* SPIF is bit 7 of SPI_STS */
            /* check SPIF = 0 ? */
        if (!(byte & 0x80))
            break;
        else
        {
            timeout--;
            data = (uint)byte;
        }
    }

    if (timeout == 0) {
        ET_ERROR(("robo_read: timeout"));
        return -1;
    }
    return 0;
}

/* Select new chip and page */
static void
robo_select(robo_info_t *robo, uint8 cid, uint8 page)
{
    /* Chip and page already selected */
    if (robo->cid == (int) cid && robo->page == (int) page)
        return;
    robo->cid = (int) cid;
    robo->page = (int) page;

    robo_write(robo, cid, 0xff, &page, 1);
}

void 
robo_select_device(void *rinfo, uint16 phyidh, uint16 phyidl)
{
    robo_info_t *robo = (robo_info_t *)rinfo;
    if (((phyidh == BCM53115_PHYID_HIGH) && \
        ((phyidl & 0xFFF0) == BCM53115_PHYID_LOW)) || \
        ((phyidh == BCM53118_PHYID_HIGH) && \
        ((phyidl & 0xFFF0) == BCM53118_PHYID_LOW)) || \
        ((phyidh == BCM53101_PHYID_HIGH) && \
        ((phyidl & 0xFFF0) == BCM53101_PHYID_LOW)) || \
        ((phyidh == BCM53125_PHYID_HIGH) && \
        ((phyidl & 0xFFF0) == BCM53125_PHYID_LOW)) || \
        ((phyidh == BCM53128_PHYID_HIGH) && \
        ((phyidl & 0xFFF0) == BCM53128_PHYID_LOW)) || \
        ((phyidh == BCM53134_PHYID_HIGH) && \
        ((phyidl & 0xFFF0) == BCM53134_PHYID_LOW)) || \
        ((phyidh == BCM89500_PHYID_HIGH) && \
        ((phyidl & 0xFFF0) == BCM89500_PHYID_LOW))) {
        robo->phytype = DIRECT_PHY_ACCESS_GE_FAMILY;
        robo->resettype = HW_RESET_GE_FAMILY;
    } else if (((phyidh == BCM53242_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53242_PHYID_LOW)) || \
                ((phyidh == BCM53262_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53262_PHYID_LOW)) || \
                ((phyidh == BCM53280_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53280_PHYID_LOW)) ||\
                ((phyidh == BCM53600_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53600_PHYID_LOW))) {
        robo->phytype = DIRECT_PHY_ACCESS_FE_FAMILY;
        robo->resettype = HW_RESET_FE_FAMILY;
    } else if (((phyidh == BCM5389_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM5389_PHYID_LOW)) || \
                ((phyidh == BCM5396_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM5396_PHYID_LOW))) {
        robo->phytype = DIRECT_PHY_ACCESS_GE_FAMILY;
        robo->resettype = HW_RESET_UNKNOWN_FAMILY;
    } else {
        robo->phytype = DIRECT_PHY_ACCESS_UNKNOWN;
        robo->resettype = HW_RESET_UNKNOWN_FAMILY;
    }
}

/* For big endian operation */

static struct mdioCtrlReg1{
#if defined(BE_HOST)
    uint8     page;
    uint8     writeAll:1;
    uint8     reserved:4;
    uint8     chipId:2;
    uint8     mdioEnable:1;
#else
    uint8     mdioEnable:1;
    uint8     chipId:2;
    uint8     reserved:4;
    uint8     writeAll:1;
    uint8     page;
#endif
}  mdioCtrl1;

static struct mdioCtrlReg2 {
#if defined(BE_HOST)
    uint8    regOffset;
    uint8    reserved:6;
    uint8    op:2;   
#else
    uint8    op:2;   
    uint8    reserved:6;
    uint8    regOffset;
#endif
} mdioCtrl2;

int     gSPIEnable = TRUE;
int     mdioDelay=100;
static uint16  *pMdioCtrl1 = (uint16 *) &mdioCtrl1;
static uint16  *pMdioCtrl2 = (uint16 *) &mdioCtrl2;
static uint16  mMdioBuf[4];
static  char mMdioUsed = FALSE;

#define PHY_ID_PAGE 0x10

#define MII_PAGE_1Xmin 0x10
#define MII_PAGE_1Xmax 0x1f
#define MII_PAGE_1Xoffset MII_PAGE_1Xmin
#define MII_PAGE_2Xmin 0x20
#define MII_PAGE_2Xmax 0x2f
#define MII_PAGE_2Xoffset MII_PAGE_1Xoffset
#define MII_PAGE_8Xmin 0x80
#define MII_PAGE_8Xmax 0x8f
#define MII_PAGE_8Xoffset 0x70
#define MII_PAGE_AXmin 0xa0
#define MII_PAGE_AXmax 0xb7
#define MII_PAGE_AXoffset MII_PAGE_AXmin
#define MII_PAGE_DXmin 0xd8
#define MII_PAGE_DXmax 0xdc
#define MII_PAGE_DXoffset 0xc0

#define GE_FAMILY_RESET_PAGE 0x00
#define GE_FAMILY_RESET_ADDR 0x79
#define GE_FAMILY_RESET_VALUE 0x90
#define FE_FAMILY_RESET_PAGE 0x03
#define FE_FAMILY_RESET_ADDR 0x7c
#define FE_FAMILY_RESET_VALUE 0x01

#define    PSEUDO_PHY       0x1e
#define    MDIO_CTRL1_ADDR  16
#define    MDIO_CTRL2_ADDR  17
#define    MDIO_DATAREG_BASE    24
#define    MDIO_OP_RD       2
#define    MDIO_OP_WR       1

static uint16
chipphy_rd(si_t *sih, uint phyaddr, uint reg);
static void
chipphy_wr(si_t *sih, uint phyaddr, uint reg, uint16 v);

/*-------------------------------------------------------------
check if this access is at the same page as the previous access
if it is in different page, then need to set the mdio_ctrl1 
---------------------------------------------------------------*/
#define    CHKSET_CHIP_PAGE(cid, page) \
        if ( !mMdioUsed ||mdioCtrl1.page != page || mdioCtrl1.chipId !=cid ) {\
            mMdioUsed = TRUE;      \
            mdioCtrl1.page = page; \
            mdioCtrl1.chipId = cid; \
            chipphy_wr(((robo_info_t *)robo_sbh)->sbh, PSEUDO_PHY, MDIO_CTRL1_ADDR, *pMdioCtrl1); \
        }

static uint16
chipphy_rd(si_t *sih, uint phyaddr, uint reg)
{
    uint32 tmp;
    uint origidx;
    gmac_commonregs_t *regs;
    si_info_t *sii;
    osl_t       *osh;
    
    sii = SI_INFO(sih);
    osh = sii->osh;

    ASSERT(phyaddr < MAXEPHY);
    ASSERT(reg < MAXPHYREG);

    /* Remember original core before switch to gmac common */
#if defined(linux)    
    origidx = ai_soc_coreidx(sih);
    regs = ai_soc_setcore(sih, GMAC_COM_CORE_ID, 0);
#else
    origidx = si_coreidx(sih);
    regs = si_setcore(sih, GMAC_COM_CORE_ID, 0);
#endif

    ASSERT(regs != NULL);

    /* set phyaccess for read/write */
    W_REG(osh, &regs->phyaccess,
          (PHYACCESS_TRIGGER_MASK | (phyaddr << PHYACCESS_CPU_PHY_ADDR_SHIFT) | 
          (reg << PHYACCESS_CPU_REG_ADDR_SHIFT)));

    /* wait for it to complete */
    SPINWAIT((R_REG(osh, &regs->phyaccess) & PHYACCESS_TRIGGER_MASK), 1000);
    tmp = R_REG(osh, &regs->phyaccess);
    if (tmp & PHYACCESS_TRIGGER_MASK) {
        ET_ERROR(("chipphy_rd: did not complete\n"));
        tmp = 0xffff;
    }

    /* Return to original core */
#if defined(linux)    
    ai_soc_setcoreidx(sih, origidx);
#else
    si_setcoreidx(sih, origidx);
#endif

    return (tmp & PHYACCESS_ACC_DATA_MASK);
}

static void
chipphy_wr(si_t *sih, uint phyaddr, uint reg, uint16 v)
{
    uint origidx;
    gmac_commonregs_t *regs;
    si_info_t *sii;
    osl_t       *osh;
    
    sii = SI_INFO(sih);
    osh = sii->osh;

    ASSERT(phyaddr < MAXEPHY);
    ASSERT(reg < MAXPHYREG);

    /* Remember original core before switch to gmac common */
#if defined(linux)    
    origidx = ai_soc_coreidx(sih);
    regs = ai_soc_setcore(sih, GMAC_COM_CORE_ID, 0);
#else
    origidx = si_coreidx(sih);
    regs = si_setcore(sih, GMAC_COM_CORE_ID, 0);
#endif

    ASSERT(regs != NULL);

    /* set phyaccess for read/write */
    W_REG(osh, &regs->phyaccess,
          (PHYACCESS_TRIGGER_MASK | PHYACCESS_WR_CMD_MASK | 
          (phyaddr << PHYACCESS_CPU_PHY_ADDR_SHIFT) | 
          (reg << PHYACCESS_CPU_REG_ADDR_SHIFT) | v));

    /* wait for it to complete */
    SPINWAIT((R_REG(osh, &regs->phyaccess) & PHYACCESS_TRIGGER_MASK), 1000);
    if (R_REG(osh, &regs->phyaccess) & PHYACCESS_TRIGGER_MASK) {
        ET_ERROR(("chipphy_wr: did not complete\n"));
    }

    /* Return to original core */
#if defined(linux)    
    ai_soc_setcoreidx(sih, origidx);
#else
    si_setcoreidx(sih, origidx);
#endif

}

void robo_mdio_reset(void) 
{
    robo_info_t *robo = (robo_info_t *)robo_sbh;

    mMdioUsed = FALSE;
    chipphy_wr(robo->sbh, PSEUDO_PHY, MDIO_CTRL1_ADDR, 0);
}

void setSPI(int fEnable)
{
    robo_info_t *robo = (robo_info_t *)robo_sbh;

    gSPIEnable = fEnable;
    mdioCtrl1.mdioEnable = 1-fEnable;
    mMdioUsed = FALSE;
    
    chipphy_wr(robo->sbh, PSEUDO_PHY, MDIO_CTRL1_ADDR, 1-fEnable);
}

static int
robo_phy_direct_set(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len)
{
    robo_info_t *robo = (robo_info_t *)(rinfo ? rinfo : robo_sbh);
    int rv = FALSE;
	uint8 pid = 0;

    if (len!=2) {
        ET_ERROR(("All MII registers are 2 bytes long\n"));
        return FALSE;
    }

    if (robo->phytype == DIRECT_PHY_ACCESS_GE_FAMILY) {
    if (page>= MII_PAGE_1Xmin && page <= MII_PAGE_1Xmax) {

        pid = (page - MII_PAGE_1Xoffset); 
        chipphy_wr(robo->sbh, pid, addr>>1, mMdioBuf[0]);

        rv = TRUE;
    }
    if (page>= MII_PAGE_8Xmin && page <= MII_PAGE_8Xmax) {

        pid = (page - MII_PAGE_8Xoffset); 
        chipphy_wr(robo->sbh, pid, addr>>1, mMdioBuf[0]);

        rv = TRUE;
    }
    } else if (robo->phytype == DIRECT_PHY_ACCESS_FE_FAMILY) {
    if (page>= MII_PAGE_AXmin && page <= MII_PAGE_AXmax) {

        pid = (page - MII_PAGE_AXoffset); 
        chipphy_wr(robo->sbh, pid, addr>>1, mMdioBuf[0]);

        rv = TRUE;
    }
    if (page>= MII_PAGE_DXmin && page <= MII_PAGE_DXmax) {

        pid = (page - MII_PAGE_DXoffset); 
        chipphy_wr(robo->sbh, pid, addr>>1, mMdioBuf[0]);

        rv = TRUE;
    }
    } else {
        rv = FALSE;
    }

    return rv;
}

static int
robo_phy_direct_get(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len)
{
    robo_info_t *robo = (robo_info_t *)(rinfo ? rinfo : robo_sbh);
    uint16 ret;
    int rv = FALSE;
	uint8 pid = 0;

    if (len!=2) {
    ET_ERROR(("All MII registers are 2 bytes long\n"));
    return FALSE;
    }

    if (robo->phytype == DIRECT_PHY_ACCESS_GE_FAMILY) {
    if (page>= MII_PAGE_1Xmin && page <= MII_PAGE_1Xmax) {

        pid = (page - MII_PAGE_1Xoffset); 
        ret = chipphy_rd(robo->sbh, pid, addr>>1);
        buf[0] = ret & 0x00ff;
        buf[1] = ret >> 8;    

        rv = TRUE;
    }
    if (page>= MII_PAGE_8Xmin && page <= MII_PAGE_8Xmax) {

        pid = (page - MII_PAGE_8Xoffset); 
        ret = chipphy_rd(robo->sbh, pid, addr>>1);
        buf[0] = ret & 0x00ff;
        buf[1] = ret >> 8;    

        rv = TRUE;
    }
    } else if (robo->phytype == DIRECT_PHY_ACCESS_FE_FAMILY) {
    if (page>= MII_PAGE_AXmin && page <= MII_PAGE_AXmax) {

        pid = (page - MII_PAGE_AXoffset); 
        ret = chipphy_rd(robo->sbh, pid, addr>>1);
        buf[0] = ret & 0x00ff;
        buf[1] = ret >> 8;    

        rv = TRUE;
    }
    if (page>= MII_PAGE_DXmin && page <= MII_PAGE_DXmax) {

        pid = (page - MII_PAGE_DXoffset); 
        ret = chipphy_rd(robo->sbh, pid, addr>>1);
        buf[0] = ret & 0x00ff;
        buf[1] = ret >> 8;    

        rv = TRUE;
    }
    } else {
        rv = FALSE;
    }

    return rv;
}

/* Write chip register */
void
robo_wreg(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len)
{
    robo_info_t *robo = (robo_info_t *)(rinfo ? rinfo : robo_sbh);

#ifdef _KERNEL_
    unsigned long flags;
    spin_lock_irqsave(&robo->lock, flags);
#endif

    if(robo->bustype == ROBO_SPI_BUS) {    
        robo_enable(robo);

        if (robo_poll_for_SPIF(robo, cid)) {
            /* timeout */
            robo_select(robo, cid, page);
#ifdef _KERNEL_
            spin_unlock_irqrestore(&robo->lock, flags);
#endif
            return;
        }

        /* Select chip and page */
        robo_select(robo, cid, page);

        robo_write(robo, cid, addr, buf, len);

        robo_disable(robo);
    } else if (robo->bustype == ROBO_MDCMDIO_BUS) {
        int i, nWord=(len+1)>>1;
        int process, phyrd_timeout_count;
        int hw_reset = 0;

        if (len > 8) {
            ET_ERROR(("length should be no greater than 8 bytes\n"));
#ifdef _KERNEL_
            spin_unlock_irqrestore(&robo->lock, flags);
#endif
            return;
        }

        memcpy((uint8 *)mMdioBuf, buf, len);
    
        /*odd number of bytes, move data from hibyte to lobyte for the last 
          byte */
        /* For BE, byte swap for the 16-bit MDIO access */
#if defined(BE_HOST)
        for (i=0; i<nWord; i++) {
            mMdioBuf[i] = BCMSWAP16(mMdioBuf[i]);
        }
#endif

        if (robo_phy_direct_set(rinfo, cid, page, addr, buf, len)) {
            /* phy direct access done */
#ifdef _KERNEL_
            spin_unlock_irqrestore(&robo->lock, flags);
#endif
            return;
        }

        for (i=0; i<nWord; i++ )  {
            chipphy_wr(robo->sbh, PSEUDO_PHY, MDIO_DATAREG_BASE+ i, mMdioBuf[i]);
        }

        CHKSET_CHIP_PAGE(cid, page);
        mdioCtrl2.op = MDIO_OP_WR;
        mdioCtrl2.regOffset = addr;
        chipphy_wr(robo->sbh, PSEUDO_PHY, MDIO_CTRL2_ADDR, *pMdioCtrl2);

        if (robo->resettype == HW_RESET_GE_FAMILY) {
            if ((page == GE_FAMILY_RESET_PAGE) && 
             (addr == GE_FAMILY_RESET_ADDR) && 
             ((mMdioBuf[0] & GE_FAMILY_RESET_VALUE) == GE_FAMILY_RESET_VALUE)) {
                hw_reset = 1;
            }
        }
        if (robo->resettype == HW_RESET_FE_FAMILY) {
            if ((page == FE_FAMILY_RESET_PAGE) && 
             (addr == FE_FAMILY_RESET_ADDR) && 
             ((mMdioBuf[0] & FE_FAMILY_RESET_VALUE) == FE_FAMILY_RESET_VALUE)) {
                hw_reset = 1;
            }
        }

        process = 1;
        phyrd_timeout_count = 0;
        
        while (process) {
            if (hw_reset) {
                /* 
                 * For HW reset operation, 
                 * waveform need to be re-enterered after HW reset configured 
                 * in oreder to complete the entire register write cycle.
                 */
                CHKSET_CHIP_PAGE(cid, page);
                for (i=0; i<nWord; i++ )  {
                    chipphy_wr(robo->sbh, PSEUDO_PHY, MDIO_DATAREG_BASE+ i, mMdioBuf[i]);
                }
            }

            *pMdioCtrl2 = chipphy_rd(robo->sbh, PSEUDO_PHY, MDIO_CTRL2_ADDR);
            if (mdioCtrl2.op == 0) {
                process = 0;
            }
            phyrd_timeout_count ++;
        
            if (phyrd_timeout_count > ROBO_DEFAULT_MDCMDIO_TIMEOUT) {
                return;
            }
        }
    } else {
        /* EB bus not support now */
    }
#ifdef _KERNEL_
    spin_unlock_irqrestore(&robo->lock, flags);
#endif

}

/* Read chip register */
void
robo_rreg(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len)
{
    robo_info_t *robo = (robo_info_t *)(rinfo ? rinfo : robo_sbh);

#ifdef _KERNEL_
    unsigned long flags;
    spin_lock_irqsave(&robo->lock, flags);
#endif

    if (robo->bustype == ROBO_SPI_BUS) {
        /* (page, addr) (0xFF, 0xFx) is reserved for the slave 8051 */
        if (page == SPI_PAGE) {
#if defined(VXWORKS) && defined(ROBODVT) /* For ROBO DVT test env on Vxworks only */
            /* (page,addr) (0xFF,0xFF) is reserved for ROBO HW reset */
            if (addr == SPI_ROBO_RESET) {
                /* Trigger the ROBO HW reset via GPIO11 on Keystone XMC */
                robo_enable(robo);
                /* Initialize gpio 0 as high */
                si_gpioout(robo->sbh, ROBO_HW_RESET_GPIO, ROBO_HW_RESET_GPIO, GPIO_DRV_PRIORITY);
                OSL_DELAY(100);
                /* enable gpio 0 */
                si_gpioouten(robo->sbh, ROBO_HW_RESET_GPIO, ROBO_HW_RESET_GPIO, GPIO_DRV_PRIORITY);
                /* Low active for the reset */
                si_gpioout(robo->sbh, ROBO_HW_RESET_GPIO, 0, GPIO_DRV_PRIORITY);
                OSL_DELAY(100);
                /* Release the reset */
                si_gpioout(robo->sbh, ROBO_HW_RESET_GPIO, ROBO_HW_RESET_GPIO, GPIO_DRV_PRIORITY);
                OSL_DELAY(100);
                /* disable gpio 0 */
                si_gpioouten(robo->sbh, ROBO_HW_RESET_GPIO, 0, GPIO_DRV_PRIORITY);
                robo_disable(robo);
            }
#endif
#ifdef _KERNEL_
            spin_unlock_irqrestore(&robo->lock, flags);
#endif
            return;
        }
        robo_enable(robo);  

        if (robo_poll_for_SPIF(robo,cid)) {
            /* timeout */
            robo_select(robo, cid, page);
#ifdef _KERNEL_
            spin_unlock_irqrestore(&robo->lock, flags);
#endif
            return;
        }
        /* Select chip and page */
        robo_select(robo, cid, page);

        /* Normal read */
        /* Discard first read */
        robo_read(robo, cid, addr, buf, 1);

        if (robo_poll_for_RACK(robo, cid)){
            /* timeout */
#ifdef _KERNEL_
            spin_unlock_irqrestore(&robo->lock, flags);
#endif
            return;
        }

        /* Read registers from dataport */
        robo_read(robo, cid, 0xf0, buf, len);

        robo_disable(robo);
    } else if (robo->bustype == ROBO_MDCMDIO_BUS) {
        int i, nWord=(len+1)>>1;   
        uint16 ret;
        int process, phyrd_timeout_count;

        if (len > 8) {
           ET_ERROR(("length should be no greater than 8 bytes\n"));
#ifdef _KERNEL_
           spin_unlock_irqrestore(&robo->lock, flags);
#endif
           return;
        }

        /* Only for switch chip unknown(first time read phyid at BDE) */
        if ((page == PHY_ID_PAGE) && (robo->phytype==DIRECT_PHY_ACCESS_UNKNOWN)) {
           if (len!=2) {
               ET_ERROR(("All MII registers are 2 bytes long\n"));
#ifdef _KERNEL_
               spin_unlock_irqrestore(&robo->lock, flags);
#endif
               return;
           }

           cid = cid<<3 | (page - PHY_ID_PAGE); 
           ret = chipphy_rd(robo->sbh, cid, addr>>1);
           buf[0] = ret & 0x00ff;
           buf[1] = ret >> 8;    
#ifdef _KERNEL_
           spin_unlock_irqrestore(&robo->lock, flags);
#endif
           return;
        }

        if (robo_phy_direct_get(rinfo, cid, page, addr, buf, len)) {
            /* phy direct access done */
#ifdef _KERNEL_
            spin_unlock_irqrestore(&robo->lock, flags);
#endif
            return;
        }

        /* if the access is to pages/chips other than the previous one
           set the mdio_ctrl1 register again  */
        CHKSET_CHIP_PAGE(cid, page); 

        mdioCtrl2.op = MDIO_OP_RD;
        mdioCtrl2.regOffset = addr;

        chipphy_wr(robo->sbh, PSEUDO_PHY, MDIO_CTRL2_ADDR, *pMdioCtrl2);

        process = 1;
        phyrd_timeout_count = 0;
        
        while (process) {
            *pMdioCtrl2 = chipphy_rd(robo->sbh, PSEUDO_PHY, MDIO_CTRL2_ADDR);
            if (mdioCtrl2.op == 0) {
                process = 0;
            }
            phyrd_timeout_count ++;
        
            if (phyrd_timeout_count > ROBO_DEFAULT_MDCMDIO_TIMEOUT) {
                return;
            }
        }

        for (i=0; i<nWord; i++ ) {
            mMdioBuf[i] = chipphy_rd(robo->sbh, PSEUDO_PHY, MDIO_DATAREG_BASE+ i);
        }
#if defined(BE_HOST)
        for (i=0; i<nWord; i++) {
            mMdioBuf[i] = BCMSWAP16(mMdioBuf[i]);
        }
#endif
        memcpy(buf, (uint8 *)mMdioBuf, len);
    } else {
        /* EB bus not support now */
    }

#ifdef _KERNEL_
    spin_unlock_irqrestore(&robo->lock, flags);
#endif

}

/* Enable reverse MII mode */
void
robo_rvmii(void *rinfo, uint8 cid)
{
    robo_info_t *robo = (robo_info_t *)rinfo;
    uint8 mii;

    /* MII port state override (page 0 register 14) */
    robo_rreg(robo, cid, ROBO_CTRL_PAGE,
          ROBO_PORT_OVERRIDE_CTRL, &mii, sizeof(mii));

    /* Bit 4 enables reverse MII mode */
    if(mii & ROBO_PORT_OVERRIDE_RVMII)
        return;

    mii |= ROBO_PORT_OVERRIDE_RVMII;

    robo_wreg(robo, cid, ROBO_CTRL_PAGE,
          ROBO_PORT_OVERRIDE_CTRL, &mii, sizeof(mii));
    /* Read back */
    robo_rreg(robo, cid, ROBO_CTRL_PAGE,
          ROBO_PORT_OVERRIDE_CTRL, &mii, sizeof(mii));

    if (!(mii & ROBO_PORT_OVERRIDE_RVMII)) {
        ET_ERROR(("robo_rvmii: error enabling mode"));
    }
}


