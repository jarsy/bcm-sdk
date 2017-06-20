/*
 * $Id: oam.h,v 1.26 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _BCM_INT_SBX_FE2000_OAM_H_
#define _BCM_INT_SBX_FE2000_OAM_H_

#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>
#include <soc/sbx/fe2000.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#include <soc/sbx/g2p3/g2p3_int.h>
#endif

#include <shared/idxres_fl.h>
#include <shared/hash_tbl.h>

#include <bcm/error.h>
#include <bcm/debug.h>
#include <bcm/oam.h>
#include <bcm/bfd.h>
#include <bcm/vlan.h>
#include <bcm/stack.h>
#include <bcm/module.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/l2.h>
#include <bcm_int/sbx/fe2000/vlan.h>
#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/fe2000/oam/tcal.h>
#include <bcm_int/sbx/fe2000/oam/ltm.h>
#include <bcm_int/sbx/fe2000/trunk.h>
#include <bcm_int/sbx/fe2000/mpls.h>
#include <bcm_int/sbx/stack.h>

#include <soc/sbx/fe2k_common/sbFe2000Pm.h> 
#include <soc/sbx/fe2k_common/sbCrc32.h> 

#include <soc/sbx/fe2k/sbFe2000InitUtils.h>
#include <soc/sbx/fe2kxt/sbFe2000XtInitUtils.h>
#include <soc/sbx/g2p3/g2p3_pp.h>
#include <soc/sbx/g2p3/g2p3_pp_rule_encode.h>
#include <soc/sbx/sbDq.h>

#define BCM_OAM_IS_INIT(unit)   (_state[unit] != NULL && _state[unit]->mutex)


/* There is no explicit ccm tx enable flags, accept either of these */
#define _SBX_OAM_CCM_TX_ENABLE   (BCM_OAM_ENDPOINT_PORT_STATE_TX |      \
                                  BCM_OAM_ENDPOINT_INTERFACE_STATE_TX)

#define BCM_OAM_MAX_CALLBACKS 1
#define OAM_RX_RULE           0

#define MAID_ID_TO_ZF(p) \
  (((p)[0] << 24) | \
   ((p)[1] << 16) | \
   ((p)[2] << 8)  | \
   ((p)[3]     ))

#define ZF_TO_MAID_ID(i, p) \
  (p)[0] = ((i) >> 24) & 0xff;   \
  (p)[1] = ((i) >> 16) & 0xff;   \
  (p)[2] = ((i) >> 8 ) & 0xff;   \
  (p)[3] = ((i)      ) & 0xff;

#define INVALID_POLICER_ID    0

#define ONE_SECOND  (1000)  /* in ms */
#define ONE_MINUTE  (60 * ONE_SECOND)

#define MAID_CCM_LEN                48
#define MAID_MPLS_LEN               20
#define MAID_MAX_LEN                48

#define OAM_SBX_MAX_MDLEVEL         8
#define OAM_SBX_MAX_PERIOD          (ONE_MINUTE * 10)

#define OAM_KEY_PACK(dest, src, size) \
   sal_memcpy(dest, src, size);       \
   dest += size;

/*  group.name(?) + group + name + level + direction + gport + (vid/mpls_label) */
#define OAM_HASH_KEY_SIZE  (MAID_MAX_LEN + 4 + 4 + 4 + 4 + 4 + 4)
typedef uint8 oam_sw_hash_key_t[OAM_HASH_KEY_SIZE];

#define OAM_DIR_DOWN 0
#define OAM_DIR_UP   1
#define OAM_NO_LEVEL 8
#define OAM_MIRROR_ID 3
#define OAM_QUEUE_OFFSET 0x80
#define OAM_MAC_LB_PORT 23
#define OAM_MAC_LB_QUEUE  0x1b
#define OAM_MAC_LOOPBACK 0
#if OAM_MAC_LOOPBACK
#define OAM_IQS_CODE   SB_G2P3_FE_PP_PARSE_UPMEP
#else
#define OAM_IQS_CODE   SB_G2P3_FE_PP_PARSE_UPMEP_ERH
#endif

