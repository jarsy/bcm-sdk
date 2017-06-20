/*
 * $Id: field.h,v 1.39 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        field.h
 * Purpose:     Internal Field Processor data structure definitions for the
 *              BCM library.
 *
 */

#ifndef _BCM_INT_FIELD_H
#define _BCM_INT_FIELD_H

#include <bcm/field.h>
#include <bcm_int/common/field.h>

#include <soc/mem.h>
#include <soc/cfp.h>
#include <soc/vm.h>


#include <sal/core/sync.h>
#include <sal/core/thread.h>

#ifdef BCM_FIELD_SUPPORT 


/* 5348 */

#define FP_KEY_PATTERN      (0xdeadbeef)

#define FP_SLICE_SZ_MAX         1024

#define BCM_FIELD_USER_NUM_UDFS_MAX     93 


/*
 * Meter setting limits.
 */
#define BCM_FIELD_METER_KBITS_BURST_MAX   (128000)             /* 128Mb = 16MB      */
#define BCM_FIELD_METER_KBITS_RATE_MAX    (1000000) /* 1000Mb */

#define BCM_53242_FIELD_METER_KBITS_BURST_MAX   (8128)             /* 1016 * 8      */

/* Value for fc->tcam_ext_numb that indicates there is no external TCAM. */
#define FP_EXT_TCAM_NONE -1


#define _FP_PARAM_INVALID   -1

#define _FP_INVALID_INDEX            (-1)
/*
 * Initial group IDs and entry IDs.
 */
#define _FP_GROUP_ID_BASE 1
#define _FP_ENTRY_ID_BASE 1
#define _FP_ENTRY_ID_MAX (0x1000000)

/*
 * Internal version of qset add that will allow UserDefined[01] to be set. 
 */
#define BCM_FIELD_QSET_ADD_INTERNAL(qset, q)  SHR_BITSET(((qset).w), (q))

/*
 * FPFx field select code macros
 *
 * These are for resolving if the select code has meaning or is really a don't
 * care.
 */

#define _FP_SELCODE_INVALID (-1)

#define _FP_SELCODE_INVALIDATE(selcode) \
     ((selcode) = _FP_SELCODE_INVALID);

#define _FP_SELCODE_IS_VALID(selcode) \
     (((selcode) == _FP_SELCODE_INVALID)?0:1)

#define _FP_SELCODE_IS_INVALID(selcode) \
     (!_FP_SELCODE_IS_VALID(selcode))

/*
 * Macros for _robo_field_entry_get() to specify which of the potentially 3
 * (triple-wide) entries to find.
 */
#define _FP_SLICE_PRIMARY     0 /* Base   slice for group */
#define _FP_SLICE_SECONDARY   1 /* Base+1 slice for group */
#define _FP_SLICE_TERTIARY    2 /* Base+2 slice for group */


/* Action allow check macro. */
#define _FP_ACTIONS_ALLOW(_val_)    \
    if (act1 == _val_) {             \
        return (TRUE);         \
    }

/*
 * Macro:
 *     FIELD_ENTRY_SLICE_IDX_CHANGE
 * Purpose:
 *     Move the software entry to a new slice index.
 * Parameters:
 *     f_ent    - entry to be moved
 *     amount   - number of indexes to move + or -
 */
#define FIELD_ENTRY_SLICE_IDX_CHANGE(f_ent, amount)                 \
    assert((f_ent) != NULL);                                         \
    /* Move the software entry to the new index. */                  \
    (f_ent)->slice_idx += (amount);

#define FIELD_ENTRY_CHAIN_IDX_CHANGE(f_ent, amount)                 \
    assert((f_ent) != NULL);                                         \
    /* Move the software entry to the new index. */                  \
    (f_ent)->chain_idx += (amount);


#define _FIELD_SELCODE_CLEAR(unit, selcode)      \
    sal_memset(&selcode, _FP_SELCODE_INVALID, sizeof(struct _field_sel_s));


#define _FP_HASH_SZ(_fc_)   \
           (0x100)
#define _FP_HASH_INDEX_MASK(_fc_) (_FP_HASH_SZ(_fc_) - 1)

