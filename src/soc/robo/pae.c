/*
 * $Id: pae.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains pae definitions internal to the BCM library.
 */
#include <soc/defs.h>

#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <soc/drv.h>
#include <bcm/error.h>
#include <soc/robo/pae.h>

/*pae database config register base addr */
#define PAE_LUE_DATABASE_CONFIG_ADDR 0x18049064
/* pae database config register */
#define PAE_REG_FLD_LUE_DATABASE_CONFIG_BASE_ADDR_LO   0
#define PAE_REG_FLD_LUE_DATABASE_CONFIG_BASE_ADDR_HI   6
#define PAE_REG_FLD_LUE_DATABASE_CONFIG_SEARCH_TYPE_LO 7
#define PAE_REG_FLD_LUE_DATABASE_CONFIG_SEARCH_TYPE_HI 12
#define PAE_REG_FLD_LUE_DATABASE_CONFIG_RULE_SETS_LO   13
#define PAE_REG_FLD_LUE_DATABASE_CONFIG_RULE_SETS_HI   18
#define PAE_REG_FLD_LUE_DATABASE_CONFIG_ENABLED_LO     19
#define PAE_REG_FLD_LUE_DATABASE_CONFIG_ENABLED_HI     19
#define PAE_REG_FLD_LUE_DATABASE_CONFIG_META_SETS_LO   21
#define PAE_REG_FLD_LUE_DATABASE_CONFIG_META_SETS_HI   25
/* bit field definitions of the LUE table control register */
#define PAE_REG_FLD_LUE_TBL_CTRL_TARGET_RULE_LO   0
#define PAE_REG_FLD_LUE_TBL_CTRL_TARGET_RULE_HI   0
#define PAE_REG_FLD_LUE_TBL_CTRL_TARGET_META_LO   1
#define PAE_REG_FLD_LUE_TBL_CTRL_TARGET_META_HI   1 
#define PAE_REG_FLD_LUE_TBL_CTRL_OPCODE_LO        2
#define PAE_REG_FLD_LUE_TBL_CTRL_OPCODE_HI        4
#define PAE_REG_FLD_LUE_TBL_CTRL_DONE_LO          5
#define PAE_REG_FLD_LUE_TBL_CTRL_DONE_HI          5
#define PAE_REG_FLD_LUE_TBL_CTRL_DB_NUM_LO        6
#define PAE_REG_FLD_LUE_TBL_CTRL_DB_NUM_HI        8
#define PAE_REG_FLD_LUE_TBL_CTRL_RULE_ADDR_LO     9
#define PAE_REG_FLD_LUE_TBL_CTRL_RULE_ADDR_HI     18

/* PAE lookup engine B1TCM */
#define PAE_MEM_LUE_LO       0x2A060000        
#define PAE_MEM_LUE_HI       0x2A07FFFF        

#define PAE_MEM_BASE(mem_name) (PAE_MEM_##mem_name##_LO)

#define PAE_LUE_RULE_PATTERN_ADDR (PAE_MEM_BASE(LUE) + 0x00)
#define PAE_LUE_RULE_PAYLOAD_ADDR (PAE_MEM_BASE(LUE) + 0x40)
#define PAE_LUE_RULE_CMD_ADDR     (PAE_MEM_BASE(LUE) + 0x60)



/* mask of n-bits */
#define PAE_MASK_32(x) ((1UL << (x)) - 1)
#define PAE_MASK_64(x) ((1ULL << (x)) - 1)


/* num bits in range */
#define PAE_WIDTH(bit_x, bit_y) ((bit_x) - (bit_y) + 1)

/* bit field manipulation */
#define PAE_HAL_BITS_SET_32(data, bit_hi, bit_lo, field) \
do { \
    (data) &= ~(PAE_MASK_32(PAE_WIDTH(bit_hi, bit_lo)) << (bit_lo)); \
    (data) |= ((PAE_MASK_32(PAE_WIDTH(bit_hi, bit_lo)) & (field)) << (bit_lo)); \
} while(0)
 
#define PAE_HAL_BITS_SET_64(data, bit_hi, bit_lo, field) \
do { \
    (data) &= ~(PAE_MASK_64(PAE_WIDTH(bit_hi, bit_lo)) << (bit_lo)); \
    (data) |= ((PAE_MASK_64(PAE_WIDTH(bit_hi, bit_lo)) & (field)) << (bit_lo)); \
} while(0)

