/*
 * $Id: oam.h,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _BCM_INT_SBX_CALADAN3_OAM_H_
#define _BCM_INT_SBX_CALADAN3_OAM_H_

#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ca_auto.h>
#ifdef BCM_CALADAN3_G3P1_SUPPORT
#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/caladan3.h>
#endif

#include <shared/idxres_fl.h>
#include <shared/hash_tbl.h>

#include <bcm/error.h>
#include <bcm/debug.h>
#include <bcm/oam.h>
#include <bcm/bfd.h>
#include <bcm/stack.h>
#include <bcm/module.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/l2.h>
#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/stack.h>
#include <soc/sbx/sbDq.h>
#include <bcm/vlan.h>
#include <bcm_int/sbx/caladan3/vlan.h>

#define BCM_C3_OAM_DEBUG_CCM(flags, ccm) if(LOG_CHECK(flags | BSL_LS_BCM_OAM)) \
                                       { _oam_dump_ccm(ccm); }

/* There is no explicit ccm tx enable flags, accept either of these */
#define BCM_C3_OAM_CCM_TX_ENABLE   (BCM_OAM_ENDPOINT_PORT_STATE_TX |      \
                                    BCM_OAM_ENDPOINT_INTERFACE_STATE_TX)

#define BCM_C3_OAM_INVALID_POLICER_ID    0

#define BCM_C3_OAM_ONE_SECOND_IN_MS  (1000)  /* in ms */
#define BCM_C3_OAM_ONE_MINUTE_IN_MS  (60 * BCM_C3_OAM_ONE_SECOND_IN_MS)

#define BCM_C3_OAM_SBX_MAX_MDLEVEL       8
#define BCM_C3_OAM_SBX_MAX_PERIOD    (BCM_C3_OAM_ONE_MINUTE_IN_MS * 10)

#define BCM_C3_OAM_ENET_ENDPOINT(t) ((t) == bcmOAMEndpointTypeEthernet)

#if 000
#define OAM_DIR_DOWN 0
#define OAM_DIR_UP   1
#define OAM_NO_LEVEL 8
#define OAM_MIRROR_ID 3
#define OAM_QUEUE_OFFSET 0x80

#define INVALID_RECORD_INDEX  (0)
#endif

/* Is the given hardward endpoint id valid */
#define ENDPOINT_ID_VALID(u, id)  (((id) != INVALID_RECORD_INDEX) && \
                                   ((id) < bcm_c3_oam_state[u]->max_endpoints))


typedef struct bcm_c3_oam_endpoint_state_s {
    dq_t               endpoint_list; /* list of endpoints associated with this endpoint      */
    bcm_oam_endpoint_t endpoint_id;   /* hw endpoint id                                       */
    uint16             name;          /* from bcm_oam_endpoint_info_t (mepid)                 */
    bcm_oam_endpoint_t local_id;      /* from bcm_oam_endpoint_info_t used by remote endpts   */
    uint32             flags;         /* from bcm_oam_endpoint_info_t  WITH_ID, REMOTE, etc   */
    bcm_oam_endpoint_type_t type;     /* OAM endpoint type enet, mpls, etc...                 */
    bcm_oam_group_t    group;         /* from bcm_oam_endpoint_info_t group identifier        */
    bcm_gport_t        gport;         /* from bcm_oam_endpoint_info_t gport assoc w endpoint  */
    uint32             direction;     /* 1 means up facing 0 means down facing                */
    int                mdlevel;       /* from bcm_oam_endpoint_info_t level - mdlevel         */
    uint32             ccm_period;    /* from bcm_oam_endpoint_info_t CCM period in ms        */ 
    int                ing_map;       /* from bcm_oam_endpoint_info_t Ingress QoS map profile */
    int                egr_map;       /* from bcm_oam_endpoint_info_t Egress QoS map profile  */
    bcm_if_t           intf_id;       /* from bcm_oam_endpoint_info_t Interface Id            */
    uint8              pkt_pri;       /* from bcm_oam_endpoint_info_t Egress mark LM/CCM msgs */
    bcm_cos_t          int_pri;       /* from bcm_oam_endpoint_info_t Egress cosq LM/CCM msgs */
    /* type specific state for enet see bcm_c3_enet_oam_endpoint_state_t */
} bcm_c3_oam_endpoint_state_t;