#define _FP_HASH_INSERT(_hash_ptr_, _inserted_ptr_, _index_)    \
           do {                                                 \
              (_inserted_ptr_)->next = (_hash_ptr_)[(_index_)]; \
              (_hash_ptr_)[(_index_)] = (_inserted_ptr_);       \
           } while (0)

#define _FP_HASH_REMOVE(_hash_ptr_, _entry_type_, _removed_ptr_, _index_)  \
           do {                                                    \
               _entry_type_ *_prev_ent_ = NULL;                    \
               _entry_type_ *_lkup_ent_ = (_hash_ptr_)[(_index_)]; \
               while (NULL != _lkup_ent_) {                        \
                   if (_lkup_ent_ != (_removed_ptr_))  {           \
                       _prev_ent_ = _lkup_ent_;                    \
                       _lkup_ent_ = _lkup_ent_->next;              \
                       continue;                                   \
                   }                                               \
                   if (_prev_ent_!= NULL) {                        \
                       _prev_ent_->next = (_removed_ptr_)->next;   \
                   } else {                                        \
                       (_hash_ptr_)[(_index_)] = (_removed_ptr_)->next; \
                   }                                               \
                   break;                                          \
               }                                                   \
           } while (0)

/*
 * Initial group IDs and entry IDs.
 */
#define _FP_ID_BASE 1
#define _FP_ID_MAX  (0x1000000)

/* Generic memory allocation routine. */
#define _FP_MEM_ALLOC(_ptr_,_size_,_descr_)                 \
            do {                                             \
                if (NULL == (_ptr_)) {                       \
                   (_ptr_) = sal_alloc((_size_), (_descr_)); \
                }                                            \
                if((_ptr_) != NULL) {                        \
                    sal_memset((_ptr_), 0, (_size_));        \
                }  else {                                    \
                    LOG_ERROR(BSL_LS_BCM_FP, \
                              (BSL_META("FP Error: Allocation failure %s\n"), (_descr_))); \
                }                                          \
            } while (0)

/*
 * Typedef:
 *     _field_udf_t
 * Purpose:
 *     Holds user-defined field (UDF) hardware metadata. 
 */
typedef struct _field_udf_s {
    uint8                  valid;     /* Indicates valid UDF             */
    int                    use_count; /* Number of groups using this UDF */
    bcm_field_qualify_t    udf_num;   /* UDFn (UDF0 or UDF1)             */
    uint32       slice_id; /* The slice ID which this UDF reside at */
    uint8                  data_qualifier; /* created by data qualifier */
} _field_udf_t;

/*
 * Struct:
 *     _field_counter32_collect_s
 * Purpose:
 *     Holds the accumulated count of FP Counters 
 *         Useful for wrap around and movement.
 *     This is used when h/w counter width is <= 32 bits
 */
typedef struct _field_counter32_collect_s {
    uint64 accumulated_counter; 
    uint32 last_hw_value;
} _field_counter32_collect_t;


/*
 * Typedef:
 *     _field_data_qualifier_s
 * Purpose:
 *     Data qualifiers description structure.
 */
typedef struct _field_data_qualifier_s {
    int    qid;                     /* Qualifier id.                     */
    bcm_field_data_offset_base_t offset_base; /* Offset base adjustment. */
    int    offset;                  /* Master word offset.               */
    uint32 flags;                   /* Offset adjustment flags.          */
    int    length;                  /* Matched data byte length.         */
    struct _field_data_qualifier_s *next;/* Next installed  qualifier.   */
} _field_data_qualifier_t, *_field_data_qualifier_p;

/*
 * Typedef:
 *     _field_data_tcam_entry_s
 * Purpose:
 *     Field data tcam entry structucture. Used for      
 *     tcam entry allocation and organization by relative priority.
 */
typedef struct _field_data_tcam_entry_s {
   uint8 ref_count;           /* udf tcam entry reference count.  */
   uint8 priority;            /* udf tcam entry priority.         */
} _field_data_tcam_entry_t;


