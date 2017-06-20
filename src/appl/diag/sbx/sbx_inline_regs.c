/*
 * $Id: sbx_inline_regs.c,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        sbx_inline_regs.c
 * Purpose:     Split out from sbx.c to cut down on compile time.
 *
 *  This file includes several massisve auto-generated functions to add
 *  the various chips to the diag shell.  Each taking several seconds to
 *  compile, causing long compile times for minor changes to sbx.c.
 */

#include <shared/bsl.h>

#include <soc/drv.h>
#include <sal/appl/sal.h>

#ifdef BCM_FE2000_SUPPORT
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#endif
#ifdef BCM_QE2000_SUPPORT
#include <soc/sbx/hal_ka_auto.h>
#endif
#ifdef BCM_BME3200_SUPPORT
#include <soc/sbx/hal_pt_auto.h>
#endif
#ifdef BCM_BM9600_SUPPORT
#include <soc/sbx/hal_pl_auto.h>
#endif

#include <appl/diag/sbx/register.h>



soc_sbx_chip_info_t soc_sbx_chip_list[SOC_MAX_NUM_DEVICES];
int soc_sbx_chip_count;


/*
* Function: sbx_str_tolower
* Purpose:  String utility function to do inplace converion
*           of  string to uppercase
*/
void
sbx_str_tolower(char *str)
{
    int idx = 0;

    if (!str) {
        return;
    }

    for (idx = 0; idx < strlen(str); idx++) {
        *(str+idx) = tolower((unsigned char)*(str+idx));
    }
}

#if (defined(BCM_FE2000_SUPPORT) || defined(BCM_QE2000_SUPPORT) || \
     defined(BCM_BME3200_SUPPORT) || defined(BCM_BM9600_SUPPORT))
static soc_sbx_chip_info_t*
sbx_chip_info_add(int chip_id, char *dev)
{
    soc_sbx_chip_info_t     *chip_info = NULL;
    if (sbx_chip_info_get(chip_id, &chip_info,0) != CMD_OK) {
        /* new Device type */
        if (soc_sbx_chip_count >= SOC_MAX_NUM_DEVICES) {
            cli_out("ERROR: sbx_chip_add. Number of devices > "
                    "SOC_MAX_NUM_DEVICES \n");
            assert(0);
        }
        chip_info = &soc_sbx_chip_list[soc_sbx_chip_count++];
        sal_strncpy(chip_info->name, dev, SBX_DEV_NAME_LEN_MAX-1);
        sbx_str_tolower(chip_info->name);
        /*coverity [overrun-local]*/
        chip_info->id = chip_id;
    }
    return chip_info;
}

