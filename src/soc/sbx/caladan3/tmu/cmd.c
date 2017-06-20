/*
 * $Id: cmd.c,v 1.41.14.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    cmd.c
 * Purpose: Caladan3 on TMU command manager
 * Requires:
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>
#include <pthread.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/sbx_drv.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>
#include <soc/sbx/caladan3/tmu/cmd.h>
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/tmu/taps/taps.h>
#include <soc/sbx/caladan3/tmu/taps/sbucket.h>
#include <soc/sbx/caladan3/tmu/taps/dbucket.h>
#include <soc/sbx/caladan3/tmu/taps/taps_util.h>
#include <soc/sbx/caladan3/util.h>
#include <shared/util.h>
#include <sal/appl/sal.h>


#ifdef VXWORKS
#define inline static
#endif

#ifdef TMU_CMD_USE_LOCKS
#ifdef TMU_CMD_USE_SPINLOCK
#define TMU_CMD_LOCK_TAKE    pthread_spin_lock(&_g_tmu_cmd_dbase[unit]->cmd_spin_lock); 
#define TMU_CMD_LOCK_GIVE    pthread_spin_unlock(&_g_tmu_cmd_dbase[unit]->cmd_spin_lock);
#else
#define TMU_CMD_LOCK_TAKE    sal_mutex_take(_g_tmu_cmd_dbase[unit]->mutex, sal_mutex_FOREVER);
#define TMU_CMD_LOCK_GIVE    sal_mutex_give(_g_tmu_cmd_dbase[unit]->mutex);
#endif
#else
#define TMU_CMD_LOCK_TAKE    
#define TMU_CMD_LOCK_GIVE    
#endif


typedef struct soc_sbx_tmu_resv_adjust_s {
    int pos;
    int width;
} soc_sbx_tmu_resv_adjust_t;

typedef struct _soc_sbx_caladan3_tmu_cmd_mgr_s {
    soc_sbx_caladan3_tmu_cmd_t *cmd_pool;
    int pool_size;
    dq_t cmd_free_list; /* list of free commands */
    dq_t cmd_alloc_list; /* intermediate list where cmd stays before enqueue */
    dq_t cmd_resp_process_list; /* intermediate queue between dequeue to free stage */
    dq_t cmd_resp_list[SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO]; /* pending response queue per fifo */
#ifdef TMU_CMD_USE_LOCKS
#ifdef TMU_CMD_USE_SPINLOCK
  #include <pthread.h>
  pthread_spinlock_t cmd_spin_lock;
#else
    sal_mutex_t mutex;  
#endif
#endif
} soc_sbx_caladan3_tmu_cmd_mgr_t; 

static soc_sbx_caladan3_tmu_cmd_mgr_t *_g_tmu_cmd_dbase[SOC_MAX_NUM_DEVICES];

static char *tmu_cmd_str[SOC_SBX_TMU_CMD_MAX+1] = {
    "NO_OP", /* SOC_SBX_TMU_CMD_NOP=0 */
    "XL_READ", /* SOC_SBX_TMU_CMD_XL_READ=1 */
    "XL_WRITE", /* SOC_SBX_TMU_CMD_XL_WRITE=2 */
    "RAW_READ", /* SOC_SBX_TMU_CMD_RAW_READ=3 */
    "RAW_WRITE", /* SOC_SBX_TMU_CMD_RAW_WRITE=4 */
    "EML_INS_BEGIN", /* SOC_SBX_TMU_CMD_EML_INS_BEGIN=5 */
    "EML_INS_END", /* SOC_SBX_TMU_CMD_EML_INS_END=6 */
    "EML_DELETE", /* SOC_SBX_TMU_CMD_EML_DELETE=7 */
    "TAPS", /* SOC_SBX_TMU_CMD_TAPS=8 */
    "LOCK", /* SOC_SBX_TMU_CMD_LOCK=9 */
    "RELEASE", /* SOC_SBX_TMU_CMD_RELEASE=10 */
    "INVALID", "INVALID", "INVALID", "INVALID", /* 11,12,13,14 */
    "TRAILER" /* SOC_SBX_TMU_CMD_TRAILER=15 */
};


/* Statistics for commands */
static void tmu_cmd_count(int unit, soc_sbx_caladan3_tmu_cmd_t *cmd);
static uint32 _tmu_cmd_counts[SOC_SBX_TMU_CMD_MAX];
static uint32   _tmu_taps_cmd_counts[SOC_SBX_TMU_TAPS_BLOCK_MAX][SOC_SBX_TMU_TAPS_SUBCMD_MAX];


static char *tmu_taps_blk_str[SOC_SBX_TMU_TAPS_BLOCK_MAX] = {
  "SOC_SBX_TMU_TAPS_MIN",
  "SOC_SBX_TMU_TAPS_RPB", 
  "SOC_SBX_TMU_TAPS_BB", 
  "SOC_SBX_TMU_TAPS_BRR"
};

static char *tmu_taps_rpb_str[SOC_SBX_TMU_TAPS_RPB_SUBCMD_MAX] = {
  "NO_OP",
  "READ", 
  "WRITE",
  "SKIP",
  "PROPAGATE",
  "REPLACE",
  "FIND_BCKT",
  "SKIP",
  "RAW_READ",
  "RAW_WRITE"
};

static char *tmu_taps_bb_str[SOC_SBX_TMU_TAPS_BB_SUBCMD_MAX] = {
  "NOP",
  "READ", 
  "WRITE", 
  "RAW_READ",
  "RAW_WRITE"
};

static char *tmu_taps_brr_str[SOC_SBX_TMU_TAPS_BRR_SUBCMD_MAX] = {
  "NOP",
  "READ", 
  "WRITE", 
  "RD_WIDE",
  "WRT_WIDE"
};





#define SOC_SBX_CALADAN3_DEF_TMU_CMD_POOL_SIZE (10*1024)    

extern int _g_tmu_key_size_bits[];

/*
 * Function
 *   tmu_cmd_mgr_uninit
 * Purpose
 *   cleanup 
 */
int tmu_cmd_mgr_uninit(int unit)
{
#ifdef TMU_CMD_USE_LOCKS
#ifdef TMU_CMD_USE_SPINLOCK
  /* create the CMD spin lock */
  if (SOC_FAILURE(pthread_spin_destroy(&_g_tmu_cmd_dbase[unit]->cmd_spin_lock)))
    {
      LOG_ERROR(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s: unit %d Failed to create cmd spinlock\n"),
                 FUNCTION_NAME(), unit));
      return SOC_E_RESOURCE;
    }
#else
  if (_g_tmu_cmd_dbase[unit]->mutex) {
    sal_mutex_destroy(_g_tmu_cmd_dbase[unit]->mutex);
  }
#endif
#endif
    if (_g_tmu_cmd_dbase[unit]->cmd_pool) 
        sal_free(_g_tmu_cmd_dbase[unit]->cmd_pool);

    if (_g_tmu_cmd_dbase[unit]) {
        sal_free(_g_tmu_cmd_dbase[unit]);
        _g_tmu_cmd_dbase[unit] = NULL;
    }

    return SOC_E_NONE;
}

/* Allocate dynamic memory command pool so users do not have to
 * allocate them on fly */
int tmu_cmd_mgr_init(int unit)
{
    int index;
 
    if (_g_tmu_cmd_dbase[unit] != NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "uint[%d] - TMU command buffer pool already initialized"),unit));
        return SOC_E_INIT;
    }

    _g_tmu_cmd_dbase[unit] = sal_alloc(sizeof(soc_sbx_caladan3_tmu_cmd_mgr_t), "tmu-cmd-dbase");
    if (_g_tmu_cmd_dbase[unit] == NULL) {
        return SOC_E_MEMORY;
    } else {

#ifdef TMU_CMD_USE_LOCKS
#ifdef TMU_CMD_USE_SPINLOCK
      /* create the CMD spin lock */
      if (SOC_FAILURE(pthread_spin_init(&_g_tmu_cmd_dbase[unit]->cmd_spin_lock,PTHREAD_PROCESS_PRIVATE)))
        {
          LOG_ERROR(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s: unit %d Failed to create cmd spinlock\n"),
                     FUNCTION_NAME(), unit));
          return SOC_E_RESOURCE;
        }
#else
      _g_tmu_cmd_dbase[unit]->mutex = sal_mutex_create("COMMAND_BUF_MUTEX");
      
      if (_g_tmu_cmd_dbase[unit]->mutex == NULL) {
        sal_free(_g_tmu_cmd_dbase[unit]);
        sal_mutex_destroy(_g_tmu_cmd_dbase[unit]->mutex);
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "uint[%d] - TMU command pool failed to create mutex\n"),unit));
        return SOC_E_RESOURCE;
      }
#endif
#endif

        _g_tmu_cmd_dbase[unit]->cmd_pool = sal_alloc(sizeof(soc_sbx_caladan3_tmu_cmd_t) * 
                                                    SOC_SBX_CALADAN3_DEF_TMU_CMD_POOL_SIZE, 
                                                    "tmu-cmd-pool");
        if (_g_tmu_cmd_dbase[unit]->cmd_pool == NULL) {
            sal_free(_g_tmu_cmd_dbase[unit]);
            return SOC_E_MEMORY;
        }

        sal_memset(_g_tmu_cmd_dbase[unit]->cmd_pool, 0, 
                   sizeof(soc_sbx_caladan3_tmu_cmd_t) * 
                   SOC_SBX_CALADAN3_DEF_TMU_CMD_POOL_SIZE);

        DQ_INIT(&_g_tmu_cmd_dbase[unit]->cmd_free_list);
        DQ_INIT(&_g_tmu_cmd_dbase[unit]->cmd_alloc_list);
        DQ_INIT(&_g_tmu_cmd_dbase[unit]->cmd_resp_process_list);

        for (index=0; index < SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO; index++) {
            DQ_INIT(&_g_tmu_cmd_dbase[unit]->cmd_resp_list[index]);
        }
        
        /* put all messages into free list */
        for (index=0; index < SOC_SBX_CALADAN3_DEF_TMU_CMD_POOL_SIZE; index++) {
            DQ_INSERT_HEAD(&_g_tmu_cmd_dbase[unit]->cmd_free_list, 
                           &_g_tmu_cmd_dbase[unit]->cmd_pool[index].list_elem);
        }
    }

    return SOC_E_NONE;
}


