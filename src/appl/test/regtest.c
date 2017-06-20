/*
 * $Id: regtest.c,v 1.92 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Register Tests
 */

#include <shared/bsl.h>

#include <sal/types.h>
#include <shared/bsl.h>
#include <soc/defs.h>
#include <soc/debug.h>
#include <bcm/link.h>
#include <soc/register.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/test.h>
#include <bcm_int/control.h>
#ifdef BCM_TRIUMPH2_SUPPORT
#include <soc/triumph2.h>
#endif
#if defined(BCM_TRIUMPH3_SUPPORT)
#include <soc/triumph3.h>
#endif /* BCM_TRIUMPH3_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
#include <soc/katana.h>
#endif
#ifdef BCM_SIRIUS_SUPPORT
#include <soc/sbx/sirius.h>
#endif
#ifdef BCM_CALADAN3_SUPPORT
#include <sal/appl/config.h>
#endif
#if defined(BCM_TOMAHAWK_SUPPORT)
#include <soc/tomahawk.h>
#endif /* BCM_TOMAHAWK_SUPPORT */

#ifdef BCM_PETRA_SUPPORT
#include <soc/dpp/drv.h>
#endif
#if defined (BCM_DFE_SUPPORT)
#include <soc/dfe/cmn/dfe_drv.h>
#endif
#ifdef BCM_DNX_SUPPORT
#include <soc/dnx/drv.h>
#endif
#if defined(BCM_DNXF_SUPPORT)
#include <soc/dnxf/cmn/dnxf_drv.h>
#endif /*BCM_DFE_SUPPORT*/
#ifdef BCM_TRIDENT2_SUPPORT
#include <soc/trident2.h>
#endif
#ifdef BCM_PETRA_SUPPORT
#include <appl/dpp/regs_filter.h>
#endif

#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT) || \
    defined (BCM_POLAR_SUPPORT) || defined (BCM_CALADAN3_SUPPORT)

STATIC int rval_test_proc_dispatch(int unit, soc_regaddrinfo_t *ainfo, void *data);

STATIC int rval_test_proc(int unit, soc_regaddrinfo_t *ainfo, void *data);
#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT) || \
    defined (BCM_CALADAN3_SUPPORT)
static int rval_test_proc_above_64(int unit, soc_regaddrinfo_t *ainfo, void *data);
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT || BCM_CALADAN3_SUPPORT */

/*
 * this is a special marker that is used in soc_reg_iterate()
 * to indicate that no more variations of the current register
 * should be iterated over.
 */
#define SOC_E_IGNORE -6000

#if defined (BCM_PETRA_SUPPORT)

int
reg_contain_one_of_the_fields(int unit,uint32 reg,uint32 *fields)
{

  int i;
  for (i=0;fields[i]!=NUM_SOC_FIELD;i++) {
      if (SOC_REG_FIELD_VALID(unit,reg,fields[i])) {
          return 1;
      }
  }
  return 0;
}

#endif

/*
 * reg_test
 *
 * Read/write/addressing tests of all SOC internal register R/W bits
 */
STATIC int
try_reg_value(struct reg_data *rd,
              soc_regaddrinfo_t *ainfo,
              char *regname,
              uint32 pattern,
              uint64 mask)
{
    uint64  pat64, rd64, wr64, rrd64, notmask;
    char    wr_str[20], mask_str[20], pat_str[20], rrd_str[20];
    int r, read_only_flag, write_only_flag;

    COMPILER_64_ZERO(pat64);
    COMPILER_64_ZERO(rd64);
    COMPILER_64_ZERO(wr64);
    COMPILER_64_ZERO(rrd64);
    COMPILER_64_ZERO(notmask);

    read_only_flag = SOC_REG_INFO(rd->unit, ainfo->reg).flags & SOC_REG_FLAG_RO ? 1 : 0;
    write_only_flag = SOC_REG_INFO(rd->unit, ainfo->reg).flags & SOC_REG_FLAG_WO ? 1 : 0;

    /* skip 64b registers in sim */
    if (SAL_BOOT_PLISIM) {
        if (!SOC_IS_XGS(rd->unit) && !SOC_IS_JERICHO_2_A0(rd->unit) && SOC_REG_IS_64(rd->unit,ainfo->reg)) {
            LOG_WARN(BSL_LS_APPL_COMMON,
                     (BSL_META("Skipping 64 bit %s register in sim\n"),regname));
            return 0;
      }
    }
#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(rd->unit)) {
        if ((r = soc_robo_anyreg_read(rd->unit, ainfo, &rd64)) < 0) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("ERROR: read reg %s failed: %s\n"),
                       regname, soc_errmsg(r)));
            return -1;
        }
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT) || \
    defined (BCM_CALADAN3_SUPPORT)
        if ((write_only_flag == 0) && ((r = soc_anyreg_read(rd->unit, ainfo, &rd64)) < 0)) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("ERROR: read reg %s failed: %s\n"),
                       regname, soc_errmsg(r)));
            return -1;
        }
#ifdef BCM_APACHE_SUPPORT
        /* Skipping CFAP registers because we do
           constrained writes to these registers.
           27th bit of AVS_REG_PMB_SLAVE_AVS_PWD_ACC_CONTROL register
           is a DONE bit which needs to be cleared for every new run
         */
        if (SOC_IS_APACHE(rd->unit)) {
            if (sal_strncasecmp(regname, "CFAPCONFIG", 10) == 0) {
                COMPILER_64_SET(mask, 0, 0);
            } else if (sal_strncasecmp(regname, "CFAPFULLRESETPOINT", 18) == 0) {
                COMPILER_64_SET(mask, 0, 0);
            } else if (sal_strncasecmp(regname, "CFAPFULLSETPOINT", 16) == 0) {
                COMPILER_64_SET(mask, 0, 0);
            } else if (sal_strncasecmp(regname, "AVS_REG_PMB_SLAVE_AVS_PWD_ACC_CONTROL", 37) == 0){
                COMPILER_64_SET(mask, 0, 0);
            } else if (sal_strncasecmp(regname, "MMU_TDM_DEBUG", 13) == 0) {
                COMPILER_64_SET(mask, 0, 0);
            }

        }
#endif
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

    COMPILER_64_SET(pat64, pattern, pattern);
    COMPILER_64_AND(pat64, mask);

    notmask = mask;
    COMPILER_64_NOT(notmask);

    wr64 = rd64;
    COMPILER_64_AND(wr64, notmask);
    COMPILER_64_OR(wr64, pat64);

    format_uint64(wr_str, wr64);
    format_uint64(mask_str, mask);

    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("Write %s: value %s mask %s\n"),
              regname, wr_str, mask_str));
#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(rd->unit)) {
        if ((r = soc_robo_anyreg_write(rd->unit, ainfo, wr64)) < 0) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("ERROR: write reg %s failed: %s wrote %s (mask %s)\n"),
                       regname, soc_errmsg(r), wr_str, mask_str));
            rd->error = r;
            return -1;
        }
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT) || \
    defined (BCM_CALADAN3_SUPPORT)
        if ((read_only_flag == 0) && ((r = soc_anyreg_write(rd->unit, ainfo, wr64)) < 0)) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("ERROR: write reg %s failed: %s wrote %s (mask %s)\n"),
                       regname, soc_errmsg(r), wr_str, mask_str));
            rd->error = r;
            return -1;
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(rd->unit)) {
        if ((r = soc_robo_anyreg_read(rd->unit, ainfo, &rrd64)) < 0) {        
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("ERROR: reread reg %s failed: %s after wrote %s (mask %s)\n"),
                       regname, soc_errmsg(r), wr_str, mask_str));
            rd->error = r;
            return -1;
        }
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT) || \
    defined (BCM_CALADAN3_SUPPORT)
        if ((write_only_flag == 0) && ((r = soc_anyreg_read(rd->unit, ainfo, &rrd64)) < 0)) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("ERROR: reread reg %s failed: %s after wrote %s (mask %s)\n"),
                       regname, soc_errmsg(r), wr_str, mask_str));
            rd->error = r;
            return -1;
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

    COMPILER_64_AND(rrd64, mask);
    format_uint64(rrd_str, rrd64);
    format_uint64(pat_str, pat64);

    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("Read  %s: value %s expecting %s\n"),
              regname, rrd_str, pat_str));

    if (!(read_only_flag | write_only_flag) && COMPILER_64_NE(rrd64, pat64)) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("ERROR %s: wrote %s read %s (mask %s)\n"),
                   regname, pat_str, rrd_str, mask_str));
        rd->error = SOC_E_FAIL;
    }

    /* put the register back the way we found it */
#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(rd->unit)) {
        if ((r = soc_robo_anyreg_write(rd->unit, ainfo, rd64)) < 0) {        
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("ERROR: rewrite reg %s failed: %s\n"),
                       regname, soc_errmsg(r)));
            rd->error = r;
            return -1;        
        }
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT) || \
    defined (BCM_CALADAN3_SUPPORT)
        if ((read_only_flag == 0) && ((r = soc_anyreg_write(rd->unit, ainfo, rd64)) < 0)) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("ERROR: rewrite reg %s failed: %s\n"),
                       regname, soc_errmsg(r)));
            rd->error = r;
            return -1;        
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

    return 0;
}

/**
 * since the nbih/nbil port macros block can be in reset mode 
 * we give the oportunity to skip them at tr1-8 
 * 
 * @author elem (16/12/2014)
 * 
 * @param blocks 
 * 
 * @return uint8 
 */
uint8 block_can_be_disabled(soc_block_type_t block)
{
    switch (block) {
        case SOC_BLK_KAPS:    
        case SOC_BLK_XLP:
        case SOC_BLK_CLP:
        case SOC_BLK_ILKN_PMH:
        case SOC_BLK_ILKN_PML:
           return 1; /* Skip these blocks can be in reset state*/
        default:
            break;
    }
    return 0;

}

uint8 blocks_can_be_disabled(soc_block_types_t blocks)
{
    soc_block_type_t block  = SOC_REG_FIRST_BLK_TYPE(blocks);
    return block_can_be_disabled(block);

}


#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT) || \
    defined (BCM_CALADAN3_SUPPORT)
/*
 * Test a register above 64 bit
 * If reg_data.flag can control a minimal test
 */
