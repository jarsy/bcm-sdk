/*
 * $Id: jer2_jer_sbusdma_desc.h Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _SOC_DNX_JER2_JER_SBUSDMA_DESC_H
#define _SOC_DNX_JER2_JER_SBUSDMA_DESC_H

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* } */

/*************
 * ENUMS     *
 *************/
/* { */

/* } */

/*************
 * MACROS    *
 *************/
/* { */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

uint32 jer2_jer_sbusdma_desc_init(int unit, uint32 desc_num_max, uint32 mem_buff_size, uint32 timeout_usec);

uint32 jer2_jer_sbusdma_desc_deinit(int unit);

uint32 jer2_jer_sbusdma_desc_wait_done(int unit);

uint32 jer2_jer_sbusdma_desc_add(int unit, soc_mem_t mem, uint32 array_index, int blk, uint32 offset, void *entry_data);

uint32 jer2_jer_sbusdma_desc_add_fifo_dma(int unit, soc_mem_t mem, uint32 array_index, int blk, uint32 offset, void *entry_data, uint32 count, uint32 addr_shift, uint8 new_desc);

uint32 jer2_jer_sbusdma_desc_status(int unit, uint32 *desc_num_max, uint32 *mem_buff_size, uint32 *timeout_usec);

uint32 jer2_jer_sbusdma_desc_is_enabled(int unit);

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif /* !_SOC_DNX_JER2_JER_SBUSDMA_DESC_H  */

