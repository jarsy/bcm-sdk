/*
 * $Id: vlan.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _BCM_INT_SBX_CALADAN3_VLAN_H_
#define _BCM_INT_SBX_CALADAN3_VLAN_H_

#include <bcm/vlan.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbTypesGlue.h>
#include <bcm_int/common/trunk.h>
#include <soc/sbx/g3p1/g3p1_cmu.h>
#include <soc/sbx/g3p1/g3p1_int.h>



#define VLAN_INV_FTE_EXC          0x7F /* invalid FTE exception code */

#define SBX_LAYER_PV2E_TABLES_ARE_PORT_VID          0
#define SBX_LAYER_PV2APPDATA_TABLES_ARE_PORT_VID    0
#define SBX_LAYER_EPV2E_TABLES_ARE_PORT_VID         0

#if SBX_LAYER_PV2E_TABLES_ARE_PORT_VID
#define SOC_SBX_G3P1_PV2E_GET(_unit, _port, _vid, _data) \
    soc_sbx_g3p1_pv2e_get(_unit, _port, _vid, _data)
#define SOC_SBX_G3P1_PV2E_SET(_unit, _port, _vid, _data) \
    soc_sbx_g3p1_pv2e_set(_unit, _port, _vid, _data)
#else /* SBX_LAYER_PV2E_TABLES_ARE_PORT_VID */
#define SOC_SBX_G3P1_PV2E_GET(_unit, _port, _vid, _data) \
    soc_sbx_g3p1_pv2e_get(_unit, _vid, _port, _data)
#define SOC_SBX_G3P1_PV2E_SET(_unit, _port, _vid, _data) \
    soc_sbx_g3p1_pv2e_set(_unit, _vid, _port, _data)
#endif /* SBX_LAYER_PV2E_TABLES_ARE_PORT_VID */


#if SBX_LAYER_EPV2E_TABLES_ARE_PORT_VID
#define SOC_SBX_G3P1_EPV2E_GET(_unit, _port, _vid, _data) \
    soc_sbx_g3p1_epv2e_get(_unit, _port, _vid, _data)
#define SOC_SBX_G3P1_EPV2E_SET(_unit, _port, _vid, _data) \
    soc_sbx_g3p1_epv2e_set(_unit, _port, _vid, _data)
#else /* SBX_LAYER_EPV2E_TABLES_ARE_PORT_VID */
#define SOC_SBX_G3P1_EPV2E_GET(_unit, _port, _vid, _data) \
    soc_sbx_g3p1_epv2e_get(_unit, _vid, _port, _data)
#define SOC_SBX_G3P1_EPV2E_SET(_unit, _port, _vid, _data) \
    soc_sbx_g3p1_epv2e_set(_unit, _vid, _port, _data)
#endif /* SBX_LAYER_EPV2E_TABLES_ARE_PORT_VID */

extern int _soc_sbx_g3p1_pv2e_set(int unit, int iport, int ivid, soc_sbx_g3p1_pv2e_t *e);

/* Convert a vlan gport id to an FT index */
#define VLAN_VGPORT_ID_TO_FT_INDEX(unit__, gid__) \
          ((gid__) + SBX_LOCAL_GPORT_FTE_BASE(unit__))

/* Convert a local gport to a vlan gport id */
#define VLAN_FT_INDEX_TO_VGPORT_ID(unit__, fti__) \
          ((fti__) - SBX_LOCAL_GPORT_FTE_BASE(unit__))

/* Indicates which fields of the lp needs to be updated. */
typedef enum {
    BCM_CALADAN3_LP_COPY_PID         = 0x0001,
    BCM_CALADAN3_LP_COPY_QOS         = 0x0002,
    BCM_CALADAN3_LP_COPY_MIRROR      = 0x0004,
    BCM_CALADAN3_LP_COPY_POLICER     = 0x0008,
    BCM_CALADAN3_LP_COPY_COUNTER     = 0x0010
} _bcm_caladan3_lp_copy_flags_t;

/*
 *  This describes what parts of the LP have diverged when applicable.
 */
typedef enum _bcm_caladan3_vlan_divergence_e {
    _CALADAN3_VLAN_DIVERGE_QOS     = 0x00000001, /* QoS settings have diverged */
    _CALADAN3_VLAN_DIVERGE_ALL     = 0x0000FFFF  /* everything has diverged */
} _bcm_caladan3_vlan_divergence_t;