STATIC int
try_reg_above_64_value(struct reg_data *rd,
                       soc_regaddrinfo_t *ainfo,
                       char *regname,
                       uint32 pattern,
                       soc_reg_above_64_val_t mask)
{
    char    wr_str[256], mask_str[256], pat_str[256], rrd_str[256];
    int r;
    soc_reg_above_64_val_t rd_val, pat, notmask, wr_val, rrd_val;

    /* skip 64b registers in sim */
    if (SAL_BOOT_PLISIM) {
        if (SOC_REG_IS_64(rd->unit,ainfo->reg)) {
            LOG_WARN(BSL_LS_APPL_COMMON,
                     (BSL_META("Skipping 64 bit %s register in sim\n"),regname));
            return 0;
      }
    }

    if ((r = soc_reg_above_64_get(rd->unit, ainfo->reg, (ainfo->port >= 0) ? ainfo->port : REG_PORT_ANY, 0, rd_val)) < 0) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("ERROR: read reg %s failed: %s\n"),
                   regname, soc_errmsg(r)));
        return -1;
    }

    SOC_REG_ABOVE_64_SET_PATTERN(pat, pattern);
    SOC_REG_ABOVE_64_AND(pat, mask);

    SOC_REG_ABOVE_64_COPY(notmask, mask);
    SOC_REG_ABOVE_64_NOT(notmask);

    SOC_REG_ABOVE_64_COPY(wr_val, rd_val);
    SOC_REG_ABOVE_64_AND(wr_val, notmask);
    SOC_REG_ABOVE_64_OR(wr_val, pat);

    format_long_integer(wr_str, wr_val, SOC_REG_ABOVE_64_MAX_SIZE_U32);
    format_long_integer(mask_str, mask, SOC_REG_ABOVE_64_MAX_SIZE_U32);

    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("Write %s: value %s mask %s\n"),
              regname, wr_str, mask_str));

    if ((r = soc_reg_above_64_set(rd->unit, ainfo->reg, (ainfo->port >= 0) ? ainfo->port : REG_PORT_ANY, 0, wr_val)) < 0) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("ERROR: write reg %s failed: %s wrote %s (mask %s)\n"),
                       regname, soc_errmsg(r), wr_str, mask_str));
        rd->error = r;
        return -1;
    }

    if ((r = soc_reg_above_64_get(rd->unit, ainfo->reg, (ainfo->port >= 0) ? ainfo->port : REG_PORT_ANY, 0, rrd_val)) < 0) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("ERROR: reread reg %s failed: %s after wrote %s (mask %s)\n"),
                       regname, soc_errmsg(r), wr_str, mask_str));
        rd->error = r;
        return -1;
    }

    SOC_REG_ABOVE_64_AND(rrd_val, mask);
    format_long_integer(rrd_str, rrd_val, SOC_REG_ABOVE_64_MAX_SIZE_U32);
    format_long_integer(pat_str, pat, SOC_REG_ABOVE_64_MAX_SIZE_U32);

    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META("Read  %s: value %s expecting %s\n"),
              regname, rrd_str, pat_str));

    if (!SOC_REG_ABOVE_64_IS_EQUAL(rrd_val, pat)) {
         LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("ERROR %s: wrote %s read %s (mask %s)\n"),
                       regname, pat_str, rrd_str, mask_str));
        rd->error = SOC_E_FAIL;
    }

    /* put the register back the way we found it */
    if ((r = soc_reg_above_64_set(rd->unit, ainfo->reg, (ainfo->port >= 0) ? ainfo->port : REG_PORT_ANY, 0, rd_val)) < 0) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("ERROR: rewrite reg %s failed: %s\n"),
                       regname, soc_errmsg(r)));
        rd->error = r;
        return -1;
    }

    return 0;
}
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT || BCM_CALADAN3_SUPPORT */

/*
 * Test a register
 * If reg_data.flag can control a minimal test
 */
STATIC int
try_reg(int unit, soc_regaddrinfo_t *ainfo, void *data)
{
    struct reg_data    *rd = data;
    uint64        mask, mask2, mask3;
    uint32     temp_mask_hi, temp_mask_lo;
    char        regname[80];
    uint32      access_flag = 0;
#ifdef BCM_HAWKEYE_SUPPORT
    uint32              miscconfig;
    int                 meter = 0;
#endif

    if (!SOC_REG_IS_VALID(unit, ainfo->reg)) {
        return SOC_E_IGNORE;        /* invalid register */
    }

    if (rd->flags & REGTEST_FLAG_ACCESS_ONLY) {
        access_flag = 1;
    }

    if ((SOC_REG_INFO(unit, ainfo->reg).flags &
        (SOC_REG_FLAG_RO | SOC_REG_FLAG_WO | SOC_REG_FLAG_INTERRUPT | SOC_REG_FLAG_GENERAL_COUNTER | SOC_REG_FLAG_SIGNAL)) &&
        !access_flag) {
        return SOC_E_IGNORE;        /* no testable bits */
    }

    if (SOC_REG_INFO(unit, ainfo->reg).regtype == soc_portreg && !SOC_PORT_VALID(unit, ainfo->port)) {
        return 0;            /* skip invalid ports */
    }

#ifdef BCM_ENDURO_SUPPORT
    if (SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit) || SOC_IS_KATANA(unit)) {
        if (ainfo->reg == OAM_SEC_NS_COUNTER_64r) {
            LOG_WARN(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit,
                                 "Skipping OAM_SEC_NS_COUNTER_64 register\n")));
            return 0;               /* skip OAM_SEC_NS_COUNTER_64 register */
        }
    }
#endif    
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        if ( !soc_feature(unit,soc_feature_ces)  && (SOC_REG_BLOCK_IS(unit, ainfo->reg, SOC_BLK_CES)) ) {
            return 0;
        }
        if ( !soc_feature(unit,soc_feature_ddr3)  && (SOC_REG_BLOCK_IS(unit, ainfo->reg, SOC_BLK_CI)) ) {
            return 0;
        }
    }
#endif

    /*
     * set mask to read-write bits fields
     * (those that are not marked untestable, reserved, read-only,
     * or write-only)
     */
    if (rd->flags & REGTEST_FLAG_MASK64) {
        mask = soc_reg64_datamask(unit, ainfo->reg, 0);
        if (!access_flag) {
            mask2 = soc_reg64_datamask(unit, ainfo->reg, SOCF_RES);
            mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_RO);
            COMPILER_64_OR(mask2, mask3);
            mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_SIG);
            COMPILER_64_OR(mask2, mask3);
            mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_WO);
            COMPILER_64_OR(mask2, mask3);
            mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_INTR);
            COMPILER_64_OR(mask2, mask3);
            mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_W1TC);
            COMPILER_64_OR(mask2, mask3);
            mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_COR);
            COMPILER_64_OR(mask2, mask3);
            mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_PUNCH);
            COMPILER_64_OR(mask2, mask3);
            mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_WVTC);
            COMPILER_64_OR(mask2, mask3);
            mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_RWBW);
            COMPILER_64_OR(mask2, mask3);
        
            COMPILER_64_NOT(mask2);
            COMPILER_64_AND(mask, mask2);
       } 
    } else {
    
        volatile uint32    m32;

        m32 = soc_reg_datamask(unit, ainfo->reg, 0);
        if (!access_flag) {
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_RES);
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_RO);
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_WO);
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_W1TC);
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_COR);
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_SIG);
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_INTR);
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_PUNCH);
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_WVTC);
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_RWBW);
#ifdef BCM_POLAR_SUPPORT
            if (SOC_IS_POLAR(unit)) {
                m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_SC);            
            }
#endif /* BCM_POLAR_SUPPORT */
        
#ifdef BCM_SIRIUS_SUPPORT
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_WVTC);
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_RWBW);
#endif
        }
        COMPILER_64_SET(mask, 0, m32);
        
    }

   /* if (mask == 0) {
         return SOC_E_IGNORE;;
    }*/
    COMPILER_64_TO_32_HI(temp_mask_hi,mask);
    COMPILER_64_TO_32_LO(temp_mask_lo,mask);

    if ((temp_mask_hi == 0) && (temp_mask_lo == 0))  {
        return SOC_E_IGNORE;
    }
    
#ifdef BCM_HAWKEYE_SUPPORT
    if (SOC_IS_HAWKEYE(unit) && soc_feature(unit, soc_feature_eee)) {
        if (ainfo->reg == EEE_DELAY_ENTRY_TIMERr) {
        /* The register r/w test should skip bit20 for register 
         *     EEE_DELAY_ENTRY_TIMER.
         * Because bit20 is a constant value before H/W one second delay 
         *     for EEE feature.
         */
            COMPILER_64_SET(mask2, 0, ~0x100000);
            COMPILER_64_AND(mask, mask2);
        }
    }
#endif /* BCM_HAWKEYE_SUPPORT */

    if (COMPILER_64_IS_ZERO(mask)) {
    return SOC_E_IGNORE;        /* no testable bits */
    }

    /*
     * Check if this register is actually implemented in HW for the
     * specified port/cos. If so, the mask is adjusted for the
     * specified port/cos based on what is acutually in HW.
     */
    if (reg_mask_subset(unit, ainfo, &mask)) {
        /* Skip this register. Returning SOC_E_NONE, instead of 
         * SOC_E_IGNORE since we may not want to skip this register
         * all the time (only for certain ports/cos)
         */
        return SOC_E_NONE;
    }

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(unit)) {
        /* coverity[overrun-call] */
        soc_robo_reg_sprint_addr(unit, regname, ainfo);
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT) || \
    defined (BCM_CALADAN3_SUPPORT)
        soc_reg_sprint_addr(unit, regname, ainfo);
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT || BCM_CALADAN3_SUPPORT */
    } 

#ifdef BCM_HAWKEYE_SUPPORT
    if (SOC_IS_HAWKEYE(unit) && (ainfo->reg != MISCCONFIGr)) {
        if (READ_MISCCONFIGr(unit, &miscconfig) < 0) {
            test_error(unit, "Miscconfig read failed\n");
             return SOC_E_IGNORE;
        }
        
        meter = soc_reg_field_get(unit, MISCCONFIGr, 
                              miscconfig, METERING_CLK_ENf);
    
        if(meter){
            soc_reg_field_set(unit, MISCCONFIGr, 
                              &miscconfig, METERING_CLK_ENf, 0);
    
            if (WRITE_MISCCONFIGr(unit, miscconfig) < 0) {
                test_error(unit, "Miscconfig setting failed\n");
                 return SOC_E_IGNORE;
            }
        }
    } 
#endif

    /*
     * minimal test
     * just Fs and 5s
     * only do first instance of each register
     * (only first port, cos, array index, and/or block)
     */
    if (rd->flags & REGTEST_FLAG_MINIMAL) {
    if (try_reg_value(rd, ainfo, regname, 0xffffffff, mask) < 0) {
        return SOC_E_IGNORE;
    }

    if (try_reg_value(rd, ainfo, regname, 0x55555555, mask) < 0) {
        return SOC_E_IGNORE;
    }
    return SOC_E_IGNORE;    /* skip other than first instance */
    }

    /*
     * full test
     */
    if (try_reg_value(rd, ainfo, regname, 0x00000000, mask) < 0) {
    return SOC_E_IGNORE;
    }

    if (try_reg_value(rd, ainfo, regname, 0xffffffff, mask) < 0) {
    return SOC_E_IGNORE;
    }

    if (try_reg_value(rd, ainfo, regname, 0x55555555, mask) < 0) {
    return SOC_E_IGNORE;
    }

    if (try_reg_value(rd, ainfo, regname, 0xaaaaaaaa, mask) < 0) {
    return SOC_E_IGNORE;
    }

#ifdef BCM_HAWKEYE_SUPPORT
     if (SOC_IS_HAWKEYE(unit) && (ainfo->reg != MISCCONFIGr)) {
        if (WRITE_MISCCONFIGr(unit, miscconfig) < 0) {
            test_error(unit, "Miscconfig setting failed\n");
            return SOC_E_IGNORE;
        }
    }
#endif

    return 0;
}

#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT) || \
    defined (BCM_CALADAN3_SUPPORT)
