/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: tmu.h,v 1.35.16.4 Broadcom SDK $
 *
 * TMU defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_TMU_H_
#define _SBX_CALADN3_TMU_H_


#include <soc/sbx/caladan3/tmu/tmu_config.h>
#include <soc/sbx/caladan3/tmu/cmd.h>
#include <soc/sbx/caladan3/tmu/taps/taps.h>

#define SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO (0)
#define SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO (2)

#define BCM_SOC_TMU_USE_LOCKS

#ifdef BCM_SOC_TMU_USE_SPINLOCK
#undef BCM_SOC_TMU_USE_SPINLOCK
#endif

#ifdef BCM_SOC_TMU_USE_LOCKS
#define  TMU_USE_LOCKS
#define  TMU_CMD_USE_LOCKS
#define  TMU_DM_USE_LOCKS
#define  TMU_DMA_USE_LOCKS
#endif

#ifdef BCM_SOC_TMU_USE_SPINLOCK
#define  TMU_USE_SPINLOCK
#define  TMU_CMD_USE_SPINLOCK
#define  TMU_DMA_USE_SPINLOCK
#endif

#ifdef  TMU_USE_LOCKS
#ifdef  TMU_USE_SPINLOCK
#include <pthread.h>
#define TMU_LOCK(unit)      pthread_spin_lock(&_tmu_dbase[unit]->tmu_spin_lock);
#define TMU_UNLOCK(unit)    pthread_spin_unlock(&_tmu_dbase[unit]->tmu_spin_lock);
#else
#define TMU_LOCK(unit)      sal_mutex_take(_tmu_dbase[unit]->tmu_mutex, sal_mutex_FOREVER);
#define TMU_UNLOCK(unit)    sal_mutex_give(_tmu_dbase[unit]->tmu_mutex); 
#endif
#else
#define TMU_LOCK(unit)    
#define TMU_UNLOCK(unit)    
#endif

/* partition */
#define SOC_SBX_CALADAN3_TMU_DEF_NUM_PARTITION (1)

typedef struct soc_sbx_caladan3_dram_config_s {
    int num_dram;
    int num_dram_banks;
    int dram_size_mbytes; /* 2 pow 20 */
    int dram_row_size_bytes; 
} soc_sbx_caladan3_dram_config_t;

typedef struct soc_sbx_caladan3_region_dram_map_s {
    uint32 dram; /* DRAM index */
    uint32 bank; /* Bank index within DRAM */
} soc_sbx_caladan3_region_dram_map_t;

/* Region Properties */
#define SOC_SBX_CALADAN3_TMU_DEF_NUM_REGION (4*1024)
#define SOC_SBX_CALADAN3_TMU_DRAM_BANK_PER_REGION (4)

typedef struct soc_sbx_caladan3_region_attr_s {
    int number; /* region number */
    soc_sbx_caladan3_region_dram_map_t map[SOC_SBX_CALADAN3_TMU_DRAM_BANK_PER_REGION];    
} soc_sbx_caladan3_region_attr_t;

typedef struct soc_sbx_caladan3_region_config_s {
    int num_horizontal; /* number of horizontal regions */
    int num_vertical; /* number of vertical regions */
    int region_size_kbytes; /* row size in kilo bytes */
    int rows_per_region_per_bank;
    soc_sbx_caladan3_region_attr_t region_attr[SOC_SBX_CALADAN3_TMU_DEF_NUM_REGION];
} soc_sbx_caladan3_region_config_t;

#define SOC_SBX_CALADAN3_TMU_TABLE_1X_REPLICATION (1)
#define SOC_SBX_CALADAN3_TMU_TABLE_2X_REPLICATION (2)
#define SOC_SBX_CALADAN3_TMU_TABLE_4X_REPLICATION (4)
#define SOC_SBX_CALADAN3_TMU_TABLE_DEF_REPLICATION (SOC_SBX_CALADAN3_TMU_TABLE_4X_REPLICATION)

#define SOC_SBX_CALADAN3_TMU_MAX_TABLE (64)
#define SOC_SBX_CALADAN3_TMU_MAX_CHAIN_TABLE (32) /* 0 - 31 */
#define SOC_SBX_CALADAN3_TMU_MIN_TABLE_NUM_ENTRIES (4096) /* 4K entries */
#define SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_UNITS (64) /* entires has to be multiple of 64bits */
#define SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_MIN (64) /* min entry size 64 bits */
#define SOC_SBX_CALADAN3_TMU_TABLE_ENTRY_SIZE_MAX (8*1024) /* max entry size 8K bits */