#define PAE_HAL_BITS_GET_32(data, bit_hi, bit_lo)  (((data) >> bit_lo) & PAE_MASK_32(PAE_WIDTH(bit_hi, bit_lo)))
#define PAE_HAL_BITS_GET_64(data, bit_hi, bit_lo)  (((data) >> bit_lo) & PAE_MASK_64(PAE_WIDTH(bit_hi, bit_lo)))

#define PAE_HAL_REG_FLD_SET_32(data, field_name, value) PAE_HAL_BITS_SET_32(data, PAE_REG_FLD_##field_name##_HI, PAE_REG_FLD_##field_name##_LO, value)
#define PAE_HAL_REG_FLD_SET_64(data, field_name, value) PAE_HAL_BITS_SET_64(data, PAE_REG_FLD_##field_name##_HI, PAE_REG_FLD_##field_name##_LO, value)
#define PAE_HAL_REG_FLD_GET_64(data, field_name) PAE_HAL_BITS_GET_64(data, PAE_REG_FLD_##field_name##_HI, PAE_REG_FLD_##field_name##_LO)

/* register read/write */
#define PAE_HAL_READ_32(addr)        _pae_read_mem32((uint32)addr,0)
#define PAE_HAL_READ_64(addr)        _pae_read_mem64((uint32)addr,0)
#define PAE_HAL_WRITE_32(addr, data) _pae_write_mem32((uint32)addr,0,(uint32)data)
#define PAE_HAL_WRITE_64(addr, data) _pae_write_mem64((uint32)addr,0,(uint64)data)

/* indexed reads/writes */
#define PAE_HAL_READ_IDX_32(addr, idx)        _pae_read_mem32((uint32)addr,(uint32)idx)
#define PAE_HAL_READ_IDX_64(addr, idx)        _pae_read_mem64((uint32)addr,(uint32)idx)
#define PAE_HAL_WRITE_IDX_32(addr, idx, data) _pae_write_mem32((uint32)addr,(uint32)idx,(uint32)data)
#define PAE_HAL_WRITE_IDX_64(addr, idx, data) _pae_write_mem64((uint32)addr,(uint32)idx,(uint64)data)

/* divide, round up */
#define PAE_DIVUP(x, y) (((x) + (y) - 1) / (y))

typedef enum {
    PAE_LUE_OP_READ = 0,
    PAE_LUE_OP_WRITE,
    PAE_LUE_OP_READ_RAW,
    PAE_LUE_OP_WRITE_RAW,
    PAE_LUE_OP_CLEAR,
    /* leave as last */
    PAE_LUE_OP_MAX
} pae_lue_op_et;

typedef enum {
    PAE_LUE_OPC_NOP = 0x0,
    PAE_LUE_OPC_READ,
    PAE_LUE_OPC_WRITE,
    PAE_LUE_OPC_RAW_READ,
    PAE_LUE_OPC_RAW_WRITE,
    /* leave as last */
    PAE_LUE_MAX
} pae_lue_opcode_et;

/* do we wait for DONE bit on DB writes? */
#define PAE_LUE_WAIT_WR_COMPLETE 1
/* timeout waiting for DONE bit */
#define PAE_LUE_READ_TIMEOUT     1000

#define PAE_LUE_NUM_DBS        8 
#define PAE_LUE_NUM_RULES      256 /*1024*/
#define PAE_LUE_NUM_IN_RULESET 8
/* number of bytes in rule / meta data */
#define PAE_LUE_RULE_NUM_BYTES 8
#define PAE_LUE_RULE_NUM_WORDS (PAE_LUE_RULE_NUM_BYTES / sizeof(uint64))
#define PAE_LUE_META_NUM_BYTES 8
#define PAE_LUE_META_NUM_WORDS (PAE_LUE_META_NUM_BYTES / sizeof(uint64))

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#define R5_BIG_ENDIAN
#define HOST_LITTLE_ENDIAN

#define PAE_SHARED_MSG_RES_CODE 0x2A040000
#define PAE_SHARED_MSG_TYPE (PAE_SHARED_MSG_RES_CODE+4)
#define PAE_SHARED_ACC_ADDR (PAE_SHARED_MSG_TYPE+4)
#define PAE_SHARED_ACC_LEN (PAE_SHARED_ACC_ADDR+4)
#define PAE_SHARED_BUF (PAE_SHARED_ACC_LEN+4)
#define PAE_SHARED_BUF_LEN 64

