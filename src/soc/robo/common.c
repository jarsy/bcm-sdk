/*
 * $Id: common.c,v 1.24 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        soc_common.c
 * Purpose:     Common functions for soc drivers
 * Requires:   
 */

#include <assert.h>
#include <sal/core/libc.h>
#include <soc/robo/mcm/driver.h>
#include <soc/robo/mcm/memregs.h>
#include <soc/error.h>
#include <soc/cm.h>
#include <soc/debug.h>
#include <soc/register.h>
#include <soc/field.h>

/*
 * Function:
 *     soc_robo_regaddrinfo_get
 * Purpose:
 *     Determine register information based on an address.
 * Parameters:
 *     unit  - RoboSwitch unit number.
 *     ainfo - (OUT)Address info describing register.
 *     addr  - Internal SOC address of register to analyze.
 * Returns:
 *      SOC_E_XXX
 */
void
soc_robo_regaddrinfo_get(int unit, soc_regaddrinfo_t *ainfo, uint32 addr)
{
    int reg, temp_port_shift;
    uint32 page, tmp_page, offset;
    uint32 reg_page, reg_offset;
    int reg_nbyte, reg_numels;

    ainfo->addr = addr;
    ainfo->valid = 1;
    ainfo->reg = INDEX(INVALID_Rr);
    temp_port_shift = 0;

    tmp_page = page = (addr >> SOC_ROBO_PAGE_BP) & 0xFF;
    if(SOC_IS_ROBO53242(unit)){
#ifdef BCM_HARRIER_SUPPORT
        if (((page >= 0xa0) && (page <= 0xb7)) ||
        	((page >= 0xd8) && (page <= 0xdc))) {
            /* Internal Port MII Registers */
            /* External MII/IMP/GMII Registers*/
            tmp_page = 0xa0;
        }else if ((page >= 0x68) && (page <= 0x84)) { 
            /* Port MIB Registers */            
            tmp_page = 0x68;
        }
#endif
    } else if(SOC_IS_ROBO53262(unit)){
#ifdef BCM_HARRIER_SUPPORT
        if (((page >= 0xa0) && (page <= 0xb7)) ||
        	((page >= 0xd8) && (page <= 0xdc))) {
            /* Internal Port MII Registers */
            /* External MII/IMP/GMII Registers*/
            tmp_page = 0xa0;
        }else if ((page >= 0x68) && (page <= 0x84)) { 
            /* Port MIB Registers */            
            tmp_page = 0x68;
        }else if ((page >= 0xb8) && (page <= 0xbc)) { 
            /* Internal serdes Registers */            
            tmp_page = 0xb8;
        }
#endif
    } else if(SOC_IS_TB(unit)){
#ifdef BCM_TB_SUPPORT
        if ((page >= 0xa0) && (page <= 0xb7)){
            /* Internal Port MII Registers */
            tmp_page = 0xa0;
        }else if ((page >= 0xb9) && (page <= 0xbc)) { 
            /* Internal serdes Registers */            
            tmp_page = 0xb9;
        } else if ((page >= 0xd8) && (page <= 0xdc)) {
            /* External MII/IMP/GMII Registers*/
            tmp_page = 0xd8;
        }
#endif /* BCM_TB_SUPPORT */
    } else if (SOC_IS_VO(unit)) {
#ifdef BCM_VO_SUPPORT
        if ((page >= 0xa0) && (page <= 0xb7)){
            /* Internal Port MII Registers */
            tmp_page = 0xa0;
        }else if ((page >= 0xc0) && (page <= 0xd7)) { 
            /* External Port MII Registers */
            tmp_page = 0xc0;
        }else if ((page >= 0xbb) && (page <= 0xbc)) { 
            /* Internal serdes Registers for ge2 /ge3 
             * Hence add temp_port_shift.
             */            
            tmp_page = 0xbb;
            temp_port_shift = 2;
        } else if ((page >= 0xd8) && (page <= 0xdc)) {
            /* External MII/IMP/GMII Registers*/
            tmp_page = 0xd8;
        }
#endif /* BCM_VO_SUPPORT */
    } else if (SOC_IS_POLAR(unit)) {
#ifdef BCM_POLAR_SUPPORT
        if (page == 0x15) { /* Port SERDES Registers */
            tmp_page = 0x15;
        } else if ((page >= 0x10) && (page <= 0x18)) { /* Port MII Registers */
            tmp_page = 0x10;
        } else if ((page >= 0x20) && (page <= 0x28)) { /* Port MIB Registers */
            tmp_page = 0x20;
        } else if ((page >= 0x80) && (page <= 0x88)) { /* external phy Registers */
            tmp_page = 0x80;
        } 
#endif /* BCM_POLAR_SUPPORT */
    } else if (SOC_IS_DINO8(unit)) {
#ifdef BCM_DINO8_SUPPORT
       if ((page >= 0x10) && (page <= 0x18)) { /* Port MII Registers */
            tmp_page = 0x10;
        } else if ((page >= 0x20) && (page <= 0x28)) { /* Port MIB Registers */
            tmp_page = 0x20;
        } else if ((page >= 0x80) && (page <= 0x88)) { /* external phy Registers */
            tmp_page = 0x80;
        } 
#endif /* BCM_DINO8_SUPPORT */
    } else if (SOC_IS_DINO16(unit)) {
#ifdef BCM_DINO16_SUPPORT
       if ((page >= 0x10) && (page <= 0x1F)) { /* Port MII Registers */
            tmp_page = 0x10;
        } else if ((page >= 0x50) && (page <= 0x60)) { /* Port MIB Registers */
            tmp_page = 0x50;
        } else if ((page >= 0x80) && (page <= 0x8F)) { /* external phy Registers */
            tmp_page = 0x80;
        } 
#endif /* BCM_DINO16_SUPPORT */
    } else {
        if ((page >= 0x10) && (page <= 0x18)) { /* Port MII Registers */
            tmp_page = 0x10;
        }
        else if ((page >= 0x20) && (page <= 0x28)) { /* Port MIB Registers */
            tmp_page = 0x20;
        }
    }
    ainfo->field = INVALID_Rf;
    ainfo->cos = -1;
    ainfo->port = -1;
    ainfo->idx = -1;

    offset = addr & 0xFF;

    for (reg = 0; reg < NUM_SOC_ROBO_REG; reg++) {
         if (&SOC_REG_INFO(unit,reg) == NULL) {            
            continue;
        }
        if (!SOC_REG_IS_VALID(unit, reg)) {
            continue;
        }
        reg_page = (SOC_REG_INFO(unit, reg).offset >> SOC_ROBO_PAGE_BP) & 0xFF;
        reg_nbyte = (DRV_SERVICES(unit)->reg_length_get)(unit, reg);
        reg_numels = SOC_REG_INFO(unit, reg).numels;
        reg_offset = SOC_REG_INFO(unit, reg).offset & 0xFF;
        if (tmp_page == reg_page) {
            if ((offset == reg_offset) ||
                ((offset > reg_offset) &&
                 (offset < (reg_offset + reg_numels*reg_nbyte)))) {
                ainfo->reg = reg;
                if (reg_numels >=8 && reg_numels <= 10) {
                    ainfo->port = offset - reg_offset + temp_port_shift;
                }
                else if (reg_numels > 1) {
                    ainfo->idx = offset - reg_offset;
                }
                break;
            }
        }
    }
    return;
}

