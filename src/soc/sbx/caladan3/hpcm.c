/*
 * $Id: hpcm.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    hpcm.c
 * Purpose: Caladan3 memory manager utility 
 * Requires:
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3/hpcm.h>

int hpcm_init(int unit, int chunk_size, int elem_size_bytes, soc_heap_mem_chunk_t **hpcm)
{
    int rv = SOC_E_NONE, index;
    soc_heap_mem_chunk_t *chunk=NULL;

    if (!hpcm) return SOC_E_PARAM;

    chunk = sal_alloc(sizeof(soc_heap_mem_chunk_t), "hpcm");
    if (chunk) {
        sal_memset(chunk, 0, sizeof(soc_heap_mem_chunk_t));
        chunk->page_size= chunk_size;
        chunk->size = chunk_size;
        chunk->elem_size_bytes = elem_size_bytes;
        DQ_INIT(&chunk->free_list);
        chunk->free_count = chunk_size;
        chunk->alloc_count = 0;

        /* allocate element space */
        chunk->elem_ctrl_pool = sal_alloc(sizeof(soc_heap_mem_elem_t) * chunk_size, "hpcm-ctrl-pool");
        chunk->elem_pool = sal_alloc(chunk_size * elem_size_bytes, "hpcm-pool");

        if (!chunk->elem_pool || !chunk->elem_ctrl_pool) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d: Failed to allocate chunk memory !!!\n"), 
                       FUNCTION_NAME(), unit));

            if (chunk->elem_ctrl_pool) sal_free(chunk->elem_ctrl_pool);
            sal_free(chunk);
            rv = SOC_E_MEMORY; 
        } else {
            for (index=0; index < chunk_size; index++) {
                DQ_INSERT_HEAD(&chunk->free_list, &chunk->elem_ctrl_pool[index].list_node);
                chunk->elem_ctrl_pool[index].elem = chunk->elem_pool + index * elem_size_bytes;
                chunk->elem_ctrl_pool[index].parent = chunk;
#ifdef TAPS_MEM_DEBUG
		chunk->elem_ctrl_pool[index].in_use = FALSE;
#endif
            }
            *hpcm = chunk;
        }
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d: Failed to allocate chunk memory !!!\n"), 
                   FUNCTION_NAME(), unit));
        rv = SOC_E_MEMORY; 
    }

    return rv;
}

int hpcm_destroy(int unit, soc_heap_mem_chunk_t *hpcm)
{
    int rv = SOC_E_NONE;

    if (!hpcm) return SOC_E_PARAM;

    sal_free(hpcm->elem_ctrl_pool);
    sal_free(hpcm->elem_pool);
    sal_free(hpcm);

    return rv;
}

int hpcm_alloc(int unit, soc_heap_mem_chunk_t *hpcm, soc_heap_mem_elem_t **hpcm_elem)
{
    int rv = SOC_E_NONE;
    soc_heap_mem_elem_t *elem = NULL;

    if (!hpcm || !hpcm_elem) return SOC_E_PARAM;


    if (hpcm->free_count == 0) {
        rv = SOC_E_MEMORY;
    } else {
        DQ_REMOVE_HEAD(&hpcm->free_list, elem);
        hpcm->free_count--;
        hpcm->alloc_count++;
        assert(hpcm->free_count + hpcm->alloc_count == hpcm->size);
        *hpcm_elem = elem;
#ifdef TAPS_MEM_DEBUG
	hpcm->elem_ctrl_pool[((uint8*)(elem->elem) - hpcm->elem_pool)/hpcm->elem_size_bytes].in_use = TRUE;
#endif
    }

    return rv;
}

int hpcm_free(int unit, soc_heap_mem_elem_t *hpcm_elem)
{
    int rv = SOC_E_NONE;
    soc_heap_mem_chunk_t *hpcm = NULL;

    if (!hpcm_elem) return SOC_E_PARAM;

    hpcm = hpcm_elem->parent;
    hpcm->alloc_count--;
    DQ_INSERT_TAIL(&hpcm->free_list, hpcm_elem);
    hpcm->free_count++;
    assert(hpcm->free_count + hpcm->alloc_count == hpcm->size);
#ifdef TAPS_MEM_DEBUG
    hpcm->elem_ctrl_pool[((uint8*)(hpcm_elem->elem) - hpcm->elem_pool)/hpcm->elem_size_bytes].in_use = FALSE;
#endif

    return rv;
}

/*
 * Alloc payload instead of hpcm_element.
 * NOTE: This function assumes the elem_pool and elem_ctrl_pool
 *     are allocated as array and indexed in same order.
 */
