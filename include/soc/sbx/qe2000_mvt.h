/*
 * $Id: qe2000_mvt.h,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _QE2000_MVT_H_
#define _QE2000_MVT_H_

#include <sal/types.h>
#include <soc/types.h>
#include <soc/error.h>

/*
 *  MVT ID type
 *
 *  This is an integer based ID number, but NULL is all bits SET.
 *
 *  Note that a VALID macro is not provided because the range may
 *  not be established by compile time (it is based upon how much
 *  memory of certain type is installed on the QE and how that is
 *  configured).
 */
typedef uint16 sbx_mvt_id_t;
#define SBX_MVT_ID_NULL 0xFFFF
#define SBX_MVT_ID_IS_NULL(x) (SBX_MVT_ID_NULL == (x))
#define SBX_MVT_ID_NOT_NULL(x) (SBX_MVT_ID_NULL != (x))

/*
 *  The egress_data_b field of the MVT entry below has four bits, all of which
 *  are passed down the egress path, but one of which is also used to select
 *  the timeout that is used when sending frames.
 */
#define SBX_MVT_EGRESS_B_TIMER_0 0x0
#define SBX_MVT_EGRESS_B_TIMER_1 0x8

/*
 * MVT Index range
 * XXX: Derive from EPlIb defines
 */
#define SBX_MVT_ID_VSI_BASE            0
#define SBX_MVT_ID_VSI_END             4095
#define SBX_MVT_ID_DYNAMIC_BASE        (SBX_MVT_ID_VSI_END + 1)
#define SBX_MVT_ID_DYNAMIC_END         ((48 * 1024) - 1)
#define SBX_MVT_ID_24K_DYNAMIC_END     ((24 * 1024) - 1)
#define SBX_MVT_ID_12K_DYNAMIC_END     ((12 * 1024) - 1)

#define SBX_MVT_ID_LOCAL_BASE          (30 * 1024)
#define SBX_MVT_ID_24K_LOCAL_BASE      (16 * 1024)
#define SBX_MVT_ID_12K_LOCAL_BASE      (8 * 1024)
#define SBX_MVT_ID_BD_SIZE             (0) /* To be Removed, or could be set to 0 */


/*
 * MVT Space
 */
#define SBX_MVT_ID_VLAN                0
#define SBX_MVT_ID_GLOBAL              1
#define SBX_MVT_ID_LOCAL               2

/*
 * MVT Internal Format
 */
#define SBX_MVT_FORMAT0                0
#define SBX_MVT_FORMAT1                1

/*
 * MVT Internal Type
 */
#define SBX_MVT_TYPE_VID               0x10
#define SBX_MVBT_TYPE_OHI              0x20



#define SBX_MAX_MVT_CHAINED            (48 * 1024) /* worst case value. Assures no s/w constraint */

/*
 *  Contents of a single MVT entry, as applicable to the QE - global view
 *  of all FEs present.
 *  FEs may use the SOC_SBX_NODE_PORT_GET macro to find the QE fabric
 *  port for a given module,port pair.
 *
 *  egress_data_b also maps the MC transmit timer in its bit 3 (0 is LSb).
 */
typedef struct sbx_qe2000_mvt_entry_s {
    soc_pbmp_t     ports;                  /* All 50 ports used/avail */
    uint16         egress_data_a;          /* only lower 14 bits used */
    uint8          egress_data_b;          /* only lower 4 bits used */
    sbx_mvt_id_t   next;                   /* next entry when chaining */
    uint8          source_knockout;        /* enable source knockout */
} sbx_qe2000_mvt_entry_t;