STATIC int
try_reg_above_64(int unit, soc_regaddrinfo_t *ainfo, void *data)
{
    struct reg_data    *rd = data;
    char        regname[80];
    soc_reg_above_64_val_t mask, mask2, mask3;
    uint32      access_flag = 0;

    if (!SOC_REG_IS_VALID(unit, ainfo->reg)) {
        return SOC_E_IGNORE;        /* invalid register */
    }

    if (rd->flags & REGTEST_FLAG_ACCESS_ONLY) {
        access_flag = 1;
    }

    if ((SOC_REG_INFO(unit, ainfo->reg).flags &
        (SOC_REG_FLAG_RO | SOC_REG_FLAG_WO | SOC_REG_FLAG_INTERRUPT | SOC_REG_FLAG_GENERAL_COUNTER | SOC_REG_FLAG_SIGNAL)) &&
       !access_flag) {
        return SOC_E_IGNORE;        /* no testable bits */
    }

    if(SOC_REG_IS_ABOVE_64(unit, ainfo->reg)) {
        if(SOC_REG_ABOVE_64_INFO(unit, ainfo->reg).size + 2 > CMIC_SCHAN_WORDS(unit)) {
            return SOC_E_IGNORE;                /* size + header larget than CMIC buffer */
        }
    }

    if (SOC_REG_INFO(unit, ainfo->reg).regtype == soc_portreg &&
        !SOC_PORT_VALID(unit, ainfo->port)) {
            return 0;            /* skip invalid ports */
    }  

    /*
     * set mask to read-write bits fields
     * (those that are not marked untestable, reserved, read-only,
     * or write-only)
     */
     soc_reg_above_64_datamask(unit, ainfo->reg, 0, mask);
    if (SOC_REG_ABOVE_64_IS_ZERO(mask)) {
        return SOC_E_IGNORE;        /* no testable bits */
    }

    if(SOC_REG_IS_ABOVE_64(unit, ainfo->reg)) {
        if(SOC_REG_ABOVE_64_INFO(unit, ainfo->reg).size + 2 > CMIC_SCHAN_WORDS(unit)) {
            return SOC_E_IGNORE;        /* size + header larget than CMIC buffer */
        }
    }

    soc_reg_sprint_addr(unit, regname, ainfo);

    soc_reg_above_64_datamask(unit, ainfo->reg, 0, mask);
    if (!access_flag) {
        soc_reg_above_64_datamask(unit, ainfo->reg, SOCF_RES, mask2);
        soc_reg_above_64_datamask(unit, ainfo->reg, SOCF_RO, mask3);
        SOC_REG_ABOVE_64_OR(mask2, mask3);
        soc_reg_above_64_datamask(unit, ainfo->reg, SOCF_SIG, mask3);
        SOC_REG_ABOVE_64_OR(mask2, mask3);
        soc_reg_above_64_datamask(unit, ainfo->reg, SOCF_WO,mask3);
        SOC_REG_ABOVE_64_OR(mask2, mask3);
        soc_reg_above_64_datamask(unit, ainfo->reg, SOCF_INTR,mask3);
        SOC_REG_ABOVE_64_OR(mask2, mask3);
        soc_reg_above_64_datamask(unit, ainfo->reg, SOCF_WVTC,mask3);
        SOC_REG_ABOVE_64_OR(mask2, mask3);
          
        SOC_REG_ABOVE_64_NOT(mask2);
        SOC_REG_ABOVE_64_AND(mask, mask2);
    } 

    if (SOC_REG_ABOVE_64_IS_ZERO(mask)  == TRUE)  {
        return SOC_E_IGNORE;
    }

    /*
     * minimal test
     * just Fs and 5s
     * only do first instance of each register
     * (only first port, cos, array index, and/or block)
     */
    if (rd->flags & REGTEST_FLAG_MINIMAL) {
        if (try_reg_above_64_value(rd, ainfo, regname, 0xffffffff, mask) < 0) {
            return SOC_E_IGNORE;
        }
    
        if (try_reg_above_64_value(rd, ainfo, regname, 0x55555555, mask) < 0) {
            return SOC_E_IGNORE;
        }
        
        return SOC_E_IGNORE;    /* skip other than first instance */
    }

    /*
     * full test
     */
    if (try_reg_above_64_value(rd, ainfo, regname, 0x00000000, mask) < 0) {
        return SOC_E_IGNORE;
    }

    if (try_reg_above_64_value(rd, ainfo, regname, 0xffffffff, mask) < 0) {
        return SOC_E_IGNORE;
    }

    if (try_reg_above_64_value(rd, ainfo, regname, 0x55555555, mask) < 0) {
        return SOC_E_IGNORE;
    }

    if (try_reg_above_64_value(rd, ainfo, regname, 0xaaaaaaaa, mask) < 0) {
        return SOC_E_IGNORE;
    }

    return 0;
}
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT || BCM_CALADAN3_SUPPORT */

STATIC int
try_reg_dispatch(int unit, soc_regaddrinfo_t *ainfo, void *data)
{
#if defined (BCM_PETRA_SUPPORT)
    reg_data_t    *rd = data;
#endif /*BCM_PETRA_SUPPORT*/

#if defined (BCM_DFE_SUPPORT)
    if(SOC_IS_DFE(unit)) { 
        int rv;
        int is_filtered = 0;
        
        rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_drv_test_reg_filter, (unit, ainfo->reg, &is_filtered));
        if (rv != SOC_E_NONE) {
            return rv;
        }
        if (is_filtered) {
            return SOC_E_IGNORE;
        }
    }
