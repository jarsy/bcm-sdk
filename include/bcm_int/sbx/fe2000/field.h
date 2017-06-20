/*
 * $Id: field.h,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: Field Processor APIs
 *
 * Purpose:
 *     'Field Processor' (FP) API for BCM88200 (SBX FE-2000 + Guadalupe-2000)
 */
#ifndef _SBX_FE2K_FIELD_H_
#define _SBX_FE2K_FIELD_H_

/*
 *  WARNING: There should be no reason to directly include this file by any
 *  other than the FE2000 field modules.
 */
#ifndef _SBX_FE2K_FIELD_H_NEEDED_
#warning "this field.h should not generally be included carelessly"
#endif /* ndef _SBX_FE2K_FIELD_H_NEEDED_ */

/******************************************************************************
 *
 *  Configuration
 */

/*
 *  _SBX_FE2000_FIELD_INDEX_COMPACT SHOULD be TRUE if you want to reduce the
 *  size of the internal field API related index values to 16 bits. This will
 *  cause a significant reduction in the overall table sizes, but will limit
 *  the total group, entry, and range counts to 65535 (0..65534, with 65535
 *  reserved to indicate end of list).  It MUST be FALSE if you need any of
 *  these index values to be larger.
 *
 *  _SBX_FE2K_FIELD_EXCESS_VERBOSITY, when TRUE, turns on what amounts to a
 *  play-by-play account of what the field APIs are doing, emitted through the
 *  debugging output interface at the VVERBOSE level.  When FALSE, rather less
 *  comes out at VVERBOSE; other message levels are unaffected.  Some of these
 *  message can be sufficiently prolific to be obnoxious or impair performance.
 *
 *  _SBX_FE2K_FIELD_DIAGNOSTIC_MODE, when TRUE, displays 'unimportant' elements
 *  when dumping field system and individual rule data.  Normally (FALSE) these
 *  elements would be suppressed (such as reserved bits).  Support for this
 *  feature is a little uneven; don't count on all such fields being there.
 *
 *  _SBX_FE2K_FIELD_DUPLICATE_ACTION_ERR specifies the result error for when
 *  trying to add an action to a rule that already has the type of action being
 *  added. The BCM API doc claims this should be BCM_E_EXISTS, but the
 *  regression tests demand that it be BCM_E_CONFIG.
 *
 *  _SBX_FE2K_FIELD_OVERLAP_ACTION_ERR specifies the result error for when
 *  trying to add an action to a rule that already has an action that overlaps
 *  the action being added (such as doing different things with a union).  I
 *  don't think this ever happens in the XGS world, so I guessed BCM_E_CONFIG.
 *
 *  _SBX_FE2K_FIELD_INVALID_ID_ERR specifies the result error for when trying
 *  to access a resource using a clearly invalid (out of range) ID.  The BCM
 *  API seems to indicate this should be BCM_E_PARAM, but the regression tests
 *  demand that it be BCM_E_NOT_FOUND.
 *
 *  _SBX_FE2K_FIELD_INVALID_QSET specifies the result error for when trying to
 *  build groups that include unsupported features in their QSets.  The BCM API
 *  doc seems to imply that it should be BCM_E_PARAM, but the regression tests
 *  don't accept anything other than BCM_E_UNAVAIL in this case.
 *
 *  _SBX_FE2K_FIELD_INVALID_ACTION specifies the result error for when trying
 *  to manipulate actions on entries for which they are unsupported.  The BCM
 *  API doc seems to imply that it should be BCM_E_PARAM, but the regression
 *  tests don't accept anything other than BCM_E_UNAVAIL in this case.
 *
 *  _SBX_FE2K_FIELD_UNKNONWN_MICROCODE specifies the result error for any
 *  action attempted against a unit whose microcode is unknown, either because
 *  there is no support for it in the source or because support for it has been
 *  disabled at compile time.
 *
 *  _SBX_FE2K_FIELD_NOT_ON_MICROCODE_ERR specifies the result error for any
 *  action attempted against a unit whose microcode is supported but for whose
 *  microcode the specific action is not supported.
 *
 *  _SBX_FE2K_FIELD_PAGE_WIDTH specifies how wide the console should be for
 *  certain debugging functions that tend to generate lists (so they can be
 *  formatted to make better use of the available console space).  Don't make
 *  it more than 16384; use zero if you want no line joining (each element will
 *  be on its own line).
 */
