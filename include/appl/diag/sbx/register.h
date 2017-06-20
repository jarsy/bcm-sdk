/* $Id: register.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        register.h
 * Purpose:     Base definitions for SBX register types
 * Requires:    
 */

#ifndef _SOC_SBX_REGISTER_H
#define _SOC_SBX_REGISTER_H

#define SBX_DEV_NAME_LEN_MAX 64
#define SBX_REG_NAME_LEN_MAX 128
#define SBX_REG_FIELDS_MAX  64
#define SBX_REG_NUM_MAX 6400

#define DUMP_TABLE_RAW          0x01
#define DUMP_TABLE_HEX          0x02
#define DUMP_TABLE_ALL          0x04
#define DUMP_TABLE_CHANGED      0x08
#define DUMP_TABLE_VERTICAL     0x10

#ifndef BCM_CALADAN3_SUPPORT
#include <soc/sbx/sbTypesGlue.h>
#endif
#include <appl/diag/shell.h>
#include <appl/diag/sbx/field.h>

typedef int soc_sbx_reg_idx;
#define SOC_SBX_REG_IS_INDIRECT 0x80000000
typedef struct soc_sbx_reg_info_s {
    char    name[SBX_REG_NAME_LEN_MAX];
    uint32  offset;
    uint32  ioffset;
    uint32  nt_mask;
    uint32  mask;
    uint8   msb;
    uint8   lsb;
    uint8   intreg;
    uint8   nfields;
    soc_sbx_field_info_t *fields[SBX_REG_FIELDS_MAX];
} soc_sbx_reg_info_t;

typedef struct soc_sbx_reg_info_list_s {
    int     count;
    soc_sbx_reg_idx idx[SBX_REG_NUM_MAX];
} soc_sbx_reg_info_list_t;

typedef struct soc_sbx_chip_info_s {
    char    name[SBX_DEV_NAME_LEN_MAX];
    int     id;
    int     nregs;
    int     maxreg;
    soc_sbx_reg_info_t** regs;
} soc_sbx_chip_info_t;

extern cmd_result_t sbx_chip_info_get(int chip_id,
                                      soc_sbx_chip_info_t **chip_info,
				      int create);

extern cmd_result_t
cmd_sbx_cmic_do_dump_table(int unit, soc_mem_t mem,
              int copyno, int index, int count, int flags);

#endif	/* !_SOC_SBX_REGISTER_H */
