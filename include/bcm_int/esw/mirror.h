/*
 * $Id: mirror.h,v 1.31 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains Mirroring definitions internal to the BCM library.
 */

#ifndef _BCM_INT_MIRROR_H
#define _BCM_INT_MIRROR_H

#include <bcm/mirror.h>

#define _BCM_MIRROR_INVALID_MTP    (-1)

/* Mirroring destination managment structure. */
typedef struct _bcm_mirror_dest_config_s {
    bcm_mirror_destination_t mirror_dest; /* Mirroring destination.  */
    int                      ref_count;   /* Reference count.        */
    struct _bcm_mirror_dest_config_s *next; /* Mirroring dest config link list for shared destination */
} _bcm_mirror_dest_config_t, *_bcm_mirror_dest_config_p;

/* MTP managment structure. */
typedef struct _bcm_mtp_config_s {
    bcm_gport_t dest_id;       /* Mirroring destination id. */
    int         ref_count;     /* Reference count.          */
    uint8       egress;        /* Indicator if egress MTP or ingress */         
} _bcm_mtp_config_t, *_bcm_mtp_config_p;

/* Legacy MTP Slot managment structure. */
typedef struct _bcm_mtp_slot_config_s {
    int       ref_count;                          /* Slot Reference count.     */
    int       slot_owner;                         /* Slot Owner module.        */
} _bcm_mtp_slot_config_t, *_bcm_mtp_slot_config_p;

#define BCM_MIRROR_MTP_METHOD_NON_DIRECTED      0
#define BCM_MIRROR_MTP_METHOD_DIRECTED_LOCKED   1
#define BCM_MIRROR_MTP_METHOD_DIRECTED_FLEXIBLE 2

#define _BCM_SWITCH_MIRROR_NON_EXCLUSIVE        0
#define _BCM_SWITCH_MIRROR_EXCLUSIVE            1

/* Flags for slot check ingress/egress status */
#define _BCM_MIRROR_SLOT_INGRESS             (1 << 0) /* Ingress mirroring */
#define _BCM_MIRROR_SLOT_EGRESS              (1 << 1) /* Egress mirroring */
#define _BCM_MIRROR_SLOT_PORT                (1 << 2) /* PORT module */
#define _BCM_MIRROR_SLOT_FP                  (1 << 3) /* FP module */

/* Mirror Container */
#define _BCM_MIRROR_SLOT_CONT0              0
#define _BCM_MIRROR_SLOT_CONT1              1

/* Flags for slot management */
#define _BCM_MIRROR_SLOT_OWNER_NONE         0      /* Slot Owner is None */
#define _BCM_MIRROR_SLOT_OWNER_PORT       (1 << 0) /* Slot Owner is PORT */
#define _BCM_MIRROR_SLOT_OWNER_FP         (1 << 1) /* Slot owner is FP */

#define BCM_MIRROR_MTP_ONE                    (1 << 0)
#define BCM_MIRROR_MTP_TWO                    (1 << 1)
#define BCM_MIRROR_MTP_THREE                  (1 << 2)
#define BCM_MIRROR_MTP_FOUR                   (1 << 3)
#define BCM_MIRROR_MTP_COUNT                    4

/* Flexible MTP legacy egress Draco 1.5 mode */
#define BCM_MIRROR_MTP_FLEX_EGRESS_D15        BCM_MIRROR_MTP_THREE
#define BCM_MIRROR_MTP_FLEX_EGRESS_D15_INDEX  2

#define BCM_MIRROR_MTP_REF_PORT_MASK            0x0000FFFF
#define BCM_MIRROR_MTP_REF_FP_MASK              0xFFFF0000
#define BCM_MIRROR_MTP_REF_FP_OFFSET            16

#define BCM_XGS3_MIRROR_MTP_MASK              (BCM_MIRROR_MTP_ONE)
#define BCM_TRX_MIRROR_MTP_MASK (BCM_MIRROR_MTP_ONE | BCM_MIRROR_MTP_TWO)
#define BCM_TR2_MIRROR_MTP_MASK (BCM_MIRROR_MTP_ONE | BCM_MIRROR_MTP_TWO | \
                                 BCM_MIRROR_MTP_THREE | BCM_MIRROR_MTP_FOUR)

#define BCM_MIRROR_MTP_ITER(_mask,_slot) \
    for ((_slot) = 0; (_slot) < BCM_MIRROR_MTP_COUNT; (_slot)++)  \
        if ((_mask) & (1 << (_slot)))

