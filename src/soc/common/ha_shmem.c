
/*! \file ha_shmem.c
 * $Id$
 *
 * HA shared memory manager.
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
int ha_shmem_ISO_C_forbids_an_empty_source_file = 0;
#if !defined(__KERNEL__) && defined (LINUX)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h> 
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
/* for the mutex type */
#include <sal/core/sync.h>
#include <shared/bsl.h>
#include <soc/ha.h>
#include <soc/ha_internal.h>
#include <shared/alloc.h>

ha_mgr_t *ha_mem[BCM_MAX_NUM_UNITS];

/*!
 * \brief ha_mem_mgr class constructor
 *
 * allocating some resources and set default values.
 *
 * \param [in] name is the shared memory file name (should be /<name>
 *
 * \retval none
 */
void ha_mem_mgr_init(int unit, uint8 enabled, const char *name)
{
    if (!enabled) {
        ha_mem[unit]->enabled = 0;
        return;    
    } else {
        ha_mem[unit]->enabled = 1;
    }
    strcpy(ha_mem[unit]->file_name, name);
    ha_mem[unit]->fh = -1;
    ha_mem[unit]->new_file = 1;
    ha_mem[unit]->mutex = sal_mutex_create("ha_mutex");
    assert (ha_mem[unit]->mutex != NULL);
    ha_mem[unit]->initialized = 0;
    ha_mem[unit]->mem_sect_idx = 0;
}

/*!
 * \brief ha_mem_mgr class destructor.
 *
 * Free all resources associated with the class.
 *
 * \retval none
 */
int ha_destroy(int unit, uint8 delete_file)
{
    int idx;
    
    if (!ha_mem[unit]->enabled) {
        return 0;
    }
    sal_mutex_take(ha_mem[unit]->mutex, sal_mutex_FOREVER);
    
    for (idx = 0; idx < ha_mem[unit]->mem_sect_idx; ++idx) {
        if (ha_mem[unit]->mem_sect[idx].mem_start) {
            munmap((void *)(ha_mem[unit]->mem_sect[idx].mem_start), ha_mem[unit]->mem_sect[idx].sect_len);
        }
    }

    if (ha_mem[unit]->fh != -1) {
        close(ha_mem[unit]->fh);
    }
    if (delete_file) {
        shm_unlink(ha_mem[unit]->file_name);
    }
    sal_mutex_give(ha_mem[unit]->mutex);
    sal_mutex_destroy(ha_mem[unit]->mutex);
    sal_free (ha_mem[unit]);
    ha_mem[unit] = NULL;
    return 0;
}

/*!
 * \brief mem_init initializes the shared memory.
 *
 * This function open shared memory file and map its memory to the memory
 * space of the process.
 *
 * \param [in] size is the 
 * \param [in] clicmd CLI command structure
 * \param [in] feature Optional feature check to be preformed on this command
 *
 * \retval 0 No errors
 * \retval -1 failed to open shared memory file (typically the name was not of
 *         /file)
 * \retval -2 memory mapping of the shared memory into the process memory space
 *         failed
 */
