/*
 * $Id: hash.h,v 1.17 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * TMU DM defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_TMU_HASH_H_
#define _SBX_CALADN3_TMU_HASH_H_

#include <soc/sbx/caladan3/tmu/tmu_config.h>


#define SOC_SBX_TMU_HASH_CONTROL_WORD_SIZE_BITS (64)

#define SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS (128)

#define SOC_SBX_CALADAN3_TMU_HASH_DEF_CHAIN_LEN (8)
#define SOC_SBX_CALADAN3_TMU_HASH_DEF_MAX_CHAIN_LEN (10)

#define _TMU_DEF_HASH_MISS_ENTRY_IDX_ (0)

typedef enum soc_sbx_tmu_hash_key_type_e {
    SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS=0,
    SOC_SBX_CALADAN3_TMU_HASH_KEY_176_BITS,
    SOC_SBX_CALADAN3_TMU_HASH_KEY_304_BITS,
    SOC_SBX_CALADAN3_TMU_HASH_KEY_424_BITS,
    SOC_SBX_CALADAN3_TMU_HASH_KEY_144_BITS,
    SOC_SBX_CALADAN3_TMU_HASH_KEY_MAX        
} soc_sbx_tmu_hash_key_type_e_t;


#define SOC_SBX_CALADAN3_TMU_HASH_KEY_MAX_BITS (442)

typedef enum tmu_hash_node_type_e {
    TMU_HASH_NODE_BUCKET,
    TMU_HASH_NODE_CHAIN,
    TMU_HASH_NODE_MAX
} tmu_hash_node_type_e_t;


typedef struct tmu_hash_entry_s {
    dq_t list_node;
    tmu_hash_node_type_e_t type;
    dq_t chain_list;  /* chain list if this is a bucket node */
    uint32 bucket_idx; 
    uint32 value[BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS)];
    uint32 *key; /* dynamic memory based on key size */
} tmu_hash_entry_t;


struct tmu_hash_table_s;
typedef struct tmu_hash_table_s tmu_hash_table_t;

typedef int (*tmu_hash_add_call_back_f)(int unit, tmu_hash_table_t *table, 
                                        tmu_hash_entry_t *entry, soc_sbx_tmu_hash_key_type_e_t type);
typedef int (*tmu_hash_del_call_back_f)(int unit, tmu_hash_table_t *table, 
                                        tmu_hash_entry_t *entry, soc_sbx_tmu_hash_key_type_e_t type);


typedef struct tmu_hash_table_node_s {
    dq_t list_node;
    dq_t bucket_list; /* list of buckets if this is a table node */
} tmu_hash_table_node_t;


struct tmu_hash_table_s {
    int                  tmu_hw_root_table_id;
    int                  max_num_entries; /* required capacity */
    int                  host_table_size; /* level1 hash table size, power of 2 */
    int                  host_max_num_entries_msb_pos;
    int                  device_max_num_entries_msb_pos;
    int                  key_size_bits;
    int                  num_entries; /* number of entries currently present */
    int                  max_chain_length; /* maximum chain length on device */
    int                  max_chains; /* maximum chains on device */
    int                  num_chains; /* number of chains on host database */
    int                  host_hw_fib_get; /* enable hw_fib_get when host queries table */
#define TMU_HASH_DEFAULT_LOADING_FACTOR (2)
    dq_t                 entry_free_list;
    dq_t                 table_node_free_list;
    tmu_hash_table_node_t **table;
    /*#define TMU_HASH_TBL_LOCK_SUPPORT */
    sal_mutex_t          lock;
#define TMU_HASH_ALLOC_BLK_CNT (256)
    int                  alloc_blk_cnt;  
    int                  num_free_entry;
    int                  num_free_table_nodes;
    /* call back functions */
    tmu_hash_add_call_back_f commit_add_f;
    tmu_hash_del_call_back_f commit_del_f;
};


typedef struct soc_sbx_tmu_hash_param_s {
    uint32 capacity; /* capacity required */
    soc_sbx_tmu_hash_key_type_e_t key_type;
    uint32 chain_length;
    uint32 num_hash_table_entries;
    uint32 num_chain_table_entries; /* units of chain length */
} soc_sbx_tmu_hash_param_t; 


typedef struct soc_sbx_tmu_hash_cfg_s {
    dq_t tmu_hash_dbase_list_node;
    soc_sbx_tmu_hash_param_t param;
    tmu_hash_table_t *table;
    int table_id;
    int fifoid;
} soc_sbx_tmu_hash_cfg_t;
typedef struct soc_sbx_tmu_hash_cfg_s *soc_sbx_tmu_hash_handle_t;




