/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_MM_H__
#define __NEMO_MM_H__

#include "nd_platform.h"
#include "nd_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

void    *ag_nd_mm_ram_alloc(size_t n_size);
void    ag_nd_mm_ram_free(void*);


typedef struct 
{
    AG_U32      n_block_count;
    AgNdList    x_block_list;   

} AgNdAllocator;

typedef struct 
{
    AG_U32      n_start;        /* block start address */
    AG_U32      n_size;         /* block size in bytes */
    AG_BOOL     b_free;         /* allocated/free flag */

    AgNdList    x_list;         

} AgNdAllocatorBlock;

AgResult    ag_nd_allocator_init(AgNdAllocator*, AG_U32 n_start, AG_U32 n_size, AG_U32 n_block_size);
AgResult    ag_nd_allocator_destroy(AgNdAllocator *p_allocator);
AgResult    ag_nd_allocator_alloc(AgNdAllocator*, AG_U32 n_size, AG_U32 n_alignment, AG_U32 *n_start);
AgResult    ag_nd_allocator_free(AgNdAllocator*, AG_U32 n_start);





#ifdef __cplusplus
}
#endif

#endif /* __NEMO_MM_H__ */