int ha_init(int unit, uint8 enabled, const char *name, int size, uint8 create_new_file)
{
    int file_size;
    void *mmap_ptr;
    ha_mem_blk_hdr_t *first_hdr;
    ha_mem_section_t first_sect;

    /* ha_mem already initialized (allocated) */
    if (ha_mem[unit]) {
        return -1;
    }
    ha_mem[unit] = sal_alloc(sizeof (ha_mgr_t) , "ha memory");
    ha_mem_mgr_init (unit, enabled, name);
    
    if (!enabled) {
        return 0;
    }
    
    sal_mutex_take(ha_mem[unit]->mutex, sal_mutex_FOREVER);
    ha_mem[unit]->sys_page_size = sysconf(_SC_PAGESIZE);
    file_size = ha_mem[unit]->sys_page_size * ((size + ha_mem[unit]->sys_page_size-1) / ha_mem[unit]->sys_page_size);
    ha_mem[unit]->blk_len = file_size;

    if (0 != ha_mgr_open_shared_mem(unit, create_new_file)) {
        LOG_ERROR(BSL_LS_SHARED_SWSTATE, (BSL_META("open_shared_mem failed\n")));
        sal_mutex_give(ha_mem[unit]->mutex);
        return -1;
    }

    /* memory map the file */
    if (MAP_FAILED == (mmap_ptr = mmap(NULL, ha_mem[unit]->blk_len, 
                                       PROT_WRITE | PROT_READ, 
                                       MAP_SHARED, ha_mem[unit]->fh, 0))) {
        LOG_ERROR(BSL_LS_SHARED_SWSTATE, (BSL_META("Failed to mmap errno=%d\n"), errno));
        close (ha_mem[unit]->fh);
        sal_mutex_give(ha_mem[unit]->mutex);
        return -2;
    }
    first_sect.mem_start = (ha_mem_blk_hdr_t *)mmap_ptr;
    first_sect.mem_end = (ha_mem_blk_hdr_t *)((uint8 *)first_sect.mem_start + ha_mem[unit]->blk_len);
    first_sect.free_mem = first_sect.mem_start;
    first_sect.sect_len = ha_mem[unit]->blk_len;
    
    assert (ha_mem[unit]->mem_sect_idx == 0);
    ha_mem[unit]->mem_sect[ha_mem[unit]->mem_sect_idx] = first_sect;
    ha_mem[unit]->mem_sect_idx++;
    
    ha_mem[unit]->free_mem = first_hdr = first_sect.mem_start;

    if (ha_mem[unit]->new_file) {
        first_hdr->signature = HA_MEM_SIGNATURE;
        first_hdr->length = ha_mem[unit]->blk_len;
        first_hdr->state = (uint8)ha_blk_free;
        first_hdr->section = 0;
        first_hdr->prev_offset = ha_mem[unit]->blk_len;
    } else {
        assert(ha_mgr_sanity_check(unit) == 0);
    }
    LOG_DEBUG(BSL_LS_SHARED_SWSTATE, (BSL_META("start=%p, end=%p, len=%d, free=%p\n"), \
            ha_mem[unit]->mem_sect[0].mem_start, \
            ha_mem[unit]->mem_sect[0].mem_end, \
            ha_mem[unit]->blk_len, \
            ha_mem[unit]->mem_sect[0].free_mem));
    sal_mutex_give(ha_mem[unit]->mutex);
    ha_mem[unit]->initialized = 1;
    return 0; 
}

/*!
 * \brief sanity_check verifies the memory validity.
 *
 * This function runs after reopening an existing file. It goes through 
 * the entire memory to verify that every block has valid signature. It also 
 * "fixes" some of the prev_offset fields. These needed to be fixed when 
 * the original allocation was made using multiple sections. When the file 
 * being reopen it will have a single section containing all the sections 
 * that were there in the original memory allocation. 
 *
 * \retval 0 no errors
 * \retval 1 the memory is corrupted
 */
/*
*/
uint8 ha_mgr_sanity_check(int unit)
{
    uint32 section = 0;
    uint32 prev_blk_len = 0;
    ha_mem_blk_hdr_t *blk_hdr = ha_mem[unit]->mem_sect[0].mem_start;
    section = blk_hdr->section;
    do {
        if (blk_hdr->signature != HA_MEM_SIGNATURE) {
            return 1;
        }
        /* search for section transition */
        if (section != blk_hdr->section) {
            blk_hdr->prev_offset = prev_blk_len;
            section = blk_hdr->section;
        }
        blk_hdr->section = 0;   /* everything belongs to the only section */
        /* update prev_blk_len for the next iteration */
        prev_blk_len = blk_hdr->length;
        blk_hdr = (ha_mem_blk_hdr_t *)((uint8_t *)blk_hdr + blk_hdr->length);
    } while (blk_hdr < ha_mem[unit]->mem_sect[0].mem_end);
    /* check the while condition if the file size was too small in ha_mgr_open_shared_mem */
    /* update the prev_offset of the first block */
    blk_hdr = ha_mem[unit]->mem_sect[0].mem_start;
    blk_hdr->prev_offset = prev_blk_len;

    return 0;
}

