
/*
 * $Id: dnxc_mem.c,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC DNXC MEM
 */
 
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MEM
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnxc/legacy/dnxc_mem.h>
#include <soc/drv.h>
#include <soc/memory.h>
#include <soc/mem.h>
#ifdef BCM_DNX_SUPPORT
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#endif /* BCM_DNX_SUPPORT */
#ifdef BCM_DNXF_SUPPORT
#include <soc/dnxf/cmn/dnxf_drv.h>
#ifdef BCM_88950_A0
#include <soc/dnxf/ramon/ramon_fabric_cell.h>
#endif
#endif /* BCM_DNXF_SUPPORT */

#ifdef CRASH_RECOVERY_SUPPORT
#include <soc/hwstate/hw_log.h>
#include <soc/dnxc/legacy/dnxc_crash_recovery.h>
#endif /* CRASH_RECOVERY_SUPPORT */

int dnxc_tbl_is_dynamic(int unit,soc_mem_t mem)
{
#ifdef BCM_DNXF_SUPPORT
    if(SOC_IS_DNXF(unit) && soc_dnxf_tbl_is_dynamic(unit, mem)) {
        return TRUE;
    }
#endif /* BCM_DNXF_SUPPORT */
#ifdef BCM_DNX_SUPPORT
    if(SOC_IS_DNX(unit) && dnx_tbl_is_dynamic(unit, mem)) {
        return TRUE;
    }
#endif /* BCM_DNX_SUPPORT */
    return FALSE;
}

/* check if a given mem contain one of the fields apear in given list*/

int dnxc_mem_contain_one_of_the_fields(int unit,const soc_mem_t mem,soc_field_t *fields)
{

  int i;
  for (i=0;fields[i]!=NUM_SOC_FIELD;i++) {
      if (SOC_MEM_FIELD_VALID(unit,mem,fields[i])) {
          return 1;
      }
  }
  return 0;
}

/* Allocate memory of a given size, and store its location in the given pointer */
uint32 dnxc_alloc_mem(
    const int unit,
    void      *mem_ptr,        /* output: Will hold the pointer to the allocated memory, must be NULL. The real type of the argument is void** is not used to avoid compilation warnings */
    const unsigned size,       /* memory size in bytes to be allocated */
    const char     *alloc_name /* name of the memory allocation, used for debugging */
)
{
    DNXC_INIT_FUNC_DEFS;
    if (mem_ptr == NULL || alloc_name == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("null parameter" )));
    } else if (*(void**)mem_ptr != NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("memory pointer value is not NULL, when attempted to allocate %s"), alloc_name));
    }
    if ((*(void**)mem_ptr = sal_alloc(size, (char*)alloc_name)) == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY,(_BSL_DNXC_MSG("Failed to allocate %u bytes for %s"), size, alloc_name));
    }
    sal_memset(*(void**)mem_ptr, 0, size); /* init the allocated memory to zero */
exit:
    DNXC_FUNC_RETURN;
}

/* deallocate memory of a given size, and store its location in the given pointer */
uint32 dnxc_free_mem(
    const int unit,
    void **mem_ptr /* holds the pointer to the allocated memory, will be set to NULL */
)
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(mem_ptr);
    if (mem_ptr == NULL || *mem_ptr == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("null parameter" )));
    }
    sal_free(*mem_ptr);
    *mem_ptr = NULL;
exit:
    DNXC_FUNC_RETURN;
}

/* deallocate memory of a given size, and store its location in the given pointer */
void dnxc_free_mem_if_not_null(
    const int unit,
    void **mem_ptr /* holds the pointer to the allocated memory, will be set to NULL */
)
{
    if (mem_ptr != NULL && *mem_ptr != NULL) {
        sal_free(*mem_ptr);
        *mem_ptr = NULL;
    }
}


/*
 * Allocate memory of a given size, for DMA access to a given.
 * If DMA is enabled for the device, a DMA buffer will be allocated, otherwise regular memory will be allocated.
 * The allocated buffer is stored in the given pointer */