#define SOC_SBX_TMU_TABLE_FLAG_NONE         (0)
#define SOC_SBX_TMU_TABLE_FLAG_WITH_ID      (1)
#define SOC_SBX_TMU_TABLE_FLAG_CHAIN        (2)

#define SOC_SBX_CALADAN3_TMU_MAX_BMP        (512)
#define SOC_SBX_CALADAN3_TMU_MAX_BMP_WORDS  (SOC_SBX_CALADAN3_TMU_MAX_BMP/32)

typedef struct soc_sbx_caladan3_table_attr_s {
    uint32 id;
    uint32 row_offset;
    uint32 region_offset;
    uint32 column_offset;
    uint32 num_row_per_region;
    uint32 num_entries_per_row;
    uint32 deadline;
    uint32 replication_factor;
    uint32 num_entries; /* must be power of 2 */
    uint32 entry_size_bits;
    uint32 flags;
    soc_sbx_caladan3_tmu_lookup_t lookup;
} soc_sbx_caladan3_table_attr_t;

typedef struct soc_sbx_caladan3_chain_table_attr_s {
    uint32 fifo_bitmap; /* valid bits 0,1,2,3  */
    uint32 length;      /* max length of chain */
    uint32 split_mode;  /* use split mode      */
    uint32 hw_managed;  /* h/w managed         */
} soc_sbx_caladan3_chain_table_attr_t;

typedef struct soc_sbx_caladan3_table_config_s {
    uint32 default_replication;
    soc_sbx_caladan3_table_attr_t table_attr[SOC_SBX_CALADAN3_TMU_MAX_TABLE];
    uint32 alloc_bmap[SOC_SBX_CALADAN3_TMU_MAX_BMP_WORDS];  /* table ID allocation bitmap, set means allocated */
    uint32 row_bmap[SOC_SBX_CALADAN3_TMU_MAX_BMP_WORDS];    /* region rows allocation bitmap, set means allocated */
    uint32 col_used_bits[SOC_SBX_CALADAN3_TMU_MAX_BMP];     /* number of used bits in a row, this has to align in units of 64 bits */
    /* col allocation bitmap for each row, each bit is 64 bits. */
    uint32 col_bmap[SOC_SBX_CALADAN3_TMU_MAX_BMP][SOC_SBX_CALADAN3_TMU_MAX_BMP_WORDS];
} soc_sbx_caladan3_table_config_t;

typedef struct _tmu_dma_mgr_dbase_s {
    VOL uint32 *dma_buffer;
    VOL uint32 *ring_pointer;
    VOL int dma_buffer_len;
    int valid;
    sal_thread_t thread_id;
    VOL int thread_running;   /* Input signal to thread */
    int thread_priority; /* info */
    int seq_num;
    int unit;
    int channel; /* associated cmic channel */
    int fifoid;
} tmu_dma_mgr_dbase_t;

typedef struct soc_sbx_caladan3_tmu_cmd_dma_s {
    tmu_dma_mgr_dbase_t cmdmgr[SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO];
} soc_sbx_caladan3_tmu_cmd_dma_t;

#define SOC_SBX_CALADAN3_TMU_DEF_RESP_TRAILER_THRESHOLD (1)

typedef struct soc_sbx_caladan3_tmu_resp_dma_s {
    tmu_dma_mgr_dbase_t respmgr[SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO];
} soc_sbx_caladan3_tmu_resp_dma_t;

/* TMU Program configuration interface                  *
 * User specifices/allocates programs & how to extract  *
 * keys from provided 512 key bits.                     */
#define SOC_SBX_CALADAN3_TMU_MAX_KEY (2)    
#define SOC_SBX_CALADAN3_TMU_MAX_PROGRAM (16)    

#define SOC_SBX_CALADAN3_TMU_MAX_KEY_BYTE_SHIFT (63)

typedef struct soc_sbx_caladan3_tmu_key_info_s {
    soc_sbx_caladan3_tmu_lookup_t lookup;
    uint32 shift[SOC_SBX_CALADAN3_TMU_MAX_KEY];
    uint32 bytes_to_mask[SOC_SBX_CALADAN3_TMU_MAX_KEY];
    int tableid; /* table id to use for first lookup */
    int taps_seg; /* taps segment id */
    int valid;
} soc_sbx_caladan3_tmu_key_info_t;