/*!
 * \brief find a block with matched block ID.
 *
 * Search the entire HA memory for an accoupied block with matching blk_id
 *
 * \param [in] blk_id is the block ID to search for
 *
 * \retval pointer to the block if was found
 * \retval NULL if the block was not found
 */
ha_mem_blk_hdr_t *ha_mgr_find_block(int unit, uint16 blk_id)
{
    int idx;
    ha_mem_blk_hdr_t *blk_hdr;

    /* iterate through all the sections */
   for (idx = 0; idx < ha_mem[unit]->mem_sect_idx; ++idx) {
        blk_hdr = ha_mem[unit]->mem_sect[idx].mem_start;
        /* seach each block in this section */
        do {
            /* validate signature */
            if (blk_hdr->signature != HA_MEM_SIGNATURE) {
                LOG_ERROR(BSL_LS_SHARED_SWSTATE, (BSL_META("invalid Ha block signature\n")));
                assert (0);
                return NULL;
            }
            /* if the block is allocated and the block ID matches then we found it */
            if ((blk_hdr->state == (uint8_t)ha_blk_allocated) && 
                (blk_hdr->blk_id == blk_id)) {
                return blk_hdr;
            }
            /* increment the block pointer to the next block */
            blk_hdr = (ha_mem_blk_hdr_t *)((uint8_t *)blk_hdr + blk_hdr->length);
        } while (blk_hdr < ha_mem[unit]->mem_sect[idx].mem_end); /* bailed out if block pointer points beyond the end */
    }
    return NULL;
}

/*!
 * \brief Find free block of size larger or equal to min_length.
 *
 * Start searching at the section containing the free_mem block. Also start
 * searching from that free_mem block.
 * In the future this module can be optimized by maintaining a linked list of
 * all the free blocks.
 *
 * \param [in] min_length is the minimal length of the free block to search for
 *
 * \retval pointer to the block if was found
 * \retval NULL if the block was not found
 */
ha_mem_blk_hdr_t *ha_mgr_find_free_block(int unit, uint32 min_length)
{
    ha_mem_blk_hdr_t *blk_hdr;
    int section_idx;
    unsigned j;
    unsigned mem_free_section;

    /*
    ** search in all the memory sections
    */
    if (ha_mem[unit]->free_mem) {
        mem_free_section = ha_mem[unit]->free_mem->section;
    } else {
        mem_free_section = 0;
    }

    for (j = 0; j < ha_mem[unit]->mem_sect_idx; j++) {
        section_idx = (j+mem_free_section) % ha_mem[unit]->mem_sect_idx;
        /* start the search at the mem_free block */
        blk_hdr = ha_mem[unit]->mem_sect[section_idx].free_mem;
        do {
            if (blk_hdr->signature != HA_MEM_SIGNATURE) {
                LOG_ERROR(BSL_LS_SHARED_SWSTATE, (BSL_META("invalid Ha block signature\n")));
                assert (0);
                return NULL;
            }

            if ((blk_hdr->state == (uint8_t)ha_blk_free) && 
                (blk_hdr->length >= min_length)) {
                blk_hdr->section = section_idx;
                return blk_hdr;
            }
            /* Increment to the next block */
            blk_hdr = (ha_mem_blk_hdr_t *)((uint8_t *)blk_hdr + blk_hdr->length);
            if (blk_hdr >= ha_mem[unit]->mem_sect[section_idx].mem_end) {
                blk_hdr = ha_mem[unit]->mem_sect[section_idx].mem_start;    /* wrap around */
            }
        } while (blk_hdr != ha_mem[unit]->mem_sect[section_idx].free_mem); /* as long as we are not where we started */
    }
    return NULL;
}

/*!
 * \brief Find the previous block in the chain.
 *
 * If the input doesn't point to the start of the section then use
 * the prev_offset to move the block pointer accordingly
 *
 * \param [in] start is a reference to the block where the function
 *             is looking for its predecesor.
 *
 * \retval pointer to the block if was found
 * \retval NULL if the block was not found
 */