/*
 *  This is used when setting native VID to allow other modules to override the
 *  default behaviours.  Normal (tranditional bridging) applications would
 *  leave none of this stuff set.
 *
 *  NVID_OVR_MASK is a mask indicating the bits that are used when replacing or
 *  inserting the tag for native VID.
 *
 *  OVERRIDE_PVV2E is a flag that indicates the caller wants a different PVV2E
 *  for [port,0,0xFFF] than that which would be used in the default rules. If
 *  this bit is clear, nothing else here matters.  All of the other bits are
 *  described based upon this being set (and they have no effect if this is
 *  clear, even if they are set).
 *
 *  USE_PVV2E is a flag that indicates a pvv2e entry is needed.  If clear, the
 *  pvv2e entry will not be created; if set, the pvv2e entry will be created.
 *
 *  SET_REPLACE indicates the value of the pvv2e entry replace flag: set is
 *  TRUE, clear is FALSE.
 *
 *  SET_KEEPORSTRIP indicates the value of the pvv2e entry keeporstrip flag:
 *  set is TRUE; clear is FALSE.
 *
 *  USE_NVID_OVR indicates that the native VID value is to replaced by the
 *  value under NVID_OVR_MASK when set; when clear, the value under
 *  NVID_OVR_MASK is ignored.
 *
 *  All of these actions only apply to g2p3, as g2p2 manages native VID in a
 *  different manner.
 */
typedef enum _bcm_caladan3_nvid_pvv2e_control_flags_e {
    BCM_CALADAN3_NVID_NVID_OVR_MASK   = 0x00000FFF,
    BCM_CALADAN3_NVID_OVERRIDE_PVV2E  = 0x00001000,
    BCM_CALADAN3_NVID_USE_PVV2E       = 0x00002000,
    BCM_CALADAN3_NVID_SET_REPLACE     = 0x00004000,
    BCM_CALADAN3_NVID_SET_KEEPORSTRIP = 0x00008000,
    BCM_CALADAN3_NVID_USE_NVID_OVR    = 0x00010000,
    BCM_CALADAN3_NVID_SET_UNTAGGEDSTRIP   = 0x00020000
} _bcm_caladan3_nvid_pvv2e_control_flags_t;

typedef enum _bcm_caladan3_nvid_action_e {
    BCM_CALADAN3_NVID_ACTION_SET,
    BCM_CALADAN3_NVID_ACTION_TOUCH
} _bcm_caladan3_nvid_action_t;

#define BCM_CALADAN3_NVID_FLAGS_MASK (BCM_CALADAN3_NVID_NVID_OVR_MASK | \
                                  BCM_CALADAN3_NVID_OVERRIDE_PVV2E | \
                                  BCM_CALADAN3_NVID_USE_PVV2E | \
                                  BCM_CALADAN3_NVID_SET_REPLACE | \
                                  BCM_CALADAN3_NVID_SET_KEEPORSTRIP | \
                                  BCM_CALADAN3_NVID_USE_NVID_OVR)

/* Use this vid for untagged traffic when using logical bridging model */
#define _BCM_VLAN_G3P1_UNTAGGED_VID 0xfff


/*
 *  This describes state in the egress translation entry lists.  It is also
 *  used to specify which fields to change in a translation entry.
 */
typedef enum _bcm_sbx_v_e_t_state_e {
    VLAN_TRANS_CHANGE_VID = 0x0001,   /* use specified destination VID */
    VLAN_TRANS_CHANGE_PRI = 0x0002    /* use specified destination priority */
} _bcm_sbx_v_e_t_state_t;


/*
 *  This structure contains information necessary to map VLAN GPORT ID to
 *  logical port ID.
 */
typedef struct _bcm_caladan3_vgp_info_s {
    uint16 *lpid;  /* map GPORT ID to logical port ID */
} _bcm_caladan3_vgp_info_t;

/*
 *  I need more state information for vlan logical ports than is provided at
 *  the BCM layer structure.  Thankfully, these additional data do not need to
 *  be propagated to other units.
 *
 *  WARNING: Order is important; do not reorder this structure carelessly!
 */