/* initialize interface structure to be empty */
int tmu_cmd_init(soc_sbx_caladan3_tmu_cmd_t *cmd)
{
    if (cmd == NULL) {
        return SOC_E_PARAM;
    }

    cmd->opcode = SOC_SBX_TMU_CMD_MAX;
    cmd->response_type = SOC_SBX_TMU_RESP_MAX;
    cmd->post_dpfx_index = POST_PFX_CHECK_DISABLE;
    return SOC_E_NONE;
}

int tmu_cmd_destory(soc_sbx_caladan3_tmu_cmd_t *cmd)
{
    if (cmd == NULL) {
        return SOC_E_PARAM;
    }
    cmd->opcode = SOC_SBX_TMU_CMD_MAX;
    cmd->response_type = SOC_SBX_TMU_RESP_MAX;
    cmd->post_dpfx_index = POST_PFX_CHECK_DISABLE;
    cmd->seqnum = 0;
    return SOC_E_NONE;
}

int tmu_cmd_alloc(int unit, soc_sbx_caladan3_tmu_cmd_t **cmd)
{
    if (cmd == NULL) {
        return SOC_E_PARAM;
    }

    if (_g_tmu_cmd_dbase[unit] == NULL) {
        return SOC_E_INIT;
    }


    TMU_CMD_LOCK_TAKE

    if (DQ_EMPTY(&_g_tmu_cmd_dbase[unit]->cmd_free_list)) {
      TMU_CMD_LOCK_GIVE
      return SOC_E_MEMORY;
    }

    *cmd = DQ_ELEMENT_GET(soc_sbx_caladan3_tmu_cmd_t*,
                         DQ_HEAD(&_g_tmu_cmd_dbase[unit]->cmd_free_list, dq_p_t),
                         list_elem);

    DQ_REMOVE(&((*cmd)->list_elem));

    DQ_INSERT_TAIL(&_g_tmu_cmd_dbase[unit]->cmd_alloc_list, &((*cmd)->list_elem));
    tmu_cmd_init(*cmd);
    
    TMU_CMD_LOCK_GIVE
      return SOC_E_NONE;
}

int tmu_cmd_free(int unit, soc_sbx_caladan3_tmu_cmd_t *cmd)
{
    if (cmd == NULL) {
        return SOC_E_PARAM;
    }

    if (_g_tmu_cmd_dbase[unit] == NULL) {
        return SOC_E_INIT;
    }

    TMU_CMD_LOCK_TAKE
    
      DQ_REMOVE(&cmd->list_elem);

    DQ_INSERT_TAIL(&_g_tmu_cmd_dbase[unit]->cmd_free_list, &cmd->list_elem);

    tmu_cmd_count(unit,cmd);

    tmu_cmd_destory(cmd);

    TMU_CMD_LOCK_GIVE

    return SOC_E_NONE;
}

int tmu_cmd_enqueue(int unit, soc_sbx_caladan3_tmu_cmd_t *cmd, int fifoid, uint8 head)
{
    if (cmd == NULL) {
        return SOC_E_PARAM;
    }

    if (fifoid < 0 || fifoid >= SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO) {
        return SOC_E_PARAM;
    }

    if (_g_tmu_cmd_dbase[unit] == NULL) {
        return SOC_E_INIT;
    }

    TMU_CMD_LOCK_TAKE
    
    DQ_REMOVE(&cmd->list_elem);

    if (head) {
        DQ_INSERT_HEAD(&_g_tmu_cmd_dbase[unit]->cmd_resp_list[fifoid], &cmd->list_elem);
    } else {
        DQ_INSERT_TAIL(&_g_tmu_cmd_dbase[unit]->cmd_resp_list[fifoid], &cmd->list_elem);
    }


    TMU_CMD_LOCK_GIVE

    return SOC_E_NONE;
}

int tmu_cmd_dequeue(int unit, soc_sbx_caladan3_tmu_cmd_t **cmd, int fifoid)
{
    if (cmd == NULL) {
        return SOC_E_PARAM;
    }

    if (fifoid < 0 || fifoid >= SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO) {
        return SOC_E_PARAM;
    }

    if (_g_tmu_cmd_dbase[unit] == NULL) {
        return SOC_E_INIT;
    }

    TMU_CMD_LOCK_TAKE
    
    *cmd = DQ_ELEMENT_GET(soc_sbx_caladan3_tmu_cmd_t*,
                         DQ_HEAD(&_g_tmu_cmd_dbase[unit]->cmd_resp_list[fifoid], dq_p_t),
                         list_elem);

    DQ_REMOVE(&(*cmd)->list_elem);

    DQ_INSERT_TAIL(&_g_tmu_cmd_dbase[unit]->cmd_resp_process_list, &(*cmd)->list_elem);

    TMU_CMD_LOCK_GIVE

    return SOC_E_NONE;
}

#define BITS2WORDIDX(pos) (BITS2WORDS(pos+1)-1)

/* assumed to be on ascending order */
/* reserved area within words are assumed consecutive for now */
static soc_sbx_tmu_resv_adjust_t eml_resv_info[][3] = {
    {{0,0}, {0,0}, {0,0}}, /* SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS - NONE */
    {{176,16}, {0, 0}, {0,0}}, /* SOC_SBX_CALADAN3_TMU_HASH_KEY_176_BITS */
    {{183,9}, {313,7}, {0,0}}, /* SOC_SBX_CALADAN3_TMU_HASH_KEY_304_BITS */
    {{183,9}, {433,15}, {0,0}} /* SOC_SBX_CALADAN3_TMU_HASH_KEY_424_BITS */
};

#define _EML_VALUE_ECC_BITS (9)

#define CMD_NOP_OPCODEf_SHFT              28
#define CMD_NOP_OPCODEf_MSK              0xF
#define CMD_NOP_SEQNUMf_SHFT              22
#define CMD_NOP_SEQNUMf_MSK             0x3F
#define CMD_NOP_ECHOf_SHFT                21
#define CMD_NOP_ECHOf_MSK                0x1

#define CMD_XLWRITE_ENTRYNUMf_SHFT         0
#define CMD_XLWRITE_ENTRYNUMf_MSK  0x7FFFFFF
#define CMD_XLWRITE_TABLEf_SHFT            0
#define CMD_XLWRITE_TABLEf_MSK          0x3f
#define CMD_XLWRITE_LOOKUPf_SHFT           6
#define CMD_XLWRITE_LOOKUPf_MSK           0x3f
#define CMD_XLWRITE_OFFSETf_SHFT          12
#define CMD_XLWRITE_OFFSETf_MSK         0x7f
#define CMD_XLWRITE_SIZEf_SHFT            27
#define CMD_XLWRITE_SIZEf_MSK           0x1f
#define CMD_XLWRITE_RSV1f_SHFT            19
#define CMD_XLWRITE_RSV1f_MSK            0x7


#define CMD_XLREAD_ENTRYNUMf_SHFT         0
#define CMD_XLREAD_ENTRYNUMf_MSK  0x7FFFFFF
#define CMD_XLREAD_TABLEf_SHFT            0
#define CMD_XLREAD_TABLEf_MSK          0x3f
#define CMD_XLREAD_LOOKUPf_SHFT           6
#define CMD_XLREAD_LOOKUPf_MSK         0x3f
#define CMD_XLREAD_KVPRf_SHFT            28
#define CMD_XLREAD_KVPRf_MSK            0xf
#define CMD_XLREAD_RSV1f_SHFT            28
#define CMD_XLREAD_RSV1f_MSK            0xf
#define CMD_XLREAD_RSV2f_SHFT            28
#define CMD_XLREAD_RSV2f_MSK            0xf


#define CMD_TAPS_RPB_NOP_TAPSIf_SHFT      30
#define CMD_TAPS_RPB_NOP_TAPSBf_SHFT      28
#define CMD_TAPS_RPB_NOP_TAPSOPf_SHFT     24

#define CMD_TAPS_SUBCMD_BB_W_OFFf_SHFT     0  
#define CMD_TAPS_SUBCMD_BB_W_SEGf_SHFT    16 
#define CMD_TAPS_SUBCMD_BB_W_FORf_SHFT    20 
#define CMD_TAPS_SUBCMD_BB_W_PREf_SHFT     0
#define CMD_TAPS_SUBCMD_BB_W_ADf_SHFT      0
#define CMD_TAPS_SUBCMD_BB_W_KSHFf_SHFT   16  
#define CMD_TAPS_SUBCMD_BB_W_LENf_SHFT    16  
#define CMD_TAPS_SUBCMD_BB_W_ALRf_SHFT    28 