typedef struct soc_sbx_caladan3_tmu_program_info_s {
#define SOC_SBX_TMU_PRG_FLAG_WITH_ID (0x1)
    int flag;
    int program_num;
#define SOC_SBX_TMU_MAX_KEY_PLODER_SUB_BLOCKS (2)    
    soc_sbx_caladan3_tmu_key_info_t key_info[SOC_SBX_TMU_MAX_KEY_PLODER_SUB_BLOCKS];
    unsigned int key_shift[SOC_SBX_TMU_MAX_KEY_PLODER_SUB_BLOCKS]; /*  key shift */
} soc_sbx_caladan3_tmu_program_info_t;

typedef struct soc_sbx_caladan3_tmu_program_config_s {
    soc_sbx_caladan3_tmu_program_info_t *info;
    uint32 alloc_bmap[SOC_SBX_CALADAN3_TMU_MAX_BMP_WORDS]; /* program allocation bitmap, set means allocated */
} soc_sbx_caladan3_tmu_program_config_t;

typedef struct soc_sbx_caladan3_tmu_control_s {
    uint8 bypass_hash;
    uint8 bypass_scrambler;

#define SOC_SBX_TMU_EML_144BITS_MODE_OFF       (0)
#define SOC_SBX_TMU_EML_144BITS_MODE_ON        (1)
#define SOC_SBX_TMU_EML_144BITS_MODE_UNKNOWN   (2)
    uint8 eml_144_mode;
} soc_sbx_caladan3_tmu_control_t;

typedef struct soc_sbx_caladan3_tmu_dbase_s {
    int num_partition;
    soc_sbx_caladan3_dram_config_t dram_cfg;
    soc_sbx_caladan3_region_config_t region_cfg;
    soc_sbx_caladan3_table_config_t table_cfg;
    soc_sbx_caladan3_tmu_cmd_dma_t cmd_dma_cfg;
    soc_sbx_caladan3_tmu_resp_dma_t resp_dma_cfg;
    soc_sbx_caladan3_tmu_program_config_t program_cfg;
    soc_sbx_caladan3_tmu_control_t control_cfg;
#ifdef  TMU_USE_LOCKS 
#ifdef TMU_USE_SPINLOCK
  pthread_spinlock_t tmu_spin_lock;
#else
  sal_mutex_t tmu_mutex;
#endif  
#endif
#ifdef TMU_DM_USE_LOCKS
    sal_mutex_t dm_mutex;
#endif
#ifdef TMU_DMA_USE_LOCKS
#ifdef TMU_DMA_USE_SPINLOCK
   pthread_spinlock_t dma_spin_lock;
#else
  sal_mutex_t dma_mutex;  
#endif
#endif
} soc_sbx_caladan3_tmu_dbase_t;

#define TMU_MAX(a,b) ((a) > (b) ? (a) : (b))
#define TMU_MIN(a,b) ((a) < (b) ? (a) : (b))

#define SOC_SBX_CALADAN3_TMU_QE_INSTANCE_NUM (16)


extern soc_sbx_caladan3_tmu_dbase_t *_tmu_dbase[SOC_MAX_NUM_DEVICES];



#define SOC_IF_TMU_UNINIT_RETURN(unit) \
    do { if (_tmu_dbase[unit] == NULL) {return SOC_E_INIT;} } while(0)

#define TMU_CI_DDR_PATTERN_INIT (1) /* set this to 0 to disable DDR pattern init */

/* flags to post commands to tmu */
typedef enum _soc_sbx_tmu_cmd_post_flag_e_s {
    /* synchronous model where each command posted to dma buffer
     * and flushed right away to hardware */
    SOC_SBX_TMU_CMD_POST_FLAG_NONE = 0,
    /* tmu accumulates commands on command buffer until command buffer is full or
     * until flush is issued */
    SOC_SBX_TMU_CMD_POST_CACHE = 1,
    /* flushed out dma buffer to hardware */
    SOC_SBX_TMU_CMD_POST_FLUSH = 2,
    SOC_SBX_TMU_CMD_POST_MAX
} soc_sbx_tmu_cmd_post_flag_e_t;

#define _SOC_SBX_TMU_VALID_POST_FLAG(flag) ((flag) >= SOC_SBX_TMU_CMD_POST_FLAG_NONE && \
                                            (flag) <  SOC_SBX_TMU_CMD_POST_MAX)

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_region_map_dump
 * Purpose:
 *     Dump region mapping
 */
