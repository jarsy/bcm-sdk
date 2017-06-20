/* $Id: multicast_imp.h,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __DPP_MULTICAST_IMP_H__
#define __DPP_MULTICAST_IMP_H__

/*
 * This file contains joint multicast implementation between Arad and Jericho.
 * It "inherits" from include/soc/dpp/multicast.h .
 */

#include <soc/dpp/multicast.h>
#include <soc/dpp/TMC/tmc_api_general.h>
#include <soc/types.h>
#include <soc/dpp/dpp_config_defs.h>
/*#include <soc/dpp/drv.h>*/

/*************
 * GLOBALS   *
 *************/


/*************
 *  MACROS   *
 *************/
/* { */



/* allowed types of an entry */
#define DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START 0  /* free entry at the start of a free block, this value is used when reading mcdb from hardware for all entries */
#define DPP_MCDS_TYPE_VALUE_FREE_BLOCK       1  /* free entry not at the start of a free block */
#define DPP_MCDS_TYPE_VALUE_INGRESS_START    2  /* a used ingress entry, at the start of the group */
#define DPP_MCDS_TYPE_VALUE_INGRESS          3  /* a used ingress entry, not at the start of the group */
#define DPP_MCDS_TYPE_VALUE_EGRESS_START     4  /* The start of a non TDM egress group */
#define DPP_MCDS_TYPE_VALUE_EGRESS           5  /* a used egress entry, non TDM, not at the start of the group */
#define DPP_MCDS_TYPE_VALUE_EGRESS_TDM_START 6  /* The start of a TDM egress group, not used in Jericho */
#define DPP_MCDS_TYPE_VALUE_EGRESS_TDM       7  /* a used egress TDM entry, not at the start of the group, not used in Jericho */

/* check traits of a given type */
#define DPP_MCDS_TYPE_IS_FREE(type) (!((type) & 6))                  /* is the entry free? */
#define DPP_MCDS_TYPE_IS_USED(type) ((type) & 6)                     /* is the entry used? */
#define DPP_MCDS_TYPE_IS_INGRESS(type) (((type) & 6) == 2)           /* is the entry used for ingress multicast */
#define DPP_MCDS_TYPE_IS_EGRESS(type) ((type) & 4)                   /* is the entry used for egress multicast */
#define DPP_MCDS_TYPE_IS_EGRESS_NORMAL(type) (((type) & 6) == 4)     /* is the entry used for non TDM egress multicast */
#define DPP_MCDS_TYPE_IS_NORMAL(type) (((type) ^ ((type) >> 1)) & 2) /* is the entry used for non TDM egress multicast or ingress multicast */
#define DPP_MCDS_TYPE_IS_TDM(type) (((type) & 6) == 6)               /* is the entry used for TDM egress multicast */
#define DPP_MCDS_TYPE_IS_EGRESS_START(type) (((type) & 5) == 4)      /* is the entry the first one of an egress multicast group */
#define DPP_MCDS_TYPE_IS_EGRESS_NOT_START(type) (((type) & 5) == 5)  /* is the entry the none first one of an egress multicast group */
#define DPP_MCDS_TYPE_IS_START(type) (((type) & 1) == 0)             /* is the entry the none first one of an egress multicast group */
#define DPP_MCDS_TYPE_IS_INGRESS_OR_TDM(type) ((type) & 2)           /* is the entry used for TDM egress multicast */
#define DPP_MCDS_TYPES_ARE_THE_SAME(t1, t2) ((((t1)^(t2)) & 6) == 0) /* checks if the two types are the same disregarding start information */
#define DPP_MCDS_TYPE_GET_START(type) ((type) & 6)                   /* returns the same type for the first entry of a group/free block */
#define DPP_MCDS_TYPE_GET_NONE_START(type) ((type) | 1)              /* returns the same type for a none first entry of a group/free block */

/*
 *free entries blocks handling
 */

/* maximum size in entries of a block of consecutive free entries, in a region where MCIDs may be automatically allocated */
#define DPP_MCDS_MAX_FREE_BLOCK_SIZE_ALLOCED 8
/* maximum size in entries of a block of consecutive free entries, in a region where MCIDs may not be automatically allocated */
#define DPP_MCDS_MAX_FREE_BLOCK_SIZE_GENERAL 8
/* maximum size in entries of a block of consecutive free entries, in any region  (max of the two macros above) */
#define DPP_MCDS_MAX_FREE_BLOCK_SIZE 8


/* flags for get_free_entries_block() */
#define DPP_MCDS_GET_FREE_BLOCKS_PREFER_SMALL   1  /* Will prefer blocks of the given size or smaller, the default is the given size or bigger. */
#define DPP_MCDS_GET_FREE_BLOCKS_PREFER_SIZE    2  /* Will prefer to return a block of a better size than in a better region. The default is opposite. */
#define DPP_MCDS_GET_FREE_BLOCKS_NO_UPDATES     4  /* Will not change the returned block: will not resize it if a smaller part of it is needed, */
                                                         /* and will not remove it from its free blocks list. */
                                                         /* The caller must handle all of this before another call is made. */
#define DPP_MCDS_GET_FREE_BLOCKS_PREFER_INGRESS 8  /* Will prefer the ingress regions over the egress regions */
#define DPP_MCDS_GET_FREE_BLOCKS_DONT_FAIL      16 /* When failing to allocate, instead of the function failing, return a block size & start of 0 */

#define ARAD_MCDS_NOF_REGIONS 3 /* number of regions (in MCDS) */



/* encoding of additional memory in MCDB entries in memory - joint encoding used by Arad and Jericho */

/* The bits of word1 of a arad_mcdb_entry_t are used for:
 * 0-18: Arad hardware entry data.
 * 0-23: Jericho hardware entry data.
 * 19/24-24: Reserved for later usage.
 * 25-25: A bit used only by testing code.
 * 26-26: For Arad egress bitmap groups (0 till at most 8191) store if the group is open.
 * 27-28: For used entries: The msb bits of the previous entry of the group. The 16 lsb bits are in the prev_entries table.
 * 29-31: Entry type. For egress non bitmap groups, need to restore after warmboot if the group was open or not (1 bit).
 *
 * For free entries we do not need to keep the (implied) hardware value of the entry, and we use these bits:
 * word1 bits  0-18: For the first entry of a free block this holds the first entry of the next free block in the list.
 * word0 bits  0-18: For the first entry of a free block this holds the first entry of the previous free block in the list.
 *                   For non first entries this holds the first entry in the free block.
 * word0 bits 19-22: For the first entry of a free block this holds the size of the free block
 *
 */

#define DPP_MC_ENTRY_SIZE 2                /* size of a MCDB entry in uint32s which must be equal to the size of arad_mcdb_entry_t */
/* mask of the hardware data bits in the msb word of an mcdb entry */
#define ARAD_MC_ENTRY_MASK_VAL 0x7ffff
#define JER_MC_ENTRY_MASK_VAL 0xffffff

#define ARAD_MC_ENTRY_MASK_WORD(mcds, index) ((mcds)->mcdb[index].word1) /* The last word of MCDB in which to apply the mask and store data in free bits*/

#define DPP_MCDS_TYPE_SHIFT 29 /* stored in the 3 msb bits */
#define DPP_MAX_NOF_MCDB_INDEX_BITS 18 /* maximum (between device types) of bits in an MCDB table index */
#define DPP_NOF_REMAINDER_PREV_BITS (DPP_MAX_NOF_MCDB_INDEX_BITS - 16) /* number of previous entry (msb) bits stored in MCDS's MCDB entry copies */
#define DPP_NOF_REMAINDER_PREV_MASK ((1 << DPP_NOF_REMAINDER_PREV_BITS) - 1)
#define DPP_MCDS_MSB_PREV_BIT_SHIFT (DPP_MCDS_TYPE_SHIFT - DPP_NOF_REMAINDER_PREV_BITS) /* bits not fitting in prev_entries */
#define DPP_MCDS_BITMAP_OPEN_SHIFT (DPP_MCDS_MSB_PREV_BIT_SHIFT - 1) /* bit previously used for marking if the bitmap groups (egress 0-8191) were open */
#define DPP_MCDS_TEST_BIT_SHIFT (DPP_MCDS_BITMAP_OPEN_SHIFT -1) /* bit saying if the bitmap group (egress 0-8191) is open */


#define DPP_MCDS_WORD1_KEEP_BITS_MASK /* The bits to keep when copying/resetting a MCDB entry */ \
  ((1 << DPP_MCDS_BITMAP_OPEN_SHIFT) | (1 << DPP_MCDS_TEST_BIT_SHIFT))


#define DPP_MCDS_TYPE_MASK 7   /* 3 bits type */
/* get and set the entry type */
#define DPP_MCDS_ENTRY_GET_TYPE(entry) ((entry)->word1 >> DPP_MCDS_TYPE_SHIFT) /* assumes the usage of the msb bits */
#define DPP_MCDS_ENTRY_SET_TYPE(entry, type_value) /* assumes the usage of the msb bits */ \
    do {(entry)->word1 = ((entry)->word1 & ~(DPP_MCDS_TYPE_MASK << DPP_MCDS_TYPE_SHIFT)) | \
      ((type_value) << DPP_MCDS_TYPE_SHIFT); } while (0)
#define DPP_MCDS_GET_TYPE(mcds, index) DPP_MCDS_ENTRY_GET_TYPE((mcds)->mcdb + (index)) /* assumes the usage of the msb bits */
#define DPP_MCDS_SET_TYPE(mcds, index, type_value) DPP_MCDS_ENTRY_SET_TYPE((mcds)->mcdb + (index), (type_value)) /* assumes the usage of the msb bits */


