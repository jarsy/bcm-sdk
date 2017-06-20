/*
 * $Id: rce.c,v 1.74.10.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    rce.c
 * Purpose: Caladan3 Rule Classifier drivers warm boot support
 * Requires:
 */

#include <sal/compiler.h>
#include <shared/shr_resmgr.h>
#include <shared/idxres_mdb.h>
#include <soc/error.h>
#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/caladan3/rce.h>
#include <soc/sbx/caladan3/rce_int.h>
#include <soc/sbx/caladan3/soc_sw_db.h>

#ifdef BCM_CALADAN3_SUPPORT
#ifdef BCM_WARM_BOOT_SUPPORT

/* Most recent version of the overall format */
#define _SOC_C3_RCE_WB_VERSION_CURR 0x0100

/*
 *  Various magic numbers used as the first quadbyte of certain structures
 */
#define _SOC_C3_RCE_WB_MAIN_INDEX_MAGIC    0x00C38CE0
#define _SOC_C3_RCE_WB_UNIT_MAGIC          0x00C38CE1
#define _SOC_C3_RCE_WB_RANGE_INDEX_MAGIC   0x00C38CE2
#define _SOC_C3_RCE_WB_ACT_TBL_INDEX_MAGIC 0x00C38CE3
#define _SOC_C3_RCE_WB_PROG_INDEX_MAGIC    0x00C38CE4
#define _SOC_C3_RCE_WB_FSET_INDEX_MAGIC    0x00C38CE5
#define _SOC_C3_RCE_WB_GROUP_INDEX_MAGIC   0x00C38CE6
#define _SOC_C3_RCE_WB_ENTRY_INDEX_MAGIC   0x00C38CE7

/*
 *  CONFIG:
 *
 *  If SOC_C3_RCE_WB_INHIBIT_WRITE_ON_RESTORE_FAIL is TRUE, a restore failure
 *  will cause the RCE driver to inhibit writing to backing store, so it will
 *  not corrupt the data stored there.  This allows another attempt to recover
 *  (possibly with a diferent version of the driver or different settings if a
 *  settings change is why the restore failed).  If it is FALSE, any recovery
 *  that does not end in a fatal error will be allowed to overwrite the backing
 *  store with new data (including a total wipe and replace if the code uses a
 *  newer version than was in the backing store).
 */
#define SOC_C3_RCE_WB_INHIBIT_WRITE_ON_RESTORE_FAIL TRUE

/*
 *  WARNING: Do not remove fields or change the layout of existing fields in
 *  the backing store structures (only add new fileds).  This allows most of
 *  the code to avoid the tedium of 'which version' as long as it uses the
 *  older fields to locate data and then uses new fields as necessary for other
 *  purposes.
 */

/*
 *  RCE backing store main index
 */
typedef struct _soc_c3_rce_wb_main_index_1_0_s {
    uint32 magic;                   /* magic number indicating validity */
    uint32 unitOffs;                /* offset to unit descriptor */
    uint32 actTblOffs;              /* offset to action table index */
    uint32 rangeOffs;               /* offset to range index */
    uint32 progOffs;                /* offset to program index */
    uint32 fsetOffs;                /* offset to fset index */
    uint32 groupOffs;               /* offset to group index */
    uint32 entryOffs;               /* offset to entry index */
} _soc_c3_rce_wb_main_index_1_0_t;
/* most recenv version of this structure */
typedef _soc_c3_rce_wb_main_index_1_0_t _soc_c3_rce_wb_main_index_t;

/*
 *  RCE backing store unit descriptor
 *
 *  For now, this is only used to verify configuration compatibility.
 *
 *  Padding descriptors are included in the unit partition.
 */
typedef struct _soc_c3_rce_wb_unit_1_0_s {
    /* since there is no 'unit index', put magic number in this struct */
    uint32 magic;                   /* magic number indicating validity */
    /* compatibility checks */
    uint32 unitFlags;               /* unit flags */
    uint32 rangeLimit;              /* number of supported ranges */
    uint32 groupLimit;              /* group limit */
    uint32 filterSetLimit;          /* filter set and group limit */
    uint32 entryLimit;              /* entry limit */
    uint32 entryBasisCounts;        /* entry basis counts */
    uint32 epochTime;               /* RCE epoch length (global clocks) */
    uint16 actionTableCount;        /* number of action tables */
    uint16 progValidFlags;          /* which programs are valid */
    uint8 lrpClockDivide;           /* LRP clock divide ratio */
    uint8 rceClockDivide;           /* RCE clock divide ratio */
    uint8 progOrder[SOC_C3_RCE_PROGRAM_COUNT]; /* program ID by exec order */
    uint16 paddingCount;            /* number of padding descriptors */
    uint32 paddingOffs;             /* offset of padding descriptors */
    uint32 paddingSize;             /* size of padding descriptors */
} _soc_c3_rce_wb_unit_1_0_t;
/* most recent version of this structure */
typedef _soc_c3_rce_wb_unit_1_0_t _soc_c3_rce_wb_unit_t;

/*
 *  RCE backing store padding descriptor
 *
 *  For now, this is only used to verify configuration compatibility.
 */
typedef struct _soc_c3_rce_wb_unit_padDesc_1_0_s {
    uint16 padSize;                 /* size of padding block */
    uint16 padStart;                /* start of padding block in imem */
} _soc_c3_rce_wb_unit_padDesc_1_0_t;
/* most recent version of this structure */
typedef _soc_c3_rce_wb_unit_padDesc_1_0_t _soc_c3_rce_wb_padDesc_t;

/*
 *  RCE backing store action table index
 *
 *  Action data segment holds the API copy of the per-entry action data.  This
 *  is represented as (entries_per_filter_set * bytes_per_entry) bytes of data
 *  for each action table.
 */
typedef struct _soc_c3_rce_wb_actTbl_index_1_0_s {
    uint32 magic;                   /* magic number indicating validity */
    uint32 actTblOffs;              /* offset to actual action table data */
    uint32 actTblDataOffs;          /* offset to action data */
    uint32 actTblTotSize;           /* tot size each action table descriptor */
} _soc_c3_rce_wb_actTbl_index_1_0_t;
/* most recent version of this structure */
typedef _soc_c3_rce_wb_actTbl_index_1_0_t _soc_c3_rce_wb_actTbl_index_t;

/*
 *  RCE backing store action table descriptor
 */
typedef struct _soc_c3_rce_wb_actTbl_1_0_s {
    /* compatibility checks */
    uint32 entryLimit;              /* entry block limit */
    uint32 firstEntryBias;          /* first entry bias */
    uint32 actionBytes;             /* number of bytes in action 'pattern' */
    /* recovery */
    uint32 dataOffset;              /* offset of this table in data */
} _soc_c3_rce_wb_actTbl_1_0_t;
/* most recent version of this structure */
typedef _soc_c3_rce_wb_actTbl_1_0_t _soc_c3_rce_wb_actTbl_t;

/*
 *  RCE backing store range index
 */
typedef struct _soc_c3_rce_wb_range_index_1_0_s {
    uint32 magic;                   /* magic number indicating validity */
    uint32 rangeOffs;               /* offset to actual range data */
    uint32 rangeTotSize;            /* total size of each range descriptor */
    uint32 rangeSize;               /* size of the range descriptor struct */
    uint32 rangeNameSize;           /* max bytes for each range's name */
} _soc_c3_rce_wb_range_index_1_0_t;
/* most receent version of this structure */
typedef _soc_c3_rce_wb_range_index_1_0_t _soc_c3_rce_wb_range_index_t;

/*
 *  RCE backing store range descriptor
 *
 *  Immediately after each range descriptor is the buffer for the name of the
 *  range (more specifically, the name of the field to which the range refers,
 *  but since this can be different for each range even if they use the same
 *  field, it might as well be considered the name of the range).
 *
 *  Since everything about ranges is virtual, all of this is needed in order to
 *  recover state.
 */
typedef struct _soc_c3_rce_wb_range_1_0_s {
    /* recovery */
    uint32 rangeFlags;              /* flags for this range */
    uint32 validProgs;              /* valid programs for this range */
    soc_c3_rce_data_header_t header;/* header from which the bits come */
    uint16 startBit;                /* starting bit for this range */
    uint16 numBits;                 /* number of bits in this range */
    uint16 lowerBound;              /* lower boundary for the range */
    uint16 upperBound;              /* upper boundary for the range */
} _soc_c3_rce_wb_range_1_0_t;
/* most recent version of this structure */
typedef _soc_c3_rce_wb_range_1_0_t _soc_c3_rce_wb_range_t;

/*
 *  RCE backing store program index
 */
typedef struct _soc_c3_rce_wb_program_index_1_0_s {
    uint32 magic;                   /* magic number indicating validity */
    uint32 progOffs;                /* offset to actual program data */
    uint32 progTotSize;             /* total size of each program descriptor */
} _soc_c3_rce_wb_program_index_1_0_t;
/* most recent version of this structure */
typedef _soc_c3_rce_wb_program_index_1_0_t _soc_c3_rce_wb_program_index_t;

/*
 *  RCE backing store program desciptor
 *
 *  Programs are arranged in backing store by execution order, and the ID of
 *  the program is used as the index of the unit data progIndex, which points
 *  to one of these slots.
 *
 *  While some of this is used to verify configuration compatibility, some of
 *  it is used to track state that is more easily and more efficiently tracked
 *  than recovered from hardware.
 */
typedef struct _soc_c3_rce_wb_program_1_0_s {
    /* compatibility checks */
    uint32 keyTime;                 /* key transfer begins, global clocks */
    uint32 switchTime;              /* result deadline, global clocks */
    uint8 actIndex[SOC_C3_RCE_RESULT_REGISTER_COUNT]; /* action tbl by res reg */
    /* recovery */
    uint32 filterSetCount;          /* number of filter sets this program */
    uint32 firstFilterSet;          /* first filter set this program */
    uint32 instrBlockCount;         /* instruction blocks count */
    uint32 instrBlockAdded;         /* added blocks for padding */
    uint32 groupHead;               /* group ID of first group in program */
    uint32 groupTail;               /* group ID of last group in program */
} _soc_c3_rce_wb_program_1_0_t;
/* most recent version of this structure */
typedef _soc_c3_rce_wb_program_1_0_t _soc_c3_rce_wb_program_t;

/*
 *  RCE backing store filter set index
 */
typedef struct _soc_c3_rce_wb_fset_index_1_0_s {
    uint32 magic;                   /* magic number for checking validity */
    uint32 fsetOffs;                /* offset to actual fset descriptors */
    uint32 imemOffs;                /* offset to imem instruction data */
    uint32 pattOffs;                /* offset to pattern data */
    uint32 fsetTotSize;             /* total size per fset descriptor */
    uint32 imemTotSize;             /* total size per imem descriptor */
} _soc_c3_rce_wb_fset_index_1_0_t;
/* most recent version of this structure */
typedef _soc_c3_rce_wb_fset_index_1_0_t _soc_c3_rce_wb_fset_index_t;

/*
 *  RCE backing store filter set descriptor
 *
 *  Filter sets are grouped by program ID and within each program sorted by
 *  their order of execution.
 */
typedef struct _soc_c3_rce_wb_fset_1_0_s {
    uint32 fsetFlags;               /* flags for the filter set */
    uint32 imemStart;               /* first block of imem */
    uint32 imemSize;                /* number of blocks of imem */
    uint32 amemBlock;               /* action memory block */
    uint32 imemDataStart;           /* offset in imem data for filter set */
    uint32 pattDataStart;           /* offset in patt data for filter set */
    uint32 filterIndex;             /* which filter set in the group */
    uint32 groupId;                 /* group to which the fset belongs */
} _soc_c3_rce_wb_fset_1_0_t;
/* most recent version of this structure */
typedef _soc_c3_rce_wb_fset_1_0_t _soc_c3_rce_wb_fset_t;

/*
 *  RCE backing store filter set instruction memory data
 *
 *  After all of the filter set descriptors, there will be one of these things
 *  for every possible instruction (so one per instruction in imem).
 *
 *  We try to keep the flags from overlapping the lower 32b of the instruction
 *  since this can cause spurious misinterpretation of the flags, so to do this
 *  we try to keep the flags where they sit on the upper 32b of the
 *  instruction, most of which are not used (43 bit instructions).
 *
 *  If someone flips machine order between save and reload, all bets are off
 *  (not just this particular one!)...
 */
typedef union _soc_c3_rce_wb_fset_imem_1_0_u {
    struct {
#if BE_HOST
        uint32 rangeFlags;              /* range flags */
        int32 rangeId;                  /* range ID */
#else /* BE_HOST */
        int32 rangeId;                  /* range ID */
        uint32 rangeFlags;              /* range flags */
#endif /* BE_HOST */
    } rangeData;
    _soc_c3_rce_instruction_t instr;    /* instruction */
} _soc_c3_rce_wb_fset_imem_1_0_t;
/* most recent version of this structure */
typedef _soc_c3_rce_wb_fset_imem_1_0_t _soc_c3_rce_wb_fset_imem_t;

/*
 *  RCE backing store group index
 */
typedef struct _soc_c3_rce_wb_group_index_1_0_s {
    uint32 magic;                   /* magic number indicating validity */
    uint32 groupOffs;               /* offset to actual group data */
    uint32 groupTotSize;            /* total size of each group descriptor */
    uint32 groupSize;               /* size of base group descriptor */
    uint32 groupQualMax;            /* number of qualifier slots per group */
    uint32 groupQualSize;           /* size of a single qualifier slot */
    uint32 groupQualNameMax;        /* bytes per group qualifier name */
} _soc_c3_rce_wb_group_index_1_0_t;
/* most recent version of this structure */
typedef _soc_c3_rce_wb_group_index_1_0_t _soc_c3_rce_wb_group_index_t;

/*
 *  RCE backing store qualifier descriptor
 *
 *  The basic hardware view of the qualifier can be restored from hardware, but
 *  there are details in the software state that would be lost, so they are
 *  tracked here.  Certain qualifier types can be fully recovered from this
 *  view, but other types will require examination of hardware.
 *
 *  Immediately after this descriptor is the space for the qualifier name.
 *
 *  For the 'sparse' qualifier types, the list of parameters must be extracted
 *  from hardware state (specifically it must be derived from the affected bit
 *  in the RCE instruction's predicate).  For 'non-sparse' qualifier types, the
 *  first bit and count are stored in the provided parameter spaces.
 */
typedef struct _soc_c3_rce_wb_qual_1_0_s {
    /* recovery */
    soc_c3_rce_qual_type_t qualType;/* qualifier type */
    uint32 paramCount;              /* number of parameters for qualifier */
    uint32 qualOffs;                /* qualifier offset in pattern */
    int32 param[2];                 /* first two parameters for qualifier */
} _soc_c3_rce_wb_qual_1_0_t;
/* most recent version of this structure */
typedef _soc_c3_rce_wb_qual_1_0_t _soc_c3_rce_wb_qual_t;

/*
 *  RCE backing store group descriptor
 *
 *  Much of a group can be recovered from hardware, but much can not.  We try
 *  to bias toward recovery from hardware when reasonable, and only include
 *  here enough hints to make it easiser, but some things must be here because
 *  they are not tracked at all in hardware.
 *
 *  Immediately after the group descriptor will follow
 *  SOC_C3_RCE_GROUP_QUAL_MAX qualifier descriptors.
 */
typedef struct _soc_c3_rce_wb_group_1_0_s {
    /* sanity checking */
    uint8 rceProgram;               /* RCE program for this group */
    uint8 resultLrpUniq;            /* unique LRP result bitmap */
    uint8 resultLrp;                /* LRP result bitmap */
    uint8 resultRce;                /* RCE result bitmap */
    /* recovery */
    uint32 groupFlags;              /* group flags */
    uint32 rangesPerFilterSet;      /* ranges per filter set */
    uint32 firstFilterSet;          /* first filter set of group in program */
    uint32 filterSetCount;          /* current number of filter sets */
    uint32 maxFilterSets;           /* maximum number of filter sets */
    uint32 dataSize;                /* size of pattern and action data */
    uint32 instrBlocks;             /* size of group's template */
    uint32 qualCount;               /* number of qualifiers */
    int32 groupPriority;            /* group priority */
    uint32 groupPrev;               /* previous group this program */
    uint32 groupNext;               /* next group this program */
    uint32 entryHead;               /* first entry ID */
    uint32 entryTail;               /* last entry ID */
    uint32 entryLastAdded;          /* most recent entry added to group */
} _soc_c3_rce_wb_group_1_0_t;
/* most recent version of this structure */
typedef _soc_c3_rce_wb_group_1_0_t _soc_c3_rce_wb_group_t;

/*
 *  RCE backing store entry index
 */
typedef struct _soc_c3_rce_wb_entry_index_1_0_s {
    uint32 magic;                   /* magic number indicating validity */
    uint32 entryOffs;               /* offset to actual entry data */
    uint32 entryTotSize;            /* total size per entry */
    uint32 entrySize;               /* size of entry (no basis counts) */
} _soc_c3_rce_wb_entry_index_1_0_t;
/* most recent version of this structure */
typedef _soc_c3_rce_wb_entry_index_1_0_t _soc_c3_rce_wb_entry_index_t;

/*
 *  RCE backing store entry descriptor
 *
 *  Note that pattern and action data are stored elsewhere -- the pattern data
 *  are stored in the filter set pattern space, and the action data are stored
 *  in the action table action space.
 */
typedef struct _soc_c3_rce_wb_entry_1_0_s {
    /* sanity checking */
    int32 entryGroup;               /* group for this entry */
    /* recovery */
    uint32 entryFlags;              /* entry flags */
    uint32 entryPosition;           /* where the entry is in the group */
    uint32 entryPosPrev;            /* where the entry was before update */
    int32 entryPriority;            /* entry priority */
    uint32 entryPrev;               /* previous entry this group */
    uint32 entryNext;               /* next entry this group */
} _soc_c3_rce_wb_entry_1_0_t;
/* most recent version of this structure */
typedef _soc_c3_rce_wb_entry_1_0_t _soc_c3_rce_wb_entry_t;

int
_soc_c3_rce_wb_purge(_soc_c3_rce_unit_desc_int_t *unitData)
{
    
    LOG_WARN(BSL_LS_SOC_COMMON,
             (BSL_META("unit %d discarding (but not"
                       " freeing or poisoning)"
                       " backing store space\n"),
              unitData->unit));
    return SOC_E_NONE;
}

