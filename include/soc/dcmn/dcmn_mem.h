/*
 * $Id: dcmn_mem.h,v 1.2 Broadcom SDK $
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _SOC_DCMN_MEM_H
#define _SOC_DCMN_MEM_H
#include <soc/mcm/allenum.h>

#define DCMN_MAX_U32S_IN_MEM_ENTRY 20

/*  check if dnx table is dynamic */
int dcmn_tbl_is_dynamic(int unit,soc_mem_t mem);

/* check if a given mem contain one of the fields apear in given list*/
int dcmn_mem_contain_one_of_the_fields(int unit,const soc_mem_t mem,soc_field_t *fields);

/* Allocate memory of a given size, and store its location in the given pointer */
uint32 dcmn_alloc_mem(
    const int unit,
    void      *mem_ptr,        /* output: Will hold the pointer to the allocated memory, must be NULL. The real type of the argument is void** is not used to avoid compilation warnings */
    const unsigned size,       /* memory size in bytes to be allocated */
    const char     *alloc_name /* name of the memory allocation, used for debugging */
);

/* deallocate memory of a given size, and store its location in the given pointer */
uint32 dcmn_free_mem(
    const int unit,
    void **mem_ptr /* holds the pointer to the allocated memory, will be set to NULL */
);

/* deallocate memory of a given size, and store its location in the given pointer */
void dcmn_free_mem_if_not_null(
    const int unit,
    void **mem_ptr /* holds the pointer to the allocated memory, will be set to NULL */
);


/*
 * Allocate memory of a given size, for DMA access to a given.
 * If DMA is enabled for the device, a DMA buffer will be allocated, otherwise regular memory will be allocated.
 * The allocated buffer is stored in the given pointer */

uint32 dcmn_alloc_dma_mem(
    const int unit,
    const uint8     is_slam,    /* If not FALSE, DMA enabled will be tested for SLAM DMA and not for table DMA */
    void            **mem_ptr,  /* Will hold the pointer to the allocated memory, must be NULL */
    const unsigned  size,       /* memory size in bytes to be allocated */
    const char      *alloc_name /* name of the memory allocation, used for debugging */
);

/* deallocate memory of a given size, and store its location in the given pointer */
uint32 dcmn_free_dma_mem(
    const int unit,
    const uint8 is_slam, /* If not FALSE, DMA enabled will be tested for SLAM DMA and not for table DMA */
    void  **mem_ptr      /* holds the pointer to the allocated memory, will be set to NULL */
);


/*
 * Functions to fill memories using SLAM DMA if possible, using a pre-allocated DMA
 * buffer per device, to which the given entry is copied.
 */

/* Init the dcmn fill table mechanism for a given unit */
uint32 dcmn_init_fill_table(
    const  int unit
);

/* De-init the dcmn fill table mechanism for a given unit */
uint32 dcmn_deinit_fill_table(
    const  int unit
);

/* Fill the whole table with the given entry, uses fast DMA filling when run on real hardware */
uint32 dcmn_fill_table_with_entry(
    const int       unit,
    const soc_mem_t mem,        /* memory/table to fill */
    const int       copyno,     /* Memory/table block to fill */
    const void      *entry_data /* The contents of the entry to fill the table with. Does not have to be DMA memory */
  );

/* Fill the specified part of the table with the given entry, uses fast DMA filling when run on real hardware */
uint32 dcmn_fill_partial_table_with_entry(
    const int       unit,
    const soc_mem_t mem,               /* memory/table to fill */
    const unsigned  array_index_start, /* First array index to fill */
    const unsigned  array_index_end,   /* Last array index to fill */
    const int       copyno,            /* Memory/table block to fill */
    const int       index_start,       /* First table/memory index to fill */
    const int       index_end,         /* Last table/memory index to fill */
    const void      *entry_data        /* The contents of the entry to fill the table with. Does not have to be DMA memory */
  );

/* 
 * This function reads from all cached memories in order to detect and fix SER errors
 */
uint32
soc_dcmn_cache_table_update_all(int unit);

int
dcmn_mem_array_wide_access(int unit, uint32 flags, soc_mem_t mem, unsigned array_index, int copyno, int index, void *entry_data,unsigned operation);

/*
 * Structures and prototypes related to PEM block access.
 * {
 */
int
dpp_do_read_table(int unit, soc_mem_t mem, unsigned array_index,
                  int index, int count, uint32 *entry_ptr) ;

/*
 * }
 */

/* SBUS defines */
#ifdef BCM_SBUSDMA_SUPPORT
#define SOC_DCMN_MAX_SBUSDMA_CHANNELS    3
#define SOC_DCMN_TDMA_CHANNEL            0
#define SOC_DCMN_TSLAM_CHANNEL           1
#define SOC_DCMN_DESC_CHANNEL            2
#define SOC_DCMN_MEM_CLEAR_CHUNK_SIZE    4 /* Use one entry buffers for SLAM DMA */
#endif /*BCM_SBUSDMA_SUPPORT*/

#endif /*_SOC_DCMN_MEM_H*/