static void
add_register(int chip_id, char *dev, char *reg, uint32 offset,
             uint32 mask, uint8 msb, uint8 lsb,
             int intreg)
{
    soc_sbx_chip_info_t *chip_info = NULL;
    soc_sbx_reg_info_t *reg_info = NULL;
    uint32 ioffset;
    int i, inst, group COMPILER_ATTRIBUTE((unused));
    char buf[256];
    char *group_name COMPILER_ATTRIBUTE((unused));
    char *n;
    int gen_regtype  COMPILER_ATTRIBUTE((unused));
#ifdef BCM_FE2000_SUPPORT
    char name[256];
#endif

    group_name = NULL;
    gen_regtype= FALSE;
    if (sbx_chip_info_get(chip_id, &chip_info,0 ) != CMD_OK) {
        /* new Device type */
        if (soc_sbx_chip_count >= SOC_MAX_NUM_DEVICES) {
            cli_out("ERROR: addRegister failed. Number of devices > "
                    "SOC_MAX_NUM_DEVICES \n");
            return;
        }
        chip_info = &soc_sbx_chip_list[soc_sbx_chip_count++];
        sal_strncpy(chip_info->name, dev, SBX_DEV_NAME_LEN_MAX-1);
        sbx_str_tolower(chip_info->name);
        chip_info->id = chip_id;
    }

    group = 0;
    inst = 1;
    if ( (chip_id == SOC_INFO_CHIP_TYPE_FE2000) ||
         (chip_id == SOC_INFO_CHIP_TYPE_FE2000XT) ){
        if (reg[0] == 'x' && reg[1] == 'm') {
            group_name = "xg";
            group = 1;
            inst = (chip_id == SOC_INFO_CHIP_TYPE_FE2000XT) ? 4 : 2;
        } else if (reg[0] == 'g' && reg[1] == 'p' && reg[2] == 'o'
                   && reg[3] == 'r' && reg[4] == 't') {
            gen_regtype = TRUE;  /* GPORT block registers are GEN_REGTYPE */
            group_name = "ag";
            group = 2;
            inst = 2;
        } else if (reg[0] == 'a' && reg[1] == 'm') {
            group_name = "ag";
            group = 2;
            inst = 2;
        }
    }

    ioffset = offset;
    for (i = 0; i < inst; i++) {
        if (chip_info->nregs >= chip_info->maxreg) {
            cli_out("ERROR: addRegister failed. Number of regs greater"
                    " than SBX_REG_NUM_MAX \n");
            return;
        }

#ifdef BCM_FE2000_SUPPORT

        if ( (chip_id == SOC_INFO_CHIP_TYPE_FE2000 ||
              chip_id == SOC_INFO_CHIP_TYPE_FE2000XT) && group > 0) {

          if (chip_id == SOC_INFO_CHIP_TYPE_FE2000XT) {
            offset = SOC_SBX_REG_IS_INDIRECT
                | (group > 1
                   ? (SAND_HAL_C2_AG0_GPORT_MEM_ACC_CTRL_OFFSET
                      + i * SAND_HAL_C2_AG_INSTANCE_ADDR_STRIDE)
                   : (SAND_HAL_C2_XG_MAC_ACC_CTRL_OFFSET
                      + i * SAND_HAL_C2_XG_INSTANCE_ADDR_STRIDE));

          } else {
            offset = SOC_SBX_REG_IS_INDIRECT
                | (group > 1
                   ? (SAND_HAL_CA_AG0_GPORT_MEM_ACC_CTRL_OFFSET
                      + i * SAND_HAL_CA_AG_INSTANCE_ADDR_STRIDE)
                   : (SAND_HAL_CA_XG_MAC_ACC_CTRL_OFFSET
                      + i * SAND_HAL_CA_XG_INSTANCE_ADDR_STRIDE));
          }

            /* coverity[secure_coding] */
	    /* coverity[dead_error_line] */
            sal_sprintf(name, "%s%d_%s",
                    (group_name)?group_name:"invalid", i, reg);
            n = name;

            /* Set GEN_REGTYPE */
            if (gen_regtype) {
                ioffset |= 0x00080000;
            }
        } else
#endif /* BCM_FE2000_SUPPORT */
        {
            n = reg;
        }
        /* coverity[secure_coding] */
        sal_sprintf(buf, "%s_%s", dev, n);
        reg_info = sal_alloc(sizeof (soc_sbx_reg_info_t), buf);
        if (!reg_info) {
            cli_out("ERROR: alloc for register info failed\n");
            return;
        }
        /* Zero out reg_info struct. */
        sal_memset(reg_info, 0, sizeof(soc_sbx_reg_info_t));

        sal_strncpy(reg_info->name, n, SBX_REG_NAME_LEN_MAX-1);
        sbx_str_tolower(reg_info->name);
        reg_info->offset = offset;
        reg_info->ioffset = ioffset;
        reg_info->mask = mask;
        reg_info->msb = msb;
        reg_info->lsb = lsb;
        reg_info->intreg = intreg;
        chip_info->regs[chip_info->nregs++] = reg_info;
    }
}