typedef enum _bcm_caladan3_vlan_gport_internal_flags_e {
    _BCM_CALADAN3_VLAN_GP_EXTERN_OHI = 0x0001     /* caller provided initial OHI */
} _bcm_caladan3_vlan_gport_internal_flags_t;

typedef struct _bcm_caladan3_vlan_gport_e_s {
    _bcm_caladan3_vlan_gport_internal_flags_t flags; /* internal flags */
} _bcm_caladan3_vlan_gport_e_t;

typedef struct _bcm_caladan3_vlan_gport_s {
    dq_t            nt;             /* link to next gport in trunk */
    bcm_vlan_port_t p;              /* BCM_VLAN_GPORT description structure */
    _bcm_caladan3_vlan_gport_e_t e; /* Private additional data structure */
} _bcm_caladan3_vlan_gport_t;

typedef struct _bcm_caladan3_trunk_state_s {
    dq_t                  vlan_port_head;
    bcm_trunk_add_info_t  tinfo;
} _bcm_caladan3_trunk_state_t;

/*
 *  This structure contains the local state information for this module.
 *
 *  It also contains a locking device, on a per unit basis, to prevent more
 *  than one concurrent access to the local data per unit.
 *
 *  Note that the 'standard' tag and strip ETEs do not have reference counters
 *  or other data (see _bcm_sbx_vlan_egr_trans_t) since they are allocated
 *  dynamically but held for the duration, and are used in all 'standard' cases
 *  (no translation applied).
 *
 *  WARNING: 'init' does *NOT* indicate initialisation complete, it indicates
 *  initialisation *IN PROGRESS*.  It is used to disregard certain fields that
 *  always seem to contain random garbage during initialisation (such as the
 *  current ET index used for egrPortVlan2Etc table entries).  If it is not set
 *  back to FALSE once init has completed, the module will assume it is still
 *  in initialisation mode and will NEVER free certain resources.
 *
 *  'Dropping tagged frames' presently only describes the behaviour for single
 *  tagged frames, as the double tagged frames go through a different path.
 */

typedef struct _bcm_sbx_caladan3_vlan_state_s {
    bcm_pbmp_t                tag_drop;       /* Dropping tagged frames */
    bcm_pbmp_t                untag_drop;     /* Dropping untagged frames */
    bcm_vlan_t                default_vid;    /* Default VID */
    unsigned int              init;           /* indicates init pending */
    _bcm_caladan3_vgp_info_t  gportInfo;      /* VLAN GPORT information */
    int                       vlanControl;    /* vlanControl bits set */
    int                       batch;          /* use batch soc apis */
    int                       batchDirty;     /* commit only dirty batches! */
    _bcm_caladan3_trunk_state_t   trunk[SBX_MAX_TRUNKS]; /* Track trunk
                                                      * membership changes
                                                      */

#ifdef BCM_CALADAN3_G3P1_SUPPORT
    _bcm_caladan3_nvid_pvv2e_control_flags_t nvflag[SBX_MAX_PORTS]; /* nvid data */
    _bcm_caladan3_vlan_divergence_t divergence[SBX_MAX_PORTS]; /* divergence */
    int                       *vlan_port_member_table;  /* Bitmap to indicate which
                                                         * ports are members of vlans.
                                                        */
    soc_sbx_g3p1_p2appdata_t    *appdata;       /* SDK only appdata table */
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
    bcm_vlan_t                nvid[SBX_MAX_PORTS]; /* Port native VIDs */
} _bcm_sbx_caladan3_vlan_state_t;


extern int _bcm_caladan3_vlan_port_vlan_vector_get(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec);

extern int _bcm_caladan3_g3p1_logical_interface_ingress_stats_enable_update (int unit,
                                                                uint32 lpi,
                                                                int get,
                                                                int *statsEnabled);

extern int _bcm_caladan3_g3p1_logical_interface_egress_stats_enable_update (int unit,
                                                            uint32 ohi,
                                                            int get,
                                                            int *statsEnabled);

extern int bcm_caladan3_g3p1_vlan_port_nativevid_touch(int unit,
                                          bcm_port_t port);

extern int _bcm_caladan3_vlan_fte_gport_get(int unit,
                               bcm_gport_t gport,
                               uint32 *ftei);

extern int bcm_caladan3_vlan_port_get_lpid(int unit,
                              bcm_gport_t gport,
                              uint32 *lpid,
                              bcm_port_t *pport);