typedef struct soc_sbx_tmu_hash_dbase_s {
    sal_mutex_t mutex;  
    dq_t hash_list; /* list of hash tables */
#define TMU_HASH_MAX_TABLE                      (8) 
#define TMU_HASH_MAX_TABLE_ENTRY                (256) 
#define TMU_HASH_MAX_ADJUST_SELECT              (8) 
    uint32 *hashtable; /* hash random table on device */
    uint32 *hash_adjust;
} soc_sbx_tmu_hash_dbase_t;

#define SOC_SBX_MAX_TMU_HASH_TABLES            (128)
#define SOC_SBX_HASHTABLE_SIZE (sizeof(uint32) * TMU_HASH_MAX_TABLE * TMU_HASH_MAX_TABLE_ENTRY)


typedef struct soc_sbx_tmu_chain_alloc_handle_s {
    dq_t free_chain_list_node;
    int table_id;
    int size; /* number of chains */
    int free_ptr; /* points to free chain which could be pushed into fifo */
    int fifoid; /* redundant but could be useful */
} soc_sbx_tmu_chain_alloc_handle_t;

#define SOC_SBX_TMU_DEF_HPCM_SIZE (4*1024)
#define SOC_SBX_TMU_DEF_FREE_LVL (64)
#define SOC_SBX_TMU_DEF_ALLOC_LVL (-1)

typedef struct soc_sbx_tmu_free_fifo_entry_s {
    dq_t list_node;
    int table_id;
    int entry_id;
    void *handle; 
} soc_sbx_tmu_free_fifo_entry_t;

typedef struct soc_sbx_tmu_free_fifo_heap_mgr_s {
    dq_t free_hpcm_list; /* list of hpcm from where entries could be allocated */
    dq_t full_hpcm_list; /* list of hpcm which are fully allocated  */
    int  max_free_lvl; /* if number of free blocks exceeds this limit some hpcm will be 
                           * release back to system memory pool */
    int  max_alloc_lvl; /* policy or limit of maximum hpcm blocks 
                           * <0, no limit on dynamic growth, >0 limit on dynamic growth */
} soc_sbx_tmu_free_fifo_heap_mgr_t;

typedef struct soc_sbx_tmu_free_fifo_info_s {
    int  key_size_bits; /* key size */
    int  chain_length;  /* length on chain on one table entry */
    dq_t chain_list;    /* list of chain table allocators */
    int  tbl_fifo_map[SOC_SBX_CALADAN3_TMU_MAX_TABLE];
    int  refcount;      /* number of users using this fifo */
    dq_t recycle_entry_list; /* list of soc_sbx_tmu_free_fifo_entry_t */
    soc_sbx_tmu_free_fifo_heap_mgr_t hpcm_mgr;
} soc_sbx_tmu_free_fifo_info_t;

typedef struct soc_sbx_tmu_hash_recycle_dma_mgr_s {
    VOL uint32 *dma_buffer;
    VOL uint32 *ring_pointer;
#define SOC_SBX_TMU_RECYCLE_RESP_DMA_MGR_RING_SIZE (4 * 1024)   
#define SOC_SBX_TMU_RECYCLE_CONSUME_WINDOW (128)   
    VOL int dma_buffer_len;
    int channel;
} soc_sbx_tmu_hash_recycle_dma_mgr_t;

typedef struct soc_sbx_tmu_hash_fifo_mgr_s {
    sal_mutex_t mutex; 
    sal_sem_t fifo_trigger; 
    sal_thread_t thread_id;
    VOL int thread_running;   /* Input signal to thread */
    int thread_priority; /* info */
    /* dma */
    VOL uint32 *dma_buffer;
    VOL int dma_buffer_len;
#define TMU_HASH_DEF_LOW_MARK_THRESH (25) /* hardware default is 25% */
#define TMU_HASH_DEF_FLOW_CTRL_DISABLE_OFFSET (4) /* adjust free fifo feed so it never gets filled up to flow control */
    int low_lvl_threshold;
#define TMU_HASH_MAX_FREE_PAGE_FIFO (4)
#define TMU_HASH_FREE_PAGE_FIFO_SIZE (1024) /* size of free fifo */
    soc_sbx_tmu_free_fifo_info_t fifo_cfg[TMU_HASH_MAX_FREE_PAGE_FIFO];
    soc_sbx_tmu_hash_recycle_dma_mgr_t recycle_ring_mgr;
} soc_sbx_tmu_hash_fifo_mgr_t;


extern soc_sbx_tmu_hash_dbase_t* tmu_hash_dbase_get(int unit);
extern soc_sbx_tmu_hash_fifo_mgr_t* tmu_hash_fifo_mgr_get(int unit);


/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_init
 * Purpose:
 *    Initialize hash database
 */
