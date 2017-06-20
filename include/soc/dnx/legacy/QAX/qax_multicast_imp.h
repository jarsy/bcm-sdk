/* $Id: jer2_qax_multicast_imp.h,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER2_QAX_MULTICAST_IMP_INCLUDED__
#define __JER2_QAX_MULTICAST_IMP_INCLUDED__

/*
 * This file contains the JER2_QAX,QUX multicast implementation definitions.
 * It "inherits" from include/soc/dnx/legacy/multicast.h .
 */

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/types.h>
#include <soc/dnx/legacy/multicast.h>
#include <soc/dnx/legacy/multicast_imp.h>
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>


/* } */
/**********************
 * DEFINES AND MACROS *
 *********************/
/* { */

#define JER2_QAX_NOF_GROUPS_PER_IRDB_ENTRY 32 /* number of groups in one CGM_IS_ING_MCm entry. */

#define JER2_QAX_ING_MC_NOF_DEST_BITS 17 /* number of bits in an ingress MC destination */
#define JER2_QAX_MC_CUD_NOF_BITS 18 /* number of bits in a MC CUD (both ingress and egress) */
#define JER2_QAX_MC_CUD_TOO_BIG (1 << JER2_QAX_MC_CUD_NOF_BITS) /* The first value to big to fit in a CUD */

/* encoding of additional memory in MCDB entries in memory - joint encoding used by Arad and Jericho */

/* The bits of word2 of a jer2_qax_mcdb_entry_t are used for:
 * 0-7: JER2_QAX hardware entry data.
 * 8-10: Reserved for later usage.
 * 11-11: A bit used only by testing code.
 * 12-28: For used entries: The previous entry of the group.
 * 29-31: Entry type. For egress non bitmap groups, need to restore after warmboot if the group was open or not (1 bit).
 *
 * For free entries we do not need to keep the (implied) hardware value of the entry, and we use these bits:
 * word1 bits  0-18: For the first entry of a free block this holds the first entry of the next free block in the list.
 * word0 bits  0-18: For the first entry of a free block this holds the first entry of the previous free block in the list.
 *                   For non first entries this holds the first entry in the free block.
 * word2 bits  0-3:  For the first entry of a free block this holds the size of the free block
 * word0,1 bits 19-31 are zero.
 *
 */

#define JER2_QAX_MC_ENTRY_SIZE 3 /* size of a MCDB entry in uint32s which must be equal to the size of jer2_qax_mcdb_entry_t */
/* mask of the hardware data bits in the msb word of an mcdb entry */
#define JER2_QAX_MC_ENTRY_MASK_VAL 0xff

#define JER2_QAX_MC_ENTRY_MASK_WORD(mcds, index) ((mcds)->mcdb[index].word2) /* The last word of MCDB in which to apply the mask and store data in free bits*/

#define JER2_QAX_MCDS_TYPE_SHIFT 29 /* The type is stored in the 3 msb bits */
#define JER2_QAX_NOF_MCDB_INDEX_BITS 17 /* number of bits in a JER2_QAX MCDB table index */
#define JER2_QAX_MCDB_INDEX_MASK ((1 << JER2_QAX_NOF_MCDB_INDEX_BITS) - 1)
#define JER2_QAX_MCDS_PREV_INDEX_SHIFT (JER2_QAX_MCDS_TYPE_SHIFT - JER2_QAX_NOF_MCDB_INDEX_BITS) /* prev entry index shift=12 */
#define JER2_QAX_MCDS_TEST_BIT_SHIFT (JER2_QAX_MCDS_PREV_INDEX_SHIFT -1) /* test bit index=11 */


#define JER2_QAX_MCDS_LAST_WORD_KEEP_BITS_MASK /* The bits to keep when copying/resetting a MCDB entry */ \
  (1 << JER2_QAX_MCDS_TEST_BIT_SHIFT)


/* get and set the entry type */
#define JER2_QAX_MCDS_ENTRY_GET_TYPE(entry) ((entry)->word2 >> JER2_QAX_MCDS_TYPE_SHIFT) /* assumes the usage of the msb bits */
#define JER2_QAX_MCDS_ENTRY_SET_TYPE(entry, type_value) /* assumes the usage of the msb bits */ \
    do {(entry)->word2 = ((entry)->word2 & ~(DNX_MCDS_TYPE_MASK << JER2_QAX_MCDS_TYPE_SHIFT)) | \
      ((type_value) << JER2_QAX_MCDS_TYPE_SHIFT); } while (0)
#define JER2_QAX_MCDS_GET_TYPE(mcds, index) JER2_QAX_MCDS_ENTRY_GET_TYPE((mcds)->mcdb + (index))
#define JER2_QAX_MCDS_SET_TYPE(mcds, index, type_value) JER2_QAX_MCDS_ENTRY_SET_TYPE((mcds)->mcdb + (index), (type_value))


/* get and set the previous entry index (the current entry index means no previous entry) */
#define JER2_QAX_MCDS_GET_PREV_ENTRY(mcds, index) \
  ((JER2_QAX_MC_ENTRY_MASK_WORD((mcds), (index)) >> JER2_QAX_MCDS_PREV_INDEX_SHIFT) & JER2_QAX_MCDB_INDEX_MASK)

#define JER2_QAX_MCDS_SET_PREV_ENTRY(mcds, index, prev_entry) \
  do { \
    JER2_QAX_MC_ENTRY_MASK_WORD((mcds), (index)) = (JER2_QAX_MC_ENTRY_MASK_WORD((mcds), (index)) & \
      ~(JER2_QAX_MCDB_INDEX_MASK << JER2_QAX_MCDS_PREV_INDEX_SHIFT)) | \
      ((prev_entry) << JER2_QAX_MCDS_PREV_INDEX_SHIFT); \
  } while(0);

#define JER2_QAX_MCDS_ENTRY_GET_PREV_ENTRY(entry, mcds, index) \
  ((((entry)->word2 >> JER2_QAX_MCDS_PREV_INDEX_SHIFT) & JER2_QAX_MCDB_INDEX_MASK) | (mcds)->prev_entries[index])
#define JER2_QAX_MCDS_ENTRY_SET_PREV_ENTRY(entry, prev_entry) \
  do { \
    (entry)->word2 = ((entry)->word2 & \
      ~(JER2_QAX_MCDB_INDEX_MASK << JER2_QAX_MCDS_PREV_INDEX_SHIFT)) | \
      ((prev_entry) << JER2_QAX_MCDS_PREV_INDEX_SHIFT); \
  } while(0);

