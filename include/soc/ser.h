/*
 * $Id: ipoll.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Functions for soft error recovery.
 *
 */

#ifndef _SOC_SER_H
#define _SOC_SER_H

#include <soc/chip.h>

typedef struct soc_ser_info_s {
    _soc_mem_ser_en_info_t *ip_mem_ser_info;
    _soc_reg_ser_en_info_t *ip_reg_ser_info;
    _soc_mem_ser_en_info_t *ep_mem_ser_info;
    _soc_reg_ser_en_info_t *ep_reg_ser_info;
    _soc_mem_ser_en_info_t *mmu_mem_ser_info;
} soc_ser_info_t;



#endif /* _SOC_SER_H */