#define BCM_MIRROR_MTP_FLEX_SLOT_SHIFT        16
#define BCM_MIRROR_MTP_FLEX_SLOT_MASK         0x3

#define BCM_MTP_SLOT_TYPE_PORT                0
#define BCM_MTP_SLOT_TYPE_FP                  1
#define BCM_MTP_SLOT_TYPE_IPFIX               2
#define BCM_MTP_SLOT_TYPE_SFLOW               3 
#define BCM_MTP_SLOT_TYPES                    4

#define BCM_MIRROR_DEST_TUNNELS  (BCM_MIRROR_DEST_TUNNEL_L2 |\
                                  BCM_MIRROR_DEST_TUNNEL_IP_GRE |\
                                  BCM_MIRROR_DEST_TUNNEL_TRILL  |\
                                  BCM_MIRROR_DEST_TUNNEL_NIV |\
                                  BCM_MIRROR_DEST_TUNNEL_ETAG |\
                                  BCM_MIRROR_DEST_TUNNEL_SFLOW)  


#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_GREYHOUND_SUPPORT)
#define BCM_TD_MIRROR_TRUNK_MAX_PORTCNT       8  

#define BCM_TD_MIRROR_ENCAP_TYPE_RSPAN        0  
#define BCM_TD_MIRROR_ENCAP_TYPE_ERSPAN       1  
#define BCM_TD_MIRROR_ENCAP_TYPE_SFLOW        2

#define BCM_TD_MIRROR_HEADER_ONLY             0
#define BCM_TD_MIRROR_HEADER_VNTAG            1
#define BCM_TD_MIRROR_HEADER_TRILL            2

#define BCM_TD_MIRROR_TRILL_VERSION           0
/* bits  31 and 30 of the TRILL header is the version */
#define BCM_TD_MIRROR_TRILL_VERSION_OFFSET    29
/* Bits 24..16 of the TRILL header is the hopecount */
#define BCM_TD_MIRROR_TRILL_HOPCOUNT_OFFSET   16
/* Bits 0..15 is the TRILL source name and TRILL destination name 
(Ingress RBridge Nickname) is positioned on bits 31..16 of the second word. */
#define BCM_TD_MIRROR_TRILL_DEST_NAME_OFFSET  16

/* 20 byte IPv4 header = 5 words */
#define _BCM_TD_MIRROR_V4_GRE_BUFFER_SZ  (0x5)

/* 6 bytes TRILL header, no options = 2 words */
#define _BCM_TD_MIRROR_TRILL_BUFFER_SZ  (0x2)  

#define _BCM_TD_MIRROR_TRILL_NAME_MASK       (0xffff)
#define _BCM_TD_MIRROR_TRILL_HOPCOUNT_MASK  (0xff)


#define _BCM_TD_MIRROR_NIV_LOOP_BIT         (0x8000)  
#define _BCM_TD_MIRROR_NIV_DST_VIF_OFFSET   16
#define _BCM_TD_MIRROR_NIV_SRC_VIF_MASK     (0xfff)
#define _BCM_TD_MIRROR_NIV_DST_VIF_MASK     (0xffff)
#endif


#define BCM_MIRROR_HEADER_ETAG            3

#define _BCM_MIRROR_ETAG_SRC_VID_OFFSET  16
#define _BCM_MIRROR_ETAG_SRC_VID_MASK    (0xfff)
#define _BCM_MIRROR_ETAG_DST_VID_MASK    (0x3fff)