int hpcm_alloc_payload(int unit, soc_heap_mem_chunk_t *hpcm, void **payload)
{
    int rv;
    soc_heap_mem_elem_t *hpcm_elem;
  
    rv = hpcm_alloc(unit, hpcm, &hpcm_elem);

    if (SOC_FAILURE(rv)) {
	*payload = NULL;
    } else {
	*payload = (void *)(hpcm_elem->elem);
    }
    return rv;
}

/*
 * Free payload instead of hpcm_element.
 * NOTE: This function assumes the elem_pool and elem_ctrl_pool
 *     are allocated as array and indexed in same order.
 */
int hpcm_free_payload(int unit, soc_heap_mem_chunk_t *hpcm, void *payload)
{
    int index;
    soc_heap_mem_elem_t *hpcm_elem;

    index = ((uint8*)payload - (uint8*)hpcm->elem_pool) / (hpcm->elem_size_bytes);
    if ((index < 0) || (index >= hpcm->size)) {
	/* can not find the matching element */
	return SOC_E_NOT_FOUND;
    }
    hpcm_elem = hpcm->elem_ctrl_pool + index;
    
    return hpcm_free(unit, hpcm_elem);
}

/*
* Init memory pool with single link
*/
int hpcm_sl_init(int unit, int chunk_size, int elem_size_bytes, soc_heap_sl_mem_chunk_t **hpcm)
{
    int rv = SOC_E_NONE, index;
    soc_heap_sl_mem_chunk_t *chunk=NULL;

    if (!hpcm) return SOC_E_PARAM;

    chunk = sal_alloc(sizeof(soc_heap_sl_mem_chunk_t), "hpcm");
    if (chunk) {
        sal_memset(chunk, 0, sizeof(soc_heap_sl_mem_chunk_t));
        chunk->page_size= chunk_size;
        chunk->size = chunk_size;
        chunk->elem_size_bytes = elem_size_bytes;
        HPCM_SL_INIT(&chunk->free_list);
        chunk->free_count = chunk_size;
        chunk->alloc_count = 0;

        /* allocate element space */
        chunk->elem_ctrl_pool = sal_alloc(sizeof(soc_heap_sl_mem_elem_t) * chunk_size, "hpcm-ctrl-pool");
        chunk->elem_pool = sal_alloc(chunk_size * elem_size_bytes, "hpcm-pool");

        if (!chunk->elem_pool || !chunk->elem_ctrl_pool) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d: Failed to allocate chunk memory !!!\n"), 
                       FUNCTION_NAME(), unit));

            if (chunk->elem_ctrl_pool) sal_free(chunk->elem_ctrl_pool);
            sal_free(chunk);
            rv = SOC_E_MEMORY; 
        } else {
            for (index=0; index < chunk_size; index++) {
                HPCM_SL_INSERT_HEAD(&chunk->free_list, &chunk->elem_ctrl_pool[index].list_node);
                chunk->elem_ctrl_pool[index].elem = chunk->elem_pool + index * elem_size_bytes;
            }
            *hpcm = chunk;
        }
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d: Failed to allocate chunk memory !!!\n"), 
                   FUNCTION_NAME(), unit));
        rv = SOC_E_MEMORY; 
    }

    return rv;
}

/*
* Get a elem from the single link
*/
INLINE static int hpcm_sl_alloc(int unit, soc_heap_sl_mem_chunk_t *hpcm, soc_heap_sl_mem_elem_t **hpcm_elem)
{
    int rv = SOC_E_NONE;
    soc_heap_sl_mem_elem_t *elem = NULL;

    if (!hpcm || !hpcm_elem) return SOC_E_PARAM;

    if (hpcm->free_count == 0) {
        rv = SOC_E_MEMORY;
    } else {
        HPCM_SL_REMOVE_HEAD(&hpcm->free_list, elem);
        hpcm->free_count--;
        hpcm->alloc_count++;
        assert(hpcm->free_count + hpcm->alloc_count == hpcm->size);
        *hpcm_elem = elem;
    }

    return rv;
}

/*
* Put back a elem from the single link
*/
INLINE static int hpcm_sl_free(int unit, soc_heap_sl_mem_chunk_t *hpcm, soc_heap_sl_mem_elem_t *hpcm_elem)
{
    int rv = SOC_E_NONE;

    if (!hpcm || !hpcm_elem) return SOC_E_PARAM;

    hpcm->alloc_count--;
    HPCM_SL_INSERT_HEAD(&hpcm->free_list, hpcm_elem);
    hpcm->free_count++;
    assert(hpcm->free_count + hpcm->alloc_count == hpcm->size);
    return rv;
}