static void
add_field(int chip_id, char *dev, char *reg, char *field, uint32 mask,
          uint32 shift, uint8 msb, uint8 lsb, uint32 type, uint32 default_val)
{
    int idx = 0;
    soc_sbx_chip_info_t     *chip_info = NULL;
    soc_sbx_reg_info_t      *reg_info[4] ={NULL, NULL, NULL, NULL};
    soc_sbx_field_info_t    *fld_info = NULL;
    char buf[256];
    char name[256];
    int i, inst = 0;
    char *n;

    if (sbx_chip_info_get(chip_id, &chip_info,0) != CMD_OK) {
        cli_out("ERROR: add_field failed. chip_id:%d not found\n", chip_id);
        return;
    }

    for (idx = 0; idx < chip_info->nregs; idx++) {
        n = chip_info->regs[idx]->name;
        if ( ((chip_id == SOC_INFO_CHIP_TYPE_FE2000) || (chip_id == SOC_INFO_CHIP_TYPE_FE2000XT))
            && (((tolower((unsigned char)(n[0])) == 'x')
                 || (tolower((unsigned char)(n[0])) == 'a'))
                && (tolower((unsigned char)(n[1])) == 'g'))
            && !sal_strcasecmp(&n[4], reg)) {
            reg_info[n[2] - '0'] = chip_info->regs[idx];
            if ( ((tolower((unsigned char)(n[0]))) == 'x') && 
                 (chip_id == SOC_INFO_CHIP_TYPE_FE2000XT) ) {
                inst = 4;
                if (reg_info[0] && reg_info[1] && reg_info[2] && reg_info[3]) {
                    break;
                }
            }else{
                inst = 2;
                if (reg_info[0] && reg_info[1]) {
                    break;
                }
            }
        } else if (!sal_strcasecmp(n, reg)) {
            reg_info[0] = chip_info->regs[idx];
            inst = 1;
            break;
        }
    }

    if (!inst) {
        cli_out("ERROR: addField failed. Adding field info before adding "
                "register type. \n");
        return;
    }

    if (inst == 2 && (!reg_info[0] || !reg_info[1])) {
        cli_out("ERROR: addField failed. only found 1 of 2 instances of"
                " register %s\n", reg);
        return;
    }
    if (inst == 4 && (!reg_info[0] || !reg_info[1] || !reg_info[2] || !reg_info[3])) {
        i = 0;
        if (reg_info[0]) i++;
        if (reg_info[1]) i++;
        if (reg_info[2]) i++;
        if (reg_info[3]) i++;
        cli_out("ERROR: addField failed. only found %d of 4 instances of"
                " register %s\n", i, reg);
        return;
    }

    for (i = 0; i < inst; i++) {
        n = reg;
        if (inst > 1) {
            /* coverity[secure_coding] */
            sal_sprintf(name, "%c%c%d%s", reg[0], reg[1], i, &reg[2]);
            n = name;
        } else {
            n = reg;
        }

        /* coverity[secure_coding] */
        sal_sprintf(buf, "%s_%s_%s", dev, n, field);
        fld_info = sal_alloc(sizeof (soc_sbx_field_info_t), buf);
        sal_memset(fld_info, 0, sizeof(soc_sbx_field_info_t));
        sal_strncpy(fld_info->name, field, SBX_REG_FIELDS_MAX-1);
        sbx_str_tolower(fld_info->name);
        fld_info->mask = mask;
        fld_info->shift = shift;
        fld_info->msb = msb;
        fld_info->lsb = lsb;
        fld_info->type = type;
        fld_info->default_val = default_val;
        reg_info[i]->fields[reg_info[i]->nfields] = fld_info;
        reg_info[i]->nfields++;

        if (reg_info[i]->nfields >= SBX_REG_FIELDS_MAX) {
            cli_out("ERROR: addField failed. Number of fields greater than "
                    "SBX_REG_FIELDS_MAX \n");
        }
    }
}
#endif /* FE2000 | QE2000 | BM3200 | BM9600 */