#define DPP_MCDS_MSB_PREV_BIT_MASK (DPP_NOF_REMAINDER_PREV_MASK << 16) /* mask of the bit in the entry index */
/* get and set the previous entry index (the current entry index means no previous entry) */
#define DPP_MCDS_GET_PREV_ENTRY(mcds, index) \
  (((ARAD_MC_ENTRY_MASK_WORD((mcds), (index)) >> (DPP_MCDS_MSB_PREV_BIT_SHIFT - 16)) & \
    DPP_MCDS_MSB_PREV_BIT_MASK) | (mcds)->prev_entries[index])

#define DPP_MCDS_SET_PREV_ENTRY(mcds, index, prev_entry) \
  do { \
    ARAD_MC_ENTRY_MASK_WORD((mcds), (index)) = (ARAD_MC_ENTRY_MASK_WORD((mcds), (index)) & \
      ~(DPP_NOF_REMAINDER_PREV_MASK << DPP_MCDS_MSB_PREV_BIT_SHIFT)) | \
      (((prev_entry) & DPP_MCDS_MSB_PREV_BIT_MASK) << (DPP_MCDS_MSB_PREV_BIT_SHIFT - 16)); \
    (mcds)->prev_entries[index] = (prev_entry) & 0xffff; \
  } while(0);

#define DPP_MCDS_ENTRY_GET_PREV_ENTRY(entry, mcds, index) \
  ((((entry)->word1 >> (DPP_MCDS_MSB_PREV_BIT_SHIFT - 16)) & \
    DPP_MCDS_MSB_PREV_BIT_MASK) | (mcds)->prev_entries[index])
#define DPP_MCDS_ENTRY_SET_PREV_ENTRY(entry, mcds, index, prev_entry) \
  do { \
    (entry)->word1 = ((entry)->word1 & \
      ~(1 << DPP_MCDS_MSB_PREV_BIT_SHIFT)) | \
      (((prev_entry) & DPP_MCDS_MSB_PREV_BIT_MASK) << (DPP_MCDS_MSB_PREV_BIT_SHIFT - 16)); \
    (mcds)->prev_entries[index] = (prev_entry) & 0xffff; \
  } while(0);

#define DPP_MCDS_FREE_NEXT_PREV_MASK 0x3ffff
#define DPP_MCDS_FREE_BLOCK_SIZE_MASK 0xf /* 4 bits for storing block size */
#define DPP_MCDS_FREE_BLOCK_SIZE_SHIFT 19
#define DPP_MCDS_GET_FREE_NEXT_ENTRY(mcds, index) /* get the next free block start */ \
  (ARAD_MC_ENTRY_MASK_WORD(mcds, index) & DPP_MCDS_FREE_NEXT_PREV_MASK)
#define DPP_MCDS_SET_FREE_NEXT_ENTRY(mcds, index, next_entry) /* set the next free block start */ \
  ARAD_MC_ENTRY_MASK_WORD(mcds, index) = (ARAD_MC_ENTRY_MASK_WORD(mcds, index) & ~DPP_MCDS_FREE_NEXT_PREV_MASK) | (next_entry)
#define DPP_MCDS_ENTRY_GET_FREE_PREV_ENTRY(entry) /* get the prev free block start or start of this block */ \
  ((entry)->word0 & DPP_MCDS_FREE_NEXT_PREV_MASK)
#define DPP_MCDS_GET_FREE_PREV_ENTRY(mcds, index) /* get the prev free block start or start of this block */ \
  ((mcds)->mcdb[index].word0 & DPP_MCDS_FREE_NEXT_PREV_MASK)
#define DPP_MCDS_SET_FREE_PREV_ENTRY(mcds, index, prev_entry) /* set the prev free block start or start of this block */ \
  (mcds)->mcdb[index].word0 = ((mcds)->mcdb[index].word0 & ~DPP_MCDS_FREE_NEXT_PREV_MASK) | (prev_entry)
#define DPP_MCDS_GET_FREE_BLOCK_SIZE(mcds, index) /* get the block size, to be called for the first block entry */ \
  (((mcds)->mcdb[index].word0 >> DPP_MCDS_FREE_BLOCK_SIZE_SHIFT) & DPP_MCDS_FREE_BLOCK_SIZE_MASK)
#define DPP_MCDS_SET_FREE_BLOCK_SIZE(mcds, index, size) /* set the block size, to be called for the first block entry */ \
  (mcds)->mcdb[index].word0 = ((mcds)->mcdb[index].word0 & ~(DPP_MCDS_FREE_NEXT_PREV_MASK << DPP_MCDS_FREE_BLOCK_SIZE_SHIFT)) | \
  ((size) << DPP_MCDS_FREE_BLOCK_SIZE_SHIFT)

/* Macros handling the storing of if an egress bitmap is open or not */
#define DPP_MCDS_ENTRY_IS_BITMAP_OPEN(   entry)    (((entry)->word1 >> DPP_MCDS_BITMAP_OPEN_SHIFT) & 1) /* returns 1 if open, 0 of closed */
#define DPP_MCDS_ENTRY_SET_BITMAP_OPEN(  entry) do {(entry)->word1 |=  (1 << DPP_MCDS_BITMAP_OPEN_SHIFT);} while (0) /* marks the bitmap group as open */
#define DPP_MCDS_ENTRY_SET_BITMAP_CLOSED(entry) do {(entry)->word1 &= ~(1 << DPP_MCDS_BITMAP_OPEN_SHIFT);} while (0) /* marks the bitmap group as closed */

/* Macros for setting/getting the test bit */
#define DPP_MCDS_ENTRY_GET_TEST_BIT(     entry)    (((entry)->word1 >> DPP_MCDS_TEST_BIT_SHIFT) & 1)
#define DPP_MCDS_ENTRY_SET_TEST_BIT_ON(  entry) do {(entry)->word1 |=  (1 << DPP_MCDS_TEST_BIT_SHIFT);} while (0)
#define DPP_MCDS_ENTRY_SET_TEST_BIT_OFF( entry) do {(entry)->word1 &= ~(1 << DPP_MCDS_TEST_BIT_SHIFT);} while (0)
#define DPP_MCDS_ENTRY_SET_TEST_BIT(entry, val) do {(entry)->word1 &= ~(1 << DPP_MCDS_TEST_BIT_SHIFT); (entry)->word1 |= (((val) & 1) << DPP_MCDS_TEST_BIT_SHIFT);} while (0)


/* handling of egress hardware format */
#define DPP_MCDS_GET_EGRESS_FORMAT(mcds, index) (SOC_IS_JERICHO(unit) ? JER_MCDS_GET_EGRESS_FORMAT(mcds, index): ARAD_MCDS_GET_EGRESS_FORMAT(mcds, index))
#define DPP_MCDS_IS_EGRESS_FORMAT_CONSECUTIVE(format) ((format) & 4)
#define DPP_MCDS_IS_EGRESS_FORMAT_CONSECUTIVE_END(format) (((format) & 5) == 4)
#define ARAD_MCDS_IS_EGRESS_FORMAT_CONSECUTIVE_NEXT(format) (((format) & 5) == 5)

#define ARAD_MCDS_GET_EGRESS_FORMAT(mcds, index) (((mcds)->mcdb[index].word1 & 0x70000) >> 16)
#define JER_MCDS_GET_EGRESS_FORMAT(mcds, index) (((mcds)->mcdb[index].word1 & 0xF00000) >> 20)

#define DPP_MULT_MAX_REPLICATIONS 4095 /* = max {DPP_MULT_MAX_INGRESS_REPLICATIONS, DPP_MULT_MAX_EGRESS_REPLICATIONS} */
#define DPP_MULT_MAX_INGRESS_REPLICATIONS DPP_MULT_MAX_REPLICATIONS /* maximum replications for an ingress multicast group */
#define DPP_MULT_MAX_EGRESS_REPLICATIONS DPP_MULT_MAX_REPLICATIONS  /* maximum replications for an egress multicast group */

/* The value used for occupied empty entries (empty MC groups) of different types */
#define DPP_MC_ING_DESTINATION_DISABLED 0x3ffff /* value of destination field that causes no replication */
#define DPP_MC_EGR_OUTLIF_DISABLED 0            /* value of outlif fields that causes no replication in formats 2,6,7 where a port is not specified */
#define DPP_MC_EGR_BITMAP_DISABLED 0            /* value of bitmap pointer field that causes no replication in format 1 */
#define DPP_MC_NOF_EGRESS_PORTS 256             /* number of PP ports that can be specified in egress MC */
#define DPP_MULT_EGRESS_PORT_INVALID (DPP_MC_NOF_EGRESS_PORTS-1) /* local port value marking invalid/disabled */
#define ARAD_MULT_EGRESS_SMALL_PORT_INVALID 127 /* local port value stored in a 7 bits field marking invalid/disabled */
#define DPP_MC_NOF_EGRESS_BITMAP_WORDS (SOC_TMC_NOF_FAP_PORTS_JERICHO/SOC_SAND_NOF_BITS_IN_UINT32) /* number uint32s in a bitmap */

#define DPP_MC_EGRESS_LINK_PTR_END 0
#define DPP_MC_INGRESS_1ST_ENTRY 0
#define DPP_MC_EGR_CUD_INVALID 0 /* This CUD represents no replication in CUD only replications */
#define IRDB_TABLE_ENTRY_WORDS 2