#define CMD_UPD_TAPS_RESPm_ERR0f_SHFT       0
#define CMD_UPD_TAPS_RESPm_ERR0f_MSK      0x1
#define CMD_UPD_TAPS_RESPm_ERR1f_SHFT       1
#define CMD_UPD_TAPS_RESPm_ERR1f_MSK      0x1
#define CMD_UPD_TAPS_RESPm_VALID0f_SHFT     0
#define CMD_UPD_TAPS_RESPm_VALID0f_MSK    0x1
#define CMD_UPD_TAPS_RESPm_VALID1f_SHFT     1
#define CMD_UPD_TAPS_RESPm_VALID1f_MSK    0x1
#define CMD_UPD_TAPS_RESPm_SIZEf_SHFT       0
#define CMD_UPD_TAPS_RESPm_SIZEf_MSK     0xFF
#define CMD_UPD_TAPS_RESPm_ERRCODEf_SHFT    0
#define CMD_UPD_TAPS_RESPm_ERRCODEf_MSK  0xFF
#define CMD_EMLm_TABLEIDf_SHFT              0
#define CMD_EMLm_TABLEIDf_MSK            0x3f


/*
 *
 * Function:
 *     tmu_cmd_seqnum_set_in_buffer
 * Purpose:
 *     Modify the seqnum field in buffer
 */
void tmu_cmd_seqnum_set_in_buffer(int seq_num, uint32 *buffer)
{
    buffer[1] = (buffer[1] >> CMD_NOP_OPCODEf_SHFT << CMD_NOP_OPCODEf_SHFT) 
           | (seq_num << CMD_NOP_SEQNUMf_SHFT)
           | (buffer[1] & MASK(CMD_NOP_SEQNUMf_SHFT));
}

inline int _tmu_cmd_eml_kv_pack(int unit,
                                soc_sbx_caladan3_tmu_cmd_t *cmd, 
                                uint32 *buffer)
{
    int index, resvidx=0, offset, bitpos=0, tmp;
    soc_sbx_tmu_resv_adjust_t *adjust = NULL;

    switch(cmd->opcode) {

    case SOC_SBX_TMU_CMD_EML_INS_BEGIN:
    case SOC_SBX_TMU_CMD_EML_INS_END:
    case SOC_SBX_TMU_CMD_EML_DELETE:

        switch (cmd->cmd.eml.key_type) {

        case SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS:
        case SOC_SBX_CALADAN3_TMU_HASH_KEY_176_BITS:
        case SOC_SBX_CALADAN3_TMU_HASH_KEY_304_BITS:
        case SOC_SBX_CALADAN3_TMU_HASH_KEY_424_BITS:

            for (index=0, offset=0; index < BITS2WORDS(_g_tmu_key_size_bits[cmd->cmd.eml.key_type]); index++) {

                if (eml_resv_info[cmd->cmd.eml.key_type][resvidx].width > 0) {
                    adjust = &eml_resv_info[cmd->cmd.eml.key_type][resvidx];
                    if (BITS2WORDIDX(adjust->pos) == index) {
                        resvidx++;
                    }
                } else {
                    resvidx++;
                }

                /* pack from key on command into buffer */
                if (adjust && (BITS2WORDIDX(adjust->pos) == index)) {
                    /* offset means number of valid bits in current packed word */
                    offset = ((adjust->pos)%32);

                    /* we need to get "offset" number of bits from original key starting on bitpos */

                    /* get (32 - bitpos%32) bits from word [bitpos/32] */
                    buffer[index] = cmd->cmd.eml.key[bitpos/32];
                    buffer[index] >>= (bitpos % 32);

                    /* get offset - (32 - bitpos%32) bits from next word */
                    if (offset > (32 - (bitpos%32))) {
                        buffer[index] |= ((cmd->cmd.eml.key[bitpos/32+1] & GEN_MASK(offset - (32 - (bitpos%32)))) << (32 - (bitpos%32)));
                    } else {
                        if ((bitpos%32) != 0) {
                            /* this should not happen on C3 */
                            assert(0);
                        }
                    }

                    /* remaining higher msb bits are assumed to be 0 */
                    buffer[index] &= GEN_MASK(offset);

                    bitpos += offset;
                } else {
                    offset = bitpos % 32;
                    buffer[index] = cmd->cmd.eml.key[bitpos/32];
                    buffer[index] >>= offset;

                    /* bit position not word alighned */
                    if (offset) {
                        tmp = cmd->cmd.eml.key[bitpos/32+1] &  GEN_MASK(offset);
                        tmp <<= (32 - offset);
                        buffer[index] |= tmp;
                    }
                    bitpos += 32;
                }
            }

#if 0
            LOG_CLI((BSL_META_U(unit,
                                "Key input: \n0x")));
            for (index=0; index < BITS2WORDS(_g_tmu_key_size_bits[cmd->cmd.eml.key_type]); index++) {
                LOG_CLI((BSL_META_U(unit,
                                    "%08x"), cmd->cmd.eml.key[index]));
            }

            LOG_CLI((BSL_META_U(unit,
                                "\nKey packed: \n0x")));
            for (index=0; index < BITS2WORDS(_g_tmu_key_size_bits[cmd->cmd.eml.key_type]); index++) {
                LOG_CLI((BSL_META_U(unit,
                                    "%08x"), buffer[index]));
            }
#endif

            /* packet values */
            for (offset=0; cmd->opcode != SOC_SBX_TMU_CMD_EML_DELETE &&
                     offset < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++,offset++) {
                buffer[index] = cmd->cmd.eml.value[offset];
            }
            break;

        default:
            return SOC_E_PARAM;
        }
        break;
    default:
        return SOC_E_PARAM;
    }

    return 0;
}

/* Buffer pointer starts at KEY, command word should be skipped for root. 
 * This funciton is shared between Root & chain unpacking. The buffer points
 * to key & single key,value are unpacked at a given time */
int tmu_cmd_eml_kv_unpack(int unit,
                          uint32 *buffer,
                          uint32 *key, 
                          uint32* value,
                          soc_sbx_tmu_hash_key_type_e_t key_type)
{
    if (!buffer || !key || !value) {
        return SOC_E_PARAM;
    }

    switch (key_type) {

    case SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS:
        sal_memcpy(key, buffer, BITS2BYTES(_g_tmu_key_size_bits[key_type]));
        break;

    case SOC_SBX_CALADAN3_TMU_HASH_KEY_176_BITS:
        sal_memcpy(key, buffer, BITS2BYTES(_g_tmu_key_size_bits[key_type]));
        key[5] &= ((1 << 16) - 1);
        break;

    case SOC_SBX_CALADAN3_TMU_HASH_KEY_304_BITS:
    case SOC_SBX_CALADAN3_TMU_HASH_KEY_424_BITS:
        assert(0);
        break;

    default:
        return SOC_E_PARAM;
    }

    sal_memcpy(value, &buffer[BITS2WORDS(_g_tmu_key_size_bits[key_type])], 
               BITS2BYTES(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS));
    value[3] &= ((1 << (32-_EML_VALUE_ECC_BITS)) - 1);

    return SOC_E_NONE;
}

/*
 * pack key into TMU taps command buffer.
 */
int _tmu_cmd_taps_key_pack(int unit,
                           soc_sbx_caladan3_tmu_cmd_t *cmd, 
                           uint32 *buffer)
{
    int status = SOC_E_NONE;
    uint32 key[BITS2WORDS(TAPS_IPV6_KEY_SIZE)];
    uint32 length = 0, max_key_size;

    if (cmd->opcode != SOC_SBX_TMU_CMD_TAPS) return SOC_E_PARAM;

    if ((cmd->cmd.taps.max_key_size != TAPS_IPV6_KEY_SIZE) &&
        (cmd->cmd.taps.max_key_size != TAPS_IPV4_KEY_SIZE)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s - unsupported max key length %d on unit %d"), 
                   FUNCTION_NAME(), cmd->cmd.taps.max_key_size, unit));
        return SOC_E_PARAM;     
    } else {
        max_key_size = cmd->cmd.taps.max_key_size;
    }

    if ((cmd->cmd.taps.blk == SOC_SBX_TMU_TAPS_BLOCK_RPB) &&
        ((cmd->cmd.taps.opcode == SOC_SBX_TMU_TAPS_RPB_SUBCMD_WRITE) ||
         (cmd->cmd.taps.opcode == SOC_SBX_TMU_TAPS_RPB_SUBCMD_PROPAGATE) ||
         (cmd->cmd.taps.opcode == SOC_SBX_TMU_TAPS_RPB_SUBCMD_REPLACE))) {
        if (cmd->cmd.taps.opcode == SOC_SBX_TMU_TAPS_RPB_SUBCMD_WRITE) {
            sal_memcpy(key, cmd->cmd.taps.subcmd.rpb_write.key, BITS2WORDS(max_key_size) * sizeof(uint32));
            length = cmd->cmd.taps.subcmd.rpb_write.length;
        } else if (cmd->cmd.taps.opcode == SOC_SBX_TMU_TAPS_RPB_SUBCMD_PROPAGATE) {
            sal_memcpy(key, cmd->cmd.taps.subcmd.rpb_propagate.key, BITS2WORDS(max_key_size) * sizeof(uint32));
            length = cmd->cmd.taps.subcmd.rpb_propagate.length;
        } else {
            sal_memcpy(key, cmd->cmd.taps.subcmd.rpb_replace.key, BITS2WORDS(max_key_size) * sizeof(uint32));
            length = cmd->cmd.taps.subcmd.rpb_replace.length;
        }
    } else if ((cmd->cmd.taps.blk == SOC_SBX_TMU_TAPS_BLOCK_BB) &&
               (cmd->cmd.taps.opcode == SOC_SBX_TMU_TAPS_BB_SUBCMD_WRITE)) {
        sal_memcpy(key, cmd->cmd.taps.subcmd.bb_write.key, BITS2WORDS(max_key_size) * sizeof(uint32));
        length = cmd->cmd.taps.subcmd.bb_write.length;
    } else {
        return SOC_E_UNAVAIL;
    }


    /* copy key to cmd buffer, NOTE: that this might overwrite
     * other fields in the cmd buffer
     */
    if (length != _TAPS_INVALIDATE_PFX_LEN_) {
        /* software key format only maitain MSBs of the key, shift MSBs left to
         * bit position required by hardware
         */
        status = taps_key_shift(max_key_size, key/*buffer*/, length, (length-max_key_size));
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s - key shift failed on unit %d"), 
                       FUNCTION_NAME(), unit));
            return status;
        }

        soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                          PDATA143_128f, &key[0]);
        soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                          PDATA127_96f, &key[1]);

        if (max_key_size == TAPS_IPV6_KEY_SIZE) {
            soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                              PDATA95_64f, &key[2]);
            soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                              PDATA63_32f, &key[3]);
            soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                              PDATA31_0f, &key[4]);
        }
    }

    return status;
}