/**
 * XXX: Eplib changes then this should. Not a good
 * dependency.
 *
 * SB_ZF_G2_EPLIB_MVTENTRY_SET_VLAN(_VLAN_, pZf) (pZf)->ulMvtdA = ((_VLAN_) & 0xfff);  (pZf)->ulMvtdB &= 0x1
 * SB_ZF_G2_EPLIB_MVTENTRY_GET_VLAN(_VLAN_, pZf) _VLAN_ = ((pZf)->ulMvtdA) & 0xfff);
 * SB_ZF_G2_EPLIB_MVTENTRY_SET_OIX(_IDX_, pZf)  (pZf)->ulMvtdA = ((_IDX_) & 0xfff); (pZf)->ulMvtdA |= ((_IDX_) & 0x18000) >> 3; (pZf)->ulMvtdB =((_IDX_) >> 12) & 0x7
 * SB_ZF_G2_EPLIB_MVTENTRY_GET_OIX(_IDX_, pZf)  _IDX_ = ((pZf)->ulMvtdA & 0xfff); _IDX_ |= ((pZf)->ulMvtdA << 3) & 0x18000; _IDX_ |= ((pZf)->ulMvtdB & 0x7) <<  12
 * SB_ZF_G2_EPLIB_MVTENTRY_SET_IPMC(_IPMC_, pZf)  ( (pZf)->ulMvtdB = (_IPMC_) & 0x1 )
 * SB_ZF_G2_EPLIB_MVTENTRY_GET_IPMC(_IPMC_, pZf)  _IPMC_ =  (pZf)->ulMvtdB & 0x1
 *
 */

#define SBX_MVT_SET_LI_OHI(mvt, ohi)                    \
    do {                                                \
        (mvt)->egress_data_a  = (ohi) & 0xfff;          \
        (mvt)->egress_data_a |= ((ohi) & 0x18000) >> 3; \
        (mvt)->egress_data_b  = ((ohi) >> 12) & 0x7 ;   \
    } while(0)

#define SBX_MVT_GET_LI_OHI(mvt, ohi)                         \
        (ohi) = (((mvt)->egress_data_a & 0xfff)          |   \
                 (((mvt)->egress_data_a << 3) & 0x18000) |   \
                 (((mvt)->egress_data_b & 0x7) << 12))

#define SBX_MVT_SET_TB(mvt, vid)                        \
    do {                                                \
        (mvt)->egress_data_a = SBX_VSI_FROM_VID(vid);   \
        (mvt)->egress_data_b = 0x0;                     \
    } while(0)

#define SBX_MVT_GET_VID(mvt, vid)                       \
    (vid) = SBX_VSI_TO_VID((mvt)->egress_data_a)

#define SBX_MVT_IS_LI(mvt)                                      \
    (((mvt)->egress_data_b) || ((mvt)->egress_data_a >= 8192))

#define SBX_MVT_SET_NOT_USED(mvt)                       \
    do {                                                \
        (mvt)->egress_data_a = 0x0;                     \
        (mvt)->egress_data_b = 0x0;                     \
    } while(0)

#define SBX_MVT_INVALID(mvt)                            \
    (((mvt)->egress_data_a == 0) && ((mvt)->egress_data_b == 0))


#define SBX_MVT_GET_EP_DISABLED(mvt, id)                     \
        (id) = (((mvt)->egress_data_a & 0x3FFF)          |   \
                 (((mvt)->egress_data_b & 0xF) << 14))


#define SBX_MVT_SET_EP_DISABLED(mvt, id)                     \
    do {                                                     \
        (mvt)->egress_data_a  = (id) & 0x3FFF;               \
        (mvt)->egress_data_b  = ((id) >> 14) & 0xF;          \
    } while(0)


/*
 *  Allocate num dynamic MVT entries.  Set entryId to SBX_MVT_ID_NULL if an
 *  error is returned so either check will function properly.
 */
extern soc_error_t soc_qe2000_mvt_entry_alloc(const int qeunit,
                                              const int num,
                                              sbx_mvt_id_t pEntryIds[]);

/*
 *  Reserve 1 or more MVT entries in the QE.  When more than one entry is
 *  requested, the block is considered 'chained' and the 'next' pointers are
 *  set accordingly.  If num entries fail to allocate, the function will
 *  de-allocate the previous entries up to the failure.  The application is
 *  responsible for managing freeing chains, and maintaining the chain when
 *  entries are removed from the middle.
 */