int
_soc_c3_rce_wb_size_calc(_soc_c3_rce_unit_desc_int_t *unitData,
                         unsigned int *expectedSize)
{
    unsigned int current;
    unsigned int index;

    /* start with nothing */
    current = 0;

    /* include the main index */
    current += sizeof(_soc_c3_rce_wb_main_index_t);

    /* include the unit descriptor */
    current += sizeof(_soc_c3_rce_wb_unit_t);
    /* include the padding descriptors */
    current += (sizeof(_soc_c3_rce_wb_padDesc_t) *
                C3_RCE_FILTER_SET_COUNT_PADDING);

    /* include the action table index */
    current += sizeof(_soc_c3_rce_wb_actTbl_index_t);
    /* include the action table descriptors */
    current += sizeof(_soc_c3_rce_wb_actTbl_t) * unitData->actTableCount;
    /* include the action table data */
    for (index = 0; index < unitData->actTableCount; index++) {
        current += (unitData->actData[index]->actionBytes *
                    unitData->actData[index]->entryLimit *
                    C3_RCE_ENTRIES_PER_FILTER_SET);
    }

    /* include the range index */
    current += sizeof(_soc_c3_rce_wb_range_index_t);
    /* include the range descriptors */
    current += ((sizeof(_soc_c3_rce_wb_range_t) +
                 (sizeof(char) * C3_RCE_RANGE_NAME_BYTES)) *
                unitData->rangeLimit);

    /* include the program index */
    current += sizeof(_soc_c3_rce_wb_program_index_t);
    /* include the program descriptors */
    current += (sizeof(_soc_c3_rce_wb_program_t) * SOC_C3_RCE_PROGRAM_COUNT);

    /* include the filter set index */
    current += sizeof(_soc_c3_rce_wb_fset_index_t);
    /* include the filter set descriptors */
    current += (sizeof(_soc_c3_rce_wb_fset_t) * unitData->groupLimit);
    /* include the filter set instruction descriptors */
    current += (sizeof(_soc_c3_rce_wb_fset_imem_t) *
                C3_RCE_IMEM_SIZE *
                C3_RCE_IMEM_INSTR_PER_BLOCK);
    /* include the filter set pattern data */
    current += (C3_RCE_IMEM_SIZE *
                C3_RCE_ENTRIES_PER_FILTER_SET);

    /* include the group index */
    current += sizeof(_soc_c3_rce_wb_group_index_t);
    /* include the group descriptors */
    current += ((sizeof(_soc_c3_rce_wb_group_t) +
                ((sizeof(_soc_c3_rce_wb_qual_t) + C3_RCE_QUAL_NAME_BYTES) *
                 SOC_C3_RCE_GROUP_QUAL_MAX)) *
                unitData->groupLimit);

    /* include the entry index */
    current += sizeof(_soc_c3_rce_wb_entry_index_t);
    /* include the entry descriptors */
    current += ((sizeof(_soc_c3_rce_wb_entry_t)
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
                  + (sizeof(uint64) * SOC_C3_RCE_NUM_BASIS_COUNTERS)
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
                 ) * unitData->entryLimit);

    *expectedSize = current;
    return SOC_E_NONE;
}

/*
 *  Function
 *    _soc_c3_rce_program_build_prefix_sparse
 *  Purpose
 *    Use opcodes in group template to recover parameters for a prefix type
 *    qualifier during warm boot recovery.
 *  Arguments
 *    (IN/OUT) qual = pointer to the qualifier with only type filled in
 *    (IN) instr = pointer to buffer for instructions
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Must update this and associated functions above when updating
 *    _soc_c3_rce_program_build_general and its associated functions.
 */
static int
_soc_c3_rce_wb_program_recover_prefix_sparse(_soc_c3_rce_qual_desc_int_t *qual,
                                             const _soc_c3_rce_instruction_t *instr)
{
    int index;
    int bit;
    uint16 bitMask;
    uint16 bitData;
    uint16 doubleByte;

    
    for (index = 1; index <= qual->paramCount; index++) {
        doubleByte = COMPILER_64_HI(instr[index]) & 0x1F;
        bitData = (COMPILER_64_LO(instr[index]) >> 16) & 0xFFFF;
        for (bit = 0, bitMask = 1;
             bit < 16;
             bit++, bitMask <<= 1) {
            if (bitData & bitMask) {
                /* found the bit */
                break;
            }
        } /* for (all bits within the selected doublebyte) */
        qual->param[index - 1] = (doubleByte << 4) | bit;
    } /* for (all bits in this qualifier) */
    return SOC_E_NONE;
}

/*
 *  Function
 *    _soc_c3_rce_wb_program_recover_postfix_sparse
 *  Purpose
 *    Use opcodes in group template to recover parameters for a postfix type
 *    qualifier during warm boot recovery.
 *  Arguments
 *    (IN/OUT) qual = pointer to the qualifier with only type filled in
 *    (IN) instr = pointer to buffer for instructions
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Must update this and associated functions above when updating
 *    _soc_c3_rce_program_build_general and its associated functions.
 */
static int
_soc_c3_rce_wb_program_recover_postfix_sparse(_soc_c3_rce_qual_desc_int_t *qual,
                                              const _soc_c3_rce_instruction_t *instr)
{
    int index;
    int bit;
    uint16 bitMask;
    uint16 bitData;
    uint16 doubleByte;

    
    for (index = 1; index <= qual->paramCount; index++) {
        doubleByte = COMPILER_64_HI(instr[index]) & 0x1F;
        bitData = (COMPILER_64_LO(instr[index]) >> 16) & 0xFFFF;
        for (bit = 0, bitMask = 1;
             bit < 16;
             bit++, bitMask <<= 1) {
            if (bitData & bitMask) {
                /* found the bit */
                break;
            }
        } /* for (all bits within the selected doublebyte) */
        qual->param[qual->paramCount - index] = (doubleByte << 4) | bit;
    } /* for (all bits in this qualifier) */
    return SOC_E_NONE;
}

/*
 *  Function
 *    _soc_c3_rce_wb_program_recover_masked_sparse
 *  Purpose
 *    Use opcodes in group template to recover parameters for a masked type
 *    qualifier during warm boot recovery.
 *  Arguments
 *    (IN/OUT) qual = pointer to the qualifier with only type filled in
 *    (IN) instr = pointer to buffer for instructions
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Must update this and associated functions above when updating
 *    _soc_c3_rce_program_build_general and its associated functions.
 */
static int
_soc_c3_rce_wb_program_recover_masked_sparse(_soc_c3_rce_qual_desc_int_t *qual,
                                             const _soc_c3_rce_instruction_t *instr)
{
    int index;
    int bit;
    uint16 bitMask;
    uint16 bitData;
    uint16 doubleByte;

    
    for (index = 0; index < qual->paramCount; index++) {
        doubleByte = COMPILER_64_HI(instr[index]) & 0x1F;
        bitData = (COMPILER_64_LO(instr[index]) >> 16) & 0xFFFF;
        for (bit = 0, bitMask = 1;
             bit < 16;
             bit++, bitMask <<= 1) {
            if (bitData & bitMask) {
                /* found the bit */
                break;
            }
        } /* for (all bits within the selected doublebyte) */
        qual->param[index] = (doubleByte << 4) | bit;
    } /* for (all bits in this qualifier) */
    return SOC_E_NONE;
}

/*
 *  Function
 *    _soc_c3_rce_wb_program_recover_exact_sparse
 *  Purpose
 *    Use opcodes in group template to recover parameters for an exact type
 *    qualifier during warm boot recovery.
 *  Arguments
 *    (IN/OUT) qual = pointer to the qualifier with only type filled in
 *    (IN) instr = pointer to buffer for instructions
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Must update this and associated functions above when updating
 *    _soc_c3_rce_program_build_general and its associated functions.
 */
static int
_soc_c3_rce_wb_program_recover_exact_sparse(_soc_c3_rce_qual_desc_int_t *qual,
                                            const _soc_c3_rce_instruction_t *instr)
{
    int index;
    int bit;
    uint16 bitMask;
    uint16 bitData;
    uint16 doubleByte;

    
    for (index = 0; index < qual->paramCount; index++) {
        doubleByte = COMPILER_64_HI(instr[index]) & 0x1F;
        bitData = (COMPILER_64_LO(instr[index]) >> 16) & 0xFFFF;
        for (bit = 0, bitMask = 1;
             bit < 16;
             bit++, bitMask <<= 1) {
            if (bitData & bitMask) {
                /* found the bit */
                break;
            }
        } /* for (all bits within the selected doublebyte) */
        qual->param[index] = (doubleByte << 4) | bit;
    } /* for (all bits in this qualifier) */
    return SOC_E_NONE;
}

/*
 *  Function
 *    _soc_c3_rce_wb_program_recover_general
 *  Purpose
 *    Use opcodes in group template to recover parameters for qualifier
 *    descriptions attached to groups during warm boot recovery.
 *  Arguments
 *    (IN/OUT) qual = pointer to the qualifier with only type filled in
 *    (IN) instr = pointer to buffer for instructions
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Must update this and associated functions above when updating
 *    _soc_c3_rce_program_build_general and its associated functions.
 */
static int
_soc_c3_rce_wb_program_recover_general(_soc_c3_rce_qual_desc_int_t *qual,
                                       const _soc_c3_rce_instruction_t *instr)
{
    switch (qual->qualType) {
    case socC3RCEQualType_prefix_sparse:
        return _soc_c3_rce_wb_program_recover_prefix_sparse(qual, instr);
    case socC3RCEQualType_postfix_sparse:
        return _soc_c3_rce_wb_program_recover_postfix_sparse(qual, instr);
    case socC3RCEQualType_masked_sparse:
        return _soc_c3_rce_wb_program_recover_masked_sparse(qual, instr);
    case socC3RCEQualType_exact_sparse:
        return _soc_c3_rce_wb_program_recover_exact_sparse(qual, instr);
    case socC3RCEQualType_prefix:
    case socC3RCEQualType_postfix:
    case socC3RCEQualType_masked:
    case socC3RCEQualType_exact:
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unexpected qualifier type %d (should have been"
                            " recovered directly\n"),
                   qual->qualType));
        return SOC_E_PARAM;
    default:
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("invalid qualifier type %d\n"),
                   qual->qualType));
        return SOC_E_PARAM;
    } /* switch (qual->qualType) */
}

/*
 *  Function
 *    _soc_c3_rce_wb_load
 *  Purpose
 *    Init RCE driver state from backing store
 *  Arguments
 *    (IN/OUT) unitData = pointer to unit information
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This assumes initial state and that it is called during soc_c3_rce_init;
 *    if it is called otherwise, it will almost certainly corrupt things.
 */
static int
_soc_c3_rce_wb_load(_soc_c3_rce_unit_desc_int_t *unitData)
{
    _soc_c3_rce_wb_main_index_t *wbIndex;       /* pointer to WB main index */
    _soc_c3_rce_wb_unit_t *wbUnit;              /* pointer to WB unit data */
    _soc_c3_rce_wb_padDesc_t *wbUnitPad;        /* pointer to WB pading data */
    _soc_c3_rce_wb_actTbl_index_t *wbActTblIdx; /* pointer to WB actTbl index */
    _soc_c3_rce_wb_actTbl_t *wbActTbl;          /* pointer to WB actTbl data */
    _soc_c3_rce_wb_range_index_t *wbRangeIdx;   /* pointer to WB range index */
    _soc_c3_rce_wb_range_t *wbRange;            /* pointer to WB range data */
    _soc_c3_rce_wb_program_index_t *wbProgIdx;  /* pointer to WB program idx */
    _soc_c3_rce_wb_program_t *wbProg;           /* pointer to WB program data */
    _soc_c3_rce_wb_fset_index_t *wbFsetIdx;     /* pointer to WB fset index */
    _soc_c3_rce_wb_fset_t *wbFset;              /* pointer to WB fset data */
    _soc_c3_rce_wb_fset_imem_t *wbFsetImem;     /* ptr to WB fset imem data */
    _soc_c3_rce_wb_group_index_t *wbGroupIdx;   /* pointer to WB group index */
    _soc_c3_rce_wb_group_t *wbGroup;            /* pointer to WB group data */
    _soc_c3_rce_wb_qual_t *wbQual;              /* pointer to WB qual data */
    _soc_c3_rce_wb_entry_index_t *wbEntryIdx;   /* pointer to WB entry index */
    _soc_c3_rce_wb_entry_t *wbEntry;            /* pointer to WB entry data */
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
    uint64 *wbBasisCounts;                      /* pointer to WB ent basis ct */
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
    char *wbName;                               /* pointer to WB name data */
    uint8 *wbAction;                            /* pointer to WB action data */
    uint8 *wbPattern;                           /* pointer to WB pattern data */
    const _soc_c3_rce_actions_uc_desc_t *actTblData; /* action table data */
    _soc_c3_rce_range_desc_int_t *rangeData;    /* pointer to range data */
    _soc_c3_rce_program_desc_int_t *progData;   /* pointer to program data */
    _soc_c3_rce_filterset_desc_int_t *fsetData; /* pointer to fset data */
    _soc_c3_rce_group_desc_int_t *groupData;    /* pointer to group data */
    _soc_c3_rce_qual_desc_int_t *qualData;      /* pointer to qualifier data */
    _soc_c3_rce_entry_desc_int_t *entryData;    /* pointer to entry data */
    _soc_c3_rce_program_block_t *progBlock = NULL;/* imem+pmem access buffer */
    uint8 *actBuffer = NULL;                    /* big workspace for actData */
    uint8 *actData[SOC_C3_RCE_RESULT_REGISTER_COUNT][SOC_C3_RCE_MAX_OCM_SEG_PER_ACTION_TABLE]; /* action table access buff */
    unsigned int actSize[SOC_C3_RCE_RESULT_REGISTER_COUNT][SOC_C3_RCE_MAX_OCM_SEG_PER_ACTION_TABLE];/* segment entry sizes */
    int result = SOC_E_NONE;                    /* working result */
    unsigned int index;                         /* working index */
    unsigned int offset;                        /* working offset */
    unsigned int address;                       /* working address */
    unsigned int scSize;                        /* working WB rel address */
    unsigned int daSize;                        /* working data rel address */
    unsigned int paSize;                        /* working patt rel address */
    unsigned int actTblId;                      /* working action table ID */
    unsigned int rangeId;                       /* working range ID */
    unsigned int programId;                     /* working program ID */
    unsigned int groupId;                       /* working group ID */
    unsigned int fsetId;                        /* working filter set ID */
    unsigned int entryId;                       /* working entry ID */
    unsigned int progBlockSize;                 /* size of imem+pmem block */
    unsigned int rowOffset;                     /* working row offset */
    unsigned int byteOffset;                    /* working byte offset */
    int temp;                                   /* temporary workspace */
    _soc_c3_rce_instruction_t instrMask;        /* instruction mask */

    RCE_EVERB((RCE_MSG1("unit %d warm boot check sanity\n"),
               unitData->unit));

    if ((!unitData->scPtr) || (!unitData->scSize)) {
        /* no space for restore; should this be a failure? */
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META("unit %d asked to load state from"
                           " backing store but there is no"
                           " backing store space\n"),
                  unitData->unit));
        result = SOC_E_NONE;
        goto error;
    }

    /* will want to strip away extraneous flags on instructions */
    COMPILER_64_SET(instrMask, ~_SOC_C3_RCE_RANGE_FLAG_IN_TEMPLATE, ~0);

    /* get and check main index */
    wbIndex = (_soc_c3_rce_wb_main_index_t*)(unitData->scPtr);
    if (_SOC_C3_RCE_WB_MAIN_INDEX_MAGIC != wbIndex->magic) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d invalid magic (%08X, expected %08X) on"
                            " backing store main index\n"),
                   unitData->unit,
                   wbIndex->magic,
                 _SOC_C3_RCE_WB_MAIN_INDEX_MAGIC));
        result = SOC_E_FAIL;
        goto error;
    }
    if ((wbIndex->unitOffs + sizeof(*wbUnit) >= unitData->scSize) ||
        (wbIndex->actTblOffs + sizeof(*wbActTblIdx) >= unitData->scSize) ||
        (wbIndex->rangeOffs + sizeof(*wbRangeIdx) >= unitData->scSize) ||
        (wbIndex->progOffs + sizeof(*wbProgIdx) >= unitData->scSize) ||
        (wbIndex->fsetOffs + sizeof(*wbFsetIdx) >= unitData->scSize) ||
        (wbIndex->groupOffs + sizeof(*wbGroupIdx) >= unitData->scSize) ||
        (wbIndex->entryOffs + sizeof(*wbEntryIdx) >= unitData->scSize)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d invalid main index -- at least one offset"
                            " is out of bounds (>= available size %u)\n"),
                   unitData->unit,
                   unitData->scSize));
    }

    /* get and check unit information */
    wbUnit = (_soc_c3_rce_wb_unit_t*)(&(unitData->scPtr[wbIndex->unitOffs]));
    if (_SOC_C3_RCE_WB_UNIT_MAGIC != wbUnit->magic) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d invalid magic (%08X, expected %08X) on"
                            " backing store unit information\n"),
                   unitData->unit,
                   wbUnit->magic,
                   _SOC_C3_RCE_WB_UNIT_MAGIC));
        result = SOC_E_FAIL;
        goto error;
    }
    if (((unitData->unitFlags & SOC_C3_RCE_UNIT_FLAGS_SWITCH2) !=
         (wbUnit->unitFlags & SOC_C3_RCE_UNIT_FLAGS_SWITCH2)) ||
        (unitData->epochTime != wbUnit->epochTime) ||
        (unitData->lrpClockDivide != wbUnit->lrpClockDivide) ||
        (unitData->rceClockDivide != wbUnit->rceClockDivide) ||
        memcmp(unitData->progOrder,
               wbUnit->progOrder,
               SOC_C3_RCE_PROGRAM_COUNT)) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META("unit %d timing has changed; it is possible there"
                           " will be issues if close to time-based capacity\n"),
                  unitData->unit));
    }
    address = (wbIndex->unitOffs +
               wbUnit->paddingOffs);
    wbUnitPad = (_soc_c3_rce_wb_padDesc_t*)(&(unitData->scPtr[address]));
    if ((C3_RCE_FILTER_SET_COUNT_PADDING != wbUnit->paddingCount) ||
        (C3_RCE_FILTER_SET_FIRST_PADDING != wbUnitPad->padSize) ||
        (unitData->imemExtra[0] != wbUnitPad->padStart)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d program padding configuration has"
                            " changed\n"),
                   unitData->unit));
        result = SOC_E_FAIL;
        goto error;
    }
    for (index = 1; index < wbUnit->paddingCount; index++) {
        address += wbUnit->paddingSize;
        wbUnitPad = (_soc_c3_rce_wb_padDesc_t*)(&(unitData->scPtr[address]));
        if ((C3_RCE_FILTER_SET_LENGTH_PADDING != wbUnitPad->padSize) ||
            (unitData->imemExtra[index] != wbUnitPad->padStart)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("unit %d program padding configuration has"
                                " changed\n"),
                       unitData->unit));
            result = SOC_E_FAIL;
            goto error;
        }
    }
    if (unitData->actTableCount != wbUnit->actionTableCount) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d action table count %u but was %u when"
                            " backing store state was saved\n"),
                   unitData->unit,
                   unitData->actTableCount,
                   wbUnit->actionTableCount));
        result = SOC_E_FAIL;
        goto error;
    }
    if (unitData->rangeLimit < wbUnit->rangeLimit) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META("unit %d range limit %u but was %u when backing"
                           " store state was saved\n"),
                  unitData->unit,
                  unitData->rangeLimit,
                  wbUnit->rangeLimit));
    }
    if (unitData->groupLimit < wbUnit->groupLimit) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META("unit %d group limit %u but was %u when backing"
                           " store state was saved\n"),
                  unitData->unit,
                  unitData->groupLimit,
                  wbUnit->groupLimit));
    }
    if (unitData->entryLimit < wbUnit->entryLimit) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META("unit %d entry limit %u but was %u when backing"
                           " store state was saved\n"),
                  unitData->unit,
                  unitData->entryLimit,
                  wbUnit->entryLimit));
    }
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
    if (wbUnit->entryBasisCounts > SOC_C3_RCE_NUM_BASIS_COUNTERS) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META("unit %d originally supported %u basis counts per"
                           " entry but only supports %u now; higher numbered"
                           " basis counts will be lost during recovery\n"),
                  unitData->unit,
                  wbUnit->entryBasisCounts,
                  SOC_C3_RCE_NUM_BASIS_COUNTERS));
    } else if (wbUnit->entryBasisCounts < SOC_C3_RCE_NUM_BASIS_COUNTERS) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META("unit %d originally supported %u basis counts per"
                           " entry but now supports %u; additional basis"
                           " counts will be zeroed during recovery\n"),
                  unitData->unit,
                  wbUnit->entryBasisCounts,
                  SOC_C3_RCE_NUM_BASIS_COUNTERS));
    }