#define _SBX_FE2K_FIELD_INDEX_COMPACT           TRUE
#define _SBX_FE2K_FIELD_EXCESS_VERBOSITY        TRUE
#define _SBX_FE2K_FIELD_DIAGNOSTIC_MODE         TRUE
#define _SBX_FE2K_FIELD_DUPLICATE_ACTION_ERR    BCM_E_CONFIG
#define _SBX_FE2K_FIELD_OVERLAP_ACTION_ERR      BCM_E_CONFIG
#define _SBX_FE2K_FIELD_INVALID_ID_ERR          BCM_E_NOT_FOUND
#define _SBX_FE2K_FIELD_INVALID_QSET_ERR        BCM_E_UNAVAIL
#define _SBX_FE2K_FIELD_INVALID_ACTION_ERR      BCM_E_UNAVAIL
#define _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR   BCM_E_UNIT
#define _SBX_FE2K_FIELD_NOT_ON_MICROCODE_ERR    BCM_E_UNAVAIL
#define _SBX_FE2K_FIELD_PAGE_WIDTH              79


/******************************************************************************
 *
 *  Module-wide definitions
 */


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

#define FIELD_MACA_FORMAT "%02X:%02X:%02X:%02X:%02X:%02X"
#define FIELD_MACA_SHOW(maca) \
                             (maca)[0], \
                             (maca)[1], \
                             (maca)[2], \
                             (maca)[3], \
                             (maca)[4], \
                             (maca)[5]

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
#if _SBX_FE2K_FIELD_INDEX_COMPACT
typedef int16               _bcm_fe2k_field_group_index;
typedef int16               _bcm_fe2k_field_entry_index;
typedef int16               _bcm_fe2k_field_range_index;
typedef int16               _bcm_fe2k_field_ptype_index;
typedef uint16              _bcm_fe2k_field_count;
#define _SBX_FE2K_FIELD_INDEX_FORMAT "%04X"
#define _SBX_FE2K_FIELD_INDEX_WIDTH 4
#else /* _SBX_FE2K_FIELD_INDEX_COMPACT */
typedef bcm_field_group_t   _bcm_fe2k_field_group_index;
typedef bcm_field_entry_t   _bcm_fe2k_field_entry_index;
typedef bcm_field_range_t   _bcm_fe2k_field_range_index;
typedef int                 _bcm_fe2k_field_ptype_index;
typedef unsigned int        _bcm_fe2k_field_count;
#define _SBX_FE2K_FIELD_INDEX_FORMAT "%08X"
#define _SBX_FE2K_FIELD_INDEX_WIDTH 8
#endif /* _SBX_FE2K_FIELD_INDEX_COMPACT */

#define _SBX_FE2K_FIELD_ENTRY_PRIO_HIGHEST 0x3FFF
#define _SBX_FE2K_FIELD_GROUP_PRIO_ANY     -0x3FFF

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
typedef struct _bcm_fe2k_field_range_s {
    /* fields mandated by the API */
    uint32 flags;                           /* indicate comparison mode */
    bcm_l4_port_t min;                      /* lowest port number to match */
    bcm_l4_port_t max;                      /* highest port number to match */
    /* fields needed for accounting */
} _bcm_fe2k_field_range_t;


/******************************************************************************
 *
 *  Exports to submodules (see main FE2000 field module for code comments)
 */

#ifdef BROADCOM_DEBUG
extern const char *_sbx_fe2000_field_qual_name[bcmFieldQualifyCount];
extern const char *_sbx_fe2000_field_action_name[bcmFieldActionCount];
extern char *_bcm_fe2000_field_mac_expand(char *result,
                                          unsigned int length,
                                          uint64 macAddr);
extern int _bcm_fe2000_field_qset_dump(const bcm_field_qset_t qset,
                                       const char *prefix);
#endif /* def BROADCOM_DEBUG */

extern signed int _bcm_fe2000_compare_entry_priority(int pri1, int pri2);
int _bcm_fe2000_qset_subset(bcm_field_qset_t qset1, bcm_field_qset_t qset2);
extern void _bcm_fe2000_bcm_debug(const int flags, const char *message);


/******************************************************************************
 *
 *  Exports from submodules (see submodules for code comments)
 */

#ifdef BCM_FE2000_P3_SUPPORT
/* G2P3 */
extern int bcm_fe2000_g2p3_field_wb_state_sync(void *unitData, int sync);
extern int bcm_fe2000_g2p3_field_init(int unit, void **unitData);
extern int bcm_fe2000_g2p3_field_detach(void *unitData);
extern int bcm_fe2000_g2p3_field_group_port_create_mode_id(void *unitData,
                                                           bcm_port_t port,
                                                           bcm_field_qset_t qset,
                                                           int pri,
                                                           bcm_field_group_mode_t mode,
                                                           bcm_field_group_t group);
