/*
 * $Id: arl.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: 	arl.h
 * Purpose: 	Defines structures and routines for ARL operations
 *              defined in:
 *              drv/arl.c      HW table management
 *              drv/arlmsg.c   ARL message handling
 */

#ifndef _SOC_ARL_H
#define _SOC_ARL_H

#include <shared/avl.h>
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SBX_SUPPORT) || defined(BCM_SAND_SUPPORT)
#include <soc/mcm/memregs.h>
#endif
#ifdef BCM_ROBO_SUPPORT        
#include <soc/robo/mcm/memregs.h>
#endif

extern int soc_arl_attach(int unit);
extern int soc_arl_detach(int unit);
extern int soc_arl_init(int unit);


typedef void (*soc_robo_arl_cb_fn)(int unit,
                              l2_arl_sw_entry_t *entry_del,
                              l2_arl_sw_entry_t *entry_add,
                              void *fn_data);
extern int soc_robo_arl_register(int unit, soc_robo_arl_cb_fn fn, 
                                    void *fn_data);
extern int soc_robo_arl_unregister(int unit, soc_robo_arl_cb_fn fn, 
                                    void *fn_data);

#define soc_robo_arl_unregister_all(unit) \
	soc_robo_arl_unregister((unit), NULL, NULL)

extern void soc_robo_arl_callback(int unit,
                             l2_arl_sw_entry_t *entry_del,
                             l2_arl_sw_entry_t *entry_add);
/*
 * ARL miscellaneous functions
 */

#define ARL_MODE_NONE		0
#define ARL_MODE_ROBO_POLL	1
#define ARL_MODE_SEARCH_VALID   ARL_MODE_ROBO_POLL
#define ARL_MODE_SCAN_VALID	2

#define ARL_ROBO_POLL_INTERVAL	3000000

extern int soc_arl_mode_set(int unit, int mode);
extern int soc_arl_mode_get(int unit, int *mode);

extern int soc_robo_arl_mode_set(int unit, int mode);
extern int soc_robo_arl_mode_get(int unit, int *mode);

extern int soc_arl_freeze(int unit);
extern int soc_arl_thaw(int unit);
extern int soc_robo_arl_freeze(int unit);
extern int soc_robo_arl_is_frozen(int unit, int *frozen);
extern int soc_robo_arl_thaw(int unit);
extern int soc_arl_frozen_cml_set(int unit, soc_port_t port, int cml,
				  int *repl_cml);
extern int soc_arl_frozen_cml_get(int unit, soc_port_t port, int *cml);
extern void _drv_arl_hash(uint8 *hash_value, uint8 length, uint16 *hash_result);
/*
 * For ARL software shadow database access
 */
extern int soc_arl_database_dump(int unit, uint32 index, 
                                     l2_arl_sw_entry_t *entry);
extern int soc_arl_database_delete(int unit, uint32 index);
extern int soc_arl_database_add(int unit, uint32 index, int pending);

/* new SOC driver to serve the ROBO's SW/HW ARL frozen sync detection */
extern void soc_arl_frozen_sync_init(int unit);
extern void soc_arl_frozen_sync_status(int unit, int *is_sync);

#define ARL_TABLE_WRITE 0  /* For ARL Write operateion */
#define ARL_TABLE_READ 1   /* For ARL Read operateion */
#define ARL_ENTRY_NULL(e1)\
    ((e1)->entry_data[0] == 0 && \
     (e1)->entry_data[1] == 0 && \
     (e1)->entry_data[2] == 0)
 

#define _ARL_SEARCH_VALID_OP_START      0x1
#define _ARL_SEARCH_VALID_OP_GET          0x2
#define _ARL_SEARCH_VALID_OP_DONE       0x3
#define _ARL_SEARCH_VALID_OP_NEXT       0x4
extern int soc_arl_search_valid(int unit, int op, void *index, void *entry, void *entry1);

#endif	/* !_SOC_ARL_H */