#else /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
    if (wbUnit->entryBasisCounts) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META("unit %d originally supported counters and was"
                           " tracking %u basis counts per entry; counter"
                           " support is disabled, so those values will be"
                           " lost during recovery\n"),
                  unitData->unit,
                  wbUnit->entryBasisCounts));
    }
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */

    /* get and verify the action table index */
    wbActTblIdx = (_soc_c3_rce_wb_actTbl_index_t*)(&(unitData->scPtr[wbIndex->actTblOffs]));
    if (_SOC_C3_RCE_WB_ACT_TBL_INDEX_MAGIC != wbActTblIdx->magic) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d invalid magic (%08X, expected %08X) on"
                            " backing store action table index\n"),
                   unitData->unit,
                   wbActTblIdx->magic,
                   _SOC_C3_RCE_WB_ACT_TBL_INDEX_MAGIC));
        result = SOC_E_FAIL;
        goto error;
    }
    scSize = (wbIndex->actTblOffs + wbActTblIdx->actTblOffs +
              (wbActTblIdx->actTblOffs * wbActTblIdx->actTblTotSize));
    if (scSize >= unitData->scSize) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d action table descriptor space %u extends"
                            " beyond the allocated backing store size %u\n"),
                   unitData->unit,
                   scSize,
                   unitData->scSize));
        result = SOC_E_FAIL;
        goto error;
    }
    /* verify the action table descriptors and data layout */
    daSize = wbIndex->actTblOffs + wbActTblIdx->actTblDataOffs;
    for (actTblId = 0; actTblId < wbUnit->actionTableCount; actTblId++) {
        address = (wbIndex->actTblOffs +
                   wbActTblIdx->actTblOffs +
                   (wbActTblIdx->actTblTotSize * actTblId));
        wbActTbl = (_soc_c3_rce_wb_actTbl_t*)(&(unitData->scPtr[address]));
        if ((wbActTbl->actionBytes != unitData->actData[actTblId]->actionBytes) ||
            (wbActTbl->firstEntryBias != unitData->actData[actTblId]->firstEntryBias) ||
            (wbActTbl->entryLimit > unitData->actData[actTblId]->entryLimit)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("unit %d action table %u configuration changed"
                                " in a way that is not supported\n"),
                       unitData->unit,
                       actTblId));
            result = SOC_E_FAIL;
            goto error;
        }
        if (wbActTbl->entryLimit < unitData->actData[actTblId]->entryLimit) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META("unit %d action table %u has grown from %u to"
                               " %u; after recovery, reversion will not be"
                               " supported\n"),
                      unitData->unit,
                      actTblId,
                      wbActTbl->entryLimit,
                      unitData->actData[actTblId]->entryLimit));
        }
        daSize += (wbActTbl->entryLimit *
                   wbActTbl->actionBytes *
                   C3_RCE_ENTRIES_PER_FILTER_SET);
    } /* for (index = 0; index < wbUnit->actionTableCount; index++) */
    if (daSize > wbIndex->rangeOffs) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d action table data space %u extends into"
                            " the range partition at %u\n"),
                   unitData->unit,
                   daSize,
                   wbIndex->rangeOffs));
        result = SOC_E_FAIL;
        goto error;
    }

    /* get and check the range index and descriptor layout */
    wbRangeIdx = (_soc_c3_rce_wb_range_index_t*)(&(unitData->scPtr[wbIndex->rangeOffs]));
    if (_SOC_C3_RCE_WB_RANGE_INDEX_MAGIC != wbRangeIdx->magic) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d invalid magic (%08X, expected %08X) on"
                            " backing store range index\n"),
                   unitData->unit,
                   wbRangeIdx->magic,
                   _SOC_C3_RCE_WB_RANGE_INDEX_MAGIC));
        result = SOC_E_FAIL;
        goto error;
    }
    scSize = (wbIndex->rangeOffs +
              wbRangeIdx->rangeOffs +
              (wbRangeIdx->rangeTotSize * wbUnit->rangeLimit));
    if (scSize >= unitData->scSize) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d range decriptor space %u extends"
                            " beyond the allocated backing store size %u\n"),
                   unitData->unit,
                   scSize,
                   unitData->scSize));
        result = SOC_E_FAIL;
        goto error;
    }
    if (scSize > wbIndex->progOffs) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d range descriptor space %u extends into the"
                            " program partition at %u\n"),
                   unitData->unit,
                   scSize,
                   wbIndex->progOffs));
        result = SOC_E_FAIL;
        goto error;
    }
    /* scan ranges to be sure we can represent all of them */
    for (rangeId = 0; rangeId < wbUnit->rangeLimit; rangeId++) {
        address = (wbIndex->rangeOffs +
                   wbRangeIdx->rangeOffs +
                   (wbRangeIdx->rangeTotSize * rangeId));
        wbRange = (_soc_c3_rce_wb_range_t*)(&(unitData->scPtr[address]));
        if (wbRange->rangeFlags & SOC_C3_RCE_RANGE_FLAG_VALID) {
            if (rangeId >= unitData->rangeLimit) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d range %u can not be restored"
                                    " since unit %d only supports %u"
                                    " ranges now\n"),
                           unitData->unit,
                           rangeId,
                           unitData->unit,
                           unitData->rangeLimit));
                result = SOC_E_FAIL;
                goto error;
            }
            address += wbRangeIdx->rangeSize;
            wbName = (char*)(&(unitData->scPtr[address]));
            index = sal_strlen(wbName);
            if (index > C3_RCE_RANGE_NAME_BYTES - 1) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d range %u name (%u chars) will"
                                    " be truncated during restore into"
                                    " %u chars\n"),
                           unitData->unit,
                           rangeId,
                           index,
                           C3_RCE_RANGE_NAME_BYTES - 1));
                /* this is only a warning */
            }
        } /* if (wbRange->rangeFlags & SOC_C3_RCE_RANGE_FLAG_VALID) */
    } /* for (index = 0; index < wbUnit->rangeLimit; index++) */

    /* get and check the program index and descriptors */
    wbProgIdx = (_soc_c3_rce_wb_program_index_t*)(&(unitData->scPtr[wbIndex->progOffs]));
    if (_SOC_C3_RCE_WB_PROG_INDEX_MAGIC != wbProgIdx->magic) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d invalid magic (%08X, expected %08X) on"
                            " backing store program index\n"),
                   unitData->unit,
                   wbRangeIdx->magic,
                   _SOC_C3_RCE_WB_PROG_INDEX_MAGIC));
        result = SOC_E_FAIL;
        goto error;
    }
    scSize = (wbIndex->progOffs +
              wbProgIdx->progOffs +
              (wbProgIdx->progTotSize * SOC_C3_RCE_PROGRAM_COUNT));
    if (scSize >= unitData->scSize) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d program descriptor space %u extends"
                            " beyond the allocated backing store size %u\n"),
                   unitData->unit,
                   scSize,
                   unitData->scSize));
        result = SOC_E_FAIL;
        goto error;
    }
    if (scSize > wbIndex->fsetOffs) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d program descriptor space %u extends into"
                            " the filter set partition at %u\n"),
                   unitData->unit,
                   scSize,
                   wbIndex->fsetOffs));
        result = SOC_E_FAIL;
        goto error;
    }
    /* scan the programs */
    for (programId = 0; programId < SOC_C3_RCE_PROGRAM_COUNT; programId++) {
        if (wbUnit->progValidFlags & (1 << programId)) {
            /* backing store says to expect a program here */
            if (unitData->progData[programId]) {
                /* there is a program here */
                address = (wbIndex->progOffs +
                           wbProgIdx->progOffs +
                           (wbProgIdx->progTotSize * programId));
                progData = unitData->progData[programId];
                wbProg = (_soc_c3_rce_wb_program_t*)(&(unitData->scPtr[address]));
                if ((wbProg->keyTime != progData->keyTime) ||
                    (wbProg->switchTime != progData->switchTime)) {
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META("unit %d program %u timing has changed;"
                                       " it is possible there will be issues"
                                       " if close to time-based capacity\n"),
                              unitData->unit,
                              programId));
                }
                if (memcmp(wbProg->actIndex,
                           progData->actIndex,
                           SOC_C3_RCE_RESULT_REGISTER_COUNT)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("unit %d program %u result register to"
                                        " action table associations changed\n"),
                               unitData->unit,
                               programId));
                    result = SOC_E_FAIL;
                    goto error;
                }
            } else { /* if (unitData->progData[index]) */
                /* there is no program here */
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d has no program %u but the backing"
                                    " store image has data for one\n"),
                           unitData->unit,
                           programId));
                result = SOC_E_FAIL;
                goto error;
            } /* if (unitData->progData[index]) */
        } else { /* if (wbUnit->progValidFlags & (1 << index)) */
            /* backing store says to expect no program here */
            if (unitData->progData[programId]) {
                /* there is a program here */
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META("unit %d has added program %u; after"
                                   " recovery, reversion will not be"
                                   " supported\n"),
                          unitData->unit,
                          programId));
            }
        } /* if (wbUnit->progValidFlags & (1 << index)) */
    } /* for (index = 0; index < SOC_C3_RCE_PROGRAM_COUNT; index++) */

    /* get and check the filter set index and descriptors and data space */
    wbFsetIdx = (_soc_c3_rce_wb_fset_index_t*)(&(unitData->scPtr[wbIndex->fsetOffs]));
    if (_SOC_C3_RCE_WB_FSET_INDEX_MAGIC != wbFsetIdx->magic) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d invalid magic (%08X, expected %08X) on"
                            " backing store filter set index\n"),
                   unitData->unit,
                   wbFsetIdx->magic,
                   _SOC_C3_RCE_WB_FSET_INDEX_MAGIC));
        result = SOC_E_FAIL;
        goto error;
    }
    scSize = (wbFsetIdx->fsetOffs +
              (wbFsetIdx->fsetTotSize * wbUnit->filterSetLimit));
    if (scSize > wbFsetIdx->imemOffs) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d filter set descriptor space %u extends"
                            " into opcode and range tracking space %u\n"),
                   unitData->unit,
                   scSize,
                   wbFsetIdx->imemOffs));
        result = SOC_E_FAIL;
        goto error;
    }
    scSize += (wbFsetIdx->imemTotSize *
               C3_RCE_IMEM_SIZE *
               C3_RCE_IMEM_INSTR_PER_BLOCK);
    if (scSize > wbFsetIdx->pattOffs) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d opcode and tracking space %u extends"
                            " into pattern space %u\n"),
                   unitData->unit,
                   scSize,
                   wbFsetIdx->pattOffs));
        result = SOC_E_FAIL;
        goto error;
    }
    scSize += wbIndex->fsetOffs;
    if (scSize > wbIndex->groupOffs) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d pattern space %u extends into group"
                            " partition %u\n"),
                   unitData->unit,
                   scSize,
                   wbIndex->groupOffs));
        result = SOC_E_FAIL;
        goto error;
    }
    if (scSize > unitData->scSize) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d pattern space %u extends beyond"
                            " backing store space %u\n"),
                   unitData->unit,
                   scSize,
                   unitData->scSize));
        result = SOC_E_FAIL;
        goto error;
    }
    /* scan the filter sets */
    for (fsetId = 0, index = 0; fsetId < wbUnit->filterSetLimit; fsetId++) {
        address = (wbIndex->fsetOffs +
                   wbFsetIdx->fsetOffs +
                   (wbFsetIdx->fsetTotSize * fsetId));
        wbFset = (_soc_c3_rce_wb_fset_t*)(&(unitData->scPtr[address]));
        if (wbFset->fsetFlags & _SOC_C3_RCE_FSET_FLAGS_EXISTS) {
            if (0 < index) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d filter sets are not correctly"
                                    " packed in backing store\n"),
                           unitData->unit));
                result = SOC_E_FAIL;
                goto error;
            }
            if (fsetId > unitData->groupLimit) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d backing store contains filter"
                                    " sets that can not be supported now\n"),
                           unitData->unit));
                result = SOC_E_FAIL;
                goto error;
            }
            address = (wbFsetIdx->imemOffs +
                       wbFset->imemDataStart);
            if ((address +
                 (wbFset->imemSize * wbFsetIdx->imemTotSize)) >
                wbFsetIdx->pattOffs) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d filter set %u offset in imem"
                                    " space %u..%u is beyond pattern"
                                    " space %u\n"),
                           unitData->unit,
                           fsetId,
                           address,
                           address +
                           (wbFset->imemSize * wbFsetIdx->imemTotSize) - 1,
                           wbFsetIdx->pattOffs));
                result = SOC_E_FAIL;
                goto error;
            }
            address = (wbIndex->fsetOffs +
                       wbFsetIdx->pattOffs +
                       wbFset->pattDataStart);
            if ((address +
                 (wbFset->imemSize * C3_RCE_ENTRIES_PER_FILTER_SET)) >
                wbIndex->groupOffs) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d filter set %u offset in pmem"
                                    " space %u..%u is beyond group"
                                    " partition %u\n"),
                           unitData->unit,
                           fsetId,
                           address,
                           address +
                           (wbFset->imemSize * C3_RCE_ENTRIES_PER_FILTER_SET) - 1,
                           wbIndex->groupOffs));
                result = SOC_E_FAIL;
                goto error;
            }
        } else { /* if (wbFset->fsetFlags & _SOC_C3_RCE_FSET_FLAGS_EXISTS) */
            /* indicate we have hit an unused filter set position */
            index++;
        } /* if (wbFset->fsetFlags & _SOC_C3_RCE_FSET_FLAGS_EXISTS) */
    } /* for (all filter set descriptors in backing store) */
    /* make sure all program references to filter sets are valid */
    for (programId = 0, fsetId = 0;
         programId < SOC_C3_RCE_PROGRAM_COUNT;
         programId++) {
        if (0 == (wbUnit->progValidFlags & (1 << programId))) {
            /* this program does not exist; skip it */
            continue;
        }
        address = (wbIndex->progOffs +
                   wbProgIdx->progOffs +
                   (wbProgIdx->progTotSize * programId));
        wbProg = (_soc_c3_rce_wb_program_t*)(&(unitData->scPtr[address]));
        if (wbProg->firstFilterSet != fsetId) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("unit %d program %u claims to start at filter"
                                " set %u but should start at %u\n"),
                       unitData->unit,
                       programId,
                       wbProg->firstFilterSet,
                       fsetId));
            result = SOC_E_FAIL;
            goto error;
        }
        if ((fsetId + wbProg->filterSetCount) > wbUnit->filterSetLimit) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("unit %d program %u filter sets %u..%u extend"
                                " past the maximum number of filter sets %u\n"),
                       unitData->unit,
                       programId,
                       fsetId,
                       fsetId + wbProg->filterSetCount - 1,
                       wbUnit->filterSetLimit));
            result = SOC_E_FAIL;
            goto error;
        }
        for (index = 0; index < wbProg->filterSetCount; index++) {
            address = (wbFsetIdx->fsetOffs +
                       (wbFsetIdx->fsetTotSize * index));
            if (address > wbFsetIdx->imemOffs) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d program %u filter set %u at %08X"
                                    " extends into imem space at %08X\n"),
                           unitData->unit,
                           programId,
                           index,
                           address,
                           wbFsetIdx->imemOffs));
                result = SOC_E_FAIL;
                goto error;
            }
            address += wbIndex->fsetOffs;
            wbFset = (_soc_c3_rce_wb_fset_t*)(&(unitData->scPtr[address]));
            if (0 == (wbFset->fsetFlags & _SOC_C3_RCE_FSET_FLAGS_EXISTS)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d program %u filter set %u does"
                                    " not exist\n"),
                           unitData->unit,
                           programId,
                           index));
                result = SOC_E_FAIL;
                goto error;
            }
        }
        fsetId += wbProg->filterSetCount;
    } /* for (all possible programs) */

    /* get and check group index and references */
    wbGroupIdx = (_soc_c3_rce_wb_group_index_t*)(&(unitData->scPtr[wbIndex->groupOffs]));
    if (_SOC_C3_RCE_WB_GROUP_INDEX_MAGIC != wbGroupIdx->magic) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d invalid magic (%08X, expected %08X) on"
                            " backing store group index\n"),
                   unitData->unit,
                   wbGroupIdx->magic,
                   _SOC_C3_RCE_WB_GROUP_INDEX_MAGIC));
        result = SOC_E_FAIL;
        goto error;
    }
    scSize = wbGroupIdx->groupTotSize * wbUnit->groupLimit;
    if ((wbGroupIdx->groupOffs + scSize) > wbIndex->entryOffs) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d group descriptor space %u..%u extends"
                            " into entry partition %u\n"),
                   unitData->unit,
                   wbIndex->groupOffs,
                   wbIndex->groupOffs + scSize - 1,
                   wbIndex->entryOffs));
        result = SOC_E_FAIL;
        goto error;
    }
    /* scan the groups */
    for (groupId = 0; groupId < wbUnit->groupLimit; groupId++) {
        address = (wbIndex->groupOffs +
                   wbGroupIdx->groupOffs +
                   (wbGroupIdx->groupTotSize * groupId));
        wbGroup = (_soc_c3_rce_wb_group_t*)(&(unitData->scPtr[address]));
        if (wbGroup->groupFlags & _SOC_C3_RCE_GROUP_FLAG_EXISTS) {
            if (groupId >= unitData->groupLimit) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d group %u exists in backing store"
                                    " but is not supported in current config,"
                                    " limited to %u groups\n"),
                           unitData->unit,
                           groupId,
                           unitData->groupLimit));
                result = SOC_E_FAIL;
                goto error;
            }
            if (!unitData->progData[wbGroup->rceProgram]) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d group %u claims to be in program"
                                    " %u, which does not exist\n"),
                           unitData->unit,
                           groupId,
                           wbGroup->rceProgram));
                result = SOC_E_FAIL;
                goto error;
            }
            if (wbGroup->qualCount > SOC_C3_RCE_GROUP_QUAL_MAX) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d group %u has %u qualifiers,"
                                    " but currently only %u per group"
                                    " are allowed\n"),
                           unitData->unit,
                           groupId,
                           wbGroup->qualCount,
                           SOC_C3_RCE_GROUP_QUAL_MAX));
                result = SOC_E_FAIL;
                goto error;
            }
            address = (wbIndex->progOffs +
                       wbProgIdx->progOffs +
                       (wbProgIdx->progTotSize * wbGroup->rceProgram));
            wbProg = (_soc_c3_rce_wb_program_t*)(&(unitData->scPtr[address]));
            if ((wbGroup->filterSetCount + wbGroup->firstFilterSet) >
                wbProg->filterSetCount) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d program %u group %u claims filter"
                                    " sets %u..%u in program, but program %u"
                                    " only has %u filter sets\n"),
                           unitData->unit,
                           wbGroup->rceProgram,
                           groupId,
                           wbGroup->firstFilterSet,
                           wbGroup->filterSetCount + wbGroup->firstFilterSet - 1,
                           wbGroup->rceProgram,
                           wbProg->filterSetCount));
            }
            for (index = 0;
                 index < SOC_C3_RCE_RESULT_REGISTER_COUNT;
                 index++) {
                if (wbGroup->resultLrp & (1 << index)) {
                    /* uses this result register */
                    if (unitData->progData[wbGroup->rceProgram]->actIndex[index] >=
                        unitData->actTableCount) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META("unit %d program %u group %u"
                                            " claims result register %u,"
                                            " but that result register is"
                                            " unused in program %u\n"),
                                   unitData->unit,
                                   wbGroup->rceProgram,
                                   groupId,
                                   index,
                                   wbGroup->rceProgram));
                        result = SOC_E_FAIL;
                        goto error;
                    }
                } /* if (the group uses this result register) */
            } /* for (all possible result registers) */
        } /* if (this group exists in the backing store) */
    } /* for (all possible groups in backing store) */
    /* make sure filter sets refer to valid ranges */
    for (programId = 0, fsetId = 0;
         programId < SOC_C3_RCE_PROGRAM_COUNT;
         programId++) {
        if (0 == (wbUnit->progValidFlags & (1 << programId))) {
            /* this program does not exist; skip it */
            continue;
        }
        address = (wbIndex->progOffs +
                   wbProgIdx->progOffs +
                   (wbProgIdx->progTotSize * programId));
        wbProg = (_soc_c3_rce_wb_program_t*)(&(unitData->scPtr[address]));
        for (index = 0; index < wbProg->filterSetCount; index++, fsetId++) {
            address = (wbIndex->fsetOffs +
                       wbFsetIdx->fsetOffs +
                       (wbFsetIdx->fsetTotSize * fsetId));
            wbFset = (_soc_c3_rce_wb_fset_t*)(&(unitData->scPtr[address]));
            groupId = wbFset->groupId;
            if (groupId > wbUnit->groupLimit) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d program %u filter set %u claims"
                                    " to be part of invalid group %u\n"),
                           unitData->unit,
                           programId,
                           index,
                           groupId));
                result = SOC_E_FAIL;
                goto error;
            }
            address = (wbIndex->groupOffs +
                       wbGroupIdx->groupOffs +
                       (wbGroupIdx->groupTotSize * groupId));
            wbGroup = (_soc_c3_rce_wb_group_t*)(&(unitData->scPtr[address]));
            if (0 == (wbGroup->groupFlags & _SOC_C3_RCE_GROUP_FLAG_EXISTS)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d program %u filter set %u claims"
                                    " to belong to non-existing group %u\n"),
                           unitData->unit,
                           programId,
                           index,
                           groupId));
                result = SOC_E_FAIL;
                goto error;
            }
            address = (wbIndex->fsetOffs +
                       wbFsetIdx->imemOffs +
                       wbFset->imemDataStart);
            wbFsetImem = (_soc_c3_rce_wb_fset_imem_t*)(&(unitData->scPtr[address]));
            for (offset = 0; offset < (wbGroup->instrBlocks << 3); offset++) {
                if (0 == offset) {
                    if (!COMPILER_64_IS_ZERO(wbFsetImem[offset].instr)) {
                        /* start instruction is not encoded in backing store */
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META("unit %d program %u filter set %u"
                                            " (group %u) instruction %u should"
                                            " be zeroed, but is %08X%08X\n"),
                                   unitData->unit,
                                   programId,
                                   index,
                                   wbFset->groupId,
                                   offset,
                                   COMPILER_64_HI(wbFsetImem[offset].instr),
                                   COMPILER_64_LO(wbFsetImem[offset].instr)));
                        result = SOC_E_FAIL;
                        goto error;
                    }
                } else if (offset <= wbGroup->rangesPerFilterSet) {
                    /* range instructions kept as data in backing store */
                    if (COMPILER_64_HI(wbFsetImem[offset].instr) &
                                       _SOC_C3_RCE_RANGE_FLAG_IN_TEMPLATE) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META("unit %d program %u filter set %u"
                                            " (group %u) instruction %u should"
                                            " be range (%d), but claims to be"
                                            " template\n"),
                                   unitData->unit,
                                   programId,
                                   index,
                                   wbFset->groupId,
                                   wbGroup->rangesPerFilterSet,
                                   offset));
                        result = SOC_E_FAIL;
                        goto error;
                    }
                    rangeId = wbFsetImem[offset].rangeData.rangeId;
                    if (rangeId) {
                        if (rangeId > wbUnit->rangeLimit) {
                            LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META("unit %d program %u filter set"
                                                " %u (group %u) instruction %u"
                                                " (%u ranges) claims to refer"
                                                " to invalid range %u\n"),
                                       unitData->unit,
                                       programId,
                                       index,
                                       wbFset->groupId,
                                       offset,
                                       wbGroup->rangesPerFilterSet,
                                       rangeId));
                            result = SOC_E_FAIL;
                            goto error;
                        }
                        rangeId--;
                        address = (wbIndex->rangeOffs +
                                   wbRangeIdx->rangeOffs +
                                   (wbRangeIdx->rangeTotSize * rangeId));
                        wbRange = (_soc_c3_rce_wb_range_t*)(&(unitData->scPtr[address]));
                        if (0 == (wbRange->rangeFlags & SOC_C3_RCE_RANGE_FLAG_VALID)) {
                            LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META("unit %d program %u filter set"
                                                " %u (group %u) instruction %u"
                                                " (%u ranges) refers to"
                                                " non-existing range %u\n"),
                                       unitData->unit,
                                       programId,
                                       index,
                                       wbFset->groupId,
                                       offset,
                                       wbGroup->rangesPerFilterSet,
                                       rangeId + 1));
                            result = SOC_E_FAIL;
                            goto error;
                        }
                    }
                } else if (offset < (wbGroup->instrBlocks << 3)) {
                    /* should be normal instructions in backing store */
                    if (0 == (COMPILER_64_HI(wbFsetImem[offset].instr) &
                                             _SOC_C3_RCE_RANGE_FLAG_IN_TEMPLATE)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META("unit %d program %u filter set %u"
                                            " (group %u) instruction %u should"
                                            " be template (%d,%d), but claims"
                                            " to be range\n"),
                                   unitData->unit,
                                   programId,
                                   index,
                                   wbFset->groupId,
                                   offset,
                                   wbGroup->rangesPerFilterSet,
                                   wbGroup->instrBlocks));
                        result = SOC_E_FAIL;
                        goto error;
                    }
                } else {
                    if (!COMPILER_64_IS_ZERO(wbFsetImem[offset].instr)) {
                        /*
                         *  Instructions added to pad the group so its template
                         *  is at least the minimum filter set length are left
                         *  zeroed in backing store.
                         */
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META("unit %d program %u filter set %u"
                                            " (group %u) instruction %u should"
                                            " be zeroed, but is %08X%08X\n"),
                                   unitData->unit,
                                   programId,
                                   index,
                                   wbFset->groupId,
                                   offset,
                                   COMPILER_64_HI(wbFsetImem[offset].instr),
                                   COMPILER_64_LO(wbFsetImem[offset].instr)));
                        result = SOC_E_FAIL;
                        goto error;
                    }
                }
            } /* for (all instructions in this filter set) */
        } /* for (all filter sets in this program) */
    } /* for (all programs) */

    /* get and check entry index and references */
    wbEntryIdx = (_soc_c3_rce_wb_entry_index_t*)(&(unitData->scPtr[wbIndex->entryOffs]));
    if (_SOC_C3_RCE_WB_ENTRY_INDEX_MAGIC != wbEntryIdx->magic) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d invalid magic (%08X, expected %08X) on"
                            " backing store entry index\n"),
                   unitData->unit,
                   wbEntryIdx->magic,
                   _SOC_C3_RCE_WB_ENTRY_INDEX_MAGIC));
        result = SOC_E_FAIL;
        goto error;
    }
    scSize = wbEntryIdx->entryTotSize * wbUnit->entryLimit;
    if ((wbIndex->entryOffs + wbEntryIdx->entryOffs + scSize) >
        unitData->scSize) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d entry descriptor space %u..%u extends"
                            " beyond backing store limit %u\n"),
                   unitData->unit,
                   wbIndex->entryOffs,
                   wbIndex->entryOffs +
                   wbEntryIdx->entryOffs +
                   scSize - 1,
                   unitData->scSize));
        result = SOC_E_FAIL;
        goto error;
    }
    for (entryId = 0; entryId < wbUnit->entryLimit; entryId++) {
        address = (wbIndex->entryOffs +
                   wbEntryIdx->entryOffs +
                   (wbEntryIdx->entryTotSize * entryId));
        wbEntry = (_soc_c3_rce_wb_entry_t*)(&(unitData->scPtr[address]));
        if (wbEntry->entryFlags & SOC_C3_RCE_ENTRY_FLAG_EXISTS) {
            if (entryId >= unitData->entryLimit) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d entry %u exists in backing store"
                                    " but is not supported in current config"
                                    " (max %u entries)\n"),
                           unitData->unit,
                           entryId,
                           unitData->entryLimit));
                result = SOC_E_FAIL;
                goto error;
            }
            address = (wbIndex->groupOffs +
                       wbGroupIdx->groupOffs +
                       (wbGroupIdx->groupTotSize * wbEntry->entryGroup));
            wbGroup = (_soc_c3_rce_wb_group_t*)(&(unitData->scPtr[address]));
            if ((wbGroup->filterSetCount * C3_RCE_ENTRIES_PER_FILTER_SET) <=
                wbEntry->entryPosition) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d program %u group %u entry %u"
                                    " claims position %u, but group only has"
                                    " filter sets for %u positions\n"),
                           unitData->unit,
                           wbGroup->rceProgram,
                           wbEntry->entryGroup,
                           entryId,
                           wbEntry->entryPosition,
                           wbGroup->filterSetCount *
                           C3_RCE_ENTRIES_PER_FILTER_SET));
                result = SOC_E_FAIL;
                goto error;
            }
        } /* if (this entry exists in backing store) */
    } /* for (all possible entries) */

    /*
     *  At this point, we have checked to be sure the current configuration is
     *  at least compatible with the configuration that was used when
     *  generating the data in the backing store.  There are still some things
     *  that can go wrong, but those are hopefully limited to allocating memory
     *  or accessing the hardware.  Not much to do about the latter, but the
     *  former could trip us up, because we're going to be growing the actual
     *  memory use for the RCE driver at the same time as keeping some possibly
     *  very large buffers around for DMA work.
     *
     *  If there is something missed in the checks above, it should be added to
     *  them, above.  The idea from here is that other than memory handling and
     *  hardware access, nothing should go wrong (and we hope neither of these
     *  cause problems).
     */

    /* figure out how big we need to make the filter set buffers */
    for (fsetId = 0, daSize = 0; fsetId < wbUnit->filterSetLimit; fsetId++) {
        address = (wbIndex->fsetOffs +
                   wbFsetIdx->fsetOffs +
                   (wbFsetIdx->fsetTotSize * fsetId));
        wbFset = (_soc_c3_rce_wb_fset_t*)(&(unitData->scPtr[address]));
        if (wbFset->fsetFlags & _SOC_C3_RCE_FSET_FLAGS_EXISTS) {
            if (daSize < wbFset->imemSize) {
                daSize = wbFset->imemSize;
            }
        }
    } /* for (all possible filter sets in backing store) */
    for (actTblId = 0, paSize = 0;
         actTblId < wbUnit->actionTableCount;
         actTblId++) {
        address = (wbIndex->actTblOffs +
                   wbActTblIdx->actTblOffs +
                   (wbActTblIdx->actTblTotSize * actTblId));
        wbActTbl = (_soc_c3_rce_wb_actTbl_t*)(&(unitData->scPtr[address]));
        paSize += wbActTbl->actionBytes;
    } /* for (all action tables in backing store) */

    /* allocate DMA buffers according to largest filter set */
    progBlockSize = (sizeof(*progBlock) * daSize);
    RCE_EVERB((RCE_MSG1("unit %d warm boot alloc %u bytes for IMEM_PMEM_DMA"
                        " workspace\n"),
               unitData->unit,
               progBlockSize));
    progBlock = sal_dma_alloc(progBlockSize,
                              "C3 RCE warm boot imem/pattern read buffer");
    if (!progBlock) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d unable to allocate %u bytes buffer for"
                            " accessing imem_pmem_dma\n"),
                   unitData->unit,
                   progBlockSize));
        result = SOC_E_MEMORY;
        goto error;
    }
    /* note: actTemp is not DMA because OCM accessor rearranges en route */
    RCE_EVERB((RCE_MSG1("unit %d warm boot alloc %u bytes for action table"
                        " workspace\n"),
               unitData->unit,
               paSize * C3_RCE_ENTRIES_PER_FILTER_SET));
    actBuffer = sal_alloc(paSize * C3_RCE_ENTRIES_PER_FILTER_SET,
                          "C3 RCE warm boot action table read buffer");
    if (!actBuffer) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %u unable to allocate %u bytes buffer for"
                            " acessing action tables\n"),
                   unitData->unit,
                   (unsigned int)(paSize * C3_RCE_ENTRIES_PER_FILTER_SET)));
        result = SOC_E_MEMORY;
        goto error;
    }

    /*
     *  Now we have memory for the workspace, so we can start on recovery of
     *  contents.  It is possible to run out of memory while recovering
     *  content, as well as to have hardware access failures, but as long as
     *  those are the only situations that can cause problems here, we are
     *  doing well.  Checks for other conditions should already be done above.
     */

    /* recover ranges (purely backing store) */
    for (rangeId = 0; rangeId < wbUnit->rangeLimit; rangeId++) {
        /*
         *  NOTE: ranges will not be 'upgraded' to support new programs
         *  during recovery; only newly created ranges will be useable in
         *  programs that were added across a warm boot.
         *
         *  It is possible to do such an 'upgrade', but would require prep work
         *  similar to what it required to create them originally.
         */
        address = (wbIndex->rangeOffs +
                   wbRangeIdx->rangeOffs +
                   (wbRangeIdx->rangeTotSize * rangeId));
        wbRange = (_soc_c3_rce_wb_range_t*)(&(unitData->scPtr[address]));
        if (wbRange->rangeFlags & SOC_C3_RCE_RANGE_FLAG_VALID) {
            /* this range exists in backing store; recover it */
            RCE_EVERB((RCE_MSG1("recover unit %d range %u\n"),
                       unitData->unit,
                       rangeId));
            address += wbRangeIdx->rangeSize;
            wbName = (char*)(&(unitData->scPtr[address]));
            paSize = sal_strlen(wbName) + 1;
            if (paSize > C3_RCE_RANGE_NAME_BYTES) {
                paSize = C3_RCE_RANGE_NAME_BYTES;
            }
            scSize = sizeof(*rangeData) + paSize;
            rangeData = sal_alloc(scSize, "C3 RCE range data");
            if (!rangeData) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unable to allocate %u bytes for range"
                                    " data\n"),
                           scSize));
                result = SOC_E_MEMORY;
                goto error;
            }
            sal_memset(rangeData, 0x00, scSize);
            rangeData->rangeFlags = wbRange->rangeFlags;
            rangeData->validProgs = wbRange->validProgs;
            rangeData->lowerBound = wbRange->lowerBound;
            rangeData->upperBound = wbRange->upperBound;
            rangeData->headerField.header = wbRange->header;
            rangeData->headerField.startBit = wbRange->startBit;
            rangeData->headerField.numBits = wbRange->numBits;
            /* we allocated space for the string just beyond the descriptor */
            rangeData->headerField.fieldName = (char*)(&(rangeData[1]));
            sal_strncpy(rangeData->headerField.fieldName,
                        wbName,
                        paSize - 1);
            /* attach this range to the unit */
            unitData->rangeData[rangeId] = rangeData;
            /*
             *  Need to fill in later: reference count
             */
        } /* if (this range is valid in backing store) */
    } /* for (all possible ranges in backing store) */

    /*
     *  Programs contain groups contain filter sets contain entries.  So,
     *  the next thing we will recover should be groups.
     */
    for (groupId = 0; groupId < wbUnit->groupLimit; groupId++) {
        address = (wbIndex->groupOffs +
                   wbGroupIdx->groupOffs +
                   (wbGroupIdx->groupTotSize * groupId));
        wbGroup = (_soc_c3_rce_wb_group_t*)(&(unitData->scPtr[address]));
        if (wbGroup->groupFlags & _SOC_C3_RCE_GROUP_FLAG_EXISTS) {
            /* this group exists in backing store; recover it */
            RCE_EVERB((RCE_MSG1("recover unit %d group %u\n"),
                       unitData->unit,
                       groupId));
            groupData = sal_alloc(sizeof(*groupData), "C3 RCE group data");
            if (!groupData) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unable to allocate %u bytes for unit %d"
                                    " group %u information\n"),
                           (unsigned int)(sizeof(*groupData)),
                           unitData->unit,
                           groupId));
                result = SOC_E_MEMORY;
                goto error;
            }
            sal_memset(groupData, 0x00, sizeof(*groupData));
            groupData->resultLrp = wbGroup->resultLrp;
            groupData->resultRce = wbGroup->resultRce;
            groupData->resultLrpUniq = wbGroup->resultLrpUniq;
            groupData->rceProgram = wbGroup->rceProgram;
            groupData->groupId = groupId;
            groupData->groupPriority = wbGroup->groupPriority;