extern int bcm_fe2000_g2p3_field_group_port_create_mode(void *unitData,
                                                        bcm_port_t port,
                                                        bcm_field_qset_t qset,
                                                        int pri,
                                                        bcm_field_group_mode_t mode,
                                                        bcm_field_group_t *group);
extern int bcm_fe2000_g2p3_field_group_create_id(void *unitData,
                                                 bcm_field_qset_t qset,
                                                 int pri,
                                                 bcm_field_group_t group);
extern int bcm_fe2000_g2p3_field_group_create(void *unitData,
                                              bcm_field_qset_t qset,
                                              int pri,
                                              bcm_field_group_t *group);
extern int bcm_fe2000_g2p3_field_group_get(void *unitData,
                                           bcm_field_group_t group,
                                           bcm_field_qset_t *qset);
extern int bcm_fe2000_g2p3_field_group_destroy(void *unitData,
                                               bcm_field_group_t group);
extern int bcm_fe2000_g2p3_field_group_install(void *unitData,
                                               bcm_field_group_t group);
extern int bcm_fe2000_g2p3_field_group_remove(void *unitData,
                                              bcm_field_group_t group);

extern int bcm_fe2000_g2p3_field_group_flush(void *unitData,
                                             bcm_field_group_t group);
extern int bcm_fe2000_g2p3_field_group_set(void *unitData,
                                           bcm_field_group_t group,
                                           bcm_field_qset_t qset);
extern int bcm_fe2000_g2p3_field_group_status_get(void *unitData,
                                                  bcm_field_group_t group,
                                                  bcm_field_group_status_t *status);
extern int bcm_fe2000_g2p3_field_range_create(void *unitData,
                                              bcm_field_range_t *range,
                                              uint32 flags,
                                              bcm_l4_port_t min,
                                              bcm_l4_port_t max);
extern int bcm_fe2000_g2p3_field_range_create_id(void *unitData,
                                                 bcm_field_range_t range,
                                                 uint32 flags,
                                                 bcm_l4_port_t min,
                                                 bcm_l4_port_t max);
extern int bcm_fe2000_g2p3_field_range_get(void *unitData,
                                           bcm_field_range_t range,
                                           uint32 *flags,
                                           bcm_l4_port_t *min,
                                           bcm_l4_port_t *max);
extern int bcm_fe2000_g2p3_field_range_destroy(void *unitData,
                                               bcm_field_range_t range);
extern int bcm_fe2000_g2p3_field_entry_create(void *unitData,
                                              bcm_field_group_t group,
                                              bcm_field_entry_t *entry);
extern int bcm_fe2000_g2p3_field_entry_create_id(void *unitData,
                                                 bcm_field_group_t group,
                                                 bcm_field_entry_t entry);
extern int bcm_fe2000_g2p3_field_entry_destroy(void *unitData,
                                               bcm_field_entry_t entry);
extern int bcm_fe2000_g2p3_field_entry_destroy_all(void *unitData);
extern int bcm_fe2000_g2p3_field_entry_copy(void *unitData,
                                            bcm_field_entry_t src_entry,
                                            bcm_field_entry_t *dst_entry);
extern int bcm_fe2000_g2p3_field_entry_copy_id(void *unitData,
                                               bcm_field_entry_t src_entry,
                                               bcm_field_entry_t dst_entry);
extern int bcm_fe2000_g2p3_field_entry_install(void *unitData,
                                               bcm_field_entry_t entry);
extern int bcm_fe2000_g2p3_field_entry_reinstall(void *unitData,
                                                 bcm_field_entry_t entry);
extern int bcm_fe2000_g2p3_field_entry_remove(void *unitData,
                                              bcm_field_entry_t entry);
extern int bcm_fe2000_g2p3_field_entry_prio_get(void *unitData,
                                                bcm_field_entry_t entry,
                                                int *prio);
extern int bcm_fe2000_g2p3_field_entry_prio_set(void *unitData,
                                                bcm_field_entry_t entry,
                                                int prio);
extern int bcm_fe2000_g2p3_field_qualify_clear(void *unitData,
                                               bcm_field_entry_t entry);

extern int bcm_fe2000_g2p3_field_qualify_InPort(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_port_t data,
                                                bcm_port_t mask);