extern soc_error_t soc_qe2000_mvt_entry_reserve(const int qeunit,
                                               const int num,
                                               const sbx_mvt_id_t pEntryIds[]);

/*
 *  Free a dynamically allocated MVT entry (return it to the pool of available
 *  dynamic entries).  Must not allow free of statically allocated entries
 *  (should return BCM_E_PARAM in this case).
 */
extern soc_error_t soc_qe2000_mvt_entry_free(const int qeunit,
                                              const sbx_mvt_id_t entryId);

/*
 *  Get the contents of an MVT entry, from the given QE's viewpoint.
 */
extern soc_error_t soc_qe2000_mvt_entry_get(const int qeunit,
                                             const sbx_mvt_id_t entryId,
                                             sbx_qe2000_mvt_entry_t *entry);

/*
 *  Verify whether an entry has been allocated
 */
extern soc_error_t soc_qe2000_mvt_entry_check(const int qeUnit,
                                              const sbx_mvt_id_t entryId);
/*
 *  Set the contents of an MVT entry, from the given QE's viewpoint.
 */
extern soc_error_t soc_qe2000_mvt_entry_set(const int qeunit,
                                             const sbx_mvt_id_t entryId,
                                             const sbx_qe2000_mvt_entry_t *entry);

/*
 * Initialize the mvt tables, and internal data structures
 */
extern soc_error_t soc_qe2000_mvt_init(const int qeUnit);


/* Keep these interfaces for now so that merge won't break customer exisiting code */
#ifndef TO_BE_DEPRECIATED_CODE
#define TO_BE_DEPRECIATED_CODE
#endif

#ifdef TO_BE_DEPRECIATED_CODE
extern soc_error_t soc_qe2000_bd_mvt_init(const int qeUnit);

extern int
_sbx_qe2000_mvt_entry_reserve(const int qeunit,
                              const int num,
                              const sbx_mvt_id_t pEntryIds[]);

extern int _sbx_qe2000_mvt_entry_alloc(const int qeunit,
                                       const int num,
                                       sbx_mvt_id_t entryIds[]);

extern int _sbx_qe2000_mvt_entry_free(const int qeunit,
                                      const sbx_mvt_id_t entryId);

extern int _sbx_qe2000_mvt_entry_get(const int qeunit,
                                     const sbx_mvt_id_t entryId,
                                     sbx_qe2000_mvt_entry_t *entry);

extern int _sbx_qe2000_mvt_entry_set(const int qeunit,
                                     const sbx_mvt_id_t entryId,
                                     const sbx_qe2000_mvt_entry_t *entry);

extern int _sbx_qe2000_mvt_init(const int qeUnit);
#endif


extern soc_error_t
soc_qe2000_mvt_initialization(const int qeUnit);

extern soc_error_t
soc_qe2000_mvt_entry_allocate(const int qeUnit,
                              const int num,
                              sbx_mvt_id_t pEntryIds[],
                              int mvt_id_block);

extern int
soc_qe2000_mvt_entry_reserve_id(const int qeUnit,
                              const int num,
                              const sbx_mvt_id_t pEntryIds[]);

extern soc_error_t
soc_qe2000_mvt_entry_free_frm_id(const int qeUnit,
                           const sbx_mvt_id_t entryId);

extern soc_error_t
soc_qe2000_mvt_entry_chk_frm_id(const int qeUnit,
				const sbx_mvt_id_t entryId);

extern soc_error_t
soc_qe2000_mvt_entry_get_frm_id(const int qeUnit,
                          const sbx_mvt_id_t entryId,
                          sbx_qe2000_mvt_entry_t *pEntry);

extern soc_error_t
soc_qe2000_mvt_entry_set_frm_id(const int qeUnit,
                          const sbx_mvt_id_t entryId,
                          const sbx_qe2000_mvt_entry_t *pEntry);

extern int
soc_qe2000_mvt_entry_id_valid(const int qeUnit,
                          const sbx_mvt_id_t entryId);

extern int
soc_qe2000_mvt_state_get(int unit, char *pbuf);

#endif /* _QE2000_MVT_H_ */


