/*
 * $Id: field.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: Field Processor APIs
 *
 * Purpose:
 *     'Field Processor' (FP) API for BCM88030 (SBX Caladan3 + Guadalupe-2000)
 */
#ifndef _SBX_CALADAN3_FIELD_H_
#define _SBX_CALADAN3_FIELD_H_

#include <shared/bsl.h>

#include <soc/sbx/caladan3/rce.h>

/*
 *  WARNING: There should be no reason to directly include this file by any
 *  other than the CALADAN3 field modules.
 */
#ifndef _SBX_CALADAN3_FIELD_H_NEEDED_
#warning "this field.h should not generally be included carelessly"
#endif /* ndef _SBX_CALADAN3_FIELD_H_NEEDED_ */

/******************************************************************************
 *
 *  Configuration
 */

/*
 *  _SBX_CALADAN3_FIELD_INDEX_COMPACT SHOULD be TRUE if you want to reduce the
 *  size of the internal field API related index values to 16 bits. This will
 *  cause a significant reduction in the overall table sizes, but will limit
 *  the total group, entry, and range counts to 65535 (0..65534, with 65535
 *  reserved to indicate end of list).  It MUST be FALSE if you need any of
 *  these index values to be larger.
 *
 *  _SBX_CALADAN3_FIELD_EXCESS_VERBOSITY, when TRUE, turns on what amounts to a
 *  play-by-play account of what the field APIs are doing, emitted through the
 *  debugging output interface at the VVERBOSE level.  When FALSE, rather less
 *  comes out at VVERBOSE; other message levels are unaffected.  Some of these
 *  message can be sufficiently prolific to be obnoxious or impair performance.
 *
 *  _SBX_CALADAN3_FIELD_DIAGNOSTIC_MODE, when TRUE, displays 'unimportant' elements
 *  when dumping field system and individual rule data.  Normally (FALSE) these
 *  elements would be suppressed (such as reserved bits).  Support for this
 *  feature is a little uneven; don't count on all such fields being there.
 *
 *  _SBX_CALADAN3_FIELD_DUPLICATE_ACTION_ERR specifies the result error for when
 *  trying to add an action to a rule that already has the type of action being
 *  added. The BCM API doc claims this should be BCM_E_EXISTS, but the
 *  regression tests demand that it be BCM_E_CONFIG.
 *
 *  _SBX_CALADAN3_FIELD_OVERLAP_ACTION_ERR specifies the result error for when
 *  trying to add an action to a rule that already has an action that overlaps
 *  the action being added (such as doing different things with a union).  I
 *  don't think this ever happens in the XGS world, so I guessed BCM_E_CONFIG.
 *
 *  _SBX_CALADAN3_FIELD_INVALID_ID_ERR specifies the result error for when trying
 *  to access a resource using a clearly invalid (out of range) ID.  The BCM
 *  API seems to indicate this should be BCM_E_PARAM, but the regression tests
 *  demand that it be BCM_E_NOT_FOUND.
 *
 *  _SBX_CALADAN3_FIELD_INVALID_QSET specifies the result error for when trying to
 *  build groups that include unsupported features in their QSets.  The BCM API
 *  doc seems to imply that it should be BCM_E_PARAM, but the regression tests
 *  don't accept anything other than BCM_E_UNAVAIL in this case.
 *
 *  _SBX_CALADAN3_FIELD_INVALID_ACTION specifies the result error for when trying
 *  to manipulate actions on entries for which they are unsupported.  The BCM
 *  API doc seems to imply that it should be BCM_E_PARAM, but the regression
 *  tests don't accept anything other than BCM_E_UNAVAIL in this case.
 *
 *  _SBX_CALADAN3_FIELD_UNKNONWN_MICROCODE specifies the result error for any
 *  action attempted against a unit whose microcode is unknown, either because
 *  there is no support for it in the source or because support for it has been
 *  disabled at compile time.
 *
 *  _SBX_CALADAN3_FIELD_NOT_ON_MICROCODE_ERR specifies the result error for any
 *  action attempted against a unit whose microcode is supported but for whose
 *  microcode the specific action is not supported.
 *
 *  _SBX_CALADAN3_FIELD_PAGE_WIDTH specifies how wide the console should be for
 *  certain debugging functions that tend to generate lists (so they can be
 *  formatted to make better use of the available console space).  Don't make
 *  it more than 16384; use zero if you want no line joining (each element will
 *  be on its own line).
 */
#define _SBX_CALADAN3_FIELD_INDEX_COMPACT           TRUE
#define _SBX_CALADAN3_FIELD_EXCESS_VERBOSITY        TRUE
#define _SBX_CALADAN3_FIELD_DIAGNOSTIC_MODE         TRUE
#define _SBX_CALADAN3_FIELD_DUPLICATE_ACTION_ERR    BCM_E_CONFIG
#define _SBX_CALADAN3_FIELD_OVERLAP_ACTION_ERR      BCM_E_CONFIG
#define _SBX_CALADAN3_FIELD_INVALID_ID_ERR          BCM_E_NOT_FOUND
#define _SBX_CALADAN3_FIELD_INVALID_QSET_ERR        BCM_E_UNAVAIL
#define _SBX_CALADAN3_FIELD_INVALID_ACTION_ERR      BCM_E_UNAVAIL
#define _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR   BCM_E_UNIT
#define _SBX_CALADAN3_FIELD_NOT_ON_MICROCODE_ERR    BCM_E_UNAVAIL
#define _SBX_CALADAN3_FIELD_PAGE_WIDTH              79