ha_mem_blk_hdr_t *ha_mgr_find_prev_block(int unit, ha_mem_blk_hdr_t *start)
{
    ha_mem_blk_hdr_t *prev_blk_hdr;
    /* check if start points to the beginning of the section */
    if (start == ha_mem[unit]->mem_sect[start->section].mem_start) {
        return NULL;    /* no previous to the start */
    }
    prev_blk_hdr = (ha_mem_blk_hdr_t *)((uint8 *)start - start->prev_offset);
    /* sanity check */
    if ((prev_blk_hdr < ha_mem[unit]->mem_sect[start->section].mem_start) || 
        (prev_blk_hdr > ha_mem[unit]->mem_sect[start->section].mem_end) ||
        (prev_blk_hdr->signature != HA_MEM_SIGNATURE)) {
        LOG_ERROR(BSL_LS_SHARED_SWSTATE, (BSL_META("invalid previous block\n")));
        assert(0);
    }

    prev_blk_hdr->section = start->section;
    return prev_blk_hdr;
}

/*!
 * \brief Resize shared memory section
 *
 * The resize of the shared memory involves two steps:
 * 1. resize the shared memory file
 * 2. Resize the memory map
 * The second step is a bit more complicated. The first attempt is to resize
 * the memory map. If that works everything is fine. The memory map resize may
 * fail due to the ability to extend the current shared memory block. The HA
 * can't allow shared memory relocation which may restrict the the possibility
 * for shared memory remapping.
 * Therefore, if the memory remap fails the HA creates a new memory map that is
 * mapped to the extension is size of the shared file. Internally, we call this
 * memory map a "section". Each section managed by section header that contains
 * information about the header length and free memory.
 *
 * \param [in] size is the extension size
 *
 * \retval pointer to the extended block 
 * \retval NULL if something failed
 */