extern void soc_sbx_caladan3_tmu_region_map_dump(int unit, int min, int max);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_table_map_dump
 * Purpose:
 *     Dump TMU table allocation
 */
void soc_sbx_caladan3_tmu_table_map_dump(int unit, int min, int max);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_driver_init
 * Purpose:
 *     Bring up TMU drivers
 */
extern int soc_sbx_caladan3_tmu_driver_init(int unit);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_driver_destroy
 * Purpose:
 *     Destroy TMU drivers
 */
extern int soc_sbx_caladan3_tmu_driver_destroy(int unit);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_post_cmd
 * Purpose:
 *     Post command to DMA command ring
 */
extern int soc_sbx_caladan3_tmu_post_cmd(int unit, 
                                         int fifoid, 
                                         soc_sbx_caladan3_tmu_cmd_t *cmd,
                                         soc_sbx_tmu_cmd_post_flag_e_t flag); 

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_master_cache_cmd
 * Purpose:
 *     Cache dma commands for all master & slave, and enter master's queue
 */
extern int soc_sbx_caladan3_tmu_master_cache_cmd(int unit, int *slave_units, int num_slaves, int fifoid, 
                                  soc_sbx_caladan3_tmu_cmd_t *cmd);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_master_flush_cmd
 * Purpose:
 *     Post command to DMA command ring for master & slaves
 */
extern int soc_sbx_caladan3_tmu_master_flush_cmd(int unit, int *slave_units, int num_slaves,  int fifoid);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_slave_cache_cmd
 * Purpose:
 *     Get the seqnum of slave unit, and post commands into slave's queue
 */
extern int soc_sbx_caladan3_tmu_slave_cache_cmd(int unit, int fifoid, 
                                  soc_sbx_caladan3_tmu_cmd_t *cmd);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_get_resp
 * Purpose:
 *     Handle response from Response Manager
 */
extern int soc_sbx_caladan3_tmu_get_resp(int unit, int fifoid, 
                                         soc_sbx_caladan3_tmu_cmd_t *expected_cmd,
                                         void *data_buffer, int data_buffer_len_bytes);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_slave_get_resp
 * Purpose:
 *     Handle response from Response Manager
 */
extern int soc_sbx_caladan3_tmu_slave_get_resp(int unit, int master_unit, int fifoid, 
                                  soc_sbx_caladan3_tmu_cmd_t *expected_cmd,
                                  void *data_buffer, int data_buffer_len);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_get_table_state
 * Purpose:
 *     get table related state
 */
extern int soc_sbx_caladan3_tmu_get_table_state(int unit, 
                                                unsigned int tableid, 
                                                int *state);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_table_alloc
 * Purpose:
 *     Allocate a table
 */
extern int soc_sbx_caladan3_tmu_table_alloc(int unit, soc_sbx_caladan3_table_attr_t *attr);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_chain_table_alloc
 * Purpose:
 *     Allocate a table
 */
extern int soc_sbx_caladan3_tmu_chain_table_alloc(int unit,
                                                  soc_sbx_caladan3_table_attr_t *attr, 
                                                  soc_sbx_caladan3_chain_table_attr_t *chain_attr);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_table_free
 * Purpose:
 *     Allocate a table
 */
extern int soc_sbx_caladan3_tmu_table_free(int unit, int tableid);


/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_program_alloc
 * Purpose:
 *     Allocate a TMU program
 */
extern int soc_sbx_caladan3_tmu_program_alloc(int unit, soc_sbx_caladan3_tmu_program_info_t *info);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_program_free
 * Purpose:
 *     Free a TMU program
 */
extern int soc_sbx_caladan3_tmu_program_free(int unit, int program_num);

/*
 *
 * Function:
 *     _soc_sbx_caladan3_tmu_table_clear
 * Purpose:
 *     clear number of entries on given table
 */
extern int soc_sbx_caladan3_tmu_table_clear(int unit, int tableid, uint32 num_entries);

extern int soc_sbx_tmu_hw_fib_get(int unit, 
                                  soc_sbx_tmu_hash_handle_t handle,
                                  uint32 hw_hash_idx,
                                  uint32 *key,
                                  uint32 *value);

extern int soc_sbx_tmu_fib_get(int unit, 
                               soc_sbx_tmu_hash_handle_t handle,
                               uint32 sw_hash_idx, 
                               uint32 sw_bucket_idx,
                               uint32 *key,
                               uint32 *value);

#endif /* _SBX_CALADN3_TMU_H_ */