/* Free blocks filds are assumed to start at bit 0 of the word */
#define JER2_QAX_MCDS_GET_FREE_NEXT_ENTRY(mcds, index) /* get the next free block start */ \
  ((mcds)->mcdb[index].word1)
#define JER2_QAX_MCDS_SET_FREE_NEXT_ENTRY(mcds, index, next_entry) /* set the next free block start */ \
  (mcds)->mcdb[index].word1 = next_entry
#define JER2_QAX_MCDS_ENTRY_GET_FREE_PREV_ENTRY(entry) /* get the prev free block start or start of this block */ \
  ((entry)->word0 & JER2_QAX_MCDB_INDEX_MASK)
#define JER2_QAX_MCDS_GET_FREE_PREV_ENTRY(mcds, index) /* get the prev free block start or start of this block */ \
  ((mcds)->mcdb[index].word0)
#define JER2_QAX_MCDS_SET_FREE_PREV_ENTRY(mcds, index, prev_entry) /* set the prev free block start or start of this block */ \
  (mcds)->mcdb[index].word0 = (prev_entry)
#define JER2_QAX_MCDS_GET_FREE_BLOCK_SIZE(mcds, index) /* get the block size, to be called for the first block entry */ \
  ((mcds)->mcdb[index].word2 & DNX_MCDS_FREE_BLOCK_SIZE_MASK)
#define JER2_QAX_MCDS_SET_FREE_BLOCK_SIZE(mcds, index, size) /* set the block size, to be called for the first block entry */ \
  (mcds)->mcdb[index].word2 = ((mcds)->mcdb[index].word2 & ~(JER2_QAX_MCDB_INDEX_MASK )) | (size)

/* Macros for setting/getting the test bit */
#define JER2_QAX_MCDS_ENTRY_GET_TEST_BIT(     entry)    (((entry)->word2 >> JER2_QAX_MCDS_TEST_BIT_SHIFT) & 1)
#define JER2_QAX_MCDS_ENTRY_SET_TEST_BIT_ON(  entry) do {(entry)->word2 |=  (1 << JER2_QAX_MCDS_TEST_BIT_SHIFT);} while (0)
#define JER2_QAX_MCDS_ENTRY_SET_TEST_BIT_OFF( entry) do {(entry)->word2 &= ~(1 << JER2_QAX_MCDS_TEST_BIT_SHIFT);} while (0)
#define JER2_QAX_MCDS_ENTRY_SET_TEST_BIT(entry, val) do {(entry)->word2 &= ~(1 << JER2_QAX_MCDS_TEST_BIT_SHIFT); (entry)->word2 |= (((val) & 1) << JER2_QAX_MCDS_TEST_BIT_SHIFT);} while (0)


#define JER2_QAX_MULT_MAX_INGRESS_REPLICATIONS 4094 /* maximum replications for an ingress multicast group */
#define JER2_QAX_MULT_MAX_EGRESS_REPLICATIONS 4095  /* maximum replications for an egress multicast group */
#define JER2_QAX_MULT_MAX_REPLICATIONS JER2_QAX_MULT_MAX_EGRESS_REPLICATIONS /* = max {DNX_MULT_MAX_INGRESS_REPLICATIONS, DNX_MULT_MAX_EGRESS_REPLICATIONS} */

#define JER2_QAX_MC_ING_DESTINATION_DISABLED 0x1ffff /* value of destination field that causes no replication */


/*
 * usage of jer2_qax_rep_data_t bits for representing JER2_QAX/QUX replications,
 * planned to sort replications in a way suitable for optimal usage in MCDB formats: 
 *
 * 0-16  destination: ingress destination, bitmap ID (not pointer), egress port
 * 17-34 1st CUD
 * 35-52 2nd CUD (if exists)
 * 53    replication type (if the destination is a bitmap or not)
 *
 * Sort order msb-lsb: replication type - 2nd CUD - 1st CUD - destination
 */

/* macros to access data inside jer2_qax_rep_data_t */

#define JER2_QAX_MCDS_REP_DATA_DEST_OFFSET 0
#define JER2_QAX_MCDS_REP_DATA_CUD1_OFFSET JER2_QAX_ING_MC_NOF_DEST_BITS
#define JER2_QAX_MCDS_REP_DATA_CUD2_OFFSET (JER2_QAX_MCDS_REP_DATA_CUD1_OFFSET + JER2_QAX_MC_CUD_NOF_BITS)
#define JER2_QAX_MCDS_REP_DATA_TYPE_OFFSET (JER2_QAX_MCDS_REP_DATA_CUD2_OFFSET + JER2_QAX_MC_CUD_NOF_BITS)
#define JER2_QAX_MCDS_REP_DATA_TYPE_SIZE 1

#define JER2_QAX_MCDS_REP_DATA_DEST_MASK ((1 << JER2_QAX_ING_MC_NOF_DEST_BITS) - 1)
#define JER2_QAX_MCDS_REP_DATA_CUD_MASK ((1 << JER2_QAX_MC_CUD_NOF_BITS) - 1)
#define JER2_QAX_MCDS_REP_DATA_CUD1_LSB_MASK (JER2_QAX_MCDS_REP_DATA_CUD_MASK << JER2_QAX_MCDS_REP_DATA_CUD1_OFFSET)
#define JER2_QAX_MCDS_REP_DATA_CUD1_MSB_MASK (JER2_QAX_MCDS_REP_DATA_CUD_MASK >> (32 - JER2_QAX_MCDS_REP_DATA_CUD1_OFFSET))
#define JER2_QAX_MCDS_REP_DATA_CUD2_LSB_MASK 0
#define JER2_QAX_MCDS_REP_DATA_CUD2_MSB_MASK (JER2_QAX_MCDS_REP_DATA_CUD_MASK << (JER2_QAX_MCDS_REP_DATA_CUD2_OFFSET - 32))
#define JER2_QAX_MCDS_REP_DATA_TYPE_MASK ((1 << JER2_QAX_MCDS_REP_DATA_TYPE_SIZE) - 1)
#define JER2_QAX_MCDS_REP_DATA_TYPE_LSB_MASK 0
#define JER2_QAX_MCDS_REP_DATA_TYPE_MSB_MASK (JER2_QAX_MCDS_REP_DATA_TYPE_MASK << (JER2_QAX_MCDS_REP_DATA_TYPE_OFFSET - 32))