ha_mem_blk_hdr_t *ha_mgr_section_resize(int unit, int size)
{
  
#if (0)
    int file_size;
    void *mmap_ptr;
    ha_mem_section_t new_sect;
    int last_section = ha_mem[unit]->mem_sect_idx - 1;
    uint32 resize_memory_blocks = (RESIZE_MEMORY_SIZE - 1 + size)/RESIZE_MEMORY_SIZE;
    uint32 addition_mem_size;
    ha_mem_blk_hdr_t *blk_hdr;

    /* set the size to be on a page boundry */
    addition_mem_size = ha_mem[unit]->sys_page_size * 
                    ((resize_memory_blocks*RESIZE_MEMORY_SIZE + ha_mem[unit]->sys_page_size - 1)
                                / ha_mem[unit]->sys_page_size);

    file_size = ha_mem[unit]->blk_len + addition_mem_size;

    /* extend the shared file size */
    if (-1 == ftruncate(ha_mem[unit]->fh, file_size)) {
        LOG_ERROR(BSL_LS_SHARED_SWSTATE, (BSL_META("Failed to trancate the file errno=%d\n"), errno));
        return NULL;
    }
    mmap_ptr = (void *)(ha_mem[unit]->mem_sect[last_section].mem_start);
    /* try to resize the memory map */
    if (
        
        MAP_FAILED == (mmap_ptr = mremap(mmap_ptr, 
                                         ha_mem[unit]->mem_sect[last_section].sect_len,                   
                                         ha_mem[unit]->mem_sect[last_section].sect_len+addition_mem_size, 
                                         0))
                                         
                                         ) {
        LOG_DEBUG(BSL_LS_SHARED_SWSTATE, (BSL_META("Failed to mremap errno=%d\n"), errno));
        /* create another memory map section */
        
        
        if (MAP_FAILED == (mmap_ptr = mmap(NULL, 
                                           addition_mem_size, 
                                           PROT_WRITE | PROT_READ, 
                                           MAP_SHARED, ha_mem[unit]->fh, ha_mem[unit]->blk_len))) {
            LOG_ERROR(BSL_LS_SHARED_SWSTATE, (BSL_META("Failed to mmap errno=%d\n"), errno));
            return NULL;
        }
        
        else {   /* was able to create new memory map section */
            /* set the new section header and add it to the memory section vector */
            new_sect.mem_start = (ha_mem_blk_hdr_t *)mmap_ptr;
            new_sect.mem_end = (ha_mem_blk_hdr_t *)
                        ((uint8_t *)new_sect.mem_start + addition_mem_size);
            new_sect.free_mem = new_sect.mem_start;
            new_sect.sect_len = addition_mem_size;
            /* push_back new_sect to mem_sect array */
            ha_mem[unit]->mem_sect[ha_mem[unit]->mem_sect_idx] = new_sect;
            ha_mem[unit]->mem_sect_idx++;
            last_section++; /* added a new section  */
            blk_hdr = new_sect.mem_start;
        }
    }
    else {  /* remapping the memory worked - simple case */
        blk_hdr = ha_mem[unit]->mem_sect[last_section].mem_end;
        /*
        ** the first block keeps the length of the last block so when 
        ** we add a new section at the end it is easy to determine where 
        ** the previously last block start 
        */
        blk_hdr->prev_offset = ha_mem[unit]->mem_sect[last_section].mem_start->prev_offset;
        ha_mem[unit]->mem_sect[last_section].mem_start->prev_offset = addition_mem_size;

        ha_mem[unit]->mem_sect[last_section].free_mem = blk_hdr;
        ha_mem[unit]->mem_sect[last_section].mem_end = (ha_mem_blk_hdr_t *)
                        ((uint8_t *)ha_mem[unit]->mem_sect[last_section].mem_end + addition_mem_size);
        ha_mem[unit]->mem_sect[last_section].sect_len += addition_mem_size;
    }
    /* Now add the free block at the end of the previous block */
    blk_hdr->signature = HA_MEM_SIGNATURE;
    blk_hdr->length = addition_mem_size;
    blk_hdr->state = (uint8)ha_blk_free;
    blk_hdr->section = last_section;
    ha_mem[unit]->blk_len = file_size;    /* update the total block length */
    ha_mem[unit]->free_mem = blk_hdr;     /* update the global free_mem    */
    LOG_DEBUG(BSL_LS_SHARED_SWSTATE, (BSL_META("start=%p, end=%p, len=%d, free=%p\n"), \
            ha_mem[unit]->mem_sect[0].mem_start, \
            ha_mem[unit]->mem_sect[0].mem_end, \
            ha_mem[unit]->blk_len, \
            ha_mem[unit]->mem_sect[0].free_mem));
    return blk_hdr;
#endif
   return NULL;
}

/*!
 * \brief Open shared memory file
 *
 * Try to open existing file first. If file was not exist create a new shared
 * file. 
 *
 * \retval 0 for success
 * \retval -1 for failure
 */
