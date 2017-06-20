/******************************************************************************
** ========================================================================
** == glue.h -  Lower level OS specific glue layer for Sandburst devices ==
** ========================================================================
**
** WORKING REVISION: $Id: glue.h,v 1.6 Broadcom SDK $
**
** $Copyright: (c) 2016 Broadcom.
** Broadcom Proprietary and Confidential. All rights reserved.$
**
** MODULE NAME:
**
**     glue.h
**
** ABSTRACT:
**
**     Provide access to Sandburst chips.
**     All access is through opaque types.
**
** LANGUAGE:
**
**     C
**
** AUTHORS:
**
**     Lennart Augustsson
**
** CREATION DATE:
**
**     22-July-2004
**
******************************************************************************/
#ifndef _THINGLUE_H_
#define _THINGLUE_H_

#include <soc/sbx/sbTypesGlue.h>
#include <soc/sbx/glue_dma.h>


/*
 * The following function pointer definition allows the user
 * to run code without actually writing to or reading from the
 * chip, thereby 'dry-running'
 */
typedef uint32 (*thin_ack_reg_func_ptr_t)(uint32 ulOffset);

extern sbhandle thin_open(const char *device, thin_bus_t bt, sb_chip_t type );
extern void thin_close(sbhandle hdl);

extern sbhandle thin_map_registers(sbhandle hdl);
extern sbhandle thin_offset_handle(sbhandle addr, uint32 offs);
extern void thin_offset_handle_remove(sbhandle hdl);
extern int thin_unmap_registers(sbhandle hdl);

extern sbreg thin_cswap32(sbhandle addr, sbreg data);

extern sbreg thin_read32(sbhandle addr, uint32 offs);
extern void thin_write32(sbhandle addr, uint32 offs, sbreg data);

void
thin_no_write(sbhandle addr, uint32 offs, sbreg data);

extern sbreg thin_read32_raw(sbhandle addr, uint32 offs);
extern void thin_write32_raw(sbhandle addr, uint32 offs, sbreg data);

extern sbStatus_t thin_malloc(void *clientData, sbFeMallocType_t type, uint32 size,
	  void **memoryP, sbDmaMemoryHandle_t *dmaHandleP);

extern sbStatus_t
thin_contig_malloc(void *clientData, sbFeMallocType_t type, uint32 size,
	  void **memoryP, sbDmaMemoryHandle_t *dmaHandleP);

extern sbStatus_t
gen_thin_malloc(void *clientData, sbFeMallocType_t type, uint32 size,
          void **memoryP, sbDmaMemoryHandle_t *dmaHandleP, uint8 contig);

extern sbStatus_t
thin_free(void *clientData, sbFeMallocType_t type, uint32 size,
	void *memory, sbDmaMemoryHandle_t dmaHandle);

extern void*
thin_host_malloc(uint32 size);

extern void
thin_host_free(void *vp);

extern void thin_delay(uint32 nanosecs);

extern void *thin_alloc_dma_memory(sbhandle hdl, uint32 size,
			    uint32 **pages, uint32 *npages);
extern void *thin_alloc_contig_dma_memory(sbhandle hdl, uint32 size,
				   uint32 *pages);

extern int thin_get_fd(sbhandle hdl);
extern int thin_get_pagesize(sbhandle hdl);

extern int thin_intr_handler(sbhandle hdl, void (*f)(void *), void *data);
extern int thin_block_intr(sbhandle hdl);
extern void thin_unblock_intr(sbhandle hdl, int token);

extern int thin_dma_read_sync(sbhandle hdl, uint32 offs, uint32 len);
extern int thin_dma_write_sync(sbhandle hdl, uint32 offs, uint32 len);

extern void thin_enable_signal(sbhandle);
extern void thin_disable_signal(sbhandle);

extern uint32 thin_get_cb_num(sbhandle hdl);

extern sbStatus_t thin_sem_open(uint32 key, int* semId);
extern sbStatus_t thin_sem_close(int semId);
extern sbStatus_t thin_sem_get(int semId);
extern sbStatus_t thin_sem_put(int semId);
extern sbStatus_t thin_sem_wait_get(int semId, int timeOut);

extern sbStatus_t thin_dryrun_set(sbhandle hdl, thin_ack_reg_func_ptr_t pfDryRun);
extern sbStatus_t thin_verbose_set(sbhandle hdl, int verblvl);


#endif /* _THINGLUE_H_ */