/* the SET,RESET macros set fields, while SET assume that the previous field value is 0 */
#define JER2_QAX_MCDS_REP_DATA_SET_DEST(data, dest) do {\
    uint64 _repdata_temp;\
    COMPILER_64_SET(_repdata_temp, 0, (dest));\
    COMPILER_64_OR(*(data), _repdata_temp);\
} while (0)
#define JER2_QAX_MCDS_REP_DATA_RESET_DEST(data, dest) do {\
    uint64 _repdata_temp, _repdata_temp2;\
    COMPILER_64_SET(_repdata_temp, ~0, ~JER2_QAX_MCDS_REP_DATA_DEST_MASK);\
    COMPILER_64_SET(_repdata_temp2, 0, (dest));\
    COMPILER_64_AND(*(data), _repdata_temp);\
    COMPILER_64_OR(*(data), _repdata_temp2);\
} while (0)
#define JER2_QAX_MCDS_REP_DATA_GET_DEST(data, dest) do {\
    (dest) = COMPILER_64_LO((*data)) & JER2_QAX_MCDS_REP_DATA_DEST_MASK;\
} while (0)

#define JER2_QAX_MCDS_REP_DATA_SET_CUD1(data, cud) do {\
    uint64 _repdata_temp;\
    COMPILER_64_SET(_repdata_temp, 0, (cud));\
    COMPILER_64_SHL(_repdata_temp, JER2_QAX_MCDS_REP_DATA_CUD1_OFFSET);\
    COMPILER_64_OR(*(data), _repdata_temp);\
} while (0)
#define JER2_QAX_MCDS_REP_DATA_RESET_CUD1(data, cud) do {\
    uint64 _repdata_temp, _repdata_temp2;\
    COMPILER_64_SET(_repdata_temp, ~JER2_QAX_MCDS_REP_DATA_CUD1_MSB_MASK, ~JER2_QAX_MCDS_REP_DATA_CUD1_LSB_MASK);\
    COMPILER_64_SET(_repdata_temp2, 0, (cud));\
    COMPILER_64_SHL(_repdata_temp2, JER2_QAX_MCDS_REP_DATA_CUD1_OFFSET);\
    COMPILER_64_AND(*(data), _repdata_temp);\
    COMPILER_64_OR(*(data), _repdata_temp2);\
} while (0)
#define JER2_QAX_MCDS_REP_DATA_GET_CUD1(data, cud) do {\
    uint64 _repdata_temp;\
    COMPILER_64_COPY(_repdata_temp, (*data));\
    COMPILER_64_SHR(_repdata_temp2, JER2_QAX_MCDS_REP_DATA_CUD1_OFFSET);\
    (cud) = COMPILER_64_LO(_repdata_temp2) & JER2_QAX_MCDS_REP_DATA_CUD_MASK;\
} while (0)

#define JER2_QAX_MCDS_REP_DATA_SET_CUD2(data, cud) do {\
    uint64 _repdata_temp;\
    COMPILER_64_SET(_repdata_temp, (cud) << (JER2_QAX_MCDS_REP_DATA_CUD2_OFFSET - 32), 0);\
    COMPILER_64_OR(*(data), _repdata_temp);\
} while (0)
#define JER2_QAX_MCDS_REP_DATA_RESET_CUD2(data, cud) do {\
    uint64 _repdata_temp, _repdata_temp2;\
    COMPILER_64_SET(_repdata_temp, ~JER2_QAX_MCDS_REP_DATA_CUD2_MSB_MASK, ~JER2_QAX_MCDS_REP_DATA_CUD2_LSB_MASK);\
    COMPILER_64_SET(_repdata_temp2, (cud) << (JER2_QAX_MCDS_REP_DATA_CUD2_OFFSET - 32), 0);\
    COMPILER_64_AND(*(data), _repdata_temp);\
    COMPILER_64_OR(*(data), _repdata_temp2);\
} while (0)
#define JER2_QAX_MCDS_REP_DATA_GET_CUD2(data, cud) do {\
    uint64 _repdata_temp;\
    COMPILER_64_COPY(_repdata_temp, (*data));\
    COMPILER_64_SHR(_repdata_temp2, JER2_QAX_MCDS_REP_DATA_CUD2_OFFSET);\
    (cud) = COMPILER_64_LO(_repdata_temp2) & JER2_QAX_MCDS_REP_DATA_CUD_MASK;\
} while (0)

#define JER2_QAX_MCDS_REP_DATA_SET_TYPE(data, rep_type) do {\
    uint64 _repdata_temp;\
    COMPILER_64_SET(_repdata_temp, (rep_type) << (JER2_QAX_MCDS_REP_DATA_TYPE_OFFSET - 32), 0);\
    COMPILER_64_OR(*(data), _repdata_temp);\
} while (0)
#define JER2_QAX_MCDS_REP_DATA_RESET_TYPE(data, rep_type) do {\
    uint64 _repdata_temp, _repdata_temp2;\
    COMPILER_64_SET(_repdata_temp, ~JER2_QAX_MCDS_REP_DATA_TYPE_MSB_MASK, ~JER2_QAX_MCDS_REP_DATA_TYPE_LSB_MASK);\
    COMPILER_64_SET(_repdata_temp2, (rep_type) << (JER2_QAX_MCDS_REP_DATA_TYPE_OFFSET - 32), 0);\
    COMPILER_64_AND(*(data), _repdata_temp);\
    COMPILER_64_OR(*(data), _repdata_temp2);\
} while (0)
#define JER2_QAX_MCDS_REP_DATA_GET_TYPE(data, rep_type) do {\
    uint64 _repdata_temp;\
    COMPILER_64_COPY(_repdata_temp, (*data));\
    COMPILER_64_SHR(_repdata_temp, JER2_QAX_MCDS_REP_DATA_TYPE_OFFSET);\
    (rep_type) = COMPILER_64_LO(_repdata_temp) & JER2_QAX_MCDS_REP_DATA_TYPE_MASK;\
} while (0)