#define DPP_MC_FREE_ENTRIES_BLOCK_LIST_EMPTY ((uint32)(-1))

#define DPP_MC_EGR_BITMAP_ID_BITS 13  /* number of bits in an egress MC bitmap ID */
#define DPP_MC_EGR_NOF_BITMAPS (1 <<  DPP_MC_EGR_BITMAP_ID_BITS) /* number of egress bitmaps (IDs starting at 0) */
#define DPP_MC_EGR_MAX_BITMAP_ID (DPP_MC_EGR_NOF_BITMAPS - 1)

#define DPP_LAST_MCDB_ENTRY(mcds) MCDS_INGRESS_LINK_END(mcds)

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/*
 * free entries blocks handling
 */

typedef uint8 dpp_free_entries_block_size_t; /* the size of a free entries block size */

typedef struct { /* A doubly linked list of free blocks in the same region and of the same size. The list is circular */
  uint32 first; /* The id of the first member in the list. */
} dpp_free_entries_block_list_t;

typedef struct { /* free block lists of a region */
  dpp_free_entries_block_size_t max_size; /* the max size of block, we have block lists of size 1 to this size */
  uint32 range_start;
  uint32 range_end; /* range start and range end */
  dpp_free_entries_block_list_t lists[DPP_MCDS_MAX_FREE_BLOCK_SIZE]; /* the lists of sizes 1 .. max_size */
} dpp_free_entries_blocks_region_t;


/*
 * This structure contains one MCDB entry, and other data in the remaining bits.
 * For used MCDB entries the hardware data bits always match (cache) the hardware.
 * For unused MCDB entries who have a constant hardware value, the data bits are used for other software purposes
 */
typedef struct
{
  uint32 word0;
  uint32 word1;
} arad_mcdb_entry_t;

typedef struct {
    uint32 base;  /* data of the standard replication */
    uint32 extra;    /* extra data */
} dpp_rep_data_t; /* type containing the type and data of any replication */

/*
 * usage of the bits of dpp_rep_data_t: 
 * base 0-7    port for egress CUD_port replications
 * base 0-12   bitmap ID for egress CUD_port replications, lsb ingress destination
 * base 13-31  CUD for egress, ingress replications
 * extra 0-5   ingress destination msb
 * extra 10-12   replication type
 * extra 13-31 19b extra CUD for egress replications, the only CUD for ingress replications
 */

/* macros to access data inside dpp_rep_data_t */

#define DPP_MCDS_REP_DATA_PORT_NOF_BITS 8
#define DPP_MCDS_REP_DATA_CUD_NOF_BITS 19
#define DPP_MCDS_REP_DATA_BM_ID_NOF_BITS DPP_MC_EGR_BITMAP_ID_BITS
#define DPP_MCDS_REP_DATA_TYPE_NOF_BITS 3
#define DPP_MCDS_REP_DATA_TYPE_OFFSET 10
#define DPP_MCDS_REP_DATA_PORT_MASK ((1 << DPP_MCDS_REP_DATA_PORT_NOF_BITS) - 1)
#define DPP_MCDS_REP_DATA_BM_ID_MASK ((1 << DPP_MCDS_REP_DATA_BM_ID_NOF_BITS) - 1)
#define DPP_MCDS_REP_DATA_TYPE_MASK ((1 << DPP_MCDS_REP_DATA_TYPE_NOF_BITS) - 1)
#define DPP_MCDS_REP_DATA_CUD_MASK ((1 << DPP_MCDS_REP_DATA_CUD_NOF_BITS) - 1)
#define DPP_MCDS_REP_DATA_EGR_CUD_SHIFT 13
#define DPP_MCDS_REP_DATA_EXTRA_CUD_SHIFT 13
#define DPP_MCDS_REP_DATA_INGR_DEST_EXTRA_BITS (18 - DPP_MCDS_REP_DATA_BM_ID_NOF_BITS)
#define DPP_MCDS_REP_DATA_INGR_DEST_EXTRA_BITS_MASK ((1 << DPP_MCDS_REP_DATA_INGR_DEST_EXTRA_BITS) - 1)

#define DPP_MCDS_REP_TYPE_INGRESS 0
#define DPP_MCDS_REP_TYPE_EGR_PORT_CUD 3
#define DPP_MCDS_REP_TYPE_EGR_CUD 2
#define DPP_MCDS_REP_TYPE_EGR_BM_CUD 1

#define DPP_MCDS_REP_DATA_SET_TYPE(data, type) do {(data)->extra |= ((type) << DPP_MCDS_REP_DATA_TYPE_OFFSET);} while (0)
#define DPP_MCDS_REP_DATA_RESET_TYPE(data, type) do {(data)->extra &= ~(((uint32)DPP_MCDS_REP_DATA_TYPE_MASK) << DPP_MCDS_REP_DATA_TYPE_OFFSET); \
  (data)->extra |= ((type) << DPP_MCDS_REP_DATA_TYPE_OFFSET);} while (0)
#define DPP_MCDS_REP_DATA_GET_TYPE(data) (((data)->extra >> DPP_MCDS_REP_DATA_TYPE_OFFSET) & DPP_MCDS_REP_DATA_TYPE_MASK)

#define DPP_MCDS_REP_DATA_SET_EXTRA_CUD(data, cud) do {(data)->extra |= (cud) << DPP_MCDS_REP_DATA_EXTRA_CUD_SHIFT;} while (0)
#define DPP_MCDS_REP_DATA_RESET_EXTRA_CUD(data, cud) do {(data)->extra &= ~(((uint32)DPP_MCDS_REP_DATA_CUD_MASK) << DPP_MCDS_REP_DATA_EXTRA_CUD_SHIFT); \
                                                         (data)->extra |= (cud) << DPP_MCDS_REP_DATA_EXTRA_CUD_SHIFT;} while (0)
#define DPP_MCDS_REP_DATA_GET_EXTRA_CUD(data) ((data)->extra >> DPP_MCDS_REP_DATA_EXTRA_CUD_SHIFT)

#define DPP_MCDS_REP_DATA_SET_EGR_PORT(data, port) do {(data)->base |= (port);} while (0)
#define DPP_MCDS_REP_DATA_RESET_EGR_PORT(data, port) do {(data)->base &= ~(uint32)DPP_MCDS_REP_DATA_PORT_MASK; (data)->base |= (port);} while (0)
#define DPP_MCDS_REP_DATA_GET_EGR_PORT(data) ((data)->base & DPP_MCDS_REP_DATA_PORT_MASK)

#define DPP_MCDS_REP_DATA_SET_EGR_BM_ID(data, bm_id) do {(data)->base |= (bm_id);} while (0)
#define DPP_MCDS_REP_DATA_RESET_EGR_BM_ID(data, bm_id) do {(data)->base &= ~(uint32)DPP_MCDS_REP_DATA_BM_ID_MASK; (data)->base |= (bm_id);} while (0)
#define DPP_MCDS_REP_DATA_GET_EGR_BM_ID(data) ((data)->base & DPP_MCDS_REP_DATA_BM_ID_MASK)

#define DPP_MCDS_REP_DATA_SET_EGR_CUD(data, cud) do {(data)->base |= (cud) << DPP_MCDS_REP_DATA_EGR_CUD_SHIFT;} while (0)
#define DPP_MCDS_REP_DATA_RESET_EGR_CUD(data, cud) do {(data)->base &= ~(((uint32)DPP_MCDS_REP_DATA_CUD) << DPP_MCDS_REP_DATA_EGR_CUD_SHIFT); \
                                                       (data)->base |= (cud) << DPP_MCDS_REP_DATA_EGR_CUD_SHIFT;} while (0)
#define DPP_MCDS_REP_DATA_GET_EGR_CUD(data) ((data)->base >> DPP_MCDS_REP_DATA_EGR_CUD_SHIFT)

#define DPP_MCDS_REP_DATA_SET_INGR_CUD(data, cud) DPP_MCDS_REP_DATA_SET_EGR_CUD(data, cud)
#define DPP_MCDS_REP_DATA_RESET_INGR_CUD(data, cud) DPP_MCDS_REP_DATA_RESET_EGR_CUD(data, cud)
#define DPP_MCDS_REP_DATA_GET_INGR_CUD(data) DPP_MCDS_REP_DATA_GET_EGR_CUD(data)

#define DPP_MCDS_REP_DATA_SET_INGR_DEST(data, dest) do {DPP_MCDS_REP_DATA_SET_EGR_BM_ID((data), (dest) & DPP_MCDS_REP_DATA_BM_ID_MASK); (data)->extra |= \
  (dest) >> DPP_MCDS_REP_DATA_BM_ID_NOF_BITS;} while (0)
#define DPP_MCDS_REP_DATA_RESET_INGR_DEST(data, dest) do {DPP_MCDS_REP_DATA_RESET_EGR_BM_ID((data), (dest) & DPP_MCDS_REP_DATA_BM_ID_MASK); \
  (data)->extra &= ~DPP_MCDS_REP_DATA_INGR_DEST_EXTRA_BITS_MASK; (data)->extra |= (dest) >> DPP_MCDS_REP_DATA_BM_ID_NOF_BITS;} while (0)
#define DPP_MCDS_REP_DATA_GET_INGR_DEST(data) (DPP_MCDS_REP_DATA_GET_EGR_BM_ID(data) | \
  (((data)->extra & DPP_MCDS_REP_DATA_INGR_DEST_EXTRA_BITS_MASK) << DPP_MCDS_REP_DATA_BM_ID_NOF_BITS))