uint32 dnxc_alloc_dma_mem(
    const int unit,
    const uint8     is_slam,    /* If not FALSE, DMA enabled will be tested for SLAM DMA and not for table DMA */
    void            **mem_ptr,  /* Will hold the pointer to the allocated memory, must be NULL */
    const unsigned  size,       /* memory size in bytes to be allocated */
    const char      *alloc_name /* name of the memory allocation, used for debugging */
)
{
    DNXC_INIT_FUNC_DEFS;
    if (mem_ptr == NULL || alloc_name == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("null parameter" )));
    } else if (*mem_ptr != NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("memory pointer value is not NULL, when attempted to allocate %s"), alloc_name));
    }
    if (is_slam == FALSE ? soc_mem_dmaable(unit, 0, 0) : soc_mem_slamable(unit, 0, 0)) { /* check if DMA is enabled */
        if ((*mem_ptr = soc_cm_salloc(unit, size, alloc_name)) == NULL){
            DNXC_EXIT_WITH_ERR(SOC_E_MEMORY,(_BSL_DNXC_MSG("Failed to allocate %u bytes of DMA memory for %s"), alloc_name));
        }
    } else {
        if ((*mem_ptr = sal_alloc(size, (char*)alloc_name)) == NULL){
            DNXC_EXIT_WITH_ERR(SOC_E_MEMORY,(_BSL_DNXC_MSG("Failed to allocate %u bytes of memory for %s"), alloc_name));
        }
    }
    sal_memset(*mem_ptr, 0, size); /* init the allocated memory to zero */
exit:
    DNXC_FUNC_RETURN;
}

/* deallocate memory of a given size, and store its location in the given pointer */
uint32 dnxc_free_dma_mem(
    const int   unit,
    const uint8 is_slam,  /* If not FALSE, DMA enabled will be tested for SLAM DMA and not for table DMA */
    void        **mem_ptr /* holds the pointer to the allocated memory, will be set to NULL */
)
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(mem_ptr);
    if (mem_ptr == NULL || *mem_ptr == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("null parameter" )));
    }
    if (is_slam == FALSE ? soc_mem_dmaable(unit, 0, 0) : soc_mem_slamable(unit, 0, 0)) { /* check if DMA is enabled */
        soc_cm_sfree(unit, *mem_ptr);
    } else {
        sal_free(*mem_ptr);
    }
    *mem_ptr = NULL;
exit:
    DNXC_FUNC_RETURN;
}


/*
 * Functions to fill memories using SLAM DMA if possible, using a pre-allocated DMA
 * buffer per device, to which the given entry is copied.
 */

static void *dma_buffers[SOC_MAX_NUM_DEVICES] = {0};
static sal_mutex_t dma_buf_mutexes[SOC_MAX_NUM_DEVICES] = {0};

/* Init the dnxc fill table mechanism for a given unit */
uint32 dnxc_init_fill_table(
    const  int unit
)
{
    DNXC_INIT_FUNC_DEFS;
    if (dma_buf_mutexes[unit]==0 && soc_mem_slamable(unit, 0, 0)) { /* check if DMA is enabled */
        DNXC_IF_ERR_EXIT(dnxc_alloc_dma_mem(unit, TRUE, dma_buffers + unit, DNXC_MAX_U32S_IN_MEM_ENTRY * sizeof(uint32), "fill_table"));
        dma_buf_mutexes[unit] = sal_mutex_create("dnxc_fill_table");
    }
exit:
    DNXC_FUNC_RETURN;
}

/* De-init the dnxc fill table mechanism for a given unit */
uint32 dnxc_deinit_fill_table(
    const  int unit
)
{
    DNXC_INIT_FUNC_DEFS;
    if (dma_buffers[unit] != NULL) {
        DNXC_IF_ERR_EXIT(sal_mutex_take(dma_buf_mutexes[unit], sal_mutex_FOREVER));
        DNXC_IF_ERR_EXIT(dnxc_free_dma_mem(unit, TRUE, dma_buffers + unit));
        sal_mutex_destroy(dma_buf_mutexes[unit]);
        dma_buf_mutexes[unit]=0;
    }
exit:
    DNXC_FUNC_RETURN;
}