/*
 * Typedef:
 *     _field_data_control_s
 * Purpose:
 *     Field data qualifiers control structucture.     
 *        
 */
typedef struct _field_data_control_s {
   uint32 usage_bmap;                 /* Offset usage bitmap.          */
   _field_data_qualifier_p data_qual; /* List of data qualifiers.      */
   _field_data_tcam_entry_t *tcam_entry_arr;/* Tcam entries/keys array.*/
} _field_data_control_t;


/*
 * Stage flags. 
 */

/* Slice enable/disable support. */
#define _FP_STAGE_SLICE_ENABLE                  (1 << 0)

/*
 * Typedef:
 *     _field_stage_t
 * Purpose:
 *     Pipeline stage field processor information.
 */
typedef struct _field_stage_s {
    _field_stage_id_t      stage_id;        /* Pipeline stage id.           */
    uint8                  flags;           /* Stage flags.                 */
    int                    tcam_sz;         /* Number of entries in TCAM.   */
    int                     tcam_bottom;
    int                    tcam_slices;     /* Number of internal slices.   */
    int                    tcam_slice_sz;/* number of entries per slice    */
    struct _field_entry_s **field_shared_entries;
    int                     field_shared_entries_free;
    struct _field_slice_s  *slices;         /* Array of slices.*/
    struct _field_range_s  *ranges;	    /* List of all ranges allocated.*/     
    uint32         range_id;     /* Seed ID for range creation     */
    _field_data_control_t *data_ctrl;       /* Data qualifiers control.       */
    struct _field_stage_s *next;            /* Next pipeline FP stage.        */
}_field_stage_t;


/*
 * Typedef:
 *     _field_policer_t
 * Purpose:
 *     This is the policer description structure. 
 *     Indexed by bcm_policer_t handle.
 */
typedef struct _field_policer_s {
    bcm_policer_t        pid;         /* Unique policer identifier.       */
    bcm_policer_config_t cfg;         /* API level policer configuration. */
    uint16               sw_ref_count;/* SW object use reference count.   */
    uint16               hw_ref_count;/* HW object use reference count.   */
    uint8                level;       /* Policer attachment level.        */
    int8                 pool_index;  /* Meter pool/slice policer resides.*/
    int                  hw_index;    /* HW index policer resides.        */
    uint32               hw_flags;    /* HW installation status flags.    */
    _field_stage_id_t    stage_id;    /* Attached entry stage id.         */
    struct _field_policer_s *next;    /* Policer lookup linked list.      */
}_field_policer_t;

#define _FP_POLICER_VALID                (1 << 0)
#define _FP_POLICER_INSTALLED            (1 << 1)

/* Committed portion in sw doesn't match hw. */
#define _FP_POLICER_COMMITTED_DIRTY   (0x80000000) 

/* Peak portion in sw doesn't match hw. */
#define _FP_POLICER_PEAK_DIRTY        (0x40000000) 


/* Policer created through meter APIs.  */
#define _FP_POLICER_INTERNAL          (0x20000000) 

#define _FP_POLICER_DIRTY             (_FP_POLICER_COMMITTED_DIRTY | \
                                       _FP_POLICER_PEAK_DIRTY)

#define _FP_POLICER_LEVEL_COUNT       (2)
/*
 * Typedef:
 *     _field_entry_policer_t
 * Purpose:
 *     This is field entry policers description structure.
 *     Used to form an array for currently attached policers. 
 */
typedef struct _field_entry_policer_s {
    bcm_policer_t  pid;         /* Unique policer identifier. */
    uint8          flags;       /* Policer/entry flags.       */
}_field_entry_policer_t;



/*
 * Typedef:
 *     _field_stat_t
 * Purpose:
 *     This is the statistics collection entity description structure.
 *     Indexed by int sid (statistics id) handle.
 *     
 */