#if (C3_RCE_ENABLE_ACTION_INDEX_COUNTERS || C3_RCE_ENABLE_ENTRY_INSERT_HEURISTIC)
            groupData->groupFlags = wbGroup->groupFlags;
#endif /* (C3_RCE_ENABLE_ACTION_INDEX_COUNTERS || C3_RCE_ENABLE_ENTRY_INSERT_HEURISTIC) */
            groupData->qualCount = wbGroup->qualCount;
            groupData->rangesPerFilterSet = wbGroup->rangesPerFilterSet;
            groupData->maxFilterSets = wbGroup->maxFilterSets;
            /* note: this is always an integral number of blocks */
            groupData->instrCount = (wbGroup->instrBlocks *
                                     C3_RCE_IMEM_INSTR_PER_BLOCK);
            groupData->instrBlocks = wbGroup->instrBlocks;
            groupData->dataSize = wbGroup->dataSize;
            groupData->filterSetCount = wbGroup->filterSetCount;
            groupData->entryLastAdded = wbGroup->entryLastAdded;
            for (index = 0, scSize = 0; index < wbGroup->qualCount; index++) {
                RCE_EVERB((RCE_MSG1("unit %d recover group %d qualifier %u\n"),
                           unitData->unit,
                           groupData->groupId,
                           index));
                offset = (wbGroupIdx->groupSize +
                          ((wbGroupIdx->groupQualSize +
                            wbGroupIdx->groupQualNameMax) *
                           index));
                wbQual = (_soc_c3_rce_wb_qual_t*)(&(unitData->scPtr[address + offset]));
                wbName = (char*)(&(unitData->scPtr[address + offset + wbGroupIdx->groupQualSize]));
                paSize = sal_strlen(wbName) + 1;
                if (paSize > C3_RCE_QUAL_NAME_BYTES) {
                    paSize = C3_RCE_QUAL_NAME_BYTES;
                }
                scSize = (sizeof(*qualData) +
                          (sizeof(qualData->param[0]) * wbQual->paramCount) +
                          paSize);
                qualData = sal_alloc(scSize, "C3 RCE group qualifier data");
                if (!qualData) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("unable to allocate %d bytes for unit"
                                        " %d group %d qualifier information\n"),
                               sizeof(*(groupData->qualData[index])) + scSize,
                               unitData->unit,
                               groupId));
                    result = SOC_E_MEMORY;
                    goto error;
                }
                sal_memset(qualData, 0x00, scSize);
                qualData->param = (int*)(&(qualData[1]));
                qualData->qualName = (char*)(&(qualData->param[wbQual->paramCount]));
                qualData->qualType = wbQual->qualType;
                qualData->paramCount = wbQual->paramCount;
                /* simple qualifiers only have two parameters; recover those */
                switch (wbQual->paramCount) {
                case 2:
                    qualData->param[1] = wbQual->param[1];
                    /* fallthrough intentional */
                case 1:
                    qualData->param[0] = wbQual->param[0];
                    break;
                default:
                    /* nothing to do here for other cases */
                    break;
                }
                sal_strncpy(qualData->qualName,
                            wbName,
                            paSize - 1);
                groupData->qualData[index] = qualData;
                groupData->qualOffs[index] = wbQual->qualOffs;
            } /* for (each qualifier in this group) */
            /*
             *  will need to fill in later: groupPrev, groupNext, instr,
             *  defaultPattern, entryCount, entryLastAdded, entryHead,
             *  entryTail, param[] for any sparse qualifiers
             */
            unitData->groupData[groupId] = groupData;
        } /* if (this group is valid in backing store) */
    } /* for (all possible groups in backing store) */

    /*
     *  Now recover filter sets.  While doing this, link the groups into their
     *  respective programs and fill in the data for the groups that we left
     *  out because it was not explicitly given with the group in backing
     *  store.  It's still there, but it's part of the filter set information
     *  instead, so this is a good time to get it back.
     */
    unitData->instrBlockCount = 0; /* just count this from scratch */
    for (programId = 0; programId < SOC_C3_RCE_PROGRAM_COUNT; programId++) {
        if (0 == (wbUnit->progValidFlags & (1 << programId))) {
            /* this program is not valid in backing store */
            continue;
        }
        address = (wbIndex->progOffs +
                   wbProgIdx->progOffs +
                   (wbProgIdx->progTotSize * programId));
        wbProg = (_soc_c3_rce_wb_program_t*)(&(unitData->scPtr[address]));
        /* update program data from backing store information */
        progData = unitData->progData[programId];
        progData->instrBlockCount = wbProg->instrBlockCount;
        progData->instrBlockAdded = wbProg->instrBlockAdded;
        unitData->instrBlockCount += (progData->instrBlockCount +
                                      progData->instrBlockAdded);
        progData->groupCount = 0;
        /* always ensure 'new' group on first fset in program */
        groupId = wbUnit->groupLimit;
        groupData = NULL; 
        /* set up a filter set list for the program */
        scSize = sizeof(*(progData->fsetData)) * wbProg->filterSetCount;
        RCE_EVERB((RCE_MSG1("unit %d recover program %u filter sets (%u)\n"),
                   unitData->unit,
                   programId,
                   wbProg->filterSetCount));
        progData->fsetData = sal_alloc(scSize,
                                       "C3 RCE program filter set list");
        if (!progData->fsetData) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("unit %d unable to allocate %u bytes for"
                                " program filter set list\n"),
                       unitData->unit,
                       scSize));
            result = SOC_E_MEMORY;
            goto error;
        }
        sal_memset(progData->fsetData, 0x00, scSize);
        progData->filterSetCount = wbProg->filterSetCount;
        /* scan the filter sets in the program */
        for (fsetId = 0; fsetId < wbProg->filterSetCount; fsetId++) {
            address = (wbIndex->fsetOffs +
                       wbFsetIdx->fsetOffs +
                       (wbFsetIdx->fsetTotSize *
                        (fsetId + wbProg->firstFilterSet)));
            wbFset = (_soc_c3_rce_wb_fset_t*)(&(unitData->scPtr[address]));
            fsetData = &(progData->fsetData[fsetId]);
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
            fsetData->fsetFlags = wbFset->fsetFlags;
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
            fsetData->groupId = wbFset->groupId;
            fsetData->filterIndex = wbFset->filterIndex;
            fsetData->imemStart = wbFset->imemStart;
            fsetData->imemSize = wbFset->imemSize;
            fsetData->amemBlock = wbFset->amemBlock;
            if (wbFset->groupId != groupId) {
                /* this fset is first in a group; get group information */
                groupId = wbFset->groupId;
                /* coverity[assigned_value] */
                address = (wbIndex->groupOffs +
                           wbGroupIdx->groupOffs +
                           (wbGroupIdx->groupTotSize * groupId));
                
                groupData = unitData->groupData[groupId];
                /* link the group into the program */
                if (!progData->groupHead) {
                    progData->groupHead = groupData;
                }
                groupData->groupPrev = progData->groupTail;
                if (groupData->groupPrev) {
                    groupData->groupPrev->groupNext = groupData;
                }
                progData->groupTail = groupData;
                progData->groupCount++;
            }
            RCE_EVERB((RCE_MSG1("recover unit %d program %u filter set %u"
                                " (group %u filter set %u) (%u)\n"),
                       unitData->unit,
                       programId,
                       fsetId,
                       groupId,
                       wbFset->filterIndex,
                       fsetId + wbProg->firstFilterSet));
            if (NULL == groupData) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d groupData is NULL\n"),
                           unitData->unit));
                result = SOC_E_INTERNAL;
                goto error;
            }
            if (groupData->rangesPerFilterSet) {
                /* need range information for this filter set */
                scSize = (sizeof(*(fsetData->rangeInfo)) *
                          groupData->rangesPerFilterSet);
                fsetData->rangeInfo = sal_alloc(scSize,
                                                "C3 RCE filter set range list");
                if (!fsetData->rangeInfo) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("unit %d unable to allocate %u bytes"
                                        " for filter set range list\n"),
                               unitData->unit,
                               scSize));
                    result = SOC_E_MEMORY;
                    goto error;
                }
                sal_memset(fsetData->rangeInfo, 0x00, scSize);
                /* but we fill it in later, when scanning entries */
            } /* if (groupData->rangesPerFilterSet) */
            if (0 == wbFset->filterIndex) {
                /*
                 *  For first filter set in a group, need to allocate space for
                 *  template instrucitons and default pattern.  We could
                 *  recover some of this from hardware, but we can't store
                 *  certain additional bits (such as range specifics or the
                 *  default pattern) in hardware, so we might as well use that
                 *  extra space to store the template instructions as well.
                 */
                scSize = (groupData->instrCount *
                          sizeof(*(groupData->instr)));
                groupData->instr = sal_alloc(scSize,
                                             "C3 RCE group instructions");
                if (!groupData->instr) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("unit %d unable to allocate %u bytes"
                                        " for group %u instruction template"),
                               unitData->unit,
                               scSize,
                               groupId));
                    result = SOC_E_MEMORY;
                    goto error;
                }
                /* we're going to directly overwrite instr, so don't zero it */
                groupData->defaultPattern = sal_alloc(groupData->instrBlocks,
                                                      "C3 RCE group default pattern");
                if (!groupData->defaultPattern) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("unit %d unable to allocate %u bytes"
                                        " for group %u default pattern\n"),
                               unitData->unit,
                               groupData->instrBlocks,
                               groupId));
                    result = SOC_E_MEMORY;
                    goto error;
                }
                /* but we're only going to set '1' bits, so zero dflt patt */
                sal_memset(groupData->defaultPattern,
                           0x00,
                           groupData->instrBlocks);
            } /* if (0 == wbFset->filterIndex) */
            address = (wbIndex->fsetOffs +
                       wbFsetIdx->imemOffs +
                       wbFset->imemDataStart);
            wbFsetImem = (_soc_c3_rce_wb_fset_imem_t*)(&(unitData->scPtr[address]));
            /* scan instructions and default pattern from backing store */
            for (index = 0; index < groupData->instrCount; index++) {
                if (0 == index) {
                    if (0 == wbFset->filterIndex) {
                        /*
                         *  Group template for start instruction only includes
                         *  the resultLrp and resultRce values; everything else
                         *  about the start instruction is generated when
                         *  building the individual filter sets.  The default
                         *  pattern for the start instruction is always zero.
                         */
                        COMPILER_64_SET(groupData->instr[index],
                                        0,
                                        (((groupData->resultLrp & 0x0F) << 20) |
                                         ((groupData->resultRce & 0x0F) << 24)));
                    }
                } else if (index <= groupData->rangesPerFilterSet) {
                    /*
                     *  All filter sets can have independent range comparisons,
                     *  so we need to recover the range information for all
                     *  filter sets in every group.
                     */
                    if (wbFsetImem[index].rangeData.rangeId) {
                        fsetData->rangeInfo[index - 1].rangeFlags = wbFsetImem[index].rangeData.rangeFlags;
                        fsetData->rangeInfo[index - 1].rangeId = wbFsetImem[index].rangeData.rangeId;
                        rangeData = unitData->rangeData[wbFsetImem[index].rangeData.rangeId - 1];
                        fsetData->rangeInfo[index - 1].lowerLimit = rangeData->lowerBound;
                        fsetData->rangeInfo[index - 1].upperLimit = rangeData->upperBound;
                        if (rangeData->headerField.numBits < 16) {
                            offset = 16 - rangeData->headerField.numBits;
                            fsetData->rangeInfo[index - 1].lowerLimit <<= offset;
                            fsetData->rangeInfo[index - 1].upperLimit <<= offset;
                            fsetData->rangeInfo[index - 1].upperLimit |= ((1 << offset) - 1);
                        }
                        RCE_EVERB((RCE_MSG1("unit %d program %u filter set %u"
                                            " group %d filter set %u instr %u"
                                            " range %u %08X %04X..%04X\n"),
                                   unitData->unit,
                                   programId,
                                   fsetId,
                                   fsetData->groupId,
                                   fsetData->filterIndex,
                                   index,
                                   fsetData->rangeInfo[index - 1].rangeId,
                                   fsetData->rangeInfo[index - 1].rangeFlags,
                                   fsetData->rangeInfo[index - 1].lowerLimit,
                                   fsetData->rangeInfo[index - 1].upperLimit));
                    } /* if (this range slot is in use) */
                    if (0 == wbFset->filterIndex) {
                        /*
                         *  Group template for range instructions is always all
                         *  NOPs; default pattern for range instructions is
                         *  always all zeroes (which we already did).
                         */
                        COMPILER_64_SET(groupData->instr[index],
                                        (_c3RCEOpcode_nop << 8),
                                        0);
                    }
                } else {
                    if (0 == wbFset->filterIndex) {
                        /* recover group template */
                        groupData->instr[index] = wbFsetImem[index].instr;
                        COMPILER_64_AND(groupData->instr[index], instrMask);
                        /* recover default pattern */
                        if (COMPILER_64_HI(groupData->instr[index]) &
                            _SOC_C3_RCE_RANGE_FLAG_PATT_DEFAULT) {
                            groupData->defaultPattern[(index >> 3)] |= (1 << (index & 7));
                        }
                    } else {
                        /* nothing more to do on later filter sets */
                        break;
                    }
                }
            } /* for (all instructions in the group's template) */
            if (0 == wbFset->filterIndex) {
                /*
                 *  Now that we have the instruction template recovered for
                 *  this group, we can recover complex qualifiers (in
                 *  particular, the sparse ones with more than two arguments).
                 */
                for (index = 0; index < groupData->qualCount; index++) {
                    qualData = groupData->qualData[index];
                    if (qualData->paramCount <= 2) {
                        /*
                         *  Simple qualifiers (up to two parameters) were kept
                         *  in backing store and so already restored; skip such
                         *  qualifiers here.
                         */
                        continue;
                    }
                    result = _soc_c3_rce_wb_program_recover_general(qualData,
                                                                    &(groupData->instr[groupData->qualOffs[index]]));
                    if (SOC_E_NONE != result) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META("unit %u program %u group %u"
                                            " unable to recover qualifier %u:"
                                            " %d (%s)\n"),
                                   unitData->unit,
                                   programId,
                                   groupId,
                                   index,
                                   result,
                                   _SHR_ERRMSG(result)));
                        goto error;
                    }
                } /* for (all qualifiers in this group) */
            } /* if (0 == wbFset->filterIndex) */
        } /* for (all filter sets in this program) */
    } /* for (all possible programs in backing store) */

    /*
     *  Entries next.  Like groups, some bits will be left out and we have to
     *  go back through things to connect everything and fill it all in.
     */
    for (entryId = 0; entryId < wbUnit->entryLimit; entryId++) {
        address = (wbIndex->entryOffs +
                   wbEntryIdx->entryOffs +
                   (wbEntryIdx->entryTotSize * entryId));
        wbEntry = (_soc_c3_rce_wb_entry_t*)(&(unitData->scPtr[address]));
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
        wbBasisCounts = (uint64*)(&(unitData->scPtr[address + wbEntryIdx->entrySize]));
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
        if (0 == (wbEntry->entryFlags & SOC_C3_RCE_ENTRY_FLAG_EXISTS)) {
            /* this entry does not exist in backing store; skip it */
            continue;
        }
        RCE_EVERB((RCE_MSG1("unit %d recover entry %u\n"),
                   unitData->unit,
                   entryId));
        groupId = wbEntry->entryGroup;
        groupData = unitData->groupData[groupId];
        programId = groupData->rceProgram;
        progData = unitData->progData[programId];
        scSize = sizeof(*entryData) + (groupData->dataSize << 1);
        entryData = sal_alloc(scSize, "C3 RCE entry data");
        if (!entryData) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("unit %d unable to allocate %u bytes for"
                                " entry %u descriptor\n"),
                       unitData->unit,
                       scSize,
                       entryId));
            result = SOC_E_MEMORY;
            goto error;
        }
        sal_memset(entryData, 0x00, scSize);
        entryData->pattData[0] = (uint8*)(&(entryData[1]));
        entryData->pattData[1] = &(entryData->pattData[0][groupData->dataSize]);
        scSize = groupData->instrBlocks;
        for (index = 0; index < SOC_C3_RCE_RESULT_REGISTER_COUNT; index++) {
            if (groupData->resultLrp & (1 << index)) {
                /* this result register is being used by this group */
                for (offset = 0; offset < index; offset++) {
                    if ((groupData->resultLrpUniq & (1 << offset)) &&
                        (progData->actIndex[offset] ==
                         progData->actIndex[index])) {
                        /* uses the same action table, so same action data */
                        break;
                    }
                }
                if (offset < index) {
                    /* uses action data space we already allocated */
                    entryData->actData[0][index] = entryData->actData[0][offset];
                    entryData->actData[1][index] = entryData->actData[1][offset];
                } else { /* if (offset < index) */
                    /* need to assign new action space for this one */
                    entryData->actData[0][index] = &(entryData->pattData[0][scSize]);
                    entryData->actData[1][index] = &(entryData->pattData[1][scSize]);
                    scSize += unitData->actData[progData->actIndex[index]]->actionBytes;
                } /* if (offset < index) */
            } /* if (the group uses this result register) */
        } /* for (all possible result registers) */
        entryData->entryId = entryId;
        entryData->entryFlags = wbEntry->entryFlags;
        entryData->entryGroup = wbEntry->entryGroup;
        entryData->entryPriority = wbEntry->entryPriority;
        entryData->entryPosition = wbEntry->entryPosition;
        entryData->entryPosBackout = wbEntry->entryPosition;
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
        entryData->entryPosPrev = wbEntry->entryPosPrev;
        for (index = 0; index < wbUnit->entryBasisCounts; index++) {
            if (index >= SOC_C3_RCE_NUM_BASIS_COUNTERS) {
                /* if we are past the supported count here, skip it */
                break;
            }
            entryData->basisCounts[index] = wbBasisCounts[index];
        }
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
        unitData->entryData[entryId] = entryData;
        groupData->entryCount++;
        /*
         *  Will need to fill in later: entryNext, entryPrev, actData[],
         *  pattData[]
         */
    } /* for (all possible entries in backing store) */

    /*
     *  Finally, another pass through the programs at filter set resolution in
     *  order to link entries into their respective groups and to fill in the
     *  entry data that were omitted during the entry recovery, and to fill in
     *  the range reference counts in the filter sets, and the top level
     *  filter-set reference counts for ranges themselves.
     */
    for (programId = 0; programId < SOC_C3_RCE_PROGRAM_COUNT; programId++) {
        if (!unitData->progData[programId]) {
            /* no program here; skip it */
            continue;
        }
        progData = unitData->progData[programId];
        address = (wbIndex->progOffs +
                   wbProgIdx->progOffs +
                   (wbProgIdx->progTotSize * programId));
        wbProg = (_soc_c3_rce_wb_program_t*)(&(unitData->scPtr[address]));
        groupId = wbUnit->groupLimit;
        wbGroup = NULL; 
        groupData = NULL; 
        entryId = wbUnit->entryLimit;
        for (fsetId = 0; fsetId < progData->filterSetCount; fsetId++) {
            fsetData = &(progData->fsetData[fsetId]);
            address = (wbIndex->fsetOffs +
                       wbFsetIdx->fsetOffs +
                       (wbFsetIdx->fsetTotSize * (wbProg->firstFilterSet +
                                                  fsetId)));
            wbFset = (_soc_c3_rce_wb_fset_t*)(&(unitData->scPtr[address]));
            /* calculate first and last position for this filter set */
            daSize = wbFset->filterIndex * C3_RCE_ENTRIES_PER_FILTER_SET;
            paSize = daSize + (C3_RCE_ENTRIES_PER_FILTER_SET - 1);
            if (wbFset->groupId != groupId) {
                /* starting a new group; get the group information */
                groupId = wbFset->groupId;
                groupData = unitData->groupData[groupId];
                address = (wbIndex->groupOffs +
                           wbGroupIdx->groupOffs +
                           (wbGroupIdx->groupTotSize * groupId));
                wbGroup = (_soc_c3_rce_wb_group_t*)(&(unitData->scPtr[address]));
                if (wbGroup->entryHead < wbUnit->entryLimit) {
                    groupData->entryHead = unitData->entryData[wbGroup->entryHead];
                }
                if (wbGroup->entryTail < wbUnit->entryLimit) {
                    groupData->entryTail = unitData->entryData[wbGroup->entryTail];
                }
                entryId = wbGroup->entryHead;
            }
            RCE_EVERB((RCE_MSG1("unit %d recover additional data for program"
                                " %u filter set %u (group %u filter set %u)"
                                " (%u) and contained entries\n"),
                       unitData->unit,
                       programId,
                       fsetId,
                       wbFset->groupId,
                       wbFset->filterIndex,
                       fsetId + wbProg->firstFilterSet));
            if (entryId < wbUnit->entryLimit) {
                entryData = unitData->entryData[entryId];
            } else { /* if (entryId < wbUnit->entryLimit) */
                entryData = NULL;
            } /* if (entryId < wbUnit->entryLimit) */
            /* link the first entry in the filter set to the filter set */
            fsetData->entryHead = entryData;
            /* make sure filter set imem/pmem block is allocated */
            result = shr_mdb_alloc_id(unitData->imemRes,
                                      wbFset->imemStart,
                                      wbFset->imemSize);
            if (SOC_E_NONE != result) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d program %u filter set %u"
                                    " (group %u filter set %u) (%u)"
                                    " unable to mark imem/pmem"
                                    " %03X..%03X as in use: %d (%s)\n"),
                           unitData->unit,
                           programId,
                           fsetId,
                           wbFset->groupId,
                           wbFset->filterIndex,
                           fsetId + wbProg->firstFilterSet,
                           wbFset->imemStart,
                           wbFset->imemStart + wbFset->imemSize - 1,
                           result,
                           _SHR_ERRMSG(result)));
                goto error;
            }
            /* this imem block exists as expected, so count it */
            unitData->imemBlockCount++;
            /* load IMEM_PMEM_DMA instead of individual PMEM parts */
            sal_dma_flush(progBlock, progBlockSize);
            result = soc_mem_read_range(unitData->unit,
                                        IMEM_PMEM_DMAm,
                                        MEM_BLOCK_ANY,
                                        fsetData->imemStart << 5,
                                        ((fsetData->imemStart +
                                          fsetData->imemSize) << 5) - 1,
                                        progBlock);
            if (SOC_E_NONE != result) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d unable to read IMEM_PMEM_DMA"
                                    " %03X..%03X: %d (%s)\n"),
                           unitData->unit,
                           fsetData->imemStart,
                           fsetData->imemStart +
                           fsetData->imemSize - 1,
                           result,
                           _SHR_ERRMSG(result)));
                goto error;
            }
            sal_dma_inval(progBlock, progBlockSize);
            /* go through action tables, mark blocks in use and load them */
            for (index = 0, byteOffset = 0;
                 index < SOC_C3_RCE_RESULT_REGISTER_COUNT;
                 index++) {
                /* coverity[var_deref_op: FALSE] */
                if (0 == (groupData->resultLrpUniq & (1 << index))) {
                    /* group does not use this result register */
                    continue;
                }
                actTblData = unitData->actData[progData->actIndex[index]]->ucData;
                /* mark this block of this action table as in use */
                temp = wbFset->amemBlock;
                result = shr_mres_alloc(unitData->amemRes,
                                        progData->actIndex[index],
                                        SHR_RES_ALLOC_WITH_ID,
                                        1,
                                        &temp);
                if (SOC_E_NONE != result) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("unit %d program %u filter set %u"
                                        " (group %u filter set %u) (%u)"
                                        " unable to mark action table %u (%s)"
                                        " block %u as in use: %d (%s)\n"),
                               unitData->unit,
                               programId,
                               fsetId,
                               wbFset->groupId,
                               wbFset->filterIndex,
                               wbProg->firstFilterSet + fsetId,
                               progData->actIndex[index],
                               actTblData->tableName,
                               wbFset->amemBlock,
                               result,
                               _SHR_ERRMSG(result)));
                    goto error;
                }
                /* account for this block being in use */
                unitData->actData[progData->actIndex[index]]->entryActive++;
                /* compute base address in action table */
                address = fsetData->amemBlock;
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
                if (fsetData->fsetFlags & _SOC_C3_RCE_FSET_FLAGS_COUNTER) {
                    /* with action-indexed counters, blocks are double size */
                    address <<= 1;
                    if (0 == (fsetData->fsetFlags & _SOC_C3_RCE_FSET_FLAGS_PHASE)) {
                        /* now phase one */
                        address++;
                    }
                }
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
                address *= C3_RCE_ENTRIES_PER_FILTER_SET;
                address += unitData->actData[progData->actIndex[index]]->firstEntryBias;
                scSize = address + (C3_RCE_ENTRIES_PER_FILTER_SET - 1);
                for (offset = 0;
                     offset < actTblData->ocmSegments;
                     offset++) {
                    /* point to where we will put this table block */
                    actData[index][offset] = &(actBuffer[byteOffset]);
                    /* keep track of how many bytes for this segment */
                    actSize[index][offset] = ((actTblData->seg[offset].bitsPerAction + 7) >> 3);
                    /* calculate size of this action table block */
                    if (actTblData->seg[offset].elemsPerAction) {
                        rowOffset = (C3_RCE_ENTRIES_PER_FILTER_SET *
                                     actTblData->seg[offset].elemsPerAction);
                    } else {
                        rowOffset = (C3_RCE_ENTRIES_PER_FILTER_SET /
                                     actTblData->seg[offset].actionsPerElem);
                    }
                    rowOffset <<= 3;
                    /* adjust next block start by size of this one */
                    byteOffset += rowOffset;
#if 0 
                    
                    result = 0;
                    sal_memset(actData[index][offset],
                               0xFF,
                               rowOffset);
#else
#if 0
                    result = soc_sbx_caladan3_ocm_port_mem_read(unitData->unit,
                                                                actTblData->seg[offset].ocmPort,
                                                                actTblData->seg[offset].ocmSeg,
                                                                address,
                                                                address,
                                                                (uint32*)(actData[index][offset]));
#else
                    /* coverity[stack_use_overflow] */
                    result = soc_sbx_caladan3_ocm_port_mem_read(unitData->unit,
                                                                actTblData->seg[offset].ocmPort,
                                                                actTblData->seg[offset].ocmSeg,
                                                                address,
                                                                scSize,
                                                                (uint32*)(actData[index][offset]));
#endif
#endif 
                    if (SOC_E_NONE != result) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META("unit %d program %u filter set %u"
                                            " (group %u filter set %u) (%u)"
                                            " unable to read action table %u"
                                            " (%s) segment %u (%s) block %u"
                                            " (%08X..%08X): %d (%s)\n"),
                                   unitData->unit,
                                   programId,
                                   fsetId,
                                   wbFset->groupId,
                                   wbFset->filterIndex,
                                   wbProg->firstFilterSet + fsetId,
                                   progData->actIndex[index],
                                   actTblData->tableName,
                                   offset,
                                   actTblData->seg[offset].segmentName,
                                   wbFset->amemBlock,
                                   address,
                                   scSize,
                                   result,
                                   _SHR_ERRMSG(result)));
                        goto error;
                    }
                } /* for (all OCM segments in this action table) */
            } /* for (all action tables used by this group) */
            /* recover the pattern and action data for each entry */
            while (entryId < wbUnit->entryLimit) {
                entryData = unitData->entryData[entryId];
                address = (wbIndex->entryOffs +
                           wbEntryIdx->entryOffs +
                           (wbEntryIdx->entryTotSize * entryId));
                wbEntry = (_soc_c3_rce_wb_entry_t*)(&(unitData->scPtr[address]));
                if (wbEntry->entryPosition > paSize) {
                    /* belongs in next filter set */
                    break;
                }
                /* link this entry in to the group */
                if (wbEntry->entryPrev < wbUnit->entryLimit) {
                    entryData->entryPrev = unitData->entryData[wbEntry->entryPrev];
                }
                if (wbEntry->entryNext < wbUnit->entryLimit) {
                    entryData->entryNext = unitData->entryData[wbEntry->entryNext];
                }
                /* get the backing store copy of pattern (API copy) */
                byteOffset = entryData->entryPosition - daSize;
                address = (wbIndex->fsetOffs +
                           wbFsetIdx->pattOffs +
                           wbFset->pattDataStart +
                           (wbGroup->instrBlocks * byteOffset));
                wbPattern = (uint8*)(&(unitData->scPtr[address]));
                sal_memcpy(entryData->pattData[0],
                           wbPattern,
                           wbGroup->instrBlocks);
                /* get the hardware copy of pattern (HW copy) */
                byteOffset = entryData->entryPosition - daSize;
                if (byteOffset < C3_RCE_ENTRIES_PER_FILTER_SET) {
                    rowOffset = byteOffset >> 5;
                    byteOffset &= 0x1F;
                    byteOffset = (byteOffset ^
                                  (C3_RCE_DMA_HOST_END_MASK ^
                                   C3_RCE_DMA_CHIP_END_MASK_IMEM));
                    for (index = 0; index < groupData->instrBlocks; index++) {
                        entryData->pattData[1][index] = progBlock[index].pattern[rowOffset][byteOffset];
                    }
                }
                /* adjust filter set range reference counts */
                for (index = 1;
                     index <= groupData->rangesPerFilterSet;
                     index++) {
                    byteOffset = index >> 3;
                    offset = 1 << (index & 7);
                    address = index - 1;
                    if (entryData->pattData[0][byteOffset] & offset) {
                        fsetData->rangeInfo[address].refCount++;
                    }
                    if (entryData->pattData[1][0] & 0x01) {
                        if (entryData->pattData[1][byteOffset] & offset) {
                            fsetData->rangeInfo[address].commitCount++;
                        }
                    }
                } /* for (all range slots in this filter set) */
                /* get the backing store copy of the action (API copy) */
                for (index = 0;
                     index < SOC_C3_RCE_RESULT_REGISTER_COUNT;
                     index++) {
                    if (0 == (wbGroup->resultLrpUniq & (1 << index))) {
                        /* not using this result register */
                        continue;
                    }
                    offset = entryData->entryPosition - daSize;
                    address = (wbIndex->actTblOffs +
                               wbActTblIdx->actTblOffs +
                               (wbActTblIdx->actTblTotSize * wbProg->actIndex[index]));
                    wbActTbl = (_soc_c3_rce_wb_actTbl_t*)(&(unitData->scPtr[address]));
                    address = (wbIndex->actTblOffs +
                               wbActTblIdx->actTblDataOffs +
                               wbActTbl->dataOffset +
                               (((wbFset->amemBlock *
                                  C3_RCE_ENTRIES_PER_FILTER_SET) +
                                 offset) * wbActTbl->actionBytes));
                    wbAction = (uint8*)(&(unitData->scPtr[address]));
                    sal_memcpy(entryData->actData[0][index],
                               wbAction,
                               wbActTbl->actionBytes);
                }
                /* get the hardware copy of the action (HW copy) */
                for (index = 0, address = 0;
                     index < SOC_C3_RCE_RESULT_REGISTER_COUNT;
                     index++) {
                    if (0 == (wbGroup->resultLrpUniq & (1 << index))) {
                        /* not using this result register */
                        continue;
                    }
                    actTblData = unitData->actData[progData->actIndex[index]]->ucData;
                    for (offset = 0;
                         offset < actTblData->ocmSegments;
                         offset++) {
                        if (!actData[index][offset]) {
                            /* segment is not used */
                            continue;
                        }
                        rowOffset = entryData->entryPosition - daSize;
                        if (rowOffset < C3_RCE_ENTRIES_PER_FILTER_SET) {
                            rowOffset *= actSize[index][offset];
                            for (byteOffset = 0;
                                 byteOffset < actSize[index][offset];
                                 byteOffset++, address++) {
                                temp = ((rowOffset + byteOffset) ^
                                        C3_RCE_DMA_HOST_END_MASK ^
                                        actTblData->seg[offset].byteOrderMask);
                                entryData->actData[1][index][address] = actData[index][offset][temp];
                            } /* for (all bytes in this action table entry) */
                        }
                    } /* for (all OCM segments in this action table) */
                } /* for (all action tables used by the entry's group) */
                /* look at the next entry in the group */
                entryId = wbEntry->entryNext;
                fsetData->entryCount++;
            } /* while (the current entry is valid) */
            /* update reference counts for ranges per this filter set */
            for (index = 0; index < groupData->rangesPerFilterSet; index++) {
                if (0 == fsetData->rangeInfo[index].rangeId) {
                    /* range slot is not used by this filter set */
                    continue;
                }
                rangeData = unitData->rangeData[fsetData->rangeInfo[index].rangeId - 1];
                if (fsetData->rangeInfo[index].refCount ||
                    fsetData->rangeInfo[index].commitCount) {
                    /* this filter set is using this range */
                    rangeData->refCount++;
                }
            } /* for (all range slots in this filter set) */
        } /* for (all filter sets in this program) */
    } /* for (all possible programs in backing store) */