/* Fill the whole table with the given entry, uses fast DMA filling when run on real hardware */
uint32 dnxc_fill_table_with_entry(
    const int       unit,
    const soc_mem_t mem,        /* memory/table to fill */
    const int       copyno,     /* Memory/table block to fill */
    const void      *entry_data /* The contents of the entry to fill the table with. Does not have to be DMA memory */
  )
{
    int should_release = 0;
    void *buffer = dma_buffers[unit];
#if defined BCM_DNX_SUPPORT && defined(PALLADIUM_BACKDOOR)
    int mem_size;
#endif
#ifdef DNXC_RUNTIME_DEBUG
    sal_time_t start_time = sal_time();
    unsigned unsigned_i;
#endif /* DNXC_RUNTIME_DEBUG */

    DNXC_INIT_FUNC_DEFS;

#ifdef BCM_DNX_SUPPORT
#ifndef PALLADIUM_BACKDOOR
#ifndef JER2_JERICHO_EMULATION_OLD_ACCELERATION_BEHAVIOR
    if (mem == IRR_MCDBm && SOC_IS_JERICHO(unit) && SOC_DNX_CONFIG(unit)->emulation_system) {
        goto fast_exit;
    }
#endif /* JER2_JERICHO_EMULATION_OLD_ACCELERATION_BEHAVIOR */
#else /* PALLADIUM_BACKDOOR */
    mem_size = SOC_MEM_INFO(unit, mem).index_max - SOC_MEM_INFO(unit, mem).index_min +1;

    /* We use backdoor for memories which have more than 256 entries. */
    if ((SOC_IS_JERICHO(unit)) && !SOC_IS_QAX(unit) && SOC_DNX_CONFIG(unit)->emulation_system && (mem_size > 256) && !SOC_IS_JERICHO_PLUS(unit)
#ifdef JER2_JERICHO_EMULATION_OLD_ACCELERATION_BEHAVIOR
      && soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "no_backdoor", 0) != 1
#endif
      ) {
        int array_mem_idx;
        char array_mem_name[1024];
        soc_error_t rv;
#ifdef JER2_JERICHO_EMULATION_OLD_ACCELERATION_BEHAVIOR
        soc_mem_t exceptions[] = {
            FDT_IPT_MESH_MCm,
            EPNI_TX_TAG_TABLEm,
            PPDB_A_FEC_SUPER_ENTRY_BANKm
        };
        int exception_idx;
        const int nof_exceptions = sizeof(exceptions) / sizeof(exceptions[0]);

        /* Check that this is not an exception */
        for (exception_idx = 0; exception_idx < nof_exceptions; exception_idx++) {
            if (mem == exceptions[exception_idx]) {
                break;
            }
        }
#endif /* JER2_JERICHO_EMULATION_OLD_ACCELERATION_BEHAVIOR */

        /* No exception */
        if (
#ifndef JER2_JERICHO_EMULATION_OLD_ACCELERATION_BEHAVIOR
          mem == IRR_MCDBm || mem == EDB_EEDB_BANKm || mem == IHB_DESTINATION_STATUSm
#else
          exception_idx == nof_exceptions
#endif
          ) {
            /* dealing with array memory case*/
            if (SOC_MEM_IS_ARRAY(unit, mem))
            {
                /* we write to each part of the array*/
                for (array_mem_idx = 0; array_mem_idx < SOC_MEM_NUMELS(unit, mem); array_mem_idx++)
                {
                    sal_sprintf(array_mem_name, "%s%d", SOC_MEM_NAME(unit, mem), array_mem_idx);
                    rv = _jer2_arad_palladium_backdoor_dispatch_full_table_write(unit, array_mem_name, entry_data, soc_mem_entry_words(unit, mem));
                    DNXC_IF_ERR_EXIT(rv);
                }
            } else {
                rv = _jer2_arad_palladium_backdoor_dispatch_full_table_write(unit, SOC_MEM_NAME(unit, mem), entry_data, soc_mem_entry_words(unit, mem));
                DNXC_IF_ERR_EXIT(rv);
            }
            
            SOC_EXIT;
        }
    }
#endif /* PALLADIUM_BACKDOOR */
#endif /* BCM_DNX_SUPPORT */

   if (entry_data == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("null buffer" )));
    } else if (!soc_mem_is_valid(unit, mem)) {
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY,(_BSL_DNXC_MSG("Invalid memory for unit" )));
    } else if ((buffer != NULL) && (soc_mem_entry_words(unit, mem) < DNXC_MAX_U32S_IN_MEM_ENTRY)) {
        DNXC_IF_ERR_EXIT(sal_mutex_take(dma_buf_mutexes[unit], sal_mutex_FOREVER));
        should_release = 1;
        sal_memcpy(buffer, entry_data, soc_mem_entry_words(unit, mem) * sizeof(uint32));
    } else {
        buffer = (void*)entry_data;
    }
    DNXC_IF_ERR_EXIT(soc_mem_fill(unit, mem, copyno, buffer));
    