/******************************************************************************
 *
 *  Module-wide definitions
 */

/*
 *  Debugging output formatting and control...
 */
#if _SBX_CALADAN3_FIELD_EXCESS_VERBOSITY
#define FIELD_EVERB(stuff)       LOG_DEBUG(BSL_LS_BCM_FP, stuff)
#else /* FIELD_EXCESS_VERBOSITY */
#define FIELD_EVERB(stuff)
#endif /* FIELD_EXCESS_VERBOSITY */


#if (_SHR_PBMP_WORD_MAX > 1)
#if (_SHR_PBMP_WORD_MAX > 2)
#if (_SHR_PBMP_WORD_MAX > 3)
#if (_SHR_PBMP_WORD_MAX > 4)
#define FIELD_PBMP_FORMAT "%08X %08X %08X %08X %08X"
#define FIELD_PBMP_SHOW(pbmp) \
                             _SHR_PBMP_WORD_GET(pbmp,4), \
                             _SHR_PBMP_WORD_GET(pbmp,3), \
                             _SHR_PBMP_WORD_GET(pbmp,2), \
                             _SHR_PBMP_WORD_GET(pbmp,1), \
                             _SHR_PBMP_WORD_GET(pbmp,0)
#else /* (_SHR_PBMP_WORD_MAX > 4) */
#define FIELD_PBMP_FORMAT "%08X %08X %08X %08X"
#define FIELD_PBMP_SHOW(pbmp) \
                             _SHR_PBMP_WORD_GET(pbmp,3), \
                             _SHR_PBMP_WORD_GET(pbmp,2), \
                             _SHR_PBMP_WORD_GET(pbmp,1), \
                             _SHR_PBMP_WORD_GET(pbmp,0)
#endif /* (_SHR_PBMP_WORD_MAX > 4) */
#else /* (_SHR_PBMP_WORD_MAX > 3) */
#define FIELD_PBMP_FORMAT "%08X %08X %08X"
#define FIELD_PBMP_SHOW(pbmp) \
                             _SHR_PBMP_WORD_GET(pbmp,2), \
                             _SHR_PBMP_WORD_GET(pbmp,1), \
                             _SHR_PBMP_WORD_GET(pbmp,0)
#endif /* (_SHR_PBMP_WORD_MAX > 3) */
#else /* (_SHR_PBMP_WORD_MAX > 2) */
#define FIELD_PBMP_FORMAT "%08X %08X"
#define FIELD_PBMP_SHOW(pbmp) \
                             _SHR_PBMP_WORD_GET(pbmp,1), \
                             _SHR_PBMP_WORD_GET(pbmp,0)
#endif /* (_SHR_PBMP_WORD_MAX > 2) */
#else /* (_SHR_PBMP_WORD_MAX > 1) */
#define FIELD_PBMP_FORMAT "%08X"
#define FIELD_PBMP_SHOW(pbmp) \
                             _SHR_PBMP_WORD_GET(pbmp,0)
#endif /* (_SHR_PBMP_WORD_MAX > 1) */

/* These next two MACRO operates on an array of characters */
#define FIELD_SHORT_FORMAT "%02X%02X"
#define FIELD_SHORT_SHOW_REVERSE(bytes) \
                             (bytes)[1], \
                             (bytes)[0]

#define FIELD_INT_FORMAT "%02X%02X%02X%02X"
#define FIELD_INT_SHOW_REVERSE(bytes) \
                             (bytes)[3], \
                             (bytes)[2], \
                             (bytes)[1], \
                             (bytes)[0]

#define FIELD_MACA_FORMAT "%02X:%02X:%02X:%02X:%02X:%02X"
#define FIELD_MACA_SHOW(maca) \
                             (maca)[0], \
                             (maca)[1], \
                             (maca)[2], \
                             (maca)[3], \
                             (maca)[4], \
                             (maca)[5]

#define FIELD_MACA_SHOW_REVERSE(maca) \
                             (maca)[5], \
                             (maca)[4], \
                             (maca)[3], \
                             (maca)[2], \
                             (maca)[1], \
                             (maca)[0]

#define FIELD_LLC_F0RMAT "%02X:%02X:%02X:"
#define FIELD_LLC_SHOW(llc) \
                             (llc).dsap, \
                             (llc).ssap, \
                             (llc).control

#define FIELD_IPV6A_FORMAT \
    "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
#define FIELD_IPV6A_SHOW(ipv6a) \
                  (ipv6a)[0], (ipv6a)[1], \
                  (ipv6a)[2], (ipv6a)[3], \
                  (ipv6a)[4], (ipv6a)[5], \
                  (ipv6a)[6], (ipv6a)[7], \
                  (ipv6a)[8], (ipv6a)[9], \
                  (ipv6a)[10], (ipv6a)[11], \
                  (ipv6a)[12], (ipv6a)[13], \
                  (ipv6a)[14], (ipv6a)[15]

/*
 *  I can't think of why you'd want to turn off certain displays, but there
 *  is a way to do so here...
 */