/*
 * Alloc payload in memory pool. If current pool is full, then alloc a new chunk in this pool. 
 */
int hpcm_sl_alloc_payload(int unit, soc_heap_sl_mem_chunk_t *hpcm, void **payload, void **hpcm_elem_handle)
{
    int rv = SOC_E_NONE;
    int index;
    int chunk_size;
    soc_heap_sl_mem_elem_t *hpcm_elem = NULL;
    soc_heap_sl_mem_chunk_t chunk;

    if (!hpcm || !payload || !hpcm_elem_handle) {
        return SOC_E_PARAM;
    }
    
    rv = hpcm_sl_alloc(unit, hpcm, &hpcm_elem);
    if (SOC_FAILURE(rv)) {
        chunk_size = hpcm->page_size;
        chunk.elem_ctrl_pool = sal_alloc(sizeof(soc_heap_sl_mem_elem_t) * chunk_size, "hpcm-ctrl-pool");
        chunk.elem_pool = sal_alloc(chunk_size * hpcm->elem_size_bytes, "hpcm-pool");
        if (!chunk.elem_pool || !chunk.elem_ctrl_pool) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d: Failed to allocate chunk memory !!!\n"), 
                       FUNCTION_NAME(), unit));

            if (chunk.elem_ctrl_pool) sal_free(chunk.elem_ctrl_pool);
            if (chunk.elem_pool) sal_free(chunk.elem_pool);
            rv = SOC_E_MEMORY; 
			COMPILER_REFERENCE(rv);
        } else {
            for (index = 0; index < chunk_size; index++) {
                HPCM_SL_INSERT_HEAD(&hpcm->free_list, &chunk.elem_ctrl_pool[index].list_node);
                chunk.elem_ctrl_pool[index].elem = chunk.elem_pool + index * hpcm->elem_size_bytes;
            }
            hpcm->free_count += chunk_size;
            hpcm->size += chunk_size;
            rv = hpcm_sl_alloc(unit, hpcm, &hpcm_elem);
        }
    }
    
    if (SOC_FAILURE(rv)) {
        *payload = NULL;
        *hpcm_elem_handle = NULL;
    } else {
        *payload = (void *)(hpcm_elem->elem);
        *hpcm_elem_handle = (void *)hpcm_elem;
    }

    return rv;
}

/*
 * Free payload instead of hpcm_element.
 */
int hpcm_sl_free_payload(int unit, soc_heap_sl_mem_chunk_t *hpcm, void *hpcm_elem_handle)
{
    if (!hpcm || !hpcm_elem_handle) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: Invalid param !!!\n"), 
                   FUNCTION_NAME()));
        return SOC_E_PARAM;
    }
    
    return hpcm_sl_free(unit, hpcm, hpcm_elem_handle);
}

/*
 * Return TRUE is this hpcm is used up.
 */
int hpcm_empty(int unit, soc_heap_mem_chunk_t *hpcm)
{
    if (!hpcm) return SOC_E_PARAM;
    assert(hpcm->free_count + hpcm->alloc_count == hpcm->size);
    return (hpcm->free_count == 0 && 
            hpcm->alloc_count == hpcm->size) ? TRUE: FALSE;
}

/*
 * Return TRUE is this hpcm is unused.
 */
int hpcm_is_unused(int unit, soc_heap_mem_chunk_t *hpcm)
{
    if (!hpcm) return SOC_E_PARAM;
    assert(hpcm->free_count + hpcm->alloc_count == hpcm->size);
    return (hpcm->free_count == hpcm->size && 
            hpcm->alloc_count == 0) ? TRUE: FALSE;
}

#ifdef TAPS_MEM_DEBUG
int hpcm_alloc_dump(int unit, soc_heap_mem_chunk_t *hpcm)
{
    int index;
    /* we assume there is only 1 chunk allocated, 
     * everything allocated will be marked as in_use
     */
    for (index = 0; index < hpcm->size; index++) {
	if (hpcm->elem_ctrl_pool[index].in_use) {
	    /* dump what's in there, assuming it's a trie node */
	    LOG_CLI((BSL_META_U(unit,
                                "pointer 0x%p in use\n"), (char *)(hpcm->elem_pool + index * hpcm->elem_size_bytes)));	    
	}
    }	

    return SOC_E_NONE;
}

#endif

#endif