/*
 * Function:
 *     soc_robo_anyreg_read
 * Purpose:
 *     Read an arbitrary (8-bit, 16-bit, 32-bit, 48-bit or 64-bit) 
 *     internal SOC register.
 * Parameters:
 *     unit  - RoboSwitch unit number.
 *     ainfo - Address info indicating register.
 *     data  - (OUT)Result stored here.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_robo_anyreg_read(int unit, soc_regaddrinfo_t *ainfo, uint64 *data)
{
    int rv = SOC_E_NONE;
    int len;
    uint64 val64;

    len = (DRV_SERVICES(unit)->reg_length_get)(unit, ainfo->reg);

    if (len < 1) {
        return SOC_E_PARAM;
    }
    rv = (DRV_SERVICES(unit)->reg_read)
        (unit, (uint32)ainfo->addr, (uint32 *)data, len);

    if (len <= 4) {
        COMPILER_64_SET(val64, 0, *(uint32 *)data);
        COMPILER_64_SET(*data, 0, 0);
        COMPILER_64_OR(*data, val64);
     }

    if (len < 8) {
        COMPILER_64_SET(val64, 0, 1);
        COMPILER_64_SHL(val64, 8 * len);
        COMPILER_64_SUB_32(val64, 1);
        COMPILER_64_AND(*data, val64);
    }

    return rv;
}

/*
 * Function:
 *      soc_robo_anyreg_write
 * Purpose:
 *      Write an arbitrary (8-bit, 16-bit, 32-bit, 48-bit or 64-bit) 
 *      internal SOC register.
 * Parameters:
 *      unit  - soc unit number
 *      ainfo - Address info indicating register.
 *      data  - Value to write.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      This routine calls the appropriate accessor based on the register 
 *      information in ainfo. If a 32-bit register is written, the upper 
 *      32 bits of data are ignored.
 */