/* Module control structure. */
typedef struct _bcm_mirror_config_s {
    int                       mtp_method;     /* Non-directed,
                                               * directed-locked, or
                                               * directed-flexible */
    int                       mode;           /* L2/L2_L3/DISABLED.        */
    _bcm_mirror_dest_config_p dest_arr;       /* Mirror destination array  */
    uint8                     dest_count;     /* Mirror destinations size. */
    _bcm_mtp_config_p         ing_mtp;        /* Ingress mtp array.        */
    uint8                     ing_mtp_count;  /* Ingress mtp array size.   */
    _bcm_mtp_config_p         egr_mtp;        /* Egress mtp array.         */
    uint8                     egr_mtp_count;  /* Egress mtp array size.    */
    _bcm_mtp_config_p         shared_mtp;     /* MTP array shared for Ing and Egr   */
    uint8                     port_im_mtp_count; /* Maximal ingress mtp allowed per port */
    uint8                     port_em_mtp_count; /* Maximal egress mtp allowed per port */
    uint8                     mtp_dev_mask;   /* MTPs allowed on device    */
    uint8                     mtp_mode_bmp;   /* MTP ingress/egress bitmap */
    int                       mtp_mode_ref_count[BCM_MIRROR_MTP_COUNT];
                                              /* MTP container references  */
    _bcm_mtp_config_p         egr_true_mtp;   /* True egress mtp array.    */
    uint8                     egr_true_mtp_count;  /* True egress mtp array size. */
    _bcm_mtp_config_p         mtp_slot[BCM_MTP_SLOT_TYPES];
                                              /* FP/other mtp slot arrays. */
    uint8                     mtp_slot_count[BCM_MTP_SLOT_TYPES];
                                          /* FP/other mtp slot array size. */
    sal_mutex_t               mutex;          /* Protection mutex.         */
    bcm_pbmp_t                pbmp_mtp_slot_used[BCM_MIRROR_MTP_COUNT];
                                         /* pbmp of mtp slot which is used */
    _bcm_mtp_slot_config_p    ing_slot_container;
                                              /* Ingress Legacy Mirror Slot
                                                 reference. Used only for devices
                                                 not supporting flex mirroring */
    _bcm_mtp_slot_config_p    egr_slot_container;
                                              /* Egress Mirror Slot
                                                 reference. Used only for devices
                                                 not supporting flex mirroring */
} _bcm_mirror_config_t, *_bcm_mirror_config_p;


#define MIRROR_INIT(_unit_) (NULL != _bcm_mirror_config[(_unit_)])
#define MIRROR_CONFIG(_unit_) (_bcm_mirror_config[(_unit_)])
#define MIRROR_CONFIG_MODE(_unit_)  ((MIRROR_CONFIG(_unit_))->mode)

#define MIRROR_CONFIG_MTP_METHOD(_unit_)  \
        ((MIRROR_CONFIG(_unit_))->mtp_method)

#define MIRROR_MTP_METHOD_IS_NON_DIRECTED(unit) \
        (BCM_MIRROR_MTP_METHOD_NON_DIRECTED == \
                    MIRROR_CONFIG_MTP_METHOD(unit))

#define MIRROR_MTP_METHOD_IS_DIRECTED_LOCKED(unit) \
        (BCM_MIRROR_MTP_METHOD_DIRECTED_LOCKED == \
                    MIRROR_CONFIG_MTP_METHOD(unit))

#define MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit) \
        (BCM_MIRROR_MTP_METHOD_DIRECTED_FLEXIBLE == \
                    MIRROR_CONFIG_MTP_METHOD(unit))

#define MIRROR_SWITCH_IS_EXCLUSIVE(unit) \
        ((_BCM_SWITCH_MIRROR_EXCLUSIVE == \
             _bcm_switch_mirror_exclusive_config[unit]) ? TRUE : FALSE)

#define MIRROR_CONFIG_ING_MTP(_unit_, _idx_) \
    ((MIRROR_CONFIG(_unit_))->ing_mtp[(_idx_)])

#define MIRROR_CONFIG_EGR_MTP(_unit_, _idx_) \
    ((MIRROR_CONFIG(_unit_))->egr_mtp[(_idx_)])

#define MIRROR_CONFIG_EGR_TRUE_MTP(_unit_, _idx_) \
    ((MIRROR_CONFIG(_unit_))->egr_true_mtp[(_idx_)])

#define MIRROR_CONFIG_TYPE_MTP(_unit_, _idx_, _type_) \
    (((MIRROR_CONFIG(_unit_))->mtp_slot[(_type_)])[(_idx_)])

#define MIRROR_CONFIG_ING_MTP_SLOT(_unit_, _slotidx_) \
     ((MIRROR_CONFIG(_unit_))->ing_slot_container[(_slotidx_)])

#define MIRROR_CONFIG_EGR_MTP_SLOT(_unit_, _slotidx_) \
     ((MIRROR_CONFIG(_unit_))->egr_slot_container[(_slotidx_)])

#define MIRROR_CONFIG_ING_MTP_SLOT_OWNER(_unit_, _slotidx_) \
     MIRROR_CONFIG_ING_MTP_SLOT((_unit_),(_slotidx_)).slot_owner