exit:
    if (should_release && sal_mutex_give(dma_buf_mutexes[unit])) {
        _bsl_error(_BSL_DNXC_MSG("Mutex give failed"));
        _rv = SOC_E_FAIL;
    }
#ifdef DNXC_RUNTIME_DEBUG
    if (((unsigned_i = sal_time() - start_time)) >= 5) {
        LOG_INFO(BSL_LS_SOC_INIT, ("==> dnxc_fill_table_with_entry(%s) ran for %u:%2.2u\n",
          SOC_MEM_NAME(unit, mem), unsigned_i / 60, unsigned_i % 60));
    }
    if (dnxc_runtime_debug_per_device[unit].run_stage <= dnxc_runtime_debug_state_initializing) {
        for (unsigned_i = 0; unsigned_i < soc_mem_entry_words(unit, mem); ++unsigned_i) {
            if (((const uint32*)entry_data)[unsigned_i]) { /* assume that entry_data is aligned */
                break;
            }
        }
        if (unsigned_i >= soc_mem_entry_words(unit, mem)) {
            LOG_INFO(BSL_LS_SOC_INIT, ("==> dnxc_fill_table_with_entry(%s) received a zero entry\n", SOC_MEM_NAME(unit, mem)));
        }
    }
#endif /* DNXC_RUNTIME_DEBUG */
#if defined(BCM_DNX_SUPPORT) && !defined(PALLADIUM_BACKDOOR) && !defined(JER2_JERICHO_EMULATION_OLD_ACCELERATION_BEHAVIOR)
    fast_exit:
#endif
    DNXC_FUNC_RETURN;
}

/* Fill the specified part of the table with the given entry, uses fast DMA filling when run on real hardware */
uint32 dnxc_fill_partial_table_with_entry(
    const int       unit,
    const soc_mem_t mem,               /* memory/table to fill */
    const unsigned  array_index_start, /* First array index to fill */
    const unsigned  array_index_end,   /* Last array index to fill */
    const int       copyno,            /* Memory/table block to fill */
    const int       index_start,       /* First table/memory index to fill */
    const int       index_end,         /* Last table/memory index to fill */
    const void      *entry_data        /* The contents of the entry to fill the table with. Does not have to be DMA memory */
  )
{
    int should_release = 0;
    void *buffer = dma_buffers[unit];
#ifdef DNXC_RUNTIME_DEBUG
    sal_time_t start_time = sal_time();
    unsigned unsigned_i;
#endif /* DNXC_RUNTIME_DEBUG */
    DNXC_INIT_FUNC_DEFS;
    if (entry_data == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("null buffer" )));
    } else if (!soc_mem_is_valid(unit, mem)) {
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY,(_BSL_DNXC_MSG("Invalid memory for unit" )));
    } else if (buffer != NULL) {
        if (soc_mem_entry_words(unit, mem) > DNXC_MAX_U32S_IN_MEM_ENTRY) {
            DNXC_EXIT_WITH_ERR(SOC_E_MEMORY,(_BSL_DNXC_MSG("Memory entry is too big for the operation" )));
        }
        DNXC_IF_ERR_EXIT(sal_mutex_take(dma_buf_mutexes[unit], sal_mutex_FOREVER));
        should_release = 1;
        sal_memcpy(buffer, entry_data, soc_mem_entry_words(unit, mem) * sizeof(uint32));
    } else {
        buffer = (void*)entry_data;
    }
    DNXC_IF_ERR_EXIT(soc_mem_array_fill_range(unit, 0, mem, array_index_start, array_index_end, copyno, index_start, index_end, buffer));
exit:
    if (should_release && sal_mutex_give(dma_buf_mutexes[unit])) {
        _bsl_error(_BSL_DNXC_MSG("Mutex give failed"));
        _rv = SOC_E_FAIL;
    }