int
soc_robo_anyreg_write(int unit, soc_regaddrinfo_t *ainfo, uint64 data)
{
    int rv = SOC_E_NONE;
    int len;
#ifdef BE_HOST
    uint32 val32=0;
#endif

    len = (DRV_SERVICES(unit)->reg_length_get)(unit, ainfo->reg);

    if (len < 1) {
        return SOC_E_PARAM;
    }
#ifdef BE_HOST
    if (len <= 4) {
        COMPILER_64_TO_32_LO(val32, data);
        rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (uint32)ainfo->addr, &val32, len);
    } else {
        rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (uint32)ainfo->addr, &data, len);
    }
#else
    rv = (DRV_SERVICES(unit)->reg_write)
        (unit, (uint32)ainfo->addr, &data, len);
#endif

    return rv;
}

#define UPDATE_BP(bp) while (*(bp)) (bp)++;

void
soc_robo_reg_sprint_addr(int unit, char *bp, soc_regaddrinfo_t *ainfo)
{
    soc_field_info_t *finfop = 0;
    int msb, lsb;
    soc_reg_info_t *reginfo;

    if (!ainfo->valid) {
        sal_sprintf(bp, "Invalid Address");
        return;
    }
    reginfo = &SOC_REG_INFO(unit, ainfo->reg);
    if (ainfo->reg != INDEX(INVALID_Rr) && ainfo->field != INVALID_Rf) {
        SOC_FIND_FIELD(ainfo->field, reginfo->fields,
                       reginfo->nFields, finfop);
        assert(finfop);
        if (!finfop) {
            sal_sprintf(bp, "Invalid field");
            return;
        }
        if (finfop->flags & SOCF_LE) {
            msb = finfop->bp + finfop->len - 1;
            lsb = finfop->bp;
        } else {
            msb = finfop->bp;
            lsb = finfop->bp + finfop->len - 1;
        }
        sal_sprintf(bp, "[%d:%d] ", msb, lsb);
        UPDATE_BP(bp);
    }

    if (ainfo->reg == INDEX(INVALID_Rr)) {
        sal_sprintf(bp, "Reserved");
        return;
    }

#ifdef SOC_NO_NAMES
    sal_sprintf(bp, "#%d", ainfo->reg);
#else
    sal_sprintf(bp, "%s", SOC_ROBO_REG_NAME(uint, ainfo->reg));
#endif /* SOC_NO_NAMES */
    UPDATE_BP(bp);

    if (ainfo->idx != -1) {
        sal_sprintf(bp, "(%d)", ainfo->idx);
        UPDATE_BP(bp);
    }
    if (ainfo->cos != -1) {
        sal_sprintf(bp, ".%d", ainfo->cos);
        UPDATE_BP(bp);
    }
    if (ainfo->port != -1) {
        /* Need SOC_PORT_NAME information */
        *bp++ = '.';
        sal_sprintf(bp, "%s", SOC_PORT_NAME(unit, ainfo->port));
        UPDATE_BP(bp);
    }
    if (finfop) {
#ifdef SOC_NO_NAMES
        sal_sprintf(bp, ".#%d", ainfo->field);
#else
        sal_sprintf(bp, ".%s", soc_robo_fieldnames[ainfo->field]);
#endif /* SOC_NO_NAMES */
        UPDATE_BP(bp);
    }
}
#undef	UPDATE_BP

/*
 * Function:
 *      soc_robo_reg_iterate
 * Purpose:
 *      Apply a function to each register in the device.
 * Parameters:
 *      unit  - soc unit number
 *      do_it - Function to apply to each register.
 *      data  - Anonymous cookie passed to do_it.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      soc_reg_iter_f is given by the following prototype.
 *      typedef int (*soc_reg_iter_f)(int unit, soc_regaddrinfo_t 
 *	*ainfo, void *data) The function passed to soc_robo_reg_iterate 
 *	may not affect all registers in the device.
 */