typedef struct _field_stat_s {
    uint32               sid;           /* Unique stat entity identifier.  */

    /* Reference counters  information.*/
    uint16               sw_ref_count;  /* SW object use reference count.   */
    uint16               hw_ref_count;  /* HW object use reference count.   */
    /* Allocated hw resource information.*/
    int                  hw_index;      /* HW index stat resides.           */
    /* Reinstall flags. */
    uint32               hw_flags;      /* HW installation status flags.    */
    /* Application requested statistics. */
    uint8                nstat;         /* User requested stats array size. */
    bcm_field_stat_t     *stat_arr;     /* User requested stats array.      */ 
    _field_counter32_collect_t _field_x32_counters[3]; /* green/yellow/red counters*/
    bcm_field_group_t    gid;           /* Group statistics entity attached.*/
    _field_stage_id_t    stage_id;      /* Attached entry stage id.         */
    bcm_field_entry_t   eid;            /* Attached entry id*/
    struct _field_stat_s *next;         /* Stat lookup linked list.         */
    /* Values after last detach. */
    uint64               *stat_values;  /* Stat value after it was detached */
                                        /* from a last entry.               */

} _field_stat_t;


/*
 * Typedef:
 *     _field_entry_stat_t
 * Purpose:
 *     This is field entry statistics collector descriptor structure.
 */
typedef struct _field_entry_stat_s {
    int            sid;         /* Unique statistics entity id.  */
    uint8          flags;       /* Statistics entity/entry flags.*/
}_field_entry_stat_t;

/* Statistics entity attached to fp entry flag. */
#define _FP_ENTRY_STAT_VALID                (1 << 0)
/* Statistics entity installed in HW. */
#define _FP_ENTRY_STAT_INSTALLED            (1 << 1)
/* Statistics entity doesn't have any counters attached. */
#define _FP_ENTRY_STAT_EMPTY                (1 << 2)

/* Statistics entity was  created through counter APIs.  */
#define _FP_STAT_INTERNAL          (1 << 0) 
/* UpdateCounter action was used with NO_YES/YES_NO. */
#define _FP_STAT_COUNTER_PAIR      (1 << 1) 
/* Arithmetic operations. */
#define _FP_STAT_ADD               (1 << 2)
#define _FP_STAT_SUBSTRACT         (1 << 3)
/* Packet/bytes selector. */
#define _FP_STAT_BYTES             (1 << 4)
/* Sw entry doesn't match hw. */
#define _FP_STAT_DIRTY             (1 << 5)
/* Stat Create with ID */
#define _FP_STAT_CREATE_ID         (1 << 6)

/*
 * Typedef:
 *     _field_control_t
 * Purpose:
 *     One structure for each StrataSwitch Device that holds the global
 *     field processor metadata for one device.
 */

typedef struct _field_control_s {
    sal_mutex_t            fc_lock;
    bcm_field_status_t     field_status; /* return status for module       */
    uint8                  color_indep;  /* default color independant flag */
    bcm_field_stage_t      stage;        /* default FP pipeline stage      */
    int                    tcam_ext_numb;/* Slice number for external      */
                                         /* TCAM (-1 if not present)       */
    _field_udf_t           *udf; 
    /* field_status->group_total elements indexed by priority */
    struct _field_group_s  *groups;      /* List of groups in unit         */
    struct _field_stage_s   *stages;     /* Pipeline stage FP info.       */
    /* For TCAM protection thread */
    sal_usecs_t     tcam_interval;       /* Update interval in usec */
    sal_sem_t       tcam_notify;     /* complete notification */
    sal_thread_t    tcam_pid;
    _field_policer_t       **policer_hash;/* Policer lookup hash.          */
    uint32                 policer_count; /* Number of active policers.    */
    _field_stat_t          **stat_hash;   /* Counter lookup hash.          */
    uint32                 stat_count;    /* Number of active counters.    */
} _field_control_t;

/*
 * Typedef:
 *     _field_sel_t
 * Purpose:
 */
typedef struct _field_sel_s {
    int8                fpf; /* field(s) select 3-bit code */
    int8                slice_sel;
    uint32          slice_map;
} _field_sel_t;

/*
 * Typedef:
 *     _field_tcam_t
 * Purpose:
 *     These are the fields that are written into or read from FP_TCAM_xxx.
 */
typedef struct _field_tcam_s {
    uint32                 field[6];
    uint32                 f_mask[6];
} _field_tcam_t;