/* get the MCDB index of the start of the egress linked list of the given MCID and core */
#define DPP_MCDS_GET_EGRESS_GROUP_START(mcds, mcid, core_id) ((mcds)->egress_mcdb_offset + (mcds)->nof_egr_ll_groups * (core_id) + (mcid))

/* get the index in mcds->reps according to core and replication number in the core */
#define DPP_MCDS_GET_REP_INDEX(core_id, i) ((DPP_MULT_MAX_REPLICATIONS * (core_id)) + (i))

#define DPP_CUD2CORE_BITS_PER_CUD 2
#define DPP_CUD2CORE_CUD_MASK ((1 << DPP_CUD2CORE_BITS_PER_CUD) - 1)
#define DPP_CUD2CORE_CUDS_PER_WORD (SOC_SAND_NOF_BITS_IN_UINT32 / DPP_CUD2CORE_BITS_PER_CUD)
#define DPP_CUD2CORE_UNDEF_VALUE DPP_CUD2CORE_CUD_MASK
#define DPP_CUD2CORE_GET_CORE(_unit, _cud, _core) do { \
        uint32 _lcl_core = 0;\
        SOCDNX_IF_ERR_EXIT( \
            sw_state_access[(_unit)].dpp.soc.arad.tm.arad_multicast.cud2core.bit_range_read( \
                (_unit), \
                (_cud) * DPP_CUD2CORE_BITS_PER_CUD, \
                0, \
                DPP_CUD2CORE_BITS_PER_CUD, \
                &_lcl_core) \
        ); \
        (_core) = _lcl_core; \
    } while (0)
#define DPP_CUD2CORE_SET_CORE(_unit, _cud, _core) do { \
        uint32 _lcl_core = (_core); \
        SOCDNX_IF_ERR_EXIT( \
            sw_state_access[(_unit)].dpp.soc.arad.tm.arad_multicast.cud2core.bit_range_write( \
                (_unit), \
                (_cud) * DPP_CUD2CORE_BITS_PER_CUD, \
                0, \
                DPP_CUD2CORE_BITS_PER_CUD,\
                &_lcl_core)\
        );\
  } while (0)


typedef enum
{
  McdsFreeBuildBlocksAdd_AllMustBeUsed = 0, /* add all entries and verify that they are currently used */
  McdsFreeBuildBlocksAddAll,                /* add all entries */
  McdsFreeBuildBlocksAddOnlyFree            /* add only entries marked as free */
} mcds_free_build_option_t;


/*Each bit in this array represents a TM port. The +1 is to allow memory operations directly in the array */
typedef uint32 dpp_mc_egr_bitmap_t[DPP_MC_NOF_EGRESS_BITMAP_WORDS + 1];


/* types of replication information, that can be passed between functions inside the mcds and prevent the need for allocation */

typedef uint32 dpp_mc_ingress_dest_t; /* 18 bits */
typedef uint32 dpp_mc_outlif_t;       /* 16 bits in Arad, 18/19 bits in Jericho */
typedef uint8  dpp_mc_local_port_t;   /* 8 bits for normal egress formats */
typedef uint16 dpp_mc_bitmap_id_t;    /* 13 bits range */

typedef uint32 dpp_mc_core_bitmap_t; /* contains one bit per core, specifying if to perform an operation on each active core */
#define DPP_MC_CORE_BITAMAP_NO_CORES ((dpp_mc_core_bitmap_t)0)
#define DPP_MC_CORE_BITAMAP_CORE_0 ((dpp_mc_core_bitmap_t)1)
#define DPP_MC_CORE_BITAMAP_ALL_ACTIVE_CORES(unit) ((DPP_MC_CORE_BITAMAP_CORE_0 << SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores) - 1)
#define DPP_MC_FOREACH_CORE(core_bitmap, core_bitmap_temp, core) \
    for ((core) = 0, (core_bitmap_temp) = (core_bitmap); (core_bitmap_temp); ++core, (core_bitmap_temp) = (core_bitmap_temp) >> 1) \
        if (core_bitmap_temp & DPP_MC_CORE_BITAMAP_CORE_0)

#define DPP_MC_NO_2ND_CUD 0 /* There is no 2nd CUD, this is also the value used by hardware */
#define DPP_MC_2ND_CUD_IS_EEI 0x80000000 /* The type of the 2nd CUD is EEI */

#define DPP_MC_2ND_CUD_TYPE_MCDS2REP(type) (((uint32)(type)) << 31) /* convert the type of the 2nd CUD from group (DPP_MC_GROUP_2ND_CUD_OUTRIF/EEI) to replication (0/DPP_MC_2ND_CUD_IS_EEIa) */
#define DPP_MC_2ND_CUD_TYPE_REP2MCDS(type) (2-(uint8)(((uint32)(type)) >> 31)) /* convert in the opposite direction */

typedef enum /* an Enum specifying if a MC group has replications with a 2nd CUD, and if so what is the type of the 2nd CUD */
{
  dpp_mc_group_2nd_cud_none = 0,   /* The MC group has no replications with a 2nd CUD */
  dpp_mc_group_2nd_cud_outrif = 1, /* The MC group has replications of type OutRif */
  dpp_mc_group_2nd_cud_eei = 2     /* The MC group has replications of type EEI */
} dpp_mc_group_2nd_cud_type_t;

typedef struct {
    uint32 dest;           /* destination */
    uint32 cud;            /* replication CUD stored in the replication entry. */
    uint32 additional_cud; /* replication CUD stored in a special entry and common to the entries following it.  */
                           /* Also encodes the type of this CUD. May be DPP_MC_NO_2ND_CUD. */
} dpp_mc_replication_t; /* represents the information of a multicast replication */


/* entry format writing function types */

/*
 * This function writes the hardware fields of egress format 0 (port+CUD replications with a link pointer) to a arad_mcdb_entry_t structure.
 */
typedef void (*dpp_egr_mc_write_entry_port_cud_f)(
    SOC_SAND_IN    int               unit,
    SOC_SAND_INOUT arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    SOC_SAND_IN    dpp_rep_data_t    *rep1,       /* replication 1 */
    SOC_SAND_IN    dpp_rep_data_t    *rep2,       /* replication 2 (7 bit port) */
    SOC_SAND_IN    uint32            next_entry   /* the next entry */
);

/*
 * This function writes the hardware fields of egress format 4/5 (port_CUD replications with no link pointer) to a arad_mcdb_entry_t structure.
 * The replications to write are specified by structure pointers, NULL pointers mean disabled replications.
 */
typedef void (*dpp_egr_mc_write_entry_port_cud_noptr_f)(
    SOC_SAND_IN    int               unit,
    SOC_SAND_INOUT arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    SOC_SAND_IN    dpp_rep_data_t    *rep1,       /* replication 1 */
    SOC_SAND_IN    dpp_rep_data_t    *rep2,       /* replication 2 */
    SOC_SAND_IN    uint8             use_next     /* If zero, select format indicating end of linked list, otherwise */
                                                  /* select format indicating that the following entry is next. */
);

/*
 * This function writes the hardware fields of egress format 2 (CUD only with link pointer) to a arad_mcdb_entry_t structure.
 */
typedef void (*dpp_egr_mc_write_entry_cud_f)(
    SOC_SAND_IN    int               unit,
    SOC_SAND_INOUT arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    SOC_SAND_IN    dpp_rep_data_t    *rep1,       /* replication 1 */
    SOC_SAND_IN    dpp_rep_data_t    *rep2,       /* replication 2  */
    SOC_SAND_IN    uint32            next_entry   /* the next entry */
);

/*
 * This function writes the hardware fields of egress format 6/7 (CUD only with no link pointer) to a arad_mcdb_entry_t structure.
 * The replications to write are specified by structure pointers, NULL pointers mean disabled replications.
 */
typedef void (*dpp_egr_mc_write_entry_cud_noptr_f)(
    SOC_SAND_IN     int               unit,
    SOC_SAND_INOUT  arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    SOC_SAND_IN     dpp_rep_data_t    *rep1,       /* replication 1 */
    SOC_SAND_IN     dpp_rep_data_t    *rep2,       /* replication 2 */
    SOC_SAND_IN     dpp_rep_data_t    *rep3,       /* replication 3 */
    SOC_SAND_IN     uint8             use_next     /* If zero, select format indicating end of linked list, otherwise */
                                                   /* select format indicating that the following entry is next. */
);

/*
 * This function writes the hardware fields of egress format 1 (bitmap+CUD) to a arad_mcdb_entry_t structure.
 */
typedef void (*dpp_egr_mc_write_entry_bm_cud_f)(
    SOC_SAND_IN    int               unit,
    SOC_SAND_INOUT arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    SOC_SAND_IN    dpp_rep_data_t    *rep,        /* the replication */
    SOC_SAND_IN    uint32            next_entry   /* the next entry */
);

/*
 * Adds the contents of a mcdb entry of the given type to the mcds buffer.
 * No more than *max_size replications are added, and the max_size value
 * is decreased by the number of added replications.
 * *group_size is increased by the number of found replications.
 * The next entry pointed to by this entry is returned in next_entry.
 */