#endif /*BCM_DFE_SUPPORT*/
#if defined (BCM_PETRA_SUPPORT)
        if(SOC_IS_ARADPLUS_AND_BELOW(unit)) {
            switch(ainfo->reg) {
                case ECI_REG_0001r:
                case EGQ_INDIRECT_COMMANDr:
                        return SOC_E_IGNORE;
                default:
                    break;
            }
    }
        if(SOC_IS_JERICHO(unit)) {
            /*
            * we skip on the following common regs after checking with noam halevi from the following reasons
            * 1. XXX_ECC_INTERRUPT_REGISTER_TEST because that the test mask is vary from block to block biut since the register
            * taken from common we cant know the actual size
            *
            * 2. XXX_INDIRECT_COMMAND   since INDIRECT_COMMAND_COUNT change by the HW we cant count on what we wrote
            *
            * 3. XXX_INDIRECT_COMMAND_WR_DATA since length vary from block to block and its width is like the widest mem in a block  
            *  we cant actually know how match bits is realy active for this register while its definition came from common
            *  and its 640 bits
            *
            *
            */
            uint32 disallowed_fields[] = {INDIRECT_COMMAND_COUNTf,INDIRECT_COMMAND_WR_DATAf, INTERRUPT_REGISTER_TESTf,ECC_INTERRUPT_REGISTER_TESTf,NUM_SOC_FIELD};
            if (ainfo->reg < rd->start_from || ainfo->reg >rd->start_from + rd->count) {
                return 1;
            }
            if (reg_contain_one_of_the_fields(unit,ainfo->reg,disallowed_fields)) {
                return 1;
            }
            if (!(rd->flags & REGTEST_FLAG_INC_PORT_MACROS)) {
                soc_block_types_t regblktype = SOC_REG_INFO(unit, ainfo->reg).block; 
                if (blocks_can_be_disabled(regblktype)) {
                    return 1;
                }

            }

        switch(ainfo->reg) {

#if defined(PLISIM)
        /* these global registers vary from block to block, making the expected value wrong */

        /* Writing to this register migth kill the core clock */
        case ECI_OGER_1000r:
        /* some blocks don't have memories and therfore they don't have these common registers  */
        case FDR_ENABLE_DYNAMIC_MEMORY_ACCESSr:
        case FMAC_ENABLE_DYNAMIC_MEMORY_ACCESSr:
        case NBIH_ENABLE_DYNAMIC_MEMORY_ACCESSr:
        case NBIL_ENABLE_DYNAMIC_MEMORY_ACCESSr:
        case FDR_INDIRECT_COMMAND_ADDRESSr:
        case FMAC_INDIRECT_COMMAND_ADDRESSr:
        case NBIH_INDIRECT_COMMAND_ADDRESSr:
        case NBIL_INDIRECT_COMMAND_ADDRESSr:
        case FDR_INDIRECT_COMMAND_DATA_INCREMENTr:
        case FMAC_INDIRECT_COMMAND_DATA_INCREMENTr:
        case NBIH_INDIRECT_COMMAND_DATA_INCREMENTr:
        case NBIL_INDIRECT_COMMAND_DATA_INCREMENTr:
        case FDR_INDIRECT_COMMAND_WIDE_MEMr:
        case FMAC_INDIRECT_COMMAND_WIDE_MEMr:
        case NBIH_INDIRECT_COMMAND_WIDE_MEMr:
        case NBIL_INDIRECT_COMMAND_WIDE_MEMr:
        case FDR_INDIRECT_FORCE_BUBBLEr:
        case FMAC_INDIRECT_FORCE_BUBBLEr:
        case NBIH_INDIRECT_FORCE_BUBBLEr:
        case NBIL_INDIRECT_FORCE_BUBBLEr:

        /* Unknown errors */
        case CLPORT_SGNDET_EARLYCRSr:
        case MACSEC_PROG_TX_CRCr:
        case MAC_PFC_CTRLr:
        case MAC_PFC_REFRESH_CTRLr:
        case SFD_OFFSETr:

        /* additional regs r/w test at tr 3 failed on emulation*/
        case IHB_INTERRUPT_MASK_REGISTERr:
        case ILKN_PMH_ECC_INTERRUPT_REGISTER_TESTr:
        case ILKN_PMH_INDIRECT_COMMAND_WR_DATAr:
        case ILKN_PMH_INDIRECT_COMMANDr:
        case ILKN_PMH_INTERRUPT_REGISTER_TESTr:
        case ILKN_PML_ECC_INTERRUPT_REGISTER_TESTr:
        case ILKN_PML_INDIRECT_COMMAND_WR_DATAr:
        case ILKN_PML_INDIRECT_COMMANDr:
        case ILKN_PML_INTERRUPT_REGISTER_TESTr:
        case ILKN_SLE_RX_CFGr:
        case ILKN_SLE_RX_DEBUG_0r:
        case ILKN_SLE_RX_ERRINS_0r:
        case ILKN_SLE_RX_LANEr:
        case ILKN_SLE_RX_STATS_WT_ERR_HIGHr:
        case ILKN_SLE_RX_STATS_WT_ERR_LOWr:
        case ILKN_SLE_RX_STATS_WT_PARITYr:
        case ILKN_SLE_RX_STATS_WT_PKT_LOWr:
        case ILKN_SLE_TX_CFGr:
        case ILKN_SLE_TX_DEBUG_0r:
        case ILKN_SLE_TX_ERRINS_0r:
        case ILKN_SLE_TX_LANEr:
        case ILKN_SLE_TX_STATS_WT_ERR_HIGHr:
        case ILKN_SLE_TX_STATS_WT_ERR_LOWr:
        case ILKN_SLE_TX_STATS_WT_PARITYr:
        case ILKN_SLE_TX_STATS_WT_PKT_LOWr:

        case ILKN_SLE_RX_AFIFO_WMr:
        case ILKN_SLE_RX_BURSTr:
        case ILKN_SLE_RX_CAL_ACCr:
        case ILKN_SLE_RX_CAL_INBAND_DYNr:
        case ILKN_SLE_RX_CAL_INBANDr:
        case ILKN_SLE_RX_CAL_OUTBAND_DYNr:
        case ILKN_SLE_RX_CAL_OUTBANDr:
        case ILKN_SLE_RX_CAL_WTr:
        case ILKN_SLE_RX_CTLr:
        case ILKN_SLE_RX_DEBUG_1r:
        case ILKN_SLE_RX_DEBUG_2r:
        case ILKN_SLE_RX_DEBUG_3r:
        case ILKN_SLE_RX_DEBUG_HOLD_0r:
        case ILKN_SLE_RX_DEBUG_HOLD_1r:
        case ILKN_SLE_RX_DEBUG_HOLD_2r:
        case ILKN_SLE_RX_DEBUG_HOLD_3r:
        case ILKN_SLE_RX_ERRINS_1r:
        case ILKN_SLE_RX_ERRINS_2r:
        case ILKN_SLE_RX_ERRINS_3r:
        case ILKN_SLE_RX_EXT_INT_2_ND_MASKr:
        case ILKN_SLE_RX_EXT_INT_FORCEr:
        case ILKN_SLE_RX_EXT_INT_MASKr:
        case ILKN_SLE_RX_FCOB_RETRANSMIT_SLOT_DYr:
        case ILKN_SLE_RX_FCOB_RETRANSMIT_SLOTr:
        case ILKN_SLE_RX_INT_2_ND_MASKr:
        case ILKN_SLE_RX_INT_FORCEr:
        case ILKN_SLE_RX_INT_MASKr:
        case ILKN_SLE_RX_LANE_2r:
        case ILKN_SLE_RX_METAFRAMEr:
        case ILKN_SLE_RX_REMAP_LANE_14_10r:
        case ILKN_SLE_RX_REMAP_LANE_19_15r:
        case ILKN_SLE_RX_REMAP_LANE_24_20r:
        case ILKN_SLE_RX_REMAP_LANE_29_25r:
        case ILKN_SLE_RX_REMAP_LANE_34_30r:
        case ILKN_SLE_RX_REMAP_LANE_39_35r:
        case ILKN_SLE_RX_REMAP_LANE_44_40r:
        case ILKN_SLE_RX_REMAP_LANE_47_45r:
        case ILKN_SLE_RX_REMAP_LANE_4_0r:
        case ILKN_SLE_RX_REMAP_LANE_9_5r:
        case ILKN_SLE_RX_RETRANSMIT_CONFIGr:
        case ILKN_SLE_RX_RETRANSMIT_TIME_CONFIG_2r:
        case ILKN_SLE_RX_RETRANSMIT_TIME_CONFIGr:
        case ILKN_SLE_RX_SEGMENT_ENABLEr:
        case ILKN_SLE_RX_SERDES_TEST_CNTLr:
        case ILKN_SLE_RX_SERDES_TEST_PATTERNAr:
        case ILKN_SLE_RX_SERDES_TEST_PATTERNBr:
        case ILKN_SLE_RX_SERDES_TEST_PATTERNCr:
        case ILKN_SLE_RX_STATS_ACCr:
        case ILKN_SLE_RX_STATS_WT_BYTE_LOWr:
        case ILKN_SLE_TX_AFIFO_WMr:
        case ILKN_SLE_TX_BURSTr:
        case ILKN_SLE_TX_CAL_ACCr:
        case ILKN_SLE_TX_CAL_INBAND_DYNr:
        case ILKN_SLE_TX_CAL_INBANDr:
        case ILKN_SLE_TX_CAL_OUTBAND_DYNr:
        case ILKN_SLE_TX_CAL_OUTBANDr:
        case ILKN_SLE_TX_CAL_WTr:
        case ILKN_SLE_TX_CTLr:
        case ILKN_SLE_TX_DEBUG_1r:
        case ILKN_SLE_TX_DEBUG_2r:
        case ILKN_SLE_TX_DEBUG_3r:
        case ILKN_SLE_TX_DEBUG_HOLD_0r:
        case ILKN_SLE_TX_DEBUG_HOLD_1r:
        case ILKN_SLE_TX_DEBUG_HOLD_2r:
        case ILKN_SLE_TX_DEBUG_HOLD_3r:
        case ILKN_SLE_TX_ERRINS_1r:
        case ILKN_SLE_TX_ERRINS_2r:
        case ILKN_SLE_TX_ERRINS_3r:
        case ILKN_SLE_TX_FCOB_RETRANSMIT_SLOT_DYr:
        case ILKN_SLE_TX_FCOB_RETRANSMIT_SLOTr:
        case ILKN_SLE_TX_FIFO_CFGr:
        case ILKN_SLE_TX_INT_2_ND_MASKr:
        case ILKN_SLE_TX_INT_FORCEr:
        case ILKN_SLE_TX_INT_MASKr:
        case ILKN_SLE_TX_LANE_2r:
        case ILKN_SLE_TX_METAFRAMEr:
        case ILKN_SLE_TX_RATE_1r:
        case ILKN_SLE_TX_REMAP_LANE_14_10r:
        case ILKN_SLE_TX_REMAP_LANE_19_15r:
        case ILKN_SLE_TX_REMAP_LANE_24_20r:
        case ILKN_SLE_TX_REMAP_LANE_29_25r:
        case ILKN_SLE_TX_REMAP_LANE_34_30r:
        case ILKN_SLE_TX_REMAP_LANE_39_35r:
        case ILKN_SLE_TX_REMAP_LANE_44_40r:
        case ILKN_SLE_TX_REMAP_LANE_47_45r:
        case ILKN_SLE_TX_REMAP_LANE_4_0r:
        case ILKN_SLE_TX_REMAP_LANE_9_5r:
        case ILKN_SLE_TX_RETRANSMIT_CONFIGr:
        case ILKN_SLE_TX_SEGMENT_ENABLEr:
        case ILKN_SLE_TX_SERDES_AFIFO_STALL_SELr:
        case ILKN_SLE_TX_SERDES_TEST_CNTLr:
        case ILKN_SLE_TX_SERDES_TEST_PATTERNAr:
        case ILKN_SLE_TX_SERDES_TEST_PATTERNBr:
        case ILKN_SLE_TX_SERDES_TEST_PATTERNCr:
        case ILKN_SLE_TX_STATS_ACCr:
        case ILKN_SLE_TX_STATS_WT_BYTE_LOWr:


        case CFC_INTERRUPT_MASK_REGISTERr: /*tr3 emulation  reg=CFC_INTERRUPT_MASK_REGISTERr,value=0x55501,expected0x55555*/ 



        case MESH_TOPOLOGY_ECC_ERR_1B_INITIATEr:
        case MESH_TOPOLOGY_ECC_ERR_1B_MONITOR_MEM_MASKr:
        case MESH_TOPOLOGY_ECC_ERR_2B_INITIATEr:
        case MESH_TOPOLOGY_ECC_ERR_2B_MONITOR_MEM_MASKr:
#endif
        /*Above registers only failed on emulation. Current only below registers failed on Jericho. Others should be tested.*/
        case EDB_PAR_ERR_INITIATEr:
        case MESH_TOPOLOGY_GLOBAL_MEM_OPTIONSr:
        case MESH_TOPOLOGY_RESERVED_MTCPr:

            return SOC_E_IGNORE;
        default:
            break;
        }
        if (SOC_IS_JERICHO_PLUS_A0(unit)) {
            switch(ainfo->reg) {


                
     
                case EDB_INDIRECT_WR_MASKr:
                case EGQ_INDIRECT_WR_MASKr:
                case IHB_INDIRECT_WR_MASKr:
                case KAPS_BBS_INDIRECT_WR_MASKr:
                case PPDB_A_INDIRECT_WR_MASKr:
                case PPDB_B_INDIRECT_WR_MASKr:

                
                case IHP_REG_00A4r:
                case IHP_REG_00A6r:

                
                case IHP_ECC_ERR_1B_MONITOR_MEM_MASKr:
                case IHP_ECC_ERR_2B_MONITOR_MEM_MASKr:

                    return SOC_E_IGNORE;
                default:
                    break;
            }
        }


        }
#endif
#ifdef BCM_POLAR_SUPPORT
        if (SOC_IS_POLAR(unit)) {
            switch (ainfo->reg) {
                case CFP_DATAr:
                case CFP_MASKr:
                case EGRESS_VID_RMK_TBL_ACSr:
                case PEAK_TEMP_MON_RESUr:
                case TEMP_MON_RESUr:
                case WATCH_DOG_CTRLr:
                case LED_EN_MAPr:
                case INT_STSr:
                    /* Skip these registers */
                    return SOC_E_IGNORE;
                default:
                    if (((ainfo->addr & 0xf000) == 0x1000) ||
                        ((ainfo->addr & 0xf000) == 0x8000) ||
                        ((ainfo->addr & 0xfff0) == 0xe020) ||
                        ((ainfo->addr & 0xfff0) == 0x0040)) {
                        /* Skip PHY, IO PAD and POWER registers */
                        return SOC_E_IGNORE;
                    }
                    break;
            }
        }
#endif /* BCM_POLAR_SUPPORT */

#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT) || \
    defined (BCM_CALADAN3_SUPPORT)

#ifdef BCM_SABER2_SUPPORT
    if (SOC_IS_SABER2(unit)) 
    {
        switch (ainfo->reg) {
        case OAMP_INDIRECT_COMMAND_WR_DATAr:
             return SOC_E_IGNORE;
        default:
             break;
        }
    }
#endif /* BCM_SABER2_SUPPORT */
#ifdef BCM_TOMAHAWK2_SUPPORT
    if (SOC_IS_TOMAHAWK2(unit)) {
        switch (ainfo->reg) {
        /* CONFIG_ERRORf: HW write */
        /* MIB_RSC_DATA_HI_LVMf and MIB_RSC_DATA_LO_LVMf: Reserved */
        case XLPORT_MIB_RSC_RAM_CONTROLr:
        case TOP_CORE_PLL0_CTRL_0r:
        case TOP_CORE_PLL0_CTRL_4r:
        case TOP_CORE_PLL0_CTRL_6r:
        case TOP_CORE_PLL0_CTRL_8r:
        case TOP_CORE_PLL0_CTRL_9r:
            return SOC_E_IGNORE;
        default:
            break;
        }
    }
#endif /* BCM_TOMAHAWK2_SUPPORT */
    if(SOC_REG_IS_ABOVE_64(unit, ainfo->reg)) {
        return try_reg_above_64(unit, ainfo, data);
    } else 
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT || BCM_CALADAN3_SUPPORT */
    { /* To work on 64 AND '32' bits regs */
        return try_reg(unit, ainfo, data);
    }
}
STATIC int
rval_test_proc_dispatch(int unit, soc_regaddrinfo_t *ainfo, void *data)
{
#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT) || \
    defined (BCM_CALADAN3_SUPPORT)
    if(SOC_REG_IS_ABOVE_64(unit, ainfo->reg)) {
        return rval_test_proc_above_64(unit, ainfo, data);
    } else 
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT || BCM_CALADAN3_SUPPORT */
    { /* To work on 64 AND '32' bits regs */
        return rval_test_proc(unit, ainfo, data);
    }
}

/*
 * Register read/write test (tr 3)
 */