#define FIELD_PRINT(stuff) bsl_printf stuff

/*
 *  Since we can compact the field API indices internally, we need types that
 *  will accurately reflect the values.  Still, I think signed indices are a
 *  pretty bad idea, but...  here they are:
 */
#if _SBX_CALADAN3_FIELD_INDEX_COMPACT
typedef int16               _bcm_caladan3_field_group_index;
typedef int16               _bcm_caladan3_field_entry_index;
typedef int16               _bcm_caladan3_field_range_index;
typedef int16               _bcm_caladan3_field_ptype_index;
typedef uint16              _bcm_caladan3_field_count;
#define _SBX_CALADAN3_FIELD_INDEX_FORMAT "%04X"
#define _SBX_CALADAN3_FIELD_INDEX_WIDTH 4
#else /* _SBX_CALADAN3_FIELD_INDEX_COMPACT */
typedef bcm_field_group_t   _bcm_caladan3_field_group_index;
typedef bcm_field_entry_t   _bcm_caladan3_field_entry_index;
typedef bcm_field_range_t   _bcm_caladan3_field_range_index;
typedef int                 _bcm_caladan3_field_ptype_index;
typedef unsigned int        _bcm_caladan3_field_count;
#define _SBX_CALADAN3_FIELD_INDEX_FORMAT "%08X"
#define _SBX_CALADAN3_FIELD_INDEX_WIDTH 8
#endif /* _SBX_CALADAN3_FIELD_INDEX_COMPACT */

#define _SBX_CALADAN3_FIELD_ENTRY_PRIO_HIGHEST 0x3FFF
#define _SBX_CALADAN3_FIELD_GROUP_PRIO_ANY     -0x3FFF

/*
 *  This describes our internal view of a range.  Seems contrived, but there's
 *  probably a reason for this.  In any case, this thing appears only to be
 *  useful (to us) for specifying TCP/UDP port number ranges.
 *
 *  Multiple entries can reference a range, and changing the range does not
 *  affect the entry, as the port data are actually stored in the entry itself
 *  rather than having a range index.  This probably violates the typical BCM
 *  API behaviour assumptions about ranges, but to implement it where changing
 *  a range affects the associated entries, we'd have to scan the hardware
 *  data, update them, and commit the changes every time somebody changes a
 *  range's values.  Doesn't really seem worth it.  Even if a range is freed
 *  with entries that used it, those entries continue to have the range data.
 *
 *  Ranges are managed using the simple freelist manager (not the block one, as
 *  they are only allocated one at a time).  No lists of ranges are kept inside
 *  this module other than the freelist managed by the freelist manager module.
 */
typedef struct _bcm_caladan3_field_range_s {
    /* fields mandated by the API */
    uint32          flags;                  /* indicate comparison mode */
    bcm_l4_port_t   min;                    /* lowest port number to match */
    bcm_l4_port_t   max;                    /* highest port number to match */
    int             invert;                 /* set if range is inverted */
    /* fields needed for accounting */
} _bcm_caladan3_field_range_t;

typedef struct _sbx_caladan3_field_header_info_s {
    bcm_field_qualify_t         bcmQualType;
    soc_c3_rce_data_header_t    header;
    int                         startBit;
    unsigned int                numBits;
    soc_c3_rce_qual_type_t      socQualType;
} _sbx_caladan3_field_header_info_t;

#include <shared/idxres_fl.h>
#include <soc/sbx/sbx_drv.h>

/* Unit lock and unit specific information needed at global layer */
typedef struct _sbx_caladan3_field_unit_info_s {
    sal_mutex_t lock;                      /* operational lock for this unit */
    soc_sbx_ucode_type_t microcode;        /* this unit's microcode type */
    void *data;                            /* this unit's private data */
} _sbx_caladan3_field_unit_info_t;

/* we need to know how many stages at compile time */

#define _FIELD_CALADAN3_G3P1_STAGES 2

/*
 *  *  Only set this TRUE if you really want a lot of output from the field_show
 *   *  and field_dump functions.  This adds a dump of any associated lists.  It
 *    *  should probably be left FALSE unless debugging these lists.  Note this AND
 *     *  _SBX_CALADAN3_FIELD_DIAGNOSTIC_MODE must be TRUE to get the lists.
 *      */
#define _FIELD_CALADAN3_G3P1_DUMP_LISTS FALSE

/*
 *  *  Set this TRUE if you want all rules that apply to an entry to be dumped;
 *   *  set it FALSE if you're happy only seeing the first rule that applies to an
 *    *  entry dumped with the other rules only displaying rule ID and counters.
 *     */
#define _FIELD_CALADAN3_G3P1_DUMP_ALL_RULES TRUE

#define _FIELD_CALADAN3_G3P1_MAX_GROUPS 40





/*
 *  Flags to describe our internal view of an entry.
 */