#ifdef DNXC_RUNTIME_DEBUG
    if (((unsigned_i = sal_time() - start_time)) >= 5) {
        LOG_INFO(BSL_LS_SOC_INIT, ("==> dnxc_fill_partial_table_with_entry(%s) ran for %u:%2.2u\n",
          SOC_MEM_NAME(unit, mem), unsigned_i / 60, unsigned_i % 60));
    }
    if (dnxc_runtime_debug_per_device[unit].run_stage <= dnxc_runtime_debug_state_initializing) {
        for (unsigned_i = 0; unsigned_i < soc_mem_entry_words(unit, mem); ++unsigned_i) {
            if (((const uint32*)entry_data)[unsigned_i]) { /* assume that entry_data is aligned */
                break;
            }
        }
        if (unsigned_i >= soc_mem_entry_words(unit, mem)) {
            LOG_INFO(BSL_LS_SOC_INIT, ("==> dnxc_fill_partial_table_with_entry(%s) received a zero entry\n", SOC_MEM_NAME(unit, mem)));
        }
    }
#endif /* DNXC_RUNTIME_DEBUG */
    DNXC_FUNC_RETURN;
}

/* 
 * This function reads from all cached memories in order to detect and fix SER errors
 */
uint32
soc_dnxc_cache_table_update_all(int unit)
{
    int 
        res = SOC_E_NONE,
        blk, mem, i ,index_cnt;
    soc_memstate_t *mem_state;
    uint32 rd_data[120];
    
    DNXC_INIT_FUNC_DEFS;
      
    for (mem = 0; mem < NUM_SOC_MEM; mem++){
        if (!SOC_MEM_IS_VALID(unit, mem)){
            continue;
        }
        mem_state = &SOC_MEM_STATE(unit, mem);
        MEM_LOCK(unit, mem);
        SOC_MEM_BLOCK_ITER(unit, mem, blk){
            if (!SOC_MEM_BLOCK_VALID(unit, mem, blk)){
                continue;
            }
            if (!(SOC_MEM_INFO(unit, mem).flags & SOC_MEM_FLAG_SER_WRITE_CACHE_RESTORE) && 
                 (mem_state->cache[blk] != NULL) && 
                  CACHE_VMAP_TST(mem_state->vmap[blk], 0) && 
                  !SOC_MEM_TEST_SKIP_CACHE(unit)) 
            {
                index_cnt = soc_mem_index_count(unit, mem);
                for (i = 0 ; i < index_cnt ; i++) {
                    /* we use SOC_MEM_DONT_USE_CACHE to read directly from HW */
                    res = soc_mem_array_read_flags(unit, mem, 0, blk, i, rd_data, SOC_MEM_DONT_USE_CACHE);
                    if (res != SOC_E_NONE) {
                        MEM_UNLOCK(unit, mem);
                        DNXC_IF_ERR_EXIT(res);
                    }
                }
            }
        }
        MEM_UNLOCK(unit, mem);
    } 

exit:
    DNXC_FUNC_RETURN; 
}

/*  
 * Returns the block's indirect read and write registers' size.
 * Supports Jericho and RAMON
 * Used only for wide memory access, thus supports only the blocks which contain
 * the following memories:
 *     Jericho:
 *         IHB_FIFO_DSP_1m
 *         IHB_FIFO_DSP_2m
 *         IHP_FIFO_8_TO_41m
 *         IRE_LAST_RECEIVED_PACKETm
 *         MMU_DRAM_ADDRESS_SPACEm
 *         OCB_OCB_ADDRESS_SPACEm
 *     RAMON:
 *         DCL_CPU_Hm
 */
static int
dnxc_mem_indirect_access_size(int unit, int blktype){
    switch (blktype) {
    /* JER2_JERICHO */
    case SOC_BLK_IRE:
        return 520;
    case SOC_BLK_IHP:
    case SOC_BLK_IPSEC:
    case SOC_BLK_MMU:
    case SOC_BLK_IHB:
    case SOC_BLK_OCB:
    case SOC_BLK_EPNI:
    case SOC_BLK_SPB:
    /* FE 3200 */
    case SOC_BLK_DCL:
    /* QUX */
    case SOC_BLK_NIF:
    /* Jericho PLUS */
    case SOC_BLK_FDT:
        return 640;
    }
    return 0;
}

