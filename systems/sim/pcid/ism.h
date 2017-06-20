/* 
 * $Id: ism.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        ism.h
 * Purpose:     Include file for ISM memory access functions.
 */

#ifndef _PCID_ISM_H
#define _PCID_ISM_H

#include "pcid.h"
#include <sys/types.h>
#include <soc/mcm/memregs.h>

#ifdef BCM_ISM_SUPPORT
int soc_internal_generic_hash_insert(pcid_info_t *pcid_info, soc_mem_t mem, int banks, 
                                     void *entry, uint32 *result);
int soc_internal_generic_hash_lookup(pcid_info_t *pcid_info, soc_mem_t mem, int banks, 
                                     void *entry, uint32 *result);
int soc_internal_generic_hash_delete(pcid_info_t *pcid_info, soc_mem_t mem, int banks, 
                                     void *entry, uint32 *result);

#endif

#endif	/* _PCID_ISM_H */

