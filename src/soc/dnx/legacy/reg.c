/* 
 * $Id: reg.c,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC register access implementation for DNX
 */

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_REG
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>

#include <soc/drv.h>
#include <shared/util.h>

/* Read a SOC register */
int
soc_dnx_reg32_read(int unit, uint32 addr, uint32 *data) {
    LOG_INFO(BSL_LS_SOC_REG,
             (BSL_META_U(unit,
                         "soc_dnx_reg32_read called. Addr:0x%x\n"), addr));
    
    addr = WORDS2BYTES(addr & (~0x00080000));

    *data = CMREAD(unit, addr);

    LOG_INFO(BSL_LS_SOC_REG,
             (BSL_META_U(unit,
                         "soc_dnx_reg32_read Addr:0x%x returned val:"
                         "0x%08x\n"), addr,*data));
    return SOC_E_NONE;
}

/* Read a 64-bit SOC register */
int
soc_dnx_reg64_read(int unit, uint32 addr, uint64 *data) {
    uint32  data_hi;
    uint32  data_lo;
    uint32  offset;

    LOG_INFO(BSL_LS_SOC_REG,
             (BSL_META_U(unit,
                         "soc_dnx_reg64_read called. Addr:0x%x\n"), addr));
    
    addr = WORDS2BYTES(addr & (~0x00080000));

    offset = 4;
    data_hi = CMREAD(unit, addr);
    data_lo = CMREAD(unit, (addr + offset));

    COMPILER_64_SET(*data, data_hi, data_lo);
    return SOC_E_NONE;
}


/* Write to a SOC register */
int
soc_dnx_reg32_write(int unit, uint32 addr, uint32 data) {

    LOG_INFO(BSL_LS_SOC_REG,
             (BSL_META_U(unit,
                         "soc_dnx_reg32_write called. Addr:0x%x, "
                         "data:0x%08x\n"), addr, data));
    
    addr = WORDS2BYTES(addr & (~0x00080000));

    CMWRITE(unit, addr, data);

    return SOC_E_NONE;
}

/* Write to a 64-bit SOC register */
int
soc_dnx_reg64_write(int unit, uint32 addr, uint64 data) {
    uint32  offset;

    LOG_INFO(BSL_LS_SOC_REG,
             (BSL_META_U(unit,
                         "soc_dnx_reg64_write called. Addr:0x%x\n"), addr));
    
    addr = WORDS2BYTES(addr & (~0x00080000));

    offset = 4;
    CMWRITE(unit, addr, (COMPILER_64_HI(data)));
    CMWRITE(unit, (addr + offset), COMPILER_64_LO(data));

    return SOC_E_NONE;
}


/* Read SOC register. Checks if the register is 32 or 64 bits */
int
soc_dnx_reg_read(int unit, soc_reg_t reg, uint32 addr, uint64 *data) {
    if (!SOC_REG_IS_VALID(unit, reg)) {
        return SOC_E_PARAM;
    }

    if (SOC_REG_IS_64(unit, reg)) {
        return soc_dnx_reg64_read(unit, addr, data);
    } else {
        uint32 data32;

        DNXC_IF_ERR_RETURN(soc_dnx_reg32_read(unit, addr, &data32));
        COMPILER_64_SET(*data, 0, data32);
    }

    return SOC_E_NONE;
}

/* Write SOC register. Checks if the register is 32 or 64 bits */
int
soc_dnx_reg_write(int unit, soc_reg_t reg, uint32 addr, uint64 data) {
    if (!SOC_REG_IS_VALID(unit, reg)) {
        return SOC_E_PARAM;
    }

    if (SOC_REG_IS_64(unit, reg)) {
        return soc_dnx_reg64_write(unit, addr, data);
    } else {
        DNXC_IF_ERR_RETURN(soc_dnx_reg32_write(unit, addr,
                                                COMPILER_64_LO(data)));
    }

    return SOC_E_NONE;
}