/* 
 * Read or write a register with a pre-determined block. 
 * Currently is being used only for wide memory indirect access. 
 */
static int
dnxc_reg_access_with_block (int unit, int is_write, soc_reg_t reg, int blk, uint32* data){
    int block, reg_size;
    uint32 addr;
    uint8 at;
    uint32 options = SOC_REG_ADDR_OPTION_NONE;

    if (is_write) {
        options |= SOC_REG_ADDR_OPTION_WRITE;
    }

    addr = soc_reg_addr_get(unit, reg, REG_PORT_ANY, 0, options, &block, &at);
    block = SOC_BLOCK_INFO(unit,blk).cmic; /* override block */
    
    if ((reg == ECI_INDIRECT_COMMAND_RD_DATAr) || (reg == ECI_INDIRECT_COMMAND_WR_DATAr)) {
        reg_size = dnxc_mem_indirect_access_size(unit, SOC_BLOCK_INFO(unit,blk).type) / 32;
    } else if (SOC_REG_IS_ABOVE_64(unit, reg)) {
        reg_size = SOC_REG_ABOVE_64_INFO(unit, reg).size;
    } else if (SOC_REG_IS_64(unit, reg)) {
        reg_size = 2;
    } else {
        reg_size = 1;
    }
               
    if (is_write){
        return soc_direct_reg_set(unit, block, addr, reg_size, data);
    } else {
        return soc_direct_reg_get(unit, block, addr, reg_size, data);
    }
}

/* 
 * Read or write wide memory, supports Jericho and RAMON 
 * returns 0 on success, -1 on fail. 
 */