/* these defines for ICC-based MEG-ID Format */
#define OAM_MEG_ID_RSVD   0x01
#define OAM_MEG_ID_FORMAT 0x20

typedef enum {
    oamInvalid, 
    oamDelay, oamLoss, oamEp, oamBfd, oamlsp1731, oamPsc
} oam_types_t;

#define OAM_IS_ENDPOINT_MPLS_Y1731(t) ((t) == oamlsp1731)

#define INVALID_RECORD_INDEX  (0)

/* Is the given hardward endpoint id valid */
#define ENDPOINT_ID_VALID(u, id)  (((id) != INVALID_RECORD_INDEX) && \
                                   ((id) < _state[u]->max_endpoints))

/* validate trunk ids */
#define OAM_TID_VALID(tid_) (((tid_) >= 0) && ((tid_) < SBX_MAX_TRUNKS))

/* Store the record index and extra information for retrieving oam endpoint
 * records from the Gu2k databases.  gport is stored as a key and data for 
 * get-by-id to resolve all info for caller.
 */

typedef struct oam_sw_hash_data_s {
    dq_t               list_node;
    dq_t               trunk_list_node;
    dq_t               lsp_list_node;
    uint16             rec_idx;      /* hw endpoint record index */
    uint16             delay_idx;    /* delay record index, if enabled */
    uint16             loss_idx;     /* loss record index, if enabled */
    uint16             peer_idx;     /* peer record index (pm) */
    uint16             maid_cookie;  /* maid cookie used for MaMepKey; it is the local ID */
    uint16             ep_name;      /* MepId for Enet, just an identifier for MPLS. */
    uint32             flags;        /* bcm endpoint_flags */
    oam_types_t        oam_type;     /* delay, loss, or Endpoint */
    uint32             type;         /* bcm_oam_endpoint_type_t */
    bcm_oam_group_t    group;
    bcm_gport_t        gport;
    uint32             vid_label;    /* vid or label */
    uint32             direction;
    tcal_id_t          tcal_id;
    bcm_mac_t          mac_address;
    uint32             lsm_idx;      /* local station match index */
    uint16             cocounter;    /* coherent counter idx (lm) */
    uint32             mdlevel;
    uint32             interval; 
    int                ing_map;      /* Ingress QoS map profile */
    int                egr_map;      /* Egress QoS map profile */
    uint32             bfd_flags;    /* BFD flag */
    uint32             local_rec_idx;
    bcm_if_t           intf_id;      /* Interface Id */
    int                lsp_trunk_id;   /* OAM over LSP over trunk */
    int                trunk_index;    /* designated port for OAM in a trunk */
    uint32             psc_slow_rate; /* PSC slow rate interval */
    uint32             psc_burst;     /* PSC burst rate */
    uint32             psc_flags;     /* PSC flags  */
    uint32             period;        /* PM period */
    uint8              pkt_pri_bitmap; /* Priority Class (LM) */
    uint8              pkt_pri;       /* Egress marking for outgoing LM messages */
    bcm_cos_t          int_pri;       /* Egress queuing for outgoing LM messages */
    int                threshold;
} oam_sw_hash_data_t;

#define  ALLOC_FT_b          0x01  
#define  ALLOC_ETEL2_b       0x02  
#define  ALLOC_ETEENCAP_b    0x04   
#define  ALLOC_OH_b          0x08   
#define  ALLOC_ESMAC_b       0x10
     
#define ALLOC_GET(type,v)  ((v) & ALLOC_##type##_b)   
#define ALLOC_SET(type,v)  (v) |= ALLOC_##type##_b 

/*  Simple data struct to track the egress path indexes
 */