#define ENTRY_SIZE 30

#define PAE_LUE_KEY_DATA_ADDR     (PAE_MEM_BASE(LUE) + 0x8000)
#define PAE_LUE_LU_CMD_ADDR       (PAE_MEM_BASE(LUE) + 0x8040)
#define PAE_LUE_LU_RESULT_ADDR    (PAE_MEM_BASE(LUE) + 0x8048)
#define PAE_LUE_LU_DATA_ADDR      (PAE_MEM_BASE(LUE) + 0x8050)

/* bit field definitions of the LUE lookup command register */
#define PAE_REG_FLD_LUE_LU_CMD_DB_NUM_LO        0
#define PAE_REG_FLD_LUE_LU_CMD_DB_NUM_HI        2
#define PAE_REG_FLD_LUE_LU_CMD_SEARCH_LO        3
#define PAE_REG_FLD_LUE_LU_CMD_SEARCH_HI        3

/* bit field definitions of the LUE lookup result register */
#define PAE_REG_FLD_LUE_LU_RESULT_DONE_LO       0
#define PAE_REG_FLD_LUE_LU_RESULT_DONE_HI       0
#define PAE_REG_FLD_LUE_LU_RESULT_MATCH_LO      1
#define PAE_REG_FLD_LUE_LU_RESULT_MATCH_HI      1
#define PAE_REG_FLD_LUE_LU_RESULT_LENGTH_LO     4
#define PAE_REG_FLD_LUE_LU_RESULT_LENGTH_HI     12
#define PAE_REG_FLD_LUE_LU_RESULT_INDEX_LO      16
#define PAE_REG_FLD_LUE_LU_RESULT_INDEX_HI      24 

uint32 bitmap_idx[PAE_LUE_NUM_DBS];
uint32 bitmap[PAE_LUE_NUM_RULES / 32];

static void 
_pae_write_mem32(uint32 addr,uint32 index,uint32 value )
{
#if !defined(__KERNEL__)
    void * map_base;
    void *virt_addr;

#if defined(LINUX)
    int32 dev_mem_fd=open("/dev/mem",O_RDWR|O_SYNC, 2);
#else
    int32 dev_mem_fd=sal_open("/dev/mem",O_RDWR|O_SYNC, 2);
#endif
    if(dev_mem_fd<0)
        return;
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_mem_fd, addr & ~MAP_MASK);

    if(map_base == (void *) -1) {
        close(dev_mem_fd);
        return;
    }
    
    virt_addr = map_base + (addr & MAP_MASK);
    *(((uint32 *) virt_addr)+index) = value;
    munmap(map_base, MAP_SIZE);
    close(dev_mem_fd);
#else
    *(((uint32 *) addr) + index) = value;
#endif

    return;
}

static uint32 
_pae_read_mem32(uint32 addr,uint32 index )
{
    uint32 value;

#if !defined(__KERNEL__)
    void * map_base;
    void *virt_addr;

#if defined(LINUX)
    int32 dev_mem_fd=open("/dev/mem",O_RDWR|O_SYNC, 2);
#else
    int32 dev_mem_fd=sal_open("/dev/mem",O_RDWR|O_SYNC, 2);
#endif
    if(dev_mem_fd<0)
        return 0xFFFFFFFF;
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_mem_fd, addr & ~MAP_MASK);

    if(map_base == (void *) -1) {
        close(dev_mem_fd);
        return 0xFFFFFFFF;
    }

    virt_addr = map_base + (addr & MAP_MASK);
    value=*(((uint32 *) virt_addr)+index) ;
    munmap(map_base, MAP_SIZE);
    close(dev_mem_fd);
#else
    value=*(((uint32 *) addr) + index) ;
#endif

    return value;
}

static void 
_pae_write_mem64(uint32 addr,uint32 index,uint64 value )
{
#if !defined(__KERNEL__)
    void * map_base;
    void *virt_addr;

#if defined(LINUX)
    int32 dev_mem_fd=open("/dev/mem",O_RDWR|O_SYNC, 2);
#else
    int32 dev_mem_fd=sal_open("/dev/mem",O_RDWR|O_SYNC, 2);
#endif
    if(dev_mem_fd<0)
        return;
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_mem_fd, addr & ~MAP_MASK);

    if(map_base == (void *) -1) {
        close(dev_mem_fd);
        return;
    }
    
    virt_addr = map_base + (addr & MAP_MASK);
    *((uint64 *) virt_addr+index) = value;
    munmap(map_base, MAP_SIZE);
    close(dev_mem_fd);