typedef enum _bcm_caladan3_g3p1_fld_entry_flags_e {
    _CALADAN3_G3P1_ENTRY_VALID    = 0x00000001, /* entry is in use */
    _CALADAN3_G3P1_ENTRY_IN_HW    = 0x00000004, /* entry is in hardware */
    _CALADAN3_G3P1_ENTRY_COUNTER  = 0x00000010, /* entry has a counter */
    _CALADAN3_G3P1_ENTRY_ACTIONS  = 0x000FFF20, /* entry actions mask */
    _CALADAN3_G3P1_ENTRY_ACT_RES  = 0x00041000, /*  actions with resources */
    _CALADAN3_G3P1_ENTRY_ACT_FTVL = 0x00001E00, /*  actions using FT/VLAN ID */
    _CALADAN3_G3P1_ENTRY_DROP     = 0x00000100, /*  entry has Drop action */
    _CALADAN3_G3P1_ENTRY_VLANNEW  = 0x00000200, /*  entry has VlanNew action */
    _CALADAN3_G3P1_ENTRY_L3SWITCH = 0x00000400, /*  entry has L3Switch action */
    _CALADAN3_G3P1_ENTRY_REDIRECT = 0x00000800, /*  entry has redirect action */
    _CALADAN3_G3P1_ENTRY_REDIRMC  = 0x00001000, /*  entry has redirectMcast act */
    _CALADAN3_G3P1_ENTRY_POLICER  = 0x00002000, /*  entry has policer act */
    _CALADAN3_G3P1_ENTRY_COSQNEW  = 0x00010000, /*  entry has CosQNew action */
    _CALADAN3_G3P1_ENTRY_DROPPREC = 0x00020000, /*  entry has DropPrecedence act */
    _CALADAN3_G3P1_ENTRY_MIRROR   = 0x00040000, /*  entry has Mirror* action */
    _CALADAN3_G3P1_ENTRY_CPYTOCPU = 0x00080000, /*  entry has CopyToCpu action */
    _CALADAN3_G3P1_ENTRY_POLFLAGS = (int)0xE0000000, /* policer flags mask */
    _CALADAN3_G3P1_ENTRY_TYPEPOL  = 0x20000000, /*  policer is typed */
    _CALADAN3_G3P1_ENTRY_MEFCOS   = 0x40000000, /*  polcier is MEF by CoS */
    _CALADAN3_G3P1_ENTRY_MEF      = (int)0x80000000  /*  policer is MEF */
} _bcm_caladan3_g3p1_fld_entry_flags_t;

/*
 *  This structure describes our internal view of an entry.
 *
 *  Entries are collected into a sorted linked list per group, plus a free
 *  list.  Each group's entries are sorted by priority, but multiple entries
 *  are allowed to have the same priority (any inserts of priority x are
 *  performed after any other priority x or higher entries in the list).  The
 *  priority rules are not simple; see the _bcm_caladan3_compare_entry_priority
 *  function for an explanation.  The free list is, like the group free list,
 *  an unsorted stack with the entries blanked before being placed into it.
 *
 *  The SBX description of the 'field' rule is huge.  So much so that including
 *  the rule data for each entry in the module's index table will increase it's
 *  already heavy memory consumption by a factor in excess of four. Because of
 *  this, we store all of the rule data in the SBX API, without replicating it
 *  here except as transient data on the stack.  While this adds quite a bit to
 *  the stack depth and processing time, the argument was that rule updates
 *  should not be happening too quickly and that static and heap data should be
 *  kept minimal.
 *
 *  The next/prev entry (rulebase) fields are to speed searches when doing
 *  inserts to the hardware -- need to find the previous SBX handle for the
 *  rulebase so we can insert 'after' it for ordering; need next so
 *  we can do O(1) insert/delete from this list.
 */
typedef struct _bcm_caladan3_g3p1_field_entry_data_tgt_s {
    bcm_module_t module;                            /* target module */
    bcm_port_t port;                                /* target port */
} _bcm_caladan3_g3p1_field_entry_data_tgt_t;
typedef union _bcm_caladan3_g3p1_field_entry_data_u {
    _bcm_caladan3_g3p1_field_entry_data_tgt_t target;   /* target module/port */
    uint32 ftHandle;                                /* FT entry handle */
    uint32 mcHandle;                                /* MC group handle */
    bcm_vlan_t VSI;                                 /* VSI */
} _bcm_caladan3_g3p1_field_entry_data_t;
#define _FIELD_CALADAN3_G3P1_RANGES_PER_ENTRY 2
typedef struct _bcm_caladan3_g3p1_field_entry_s {
    int16 priority;                                 /* priority */
    _bcm_caladan3_g3p1_field_entry_data_t ftvlData;     /* FT / VLAN data */
    bcm_policer_t policer;                          /* policer */
    bcm_field_range_t range[_FIELD_CALADAN3_G3P1_RANGES_PER_ENTRY]; /* ranges */
    _bcm_caladan3_g3p1_fld_entry_flags_t entryFlags;    /* internal flags */
    _bcm_caladan3_field_group_index group;              /* group */
    _bcm_caladan3_field_entry_index prevEntry;          /* prev entry (group) */
    _bcm_caladan3_field_entry_index nextEntry;          /* next entry (group) */
    _bcm_caladan3_field_entry_index prevEntryRb;        /* next entry (rulebase) */
    _bcm_caladan3_field_entry_index nextEntryRb;        /* next entry (rulebase) */
} _bcm_caladan3_g3p1_field_entry_t;

