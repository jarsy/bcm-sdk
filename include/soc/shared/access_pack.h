/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    access_pack.h
 * Purpose: Miscellaneous routine for device db access
 */

#ifndef   _SOC_SHARED_ACESS_PACK_H_
#define   _SOC_SHARED_ACESS_PACK_H_

#include <shared/utilex/utilex_rhlist.h>

#define ACC_NO_READ     0x01
#define ACC_NO_WB       0x02

typedef struct
{
    int flags;
} shr_reg_data_t;

typedef struct
{
    int flags;
} shr_mem_data_t;

shr_error_e
shr_access_device_init(
    int unit);

int
shr_access_reg_no_read_get(
        int         unit,
        soc_reg_t   reg);

int
shr_access_mem_no_read_get(
        int         unit,
        soc_mem_t   reg);

int
shr_access_reg_no_wb_get(
        int         unit,
        soc_reg_t   reg);

int
shr_access_mem_no_wb_get(
        int         unit,
        soc_mem_t   mem);

#endif /* _SOC_SHARED_ACESS_PACK_H_ */