int ha_mgr_open_shared_mem(int unit, uint8 create_new_file)
{
    struct stat stat_buf;

    if (create_new_file) {
        shm_unlink(ha_mem[unit]->file_name);
    }

    /* Try opening existing shared file */
    ha_mem[unit]->fh = shm_open(ha_mem[unit]->file_name, (int)O_RDWR | (create_new_file ? (int)O_TRUNC : 0), (mode_t)(S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP));
    
    /* file exists and we are in cold boot mode */
    if (ha_mem[unit]->fh != -1 && create_new_file) {
        if (-1 == ftruncate(ha_mem[unit]->fh, ha_mem[unit]->blk_len)) {
            LOG_ERROR(BSL_LS_SHARED_SWSTATE, (BSL_META("Failed to trancate the file errno=%d\n"), errno));
            close (ha_mem[unit]->fh);
            return -1;
        } 
    /* file doesn't exist */
    } else if (-1 == ha_mem[unit]->fh) {
        LOG_VERBOSE(BSL_LS_SHARED_SWSTATE, (BSL_META("new file")));
        /* existing file is not there, create new file */
        ha_mem[unit]->fh = shm_open(ha_mem[unit]->file_name, (int)(O_CREAT | O_RDWR), 
                      (mode_t)(S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP));
        if (-1 == ha_mem[unit]->fh) {
            LOG_ERROR(BSL_LS_SHARED_SWSTATE, (BSL_META("Failed to open shared file errno=%d\n"), errno));
            return -1;
        }
        /* when file created its size set to 0. Adjust it to the desired size */
        if (-1 == ftruncate(ha_mem[unit]->fh, ha_mem[unit]->blk_len)) {
            LOG_ERROR(BSL_LS_SHARED_SWSTATE, (BSL_META("Failed to trancate the file errno=%d\n"), errno));
            close (ha_mem[unit]->fh);
            return -1;
        }
    }
    /* warm boot */
    else {   /* file opened successfully without the create flag */
        if (0 == fstat(ha_mem[unit]->fh, &stat_buf)) { /* get the current file size */
            LOG_VERBOSE(BSL_LS_SHARED_SWSTATE, (BSL_META("file size=%ld\n"), stat_buf.st_size));
            if ((uint32_t)stat_buf.st_size < ha_mem[unit]->blk_len) {
                /* increase the file size if too small */
                if (-1 == ftruncate(ha_mem[unit]->fh, ha_mem[unit]->blk_len)) {
                    LOG_ERROR(BSL_LS_SHARED_SWSTATE, (BSL_META("Failed to trancate the file errno=%d\n"), errno));
                    close (ha_mem[unit]->fh);
                    return -1;
                }
            }
            else {
                /* update the blk_len according to the file size */
                ha_mem[unit]->blk_len = stat_buf.st_size;   
            }
        }
        assert(!create_new_file);
        ha_mem[unit]->new_file = 0;
        LOG_VERBOSE(BSL_LS_SHARED_SWSTATE, (BSL_META("file exist\n")));
    }
    return 0;
}

/* return 1 if there is already allocated memory for this module and sub id */
int ha_mem_is_already_alloc(int unit, unsigned char mod_id, 
                          unsigned char sub_id)
{
    /* blk_hdr points to the first allocated block */
    ha_mem_blk_hdr_t *blk_hdr;
    uint16 blk_id = (mod_id << 8) | sub_id;

    blk_hdr = ha_mgr_find_block(unit, blk_id);
    if (!blk_hdr) {
        return 0;
    }
    return 1;
}

/*!
 * \brief Allocate HA memory block for the application
 *
 * This function searches for a block with matches mod and sub ID.
 * If such a block was not found the function allocates a new memory block
 * of the proper length and returns a pointer to the memory.
 * The version is used only in case that a matched block was found. In
 * this case if the versions are different the function will call
 * a method that will upgrade/downgrade the structure to the required
 * version. The structure signature identifies the schema that was used
 * to generate the data structure contained in this block
 *
 * \param [in] mod_id is the module ID
 * \param [in] sub_id is the sub module ID
 * \param [in] version is the application structure version 
 * \param [in] struct_sig is a signature over the structure schema
 * \param [in/out] length is a pointer contains the desired length The
 *        length also contains the actual allocated memory length.
 *
 * \retval pointer to the allocated block
 * \retval NULL if the block was not allocated
 */