typedef uint32 (*dpp_get_replications_from_entry_f)(
    SOC_SAND_IN    int     unit,
    SOC_SAND_IN    int     core,        /* relevant when hardware has a per core linked list */
    SOC_SAND_IN    uint8   get_bm_reps, /* Should the function return bitmap replications (if non zero) or ignore them */
    SOC_SAND_IN    uint32  entry_index, /* table index of the entry */
    SOC_SAND_IN    uint32  entry_type,  /* the type of the entry */
    SOC_SAND_INOUT uint32  *cud2,       /* the current 2nd CUD of replications */
    SOC_SAND_INOUT uint16  *max_size,   /* the maximum number of replications to return from the group, decreased by the number of returned replications */
    SOC_SAND_INOUT uint16  *group_size, /* incremented by the number of found replications (even if they are not returned) */
    SOC_SAND_OUT   uint32  *next_entry  /* the next entry */
);

/*
 * sets an ingress replication in BCM API encoding from the given CUD and destination in hardware encoding.
 */
typedef uint32 (*dpp_convert_ingress_replication_hw2api_f)(
    SOC_SAND_IN  int          unit,
    SOC_SAND_IN  uint32       cud,            /* CUD to be converted */
    SOC_SAND_IN  uint32       dest,           /* destination to be converted */
    SOC_SAND_OUT soc_gport_t  *port_array,    /* output array to contain ports/destinations */
    SOC_SAND_OUT soc_if_t     *encap_id_array /* output array to contain encapsulations/CUDs/outlifs */
);

/*
 * Set a linked list of the input egress entries, possibly using a provided free block as the first allocation.
 */
typedef uint32 (*dpp_set_egress_linked_list_f)(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  uint8                         is_group_start,      /* specifies if list_prev is a group start to be filled or not */
    SOC_SAND_IN  uint32                        group_id,            /* If is_group_start this is the group ID, otherwise this is the entry preceding the single linked list to be created */
    SOC_SAND_IN  uint32                        list_end,            /* The entry that end of the created linked list will point to */
    SOC_SAND_IN  uint32                        alloced_block_start, /* start index of an allocated block to use for the free list */
    SOC_SAND_IN  dpp_free_entries_block_size_t alloced_block_size,  /* size of the allocated block to use, should be 0 if none */
    SOC_SAND_IN  dpp_mc_core_bitmap_t          cores_to_set,        /* cores of linked lists to set */
    SOC_SAND_OUT uint32                        *list_start,         /* The first entry of the created linked list */
    SOC_SAND_OUT SOC_TMC_ERROR                 *out_err             /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
);

typedef struct {
    dpp_mcds_common_t common; /* This member must be the first to implement the polymorphism */
    uint32 nof_unoccupied; /* number of free entries in the mcdb */

    /* The next two entries are both either null or an array of size DPP_LAST_MCDB_ENTRY(mcds)+1 */
    arad_mcdb_entry_t *mcdb;    /* mcdb cache, free list, and other data in the free bits */
    uint16 *prev_entries;       /* for used mcdb entries this will hold the previous entry in the linked list */
    uint32 free_value[2];       /* The value of a free entry, also the value of the start of an empty egress linked list. */
    uint32 empty_ingr_value[2]; /* The value of the start of an empty ingress linked list. */
    uint32 msb_word_mask      ; /* mask of the hardware data bits in the msb word of an mcdb entry */
    uint32 ingr_word1_replication_mask; /* bits used in word1 to compare the replication (CUD+destination) */
    uint32 max_ingr_cud_field;  /* The maximum value in an ingress CUD field */  
    uint32 max_egr_cud_field;   /* The maximum value in an egress CUD field */
    uint32 egress_mcdb_offset;  /* The offset in the MCDB to which the MCID is added to get the first entry of the group of core 0 */
    uint32 nof_egr_ll_groups;   /* The number of egress linked list groups (per core) which is SOC_DPP_CONFIG(unit)->tm.nof_mc_ids - (SOC_DPP_CONFIG(unit)->tm.multicast_egress_bitmap_group_range.mc_id_high + 1) */
    int unit; /* The device unit of the mcds */
    dpp_free_entries_blocks_region_t free_general; /* free lists of entries that are not automatically allocated */
    dpp_free_entries_blocks_region_t ingress_alloc_free; /* free lists of entries that are automatically allocated; ingress and egress separately */
    dpp_free_entries_blocks_region_t egress_alloc_free;

    dpp_egr_mc_write_entry_port_cud_f egr_mc_write_entry_port_cud;
    dpp_egr_mc_write_entry_port_cud_noptr_f egr_mc_write_entry_port_cud_noptr;
    dpp_egr_mc_write_entry_cud_f egr_mc_write_entry_cud;
    dpp_egr_mc_write_entry_cud_noptr_f egr_mc_write_entry_cud_noptr;
    dpp_egr_mc_write_entry_bm_cud_f egr_mc_write_entry_bm_cud;
    dpp_get_replications_from_entry_f get_replications_from_entry;
    dpp_convert_ingress_replication_hw2api_f convert_ingress_replication_hw2api;
    dpp_set_egress_linked_list_f set_egress_linked_list;

#ifdef DONOT_USE_SW_DB_FOR_MULTICAST
    uint32 *egress_groups_open_data; /* replicate the information of is each egress group, in warm boot it is stored to the warm boot file */
#endif

    /* Information used to describe group replications, can be used to pass information between functions and internally in a function */
    /* The egress replication destination ports are TM/PP ports */
    dpp_rep_data_t reps[DPP_MULT_MAX_REPLICATIONS * SOC_DPP_DEFS_MAX(NOF_CORES)]; /* used to store replication data of a group */

    uint16 nof_reps[SOC_DPP_DEFS_MAX(NOF_CORES)]; /* per core: used to store replication data of a group */
    uint16 nof_ingr_reps;            /* number of ingress replications or egress port replications in non standard egress groups */
    uint16 nof_egr_bitmap_reps[SOC_DPP_DEFS_MAX(NOF_CORES)];      /* number of egress replications of type bitmap + outlif */
    uint16 nof_egr_port_outlif_reps[SOC_DPP_DEFS_MAX(NOF_CORES)]; /* per core: number of egress replications of type port + outlif */
    uint16 nof_egr_outlif_reps[SOC_DPP_DEFS_MAX(NOF_CORES)];      /* per core: number of egress replications of type outlif */
    uint8 info_2nd_cud; /* information on the 2nd CUD of group replications, and if replications with a 2nd CUD exist */

    uint16 max_nof_ingr_replications; /* maximum number of ingress replications, depending buffer type, leaving room for mirror+snoop */
    uint16 max_nof_mmc_replications; /* maximum number of ingress replications in a mini-multicast buffer, leaving room for mirror+snoop */

} dpp_mcds_base_t; /* base structure shared by Arad and Jericho */


/* values of the info_2nd_cud field of dpp_mcds_base_t specifying if a MC group has replications with a 2nd CUD, and if so what is the type of the 2nd CUD */
#define DPP_MC_GROUP_2ND_CUD_NONE   0 /* The MC group has no replications with a 2nd CUD */
#define DPP_MC_GROUP_2ND_CUD_EEI    1 /* The MC group has replications of type EEI */
#define DPP_MC_GROUP_2ND_CUD_OUTRIF 2 /* The MC group has replications of type OutRif */

/* } */

/*
 * MCDS functions
 */

/*
 * Get a pointer to the mcdb entry with the given index in the mcds
 */
dpp_mcdb_entry_t* dpp_mcds_get_mcdb_entry(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  uint32 mcdb_index
);


/* function to compare dpp_rep_data_t replications, for sorting usage */
int dpp_rep_data_t_compare(void *a, void *b);

/*
 * Write a MCDB entry to hardware from the mcds.
 * Using only this function for writes, and using it after mcds mcdb used
 * entries updates, ensures consistency between the mcds and the hardware.
 */

uint32 dpp_mcds_write_entry(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32 mcdb_index /* index of entry to write */
);

/*
 * Copy the src_index entry to the dst_index entry, and write the dst_index entry to hardware.
 * Both the hardware and mcds is copied. So be bery careful if using this to copy a free entry.
 */
uint32 dpp_mcdb_copy_write(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32 src_index, /* index of entry to be copied */
    SOC_SAND_IN uint32 dst_index  /* index of entry to be copied to and written to disk */
);

/*
 * clear the replications data in the mcds
 */
void dpp_mcds_clear_replications(dpp_mcds_base_t *mcds, const uint32 group_type);

/*
 * This functions copies the replication data from the mcds into the given gport and encap_id arrays.
 * It is an error if more entries are to be copied than available in the arrays.
 */
uint32 dpp_mcds_copy_replications_to_arrays(
    SOC_SAND_IN  int          unit,
    SOC_SAND_IN  uint8        is_egress,           /* are the replications for an egress multicast group (opposed to ingress) */
    SOC_SAND_IN  uint32       arrays_size,         /* size of output arrays */
    SOC_SAND_OUT soc_gport_t  *port_array,         /* output array to contain logical ports/destinations, used if !reps */
    SOC_SAND_OUT soc_if_t     *encap_id_array,     /* output array to contain encapsulations/CUDs/outlifs, used if !reps */
    SOC_SAND_OUT soc_multicast_replication_t *reps /* output replication array (array of size mc_group_size*/
);

/*
 * This functions copies the replication data from the given port and ecap_id arrays into the mcds.
 * It is an error if the mcds is filled beyond the maximum size of a mulitcast group.
 * We currently assume that the destination/port translation is done by bcm code before calling this function.
 */
uint32 dpp_mcds_copy_replications_from_arrays(
    SOC_SAND_IN  int       unit,
    SOC_SAND_IN  uint8     is_egress,       /* are the replications for an egress multicast group (opposed to ingress) */
    SOC_SAND_IN  uint8     do_clear,        /* if zero, replications will be added in addition to existing ones, otherwise previous replications will be cleared */
    SOC_SAND_IN  uint32    arrays_size,     /* size of output arrays */
    SOC_SAND_IN  dpp_mc_replication_t *reps /* input array containing replications */
);