int
reg_test(int unit, args_t *a, void *pa)
{
    struct reg_data rd;
    int r=0;
    int rv = 0;
    char *s;
#if defined(BCM_HAWKEYE_SUPPORT) || defined (BCM_ESW_SUPPORT) || \
    defined (BCM_SIRIUS_SUPPORT) /* DPPCOMPILEENABLE */
    uint32 tem;
#endif /* BCM_HAWKEYE_SUPPORT || BCM_ESW_SUPPORT  || DPPCOMPILEENABE */
#if defined(BCM_HURRICANE3_SUPPORT)
    int port;
#endif
#if defined(BCM_HURRICANE3_SUPPORT)
    int cosq, idx;
#endif /* BCM_HURRICANE3_SUPPORT */

    COMPILER_REFERENCE(pa);

    if (!SOC_UNIT_VALID(unit)) {
        return SOC_E_UNIT;
    }
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META_U(unit,
                         "Register read/write test\n")));

    rd.unit = unit;
    rd.error = SOC_E_NONE;
    rd.flags = 0;
    rd.start_from = 0;
    rd.count = NUM_SOC_REG;

    if (a)
    {
        while ((s = ARG_GET(a)) != NULL) {
            if (sal_strcasecmp(s, "mini") == 0 ||
                sal_strcasecmp(s, "minimal") == 0) {
                rd.flags |= REGTEST_FLAG_MINIMAL;
                continue;
            }
            if (sal_strcasecmp(s, "mask64") == 0 ||
                sal_strcasecmp(s, "datamask64") == 0) {
                rd.flags |= REGTEST_FLAG_MASK64;
                continue;
            }
            if (sal_strcasecmp(s, "IncPm") == 0) {
                rd.flags |= REGTEST_FLAG_INC_PORT_MACROS;
                continue;
            }
            if (sal_strcasecmp(s, "StartFrom") == 0) {
                char *sf = ARG_GET(a);
                rd.start_from  = sal_ctoi(sf,0);
                continue;
            }
            if (sal_strcasecmp(s, "RegsNo") == 0) {
                char *sf = ARG_GET(a);
                rd.count  = sal_ctoi(sf,0);
                continue;
            }
            if (sal_strcasecmp(s, "AccessOnly") == 0) {
                rd.flags |= REGTEST_FLAG_ACCESS_ONLY;
                continue;
            }
            LOG_WARN(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit,
                                 "WARNING: unknown argument '%s' ignored\n"), s));
        }
    }

    if (BCM_UNIT_VALID(unit)) {
        rv = bcm_linkscan_enable_set(unit, 0); /* disable linkscan */
        if(rv != SOC_E_UNAVAIL) { /* if unavail - no need to disable */
            BCM_IF_ERROR_RETURN(rv);
        }
    }

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        if ((r = soc_sirius_reset(unit)) < 0) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Unable to reset unit %d: %s\n"),
                       unit, soc_errmsg(r)));
            goto done;
        }
    }
    else
#endif
    {
#if defined (BCM_PETRA_SUPPORT)   
        if (SOC_IS_ARAD(unit)) {
            soc_counter_stop(unit);
            if ((r = soc_dpp_device_reset(unit, SOC_DPP_RESET_MODE_REG_ACCESS, SOC_DPP_RESET_ACTION_INOUT_RESET)) < 0) {
                LOG_ERROR(BSL_LS_APPL_COMMON, (BSL_META_U(unit, "ERROR: Unable to reinit unit %d: %s\n"), unit, soc_errmsg(r)));
                goto done;
            }
        } else
#endif
#ifdef BCM_DFE_SUPPORT
        if (SOC_IS_DFE(unit))
        {
            r = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_drv_blocks_reset, (unit, 0 , NULL));
            if (r != SOC_E_NONE)
            {
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Unable to reinit unit %d: %s\n"), unit, soc_errmsg(r)));
                goto done;
            }
        } else 
#endif
        {
#ifdef BCM_CALADAN3_SUPPORT
            if (SOC_IS_CALADAN3(unit)) {
                sal_config_set("diag_emulator_partial_init", "1");
            }
#endif
            if(!(SOC_IS_DNXF(unit) || SOC_IS_DNX(unit))) {
                if ((r = soc_reset_init(unit)) < 0) {
                    LOG_ERROR(BSL_LS_APPL_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: Unable to reset unit %d: %s\n"),
                               unit, soc_errmsg(r)));
                    goto done;
                }
            }
        }
    }
    
    if (SOC_IS_HB_GW(unit) && soc_feature(unit, soc_feature_bigmac_rxcnt_bug)) {
        /*
         * We need to wait for auto-negotiation to complete to ensure
         * that the BigMAC Rx counters will respond correctly.
         */
        sal_usleep(500000);
    }

    /* When doing E2EFC register/memory read/write tests, it is strongly recommended to set this parity genrate 
     *  enable bit to 0. This will prevent hardware automatically overwritting the ECC and
     *  parity field in E2EFC registers/memories contents and failing the tests.
     */
#if defined(BCM_HAWKEYE_SUPPORT) || defined(BCM_HURRICANE2_SUPPORT) /* DPPCOMPILEENABLE */
    if (soc_reg_field_valid(unit, MISCCONFIGr, E2EFC_PARITY_GEN_ENf)) {
        SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &tem));
        soc_reg_field_set(unit, MISCCONFIGr, &tem, E2EFC_PARITY_GEN_ENf, 0);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, tem));
    } else if (soc_reg_field_valid(unit, MISCCONFIGr, PARITY_CHECK_ENf)) {
        /* If there is no E2EFC_PARITY_GEN_EN field, 
         * try to disable PARITY_ENABLE_EN 
         */
        SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &tem));
        soc_reg_field_set(unit, MISCCONFIGr, &tem, PARITY_CHECK_ENf, 0);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, tem)); 
    }
#endif     /* DPPCOMPILEENABLE */

    /* When doing REMOTE_PFC_STATE register read/write tests, it is recommended to set
     * the REMOTE_PFC_XOFF_TC_TIME_OUT register to zero.
     * It will prevent the REMOTE_PFC_STATE register was modified by HW,
     * once the timeout event is happened.
     */
#if defined(BCM_HURRICANE3_SUPPORT)
    if (SOC_REG_IS_VALID(unit, REMOTE_PFC_STATEr)) {
        cosq = NUM_COS(unit);
        PBMP_ALL_ITER(unit, port) {
            for (idx = 0; idx < cosq; idx++) {
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, 
                    REMOTE_PFC_XOFF_TC_TIME_OUTr,port, idx, 0));
            }
        }
    }
#endif /* BCM_HURRICANE3_SUPPORT */
   
#ifdef BCM_TRIUMPH3_SUPPORT
    if (SOC_IS_TRIUMPH3(unit)) {
        soc_port_t port;
        soc_port_t port_max = SOC_INFO(unit).cpu_hg_index;

        /* Initialize the MMU port mapping so the backpressure
         * registers work properly.
         */
        for (port = 0; port < port_max; port++) {
            SOC_IF_ERROR_RETURN
                (WRITE_MMU_TO_PHY_PORT_MAPPINGr(unit, port, port));
        }

        SOC_IF_ERROR_RETURN
            (WRITE_MMU_TO_PHY_PORT_MAPPINGr(unit, 0, 59));
        SOC_IF_ERROR_RETURN
            (WRITE_MMU_TO_PHY_PORT_MAPPINGr(unit, 59, 0));

        /* Turn off the background MMU processes */
        if ((rv = _soc_triumph3_mem_parity_control(unit, INVALIDm,
                                           SOC_BLOCK_ALL, FALSE)) < 0) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Unable to stop HW updates on unit %d: %s\n"),
                       unit, soc_errmsg(r)));
            goto done;
        }
    } else
#endif /* BCM_TRIUMPH3_SUPPORT */ 
#ifdef BCM_APACHE_SUPPORT
    if (SOC_IS_APACHE(unit)) {
        /* Turn off the background h/w updates, enable cpu access */
        if ((rv = soc_td2_reg_cpu_write_control(unit, TRUE)) < 0) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Unable to stop HW updates on unit %d: %s\n"),
                       unit, soc_errmsg(r)));
            goto done;
        }
    } else
#endif /* BCM_APACHE_SUPPORT */
    {
#ifdef BCM_TOMAHAWK_SUPPORT
        if (SOC_IS_TOMAHAWKX(unit)) {
            /* Turn off the background h/w updates, enable cpu access */
            if ((rv = soc_tomahawk_reg_cpu_write_control(unit, TRUE)) < 0) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Unable to stop HW updates on unit %d: %s\n"),
                           unit, soc_errmsg(r)));
                goto done;
            }
        }
#endif /* BCM_TOMAHAWK_SUPPORT */ 
    }

    /*
     * If try_reg returns -1, there was a register access failure rather
     * than a bit error.
     *
     * If try_reg returns 0, then rd.error is -1 if there was a bit
     * error.
     */
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, IARB_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, IPARS_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, IVLAN_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, ISW1_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, ISW2_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, IL_DEBUG_CONFIGr, 9,
                                    IL_TREX2_DEBUG_LOCKf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, IL_DEBUG_CONFIGr, 13,
                                    IL_TREX2_DEBUG_LOCKf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_THD_1_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_THD_0_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_PORT_THD_0_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_PORT_THD_1_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_CFG_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_PORT_CFG_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G0_BUCKET_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G0_CONFIG_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G1_BUCKET_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G1_CONFIG_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G2_BUCKET_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G2_CONFIG_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G3_BUCKET_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G3_CONFIG_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_CPU_PKT_ENABLE_ECCf, 0));
    }
#endif
    {
#ifdef BCM_TRIDENT2_SUPPORT
        if (SOC_IS_TD2P_TT2P(unit)) {
            /* Turn off the background h/w updates, enable cpu access */
            if ((rv = soc_td2_reg_cpu_write_control (unit, TRUE)) < 0) {
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Unable to stop HW updates : %s\n"),
                           soc_errmsg(r)));
                goto done;
            }
        }
#endif /* BCM_TRIDENT2_SUPPORT */
    }

#ifdef BCM_CALADAN3_SUPPORT
    if (SOC_IS_CALADAN3(unit)) {
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, RC_GLOBAL_DEBUGr, REG_PORT_ANY,
                                    TREX2_DEBUG_ENABLEf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, QM_DEBUGr, REG_PORT_ANY,
                                    QM_TREX2_DEBUG_ENABLEf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, CI_RESETr, REG_PORT_ANY,
                                    TREX2_DEBUG_ENABLEf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, PT_GEN_CONFIGr, (SOC_REG_ADDR_INSTANCE_MASK | 0),
                                    PT_TREX2_DEBUG_ENABLEf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, PT_GEN_CONFIGr, (SOC_REG_ADDR_INSTANCE_MASK | 1),
                                    PT_TREX2_DEBUG_ENABLEf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, PR_GLOBAL_CONFIGr, (SOC_REG_ADDR_INSTANCE_MASK | 0),
                                    TREX2_DEBUG_ENABLEf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, PR_GLOBAL_CONFIGr, (SOC_REG_ADDR_INSTANCE_MASK | 1),
                                    TREX2_DEBUG_ENABLEf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, PB_CONFIGr, REG_PORT_ANY,
                                    PB_TREX2_DEBUG_ENABLEf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, CX_GLOBAL_DEBUGr, REG_PORT_ANY,
                                    TREX2_DEBUG_ENABLEf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, TMA_DEBUGr, REG_PORT_ANY,
                                    TREX2_DEBUG_ENABLEf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, TMB_DEBUGr, REG_PORT_ANY,
                                    TREX2_DEBUG_ENABLEf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, CO_GLOBAL_CONFIGr, (SOC_REG_ADDR_INSTANCE_MASK|0),
                                    TREX2_DEBUG_ENABLEf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, CO_GLOBAL_CONFIGr, (SOC_REG_ADDR_INSTANCE_MASK|1),
                                    TREX2_DEBUG_ENABLEf, 1));
    
    
    }