#define MIRROR_CONFIG_EGR_MTP_SLOT_OWNER(_unit_, _slotidx_) \
      MIRROR_CONFIG_EGR_MTP_SLOT((_unit_),(_slotidx_)).slot_owner

#define MIRROR_CONFIG_ING_MTP_SLOT_REF(_unit_, _slotidx_) \
    MIRROR_CONFIG_ING_MTP_SLOT((_unit_),(_slotidx_)).ref_count

#define MIRROR_CONFIG_EGR_MTP_SLOT_REF(_unit_, _slotidx_) \
    MIRROR_CONFIG_EGR_MTP_SLOT((_unit_),(_slotidx_)).ref_count

#define MIRROR_CONFIG_MTP(_unit_, _idx_, _flags_) \
    ((soc_feature(_unit_, soc_feature_mirror_flexible) &&    \
      !MIRROR_MTP_METHOD_IS_DIRECTED_FLEXIBLE(unit)) ?       \
     ((_flags_ & (BCM_MIRROR_PORT_INGRESS |                  \
                  BCM_MIRROR_PORT_EGRESS)) ?                 \
      &(MIRROR_CONFIG_SHARED_MTP(_unit_, _idx_)) :           \
      &(MIRROR_CONFIG_EGR_TRUE_MTP(_unit_, _idx_))) :        \
     ((_flags_ & BCM_MIRROR_PORT_INGRESS) ?                  \
      &MIRROR_CONFIG_ING_MTP(_unit_, _idx_) :                \
      ((_flags_ & BCM_MIRROR_PORT_EGRESS) ?                  \
       &MIRROR_CONFIG_EGR_MTP(_unit_, _idx_) :               \
       &MIRROR_CONFIG_EGR_TRUE_MTP(_unit_, _idx_))))

#define MIRROR_CONFIG_SHARED_MTP(_unit_, _idx_) \
    ((MIRROR_CONFIG(_unit_))->shared_mtp[(_idx_)])

#define MIRROR_CONFIG_ING_MTP_DEST(_unit_, _idx_) \
    MIRROR_CONFIG_ING_MTP((_unit_), (_idx_)).dest_id

#define MIRROR_CONFIG_EGR_MTP_DEST(_unit_, _idx_) \
    MIRROR_CONFIG_EGR_MTP((_unit_),(_idx_)).dest_id

#define MIRROR_CONFIG_EGR_TRUE_MTP_DEST(_unit_, _idx_) \
    MIRROR_CONFIG_EGR_TRUE_MTP((_unit_),(_idx_)).dest_id

#define MIRROR_CONFIG_SHARED_MTP_DEST(_unit_, _idx_) \
    MIRROR_CONFIG_SHARED_MTP((_unit_),(_idx_)).dest_id

#define MIRROR_CONFIG_MTP_DEV_MASK(_unit_) \
    ((MIRROR_CONFIG(_unit_))->mtp_dev_mask)

#define MIRROR_CONFIG_EGR_MTP_REF_COUNT(_unit_, _idx_) \
    MIRROR_CONFIG_EGR_MTP((_unit_),(_idx_)).ref_count

#define MIRROR_CONFIG_EGR_TRUE_MTP_REF_COUNT(_unit_, _idx_) \
    MIRROR_CONFIG_EGR_TRUE_MTP((_unit_),(_idx_)).ref_count

#define MIRROR_CONFIG_SHARED_MTP_REF_COUNT(_unit_, _idx_) \
    MIRROR_CONFIG_SHARED_MTP((_unit_),(_idx_)).ref_count

#define MIRROR_CONFIG_MTP_MODE_BMP(_unit_) \
    ((MIRROR_CONFIG(_unit_))->mtp_mode_bmp)

#define MIRROR_CONFIG_ING_MTP_REF_COUNT(_unit_, _idx_) \
    MIRROR_CONFIG_ING_MTP((_unit_), (_idx_)).ref_count

#define MIRROR_CONFIG_MTP_MODE_REF_COUNT(_unit_, _idx_) \
    ((MIRROR_CONFIG(_unit_))->mtp_mode_ref_count[(_idx_)])

#define MIRROR_CONFIG_TYPE_MTP_SLOT(_unit_, _idx_, _type_) \
    MIRROR_CONFIG_TYPE_MTP((_unit_),(_idx_),(_type_)).dest_id

