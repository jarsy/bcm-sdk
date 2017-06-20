/*
 * $Id: util.c,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    ppe.c
 * Purpose: Caladan3 driver utility file
 * Requires:
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/util.h>
#include <soc/sbx/caladan3/util.h>
#include <sal/appl/io.h>

int soc_sbx_caladan3_reg32_expect_field_timeout(int unit,
                                                soc_reg_t reg,/* register */
                                                int blk_instance, /* blk instance */
                                                int index, /* index */
                                                int port, /* port id is applicable */
                                                int field_id, /* field ID */
                                                int field_value, /* field value to wait on */
                                                int wait_time_usec) /* wait time in usec */
{
    int value=0, status = SOC_E_NONE;
    soc_timeout_t  wait;
    uint32 regval;

#ifdef BCM_WARM_BOOT_SUPPORT
    if(SOC_WARM_BOOT(unit))
    {
      return SOC_E_NONE;
    }
#endif /* BCM_WARM_BOOT_SUPPORT */


    blk_instance = (blk_instance >= 0)? SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(blk_instance):0;

    if (wait_time_usec <= 0) {
            wait_time_usec = SOC_SBX_CALADAN3_DEFAULT_WAIT_TIME_USEC;
    }

    soc_timeout_init(&wait, wait_time_usec, 0);

    while (TRUE) {
        if (port >= 0) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, reg, port, 0, &regval));
        } else {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, reg, blk_instance, index, &regval)); 
        }
        LOG_VERBOSE(BSL_LS_SOC_REG,
                    (BSL_META_U(unit,
                                "\nWaiting for event reg %d field %d, regval %x"),
                     reg, field_id, regval));
        if (field_id > 0) {
            value = soc_reg_field_get(unit, reg, regval, field_id);
        } else {
            value = regval;
        }
        if (value == field_value) {
            break;
        } 

        if (soc_timeout_check(&wait)) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "Unit(%d) Polling Register %s Field %s Exp: %x Obs: %x  !!! \n"),
                      unit, SOC_REG_NAME(unit, reg), SOC_FIELD_NAME(unit, field_id),
                      field_value, value));
            status = SOC_E_TIMEOUT;
            break;
        }
    }

    /* fake success on PCID simulation */
    if (status == SOC_E_TIMEOUT && SAL_BOOT_PLISIM) {
        status = SOC_E_NONE;
    }

    return status;
}

int soc_sbx_caladan3_msb_bit_pos(unsigned int val) {
    int i = 0;

    for (i = (sizeof(val) * 8) - 1; i > 0; i--) {
        if (val & (0x1 << i)) {
            return i;
        }
    }

    return 0;
}

int soc_sbx_caladan3_round_power_of_two(int unit, unsigned int *value, uint8 down /*true-round down, false-up*/)
{
    unsigned int val=0;

    if (!value) {
        return SOC_E_PARAM;
    }

    val = *value;

    if(!SOC_SBX_POWER_OF_TWO(*value)) {
        if (!down) {
            val--;
        }

        val |= val >> 1;
        val |= val >> 2;
        val |= val >> 4;
        val |= val >> 8;
        val |= val >> 16;

        if (down) {
            val -= (val >> 1);
        } else {
            val++;
        }

        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "unit %d Allocated table size %d not power of 2. Rounding it to size %d !!!\n"), 
                     unit, *value, val));
        *value = val;
    }

    return SOC_E_NONE;
}

void soc_sbx_caladan3_cmic_endian(uint8 *buffer, uint32 size)
{
    uint32 c;
    int i, j;
    for(j=0,i=size/4-1; i>j; j++, i--) {
        c = *((uint32*)buffer+j);
        *((uint32*)buffer+j) = *((uint32*)buffer+i);
        *((uint32*)buffer+i) = c;
    }
}

int soc_sbx_caladan3_reg32_dump(int unit, int reg, uint32 regval)
{
    soc_reg_info_t *reginfo = &SOC_REG_INFO(unit, reg);
    uint32          val, resval, resfld;
    int             nprint, f;
    char fld_sep[] = ",";

    if (!SOC_IS_CALADAN3(unit)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s:%d Supported only on Caladan3 !!!! \n"), 
                   FUNCTION_NAME(), unit));
    }

    resval = reginfo->rst_val_lo;

    /* print only fields that have been changed from default value */
    LOG_CLI((BSL_META_U(unit,
                        "%s: \n"), SOC_REG_NAME(unit,reg)));
    nprint = 0;
    for (f = reginfo->nFields - 1; f >= 0; f--) {
        soc_field_info_t *fld = &reginfo->fields[f];
        val = soc_reg_field_get(unit, reg, regval, fld->field);
        resfld = soc_reg_field_get(unit, reg, resval, fld->field);
        if (val == resfld) {
            continue;
        }

        if (nprint > 0) {
            LOG_CLI((BSL_META_U(unit,
                                "%s "), fld_sep));
        }

        LOG_CLI((BSL_META_U(unit,
                            "%s= %d "),SOC_FIELD_NAME(unit, fld->field),val));
        nprint += 1;
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n")));
    return SOC_E_NONE;
}

int soc_sbx_caladan3_reg32_reset_val_get(int unit, int reg, 
                                         uint32 *regval, uint32 flags)
{
    soc_reg_info_t *reginfo = &SOC_REG_INFO(unit, reg);
    int             f;
    uint32          field;

    if (!SOC_IS_CALADAN3(unit)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s:%d Supported only on Caladan3 !!!! \n"), 
                   FUNCTION_NAME(), unit));
    }
    
    if (!regval) return SOC_E_PARAM;
    if (flags && (flags != SOCF_W1TC)) return SOC_E_PARAM;

    *regval = reginfo->rst_val_lo;

    if (flags & SOCF_W1TC) {
        for (f = reginfo->nFields - 1; f >= 0; f--) {
            soc_field_info_t *fld = &reginfo->fields[f];
            
            if (fld->flags & SOCF_W1TC) {
                field = 1;
                soc_reg_field_set(unit, reg, regval, fld->field, field);
            }
        }
    }

    return SOC_E_NONE;
}

#endif