extern int soc_sbx_caladan3_tmu_hash_init(int unit, uint8 bypass);
/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_uninit
 * Purpose:
 *    cleanup
 */
extern int soc_sbx_caladan3_tmu_hash_uninit(int unit);

/*
 *
 * Function:
 *     soc_sbx_caladan3_hash_table_alloc
 * Purpose:
 *     Allocate a Hash table
 */
extern int soc_sbx_caladan3_tmu_hash_table_alloc(int unit, 
                                                 soc_sbx_tmu_hash_param_t *param,
                                                 soc_sbx_tmu_hash_handle_t *handle);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_table_free
 * Purpose:
 *     Free an preallocated hash table
 */
extern int soc_sbx_caladan3_tmu_hash_table_free(int unit,
                                                soc_sbx_tmu_hash_handle_t handle);


/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_entry_add
 * Purpose:
 *     add entry to hash table
 */
extern int soc_sbx_caladan3_tmu_hash_entry_add(int unit, 
                                               soc_sbx_tmu_hash_handle_t handle,
                                               uint32 *key, uint32 *value);
     
/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_entry_delete
 * Purpose:
 *     delete entry from hash table
 */
extern int soc_sbx_caladan3_tmu_hash_entry_delete(int unit, 
                                                  soc_sbx_tmu_hash_handle_t handle, 
                                                  uint32 *key);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_entry_update
 * Purpose:
 *     update entry on hash table
 */
extern int soc_sbx_caladan3_tmu_hash_entry_update(int unit,
                                                  soc_sbx_tmu_hash_handle_t handle, 
                                                  uint32 *key, 
                                                  uint32 *value);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_entry_add
 * Purpose:
 *     add entry to hash table
 */
extern int soc_sbx_caladan3_tmu_hash_entry_get(int unit, 
                                               soc_sbx_tmu_hash_handle_t handle, 
                                               uint32 *key, 
                                               uint32 *value);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_entry_hw_get
 * Purpose:
 *     get hash entry from hardware
 */
extern int soc_sbx_caladan3_tmu_hash_entry_hw_get(int unit, 
                                               soc_sbx_tmu_hash_handle_t handle, 
                                               uint32 *key, 
                                               uint32 *value);


/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_table_id_get
 * Purpose:
 *     get table id from hash handle table
 */
extern int soc_sbx_caladan3_tmu_hash_table_id_get(int unit, 
					       soc_sbx_tmu_hash_handle_t handle, 
					       uint32 *id);
/*
 *
 * Function:
 *    soc_sbx_caladan3_tmu_hash_iterator_first
 * Purpose:
 *    initialize iterator
 * returns SOC_E_LIMIT if uninitialized
 */
extern int soc_sbx_caladan3_tmu_hash_iterator_first(int unit, 
                                                    soc_sbx_tmu_hash_handle_t handle, 
                                                    uint32 *key);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_iterator_next
 * Purpose:
 *   iterates
 *   returns SOC_E_LIMIT if uninitialized
 */
extern int soc_sbx_caladan3_tmu_hash_iterator_next(int unit, 
                                                   soc_sbx_tmu_hash_handle_t handle,
                                                   uint32 *key,
                                                   uint32 *next_key);
/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_miss_pattern_int
 * Purpose:
 *   Intialize miss pattern hash entry. Hash lookup returns this value 
 *   when hash lookup fails. For now 
 */
extern int soc_sbx_caladan3_tmu_hash_miss_pattern_init(int unit, 
                                                       unsigned int *entry);

/*
 *   Function
 *     soc_sbx_caladan3_tmu_hash_fifo_feed
 *   Purpose
 *      Wakeup TMU fifo manager thread when the free fifo falls below configured
 *      low water mark threshold
 */
extern int soc_sbx_caladan3_tmu_hash_fifo_feed_trigger(int unit);

extern int soc_sbx_caladan3_tmu_hash_table_fifoid_get(int unit, 
                                                      soc_sbx_tmu_hash_handle_t handle,
                                                      int *fifoid);
extern int soc_sbx_tmu_hash_table_fifo_push_empty_get(int unit, int fifoid);

extern int soc_sbx_caladan3_tmu_hash_entry_verify(int unit, 
                                                  soc_sbx_tmu_hash_handle_t handle,
                                                  uint32 *key, 
                                                  uint32 *value,
                                                  uint8   dump);

extern int soc_sbx_caladan3_tmu_hash_bulk_delete(int unit, 
						 soc_sbx_tmu_hash_handle_t handle, 
						 uint32 *filter_key,
						 uint32 *filter_key_mask,
						 uint32 *filter_value,
						 uint32 *filter_value_mask);

extern unsigned int soc_sbx_tmu_hash_fifo_crc(int unit);


#endif /* _SBX_CALADN3_TMU_HASH_H_ */
