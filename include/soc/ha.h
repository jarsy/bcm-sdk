/*! \file ha.h
 * $Id$
 *
 * HA API header file
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef __HA_H__
#define __HA_H__
#if !defined(__KERNEL__) && defined (LINUX)

#include <bcm/types.h>
#include <soc/types.h>


int ha_init(int unit, uint8 enabled, const char *name, int size, uint8 create_new_file);
/* return 1 if there is already allocated memory for this module and sub id */
int ha_mem_is_already_alloc(int unit, unsigned char mod_id, 
                          unsigned char sub_id);
void *ha_mem_alloc(int unit, unsigned char mod_id, 
                          unsigned char sub_id, 
                          unsigned char version, 
                          unsigned int struct_sig,
                          unsigned int *length);
void *ha_mem_realloc(int unit, void *mem, unsigned int *length);
int ha_mem_free(int unit, void *mem);
int ha_destroy(int unit, uint8 delete_file);

typedef enum {
    HA_HW_LOG_Mem_Pool = 200, /* start from 200 to prevent conflict with scache modids */
    HA_KBP_Mem_Pool    = 201,
    HA_KAPS_Mem_Pool   = 202,
    HA_CR_Mem_Pool     = 203
} HA_mod_id_tl;

#endif /* __KERNEL__ */
#endif
