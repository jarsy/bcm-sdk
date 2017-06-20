/******************************************************************************
** ========================================================================
** == glue.h -  Lower level OS specific glue layer for Sandburst devices ==
** ========================================================================
**
** WORKING REVISION: $Id: glue_dma.h,v 1.3 Broadcom SDK $
**
** $Copyright: (c) 2016 Broadcom.
** Broadcom Proprietary and Confidential. All rights reserved.$
**
** MODULE NAME:
**
**     glue_p.h
**
** ABSTRACT:
**
**     Private glue header file.  This file will most likely be unecessary once
** simserver learning interfaces with the QE_NIC driver.
**
** LANGUAGE:
**
**     C
**
** AUTHORS:
**
**     Sean Campbell
**
** CREATION DATE:
**
**     07-Dec-2004
**
******************************************************************************/
#ifndef _THINGLUE_DMA_H_
#define _THINGLUE_DMA_H_

#include "sbTypesGlue.h"

#ifdef SB_LINUX
#include <linux/sbthinioctl.h>

/* For Interfacing with the DMA */
struct dma {
  struct sb_alloc_mem  alloc;  /* alloc information */
  char                *base;   /* Pointer to the Register Base */
  uint32             page_size; /* Page Size */
};
#endif /* SB_LINUX */

extern sbhandle thin_dma_handle(sbhandle hdl, uint32 addr);

extern sbStatus_t thin_getHba(sbDmaMemoryHandle_t mHandle, uint32 *address, uint32 words,
	  sbFeDmaHostBusAddress_t *hbaP);

extern sbStatus_t thin_writeSync(sbDmaMemoryHandle_t mHandle, uint32 *address, uint32 words);

extern sbStatus_t thin_readSync(sbDmaMemoryHandle_t mHandle, uint32 *address, uint32 words);

extern sbSyncToken_t thin_isrSync(void *data);

extern void thin_isrUnsync(void *data, sbSyncToken_t token);

extern void thin_async_cback(sbFeAsyncCallbackArgument_p_t arg);

/* ENHANCE...should probably rename these two to alleviate potential confusion */
extern sbStatus_t thin_wait_for_async(void *clientData, uint32 cnt);

extern sbStatus_t thin_wait_for_async_type(void *clientData, int type);

#endif /* _THINGLUE_DMA_H_ */
