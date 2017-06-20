/*
 * $Id: l2.h,v 1.21 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef _BCM_INT_ROBO_L2_H
#define _BCM_INT_ROBO_L2_H
#define BCM_L2_HASH_ENABLE	0x1
#define BCM_L2_HASH_DISABLE	0x2

typedef struct  _bcm_robo_l2_gport_params_s {
    int     param0;
    int     param1;
    uint32  type;
}_bcm_robo_l2_gport_params_t;

extern int _bcm_robo_l2_gport_parse(int unit, bcm_l2_addr_t *l2addr, 
                                   _bcm_robo_l2_gport_params_t *params);
extern int bcm_robo_l2_802dot1Q_learning_enable_set(int unit, int enable);
extern int bcm_robo_l2_802dot1Q_learning_enable_get(int unit, int *enable);
extern int bcm_robo_l2_hash_select(int unit, int hash_algorithm);

#define     L2_ROBO_MEM_CHUNKS_DEFAULT   100
typedef struct _bcm_robo_l2_traverse_s {
    void                        *pattern;   /* Pattern to match on */
    bcm_l2_addr_t               *data;      /* L2 Entry */
    int                         mem_idx;    /* Index of currently read entry */
    bcm_l2_traverse_cb          user_cb;    /* User callback function */
    void                        *user_data; /* Data provided by the user, cookie */
}_bcm_robo_l2_traverse_t;

typedef struct _bcm_robo_l2_replace_s {
    bcm_mac_t           match_mac;
    bcm_vlan_t          match_vid;
    bcm_module_t        match_module;
    bcm_port_t          match_port;
    bcm_trunk_t         match_trunk;
    int                 isTrunk;
    int                 isStatic;
    int                 isMcast;
    int                 isPending;
    int                 isDel;
    bcm_module_t        new_module;
    bcm_port_t          new_port;
    bcm_trunk_t         new_trunk;
}_bcm_robo_l2_replace_t;

/* need API approve to add this flag to be a formal BCM flag  */
#define _BCM_L2_TRAVERSE_MATCH_PENDING  (1 << 7)   /* Match L2 pending */

/* definition for all ROBO supported flags in bcm_l2_matched_traverse() */
#define _BCM_L2_ROBO_MATCH_TRAVERSE_SUPPORT_FLAGS   \
        (BCM_L2_TRAVERSE_MATCH_STATIC | BCM_L2_TRAVERSE_MATCH_MAC | \
        BCM_L2_TRAVERSE_MATCH_VLAN | BCM_L2_TRAVERSE_MATCH_DEST |   \
        BCM_L2_TRAVERSE_IGNORE_DISCARD_SRC | BCM_L2_TRAVERSE_IGNORE_DES_HIT | \
        _BCM_L2_TRAVERSE_MATCH_PENDING)

/* data structure for ROBO's L2 match traverse routine 
 *  1. op_flags is the flags to indicate the requested matching keys 
 *  2. matched_flags is the flags to report matched result.
 *
 */
typedef struct _bcm_robo_l2_match_s {
    uint32              op_flags;   /* the requested matching keys */
    bcm_mac_t           mac;  /* MAC addr for match */
    bcm_vlan_t          vid;  /* VID addr for match */
    bcm_port_t          port; /* Port addr for match */
    uint32              mc_group; /* MC_ID for match (PBMP for GEX chips) */
    uint32              match_others;   /* hit/dis_sa/dis_da/static/pending */
    uint32              result_flags;  /* to report the matched status */
}_bcm_robo_l2_match_t;

#define _BCM_ROBO_L2_MATCH_OP_MAC       (1 << 0)
#define _BCM_ROBO_L2_MATCH_OP_VID       (1 << 1)
#define _BCM_ROBO_L2_MATCH_OP_DEST      (1 << 2)  
#define _BCM_ROBO_L2_MATCH_OP_HIT       (1 << 3)    /* destination hit */
#define _BCM_ROBO_L2_MATCH_OP_DIS_SRC   (1 << 4)  
#define _BCM_ROBO_L2_MATCH_OP_DIS_DST   (1 << 5)  
#define _BCM_ROBO_L2_MATCH_OP_STATIC    (1 << 6)  
#define _BCM_ROBO_L2_MATCH_OP_PENDING   (1 << 7)  
#define _BCM_ROBO_L2_MATCH_OP_ALL     \
        (_BCM_ROBO_L2_MATCH_OP_MAC | _BCM_ROBO_L2_MATCH_OP_VID |    \
        _BCM_ROBO_L2_MATCH_OP_DEST | _BCM_ROBO_L2_MATCH_OP_HIT |    \
        _BCM_ROBO_L2_MATCH_OP_DIS_SRC | _BCM_ROBO_L2_MATCH_OP_DIS_DST |     \
        _BCM_ROBO_L2_MATCH_OP_STATIC | _BCM_ROBO_L2_MATCH_OP_PENDING) 

#define _BCM_ROBO_L2_MACHED_DIFF(op, result)    ((op) & ~(result))
#define _BCM_ROBO_L2_FULL_MACHED(op, result)     \
        ((_BCM_ROBO_L2_MACHED_DIFF((op), (result))) == 0)


/* definition to indentify the L2 travrse return type */
#define _BCM_ROBO_L2TRV_RESP_BCM_NORMAL 0x00    /* BCM_E_XXX */
#define _BCM_ROBO_L2TRV_RESP_TRAVERSED  0x01    /* traversed count */

#ifdef BCM_TB_SUPPORT
#define _TB_ARL_STATUS_VALID    0x3     /* filed value for TB only. */
#define _TB_ARL_STATUS_PENDING  0x1     /* filed value for TB only. */
#define _TB_ARL_STATUS_INVALID  0x0     /* filed value for TB only. */


/* TB's new feature about fast aging on independent control on the l2 entry 
 * with Static/Dynamic status and Unicast/Multicast type.
 *  
 *  Below define serve such feature for TB and the usage is for L2 replace 
 *  API only. The definitions below must not conflict with the 
 *  BCM_L2_REPLACE_XXX in l2.h of bcm layer.
 *
 *  P.S BCM_L2_REPLACE_MATCH_STATIC will be translated to 
 *      _BCM_TB_L2_REPLACE_MATCH_STATIC for TB support. 
 */
#define _BCM_TB_L2_REPLACE_MATCH_STATIC         0x10000
#define _BCM_TB_L2_REPLACE_MATCH_STATIC_ONLY    0x20000
#define _BCM_TB_L2_REPLACE_MATCH_MCAST          0x40000
#define _BCM_TB_L2_REPLACE_MATCH_MCAST_ONLY     0x80000

#endif  /* BCM_TB_SUPPORT */

#endif