extern int bcm_fe2000_g2p3_field_qualify_InPort_get(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_port_t *data,
                                                bcm_port_t *mask);
extern int bcm_fe2000_g2p3_field_qualify_OutPort(void *unitData,
                                                 bcm_field_entry_t entry,
                                                 bcm_port_t data,
                                                 bcm_port_t mask);
extern int bcm_fe2000_g2p3_field_qualify_OutPort_get(void *unitData,
                                                     bcm_field_entry_t entry,
                                                     bcm_port_t *data,
                                                     bcm_port_t *mask);
extern int bcm_fe2000_g2p3_field_qualify_InPorts_get(void *unitData,
                                                     bcm_field_entry_t entry,
                                                     bcm_pbmp_t *data,
                                                     bcm_pbmp_t *mask);
extern int bcm_fe2000_g2p3_field_qualify_OutPorts_get(void *unitData,
                                                      bcm_field_entry_t entry,
                                                      bcm_pbmp_t *data,
                                                      bcm_pbmp_t *mask);
extern int bcm_fe2000_g2p3_field_qualify_OuterVlan(void *unitData,
                                                   bcm_field_entry_t entry,
                                                   bcm_vlan_t data,
                                                   bcm_vlan_t mask);
extern int bcm_fe2000_g2p3_field_qualify_OuterVlan_get(void *unitData,
                                                   bcm_field_entry_t entry,
                                                   bcm_vlan_t *data,
                                                   bcm_vlan_t *mask);
extern int bcm_fe2000_g2p3_field_qualify_OuterVlanId(void *unitData,
                                                     bcm_field_entry_t entry,
                                                     bcm_vlan_t data,
                                                     bcm_vlan_t mask);
extern int bcm_fe2000_g2p3_field_qualify_OuterVlanId_get(void *unitData,
                                                     bcm_field_entry_t entry,
                                                     bcm_vlan_t *data,
                                                     bcm_vlan_t *mask);
extern int bcm_fe2000_g2p3_field_qualify_OuterVlanPri(void *unitData,
                                                      bcm_field_entry_t entry,
                                                      uint8 data,
                                                      uint8 mask);
extern int bcm_fe2000_g2p3_field_qualify_OuterVlanPri_get(void *unitData,
                                                      bcm_field_entry_t entry,
                                                      uint8 *data,
                                                      uint8 *mask);
extern int bcm_fe2000_g2p3_field_qualify_EtherType(void *unitData,
                                                   bcm_field_entry_t entry,
                                                   uint16 data,
                                                   uint16 mask);
extern int bcm_fe2000_g2p3_field_qualify_EtherType_get(void *unitData,
                                                   bcm_field_entry_t entry,
                                                   uint16 *data,
                                                   uint16 *mask);
extern int bcm_fe2000_g2p3_field_qualify_IpProtocol(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    uint8 data,
                                                    uint8 mask);
extern int bcm_fe2000_g2p3_field_qualify_IpProtocol_get(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    uint8 *data,
                                                    uint8 *mask);
extern int bcm_fe2000_g2p3_field_qualify_SrcIp(void *unitData,
                                               bcm_field_entry_t entry,
                                               bcm_ip_t data,
                                               bcm_ip_t mask);
extern int bcm_fe2000_g2p3_field_qualify_SrcIp_get(void *unitData,
                                               bcm_field_entry_t entry,
                                               bcm_ip_t *data,
                                               bcm_ip_t *mask);
extern int bcm_fe2000_g2p3_field_qualify_DstIp(void *unitData,
                                               bcm_field_entry_t entry,
                                               bcm_ip_t data,
                                               bcm_ip_t mask);
extern int bcm_fe2000_g2p3_field_qualify_DstIp_get(void *unitData,
                                               bcm_field_entry_t entry,
                                               bcm_ip_t *data,
                                               bcm_ip_t *mask);
extern int bcm_fe2000_g2p3_field_qualify_SrcIp6(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_ip6_t data,
                                                bcm_ip6_t mask);
extern int bcm_fe2000_g2p3_field_qualify_DstIp6(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_ip6_t data,
                                                bcm_ip6_t mask);
extern int bcm_fe2000_g2p3_field_qualify_DSCP(void *unitData,
                                              bcm_field_entry_t entry,
                                              uint8 data,
                                              uint8 mask);
extern int bcm_fe2000_g2p3_field_qualify_DSCP_get(void *unitData,
                                              bcm_field_entry_t entry,
                                              uint8 *data,
                                              uint8 *mask);