#define JER2_QAX_MCDS_REP_DATA_GET_FIELDS(data, dest, cud1, cud2, rep_type) do {\
    uint64 _repdata_temp;\
    COMPILER_64_COPY(_repdata_temp, (*data));\
    (dest) = COMPILER_64_LO((*data)) & JER2_QAX_MCDS_REP_DATA_DEST_MASK;\
    COMPILER_64_SHR(_repdata_temp, JER2_QAX_MCDS_REP_DATA_CUD1_OFFSET);\
    (cud1) = COMPILER_64_LO(_repdata_temp) & JER2_QAX_MCDS_REP_DATA_CUD_MASK;\
    COMPILER_64_SHR(_repdata_temp, JER2_QAX_MCDS_REP_DATA_CUD2_OFFSET - JER2_QAX_MCDS_REP_DATA_CUD1_OFFSET);\
    (cud2) = COMPILER_64_LO(_repdata_temp) & JER2_QAX_MCDS_REP_DATA_CUD_MASK;\
    COMPILER_64_SHR(_repdata_temp, JER2_QAX_MCDS_REP_DATA_TYPE_OFFSET - JER2_QAX_MCDS_REP_DATA_CUD2_OFFSET);\
    (rep_type) = COMPILER_64_LO(_repdata_temp) & JER2_QAX_MCDS_REP_DATA_TYPE_MASK;\
} while (0)

#define JER2_QAX_MCDS_REP_DATA_CLEAR(data) do {COMPILER_64_ZERO((*data));} while (0)

#define JER2_QAX_MCDS_REP_TYPE_DEST 0
#define JER2_QAX_MCDS_REP_TYPE_BM 1


/* get the MCDB index of the start of the egress linked list of the given MCID */
#define JER2_QAX_MCDS_GET_EGRESS_GROUP_START(mcds, mcid) ((mcds)->egress_mcdb_offset + (mcid))

#define JER2_QAX_GET_MCDB_ENTRY(mcds, index) ((mcds)->mcdb + (index))

#define JER2_QAX_LAST_MCDB_ENTRY (0x17fff)

#define JER2_QAX_NO_MCDB_INDEX ((uint32)(-1))

#define JER2_QAX_MC_INGRESS_LINK_PTR_END (0x1ffff) /* marks the end of an ingress linked list, the last entry in the MCDB */

/* values for the state field of jer2_qax_mcds_t */
#define JER2_QAX_MCDS_STATE_INITED 1 /* linked list creation data structures were initialized */
#define JER2_QAX_MCDS_STATE_STARTED 2 /* linked list wrote linked list entries */
#define JER2_QAX_MCDS_STATE_FINISHED 3 /* linked list creation finished successfully */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

typedef uint64 jer2_qax_rep_data_t; /* type containing the type and data of any JER2_QAX replication */

/*
 * This structure contains one MCDB entry (72 bits), and other data in the remaining bits.
 * For used MCDB entries the hardware data bits always match (cache) the hardware.
 * For unused MCDB entries who have a constant hardware value, the data bits are used for other software purposes
 *
 * We assume that we can access this entry as a uint32 array[3] for memory field access functions.
 */
typedef struct
{
  uint32 word0;
  uint32 word1;
  uint32 word2;
} jer2_qax_mcdb_entry_t;

typedef struct {
    dnx_mcds_common_t common; /* This member must be the first to implement the polymorphism */
    uint32 nof_unoccupied; /* number of free entries in the mcdb */

    /* The next two entries are both either null or an array of size DNX_LAST_MCDB_ENTRY(mcds)+1 */
    jer2_qax_mcdb_entry_t *mcdb;    /* mcdb cache, free list, and other data in the free bits */
    jer2_qax_mcdb_entry_t free_value; /* The value of a free entry, also the value of the start of an empty egress linked list. */
    jer2_qax_mcdb_entry_t empty_ingr_value; /* The value of the start of an empty ingress linked list. */
    uint32 egress_mcdb_offset;   /* The offset in the MCDB to which the MCID is added to get the first entry of the group */
    uint32 ingress_bitmap_start; /* The start of the MCDB entries reserved for ingress bitmaps and the first entry that can't be used for linked lists */
    uint32 egress_bitmap_start;  /* The start of the MCDB entries reserved for egress bitmaps */
    int unit; /* The device unit of the mcds */
    dnx_free_entries_blocks_region_t no_starts; /* free lists of entries that that may not start a linked list */
    dnx_free_entries_blocks_region_t ingress_starts; /* free lists of entries that may start an ingress linked list */
    dnx_free_entries_blocks_region_t egress_starts; /* free lists of entries that may start an ingress linked list */

#ifdef DONOT_USE_SW_DB_FOR_MULTICAST
    uint32 *egress_groups_open_data; /* replicate the information of is each egress group, in warm boot it is stored to the warm boot file */
#endif

    /*
     * All remaining members are only used internally inside one API,
     * to describe its state like used replications and linked list being created.
     */

    /* The egress replication destination ports are TM/PP ports */
    jer2_qax_rep_data_t reps[JER2_QAX_MULT_MAX_REPLICATIONS]; /* used to store replication data of a group */

    jer2_qax_mcdb_entry_t *cur_entry;  /* A pointer to the current linked list entry being added */
    jer2_qax_mcdb_entry_t start_entry; /* the contents of the first entry of the linked list to be created */
    uint32 cur_entry_index;       /* The index of the current linked list entry being added */
    /*
     * The linked list are built in the MCDB from the end to the start.
     * First the first entry of the group is stored in start_entry, and then the linked list
     * is built from what will be the end of the linked list towards its start.
     * The interim list list from start to end is: list_start -> to be created ->linked_list->created entries->linked_list_end->list_end
     */
    uint32 list_start;        /* the MCDB index of the first entry in the linked list (when creating all of it), or of the previous entry before the created linked list */
    uint32 list_end;          /* the MCDB index pointed to by the last entry of the linked list (provided as input). */
    uint32 linked_list;       /* the MCDB index of the first entry in the linked list created so far (excluding the first entry of the group */
    uint32 linked_list_end;   /* the MCDB index of the last entry in the created linked list (which is the second entry that is added after the first entry of the group */

    uint32 alloc_flags;       /* flags for free block allocation - depend on ingress/egress operation */
    uint32 hw_end;            /* the index marking the end of a linkes list in hardware: JER2_QAX_MC_INGRESS_LINK_PTR_END/DNX_MC_EGRESS_LINK_PTR_END */
    uint32 block_start;       /* MCDB index of the current allocated free block */
    DNX_TMC_ERROR out_err;    /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */

    uint16 nof_reps;          /* used to store replication data of a group */
    uint16 nof_bitmap_reps;   /* number of replications of type bitmap + CUD (or two CUDs for ingress MC) */
    uint16 nof_dest_cud_reps; /* number of replications of type port + one/two CUDs */
    uint16 nof_possible_reps; /* The number of possible replications in the group (assuming bitmaps replicate to all ports), used to check passing passing the supported number of replications */
    uint8 group_type;         /* group type: DNX_MCDS_TYPE_VALUE_INGRESS/EGRESS */
    uint8 group_type_start;   /* start group type: DNX_MCDS_TYPE_VALUE_{IN,E}GRESS_START */
    uint8 writing_full_list;  /* boolean: are we writing the full linked list of the MC group (and not part of the linked list) */
    uint8 is_group_start_to_be_filled; /* boolean: Does start_entry need to be filled */
    uint8 state;              /* The state of the processing of the current linked list entry, used for cleanup in case of failures */
    dnx_free_entries_block_size_t block_size; /* size of current MCDB free block allocated */

} jer2_qax_mcds_t; /* JER2_QAX/QUX MCDS structure */

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