error:
    if (SOC_E_NONE != result) {
        unitData->unitFlags |= SOC_C3_RCE_UNIT_RESTORE_FAIL;
    }
    if (actBuffer) {
        sal_free(actBuffer);
    }
    if (progBlock) {
        sal_dma_free(progBlock);
    }
    /* coverity[leaked_storage] */
    return result;
}

int
_soc_c3_rce_wb_full_sync(_soc_c3_rce_unit_desc_int_t *unitData)
{
    _soc_c3_rce_wb_main_index_t *wbIndex;       /* pointer to WB main index */
    _soc_c3_rce_wb_unit_t *wbUnit;              /* pointer to WB unit data */
    _soc_c3_rce_wb_padDesc_t *wbUnitPad;        /* pointer to WB pading data */
    _soc_c3_rce_wb_actTbl_index_t *wbActTblIdx; /* pointer to WB actTbl index */
    _soc_c3_rce_wb_actTbl_t *wbActTbl;          /* pointer to WB actTbl data */
    _soc_c3_rce_wb_range_index_t *wbRangeIdx;   /* pointer to WB range index */
    _soc_c3_rce_wb_range_t *wbRange;            /* pointer to WB range data */
    _soc_c3_rce_wb_program_index_t *wbProgIdx;  /* pointer to WB program idx */
    _soc_c3_rce_wb_program_t *wbProg;           /* pointer to WB program data */
    _soc_c3_rce_wb_fset_index_t *wbFsetIdx;     /* pointer to WB fset index */
    _soc_c3_rce_wb_fset_t *wbFset;              /* pointer to WB fset data */
    _soc_c3_rce_wb_fset_imem_t *wbFsetImem;     /* ptr to WB fset imem data */
    _soc_c3_rce_wb_group_index_t *wbGroupIdx;   /* pointer to WB group index */
    _soc_c3_rce_wb_group_t *wbGroup;            /* pointer to WB group data */
    _soc_c3_rce_wb_qual_t *wbQual;              /* pointer to WB qual data */
    _soc_c3_rce_wb_entry_index_t *wbEntryIdx;   /* pointer to WB entry index */
    _soc_c3_rce_wb_entry_t *wbEntry;            /* pointer to WB entry data */
    char *wbName;                               /* pointer to WB name data */
    uint8 *wbAction;                            /* pointer to WB action data */
    uint8 *wbPattern;                           /* pointer to WB pattern data */
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
    uint64 *wbBasisCounts;                      /* pointer to WB ent basis ct */
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
    _soc_c3_rce_range_desc_int_t *rangeData;    /* pointer to range data */
    _soc_c3_rce_program_desc_int_t *progData;   /* pointer to program data */
    _soc_c3_rce_filterset_desc_int_t *fsetData; /* pointer to fset data */
    _soc_c3_rce_group_desc_int_t *groupData;    /* pointer to group data */
    _soc_c3_rce_entry_desc_int_t *entryData;    /* pointer to entry data */
    int result = SOC_E_NONE;                    /* working result */
    unsigned int actTblId;                      /* working action table ID */
    unsigned int progId;                        /* working program ID */
    unsigned int fsetId;                        /* working filters set ID */
    unsigned int groupId;                       /* working group ID */
    unsigned int entryId;                       /* working entry ID */
    unsigned int rangeId;                       /* working range ID */
    unsigned int index;                         /* working index */
    unsigned int offset;                        /* working offset */
    unsigned int fset;                          /* working filter set index */
    unsigned int address;                       /* working address */
    unsigned int scSize;                        /* working WB rel address */
    unsigned int daSize;                        /* working data rel address */
    unsigned int paSize;                        /* working patt rel address */
    _soc_c3_rce_instruction_t instrTemp;        /* working instruction temp */

    if ((!unitData->scPtr) || (!unitData->scSize)) {
        /* no space for save; just return success */
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META("unit %d asked to save state to backing store but"
                           " there is no backing store space\n"),
                  unitData->unit));
        result = SOC_E_NONE;
        goto error;
    }

    /* make sure everything is clear */
    sal_memset(unitData->scPtr, 0x00, unitData->scSize);
    scSize = 0;

    /* set up main index */
    RCE_EVERB((RCE_MSG1("unit %d prepare warm boot index: %08X\n"),
               unitData->unit,
               scSize));
    wbIndex = (_soc_c3_rce_wb_main_index_t*)(unitData->scPtr);
    wbIndex->magic = _SOC_C3_RCE_WB_MAIN_INDEX_MAGIC;
    /* adjust for size of main index */
    scSize += sizeof(*wbIndex);

    /* set up unit information */
    RCE_EVERB((RCE_MSG1("unit %d prepare warm boot unit descriptor: %08X\n"),
               unitData->unit,
               scSize));
    wbIndex->unitOffs = scSize;
    wbUnit = (_soc_c3_rce_wb_unit_t*)(&(unitData->scPtr[wbIndex->unitOffs]));
    wbUnit->magic = _SOC_C3_RCE_WB_UNIT_MAGIC;
    wbUnit->unitFlags = unitData->unitFlags;
    wbUnit->rangeLimit = unitData->rangeLimit;
    wbUnit->groupLimit = unitData->groupLimit;
    wbUnit->filterSetLimit = unitData->groupLimit;
    wbUnit->entryLimit = unitData->entryLimit;
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
    wbUnit->entryBasisCounts = SOC_C3_RCE_NUM_BASIS_COUNTERS;