/*
 * unpack key from TMU taps response buffer
 */
int _tmu_cmd_taps_key_unpack(int unit,
                             soc_sbx_caladan3_tmu_cmd_t *cmd, 
                             uint32 *buffer)
{
    int status = SOC_E_UNAVAIL;

    return status;
}

int tmu_cmd_writeBuf(int unit,
                     soc_sbx_caladan3_tmu_cmd_t *cmd, 
                     uint32 *buffer,
                     unsigned int length)
{
    uint32 index;
    int status = SOC_E_NONE;
    uint32 buff_cnt;
    uint32 buff_offset;

    /*validate interface structure parameters*/
    if (cmd == NULL || buffer == NULL || length == 0) {
        return SOC_E_PARAM;
    }

    for (index=0; index < BITS2WORDS(SOC_SBX_TMU_CMD_WORD_SIZE); index++) {
      buffer[index] = 0;
    }

    buffer[1] |= (cmd->opcode << CMD_NOP_OPCODEf_SHFT ) |
      (cmd->seqnum << CMD_NOP_SEQNUMf_SHFT );

    switch(cmd->opcode) {
    case SOC_SBX_TMU_CMD_NOP:
        {
            soc_mem_field_set(unit, TMB_UPDATER_CMD_NOPm,
                              buffer, ECHOf, &cmd->cmd.nop.echo);
        }
        break;
    case SOC_SBX_TMU_CMD_XL_READ:
        {
          
          buffer[0] |=  (cmd->cmd.xlread.entry_num << CMD_XLREAD_ENTRYNUMf_SHFT) |
            (cmd->cmd.xlread.kv_pairs << CMD_XLREAD_KVPRf_SHFT  );

          buffer[1]  |=  cmd->cmd.xlread.table   << CMD_XLREAD_TABLEf_SHFT |
            (cmd->cmd.xlread.lookup << CMD_XLREAD_LOOKUPf_SHFT  );      ;

	}
        break;
    case SOC_SBX_TMU_CMD_XL_WRITE:     
        {

            buffer[1] |= cmd->cmd.xlwrite.table |
              (cmd->cmd.xlwrite.lookup  << CMD_XLWRITE_LOOKUPf_SHFT) |
              (cmd->cmd.xlwrite.offset  << CMD_XLWRITE_OFFSETf_SHFT );
            buffer[0] = (cmd->cmd.xlwrite.entry_num  << CMD_XLWRITE_ENTRYNUMf_SHFT) |
              (cmd->cmd.xlwrite.size  <<  CMD_XLWRITE_SIZEf_SHFT);                     

	    /* write entries */
	    buff_cnt = BITS2WORDS(cmd->cmd.xlwrite.value_size);
	    buff_offset = BITS2WORDS(SOC_SBX_TMU_CMD_WORD_SIZE);
	    
	    for (index=0; index < buff_cnt; index++) {
		buffer[index + buff_offset] = cmd->cmd.xlwrite.value_data[index];
	    }
        }
        break;
    case SOC_SBX_TMU_CMD_RAW_READ:
        break;
    case SOC_SBX_TMU_CMD_RAW_WRITE:
        break;
    case SOC_SBX_TMU_CMD_EML_INS_BEGIN:
    case SOC_SBX_TMU_CMD_EML_INS_END:
    case SOC_SBX_TMU_CMD_EML_DELETE:
        {
            soc_mem_field_set(unit, TMB_UPDATER_CMD_EML_INS_BEGIN_ENDm,
                              buffer, FILTERf, &cmd->cmd.eml.filter);
            soc_mem_field_set(unit, TMB_UPDATER_CMD_EML_INS_BEGIN_ENDm,
                              buffer, SIZEf, &cmd->cmd.eml.key_type);
            soc_mem_field_set(unit, TMB_UPDATER_CMD_EML_INS_BEGIN_ENDm,
                              buffer, TABLEf, &cmd->cmd.eml.table_id);
            status = _tmu_cmd_eml_kv_pack(unit, cmd,
                                          buffer + BITS2WORDS(SOC_SBX_TMU_CMD_WORD_SIZE));
        }
        break;
    case SOC_SBX_TMU_CMD_TAPS:
        /* offset buffer */
        buffer += BITS2WORDS(SOC_SBX_TMU_CMD_WORD_SIZE);

        /* clear buffer */
        for (index=0; index < BITS2WORDS(SOC_SBX_TMU_TAPS_CMD_SIZE); index++) {
            buffer[index] = 0;
        }

        /* set the taps fields common for all blocks, sanity check here? */     
        buffer[7] |= (cmd->cmd.taps.opcode << CMD_TAPS_RPB_NOP_TAPSOPf_SHFT)  |
                      (cmd->cmd.taps.blk << CMD_TAPS_RPB_NOP_TAPSBf_SHFT) |
                      (cmd->cmd.taps.instance << CMD_TAPS_RPB_NOP_TAPSIf_SHFT);

        switch (cmd->cmd.taps.blk) {        
            case SOC_SBX_TMU_TAPS_BLOCK_RPB:
                /* Taps RPB block */
                switch (cmd->cmd.taps.opcode) {
                    case SOC_SBX_TMU_TAPS_RPB_SUBCMD_NOP:
                    case SOC_SBX_TMU_TAPS_RPB_SUBCMD_FIND_BUCKET:
                    case SOC_SBX_TMU_TAPS_RPB_SUBCMD_RAW_READ:
                    case SOC_SBX_TMU_TAPS_RPB_SUBCMD_RAW_WRITE:
                        
                        break;
                    case SOC_SBX_TMU_TAPS_RPB_SUBCMD_WRITE:
                        /* pack key need to be done first since it might 
                         * modify the other fields in the buffer
                         */
                        status = _tmu_cmd_taps_key_pack(unit, cmd, buffer);

                        soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                                          SEGf, &cmd->cmd.taps.subcmd.rpb_write.segment);
                        soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                                          OFFSETf, &cmd->cmd.taps.subcmd.rpb_write.offset);
                        soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                                          TARGETf, &cmd->cmd.taps.subcmd.rpb_write.target);
                        soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                                          BPM_LENGTHf, &cmd->cmd.taps.subcmd.rpb_write.bpm_length);
                        soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                                          KSHIFTf, &cmd->cmd.taps.subcmd.rpb_write.kshift);
                        soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                                          BUCKETf, &cmd->cmd.taps.subcmd.rpb_write.bucket);
                        soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                                          BEST_MATCHf, &cmd->cmd.taps.subcmd.rpb_write.best_match);
                        soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                                          ALIGN_RIGHTf, &cmd->cmd.taps.subcmd.rpb_write.align_right);
                        soc_mem_field32_set(unit, TAPS_RPB_CMD_WRITEm, buffer, Gf, 0);
                        soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                                          PLENGTHf, &cmd->cmd.taps.subcmd.rpb_write.length);
                        break;
                    case SOC_SBX_TMU_TAPS_RPB_SUBCMD_READ:
                        soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                                          SEGf, &cmd->cmd.taps.subcmd.rpb_read.segment);
                        soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                                          OFFSETf, &cmd->cmd.taps.subcmd.rpb_read.offset);
                        soc_mem_field_set(unit, TAPS_RPB_CMD_WRITEm, buffer,
                                          TARGETf, &cmd->cmd.taps.subcmd.rpb_read.target);
                        break;
                    case SOC_SBX_TMU_TAPS_RPB_SUBCMD_PROPAGATE:
                        /* pack key need to be done first since it might 
                         * modify the other fields in the buffer
                         */
                        status = _tmu_cmd_taps_key_pack(unit, cmd, buffer);

                        soc_mem_field_set(unit, TAPS_RPB_CMD_PROPAGATEm, buffer,
                                          SEGf, &cmd->cmd.taps.subcmd.rpb_propagate.segment);
                        soc_mem_field_set(unit, TAPS_RPB_CMD_PROPAGATEm, buffer,
                                          BEST_MATCHf, &cmd->cmd.taps.subcmd.rpb_propagate.best_match);
                        soc_mem_field_set(unit, TAPS_RPB_CMD_PROPAGATEm, buffer,
                                          PLENGTHf, &cmd->cmd.taps.subcmd.rpb_propagate.length);
                        break;
                    case SOC_SBX_TMU_TAPS_RPB_SUBCMD_REPLACE:
                        /* pack key need to be done first since it might 
                         * modify the other fields in the buffer
                         */
                        status = _tmu_cmd_taps_key_pack(unit, cmd, buffer);

                        soc_mem_field_set(unit, TAPS_RPB_CMD_REPLACEm, buffer,
                                          SEGf, &cmd->cmd.taps.subcmd.rpb_replace.segment);
                        soc_mem_field_set(unit, TAPS_RPB_CMD_REPLACEm, buffer,
                                          BPM_LENGTHf, &cmd->cmd.taps.subcmd.rpb_replace.bpm_length);
                        soc_mem_field_set(unit, TAPS_RPB_CMD_REPLACEm, buffer,
                                          BEST_MATCHf, &cmd->cmd.taps.subcmd.rpb_replace.best_match);
                        soc_mem_field_set(unit, TAPS_RPB_CMD_REPLACEm, buffer,
                                          PLENGTHf, &cmd->cmd.taps.subcmd.rpb_replace.length);
                        break;
                    default:
                        return SOC_E_PARAM;
                }
                break;
            case SOC_SBX_TMU_TAPS_BLOCK_BB:
                /* Taps BB block */
                switch (cmd->cmd.taps.opcode) {
                    case SOC_SBX_TMU_TAPS_BB_SUBCMD_NOP:
                    case SOC_SBX_TMU_TAPS_BB_SUBCMD_RAW_READ:
                    case SOC_SBX_TMU_TAPS_BB_SUBCMD_RAW_WRITE:
                        
                        break;
                    case SOC_SBX_TMU_TAPS_BB_SUBCMD_WRITE:
                        /* pack key need to be done first since it might 
                         * modify the other fields in the buffer
                         */
                        status = _tmu_cmd_taps_key_pack(unit, cmd, buffer);

                        buffer[7] |=  (cmd->cmd.taps.subcmd.bb_write.offset << CMD_TAPS_SUBCMD_BB_W_OFFf_SHFT ) |
                          (cmd->cmd.taps.subcmd.bb_write.segment << CMD_TAPS_SUBCMD_BB_W_SEGf_SHFT) |
                          (cmd->cmd.taps.subcmd.bb_write.format << CMD_TAPS_SUBCMD_BB_W_FORf_SHFT);
                        
                        buffer[6] = (cmd->cmd.taps.subcmd.bb_write.prefix_id << CMD_TAPS_SUBCMD_BB_W_PREf_SHFT) |
                          (cmd->cmd.taps.subcmd.bb_write.kshift  << CMD_TAPS_SUBCMD_BB_W_KSHFf_SHFT);

                        buffer[4] |= (cmd->cmd.taps.subcmd.bb_write.length  << CMD_TAPS_SUBCMD_BB_W_KSHFf_SHFT) |
                          (cmd->cmd.taps.subcmd.bb_write.align_right << CMD_TAPS_SUBCMD_BB_W_ALRf_SHFT);

                        break;
                    case SOC_SBX_TMU_TAPS_BB_SUBCMD_READ:

                        soc_mem_field_set(unit, TAPS_BB_CMD_WRITEm, buffer,
                                          SEGf, &cmd->cmd.taps.subcmd.bb_read.segment);
                        soc_mem_field_set(unit, TAPS_BB_CMD_WRITEm, buffer,
                                          OFFSETf, &cmd->cmd.taps.subcmd.bb_read.offset);
                        soc_mem_field_set(unit, TAPS_BB_CMD_WRITEm, buffer,
                                          FORMATf, &cmd->cmd.taps.subcmd.bb_read.format);
                        soc_mem_field_set(unit, TAPS_BB_CMD_WRITEm, buffer,
                                          KSHIFTf, &cmd->cmd.taps.subcmd.bb_read.kshift);
                        soc_mem_field_set(unit, TAPS_BB_CMD_WRITEm, buffer,
                                          PNUMBERf, &cmd->cmd.taps.subcmd.bb_read.prefix_id);
                        soc_mem_field_set(unit, TAPS_BB_CMD_WRITEm, buffer,
                                          ALIGN_RIGHTf, &cmd->cmd.taps.subcmd.bb_read.align_right);
                        break;
                    default:
                        return SOC_E_PARAM;
                }
                break;
            case SOC_SBX_TMU_TAPS_BLOCK_BRR:
                /* Taps BRR block */
                switch (cmd->cmd.taps.opcode) {
                    case SOC_SBX_TMU_TAPS_BRR_SUBCMD_NOP:
                    case SOC_SBX_TMU_TAPS_BRR_SUBCMD_READ_WIDE:
                    case SOC_SBX_TMU_TAPS_BRR_SUBCMD_WRITE_WIDE:
                        
                        break;
                    case SOC_SBX_TMU_TAPS_BRR_SUBCMD_WRITE:
                      buffer[7] |= (cmd->cmd.taps.subcmd.bb_write.offset <<  CMD_TAPS_SUBCMD_BB_W_OFFf_SHFT)  |
                        (cmd->cmd.taps.subcmd.bb_write.segment << CMD_TAPS_SUBCMD_BB_W_SEGf_SHFT) |
                        (cmd->cmd.taps.subcmd.bb_write.format <<  CMD_TAPS_SUBCMD_BB_W_FORf_SHFT);
                      
                      buffer[6] = cmd->cmd.taps.subcmd.brr_write.prefix_id << CMD_TAPS_SUBCMD_BB_W_PREf_SHFT;
                      buffer[4] = cmd->cmd.taps.subcmd.brr_write.adata << CMD_TAPS_SUBCMD_BB_W_ADf_SHFT  ;

                break;

                    case SOC_SBX_TMU_TAPS_BRR_SUBCMD_READ:
                        soc_mem_field_set(unit, TAPS_BRR_CMD_WRITEm, buffer,
                                          SEGf, &cmd->cmd.taps.subcmd.brr_read.segment);
                        soc_mem_field_set(unit, TAPS_BRR_CMD_WRITEm, buffer,
                                          OFFSETf, &cmd->cmd.taps.subcmd.brr_read.offset);
                        soc_mem_field_set(unit, TAPS_BRR_CMD_WRITEm, buffer,
                                          FORMATf, &cmd->cmd.taps.subcmd.brr_read.format);
                        soc_mem_field_set(unit, TAPS_BRR_CMD_WRITEm, buffer,
                                          PNUMBERf, &cmd->cmd.taps.subcmd.brr_read.prefix_id);
                        break;
                    default:
                        return SOC_E_PARAM;
                }
                break;
            default:
                return SOC_E_PARAM;
        }
        break;
    case SOC_SBX_TMU_CMD_LOCK:
        break;
    case SOC_SBX_TMU_CMD_RELEASE:
        break;
    case SOC_SBX_TMU_CMD_TRAILER:
        /* nothing to do */
        break;
    default:
        return 1; /* non zero error */
    }

    return status;
}