/*
 *  This structure describes our internal view of a group.  Note this has a lot
 *  of information that is BCM specific state instead of anything useful for
 *  the SBX implementation.
 *
 *  Groups are arranged into lists for each stage, plus a free list.  The stage
 *  lists are strict priority sort (highest numeric priority first and lowest
 *  numeric priority last).  The free list is more of an unsorted stack (but it
 *  doesn't matter because entries in the free list are zeroed first).
 *
 *  Each group can contain a list of entries.
 *
 *  The counters field indicates the *potential* number of counters, and will
 *  be equal to the number of entries unless some entries are sharing counters,
 *  in which case it will be less.
 *
 *  Here, rulebase is the rulebase index from the unit data; if the group is
 *  not valid, the rulebase will be _FIELD_CALADAN3_INVALID_RULEBASE, unless the
 *  group is in the process of being allocated, then it's ...TEMP_RULEBASE.
 */
typedef struct _bcm_caladan3_g3p1_field_group_s {
    bcm_field_qset_t qset;                        /* qualifier set bitmap */
    int priority;                                 /* priority */
    _bcm_caladan3_field_count entries;                /* number of entries */
    _bcm_caladan3_field_count counters;               /* number of counters */
    _bcm_caladan3_field_entry_index entryHead;        /* first entry ID */
    _bcm_caladan3_field_entry_index entryTail;        /* last entry ID */
    _bcm_caladan3_field_group_index nextGroup;        /* next group ID */
    _bcm_caladan3_field_group_index prevGroup;        /* previous group ID */
    uint8 rulebase;                               /* which rulebase */
} _bcm_caladan3_g3p1_field_group_t;
#define _FIELD_CALADAN3_INVALID_RULEBASE 0xFF         /* not allocated */
#define _FIELD_CALADAN3_TEMP_RULEBASE 0xFE            /* allocated but no rb */


/*
 *  Our internal view of a rulebase.
 *
 *  The entryHead field is so we can more quickly find entry information when
 *  searching for placement within the rulebase.
 */
typedef struct _bcm_caladan3_g3p1_field_rulebase_s {
    bcm_pbmp_t ports;                                        /* pbmp */
    bcm_field_qset_t qset;                                   /* qualifiers */
    SHR_BITDCL action[_SHR_BITDCLSIZE(bcmFieldActionCount)]; /* actions */
    _bcm_caladan3_field_count rules;                             /* curr rules */
    _bcm_caladan3_field_count rulesMax;                          /* max rules */
    _bcm_caladan3_field_count entries;                           /* curr entries */
    _bcm_caladan3_field_count entriesMax;                        /* max entries */
    _bcm_caladan3_field_entry_index entryHead;                   /* first entry */
    _bcm_caladan3_field_group_index groupHead;                   /* first group */
} _bcm_caladan3_g3p1_field_rulebase_t;

/*
 *  This is the global struct per unit; it has local data and pointers to the
 *  arrays that are allocated on init to store the above structures.
 *
 *  We don't use the indexed resource freelist handler here for groups and
 *  entries because they already have to be linked into lists while in use, so
 *  building a linked list of free elements here effectively is cheaper in
 *  terms of both processor and memory than using the indexed resource freelist
 *  handler to implement the list.
 *
 *  We do use the indexed resource freelist handler for the ranges since they
 *  don't have to be linked into lists and therefore the indexed resource
 *  freelist structure is smaller than expanding ranges to be linkable, and
 *  safer than reusing some other field as linkage.
 *
 *  Note that the BCM 'stage' concept maps directly to the SBX 'rulebase'.
 */
typedef struct _bcm_caladan3_g3p1_field_glob_s {
    _bcm_caladan3_g3p1_field_rulebase_t rulebase[_FIELD_CALADAN3_G3P1_STAGES];/* rbs */
    _bcm_caladan3_g3p1_field_group_t *group;            /* ptr to group array */
    _bcm_caladan3_g3p1_field_entry_t *entry;            /* ptr to entries array */
    _bcm_caladan3_field_range_t *range;                 /* ptr to ranges array */
    shr_idxres_list_handle_t rangeFree;             /* range free list */
    int unit;                                       /* BCM layer unit ID */
    _bcm_caladan3_field_count groupFreeCount;           /* free groups */
    _bcm_caladan3_field_count groupTotal;               /* total groups */
    _bcm_caladan3_field_count entryFreeCount;           /* free entries */
    _bcm_caladan3_field_count entryTotal;               /* total entries */
    _bcm_caladan3_field_count rangeTotal;               /* total ranges */
    _bcm_caladan3_field_group_index groupFreeHead;      /* head of free groups */
    _bcm_caladan3_field_entry_index entryFreeHead;      /* head of free entries */
    bcm_policer_t dropPolicer;                      /* policer for drop actn */
    uint8 uMaxSupportedStages;
} _bcm_caladan3_g3p1_field_glob_t;

/* Table for providing parameters to soc rce action calls. */

typedef struct _bcm_c3_rce_action_arg_s {
    unsigned int    index;          /* SOC index of action to read/write */
    unsigned int    modIndex;       /* SOC index of enable or modifier bits */
    unsigned int    disableVal;     /* SOC value to disable action */
} _bcm_c3_rce_action_arg_t;


/******************************************************************************
 *
 *  Exports to submodules (see main CALADAN3 field module for code comments)
 */