/*
 * This functions copies the replication data from a consecutive egress entries block into the mcds.
 * It is an error if the mcds is filled beyond the maximum size of a multicast group.
 * All the block entries except for the last point implicitly to the next entry using formats 5,7.
 */
uint32
  dpp_mcds_copy_replications_from_egress_block(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  uint8                         do_clear,    /* if zero, replications will be added in addition to existing ones, otherwise previous replications will be cleared */
    SOC_SAND_IN  uint32                        block_start, /* index of the block start */
    SOC_SAND_IN  dpp_free_entries_block_size_t block_size,  /* number of entries in the block */
    SOC_SAND_INOUT uint32                      *cud2,       /* the current 2nd CUD of replications */
    SOC_SAND_OUT uint32                        *next_entry  /* the next entry pointed to by the last block entry */
);

/* functions to add replications of a specific types to the group stored in the mcds */
void dpp_add_ingress_replication(
  dpp_mcds_base_t *mcds,
  const uint32     cud,
  const uint32     dest
);

void dpp_add_egress_replication_port_cud(
  dpp_mcds_base_t *mcds,
  unsigned         core,
  const uint32     cud,
  const uint32     cud2,
  const uint32     port
);

void dpp_add_egress_replication_cud(
  dpp_mcds_base_t *mcds,
  unsigned         core,
  const uint32     cud,
  const uint32     cud2
);

void dpp_add_egress_replication_bitmap(
  dpp_mcds_base_t *mcds,
  uint32           core,
  const uint32     cud,
  const uint32     cud2,
  const uint32     bm_id
);


/*
 * free blocks handling functions
 */

/*
 * Init a region with the given maximum block size. All lists will be marked as empty.
 * Device independent.
 */
void dpp_mcds_init_region(dpp_free_entries_blocks_region_t *region, dpp_free_entries_block_size_t max_size, uint32 range_start, uint32 range_end);

/*
 * Add free entries in the given range as blocks to the lists of the given region.
 * If (check_free) add only blocks marked as free.
 * Otherwise add all entries in the range are expected to be marked used and they will be marked as free.
 */
uint32 dpp_mcds_build_free_blocks(
    SOC_SAND_IN    int                              unit,   /* used only if check_free is zero */
    SOC_SAND_INOUT dpp_mcds_base_t                  *mcds,
    SOC_SAND_IN    uint32                           start_index, /* start index of the range to work on */
    SOC_SAND_IN    uint32                           end_index,   /* end index of the range to work on, if smaller than start_index hen do nothing */
    SOC_SAND_INOUT dpp_free_entries_blocks_region_t *region,     /* region to contain the block in its lists */
    SOC_SAND_IN    mcds_free_build_option_t         entry_option /* which option to use in selecting entries to add and verifying them */
);

/*
 * Get a free block of size 1 at a given location.
 * Used for getting the first entry of a multicast group.
 * Does not mark mcdb_index as used.
 */
uint32 dpp_mcds_reserve_group_start(
    SOC_SAND_INOUT dpp_mcds_base_t *mcds,
    SOC_SAND_IN    uint32           mcdb_index /* the mcdb indx to reserve */
);

/*
 * Get a free entries block of a given size, according to flags that needs to be used to start a multicast group.
 * Returns the start index and the number of entries in the block.
 */
uint32 dpp_mcds_get_free_entries_block(
    SOC_SAND_INOUT dpp_mcds_base_t              *mcds,
    SOC_SAND_IN    uint32                        flags,        /* DPP_MCDS_GET_FREE_BLOCKS_* flags that affect what the function does group */
    SOC_SAND_IN    dpp_free_entries_block_size_t wanted_size,  /* needed size of the free block group */
    SOC_SAND_IN    dpp_free_entries_block_size_t max_size,     /* do not return blocks above this size, if one would have been returned, split it */
    SOC_SAND_OUT   uint32                        *block_start, /* the start of the relocation block */
    SOC_SAND_OUT   dpp_free_entries_block_size_t *block_size   /* the size of the returned block */
);

/*
 * Get the region corresponding to the given mcdb index
 */
dpp_free_entries_blocks_region_t* dpp_mcds_get_region(dpp_mcds_base_t *mcds, uint32 mcdb_index);

/*
 * Return the region corresponding to the given mcdb index,
 * and get the max consecutive entries sub range from the range that includes mcdb_index.
 */
dpp_free_entries_blocks_region_t* dpp_mcds_get_region_and_consec_range(dpp_mcds_base_t *mcds, uint32 mcdb_index, uint32 *range_start, uint32 *range_end);

/*
 * Relocate the given used entries, not disturbing multicast traffic to
 * the group containing the entries.
 * MCDS and hardware are updated accordingly.
 * If relocation_block_size is 0, then the function calculates
 * relocation_block_start and relocation_block_size by itself to suit mcdb_index.
 * After successful relocation, the block that was relocated is freed, except for entry mcdb_index.
 *
 * This function may overwrite the mc group replications stored in the mcds.
 */


uint32 dpp_mcdb_relocate_entries(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  uint32                        mcdb_index,             /* table index needed for the start of a group */
    SOC_SAND_IN  uint32                        relocation_block_start, /* the start of the relocation block */
    SOC_SAND_IN  dpp_free_entries_block_size_t relocation_block_size,  /* the size of the relocation block */
    SOC_SAND_OUT SOC_TMC_ERROR                 *out_err                /* return possible errors that the caller may want to ignore: insufficient memory */
);

uint32 dpp_mcds_unoccupied_get(
    SOC_SAND_IN dpp_mcds_base_t *mcds
);



/*
 * Free a linked list that is not used any more, updating mcds and hardware.
 * The given linked list must not include the first entry of the group.
 * All entries in the linked list should be of the given type.
 */

uint32 dpp_mcdb_free_linked_list(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  uint32                       first_index, /* table index of the first entry in the linked list to free */
    SOC_SAND_IN  uint32                       entries_type /* the type of the entries in the list */
);

/*
 * Free a linked list that is not used any more, updating mcds and hardware.
 * Stop at the given entry, and do not free it.
 * The given linked list must not include the first entry of the group.
 * All entries in the linked list should be of the given type.
 */

uint32
  dpp_mcdb_free_linked_list_till_my_end(
    SOC_SAND_IN  int    unit,
    SOC_SAND_IN  uint32    first_index,  /* table index of the first entry in the linked list to free */
    SOC_SAND_IN  uint32    entries_type, /* the type of the entries in the list */
    SOC_SAND_IN  uint32    end_index     /* the index of the end of the list to be freed */
);



/*
 * generic MC functions
 */

/*
 * This function checks if the multicast group is open (possibly empty).
 * returns TRUE if the group is open (start of a group), otherwise FALSE.
 */
int dpp_mult_does_group_exist(
    SOC_SAND_IN  int     unit,
    SOC_SAND_IN  uint32  mcid,       /* MC ID of the group */
    SOC_SAND_IN  int     is_egress,  /* is the MC group an egress group */
    SOC_SAND_OUT uint8   *does_exit
);

/* Mark the given egress group as open in the warm boot data */
uint32 dpp_egress_group_open_set(
    SOC_SAND_IN  int     unit, /* device */
    SOC_SAND_IN  uint32  group_id,  /* multicast ID */
    SOC_SAND_IN  uint8   is_open    /* non zero value will mark the group as open */
);

/*
 * Return the contents of a multicast linked list group of a given type.
 * The contents is returned in the mcds buffer.
 * if the group size is bigger than the specified max_size, only max_size entries are returned,
 * it will not be an error, and the actual size of the group is returned.
 * The group must be open.
 */
uint32 dpp_mcds_get_group(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  dpp_mc_core_bitmap_t cores_to_get, /* the linked list cores to get */
    SOC_SAND_IN  uint8                do_clear,     /* if zero, replications will be added in addition to existing ones, otherwise previous replications will be cleared */
    SOC_SAND_IN  uint8                get_bm_reps,  /* Should the function return bitmap replications in every core (if non zero) or only in core 0 */
    SOC_SAND_IN  uint32               group_id,     /* the mcid of the group */
    SOC_SAND_IN  uint32               group_type,   /* the type of the group (of the entries in the list) */
    SOC_SAND_IN  uint16               max_size,     /* the maximum number of members to return from the group */
    SOC_SAND_OUT uint16               *group_size   /* the returned actual group size */
);

/*
 * This functions removes the given (single) replication (dest and cud) from the mcds.
 * It is an error if the mcds is filled beyond the maximum size of a multicast group.
 * We currently assume that the destination/port translation is done by bcm code before calling this function.
 */
uint32 dpp_mult_remove_replication(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32               group_type, /* to what type of group does the replication belong */
    SOC_SAND_IN  uint32               dest,       /* output array to contain ports/destinations */
    SOC_SAND_IN  soc_if_t             cud,        /* output array to contain encapsulations/CUDs/outlifs */
    SOC_SAND_IN  soc_if_t             cud2,       /* output array to contain second encapsulations/CUDs/outlifs */
    SOC_SAND_OUT SOC_TMC_ERROR        *out_err,   /* return possible errors that the caller may want to ignore: replication does not exist */
    SOC_SAND_OUT dpp_mc_core_bitmap_t *cores      /* return the linked list cores of the removed replication */
);



/* Ingress MC functions */