/*
 * Typedef:
 *     _qual_info_t
 * Purpose:
 *     Holds format info for a particular qualifier's access parameters. These
 *     nodes are stored in qualifier lists for groups and in the FPFx tables
 *     for each chip.
 */
typedef struct _qual_info_s {
    bcm_field_qualify_t    qid;     /* Which Qualifier              */
    drv_cfp_ram_t              mem;     /* Table                        */
    drv_cfp_field_t            field;     /* FPFx field choice            */
    struct _qual_info_s    *next;
} _qual_info_t;

/*
 * Typedef:
 *     _field_counter_t
 * Purpose:
 *     Holds the counter parameters to be written into FP_POLICY_TABLE
 *     (Firebolt) or FP_INTERNAL (Easyrider).
 */
typedef struct _field_counter_s {
    int                    index;
    uint8                  entries;    /* Number of entries using counter */
    _field_counter32_collect_t _field_x32_counters;
} _field_counter_t;

/*
 * Typedef:
 *     _field_slice_t
 * Purpose:
 *     This has the fields specific to a hardware slice.
 * Notes:
 */
typedef struct _field_slice_s {
    struct _field_group_s  *group;        /* ref to group that owns slice  */
    uint8                  slice_numb;    /* Hardware slice number         */
    uint8                  inst_flg;      /* Installed Flag                */
    bcm_field_qset_t       qset;          /* Q-set supported by this slice */
    _field_sel_t           sel_codes;     /* selects meaning of FPF[0-4]   */
    _qual_info_t           *qual_list;    /* offset & width for qualifiers */
    _field_stage_id_t      stage_id;      /* Pipeline stage slice belongs.  */
} _field_slice_t;


/*
 * Typedef:
 *     _field_group_t
 * Purpose:
 *     This is the logical group's internal storage structure. It has 1, 2 or
 *     3 slices, each with physical entry structures.
 * Notes:
 *   'ent_qset' should always be a subset of 'qset'.
 */
typedef struct _field_group_s {
    int                    unit;        /* bcm unit ID                        */
    bcm_field_group_t      gid;         /* opaque handle                      */
    bcm_field_qset_t       qset;        /* This group's Qualifier Set         */
    bcm_field_group_mode_t mode;        /* wide-mode choice                   */
    _field_slice_t         *slices;     /* Pointer into control's slice array */
    _field_stage_id_t      stage_id;       /* FP pipeline stage id.         */
    /*
     * Public data for each group: The number of used and available entries,
     * counters, and meters for a field group.
     */
    bcm_field_group_status_t group_status;
    bcm_field_aset_t         aset;
    struct _field_group_s  *next;         /* For storing in a linked-list  */
} _field_group_t;

/*
 * Typedef:
 *     _field_action_t
 * Purpose:
 *     This is the real action storage structure that is hidden behind
 *     the opaque handle bcm_field_action_t.
 */
typedef struct _field_action_s {
    bcm_field_action_t     action;       /* action type               */
    uint32                 param0;       /* Action specific parameter */
    uint32                 param1;       /* Action specific parameter */
    uint8                  inst_flg;     /* Installed Flag            */
    struct _field_action_s *next;
} _field_action_t;

/*
 * Typedef:
 *     _field_meter_t
 * Purpose:
 *
 */
typedef struct _field_meter_s {
    uint32              peak_kbits_sec;     /* Peak meter rate in Kb/sec     */
    uint32              peak_kbits_burst;   /* Peak burst size in Kbits      */
    uint32              commit_kbits_sec;   /* Committed meter rate Kb/sec   */
    uint32              commit_kbits_burst; /* Committed burst size in Kbits */
    uint32              index;              /* ID of meter pair (FB:0-63)    */
    uint8               entries;            /* Number of entries using meter */
    uint8               inst_flg;           /* Installed Flag                */
} _field_meter_t;