extern const char *_sbx_caladan3_field_qual_name[bcmFieldQualifyCount];
extern const char *_sbx_caladan3_field_action_name[bcmFieldActionCount];
#ifdef BROADCOM_DEBUG
extern char *_bcm_caladan3_field_mac_expand(char *result,
                                          unsigned int length,
                                          uint64 macAddr);
extern int _bcm_caladan3_field_qset_dump(const bcm_field_qset_t qset,
                                       const char *prefix);
#endif /* def BROADCOM_DEBUG */

extern signed int _bcm_caladan3_compare_entry_priority(int pri1, int pri2);
int _bcm_caladan3_qset_subset(bcm_field_qset_t qset1, bcm_field_qset_t qset2);
extern void _bcm_caladan3_bcm_debug(const int flags, const char *message);

extern _sbx_caladan3_field_header_info_t caladan3_rce_field_info[];
extern _sbx_caladan3_field_header_info_t caladan3_ip6_rce_field_info[];


/******************************************************************************
 *
 *  Exports from submodules (see submodules for code comments)
 */

#ifdef BCM_CALADAN3_G3P1_SUPPORT
/* G3P1 */
extern int bcm_caladan3_g3p1_field_init(int unit, void **unitData);
extern int bcm_caladan3_g3p1_field_detach(void *unitData);
extern int bcm_caladan3_g3p1_field_group_port_create_mode_id(void *unitData,
                                                           bcm_port_t port,
                                                           bcm_field_qset_t qset,
                                                           int pri,
                                                           bcm_field_group_mode_t mode,
                                                           bcm_field_group_t group);
extern int bcm_caladan3_g3p1_field_group_port_create_mode(void *unitData,
                                                        bcm_port_t port,
                                                        bcm_field_qset_t qset,
                                                        int pri,
                                                        bcm_field_group_mode_t mode,
                                                        bcm_field_group_t *group);
extern int bcm_caladan3_g3p1_field_group_create_id(void *unitData,
                                                 bcm_field_qset_t qset,
                                                 int pri,
                                                 bcm_field_group_t group);
extern int bcm_caladan3_g3p1_field_group_create(void *unitData,
                                              bcm_field_qset_t qset,
                                              int pri,
                                              bcm_field_group_t *group);
extern int bcm_caladan3_g3p1_field_group_get(void *unitData,
                                           bcm_field_group_t group,
                                           bcm_field_qset_t *qset);
extern int bcm_caladan3_g3p1_field_group_destroy(void *unitData,
                                               bcm_field_group_t group);
extern int bcm_caladan3_g3p1_field_group_install(void *unitData,
                                               bcm_field_group_t group);
extern int bcm_caladan3_g3p1_field_group_remove(void *unitData,
                                              bcm_field_group_t group);

extern int bcm_caladan3_g3p1_field_group_flush(void *unitData,
                                             bcm_field_group_t group);
extern int bcm_caladan3_g3p1_field_group_set(void *unitData,
                                           bcm_field_group_t group,
                                           bcm_field_qset_t qset);
extern int bcm_caladan3_g3p1_field_group_status_get(void *unitData,
                                                  bcm_field_group_t group,
                                                  bcm_field_group_status_t *status);
extern int bcm_caladan3_g3p1_field_range_create(void *unitData,
                                              bcm_field_range_t *range,
                                              uint32 flags,
                                              bcm_l4_port_t min,
                                              bcm_l4_port_t max);
extern int bcm_caladan3_g3p1_field_range_create_id(void *unitData,
                                                 bcm_field_range_t range,
                                                 uint32 flags,
                                                 bcm_l4_port_t min,
                                                 bcm_l4_port_t max);
extern int bcm_caladan3_g3p1_field_range_get(void *unitData,
                                           bcm_field_range_t range,
                                           uint32 *flags,
                                           bcm_l4_port_t *min,
                                           bcm_l4_port_t *max);
extern int bcm_caladan3_g3p1_field_range_destroy(void *unitData,
                                               bcm_field_range_t range);
extern int bcm_caladan3_g3p1_field_entry_create(void *unitData,
                                              bcm_field_group_t group,
                                              bcm_field_entry_t *entry);
extern int bcm_caladan3_g3p1_field_entry_create_id(void *unitData,
                                                 bcm_field_group_t group,
                                                 bcm_field_entry_t entry);
extern int bcm_caladan3_g3p1_field_entry_destroy(void *unitData,
                                               bcm_field_entry_t entry);
extern int bcm_caladan3_g3p1_field_entry_destroy_all(void *unitData);
extern int bcm_caladan3_g3p1_field_entry_copy(void *unitData,
                                            bcm_field_entry_t src_entry,
                                            bcm_field_entry_t *dst_entry);
extern int bcm_caladan3_g3p1_field_entry_copy_id(void *unitData,
                                               bcm_field_entry_t src_entry,
                                               bcm_field_entry_t dst_entry);
extern int bcm_caladan3_g3p1_field_entry_install(void *unitData,
                                               bcm_field_entry_t entry);
extern int bcm_caladan3_g3p1_field_entry_reinstall(void *unitData,
                                                 bcm_field_entry_t entry);
extern int bcm_caladan3_g3p1_field_entry_remove(void *unitData,
                                              bcm_field_entry_t entry);
extern int bcm_caladan3_g3p1_field_entry_prio_get(void *unitData,
                                                bcm_field_entry_t entry,
                                                int *prio);