int tmu_cmd_readBuf(int unit,
                    soc_sbx_caladan3_tmu_cmd_t *cmd, 
                    uint32 *buffer,
                    unsigned int length,
                    uint8 response)
{
     uint clear=0;    
    /*validate interface structure parameters*/
    if (cmd == NULL || buffer == NULL || length == 0) {
        return SOC_E_PARAM;
    }

    cmd->opcode = (buffer[1] >> CMD_NOP_OPCODEf_SHFT)  & CMD_NOP_OPCODEf_MSK;
    cmd->seqnum = (buffer[1] >> CMD_NOP_SEQNUMf_SHFT)  & CMD_NOP_SEQNUMf_MSK;
    
    if (response) {
        cmd->response_type = (cmd->opcode == SOC_SBX_TMU_CMD_TAPS) ? SOC_SBX_TMU_TAPS_RESPONSE : SOC_SBX_TMU_RESPONSE;

        if (cmd->response_type == SOC_SBX_TMU_TAPS_RESPONSE) {
          cmd->resp_ctrl.taps.err0 = (buffer[1] >> CMD_UPD_TAPS_RESPm_ERR0f_SHFT ) & CMD_UPD_TAPS_RESPm_ERR0f_MSK ;
          cmd->resp_ctrl.taps.err1 = (buffer[1] >> CMD_UPD_TAPS_RESPm_ERR1f_SHFT ) & CMD_UPD_TAPS_RESPm_ERR1f_MSK ;
          cmd->resp_ctrl.taps.valid0 = (buffer[0] >> CMD_UPD_TAPS_RESPm_VALID0f_SHFT ) & CMD_UPD_TAPS_RESPm_VALID0f_MSK;
          cmd->resp_ctrl.taps.valid1 = (buffer[0] >> CMD_UPD_TAPS_RESPm_VALID1f_SHFT ) & CMD_UPD_TAPS_RESPm_VALID1f_MSK;
        } else {
          cmd->resp_ctrl.resp.size = (buffer[0] >> CMD_UPD_TAPS_RESPm_SIZEf_SHFT) & CMD_UPD_TAPS_RESPm_SIZEf_MSK;
          cmd->resp_ctrl.resp.errcode = (buffer[1] >> CMD_UPD_TAPS_RESPm_ERRCODEf_SHFT) & CMD_UPD_TAPS_RESPm_ERRCODEf_MSK;
        }

    }

    switch(cmd->opcode) {
    case SOC_SBX_TMU_CMD_NOP:
        {
            cmd->cmd.nop.echo = (buffer[1] >> CMD_NOP_ECHOf_SHFT ) & CMD_NOP_ECHOf_MSK;
        }
        break;
    case SOC_SBX_TMU_CMD_XL_READ:
        {
            if (!response) {

              cmd->cmd.xlread.entry_num = (buffer[0] >>  CMD_XLREAD_ENTRYNUMf_SHFT) &  CMD_XLREAD_ENTRYNUMf_MSK;
              cmd->cmd.xlread.lookup = (buffer[1] >> CMD_XLREAD_LOOKUPf_SHFT ) & CMD_XLREAD_LOOKUPf_MSK;
              cmd->cmd.xlread.table = (buffer[1] >> CMD_XLREAD_TABLEf_SHFT ) & CMD_XLREAD_TABLEf_MSK;
              cmd->cmd.xlread.kv_pairs = (buffer[0] >> CMD_XLREAD_KVPRf_SHFT ) & CMD_XLREAD_KVPRf_MSK;
              
              soc_mem_field_get(unit, TMB_UPDATER_CMD_XL_READm,
                                buffer, RESV1f, &clear);
              soc_mem_field_get(unit, TMB_UPDATER_CMD_XL_READm,
                                buffer, RESV2f, &clear);
            } else {
            }
        }
        break;
    case SOC_SBX_TMU_CMD_XL_WRITE:
        {
          if (!response) {
            cmd->cmd.xlwrite.entry_num = (buffer[0] >> CMD_XLWRITE_ENTRYNUMf_SHFT ) & CMD_XLWRITE_ENTRYNUMf_MSK;
            cmd->cmd.xlwrite.size = (buffer[0] >> CMD_XLWRITE_SIZEf_SHFT ) & CMD_XLWRITE_SIZEf_MSK;
            cmd->cmd.xlwrite.table = (buffer[1] >> CMD_XLWRITE_TABLEf_SHFT) & CMD_XLWRITE_TABLEf_MSK; 
            cmd->cmd.xlwrite.lookup = (buffer[1] >> CMD_XLWRITE_LOOKUPf_SHFT) & CMD_XLWRITE_LOOKUPf_MSK;
            cmd->cmd.xlwrite.offset = (buffer[1]>> CMD_XLWRITE_OFFSETf_SHFT) & CMD_XLWRITE_OFFSETf_MSK;
            clear = (buffer[1] >> CMD_XLWRITE_RSV1f_SHFT) & CMD_XLWRITE_RSV1f_MSK;
            cmd->cmd.eml.table_id = (buffer[1] >> CMD_EMLm_TABLEIDf_SHFT ) & CMD_EMLm_TABLEIDf_MSK ;

          } else {
          }
        }
        break;
    case SOC_SBX_TMU_CMD_RAW_READ:
        break;
    case SOC_SBX_TMU_CMD_RAW_WRITE:
        break;
    case SOC_SBX_TMU_CMD_EML_INS_BEGIN:
    case SOC_SBX_TMU_CMD_EML_INS_END:
    case SOC_SBX_TMU_CMD_EML_DELETE:
        break;
    case SOC_SBX_TMU_CMD_TAPS:  
        break;
    case SOC_SBX_TMU_CMD_LOCK:
        break;
    case SOC_SBX_TMU_CMD_RELEASE:
        break;
    case SOC_SBX_TMU_CMD_TRAILER:
        /* nothing to do */
        break;
    default:
        return 1; /* non zero error */
    }

    return SOC_E_NONE; 
}

