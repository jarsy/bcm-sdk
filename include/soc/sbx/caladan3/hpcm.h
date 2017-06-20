/*
 * $Id: hpcm.h,v 1.4.6.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * hpcm  : Heap chunk memory manager
 *         allocates pool of dynamic memory & manages them without need for 
 *         multiple malloc's
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADAN3_HPCM_H_
#define _SBX_CALADAN3_HPCM_H_

#include <soc/types.h>
#include <soc/sbx/sbDq.h>

#ifndef TAPS_MEM_DEBUG
/* Note this assumes only 1 chunk is used */
/* #define TAPS_MEM_DEBUG*/
#endif

/*
 * Single link
 */
typedef struct sl_s *sl_p_t;
typedef struct sl_s
{
  VOL sl_p_t next_node;
}sl_t;
#define HPCM_SL_INIT(l)  do { ((l)->next_node) = NULL; } while (0)

#define HPCM_SL_INSERT_HEAD(l, e)       \
do                                      \
{                                       \
    sl_p_t p_elem;                      \
    p_elem = (sl_p_t)(e);               \
    p_elem->next_node = (l)->next_node; \
    (l)->next_node = p_elem;            \
} while (0)

#define HPCM_SL_REMOVE_HEAD(l, e)       \
do                                      \
{                                       \
    sl_p_t p_elem;                      \
    p_elem = (sl_p_t)(e);               \
    p_elem = (l)->next_node;            \
    e = (void *)p_elem;                 \
    (l)->next_node = p_elem->next_node; \
} while (0)

struct soc_heap_mem_chunk_s;
typedef struct soc_heap_mem_chunk_s soc_heap_mem_chunk_t;

typedef struct soc_heap_mem_elem_s {
    dq_t list_node;
    void *elem;
    soc_heap_mem_chunk_t *parent;
#ifdef TAPS_MEM_DEBUG
    int  in_use;
#endif
} soc_heap_mem_elem_t;

struct soc_heap_mem_chunk_s {
    dq_t list_node;
    dq_t free_list;
    int  size;
    int  page_size;
    int  elem_size_bytes;
    int  alloc_count;
    int  free_count;
    soc_heap_mem_elem_t *elem_ctrl_pool;
    uint8 *elem_pool;
};

struct soc_heap_sl_mem_chunk_s;
typedef struct soc_heap_sl_mem_chunk_s soc_heap_sl_mem_chunk_t;

typedef struct soc_heap_sl_mem_elem_s {
    sl_t list_node;
    void *elem;
} soc_heap_sl_mem_elem_t;

struct soc_heap_sl_mem_chunk_s {
    dq_t list_node;
    sl_t free_list;
    int  size;
    int  page_size;
    int  elem_size_bytes;
    int  alloc_count;
    int  free_count;
    soc_heap_sl_mem_elem_t *elem_ctrl_pool;
    uint8 *elem_pool;
};

/* hpcm -> heap chunk manager */
extern int hpcm_init(int unit, int chunk_size, int elem_size, soc_heap_mem_chunk_t **hpcm);

extern int hpcm_destroy(int unit, soc_heap_mem_chunk_t *hpcm);

extern int hpcm_alloc(int unit, soc_heap_mem_chunk_t *hpcm, soc_heap_mem_elem_t **hpcm_elem);

extern int hpcm_free(int unit, soc_heap_mem_elem_t *hpcm_elem);

extern int hpcm_empty(int unit, soc_heap_mem_chunk_t *hpcm);

extern int hpcm_is_unused(int unit, soc_heap_mem_chunk_t *hpcm);

extern int hpcm_alloc_payload(int unit, soc_heap_mem_chunk_t *hpcm, void **payload);

extern int hpcm_free_payload(int unit, soc_heap_mem_chunk_t *hpcm, void *payload);

extern int hpcm_sl_init(int unit, int chunk_size, int elem_size_bytes, soc_heap_sl_mem_chunk_t **hpcm);

extern int hpcm_sl_alloc_payload(int unit, soc_heap_sl_mem_chunk_t *hpcm, void **payload, void **hpcm_elem_handle);

extern int hpcm_sl_free_payload(int unit, soc_heap_sl_mem_chunk_t *hpcm, void *hpcm_elem_handle);

#ifdef TAPS_MEM_DEBUG
extern int hpcm_alloc_dump(int unit, soc_heap_mem_chunk_t *hpcm);
#endif

#endif /* _SBX_CALADAN3_HPCM_H_ */