void *ha_mem_alloc(int unit, unsigned char mod_id, 
                          unsigned char sub_id, 
                          unsigned char version, 
                          unsigned int struct_sig,
                          unsigned int *length)
{
    /* blk_hdr points to the first allocated block */
    ha_mem_blk_hdr_t *blk_hdr;
    uint16 blk_id = (mod_id << 8) | sub_id;
    uint32 space_to_allocate;
    ha_mem_blk_hdr_t *new_blk_hdr = NULL;
    
    /* transparent */
    if (!ha_mem[unit]->enabled) {
        return malloc(*length);
    }
    sal_mutex_take(ha_mem[unit]->mutex, sal_mutex_FOREVER);
    /* search for a block with match mod and sub ID */
    blk_hdr = ha_mgr_find_block(unit, blk_id);
    if (!blk_hdr) {   /* buffer was not allocated before */
        LOG_VERBOSE(BSL_LS_SHARED_SWSTATE, 
                        (BSL_META("allocating new buffer mod_id=%d, sub_id=%d free=%p\n"), 
                                    mod_id, sub_id, ha_mem[unit]->free_mem));
        /* make sure that the length is multiplication of 8 */
        space_to_allocate = ((*length + sizeof(ha_mem_blk_hdr_t) + 7) >> 3) << 3;
        /* now we assume that we will find a free block large enough. */
        blk_hdr = ha_mgr_find_free_block(unit, space_to_allocate);
        if (!blk_hdr) {
            LOG_VERBOSE(BSL_LS_SHARED_SWSTATE, (BSL_META("calling section_resize\n")));
            /* if we can't find free block that is large enough we need to 
             * extend the shared memory */
            blk_hdr = ha_mgr_section_resize(unit, space_to_allocate);
            if (!blk_hdr) {  /* shared memory extension fails? */
                sal_mutex_give(ha_mem[unit]->mutex);
                return NULL;
            }
        }

        /* set the new free block if some space had left */
        if (blk_hdr->length > space_to_allocate + sizeof(ha_mem_blk_hdr_t)) {
            new_blk_hdr = (ha_mem_blk_hdr_t *)((uint8_t *)blk_hdr + space_to_allocate);
            new_blk_hdr->length = blk_hdr->length - space_to_allocate;
            new_blk_hdr->signature = HA_MEM_SIGNATURE;
            new_blk_hdr->state = (uint8_t)ha_blk_free;
            new_blk_hdr->section = blk_hdr->section;
            blk_hdr->length = space_to_allocate;
            new_blk_hdr->prev_offset = space_to_allocate;
            /* if this is the last block need to adjust the prev offset of the first block */
            if ((ha_mem_blk_hdr_t *)
                ((uint8_t *)new_blk_hdr + new_blk_hdr->length) == ha_mem[unit]->mem_sect[blk_hdr->section].mem_end) {
                ha_mem[unit]->mem_sect[blk_hdr->section].mem_start->prev_offset = new_blk_hdr->length;
            }
        }
        blk_hdr->blk_id = blk_id;
        blk_hdr->version = version;
        blk_hdr->state = (uint8_t)ha_blk_allocated;
        if (blk_hdr == ha_mem[unit]->free_mem) {
            /* find new value for the free mem  */
            if (new_blk_hdr) {
                ha_mem[unit]->free_mem = new_blk_hdr;
            } else {  /* need to find free block */
                ha_mem[unit]->free_mem = ha_mgr_find_free_block(unit, (uint32)(sizeof(ha_mem_blk_hdr_t)+1));
                if (!ha_mem[unit]->free_mem) {         /* make sure that free_mem is not null */
                    ha_mem[unit]->free_mem = ha_mem[unit]->mem_sect[0].mem_start;
                    ha_mem[unit]->free_mem->section = 0;
                }
            }
        }
    } else {
        *length = blk_hdr->length - sizeof(ha_mem_blk_hdr_t);
    }

    sal_mutex_give(ha_mem[unit]->mutex);
    return (void *)((uint8 *)blk_hdr + sizeof(ha_mem_blk_hdr_t));
}

/*!
 * \brief reallocate existing memory 
 *
 * If the new desired size is smaller/equal to the current block length
 * do nothing. Otherwise, allocate new memory block using the length
 * parameter, copy the old memory onto the new block and free the old block
 *
 * \param [in] mem is the old memoy handle
 * \param [in/out] length is a pointer contains the desired length The
 *        length also contains the actual allocated memory length.
 *
 * \retval pointer to the block if was reallocated
 * \retval NULL otherwise
 */