#endif

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(unit)) {
        if (soc_robo_reg_iterate(unit, try_reg_dispatch, &rd) < 0) {
            LOG_INFO(BSL_LS_APPL_TESTS,
                     (BSL_META_U(unit,
                                 "Continuing test.\n")));
               rv = 0;
        } else {
            rv = rd.error;
        }
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT) || \
    defined (BCM_CALADAN3_SUPPORT)
        if (soc_reg_iterate(unit, try_reg_dispatch, &rd) < 0) {
            LOG_INFO(BSL_LS_APPL_TESTS,
                     (BSL_META_U(unit,
                                 "Continuing test.\n")));
            rv = 0;
        } else {
            rv = rd.error;
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT || BCM_CALADAN3_SUPPORT */
    } 

#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT)
    /* Re-Enable Parity Gen */
    if (soc_reg_field_valid(unit, MISCCONFIGr, E2EFC_PARITY_GEN_ENf)) {
        SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &tem));
        soc_reg_field_set(unit, MISCCONFIGr, &tem, E2EFC_PARITY_GEN_ENf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, tem));
    } else if (soc_reg_field_valid(unit, MISCCONFIGr, PARITY_CHECK_ENf)) {
        /* Re-enable PARITY_ENABLE */
        if (soc_property_get(unit, spn_PARITY_ENABLE, TRUE)) {
            SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &tem));
            soc_reg_field_set(unit, MISCCONFIGr, &tem, PARITY_CHECK_ENf, 1);
            SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, tem)); 
        }
    }
#endif
   
#ifdef BCM_TRIUMPH3_SUPPORT
    if (SOC_IS_TRIUMPH3(unit)) {
        if ((rv = _soc_triumph3_mem_parity_control(unit, INVALIDm,
                                           SOC_BLOCK_ALL, TRUE)) < 0) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Unable to restart HW updates on unit %d: %s\n"),
                       unit, soc_errmsg(r)));
            goto done;
        }
    } else
#endif /* BCM_TRIUMPH3_SUPPORT */
#ifdef BCM_APACHE_SUPPORT
    if (SOC_IS_APACHE(unit)) {
        /* Turn on the background h/w updates, enable cpu access */
        if ((rv = soc_td2_reg_cpu_write_control(unit, FALSE)) < 0) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Unable to restart HW updates on unit %d: %s\n"),
                       unit, soc_errmsg(r)));
            goto done;
        }
    } else
#endif /* BCM_APACHE_SUPPORT */
    {
#ifdef BCM_TOMAHAWK_SUPPORT
        if (SOC_IS_TOMAHAWKX(unit)) {
            /* Turn off the background h/w updates, enable cpu access */
            if ((rv = soc_tomahawk_reg_cpu_write_control(unit, FALSE)) < 0) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Unable to stop HW updates on unit %d: %s\n"),
                           unit, soc_errmsg(r)));
                goto done;
            }
        }
#endif /* BCM_TOMAHAWK_SUPPORT */ 
    }

done:
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, IARB_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, IPARS_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, IVLAN_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, ISW1_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, ISW2_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, IL_DEBUG_CONFIGr, 9,
                                    IL_TREX2_DEBUG_LOCKf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, IL_DEBUG_CONFIGr, 13,
                                    IL_TREX2_DEBUG_LOCKf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_THD_1_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_THD_0_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_PORT_THD_0_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_PORT_THD_1_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_CFG_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_PORT_CFG_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G0_BUCKET_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G0_CONFIG_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G1_BUCKET_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G1_CONFIG_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G2_BUCKET_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G2_CONFIG_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G3_BUCKET_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G3_CONFIG_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_CPU_PKT_ENABLE_ECCf, 1));
    }
#endif
#ifdef BCM_CALADAN3_SUPPORT
    if (SOC_IS_CALADAN3(unit)) {
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, RC_GLOBAL_DEBUGr, REG_PORT_ANY,
                                    TREX2_DEBUG_ENABLEf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, QM_DEBUGr, REG_PORT_ANY,
                                    QM_TREX2_DEBUG_ENABLEf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, CI_RESETr, REG_PORT_ANY,
                                    TREX2_DEBUG_ENABLEf, 0));

        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, PT_GEN_CONFIGr, (SOC_REG_ADDR_INSTANCE_MASK | 0),
                                    PT_TREX2_DEBUG_ENABLEf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, PT_GEN_CONFIGr, (SOC_REG_ADDR_INSTANCE_MASK | 1),
                                    PT_TREX2_DEBUG_ENABLEf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, PR_GLOBAL_CONFIGr, (SOC_REG_ADDR_INSTANCE_MASK | 0),
                                    TREX2_DEBUG_ENABLEf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, PR_GLOBAL_CONFIGr, (SOC_REG_ADDR_INSTANCE_MASK | 1),
                                    TREX2_DEBUG_ENABLEf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, PB_CONFIGr, REG_PORT_ANY,
                                    PB_TREX2_DEBUG_ENABLEf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, CX_GLOBAL_DEBUGr, REG_PORT_ANY,
                                    TREX2_DEBUG_ENABLEf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, TMA_DEBUGr, REG_PORT_ANY,
                                    TREX2_DEBUG_ENABLEf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, TMB_DEBUGr, REG_PORT_ANY,
                                    TREX2_DEBUG_ENABLEf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, CO_GLOBAL_CONFIGr, (SOC_REG_ADDR_INSTANCE_MASK|0),
                                    TREX2_DEBUG_ENABLEf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, CO_GLOBAL_CONFIGr, (SOC_REG_ADDR_INSTANCE_MASK|1),
                                    TREX2_DEBUG_ENABLEf, 0));
    
        sal_config_set("diag_emulator_partial_init", "0");
        if ((r = soc_reset_init(unit)) < 0) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Unable to reset unit %d: %s\n"), unit, soc_errmsg(r)));
            return r;
        }
    }
#endif
    if (rv < 0) {
    test_error(unit, "Register read/write test failed\n");
    return rv;
    }

    return rv;
}

#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT) || \
    defined (BCM_CALADAN3_SUPPORT)
/*
 * rval_test
 *
 * Reset SOC and compare reset values of all SOC registers with regsfile
 */

STATIC int
rval_test_proc_above_64(int unit, soc_regaddrinfo_t *ainfo, void *data)
{
    struct reg_data *rd = data;
    char            buf[80];
    soc_reg_above_64_val_t          rmsk, rval, rrd_val;
    int             r;
    char    wr_str[256], mask_str[256], rval_str[256], rrd_str[256];
    soc_reg_t       reg;

    reg = ainfo->reg;

   

    SOC_REG_ABOVE_64_RST_VAL_GET(unit, reg, rval);
    SOC_REG_ABOVE_64_RST_MSK_GET(unit, reg, rmsk);
    SOC_REG_ABOVE_64_AND(rval, rmsk);

    if (rval_test_skip_reg(unit, ainfo, rd)) {
        /* soc_reg_sprint_addr(unit, buf, ainfo);
            LOG_WARN(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit,
                                 "Skipping register %s\n"), buf)); */
        return 0;
    }

    if (SOC_REG_ABOVE_64_IS_ZERO(rmsk)) {
        return 0;   /* No reset value */
    }

    if(SOC_REG_IS_ABOVE_64(unit, ainfo->reg)) {
        if(SOC_REG_ABOVE_64_INFO(unit, ainfo->reg).size + 2 > CMIC_SCHAN_WORDS(unit)) {
            return SOC_E_IGNORE;  /* size + header larget than CMIC buffer */
            }
    }

    soc_reg_sprint_addr(unit, buf, ainfo);


    if ((r = soc_reg_above_64_get(rd->unit, ainfo->reg, (ainfo->port >= 0) ? ainfo->port : REG_PORT_ANY, 0, rrd_val)) < 0) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: reread reg %s failed: %s after wrote %s (mask %s)\n"),
                   buf, soc_errmsg(r), wr_str, mask_str));
        rd->error = SOC_E_FAIL; 
    }

  
    SOC_REG_ABOVE_64_AND(rrd_val, rmsk);
   
    format_long_integer(rrd_str, rrd_val, SOC_REG_ABOVE_64_MAX_SIZE_U32);
    format_long_integer(rval_str, rval, SOC_REG_ABOVE_64_MAX_SIZE_U32);
    format_long_integer(mask_str, rmsk, SOC_REG_ABOVE_64_MAX_SIZE_U32);


    if (!SOC_REG_ABOVE_64_IS_EQUAL(rrd_val, rval)) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "ERROR %s: default %s read %s (mask %s)\n"),
                   buf, rval_str, rrd_str, mask_str));
        rd->error = SOC_E_FAIL;
    }

    return 0;
}
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT || BCM_CALADAN3_SUPPORT */

/*
 * rval_test
 *
 * Reset SOC and compare reset values of all SOC registers with regsfile
 */

STATIC int
rval_test_proc(int unit, soc_regaddrinfo_t *ainfo, void *data)
{
    struct reg_data *rd = data;
    char            buf[80];
    uint64          value, rmsk, rval, chk;
    char            val_str[20], rmsk_str[20], rval_str[20];
    int             r;
    uint64          mask2,mask3;
    uint32          temp_mask_hi, temp_mask_lo;
    soc_reg_t       reg;

    reg = ainfo->reg;

    /* NOTE: This is experimental */
#ifdef  BCM_TRIUMPH3_SUPPORT
    if (SOC_IS_TRIUMPH3(unit) && 
        (reg == EGR_OUTER_TPIDr || reg == ING_MPLS_TPIDr ||
         reg == ING_OUTER_TPIDr)) {
        reg = reg+ainfo->idx+1;
    }
#endif
    SOC_REG_RST_MSK_GET(unit, reg, rmsk);

    if (rval_test_skip_reg(unit, ainfo, rd)) {
        /* soc_reg_sprint_addr(unit, buf, ainfo);
         LOG_WARN(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "Skipping register %s\n"), buf));*/

        return 0;
    }

#ifdef BCM_ENDURO_SUPPORT
    if (SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) {
        if (ainfo->reg == OAM_SEC_NS_COUNTER_64r) {
            LOG_WARN(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit,
                                 "Skipping OAM_SEC_NS_COUNTER_64 register\n")));
            return 0;                /* skip OAM_SEC_NS_COUNTER_64 register */
        }
    }
#endif    

#ifdef BCM_HAWKEYE_SUPPORT
    if (SOC_IS_HAWKEYE(unit)) {
        if (ainfo->reg == MAC_MODEr) {
            LOG_WARN(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit,
                                 "Skipping MAC_MODE register\n")));
            return 0;                /* skip MAC_MODE register */
        }
    }