#else
    *((uint64 *) addr + index) = value;
#endif

    return;
}

static uint64 
_pae_read_mem64(uint32 addr,uint32 index )
{
    uint64 value;

#if !defined(__KERNEL__)
    void * map_base;
    void *virt_addr;

#if defined(LINUX)
    int32 dev_mem_fd=open("/dev/mem",O_RDWR|O_SYNC, 2);
#else
    int32 dev_mem_fd=sal_open("/dev/mem",O_RDWR|O_SYNC, 2);
#endif
    if(dev_mem_fd<0){
        return COMPILER_64_INIT(0xFFFFFFFF, 0xFFFFFFFF);
    }
    
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_mem_fd, addr & ~MAP_MASK);

    if(map_base == (void *) -1) {
        close(dev_mem_fd);
        return COMPILER_64_INIT(0xFFFFFFFF, 0xFFFFFFFF);
    }
    virt_addr = map_base + (addr & MAP_MASK);
    value=*(((uint64 *) virt_addr)+index) ;
    munmap(map_base, MAP_SIZE);
    close(dev_mem_fd);
#else
    value=*(((uint64 *) addr)+index) ;
#endif

    return value;
}

static void 
_pae_byte_copy(uint32 addr, uint8 * buf,uint32 buf_len)
{
    uint32 i;

#if !defined(__KERNEL__)
    void * map_base;
    void *virt_addr;

#if defined(LINUX)
    int32 dev_mem_fd=open("/dev/mem",O_RDWR|O_SYNC, 2);
#else
    int32 dev_mem_fd=sal_open("/dev/mem",O_RDWR|O_SYNC, 2);
#endif
    if(dev_mem_fd<0)
        return;
    
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_mem_fd, addr & ~MAP_MASK);

    if(map_base == (void *) -1){
        close(dev_mem_fd);
        return;
    }
    
    virt_addr = map_base + (addr & MAP_MASK);

    for(i=0;i<buf_len;i++)
        *(((uint8 *) virt_addr+i)) = *(buf+i);
    munmap(map_base, MAP_SIZE);
    close(dev_mem_fd);
#else
    for(i=0;i<buf_len;i++)
        *(((uint8 *) addr+i)) = *(buf+i);
#endif

    return;
}

static uint32 
pae_host_to_r5_32(uint32 n)
{
#ifdef R5_BIG_ENDIAN 
#ifdef HOST_LITTLE_ENDIAN
    return soc_htonl(n);
#endif
#else
    return n;
#endif
}

static uint32 
pae_r5_to_host_32(uint32 n)
{
#ifdef R5_BIG_ENDIAN 
#ifdef HOST_LITTLE_ENDIAN
    return soc_htonl(n);
#endif
#else
    return n;
#endif
}

static void
pae_lue_be_bitset(uint8 *cp,
            uint32 byte_base,
            uint32 bit_pos,
            uint8 bit_value)
{
    uint32 byte_offs, bit_offs;
    uint8 byte_mask, *byte_p;

    byte_offs = bit_pos / 8;
    bit_offs  = bit_pos % 8;
    byte_mask = 1 << bit_offs;

    byte_p = cp + byte_base - byte_offs;

    if (bit_value)
        *byte_p |= byte_mask;
    else
        *byte_p &= ~byte_mask;  
}

static void
pae_lue_be_shift_up(uint8 *cp, 
            uint32 byte_base,
            uint32 bit_pos)
{
    uint32 byte_offs, bit_offs, i;
    uint8 mask_hi, mask_lo, *last_p;

    byte_offs = bit_pos / 8;
    bit_offs  = bit_pos % 8;

    last_p = cp + byte_base - byte_offs;

    for (i = 0; (cp + i) != last_p; i++) {
        cp[i] <<= 1;
        cp[i] |= (cp[i+1] >> 7);
    }

    mask_hi = 0xff << bit_offs;
    mask_lo = ~mask_hi;

    *last_p = ((*last_p & mask_hi) << 1) | (*last_p & mask_lo);
}