void *ha_mem_realloc(int unit, void *mem, unsigned int *length)
{
    ha_mem_blk_hdr_t *blk_hdr;
    void *new_blk_hdr;

    /* transparent */
    if (!ha_mem[unit]->enabled) {
        return realloc(mem, *length);
    }    
    
    if (!mem) {
        return NULL;
    }
    blk_hdr = (ha_mem_blk_hdr_t *)((uint8 *)mem - sizeof(ha_mem_blk_hdr_t));
    /* sanity check */
    if ((blk_hdr->signature != HA_MEM_SIGNATURE) || 
        (blk_hdr->state != (uint8)ha_blk_allocated)) {
        return NULL;
    }

    if (blk_hdr->length >= *length) {  /* is the new length shorter? */
        return mem;
    }
    new_blk_hdr = ha_mem_alloc(unit, (uint8_t)(blk_hdr->blk_id >> 8),
                               (uint8_t)(blk_hdr->blk_id & 0xFF),
                               blk_hdr->version,
                               blk_hdr->struct_sig,
                               length);
    if (!new_blk_hdr) {
        return NULL;
    }
    memcpy (new_blk_hdr, 
            mem, 
            blk_hdr->length - sizeof(ha_mem_blk_hdr_t));

    /* free the old block */
    ha_mem_free(unit, mem);
     
    return new_blk_hdr;
}

/*!
 * \brief free memory block back into the pool
 *
 * Free memory tries to find adjacent free memory blocks to concatinate them
 * into a larger memory block
 *
 * \param [in] mem is the old memoy handle
 *
 * \retval 0 on success
 * \retval -1 otherwise
 */
int ha_mem_free(int unit, void *mem)
{
    ha_mem_blk_hdr_t *blk_hdr = (ha_mem_blk_hdr_t *)((uint8 *)mem - sizeof(ha_mem_blk_hdr_t));
    ha_mem_blk_hdr_t *adj_blk_hdr;

    /* transparent */
    if (!ha_mem[unit]->enabled){
        sal_free(mem);
        return 0;
    }
    
    if (!mem) {
        return -1;
    }
    /* sanity check that the block is valid */
    if ((blk_hdr->signature != HA_MEM_SIGNATURE) ||
        (blk_hdr->state != (uint8_t)ha_blk_allocated)) {
        return -1;
    }

    sal_mutex_take(ha_mem[unit]->mutex, sal_mutex_FOREVER);
    /* free the block */
    blk_hdr->state = (uint8_t)ha_blk_free;
    /* if the next block is free merge them together. */
    adj_blk_hdr = (ha_mem_blk_hdr_t *)((uint8_t *)blk_hdr + blk_hdr->length);
    if (adj_blk_hdr < ha_mem[unit]->mem_sect[blk_hdr->section].mem_end) {
        if (adj_blk_hdr->signature != HA_MEM_SIGNATURE) {
            LOG_ERROR(BSL_LS_SHARED_SWSTATE, (BSL_META("invalid HA block signature\n")));
            assert (0);
        }
        if (adj_blk_hdr->state == (uint8_t)ha_blk_free) {
            blk_hdr->length += adj_blk_hdr->length; /* concatenate the next block */
            /* adjust the previous offset of the next block */
            adj_blk_hdr = (ha_mem_blk_hdr_t *)((uint8 *)blk_hdr + blk_hdr->length);
            if (adj_blk_hdr < ha_mem[unit]->mem_sect[blk_hdr->section].mem_end) {
                adj_blk_hdr->prev_offset = blk_hdr->length;
            }
        }
    }

    /* now see if this block can be concatenated with the previous block */
    adj_blk_hdr = ha_mgr_find_prev_block(unit, blk_hdr);
    if (adj_blk_hdr && (adj_blk_hdr->state == (uint8_t)ha_blk_free)) {
        adj_blk_hdr->length += blk_hdr->length; /* add blk_hdr to the previous block */
        /* adjust the previous offset of the next block */
        blk_hdr = (ha_mem_blk_hdr_t *)((uint8 *)adj_blk_hdr + adj_blk_hdr->length);
        if (blk_hdr < ha_mem[unit]->mem_sect[adj_blk_hdr->section].mem_end) {
            blk_hdr->prev_offset = adj_blk_hdr->length;
        }
    }
    sal_mutex_give(ha_mem[unit]->mutex);
    return 0;
}

#endif /* __KERNEL__ */