extern int _bcm_caladan3_vlan_port_tagged_ingress_drop_get(const int unit,
                                              const bcm_port_t port,
                                              bcm_vlan_t *vlan,
                                              int *drop);

extern int _bcm_caladan3_vlan_port_untagged_ingress_drop_get(const int unit,
                                                const bcm_port_t port,
                                                int *drop);

extern int _bcm_caladan3_vlan_port_untagged_ingress_drop_set(const int unit,
                                                const bcm_port_t port,
                                                int drop);

extern int _bcm_caladan3_vlan_port_tagged_ingress_drop_set(const int unit,
                                              const bcm_port_t port,
                                              const bcm_vlan_t vlan,
                                              const int drop);

extern int _bcm_caladan3_vlan_port_fetch(int unit,
                                         bcm_vlan_t vid,
                                         pbmp_t* pbmp,
                                         pbmp_t* ubmp);

extern int _bcm_caladan3_vlan_port_filter_get(const int unit,
                                 const bcm_port_t port,
                                 int *ingress,
                                 int *egress);

extern int _bcm_caladan3_vlan_port_ingress_filter_set(const int unit,
                                         const bcm_port_t port,
                                         const int filter);

extern int _bcm_caladan3_vlan_port_egress_filter_set(const int unit,
                                        const bcm_port_t port,
                                        const int filter);

extern int _bcm_caladan3_map_vlan_gport_target(int unit,
                                  bcm_gport_t gport,
                                  bcm_module_t *locMod,
                                  bcm_module_t *tgtMod,
                                  bcm_port_t *tgtPort,
                                  uint32 *portFte,
                                  uint32 *portQueue);

extern int _bcm_caladan3_vlan_port_vlan_vector_set(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec);

extern int bcm_caladan3_lp_get_by_pv(int unit,
                        int iport,
                        int ivid,
                        soc_sbx_g3p1_lp_t *lpData);

extern int bcm_caladan3_lp_check_diverged(int unit,
                             int iport,
                             int ivid,
                             _bcm_caladan3_vlan_divergence_t *divergence);

extern int bcm_caladan3_lp_set_by_pv_and_diverge(int unit,
                                    int iport,
                                    int ivid,
                                    _bcm_caladan3_vlan_divergence_t divergence,
                                    soc_sbx_g3p1_lp_t *lpData);

extern int bcm_caladan3_vlan_port_lp_replicate(const int unit,
                                  const bcm_port_t port,
                                  const _bcm_caladan3_lp_copy_flags_t flags,
                                  const soc_sbx_g3p1_lp_t *oldValues,
                                  const soc_sbx_g3p1_lp_t *newValues);

extern int
bcm_caladan3_vlan_default_get(int unit,
                            bcm_vlan_t *vid_ptr);

extern int
bcm_caladan3_vlan_vgp_frame_max_access(int unit, bcm_gport_t gport,
                                   int *size, int set);

extern int
_bcm_caladan3_vlan_vsi_gport_get(int unit,
                               bcm_gport_t gport,
                               bcm_vlan_t *vsi);

extern int
_bcm_caladan3_vlan_check_exists(const int unit,
                              const bcm_vlan_t vid,
                              int *valid);

extern int
_bcm_caladan3_vlan_vsi_gport_add(int unit, bcm_vlan_t vsi, bcm_gport_t gport);

extern int
_bcm_caladan3_vlan_vsi_gport_delete(int unit, bcm_vlan_t vsi, bcm_gport_t gport);

extern int
bcm_caladan3_vlan_port_qosmap_set(int unit, bcm_gport_t gport,
                                  int ingrMap, int egrMap,
                                  uint32 ingFlags, uint32 egrFlags);

extern int
bcm_caladan3_vlan_port_qosmap_get(int unit, bcm_gport_t gport,
                                  int *ing_idx, int *egr_idx,
                                  uint32 *ing_flags, uint32 *egr_flags);

extern int
bcm_caladan3_vgp_policer_set(int unit, bcm_gport_t gport, bcm_policer_t pol_id);

extern int
_bcm_caladan3_g3p1_vlan_nvid_pvv2e_flags_set
          (int unit,
           int iport,
           _bcm_caladan3_nvid_pvv2e_control_flags_t flags);

#endif  /* _BCM_INT_SBX_CALADAN3_VLAN_H_ */