/*
 * This function writes ingress format to a mcds mcdb entry and then to hardware the input.
 * The input is the replication data (destination and outlif), pointer to next entry,
 * and the pointer to the previous entry.
 */
uint32 dpp_mult_ing_multicast_group_entry_to_tbl(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  dpp_mc_id_t         multicast_id_ndx, /* mcdb to write to */
    SOC_SAND_IN  SOC_TMC_MULT_ING_ENTRY  *ing_entry,       /* replication data */
    SOC_SAND_IN  uint32               next_entry,       /* the next entry */
    SOC_SAND_IN  uint32               prev_entry        /* the previous entry written only to mcds */
);



/* Egress MC functions */

/* Check if the given egress group is created=open, will return 1 if the group is marked as open, or 0 */
int dpp_egress_group_open_get(
    SOC_SAND_IN  int    unit,
    SOC_SAND_IN  uint32 group_id, /* multicast ID */
    SOC_SAND_OUT uint8 *is_open
);

/*
 * Set a linked list of the input egress entries, possibly using a provided free block as the first allocation.
 * The replications are taken form the mcds.
 * If is_group_start is non zero, then list_prev is the (free and reserved) group start entry and it is set with replications.
 * Otherwise we do not handle the start of the egress group so there is no need for special handling of the first entry.
 * If the function fails, it will free the given allocated block.
 * In the start_block_index entry, link to the previous entry according to the previous entry of input_block_index.
 * On failure allocated entries are freed, including alloced_block_start.
 * Linked lists replaced by new linked lists are not freed by this function.
 */

uint32 dpp_mcds_set_egress_linked_list(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  uint8                         is_group_start,      /* specifies if list_prev is a group start to be filled or not */
    SOC_SAND_IN  uint32                        group_id,            /* If is_group_start this is the group ID, otherwise this is the entry preceding the single linked list to be created */
    SOC_SAND_IN  uint32                        list_end,            /* The entry that end of the created linked list will point to, Same one for all given cores */
    SOC_SAND_IN  uint32                        alloced_block_start, /* start index of an allocated block to use for the free list */
    SOC_SAND_IN  dpp_free_entries_block_size_t alloced_block_size,  /* size of the allocated block to use, should be 0 if none */
    SOC_SAND_IN  dpp_mc_core_bitmap_t          cores_to_set,        /* cores of linked lists to set */
    SOC_SAND_OUT uint32                        *list_start,         /* The first entry of the created linked list */
    SOC_SAND_OUT SOC_TMC_ERROR                 *out_err             /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
);


/* linked list handling functions */

/*********************************************************************
* Initialize MC replication database
* The initialization accesses the replication table as if it was an
* Ingress replication, for all entries (including Egress MC)
**********************************************************************/
uint32 dpp_mult_rplct_tbl_entry_unoccupied_set_all(
    SOC_SAND_IN  int unit
);

/*
 * Given a table index that needs to be used to start a multicast group,
 * returns the start index and the number of entries that need to be relocated.
 * If the number of entries returned is 0, a relocation is not needed.
 */
uint32
  dpp_mcds_get_relocation_block(
    SOC_SAND_IN  dpp_mcds_base_t              *mcds,
    SOC_SAND_IN  uint32                        mcdb_index,              /* table index needed for the start of a group */
    SOC_SAND_OUT uint32                        *relocation_block_start, /* the start of the relocation block */
    SOC_SAND_OUT dpp_free_entries_block_size_t *relocation_block_size   /* the size of the relocation block, 0 if relocation is not needed */
);


/*
 * Functions called from outside the dpp multicast code.
 */

/*
 * Initialize the multicast part of the software database.
 * Do not fill the data from hardware yet.
 * dpp_mcds_multicast_init2() will be called to do so when we can access the MCDB using DMA.
 */
uint32 dpp_mcds_multicast_init(
    SOC_SAND_IN int      unit
);

/*
 * Initialize the multicast part of the software database.
 * Must be run after dpp_mcds_multicast_init() was called successfully, and when DMA is up.
 * fills the multicast data from hardware.
 */
uint32 dpp_mcds_multicast_init2(
    SOC_SAND_IN int      unit
);

/*
 * free allocations done in the multicat init
 */
uint32 dpp_mcds_multicast_terminate(
    SOC_SAND_IN int unit
);

/*
 * Initialization of the Arad blocks configured in this module.
 * Called as part of the initialization sequence.
 */
uint32 dpp_mc_init(
    SOC_SAND_IN  int                 unit
);

/*
 * This function checks if the multicast group is open (possibly empty).
 * returns TRUE if the group is open (start of a group), otherwise FALSE.
 */
uint32 dpp_mult_does_group_exist_ext( 
    SOC_SAND_IN  int             unit,
    SOC_SAND_IN  SOC_TMC_MULT_ID mcid,      /* MC ID of the group */
    SOC_SAND_IN  int             is_egress, /* is the MC group an egress group */
    SOC_SAND_OUT uint8           *is_open   /* returns FALSE if not open */
);


/*
 * Ingress Multicast functions called from outside the dpp multicast code.
 */

/*
 * This API sets the ingress group to the given replications,
 * configuring its linked list; and creates the group if it did not exist.
 */
uint32 dpp_mult_ing_group_open(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  dpp_mc_id_t            multicast_id_ndx, /* group mcid */
    SOC_SAND_IN  SOC_TMC_MULT_ING_ENTRY *mc_group,        /* group replications to set */
    SOC_SAND_IN  uint32                 mc_group_size,    /* number of group replications (size of mc_group) */
    SOC_SAND_OUT SOC_TMC_ERROR          *out_err          /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
);
 
/*
 * Closes the ingress muticast group, freeing its linked list.
 * Do nothing if the group is not open.
 */
uint32 dpp_mult_ing_group_close(
    SOC_SAND_IN  int            unit,
    SOC_SAND_IN  dpp_mc_id_t      multicast_id_ndx /* group mcid to close */
);

/*
 * This API sets the ingress group to the given replications, configuring its linked list.
 * The group must exist.
 */
uint32 dpp_mult_ing_group_update(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  dpp_mc_id_t          multicast_id_ndx,    /* group mcid */
    SOC_SAND_IN  SOC_TMC_MULT_ING_ENTRY   *mc_group,           /* group replications to set */
    SOC_SAND_IN  uint32                mc_group_size,       /* number of group replications (size of mc_group) */
    SOC_SAND_OUT SOC_TMC_ERROR         *out_err             /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
);

uint32 dpp_mult_ing_traffic_class_map_set(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  SOC_TMC_MULT_ING_TR_CLS_MAP *map
);
uint32 dpp_mult_ing_traffic_class_map_get(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_OUT SOC_TMC_MULT_ING_TR_CLS_MAP *map
);

/*
 * Adds the given replication to the ingress multicast group.
 * It is an error if the group is not open.
 */
uint32 dpp_mult_ing_destination_add(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  dpp_mc_id_t            multicast_id_ndx, /* group mcid */
    SOC_SAND_IN  SOC_TMC_MULT_ING_ENTRY *replication,     /* replication to add */
    SOC_SAND_OUT SOC_TMC_ERROR          *out_err          /* return possible errors that the caller may want to ignore : insufficient memory or duplicate replication */
);

/*
 * Removes the given replication from the ingress multicast group.
 * It is an error if the group is not open or does not contain the replication.
 */
uint32 dpp_mult_ing_destination_remove(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  dpp_mc_id_t            multicast_id_ndx, /* group mcid */
    SOC_SAND_IN  SOC_TMC_MULT_ING_ENTRY *entry,           /* replication to remove */
    SOC_SAND_OUT SOC_TMC_ERROR          *out_err          /* return possible errors that the caller may want to ignore : replication does not exist */
);

/*********************************************************************
*     Returns the size of the multicast group with the
*     specified multicast id.
*********************************************************************/
uint32 dpp_mult_ing_group_size_get(
    SOC_SAND_IN  int         unit,
    SOC_SAND_IN  dpp_mc_id_t multicast_id_ndx,
    SOC_SAND_OUT uint32      *mc_group_size
);

/*
 * Gets the ingress multicast group with the specified multicast id.
 * will return up to mc_group_size replications, and the exact
 * The group's replication number is returned in exact_mc_group_size.
 * The number of replications returned in the output arrays is
 * min{mc_group_size, exact_mc_group_size}.
 * It is not an error if the group is not open.
 */
uint32 dpp_mult_ing_get_group(
    SOC_SAND_IN  int         unit,
    SOC_SAND_IN  dpp_mc_id_t group_mcid,           /* group id */
    SOC_SAND_IN  uint32      mc_group_size,        /* maximum replications to return */
    SOC_SAND_OUT soc_gport_t *ports,               /* output ports (array of size mc_group_size) */
    SOC_SAND_OUT soc_if_t    *cuds,                /* output ports (array of size mc_group_size) */
    SOC_SAND_OUT uint32      *exact_mc_group_size, /* the number of replications in the group will be returned here */
    SOC_SAND_OUT uint8       *is_open              /* will return if the group is open (false or true) */
);

/*********************************************************************
*     Closes all opened ingress multicast groups.
*********************************************************************/
uint32 dpp_mult_ing_all_groups_close(
    SOC_SAND_IN  int unit
);


/*
 * Egress Multicast functions called from outside the dpp multicast code.
 */

/*********************************************************************
*     This procedure configures the range of values of the
*     multicast ids entry points that their multicast groups
*     are to be found according to a bitmap (as opposed to a
*     Link List). Only the max bitmap ID is configurable.
*********************************************************************/
uint32 dpp_mult_eg_bitmap_group_range_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE *info
);