int tmu_cmd_is_empty_response(int unit, soc_sbx_caladan3_tmu_cmd_t *cmd, uint8 *empty)
{
    if (cmd == NULL || empty == NULL) {
        return SOC_E_PARAM;
    }

    if (cmd->opcode == SOC_SBX_TMU_CMD_NOP && 
        cmd->cmd.nop.echo == 0) {
        *empty = TRUE;
    } else {
        *empty = FALSE;
    }

    return SOC_E_NONE;
}

/* determine number of octets required to pack a given command */
int tmu_cmd_get_size_bits(soc_sbx_caladan3_tmu_cmd_t *cmd, int *bits)
{
    if (cmd == NULL || bits == NULL) {
        return SOC_E_PARAM;
    }

    *bits = 64; /* most of messages are 64 bits */

    switch (cmd->opcode) {
    case SOC_SBX_TMU_CMD_XL_WRITE:
        *bits += cmd->cmd.xlwrite.value_size;
        break;
    case SOC_SBX_TMU_CMD_EML_INS_BEGIN:
    case SOC_SBX_TMU_CMD_EML_INS_END:
    case SOC_SBX_TMU_CMD_EML_DELETE:
        *bits += _g_tmu_key_size_bits[cmd->cmd.eml.key_type] + 
                 ((cmd->opcode == SOC_SBX_TMU_CMD_EML_DELETE) ? 0:SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS);
        break;
    case SOC_SBX_TMU_CMD_TAPS:
        *bits = 64 * 5; /* taps command are 320 bits */
        break;
    default:
        break;
    }

    return SOC_E_NONE;
}

void tmu_resp_error_print(int unit, soc_sbx_caladan3_tmu_cmd_t *resp)
{
    if (!resp || resp->response_type == SOC_SBX_TMU_RESP_MAX) {
        LOG_CLI((BSL_META_U(unit,
                            "Unit[%d]: Bad input argument !!\n"), unit));
        return;
    }

    if (resp->response_type == SOC_SBX_TMU_RESPONSE) {
        /* non-TAPS response */
        if (resp->resp_ctrl.resp.errcode > 0) {
            switch(resp->resp_ctrl.resp.errcode) {
                case 0:
                    LOG_CLI((BSL_META_U(unit,
                                        "Response: Success \n")));
                    break;
                    
                case TMU_ERR_CODE_EML_NO_HARDWARE_CHAIN:
                    LOG_CLI((BSL_META_U(unit,
                                        " TMU Error Type = TMU_ERR_CODE_EML_NO_HARDWARE_CHAIN !!\n")));
                    break;
                    
                case TMU_ERR_CODE_EML_NO_FREE_CHAIN:
                    LOG_CLI((BSL_META_U(unit,
                                        " TMU Error Type = TMU_ERR_CODE_EML_NO_FREE_CHAIN !!\n")));
                    break;
                    
                case TMU_ERR_CODE_EML_CHAIN0_FULL:
                    LOG_CLI((BSL_META_U(unit,
                                        " TMU Error Type = TMU_ERR_CODE_EML_CHAIN0_FULL !!\n")));
                    break;
                    
                case TMU_ERR_CODE_EML_CHAIN1_FULL:
                    LOG_CLI((BSL_META_U(unit,
                                        " TMU Error Type = TMU_ERR_CODE_EML_CHAIN1_FULL !!\n")));
                    break;
                    
                case TMU_ERR_CODE_EML_KEY_NOT_FOUND:
                    LOG_CLI((BSL_META_U(unit,
                                        " TMU Error Type = TMU_ERR_CODE_EML_KEY_NOT_FOUND !!\n")));
                    break;
                    
                case TMU_ERR_CODE_EML_LOCK_FAIL:
                    LOG_CLI((BSL_META_U(unit,
                                        " TMU Error Type = TMU_ERR_CODE_EML_LOCK_FAIL !!\n")));
                    break;
                    
                case TMU_ERR_CODE_EML_FILTER_NO_UPDATE:
                    LOG_CLI((BSL_META_U(unit,
                                        " TMU Error Type = TMU_ERR_CODE_EML_FILTER_NO_UPDATE !!\n")));
                    break;
                    
                case TMU_ERR_CODE_EML_NO_BD_MATCH:
                    LOG_CLI((BSL_META_U(unit,
                                        " TMU Error Type = TMU_ERR_CODE_EML_NO_BD_MATCH !!\n")));
                    break;
                    
                case TMU_ERR_CODE_RAW_STRADDELS_ROW:
                    LOG_CLI((BSL_META_U(unit,
                                        " TMU Error Type = TMU_ERR_CODE_RAW_STRADDELS_ROW !!\n")));
                    break;
                    
                case TMU_ERR_CODE_BAD_WRITE_SIZE:
                    LOG_CLI((BSL_META_U(unit,
                                        " TMU Error Type = TMU_ERR_CODE_BAD_WRITE_SIZE !!\n")));
                    break;
                    
                case TMU_ERR_CODE_RAW_KV_PAIRS_IS_0:
                    LOG_CLI((BSL_META_U(unit,
                                        " TMU Error Type = TMU_ERR_CODE_RAW_KV_PAIRS_IS_0 !!\n")));
                    break;
                    
                case TMU_ERR_CODE_SRAM_ECC_ERROR:
                    LOG_CLI((BSL_META_U(unit,
                                        " TMU Error Type = TMU_ERR_CODE_SRAM_ECC_ERROR !!\n")));
                    break;
                    
                case TMU_ERR_CODE_DRAM_ECC_ERROR:
                    LOG_CLI((BSL_META_U(unit,
                                        " TMU Error Type = TMU_ERR_CODE_DRAM_ECC_ERROR !!\n")));
                    break;
                    
                default:
                    LOG_CLI((BSL_META_U(unit,
                                        " TMU Error Type = UNKNOWN ERROR CODE \n")));
                    break;
            }
        }
    } else {
        /* TAPS response */
        if (resp->resp_ctrl.taps.err0) {
            LOG_CLI((BSL_META_U(unit,
                                "Response: TMU TAPS 0 ERROR \n")));          
        } else if (resp->resp_ctrl.taps.err1) {
            LOG_CLI((BSL_META_U(unit,
                                "Response: TMU TAPS 1 ERROR \n")));
        } else {
            LOG_CLI((BSL_META_U(unit,
                                "Response: TMU TAPS Success \n")));
        }
    }
}

void tmu_dump_cmd_buffer(uint32 *buf, int len)  
{
    while(--len >= 0) {
        LOG_CLI((BSL_META("0x%08x "), buf[len]));
    }
    LOG_CLI((BSL_META("\n")));                                                
}