/*
 * Typedef:
 *     _field_range_t
 * Purpose:
 *     Internal management of Range Checkers. There are two styles or range
 *     checkers, the Firebolt style that only chooses source or destination
 *     port, without any sense of TCP vs. UDP or inverting. This style writes
 *     into the FP_RANGE_CHECK table. The Easyrider style is able to specify
 *     TCP vs. UDP.
 *     The choice of styles is based on the user supplied flags.
 *     If a Firebolt style checker is sufficient, that will be used. If an
 *     Easyrider style checker is required then that will be used.
 *
 */
typedef struct _field_range_s {
    uint32                 flags;
    bcm_field_range_t      rid;
    bcm_l4_port_t          min, max;
    int                    hw_index;
    uint8                  style;        /* Simple (FB) or more complex (ER) */
    struct _field_range_s *next;
} _field_range_t;

/*
 * Typedef:
 *     _field_entry_t
 * Purpose:
 *     This is the physical entry structure, hidden behind the logical
 *     bcm_field_entry_t handle.
 * Notes:
 *     Entries are stored in linked list in the under a slice's _field_slice_t
 *     structure.
 *
 *     Each entry can use 0 or 1 counters. Multiple entries may use the
 *     same counter. The only requirement is that the counter be within
 *     the same slice as the entry.
 *
 *     Similarly each entry can use 0 or 1 meter. Multiple entries may use
 *     the same meter. The only requirement is that the meter be within
 *     the same slice as the entry.
 */

typedef struct _field_entry_s {
    int                    unit;       /* BCM unit ID                        */
    bcm_field_entry_t      eid;        /* BCM unit unique entry identifier   */
    bcm_field_group_t      gid;      /* BCM group ideentifier */
    int                    prio;       /* Entry priority                     */
    uint16                 slice_idx;  /* TCAM index w/in slice (0-127 FB)   */
    uint16              chain_idx;
    int               slice_id;
    uint8                  ext;        /* 0 = internal, 1 = external         */
    uint8                  color_indep;/* L3 switch actions color independent*/
    _field_tcam_t          tcam;       /* Fields to be written into FP_TCAM  */
    _field_action_t        *actions;   /* linked list of actions for entry   */
    _field_slice_t         *fs;        /* Slice where entry lives            */
    _field_group_t         *group;     /* Group where entry lives            */
    uint16                  flags;
    void                    *drv_entry;
    _field_entry_stat_t    statistic;  /* Statistics collection entity.      */
    _field_entry_policer_t policer[_FP_POLICER_LEVEL_COUNT];
    struct _field_entry_s  *ent_copy;
    struct _field_entry_s  *next;
} _field_entry_t;


/* Software entry differs from one in hardware. */
#define _FP_ENTRY_DIRTY                      (1 << 0)
#define _FP_ENTRY_INSTALL                   (1 << 1)
#define _FP_ENTRY_WITH_CHAIN            (1 << 2)
#define _FP_ENTRY_CHAIN_SLICE           (1 << 3)  /* chain slice (slice 3) only*/

/* Field entry enabled in hw. */
#define _FP_ENTRY_ENABLED                    (1 << 4)



#define _BCM_FP_MAIN_ENTRY 0    /* slice 0 */
#define _BCM_FP_CHAIN_ENTRY 1   /* slice 3 */

#define BCM_ROBO_FIELD_TCAM_THREAD_INTERVAL     60000000 /* 60 sec */
#define BCM_ROBO_FIELD_TCAM_THREAD_CHUNK_SIZE     16 
#define BCM_ROBO_FIELD_TCAM_THREAD_PRI      101

/* 
  * Internal used flow id define for bcmFieldActionNewClassId param1.
  * When param1=_bcmFieldActionPrivateUsed, indicate this flow id is used 
  * for other BCM APIs (ex, vlan translation )
  */
#define _bcmFieldActionPrivateUsed    _DRV_FP_ACTION_PRIVATE 

/*
 * Prototypes of Field Processor utility funtions
 */
extern int _robo_field_control_get(int unit, _field_control_t **fc);
extern int _robo_field_group_get(int unit, bcm_field_group_t gid, _field_group_t **group_p);
extern int _robo_field_entry_get(int unit, bcm_field_entry_t entry,_field_entry_t **f_ent,
                                       int slice);