#endif    
#ifdef BCM_KATANA_SUPPORT
        if (SOC_IS_KATANA(unit)) {
            if ( !soc_feature(unit,soc_feature_ces)  && (SOC_REG_BLOCK_IS(unit, ainfo->reg, SOC_BLK_CES)) ) {
                return 0;
            }
            if ( !soc_feature(unit,soc_feature_ddr3)  && (SOC_REG_BLOCK_IS(unit, ainfo->reg, SOC_BLK_CI)) ) {
                return 0;
            }
        }
#endif    

#ifdef BCM_HURRICANE2_SUPPORT
    if (SOC_IS_HURRICANE2(unit)) {
        if (ainfo->reg == TOP_XG_PLL0_CTRL_3r) {
            LOG_WARN(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit,
                                 "Skipping TOP_XG_PLL0_CTRL_3 register\n"))); 
            return 0;
        }
    }
#endif
    
    if (SAL_BOOT_PLISIM) {
        if (!SOC_IS_XGS(rd->unit) && SOC_REG_IS_64(rd->unit,ainfo->reg)) {
#ifdef BCM_POLAR_SUPPORT
            if (SOC_IS_POLAR(unit)) {
                /* coverity[overrun-call] */
                soc_robo_reg_sprint_addr(unit, buf, ainfo);
            } else 
#endif /* BCM_POLAR_SUPPORT */
            {
#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT)
                soc_reg_sprint_addr(unit, buf, ainfo);
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT */
            }
            LOG_WARN(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit,
                                 "Skipping 64 bit %s register in sim\n"),buf));
            return 0;
      }
    }

    if (SOC_IS_SAND(unit)) { /* NOTE: Without this check tr 1 breaks for all XGS chips */
        if (rd->flags & REGTEST_FLAG_MASK64) {
            rmsk = soc_reg64_datamask(unit, ainfo->reg, 0);
            mask2 = soc_reg64_datamask(unit, ainfo->reg, SOCF_IGNORE_DEFAULT_TEST);
            mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_WO);

            COMPILER_64_OR(mask2, mask3); 

        /*    mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_INTR);
            COMPILER_64_OR(mask2, mask3);

            mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_COR);
            COMPILER_64_OR(mask2, mask3); */
            mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_W1TC);
            COMPILER_64_OR(mask2, mask3);

            mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_SIG);
            COMPILER_64_OR(mask2, mask3);
            
            COMPILER_64_NOT(mask2);
            COMPILER_64_AND(rmsk, mask2);
            
            
        } else {
        
            volatile uint32 m32;
    
            m32 = soc_reg_datamask(unit, ainfo->reg, 0);
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_IGNORE_DEFAULT_TEST);
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_WO);
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_SIG);

            
#ifdef BCM_SIRIUS_SUPPORT
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_PUNCH);
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_WVTC);
            m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_RWBW);
#endif
            COMPILER_64_SET(rmsk, 0, m32);
            
        }

        COMPILER_64_TO_32_HI(temp_mask_hi,rmsk);
        COMPILER_64_TO_32_LO(temp_mask_lo,rmsk);
    
        if ((temp_mask_hi == 0) && (temp_mask_lo == 0))  {
            return 0;
        }
    }

    /*
     * Check if this register is actually implemented in HW for the
     * specified port/cos. If so, the mask is adjusted for the
     * specified port/cos based on what is acutually in HW.
     */
    if (reg_mask_subset(unit, ainfo, &rmsk)) {
        return 0;
    }

    if (COMPILER_64_IS_ZERO(rmsk)) {
        return 0;   /* No reset value */
    }

    SOC_REG_RST_VAL_GET(unit, reg, rval);
#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(unit)) {
        /* coverity[overrun-call] */
        soc_robo_reg_sprint_addr(unit, buf, ainfo);
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT)
        soc_reg_sprint_addr(unit, buf, ainfo);
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT */
    }

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(rd->unit)) {
        if ((r = soc_robo_anyreg_read(rd->unit, ainfo, &value)) < 0) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: read reg %s (0x%x) failed: %s\n"),
                       buf, ainfo->addr, soc_errmsg(r)));
            rd->error = r;
            return -1;
        }
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT) || \
    defined (BCM_CALADAN3_SUPPORT)
        if ((r = soc_anyreg_read(rd->unit, ainfo, &value)) < 0) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: read reg %s (0x%x) failed: %s\n"),
                       buf, ainfo->addr, soc_errmsg(r)));
            rd->error = r;
        return -1;        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT || BCM_CALADAN3_SUPPORT */
    }

    format_uint64(val_str, value);
    format_uint64(rmsk_str, rmsk);
    format_uint64(rval_str, rval);
    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META_U(unit,
                         "Read %s: reset mask %s, reset value %s, read %s\n"),
              buf, rmsk_str, rval_str, val_str));

    /* Check reset value is correct */
    COMPILER_64_ZERO(chk);
    COMPILER_64_ADD_64(chk, value);
    COMPILER_64_XOR(chk, rval);
    COMPILER_64_AND(chk, rmsk);

    if (!COMPILER_64_IS_ZERO(chk)) {
        COMPILER_64_AND(rval, rmsk);
        format_uint64(rval_str, rval);
        COMPILER_64_AND(value, rmsk);
        format_uint64(val_str, value);
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s: expected %s, got %s, reset mask %s\n"),
                   buf, rval_str, val_str, rmsk_str));
        rd->error = SOC_E_FAIL;
    }

    return 0;
}

/*
 * Register reset value test (tr 1)
 */
int
rval_test(int unit, args_t *a, void *pa)
{
    struct reg_data rd;
    int r, rv = -1;
    char *s;

    COMPILER_REFERENCE(pa);

    LOG_INFO(BSL_LS_APPL_TESTS,
             (BSL_META_U(unit,
                         "Register reset value test\n")));

    rd.unit = unit;
    rd.error = SOC_E_NONE;
    rd.flags = 0;
    rd.start_from = 0;
    rd.count = NUM_SOC_REG;
    while ((s = ARG_GET(a)) != NULL) {
        if (sal_strcasecmp(s, "IncPm") == 0) {
            rd.flags |= REGTEST_FLAG_INC_PORT_MACROS;
            continue;
        }
        if (sal_strcasecmp(s, "StartFrom") == 0) {
            char *sf = ARG_GET(a);
            rd.start_from  = sal_ctoi(sf,0);
            continue;
        }
        if (sal_strcasecmp(s, "RegsNo") == 0) {
            char *sf = ARG_GET(a);
            rd.count  = sal_ctoi(sf,0);
            continue;
        }
        LOG_WARN(BSL_LS_APPL_COMMON,
                 (BSL_META_U(unit,
                             "WARNING: unknown argument '%s' ignored\n"), s));
    }

    if (!SOC_UNIT_VALID(unit)) {
        return SOC_E_UNIT;
    }
  /*  if (!SOC_IS_ARAD(unit)) {*/
        if (BCM_UNIT_VALID(unit)) {
            rv = bcm_linkscan_enable_set(unit, 0); /* disable linkscan */
            if(rv != SOC_E_UNAVAIL) { /* if unavail - no need to disable */
                BCM_IF_ERROR_RETURN(rv);
            }
        }
   /* } */
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit)) {
        soc_triumph2_pipe_mem_clear(unit);
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        if ((r = soc_sirius_reset(unit)) < 0) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Unable to reset unit %d: %s\n"),
                       unit, soc_errmsg(r)));
            goto done;
        }
    }
    else
#endif
    {
#if defined(BCM_DFE_SUPPORT)
        if (SOC_IS_FE1600(unit))
        {
            soc_counter_stop(unit);
            sal_sleep(1);            
            r = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_drv_blocks_reset, (unit, 0 , NULL));
            if (r != SOC_E_NONE)
            {
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Unable to reinit unit %d: %s\n"), unit, soc_errmsg(r)));
                goto done;
            }
            sal_sleep(1);
        } else 
#endif
#if defined(BCM_DFE_SUPPORT) || defined(BCM_PETRA_SUPPORT)
        if (SOC_IS_DFE(unit) || SOC_IS_ARAD(unit))
        {
            soc_counter_stop(unit);
            sal_sleep(1);            
            if ((r = soc_device_reset(unit, SOC_DCMN_RESET_MODE_REG_ACCESS,SOC_DCMN_RESET_ACTION_INOUT_RESET)) < 0) {
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Unable to reinit unit %d: %s\n"), unit, soc_errmsg(r)));
                rv = BCM_E_FAIL;
                goto done;
            }
            sal_sleep(1);

        } else 
#endif
        {
#ifdef BCM_CALADAN3_SUPPORT
            if (SOC_IS_CALADAN3(unit)) {
                sal_config_set("diag_emulator_partial_init", "1");
            }
#endif
#if defined(BCM_PETRA_SUPPORT)
            if (!SOC_IS_ARAD(unit)) 
#endif
            {
#ifdef BCM_POLAR_SUPPORT
                if (SOC_IS_POLAR(unit)) {
                    if ((r = soc_robo_chip_reset(unit)) < 0) {
                        LOG_ERROR(BSL_LS_APPL_COMMON,
                                  (BSL_META_U(unit,
                                              "ERROR: Unable to reset unit %d: %s\n"),
                                   unit, soc_errmsg(r)));
                        goto done;
                    }
                } else 
#endif /* BCM_POLAR_SUPPORT */
                {
                    if ((r = soc_reset_init(unit)) < 0) {
                        LOG_ERROR(BSL_LS_APPL_COMMON,
                                  (BSL_META_U(unit,
                                              "ERROR: Unable to reset unit %d: %s\n"),
                                   unit, soc_errmsg(r)));
                        goto done;
                    }
                }
            }
        }
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) 
        || SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit)
        || SOC_IS_HURRICANE(unit)) {
        /* reset the register XQPORT_XGXS_NEWCTL_REG to 0x0 */
        soc_port_t port;
        PBMP_PORT_ITER(unit, port) {
            switch(port) {
            case 26:
            case 27:
            case 28:
            case 29:
                if(SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) {
                    SOC_IF_ERROR_RETURN(WRITE_XQPORT_XGXS_NEWCTL_REGr(
                                                       unit, port, 0x0));
                } else {
                    SOC_IF_ERROR_RETURN(WRITE_XPORT_XGXS_NEWCTL_REGr(
                                                       unit, port, 0x0));
                }
                break;
            case 30:
            case 34:
            case 38:
            case 42:
            case 46:
            case 50:
                SOC_IF_ERROR_RETURN(WRITE_XQPORT_XGXS_NEWCTL_REGr(unit, port, 0x0));
                break;
            default:
                break;
            }
        }
    }
#endif
    sal_usleep(10000);

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(unit)) {
        if (soc_robo_reg_iterate(unit, rval_test_proc_dispatch, &rd) < 0) {
            goto done;
        }
    } else 
#endif /* BCM_POLAR_SUPPORT */
    {
#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT) || defined (BCM_SAND_SUPPORT)
        if (soc_reg_iterate(unit, rval_test_proc_dispatch, &rd) < 0) {
            goto done;
        }
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT || BCM_SAND_SUPPORT */
    }

    rv = rd.error;

 done:
    if (rv < 0) {
        test_error(unit, "Register reset value test failed\n");
    }