typedef struct oam_group_desc_s {
    bcm_oam_group_info_t     *state;          /* single group info soft state array indexed by group */
    dq_t                      endpoint_list;  /* list of endpoints associated 
                                               * with this group */
} bcm_c3_oam_group_desc_t;

/*
 * OAM_LOCK/UNLOCK should only be used to synchronize with
 * interrupt handlers.  Code protected by these calls should
 * only involve memory references.
 */

#define OAM_LOCK(unit)        \
    if (bcm_c3_oam_state[unit] != NULL) sal_mutex_take(bcm_c3_oam_state[unit]->mutex, sal_mutex_FOREVER)
#define OAM_UNLOCK(unit)      \
    if (bcm_c3_oam_state[unit] != NULL) sal_mutex_give(bcm_c3_oam_state[unit]->mutex)

#define OAM_THREAD_NOTIFY(unit) {                                       \
    if (!oam_control.cb_notify_given) {                                 \
        oam_control.cb_notify_given = TRUE;                             \
        sal_sem_give(oam_control.cb_notify);                            \
    }                                                                   \
}


typedef struct { 
    bcm_oam_event_cb         event_cb;
    void                    *user_data;
} bcm_c3_oam_cb_info_t;


/* used in mamep_htbl key */
/*  group.name(MAID + group + name + level + direction + gport + type specific data  */
#define BCM_C3_OAM_ENDPOINT_HASH_KEY_SIZE  (BCM_OAM_GROUP_NAME_LENGTH + 4 + 2 + 4 + 4 + 4 + 4)
typedef uint8 bcm_c3_oam_endpoint_hash_key_t[BCM_C3_OAM_ENDPOINT_HASH_KEY_SIZE];

/* Global state per device */
typedef struct bcm_c3_oam_state_s {
    /*** Endpoint resource pool ***/
    shr_idxres_list_handle_t     endpoint_pool;           /* Endpoint resource management */
    uint32                       endpoint_reserved_lo;
    uint32                       endpoint_reserved_hi;
    bcm_c3_oam_endpoint_state_t *endpoint_state;          /* local storage of hash data,
                                                           * hash module stores pointers
                                                           */

    /*** Group resource pool ***/
    shr_idxres_list_handle_t     group_pool;              /* Group resource management */
    uint32                       group_reserved_lo;
    uint32                       group_reserved_hi;


    /*** Timer ***/
    shr_idxres_list_handle_t     timer_pool;              /* Timers resource management 0 
                                                           * to num_oam_timers(64K or less)
                                                           */
    uint32                      *timer_handle;            /* Timer handle to pass to soc
                                                           * cop layer, array indexed by
                                                           * watchdog_id (0-num_oam_timers)
                                                           */

    uint32                       num_oam_timers;

    shr_htb_hash_table_t         timer_htbl;              /* Hash table to find endpoints
                                                           * based on timer ids 
                                                           */
    bcm_c3_oam_cb_info_t         event_cb_info[bcmOAMEventCount]; /* event callback and user data */


    shr_htb_hash_table_t         mamep_htbl;              /* Hash table to find endpoints
                                                           * based on MAID/MEP 
                                                           */


    bcm_c3_oam_group_desc_t     *group_info;             /* array of group soft state; 
                                                          * tracks groups<->endpoints 
                                                          */
    sal_mutex_t                  mutex;                   /* Sync for handler list */
    uint32                       max_endpoints;

} bcm_c3_oam_state_t;