extern int bcm_caladan3_g3p1_field_entry_prio_set(void *unitData,
                                                bcm_field_entry_t entry,
                                                int prio);
extern int bcm_caladan3_g3p1_field_qualify_clear(void *unitData,
                                               bcm_field_entry_t entry);

extern int bcm_caladan3_g3p1_field_qualify_InPort(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_port_t data,
                                                bcm_port_t mask);
extern int bcm_caladan3_g3p1_field_qualify_InPort_get(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_port_t *data,
                                                bcm_port_t *mask);
extern int bcm_caladan3_g3p1_field_qualify_OutPort(void *unitData,
                                                 bcm_field_entry_t entry,
                                                 bcm_port_t data,
                                                 bcm_port_t mask);
extern int bcm_caladan3_g3p1_field_qualify_OutPort_get(void *unitData,
                                                     bcm_field_entry_t entry,
                                                     bcm_port_t *data,
                                                     bcm_port_t *mask);
extern int bcm_caladan3_g3p1_field_qualify_OuterVlan(void *unitData,
                                                   bcm_field_entry_t entry,
                                                   bcm_vlan_t data,
                                                   bcm_vlan_t mask);
extern int bcm_caladan3_g3p1_field_qualify_OuterVlan_get(void *unitData,
                                                   bcm_field_entry_t entry,
                                                   bcm_vlan_t *data,
                                                   bcm_vlan_t *mask);
extern int bcm_caladan3_g3p1_field_qualify_OuterVlanId(void *unitData,
                                                     bcm_field_entry_t entry,
                                                     bcm_vlan_t data,
                                                     bcm_vlan_t mask);
extern int bcm_caladan3_g3p1_field_qualify_OuterVlanId_get(void *unitData,
                                                     bcm_field_entry_t entry,
                                                     bcm_vlan_t *data,
                                                     bcm_vlan_t *mask);
extern int bcm_caladan3_g3p1_field_qualify_OuterVlanPri(void *unitData,
                                                      bcm_field_entry_t entry,
                                                      uint8 data,
                                                      uint8 mask);
extern int bcm_caladan3_g3p1_field_qualify_OuterVlanPri_get(void *unitData,
                                                      bcm_field_entry_t entry,
                                                      uint8 *data,
                                                      uint8 *mask);
extern int bcm_caladan3_g3p1_field_qualify_OuterVlanCfi(void *unitData,
                                                      bcm_field_entry_t entry,
                                                      uint8 data,
                                                      uint8 mask);
extern int bcm_caladan3_g3p1_field_qualify_OuterVlanCfi_get(void *unitData,
                                                      bcm_field_entry_t entry,
                                                      uint8 *data,
                                                      uint8 *mask);
extern int bcm_caladan3_g3p1_field_qualify_EtherType(void *unitData,
                                                   bcm_field_entry_t entry,
                                                   uint16 data,
                                                   uint16 mask);
extern int bcm_caladan3_g3p1_field_qualify_EtherType_get(void *unitData,
                                                   bcm_field_entry_t entry,
                                                   uint16 *data,
                                                   uint16 *mask);
extern int bcm_caladan3_g3p1_field_qualify_IpProtocol(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    uint8 data,
                                                    uint8 mask);
extern int bcm_caladan3_g3p1_field_qualify_IpProtocol_get(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    uint8 *data,
                                                    uint8 *mask);
extern int bcm_caladan3_g3p1_field_qualify_SrcIp(void *unitData,
                                               bcm_field_entry_t entry,
                                               bcm_ip_t data,
                                               bcm_ip_t mask);
extern int bcm_caladan3_g3p1_field_qualify_SrcIp_get(void *unitData,
                                               bcm_field_entry_t entry,
                                               bcm_ip_t *data,
                                               bcm_ip_t *mask);
extern int bcm_caladan3_g3p1_field_qualify_DstIp(void *unitData,
                                               bcm_field_entry_t entry,
                                               bcm_ip_t data,
                                               bcm_ip_t mask);
extern int bcm_caladan3_g3p1_field_qualify_DstIp_get(void *unitData,
                                               bcm_field_entry_t entry,
                                               bcm_ip_t *data,
                                               bcm_ip_t *mask);
extern int bcm_caladan3_g3p1_field_qualify_SrcIp6(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_ip6_t data,
                                                bcm_ip6_t mask);
extern int bcm_caladan3_g3p1_field_qualify_SrcIp6_get(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_ip6_t *data,
                                                bcm_ip6_t *mask);
extern int bcm_caladan3_g3p1_field_qualify_DstIp6(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_ip6_t data,
                                                bcm_ip6_t mask);
extern int bcm_caladan3_g3p1_field_qualify_DstIp6_get(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_ip6_t *data,
                                                bcm_ip6_t *mask);
extern int bcm_caladan3_g3p1_field_qualify_Ip6NextHeader(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    uint8 data,
                                                    uint8 mask);
extern int bcm_caladan3_g3p1_field_qualify_Ip6NextHeader_get(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    uint8 *data,
                                                    uint8 *mask);
extern int bcm_caladan3_g3p1_field_qualify_Ip6TrafficClass(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    uint8 data,
                                                    uint8 mask);
extern int bcm_caladan3_g3p1_field_qualify_Ip6TrafficClass_get(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    uint8 *data,
                                                    uint8 *mask);