#else /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
    wbUnit->entryBasisCounts = 0;
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
    wbUnit->epochTime = unitData->epochTime;
    wbUnit->actionTableCount = unitData->actTableCount;
    wbUnit->lrpClockDivide = unitData->lrpClockDivide;
    wbUnit->rceClockDivide = unitData->rceClockDivide;
    for (progId = 0; progId < SOC_C3_RCE_PROGRAM_COUNT; progId++) {
        wbUnit->progOrder[progId] = unitData->progOrder[progId];
    }
    wbUnit->paddingCount = C3_RCE_FILTER_SET_COUNT_PADDING;
    wbUnit->paddingSize = sizeof(*wbUnitPad);
    wbUnit->paddingOffs = sizeof(*wbUnit);
    address = (wbIndex->unitOffs +
               wbUnit->paddingOffs);
    wbUnitPad = (_soc_c3_rce_wb_padDesc_t*)(&(unitData->scPtr[address]));
    wbUnitPad->padSize = C3_RCE_FILTER_SET_FIRST_PADDING;
    wbUnitPad->padStart = unitData->imemExtra[0];
    for (fsetId = 1; fsetId < wbUnit->paddingCount; fsetId++) {
        address += wbUnit->paddingSize;
        wbUnitPad = (_soc_c3_rce_wb_padDesc_t*)(&(unitData->scPtr[address]));
        wbUnitPad->padSize = C3_RCE_FILTER_SET_LENGTH_PADDING;
        wbUnitPad->padStart = unitData->imemExtra[fsetId];
    }
    /* adjust for size of unit information */
    scSize += sizeof(*wbUnit);
    scSize += sizeof(*wbUnitPad) * wbUnit->paddingCount;

    /* set up action table index */
    RCE_EVERB((RCE_MSG1("unit %d prepare warm boot action table index"
                        " (%u action tables): %08X\n"),
               unitData->unit,
               wbUnit->actionTableCount,
               scSize));
    wbIndex->actTblOffs = scSize;
    wbActTblIdx = (_soc_c3_rce_wb_actTbl_index_t*)(&(unitData->scPtr[wbIndex->actTblOffs]));
    wbActTblIdx->magic = _SOC_C3_RCE_WB_ACT_TBL_INDEX_MAGIC;
    wbActTblIdx->actTblOffs = sizeof(*wbActTblIdx);
    wbActTblIdx->actTblTotSize = sizeof(*wbActTbl);
    wbActTblIdx->actTblDataOffs = (wbActTblIdx->actTblOffs +
                                  (wbActTblIdx->actTblTotSize *
                                   wbUnit->actionTableCount));
    /* set up action table information */
    for (actTblId = 0, daSize = 0;
         actTblId < wbUnit->actionTableCount;
         actTblId++) {
        /* copy some info about this action table to backing store */
        RCE_EVERB((RCE_MSG1("unit %d prepare warm boot action table %u (%s)"
                            " descriptor\n"),
                   unitData->unit,
                   actTblId,
                   unitData->actData[actTblId]->ucData->tableName));
        address = (wbIndex->actTblOffs +
                   wbActTblIdx->actTblOffs +
                   (wbActTblIdx->actTblTotSize * actTblId));
        wbActTbl = (_soc_c3_rce_wb_actTbl_t*)(&(unitData->scPtr[address]));
        wbActTbl->entryLimit = unitData->actData[actTblId]->entryLimit;
        wbActTbl->firstEntryBias = unitData->actData[actTblId]->firstEntryBias;
        wbActTbl->actionBytes = unitData->actData[actTblId]->actionBytes;
        wbActTbl->dataOffset = daSize;
        /* adjust offset in data space for next action table */
        daSize += (wbActTbl->entryLimit *
                   wbActTbl->actionBytes *
                   C3_RCE_ENTRIES_PER_FILTER_SET);
    } /* for (index = 0; index < unitData->actTableCount; index++) */
    /* adjust for size of action table index and information and data */
    scSize += wbActTblIdx->actTblOffs;
    scSize += (wbActTblIdx->actTblTotSize * wbUnit->actionTableCount);
    scSize += daSize;

    /* set up range index */
    RCE_EVERB((RCE_MSG1("unit %d prepare warm boot range index"
                        " (%u range capacity): %08X\n"),
               unitData->unit,
               wbUnit->rangeLimit,
               scSize));
    wbIndex->rangeOffs = scSize;
    wbRangeIdx = (_soc_c3_rce_wb_range_index_t*)(&(unitData->scPtr[wbIndex->rangeOffs]));
    wbRangeIdx->magic = _SOC_C3_RCE_WB_RANGE_INDEX_MAGIC;
    wbRangeIdx->rangeOffs = sizeof(*wbRangeIdx);
    wbRangeIdx->rangeSize = sizeof(*wbRange);
    wbRangeIdx->rangeNameSize = C3_RCE_RANGE_NAME_BYTES;
    wbRangeIdx->rangeTotSize = (wbRangeIdx->rangeSize +
                                (wbRangeIdx->rangeNameSize * sizeof(char)));
    /* set up range infromation */
    for (rangeId = 0; rangeId < wbUnit->rangeLimit; rangeId++) {
        /* copy all existing ranges to backing store */
        if (unitData->rangeData[rangeId]) {
            /* copy range to backing store */
            rangeData = unitData->rangeData[rangeId];
            RCE_EVERB((RCE_MSG1("unit %d prepare warm boot range %u (%s)"
                                " descriptor\n"),
                       unitData->unit,
                       rangeId + 1,
                       rangeData->headerField.fieldName));
            address = (wbIndex->rangeOffs +
                       wbRangeIdx->rangeOffs +
                       (wbRangeIdx->rangeTotSize * rangeId));
            wbRange = (_soc_c3_rce_wb_range_t*)(&(unitData->scPtr[address]));
            wbName = (char*)&(unitData->scPtr[address + wbRangeIdx->rangeSize]);
            wbRange->rangeFlags = rangeData->rangeFlags;
            wbRange->validProgs = rangeData->validProgs;
            wbRange->header = rangeData->headerField.header;
            wbRange->startBit = rangeData->headerField.startBit;
            wbRange->numBits = rangeData->headerField.numBits;
            wbRange->lowerBound = rangeData->lowerBound;
            wbRange->upperBound = rangeData->upperBound;
            sal_strncpy(wbName,
                        rangeData->headerField.fieldName,
                        wbRangeIdx->rangeNameSize - 1);
        } /* if (unitData->rangeData[index]) */
    } /* for (index = 0; index < unitData->rangeLimit; index++) */
    /* adjust for size of range index and information */
    scSize += wbRangeIdx->rangeOffs;
    scSize += wbRangeIdx->rangeTotSize * wbUnit->rangeLimit;

    /* set up program index */
    RCE_EVERB((RCE_MSG1("unit %d prepare warm boot program index: %08X\n"),
               unitData->unit,
               scSize));
    wbIndex->progOffs = scSize;
    wbProgIdx = (_soc_c3_rce_wb_program_index_t*)(&(unitData->scPtr[wbIndex->progOffs]));
    wbProgIdx->magic = _SOC_C3_RCE_WB_PROG_INDEX_MAGIC;
    wbProgIdx->progOffs = sizeof(*wbProgIdx);
    wbProgIdx->progTotSize = sizeof(*wbProg);
    /* set up program information */
    for (progId = 0; progId < SOC_C3_RCE_PROGRAM_COUNT; progId++) {
        if (unitData->progData[progId]) {
            progData = unitData->progData[progId];
            RCE_EVERB((RCE_MSG1("unit %d prepare warm boot program %u (%s)"
                                " descriptor\n"),
                       unitData->unit,
                       progId,
                       progData->ucData->programName));
            /* mark program as existing in unit data */
            wbUnit->progValidFlags |= (1 << progId);
            /* copy this program's info to its backing store space */
            address = (wbIndex->progOffs +
                       wbProgIdx->progOffs +
                       (progId * wbProgIdx->progTotSize));
            wbProg = (_soc_c3_rce_wb_program_t*)(&(unitData->scPtr[address]));
            wbProg->keyTime = progData->keyTime;
            wbProg->switchTime = progData->switchTime;
            if (progData->groupHead) {
                wbProg->groupHead = progData->groupHead->groupId;
            } else {
                wbProg->groupHead = wbUnit->groupLimit;
            }
            if (progData->groupTail) {
                wbProg->groupTail = progData->groupTail->groupId;
            } else {
                wbProg->groupTail = wbUnit->groupLimit;
            }
            for (offset = 0;
                 offset < SOC_C3_RCE_RESULT_REGISTER_COUNT;
                 offset++) {
                wbProg->actIndex[offset] = progData->actIndex[offset];
            }
        } /* if (this program exists) */
    } /* for (all possible programs) */
    /* adjust for size of program index and information */
    scSize += wbProgIdx->progOffs;
    scSize += (wbProgIdx->progTotSize * SOC_C3_RCE_PROGRAM_COUNT);

    /* set up fset index */
    RCE_EVERB((RCE_MSG1("unit %d prepare warm boot filter set index"
                        " (%u filter set capacity): %08X\n"),
               unitData->unit,
               wbUnit->filterSetLimit,
               scSize));
    wbIndex->fsetOffs = scSize;
    wbFsetIdx = (_soc_c3_rce_wb_fset_index_t*)(&(unitData->scPtr[wbIndex->fsetOffs]));
    wbFsetIdx->magic = _SOC_C3_RCE_WB_FSET_INDEX_MAGIC;
    wbFsetIdx->fsetOffs = sizeof(*wbFsetIdx);
    wbFsetIdx->fsetTotSize = sizeof(*wbFset);
    wbFsetIdx->imemTotSize = sizeof(*wbFsetImem);
    wbFsetIdx->imemOffs = (wbFsetIdx->fsetOffs +
                           (wbFsetIdx->fsetTotSize *
                            wbUnit->groupLimit));
    wbFsetIdx->pattOffs = (wbFsetIdx->imemOffs +
                           (sizeof(_soc_c3_rce_wb_fset_imem_t) *
                            C3_RCE_IMEM_SIZE *
                            C3_RCE_IMEM_INSTR_PER_BLOCK));
    /* set up fset information */
    COMPILER_64_SET(instrTemp, _SOC_C3_RCE_RANGE_FLAG_IN_TEMPLATE, 0);
    for (progId = 0, fset = 0, daSize = 0, paSize = 0;
         progId < SOC_C3_RCE_PROGRAM_COUNT;
         progId++) {
        if (!(unitData->progData[progId])) {
            /* this program does not exist; skip it */
            continue;
        }
        progData = unitData->progData[progId];
        address = (wbIndex->progOffs +
                   wbProgIdx->progOffs +
                   (progId * wbProgIdx->progTotSize));
        wbProg = (_soc_c3_rce_wb_program_t*)(&(unitData->scPtr[address]));
        wbProg->filterSetCount = progData->filterSetCount;
        wbProg->firstFilterSet = fset;
        wbProg->instrBlockAdded = progData->instrBlockAdded;
        wbProg->instrBlockCount = progData->instrBlockCount;
        RCE_EVERB((RCE_MSG1("unit %d prepare warm boot filter sets for"
                            " program %u (%u filter sets)\n"),
                   unitData->unit,
                   progId,
                   wbProg->filterSetCount));
        for (fsetId = 0;
             fsetId < progData->filterSetCount;
             fsetId++, fset++) {
            /* copy this filter set's data to backing store */
            fsetData = &(progData->fsetData[fsetId]);
            RCE_EVERB((RCE_MSG1("unit %d prepare warm boot program %u filter"
                                " set %u descriptor\n"),
                       unitData->unit,
                       progId,
                       fsetId));
            address = (wbIndex->fsetOffs +
                       wbFsetIdx->fsetOffs +
                       (fset * wbFsetIdx->fsetTotSize));
            wbFset = (_soc_c3_rce_wb_fset_t*)(&(unitData->scPtr[address]));
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
            wbFset->fsetFlags = (fsetData->fsetFlags |
                                 _SOC_C3_RCE_FSET_FLAGS_EXISTS);
            if (0 == (fsetData->fsetFlags & _SOC_C3_RCE_FSET_FLAGS_EXISTS)) {
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META("unit %d program %u filter set %u does"
                                   " not have 'exists' flag set\n"),
                          unitData->unit,
                          progId,
                          fsetId));
            }
