/*
 * $Id: pci.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Routines for accessing BCM56xx PCI memory mapped registers
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <sal/core/boot.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/error.h>
#include <soc/cmic.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif

/* The buffer used by soc_pci_off2name */
static char _soc_pci_off2name_buf[SOC_MAX_NUM_DEVICES][40];

/*
 * CMIC PCI Memory-Accessible registers.
 *
 * NOTE: Names must be kept in correct order to correspond with offsets.
 */

STATIC char *_soc_pci_reg_names[] = {
    "CMIC_SCHAN_CTRL",		/* Start at offset 0x50 */
    "CMIC_ARL_DMA_ADDR", "CMIC_ARL_DMA_CNT",
    "CMIC_SCHAN_ERR",
    "CMIC_COS_ENABLE_COS0", "CMIC_COS_ENABLE_COS1",
    "CMIC_COS_ENABLE_COS2", "CMIC_COS_ENABLE_COS3",
    "CMIC_COS_ENABLE_COS4", "CMIC_COS_ENABLE_COS5",
    "CMIC_COS_ENABLE_COS6", "CMIC_COS_ENABLE_COS7",
    "CMIC_ARL_MBUF00", "CMIC_ARL_MBUF01", "CMIC_ARL_MBUF02", "CMIC_ARL_MBUF03",
    0, 0, 0, 0,
    "CMIC_ARL_MBUF10", "CMIC_ARL_MBUF11", "CMIC_ARL_MBUF12", "CMIC_ARL_MBUF13",
    0, 0, 0, 0,
    "CMIC_ARL_MBUF20", "CMIC_ARL_MBUF21", "CMIC_ARL_MBUF22", "CMIC_ARL_MBUF23",
    0, 0, 0, 0,
    "CMIC_ARL_MBUF30", "CMIC_ARL_MBUF31", "CMIC_ARL_MBUF32", "CMIC_ARL_MBUF33",
    0, 0, 0, 0,
    "CMIC_DMA_CTRL", "CMIC_DMA_STAT", "CMIC_HOL_STAT", "CMIC_CONFIG",
    "CMIC_DMA_DESC0", "CMIC_DMA_DESC1", "CMIC_DMA_DESC2", "CMIC_DMA_DESC3",
    "CMIC_I2C_SLAVE_ADDR", "CMIC_I2C_DATA", "CMIC_I2C_CTRL", "CMIC_I2C_STAT",
    "CMIC_I2C_SLAVE_XADDR", "CMIC_I2C_GP0", "CMIC_I2C_GP1",
    "CMIC_I2C_RESET", "CMIC_LINK_STAT",
    "CMIC_IRQ_STAT", "CMIC_IRQ_MASK",
    "CMIC_MEM_FAIL",
    "CMIC_IGBP_WARN", "CMIC_IGBP_DISCARD",
    "CMIC_MIIM_PARAM", "CMIC_MIIM_READ_DATA",
    "CMIC_SCAN_PORTS",
    "CMIC_STAT_DMA_ADDR", "CMIC_STAT_DMA_SETUP",
    "CMIC_STAT_DMA_PORTS", "CMIC_STAT_DMA_CURRENT",
    "CMIC_ENDIAN_SELECT",
};

char *
soc_pci_off2name(int unit, uint32 offset)
{
    int			led = soc_feature(unit, soc_feature_led_proc);

    assert((offset & 3) == 0);

#ifdef BCM_CMICM_SUPPORT
    if(soc_feature(unit, soc_feature_cmicm)) {
        sal_strncpy(_soc_pci_off2name_buf[unit],
                    soc_cmicm_addr_name (offset), 39);
        _soc_pci_off2name_buf[unit][39] = 0;
        return _soc_pci_off2name_buf[unit];
    }
#endif /* CMICM Support */

    /* CMIC_SCHAN_MESSAGE begins at 0x800 on some chips */

    if (offset < 0x50) {
        sal_sprintf(_soc_pci_off2name_buf[unit], "CMIC_SCHAN_D%02d",
                    offset / 4);
    } else if ((int) offset >= CMIC_SCHAN_MESSAGE(unit, 0) &&
        (int)offset < CMIC_SCHAN_MESSAGE(unit, CMIC_SCHAN_WORDS(unit))) {
        sal_sprintf(_soc_pci_off2name_buf[unit], "CMIC_SCHAN_D%02d",
                    (offset - CMIC_SCHAN_MESSAGE(unit, 0)) / 4);
    } else if (led && offset == 0x1000) {
        sal_strncpy(_soc_pci_off2name_buf[unit], "CMIC_LED_CTRL",
                    sizeof(_soc_pci_off2name_buf[unit]));
    } else if (led && offset == 0x1004) {
        sal_strncpy(_soc_pci_off2name_buf[unit], "CMIC_LED_STATUS",
                    sizeof(_soc_pci_off2name_buf[unit]));
    } else if (led && offset >= 0x1800 && offset < 0x1c00) {
        sal_sprintf(_soc_pci_off2name_buf[unit], "CMIC_LED_PROG%02x",
                   (offset - 0x1800) / 4);
    } else if (led && offset >= 0x1c00 && offset < 0x2000) {
        sal_sprintf(_soc_pci_off2name_buf[unit], "CMIC_LED_DATA%02x",
                   (offset - 0x1c00) / 4);
    } else if ((offset - 0x50) < 4 * (uint32)COUNTOF(_soc_pci_reg_names) &&
        _soc_pci_reg_names[(offset - 0x50) / 4] != NULL) {
        sal_strncpy(_soc_pci_off2name_buf[unit],
                    _soc_pci_reg_names[(offset - 0x50) / 4], 39);
        _soc_pci_off2name_buf[unit][39] = 0;
    } else {
        sal_sprintf(_soc_pci_off2name_buf[unit], "CMIC_UNUSED_0x%04x", offset);
    }

    return _soc_pci_off2name_buf[unit];
}