#define MIRROR_CONFIG_TYPE_MTP_REF_COUNT(_unit_, _idx_, _type_) \
    MIRROR_CONFIG_TYPE_MTP((_unit_),(_idx_),(_type_)).ref_count

#define MIRROR_CONFIG_PBMP_MTP_SLOT_USED(_unit_, _idx_) \
    ((MIRROR_CONFIG(_unit_))->pbmp_mtp_slot_used[(_idx_)])

#define MIRROR_DEST_CONFIG_COUNT(_unit_) \
    ((MIRROR_CONFIG(_unit_))->dest_count)

#define MIRROR_DEST_CONFIG(_unit_, _idx_) \
    ((MIRROR_CONFIG(_unit_))->dest_arr[BCM_GPORT_MIRROR_GET(_idx_)])

#define MIRROR_DEST(_unit_, _idx_) \
    (&MIRROR_DEST_CONFIG((_unit_),(_idx_)).mirror_dest)

#define MIRROR_DEST_GPORT(_unit_, _idx_) \
    (MIRROR_DEST((_unit_), (_idx_))->gport)

#define MIRROR_DEST_REF_COUNT(_unit_, _idx_) \
    (MIRROR_DEST_CONFIG((_unit_), (_idx_)).ref_count)

#define MIRROR_LOCK(_unit_)  \
            sal_mutex_take(MIRROR_CONFIG(_unit_)->mutex, sal_mutex_FOREVER)
#define MIRROR_UNLOCK(_unit_)   sal_mutex_give(MIRROR_CONFIG(_unit_)->mutex)
extern int _bcm_esw_mirror_fp_dest_add(int unit, int modid, int port, 
                                       uint32 flags, int *mtp_index);

extern int _bcm_esw_mirror_fp_dest_delete(int unit, int mtp_index, uint32 flags);

extern int _bcm_esw_mirror_stk_update(int unit, bcm_module_t modid, bcm_port_t port,
                                      bcm_pbmp_t pbmp);
extern int _bcm_esw_mirror_flexible_get(int unit, int *enable);
extern int _bcm_esw_mirror_flexible_set(int unit, int enable);
extern int bcm_esw_mirror_deinit(int unit);

extern void bcm_esw_mirror_lock(int unit);
extern void bcm_esw_mirror_unlock(int unit);

extern int _bcm_esw_mtp_slot_valid_get(int unit,
                                       uint32 flags,int *mtp_slot_status);
extern int _bcm_esw_mirror_fp_slot_add_ref(int unit,
                                           uint32 flags,
                                           uint32 mtp_slot);
extern int _bcm_esw_mirror_fp_slot_del_ref(int unit,
                                           uint32 flags,
                                           uint32 mtp_slot);
extern int _bcm_esw_mirror_exclusive_get(int unit, int *enable);
extern int _bcm_esw_mirror_exclusive_set(int unit, int enable);
extern int _bcm_esw_mirror_enable_set(int unit, int port, int enable);
extern int _bcm_esw_mirror_mtp_match(int unit, bcm_gport_t gport, uint32 flags,
                                     int *match_idx);
extern int _bcm_esw_mirror_mtp_reserve(int unit, bcm_gport_t dest_id,
                                       uint32 flags, int *index_used);
extern int _bcm_esw_mirror_mtp_unreserve(int unit, int mtp_index, int is_port,
                                         uint32 flags);
extern int
bcmi_esw_mirror_port_attach(int unit, bcm_port_t port);
extern int
bcmi_esw_mirror_port_detach(int unit, bcm_port_t port);

#if defined(BCM_TRX_SUPPORT)
/* 18 byte L2 header + 20 byte IPv4 header + 4 byte GRE header = 11 words */
#define _BCM_TRX_MIRROR_TUNNEL_BUFFER_SZ  (0x11) 
#endif /* BCM_TRX_SUPPORT */
#if defined(BCM_XGS3_SWITCH_SUPPORT)
extern int _bcm_xgs3_mirror_trunk_update(int unit, bcm_trunk_t tid);
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
extern int _bcm_esw_mirror_sync(int unit);
extern int _bcm_esw_mirror_mtp_to_modport(int unit, int mtp_index,
                                          int modport, int flags,
                                          bcm_module_t *modid,
                                          bcm_gport_t *port);
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
extern void _bcm_mirror_sw_dump(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#endif	/* !_BCM_INT_MIRROR_H */