typedef struct egr_path_desc_s {
    uint32    ftIdx;
    uint32    eteL2Idx;
    uint32    eteEncapIdx;
    uint32    ohIdx;
    uint8     allocated;
    int         is_trunk;          /* Set if egrpath is over a trunk for non-ethernet endpoints */
    int         trunk_id;          /* Trunk ID if egrpath is over a trunk */
    
    soc_sbx_g2p3_ft_t        fte;
    soc_sbx_g2p3_etel2_t     eteL2;
    soc_sbx_g2p3_eteencap_t  eteEncap;
    soc_sbx_g2p3_oi2e_t      oh;

    bcm_gport_t port; /* modport or local gport for mpls endpoints */   
} egr_path_desc_t;

typedef struct oam_group_desc_s {
    bcm_oam_group_info_t     *group;       /* single group info soft state */
    dq_t                      ep_list;     /* list of endpoints associated 
                                            * with this group */
} oam_group_desc_t;


typedef struct oam_timer_q_elt_s {
    uint32 timer;
    uint8  rdi;
} oam_timer_q_elt_t;

typedef struct oam_timer_queue_s {
    int                 tail;
    int                 lock;
    int                 max_depth;
    int                 intr_qid;
    oam_timer_q_elt_t  *queue[2];
} oam_timer_queue_t;


#define OAM_TIMER_QUEUE(unit)  (&(_state[unit]->timer_q))

/*
 * Configure the RX interval as 3.5 times larger than the requested value
 * to allow for 2/3 packet loss.
 */
#define OAM_DEFAULT_WINDOW_MULTIPLIER (3.5)

/*
 * OAM_LOCK/UNLOCK should only be used to synchronize with
 * interrupt handlers.  Code protected by these calls should
 * only involve memory references.
 */

#define OAM_LOCK(unit)        \
    sal_mutex_take(_state[unit]->mutex, sal_mutex_FOREVER)
#define OAM_UNLOCK(unit)      \
    sal_mutex_give(_state[unit]->mutex)

#define OAM_THREAD_NOTIFY(unit) {                                       \
    if (!oam_control.cb_notify_given) {                                 \
        oam_control.cb_notify_given = TRUE;                             \
        sal_sem_give(oam_control.cb_notify);                            \
    }                                                                   \
}


typedef struct { 
    bcm_oam_event_cb         epCb;
    void                    *userData;
} _oam_cb_info_t;

/* Store the ucode endpoint type values */
typedef struct _g2p3_ep_ucode_type_ids_s {
    uint32  bfd;
    uint32  lm;
    uint32  dm;
    uint32  ccm_first;
    uint32  ccm_second;
    uint32  ccm_peer;
    uint32  psc;
    uint32  invalid;
} _g2p3_ep_ucode_type_ids_t;

/* Store the ucode subtype values */
typedef struct _g2p3_ep_ucode_subtypes_s {
    uint32  cfm;
    uint32  lm;
    uint32  dm;
} _g2p3_ep_ucode_subtypes_t;

/* Store the ucode transport layer values */
typedef struct _g2p3_ep_ucode_transport_s {
    uint32  enet;
    uint32  pwe;
    uint32  lsp;
} _g2p3_ep_ucode_transport_t;

typedef struct fe2k_oam_wb_ep_meta_s {
    uint16      group;
    uint16      lsmi;
} fe2k_oam_wb_ep_meta_t;

#define OAM_RCVR_MAX_EP  0x2000

/*   Version 1.0 of the scache layout for oam level-2 recovery
 *  The version is stored in state[u]->wb_cache->version
 *  This memory map is applied to state[u]->wb_cache->cache;
 *  This memory map is used to define the layout and amount of memory 
 *    allocated for the purposes of warmboot - if its not here - 
 *    it's not stored.
 */
typedef struct fe2k_oam_wb_mem_layout_s {
    fe2k_oam_wb_ep_meta_t  ep_meta[OAM_RCVR_MAX_EP];    /* index = Ep_Idx => 
                                                         * meta data */
} fe2k_oam_wb_mem_layout_t;