extern int bcm_fe2000_g2p3_field_qualify_TcpControl(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    uint8 data,
                                                    uint8 mask);
extern int bcm_fe2000_g2p3_field_qualify_TcpControl_get(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    uint8 *data,
                                                    uint8 *mask);
extern int bcm_fe2000_g2p3_field_qualify_RangeCheck(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    bcm_field_range_t range,
                                                    int invert);
extern int bcm_fe2000_g2p3_field_qualify_RangeCheck_get(void *unitData,
                                                    bcm_field_entry_t entry,
                                                    int max_count,
                                                    bcm_field_range_t *range,
                                                    int *invert,
                                                    int *count);
extern int bcm_fe2000_g2p3_field_qualify_SrcMac(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_mac_t data,
                                                bcm_mac_t mask);
extern int bcm_fe2000_g2p3_field_qualify_SrcMac_get(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_mac_t *data,
                                                bcm_mac_t *mask);
extern int bcm_fe2000_g2p3_field_qualify_DstMac(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_mac_t data,
                                                bcm_mac_t mask);
extern int bcm_fe2000_g2p3_field_qualify_DstMac_get(void *unitData,
                                                bcm_field_entry_t entry,
                                                bcm_mac_t *data,
                                                bcm_mac_t *mask);

extern int bcm_fe2000_g2p3_field_action_add(void *unitData,
                                            bcm_field_entry_t entry,
                                            bcm_field_action_t action,
                                            uint32 param0,
                                            uint32 param1);
extern int bcm_fe2000_g2p3_field_action_get(void *unitData,
                                            bcm_field_entry_t entry,
                                            bcm_field_action_t action,
                                            uint32 *param0,
                                            uint32 *param1);
extern int bcm_fe2000_g2p3_field_action_remove(void *unitData,
                                               bcm_field_entry_t entry,
                                               bcm_field_action_t action);
extern int bcm_fe2000_g2p3_field_action_remove_all(void *unitData,
                                                   bcm_field_entry_t entry);
extern int bcm_fe2000_g2p3_field_counter_create(void *unitData,
                                                bcm_field_entry_t entry);
extern int bcm_fe2000_g2p3_field_counter_share(void *unitData,
                                               bcm_field_entry_t src_entry,
                                               bcm_field_entry_t dst_entry);
extern int bcm_fe2000_g2p3_field_counter_destroy(void *unitData,
                                                 bcm_field_entry_t entry);
extern int bcm_fe2000_g2p3_field_counter_set(void *unitData,
                                             bcm_field_entry_t entry,
                                             int counter_num,
                                             uint64 val);
extern int bcm_fe2000_g2p3_field_counter_get(void *unitData,
                                             bcm_field_entry_t entry,
                                             int counter_num,
                                             uint64 *val);
extern int bcm_fe2000_g2p3_field_entry_policer_attach(void *unitData,
                                                      bcm_field_entry_t entry_id,
                                                      int level,
                                                      bcm_policer_t policer_id);
extern int bcm_fe2000_g2p3_field_entry_policer_detach(void *unitData,
                                                      bcm_field_entry_t entry_id,
                                                      int level);
extern int bcm_fe2000_g2p3_field_entry_policer_detach_all(void *unitData,
                                                          bcm_field_entry_t entry_id);
extern int bcm_fe2000_g2p3_field_entry_policer_get(void *unitData,
                                                   bcm_field_entry_t entry_id,
                                                   int level,
                                                   bcm_policer_t *policer_id);

extern int bcm_fe2000_g2p3_field_show(void *unitData, const char *pfx);
extern int bcm_fe2000_g2p3_field_entry_dump(void *unitData,
                                            bcm_field_entry_t entry);
extern int bcm_fe2000_g2p3_field_group_dump(void *unitData,
                                            bcm_field_group_t group);
extern int bcm_fe2000_g2p3_field_qualify_InPorts(void *unitData,
                                                 bcm_field_entry_t entry,
                                                 bcm_pbmp_t data,
                                                 bcm_pbmp_t mask);
extern int bcm_fe2000_g2p3_field_qualify_OutPorts(void *unitData,
                                                  bcm_field_entry_t entry,
                                                  bcm_pbmp_t data,
                                                  bcm_pbmp_t mask);
#endif /* def BCM_FE2000_P3_SUPPORT */

#endif /* ndef _SBX_FE2K_FIELD_H_ */