static void
pae_lue_bmap32_set(uint32 *p, uint32 idx, uint8 value)
{
    uint32 mask, word, bit;

    word = idx / 32;
    bit  = idx % 32;
    mask = 0x1 << bit;

    if (value)
    p[word] |= mask;
    else
    p[word] &= ~mask;
}

static uint32
pae_lue_bmap32_get(uint32 *p, uint32 idx)
{
    uint32 mask, word, bit;

    word = idx / 32;
    bit  = idx % 32;
    mask = 0x1 << bit;

    return (p[word] & mask) >> bit;
}

static int
pae_lue_table_rd_wr(pae_lue_op_et op,
            uint32 db_num,
            uint32 idx,
            uint8 *pat_p,
            uint8 *pay_p,
            uint32 pat_bytes,
            uint32 pay_bytes)
{
    uint32 word, opcode, timeout, do_pat, do_pay, read, write, clr;
    int32 byte_idx, i;
    uint64 data;

    clr    = (op == PAE_LUE_OP_CLEAR) ? 1 : 0;
    do_pat = (clr | (pat_p != 0)) ? 1 : 0;
    do_pay = (clr | (pay_p != 0)) ? 1 : 0;

    read  = 0;
    write = 0;

    switch (op) {
        case PAE_LUE_OP_READ      : opcode = PAE_LUE_OPC_READ; read = 1;       break;
        case PAE_LUE_OP_WRITE     : opcode = PAE_LUE_OPC_WRITE; write = 1;     break;
        case PAE_LUE_OP_READ_RAW  : opcode = PAE_LUE_OPC_RAW_READ; read = 1;   break;
        case PAE_LUE_OP_WRITE_RAW : opcode = PAE_LUE_OPC_RAW_WRITE; write = 1; break;
        case PAE_LUE_OP_CLEAR     : opcode = PAE_LUE_OPC_WRITE; write = 1;     break;
        default                   : return SOC_E_PARAM;
    }

    if (write) {
        if (do_pat) {
            byte_idx = pat_bytes - 1;
            for (word = 0; word < PAE_LUE_RULE_NUM_WORDS; word++) {
                data = 0;
                if (!clr) {/*not clear operation */
                    for (i = 0; i < 8; i++) {
                        if (byte_idx < 0)
                            break;
                        data |= (((uint64)pat_p[byte_idx--]) << (i * 8));
                    }
                }
                PAE_HAL_WRITE_IDX_64(PAE_LUE_RULE_PATTERN_ADDR, word, data);
            }
        }
        
        if (do_pay) {
            byte_idx = 0;
            for (word = 0; word < PAE_LUE_META_NUM_WORDS; word++) {
                data = 0;
                if (!clr) {
                    for (i = 0; i < 8; i++) {
                        if (byte_idx >= pay_bytes)
                            break;
                        data |= (((uint64)pay_p[byte_idx++]) << (i * 8));
                    }
                }
                PAE_HAL_WRITE_IDX_64(PAE_LUE_RULE_PAYLOAD_ADDR, word, data);
            }
        }
    }

    data = 0;
    PAE_HAL_REG_FLD_SET_64(data, LUE_TBL_CTRL_TARGET_RULE, do_pat);
    PAE_HAL_REG_FLD_SET_64(data, LUE_TBL_CTRL_TARGET_META, do_pay);
    PAE_HAL_REG_FLD_SET_64(data, LUE_TBL_CTRL_OPCODE, opcode);
    PAE_HAL_REG_FLD_SET_64(data, LUE_TBL_CTRL_DONE, 0);
    PAE_HAL_REG_FLD_SET_64(data, LUE_TBL_CTRL_DB_NUM, db_num);
    PAE_HAL_REG_FLD_SET_64(data, LUE_TBL_CTRL_RULE_ADDR, idx);
    PAE_HAL_WRITE_64(PAE_LUE_RULE_CMD_ADDR, data);

    if (read | PAE_LUE_WAIT_WR_COMPLETE) {
        timeout = PAE_LUE_READ_TIMEOUT;
        while (--timeout) {
            data = PAE_HAL_READ_64(PAE_LUE_RULE_CMD_ADDR);
            if (PAE_HAL_REG_FLD_GET_64(data, LUE_TBL_CTRL_DONE)){
                break;
            }
        }
        if (!timeout){
            return SOC_E_TIMEOUT;
        }
    }

    if (read) {
        if (do_pat) {
            byte_idx = pat_bytes - 1;

            for (word = 0; word < PAE_LUE_RULE_NUM_WORDS; word++) {
                data = PAE_HAL_READ_IDX_64(PAE_LUE_RULE_PATTERN_ADDR, word);
                for (i = 0; i < 8; i++) {
                    if (byte_idx < 0)
                        break;
                    pat_p[byte_idx--] = data >> (i * 8);
                }
                if (byte_idx < 0)
                    break;
            }
        }

        if (do_pay) {
            byte_idx = 0;
            for (word = 0; word < PAE_LUE_META_NUM_WORDS; word++) {
                data = PAE_HAL_READ_IDX_64(PAE_LUE_RULE_PAYLOAD_ADDR, word);
                    for (i = 0; i < 8; i++) {
                        if (byte_idx >= pay_bytes)
                            break;
                        pat_p[byte_idx++] = data >> (i * 8);
                    }
                if (byte_idx >= pay_bytes)
                    break;
            }
        }
    }

    return SOC_E_NONE;
}