int
dnxc_mem_array_wide_access(int unit, uint32 flags, soc_mem_t mem, unsigned array_index, int copyno, int index, void *entry_data,
                           unsigned operation) /* operation should be 1 for read and 0 for write */
{
    uint8
        acc_type;
    int
        rv = -1,
        words_left,
        blk;
    uint32  
        data32[1],
        address,
        indirect_size,
        dynamic_access_orig = 0,
        *entry_words = (uint32*)entry_data;
    soc_reg_above_64_val_t
        rd_data;
    uint8 orig_read_mode = SOC_MEM_FORCE_READ_THROUGH(unit);
    uint32 cache_consistency_check = 0;

    assert(operation == 0 || operation == 1); /* write = 0, read = 1 */

#ifdef CRASH_RECOVERY_SUPPORT
        if (BCM_UNIT_DO_HW_READ_WRITE(unit)){
            soc_dnxc_cr_suppress(unit, dnxc_cr_no_support_wide_mem);
        }
#endif /* CRASH_RECOVERY_SUPPORT */

    if (index < 0) {
        index = -index; /* get rid of cache marking, do not support cache */
    }
    
    words_left = soc_mem_entry_words(unit, mem);
    
    indirect_size = dnxc_mem_indirect_access_size(unit, SOC_BLOCK_INFO(unit, SOC_MEM_BLOCK_ANY(unit,mem)).type);
    if (indirect_size == 0) {
        cli_out("unit %d: invalid block for indirect access. blk=%d\n", unit, SOC_MEM_BLOCK_ANY(unit,mem));
        goto done;
    }

    MEM_LOCK(unit, mem);

    /* loop over the blocks */
    SOC_MEM_BLOCK_ITER(unit, mem, blk) {
        if (copyno != COPYNO_ALL && copyno != SOC_CORE_ALL && copyno != blk) {
            continue;
        }

        if ((flags & SOC_MEM_DONT_USE_CACHE) != SOC_MEM_DONT_USE_CACHE) {
            if (operation == 0) {
                _soc_mem_write_cache_update(unit, mem, blk, 0, index, array_index, entry_data, NULL, NULL, NULL);
            } else {
                SOC_MEM_FORCE_READ_THROUGH_SET(unit, 0);
                if (TRUE == _soc_mem_read_cache_attempt(unit, flags, mem, blk, index, array_index, entry_data, NULL, &cache_consistency_check)) {
                    rv = 0;
                    SOC_MEM_FORCE_READ_THROUGH_SET(unit, orig_read_mode);
                    goto done;
                }
            }
        }

        if (soc_mem_is_signal(unit, mem)) {
            /* Save original dynamic memory access value */
            if (dnxc_reg_access_with_block(unit, 0, ECI_ENABLE_DYNAMIC_MEMORY_ACCESSr, blk, data32) != SOC_E_NONE){
                    cli_out("unit %d: Failed reading from reg=ENABLE_DYNAMIC_MEMORY_ACCESSr blk=%d\n", unit, blk);
                    goto done;
            }
            dynamic_access_orig = soc_reg_field_get(unit, ECI_ENABLE_DYNAMIC_MEMORY_ACCESSr, *data32, ENABLE_DYNAMIC_MEMORY_ACCESSf);

            if (dynamic_access_orig == 0) {
                /* Enable dynamic memory access */
                *data32 = 0;
                soc_reg_field_set(unit, ECI_ENABLE_DYNAMIC_MEMORY_ACCESSr, data32, ENABLE_DYNAMIC_MEMORY_ACCESSf, 1);
                if (dnxc_reg_access_with_block(unit, 1, ECI_ENABLE_DYNAMIC_MEMORY_ACCESSr, blk, data32) != SOC_E_NONE){
                    cli_out("unit %d: Failed writing to reg=ENABLE_DYNAMIC_MEMORY_ACCESSr blk=%d (data: %d)\n", unit, blk, *data32);
                    goto done;
                }
            }
        }

        address = soc_mem_addr_get(unit, mem, array_index, blk, index, &acc_type);
        /* set start address (address is being automatically incremented by design) */
        *data32 = 0;
        soc_reg_field_set(unit, ECI_INDIRECT_COMMAND_ADDRESSr, data32, INDIRECT_COMMAND_ADDRf, address);
        soc_reg_field_set(unit, ECI_INDIRECT_COMMAND_ADDRESSr, data32, INDIRECT_COMMAND_TYPEf, operation);
        if (dnxc_reg_access_with_block(unit, 1, ECI_INDIRECT_COMMAND_ADDRESSr, blk, data32) != SOC_E_NONE){
            cli_out("unit %d: Failed writing to reg=INDIRECT_COMMAND_ADDRESS blk=%d (data: %d)\n", unit, blk, *data32);
            goto done;
        }
        
        /* Trigger action automatically on write data */
        *data32 = 0;
        if(operation == 0)
            soc_reg_field_set(unit, ECI_INDIRECT_COMMANDr, data32, INDIRECT_COMMAND_TRIGGER_ON_DATAf, 1);
        else
            soc_reg_field_set(unit, ECI_INDIRECT_COMMANDr, data32, INDIRECT_COMMAND_TRIGGERf, 1);

        soc_reg_field_set(unit, ECI_INDIRECT_COMMANDr, data32, INDIRECT_COMMAND_TIMEOUTf, 0x7fff);
        if (dnxc_reg_access_with_block(unit, 1, ECI_INDIRECT_COMMANDr, blk, data32) != SOC_E_NONE){
            cli_out("unit %d: Failed writing to reg=INDIRECT_COMMAND blk=%d (data: %d)\n", unit, blk, *data32);
            goto done;
        }

        while (words_left > 0)
        {
            /* write data */
            if (operation == 0) {
                if (dnxc_reg_access_with_block(unit, 1, ECI_INDIRECT_COMMAND_WR_DATAr, blk, entry_words) != SOC_E_NONE){
                    cli_out("unit %d: Failed writing to reg=INDIRECT_COMMAND_WR_DATA blk=%d (data: %u)\n", unit, blk, *entry_words);
                    goto done;
                }
            }
            /* Get read data */
            if (operation == 1) { 
                if (dnxc_reg_access_with_block(unit, 0, ECI_INDIRECT_COMMAND_RD_DATAr, blk, rd_data) != SOC_E_NONE){
                    cli_out("unit %d: Failed reading from reg=INDIRECT_COMMAND_RD_DATA blk=%d\n", unit, blk);
                    goto done;
                }
                memcpy(entry_words, rd_data, indirect_size/8);
            }
            
            entry_words += indirect_size/32; 
            words_left  -= indirect_size/32;
        }

        if (soc_mem_is_signal(unit, mem) && dynamic_access_orig == 0) {
            /* Disable dynamic memory access */
            *data32 = 0;
            soc_reg_field_set(unit, ECI_ENABLE_DYNAMIC_MEMORY_ACCESSr, data32, ENABLE_DYNAMIC_MEMORY_ACCESSf, 0);
            if (dnxc_reg_access_with_block(unit, 1, ECI_ENABLE_DYNAMIC_MEMORY_ACCESSr, blk, data32) != SOC_E_NONE){
                cli_out("unit %d: Failed writing to reg=ENABLE_DYNAMIC_MEMORY_ACCESSr blk=%d (data: %d)\n", unit, blk, *data32);
                goto done;
            }
        }

    } /* finished looping over blocks */

    rv = 0;

 done:
    MEM_UNLOCK(unit, mem);
    return rv;
}
/*
 * Procedures related to PEM block access - 'wide' access.
 * {
 */