int
soc_robo_reg_iterate(int unit, soc_reg_iter_f do_it, void *data)
{
    soc_reg_t reg;
    soc_port_t port;
    soc_regaddrinfo_t ainfo;
    soc_block_t blk = 0;
    soc_block_types_t regblktype;
    int numels, idx = 0;
    pbmp_t bm;
    int rv = SOC_E_NONE;

    ainfo.valid = 1;
    ainfo.field = INVALID_Rf;

    for (reg = 0; reg < NUM_SOC_ROBO_REG; reg++) {
         if (&SOC_REG_INFO(unit,reg) == NULL) {            
            continue;
        }
        if (!SOC_REG_IS_VALID(unit, reg)) {
            continue;
        }
        ainfo.reg = reg;
        numels = SOC_REG_NUMELS(unit, reg);
        regblktype = SOC_REG_INFO(unit, reg).block;
        switch (SOC_REG_INFO(unit, reg).regtype) {
        case soc_portreg:
            if (SOC_BLOCK_IS(regblktype,SOC_BLK_SYS)) {
                ainfo.port = -1;
                ainfo.idx = -1;
                ainfo.addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, reg, 0, 0);
                ainfo.cos = -1;
                rv = (*do_it)(unit, &ainfo, data);
            } else {                
                PBMP_PORT_ITER(unit, port) {
                    ainfo.port = port;
                    ainfo.idx = -1;
                    ainfo.addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, reg, ainfo.port, 0);
                    ainfo.cos = -1;
                    rv = (*do_it)(unit, &ainfo, data);
                }
            }
            break;
        case soc_phy_reg:
            if (SOC_BLOCK_IN_LIST(regblktype,SOC_BLK_PORT) ||
                SOC_BLOCK_IS(regblktype,SOC_BLK_INTER) ||
                SOC_BLOCK_IS(regblktype,SOC_BLK_EXTER) ) {
                    SOC_PBMP_CLEAR(bm);
                    SOC_BLOCKS_ITER(unit, blk, regblktype) {
                        SOC_PBMP_OR(bm, SOC_BLOCK_BITMAP(unit, blk));
                    }
            }else {
                SOC_PBMP_ASSIGN(bm, PBMP_ALL(unit));
            }

            PBMP_ITER(bm, port) {
                ainfo.port = port;
                ainfo.idx = -1;
                ainfo.addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, reg, ainfo.port, 0);
                ainfo.cos = -1;
                rv = (*do_it)(unit, &ainfo, data);
            }
            break;
        case soc_cosreg:
            break;
        case soc_genreg:
            if (SOC_BLOCK_IS(regblktype,SOC_BLK_SYS)) {
                if (numels == 1) {
                    ainfo.port = -1;
                    ainfo.idx = -1;
                    ainfo.addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, reg, 0, 0);
                    ainfo.cos = -1;
                    rv = (*do_it)(unit, &ainfo, data);
                } else {
                    for (idx = 0; idx < numels; idx++) {
                        ainfo.port = -1;
                        ainfo.idx = idx;
                        ainfo.addr = (DRV_SERVICES(unit)->reg_addr)
                            (unit, reg, 0, idx);
                        ainfo.cos = -1;
                        rv = (*do_it)(unit, &ainfo, data);
                    }
                }
            } else {
                if(SOC_BLOCK_IN_LIST(regblktype,SOC_BLK_PORT)||
                    SOC_BLOCK_IS(regblktype,SOC_BLK_EXTER)){
                    SOC_PBMP_CLEAR(bm);
                    SOC_BLOCKS_ITER(unit, blk, regblktype) {
                        SOC_PBMP_OR(bm, SOC_BLOCK_BITMAP(unit, blk));
                    }
                } else {
                    SOC_PBMP_ASSIGN(bm, PBMP_ALL(unit));
                }
                PBMP_ITER(bm, port) {
                    ainfo.port = port;
                    ainfo.idx = -1;
                    ainfo.addr = (DRV_SERVICES(unit)->reg_addr)
                        (unit, reg, ainfo.port, 0);
                    ainfo.cos = -1;
                    rv = (*do_it)(unit, &ainfo, data);
                }
            }
            break;
        case soc_spi_reg:
            /* Avoid to dump the SPI related registers */
            break;
        case soc_cpureg:
            /* Skip CPU registers in soc iteration. */
            break;
        default:
            assert(0);
            break;
        }
    }

    return rv;
}