int
soc_pae_lue_idx_alloc(
            int    unit,
            uint32 db_num, 
            uint32 *idx)
{
    uint32 start, min, max, i;

    min = PAE_RULE_BASE * PAE_LUE_NUM_IN_RULESET;
    max = min + PAE_RULE_SET * PAE_LUE_NUM_IN_RULESET;  
    start = bitmap_idx[db_num];

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return SOC_E_UNAVAIL;
    }

    if ((start < min) || (start > max)){
        return SOC_E_RESOURCE;
    }
    
    for (i = start; i < max; i++) {
        if (!pae_lue_bmap32_get(bitmap, i)) {
            pae_lue_bmap32_set(bitmap, i, 1);
            bitmap_idx[db_num] = i;
            *idx = i - min;
            return SOC_E_NONE;
        }
    }

    for (i = min; i < start; i++) {
        if (!pae_lue_bmap32_get(bitmap, i)) {
            pae_lue_bmap32_set(bitmap, i, 1);
            bitmap_idx[db_num] = i;
            *idx = i - min;
            return SOC_E_NONE;
        }
    }

    return SOC_E_RESOURCE;
}

int
soc_pae_lue_idx_free(
            int    unit,
            uint32 db_num, 
            uint32 idx)
{
    uint32 min, max;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return SOC_E_UNAVAIL;
    }

    if (db_num > 7){
        return SOC_E_PARAM;
    }

    min = PAE_RULE_BASE * PAE_LUE_NUM_IN_RULESET;
    max = min + PAE_RULE_SET * PAE_LUE_NUM_IN_RULESET;  

    if (idx > (max - min)){
        return SOC_E_PARAM;
    }
    
    pae_lue_bmap32_set(bitmap, min + idx, 0);

    return SOC_E_NONE;
}

int
soc_pae_lue_db_config(
            int    unit,
            uint32 db_num,
            uint32 enabled,
            uint32 rule_sets,
            uint32 rule_base,
            uint32 meta_base,
            uint32 key_size_encoded,
            uint32 payload_size)
{
    uint32 data = 0;

    if(!SOC_IS_NORTHSTARPLUS(unit)){
        return SOC_E_UNAVAIL;
    }

    if (db_num !=0)/*only one database is supported,db 0*/
        return SOC_E_PARAM;
    PAE_HAL_REG_FLD_SET_32(data, LUE_DATABASE_CONFIG_BASE_ADDR, rule_base);
    PAE_HAL_REG_FLD_SET_32(data, LUE_DATABASE_CONFIG_SEARCH_TYPE, key_size_encoded);
    PAE_HAL_REG_FLD_SET_32(data, LUE_DATABASE_CONFIG_RULE_SETS, rule_sets);
    PAE_HAL_REG_FLD_SET_32(data, LUE_DATABASE_CONFIG_ENABLED, enabled);
    PAE_HAL_REG_FLD_SET_32(data, LUE_DATABASE_CONFIG_META_SETS, meta_base);
    PAE_HAL_WRITE_IDX_32(PAE_LUE_DATABASE_CONFIG_ADDR, db_num, data);
    return SOC_E_NONE;
}

