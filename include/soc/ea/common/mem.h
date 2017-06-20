/*
 * $Id: mem.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Arm Processor Subsystem remote management library
 */

#ifndef SOC_EA_COMMON_MEM_H
#define SOC_EA_COMMON_MEM_H

#include <sal/types.h>


#define SOC_EA_MEM_E_SUCCESS            0x00000000
#define SOC_EA_MEM_E_INVALID_PARAMETER  0x80000000
#define SOC_EA_MEM_E_OUT_OF_SPACE       0x80000001
#define SOC_EA_MEM_E_NO_MEM             0x80000002
#define SOC_EA_MEM_E_POOL_NOT_EMPTY     0x80000003
#define SOC_EA_MEM_E_INTERNAL_ERROR     0x80000004
#define SOC_EA_MEM_E_DUPLICATE_OP       0x80000005


typedef struct soc_ea_mem_cfg_entry_s {
    uint32      blk_sz;				/* The size of per block*/
    uint32      blk_num;			/* How many blocks do you want to apply?*/
} soc_ea_mem_cfg_entry_t;


/* Upper layer provides this information to create an EA static memory pool */
typedef struct soc_ea_mem_cfg_s {
    uint32  	n_entry;				/* How many entries do we have? */
    soc_ea_mem_cfg_entry_t  *entries;	/* Entry tables */
} soc_ea_mem_cfg_t;



typedef struct soc_ea_mem_blk_chain_s {
    struct soc_ea_mem_blk_chain_s    * head;   	/* head of the chain */
    struct soc_ea_mem_blk_chain_s    * tail;   	/* tail of the chain */
    struct soc_ea_mem_blk_chain_s    * next;	
    uint32          buf_sz;						
    uint8         * buf;
    uint8         * wr_ptr;
    uint8         * rd_ptr;
    uint32          idx;
} soc_ea_mem_blk_chain_t;

extern void soc_ea_mem_init(void);
extern void  * soc_ea_mem_pool_create (soc_ea_mem_cfg_t * mem_cfg, char * pool_name, uint32 * err_code);
extern uint32  soc_ea_mem_pool_free (void * mem_pool);
extern uint8 * soc_ea_mem_blk_alloc (void * mem_pool, uint32 sz, uint32 *err_code);
extern uint32  soc_ea_mem_blk_free (uint8 * buf);

extern soc_ea_mem_blk_chain_t * soc_ea_mem_blk_chain_alloc (void * mem_pool, uint32 sz, uint32 *err_code);
extern soc_ea_mem_blk_chain_t * soc_ea_mem_blk_chain_append (
	soc_ea_mem_blk_chain_t * chain, soc_ea_mem_blk_chain_t * new_entry, uint32 * err_code);
extern uint32 soc_ea_mem_blk_chain_free (soc_ea_mem_blk_chain_t * chain);

#ifdef BROADCOM_DEBUG
extern void soc_ea_mem_dump(void);
#endif

#endif /* SOC_EA_COMMON_MEM_H */