#ifdef JER2_QAX_TO_BE_IMPLEMENTED 
/*
 * Get the (pointer to the) next entry from the given entry.
 * The entry type (ingress/egress/egress) TDM is given as an argument.
 */
uint32 jer2_qax_mcdb_get_next_pointer(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  uint32  entry,      /* entry from which to get the next entry pointer */
    DNX_SAND_IN  uint32  entry_type, /* the type of the entry */
    DNX_SAND_OUT uint32  *next_entry /* the output next entry */
);

/*
 * Set the pointer to the next entry in the given entry.
 * The entry type (ingress/egress/egress) TDM is given as an argument.
 * changes both the mcds and hardware.
 */
uint32 jer2_qax_mcdb_set_next_pointer(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  uint32  entry_to_set, /* index of the entry in which to set the pointer */
    DNX_SAND_IN  uint32  entry_type,   /* the type of entry_to_set */
    DNX_SAND_IN  uint32  next_entry    /* the entry that entry_to_set will point to */
);

/*
 * Adds the contents of a mcdb entry of the given type to the mcds buffer.
 * No more than *max_size replications are added, and the max_size value
 * is decreased by the number of added replications.
 * *group_size is increased by the number of found replications.
 * The next entry pointed to by this entry is returned in next_entry.
 */
uint32 jer2_qax_mcds_get_replications_from_entry(
    DNX_SAND_IN    int     unit,
    DNX_SAND_IN    int     core,        /* relevant when hardware has a per core linked list */
    DNX_SAND_IN    uint8   get_bm_reps, /* Should the function return bitmap replications (if non zero) or ignore them */
    DNX_SAND_IN    uint32  entry_index, /* table index of the entry */
    DNX_SAND_IN    uint32  entry_type,  /* the type of the entry */
    DNX_SAND_INOUT uint32  *cud2,       /* the current 2nd CUD of replications */
    DNX_SAND_INOUT uint16  *max_size,   /* the maximum number of replications to return from the group, decreased by the number of returned replications */
    DNX_SAND_INOUT uint16  *group_size, /* incremented by the number of found replications (even if they are not returned) */
    DNX_SAND_OUT   uint32  *next_entry  /* the next entry */
);

/*
 * sets an ingress replication in BCM API encoding from the given CUD and destination in hardware encoding.
 */
uint32 jer2_qax_convert_ingress_replication_hw2api(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  uint32       cud,            /* CUD to be converted */
    DNX_SAND_IN  uint32       dest,           /* destination to be converted */
    DNX_SAND_OUT soc_gport_t  *port_array,    /* output array to contain ports/destinations */
    DNX_SAND_OUT soc_if_t     *encap_id_array /* output array to contain encapsulations/CUDs/outlifs */
);

/*
 * This function encodes the destination and the CUD as will be written to the hardware fields of IRR_MCDB.
 * The input is the replication data (destination structure and outlif).
 * It is permitted to call the functions to change its input with these
 * arguments: unit, entry, &entry->destination.id, &entry->cud .
 * No locking is needed for this function.
 */
uint32 jer2_qax_mult_ing_encode_entry(
    DNX_SAND_IN    int                    unit,
    DNX_SAND_IN    DNX_TMC_MULT_ING_ENTRY *ing_entry,       /* replication data */
    DNX_SAND_OUT   uint32                 *out_destination, /* the destination field */
    DNX_SAND_OUT   uint32                 *out_cud          /* the CUD/outlif field */
  );
#endif /* JER2_QAX_TO_BE_IMPLEMENTED */ 


/*
 * Write a MCDB entry to hardware from the mcds.
 * Using only this function for writes, and using it after mcds mcdb used
 * entries updates, ensures consistency between the mcds and the hardware.
 */

uint32
  jer2_qax_mcds_write_entry(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 mcdb_index /* index of entry to write */
);


/*
 * Get, set, increase and decrease the stored number of free entries
 */
uint32 jer2_qax_mcds_unoccupied_get(
    DNX_SAND_IN jer2_qax_mcds_t *mcds
);


/*
 * This functions copies the replication data from the given replication array into the mcds.
 * It is an error if the mcds is filled beyond the maximum size of a multicast group.
 * We currently assume that the destination/port translation is done by bcm code before calling this function.
 * The function also converts each logical port to core + TM port in Jericho.
 */
uint32 jer2_qax_mcds_copy_replications_from_arrays(
    DNX_SAND_IN  int       unit,
    DNX_SAND_IN  uint8     is_egress,       /* are the replications for an egress multicast group (opposed to ingress) */
    DNX_SAND_IN  uint8     do_clear,        /* if zero, replications will be added in addition to existing ones, otherwise previous replications will be cleared */
    DNX_SAND_IN  uint32    arrays_size,     /* size of output arrays */
    DNX_SAND_IN  dnx_mc_replication_t *reps /* input array containing replications using logical ports */
);

/*
 * This functions copies the replication data from the mcds into the given gport and encap_id arrays.
 * It is an error if more entries are to be copied than available in the arrays.
 */
uint32 jer2_qax_mcds_copy_replications_to_arrays(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  uint8        is_egress,           /* are the replications for an egress multicast group (opposed to ingress) */
    DNX_SAND_IN  uint32       arrays_size,         /* size of output arrays */
    DNX_SAND_OUT soc_gport_t  *port_array,         /* output array to contain logical ports/destinations, used if !reps */
    DNX_SAND_OUT soc_if_t     *encap_id_array,     /* output array to contain encapsulations/CUDs/outlifs, used if !reps */
    DNX_SAND_OUT soc_multicast_replication_t *reps /* output replication array (array of size mc_group_size*/
);


/*
 * Free blocks handling functions specific to JER2_QAX.
 */