int
soc_pae_lue_rule_add(
            int    unit,
            uint32 db_num,
            uint8 *pat_p,
            uint32 pat_nbits,
            uint8 *pay_p,
            uint32 rule_idx)
{
    uint32 term_bit, key_bytes, enc_bytes, pad_byte, pay_bytes, bits_1st_byte;
    uint32 pat_bytes, align_byte;
    uint8  pattern[64];
    int    rc;

    if(!SOC_IS_NORTHSTARPLUS(unit)){
        return SOC_E_UNAVAIL;
    }

    if (db_num !=0)/*only one database is supported,db 0*/
      return SOC_E_PARAM;

/*
    rc = soc_pae_lue_idx_alloc(db_num, &idx);
    if (rc != SOC_E_NONE)
      return rc; */

    /* bytes required to represent key, encoded key, padding?, payload */
    key_bytes = PAE_DIVUP(PAE_KEY_SIZE, 8);
    enc_bytes = PAE_DIVUP(PAE_KEY_SIZE + 2, 8);
    pad_byte  = enc_bytes - key_bytes;
    pay_bytes = PAE_DIVUP(PAE_PAYLOAD_SIZE, 8);

    /* how many bits are in first byte, and size of pattern */
    bits_1st_byte = PAE_KEY_SIZE - (key_bytes - 1) * 8;
    pat_bytes	  = 1 + PAE_DIVUP(pat_nbits - bits_1st_byte, 8);

    /* byte-alignment for big-endian packing, determine term bit */
    align_byte = enc_bytes - 1;
    term_bit   = PAE_KEY_SIZE - pat_nbits + 1;

    /* copy pattern, possibly pad front with extra byte for shift-up */
    sal_memset(pattern, 0, sizeof(pattern));
    sal_memcpy(pattern + pad_byte, pat_p, pat_bytes);

    /* shift entire pattern up, put valid bit at LSB (bit 0) */
    pae_lue_be_shift_up(pattern, align_byte, 0);
    pae_lue_be_bitset(pattern, align_byte, 0, 1);

    /* shift pattern up from term, put in the term delimiter bit */
    pae_lue_be_shift_up(pattern, align_byte, term_bit);
    pae_lue_be_bitset(pattern, align_byte, term_bit, 1);

    rc = pae_lue_table_rd_wr(PAE_LUE_OP_WRITE,
                            db_num, 
                            rule_idx, 
                            pattern,
                            pay_p,
                            key_bytes,
                            pay_bytes);

    if (rc != SOC_E_NONE) {
        soc_pae_lue_idx_free(unit, db_num, rule_idx);
        return rc;
    }
    return SOC_E_NONE;
}

int
soc_pae_lue_rule_del( 
            int    unit,
            uint32 db_num, 
            uint32 rule_idx)
{
    int rc;

    if(!SOC_IS_NORTHSTARPLUS(unit)){
        return SOC_E_UNAVAIL;
    }

    if (db_num !=0){/*only one database is supported,db 0*/
        return SOC_E_PARAM;
    }

    rc = pae_lue_table_rd_wr(PAE_LUE_OP_CLEAR,
        db_num, 
        rule_idx, 
        0, /* unused */
        0, /* unused */
        0, /* unused */
        0  /* unused */);

    if (rc != SOC_E_NONE)
        return rc;

    return soc_pae_lue_idx_free(unit, db_num, rule_idx);
}

pae_response_code 
soc_pae_get_cmd_response(int unit)
{
    if(!SOC_IS_NORTHSTARPLUS(unit)){
        return PAE_RES_INVALID;
    }

    return pae_r5_to_host_32(PAE_HAL_READ_32(PAE_SHARED_MSG_RES_CODE));
}

int 
soc_pae_set_cmd_response(int unit, pae_response_code response_code)
{
    uint32 r5_res_code;

    if(!SOC_IS_NORTHSTARPLUS(unit)){
        return SOC_E_UNAVAIL;
    }

    r5_res_code=pae_host_to_r5_32(response_code);
    PAE_HAL_WRITE_32(PAE_SHARED_MSG_RES_CODE,r5_res_code);

    return SOC_E_NONE;
}