#else /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
            wbFset->fsetFlags = _SOC_C3_RCE_FSET_FLAGS_EXISTS;
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
            wbFset->groupId = fsetData->groupId;
            wbFset->filterIndex = fsetData->filterIndex;
            wbFset->amemBlock = fsetData->amemBlock;
            wbFset->imemStart = fsetData->imemStart;
            wbFset->imemSize = fsetData->imemSize;
            wbFset->imemDataStart = daSize;
            wbFset->pattDataStart = paSize;
            address = (wbIndex->fsetOffs +
                       wbFsetIdx->imemOffs +
                       wbFset->imemDataStart);
            wbFsetImem = (_soc_c3_rce_wb_fset_imem_t*)(&(unitData->scPtr[address]));
            groupData = unitData->groupData[wbFset->groupId];
            /* copy the range/instruction information to backing store */
            for (index = 0; index < groupData->instrCount; index++) {
                if (0 == index) {
                    /* start instruction data are in group spec */
                    COMPILER_64_ZERO(wbFsetImem[index].instr);
                } else if (index <= groupData->rangesPerFilterSet) {
                    /* ranges can vary by filter set, so copy those */
                    wbFsetImem[index].rangeData.rangeFlags = fsetData->rangeInfo[index - 1].rangeFlags;
                    wbFsetImem[index].rangeData.rangeId = fsetData->rangeInfo[index - 1].rangeId;
                } else {
                    /* copy instruction and set as part of template */
                    /* default pattern bit is already in these instructions */
                    wbFsetImem[index].instr = groupData->instr[index];
                    /* but we also want to mark that it is part of template */
                    COMPILER_64_OR(wbFsetImem[index].instr, instrTemp);
                }
            } /* for (all instructions in this filter set) */
            /* adjust position in imem instruction table */
            daSize += (wbFsetIdx->imemTotSize *
                       wbFset->imemSize *
                       C3_RCE_IMEM_INSTR_PER_BLOCK);
            /* adjust position in pattern table */
            paSize += (wbFset->imemSize *
                       C3_RCE_ENTRIES_PER_FILTER_SET);
        } /* for (all filter sets in this program) */
    } /* for (all possible programs) */
    /* adjust for size of fset index, information, imem space, pattern space */
    scSize += wbFsetIdx->fsetOffs;
    scSize += (wbFsetIdx->fsetTotSize * wbUnit->groupLimit);
    scSize += (wbFsetIdx->imemTotSize *
               C3_RCE_IMEM_SIZE *
               C3_RCE_IMEM_INSTR_PER_BLOCK);
    scSize += (C3_RCE_IMEM_SIZE *
               C3_RCE_ENTRIES_PER_FILTER_SET);

    /* set up group index */
    RCE_EVERB((RCE_MSG1("unit %d prepare warm boot group index"
                        " (%u group capacity): %08X\n"),
               unitData->unit,
               wbUnit->groupLimit,
               scSize));
    wbIndex->groupOffs = scSize;
    wbGroupIdx = (_soc_c3_rce_wb_group_index_t*)(&(unitData->scPtr[wbIndex->groupOffs]));
    wbGroupIdx->magic = _SOC_C3_RCE_WB_GROUP_INDEX_MAGIC;
    wbGroupIdx->groupOffs = sizeof(*wbGroupIdx);
    wbGroupIdx->groupSize = sizeof(*wbGroup);
    wbGroupIdx->groupQualSize = sizeof(*wbQual);
    wbGroupIdx->groupQualMax = SOC_C3_RCE_GROUP_QUAL_MAX;
    wbGroupIdx->groupQualNameMax = C3_RCE_QUAL_NAME_BYTES;
    wbGroupIdx->groupTotSize = (sizeof(*wbGroup) +
                                ((wbGroupIdx->groupQualSize +
                                  wbGroupIdx->groupQualNameMax) *
                                 wbGroupIdx->groupQualMax));
    /* set up group information */
    for (groupId = 0; groupId < wbUnit->groupLimit; groupId++) {
        if (unitData->groupData[groupId]) {
            /* copy group's data to backing store */
            groupData = unitData->groupData[groupId];
            RCE_EVERB((RCE_MSG1("unit %d prepare warm boot group %u"
                                " descriptor\n"),
                       unitData->unit,
                       groupId));
            address = (wbIndex->groupOffs +
                       wbGroupIdx->groupOffs +
                       (groupId * wbGroupIdx->groupTotSize));
            wbGroup = (_soc_c3_rce_wb_group_t*)(&(unitData->scPtr[address]));
            wbGroup->groupPriority = groupData->groupPriority;
            wbGroup->instrBlocks = groupData->instrBlocks;
            wbGroup->filterSetCount = groupData->filterSetCount;
            wbGroup->maxFilterSets = groupData->maxFilterSets;
            wbGroup->qualCount = groupData->qualCount;
            wbGroup->rangesPerFilterSet = groupData->rangesPerFilterSet;
            wbGroup->rceProgram = groupData->rceProgram;
            wbGroup->resultLrpUniq = groupData->resultLrpUniq;
            wbGroup->resultLrp = groupData->resultLrp;
            wbGroup->resultRce = groupData->resultRce;
#if (C3_RCE_ENABLE_ACTION_INDEX_COUNTERS || C3_RCE_ENABLE_ENTRY_INSERT_HEURISTIC)
            wbGroup->groupFlags = (groupData->groupFlags |
                                   _SOC_C3_RCE_GROUP_FLAG_EXISTS);
#else /* (C3_RCE_ENABLE_ACTION_INDEX_COUNTERS || C3_RCE_ENABLE_ENTRY_INSERT_HEURISTIC) */
            wbGroup->groupFlags = _SOC_C3_RCE_GROUP_FLAG_EXISTS;
#endif /* (C3_RCE_ENABLE_ACTION_INDEX_COUNTERS || C3_RCE_ENABLE_ENTRY_INSERT_HEURISTIC) */
            wbGroup->dataSize = groupData->dataSize;
            wbGroup->entryLastAdded = groupData->entryLastAdded;
            if (groupData->groupPrev) {
                wbGroup->groupPrev = groupData->groupPrev->groupId;
            } else {
                wbGroup->groupPrev = wbUnit->groupLimit;
            }
            if (groupData->groupNext) {
                wbGroup->groupNext = groupData->groupNext->groupId;
            } else {
                wbGroup->groupNext = wbUnit->groupLimit;
            }
            if (groupData->entryHead) {
                wbGroup->entryHead = groupData->entryHead->entryId;
            } else {
                wbGroup->entryHead = wbUnit->entryLimit;
            }
            if (groupData->entryTail) {
                wbGroup->entryTail = groupData->entryTail->entryId;
            } else {
                wbGroup->entryTail = wbUnit->entryLimit;
            }
            address = (wbIndex->groupOffs +
                       wbGroupIdx->groupOffs +
                       (groupId * wbGroupIdx->groupTotSize) +
                       wbGroupIdx->groupSize);
            /* copy group's qualifier information to backing store */
            for (offset = 0;
                  offset < wbGroup->qualCount;
                  offset++, address += (wbGroupIdx->groupQualSize +
                                        wbGroupIdx->groupQualNameMax)) {
                wbQual = (_soc_c3_rce_wb_qual_t*)(&(unitData->scPtr[address]));
                wbName = (char*)(&(unitData->scPtr[(address +
                                                    wbGroupIdx->groupQualSize)]));

                wbQual->qualType = groupData->qualData[offset]->qualType;
                wbQual->paramCount = groupData->qualData[offset]->paramCount;
                wbQual->qualOffs = groupData->qualOffs[offset];
                /* simple qualifiers only have two parameters; keep those */
                switch (wbQual->paramCount) {
                case 2:
                    wbQual->param[1] = groupData->qualData[offset]->param[1];
                    /* fallthrough intentional */
                case 1:
                    wbQual->param[0] = groupData->qualData[offset]->param[0];
                    break;
                default:
                    /* nothing to do here for other cases */
                    break;
                }
                sal_strncpy(wbName,
                            groupData->qualData[offset]->qualName,
                            wbGroupIdx->groupQualNameMax - 1);
            } /* for (all qualifiers in this group) */
        } /* if (this group exists) */
    } /* for (all possible groups in this unit) */
    /* assocaite groups with their filter sets */
    RCE_EVERB((RCE_MSG1("unit %d warm boot associate filter sets to groups\n"),
               unitData->unit));
    for (progId = 0; progId < SOC_C3_RCE_PROGRAM_COUNT; progId++) {
        if (wbUnit->progValidFlags & (1 << progId)) {
            address = (wbIndex->progOffs +
                       wbProgIdx->progOffs +
                       (progId * wbProgIdx->progTotSize));
            wbProg = (_soc_c3_rce_wb_program_t*)(&(unitData->scPtr[address]));
            for (groupId = wbProg->groupHead, fset = 0;
                 groupId < wbUnit->groupLimit;
                 groupId = wbGroup->groupNext) {
                /* set group first fset ref to proper point in fset table */
                address = (wbIndex->groupOffs +
                           wbGroupIdx->groupOffs +
                           (groupId * wbGroupIdx->groupTotSize));
                wbGroup = (_soc_c3_rce_wb_group_t*)(&(unitData->scPtr[address]));
                wbGroup->firstFilterSet = fset;
                fset += wbGroup->filterSetCount;
            } /* for (all groups in this program) */
        } /* if (this program is valid) */
    } /* for (all possible programs in this unit) */
    /* adjust for size of group index and group data */
    scSize += wbGroupIdx->groupOffs;
    scSize += (wbGroupIdx->groupTotSize * wbUnit->groupLimit);

    /* set up entry index */
    RCE_EVERB((RCE_MSG1("unit %d prepare warm boot entry index"
                        " (%u entry capacity): %08X\n"),
               unitData->unit,
               wbUnit->entryLimit,
               scSize));
    wbIndex->entryOffs = scSize;
    wbEntryIdx = (_soc_c3_rce_wb_entry_index_t*)(&(unitData->scPtr[wbIndex->entryOffs]));
    wbEntryIdx->magic = _SOC_C3_RCE_WB_ENTRY_INDEX_MAGIC;
    wbEntryIdx->entryOffs = sizeof(*wbEntryIdx);
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
    wbEntryIdx->entryTotSize = (sizeof(*wbEntry) +
                                (sizeof(*wbBasisCounts) *
                                 wbUnit->entryBasisCounts));