/*
 * Get the region corresponding to the given mcdb index
 */
dnx_free_entries_blocks_region_t* jer2_qax_mcds_get_region(jer2_qax_mcds_t *mcds, uint32 mcdb_index);

/*
 * Add free entries in the given range as blocks to the lists of the given region.
 * If (check_free) add only blocks marked as free.
 * Otherwise add all entries in the range are expected to be marked used and they will be marked as free.
 */
uint32 jer2_qax_mcds_build_free_blocks(
    DNX_SAND_IN    int                              unit,   /* used only if check_free is zero */
    DNX_SAND_INOUT jer2_qax_mcds_t                       *mcds,
    DNX_SAND_IN    uint32                           start_index, /* start index of the range to work on */
    DNX_SAND_IN    uint32                           end_index,   /* end index of the range to work on, if smaller than start_index then do nothing */
    DNX_SAND_INOUT dnx_free_entries_blocks_region_t *region,     /* region to contain the block in its lists */
    DNX_SAND_IN    dnx_mcds_free_build_option_t         entry_option /* which option to use in selecting entries to add and verifying them */
);

/*
 * Get a free entries block of a given size, according to flags that needs to be used to start a multicast group.
 * Returns the start index and the number of entries in the block.
 */
uint32 jer2_qax_mcds_get_free_entries_block(
    DNX_SAND_INOUT jer2_qax_mcds_t                    *mcds,
    DNX_SAND_IN    uint32                        flags,        /* DNX_MCDS_GET_FREE_BLOCKS_* flags that affect what the function does group */
    DNX_SAND_IN    dnx_free_entries_block_size_t wanted_size,  /* needed size of the free block group */
    DNX_SAND_IN    dnx_free_entries_block_size_t max_size,     /* do not return blocks above this size, if one would have been returned, split it */
    DNX_SAND_OUT   uint32                        *block_start, /* the start of the relocation block */
    DNX_SAND_OUT   dnx_free_entries_block_size_t *block_size   /* the size of the returned block */
);

/*
 * Get a free block of size 1 at a given location.
 * Used for getting the first entry of a multicast group.
 * Does not mark mcdb_index as used.
 */
uint32 jer2_qax_mcds_reserve_group_start(
    DNX_SAND_INOUT jer2_qax_mcds_t *mcds,
    DNX_SAND_IN    uint32           mcdb_index /* the mcdb index to reserve */
);


/*
 * Initialize the MultiCast Data Structures.
 * Do not fill the data from hardware yet.
 * jer2_qax_mcds_multicast_init2() will be called to do so when we can access the MCDB using DMA.
 */
uint32
    jer2_qax_mcds_multicast_init(
      DNX_SAND_IN int unit
);

/*
 * Initialize the MultiCast Data Structures.
 * Must be run after jer2_qax_mcds_multicast_init() was called successfully, and when DMA is up.
 * fills the multicast data from hardware.
 */
uint32
    jer2_qax_mcds_multicast_init2(
      DNX_SAND_IN int unit
);

/*
 * De-initialize the MultiCast Data Structures.
 */
uint32
    jer2_qax_mcds_multicast_terminate(
        DNX_SAND_IN int unit
    );

/*
 * This function checks if the multicast group is open (possibly empty).
 * returns TRUE if the group is open (start of a group), otherwise FALSE.
 */
uint32 jer2_qax_mult_does_group_exist(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_MULT_ID mcid,       /* MC ID of the group */
    DNX_SAND_IN  int             is_egress,  /* is the MC group an egress group */
    DNX_SAND_OUT uint8           *does_exist /* output: if the group exists */
);


/* function to compare replications, assumes the comparison function used by dnx_sand_os_qsort does not require negatice return values  */
int jer2_qax_rep_data_t_compare(void *a, void *b);

/* compare two replications, and return 0 if they are not exactly the same, non zero otherwise */
#define JER2_QAX_EQ_REP_DATA(rep1, rep2) COMPILER_64_EQ(*(rep1), *(rep2))

/*
 * Relocate the given used entries, not disturbing multicast traffic to
 * the group containing the entries.
 * SWDB and hardware are updated accordingly.
 * If relocation_block_size is 0, then the function calculates
 * relocation_block_start and relocation_block_size by itself to suit mcdb_index.
 * After successful relocation, the block that was relocated is freed, except for entry mcdb_index.
 *
 * This function may overwrite the mc group replications stored in the mcds.
 * Returns possible errors that the caller may want to ignore in mcds->out_err: insufficient memory.
 */

uint32
  jer2_qax_mcdb_relocate_entries(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  uint32                        mcdb_index,             /* table index needed for the start of a group */
    DNX_SAND_IN  uint32                        relocation_block_start, /* the start of the relocation block */
    DNX_SAND_IN  dnx_free_entries_block_size_t relocation_block_size   /* the size of the relocation block */
);


/*
 * Free a linked list that is not used any more, updating mcds and hardware.
 * Stop at the given entry, and do not free it.
 * The given linked list must not include the first entry of the group.
 * All entries in the linked list should be of the given type.
 */

uint32
  jer2_qax_mcdb_free_linked_list_till_my_end(
    DNX_SAND_IN int    unit,
    DNX_SAND_IN uint32 first_index,  /* table index of the first entry in the linked list to free */
    DNX_SAND_IN uint32 entries_type, /* the type of the entries in the list */
    DNX_SAND_IN uint32 end_index     /* the index of the end of the list to be freed */
);

/*
 * Free a linked list that is not used any more, updating mcds and hardware.
 * The given linked list must not include the first entry of the group.
 * All entries in the linked list should be of the given type.
 */

uint32
  jer2_qax_mcdb_free_linked_list(
    DNX_SAND_IN int     unit,
    DNX_SAND_IN uint32  first_index, /* table index of the first entry in the linked list to free */
    DNX_SAND_IN uint32  entries_type /* the type of the entries in the list */
);

/*
 * Return the contents of a multicast linked list group of a given type.
 * The contents is returned in the mcds buffer. Egress destination ports are TM/PP ports.
 * if the group size is bigger than the specified max_size, only max_size entries are returned,
 * it will not be an error, and the actual size of the group is returned.
 * The group must be open.
 */

uint32 jer2_qax_mcds_get_group(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  uint8                do_clear,     /* if zero, replications will be added in addition to existing ones, otherwise previous replications will be cleared */
    DNX_SAND_IN  uint32               group_id,     /* the mcid of the group */
    DNX_SAND_IN  uint32               group_type,   /* the type of the group (of the entries in the list) */
    DNX_SAND_IN  uint16               max_size,     /* the maximum number of members to return from the group */
    DNX_SAND_OUT uint16               *group_size   /* the returned actual group size */
);