#define OAM_WB_VERSION_1_0        SOC_SCACHE_VERSION(1,0)
#define OAM_WB_CURRENT_VERSION    OAM_WB_VERSION_1_0

typedef struct _oam_trunk_data_t_s {
    dq_t                 endpoint_list;    /* list of endpoints on this LAG */
    bcm_trunk_add_info_t add_info;         /* current trunk-add-info for this trunk. */
} _oam_trunk_data_t;

/* Global state per device */
typedef struct oam_state_s {
    shr_idxres_list_handle_t  ep_pool;     /* Endpoint resource managment */
    shr_idxres_list_handle_t  group_pool;  /* Group resource managment */
    shr_idxres_list_handle_t  timer_pool;  /* Timers resource managment */
    shr_idxres_list_handle_t  coco_pool;   /* Coherent counter resource mgt */
    shr_htb_hash_table_t      mamep_htbl;  /* Hash table to find endpoints
                                            * based on MAID/MEP */
    shr_htb_hash_table_t      timer_htbl;  /* Hash table to find endpoints
                                            * based on timer ids */
    _oam_cb_info_t            wd_cb_data;   /* user watchdog timeout callback */
    oam_sw_hash_data_t       *hash_data_store; /* local storage of hash data,
                                                * hash module stores pointers
                                                */
    oam_group_desc_t         *group_info;    /* array of group soft state; 
                                              * tracks groups<->endpoints */
    _oam_trunk_data_t        *trunk_data;    /* endpoint and port trunk 
                                              * membership; 
                                              * indexed by TrunkID */
    sal_mutex_t               mutex;         /* Sync for handler list */
    uint32                  max_pids;
    uint32                  max_endpoints;
    uint32                  ep_reservedLow;
    uint32                  ep_reservedHigh;
    uint32                  grp_reservedLow;
    uint32                  grp_reservedHigh;
    uint32                  eteencap_lsm;
    oam_timer_queue_t         timer_q;       /* interrupt->cb handler thread q */
    _g2p3_ep_ucode_type_ids_t ep_type_ids;
    _g2p3_ep_ucode_subtypes_t ep_subtypes;
    _g2p3_ep_ucode_transport_t ep_transport;
    uint32                  timer_base;    /* base addr of oamtimer 
                                              * policer bank */
    uint32                  up_init;       /* flag gets set to 1 after 
                                              * up-mep init. */
    uint32                  isSmacPri;     /* flag gets set to 1 after first up MEP config. */
    uint32                  isSmacAux;     /* flag gets set to 1 after secondary up MEP config. */

    /* the states below are global scratch within API. It is included here so
     * its synchronized using state semaphore and can be shared by various funtions.
     * It is not required to cache the below two states */
    bcm_l3_intf_t             outgoing_tunnel_intf;
    bcm_l3_intf_t             incoming_tunnel_intf;
    bcm_l3_egress_t           outgoing_tunnel_egr; 
    bcm_l3_egress_t           incoming_tunnel_egr;
        
#ifdef BCM_WARM_BOOT_SUPPORT 
    uint32                    wb_hdl;
#endif
} oam_state_t;


#ifdef BCM_WARM_BOOT_SUPPORT 
extern int _oam_wb_layout_get(int unit, fe2k_oam_wb_mem_layout_t **l);
#endif /* */


#define SUPPORTED_ENDPOINT_FLAGS  (BCM_OAM_ENDPOINT_REPLACE            | \
                                   BCM_OAM_ENDPOINT_WITH_ID            | \
                                   BCM_OAM_ENDPOINT_REMOTE             | \
                                   BCM_OAM_ENDPOINT_UP_FACING          | \
                                   _SBX_OAM_CCM_TX_ENABLE              | \
                                   BCM_OAM_ENDPOINT_USE_QOS_MAP        | \
                                   BCM_OAM_ENDPOINT_INTERMEDIATE )