#else /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
    wbEntryIdx->entryTotSize = sizeof(*wbEntry);
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
    wbEntryIdx->entrySize = sizeof(*wbEntry);
    /* set up entry information */
    for (entryId = 0; entryId < wbUnit->entryLimit; entryId++) {
        if (unitData->entryData[entryId]) {
            /* copy this entry's data to backing store */
            entryData = unitData->entryData[entryId];
            RCE_EVERB((RCE_MSG1("unit %d prepare warm boot entry %u"
                                " descriptor\n"),
                       unitData->unit,
                       entryId));
            address = (wbIndex->entryOffs +
                       wbEntryIdx->entryOffs +
                       (entryId * wbEntryIdx->entryTotSize));
            wbEntry = (_soc_c3_rce_wb_entry_t*)(&(unitData->scPtr[address]));
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
            wbBasisCounts = (uint64*)(&(unitData->scPtr[address + wbEntryIdx->entrySize]));
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
            wbEntry->entryFlags = entryData->entryFlags;
            wbEntry->entryGroup = entryData->entryGroup;
            wbEntry->entryPosition = entryData->entryPosition;
            wbEntry->entryPriority = entryData->entryPriority;
            if (entryData->entryPrev) {
                wbEntry->entryPrev = entryData->entryPrev->entryId;
            } else {
                wbEntry->entryPrev = wbUnit->entryLimit;
            }
            if (entryData->entryNext) {
                wbEntry->entryNext = entryData->entryNext->entryId;
            } else {
                wbEntry->entryNext = wbUnit->entryLimit;
            }
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
            wbEntry->entryPosPrev = entryData->entryPosPrev;
            for (index = 0;
                 index < wbUnit->entryBasisCounts;
                 index++) {
                wbBasisCounts[index] = entryData->basisCounts[index];
            }
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
            /* get group and program info for this entry */
            address = (wbIndex->groupOffs +
                       wbGroupIdx->groupOffs +
                       (wbEntry->entryGroup * wbGroupIdx->groupTotSize));
            wbGroup = (_soc_c3_rce_wb_group_t*)(&(unitData->scPtr[address]));
            address = (wbIndex->progOffs +
                       wbProgIdx->progOffs +
                       (wbGroup->rceProgram * wbProgIdx->progTotSize));
            wbProg = (_soc_c3_rce_wb_program_t*)(&(unitData->scPtr[address]));
            /* find this entry's filter set and its info */
            fset = ((wbEntry->entryPosition /
                     C3_RCE_ENTRIES_PER_FILTER_SET) +
                    wbGroup->firstFilterSet +
                    wbProg->firstFilterSet);
            address = (wbIndex->fsetOffs +
                       wbFsetIdx->fsetOffs +
                       (fset * wbFsetIdx->fsetTotSize));
            wbFset = (_soc_c3_rce_wb_fset_t*)(&(unitData->scPtr[address]));
            offset = (wbEntry->entryPosition -
                      (wbFset->filterIndex * C3_RCE_ENTRIES_PER_FILTER_SET));
            /* figure out which action table the entry uses */
            for (index = 0;
                 index < SOC_C3_RCE_RESULT_REGISTER_COUNT;
                 index++) {
                if ((1 << index) & wbGroup->resultLrpUniq) {
                    /* found the result register used by this group */
                    break;
                }
            }
            /* coverity[overrun-local] */
            actTblId = wbProg->actIndex[index];
            /* get the appropriate action table's information */
            address = (wbIndex->actTblOffs +
                       wbActTblIdx->actTblOffs +
                       (actTblId * wbActTblIdx->actTblTotSize));
            wbActTbl = (_soc_c3_rce_wb_actTbl_t*)(&(unitData->scPtr[address]));
            /* copy this entry's 'API-side' action data to backing store */
            address = (wbIndex->actTblOffs +
                       wbActTblIdx->actTblDataOffs +
                       wbActTbl->dataOffset +
                       (((wbFset->amemBlock * C3_RCE_ENTRIES_PER_FILTER_SET) +
                         offset) * wbActTbl->actionBytes));
            groupData = unitData->groupData[entryData->entryGroup];
            wbAction = (uint8*)(&(unitData->scPtr[address]));
            sal_memcpy(wbAction,
                       entryData->actData[0][index],
                       wbActTbl->actionBytes);
            /* copy this entry's 'API-side' pattern data to backing store */
            address = (wbIndex->fsetOffs +
                       wbFsetIdx->pattOffs +
                       wbFset->pattDataStart +
                       (offset * wbGroup->instrBlocks));
            wbPattern = (uint8*)(&(unitData->scPtr[address]));
            sal_memcpy(wbPattern,
                       entryData->pattData[0],
                       wbGroup->instrBlocks);
        } /* if (this entry is valid) */
    } /* for (all possible entries) */
    /* adjust for size of entry index and entry data */
    scSize += wbEntryIdx->entryOffs;
    scSize += (wbEntryIdx->entryTotSize * wbUnit->entryLimit);

    RCE_EVERB((RCE_MSG1("unit %d prepared backing store size %08X\n"),
               unitData->unit,
               scSize));

    /* more sanity checking... */
    if (scSize > unitData->scSize) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d backing store size %u greater than"
                            " allocated %u; probable corruption!\n"),
                   unitData->unit,
                   scSize,
                   unitData->scSize));
        result = SOC_E_INTERNAL;
        goto error;
    } else if (scSize < unitData->scSize) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META("unit %d backing store size %u less than"
                           " allocated %u; probably okay but not optimal\n"),
                  unitData->unit,
                  scSize,
                  unitData->scSize));
        /* but it's not (really) an error */
    }

error:
    return result;
}

int
_soc_c3_rce_wb_init(_soc_c3_rce_unit_desc_int_t *unitData)
{
    int result = SOC_E_NONE;                /* working result */
    int wbFlags;                            /* backing store access flags */
    unsigned int wbSize;                    /* expected backing store size */
    soc_scache_handle_t scHandle;           /* handle for scache APIs */
    uint32 scSize;                          /* size for scache APIs */
    uint8 *scPtr = NULL;                    /* pointer for scache APIs */
    uint16 scVer;                           /* version for scache APIs */
    int scExists;                           /* exists flag for scache APIs */

    /* set up backing store access data */
    wbFlags = SOC_CALADAN3_SCACHE_DEFAULT;
    SOC_SCACHE_HANDLE_SET(scHandle,
                          unitData->unit,
                          SOC_SBX_WB_MODULE_RCE,
                          0);
    /* calculate expected size of backing store */
    result = _soc_c3_rce_wb_size_calc(unitData, &wbSize);
    if (SOC_E_NONE != result) {
        /* called function displayed diagnostic */
        goto error;
    }
    scSize = wbSize;
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META("unit %d backing store needed size %u bytes\n"),
                 unitData->unit,
                 wbSize));
    if (SOC_WARM_BOOT(unitData->unit)) {
        /* warm boot mode; try to recover from existing state */
        result = soc_caladan3_scache_ptr_get(unitData->unit,
                                             scHandle,
                                             socScacheRetrieve,
                                             wbFlags,
                                             &scSize,
                                             &scPtr,
                                             _SOC_C3_RCE_WB_VERSION_CURR,
                                             &scVer,
                                             &scExists);
        if (SOC_E_NONE != result) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("unit %d unable to retrieve scache segment for"
                                " warm boot backing store: %d (%s)\n"),
                       unitData->unit,
                       result,
                       _SHR_ERRMSG(result)));
            goto error;
        } else {
            RCE_EVERB((RCE_MSG1("unit %d retrieved scache segment of %u"
                                " bytes (needed %u bytes)\n"),
                       unitData->unit,
                       scSize,
                       wbSize));
        }
        unitData->scHandle = scHandle;
        unitData->scSize = scSize;
        unitData->scPtr = scPtr;
        result = _soc_c3_rce_wb_load(unitData);
#if SOC_C3_RCE_WB_INHIBIT_WRITE_ON_RESTORE_FAIL
        if (unitData->unitFlags & SOC_C3_RCE_UNIT_RESTORE_FAIL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("unit %d failed to recover from backing store;"
                                " writes to backing store will be disabled"
                                " until either reinit without reload or reinit"
                                " with reload succeeds\n"),
                       unitData->unit));
            goto error;
        }
#endif /* SOC_C3_RCE_WB_INHIBIT_WRITE_ON_RESTORE_FAIL */
        if (SOC_E_NONE != result) {
            /* called function displayed diagnostic */
            goto error;
        }
        if (_SOC_C3_RCE_WB_VERSION_CURR != scVer) {
            /* old version of warm boot; force immediate resync to new ver */
            result = soc_caladan3_scache_ptr_get(unitData->unit,
                                                 scHandle,
                                                 socScacheRealloc,
                                                 wbFlags,
                                                 &scSize,
                                                 &scPtr,
                                                 _SOC_C3_RCE_WB_VERSION_CURR,
                                                 &scVer,
                                                 &scExists);
            if (SOC_E_NONE != result) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d unable to allocate %u bytes"
                                    " scache as warm boot backing store:"
                                    " %d (%s)\n"),
                         unitData->unit,
                         wbSize,
                         result,
                         _SHR_ERRMSG(result)));
                goto error;
            } else {
                RCE_EVERB((RCE_MSG1("unit %d allocated scache segment of %u"
                                    " bytes (needed %u bytes)\n"),
                           unitData->unit,
                           scSize,
                           wbSize));
            }
            if (scSize < wbSize) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("unit %d scache space got %u bytes but"
                                    " need at least %u bytes\n"),
                           unitData->unit,
                           scSize,
                           wbSize));
                result = SOC_E_MEMORY;
                goto error;
            }
            unitData->scHandle = scHandle;
            unitData->scSize = scSize;
            unitData->scPtr = scPtr;
            result = _soc_c3_rce_wb_full_sync(unitData);
            if (SOC_E_NONE != result) {
                /* called function displayed diagnostic */
                goto error;
            }
        } /* if (_SOC_C3_RCE_WB_VERSION_CURR != scVer) */
    } else { /* if (SOC_WARM_BOOT(unitData->unit)) */
        /* cold boot mode; create/init backing store state */
        result = soc_caladan3_scache_ptr_get(unitData->unit,
                                             scHandle,
                                             socScacheCreate,
                                             wbFlags,
                                             &scSize,
                                             &scPtr,
                                             _SOC_C3_RCE_WB_VERSION_CURR,
                                             &scVer,
                                             &scExists);
        if (SOC_E_NONE != result) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("unit %d unable to allocate %u bytes scache"
                                " as warm boot backing store: %d (%s)\n"),
                       unitData->unit,
                       wbSize,
                       result,
                       _SHR_ERRMSG(result)));
            goto error;
        } else {
            RCE_EVERB((RCE_MSG1("unit %d allocated scache segment of %u"
                                " bytes (needed %u bytes)\n"),
                       unitData->unit,
                       scSize,
                       wbSize));
        }
        if (scSize < wbSize) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("unit %d scache space got %u bytes but need"
                                " at least %u bytes\n"),
                       unitData->unit,
                       scSize,
                       wbSize));
            result = SOC_E_MEMORY;
            goto error;
        }
        unitData->scHandle = scHandle;
        unitData->scSize = scSize;
        unitData->scPtr = scPtr;
        result = _soc_c3_rce_wb_full_sync(unitData);
        if (SOC_E_NONE != result) {
            /* called function displayed diagnostic */
            goto error;
        }
    } /* if (SOC_WARM_BOOT(unitData->unit)) */
error:
    if (SOC_E_NONE != result) {
        /* something went wrong; can't use backing store */
        unitData->scPtr = NULL;
        unitData->scSize = 0;
    }
    
    return result;
}

int
_soc_c3_rce_wb_immed_sync(_soc_c3_rce_unit_desc_int_t *unitData)
{
    soc_scache_handle_t scHandle;           /* handle for scache APIs */
    unsigned int wbSize;                    /* expected backing store size */
    int result;                             /* working result */

    SOC_SCACHE_HANDLE_SET(scHandle,
                          unitData->unit,
                          SOC_SBX_WB_MODULE_RCE,
                          0);
    result = _soc_c3_rce_wb_size_calc(unitData, &wbSize);
    if (SOC_E_NONE != result) {
        /* called function displayed diagnostic */
        goto error;
    }
    if ((!unitData->scPtr) || (!unitData->scSize)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d has no backing store buffer\n"),
                   unitData->unit));
        result = SOC_E_RESOURCE;
        goto error;
    }
    if (wbSize > unitData->scSize) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d buffer %u too small (need %u)\n"),
                   unitData->unit,
                   unitData->scSize,
                   wbSize));
        result = SOC_E_MEMORY;
        goto error;
    }
    if (scHandle != unitData->scHandle) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("unit %d handle %08X but expected %08X\n"),
                   unitData->unit,
                   unitData->scHandle,
                   scHandle));
        result = SOC_E_CONFIG;
        goto error;
    }
    result = _soc_c3_rce_wb_full_sync(unitData);

error:
    return result;
}

#endif /* def BCM_WARM_BOOT_SUPPORT */
#endif /* def BCM_CALADAN3_SUPPORT */