int
dnx_do_read_table(int unit, soc_mem_t mem, unsigned array_index,
                  int index, int count, uint32 *entry_ptr)
{
    int       kk, ii;
    int       rv ;
    int       copyno ;
/*    int       dump_disable_cache; - It may be need in feature plan */

    rv = 0 ;
/*    dump_disable_cache = 1 ;*/
    if (mem >= NUM_SOC_MEM) {
        LOG_INFO(BSL_LS_APPL_COMMON,
            (BSL_META_U(unit, "%s(): Illegal mem specifier: %d\n"),
            __FUNCTION__,(int)mem)) ;
        rv = -1 ;
        goto exit ;
    }
    copyno = SOC_MEM_BLOCK_MIN(unit, mem) ;
    if (copyno != SOC_MEM_BLOCK_MAX(unit, mem)) {
        LOG_INFO(BSL_LS_APPL_COMMON,
            (BSL_META_U(unit, "%s(): Memory has more than one block: table %s.%s. Num blocks %d\r\n"),
            __FUNCTION__,SOC_MEM_UFNAME(unit, mem), SOC_BLOCK_NAME(unit, copyno), (SOC_MEM_BLOCK_MAX(unit, mem) - copyno))) ;
        rv = -2 ;
        goto exit ;
    }

    for (kk = index; kk < index + count; kk++) {
        /* may be needed in feature plan
          else if(!(dump_disable_cache)) {
           ii = soc_mem_array_read(unit, mem, array_index, copyno, kk, entry_ptr) ;
        } else */ {
           ii = soc_mem_array_read_flags(unit, mem, array_index, copyno, kk, entry_ptr, SOC_MEM_DONT_USE_CACHE);
        }
        if (ii < 0) {
            LOG_INFO(BSL_LS_APPL_COMMON,
                (BSL_META_U(unit, "%s(): Read ERROR: table %s.%s[%d]: %s\n"),
                __FUNCTION__,SOC_MEM_UFNAME(unit, mem), SOC_BLOCK_NAME(unit, copyno), kk, soc_errmsg(ii))) ;
            rv = -3 ;
            goto exit ;
        }
    }
 exit:
    return rv;
}


/*
 * }
 */
#ifdef DNXC_RUNTIME_DEBUG
dnxc_runtime_debug_t dnxc_runtime_debug_per_device[SOC_MAX_NUM_DEVICES] = {{0}};

/* update time counter and print time since start and since last call */
void dnxc_runtime_debug_update_print_time(int unit, const char *string_to_print) {
    unsigned secs_s, mins_s; /* time since start */
    unsigned secs_l, mins_l; /* time since last check */
    dnxc_runtime_debug_t *debug = dnxc_runtime_debug_per_device + unit;
    sal_time_t current_secs = sal_time();
    if (debug->run_stage == dnxc_runtime_debug_state_loading) {
        debug->run_stage = dnxc_runtime_debug_state_initializing;
        debug->last_time = debug->start_time = sal_time();
    }

    secs_s = current_secs - debug->start_time;
    secs_l = current_secs - debug->last_time;
    mins_s = secs_s / 60;
    mins_l = secs_l / 60;
    debug->last_time = current_secs;
    LOG_INFO(BSL_LS_SOC_INIT, ("==> u:%d %s from start: %u:%2.2u:%2.2u  from last: %u:%2.2u:%2.2u\n",
      unit, string_to_print, mins_s / 60, mins_s % 60, secs_s % 60, mins_l / 60, mins_l % 60, secs_l % 60));
}
#endif /* DNXC_RUNTIME_DEBUG */

#undef _ERR_MSG_MODULE_NAME