/*
 * Remove the given replications from the given linked list MC group, by changing existing entries in the linked list.
 * Does not allocate MCDB entries, so does not fail due to the MCDB being too full.
 * Leaves non optimal linked list.
 * Does not change the MC group at once when needing to change multiple entries.
 * then remove the replication from the linked list. If it is the only
 * replication in the linked list, the entry is removed from the group.
 * Does not support the TDM format.
 */
uint32 jer2_qax_mcds_remove_replications_from_group(
    DNX_SAND_IN int                  unit,       /* device */
    DNX_SAND_IN dnx_mc_id_t          group_mcid, /* group mcid */
    DNX_SAND_IN uint32               nof_reps,   /* number of replications to remove */
    DNX_SAND_IN dnx_mc_replication_t *reps       /* input array containing replications to remove */
);

/*
 * This functions removes the given replications from the mcds.
 * It is an error if the mcds is filled beyond the maximum size of a multicast group.
 * We currently assume that the destination/port translation is done by bcm code before calling this function.
 */
uint32 jer2_qax_mult_remove_replications(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  uint32               nof_reps,   /* number of replications to remove */
    DNX_SAND_IN  dnx_mc_replication_t *reps       /* input array containing replications to remove */
);

/*********************************************************************
* Initialize MC replication database
* The initialization accesses the replication table as if it was an
* Ingress replication, for all entries (including Egress MC)
**********************************************************************/
uint32 jer2_qax_mult_rplct_tbl_entry_unoccupied_set_all(
    DNX_SAND_IN  int unit
);


/*
 * Ingres multicast MBCM functions
 */

/*
 * This API sets the ingress group to the given replications,
 * configuring its linked list; and creates the group if it did not exist.
 */
uint32 jer2_qax_mult_ing_group_open(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  dnx_mc_id_t            multicast_id_ndx, /* group mcid */
    DNX_SAND_IN  DNX_TMC_MULT_ING_ENTRY *mc_group,        /* group replications to set */
    DNX_SAND_IN  uint32                 mc_group_size,    /* number of group replications (size of mc_group) */
    DNX_SAND_OUT DNX_TMC_ERROR          *out_err          /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
);

/*
 * Closes the ingress multicast group, freeing its linked list.
 * Do nothing if the group is not open.
 */
uint32 jer2_qax_mult_ing_group_close(
    DNX_SAND_IN  int            unit,
    DNX_SAND_IN  dnx_mc_id_t      multicast_id_ndx /* group mcid to close */
);

/*
 * This API sets the ingress group to the given replications, configuring its linked list.
 * The group must exist.
 */
uint32 jer2_qax_mult_ing_group_update(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  dnx_mc_id_t          multicast_id_ndx,    /* group mcid */
    DNX_SAND_IN  DNX_TMC_MULT_ING_ENTRY   *mc_group,           /* group replications to set */
    DNX_SAND_IN  uint32                mc_group_size,       /* number of group replications (size of mc_group) */
    DNX_SAND_OUT DNX_TMC_ERROR         *out_err             /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
);

/*
 * Adds the given replications to the existing ingress multicast group.
 * It is an error if the group is not open.
 */
uint32 jer2_qax_mult_ing_add_replications(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  dnx_mc_id_t                 mcid,     /* group mcid */
    DNX_SAND_IN  uint32                      nof_reps, /* number of replications to add */
    DNX_SAND_IN  soc_multicast_replication_t *reps,    /* input array containing replications to add */
    DNX_SAND_OUT DNX_TMC_ERROR               *out_err  /* return possible errors that the caller may want to ignore : insufficient memory or duplicate replication */
);

/*
 * Removes the given replications from the existing ingress multicast group.
 * It is an error if the group is not open or does not contain the replication.
 */
uint32 jer2_qax_mult_ing_remove_replications(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  dnx_mc_id_t                 mcid,     /* group mcid */
    DNX_SAND_IN  uint32                      nof_reps, /* number of replications to remove */
    DNX_SAND_IN  soc_multicast_replication_t *reps,    /* input array containing replications to add */
    DNX_SAND_OUT DNX_TMC_ERROR               *out_err  /* return possible errors that the caller may want to ignore : replication does not exist */
);

/*********************************************************************
*     Returns the size of the multicast group with the
*     specified multicast id.
*********************************************************************/
uint32 jer2_qax_mult_ing_group_size_get(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  dnx_mc_id_t multicast_id_ndx,
    DNX_SAND_OUT uint32      *mc_group_size
);

/*
 * Gets the ingress multicast group with the specified multicast id.
 * will return up to mc_group_size replications, and the exact
 * The group's replication number is returned in exact_mc_group_size.
 * The number of replications returned in the output arrays is
 * min{mc_group_size, exact_mc_group_size}.
 * It is not an error if the group is not open.
 */
uint32 jer2_qax_mult_ing_get_group(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  dnx_mc_id_t group_mcid,           /* group id */
    DNX_SAND_IN  uint32      mc_group_size,        /* maximum replications to return */
    DNX_SAND_OUT soc_gport_t *ports,               /* output ports (array of size mc_group_size) */
    DNX_SAND_OUT soc_if_t    *cuds,                /* output ports (array of size mc_group_size) */
    DNX_SAND_OUT uint32      *exact_mc_group_size, /* the number of replications in the group will be returned here */
    DNX_SAND_OUT uint8       *is_open              /* will return if the group is open (false or true) */
);

/*
 * This (MBCM) function encodes the destination and the CUD as will be written to the hardware fields of IRR_MCDB.
 * The input is the replication data (destination structure and outlif).
 * It is permitted to call the functions to change its input with these
 * arguments: unit, entry, &entry->destination.id, &entry->cud .
 * No locking is needed for this function.
 */
uint32 jer2_qax_mult_ing_encode_entry(
    DNX_SAND_IN    int                    unit,
    DNX_SAND_IN    DNX_TMC_MULT_ING_ENTRY *ing_entry,       /* replication data */
    DNX_SAND_OUT   uint32                 *out_destination, /* the destination field */
    DNX_SAND_OUT   uint32                 *out_cud          /* the CUD/outlif field */
);



/*
 * egress MC functions
 */