extern int bcm_caladan3_g3p1_field_qualify_DSCP(void *unitData,
                                              bcm_field_entry_t entry,
                                              uint8 data,
                                              uint8 mask);
extern int bcm_caladan3_g3p1_field_qualify_L4SrcPort(void *unitData,
                                                   bcm_field_entry_t entry,
                                                   bcm_l4_port_t data,
                                                   bcm_l4_port_t mask);
extern int bcm_caladan3_g3p1_field_qualify_L4SrcPort_get(void *unitData,
                                                   bcm_field_entry_t entry,
                                                   bcm_l4_port_t *data,
                                                   bcm_l4_port_t *mask);
extern int bcm_caladan3_g3p1_field_qualify_L4DstPort(void *unitData,
                                                   bcm_field_entry_t entry,
                                                   bcm_l4_port_t data,
                                                   bcm_l4_port_t mask);
extern int bcm_caladan3_g3p1_field_qualify_L4DstPort_get(void *unitData,
                                                   bcm_field_entry_t entry,
                                                   bcm_l4_port_t *data,
                                                   bcm_l4_port_t *mask);
extern int bcm_caladan3_g3p1_field_qualify_DSCP_get(void *unitData,
                                              bcm_field_entry_t entry,
                                              uint8 *data,
                                              uint8 *mask);
extern int bcm_caladan3_g3p1_field_qualify_TcpControl(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    uint8 data,
                                                    uint8 mask);
extern int bcm_caladan3_g3p1_field_qualify_TcpControl_get(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    uint8 *data,
                                                    uint8 *mask);
extern int bcm_caladan3_g3p1_field_qualify_RangeCheck(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    bcm_field_range_t range,
                                                    int invert);
extern int bcm_caladan3_g3p1_field_qualify_RangeCheck_get(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    int max_count,
                                                    bcm_field_range_t *range,
                                                    int *invert,
                                                    int *count);
extern int bcm_caladan3_g3p1_field_qualify_SrcMac(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_mac_t data,
                                                bcm_mac_t mask);
extern int bcm_caladan3_g3p1_field_qualify_SrcMac_get(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_mac_t *data,
                                                bcm_mac_t *mask);
extern int bcm_caladan3_g3p1_field_qualify_DstMac(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_mac_t data,
                                                bcm_mac_t mask);
extern int bcm_caladan3_g3p1_field_qualify_DstMac_get(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_mac_t *data,
                                                bcm_mac_t *mask);

extern int bcm_caladan3_g3p1_field_action_add(void *unitData,
                                            bcm_field_entry_t entry,
                                            bcm_field_action_t action,
                                            uint32 param0,
                                            uint32 param1);
extern int bcm_caladan3_g3p1_field_action_get(void *unitData,
                                            bcm_field_entry_t entry,
                                            bcm_field_action_t action,
                                            uint32 *param0,
                                            uint32 *param1);
extern int bcm_caladan3_g3p1_field_action_remove(void *unitData,
                                               bcm_field_entry_t entry,
                                               bcm_field_action_t action);
extern int bcm_caladan3_g3p1_field_action_remove_all(void *unitData,
                                                   bcm_field_entry_t entry);
extern int bcm_caladan3_g3p1_field_counter_create(void *unitData,
                                                bcm_field_entry_t entry);
extern int bcm_caladan3_g3p1_field_counter_share(void *unitData,
                                               bcm_field_entry_t src_entry,
                                               bcm_field_entry_t dst_entry);
extern int bcm_caladan3_g3p1_field_counter_destroy(void *unitData,
                                                 bcm_field_entry_t entry);
extern int bcm_caladan3_g3p1_field_counter_set(void *unitData,
                                             bcm_field_entry_t entry,
                                             int counter_num,
                                             uint64 val);
extern int bcm_caladan3_g3p1_field_counter_get(void *unitData,
                                             bcm_field_entry_t entry,
                                             int counter_num,
                                             uint64 *val);
extern int bcm_caladan3_g3p1_field_entry_policer_attach(void *unitData,
                                                      bcm_field_entry_t entry_id,
                                                      int level,
                                                      bcm_policer_t policer_id);
extern int bcm_caladan3_g3p1_field_entry_policer_detach(void *unitData,
                                                      bcm_field_entry_t entry_id,
                                                      int level);
extern int bcm_caladan3_g3p1_field_entry_policer_detach_all(void *unitData,
                                                          bcm_field_entry_t entry_id);
extern int bcm_caladan3_g3p1_field_entry_policer_get(void *unitData,
                                                   bcm_field_entry_t entry_id,
                                                   int level,
                                                   bcm_policer_t *policer_id);

extern int bcm_caladan3_g3p1_field_show(void *unitData, const char *pfx);
extern int bcm_caladan3_g3p1_field_entry_dump(void *unitData,
                                            bcm_field_entry_t entry);
extern int bcm_caladan3_g3p1_field_group_dump(void *unitData,
                                            bcm_field_group_t group);
extern void bcm_caladan3_g3p1_field_unit_data_size_get(void *unitData,
                                                  unsigned int *unit_data_size);
#endif /* ifdef BCM_CALADAN3_G3P1_SUPPORT */

#endif /* ifndef _SBX_CALADAN3_FIELD_H_ */