/* If SOC_PCI_DEBUG not defined, then these functions are inlined in cmic.h */
#ifdef	SOC_PCI_DEBUG
/*
 * Get a CMIC register in PCI space using more "soc-like" semantics.
 * Input address is relative to the base of CMIC registers.
 */
int
soc_pci_getreg(int unit, uint32 addr, uint32 *datap)
{
	uint32 addr32 = addr;
#if defined(BCM_IPROC_SUPPORT)  && defined(IPROC_NO_ATL)
	addr32 += SOC_DRIVER(unit)->cmicd_base;
#endif
    *datap = CMREAD(unit, addr32);
#if defined(CMIC_SOFT_BYTE_SWAP)
    *datap = CMIC_SWAP32(*datap);
#endif
    LOG_VERBOSE(BSL_LS_SOC_PCI,
             (BSL_META_U(unit,
                         "PCI%d memR(0x%x)=0x%x\n"), unit, addr32, *datap));
    return SOC_E_NONE;
}

/*
 * Get a CMIC register in PCI space.
 * Input address is relative to the base of CMIC registers.
 */
uint32
soc_pci_read(int unit, uint32 addr)
{
    uint32 data, addr32 = addr;
#if defined(BCM_IPROC_SUPPORT)  && defined(IPROC_NO_ATL)
	addr32 += SOC_DRIVER(unit)->cmicd_base;
#endif
	data = CMREAD(unit, addr32);
#if defined(CMIC_SOFT_BYTE_SWAP)
    data = CMIC_SWAP32(data);
#endif
    LOG_VERBOSE(BSL_LS_SOC_PCI,
             (BSL_META_U(unit,
                         "PCI%d memR(0x%x)=0x%x\n"), unit, addr32, data));
    return data;
}

/*
 * Set a CMIC register in PCI space.
 * Input address is relative to the base of CMIC registers.
 */
int
soc_pci_write(int unit, uint32 addr, uint32 data)
{
	uint32 addr32 = addr;
#if defined(BCM_IPROC_SUPPORT)  && defined(IPROC_NO_ATL)
	addr32 += SOC_DRIVER(unit)->cmicd_base;
#endif
#if defined(CMIC_SOFT_BYTE_SWAP)
    data = CMIC_SWAP32(data);
#endif
    LOG_VERBOSE(BSL_LS_SOC_PCI,
             (BSL_META_U(unit,
                         "PCI%d memW(0x%x)=0x%x\n"), unit, addr32, data));
    CMWRITE(unit, addr32, data);
    return 0;
}

/*
 * Read a register from the PCI Config Space
 */
uint32
soc_pci_conf_read(int unit, uint32 addr)
{
    uint32 data;
    data = CMCONFREAD(unit, addr);
    LOG_VERBOSE(BSL_LS_SOC_PCI,
             (BSL_META_U(unit,
                         "PCI%d ConfigR(0x%x)=0x%x\n"), unit, addr, data));
    return data;
}

/*
 * Write a value to the PCI Config Space
 */
int
soc_pci_conf_write(int unit, uint32 addr, uint32 data)
{
    LOG_VERBOSE(BSL_LS_SOC_PCI,
             (BSL_META_U(unit,
                         "PCI%d ConfigW(0x%x)=0x%x\n"), unit, addr, data));
    CMCONFWRITE(unit, addr, data);
    return 0;
}
#else
/* Get CMIC PCI Register */
int
soc_pci_getreg(int unit, uint32 addr, uint32 *datap)
{
    *datap = CMREAD(unit, addr);
#if defined(CMIC_SOFT_BYTE_SWAP)
    *datap = CMIC_SWAP32(*datap);
#endif
    return SOC_E_NONE;
}
#endif	/* SOC_PCI_DEBUG */