#define PRINT_VAL(X) \
    LOG_CLI((BSL_META("%s = \t0x%08x = %u\n"), #X, cmd->X, cmd->X));

void tmu_cmd_printf(int unit, soc_sbx_caladan3_tmu_cmd_t *cmd)
{
    if (cmd == NULL) {
        LOG_CLI((BSL_META_U(unit,
                            "!! NULL command argument \n")));
    } else {    
        LOG_CLI((BSL_META_U(unit,
                            " On unit %d Message Type = %s \n"), unit, tmu_cmd_str[cmd->opcode])); 
        PRINT_VAL(opcode);
        PRINT_VAL(seqnum);

        if (cmd->response_type == SOC_SBX_TMU_RESP_MAX) {
        switch(cmd->opcode) {
        case SOC_SBX_TMU_CMD_NOP:
            PRINT_VAL(cmd.nop.echo);
            break;
        case SOC_SBX_TMU_CMD_XL_READ:
            PRINT_VAL(cmd.xlread.table);
            PRINT_VAL(cmd.xlread.lookup);
            PRINT_VAL(cmd.xlread.kv_pairs);
            PRINT_VAL(cmd.xlread.entry_num);
            break;
        case SOC_SBX_TMU_CMD_XL_WRITE:
            PRINT_VAL(cmd.xlwrite.table);
            PRINT_VAL(cmd.xlwrite.lookup);
            PRINT_VAL(cmd.xlwrite.offset);
            PRINT_VAL(cmd.xlwrite.size);
            PRINT_VAL(cmd.xlwrite.entry_num);
            LOG_CLI((BSL_META_U(unit,
                                " Value Dump: ")));
            tmu_dump_cmd_buffer(&(cmd->cmd.xlwrite.value_data[0]), BITS2WORDS(cmd->cmd.xlwrite.value_size));
            break;
        case SOC_SBX_TMU_CMD_RAW_READ:
            
            break;
        case SOC_SBX_TMU_CMD_RAW_WRITE:
            
            break;
        case SOC_SBX_TMU_CMD_EML_INS_BEGIN:
        case SOC_SBX_TMU_CMD_EML_INS_END:
        case SOC_SBX_TMU_CMD_EML_DELETE:
            PRINT_VAL(cmd.eml.filter);
            PRINT_VAL(cmd.eml.key_type);
            PRINT_VAL(cmd.eml.table_id);
            LOG_CLI((BSL_META_U(unit,
                                "# KEY Dump: ")));
            tmu_dump_cmd_buffer(cmd->cmd.eml.key, BITS2WORDS(_g_tmu_key_size_bits[cmd->cmd.eml.key_type]));
            if (cmd->opcode != SOC_SBX_TMU_CMD_EML_DELETE) {
                LOG_CLI((BSL_META_U(unit,
                                    "$ Value Dump: ")));
                tmu_dump_cmd_buffer(cmd->cmd.eml.value, BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS));
            }
            break;
        case SOC_SBX_TMU_CMD_TAPS:
            PRINT_VAL(cmd.taps.instance);
            PRINT_VAL(cmd.taps.blk);
            PRINT_VAL(cmd.taps.opcode);
            switch (cmd->cmd.taps.blk) {            
                case SOC_SBX_TMU_TAPS_BLOCK_RPB:
                    switch (cmd->cmd.taps.opcode) {
                        case SOC_SBX_TMU_TAPS_RPB_SUBCMD_NOP:
                        case SOC_SBX_TMU_TAPS_RPB_SUBCMD_FIND_BUCKET:
                        case SOC_SBX_TMU_TAPS_RPB_SUBCMD_RAW_READ:
                        case SOC_SBX_TMU_TAPS_RPB_SUBCMD_RAW_WRITE:
                            break;
                        case SOC_SBX_TMU_TAPS_RPB_SUBCMD_WRITE:
                            PRINT_VAL(cmd.taps.subcmd.rpb_write.segment);
                            PRINT_VAL(cmd.taps.subcmd.rpb_write.offset);
                            PRINT_VAL(cmd.taps.subcmd.rpb_write.target);
                            PRINT_VAL(cmd.taps.subcmd.rpb_write.bpm_length);
                            PRINT_VAL(cmd.taps.subcmd.rpb_write.kshift);
                            PRINT_VAL(cmd.taps.subcmd.rpb_write.bucket);
                            PRINT_VAL(cmd.taps.subcmd.rpb_write.best_match);
                            PRINT_VAL(cmd.taps.subcmd.rpb_write.align_right);
                            PRINT_VAL(cmd.taps.subcmd.rpb_write.length);
                            LOG_CLI((BSL_META_U(unit,
                                                "# KEY Dump: Length %d "),
                                     cmd->cmd.taps.subcmd.rpb_write.length));
                            tmu_dump_cmd_buffer(cmd->cmd.taps.subcmd.rpb_write.key,
                                                BITS2WORDS(cmd->cmd.taps.subcmd.rpb_write.length));
                            break;
                        case SOC_SBX_TMU_TAPS_RPB_SUBCMD_READ:
                            PRINT_VAL(cmd.taps.subcmd.rpb_read.segment);
                            PRINT_VAL(cmd.taps.subcmd.rpb_read.offset);
                            PRINT_VAL(cmd.taps.subcmd.rpb_read.target);
                            break;
                        case SOC_SBX_TMU_TAPS_RPB_SUBCMD_PROPAGATE:
                            PRINT_VAL(cmd.taps.subcmd.rpb_propagate.segment);
                            PRINT_VAL(cmd.taps.subcmd.rpb_propagate.best_match);
                            PRINT_VAL(cmd.taps.subcmd.rpb_propagate.length);
                            LOG_CLI((BSL_META_U(unit,
                                                "# KEY Dump: Length %d "),
                                     cmd->cmd.taps.subcmd.rpb_propagate.length));
                            tmu_dump_cmd_buffer(cmd->cmd.taps.subcmd.rpb_propagate.key,
                                                BITS2WORDS(cmd->cmd.taps.subcmd.rpb_propagate.length));
                            break;
                        case SOC_SBX_TMU_TAPS_RPB_SUBCMD_REPLACE:
                            PRINT_VAL(cmd.taps.subcmd.rpb_replace.segment);
                            PRINT_VAL(cmd.taps.subcmd.rpb_replace.bpm_length);
                            PRINT_VAL(cmd.taps.subcmd.rpb_replace.best_match);
                            PRINT_VAL(cmd.taps.subcmd.rpb_replace.length);
                            LOG_CLI((BSL_META_U(unit,
                                                "# KEY Dump: Length %d "),
                                     cmd->cmd.taps.subcmd.rpb_replace.length));
                            tmu_dump_cmd_buffer(cmd->cmd.taps.subcmd.rpb_replace.key,
                                                BITS2WORDS(cmd->cmd.taps.subcmd.rpb_replace.length));
                            break;
                        default:
                            break;
                    }
                    break;
                case SOC_SBX_TMU_TAPS_BLOCK_BB:
                    switch (cmd->cmd.taps.opcode) {
                        case SOC_SBX_TMU_TAPS_BB_SUBCMD_NOP:
                        case SOC_SBX_TMU_TAPS_BB_SUBCMD_RAW_READ:
                        case SOC_SBX_TMU_TAPS_BB_SUBCMD_RAW_WRITE:
                            break;
                        case SOC_SBX_TMU_TAPS_BB_SUBCMD_WRITE:
                            PRINT_VAL(cmd.taps.subcmd.bb_write.segment);
                            PRINT_VAL(cmd.taps.subcmd.bb_write.offset);
                            PRINT_VAL(cmd.taps.subcmd.bb_write.format);
                            PRINT_VAL(cmd.taps.subcmd.bb_write.kshift);
                            PRINT_VAL(cmd.taps.subcmd.bb_write.prefix_id);
                            PRINT_VAL(cmd.taps.subcmd.bb_write.align_right);
                            PRINT_VAL(cmd.taps.subcmd.bb_write.length);
                            LOG_CLI((BSL_META_U(unit,
                                                "# KEY Dump: Length %d "),
                                     cmd->cmd.taps.subcmd.bb_write.length));
                            tmu_dump_cmd_buffer(cmd->cmd.taps.subcmd.bb_write.key,
                                                BITS2WORDS(cmd->cmd.taps.subcmd.bb_write.length));
                            break;
                        case SOC_SBX_TMU_TAPS_BB_SUBCMD_READ:
                            PRINT_VAL(cmd.taps.subcmd.bb_read.segment);
                            PRINT_VAL(cmd.taps.subcmd.bb_read.offset);
                            PRINT_VAL(cmd.taps.subcmd.bb_read.format);
                            PRINT_VAL(cmd.taps.subcmd.bb_read.kshift);
                            PRINT_VAL(cmd.taps.subcmd.bb_read.prefix_id);
                            PRINT_VAL(cmd.taps.subcmd.bb_read.align_right);
                            break;
                        default:
                            break;
                    }
                    break;
                case SOC_SBX_TMU_TAPS_BLOCK_BRR:
                    switch (cmd->cmd.taps.opcode) {
                        case SOC_SBX_TMU_TAPS_BRR_SUBCMD_NOP:
                        case SOC_SBX_TMU_TAPS_BRR_SUBCMD_READ_WIDE:
                        case SOC_SBX_TMU_TAPS_BRR_SUBCMD_WRITE_WIDE:
                            break;
                        case SOC_SBX_TMU_TAPS_BRR_SUBCMD_WRITE:
                            PRINT_VAL(cmd.taps.subcmd.brr_write.segment);
                            PRINT_VAL(cmd.taps.subcmd.brr_write.offset);
                            PRINT_VAL(cmd.taps.subcmd.brr_write.format);
                            PRINT_VAL(cmd.taps.subcmd.brr_write.prefix_id);
                            PRINT_VAL(cmd.taps.subcmd.brr_write.adata);
                            break;
                        case SOC_SBX_TMU_TAPS_BRR_SUBCMD_READ:
                            PRINT_VAL(cmd.taps.subcmd.brr_read.segment);
                            PRINT_VAL(cmd.taps.subcmd.brr_read.offset);
                            PRINT_VAL(cmd.taps.subcmd.brr_read.format);
                            PRINT_VAL(cmd.taps.subcmd.brr_read.prefix_id);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case SOC_SBX_TMU_CMD_LOCK:
            break;
        case SOC_SBX_TMU_CMD_RELEASE:
            break;
        case SOC_SBX_TMU_CMD_TRAILER:
            break;
        default:
            break;
        }
        } else {
            if (cmd->response_type == SOC_SBX_TMU_TAPS_RESPONSE) {
                LOG_CLI((BSL_META_U(unit,
                                    "add support!! \n")));
            } else {
                PRINT_VAL(resp_ctrl.resp.size);
                PRINT_VAL(resp_ctrl.resp.errcode);
                tmu_resp_error_print(unit, cmd);
            }
            LOG_CLI((BSL_META_U(unit,
                                "Trailer on Response: %s \n"), 
                     (cmd->response_trailer)? "YES":"NO"));
        }
    }
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_cmd_mgr_stats_dump
 * Purpose:
 *     Dump command manager statistics
 */
void soc_sbx_caladan3_cmd_mgr_stats_dump(int unit)
{
    int count=0, index;

    TMU_CMD_LOCK_TAKE

    DQ_LENGTH(&_g_tmu_cmd_dbase[unit]->cmd_free_list, count);
    if (count > 0) 
        LOG_CLI((BSL_META_U(unit,
                            "Free commands: %d\n"), count));
    DQ_LENGTH(&_g_tmu_cmd_dbase[unit]->cmd_alloc_list, count);
    if (count > 0) 
        LOG_CLI((BSL_META_U(unit,
                            "alloc commands: %d\n"), count));

    for (index=0; index < SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO; index++) {
        DQ_LENGTH(&_g_tmu_cmd_dbase[unit]->cmd_resp_list[index], count);
        if (count > 0) 
            LOG_CLI((BSL_META_U(unit,
                                "fifoid:%d response list: %d\n"), index, count));
    }

    DQ_LENGTH(&_g_tmu_cmd_dbase[unit]->cmd_resp_process_list, count);
    if (count > 0) 
        LOG_CLI((BSL_META_U(unit,
                            "response process list: %d\n"), count));

    TMU_CMD_LOCK_GIVE


}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_table_map_dump
 * Purpose:
 *     Dump TMU table allocation
 */
void soc_sbx_caladan3_cmd_mgr_dump(int unit, int fifoid)
{
    dq_p_t elem;
    int count=0;

    /* initialize sws database */
    if (_g_tmu_cmd_dbase[unit] == NULL) {
        LOG_CLI((BSL_META_U(unit,
                            "!!! Command Manager uninitialized !!!\n")));
    } else {

        TMU_CMD_LOCK_TAKE

        DQ_LENGTH(&_g_tmu_cmd_dbase[unit]->cmd_free_list, count);
        LOG_CLI((BSL_META_U(unit,
                            "Free commands: %d\n"), count));
        DQ_LENGTH(&_g_tmu_cmd_dbase[unit]->cmd_alloc_list, count);
        LOG_CLI((BSL_META_U(unit,
                            "alloc commands: %d\n"), count));
        DQ_LENGTH(&_g_tmu_cmd_dbase[unit]->cmd_resp_list[fifoid], count);
        LOG_CLI((BSL_META_U(unit,
                            "fifoid:%d response list: %d\n"), fifoid, count));
        DQ_LENGTH(&_g_tmu_cmd_dbase[unit]->cmd_resp_process_list, count);
        LOG_CLI((BSL_META_U(unit,
                            "response process list: %d\n"), count));
    
        LOG_CLI((BSL_META_U(unit,
                            "### Outstanding Response list ### \n")));
        DQ_TRAVERSE(&_g_tmu_cmd_dbase[unit]->cmd_resp_list[fifoid], elem) {
            soc_sbx_caladan3_tmu_cmd_t *cmd = DQ_ELEMENT_GET(soc_sbx_caladan3_tmu_cmd_t*, elem, list_elem); 
            tmu_cmd_printf(unit, cmd);
        } DQ_TRAVERSE_END(&_g_tmu_cmd_dbase[unit]->cmd_resp_list[fifoid], elem);
        LOG_CLI((BSL_META_U(unit,
                            "################################## \n")));
        
        LOG_CLI((BSL_META_U(unit,
                            "### Processed Response list ### \n")));
        DQ_TRAVERSE(&_g_tmu_cmd_dbase[unit]->cmd_resp_process_list, elem) {
            soc_sbx_caladan3_tmu_cmd_t *cmd = DQ_ELEMENT_GET(soc_sbx_caladan3_tmu_cmd_t*, elem, list_elem); 
            tmu_cmd_printf(unit, cmd);
        } DQ_TRAVERSE_END(&_g_tmu_cmd_dbase[unit]->cmd_resp_process_list[fifoid], elem);
        LOG_CLI((BSL_META_U(unit,
                            "################################## \n")));

        LOG_CLI((BSL_META_U(unit,
                            "### Alloc Response list ### \n")));
        DQ_TRAVERSE(&_g_tmu_cmd_dbase[unit]->cmd_alloc_list, elem) {
            soc_sbx_caladan3_tmu_cmd_t *cmd = DQ_ELEMENT_GET(soc_sbx_caladan3_tmu_cmd_t*, elem, list_elem); 
            tmu_cmd_printf(unit, cmd);
        } DQ_TRAVERSE_END(&_g_tmu_cmd_dbase[unit]->cmd_alloc_list[fifoid], elem);
        LOG_CLI((BSL_META_U(unit,
                            "################################## \n")));

        TMU_CMD_LOCK_GIVE
    }
}


/*
 *
 * Function:
 *     tmu_cmd_count_clear
 * Purpose:
 *     Clear the counts of TMU commands
 */

void tmu_cmd_count_clear()
{
  uint32 i,j;

  for(i=0;i < SOC_SBX_TMU_CMD_MAX; i++)
    _tmu_cmd_counts[i]=0;

  for(i=0;i < SOC_SBX_TMU_TAPS_BLOCK_MAX; i++)
    for(j=0;j < SOC_SBX_TMU_TAPS_SUBCMD_MAX; j++)
      _tmu_taps_cmd_counts[i][j]=0;

}

/*
 *
 * Function:
 *     tmu_cmd_count_print
 * Purpose:
 *     Print the counts of TMU commands
 */

void tmu_cmd_count_print()
{
    uint32 taps_blk;
    uint32 index;
    uint32 rpb_subcmd,bb_subcmd,brr_subcmd;

    LOG_CLI((BSL_META("\nCounts for TMU Commands:\n\n")));

    for(index=0;index < SOC_SBX_TMU_CMD_RELEASE; index++)
      {
        if(index != SOC_SBX_TMU_CMD_TAPS)
          LOG_CLI((BSL_META("%s:\t\tCount %d\n"),tmu_cmd_str[index],_tmu_cmd_counts[index]));
        else{
          LOG_CLI((BSL_META("%s:\n"),tmu_cmd_str[index]));
          for(taps_blk=1; taps_blk < SOC_SBX_TMU_TAPS_BLOCK_MAX; taps_blk++)
            {
              LOG_CLI((BSL_META("  %s:\n"),tmu_taps_blk_str[taps_blk]));
              switch(taps_blk) {
              case SOC_SBX_TMU_TAPS_BLOCK_RPB:
                for(rpb_subcmd=0;rpb_subcmd< SOC_SBX_TMU_TAPS_RPB_SUBCMD_MAX; rpb_subcmd++)
                  LOG_CLI((BSL_META("      %s\t\t\tcount: %d\n"),tmu_taps_rpb_str[rpb_subcmd],_tmu_taps_cmd_counts[taps_blk][rpb_subcmd]));
                break;
              case SOC_SBX_TMU_TAPS_BLOCK_BB:
                for(bb_subcmd=0;bb_subcmd< SOC_SBX_TMU_TAPS_BB_SUBCMD_MAX; bb_subcmd++)
                  LOG_CLI((BSL_META("      %s\t\t\tcount: %d\n"),tmu_taps_bb_str[bb_subcmd],_tmu_taps_cmd_counts[taps_blk][bb_subcmd]));
                break;
              case SOC_SBX_TMU_TAPS_BLOCK_BRR:
                for(brr_subcmd=0;brr_subcmd< SOC_SBX_TMU_TAPS_BRR_SUBCMD_MAX; brr_subcmd++)
                  LOG_CLI((BSL_META("      %s\t\t\tcount: %d\n"),tmu_taps_brr_str[brr_subcmd],_tmu_taps_cmd_counts[taps_blk][brr_subcmd]));
                break;
              }
            }
        }
      }
}
/*
 *
 * Function:
 *     tmu_cmd_count
 * Purpose:
 *     Updates counter for the command
 */

void tmu_cmd_count(int unit, soc_sbx_caladan3_tmu_cmd_t *cmd)
{

  /* Check if we have a bad input */
  if (cmd == NULL) {
        LOG_CLI((BSL_META_U(unit,
                            "!! NULL command argument \n")));
        return;
  }

  /* Opcodes that are not for Taps are counted here */
  if(cmd->opcode != SOC_SBX_TMU_CMD_TAPS ) {
    
    /* Increment the count for the specific type */
    _tmu_cmd_counts[cmd->opcode]++;
    
  } 
  else /* Taps counters are broken out for specific Taps blocks and sub-command*/
    {
    /* Increment the count for the specific taps command type and subtype */
      _tmu_taps_cmd_counts[cmd->cmd.taps.blk][cmd->cmd.taps.opcode]++;
    }
}



#endif
 