#ifdef BCM_CALADAN3_SUPPORT
    if (SOC_IS_CALADAN3(unit)) {
        sal_config_set("diag_emulator_partial_init", "0");
    }
#endif

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        if ((r = soc_sirius_reset(unit)) < 0) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Unable to reset unit %d: %s\n"),
                       unit, soc_errmsg(r)));
        }
    }
    else
#endif
    {
#ifdef BCM_PETRA_SUPPORT
        if (SOC_IS_ARAD(unit))
        {
            if ((r = soc_dpp_device_reset(unit, SOC_DPP_RESET_MODE_REG_ACCESS,SOC_DPP_RESET_ACTION_INOUT_RESET)) < 0) {
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Unable to reinit unit %d: %s\n"), unit, soc_errmsg(r)));
            
            }
        } else 
#endif
#ifdef BCM_DFE_SUPPORT
        if (SOC_IS_DFE(unit))
        {
            
            LOG_WARN(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "Warning: Run 'tr 141' in order to reset unit %d\n"), unit));
            
        } else 
#endif
        if ((r = soc_reset_init(unit)) < 0) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Unable to reset unit %d: %s\n"),
                       unit, soc_errmsg(r)));
        }
    }

    return rv;
}


#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT */

#if defined(BCM_DFE_SUPPORT)
 
#define BRDC_BLOCKS_TEST_MAX_BLOCKS     (10)
/*
 * Function:
 *      brdc_blk_test_info_get
 * Purpose:
 *      Returns necessary info on device broadcast blocks
 * Parameters:
 *      unit                        - (IN)  Unit number.
 *      max_size                    - (IN)  max number of broadcast blocks
 *      brdc_info                   - (OUT) structure which holds the required info about each broadcast block
 *      actual_size                 - (OUT) number of broadcast blocks
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */

STATIC int 
brdc_blk_test_info_get(int unit, int max_size, soc_reg_brdc_block_info_t *brdc_info, int *actual_size)
{
    int rv = BCM_E_UNAVAIL;

    *actual_size = 0;

#ifdef BCM_DFE_SUPPORT
    if (SOC_IS_DFE(unit))
    {
        rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_drv_test_brdc_blk_info_get, (unit, max_size, brdc_info, actual_size));
        if (rv < 0)
        {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: unit %d : %s\n"),
                                unit, soc_errmsg(rv)));
            return rv;
        }
    }
#endif

    return rv;
}

/*
 * Function:
 *      brdc_blk_test_reg_filter
 * Purpose:
 *      Special registers should not be tested in broadcast block test
 * Parameters:
 *      unit                        - (IN)  Unit number.
 *      reg                         - (IN)  relevant reg
 *      is_filtered                 - (OUT) if 1 - do not test this reg
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */

STATIC int 
brdc_blk_test_reg_filter(int unit, soc_reg_t reg, int *is_filter)
{
    int rv = BCM_E_NONE;

    /*filter read only, interrupt registers, counters and signal*/
    if (SOC_REG_INFO(unit, reg).flags &
            (SOC_REG_FLAG_RO | SOC_REG_FLAG_INTERRUPT | SOC_REG_FLAG_GENERAL_COUNTER | SOC_REG_FLAG_SIGNAL)) {
            *is_filter = 1;
            return rv;
    }

    /*additional filetr per device*/
#ifdef BCM_DFE_SUPPORT
    if (SOC_IS_DFE(unit))
    {
        rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_drv_test_brdc_blk_filter, (unit, reg, is_filter));
        if (rv < 0)
        {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                        (BSL_META_U(unit, "ERROR: unit %d register %d : %s\n"), unit, reg, soc_errmsg(rv)));
            return rv;
        }
        return rv;
    }
#endif

    return rv;
}
/*
 * Function:
 *      brdc_blk_test_reg_addr_get
 * Purpose:
 *      Reading a register value of any kind: 32, 64, above 64.
 *      Giving the addr and the schan block number.
 *  
 * Parameters:
 *      unit                        - (IN)  Unit number
 *      reg                         - (IN)  relevant register of the broadcast block
 *      acc_type                    - (IN)  access type
 *      addr                        - (IN)  register address
 *      block                       - (IN)  schan block number to read from
 *      data                        - (OUT) register value
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
STATIC int
brdc_blk_test_reg_addr_get(int unit, soc_reg_t reg, int acc_type, int addr, int block, soc_reg_above_64_val_t *data)
{
    int reg_size;
    uint64 data64;
    uint32 data32;
    int rv;

    SOC_REG_ABOVE_64_CLEAR(*data);

    if (SOC_REG_IS_ABOVE_64(unit, reg)) 
    {
        reg_size = SOC_REG_ABOVE_64_INFO(unit, reg).size;
        return soc_direct_reg_get(unit, block, addr, reg_size, *data);
    }  else if (SOC_REG_IS_64(unit, reg)) {
        
        rv =  _soc_reg64_get(unit, block, acc_type, addr, &data64);
        SOC_REG_ABOVE_64_WORD_SET(*data, COMPILER_64_LO(data64), 0);
        SOC_REG_ABOVE_64_WORD_SET(*data, COMPILER_64_HI(data64), 1);
        return rv;
    }  else {
        rv = _soc_reg32_get(unit, block, acc_type, addr, &data32);
        SOC_REG_ABOVE_64_WORD_SET(*data, data32 , 0);
        return rv;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      brdc_blk_test
 * Purpose:
 *      Main Broadcast Block test function -
 *      This test role is to set a specific register  in broadcast block,
 *      And to make sure that all the blocks that controlled by the broadcast block changed.
 *  
 * Parameters:
 *      unit                        - (IN)  Unit number.
 *      a                           - (IN)  test args - not used
 *      pa                          - (IN)  not used
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int
brdc_blk_test(int unit, args_t *a, void *pa)
{
    int rv = BCM_E_NONE;
    soc_block_t brdc_block;
    soc_reg_t reg;
    soc_reg_above_64_val_t reg_above_64, reg_above_64_get;
    int block_dummy;
    uint8 acc_type;
    uint32 addr;
    int index, blk_instance;
    soc_reg_brdc_block_info_t *brdc_blk_info = NULL;
    int count, test_block;
    int is_filter;
    int blk_index;
    int result = BCM_E_NONE;
    char *reg_name;
#if defined(SOC_NO_NAMES) 
    char buffer[15];
#endif

    /*write value is fixed to 1*/
    SOC_REG_ABOVE_64_CLEAR(reg_above_64);
    SOC_REG_ABOVE_64_WORD_SET(reg_above_64, 1, 0);

    brdc_blk_info = sal_alloc(sizeof(*brdc_blk_info) * BRDC_BLOCKS_TEST_MAX_BLOCKS, "brdc_blk_test.brdc_blk_info");
    if(brdc_blk_info == NULL) {
        cli_out("Memory allocation failure\n");
        return CMD_FAIL;
    }
    rv = brdc_blk_test_info_get(unit, BRDC_BLOCKS_TEST_MAX_BLOCKS ,brdc_blk_info, &count);
    if (rv < 0)
    {
        LOG_ERROR(BSL_LS_APPL_TESTS,
                    (BSL_META_U(unit, "brdc_blk_test: ERROR: unit %d : %s\n"), unit, soc_errmsg(rv)));
        sal_free(brdc_blk_info);
        return rv;
    }

    for (test_block = 0; test_block < count; test_block++)
    {

        brdc_block = brdc_blk_info[test_block].blk_type;

        /*Iterate over all relevant block registers (brdc_block)*/
        for (reg = 0; reg < NUM_SOC_REG; reg++) 
        {
            if (!SOC_REG_IS_VALID(unit, reg)) 
            {
                continue;
            }
     
            if (SOC_BLOCK_IN_LIST(SOC_REG_INFO(unit, reg).block, brdc_block)) 
            {
#if defined(SOC_NO_NAMES)
                sal_sprintf(buffer, "%d", reg);
                reg_name = buffer;
#else
                reg_name = SOC_REG_NAME(unit, reg);
#endif
                /*register filter*/
                rv = brdc_blk_test_reg_filter(unit, reg, &is_filter);
                if (rv < 0)
                {
                    LOG_ERROR(BSL_LS_APPL_TESTS,
                                (BSL_META_U(unit, "brdc_blk_test: ERROR: unit %d, register %s : %s\n"), unit, reg_name, soc_errmsg(rv)));
                    sal_free(brdc_blk_info);
                    return rv;
                }
                if (is_filter)
                {
                    LOG_VERBOSE(BSL_LS_APPL_TESTS,
                        (BSL_META_U(unit, "brdc_blk_test: Filtering unit %d register %s\n"), unit, reg_name));
                    continue;
                }

                LOG_VERBOSE(BSL_LS_APPL_TESTS,
                                (BSL_META_U(unit, "brdc_blk_test: Testing unit %d register %s \n"), unit, reg_name));
                
                /*Iterate over all possible indexes*/
                for (index = 0; index < SOC_REG_INFO(unit, reg).numels; index++)
                {
                    /*write to broadcast block*/
                    rv = soc_reg_above_64_set(unit, reg, 0, index, reg_above_64);
                    if (rv < 0)
                    {
                        LOG_ERROR(BSL_LS_APPL_TESTS,
                                    (BSL_META_U(unit, "brdc_blk_test: ERROR: unit %d, register %s : %s\n"), unit, reg_name, soc_errmsg(rv)));
                        sal_free(brdc_blk_info);
                        return rv;
                    }

                    /*read all relevant blocks and make sure equals*/
                    addr = soc_reg_addr_get(unit, reg, REG_PORT_ANY, index,
                                            SOC_REG_ADDR_OPTION_NONE,
                                            &block_dummy, &acc_type);
                    for (blk_index = 0; brdc_blk_info[test_block].blk_ids[blk_index] != -1 ; blk_index++)
                    {
                        blk_instance = brdc_blk_info[test_block].blk_ids[blk_index]; /*schan number of the relevant blk*/

                        SOC_REG_ABOVE_64_CLEAR(reg_above_64_get);

                         rv = brdc_blk_test_reg_addr_get(unit, reg, acc_type, addr, SOC_BLOCK2SCH(unit, blk_instance), &reg_above_64_get);
                         if (rv < 0)
                         {
                             LOG_ERROR(BSL_LS_APPL_TESTS,
                                        (BSL_META_U(unit, "brdc_blk_test: ERROR: unit %d, register %s : %s\n"), unit, reg_name, soc_errmsg(rv)));
                             result = BCM_E_FAIL;
                             continue;
                         }

                         if (!SOC_REG_ABOVE_64_IS_EQUAL(reg_above_64_get, reg_above_64))
                         {
                             LOG_ERROR(BSL_LS_APPL_TESTS,
                                            (BSL_META_U(unit, "brdc_blk_test: ERROR: unit %d, register %s : %s\n"), unit, reg_name, soc_errmsg(BCM_E_FAIL)));
                             result = BCM_E_FAIL;
                             continue;
                         }
                    } /*blk controled by the brdc blk iteration*/

                } /*index iteration*/
            }

        } /*reg iteration*/

    } /*brdc blk iteration*/

    sal_free(brdc_blk_info);
    return result;
}

#endif /*defined(BCM_DFE_SUPPORT) */