/*********************************************************************
*     This procedure configures the range of values of the
*     multicast ids entry points that their multicast groups
*     are to be found according to a bitmap (as opposed to a
*     Link List).
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32 dpp_mult_eg_bitmap_group_range_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE *info
);

/*
 * This function opens a bitmap group, and sets it to replicate to the given ports.
 */
uint32 dpp_mult_eg_bitmap_group_create(
    SOC_SAND_IN  int         unit,
    SOC_SAND_IN  dpp_mc_id_t multicast_id_ndx /* group mcid */
);

/* This function closed a bitmap group, clearing its hardware replications.  */
uint32 dpp_mult_eg_bitmap_group_close(
    SOC_SAND_IN  int          unit,
    SOC_SAND_IN  dpp_mc_id_t  multicast_id_ndx
);

/*********************************************************************
*     This API updates the egress-multicast-replication
*     definitions for the specific multicast-id, and creates
*     in the device the needed link-list/bitmap. The user only
*     specifies the multicast-id and copies. All inner
*     link-list nodes and bitmap are allocated and handled by
*     the driver.
*********************************************************************/
uint32 dpp_mult_eg_bitmap_group_update(
                  int                                   unit,
                  dpp_mc_id_t                           multicast_id_ndx,
                  SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *group
);

/*
 * Gets the egress replications (ports) of the given bitmap.
 * If the bitmap is a vlan egress group, it does not have to be open/created.
 */
uint32 dpp_mult_eg_bitmap_group_get(
    SOC_SAND_IN  int                                   unit,
    SOC_SAND_IN  dpp_mc_id_t                           bitmap_id, /* ID of the bitmap */
    SOC_SAND_OUT SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *group     /* output port bitmap */
);

/*********************************************************************
*     Add port members of the Egress-Multicast and/or modify
*     the number of logical copies required at port.
*********************************************************************/
uint32 dpp_mult_eg_bitmap_group_port_add(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  dpp_mc_id_t         multicast_id_ndx,
    SOC_SAND_IN  SOC_TMC_FAP_PORT_ID port,
    SOC_SAND_OUT SOC_TMC_ERROR       *out_err      /* return possible errors that the caller may want to ignore */
);
/*********************************************************************
*   Add port replications from a bitmap to an Egress-Multicast bitmap group.
*********************************************************************/
uint32 dpp_mult_eg_bitmap_group_bm_add(
                  int                 unit,
                  dpp_mc_id_t         multicast_id_ndx,
                  SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *bitmap, /* TM ports to add */
                  SOC_TMC_ERROR                         *out_err /* return possible errors that the caller may want to ignore */
);

/*********************************************************************
*     Removes a port member of the egress multicast.
*********************************************************************/
uint32 dpp_mult_eg_bitmap_group_port_remove(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  dpp_mc_id_t         multicast_id_ndx,
    SOC_SAND_IN  SOC_TMC_FAP_PORT_ID port,
    SOC_SAND_OUT SOC_TMC_ERROR       *out_err      /* return possible errors that the caller may want to ignore */
);
/*********************************************************************
*   Add port replications from a bitmap to an Egress-Multicast bitmap group.
*********************************************************************/
uint32 dpp_mult_eg_bitmap_group_bm_remove(
                  int                 unit,
                  dpp_mc_id_t         multicast_id_ndx,
                  SOC_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *bitmap, /* TM ports to remove */
                  SOC_TMC_ERROR                         *out_err /* return possible errors that the caller may want to ignore */
);
/*********************************************************************
*     Closes all opened egress multicast groups in range of
*     vlan membership.
*********************************************************************/
uint32 dpp_mult_eg_bitmap_group_close_all_groups(
    SOC_SAND_IN  int                 unit
);

/*********************************************************************
*     This API closes egress-multicast-replication group for
*     the specific multicast-id. The user only specifies the
*     multicast-id. All inner link-list/bitmap nodes are freed
*     and handled by the driver
*********************************************************************/
uint32 dpp_mult_eg_group_close(
    SOC_SAND_IN  int    unit,
    SOC_SAND_IN  dpp_mc_id_t multicast_id_ndx
);

/*
 * Returns the size of the multicast group with the specified multicast id.
 * Not needed for bcm APIs, so not tested.
 * returns 0 for non open groups.
 */
uint32 dpp_mult_eg_group_size_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  dpp_mc_id_t           multicast_id_ndx,
    SOC_SAND_OUT uint32                 *mc_group_size
);

/*
 * Gets the egress multicast group with the specified multicast id.
 * will return up to mc_group_size replications, and the exact
 * Works with both TDM and non TDM groups.
 * The group's replication number is returned in exact_mc_group_size.
 * The number of replications returned in the output arrays is
 * min{mc_group_size, exact_mc_group_size}.
 * It is not an error if the group is not open.
 */
uint32 dpp_mult_eg_get_group(
    SOC_SAND_IN  int            unit,
    SOC_SAND_IN  dpp_mc_id_t    group_mcid,           /* group id */
    SOC_SAND_IN  uint32         mc_group_size,        /* maximum replications to return */
    SOC_SAND_OUT soc_gport_t    *ports,               /* output logical ports (array of size mc_group_size) used if !reps */
    SOC_SAND_OUT soc_if_t       *cuds,                /* output CUDs (array of size mc_group_size) used if !reps */
    SOC_SAND_OUT soc_multicast_replication_t *reps,   /* output replication array (array of size mc_group_size*/
    SOC_SAND_OUT uint32         *exact_mc_group_size, /* the number of replications in the group will be returned here */
    SOC_SAND_OUT uint8          *is_open              /* will return if the group is open (false or true) */
);

/*
 * This API sets the egress group to the given replications,
 * configuring its linked list.
 * If the group does not exist, it will be created or an error will be returned based on allow_create.
 * Creation may involve relocating the mcdb entry which will be the start
 * of the group, and possibly other consecutive entries.
 *
 * We always want to create entries with pointers from port+outlif couples and from bitmaps.
 * We need to leave one entry with a pointer for the start of the group.
 * every block of entries with no pointers ends with an entry pointer, except for the end of the group.
 */
uint32 dpp_mult_eg_group_set(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  dpp_mc_id_t          mcid,         /* the group mcid */
    SOC_SAND_IN  uint8                allow_create, /* if non zero, will create the group if it does not exist */
    SOC_SAND_IN  uint32               group_size,   /* size of ports and cuds to read group replication data from */
    SOC_SAND_IN  dpp_mc_replication_t *reps,        /* input array containing replications */
    SOC_SAND_OUT SOC_TMC_ERROR        *out_err      /* return possible errors that the caller may want to ignore */
);

/*
 * Adds the given replication to the non bitmap egress multicast group.
 * It is an error if the group is not open.
 */
uint32 dpp_mult_eg_reps_add(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  dpp_mc_id_t           group_mcid, /* group mcid */
    SOC_SAND_IN  uint32                nof_reps,   /* number of replications to add */
    SOC_SAND_IN  dpp_mc_replication_t  *reps,      /* input array containing replications to add*/
    SOC_SAND_OUT SOC_TMC_ERROR         *out_err    /* return possible errors that the caller may want to ignore */
);

/*
 * Removes the given replication from the non bitmap egress multicast group.
 * It is an error if the group is not open or does not contain the replication.
 */
uint32 dpp_mult_eg_reps_remove(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  dpp_mc_id_t           group_mcid,   /* group mcid */
    SOC_SAND_IN  uint32                nof_reps,     /* number of replications to remove */
    SOC_SAND_IN  dpp_mc_replication_t  *reps,        /* input array containing replications to remove */
    SOC_SAND_OUT SOC_TMC_ERROR         *out_err      /* return possible errors that the caller may want to ignore */
);


/*
 * Set the outlif (cud) to (local) port mapping from the given cud to the given port.
 */
uint32 dpp_mult_cud_to_port_map_set(
    SOC_SAND_IN int                 unit, /* input device */
    SOC_SAND_IN uint32              flags,/* flags - currently non zero value means allowing to switch cores */
    SOC_SAND_IN uint32              cud,  /* input cud/outlif */
    SOC_SAND_IN SOC_TMC_FAP_PORT_ID port  /* input (local egress) port */
);

/*
 * Get the outlif (cud) to (local) port mapping from the given cud.
 */
uint32 dpp_mult_cud_to_port_map_get(
    SOC_SAND_IN  int                 unit, /* input device */
    SOC_SAND_IN  uint32              cud,  /* input cud/outlif */
    SOC_SAND_OUT SOC_TMC_FAP_PORT_ID *port /* output (local egress) port */
);


/*
* Return the contents of the MCDB hardware entry, for reconstruction in case of SER.
*/
soc_error_t dpp_mult_get_entry(
    SOC_SAND_IN  int    unit,
    SOC_SAND_IN  uint32 mcdb_index,
    SOC_SAND_OUT uint32 *entry /* output MCDB entry */
);

/*
* Return array of indexes of the MCDB entries of a specific MC group and the size of the group.
*/
uint32
dpp_mcdb_index_get(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32 group_id, /* the mcid of the group */
    SOC_SAND_IN int is_egress, /* 1 if group is egress */
    SOC_SAND_OUT uint16 *group_size, /* the size of the group */
    SOC_SAND_OUT uint32 *index_core0, /* array of MCDB indexes for core 0 */
    SOC_SAND_OUT uint32 *index_core1 /* array of MCDB indexes for core 1 */
);
#endif /* __DPP_MULTICAST_IMP_H__ */