#define SUPPORTED_ENDPOINT_FLAGS  (BCM_OAM_ENDPOINT_REPLACE            | \
                                   BCM_OAM_ENDPOINT_WITH_ID            | \
                                   BCM_OAM_ENDPOINT_REMOTE             | \
                                   BCM_OAM_ENDPOINT_UP_FACING          | \
                                   BCM_C3_OAM_CCM_TX_ENABLE            | \
                                   BCM_OAM_ENDPOINT_USE_QOS_MAP        | \
                                   BCM_OAM_ENDPOINT_INTERMEDIATE )

#define BCM_C3_OAM_MAID_PACK_TO_WORD(p) \
  (((p)[0] << 24) | \
   ((p)[1] << 16) | \
   ((p)[2] << 8)  | \
   ((p)[3]     ))

#define BCM_C3_OAM_UNPACK_WORD_TO_MAID(i, p) \
  (p)[0] = ((i) >> 24) & 0xff;   \
  (p)[1] = ((i) >> 16) & 0xff;   \
  (p)[2] = ((i) >> 8 ) & 0xff;   \
  (p)[3] = ((i)      ) & 0xff;

void bcm_caladan3_oam_dump_reservation(int unit);

int bcm_c3_oam_timer_allocate(int unit, int ep_rec_index, int timeout_in_ms,
                              int started, uint32 *watchdog_id);
int bcm_c3_oam_timer_free(int unit, uint32 watchdog_id);

int bcm_c3_oam_group_info_endpoint_list_count(int unit, int group);
int bcm_c3_oam_group_info_endpoint_list_add(int unit, int group, int endpoint_id);
int bcm_c3_oam_group_info_endpoint_list_remove(int unit, int group, int endpoint_id);

int bcm_c3_oam_is_endpoint_id_in_reserved_range(int unit, bcm_oam_endpoint_t endpoint_id);

int bcm_caladan3_oam_detach(int unit);

int
bcm_c3_oam_group_get(int unit, bcm_oam_group_t group, 
                     bcm_oam_group_info_t *group_info);
int
bcm_caladan3_oam_endpoint_get(int unit, bcm_oam_endpoint_t endpoint, 
                              bcm_oam_endpoint_info_t *endpoint_info);
int
bcm_c3_oam_endpoint_state_from_key_get(int unit, bcm_c3_oam_endpoint_hash_key_t key,
                                       bcm_c3_oam_endpoint_state_t *endpoint_state, int remove);
void
bcm_c3_oam_endpoint_state_from_id_get(int unit, bcm_c3_oam_endpoint_state_t *endpoint_state, bcm_oam_endpoint_t endpoint_id);
int
bcm_c3_oam_endpoint_key_set(int unit, bcm_c3_oam_endpoint_hash_key_t endpoint_key,
                            bcm_oam_endpoint_t endpoint_id);
int
bcm_c3_oam_endpoint_info_from_id_get(int unit,  bcm_oam_endpoint_t endpoint_id, bcm_oam_endpoint_info_t *endpoint_info);

int
bcm_c3_oam_endpoint_is_allocated(int unit, bcm_oam_endpoint_t endpoint_id);

int
bcm_c3_oam_endpoint_allocate(int unit, bcm_oam_endpoint_t *endpoint_id, int is_with_id);

int
bcm_c3_oam_endpoint_free(int unit, bcm_oam_endpoint_t endpoint_id);
int
bcm_c3_oam_endpoint_state_set(int unit, bcm_oam_endpoint_t endpoint_id, bcm_oam_endpoint_info_t *endpoint_info);

int
bcm_c3_oam_endpoint_state_clear(int unit, bcm_oam_endpoint_t endpoint_id);

uint32
bcm_c3_oam_maid_crc32_get(uint8 *maid);

int
bcm_caladan3_oam_group_range_reserve(int unit, int reserve_hi, uint32 val);
int 
bcm_caladan3_oam_group_range_get(int unit, uint32 *low, uint32 *high);
int
bcm_caladan3_oam_endpoint_range_reserve(int unit, int reserve_hi, uint32 val);
int 
bcm_caladan3_oam_endpoint_range_get(int unit, uint32 *low, uint32 *high);
#endif  /* _BCM_INT_SBX_CALADAN3_OAM_H_  */