extern int _robo_field_filter_enable_set(int unit, uint32 state);
extern int  _robo_field_qual_info_create(bcm_field_qualify_t qid, soc_robo_mem_t mem,
                                    soc_robo_field_t fpf, int offset, int width,
                                    _qual_info_t **fq_p);
extern int  _robo_field_qual_list_copy(_qual_info_t **fq_dst,
                                  const _qual_info_t *fq_src);
extern void _robo_field_qual_list_destroy(_qual_info_t **f_qual);
extern int _robo_field_qual_value_set(int unit, bcm_field_entry_t entry, bcm_field_qualify_t qi,
                                 uint32 *p_data, uint32 *p_mask);
extern int _robo_field_qual_value32_set(int unit, bcm_field_entry_t entry, bcm_field_qualify_t qi,
                                   uint32 data, uint32 mask);
extern int _robo_field_qual_value_get(int unit, bcm_field_entry_t entry, bcm_field_qualify_t qual, 
                      uint32 *p_data, uint32 *p_mask);
extern int _robo_field_qual_value32_get(int unit, bcm_field_entry_t entry, bcm_field_qualify_t qual, 
                      uint32 *p_data, uint32 *p_mask);
extern int _robo_field_qual_value16_get(int unit, bcm_field_entry_t entry, bcm_field_qualify_t qual, 
                      uint16 *p_data, uint16 *p_mask);
extern int _robo_field_qual_value8_get(int unit, bcm_field_entry_t entry, bcm_field_qualify_t qual, 
                      uint8 *p_data, uint8 *p_mask);

extern int _robo_field_qset_union(const bcm_field_qset_t *qset1,
                             const bcm_field_qset_t *qset2,
                             bcm_field_qset_t *qset_union);
extern int _robo_field_in_odd_slice(_field_entry_t *f_ent);
extern bcm_field_qualify_t _robo_field_qual_next(bcm_field_qset_t qset,
                                            bcm_field_qualify_t qual);
extern bcm_field_qualify_t _robo_field_qual_first(bcm_field_qset_t qset);

extern int _robo_field_qset_is_subset(const bcm_field_qset_t *qset1,
                                 const bcm_field_qset_t *qset2);
extern bcm_field_qset_t _robo_field_qset_diff(const bcm_field_qset_t qset_1,
                                         const bcm_field_qset_t qset_2);
extern int _robo_field_qset_is_empty(bcm_field_qset_t qset);

extern int _robo_field_entry_tcam_idx_get(_field_stage_t *stage_fc, _field_entry_t *f_ent,
                    int *tcam_idx, int *tcam_chain_idx);

extern int _robo_field_thread_stop(int unit);

extern int _bcm_robo_field_policer_create(int unit, bcm_policer_config_t *pol_cfg, 
                      uint32 flags, bcm_policer_t *pid);
extern int _bcm_robo_field_policer_destroy(int unit, bcm_policer_t policer_id);
extern int _bcm_robo_field_policer_destroy_all(int unit);
extern int _bcm_robo_field_policer_get(int unit, bcm_policer_t policer_id, 
                    bcm_policer_config_t *pol_cfg);
extern int _bcm_robo_field_policer_set(int unit, bcm_policer_t policer_id, 
                    bcm_policer_config_t *pol_cfg);
extern int _bcm_robo_field_policer_traverse(int unit, bcm_policer_traverse_cb cb, 
                         void *user_data);

#ifdef BROADCOM_DEBUG
extern void _robo_field_selcode_dump(int unit, char *prefix, _field_sel_t sel_codes,
                                char *suffix) ;
extern void _robo_field_qset_debug(bcm_field_qset_t qset);
extern void _robo_field_qset_dump(char *prefix, bcm_field_qset_t qset, char* suffix);
extern void _robo_field_qual_info_dump(const _qual_info_t *qinf);
extern void _robo_field_qual_info_tbl_dump(uint32 size, _qual_info_t *qi_tbl[],
                                      char *tbl_name);
#endif /* DEBUG */

#endif   /* BCM_FIELD_SUPPORT */
#endif  /* !_BCM_INT_FIELD_H */