/* mpls lsp egress interface */
#define OAM_MPLS_LSP_OUT_IF(u)      _state[(u)]->outgoing_tunnel_intf
#define OAM_MPLS_LSP_IN_IF(u)       _state[(u)]->incoming_tunnel_intf
#define OAM_MPLS_LSP_OUT_EGR_IF(u)  _state[(u)]->outgoing_tunnel_egr
#define OAM_MPLS_LSP_IN_EGR_IF(u)   _state[(u)]->incoming_tunnel_egr

/* group info state accessors */
#define OAM_GROUP_INFO(u,g)     _state[(u)]->group_info[(g)].group
#define OAM_GROUP_EP_LIST(u,g)  _state[(u)]->group_info[(g)].ep_list

/* group info state accessors */
#define OAM_GROUP_EP_LIST_EMPTY(u,g) \
     (DQ_EMPTY(&OAM_GROUP_EP_LIST(u,g)))

#define _BCM_FE2000_OAM_GET_EP_DATA_FROM_LIST(e, var) \
            (var) = DQ_ELEMENT_GET(oam_sw_hash_data_t*,   \
                               (e), list_node)


/********************************************************/
extern void _oam_oamep_prepare(int unit, 
                               soc_sbx_g2p3_oamep_t *oamep);

extern int _oam_timer_free(int unit, uint32 *id);

extern int _oam_timer_allocate(int unit, int ep_rec_index, 
                               double interval_ms, double window_multiplier,
                               int started, uint32 *id);

extern int _oam_dump_ep(int unit, bcm_oam_endpoint_info_t *ep_info);

extern void _oam_egr_path_init(int unit, egr_path_desc_t *egrPath);

extern int _oam_egr_path_get(int unit, egr_path_desc_t *egrPath, 
                             uint32 epIdx);

extern int _oam_egr_path_commit(int unit, egr_path_desc_t *egrPath);

extern int _oam_egr_path_alloc(int unit, bcm_mac_t smac, 
                               egr_path_desc_t *egrPath);

extern int _oam_egr_path_clone_upmep(int unit, egr_path_desc_t *clone,
                               bcm_gport_t gport, int vid, bcm_mac_t smac);

extern int _oam_egr_path_clone_downmep(int unit, egr_path_desc_t *clone,
                               uint32 rec_idx);

extern int _oam_egr_path_update(int unit, egr_path_desc_t *egrPath,  
                                bcm_oam_endpoint_info_t *ep_info,
                                bcm_trunk_add_info_t *trunk_info); 

extern int _oam_egr_path_free(int unit, egr_path_desc_t *egrPath);

extern int _oam_egr_path_dmac_set(int unit, egr_path_desc_t *egrPath, 
                                  bcm_mac_t dmac, int dmaclsbset);

extern int _oam_egr_path_smac_set(int unit, egr_path_desc_t *egrPath,
                                  bcm_mac_t smac);

extern void _oam_hash_data_clear(oam_sw_hash_data_t *hash_data);

extern int _oam_copy_endpoint(int unit, 
                              oam_sw_hash_data_t *hash_data,
                              bcm_oam_endpoint_info_t *ep_info);

extern int bcm_fe2000_oam_endpoint_range_reserve(int unit, 
                                                 int highOrLow, 
                                                 uint32 val);

extern int bcm_fe2000_oam_endpoint_range_get(int unit, 
                                             uint32 *low, 
                                             uint32 *high);

extern int bcm_fe2000_oam_group_range_reserve(int unit, 
                                              int highOrLow, 
                                              uint32 val);

extern int bcm_fe2000_oam_group_range_get(int unit, 
                                          uint32 *low, 
                                          uint32 *high);

extern void bcm_fe2000_oam_dump_reservation(int unit);

extern int _bcm_fe2000_oam_ep_check_and_reserve(int unit, int id);

extern int _bcm_fe2000_oam_ep_check_and_free(int unit, int id);

/********************************************************/
#endif  /* _BCM_INT_SBX_FE2000_OAM_H_  */