int
sbx_qe2000_reg_list_init(void)
{
    int rv = 0;

#if defined(BCM_QE2000_SUPPORT)

    soc_sbx_chip_info_t     *chip_info = sbx_chip_info_add(SOC_INFO_CHIP_TYPE_QE2000, "KA");
    chip_info->regs = sal_alloc(sizeof(soc_sbx_reg_info_t *) * 2046, "qe2k regs");
    if (chip_info->regs == NULL)
      return rv;
    chip_info->maxreg = 2046;

  /*
  ** Redefined the macro SAND_HAL_ADD_REG so that a register record
  ** is entered into the device database.
  */
#undef SAND_HAL_ADD_REG
#define SAND_HAL_ADD_REG(device, sDevice, reg, sReg, offset, \
                         no_test_mask, mask, msb, lsb, intreg) \
    add_register(SOC_INFO_CHIP_TYPE_QE2000, sDevice, sReg, offset, mask, msb, lsb, \
                intreg);

  /*
  ** Redefined the macro SAND_HAL_ADD_FIELD so that a bit field is added
  ** to the specified register.
  */
#undef SAND_HAL_ADD_FIELD
#define SAND_HAL_ADD_FIELD(device,sDevice, reg, sReg, field, sField, \
                           mask, shift, msb,lsb,type,defaultVal) \
    add_field(SOC_INFO_CHIP_TYPE_QE2000, sDevice, sReg, sField, mask, shift, \
             msb, lsb, type, defaultVal);

#undef SAND_HAL_KA_AUTO_H
#undef HAL_KA_INLINE_H
#include <soc/sbx/hal_ka_inline.h>

#endif
    return rv;

}

int
sbx_bme3200_reg_list_init(void)
{
    int rv = 0;

#if defined(BCM_BME3200_SUPPORT)

    soc_sbx_chip_info_t     *chip_info = sbx_chip_info_add(SOC_INFO_CHIP_TYPE_BME3200, "PT");
    chip_info->regs = sal_alloc(sizeof(soc_sbx_reg_info_t *) * 1725, "bme regs");
    if (chip_info->regs == NULL)
      return rv;
    chip_info->maxreg = 1725;

  /*
  ** Redefined the macro SAND_HAL_ADD_REG so that a register record
  ** is entered into the device database.
  */
#undef SAND_HAL_ADD_REG
#define SAND_HAL_ADD_REG(device, sDevice, reg, sReg, offset, \
                         no_test_mask, mask, msb, lsb, intreg) \
    add_register(SOC_INFO_CHIP_TYPE_BME3200, sDevice, sReg, offset, mask, msb, \
                lsb, intreg);

  /*
  ** Redefined the macro SAND_HAL_ADD_FIELD so that a bit field is added
  ** to the specified register.
  */
#undef SAND_HAL_ADD_FIELD
#define SAND_HAL_ADD_FIELD(device,sDevice, reg, sReg, field, sField, \
                           mask, shift, msb,lsb,type,defaultVal) \
    add_field(SOC_INFO_CHIP_TYPE_BME3200, sDevice, sReg, sField, mask, shift, \
             msb, lsb, type, defaultVal);

#undef SAND_HAL_PT_AUTO_H
#undef HAL_PT_INLINE_H
#include <soc/sbx/hal_pt_inline.h>

#endif
    return rv;
}

int
sbx_bm9600_reg_list_init(void)
{
    int rv = 0;

#if defined(BCM_BM9600_SUPPORT)
    soc_sbx_chip_info_t     *chip_info = sbx_chip_info_add(SOC_INFO_CHIP_TYPE_BM9600, "PL");
    chip_info->regs = sal_alloc(sizeof(soc_sbx_reg_info_t *) * 5500, "bm regs");
    if (chip_info->regs == NULL)
      return rv;
    chip_info->maxreg = 5500;

  /*
  ** Redefined the macro SAND_HAL_ADD_REG so that a register record
  ** is entered into the device database.
  */
#undef SAND_HAL_ADD_REG
#define SAND_HAL_ADD_REG(device, sDevice, reg, sReg, offset, \
                         no_test_mask, mask, msb, lsb, intreg) \
    add_register(SOC_INFO_CHIP_TYPE_BM9600, sDevice, sReg, offset, mask, msb, \
                lsb, intreg);

  /*
  ** Redefined the macro SAND_HAL_ADD_FIELD so that a bit field is added
  ** to the specified register.
  */