int 
soc_pae_send_msg(int unit, pae_msg_type msg_type, uint32 addr, uint32 len, void * buf)
{
    uint32 r5_len,r5_msg_type,r5_addr;
    pae_response_code res_code=PAE_RES_FAILURE;
    uint32 loop_i;

    if(!SOC_IS_NORTHSTARPLUS(unit)){
        return SOC_E_UNAVAIL;
    }

    soc_pae_set_cmd_response(unit, PAE_RES_INVALID);
    r5_len=pae_host_to_r5_32(len);
    r5_msg_type=pae_host_to_r5_32(msg_type);
    r5_addr=pae_host_to_r5_32(addr);

    switch(msg_type){
        case PAE_WRITE_BUF:
            _pae_byte_copy(PAE_SHARED_BUF,buf, len);
        break;

        default:
        return SOC_E_PARAM;
    }

    PAE_HAL_WRITE_32(PAE_SHARED_ACC_LEN,r5_len);
    PAE_HAL_WRITE_32(PAE_SHARED_ACC_ADDR,r5_addr);	
    PAE_HAL_WRITE_32(PAE_SHARED_MSG_TYPE,r5_msg_type);

    for(loop_i=0;loop_i<PAE_RES_WAIT_MS;loop_i++){
        sal_usleep(1000);
        res_code=soc_pae_get_cmd_response(unit);
        if(res_code==PAE_RES_OK)
        break;
    }
    
    if(res_code==PAE_RES_OK)
        return SOC_E_NONE;
    else
        return SOC_E_FAIL;
}

uint32 
soc_pae_get_sratch_pad_entry_addr(int unit, uint32 idx)
{
    if(!SOC_IS_NORTHSTARPLUS(unit)){
        return SOC_E_UNAVAIL;
    }

    return (PAE_SCRATCH_PAD_START_ADDR+(ENTRY_SIZE*idx));
}

/*******************************LOOK UP for TEST ************************/
int
soc_pae_lue_lookup(uint32 db_num,
                   uint8  *key_p, 
                   uint32 *match_res_p,
                   uint32 *length_res_p,
                   uint32 *index_res_p,
                   uint8  *data_res_p)
{
    uint32 word, key_bytes, pay_bytes, timeout;
    int    byte_idx, i;
    uint64 data;

    if (db_num !=0)
        return SOC_E_PARAM;

    key_bytes = PAE_DIVUP(PAE_KEY_SIZE, 8);
    pay_bytes = PAE_DIVUP(8*8, 8);

    byte_idx = key_bytes - 1;
    for (word = 0; word < PAE_LUE_RULE_NUM_WORDS; word++) {
        data = 0;
        for (i = 0; i < 8; i++) {
            if (byte_idx < 0)
                break;
            data |= (((uint64)key_p[byte_idx--]) << (i * 8));
        }
        PAE_HAL_WRITE_IDX_64(PAE_LUE_KEY_DATA_ADDR, word, data);
    }

    data = 0;
    PAE_HAL_REG_FLD_SET_64(data, LUE_LU_CMD_DB_NUM, db_num);
    PAE_HAL_REG_FLD_SET_64(data, LUE_LU_CMD_SEARCH, 1);
    PAE_HAL_WRITE_64(PAE_LUE_LU_CMD_ADDR, data);

    timeout = PAE_LUE_READ_TIMEOUT;
    while (--timeout) {
        data = PAE_HAL_READ_64(PAE_LUE_LU_RESULT_ADDR);
        if (PAE_HAL_REG_FLD_GET_64(data, LUE_LU_RESULT_DONE))
            break;
    }

    if (!timeout)
        return SOC_E_TIMEOUT;

    *match_res_p = PAE_HAL_REG_FLD_GET_64(data, LUE_LU_RESULT_MATCH);

    if (!*match_res_p)
        return SOC_E_NOT_FOUND;

    *length_res_p = PAE_HAL_REG_FLD_GET_64(data, LUE_LU_RESULT_LENGTH);
    *index_res_p  = PAE_HAL_REG_FLD_GET_64(data, LUE_LU_RESULT_INDEX);

    if (data_res_p) {
        byte_idx = 0;

        for (word = 0; word < PAE_LUE_META_NUM_WORDS; word++) {
            data = PAE_HAL_READ_IDX_64(PAE_LUE_LU_DATA_ADDR, word);
            for (i = 0; i < 8; i++) {
                if (byte_idx >= pay_bytes)
                    break;
                data_res_p[byte_idx++] = data >> (i * 8);
            }

            if (byte_idx >= pay_bytes)
                break;
        }
    }    

    return SOC_E_NONE;
}

#endif
#endif
#endif