/*
 * soc_pci_test checks PCI memory range 0x00-0x4f
 */

int
soc_pci_test(int unit)
{
    int i;
    uint32 tmp, reread;
    uint32 pat;
#ifdef BCM_CMICM_SUPPORT
    int cmc = SOC_PCI_CMC(unit);
#endif

    SCHAN_LOCK(unit);

    /* Check for address uniqueness */

#ifdef BCM_CMICM_SUPPORT
    if(soc_feature(unit, soc_feature_cmicm)) {
        for (i = 0; i < CMIC_SCHAN_WORDS(unit); i++) {
            pat = 0x55555555 ^ (i << 24 | i << 16 | i << 8 | i);
            soc_pci_write(unit, CMIC_CMCx_SCHAN_MESSAGEn(cmc, i), pat);
        }

        for (i = 0; i < CMIC_SCHAN_WORDS(unit); i++) {
            pat = 0x55555555 ^ (i << 24 | i << 16 | i << 8 | i);
            tmp = soc_pci_read(unit, CMIC_CMCx_SCHAN_MESSAGEn(cmc, i));
            if (tmp != pat) {
                goto error;
            }
        }
    } else
#endif
    {
        for (i = 0; i < CMIC_SCHAN_WORDS(unit); i++) {
            pat = 0x55555555 ^ (i << 24 | i << 16 | i << 8 | i);
            soc_pci_write(unit, CMIC_SCHAN_MESSAGE(unit, i), pat);
        }

        for (i = 0; i < CMIC_SCHAN_WORDS(unit); i++) {
            pat = 0x55555555 ^ (i << 24 | i << 16 | i << 8 | i);
            tmp = soc_pci_read(unit, CMIC_SCHAN_MESSAGE(unit, i));
            if (tmp != pat) {
                goto error;
            }
        }
    }
    if (!SAL_BOOT_QUICKTURN) {  /* Takes too long */
        /* Rotate walking zero/one pattern through each register */

        pat = 0xff7f0080;       /* Simultaneous walking 0 and 1 */

        for (i = 0; i < CMIC_SCHAN_WORDS(unit); i++) {
            int j;

            for (j = 0; j < 32; j++) {
#ifdef BCM_CMICM_SUPPORT
                if(soc_feature(unit, soc_feature_cmicm)) {
                    soc_pci_write(unit, CMIC_CMCx_SCHAN_MESSAGEn(cmc, i), pat);
                    tmp = soc_pci_read(unit, CMIC_CMCx_SCHAN_MESSAGEn(cmc, i));
                } else
#endif
                {
                    soc_pci_write(unit, CMIC_SCHAN_MESSAGE(unit, i), pat);
                    tmp = soc_pci_read(unit, CMIC_SCHAN_MESSAGE(unit, i));
                }
                if (tmp != pat) {
                    goto error;
                }
                pat = (pat << 1) | ((pat >> 31) & 1);	/* Rotate left */
            }
        }
    }

    /* Clear to zeroes when done */

    for (i = 0; i < CMIC_SCHAN_WORDS(unit); i++) {
#ifdef BCM_CMICM_SUPPORT
        if(soc_feature(unit, soc_feature_cmicm)) {
            soc_pci_write(unit, CMIC_CMCx_SCHAN_MESSAGEn(cmc, i), 0);
        } else
#endif
        {
            soc_pci_write(unit, CMIC_SCHAN_MESSAGE(unit, i), 0);
        }
    }

    SCHAN_UNLOCK(unit);
    return 0;

 error:
#ifdef BCM_CMICM_SUPPORT
    if(soc_feature(unit, soc_feature_cmicm)) {
        reread = soc_pci_read(unit, CMIC_CMCx_SCHAN_MESSAGEn(cmc, i));
    } else
#endif
    {
        reread = soc_pci_read(unit, CMIC_SCHAN_MESSAGE(unit, i));
    }
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "FATAL PCI error testing PCIM[0x%x]:\n"
                          "Wrote 0x%x, read 0x%x, re-read 0x%x\n"),
               i, pat, tmp, reread));

    SCHAN_UNLOCK(unit);
    return SOC_E_INTERNAL;
}

/*
 * Do a harmless memory read from the address CMIC_OFFSET_TRIGGER.  This
 * can be called from error interrupts, memory test miscompare, etc. to
 * trigger a logic analyzer that is waiting for a PCI memory read of the
 * trigger address.
 */

void
soc_pci_analyzer_trigger(int unit)
{
    (void) soc_pci_read(unit, CMIC_OFFSET_TRIGGER);
}