#undef SAND_HAL_ADD_FIELD
#define SAND_HAL_ADD_FIELD(device,sDevice, reg, sReg, field, sField, \
                           mask, shift, msb,lsb,type,defaultVal) \
    add_field(SOC_INFO_CHIP_TYPE_BM9600, sDevice, sReg, sField, mask, shift, \
             msb, lsb, type, defaultVal);

#undef SAND_HAL_PL_AUTO_H
#undef HAL_PL_INLINE_H
#include <soc/sbx/hal_pl_inline.h>

#endif

    return rv;
}


int
sbx_fe2000_reg_list_init(void)
{
    int rv = 0;

#if defined(BCM_FE2000_SUPPORT)
    soc_sbx_chip_info_t     *chip_info = sbx_chip_info_add(SOC_INFO_CHIP_TYPE_FE2000, "CA");
    chip_info->regs = sal_alloc(sizeof(soc_sbx_reg_info_t *) * 4800, "fe2k regs");
    if (chip_info->regs == NULL)
      return rv;
    chip_info->maxreg = 4800;

  /*
  ** Redefined the macro SAND_HAL_ADD_REG so that a register record
  ** is entered into the device database.
  */
#undef SAND_HAL_ADD_REG
#define SAND_HAL_ADD_REG(device, sDevice, reg, sReg, offset, \
                         no_test_mask, mask, msb, lsb, intreg) \
    add_register(SOC_INFO_CHIP_TYPE_FE2000, sDevice, sReg, offset, mask, msb, \
                lsb, intreg);

  /*
  ** Redefined the macro SAND_HAL_ADD_FIELD so that a bit field is added
  ** to the specified register.
  */
#undef SAND_HAL_ADD_FIELD
#define SAND_HAL_ADD_FIELD(device,sDevice, reg, sReg, field, sField, \
                           mask, shift, msb,lsb,type,defaultVal) \
    add_field(SOC_INFO_CHIP_TYPE_FE2000, sDevice, sReg, sField, mask, shift, \
             msb, lsb, type, defaultVal);

#undef SAND_HAL_CA_AUTO_H
#undef HAL_CA_INLINE_H
#include <soc/sbx/hal_ca_inline.h>

#endif
    return rv;
}

int
sbx_fe2000xt_reg_list_init(void)
{
    int rv = 0;

#if defined(BCM_FE2000_SUPPORT)
    soc_sbx_chip_info_t     *chip_info = sbx_chip_info_add(SOC_INFO_CHIP_TYPE_FE2000XT, "C2");
    chip_info->regs = sal_alloc(sizeof(soc_sbx_reg_info_t *) * 6400, "fe2k regs");
    if (chip_info->regs == NULL)
      return rv;
    chip_info->maxreg = 6400;

  /*
  ** Redefined the macro SAND_HAL_ADD_REG so that a register record
  ** is entered into the device database.
  */
#undef SAND_HAL_ADD_REG
#define SAND_HAL_ADD_REG(device, sDevice, reg, sReg, offset, \
                         no_test_mask, mask, msb, lsb, intreg) \
    add_register(SOC_INFO_CHIP_TYPE_FE2000XT, sDevice, sReg, offset, mask, msb, \
                lsb, intreg);

  /*
  ** Redefined the macro SAND_HAL_ADD_FIELD so that a bit field is added
  ** to the specified register.
  */
#undef SAND_HAL_ADD_FIELD
#define SAND_HAL_ADD_FIELD(device,sDevice, reg, sReg, field, sField, \
                           mask, shift, msb,lsb,type,defaultVal) \
    add_field(SOC_INFO_CHIP_TYPE_FE2000XT, sDevice, sReg, sField, mask, shift, \
             msb, lsb, type, defaultVal);

#undef SAND_HAL_C2_AUTO_H
#undef HAL_C2_INLINE_H
#include <soc/sbx/hal_c2_inline.h>

#endif
    return rv;
}