/*
 * Sets the egress group to the given replications configuring its linked list.
 * If the group does not exist, it will be created or an error will be returned based on allow_create.
 * Creation may involve relocating the MCDB entry which will be the start
 * of the group, and possibly other consecutive entries.
 *
 * We always want to create entries with pointers from port+outlif couples and from bitmaps.?????
 * We need to leave one entry with a pointer for the start of the group.
 * every block of entries with no pointers ends with an entry pointer, except for the end of the group.
 */
uint32 jer2_qax_mult_eg_group_set(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  dnx_mc_id_t          mcid,         /* the group mcid */
    DNX_SAND_IN  uint8                allow_create, /* if non zero, will create the group if it does not exist */
    DNX_SAND_IN  uint32               group_size,   /* size of ports and cuds to read group replication data from */
    DNX_SAND_IN  dnx_mc_replication_t *reps,        /* input array containing replications (using logical ports) */
    DNX_SAND_OUT DNX_TMC_ERROR        *out_err      /* return possible errors that the caller may want to ignore */
);

/*
 * This API closes an egress-multicast-replication group for the given multicast-id.
 * The user only specifies the multicast-id.
 * All inner link-list/bitmap nodes are freed and handled by the driver.
 */
uint32 jer2_qax_mult_eg_group_close(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  dnx_mc_id_t mcid
);

/*
 * Adds the given replications to the egress multicast group.
 * It is an error if the group is not open.
 */
uint32 jer2_qax_mult_eg_reps_add(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  dnx_mc_id_t           group_mcid, /* group mcid */
    DNX_SAND_IN  uint32                nof_reps,   /* number of replications to add */
    DNX_SAND_IN  dnx_mc_replication_t  *reps,      /* input array containing replications to add*/
    DNX_SAND_OUT DNX_TMC_ERROR         *out_err    /* return possible errors that the caller may want to ignore */
);

/*
 * Removes the given replication from the non bitmap egress multicast group.
 * It is an error if the group is not open or does not contain the replication.
 */
uint32 jer2_qax_mult_eg_reps_remove(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  dnx_mc_id_t           group_mcid,   /* group mcid */
    DNX_SAND_IN  uint32                nof_reps,     /* number of replications to remove */
    DNX_SAND_IN  dnx_mc_replication_t  *reps,        /* input array containing replications to remove */
    DNX_SAND_OUT DNX_TMC_ERROR         *out_err      /* return possible errors that the caller may want to ignore */
);

/*
 * Returns the size of the multicast group with the specified multicast id.
 * Not needed for bcm APIs, so not tested.
 * returns 0 for non open groups.
 */
uint32 jer2_qax_mult_eg_group_size_get(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  dnx_mc_id_t  multicast_id_ndx,
    DNX_SAND_OUT uint32       *mc_group_size
);

/*
 * Gets the egress multicast group with the specified multicast id.
 * will return up to mc_group_size replications, and the exact group size.
 * The group's replications number is returned in exact_mc_group_size.
 * The number of replications returned in the output arrays is
 * min{mc_group_size, exact_mc_group_size}.
 * It is not an error if the group is not open.
 */
uint32 jer2_qax_mult_eg_get_group(
    DNX_SAND_IN  int            unit,
    DNX_SAND_IN  dnx_mc_id_t    group_mcid,           /* group id */
    DNX_SAND_IN  uint32         mc_group_size,        /* maximum replications to return */
    DNX_SAND_OUT soc_gport_t    *ports,               /* output logical ports (array of size mc_group_size) used if !reps */
    DNX_SAND_OUT soc_if_t       *cuds,                /* output CUDs (array of size mc_group_size) used if !reps */
    DNX_SAND_OUT soc_multicast_replication_t *reps,   /* output replication array (array of size mc_group_size*/
    DNX_SAND_OUT uint32         *exact_mc_group_size, /* the number of replications in the group will be returned here */
    DNX_SAND_OUT uint8          *is_open              /* will return if the group is open (false or true) */
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
 *
 * JER2_QAX egress MC has four formats:
 * TAR_MCDB_EGRESS_BITMAP_POINTER_FORMATm: "one" bitmap replication with one CUD. Uses bitmap pointer, CUD, link pointer. The only format for bitmap replications
 * TAR_MCDB_EGRESS_TWO_COPIES_FORMATm: Two replications with one CUD each. Uses 2x(port, CUD), link pointer.
 * TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm: Two replications with the same two CUDs. Uses 2xPort, 2xCUD, link pointer. The only format supporting port replications with two CUDs.
 * TAR_MCDB_EGRESS_TDM_FORMATm: Four replications with the same single CUD. Uses 4xPort, CUD, link pointer.
 *   Preferred to TAR_MCDB_EGRESS_TWO_COPIES_FORMATm if multiple port+one CUD replications share the same CUD.
 *
 * JER2_QAX ingress MC has three formats:
 * TAR_MCDB_BITMAP_REPLICATION_ENTRYm: "one" bitmap replication with two CUDs. Uses bitmap pointer, 2xCUD, link pointer. The only format for bitmap replications
 * TAR_MCDB_SINGLE_REPLICATIONm: One replications with two CUDs. Uses destination, 2xCUD, link pointer.
 * TAR_MCDB_DOUBLE_REPLICATIONm: Two replications, each with one CUD. Uses 2x(Port, CUD), link bit.
 *   The only format with no link pointer, either the end of the group or links to the next entry. 
 */

uint32 jer2_qax_mcds_set_linked_list(
    DNX_SAND_IN int                           unit,
    DNX_SAND_IN uint8                         list_type,           /* specifies if list_prev is a group start to be filled or not */
    DNX_SAND_IN uint32                        group_id,            /* If list_type is *_START this is the group ID, otherwise this is
                                                                      the entry preceding the single linked list to be created */
    DNX_SAND_IN uint32                        list_end,            /* The entry that end of the created linked list will point to, Same one for all given cores */
    DNX_SAND_IN uint32                        alloced_block_start, /* start index of an allocated block to use for the free list */
    DNX_SAND_IN dnx_free_entries_block_size_t alloced_block_size   /* size of the allocated block to use, should be 0 if none */
);



/*
* Return the contents of the MCDB hardware entry, for reconstruction in case of SER.
*/
soc_error_t jer2_qax_mult_get_entry(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32 mcdb_index,
    DNX_SAND_OUT uint32 *entry /* output MCDB entry */
);

/* } */

#endif /* __JER2_QAX_MULTICAST_IMP_INCLUDED__*/
